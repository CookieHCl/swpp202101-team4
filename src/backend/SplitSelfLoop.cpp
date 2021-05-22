/*
    SplitSelfLoopPass
*/

#include "SplitSelfLoop.h"

using namespace llvm;
using namespace std;

namespace backend {

PreservedAnalyses SplitSelfLoopPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVMContext &Context = M.getContext();

  for (Function &F : M) {
    if (F.isDeclaration()) continue;

    vector<BasicBlock*> toAdd;

    for (BasicBlock &BB : F) {
      Instruction *I = BB.getTerminator();
      bool splitNeeded = false;

      for (unsigned i = 0; i < I->getNumSuccessors(); i++) {
        if (&BB == I->getSuccessor(i)) {
          splitNeeded = true;
          break;
        }
      }

      if (!splitNeeded) continue;

      // split self loop
      BasicBlock *newBB = BasicBlock::Create(Context, BB.getName().str() + ".splitted");
      I->replaceSuccessorWith(&BB, newBB);

      // unconditional branch to go back to the old BB
      BranchInst::Create(&BB, newBB);

      // update phi nodes
      for (PHINode &phi : BB.phis()) {
        int incomingNum = -1;
        for (unsigned i = 0; i < phi.getNumIncomingValues(); i++) {
          if (phi.getIncomingBlock(i) == &BB) {
            incomingNum = i;
            break;
          }
        }

        assert(incomingNum != -1);
        phi.setIncomingBlock(incomingNum, newBB);
      }

      toAdd.push_back(newBB);
    }

    // add new BBs
    for (BasicBlock *newBB : toAdd) {
      newBB->insertInto(&F);
    }

    toAdd.clear();
  }

  return PreservedAnalyses::all();
}
}  // namespace backend