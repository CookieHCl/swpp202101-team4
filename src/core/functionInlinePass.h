#ifndef FUNCTIONINLINEPASS_H
#define FUNCTIONINLINEPASS_H

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/InlineCost.h"

#include "llvm/IR/PassManager.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include "../backend/LivenessAnalysis.h"

#include <vector>
#include <map>
using namespace std;
using namespace llvm;
using namespace backend;

class FunctionInlinePass : public PassInfoMixin<FunctionInlinePass> {
public:
    
  //Register file size of the target machine.
  static const unsigned REGISTER_CAP = 32;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  
};

#endif