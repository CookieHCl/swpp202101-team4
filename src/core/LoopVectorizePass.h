#ifndef LOOPVECTORIZEPASS_H
#define LOOPVECTORIZEPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/ilist_iterator.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PatternMatch.h"

#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Type.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

#include <vector>

#define LVTWO(x) x, x
#define DECLARE_VECTOR_FUNCTION(retType, name, argTypeArray, M) { \
  Function::Create(FunctionType::get(retType, argTypeArray, false), GlobalValue::ExternalLinkage, Twine(name), &M); \
}

class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
private:
  static const int64_t full2 = 3;  // b'11
  static const int64_t full4 = 15;  // b'1111
  static const int64_t full8 = 255; // b'11111111
  Type *VoidType;
  Type *Int64Type;
  Type *Int64PtrType;
  Type *Vec2Int64Type;
  Type *Vec4Int64Type;
  Type *Vec8Int64Type;
  class Induction {
    public:
    Induction(Value *v, int unit) : v(v), unit(unit) {}
    Value *v;
    int unit;
  };
  int numInduction(Value *v);
  Induction getInduction(Loop *L, LoopInfo &LI);
  SmallVector<Instruction*, 16> getInductionPtrs(BasicBlock *BB, Value *ind);
public:
  LoopVectorizePass(Module &M);
  PreservedAnalyses vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
