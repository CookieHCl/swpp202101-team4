#ifndef BRANCHPREDICTPASS_H
#define BRANCHPREDICTPASS_H

#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::PatternMatch;

class BranchPredictPass : public PassInfoMixin<BranchPredictPass> {
private:
  // threshold to use probability
  const long double PROB_THRESHOLD = 0.9L;

  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const {
    return isVerbose ? outs() : nulls();
  }

public:
  BranchPredictPass(bool isVerbose = false) : isVerbose(isVerbose) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif
