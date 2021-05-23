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

  DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element2", ArrayRef<Type*>({Vec2Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element4", ArrayRef<Type*>({Vec4Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(Int64Type, "extract_element8", ArrayRef<Type*>({Vec8Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(Vec2Int64Type, "vload2", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(Vec4Int64Type, "vload4", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(Vec8Int64Type, "vload8", ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(VoidType, "vstore2", ArrayRef<Type*>({LVTWO(Int64Type), Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(VoidType, "vstore4", ArrayRef<Type*>({LVTWO(LVTWO(Int64Type)), Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION(VoidType, "vstore8", ArrayRef<Type*>({LVTWO(LVTWO(LVTWO(Int64Type))), Int64PtrType, Int64Type}), M);
}

/*
Loop Carry Dependencec가 없어야 한다.
Increment Unit과 refernece Unit이 같아야 한다.
Increment 방식일 때, 가능하다.
1. Increment Unit을 찾는 방법
1) Increment 변수를 찾는다.
2) Increment part를 찾는다.
3) Increment에 더해지는 값을 찾는다.

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


SmallVector<Instruction*, 16> LoopVectorizePass::getInductionPtrs(BasicBlock *BB, Value *ind) {
  SmallVector<Instruction*, 16> indPtrs;
  for (Instruction &inst : *BB) {
    Value *ptr, *v, *mayInd, *c;
    GetElementPtrInst *GED;
    LoadInst *lInst = dyn_cast<LoadInst>(&inst);
    Instruction *lInst2;
    if (lInst && (GED = dyn_cast<GetElementPtrInst>(lInst->getPointerOperand()))) {
      if ((ptr = dyn_cast<LoadInst>(GED->getPointerOperand())) && 
          match(GED->getOperand(1), m_SExt(m_Add(m_Instruction(lInst2), m_Value(c))))) {
        indPtrs.push_back(lInst);
      }
    }
  }
  return indPtrs;
}


int LoopVectorizePass::numInduction(Value *v) {
  for (User *u : v->users()) {
    LoadInst *lInst = dyn_cast<LoadInst>(u);
    if (lInst) {
      for (User *ul : lInst->users()) {
        Value *x, *y;
        if (match(ul, m_Add(m_Value(x), m_Value(y)))) {
          if (ConstantInt *c = dyn_cast<ConstantInt>(x)) return c->getSExtValue();
          if (ConstantInt *c = dyn_cast<ConstantInt>(y)) return c->getSExtValue();
        }
      }
    }
  }
  return 0;
}


LoopVectorizePass::Induction LoopVectorizePass::getInduction(Loop *L, LoopInfo &LI) {
  SmallVector<BasicBlock*, 16> BBs;
  L->getExitingBlocks(BBs);
  Induction induction = Induction(nullptr, 0);
  for (BasicBlock *BB : BBs){
    Instruction *inst = BB->getTerminator();
    Value *x, *y;
    ICmpInst::Predicate pred;
    BasicBlock *bb_true, *bb_false;
    if (match(inst, m_Br(m_ICmp(pred, m_Value(x), m_Value(y)), bb_true, bb_false))) {
      LoadInst *lInst = dyn_cast<LoadInst>(x);
      int num;
      if (lInst && (num = this->numInduction(lInst->getPointerOperand()))) induction = Induction(lInst->getPointerOperand(), num);
      lInst = dyn_cast<LoadInst>(y);
      if (lInst && (num = this->numInduction(lInst->getPointerOperand()))) {
        if (induction.v == nullptr) induction = Induction(lInst->getPointerOperand(), num);
        else induction = Induction(nullptr, 0);
      }
    }
  }
  return induction;
}


PreservedAnalyses LoopVectorizePass::vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE) {
  Induction induction = this->getInduction(L, LI);
  if (induction.v == nullptr) return PreservedAnalyses::all();

  outs() << "Induction : " << *induction.v <<'\n';

  
  for (BasicBlock *BB : L->getBlocksVector()) {
    SmallVector<Instruction*, 16> ptrs = this->getInductionPtrs(BB, induction.v);
    if (!ptrs.empty()) {
      
    }
    for (Value *v : ptrs)
      outs() << *v;
  }

      

  return PreservedAnalyses::all();
}

PreservedAnalyses LoopVectorizePass::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  for (Loop *L : LI.getLoopsInPreorder()) {
    if (L->isInnermost())
      this->vectorize(L, LI, SE);
  }
  
  return PreservedAnalyses::none();
}
