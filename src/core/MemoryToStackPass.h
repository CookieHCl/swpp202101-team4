#ifndef MEMORYTOSTACKPASS_H
#define MEMORYTOSTACKPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class MemoryToStackPass : public PassInfoMixin<MemoryToStackPass> {
private:
  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const {
    return isVerbose ? outs() : nulls();
  }

public:
  MemoryToStackPass(bool isVerbose = false) : isVerbose(isVerbose) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

#endif
