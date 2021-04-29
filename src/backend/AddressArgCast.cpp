#include "AddressArgCast.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

PreservedAnalyses AddressArgCastPass::run(Module& M, ModuleAnalysisManager& MAM) {
    for(Function& F : M) {
        for(auto it = inst_begin(F); it != inst_end(F); ++it) {
            CallInst* CI = dyn_cast<CallInst>(&*it);

            if(!CI) continue;

            for(auto& use : CI->arg_operands()) {
                Value* operand = use.get();
                if(isa<AllocaInst>(operand) || isa<GlobalVariable>(operand)) {
                    BitCastInst* BCI = new BitCastInst(operand, operand->getType(), operand->getName(), CI);
                    use.set(BCI);
                }
            }
        }
    }
    return PreservedAnalyses::all();
}

}