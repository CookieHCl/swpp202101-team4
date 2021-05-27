#include "BranchPredictPass.h"

PreservedAnalyses BranchPredictPass::run(Function &F, FunctionAnalysisManager &FAM) {
  logs() << "---------- Start BranchPredictPass ----------\n";

  // set of BB that has recursive call
  SmallPtrSet<BasicBlock*, 4> RecursiveBlocks;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        if (CB->getCalledFunction() == &F) {
          RecursiveBlocks.insert(&BB);
          break;
        }
      }
    }
  }

  // branch probability info to check branch probability
  BranchProbabilityInfo &BPI = FAM.getResult<BranchProbabilityAnalysis>(F);

  for (BasicBlock &BB : F) {
    // if block is not well formed, return
    Instruction *Terminator = BB.getTerminator();
    if (!Terminator) {
      continue;
    }

    Value *Cond;
    BasicBlock *TrueBB, *FalseBB;
    // check if terminator is br
    if (!match(Terminator, m_Br(m_Value(Cond), m_BasicBlock(TrueBB), m_BasicBlock(FalseBB)))) {
      continue;
    }

    // get branch probability
    const BranchProbability &TrueBBProb = BPI.getEdgeProbability(&BB, TrueBB);
    const BranchProbability &FalseBBProb = BPI.getEdgeProbability(&BB, FalseBB);

    // check if blocks has recursive call
    bool isTrueBBRecursive = RecursiveBlocks.contains(TrueBB);
    bool isFalseBBRecursive = RecursiveBlocks.contains(FalseBB);

    logs() << "Found br: " << *Terminator << '\n';
    logs() << "True br "
        << (isTrueBBRecursive ? " is recursive, " : "is not recursive, ")
        << "probability: " << TrueBBProb << '\n';
    logs() << "False br "
        << (isFalseBBRecursive ? " is recursive, " : "is not recursive, ")
        << "probability: " << FalseBBProb << '\n';

    // if true branch is more likely than false branch, swap
    bool shouldSwap = TrueBBProb > FalseBBProb;
    if (isTrueBBRecursive ^ isFalseBBRecursive && // check if only one is recursive
      isTrueBBRecursive ^ shouldSwap) { // check if result agree with probability
      // only TrueBB is recursive & TrueBBProb <= FalseBBProb
      // or only FalseBB is recursive & TrueBBProb > FalseBBProb
      // contradiction!

      // heuristic: use recursive if max prob < 0.9, use prob if max prob >= 0.9
      // use floating point to avoid overflow
      bool isProbUnreliable = std::max(TrueBBProb, FalseBBProb).getNumerator()
        < 0.9 * BranchProbability::getDenominator();

      logs() << "Contradiction; "
          << (isProbUnreliable ? "Using recursive\n" : "Using probability\n");
      if (isProbUnreliable) {
        shouldSwap = !shouldSwap;
      }
    }

    if (shouldSwap) {
      logs() << "Swapping branch..\n";

      if (auto *CmpCond = dyn_cast<CmpInst>(Cond)) {
        logs() << *Cond << " is icmp or fcmp\n";
        CmpCond->setPredicate(CmpCond->getInversePredicate());
        logs() << "Swapped condition to " << *Cond << '\n';
      }
      else {
        logs() << "Condition is not icmp or fcmp\n";
        // implement "not" with select
        auto TrueConst = ConstantInt::getTrue(F.getContext());
        auto FalseConst = ConstantInt::getFalse(F.getContext());
        Cond = SelectInst::Create(Cond, FalseConst, TrueConst, Twine(),
          /* insert before */ Terminator);
        logs() << "Adding not operation " << *Cond << '\n';
      }

      auto NewInst = BranchInst::Create(FalseBB, TrueBB, Cond);
      logs() << "Replace br with " << *NewInst << '\n';
      ReplaceInstWithInst(Terminator, NewInst);
    }
  }

  logs() << "---------- End BranchPredictPass ----------\n";
  return PreservedAnalyses::none();
}
