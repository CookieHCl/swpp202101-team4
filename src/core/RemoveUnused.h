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
  SmallPtrSet<Value*, 16> getNecessaryValues(Function &F);
  bool propagateUsed(Value *v, SmallPtrSet<Value*, 16> &usedSet, SmallPtrSet<Value*, 16> &checkedSet);
public:
  void eraseInstructionsRecursive(Instruction *inst);
  SmallPtrSet<Value*, 16> getUsedValues(Function &F);
  SmallPtrSet<Value*, 16> getRecursiveUsers(Value *v);
  SmallPtrSet<Value*, 16> getInstPredecessors(Instruction *inst);
  SmallPtrSet<Instruction*, 16> getUserInstSet(SmallPtrSet<Value*, 16> &valueSet, Function &F);
  SmallPtrSet<GlobalValue*, 16> getGloalValues(Function &F);
  SmallPtrSet<Value*, 16> getPredecessorSet(SmallPtrSet<Value*, 16> &valueSet, Function &F); 

  void eraseInstructions(SmallPtrSet<Instruction*, 16> insts);
  vector<BasicBlock*> getUnreachableBBs(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnreachableBBs(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnusedInstructions(Function &F, FunctionAnalysisManager &FAM);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
