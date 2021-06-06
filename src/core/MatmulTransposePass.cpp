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
          if(!(addInst && addInst->getOpcode() == 13 && addInst->getOperand(0) == v)) continue;

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

void MatmulTransposePass::loopInterChange(Function &F, FunctionAnalysisManager &FAM) {
  if(F.getName() != "matmul") return;

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
      
      BasicBlock *OuterLoopBody = OuterLoopHeaderBI->getSuccessor(0);
      BasicBlock *InnerLoopBody = InnerLoopHeaderBI->getSuccessor(0);
      BasicBlock *OuterLoopEnd = OuterLoopHeaderBI->getSuccessor(1);
      BasicBlock *InnerLoopEnd = InnerLoopHeaderBI->getSuccessor(1);

      PHINode *InnerLoopcond = L->getCanonicalInductionVariable();
      PHINode *OuterLoopcond = Outer->getCanonicalInductionVariable();

      OuterLoopHeaderBI->setSuccessor(0, InnerLoopBody);
      OuterLoopHeaderBI->setSuccessor(1, InnerLoopEnd);
      InnerLoopHeaderBI->setSuccessor(0, OuterLoopBody);
      InnerLoopHeaderBI->setSuccessor(1, OuterLoopEnd);

      OuterLoopcond->setIncomingBlock(0, InnerIncomming);
      InnerLoopcond->setIncomingBlock(0, OuterIncomming);

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
