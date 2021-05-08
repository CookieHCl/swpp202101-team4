#include "Team4Header.h"

/*
 * 
 */
PreservedAnalyses FunctionInlinePass::run(Module &M, ModuleAnalysisManager &MAM){

  // InlineFunctionInfo
  FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  function_ref<AssumptionCache &(Function &)> GetAssumptionCache =
      [&](Function &F) -> AssumptionCache & {
      return FAM.getResult<AssumptionAnalysis>(F);
  };
  InlineFunctionInfo IFI(nullptr, GetAssumptionCache);

  // Module, Register Graph
  this->M = &M;
  RegisterGraph tRG(M);
  this->RG = &tRG;

  // caller to inline
  vector<CallBase *> do_inline;

  for(Function& F : M) {
    if(F.isDeclaration()) continue;
    
    // If there is no extra register, do nothing for the function.
    unsigned numColor = RG->getNumColors(&F);
    if(numColor >= REGISTER_CAP) continue;
    for(BasicBlock& BB : F) {
      for(Instruction& I : BB) {

        // Find call instruction to inlining
        auto *caller = dyn_cast<CallInst>(&I);
        if(!caller) continue;
        
        // If the sum of colors exceeds the REGISTER_CAP by inlining, do not inlining
        Function *callee;
        if(!(callee = caller->getCalledFunction())) continue;
        unsigned f_numColor = RG->getNumColors(callee);
        if(numColor + f_numColor > REGISTER_CAP) continue;

        // Minimal filter to detect invalid constructs for inlining
        if(!(isInlineViable(*callee).isSuccess())) continue;

        do_inline.push_back(caller);
      }
    }
  }

  // Inlining
  for(CallBase *caller : do_inline) {
    InlineFunction(*caller, IFI);
  }

  return PreservedAnalyses::all();
}