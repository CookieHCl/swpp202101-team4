#ifndef MEMORYTOSTACKPASS_H
#define MEMORYTOSTACKPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class MemoryToStackPass : public PassInfoMixin<MemoryToStackPass> {
private:
  static const char* STACK_POINTER_NAME; // "____sp"
  static const char* NEW_MALLOC_NAME; // "____malloc"
  static const char* NEW_FREE_NAME; // "____free"
  static const uint64_t STACK_SIZE = 102400;

  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const {
    return isVerbose ? outs() : nulls();
  }

  Function* createNewMalloc(Module &M, Function* OrigMalloc,
      Value* StackPointer, IntegerType* Int64Ty);
  Function* createNewFree(Module &M, Function* OrigFree, IntegerType* Int64Ty);

  void replaceFunction(Module &M, Function* OrigFun, Function* NewFun);

public:
  MemoryToStackPass(bool isVerbose = false) : isVerbose(isVerbose) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

#endif
