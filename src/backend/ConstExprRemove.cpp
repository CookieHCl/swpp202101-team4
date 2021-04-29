#include "ConstExprRemove.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

PreservedAnalyses ConstExprRemovePass::run(Module& M, ModuleAnalysisManager& MAM) {
    for(Function& F : M) {
        for(BasicBlock& BB : F) {
            for(auto it = BB.rbegin(); it != BB.rend(); ++it) {
                Instruction& I = *it;

                for(auto& use : I.operands()) {
                    Value* operand = use.get();

                    //ConstantExpr includes 'inlined' operations as in
                    //store i32 %val, %i32* getelementptr...
                    ConstantExpr* expr = dyn_cast<ConstantExpr>(operand);
                    if(expr) {
                        outs() << "\n";
                        Instruction* newI = expr->getAsInstruction();
                        newI->insertBefore(&I);
                        use.set(newI);
                    }
                }
            }
        }
    }
    return PreservedAnalyses::all();
}

}