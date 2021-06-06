#include "LoopSimplifyUtil.h"

void insertPreheader(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty()) return;

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);

  for (Loop *L : LI.getLoopsInPreorder())
    InsertPreheaderForLoop(L, &DT, &LI, &MSSAU, true);

  FAM.invalidate(F, PreservedAnalyses::none());
}

void rotateLoop(Function &F, FunctionAnalysisManager &FAM) {
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
  for (Loop *L : LI.getLoopsInPreorder()) {
    if (LoopRotation(L, &LI, &TTI, &AC, &DT, &SE, &MSSAU, SQ, true, -1, true)) {
      LAM.invalidate(*L, PreservedAnalyses::none());
      isChanged = true;
    }
  }

  if (isChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

void makeSimplifyLCSSA(Function &F, FunctionAnalysisManager &FAM) {
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
    if (isChanged) LAM.invalidate(*L, PreservedAnalyses::none());
    isGlobalChanged |= isChanged;
  }

  if (isGlobalChanged) FAM.invalidate(F, getLoopPassPreservedAnalyses());
}

void makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);

  SmallVector<AllocaInst*, 8> allocaInstVector;
  bool isChanged = false;

  while (true) {
    allocaInstVector.clear();
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
      if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(&*I))
        if (isAllocaPromotable(allocaInst)) allocaInstVector.push_back(allocaInst);

    if (allocaInstVector.empty()) break;

    PromoteMemToReg(allocaInstVector, DT, &AC);
    isChanged = true;
  }

  if (isChanged) FAM.invalidate(F, PreservedAnalyses::none());
}
