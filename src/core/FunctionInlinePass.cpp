#include "FunctionInlinePass.h"


/*************************************************
 *             Function Inine Pass
 
 * If the number of registers does not overflow,
 * overflow meaning that it cannot be covered by
 * 32 registers, even inline the callee function
 * of a function call, inline it.
 
 ************************************************/

PreservedAnalyses FunctionInlinePass::run(Module &M, ModuleAnalysisManager &MAM){
  // check if changed to return correct preserved analyses
  bool Changed = false;

  // InlineFunctionInfo
  FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  InlineFunctionInfo IFI(nullptr, [&](Function &F) -> AssumptionCache & {
      return FAM.getResult<AssumptionAnalysis>(F);
  });

  //Register graph for the module.
  unique_ptr<Module> copyM = CloneModule(M);
  RegisterGraph RG(*copyM);

  // caller to inline and Function's numColor
  vector<pair<CallBase *, unsigned> > do_inline;
  map<Function *, unsigned> numColors;
  map<Function *, unsigned> numInsts;

  for(Function& F : M) {

    if(F.isDeclaration()) continue;
    // The number of registers can be found through
    // backend::RegisterGraph.getNumColors(Function*)
    Function* copyF = copyM->getFunction(F.getName());
    numColors[&F] = RG.getNumColors(copyF);
    numInsts[&F] = F.getInstructionCount();
  }

  for(Function& F : M) {
    
    if(F.isDeclaration()) continue;
    
    // If there is no extra register, do nothing for the function.

    unsigned numColor = numColors[&F];
    unsigned numInst = numInsts[&F];
    if(numColor >= REGISTER_CAP) continue;
    if(numInst >= LIMIT_INSTS) continue;

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

        // Check Attribute
        if(callee->hasFnAttribute(Attribute::NoInline)) continue;

        // If the sum of colors exceeds the REGISTER_CAP by inlining, do not inlining
        unsigned f_numColor = numColors[callee];
        unsigned f_numInst = numInsts[callee];
        if(callee->arg_size() + f_numColor > REGISTER_CAP) continue;
        if(numInst + f_numInst > LIMIT_INSTS) {
          // An index that element have maximum register count
          unsigned i_m = -1;

          for(int i = st; i < do_inline.size(); ++i) {
            if(i_m == -1) i_m = i;
            else if(do_inline[i_m].second < do_inline[i].second) i_m = i;
          }
          if(i_m != -1 && f_numInst < do_inline[i_m].second) {
            numInst += f_numInst - do_inline[i_m].second;
            do_inline[i_m] = make_pair(caller, f_numInst);
          }
        }
        else {
          numInst += f_numInst;
          do_inline.push_back(make_pair(caller, f_numInst));
        }
      }
    }

    for(int i = st; i < do_inline.size(); ++i) {
      Function *callee = do_inline[i].first->getCalledFunction();
      if(numColor < numColors[callee] + callee->arg_size())
        numColor = numColors[callee] + callee->arg_size();
    }

    // update numColors
    numColors[&F] = numColor;
    numInsts[&F] = numInst;
  }

  for(auto& e : do_inline) {

    // Inline caller without insert Lifetime
    CallBase *caller = e.first;
    // llvm::InlineFunctionInfo.InlineFunction(CallBase &CB, InlineFunctionInfo &IFI,
    //                                         AAResults *CalleeAAR,
    //                                         bool InsertLifetime,
    //                                         Function *ForwardVarArgsTo)
    Changed |= InlineFunction(*caller, IFI, nullptr, false, nullptr).isSuccess();
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
