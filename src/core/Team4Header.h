#ifndef TEAM4HEADER_H
#define TEAM4HEADER_H


#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

#include <queue>


class RemoveUnusedPass : public llvm::PassInfoMixin<RemoveUnusedPass> {
public:
  SetVector<BasicBlock*> getUnreachableBB(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnreachableBB(Function &F, FunctionAnalysisManager &FAM);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
