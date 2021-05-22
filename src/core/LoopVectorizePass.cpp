#include "Team4Header.h"


void print(Function &F) {
  outs() << F;
}


void createDeclaration(Module &M) {
  auto &context = M.getContext();
  Type *intType = Type::getInt64Ty(context);
  unsigned eq = 2;
  VectorType *vt = VectorType::get(intType, 2, false);
  Twine name = Twine("vload2");
  GlobalValue::LinkageTypes linkage = GlobalValue::ExternalLinkage;
  FunctionType *ft = FunctionType::get(vt, ArrayRef<Type*>({intType, intType}), false);
  Function *f = Function::Create(ft, linkage, 0, Twine("vload2"), &M);
  outs() << *f;
}


PreservedAnalyses LoopVectorizePass::run(Module &M, ModuleAnalysisManager &MAM) {
  createDeclaration(M);
  return PreservedAnalyses::all();
}
