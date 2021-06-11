#include "LoopUnrollPass.h"

#define UNROLL_UNIT 8
#define UNROLL_MINUMUM_UNIT 2

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
  // ULO.count means loop copied count.
  // By default, this is UNROLL_UNIT(8 in asm) but can be adjusted depending on default loop step.
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

  ULO.PreserveCondBr = false;
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

      ULO.Count = UNROLL_UNIT;  // By default

      const Optional<Loop::LoopBounds> opBound = L->getBounds(SE);

      if (opBound.hasValue()) {
        const Loop::LoopBounds bound = opBound.getValue();
        Instruction *stepInst = &bound.getStepInst();
        Value *stepValue = bound.getStepValue();

        logs() << "Step inst : " << *stepInst << "\n";

        if (!isa<AddOperator>(stepInst)) {
          logs() << "Step inst is not Add. Do not unroll.\n\n";
          continue;
        }

        logs() << "Step Value : " << *stepValue << "\n";
        ConstantInt *stepInt = dyn_cast<ConstantInt>(stepValue);

        if (!stepInt) {
          logs() << "Do not allow non-constant step value. Do not unroll.\n\n";
          continue;
        }

        const APInt Int = stepInt->getUniqueInteger();
        // Prevent large number and -2^31. We do not need them.
        if (Int.getActiveBits() > 32) {
          logs() << "Do not allow too big step. Do not unroll.\n\n";
          continue;
        }

        const int stepSize = Int.getSExtValue();

        if (stepSize <= 0) {
          logs() << "Only support positive step size. Do not unroll.\n\n";
          continue;
        }

        logs() << "Step Value is ConstantInt [" << *stepInt << "] which in int " << stepSize << "\n";

        const unsigned absStepSize = stepSize > 0 ? stepSize : (-stepSize);

        // General method for any UNROLL_UNIT
        for (unsigned i = 1; i <= UNROLL_UNIT; ++i)
          if (((i * absStepSize) % UNROLL_UNIT) == 0) {
            ULO.Count = i;
            break;
          }
      }

      logs() << "Unroll Factor : " << ULO.Count << "\n";
      if (ULO.Count < UNROLL_MINUMUM_UNIT) {
        logs() << "Do not vectorize due to unroll factor (" << ULO.Count << " < MinimumUnit(2))\nDo not Unroll.\n\n";
        continue;
      }

      LoopUnrollResult result = UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, &ORE, PreserveLCSSA, &Remainder);

      logs() << "LoopUnrollResult\n";
      switch(result) {
        case LoopUnrollResult::Unmodified: {
          logs() << "- Unmodified\n";
          continue;
        };
        case LoopUnrollResult::PartiallyUnrolled: logs() << "- Partial\n";
        case LoopUnrollResult::FullyUnrolled: logs() << "- Full\n";
      }

      isChanged = true;

      if (Remainder) logs() << "There is remainder Loop : " << *Remainder << "\n";

      // Merge blocks as much as possible
      while (true) {
        bool isMerged = false;
        vector<BasicBlock*> BBs = L->getBlocksVector();

        for (BasicBlock *BB : BBs) {
          if (isa<BranchInst>(BB->getTerminator()))
            isMerged |= MergeBlockIntoPredecessor(BB, &DTU, &LI);
        }

        // If cannot merge new blocks, terminate.
        if (!isMerged) break;
      }

      logs() << "[LoopUnrollPass] END unroll " << *L << "\n";
    }

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}