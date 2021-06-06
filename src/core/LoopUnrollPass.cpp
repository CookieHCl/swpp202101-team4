#include "LoopUnrollPass.h"

#define UNROLL_UNIT 8

/*
 *                            Loop Unroll Pass
 *
 *  This Pass unroll innermost loop. Unroll unit is fixed integer 8.
 *  Unrolled instructions reduce incrementing cost and branch cost.
 *  This pass should proceed LoopVectorizePass to enhance vectorize ability.
 *
 */


PreservedAnalyses LoopUnrollPass::run(Function &F, FunctionAnalysisManager &FAM) {
  makeAllocaAsPHI(F, FAM);
  insertPreheader(F, FAM);
  rotateLoop(F, FAM);

  // Loop Unroll should be LCSSA form.
  makeSimplifyLCSSA(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Eager);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);

  // IMPORTANT! To make Remainder loop, PreserveLCSSA. If not, it may raise PHINode related assertion.
  bool PreserveLCSSA = true;

  UnrollLoopOptions ULO;
  // ULO.count means loop copied count
  ULO.Count = UNROLL_UNIT;
  // TripCount is an upper bound on the number of times the loop header runs.
  // Note that we usually do not know exact trip count.
  ULO.TripCount = 0;
  // TripMultiple divides the number of times that the LatchBlock may execute without exiting the loop.
  ULO.TripMultiple = 1;

  // Do not set force. Force will set !allowruntime. Without allow loopunrolling, we do not need it.
  ULO.Force = false;
  // SET allow runtime. If not, it performs only llvm-ir unrolling, not assembly.
  ULO.AllowRuntime = true;
  // We eventually aim load/store vectorize, so unroll anyway.
  ULO.AllowExpensiveTripCount = true;

  ULO.PreserveCondBr = true;
  ULO.PreserveOnlyFirst = false;
  // We cannot assume minimum executiion count.
  ULO.PeelCount = 0;
  // Do not unrool remainder.
  ULO.UnrollRemainder = false;
  // This will help vectorize.
  ULO.ForgetAllSCEV = true;

  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if (L->isInnermost()) {
      OptimizationRemarkEmitter ORE(&F);
      logs() << "[LoopUnrollPass] try unroll " << *L <<"\n";
      Loop *Remainder = nullptr;
      LoopUnrollResult result = UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, &ORE, PreserveLCSSA, &Remainder);

      logs() << "LoopUnrollResult\n";
      switch(result) {
        case LoopUnrollResult::Unmodified: logs() << "- Unmodified\n";
        case LoopUnrollResult::PartiallyUnrolled: logs() << "- Partial\n";
        case LoopUnrollResult::FullyUnrolled: logs() << "- Full\n";
      }

      if (result != LoopUnrollResult::Unmodified) isChanged = true;

      if (Remainder) logs() << "There is remainder Loop : " << *Remainder << "\n";

      // Merge blocks as much as possible
      while (true) {
        bool isMerged = false;
        vector<BasicBlock*> BBs = L->getBlocksVector();

        for (BasicBlock *BB : BBs) isMerged |= MergeBlockIntoPredecessor(BB, &DTU, &LI);

        // If cannot merge new blocks, terminate.
        if (!isMerged) break;
      }

      logs() << "[LoopUnrollPass] END unroll " << *L << "\n";
    }

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}