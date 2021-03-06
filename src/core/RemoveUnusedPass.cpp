#include "RemoveUnusedPass.h"

/*
 *                      Remove Unused Pass
 *
 *  This Pass remove unreachable blocks and unused variables.
 *  It reduces cost caused by unused calculation.
 *  This Pass assumes all arguments and global values are necessary.
 *  
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
    if (!usedValues.contains(inst)) unusedInsts.insert(inst);
  }
  this->eraseInstructions(unusedInsts);

  return !unusedInsts.empty();
}

void RemoveUnusedPass::eraseInstructions(SmallPtrSet<Instruction*, 16> insts) {
  // Erase instructions safely until cannot erase.
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

SmallPtrSet<Value*, 16> RemoveUnusedPass::getGlobalValues(Function &F) {
  // Get global Values of Module (parent of Function F)
  SmallPtrSet<Value*, 16> globalValues;
  Module *M = F.getParent();

  for (auto gv_iter = M->global_begin(), gv_end = M->global_end(); gv_iter != gv_end; ++gv_iter) {
    GlobalValue *gv = &*gv_iter;
    globalValues.insert(gv);
  }

  return globalValues;
}

template<typename Lambda>
SmallPtrSet<Value*, 16> RemoveUnusedPass::getRecursiveUsers(Value *v, SmallPtrSet<Value*, 16> &checked, Lambda condition) {
  // Get DFS users
  // Only contain given condition
  SmallPtrSet<Value*, 16> nonInst;
  checked.insert(v);
  if (condition(v)) nonInst.insert(v);
  for (User *u : v->users())
    if (!checked.contains(u)) set_union(nonInst, this->getRecursiveUsers(u, checked, condition));
  return nonInst;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getNecessaryValues(Function &F) {
  // Contain GlobalValue and related Non-ControlFlow values
  SmallPtrSet<Value*, 16> necessaryValues = this->getGlobalValues(F);

  // Contain Arguments
  for (Argument &arg : F.args()) necessaryValues.insert(&arg);

  SmallPtrSet<Value*, 16> prevNecessaryValues;
  SmallPtrSet<Value*, 16> checked;
  do {
    set_union(prevNecessaryValues, necessaryValues);
    for (Value *v : prevNecessaryValues) {
      set_union(necessaryValues, this->getRecursiveUsers(v, checked, [](Value *u) { return !isa<BasicBlock>(u); }));
      Instruction *inst = dyn_cast<Instruction>(v);
      if (inst && inst->mayHaveSideEffects())
        for (Value *op : inst->operands())
          necessaryValues.insert(op);
    }
  } while (prevNecessaryValues.size() != necessaryValues.size());

  necessaryValues.clear();
  for (Value *v : prevNecessaryValues) {
    // mayHaveSideEffects contains "exception" and "memoryWrite" instructions.
    // The instructions except them are definitely unnecessary.
    Instruction *inst = dyn_cast<Instruction>(v);
    if (inst && !inst->mayHaveSideEffects()) continue;
    // To check saving value in getUsedValue, all proceeded pointer values must be saved.
    // This can remain unused pointer address.
    StoreInst *sInst = dyn_cast<StoreInst>(v);
    if (sInst && !sInst->getValueOperand()->getType()->isPointerTy()) continue;
    necessaryValues.insert(v);
  }

  // Contain unsafe to remove values except store instruction
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    // isSafeToRemove means (!CallInst || !mayHaveSideEffect) && (!isTerminator).
    // StoreInst is handled specially, so do not contain.
    if (!inst->isSafeToRemove() && !isa<StoreInst>(inst))
      necessaryValues.insert(inst);
  }

  return necessaryValues;
}

SmallPtrSet<Instruction*, 16> RemoveUnusedPass::getSideEffectsInst(Function &F) {
  // return mayHaveSideEffects(exception, write) instructions in Function F
  SmallPtrSet<Instruction*, 16> sideSet;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (inst->mayHaveSideEffects()) sideSet.insert(inst);
  }
  return sideSet;
}

template<typename Lambda1, typename Lambda2>
SmallPtrSet<Value*, 16> RemoveUnusedPass::getInstPredecessors(Instruction *inst, Lambda1 valueCondition, Lambda2 operandCondition) {
  // BFS Instruction Predecessors.
  // BFS search operand of operator
  SmallPtrSet<Value*, 16> predSet;
  queue<Value*> predQueue;
  predQueue.push(inst);
  while (!predQueue.empty()) {
    Value* v = predQueue.front();
    predQueue.pop();
    if (!predSet.contains(v)) {
      if (valueCondition(v)) predSet.insert(v);

      if (Operator *OP = dyn_cast<Operator>(v))
        for (Value *op : OP->operands())
          if (operandCondition(OP, op))
            predQueue.push(op);
    }
  }
  return predSet;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getPredecessorSet(SmallPtrSet<Value*, 16> &valueSet, Function &F) {
  // get predecessors of given valueSet in function
  SmallPtrSet<Value*, 16> predSet;
  for (Value *v : valueSet) {
    if (Instruction *inst = dyn_cast<Instruction>(v))
      set_union(predSet, this->getInstPredecessors(inst,
      [](Value *v) { return !isa<BasicBlock>(v) && !isa<ConstantData>(v) && !isa<ConstantExpr>(v); },
      [](Operator *OP, Value *v) { return true; }));
  }
  return predSet;
}

bool RemoveUnusedPass::isNecessaryInst(Instruction *inst, SmallPtrSet<Value*, 16> &necessarySet) {
  // check given instruction is necessary.
  Instruction *target = inst;
  if (StoreInst *sInst = dyn_cast<StoreInst>(inst)) {
    Value *ptr = sInst->getPointerOperand();
    if (necessarySet.contains(ptr)) return true;
    if (Instruction *targetInst = dyn_cast<Instruction>(ptr)) target = targetInst;
    else return false;
  }
  
  SmallPtrSet<Value*, 16> intersectSet(necessarySet);
  set_intersect(intersectSet, this->getInstPredecessors(target,
      // Do not contain BasicBlock
      [](Value *v) { return !isa<BasicBlock>(v); },
      [](Operator *OP, Value *v) {
        // For GEP, contain only Pointer operand.
        // Note that this condition is minimum condition except GEP condition.
        // This can be reinforced.
        if (GEPOperator *GEPOP = dyn_cast<GEPOperator>(OP)) return v == GEPOP->getPointerOperand();
        return true;
      }));
  if (!intersectSet.empty()) return true;

  return false;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getUsedValues(Function &F) {
  // Return may used values set in function
  // Ideally, the return set does not contain used values.
  // Main idea of getUsedValues is as follows.
  // 0. Init necessary values
  // 1. Extend necessary values by checking and inserting instructions may cause SideEffects
  // 2. For neceesary values, contains their predecessors
  // 3. Goto 1 until necessary values set does not change.
  SmallPtrSet<Value*, 16> necessaryValues = this->getNecessaryValues(F);
  SmallPtrSet<Instruction*, 16> sideInsts = this->getSideEffectsInst(F);

  unsigned prevSize, currentSize;
  do {
    for (Instruction *inst : sideInsts)
      if (isNecessaryInst(inst, necessaryValues))
        necessaryValues.insert(inst);

    SmallPtrSet<Value*, 16> predecessorSet = this->getPredecessorSet(necessaryValues, F);

    prevSize = necessaryValues.size();
    set_union(necessaryValues, predecessorSet);
    currentSize = necessaryValues.size();
  } while (prevSize != currentSize);
  
  return necessaryValues;
}

PreservedAnalyses RemoveUnusedPass::run(Function &F, FunctionAnalysisManager &FAM) {
  bool isUnreachableExist = this->eraseUnreachableBB(F, FAM);
  bool isUnusedExist = this->eraseUnusedInstruction(F, FAM);
  return isUnreachableExist || isUnusedExist ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
