#include "MatmulTransposePass.h"


/* In order to optimize the loop, its induction need to be a PHInode. (Scalar Evolution)
 */
bool MatmulTransposePass::makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM) {
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
  return isChanged;
}

/* STEP 1. remove %Sum.0 register in matmul1.ll
 *  this step is only for matmul1
 *  Interchange is possible only when %sum.0 is deleted.
 */
bool MatmulTransposePass::rmSumRegister(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  for (Loop *L : LI.getLoopsInPreorder()) {
    if (L->isInnermost()) {
      BasicBlock *incomming;    /* Incomming edge */
      BasicBlock *backedge;     /* Back edge */
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

          // All values to find ptrElement of storeInst should be loopInvariant about L
          //  C[i * dim + j] should be loopInvariant in matmul1
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
                if(I1 -> hasOneUser()) relPtrInsts.push_back(I1);   /* there is only one user */
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
            logs() << "[@" << F.getName() <<"] remove sum reg!!" << "\n";
            return true;
          }
        }
      }
    }
  }
  return false;
}

/* find Canonical Variable
 *  Initialize with constant and added by a constant.
 */
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

/* CHECK 1. constant range
 *  range of CanonicalVariable must be constant
 */
bool MatmulTransposePass::isConstantRange(Loop *L, const SCEV *target, Loop *Outer, ScalarEvolution &SE, bool from) {
  SCEVTypes scevType = target->getSCEVType();
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
        rev &= this->isConstantRange(L, narySCEV->getOperand(i), Outer, SE, from);
      return rev;
    }
    case SCEVTypes::scAddRecExpr: {
      const SCEVAddRecExpr *addRecSCEV = dyn_cast<SCEVAddRecExpr>(target);
      if(from && addRecSCEV->getLoop() != L) return false;
      if(!from && addRecSCEV->getLoop() == L) return false;
      bool rev = true;
      for (int i = 0; i < addRecSCEV->getNumOperands(); ++i)
        rev &= this->isConstantRange(L, addRecSCEV->getOperand(i), Outer, SE, from);
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
      return this->isConstantRange(L, castSCEV->getOperand(), Outer, SE, from);
    }
    default: return false;
  }
}

/* CHECK 2. OuterLoop body = InnerLoop body
 *  no Call Instruction & no Store or Load in OuterLoop body
 */
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

void MatmulTransposePass::updateVectorizableCnt(const SCEV *target, Loop *InnerLoop, Loop *OuterLoop) {
  SCEVTypes scevType = target->getSCEVType();
  switch (scevType) {
    case SCEVTypes::scAddRecExpr: {
      const SCEVAddRecExpr *addRecSCEV = dyn_cast<SCEVAddRecExpr>(target);
      if(!addRecSCEV || addRecSCEV->getNumOperands() != 2) return;
      const SCEVConstant *addv = dyn_cast<SCEVConstant>(addRecSCEV->getOperand(1));
      if(!addv || !addv->getValue()->isOne()) return;
      if(addRecSCEV->getLoop() == InnerLoop) vectorizableCntBefore++;
      if(addRecSCEV->getLoop() == OuterLoop) vectorizableCntAfter++;
      return;
    }
    case SCEVTypes::scTruncate:
    case SCEVTypes::scZeroExtend:
    case SCEVTypes::scSignExtend: {
      const SCEVCastExpr *castSCEV = dyn_cast<SCEVCastExpr>(target);
      updateVectorizableCnt(castSCEV->getOperand(), InnerLoop, OuterLoop);
      return;
    }
    default: return;
  }
}

/* This function is for isThereOnlySigmaStore (CHECK 3.)
 *  Check the target is loop invariant about OuterLoop
 *  or Load Instruction that has loop invariant index and source different from ptrAddr.
 */
bool MatmulTransposePass::isValidtoAdded(const SCEV *target, Value *ptrAddr, Loop *InnerLoop, Loop *OuterLoop, ScalarEvolution &SE) {
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
        rev &= this->isValidtoAdded(narySCEV->getOperand(i), ptrAddr, InnerLoop, OuterLoop, SE);
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
        updateVectorizableCnt(SE.getSCEV(ptrInst->getOperand(1)), InnerLoop, OuterLoop);
        return this->isValidtoAdded(SE.getSCEV(ptrInst->getOperand(1)), ptrAddr, InnerLoop, OuterLoop, SE);
      }
      return false;
    }
    case SCEVTypes::scTruncate:
    case SCEVTypes::scZeroExtend:
    case SCEVTypes::scSignExtend: {
      const SCEVCastExpr *castSCEV = dyn_cast<SCEVCastExpr>(target);
      return this->isValidtoAdded(castSCEV->getOperand(), ptrAddr, InnerLoop, OuterLoop, SE);
    }
    default: return false;
  }
}

/* CHECK 3. check body load store relation
 *  The store Instruction should have this form below.
 *  
 *  Array[(loop Invariant index)] += (loop Invariant value / Load from another array)
 */
bool MatmulTransposePass::isThereOnlySigmaStore(Loop *InnerLoop, Loop *OuterLoop, ScalarEvolution &SE) {  
  for (BasicBlock *BB : InnerLoop->getBlocks()) {
    for (Instruction &I : *BB) {
      if(dyn_cast<CallInst>(&I)) return false;

      StoreInst *store;
      if(!(store = dyn_cast<StoreInst>(&I))) continue;    /* Check for all store Instructions */
      if(!store->isSimple()) return false;

      // store value to ptr -> value should be (load ptr) + (something)
      // that means *ptr += something; in C
      BinaryOperator* addInst = dyn_cast<BinaryOperator>(store->getValueOperand());
      if(!addInst || addInst->getOpcode() != Instruction::Add) return false;

      LoadInst *load = dyn_cast<LoadInst>(addInst->getOperand(0));
      if(!load || !load->isSimple()) return false;
      if(store->getPointerOperand() != load->getPointerOperand()) return false;

      // now check (something)
      GetElementPtrInst *ptrInst = dyn_cast<GetElementPtrInst>(store->getPointerOperand());
      if(!ptrInst) return false;
      
      // load, store
      updateVectorizableCnt(SE.getSCEV(ptrInst->getOperand(1)), InnerLoop, OuterLoop);
      updateVectorizableCnt(SE.getSCEV(ptrInst->getOperand(1)), InnerLoop, OuterLoop);
      
      Value *v = addInst->getOperand(1);
      const SCEV *value = SE.getSCEV(v);
      if(!isValidtoAdded(value, ptrInst->getOperand(0), InnerLoop, OuterLoop, SE)) return false;
      logs() << "  SCEV += " << *value << "\n";
    }
  }
  return true;
}

/* STEP 2. InterChange Loop if possible
 */
bool MatmulTransposePass::loopInterChange(Function &F, FunctionAnalysisManager &FAM) {
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

  Loop *Outer = nullptr;

  for (Loop *L : LI.getLoopsInPreorder()) {
    if(L->isInnermost() && Outer) {
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

      // Analyze the structure of the loop
      //  and determining whether the structure is what we want.
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

      // Now We should determine whether it is interchangeable or not.
      // There are three criteria for judging.
      // 
      // CHECK 1. constant range
      //  range of CanonicalVariable must be constant
      if(!(isConstantRange(L, SE.getSCEV(InnerLoopCanVar), Outer, SE, true) &&
           isConstantRange(Outer, SE.getSCEV(OuterLoopCanVar), Outer, SE, true))) continue;
      
      // CHECK 2. OuterLoop body = InnerLoop body
      //  no Call Instruction & no Store or Load in OuterLoop body
      if(!noAdditionalOuterBody(L, Outer)) continue;

      // CHECK 3. check body load store relation
      //  The store Instruction should have this form below.
      //  
      //  Array[(loop Invariant index)] += (loop Invariant value / Load from another array)
      vectorizableCntBefore = 0;
      vectorizableCntAfter = 0;
      if(!isThereOnlySigmaStore(L, Outer, SE)) continue;

      logs() << "  vectorizableCntBefore " << vectorizableCntBefore << "\n";
      logs() << "  vectorizableCntAfter  " << vectorizableCntAfter << "\n";
      
      // check profit
      if(vectorizableCntBefore > vectorizableCntAfter) continue;

      // Interchange L and Outer
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
      
      FAM.invalidate(F, PreservedAnalyses::none());
      logs() << "[@" << F.getName() <<"] InterChanged!!" << "\n";
      return true;
    }
    Outer = L;
  }
  return false;
}

/* STEP 3. hoist Load Instructions to OuterLoop when LoopInvariant about InnerLoop
 */
bool MatmulTransposePass::hoistLoad(Function &F, FunctionAnalysisManager &FAM) {
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

  Loop *Outer = nullptr;

  for (Loop *L : LI.getLoopsInPreorder()) {
    if(L->isInnermost() && Outer) {
      for (BasicBlock *BB : L->getBlocks()) {
        // L : InnerLoop  Outer : OuterLoop
        for (Instruction &I : *BB) {
          LoadInst *load = dyn_cast<LoadInst>(&I);
          if(!load || !load->isSimple()) continue;
          GetElementPtrInst *ptrInst = dyn_cast<GetElementPtrInst>(load->getPointerOperand());
          if(!ptrInst) continue;
          if(!L->isLoopInvariant(ptrInst->getOperand(0))) continue;
          if(!isConstantRange(L, SE.getSCEV(ptrInst->getOperand(1)), L, SE, false)) continue;

          vector<Instruction *> relPtrInsts;
          relPtrInsts.push_back(ptrInst);
          bool flag = true;

          for (int now = 0; now < relPtrInsts.size(); ++now) {
            for (unsigned i = 0; i < relPtrInsts[now]->getNumOperands(); ++i) {
              Value *v = relPtrInsts[now]->getOperand(i);
              Instruction *I1 = dyn_cast<Instruction>(v);
              if(I1 && I1->getParent() == BB) {
                if(I1->hasOneUser()) relPtrInsts.push_back(I1);
                else flag = false;
              }
              else if(!L->isLoopInvariant(v)) flag = false;
            }
            if(!flag) break;
          }
          if(!flag) continue;

          Instruction *insertBeforeThis = nullptr;
          for (BasicBlock *BB1 : Outer->getBlocks()) {
            if(L->contains(BB1)) continue;
            BranchInst *br = dyn_cast<BranchInst>(BB1->getTerminator());
            if(br && br->getNumSuccessors() == 1 && br->getSuccessor(0) == L->getHeader()){
              insertBeforeThis = br;
            }
          }
          if(!insertBeforeThis) continue;
          
          Instruction *cloned;
          for(int i = relPtrInsts.size() - 1; i >= 0 ; --i) {
            cloned = relPtrInsts[i]->clone();
            cloned->insertBefore(insertBeforeThis);
            relPtrInsts[i]->replaceAllUsesWith(cloned);
          }
          cloned = load->clone();
          cloned->insertBefore(insertBeforeThis);
          load->replaceAllUsesWith(cloned);

          // now we can move relPtrInsts into Loop
          load->eraseFromParent();
          for(int i = 0; i < relPtrInsts.size(); ++i)
            relPtrInsts[i]->eraseFromParent();
          
          FAM.invalidate(F, PreservedAnalyses::none());
          logs() << "[@" << F.getName() <<"] Hoisted!!" << "\n";
          return true;
        }
      }
    }
    Outer = L;
  }
  return false;
}

PreservedAnalyses MatmulTransposePass::run(Function &F, FunctionAnalysisManager &FAM) {
  logs() << "[MatmulTranspose Progress in @" << F.getName() << "]\n";
  bool isChanged = false;

  // In order to optimize the loop, its induction need to be a PHInode. (Scalar Evolution)
  isChanged |= this->makeAllocaAsPHI(F, FAM);

  // STEP 1. remove %Sum.0 register in matmul1.ll
  //  this step is only for matmul1
  //  Interchange is possible only when %sum.0 is deleted.
  logs() << "rmSumReg!\n";
  isChanged |= this->rmSumRegister(F, FAM);

  // STEP 2. InterChange Loop if possible
  logs() << "Interchange!\n";
  isChanged |= this->loopInterChange(F, FAM);

  // STEP 3. hoist Load Instructions to OuterLoop when LoopInvariant about InnerLoop
  logs() << "hoistLoad!\n";
  while(this->hoistLoad(F, FAM)) isChanged = true;

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
