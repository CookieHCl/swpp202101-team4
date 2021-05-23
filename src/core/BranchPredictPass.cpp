#include "BranchPredictPass.h"

PreservedAnalyses BranchPredictPass::run(Function &F, FunctionAnalysisManager &FAM) {
  printDebug("---------- Start BranchPredictPass ----------");

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

    printDebug("Found br: ", *Terminator);
    printDebug("True br ",
      isTrueBBRecursive ? " is recursive, " : "is not recursive, ",
      "probability: ", TrueBBProb);
    printDebug("False br ",
      isFalseBBRecursive ? " is recursive, " : "is not recursive, ",
      "probability: ", FalseBBProb);

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

      printDebug("Contradiction; Use ", isProbUnreliable ? "recursive" : "prob");
      if (isProbUnreliable) {
        shouldSwap = !shouldSwap;
      }
    }

    if (shouldSwap) {
      printDebug("Swapping branch..");

      if (auto *CmpCond = dyn_cast<CmpInst>(Cond)) {
        printDebug(*Cond, " is icmp or fcmp");
        CmpCond->setPredicate(CmpCond->getInversePredicate());
        printDebug("Swapped condition to ", *Cond);
      }
      else {
        printDebug("Condition is not icmp or fcmp;");
        // implement "not" with xor
        auto MaxConst = Constant::getAllOnesValue(Cond->getType());
        Cond = BinaryOperator::Create(Instruction::Xor, Cond, MaxConst, Twine(),
          /* insert before */ Terminator);
        printDebug("Adding not operation ", *Cond);
      }

      auto NewInst = BranchInst::Create(FalseBB, TrueBB, Cond);
      printDebug("Replace br with ", *NewInst);
      ReplaceInstWithInst(Terminator, NewInst);
    }
  }

  printDebug("---------- End BranchPredictPass ----------");
  return PreservedAnalyses::none();
}