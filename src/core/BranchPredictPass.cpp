#include "BranchPredictPass.h"

PreservedAnalyses BranchPredictPass::run(Function &F, FunctionAnalysisManager &FAM) {
  printDebug("---------- Start BranchPredictPass ----------");

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

    printDebug("Found br: ", *Terminator);
    printDebug("True br probability: ", TrueBBProb);
    printDebug("False br probability: ", FalseBBProb);

    // false branch should be more likely than true branch
    if (TrueBBProb > FalseBBProb) {
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