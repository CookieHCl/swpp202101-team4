#include "LoopUnrollPass.h"



PreservedAnalyses LoopUnrollPass::run(Function &F, FunctionAnalysisManager &FAM) {
  makeAllocaAsPHI(F, FAM);
  rotateLoop(F, FAM);

  // Loop Unroll should be LCSSA form.
  makeSimplifyLCSSA(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  OptimizationRemarkEmitter ORE(&F);
  bool PreserveLCSSA = false;

  UnrollLoopOptions ULO;
  // ULO.count means loop copied count
  ULO.Count = 8;
  // TripCount is an upper bound on the number of times the loop header runs.
  // Note that we usually do not know exact trip count.
  ULO.TripCount = 0;
  // TripMultiple divides the number of times that the LatchBlock may execute without exiting the loop.
  ULO.TripMultiple = 1;

  // Do not set force. Force will set !allowruntime. Without allow loopunrolling, we do not need it.
  ULO.Force = false;
  // SET allow runtime.
  ULO.AllowRuntime = true;
  ULO.AllowExpensiveTripCount = true;

  ULO.PreserveCondBr = true;
  ULO.PreserveOnlyFirst = false;
  ULO.PeelCount = 0;
  ULO.UnrollRemainder = false;
  // This will help vectorize.
  ULO.ForgetAllSCEV = true;

  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if (L->isInnermost()) {
      logs() << "[LoopUnrollPass] try unroll " << *L <<"\n";
      Loop *Remainder = nullptr;
      LoopUnrollResult result = UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, &ORE, PreserveLCSSA, &Remainder);
      logs() << "Result\n";
      switch(result) {
        case LoopUnrollResult::Unmodified: logs() << "Unmodified\n";
        case LoopUnrollResult::PartiallyUnrolled: logs() << "Partial\n";
        case LoopUnrollResult::FullyUnrolled: logs() << "Full\n";
      }

      if (result != LoopUnrollResult::Unmodified) isChanged = true;

      if (Remainder) logs() << "There is remainder Loop : " << *Remainder << "\n";
      logs() << "[LoopUnrollPass] END unroll " << *L << "\n";
    }

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}