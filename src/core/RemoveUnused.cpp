#include "Team4Header.h"

/*
  This Pass remove unreachable blocks and unused variables.
  It reduces cost caused by unused calculation.
*/

vector<BasicBlock*> RemoveUnusedPass::getUnreachableBBs(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  vector<BasicBlock*> unreachableBBs;
  for (BasicBlock &BB : F)
    if (!DT.isReachableFromEntry(&BB))
      unreachableBBs.push_back(&BB);
  return unreachableBBs;
}

void RemoveUnusedPass::eraseUnreachableBBs(Function &F, FunctionAnalysisManager &FAM) {
  auto unreachables = this->getUnreachableBBs(F, FAM);

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

void RemoveUnusedPass::eraseUnusedInstructions(Function &F, FunctionAnalysisManager &FAM) {
  SmallPtrSet<Value*, 16> usedValues = this->getUsedValues(F);
  SmallPtrSet<Instruction*, 16> unusedInsts;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (!usedValues.count(inst))
      unusedInsts.insert(inst);
  }
  this->eraseInstructions(unusedInsts);
}

void RemoveUnusedPass::eraseInstructions(SmallPtrSet<Instruction*, 16> insts) {
  outs() << "Unused\n";
  for (Instruction *inst : insts)
    outs() << *inst << "\n";
  outs() << "Unused END\n";
  queue<Instruction*> safeRemoveInsts;
  while (true) {
    for (Instruction *inst : insts)
      if (inst->use_empty())
        safeRemoveInsts.push(inst);
    
    if (safeRemoveInsts.empty())
      return;

    while (!safeRemoveInsts.empty()) {
      Instruction *inst = safeRemoveInsts.front();
      safeRemoveInsts.pop();
      inst->eraseFromParent();
      insts.erase(inst);
    }
  }
}

SmallPtrSet<GlobalValue*, 16> RemoveUnusedPass::getGloalValues(Function &F) {
  SmallPtrSet<GlobalValue*, 16> globalValues;
  Module *M = F.getParent();

  for (auto gv_iter = M->global_begin(), gv_end = M->global_end(); gv_iter != gv_end; ++gv_iter) {
    GlobalValue *gv = &*gv_iter;
    globalValues.insert(gv);
  }

  return globalValues;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getRecursiveUsers(Value *v) {
  SmallPtrSet<Value*, 16> recursiveUsers;
  queue<Value*> userQueue;
  userQueue.push(v);
  while (!userQueue.empty()) {
    Value *u = userQueue.front();
    userQueue.pop();
    if (!recursiveUsers.count(u)) {
      recursiveUsers.insert(u);
      for (Value *U : u->users())
        userQueue.push(U);
    }
  }
  outs() << "Recursive Users of " << *v << "\n";

  for (Value *u : recursiveUsers)
    outs() << *u << "\n";
  
  outs() << "Recursive End\n";

  return recursiveUsers;
}

// Return Global variable, functions and arguments
// and local not safe to remove excpet store instructions.
SmallPtrSet<Value*, 16> RemoveUnusedPass::getNecessaryValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues;
  SmallPtrSet<Instruction*, 16> necessaryInsts;
  set_union(necessaryValues, this->getGloalValues(F));

  for (Argument &arg : F.args()) {
    necessaryValues.insert(&arg);
  }

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (!inst->isSafeToRemove() && !isa<StoreInst>(inst))
      necessaryValues.insert(inst);
  }

  return necessaryValues;
}

SmallPtrSet<Instruction*, 16> RemoveUnusedPass::getUserInstSet(SmallPtrSet<Value*, 16> &valueSet, Function &F) {
  SmallPtrSet<Instruction*, 16> userInsts;
  for (Value *v : valueSet) {
    if (!isa<Constant>(v))
      for (Value *u : this->getRecursiveUsers(v)) {
        Instruction *inst = dyn_cast<Instruction>(u);
        if (inst && inst->getParent() && inst->mayHaveSideEffects())
          if (inst->getParent()->getParent() == &F)
            userInsts.insert(inst);
      }
  }
  return userInsts;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getInstPredecessors(Instruction *inst) {
  SmallPtrSet<Value*, 16> predSet;
  queue<Value*> predQueue;
  predQueue.push(inst);
  while (!predQueue.empty()) {
    Value* v = predQueue.front();
    predQueue.pop();
    if (!predSet.count(v)) {
      if (!isa<BasicBlock>(v) && !isa<Constant>(v))
        predSet.insert(v);
      if (Instruction *I = dyn_cast<Instruction>(v))
        for (Value *op : I->operands())
          predQueue.push(op);
    }
  }
  return predSet;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getPredecessorSet(SmallPtrSet<Value*, 16> &valueSet, Function &F) {
  SmallPtrSet<Value*, 16> predSet;
  for (Value *v : valueSet) {
    if (Instruction *inst = dyn_cast<Instruction>(v))
      set_union(predSet, this->getInstPredecessors(inst));
  }
  return predSet;
}

/*
그냥 필수적인거 고르고, 그게 쓰이는 MemOp 찾고, MemOp에 쓰이는 변수 체인 넣는식으로 여러번 하는게 나을듯.
언제까지? 새로운 MemOp이 없을때까지.
*/
/*
1. 필수적인걸 고른다
2. 필수적인거 실행하기 위한 변수 추가한다.
3. 추가한 변수 중 MemOps를 필수에 추가한다.

2가 없거나 3이 없으면 종료한다.
글로벌에 저장하는건 어쩔 수 없는데
로컬에 저장하려고 하는데, 그 주소가 그동안 안쓰인 주소라면 의심해봐야한다.
*/
SmallPtrSet<Value*, 16> RemoveUnusedPass::getUsedValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues = this->getNecessaryValues(F);
  outs() << "NECESSARY\n";
  for (Value *v : necessaryValues)
    outs() << *v << "\n";
  outs() << "NECESSARY END\n";

  while (true) {
    set_union(necessaryValues, this->getUserInstSet(necessaryValues, F));
    outs() << "NECESSARY START\n";
    for (Value *n : necessaryValues)
      outs() << *n << "\n";
    outs() << "NECESSARY END\n";
    SmallPtrSet<Value*, 16> predecessorSet = this->getPredecessorSet(necessaryValues, F);
    outs() << "PRED START\n";
    for (Value *n : predecessorSet)
      outs() << *n << "\n";
    outs() << "PRED END\n";
    SmallPtrSet<Value*, 16> diffSet;
    set_union(diffSet, predecessorSet);
    set_subtract(diffSet, necessaryValues);
    outs() << "DIFF START" << "\n";
    for (Value *v : diffSet)
      outs() << *v << "\n";
    outs() << "DIFF END" << "\n";
    if (diffSet.empty()) break;
    else {
      set_union(necessaryValues, this->getUserInstSet(diffSet, F));
      set_union(necessaryValues, diffSet);
    }
  }
  
  return necessaryValues;
}

PreservedAnalyses RemoveUnusedPass::run(Function &F, FunctionAnalysisManager &FAM) {
  this->eraseUnreachableBBs(F, FAM);
  this->eraseUnusedInstructions(F, FAM);
  return PreservedAnalyses::all();
}
