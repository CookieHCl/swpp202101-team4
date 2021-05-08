#include "Team4Header.h"

/*
  This Pass remove unreachable blocks and unused variables.
  It reduces cost caused by 
*/


SetVector<BasicBlock*> RemoveUnusedPass::getUnreachableBB(Function &F, FunctionAnalysisManager &FAM) {
  SetVector<BasicBlock*> unreachableSet;
  queue<BasicBlock*> blockQueue;

  for (BasicBlock &BB : F) {
    unreachableSet.insert(&BB);
  }
  blockQueue.push(&F.getEntryBlock());

  while (!blockQueue.empty()) {
    BasicBlock *BB = blockQueue.front();
    blockQueue.pop();
    if (unreachableSet.contains(BB)) {
      unreachableSet.remove(BB);
      Instruction *terminator = BB->getTerminator();
      if (terminator)
        for (unsigned int idx = 0; idx < terminator->getNumSuccessors(); ++idx)
          blockQueue.push(terminator->getSuccessor(idx));
    }
  }

  return unreachableSet;
}

void RemoveUnusedPass::eraseUnreachableBB(Function &F, FunctionAnalysisManager &FAM){
  const auto unreachableSet = getUnreachableBB(F, FAM);
  for (BasicBlock *BB : unreachableSet) {
    BB->eraseFromParent();
  }
}

PreservedAnalyses RemoveUnusedPass::run(Function &F, FunctionAnalysisManager &FAM) {
  RemoveUnusedPass::eraseUnreachableBB(F, FAM);
  return PreservedAnalyses::all();
}
