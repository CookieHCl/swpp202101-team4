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
class ChainUnit {
public:
  BinaryOperator::BinaryOps binOp;
  Value *value;
  ConstantInt *constant;
  bool isReversed;
  bool isValid() { return this->value != NULL; }
  ChainUnit(BinaryOperator::BinaryOps binOp, Value *v, ConstantInt* c, 
            bool isReversed) : binOp(binOp), value(v), constant(c), isReversed(isReversed) {};
  static ChainUnit createChainUnit(Instruction *inst);
  static ChainUnit blank() { return {Instruction::Add, NULL, NULL, false}; }
  static vector<BinaryOperator::BinaryOps> getAvailableOps(){
    return {Instruction::Add, Instruction::Mul, Instruction::Sub, Instruction::Shl, Instruction::LShr, Instruction::AShr};
  }
  Instruction* createInstruction(Instruction *flagInst);
  ChainUnit merge(ChainUnit &sourceUnit);
  friend raw_ostream& operator << (raw_ostream& os, ChainUnit &unit) {
    if (unit.isValid()) 
      return os << "value : " << *unit.value << " / const : " << *unit.constant 
                << (unit.isReversed ? " (Reversed order)" : " (In order)");
    else return os << "INVALID ChainUnit";
  }
};
private:
  bool isVerbose = false;
  raw_ostream& logs() const { return isVerbose ? outs() : nulls(); }
  bool MergeConstant(Function &F, BinaryOperator::BinaryOps ops);
  bool tryMergeConstant(Instruction *inst, BinaryOperator::BinaryOps ops);
public:
  ConstantMergePass(bool isVerbose = false) : isVerbose(isVerbose) {};
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

#endif
