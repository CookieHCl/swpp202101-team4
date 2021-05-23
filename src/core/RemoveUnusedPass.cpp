#include "Team4Header.h"

/*
  This Pass remove unreachable blocks and unused variables.
  It reduces cost caused by unused calculation.
*/

vector<BasicBlock*> RemoveUnusedPass::getUnreachableBB(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  vector<BasicBlock*> unreachableBBs;
  for (BasicBlock &BB : F)
    if (!DT.isReachableFromEntry(&BB))
      unreachableBBs.push_back(&BB);
  return unreachableBBs;
}

bool RemoveUnusedPass::eraseUnreachableBB(Function &F, FunctionAnalysisManager &FAM) {
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

  return !unreachables.empty();
}

bool RemoveUnusedPass::eraseUnusedInstruction(Function &F, FunctionAnalysisManager &FAM) {
  SmallPtrSet<Value*, 16> usedValues = this->getUsedValues(F);
  SmallPtrSet<Instruction*, 16> unusedInsts;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (!usedValues.count(inst)) unusedInsts.insert(inst);
  }
  this->eraseInstructions(unusedInsts);

  return !unusedInsts.empty();
}

void RemoveUnusedPass::eraseInstructions(SmallPtrSet<Instruction*, 16> insts) {
  queue<Instruction*> safeRemoveInsts;
  while (true) {
    for (Instruction *inst : insts)
      if (inst->use_empty()) safeRemoveInsts.push(inst);
    
    if (safeRemoveInsts.empty()) return;

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

template<typename Lambda>
SmallPtrSet<Value*, 16> RemoveUnusedPass::getRecursiveUsers(Value *v, SmallPtrSet<Value*, 16> &checked, Lambda condition) {
  SmallPtrSet<Value*, 16> nonInst;
  checked.insert(v);
  if (condition(v)) nonInst.insert(v);
  for (User *u : v->users()) 
    if (!checked.count(u)) set_union(nonInst, this->getRecursiveUsers(u, checked, condition));      
  return nonInst;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getNecessaryValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues;

  // Contain GlobalValue and related Non-ControlFlow values
  set_union(necessaryValues, this->getGloalValues(F));

  // Contain Arguments
  for (Argument &arg : F.args()) necessaryValues.insert(&arg);

  SmallPtrSet<Value*, 16> prevNecessaryValues;
  do {
    set_union(prevNecessaryValues, necessaryValues);
    for (Value *v : prevNecessaryValues) {
      SmallPtrSet<Value*, 16> checked;
      set_union(necessaryValues, this->getRecursiveUsers(v, checked, [](Value *u) { return !isa<BasicBlock>(u); }));
      Instruction *inst = dyn_cast<Instruction>(v);
      if (inst && inst->mayHaveSideEffects())
        for (Value *op : inst->operands())
          necessaryValues.insert(op);
    }
  } while (prevNecessaryValues.size() != necessaryValues.size());

  necessaryValues.clear();
  for (Value *v : prevNecessaryValues) {
    Instruction *inst = dyn_cast<Instruction>(v);
    if (inst && !inst->mayHaveSideEffects()) continue;
    StoreInst *sInst = dyn_cast<StoreInst>(v);
    if (sInst && !sInst->getValueOperand()->getType()->isPointerTy()) continue;
    necessaryValues.insert(v);
  }

  // Contain unsafe to remove values except store instruction
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (!inst->isSafeToRemove() && !isa<StoreInst>(inst))
      necessaryValues.insert(inst);
  }

  return necessaryValues;
}

SmallPtrSet<Instruction*, 16> RemoveUnusedPass::getSideEffectsInst(Function &F) {
  SmallPtrSet<Instruction*, 16> sideSet;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (inst->mayHaveSideEffects()) sideSet.insert(inst);
  }
  return sideSet;
}

template<typename Lambda1, typename Lambda2> 
SmallPtrSet<Value*, 16> RemoveUnusedPass::getInstPredecessors(Instruction *inst, Lambda1 valueCondition, Lambda2 operandCondition) {
  SmallPtrSet<Value*, 16> predSet;
  queue<Value*> predQueue;
  predQueue.push(inst);
  while (!predQueue.empty()) {
    Value* v = predQueue.front();
    predQueue.pop();
    if (!predSet.count(v)) {
      if (valueCondition(v)) predSet.insert(v);
    
      if (Instruction *I = dyn_cast<Instruction>(v))
        for (Value *op : I->operands())
          if (operandCondition(I, op))
            predQueue.push(op);
    }
  }
  return predSet;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getPredecessorSet(SmallPtrSet<Value*, 16> &valueSet, Function &F) {
  SmallPtrSet<Value*, 16> predSet;
  for (Value *v : valueSet) {
    if (Instruction *inst = dyn_cast<Instruction>(v))
      set_union(predSet, this->getInstPredecessors(inst, 
      [](Value *v) { return !isa<BasicBlock>(v) && !isa<ConstantData>(v) && !isa<ConstantExpr>(v); }, 
      [](Instruction *I, Value *v) { return true; }));
  }
  return predSet;
}

bool RemoveUnusedPass::isNecessaryInst(Instruction *inst, SmallPtrSet<Value*, 16> &necessarySet) {
  Instruction *target = inst;
  if (StoreInst *sInst = dyn_cast<StoreInst>(inst)) {
    Value *ptr = sInst->getPointerOperand();
    if (necessarySet.count(ptr)) return true;
    if (Instruction *targetInst = dyn_cast<Instruction>(ptr)) target = targetInst;
    else return false;
  }
  
  SmallPtrSet<Value*, 16> intersectSet(necessarySet);
  set_intersect(intersectSet, this->getInstPredecessors(target, 
      [](Value *v) { return !isa<BasicBlock>(v) && !isa<ConstantData>(v) && !isa<ConstantExpr>(v); }, 
      [](Instruction *I, Value *v) { 
        if (GetElementPtrInst *Ginst = dyn_cast<GetElementPtrInst>(I)) return v == Ginst->getPointerOperand();
        return true; 
      }));
  if (!intersectSet.empty()) return true;

  return false;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getUsedValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues = this->getNecessaryValues(F);
  SmallPtrSet<Instruction*, 16> sideInsts = this->getSideEffectsInst(F);

  while (true) {
    for (Instruction *inst : sideInsts)
      if (isNecessaryInst(inst, necessaryValues))
        necessaryValues.insert(inst);

    SmallPtrSet<Value*, 16> predecessorSet = this->getPredecessorSet(necessaryValues, F);
    SmallPtrSet<Value*, 16> diffSet(predecessorSet);
    set_subtract(diffSet, necessaryValues);

    if (diffSet.empty()) break;
    else set_union(necessaryValues, diffSet);
  }
  
  return necessaryValues;
}

PreservedAnalyses RemoveUnusedPass::run(Function &F, FunctionAnalysisManager &FAM) {
  bool isUnreachableExist = this->eraseUnreachableBB(F, FAM);
  bool isUnusedExist = this->eraseUnusedInstruction(F, FAM);
  return isUnreachableExist || isUnusedExist ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
