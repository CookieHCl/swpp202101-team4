#ifndef LOOPVECTORIZEPASS_H
#define LOOPVECTORIZEPASS_H

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

#include <vector>

using namespace llvm;
using namespace std;


class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
  using ChainID = const Value*;
  using InstChain = SmallVector<Instruction*, 8>;
  using InstChainMap = MapVector<ChainID, InstChain>;
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

  ChainID getChainID(const Value *Ptr);
  std::pair<InstChainMap, InstChainMap> collectInstructions(BasicBlock *BB, TargetTransformInfo &TTI);
  Function* getVectorCallee(int dimension, LoopVectorizePass::CalleeType calleeType);
  void fillVectorArgument(Value *address, const int64_t mask, SmallVector<Value*, 8> &Args);
  void makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM);
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
