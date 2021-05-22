#ifndef REMOVEUNUSEDPASS_H
#define REMOVEUNUSEDPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

#include <vector>
#include <queue>


class RemoveUnusedPass : public llvm::PassInfoMixin<RemoveUnusedPass> {
private:
  template<typename Iter>
  void printValues(Iter &iter, string str);
  SmallPtrSet<Value*, 16> getNecessaryValues(Function &F);
  bool isNecessaryInst(Instruction *inst, SmallPtrSet<Value*, 16> &necessarySet);
  SmallPtrSet<Instruction*, 16> getSideEffectsInst(Function &F);
  SmallPtrSet<Value*, 16> getRecursiveNonControlFlowValues(Value *v, SmallPtrSet<Value*, 16> &checked);

public:
  template<typename Lambda1, typename Lambda2> 
  SmallPtrSet<Value*, 16> getInstPredecessors(Instruction *inst, Lambda1 valueCondition, Lambda2 operandCondition);
  SmallPtrSet<GlobalValue*, 16> getGloalValues(Function &F);
  SmallPtrSet<Value*, 16> getPredecessorSet(SmallPtrSet<Value*, 16> &valueSet, Function &F); 
  SmallPtrSet<Value*, 16> getUsedValues(Function &F);
  vector<BasicBlock*> getUnreachableBBs(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnreachableBBs(Function &F, FunctionAnalysisManager &FAM);
  void eraseInstructions(SmallPtrSet<Instruction*, 16> insts);
  void eraseUnusedInstructions(Function &F, FunctionAnalysisManager &FAM);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
