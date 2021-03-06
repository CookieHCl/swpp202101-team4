#ifndef LOOPVECTORIZEPASS_H
#define LOOPVECTORIZEPASS_H

#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/ilist_iterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "LoopSimplifyUtil.h"

#include <vector>
#include <queue>

using namespace llvm;
using namespace std;


class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
public:
  class ConsecutiveScheme {
  public:
    int sourceID;
    const SCEV *scev;
    Instruction *memInst;
    Value *ptr;
    int displ;
    void setSourceID(int sourceID) { this->sourceID = sourceID; }
    void setDispl(int displ) { this->displ = displ; }
    bool operator < (const ConsecutiveScheme &scheme) const {
      return (sourceID < scheme.sourceID )? true : ((sourceID == scheme.sourceID) ? (displ < scheme.displ) : false);
    }
    friend raw_ostream& operator << (raw_ostream& os, const ConsecutiveScheme &scheme) {
      return os << "[" << *scheme.scev << "](" << scheme.ptr->getName() << ") source " << scheme.sourceID << " displ " << scheme.displ;
    }
    ConsecutiveScheme(Instruction *inst, int sourceID, ScalarEvolution &SE);
  };
  using ChainID = const Value*;
  using InstChain = SmallVector<Instruction*, 8>;
  using InstChainMap = MapVector<ChainID, InstChain>;
  using ConScheme = ConsecutiveScheme;
  using ConProperty = pair<bool, int64_t>;
private:
  bool isVerbose;
  raw_ostream& logs() const { return isVerbose ? outs() : nulls(); }
  Type *VoidType, *Int64Type, *Int64PtrType, *Vec2Int64Type, *Vec4Int64Type, *Vec8Int64Type;
  Function *extractElement2, *extractElement4, *extractElement8, *vLoad2, *vLoad4, *vLoad8; 
  Function *vStore2, *vStore4, *vStore8;
  enum class CalleeType : unsigned {
    LOAD = 0,
    EXTRACT = 1,
    STORE = 2
  };
  bool isReferSameMemory(Instruction *inst, Instruction *pivot, ScalarEvolution &SE);
  bool isLoadForwardable(Instruction *inst1, Instruction *inst2, DominatorTree &DT, ScalarEvolution &SE);
  bool isStoreBackwardable(Instruction *inst1, Instruction *inst2, DominatorTree &DT, ScalarEvolution &SE);
  std::vector<ConsecutiveScheme> createSchemes(InstChain &instChain, ScalarEvolution &SE);
  ConProperty measureConsecutiveProperty(const SCEV *scev1, const SCEV *scev2, ScalarEvolution &SE);
  ChainID getChainID(const Value *Ptr);
  std::pair<InstChainMap, InstChainMap> collectInstructions(BasicBlock *BB, TargetTransformInfo &TTI);
  Function* getVectorCallee(int dimension, LoopVectorizePass::CalleeType calleeType);
  template<typename Type> int getSCEVOperandUnmatchedIndex(const Type *scev1, const Type *scev2);
  void fillVectorArgument(Value *address, const int64_t mask, SmallVector<Value*, 8> &Args);
  bool vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE, TargetTransformInfo &TTI, const DataLayout &DL, DominatorTree &DT);
  bool vectorizeMap(InstChainMap &instChainMap, ScalarEvolution &SE, const DataLayout &DL, DominatorTree &DT);
  bool vectorizeInstructions(InstChain &instChain, ScalarEvolution &SE, const DataLayout &DL, DominatorTree &DT);
  void vectorizeLoadInsts(InstChain &instChain, const int dimension, const int64_t mask, Instruction *first);
  void vectorizeStoreInsts(InstChain &instChain, const int dimension, const int64_t mask, Instruction *first);
public:
  LoopVectorizePass(Module &M, bool isVerbose = false);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
