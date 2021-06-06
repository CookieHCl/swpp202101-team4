#ifndef MATMULTRANSPOSEPASS_H
#define MATMULTRANSPOSEPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"

#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/ilist_iterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ilist.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <vector>

using namespace llvm;
using namespace std;

class MatmulTransposePass : public llvm::PassInfoMixin<MatmulTransposePass> {
private:
  const bool isVerbose;
  // stream for logging; only prints if verbose
  raw_ostream& logs() const { return isVerbose ? outs() : nulls(); }

  PHINode *getCanonicalVariable(Loop *L);
  bool isValidtoAdded(const SCEV *target, Value *ptrAddr, Loop *OuterLoop, ScalarEvolution &SE);

  bool isConstantRange(Loop *L, const SCEV *target, Loop *Outer, ScalarEvolution &SE);
  bool noAdditionalOuterBody(Loop *InnerLoop, Loop *OuterLoop);
  bool isThereOnlySigmaStore(Loop *InnerLoop, Loop *OuterLoop, ScalarEvolution &SE);

  void rmSumRegister(Function &F, FunctionAnalysisManager &FAM);
  void makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM);
  void loopInterChange(Function &F, FunctionAnalysisManager &FAM);

public:
  MatmulTransposePass(bool isVerbose = false) : isVerbose(isVerbose) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif