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
    if (!usedValues.count(inst))
      unusedInsts.insert(inst);
  }
  this->eraseInstructions(unusedInsts);
}

void RemoveUnusedPass::eraseInstructions(SmallPtrSet<Instruction*, 16> insts) {
  this->printValues(insts, "Unused");
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
  return recursiveUsers;
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

SmallPtrSet<Value*, 16> RemoveUnusedPass::getGlobalRelatedNecessarySet(GlobalValue *gv) {
  SmallPtrSet<Value*, 16> checked;
  return this->getRecursiveNonControlFlowValues(gv, checked);
}


// Return Global variable, functions and arguments
// and local not safe to remove excpet store instructions.
SmallPtrSet<Value*, 16> RemoveUnusedPass::getNecessaryValues(Function &F) {
  SmallPtrSet<Value*, 16> necessaryValues;
  for (GlobalValue *gv : this->getGloalValues(F)) {
    necessaryValues.insert(gv);
    set_union(necessaryValues, this->getGlobalRelatedNecessarySet(gv));
  }

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
    if (!isa<Constant>(v)) {
      SmallPtrSet<Value*, 16> userSet = this->getRecursiveUsers(v);
      for (Value *u : userSet) {
        Instruction *inst = dyn_cast<Instruction>(u);
        BasicBlock *bb = inst->getParent();
        if (inst && inst->mayHaveSideEffects() && bb && bb->getParent() == &F) {
          StoreInst *sInst = dyn_cast<StoreInst>(inst);
          if (sInst && !userSet.count(sInst->getPointerOperand()) && 
              !valueSet.count(sInst->getPointerOperand())) continue;
          userInsts.insert(inst);
        }          
      }
    }
  }
  return userInsts;
}

SmallPtrSet<Instruction*, 16> RemoveUnusedPass::getSideEffectsInst(Function &F) {
  SmallPtrSet<Instruction*, 16> sideSet;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *inst = &*I;
    if (inst->mayHaveSideEffects()) {
      sideSet.insert(inst);
      outs() << *inst << "\n";
      for (Value *op : inst->operands())
        if (isa<Constant>(op))
          outs() << "  " << *op << "\n";
    }
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
      if (valueCondition(v))
        predSet.insert(v);
      
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
      set_union(predSet, this->getInstPredecessors(inst, [](Value *v) {
        return !isa<BasicBlock>(v) && !isa<ConstantData>(v) && !isa<ConstantExpr>(v);
      }, [](Instruction *I, Value *v) { return true; }));
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
  set_intersect(intersectSet, this->getInstPredecessors(target, [](Value *v) {
        return !isa<BasicBlock>(v) && !isa<ConstantData>(v) && !isa<ConstantExpr>(v);
      }, [](Instruction *I, Value *v) { 
        if (GetElementPtrInst *Ginst = dyn_cast<GetElementPtrInst>(I)) return v == Ginst->getPointerOperand();
        return true; 
      }));
  if (!intersectSet.empty()) return true;

  return false;
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
  SmallPtrSet<Instruction*, 16> sideInsts = this->getSideEffectsInst(F);
  outs() << "NECESSARY\n";
  for (Value *v : necessaryValues)
    outs() << *v << "\n";
  outs() << "NECESSARY END\n";

  outs() << "Side\n";
  for (Instruction *inst : sideInsts)
    outs() << *inst << "\n";
  outs() << "Side END\n";

  while (true) {
    for (Instruction *inst : sideInsts) {
      if (isNecessaryInst(inst, necessaryValues))
        necessaryValues.insert(inst);
    }
    outs() << "NECESSARY START\n";
    for (Value *n : necessaryValues)
      outs() << *n << "\n";
    outs() << "NECESSARY END\n";
    SmallPtrSet<Value*, 16> predecessorSet = this->getPredecessorSet(necessaryValues, F);
    outs() << "PRED START\n";
    for (Value *n : predecessorSet)
      outs() << *n << "\n";
    outs() << "PRED END\n";
    SmallPtrSet<Value*, 16> diffSet(predecessorSet);
    set_subtract(diffSet, necessaryValues);
    outs() << "DIFF START" << "\n";
    for (Value *v : diffSet)
      outs() << *v << "\n";
    outs() << "DIFF END" << "\n";
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
