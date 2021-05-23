#include "FunctionInlinePass.h"


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
  unique_ptr<Module> copyM = CloneModule(M);
  RegisterGraph tRG(*copyM);
  RegisterGraph *RG = &tRG;

  // caller to inline and Function's numColor
  vector<pair<CallBase *, unsigned> > do_inline;
  map<Function *, unsigned> numColors;

  for(Function& F : M) {

    if(F.isDeclaration()) continue;
    // The number of registers can be found through
    // backend::RegisterGraph.getNumColors(Function*)
    for(Function& copyF : *copyM) {

      if(copyF.isDeclaration()) continue;

      if(F.getName() == copyF.getName()) {
        numColors[&F] = RG->getNumColors(&copyF);
        break;
      }
    }
  }

  for(Function& F : M) {
    
    if(F.isDeclaration()) continue;
    
    // If there is no extra register, do nothing for the function.

    unsigned numColor = numColors[&F];
    if(numColor >= REGISTER_CAP) continue;

    // Where F's callers start to be stored in do_inline
    int st = do_inline.size();

    for(BasicBlock& BB : F) {
      for(Instruction& I : BB) {

        // Find call instruction to inlining
        auto *caller = dyn_cast<CallInst>(&I);
        if(!caller) continue;
        
        // Minimal filter to detect invalid constructs for inlining
        Function *callee = caller->getCalledFunction();
        if(!callee || callee->isDeclaration()) continue;
        if(!(isInlineViable(*callee).isSuccess())) continue;

        // If the sum of colors exceeds the REGISTER_CAP by inlining, do not inlining
        unsigned f_numColor = numColors[callee];
        if(numColor + f_numColor > REGISTER_CAP) {
          // An index that element have maximum register count
          unsigned i_m = -1;

          for(int i = st; i < do_inline.size(); ++i) {
            if(i_m == -1) i_m = i;
            else if(do_inline[i_m].second < do_inline[i].second) i_m = i;
          }
          if(i_m != -1 && f_numColor < do_inline[i_m].second) {
            numColor += f_numColor - do_inline[i_m].second;
            do_inline[i_m] = make_pair(caller, f_numColor);
          }
        }
        else {
          numColor += f_numColor;
          do_inline.push_back(make_pair(caller, f_numColor));
        }
      }
    }
    // update numColors
    numColors[&F] = numColor;
  }

  for(auto& e : do_inline) {

    // Inline caller without insert Lifetime
    CallBase *caller = e.first;
    // llvm::InlineFunctionInfo.InlineFunction(CallBase &CB, InlineFunctionInfo &IFI,
    //                                         AAResults *CalleeAAR,
    //                                         bool InsertLifetime,
    //                                         Function *ForwardVarArgsTo)
    InlineFunction(*caller, IFI, nullptr, false, nullptr);
  }

  return PreservedAnalyses::none();
}
