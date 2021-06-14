#ifndef LOOPUNROLLINGPASS_H
#define LOOPUNROLLINGPASS_H

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
