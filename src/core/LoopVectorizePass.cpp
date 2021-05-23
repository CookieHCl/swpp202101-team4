#include "Team4Header.h"
  

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


PreservedAnalyses LoopVectorizePass::run(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();

  return PreservedAnalyses::all();
}
