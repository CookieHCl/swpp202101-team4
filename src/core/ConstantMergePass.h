#ifndef CONSTANTMERGEPASS_H
#define CONSTANTMERGEPASS_H

#include "LoopSimplifyUtil.h"

#include <vector>

using namespace llvm;
using namespace std;

class ConstantMergePass : public llvm::PassInfoMixin<ConstantMergePass> {
  bool isVerbose = false;
public:
  ConstantMergePass::ConstantMergePass(bool isVerbose = false) : isVerbose(isVerbose) {};
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif