#include "MatmulTransposePass.h"


void MatmulTransposePass::makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);

  SmallVector<AllocaInst*, 8> allocaInstVector;
  bool isChanged = false;

  while (true) {
    allocaInstVector.clear();
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
      if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(&*I))
        if (isAllocaPromotable(allocaInst)) allocaInstVector.push_back(allocaInst);

    if (allocaInstVector.empty()) break;

    PromoteMemToReg(allocaInstVector, DT, &AC);
    isChanged = true;
  }

  if (isChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

/*
 * remove %sum.0 value in matmul1.ll for Transpose
 */
void MatmulTransposePass::rmSumRegister(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  for (Loop *L : LI.getLoopsInPreorder()) {
    if (L->isInnermost()) {
      BasicBlock *incomming;    // Incomming edge
      BasicBlock *backedge;     // Back edge
      if(!L->getIncomingAndBackEdge(incomming, backedge)) continue;
      PHINode *loop_cond = L->getCanonicalInductionVariable();

      for (BasicBlock *BB : L->getBlocks()) {
        for (Instruction &I : *BB) {
          PHINode *phi = dyn_cast<PHINode>(&I);
          if(!phi || loop_cond == phi) continue;

          // check I is %sum.0 in matmul1.ll
          if(phi->getNumIncomingValues() != 2) continue;

          Value *v = dyn_cast<Value>(phi);  // %sum.0
          Value *v0 = phi->getIncomingValue(0);
          Value *v1 = phi->getIncomingValue(1);
          BasicBlock *bb0 = phi->getIncomingBlock(0);
          BasicBlock *bb1 = phi->getIncomingBlock(1);

          // one is from incomming(initialize) other is from Loop L 
          if(bb1 == incomming) { swap(bb0, bb1); swap(v0, v1); }
          if(bb0 != incomming || !(L->contains(bb1))) continue;

          // initialize with 0
          Constant *zero = dyn_cast<Constant>(v0);
          if(!zero || !(zero->isZeroValue())) continue;

          // v1 = add %sum.0 %mul.11
          BinaryOperator *addInst = dyn_cast<BinaryOperator>(v1);
          if(!(addInst && addInst->getOpcode() == Instruction::Add && addInst->getOperand(0) == v)) continue;

          // c[i * dim + j] = sum in backedge
          StoreInst *storeInst = nullptr;
          GetElementPtrInst *ptrInst = nullptr;
          bool flag = false;
          for(auto u = v->user_begin(); u != v->user_end(); ++u) {
            if(*u == addInst) continue;  // except addInst
            if((storeInst = dyn_cast<StoreInst>(*u)) != nullptr && storeInst->isSimple() &&
                (ptrInst = dyn_cast<GetElementPtrInst>(getLoadStorePointerOperand(storeInst))) != nullptr) flag = true;
            else flag = false;
          }
          if(!flag) continue;

          // All values to find ptrElement of storeInst must be loopInvariant
          //  that means C[i * dim + j] must be loopInvariant
          
          BasicBlock *for_end = storeInst->getParent();
          vector<Instruction *> relPtrInsts;
          relPtrInsts.push_back(ptrInst);
          flag = true;

          for (int now = 0; now < relPtrInsts.size(); ++now) {
            for (unsigned i = 0; i < relPtrInsts[now]->getNumOperands(); ++i) {
              Value *v = relPtrInsts[now]->getOperand(i);
              if(!L->isLoopInvariant(v)) flag = false;
              Instruction *I1 = dyn_cast<Instruction>(v);
              if(I1 && I1->getParent() == for_end) {
                if(I1 -> hasOneUser()) relPtrInsts.push_back(I1);
                else flag = false;
              }
            }
            if(!flag) break;
          }
          
          if(flag && DT.dominates(incomming, for_end)) {
            // now we can move relPtrInsts into Loop
            storeInst->removeFromParent();
            for(int i = 0; i < relPtrInsts.size(); ++i)
              relPtrInsts[i]->removeFromParent();
            for(int i = relPtrInsts.size() - 1; i >= 0 ; --i)
              addInst->getParent()->getInstList().insert(addInst->getIterator(), relPtrInsts[i]);
            addInst->getParent()->getInstList().insertAfter(addInst->getIterator(), storeInst);

            // create loadInst before addInst
            LoadInst *load = new LoadInst(ptrInst->getSourceElementType(), ptrInst, Twine(), addInst);
            addInst->setOperand(0, load);
            storeInst->setOperand(0, addInst);
            phi->eraseFromParent();

            FAM.invalidate(F, PreservedAnalyses::none());
            return;
          }
        }
      }
    }
  }
}

PHINode *MatmulTransposePass::getCanonicalVariable(Loop *L) {
  BasicBlock *H = L->getHeader();

  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  if (!L->getIncomingAndBackEdge(Incoming, Backedge))
    return nullptr;

  // Loop over all of the PHI nodes, looking for a canonical indvar.
  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (ConstantInt *CI =
            dyn_cast<ConstantInt>(PN->getIncomingValueForBlock(Incoming)))
      if (Instruction *Inc =
              dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge)))
        if (Inc->getOpcode() == Instruction::Add && Inc->getOperand(0) == PN)
          if (ConstantInt *CI = dyn_cast<ConstantInt>(Inc->getOperand(1)))
            return PN;
  }
  return nullptr;
}

bool MatmulTransposePass::isConstantRange(Loop *L, const SCEV *target, Loop *Outer, ScalarEvolution &SE) {
  SCEVTypes scevType = target->getSCEVType();
  logs() << "isConstantRange : " << *target <<"\n";
  switch (scevType) {
    case SCEVTypes::scMulExpr:
    case SCEVTypes::scUDivExpr:
    case SCEVTypes::scUMaxExpr:
    case SCEVTypes::scSMaxExpr:
    case SCEVTypes::scUMinExpr:
    case SCEVTypes::scSMinExpr:
    case SCEVTypes::scAddExpr: {
      const SCEVNAryExpr *narySCEV = dyn_cast<SCEVNAryExpr>(target);
      bool rev = true;
      for (int i = 0; i < narySCEV->getNumOperands(); ++i)
        rev &= this->isConstantRange(L, narySCEV->getOperand(i), Outer, SE);
      return rev;
    }
    case SCEVTypes::scAddRecExpr: {
      const SCEVAddRecExpr *addRecSCEV = dyn_cast<SCEVAddRecExpr>(target);
      if(addRecSCEV->getLoop() != L) return false;
      bool rev = true;
      for (int i = 0; i < addRecSCEV->getNumOperands(); ++i)
        rev &= this->isConstantRange(L, addRecSCEV->getOperand(i), Outer, SE);
      return rev;
    }
    case SCEVTypes::scConstant: return true;
    case SCEVTypes::scUnknown:{
      const SCEVUnknown *valSCEV = dyn_cast<SCEVUnknown>(target);
      return Outer->isLoopInvariant(valSCEV->getValue());
    }
    case SCEVTypes::scTruncate:
    case SCEVTypes::scZeroExtend:
    case SCEVTypes::scSignExtend: {
      const SCEVCastExpr *castSCEV = dyn_cast<SCEVCastExpr>(target);
      return this->isConstantRange(L, castSCEV->getOperand(), Outer, SE);
    }
    default: return false;
  }
}

bool MatmulTransposePass::noAdditionalOuterBody(Loop *InnerLoop, Loop *OuterLoop) {
  for (BasicBlock *BB : OuterLoop->getBlocks()) {
    if(InnerLoop->contains(BB)) continue;
    for (Instruction &I : *BB) {
      if(!InnerLoop->isLoopInvariant(&I)) return false;
      if(dyn_cast<StoreInst>(&I) || dyn_cast<CallInst>(&I)) return false;
    }
  }
  return true;
}

bool MatmulTransposePass::isValidtoAdded(const SCEV *target, Value *ptrAddr, Loop *OuterLoop, ScalarEvolution &SE) {
  SCEVTypes scevType = target->getSCEVType();
  switch (scevType) {
    case SCEVTypes::scAddRecExpr:
    case SCEVTypes::scMulExpr:
    case SCEVTypes::scUDivExpr:
    case SCEVTypes::scUMaxExpr:
    case SCEVTypes::scSMaxExpr:
    case SCEVTypes::scUMinExpr:
    case SCEVTypes::scSMinExpr:
    case SCEVTypes::scAddExpr: {
      const SCEVNAryExpr *narySCEV = dyn_cast<SCEVNAryExpr>(target);
      bool rev = true;
      for (int i = 0; i < narySCEV->getNumOperands(); ++i)
        rev &= this->isValidtoAdded(narySCEV->getOperand(i), ptrAddr, OuterLoop, SE);
      return rev;
    }
    case SCEVTypes::scConstant: return true;
    case SCEVTypes::scUnknown:{
      const SCEVUnknown *valSCEV = dyn_cast<SCEVUnknown>(target);
      Value *Inst = valSCEV->getValue();
      if(OuterLoop->isLoopInvariant(Inst)) return true;
      if(LoadInst *load = dyn_cast<LoadInst>(Inst)) {
        if(!load->isSimple()) return false;
        GetElementPtrInst *ptrInst = dyn_cast<GetElementPtrInst>(load->getPointerOperand());
        if(!ptrInst) return false;
        if(ptrInst->getOperand(0) == ptrAddr) return false;
        return this->isValidtoAdded(SE.getSCEV(ptrInst->getOperand(1)), ptrAddr, OuterLoop, SE);
      }
      return false;
    }
    case SCEVTypes::scTruncate:
    case SCEVTypes::scZeroExtend:
    case SCEVTypes::scSignExtend: {
      const SCEVCastExpr *castSCEV = dyn_cast<SCEVCastExpr>(target);
      return this->isValidtoAdded(castSCEV->getOperand(), ptrAddr, OuterLoop, SE);
    }
    default: return false;
  }
}

bool MatmulTransposePass::isThereOnlySigmaStore(Loop *InnerLoop, Loop *OuterLoop, ScalarEvolution &SE) {
  for (BasicBlock *BB : InnerLoop->getBlocks()) {
    for (Instruction &I : *BB) {
      if(dyn_cast<CallInst>(&I)) return false;

      StoreInst *store;
      if(!(store = dyn_cast<StoreInst>(&I))) continue;
      if(!store->isSimple()) return false;

      // store value to ptr -> value should be (load ptr) + (something)
      // that means *ptr += something; in C
      BinaryOperator* addInst = dyn_cast<BinaryOperator>(store->getValueOperand());
      if(!addInst || addInst->getOpcode() != Instruction::Add) return false;                    // should be addInst

      LoadInst *load = dyn_cast<LoadInst>(addInst->getOperand(0));
      if(!load || !load->isSimple()) return false;
      if(store->getPointerOperand() != load->getPointerOperand()) return false;   // should be same ptr

      // now check (something)
      GetElementPtrInst *ptrInst = dyn_cast<GetElementPtrInst>(store->getPointerOperand());
      if(!ptrInst) return false;
      
      Value *v = addInst->getOperand(1);
      const SCEV *value = SE.getSCEV(v);
      if(!isValidtoAdded(value, ptrInst->getOperand(0), OuterLoop, SE)) return false;
      logs() << "  SCEV += " << *value << "\n";
    }
  }
  return true;
}

void MatmulTransposePass::loopInterChange(Function &F, FunctionAnalysisManager &FAM) {
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  DependenceInfo &DI = FAM.getResult<DependenceAnalysis>(F);
  OptimizationRemarkEmitter &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  Loop *Outer = nullptr;

  for (Loop *L : LI.getLoopsInPreorder()) {
    if(L->isInnermost()) {
      BasicBlock *InnerLoopHeader = L->getHeader();
      BasicBlock *OuterLoopHeader = Outer->getHeader();
      BasicBlock *InnerLoopLatch = L->getLoopLatch();
      BasicBlock *OuterLoopLatch = Outer->getLoopLatch();
      BasicBlock *InnerIncomming, *InnerBackedge;
      BasicBlock *OuterIncomming, *OuterBackedge;

      if(!L->getIncomingAndBackEdge(InnerIncomming, InnerBackedge)) continue;
      if(!Outer->getIncomingAndBackEdge(OuterIncomming, OuterBackedge)) continue;
      
      BranchInst *OuterLoopLatchBI =
          dyn_cast<BranchInst>(OuterLoopLatch->getTerminator());
      BranchInst *InnerLoopLatchBI =
          dyn_cast<BranchInst>(InnerLoopLatch->getTerminator());
      BranchInst *OuterLoopHeaderBI =
          dyn_cast<BranchInst>(OuterLoopHeader->getTerminator());
      BranchInst *InnerLoopHeaderBI =
          dyn_cast<BranchInst>(InnerLoopHeader->getTerminator());
      BranchInst *OuterIncommingBI =
          dyn_cast<BranchInst>(OuterIncomming->getTerminator());
      BranchInst *InnerIncommingBI =
          dyn_cast<BranchInst>(InnerIncomming->getTerminator());

      if(!(OuterLoopLatchBI && InnerLoopLatchBI && OuterLoopHeaderBI && InnerLoopHeaderBI && OuterIncommingBI && InnerIncommingBI)) continue;
      if(!(OuterLoopLatchBI->getNumSuccessors() == 1 && InnerLoopLatchBI->getNumSuccessors() == 1 &&
          OuterLoopHeaderBI->getNumSuccessors() == 2 && InnerLoopHeaderBI->getNumSuccessors() == 2 &&
          OuterIncommingBI->getNumSuccessors() == 1 && InnerIncommingBI->getNumSuccessors() == 1)) continue;
      if(!(OuterLoopLatchBI->getSuccessor(0) == OuterLoopHeader && InnerLoopLatchBI->getSuccessor(0) == InnerLoopHeader)) continue;
      if(!(Outer->contains(OuterLoopHeaderBI->getSuccessor(0)) && L->contains(InnerLoopHeaderBI->getSuccessor(0)))) continue;

      bool flag = true;
      BranchInst *InnerToLatchBI, *OuterToLatchBI;
      for (BasicBlock *BB : L->getBlocks()) {
        if(BB == InnerLoopHeader) continue;
        BranchInst *br = dyn_cast<BranchInst>(BB->getTerminator());
        if(!br || br->getNumSuccessors() != 1) flag = false;
        else if(br->getSuccessor(0) == InnerLoopLatch) InnerToLatchBI = br;
      }
      for (BasicBlock *BB : Outer->getBlocks()) {
        if(BB == OuterLoopHeader || L->contains(BB)) continue;
        BranchInst *br = dyn_cast<BranchInst>(BB->getTerminator());
        if(!br || br->getNumSuccessors() != 1) flag = false;
        else if(br->getSuccessor(0) == OuterLoopLatch) OuterToLatchBI = br;
      }
      if(!flag) continue;

      PHINode *InnerLoopCanVar = getCanonicalVariable(L);
      PHINode *OuterLoopCanVar = getCanonicalVariable(Outer);
      if(!InnerLoopCanVar || !OuterLoopCanVar) continue;

      // CHECK 1. constant range
      if(!(isConstantRange(L, SE.getSCEV(InnerLoopCanVar), Outer, SE) &&
           isConstantRange(Outer, SE.getSCEV(OuterLoopCanVar), Outer, SE))) continue;
      
      // CHECK 2. OuterLoop body = InnerLoop body
      if(!noAdditionalOuterBody(L, Outer)) continue;

      // CHECK 3. check body load store relation
      if(!isThereOnlySigmaStore(L, Outer, SE)) continue;

      InnerToLatchBI->setSuccessor(0, OuterLoopLatch);
      OuterToLatchBI->setSuccessor(0, InnerLoopLatch);
      
      BasicBlock *OuterLoopBody = OuterLoopHeaderBI->getSuccessor(0);
      BasicBlock *InnerLoopBody = InnerLoopHeaderBI->getSuccessor(0);
      BasicBlock *OuterLoopEnd = OuterLoopHeaderBI->getSuccessor(1);
      BasicBlock *InnerLoopEnd = InnerLoopHeaderBI->getSuccessor(1);

      OuterLoopHeaderBI->setSuccessor(0, InnerLoopBody);
      OuterLoopHeaderBI->setSuccessor(1, InnerLoopEnd);
      InnerLoopHeaderBI->setSuccessor(0, OuterLoopBody);
      InnerLoopHeaderBI->setSuccessor(1, OuterLoopEnd);

      OuterLoopCanVar->setIncomingBlock(0, InnerIncomming);
      InnerLoopCanVar->setIncomingBlock(0, OuterIncomming);

      OuterIncommingBI->setSuccessor(0, InnerLoopHeader);
      InnerIncommingBI->setSuccessor(0, OuterLoopHeader);

      logs() << "InnerLoopLatchBI : " << *InnerLoopLatchBI << "\n";
      logs() << "InnerLoopHeaderBI : " << *InnerLoopHeaderBI << "\n";
      logs() << "OuterLoopLatchBI : " << *OuterLoopLatchBI << "\n";
      logs() << "OuterLoopHeaderBI : " << *OuterLoopHeaderBI << "\n";
      
      FAM.invalidate(F, PreservedAnalyses::none());
      return;
    }
    Outer = L;
  }
}

PreservedAnalyses MatmulTransposePass::run(Function &F, FunctionAnalysisManager &FAM) {
  bool isChanged = false;
  logs() << "[MatmulTranspose Progress in @" << F.getName() << "]\n";

  // In order to optimize the loop, its induction need to be a PHInode. (Scalar Evolution)
  this->makeAllocaAsPHI(F, FAM);
  this->rmSumRegister(F, FAM);
  this->loopInterChange(F, FAM);

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
