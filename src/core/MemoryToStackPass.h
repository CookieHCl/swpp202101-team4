#ifndef MEMORYTOSTACKPASS_H
#define MEMORYTOSTACKPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <queue>

using namespace llvm;

class MemoryToStackPass : public PassInfoMixin<MemoryToStackPass> {
private:
  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const {
    return isVerbose ? outs() : nulls();
  }

  using ConstFP = const Function* const;

  void replaceAlloca(Module &M, Function* NewMalloc);
  void replaceFunction(Module &M, ConstFP OrigFun, Function* NewFun);
  void removeUnnessaryFree(Module &M, ConstFP NewMalloc, ConstFP NewFree);

public:
  MemoryToStackPass(bool isVerbose = false) : isVerbose(isVerbose) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

#endif
