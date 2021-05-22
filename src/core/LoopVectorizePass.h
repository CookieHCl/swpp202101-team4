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


class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};


#endif
