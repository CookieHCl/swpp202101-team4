#ifndef LOOPVECTORIZEPASS_H
#define LOOPVECTORIZEPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/ADT/Twine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Type.h"

using namespace llvm;
using namespace std;

#include <vector>

#define LVTWO(x) x, x
#define DECLARE_VECTOR_FUNCTION(name, ret, argArray, M) { \
  Function::Create(FunctionType::get(ret, argArray, false), GlobalValue::ExternalLinkage, Twine(name), &M); \
}

class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
private:
  static const int64_t full2 = 3;  // b'11
  static const int64_t full4 = 15;  // b'1111
  static const int64_t full8 = 255; // b'11111111
  Type *Int64Type;
  Type *Int64PtrType;
  Type *Vec2Int64Type;
  Type *Vec4Int64Type;
  Type *Vec8Int64Type;
  void setVectorSetting(Module &M);
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

};


#endif
