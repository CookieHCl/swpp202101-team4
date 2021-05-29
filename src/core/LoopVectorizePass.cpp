#include "LoopVectorizePass.h"


/*
 *                  Loop Vectorize Pass
 *
 *  This Pass vectorize loop store/load instruction.
 *  Individual memory accesses are expensive.
 *  Convert memory access instructions into vector instructions.
 *  
 */ 


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

// Vectorize Instructions composed with three step
// 1) Check whether the memory access are consecutive.
// 2) Check whether loop carried dependence exists.
// 3) Vectorize
// 3-1) Reorder instruction. For load, up-reorder and store, down-reorder
// 3-2) Add function call (vload/vstore) and replace instruction.
bool LoopVectorizePass::vectorizeInstructions(LoopVectorizePass::InstChain &instChain, ScalarEvolution &SE,
                                              const DataLayout &DL, DominatorTree &DT) {
  if (instChain.size() < 2) return false;

  bool isChanged = false;
  Value *Ptr = getLoadStorePointerOperand(instChain.front());
  const bool isLoad = isa<LoadInst>(instChain.front());
  const SCEV *PtrSCEV = SE.getSCEV(Ptr);
  unsigned PtrBitWidth = DL.getPointerSizeInBits(0);
  APInt Size(PtrBitWidth, DL.getTypeStoreSize(Ptr->getType()->getPointerElementType()));
  const int lenChain = instChain.size();
  logs() << "PtrWidth : " << PtrBitWidth << "\nSize     : " << Size << "\n";
  logs() << "PtrSCEV  : " << *PtrSCEV << "\nIsLoad   : " << (isLoad ? "true" : "false") << "\n";

  // This is very strong condition. Should be weaken later.
  // This hard condition covers marginal condition. e.g. range(0, 30, 4) is not vectorized.
  // SCEV manages margin condition, so non-consecutive case are failed.
  for (int i = 1; i < lenChain; ++i) {
    const SCEV *PtrSCEVA = SE.getSCEV(getLoadStorePointerOperand(instChain[i]));
    const SCEV *ConstDelta = SE.getConstant(Size * i);
    const SCEV *Delta = SE.getMinusSCEV(PtrSCEVA, PtrSCEV);
    logs() << "PtrSCEVA : " << *PtrSCEVA << " ( Delta : " << *Delta << ")\n";
    if (Delta != ConstDelta) return isChanged;
  }

  logs() << "[Chain is CONSECUTIVE]\n";

  // Max Vectorize unit is 8. Therefore, split in 8.
  // This is not the best case: 0-6 impossible and 7 possible + 8 possible and 9-15 impossible
  // Require more elaborate scheduling.
  for (int i = 0; i < lenChain; i += 8) {
    const int num = ((i + 8) < lenChain ? (i + 8) : lenChain) - i;

    if (num < 2) continue;

    Instruction *first = instChain[i];
    Instruction *last = instChain[i + num - 1];
    InstChain toVectorize;
    int64_t mask = 0;  // Load mask, but also can be used for store. (e.g. 0101 means vectorize 0th and 2nd element)
    // The 1-masked location means load location index. e.g. 0001 means 0th element, 0010 means 1st element.

    // Detecting loop carried dependence. Note that this is also strong condition, wil be weaken.
    // Example of Loop Carried Dependence:
    //   A[i + 1] = A[i];
    //   A[i + 2] = A[i + 1];  in this case, we cannot vectorize because of sequential memory access pattern.
    std::vector<std::pair<Instruction*, const SCEV*>> SCEVItems;
    if (isLoad) {
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
      mask |= 1;
      toVectorize.push_back(instChain[i]);

      for (auto P = first->getIterator(), E = last->getIterator(); P != E; ++P)
        if (StoreInst *storeInst = dyn_cast<StoreInst>(&*P))
          SCEVItems.push_back(make_pair(&*P, SE.getSCEV(storeInst->getPointerOperand())));

      // Note that the condition. DT.dominates means pair.first(StoreInst) is preceeding  current load Inst
      // and the memory access location is same (Scalar Evolution term)
      for (int j = 1; j < num; ++j) {
        Instruction *current = instChain[i + j];
        const SCEV *loadLocation = SE.getSCEV(getLoadStorePointerOperand(current));
        if (any_of(SCEVItems,
            [current, loadLocation, &DT, &SE](std::pair<Instruction*, const SCEV*> pair) {
              return DT.dominates(pair.first, current) && (SE.getMinusSCEV(pair.second, loadLocation)->isZero());
            })) continue;
        mask |= (1 << j);
        toVectorize.push_back(current);
      }
    } else {
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
      for (auto P = first->getIterator(), E = last->getIterator(); P != E; ++P)
        if (LoadInst *loadInst = dyn_cast<LoadInst>(&*P))
          SCEVItems.push_back(make_pair(&*P, SE.getSCEV(loadInst->getPointerOperand())));

      // Note that the condition. DT.dominates means current store is preceeding pair.first(LoadInst)
      // and the memory access location is same (Scalar Evolution term)
      for (int j = 0; j < num - 1; ++j) {
        Instruction *current = instChain[i + j];
        const SCEV *storeLocation = SE.getSCEV(getLoadStorePointerOperand(current));
        if (any_of(SCEVItems,
            [current, storeLocation, &DT, &SE](std::pair<Instruction*, const SCEV*> pair) {
              return DT.dominates(current, pair.first) && (SE.getMinusSCEV(pair.second, storeLocation)->isZero());
            })) continue;
        mask |= (1 << j);
        toVectorize.push_back(current);
      }
      toVectorize.push_back(instChain[i + num - 1]);
      mask |= (1 << (num - 1));
    }

    const int newNum = toVectorize.size();
    if (newNum < 2) continue;

    isChanged = true;

    const int dimension = 4 * int(newNum > 4) + 2 * int(newNum > 2) + int(newNum > 1) + 1;

    if (isLoad) vectorizeLoadInsts(toVectorize, dimension, mask, first);
    else vectorizeStoreInsts(toVectorize, dimension, mask, first);
  }
  return isChanged;
}

bool LoopVectorizePass::vectorizeMap(LoopVectorizePass::InstChainMap &instChainMap, ScalarEvolution &SE, const DataLayout &DL,  DominatorTree &DT) {
  bool isChanged = false;
  for (std::pair<ChainID, InstChain> &chainItem : instChainMap) {
    logs() << "[Source] " << *chainItem.first << "\n";
    for (unsigned idx = 0; idx < chainItem.second.size(); ++idx)
      logs() << ((idx < chainItem.second.size() - 1) ? "|- " : "`- ") << *chainItem.second[idx] << "\n";
    isChanged |= vectorizeInstructions(chainItem.second, SE, DL, DT);
  }
  return isChanged;
}

bool LoopVectorizePass::vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE, TargetTransformInfo &TTI, const DataLayout &DL,  DominatorTree &DT) {
  bool isChanged = false;

  // About BasicBlocks in one Loop, collect vectorizable instructions and vectorize.
  for (BasicBlock *BB : L->getBlocks()) {
    InstChainMap loadChainMap, storeChainMap;
    std::tie(loadChainMap, storeChainMap) = collectInstructions(BB, TTI);
    isChanged |= vectorizeMap(loadChainMap, SE, DL, DT);
    isChanged |= vectorizeMap(storeChainMap, SE, DL, DT);
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
