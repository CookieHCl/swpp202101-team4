#include "LoopUnrollPass.h"


void LoopUnrollPass::rotateLoop(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty()) return;

  LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  SimplifyQuery SQ = getBestSimplifyQuery(FAM, F);

  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if ((isChanged = LoopRotation(L, &LI, &TTI, &AC, &DT, &SE, &MSSAU, SQ, true, -1, true)))
      LAM.invalidate(*L, getLoopPassPreservedAnalyses());

  if (isChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

void LoopUnrollPass::makeSimplifyLCSSA(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty()) return;

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  
  bool isGlobalChanged = false;
  for (Loop *L : LI.getLoopsInPreorder()) {
    bool isChanged = false;
    isChanged |= simplifyLoop(L, &DT, &LI, &SE, &AC, &MSSAU, false);
    isChanged |= formLCSSARecursively(*L, DT, &LI, nullptr);
    isGlobalChanged |= isChanged;
    if (isChanged) LAM.invalidate(*L, getLoopPassPreservedAnalyses());
  }

  if (isGlobalChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
  
}