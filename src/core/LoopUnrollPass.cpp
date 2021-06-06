#include "LoopUnrollPass.h"



PreservedAnalyses LoopUnrollPass::run(Function &F, FunctionAnalysisManager &FAM) {
  makeAllocaAsPHI(F, FAM);
  rotateLoop(F, FAM);
  makeSimplifyLCSSA(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  OptimizationRemarkEmitter ORE(&F);
  bool PreserveLCSSA = false;

  UnrollLoopOptions ULO;
  ULO.Count = 8;
  ULO.TripCount = 0;
  ULO.TripMultiple = 1;

  ULO.Force = false;
  ULO.AllowRuntime = true;
  ULO.AllowExpensiveTripCount = true;

  ULO.PreserveCondBr = true;
  ULO.PreserveOnlyFirst = false;
  ULO.PeelCount = 0;
  ULO.UnrollRemainder = false;
  ULO.ForgetAllSCEV = true;

  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if (L->isInnermost()) {
      Loop *Remainder = nullptr;
      LoopUnrollResult result = UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, &ORE, PreserveLCSSA, &Remainder);
      logs() << "Result : ";
      switch(result) {
        case LoopUnrollResult::Unmodified: logs() << "Unmodified\n";
        case LoopUnrollResult::PartiallyUnrolled: logs() << "Partial\n";
        case LoopUnrollResult::FullyUnrolled: logs() << "Full\n";
      }
      if (Remainder) {
        logs() << "There is remainder \n";
        logs() << *Remainder << "\n";
      }
    }

  return PreservedAnalyses::all();
}