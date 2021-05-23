#include "Team4Header.h"

void LoopVectorizePass::setVectorSetting(Module &M) {
  LLVMContext &context = M.getContext();
  this->Int64Type = Type::getInt64Ty(context);
  this->Int64PtrType = Type::getInt64PtrTy(context);
  this->Vec2Int64Type = VectorType::get(this->Int64Type, 2, false);
  this->Vec4Int64Type = VectorType::get(this->Int64Type, 4, false);
  this->Vec8Int64Type = VectorType::get(this->Int64Type, 8, false);
  Type *voidType = Type::getVoidTy(context);

  DECLARE_VECTOR_FUNCTION("extract_element2", Int64Type, ArrayRef<Type*>({Vec2Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("extract_element4", Int64Type, ArrayRef<Type*>({Vec4Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("extract_element8", Int64Type, ArrayRef<Type*>({Vec8Int64Type, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vload2", Vec2Int64Type, ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vload4", Vec4Int64Type, ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vload8", Vec8Int64Type, ArrayRef<Type*>({Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vstore2", voidType, ArrayRef<Type*>({LVTWO(Int64Type), Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vstore4", voidType, ArrayRef<Type*>({LVTWO(LVTWO(Int64Type)), Int64PtrType, Int64Type}), M);
  DECLARE_VECTOR_FUNCTION("vstore8", voidType, ArrayRef<Type*>({LVTWO(LVTWO(LVTWO(Int64Type))), Int64PtrType, Int64Type}), M);
}

PreservedAnalyses LoopVectorizePass::vectorize(Function &F, FunctionAnalysisManager &FAM) {

  return PreservedAnalyses::all();
}

PreservedAnalyses LoopVectorizePass::run(Module &M, ModuleAnalysisManager &MAM) {
  this->setVectorSetting(M);
  FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  for (Function &F : M) {
    this->vectorize(F, FAM);
  }
  return PreservedAnalyses::all();
}
