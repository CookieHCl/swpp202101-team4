#ifndef CONSTANTMERGEPASS_H
#define CONSTANTMERGEPASS_H

#include "LoopSimplifyUtil.h"
#include "llvm/IR/PatternMatch.h"

#include <vector>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

class ConstantMergePass : public llvm::PassInfoMixin<ConstantMergePass> {
public:
  using ChainUnit = std::pair<Value*, ConstantInt*>;
private:
  bool isVerbose = false;
  raw_ostream& logs() const { return isVerbose ? outs() : nulls(); }
  ChainUnit getChainUnit(Instruction *inst);
  bool MergeConstant(Function &F, BinaryOperator::BinaryOps ops);
  bool tryMergeConstant(Instruction *inst, BinaryOperator::BinaryOps ops);
public:
  ConstantMergePass(bool isVerbose = false) : isVerbose(isVerbose) {};
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif