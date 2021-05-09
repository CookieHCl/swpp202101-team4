#ifndef TEAM4HEADER_H
#define TEAM4HEADER_H


#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

#include <queue>
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
