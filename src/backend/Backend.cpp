
#include "Backend.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_os_ostream.h"
#include <string>
#include <sstream>

using namespace llvm;
using namespace std;
using namespace backend;

namespace backend {

// Return sizeof(T) in bytes.
unsigned getAccessSize(Type *T) {
  if (isa<PointerType>(T))
    return 8;
  else if (isa<IntegerType>(T)) {
    return T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8);
  } else if (isa<ArrayType>(T)) {
    return getAccessSize(T->getArrayElementType()) * T->getArrayNumElements();
  }
  assert(false && "Unsupported access size type!");
}

unsigned getBitWidth(Type* T) {
  if (isa<PointerType>(T))
    return 64;
  else if (isa<IntegerType>(T)) {
    return T->getIntegerBitWidth();
  }
  assert(false && "Unsupported access size type!");
}

//---------------------------------------------------------------
//class Backend
//---------------------------------------------------------------

PreservedAnalyses Backend::run(Module &M, ModuleAnalysisManager &MAM) {
  if (verifyModule(M, &errs(), nullptr)) {
    errs() << "Syntax error: Input module is not syntactically correct";
    exit(1);
  }
  
  RegisterGraph RG(M);

  //Build symbol table and rename all.
  //Every values are mapped to the symbol table.
  SymbolMap symbolMap(&M, TM, RG);

  //Process alloca, stack pointer and related instructions.
  //- Allocas should reside only on the entry block.
  //- entry block should not have a predecessor block.
  //   => Else, raise error.
  //- allocas and its direct uses are renamed: __st__offset__ ex) __st__8__
  map<Function*, unsigned> spOffsetMap = processAlloca(M, symbolMap);

/*
  TODO : features to be implemented in PR2
  //SSA is eliminated.
  //phi node is yet not deleted(assembly emitter will delete it),
  //but we ensure that every reg in phi nodes point to same registers.
  //  ex) __r3__.7 = phi [%__r3__.2, %BB1], [%__r3__.0, %BB2], [%__arg0__, %BB3]
  //  NOT) __r3__.7 = phi [%__r2__.2, %BB1], [%__r5__.0, %BB2], [%__arg0__, %BB3]
  */
  SSAElimination(M, symbolMap, RG);

  ElimBackendMovInst(M, symbolMap);
  
  if (verifyModule(M, &errs(), nullptr)) {
    errs() << "BUG: SSAElimination created an ill-formed module!\n";
    errs() << M;
    exit(1);
  }

  //Debug code
  if(printProcess) {
    //Print GV. All GVs should be mapped.
    for(auto& glv : M.globals()) {
      outs() << "(" << symbolMap.get(&glv)->getName() << ")" << glv.getName() << "\n";
    }

    for(auto& F : M) {
      //Print function & argument. All arguments should be mapped.
      outs() << F.getName() << "(";
      for(auto& arg : F.args()) {
        outs() << " (" << symbolMap.get(&arg)->getName() << ")" << arg.getName();
      }
      outs() << " )\n";

      //Print Instructions. All instructions should be mapped.
      for(auto& BB : F) {
        outs() << "  " << BB.getName() << "\n";
        for(auto& I : BB) {
          if(isa<AllocaInst>(I)) {
            outs() << "    (" << symbolMap.get(&I)->getName() << ")" << I << "\n";
          }
          else if (RG.doNotConsider(I)) {
            outs() << "  ";
            I.print(outs());
            outs() << "\n";
          }
          else
            outs() << "    (" << symbolMap.get(&I)->getName() << ")" << I << "\n";
        }
      }
    }
    M.print(outs(), nullptr);
  } 
  
  // Now, let's emit assembly!
  error_code EC;
  raw_ostream *os =
    outputFile == "-" ? &outs() : new raw_fd_ostream(outputFile, EC);

  if (EC) {
    errs() << "Cannot open file: " << outputFile << "\n";
    exit(1);
  }

  AssemblyEmitter Emitter(os, TM, symbolMap, spOffsetMap);
  for(Function& F : M){
    if(F.isDeclaration()) continue;
    Emitter.visit(F);
    *os << "end " << symbolMap.get(&F)->getName() << "\n\n";
  }

  if (os != &outs()) delete os;
  
  return PreservedAnalyses::all();
}

// Author: Deokin Jeong (Code style may be different)
// For each basic block, construct a directed graph from branch instruction and phi nodes.
// If we reverse direction of all edges in graph, then graph forms a functional graph.
// Cause every vertex of functional graph has only one outgoing edge, it is easier to 
// traverse graph.
// Graph traverse starts from vertex without an incoming edge, and ends to loop.
// If we detect a loop, there are two options for this.
//   (i)  If there is not used (physical) register, then use it as temporary register to 
//        move values.
//   (ii) Otherwise, just repeat (xor)swapping two registers.
// Following function implements above explanation.
void Backend::SSAElimination(Module &M, SymbolMap &symbolMap, RegisterGraph& RG) {
    LLVMContext &Context = M.getContext();
    for(Function &F : M) {
        if(F.isDeclaration()) {
            continue;
        }
        for(BasicBlock &BB : F) {
            // To represent graph, following vector(adjacent list) is used.
            vector<vector<Symbol *>> adjList(32);
            Instruction *I = BB.getTerminator();
            // If terminator is return statement, then pass it.
            if(isa<ReturnInst>(I)) {
                continue;
            }
            // For every successor, find phi nodes from BB and construct graph.
            for(unsigned i = 0; i < I->getNumSuccessors(); i++) {
                addEdges(BB, *(I->getSuccessor(i)), symbolMap, adjList);
            }

            // To store the number of incoming edge.
            // If there is u -> v edge in graph, it means the value of register v
            // should move to register u.
            
            vector<unsigned> indegree(32, 0), outdegree(32, 0);
            set<int> unused;
            for(unsigned i = 0; i < 32; i++) {
                bool added[32] = {0,};
                for(auto &there : adjList[i]) {
                    int reg = TM.regNo(there);
                    if(reg != -1 && !added[reg]) {
                        indegree[reg]++;
                        outdegree[i]++;
                        added[reg] = true;
                    }
                }
                // If a register has no outgoing edge, store and use as temporary register.
                if(i >= RG.getNumColors(&F)) {
                    unused.insert(i);
                }
            }
            // validate phi elim graph
            for (unsigned i=0; i<32; i++) {
              assert(outdegree[i] < 2);
            }
            
            // countRest represents the number of untracked vertices, 
            // but this is useless (until now).
            int countRest = 32 - (int)unused.size();

            // Use queue to traverse graph
            queue<unsigned> q;
            for(unsigned i = 0; i < 32; i++) {
                if(!indegree[i] && unused.find(i) == unused.end()) {
                    q.push(i);
                }
            }
            while(!q.empty()) {
                unsigned curReg = q.front();
                q.pop();
                countRest--;
                if(adjList[curReg].empty()) {
                    continue;
                }
                // findLeastReg() finds endmost register which is allocated to r(adjList[curReg][0]). 
                Value *value = findLeastReg(adjList[curReg][0], BB, symbolMap);
                
                // Create new Mul instruction to move value to curReg.
                // TODO: Fix below APInt(32, 1) to proper data type.
                if(value->getType()->isPointerTy()) {
                    Instruction *ptoi = CastInst::CreateBitOrPointerCast(value, IntegerType::getInt64Ty(Context));
                    ptoi->insertBefore(BB.getTerminator());
                    symbolMap.set(ptoi, TM.reg(curReg));

                    Instruction *itop = CastInst::CreateBitOrPointerCast(ptoi, value->getType());
                    itop->insertBefore(BB.getTerminator());
                    symbolMap.set(itop, TM.reg(curReg));
                } else {
                    Instruction *moveInst = BinaryOperator::CreateMul(value, ConstantInt::get(Context, APInt(value->getType()->getIntegerBitWidth(), 1)));
                    moveInst->insertBefore(BB.getTerminator());
                    symbolMap.set(moveInst, TM.reg(curReg));
                }
                
                if(TM.regNo(adjList[curReg][0]) != -1
                && !--indegree[TM.regNo(adjList[curReg][0])]) {
                    q.push(TM.regNo(adjList[curReg][0]));
                }
            }
            // countRest is under 1 if graph has no loop or only one self loop.
            if(countRest <= 1) {
                continue;
            } else if(unused.size() > 0) { // There is an extra register to use.
                int registerNum = *unused.begin();
                for(unsigned i = 0; i < 32; i++) {
                    // indegree[i] > 0 means register i is part of the loop.
                    if(indegree[i] > 0) {
                        // Move the value of register i to temporary register.
                        q.push(i);
                        Value *value = findLeastReg(TM.reg(i), BB, symbolMap);
                        Instruction *moveToTemp;
                        if(value->getType()->isPointerTy()) {
                            Instruction *ptoi = CastInst::CreatePointerCast(value, IntegerType::getInt64Ty(Context));
                            ptoi->insertBefore(BB.getTerminator());
                            symbolMap.set(ptoi, TM.reg(registerNum));

                            moveToTemp = ptoi;

                            Instruction *itop = CastInst::CreateBitOrPointerCast(ptoi, value->getType());
                            itop->insertBefore(BB.getTerminator());
                            symbolMap.set(itop, TM.reg(registerNum));
                        } else {
                            moveToTemp = BinaryOperator::CreateMul(value, ConstantInt::get(Context, APInt(value->getType()->getIntegerBitWidth(), 1)));
                            moveToTemp->insertBefore(BB.getTerminator());
                            symbolMap.set(moveToTemp, TM.reg(registerNum));
                        }
                        unsigned lastQueue;
                        // Move the value in the loop.
                        while(!q.empty()) {
                            unsigned curReg = q.front();
                            q.pop();
                            countRest--;
                            if(TM.regNo(adjList[curReg][0]) == i) {
                                lastQueue = curReg;
                                break;
                            }
                            Value *nextValue = findLeastReg(adjList[curReg][0], BB, symbolMap);
                            if(nextValue->getType()->isPointerTy()) {
                                Instruction *ptoi = CastInst::CreateBitOrPointerCast(nextValue, IntegerType::getInt64Ty(Context));
                                ptoi->insertBefore(BB.getTerminator());
                                symbolMap.set(ptoi, TM.reg(curReg));

                                moveToTemp = ptoi;

                                Instruction *itop = CastInst::CreateBitOrPointerCast(ptoi, nextValue->getType());
                                itop->insertBefore(BB.getTerminator());
                                symbolMap.set(itop, TM.reg(curReg));
                            } else {
                                Instruction *moveInst = BinaryOperator::CreateMul(nextValue, ConstantInt::get(Context, APInt(nextValue->getType()->getIntegerBitWidth(), 1)));
                                moveInst->insertBefore(BB.getTerminator());
                                symbolMap.set(moveInst, TM.reg(curReg));
                            }

                            if(TM.regNo(adjList[curReg][0]) != -1
                            && !--indegree[TM.regNo(adjList[curReg][0])]) {
                                q.push(TM.regNo(adjList[curReg][0]));
                            }
                        }
                        // At the end, move a value of temporary register to a register before register i.
                        Instruction *moveFromTemp = BinaryOperator::CreateMul(moveToTemp, ConstantInt::get(Context, APInt(moveToTemp->getType()->getIntegerBitWidth(), 1)));
                        moveFromTemp->insertBefore(BB.getTerminator());
                        symbolMap.set(moveFromTemp, TM.reg(lastQueue));
                        if(value->getType()->isPointerTy()) {
                            Instruction *itop = CastInst::CreateBitOrPointerCast(moveFromTemp, value->getType());
                            itop->insertBefore(BB.getTerminator());
                            symbolMap.set(itop, TM.reg(lastQueue));
                        }

                        // Cause the graph can have at most 2 loops, so continue to traverse.
                    }
                }
            } else {
                // Swap two adjacent registers.
                for(unsigned i = 0; i < 32; i++) {
                    if(indegree[i] > 0) {
                        q.push(i);
                        Value *startValue = findLeastReg(TM.reg(i), BB, symbolMap);
                        while(!q.empty()) {
                            unsigned curReg = q.front();
                            q.pop();

                            if(adjList[curReg].empty() || TM.regNo(adjList[curReg][0]) == i) {
                                break;
                            }

                            Value *nextValue = findLeastReg(adjList[curReg][0], BB, symbolMap);

                            if(startValue->getType()->isPointerTy()) {
                                Instruction *ptoi1 = CastInst::CreateBitOrPointerCast(startValue, IntegerType::getInt64Ty(Context));
                                ptoi1->insertBefore(BB.getTerminator());
                                symbolMap.set(ptoi1, TM.reg(curReg));

                                Instruction *ptoi2 = CastInst::CreateBitOrPointerCast(nextValue, IntegerType::getInt64Ty(Context));
                                ptoi2->insertBefore(BB.getTerminator());
                                symbolMap.set(ptoi2, TM.reg(TM.regNo(adjList[curReg][0])));

                                Instruction *xor1 = BinaryOperator::CreateXor(ptoi1, ptoi2);
                                xor1->insertBefore(BB.getTerminator());
                                symbolMap.set(xor1, TM.reg(TM.regNo(adjList[curReg][0])));

                                Instruction *xor2 = BinaryOperator::CreateXor(ptoi1, xor1);
                                xor2->insertBefore(BB.getTerminator());
                                symbolMap.set(xor2, TM.reg(curReg));

                                Instruction *xor3 = BinaryOperator::CreateXor(xor1, xor2);
                                xor3->insertBefore(BB.getTerminator());
                                symbolMap.set(xor3, TM.reg(TM.regNo(adjList[curReg][0])));

                                Instruction *itop1 = CastInst::CreateBitOrPointerCast(xor2, startValue->getType());
                                itop1->insertBefore(BB.getTerminator());
                                symbolMap.set(itop1, TM.reg(curReg));

                                Instruction *itop2 = CastInst::CreateBitOrPointerCast(xor3, nextValue->getType());
                                itop2->insertBefore(BB.getTerminator());
                                symbolMap.set(itop2, TM.reg(curReg));

                                startValue = itop2;
                            } else {
                                // XOR swapping
                                Instruction *xor1 = BinaryOperator::CreateXor(startValue, nextValue);
                                xor1->insertBefore(BB.getTerminator());
                                symbolMap.set(xor1, adjList[curReg][0]);

                                Instruction *xor2 = BinaryOperator::CreateXor(startValue, xor1);
                                xor2->insertBefore(BB.getTerminator());
                                symbolMap.set(xor2, TM.reg(curReg));

                                Instruction *xor3 = BinaryOperator::CreateXor(xor1, xor2);
                                xor3->insertBefore(BB.getTerminator());
                                symbolMap.set(xor3, adjList[curReg][0]);

                                startValue = xor3;
                            }

                            if(!--indegree[TM.regNo(adjList[curReg][0])]
                            && TM.regNo(adjList[curReg][0]) != i) {
                                q.push(TM.regNo(adjList[curReg][0]));
                            }
                        }
                    }
                }
            }
        }
        for(BasicBlock &BB : F) {
            for(Instruction &I : BB) {
                if(auto *phi = dyn_cast<PHINode>(&I)) {
                    for(unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
                        Value *value = phi->getIncomingValue(i);
                        BasicBlock *block = phi->getIncomingBlock(i);
                        if(isa<ConstantInt>(value)) {
                            Instruction *ctoi = BinaryOperator::CreateMul(value, ConstantInt::get(Context, APInt(value->getType()->getIntegerBitWidth(), 1)));
                            ctoi->insertBefore(block->getTerminator());
                            symbolMap.set(ctoi, symbolMap.get(phi));
                        }
                        if(isa<ConstantPointerNull>(value)) {
                            Instruction *ptoi = CastInst::CreateBitOrPointerCast(value, IntegerType::getInt64Ty(Context));
                            ptoi->insertBefore(block->getTerminator());
                            symbolMap.set(ptoi, symbolMap.get(phi));
                        }
                    }
                }
            }
        }
    }
}

// This function elimiates __backend_mov__ after SSAElimination
void Backend::ElimBackendMovInst(Module &M, SymbolMap &symbolMap) {
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto it = BB.begin(), end = BB.end(); it != end;) {
        Instruction &I = *it++;
        CallInst *call = dyn_cast<CallInst>(&I);
        if (call == NULL) continue;

        string callFn = call->getCalledFunction()->getName().str();
        if (callFn != "__backend_mov__") continue;

        Value *O = call->getOperand(0);

        if (symbolMap.get(&I) == symbolMap.get(O)) {
          // can eliminate __backend_mov__
          I.replaceAllUsesWith(O);
          I.eraseFromParent();
        } else {
          // should be replaced into mov; i.e. I = mul i64 O, 1
          Instruction *mov = BinaryOperator::CreateMul(O, ConstantInt::get(M.getContext(), APInt(O->getType()->getIntegerBitWidth(), 1)));
          symbolMap.set(mov, symbolMap.get(&I));
          BB.getInstList().insertAfter(I.getIterator(), mov);
          I.replaceAllUsesWith(mov);
          I.eraseFromParent();
        }
      }
    }
  }
}

// This function finds endmost register which is allocated to register i. 
Value *Backend::findLeastReg(Symbol *reg, BasicBlock &BB, SymbolMap &symbolMap) {
    if(TM.argNo(reg) != -1) {
        return BB.getParent()->getArg(TM.argNo(reg));
    } else if(reg->getName().substr(0, 3) == "gvp") {
        for(auto &G : BB.getParent()->getParent()->globals()) {
            if(symbolMap.get(&G)->getName() == reg->getName()) {
                return &G;
            }
        }
    }
    Instruction *brInst = BB.getTerminator();
    if(isa<ReturnInst>(brInst)) {
        return nullptr;
    }
    for(unsigned i = 0; i < brInst->getNumSuccessors(); i++) {
        BasicBlock *dstBB = brInst->getSuccessor(i);
        for(Instruction &I : *dstBB) {
            PHINode *phi = dyn_cast<PHINode>(&I);
            if(phi == nullptr) {
                continue;
            }
            for(unsigned j = 0; j < phi->getNumIncomingValues(); j++) {
                if(phi->getIncomingBlock(j) != &BB) {
                    continue;
                }
                Value *value = phi->getIncomingValue(j);
                Symbol *symbol = symbolMap.get(value);
                if(symbol && symbol == reg) {
                    return value;
                }
            }
        }
    }
    assert(false && "Error: There is no value with [reg] allocation");
    return nullptr;
}

// This function finds edges between two basic blocks(srcBB, dstBB).
void Backend::addEdges(BasicBlock &srcBB, BasicBlock &dstBB, SymbolMap &symbolMap, vector<vector<Symbol *>> &adjList) {
    LLVMContext &Context = srcBB.getParent()->getParent()->getContext();
    for(Instruction &I : dstBB) {
        PHINode *phi = dyn_cast<PHINode>(&I);
        if(phi == nullptr) {
            continue;
        }
        // Only phi nodes
        Symbol *phiSymbol = symbolMap.get(dyn_cast<Value>(&I));
        assert(phiSymbol && "Error: Symbol of phi does not exist");
        string phiName = phiSymbol->getName();
        int phiNum = TM.regNo(phiSymbol);
        for(unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
            BasicBlock *from = phi->getIncomingBlock(i);
            if(&srcBB != from) {
                continue;
            }
            Value *value = phi->getIncomingValue(i);
            Symbol *instSymbol = symbolMap.get(value);
            if(instSymbol == nullptr || phiSymbol == instSymbol) {
                continue;
            }
            adjList[phiNum].push_back(instSymbol);
        }
    }
}

map<Function*, unsigned> Backend::processAlloca(Module& M, SymbolMap& SM) {

  map<Function*, unsigned> spOffsetMap;

  for(Function& F : M) {
    if(F.isDeclaration()) continue;
    BasicBlock& entry = F.getEntryBlock();

    //Process alloca instructions which are only in the entry block.
    //acc: total stack accumulation of a function before an alloca inst.
    unsigned acc = 0;
    for(Instruction& I : entry) {

      AllocaInst* alloca = dyn_cast<AllocaInst>(&I);
      if(alloca) {
        //Update SymbolMap.
        Memory* stackaddr = new Memory(TM.sp(), acc);
        SM.set(alloca, stackaddr);
        //Update acc
        unsigned size = getAccessSize(alloca->getAllocatedType());
        acc += (size + 7) / 8 * 8;
      }
    }

    spOffsetMap[&F] = acc;
  }

  return spOffsetMap;
}
 
//---------------------------------------------------------------
//class SymbolMap
//---------------------------------------------------------------

SymbolMap::SymbolMap(Module* M, TargetMachine& TM, RegisterGraph& RG) : M(M), TM(TM) {
  //Initiate Machine symbols.

  for(Function& F : *M) {
    assert(F.hasName() && "All functions in module should be named");
    symbolTable[&F] = new Func(F.getName().str());

    unsigned unnamedBB = 0;

    //Assign registers for arguments
    int i = 0;
    for(Value& arg : F.args()) {
      symbolTable[&arg] = TM.arg(i);
      i++;
    }

    //Assign registers for instructions
    for(BasicBlock& BB : F) {

      symbolTable[&BB] = new Block(BB.hasName() ? BB.getName().str() : "_defaultBB" + to_string(unnamedBB++));

      for(Instruction& I : BB) {
        // check if I is a vload, and if it is, make it as a fakeReg
        CallInst *call = dyn_cast<CallInst>(&I);
        if (call != NULL && UnfoldVectorInstPass::VLOADS.find(call->getCalledFunction()->getName().str()) != UnfoldVectorInstPass::VLOADS.end()) {
          Symbol *s = TM.fakeReg();
          symbolTable[&I] = s;
          continue;
        }

        //If not colored(alloca and its derivatives), do nothing.
        if(RG.findValue(&I) == -1) continue;

        unsigned c = RG.getValueToColor(&F, &I);
        Symbol* s = TM.reg(c);
        symbolTable[&I] = s;

      }
    }
  }

  //Assign registers for Global variables
  unsigned acc = 0; //accumulated offset from the gvp pointer
  for(Value& gv : M->globals()) {
    if(!isa<GlobalVariable>(gv)) continue;
    unsigned size = getAccessSize(dyn_cast<GlobalVariable>(&gv)->getValueType());
    Memory* gvaddr = new Memory(TM.gvp(), acc);
    symbolTable[&gv] = gvaddr;
    acc += (size+7) / 8 * 8;
  }
}

void SymbolMap::set(Value* value, Symbol* symbol) {
  symbolTable[value] = symbol;
}

Symbol* SymbolMap::get(Value* value) {
  if(symbolTable.find(value) == symbolTable.end()) return nullptr;
  return symbolTable[value];
}

} //end namespace backend