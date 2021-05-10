#include "functionInlinePass.h"


/*************************************************
 *             Function Inine Pass
 
 * If the number of registers does not overflow,
 * overflow meaning that it cannot be covered by
 * 32 registers, even inline the callee function
 * of a function call, inline it.
 
 ************************************************/

PreservedAnalyses FunctionInlinePass::run(Module &M, ModuleAnalysisManager &MAM){

  // InlineFunctionInfo
  FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  InlineFunctionInfo IFI(nullptr, [&](Function &F) -> AssumptionCache & {
      return FAM.getResult<AssumptionAnalysis>(F);
  });

  //Register graph for the module.
  RegisterGraph tRG(M);
  RegisterGraph *RG = &tRG;

  // caller to inline
  vector<CallBase *> do_inline;

  for(Function& F : M) {
    if(F.isDeclaration()) continue;
    
    // If there is no extra register, do nothing for the function.

    // The number of registers can be found through
    // backend::RegisterGraph.getNumColors(Function*)
    unsigned numColor = RG->getNumColors(&F);
    if(numColor >= REGISTER_CAP) continue;
    for(BasicBlock& BB : F) {
      for(Instruction& I : BB) {

        // Find call instruction to inlining
        auto *caller = dyn_cast<CallInst>(&I);
        if(!caller) continue;
        
        // If the sum of colors exceeds the REGISTER_CAP by inlining, do not inlining
        Function *callee = caller->getCalledFunction();
        if(!callee) continue;
        unsigned f_numColor = RG->getNumColors(callee);
        if(numColor + f_numColor > REGISTER_CAP) continue;

        // Minimal filter to detect invalid constructs for inlining
        if(!(isInlineViable(*callee).isSuccess())) continue;

        numColor += f_numColor;
        do_inline.push_back(caller);
      }
    }
  }

  for(CallBase *caller : do_inline) {

    // Inline caller with not insert Lifetime

    // llvm::InlineFunctionInfo.InlineFunction(CallBase &CB, InlineFunctionInfo &IFI,
    //                                         AAResults *CalleeAAR,
    //                                         bool InsertLifetime,
    //                                         Function *ForwardVarArgsTo)
    InlineFunction(*caller, IFI, nullptr, false, nullptr);
  }

  return PreservedAnalyses::all();
}
