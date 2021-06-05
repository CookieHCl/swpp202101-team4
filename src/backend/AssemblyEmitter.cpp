#include "Backend.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

string AssemblyEmitter::name(Value* v) {
    if(!v || isa<ConstantPointerNull>(v) || v->getType()->isVoidTy()) {
        return "0";
    }
    if(isa<ConstantInt>(v)) {
        //return the value itself.
        return to_string(dyn_cast<ConstantInt>(v)->getZExtValue());
    }
    return SM->get(v)->getName();
}

//static functions for emitting common formats.
string AssemblyEmitter::emitInst(vector<string> printlist) {
    string str = "  ";
    for(string s : printlist) {
        str += s + " ";
    }
    str += "\n";
    return str;
}
string AssemblyEmitter::emitBinary(Instruction* v, string opcode, string op1, string op2) {
    return emitInst({name(v), "=", opcode, op1, op2, stringBandWidth(v)});
}
string AssemblyEmitter::emitCopy(Instruction* v, Value* op) {
    Memory* mem = SM->get(op)? SM->get(op)->castToMemory() : NULL;
    if(mem) {
        if(mem->getBase() == TM->gvp()) {
            return emitBinary(v, "add", "204800", to_string(mem->getOffset()));    
        }
        return emitBinary(v, "add", mem->getBase()->getName(), to_string(mem->getOffset()));
    }
    return emitBinary(v, "mul", name(op), "1");
}

string AssemblyEmitter::stringBandWidth(Value* v) {
    if(isa<Function>(v) || isa<BasicBlock>(v)) {
        assert(false && "v should be a digit-typed value");
    }
    return to_string(getBitWidth(v->getType()));
}

AssemblyEmitter::AssemblyEmitter(raw_ostream *fout, TargetMachine& TM, SymbolMap& SM, map<Function*, unsigned>& spOffset) :
            fout(fout), TM(&TM), SM(&SM), spOffset(spOffset) {}

void AssemblyEmitter::visitFunction(Function& F) {
    //print the starting code.
    //finishing code will be printed outside the AssemblyEmitter.
    *fout << "start " << name(&F) << " " << F.arg_size() << ":\n";
}
void AssemblyEmitter::visitBasicBlock(BasicBlock& BB) {
    *fout << "." << name(&BB) << ":\n";

    //If entry block, modify SP.
    if(&(BB.getParent()->getEntryBlock()) == &BB) {
        //if main, import GV within.
        //this code should happen only if GV array was in the initial program.
        //GV values are all lowered into alloca + calls
        if(BB.getParent()->getName() == "main" && BB.getModule()->getGlobalList().size()!=0) {
            *fout << "  ; Init global variables\n";
            for(auto& gv : BB.getModule()->globals()) {
                //temporarily stores the GV pointer.
                unsigned size = (getAccessSize(gv.getValueType()) + 7) / 8 * 8;
                *fout << emitInst({"r1 = malloc", to_string(size)});
                if(gv.hasInitializer() && !gv.getInitializer()->isZeroValue()) {
                    *fout << emitInst({"store", to_string(getAccessSize(gv.getValueType())), name(gv.getInitializer()), "r1 0"});
                }
            }
        }
        if(spOffset[BB.getParent()] != 0) {
            *fout << "  ; Init stack pointer\n";
            *fout << emitInst({"sp = sub sp",to_string(spOffset[BB.getParent()]),"64"});
        }
    }
}

//Compare insts.
void AssemblyEmitter::visitICmpInst(ICmpInst& I) {
    *fout << emitInst({name(&I), "= icmp", I.getPredicateName(I.getPredicate()).str(), name(I.getOperand(0)), name(I.getOperand(1)), stringBandWidth(I.getOperand(0))});
}

//Alloca inst.
void AssemblyEmitter::visitAllocaInst(AllocaInst& I) {
    //Do nothing.
}

//Memory Access insts.
void AssemblyEmitter::visitLoadInst(LoadInst& I) {
    Value* ptr = I.getPointerOperand();
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->sp()) {
            *fout << emitInst({name(&I), "= load", size ,"sp", to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({name(&I), "= load", size, "204800", to_string(mem->getOffset())});
        }
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        *fout << emitInst({name(&I), "= load", size, reg->getName(), "0"});
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}
void AssemblyEmitter::visitStoreInst(StoreInst& I) {
    Value* ptr = I.getPointerOperand();
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Value* val = I.getValueOperand();
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->sp()) {
            *fout << emitInst({"store", size, name(val), "sp", to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({"store", size, name(val), "204800", to_string(mem->getOffset())});
        }
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}

//PHI Node inst.
void AssemblyEmitter::visitPHINode(PHINode& I) {
    //Do nothing.
}

//Reformatting(no value changes) insts.
void AssemblyEmitter::visitTruncInst(TruncInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitZExtInst(ZExtInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitSExtInst(SExtInst& I) {
    unsigned beforeBits = getBitWidth(I.getOperand(0)->getType());
    unsigned afterBits = getBitWidth(I.getType());
    assert(afterBits > beforeBits && "SExt must increase the bandwidth");
    *fout << emitBinary(&I, "mul", name(I.getOperand(0)), to_string(1llu<<(afterBits-beforeBits)));
    *fout << emitBinary(&I, "sdiv", name(&I), to_string(1llu<<(afterBits-beforeBits)));
}
void AssemblyEmitter::visitPtrToIntInst(PtrToIntInst& I) {
    Value* ptr = I.getPointerOperand();
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(symbol) {
        if(Memory* mem = symbol->castToMemory()) {
            if(mem->getBase() == TM->sp()) {
                *fout << emitBinary(&I, "add", "sp", to_string(mem->getOffset()));
            }
            else if(mem->getBase() == TM->gvp()) {
                *fout << emitBinary(&I, "add", "204800", to_string(mem->getOffset()));
            }
            else assert(false && "base of memory pointers should be sp or gvp");
        }
        //else a pointer stored in register,
        else if(Register* reg = symbol->castToRegister()) {
            //if from and to values are stored in a different source, copy.
            if(SM->get(&I) != SM->get(I.getOperand(0))) {
                *fout << emitCopy(&I, I.getOperand(0));
            }
        }
        return;
    }
    //else ptr is null
    if(isa<ConstantPointerNull>(ptr)) {
        *fout << emitBinary(&I, "mul", "0", "0");
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}
void AssemblyEmitter::visitIntToPtrInst(IntToPtrInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitBitCastInst(BitCastInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}

//Select inst.
void AssemblyEmitter::visitSelectInst(SelectInst& I) {
    *fout << emitInst({name(&I), "= select", name(I.getCondition()), name(I.getTrueValue()), name(I.getFalseValue())});
}

void AssemblyEmitter::visitCallInst(CallInst& I) {
    //Process malloc()&free() from other plain call insts.
    Function* F = I.getCalledFunction();
    string Fname = F->getName().str();
    
    //Collect all arguments
    vector<string> args;
    for(Use& arg : I.args()) {
        args.push_back(name(arg.get()));
    }
    if(Fname == "malloc") {
        assert(args.size()==1 && "argument of malloc() should be 1");
        *fout << emitInst({name(&I), "= malloc", name(I.getArgOperand(0))});
    }
    else if(Fname == "free") {
        assert(args.size()==1 && "argument of free() should be 1");
        *fout << emitInst({"free", name(I.getArgOperand(0))});
    }
    else if(Fname == "____stackalloc") {
        assert(args.size()==1 && "argument of ____stackalloc() should be 1");
        *fout << emitInst({"sp = sub sp", name(I.getArgOperand(0)), "64"})
            << emitInst({name(&I), "= mul sp 1 64"});
    }
    else if(UnfoldVectorInstPass::VLOADS.find(Fname) != UnfoldVectorInstPass::VLOADS.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto it = I.getIterator();
        for (it++; ;) {
            CallInst *NI = dyn_cast<CallInst>(&*it++);
            if (NI == NULL) break;

            string niFn = NI->getCalledFunction()->getName().str();
            if (niFn != "extract_element2" && niFn != "extract_element4" && niFn != "extract_element8") break;

            ConstantInt *C = dyn_cast<ConstantInt>(NI->getOperand(1));
            assert(C != NULL && "extract_element should retrieve a constant argument as a dim");

            int pos = C->getZExtValue();
            assert(0 <= pos && pos < asmb.size());

            asmb[pos] = name(NI);
        }

        asmb.push_back("= vload");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        asmb.push_back(name(I.getOperand(0))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::VSTORES.find(Fname) != UnfoldVectorInstPass::VSTORES.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());

        asmb.push_back("vstore");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto *maskVal = dyn_cast<ConstantInt>(I.getOperand(n + 1));
        assert(maskVal != NULL && "mask should be a constant integer");

        int mask = maskVal->getZExtValue();
        assert(0 <= mask && mask < (1 << n) && "invalid mask number");

        for (int i=0; i<n; i++) {
            if (!(mask & (1 << i))) continue;

            Value *V = I.getOperand(i);
            asmb[2+i] = name(V);
        }

        asmb.push_back(name(I.getOperand(n))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::EXTRACT_ELEMENTS.find(Fname) != UnfoldVectorInstPass::EXTRACT_ELEMENTS.end()) {
        // do nothing
    }
	else if(F->getReturnType()->isVoidTy()) {
		vector<string> printlist = {"call", Fname};
		printlist.insert(printlist.end(), args.begin(), args.end());
		*fout << emitInst(printlist);
	}
    //ordinary function calls.
    else {
        vector<string> printlist = {name(&I), "= call", Fname};
        printlist.insert(printlist.end(), args.begin(), args.end());
        *fout << emitInst(printlist);
    }
}

//Terminator insts.
void AssemblyEmitter::visitReturnInst(ReturnInst& I) {
    //increase sp(which was decreased in the beginning of the function.)
    Function* F = I.getFunction();
    if(spOffset[F] > 0) {
        *fout << emitInst({"sp = add sp",to_string(spOffset[F]),"64"});
    }
    *fout << emitInst({"ret", name(I.getReturnValue())});
}
void AssemblyEmitter::visitBranchInst(BranchInst& I) {
    if(I.isConditional()) {
        assert(I.getNumSuccessors() == 2 && "conditional branches must have 2 successors");
        *fout << emitInst({"br", name(I.getCondition()), "." + name(I.getSuccessor(0)), "." + name(I.getSuccessor(1))});
    }
    else {
        assert(I.getNumSuccessors() == 1 && "unconditional branches must have 1 successor");
        *fout << emitInst({"br", "." + name(I.getSuccessor(0))});
    }
}
void AssemblyEmitter::visitSwitchInst(SwitchInst& I) {
    string asmb("switch " + name(I.getCondition()));
    for(auto& c : I.cases()) {
        if(c.getCaseIndex() == I.case_default()->getCaseIndex()) continue;
        asmb.append(" " + name(c.getCaseValue()) + " ." + name(c.getCaseSuccessor()));
    }
    asmb.append(" ." + name(I.case_default()->getCaseSuccessor()));
    *fout << asmb << "\n";
}
void AssemblyEmitter::visitBinaryOperator(BinaryOperator& I) {
    string opcode = "";
    switch(I.getOpcode()) {
    case Instruction::UDiv: opcode = "udiv"; break;
    case Instruction::SDiv: opcode = "sdiv"; break;
    case Instruction::URem: opcode = "urem"; break;
    case Instruction::SRem: opcode = "srem"; break;
    case Instruction::Mul:  opcode = "mul"; break;
    case Instruction::Shl:  opcode = "shl"; break;
    case Instruction::AShr: opcode = "ashr"; break;
    case Instruction::LShr: opcode = "lshr"; break;
    case Instruction::And:  opcode = "and"; break;
    case Instruction::Or:   opcode = "or"; break;
    case Instruction::Xor:  opcode = "xor"; break;
    case Instruction::Add:  opcode = "add"; break;
    case Instruction::Sub:  opcode = "sub"; break;
    default: assert(false && "undefined binary operation");
    }

    *fout << emitBinary(&I, opcode, name(I.getOperand(0)), name(I.getOperand(1)));
}

}