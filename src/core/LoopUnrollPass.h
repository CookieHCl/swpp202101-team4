#ifndef LOOPUNROLLINGPASS_H
#define LOOPUNROLLINGPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/ilist_iterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#include "LoopSimplifyUtil.h"

#include <vector>

using namespace llvm;
using namespace std;

class LoopUnrollPass : public llvm::PassInfoMixin<LoopUnrollPass> {
private:
  bool isVerbose;
  raw_ostream& logs() const { return isVerbose ? outs() : nulls(); }
public:
  LoopUnrollPass(bool isVerbose = false) : isVerbose(isVerbose) {};
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif
