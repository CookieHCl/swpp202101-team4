#include "Team4Header.h"

/*
  This Pass remove unreachable blocks and unused variables.
  It reduces cost caused by unused calculation.
*/

template <typename T> void RemoveUnusedPass::sortDominated(vector<T> &vec, DominatorTree &DT) {
  std::sort(vec.begin(), vec.end(), [&DT](T a, T b) -> bool {
    return DT.dominates(a, b);
  });
}

void RemoveUnusedPass::eraseInstructionsRecursive(Instruction *inst) {
  while(!inst->user_empty()) {
    Instruction *childInst = inst->user_back();
    this->eraseInstructionsRecursive(childInst);
  }
  inst->eraseFromParent();
}

vector<BasicBlock*> RemoveUnusedPass::getUnreachableBB(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  vector<BasicBlock*> unreachableBBs;
  for (BasicBlock &BB : F)
    if (!DT.isReachableFromEntry(&BB))
      unreachableBBs.push_back(&BB);
  return unreachableBBs;
}

void RemoveUnusedPass::eraseUnreachableBB(Function &F, FunctionAnalysisManager &FAM) {
  auto unreachables = this->getUnreachableBB(F, FAM);

  for (BasicBlock *BB : unreachables) {
    for (BasicBlock *succ : successors(BB))
      succ->removePredecessor(BB);
    for (Instruction &inst : *BB) {
      if (!inst.use_empty()) inst.dropAllReferences();
    }
    BB->getInstList().clear();
  }

  for (BasicBlock *BB : unreachables)
    BB->eraseFromParent();
}

void RemoveUnusedPass::eraseUnusedInstruction(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  vector<Instruction*> unusedInsts;

  // gather defined variables
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (!inst->getType()->isVoidTy()) unusedInsts.push_back(inst);
  }

  // sort in reversed dominated order
  this->sortDominated(unusedInsts, DT);
  std::reverse(unusedInsts.begin(), unusedInsts.end());

  // remove if no use
  for (Instruction *inst : unusedInsts)
    if (inst->use_empty())
      inst->eraseFromParent();
}

void RemoveUnusedPass::eraseUnloadedAlloca(Function &F, FunctionAnalysisManager &FAM) {
  vector<AllocaInst*> unloadedAllocas;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (AllocaInst *alloc = dyn_cast<AllocaInst>(inst)) {
      bool isLoadExist = any_of(alloc->users(), [](User *u) {return isa<LoadInst>(u);});
      if (!isLoadExist) unloadedAllocas.push_back(alloc);
    }
  }

  for (AllocaInst *alloc : unloadedAllocas) {
    this->eraseInstructionsRecursive(alloc);
  }
}

PreservedAnalyses RemoveUnusedPass::run(Function &F, FunctionAnalysisManager &FAM) {
  this->eraseUnreachableBB(F, FAM);
  this->eraseUnloadedAlloca(F, FAM);
  this->eraseUnusedInstruction(F, FAM);
  return PreservedAnalyses::all();
}
