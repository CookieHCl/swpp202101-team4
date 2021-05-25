#include "LoopVectorizePass.h"
  

/*
 *                  Loop Vectorize Pass
 *
 *  This Pass vectorize loop store/load instruction.
 *  Individual memory accesses are expensive.
 *  Convert memory access instructions into vector instructions.
 *  
 */ 


LoopVectorizePass::LoopVectorizePass(Module &M) : PassInfoMixin() {
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

/*
Loop Carry Dependencec가 없어야 한다.
Increment Unit과 refernece Unit이 같아야 한다.
Increment 방식일 때, 가능하다.
1. Increment Unit을 찾는 방법
1) Increment 변수를 찾는다.
2) Increment part를 찾는다.
3) Increment에 더해지는 값을 찾는다. 여기까지 됐다.

2. Reference Unit을 찾는 방법
1) Body를 찾는다.
2) 한 데이터가 Body 안에서 Increment Unit 만큼의 

일단 루프 안에서 store를 모두 찾는다. store 가 없으면 루프가 이뤄질수가 없다. (SSA)


바꾸는 방법
1. Reorder 한다
2. 묶는다.
3. 바꾼다.


vector instructino이 있다고 더 못바꾸는건... 맞네.
그래 vector instruction이 있으면 못바꾼다.
대신 loop unrolling 에서 더 refine 하면 좋겠다. constant size loop를 펴줬으면 좋겠다.

Loop unrolling 에서 할 것.
1) Constant size loop를 없앤다
2) 8개 안 찬 단위는 더 편다.


body에 load가 있으면 vectorize 한다.
store가 있어도 vectorize 한다.

*/



PreservedAnalyses LoopVectorizePass::vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE) {
  PreservedAnalyses PA = PreservedAnalyses::all();
  outs() << *L << "\n";

  // check induction argument
  auto opLB = L->getBounds(SE);
  if (!opLB.hasValue()) return PA;

  Loop::LoopBounds LB = opLB.getValue();
  ConstantInt *step = dyn_cast<ConstantInt>(LB.getStepValue());
  if (!step) return PA;

  int64_t stepSize = step->getSExtValue();
  if (stepSize != 4) return PA;

  // check data
  

  outs() << *LB.getStepValue() << "\n";
  outs() << LB.getInitialIVValue() << "\n";
  outs() << LB.getFinalIVValue() << "\n";

  return PreservedAnalyses::all();
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

void LoopVectorizePass::rotateLoop(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty()) return;

  LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  SimplifyQuery SQ = getBestSimplifyQuery(FAM, F);

  bool isChanged = false;
  for (Loop *L : LI.getLoopsInPreorder())
    if ((isChanged = LoopRotation(L, &LI, &TTI, &AC, &DT, &SE, &MSSAU, SQ, true, -1, true)))
      LAM.invalidate(*L, getLoopPassPreservedAnalyses());

  if (isChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

void LoopVectorizePass::makeSimplifyLCSSA(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty()) return;

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  
  bool isGlobalChanged = false;
  for (Loop *L : LI.getLoopsInPreorder()) {
    bool isChanged = false;
    isChanged |= simplifyLoop(L, &DT, &LI, &SE, &AC, &MSSAU, false);
    isChanged |= formLCSSARecursively(*L, DT, &LI, nullptr);
    isGlobalChanged |= isChanged;
    if (isChanged) LAM.invalidate(*L, getLoopPassPreservedAnalyses());
  }

  if (isGlobalChanged) FAM.invalidate(F, PreservedAnalyses::none());
}

PreservedAnalyses LoopVectorizePass::run(Function &F, FunctionAnalysisManager &FAM) {
  this->makeAllocaAsPHI(F, FAM);
  this->makeSimplifyLCSSA(F, FAM);
  this->rotateLoop(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  for (Loop *L : LI.getLoopsInPreorder())
    if (L->isInnermost())
      this->vectorize(L, LI, SE);

  return PreservedAnalyses::none();
}
