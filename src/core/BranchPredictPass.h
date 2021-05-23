#ifndef BRANCHPREDICTPASS_H
#define BRANCHPREDICTPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::PatternMatch;

class BranchPredictPass : public PassInfoMixin<BranchPredictPass> {
private:
  //if true, prints the intermediate steps.
  bool printProcess;

  // print messages in line if verbose
  // https://stackoverflow.com/a/9040913
  // usage: printDebug("a", "b", "c"); -> outs() << "a" << "b" << "c" << '\n';
  void printDebug() {
    if (printProcess) {
      outs() << '\n';
    }
  }
  template<typename First, typename ... Strings>
  void printDebug(First& arg, const Strings&... rest) {
    if (printProcess) {
      outs() << arg;
      printDebug(rest...);
    }
  }

public:
  BranchPredictPass(bool printProcess = false) : printProcess(printProcess) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif
