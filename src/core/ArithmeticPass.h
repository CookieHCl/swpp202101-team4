#ifndef ARITHMETICPASS_H
#define ARITHMETICPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <cassert>
#include <vector>
using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

class ArithmeticPass : public PassInfoMixin<ArithmeticPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif