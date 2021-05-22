#ifndef SPLIT_SELF_LOOP_H
#define SPLIT_SELF_LOOP_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <vector>

using namespace llvm;
using namespace std;

namespace backend {
class SplitSelfLoopPass : public PassInfoMixin<SplitSelfLoopPass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);

private:

};
}

#endif 