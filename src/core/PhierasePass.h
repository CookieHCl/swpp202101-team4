#ifndef PHIERASEPASS_H
#define PHIERASEPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <cassert>
#include <vector>
#include <queue>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

class PhierasePass : public PassInfoMixin<PhierasePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif