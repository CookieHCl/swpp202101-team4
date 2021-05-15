#ifndef REMOVEUNUSEDPASS_H
#define REMOVEUNUSEDPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

#include <vector>


class RemoveUnusedPass : public llvm::PassInfoMixin<RemoveUnusedPass> {
private:
  template <typename T> void sortDominated(vector<T> &vec, DominatorTree &DT);
  void eraseInstructionsRecursive(Instruction *inst);
public:
  vector<BasicBlock*> getUnreachableBB(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnreachableBB(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnusedInstruction(Function &F, FunctionAnalysisManager &FAM);
  void eraseUnloadedAlloca(Function &F, FunctionAnalysisManager &FAM);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
