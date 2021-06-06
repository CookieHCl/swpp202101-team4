#include "LoopVectorizePass.h"


/*
 *                  Loop Vectorize Pass
 *
 *  This Pass vectorize loop store/load instruction.
 *  Individual memory accesses are expensive.
 *  Convert memory access instructions into vector instructions.
 *  
 */ 


#define ADDRECSCEV_START_INDEX 0
#define MAX_VECTOR_SIZE 8
#define LVTWO(x) x, x
#define DECLARE_VECTOR_FUNCTION(retType, name, argTypeArray, M) \
  Function::Create(FunctionType::get(retType, argTypeArray, false), GlobalValue::ExternalLinkage, Twine(name), &M)\


LoopVectorizePass::LoopVectorizePass(Module &M, bool isVerbose) : PassInfoMixin(), isVerbose(isVerbose) {
  LLVMContext &context = M.getContext();
  this->Int64Type = Type::getInt64Ty(context);
  this->Int64PtrType = Type::getInt64PtrTy(context);
  this->Vec2Int64Type = VectorType::get(this->Int64Type, 2, false);
  this->Vec4Int64Type = VectorType::get(this->Int64Type, 4, false);
  this->Vec8Int64Type = VectorType::get(this->Int64Type, 8, false);
  this->VoidType = Type::getVoidTy(context);

  extractElement2 = DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element2", ArrayRef<Type*>({Vec2Int64Type, Int64Type}), M);
  extractElement4 = DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element4", ArrayRef<Type*>({Vec4Int64Type, Int64Type}), M);
  extractElement8 = DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element8", ArrayRef<Type*>({Vec8Int64Type, Int64Type}), M);
  vLoad2 = DECLARE_VECTOR_FUNCTION(Vec2Int64Type, "vload2", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  vLoad4 = DECLARE_VECTOR_FUNCTION(Vec4Int64Type, "vload4", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  vLoad8 = DECLARE_VECTOR_FUNCTION(Vec8Int64Type, "vload8", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  vStore2 = DECLARE_VECTOR_FUNCTION(VoidType, "vstore2", ArrayRef<Type*>({LVTWO(Int64Type), Int64PtrType, Int64Type}), M);
  vStore4 = DECLARE_VECTOR_FUNCTION(VoidType, "vstore4", ArrayRef<Type*>({LVTWO(LVTWO(Int64Type)), Int64PtrType, Int64Type}), M);
  vStore8 = DECLARE_VECTOR_FUNCTION(VoidType, "vstore8", ArrayRef<Type*>({LVTWO(LVTWO(LVTWO(Int64Type))), Int64PtrType, Int64Type}), M);
}

LoopVectorizePass::ChainID LoopVectorizePass::getChainID(const Value *Ptr) {
  const Value *objectPtr = getUnderlyingObject(Ptr);
  if (const auto *selectInst = dyn_cast<SelectInst>(objectPtr)) return selectInst->getCondition();
  return objectPtr;
}

std::pair<LoopVectorizePass::InstChainMap, LoopVectorizePass::InstChainMap> 
LoopVectorizePass::collectInstructions(BasicBlock *BB, TargetTransformInfo &TTI) {
  InstChainMap loadChainMap;
  InstChainMap storeChainMap;

  for (Instruction &I : *BB) {
    if (!I.mayReadOrWriteMemory()) continue;
    if (LoadInst *loadInst = dyn_cast<LoadInst>(&I)) {
      if (!loadInst->isSimple()) continue;
      if (!TTI.isLegalToVectorizeLoad(loadInst)) continue;
      IntegerType *Ty = dyn_cast<IntegerType>(loadInst->getType());
      if (!Ty || Ty->getBitWidth() != 64u) continue;

      ChainID cid = getChainID(loadInst->getPointerOperand());
      loadChainMap[cid].push_back(&I);
    }
    else if (StoreInst *storeInst = dyn_cast<StoreInst>(&I)) {
      if (!storeInst->isSimple()) continue;
      if (!TTI.isLegalToVectorizeStore(storeInst)) continue;
      IntegerType *Ty = dyn_cast<IntegerType>(storeInst->getValueOperand()->getType());
      if (!Ty || Ty->getBitWidth() != 64u) continue;

      ChainID cid = getChainID(storeInst->getPointerOperand());
      storeChainMap[cid].push_back(&I);
    }
  }

  return {loadChainMap, storeChainMap};
}

Function* LoopVectorizePass::getVectorCallee(int dimension, LoopVectorizePass::CalleeType calleeType) {
  Function *callee;
  switch (dimension) {
    case 2: callee = (calleeType == CalleeType::LOAD) ? vLoad2 : (calleeType == CalleeType::STORE ? vStore2 : extractElement2); break;
    case 4: callee = (calleeType == CalleeType::LOAD) ? vLoad4 : (calleeType == CalleeType::STORE ? vStore4 : extractElement4); break;
    case 8: callee = (calleeType == CalleeType::LOAD) ? vLoad8 : (calleeType == CalleeType::STORE ? vStore8 : extractElement8); break;
    default: llvm_unreachable("Wrong dimension");
  }
  return callee;
}

void LoopVectorizePass::fillVectorArgument(Value *address, const int64_t mask, SmallVector<Value*, 8> &Args) {
  Value *Ptr = address;
  Value *Mask = ConstantInt::getSigned(Int64Type, mask);
  Args.push_back(Ptr);
  Args.push_back(Mask);
}

void LoopVectorizePass::vectorizeLoadInsts(InstChain &instChain, const int dimension, const int64_t mask, Instruction *first) {
  const int num = instChain.size();

  for (int i = 1; i < num; ++i)
    instChain[i]->moveAfter(instChain[i - 1]);

  Function *load = getVectorCallee(dimension, CalleeType::LOAD);
  Function *extract = getVectorCallee(dimension, CalleeType::EXTRACT);
  Value *Address = getLoadStorePointerOperand(first);
  SmallVector<Value*, 8> Args;
  fillVectorArgument(Address, mask, Args);

  CallInst *vLoad = CallInst::Create(load->getFunctionType(), load, ArrayRef<Value*>(Args), "", instChain.front());

  int chainIndex = 0;
  for (int i = 0; i < dimension; ++i) {
    if (int64_t(1 << i) & mask) {
      Instruction *inst = instChain[chainIndex++];
      Value *Index = ConstantInt::getSigned(Int64Type, int64_t(i));
      CallInst *vExtract = CallInst::Create(extract->getFunctionType(), extract, ArrayRef<Value*>({vLoad, Index}), "");
      ReplaceInstWithInst(inst, vExtract);
    }
  }
}

void LoopVectorizePass::vectorizeStoreInsts(InstChain &instChain, const int dimension, const int64_t mask, Instruction *first) {
  const int num = instChain.size();

  for (int i = num - 2 ; i >= 0; --i)
    instChain[i]->moveBefore(instChain[i + 1]);

  Function *store = getVectorCallee(dimension, CalleeType::STORE);
  SmallVector<Value*, 8> Args;
  Value *Address = getLoadStorePointerOperand(first);

  int index = 0;
  for (int i = 0; i < dimension; ++i) {
    Value *arg;
    if ((1 << i) & mask) arg = dyn_cast<StoreInst>(instChain[index++])->getValueOperand();
    else arg = ConstantInt::getSigned(dyn_cast<IntegerType>(Int64Type), 0u);
    Args.push_back(arg);
  }

  fillVectorArgument(Address, mask, Args);

  CallInst::Create(store->getFunctionType(), store, ArrayRef<Value*>(Args), "", instChain.front());

  for (Instruction *inst : instChain)
    inst->eraseFromParent();
}

template<typename Type> int LoopVectorizePass::getSCEVOperandUnmatchedIndex(const Type *scev1, const Type *scev2) {
  if (scev1->getNumOperands() != scev2->getNumOperands()) return -1;
  if (scev1->getNoWrapFlags() != scev2->getNoWrapFlags()) return -1;

  int unmatchedIndex = -1;
  for (int i = 0; i < scev1->getNumOperands(); ++i) {
    const SCEV *op1 = scev1->getOperand(i);
    const SCEV *op2 = scev2->getOperand(i);
    if (op1 == op2) continue;

    if (unmatchedIndex >= 0) return -1;

    unmatchedIndex = i;
  }
  return unmatchedIndex;
}

LoopVectorizePass::ConProperty LoopVectorizePass::measureConsecutiveProperty(const SCEV *scev1, const SCEV *scev2, ScalarEvolution &SE) {
  SCEVTypes scevType = scev1->getSCEVType();

  if (scevType != scev2->getSCEVType()) return {false, 0};

  if (scev1 == scev2) return {true, 0};

  const SCEV *Delta = SE.getMinusSCEV(scev2, scev1);
  if (const SCEVConstant *constDelta = dyn_cast<SCEVConstant>(Delta)) {
    APInt Int = constDelta->getAPInt();
    return {true, Int.getSExtValue()};
  }

  switch (scevType) {
    case SCEVTypes::scAddExpr: {
      const SCEVAddExpr *addSCEV1 = dyn_cast<SCEVAddExpr>(scev1);
      const SCEVAddExpr *addSCEV2 = dyn_cast<SCEVAddExpr>(scev2);
      int unmatched = this->getSCEVOperandUnmatchedIndex(addSCEV1, addSCEV2);
      if (unmatched < 0) return {false, 0};

      return this->measureConsecutiveProperty(addSCEV1->getOperand(unmatched), addSCEV2->getOperand(unmatched), SE);
    }
    case SCEVTypes::scAddRecExpr: {
      const SCEVAddRecExpr *addRecSCEV1 = dyn_cast<SCEVAddRecExpr>(scev1);
      const SCEVAddRecExpr *addRecSCEV2 = dyn_cast<SCEVAddRecExpr>(scev2);
      int unmatched = this->getSCEVOperandUnmatchedIndex(addRecSCEV1, addRecSCEV2);
      if (unmatched != ADDRECSCEV_START_INDEX) return {false, 0};

      return this->measureConsecutiveProperty(addRecSCEV1->getStart(), addRecSCEV2->getStart(), SE);
    }
    case SCEVTypes::scTruncate:
    case SCEVTypes::scZeroExtend:
    case SCEVTypes::scSignExtend: {
      const SCEVIntegralCastExpr *intSCEV1 = dyn_cast<SCEVIntegralCastExpr>(scev1);
      const SCEVIntegralCastExpr *intSCEV2 = dyn_cast<SCEVIntegralCastExpr>(scev2);
      return this->measureConsecutiveProperty(intSCEV1->getOperand(), intSCEV2->getOperand(), SE);
    }
    case SCEVTypes::scMulExpr: {
      const SCEVMulExpr *mulSCEV1 = dyn_cast<SCEVMulExpr>(scev1);
      const SCEVMulExpr *mulSCEV2 = dyn_cast<SCEVMulExpr>(scev2);
      int unmatched = this->getSCEVOperandUnmatchedIndex(mulSCEV1, mulSCEV2);
      if (unmatched < 0) return {false, 0};

      unsigned multiplier = 1;
      for (int i = 0; i < mulSCEV1->getNumOperands(); ++i) {
        if (i == unmatched) continue;

        if (const SCEVConstant *constSCEV = dyn_cast<SCEVConstant>(mulSCEV1->getOperand(i))) {
          APInt Int = constSCEV->getAPInt();
          if (Int.getActiveBits() > 64) return {false, 0};
          multiplier *= Int.getZExtValue();
        } else return {false, 0};
      }

      ConProperty property = this->measureConsecutiveProperty(mulSCEV1->getOperand(unmatched), mulSCEV2->getOperand(unmatched), SE);

      return {property.first, property.second * multiplier};
    }
    default: return {false, 0};
  }
}

LoopVectorizePass::ConsecutiveScheme::ConsecutiveScheme(Instruction *inst, int sourceID, ScalarEvolution &SE) : sourceID(sourceID), memInst(inst) {
  this->ptr = getLoadStorePointerOperand(inst);
  this->scev = SE.getSCEV(this->ptr);
  this->displ = 0;
}

vector<LoopVectorizePass::ConScheme> LoopVectorizePass::createSchemes(LoopVectorizePass::InstChain &instChain, ScalarEvolution &SE) {
  std::vector<ConScheme> schemes;
  int numChain = instChain.size();
  for (int i = 0; i < numChain; ++i)
    schemes.push_back(ConScheme(instChain[i], i, SE));

  // Compare n * (n-1) / 2 times to check consecutivity
  for (int i = 0; i < numChain; ++i)
    for (int j = i + 1; j < numChain; ++j) {
      ConProperty conProperty = measureConsecutiveProperty(schemes[i].scev, schemes[j].scev, SE);
      if (!conProperty.first) continue;

      const int delta = conProperty.second;
      schemes[j].setSourceID(schemes[i].sourceID);
      schemes[j].setDispl(schemes[i].displ + delta);
    }

  logs() << "Created Schemes\n";
  for (ConScheme &scheme : schemes)
    logs() << scheme << "\n";

  return schemes;
}

bool LoopVectorizePass::isReferSameMemory(Instruction *inst, Instruction *pivot, ScalarEvolution &SE){
  const SCEV *pivotSCEV = SE.getSCEV(getLoadStorePointerOperand(pivot));
  const SCEV *targetSCEV = SE.getSCEV(getLoadStorePointerOperand(inst));
  ConProperty property = this->measureConsecutiveProperty(pivotSCEV, targetSCEV, SE);
  return property.first && (property.second == 0);
}

bool LoopVectorizePass::isLoadForwardable(Instruction *inst1, Instruction *inst2, DominatorTree &DT, ScalarEvolution &SE) {
  // For load chain, we should avoid "Load after Store".
  // Hence load are up-reordered, first chain element always can be vectorized.
  // And last chain element is difficult to vectorize
  // because there is the highest probability that at least one store exists.
  //   ADD                                                                LOAD
  //   LOAD    -> This can be vectorized. Just think LOAD goes up. ->     ADD
  //    ...                                                               ...
  //   STORE                                                              STORE
  //
  //  STORE
  //   ...    -> This can not be vectorized. If store move below LOAD and LOAD access same address as load,
  //   LOAD      the result will change.

  if (inst1->getParent() != inst2->getParent()) return false;
  if (!isa<LoadInst>(inst1) || !isa<LoadInst>(inst2)) return false;

  bool isOneDominate = DT.dominates(inst1, inst2);
  // -- Inst sequence -->
  //  ... To ...... From ...
  // From want to be in right back of To
  Instruction *instFrom = isOneDominate ? inst2 : inst1;
  Instruction *instTo = isOneDominate ? inst1 : inst2;

  SmallVector<Instruction*> intervalStore;
  for (auto P = instTo->getIterator(), E = instFrom->getIterator(); P != E; ++P)
    if (isa<StoreInst>(&*P)) intervalStore.push_back(&*P);

  return !any_of(intervalStore, [&](Instruction *inst){ return this->isReferSameMemory(inst, instFrom, SE); });
}

bool LoopVectorizePass::isStoreBackwardable(Instruction *inst1, Instruction *inst2, DominatorTree &DT, ScalarEvolution &SE) {
  // For Store chain, we should avoid "Load after Store".
  // Hence store are down-reordered, last chain element always can be vectorized.
  // First chain element is difficult to vectorize
  // because there is the highest probability that at least one load exists.
  //  LOAD                                                               LOAD
  //   ...    -> This can be vectorized. Just think STORE goes down. ->  ...
  //  STORE                                                              ADD
  //   ADD                                                              STORE
  //
  //  STORE
  //   ...    -> This can not be vectorized. If store move below LOAD and LOAD access same address as load,
  //   LOAD      the result will change.

  if (inst1->getParent() != inst2->getParent()) return false;
  if (!isa<StoreInst>(inst1) || !isa<StoreInst>(inst2)) return false;

  bool isOneDominate = DT.dominates(inst1, inst2);
  Instruction *instFrom = isOneDominate ? inst1 : inst2;
  Instruction *instTo = isOneDominate ? inst2 : inst1;

  SmallVector<Instruction*> intervalLoad;
  for (auto P = instFrom->getIterator(), E = instTo->getIterator(); P != E; ++P)
    if (isa<LoadInst>(&*P)) intervalLoad.push_back(&*P);

  return !any_of(intervalLoad, [&](Instruction *inst){ return this->isReferSameMemory(inst, instFrom, SE); });
}

// Vectorize Instructions composed with three step
// 1) Check whether the memory access are consecutive.
// 2) Check whether loop carried dependence exists.
// 3) Vectorize
// 3-1) Reorder instruction. For load, up-reorder and store, down-reorder
// 3-2) Add function call (vload/vstore) and replace instruction.
bool LoopVectorizePass::vectorizeInstructions(LoopVectorizePass::InstChain &instChain, Loop *L,  ScalarEvolution &SE,
                                              const DataLayout &DL, DominatorTree &DT) {
  const int lenChain = instChain.size();

  if (lenChain < 2) return false;

  bool isChanged = false;
  Value *Ptr = getLoadStorePointerOperand(instChain.front());
  const bool isLoad = isa<LoadInst>(instChain.front());

  unsigned PtrBitWidth = DL.getPointerSizeInBits(0);
  APInt Size(PtrBitWidth, DL.getTypeStoreSize(Ptr->getType()->getPointerElementType()));
  unsigned ptrSize = Size.getZExtValue();

  std::vector<ConScheme> conSchemes = createSchemes(instChain, SE);
  std::vector<ConScheme> validSchemes;
  std::copy_if(conSchemes.begin(), conSchemes.end(), back_inserter(validSchemes), [ptrSize](ConScheme s) { return s.displ % ptrSize == 0; });
  if (validSchemes.size() <= 1) return false;

  SetVector<int> sourceIDs;
  for (ConScheme scheme : validSchemes)
    sourceIDs.insert(scheme.sourceID);

  for (int sid : sourceIDs) {
    std::vector<ConScheme> sidSchemes;
    std::copy_if(validSchemes.begin(), validSchemes.end(), back_inserter(sidSchemes), [sid](ConScheme s) { return s.sourceID == sid; });
    std::queue<ConScheme> schemeQueue;
    for (ConScheme &scheme : sidSchemes) schemeQueue.push(scheme);

    logs() << "SourceID : " << sid << "\n";
    for (ConScheme &scheme : sidSchemes)
      logs() << scheme << "\n";

    while (!schemeQueue.empty()) {
      // For all iteration, at least one element is popped so must be terminated.
      ConScheme scheme = schemeQueue.front();
      schemeQueue.pop();
      std::vector<ConScheme> toVectorize = {scheme};

      const unsigned queueLen = schemeQueue.size();
      for (unsigned i = 0; i < queueLen; ++i) {
        ConScheme toCheck = schemeQueue.front();
        schemeQueue.pop();

        if ((toCheck.displ - scheme.displ < MAX_VECTOR_SIZE * ptrSize) &&
            ((isLoad && isLoadForwardable(scheme.memInst, toCheck.memInst, DT, SE)) ||
             (!isLoad && isStoreBackwardable(scheme.memInst, toCheck.memInst, DT, SE)))) {
            toVectorize.push_back(toCheck);
            continue;
          }

        schemeQueue.push(toCheck);
      }

      logs() << "To Vectorize\n";
      for (ConScheme scheme : toVectorize) logs() << scheme << "\n";

      if (toVectorize.size() < 2) continue;

      ConScheme maxDisplScheme = *max_element(toVectorize.begin(), toVectorize.end());
      ConScheme minDisplScheme = *min_element(toVectorize.begin(), toVectorize.end());

      // This is for only duplicated load/store with no dependence like
      // %0 = load i64, i64* @checked, align 8
      // %1 = load i64, i64* @checked, align 8
      // or
      // store i64 %1, i64* %add.ptr, align 8
      // store i64 %2, i64* %add.ptr, align 8
      // For load, replace all uses with first load.
      // For store, remove except last store.
      if (maxDisplScheme.displ == minDisplScheme.displ) {
        const unsigned duplicatedNum = toVectorize.size();
        if (isLoad) {
          Instruction *first = toVectorize.front().memInst;
          logs() << "First : " << *first << "\n";
          for (int i = 1; i < duplicatedNum; ++i) {
            logs() << *toVectorize[i].memInst << "\n";
            toVectorize[i].memInst->replaceAllUsesWith(first);
          }
        } else {
          for (int i = 0; i < duplicatedNum - 1; ++i)
            toVectorize[i].memInst->eraseFromParent();
        }
        isChanged = true;

        // Do not perform vectorize
        continue;
      }

      // For now on, there are at least two distinct ConsecutiveScheme
      const unsigned maxLen = 1 + (maxDisplScheme.displ - minDisplScheme.displ) / ptrSize;
      const int dimension = 4 * int(maxLen > 4) + 2 * int(maxLen > 2) + int(maxLen > 1) + 1;

      int64_t mask = 0;
      Instruction *first = minDisplScheme.memInst;
      InstChain vectorizeChain;
      for (ConScheme scheme : toVectorize) {
        vectorizeChain.push_back(scheme.memInst);
        unsigned index = (scheme.displ - minDisplScheme.displ) / ptrSize;
        mask |= (1 << index);
      }

      if (isLoad) vectorizeLoadInsts(vectorizeChain, dimension, mask, minDisplScheme.memInst);
      else vectorizeStoreInsts(vectorizeChain, dimension, mask, minDisplScheme.memInst);

      isChanged = true;
    }

  }

  return isChanged;
}

bool LoopVectorizePass::vectorizeMap(LoopVectorizePass::InstChainMap &instChainMap, Loop *L, ScalarEvolution &SE, const DataLayout &DL,  DominatorTree &DT) {
  bool isChanged = false;
  for (std::pair<ChainID, InstChain> &chainItem : instChainMap) {
    logs() << "[Source] " << *chainItem.first << "\n";
    for (unsigned idx = 0; idx < chainItem.second.size(); ++idx)
      logs() << ((idx < chainItem.second.size() - 1) ? "|- " : "`- ") << *chainItem.second[idx] << "\n";
    isChanged |= vectorizeInstructions(chainItem.second, L, SE, DL, DT);
  }
  return isChanged;
}

bool LoopVectorizePass::vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE, TargetTransformInfo &TTI, const DataLayout &DL,  DominatorTree &DT) {
  bool isChanged = false;

  // About BasicBlocks in one Loop, collect vectorizable instructions and vectorize.
  for (BasicBlock *BB : L->getBlocks()) {
    InstChainMap loadChainMap, storeChainMap;
    std::tie(loadChainMap, storeChainMap) = collectInstructions(BB, TTI);
    isChanged |= vectorizeMap(loadChainMap, L, SE, DL, DT);
    isChanged |= vectorizeMap(storeChainMap, L, SE, DL, DT);
  }

  return isChanged;
}

void LoopVectorizePass::makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM) {
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

PreservedAnalyses LoopVectorizePass::run(Function &F, FunctionAnalysisManager &FAM) {
  // In order to optimize the loop, its induction need to be a PHInode. (Scalar Evolution)
  this->makeAllocaAsPHI(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  const DataLayout &DL = F.getParent()->getDataLayout();

  logs() << "[LoopVectorize Progress in @" << F.getName() << "]\n";
  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if (L->isInnermost())
      isChanged |= this->vectorize(L, LI, SE, TTI, DL, DT);

  logs() << "[" << (isChanged ? "(CHANGED) " : "")<< "END LoopVectorize Progress in @" << F.getName() << "]\n\n";
  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
