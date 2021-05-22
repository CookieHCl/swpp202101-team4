#include "Team4Header.h"

/*
  This Pass remove unreachable blocks and unused variables.
  It reduces cost caused by unused calculation.
*/

template<typename Iter>
void RemoveUnusedPass::printValues(Iter &iter, string str) {
  outs() << str << " START\n";
  for (Value *v : iter)
    outs() << *v << "\n";
  outs() << str << " END\n";
}

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
    for (Instruction &inst : *BB)
      if (!inst.use_empty()) inst.dropAllReferences();
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
    if (!usedValues.count(inst)) unusedInsts.insert(inst);
  }
  this->eraseInstructions(unusedInsts);
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

SmallPtrSet<Value*, 16> RemoveUnusedPass::getRecursiveNonControlFlowValues(Value *v, SmallPtrSet<Value*, 16> &checked) {
  SmallPtrSet<Value*, 16> nonInst;
  for (User *u : v->users())
    if (!checked.count(u)) {
      checked.insert(u);
      if (!isa<Instruction>(u) && !isa<BasicBlock>(u)) {
        nonInst.insert(u);
        set_union(nonInst, this->getRecursiveNonControlFlowValues(u, checked));
      }
    }
  return nonInst;
}

SmallPtrSet<Value*, 16> RemoveUnusedPass::getNecessaryValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues;

  // Contain GlobalValue and related Non-ControlFlow values
  for (GlobalValue *gv : this->getGloalValues(F)) {
    necessaryValues.insert(gv);
    SmallPtrSet<Value*, 16> checked;
    set_union(necessaryValues, this->getRecursiveNonControlFlowValues(gv, checked));
  }

  // Contain Arguments
  for (Argument &arg : F.args()) necessaryValues.insert(&arg);


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
  this->eraseUnreachableBBs(F, FAM);
  this->eraseUnusedInstructions(F, FAM);
  return PreservedAnalyses::all();
}
