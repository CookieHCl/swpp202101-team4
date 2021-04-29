#include "UnfoldVectorInst.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

map<Instruction*, unsigned int> ReservedColor;

set<string> UnfoldVectorInstPass::VLOADS = {
    "vload2", "vload4", "vload8"
};
set<string> UnfoldVectorInstPass::EXTRACT_ELEMENTS = {
    "extract_element2", "extract_element4", "extract_element8"
};
set<string> UnfoldVectorInstPass::VSTORES = {
    "vstore2", "vstore4", "vstore8"
};

static unsigned int bitcount(unsigned int mask) {
    int s = 0;
    for (; mask; mask>>=1) s += mask & 1;
    return s;
}

static unsigned int max_mask_number(string fn) {
    int n = atoi(fn.substr(fn.size() - 1, 1).c_str());
    return (1 << n) - 1;
}

PreservedAnalyses UnfoldVectorInstPass::run(Module& M, ModuleAnalysisManager& MAM) {
    LLVMContext &Context = M.getContext();
    FunctionType *MovFnTy = FunctionType::get(Type::getInt64Ty(Context), {Type::getInt64Ty(Context)}, false);

    assert(M.getFunction(MOV_FN) == NULL && "there should not be a function named __backend_mov__");
    Function *MovFn = Function::Create(MovFnTy, Function::ExternalLinkage, MOV_FN, M);

    for(Function& F : M) {
        for(auto it = inst_begin(F); it != inst_end(F); ++it) {
            CallInst* CI = dyn_cast<CallInst>(&*it);

            if(!CI) continue;

            BasicBlock *BB = CI->getParent();

            string callFn = CI->getCalledFunction()->getName().str();

            /* Unfold vloadX and extractElementX for safe RA
             * before:
             *   %v = call <4 x i64> @vload4(i8* %ptr, i64 %mask)
             *   %v0 = call i64 @extract_element4(<4 x i64> %v, i64 0)
             *   ...
             *   %v3 = call i64 @extract_element4(<4 x i64> %v, i64 3)
             * 
             * after:
             *   %v = call <4 x i64> @vload4(i8* %ptr, i64 %mask)
             *   %e0 = call i64 @extract_element4(<4 x i64> %v, i64 0)
             *   ...
             *   %e3 = call i64 @extract_element4(<4 x i64> %v, i64 3)
             *   %v0 = call i64 @__backend_mov__(i64 %e0)
             *   ...
             *   %v1 = call i64 @__backend_mov__(i64 %e3)
             * 
             * where %e0, %e1, %e2, and %e3 have the strongest preemption on RA,
             * with a relatively shorten liveness boundaries.
            */
            if (VLOADS.find(callFn) != VLOADS.end()) {
                unsigned int dim = CI->getCalledFunction()->arg_size() - 1;
                auto *maskVal = dyn_cast<ConstantInt>(CI->getOperand(dim));
                assert(maskVal != NULL && "mask should be a constant integer");

                int mask = maskVal->getZExtValue();
                assert(0 <= mask && mask <= max_mask_number(callFn) && "invalid mask number");

                unsigned int bc = bitcount(mask);

                vector<CallInst *> extractElements;

                auto itt = it;
                // there should be `bc` consecutive `extract_elementX`s
                for (int i = 0; i < bc; i++) {
                    CallInst *EE = dyn_cast<CallInst>(&*(++itt));
                    assert(EE != NULL && "should be a extract_element instruction");

                    string eeFn = EE->getCalledFunction()->getName().str();
                    assert(EXTRACT_ELEMENTS.find(eeFn) != EXTRACT_ELEMENTS.end());

                    ConstantInt *Index = dyn_cast<ConstantInt>(EE->getOperand(1));
                    assert(Index != NULL && "index of extract_elementX should be a constant int");

                    int index = Index->getZExtValue();
                    assert((mask & (1 << index)) && "mask and extract_elementX index mismatch");
                    mask &= ~(1 << index); // no repetition

                    extractElements.push_back(EE);
                }

                unsigned int color = 0;

                for (int i = 0; i < bc; i++) {
                    CallInst *EE = extractElements[i];

                    vector<Use*> EEPrevUses;

                    for (auto itu = EE->use_begin(), endu = EE->use_end(); itu != endu;) {
                        Use &U = *itu++;
                        EEPrevUses.push_back(&U);
                    }

                    CallInst *ENew = CallInst::Create(MovFnTy, MovFn, {EE});
                    BB->getInstList().insertAfter(itt->getIterator(), ENew);

                    // replace all uses
                    for (auto use : EEPrevUses) {
                        use->set(ENew);
                    }

                    EEPrevUses.clear();
                    ReservedColor[EE] = color++; // immortal register coloring
                    itt++;
                }

                it = itt;
                extractElements.clear();
            }
            /* Unfold vstoreX for safe RA
             * before:
             *   call void @vstore4(i64 v0, i64 v1, i64 v2, i64 v3, i64* %ptr, i64 %mask)
             * 
             * after:
             *   %e0 = call i64 @__backend_mov__(i64 %v0)
             *   ...
             *   %e1 = call i64 @__backend_mov__(i64 %v3)
             *   call void @vstore4(i64 e0, i64 e1, i64 e2, i64 e3, i64* %ptr, i64 %mask)
             * 
             * where %e0, %e1, %e2, and %e3 have the strongest preemption on RA,
             * with a relatively shorten liveness boundaries.
            */
            else if (VSTORES.find(callFn) != VSTORES.end()) {
                unsigned int dim = CI->getCalledFunction()->arg_size() - 2;
                auto *maskVal = dyn_cast<ConstantInt>(CI->getOperand(dim + 1));
                assert(maskVal != NULL && "mask should be a constant integer");

                int mask = maskVal->getZExtValue();
                assert(0 <= mask && mask <= max_mask_number(callFn) && "invalid mask number");

                unsigned int color = 0;

                map<Instruction*, Instruction*> replaceMap;

                for (int i = 0; i < dim; i++) {
                    if (!(mask & (1 << i))) continue;

                    Instruction *I = dyn_cast<Instruction>(CI->getOperand(i));
                    if (!I) continue; // no need to RA for a constant operand

                    // insert new %ei instructions before CI
                    // and replace the operand's use with %ei
                    if (replaceMap.find(I) == replaceMap.end()) {
                        CallInst *ENew = CallInst::Create(MovFnTy, MovFn, {I}, "", CI);
                        CI->getOperandUse(i).set(ENew);

                        ReservedColor[ENew] = color++; // immortal register coloring
                        replaceMap[I] = ENew;
                    } else {
                        // to cope with repetitions: vstore4 %a %a %a %a
                        Instruction *ENew = replaceMap[I];
                        CI->getOperandUse(i).set(ENew);
                    }
                }

                replaceMap.clear();
            } else {
                assert(EXTRACT_ELEMENTS.find(callFn) == EXTRACT_ELEMENTS.end());
                assert(callFn != "__backend_mov__");
            }
        }
    }
    return PreservedAnalyses::all();
}

}