#ifndef BRANCHPREDICTPASS_H
#define BRANCHPREDICTPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class BranchPredictPass : public PassInfoMixin<BranchPredictPass> {
private:
  //if true, prints the intermediate steps.
  bool printProcess;

public:
  BranchPredictPass(bool printProcess = false) : printProcess(printProcess) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif
