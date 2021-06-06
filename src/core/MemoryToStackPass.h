#ifndef MEMORYTOSTACKPASS_H
#define MEMORYTOSTACKPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

class MemoryToStackPass : public PassInfoMixin<MemoryToStackPass> {
private:
  static const char* STACK_POINTER_NAME; // "____sp"
  static const char* NEW_MALLOC_NAME; // "____malloc"
  static const char* NEW_FREE_NAME; // "____free"

  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const {
    return isVerbose ? outs() : nulls();
  }

  void replaceAlloca(Module &M, Function* NewMalloc);
  void replaceFunction(Module &M, Function* OrigFun, Function* NewFun);

public:
  MemoryToStackPass(bool isVerbose = false) : isVerbose(isVerbose) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

#endif
