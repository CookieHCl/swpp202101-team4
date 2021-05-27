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

#define LVTWO(x) x, x
#define DECLARE_VECTOR_FUNCTION(retType, name, argTypeArray, M) \
  Function::Create(FunctionType::get(retType, argTypeArray, false), GlobalValue::ExternalLinkage, Twine(name), &M)\

using namespace llvm;
using namespace std;


class LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
  using ChainID = const Value*;
  using InstChain = SmallVector<Instruction*, 8>;
  using InstChainMap = MapVector<ChainID, InstChain>;
private:
  Type *VoidType, *Int64Type, *Int64PtrType, *Vec2Int64Type, *Vec4Int64Type, *Vec8Int64Type;
  Function *extractElement2, *extractElement4, *extractElement8, *vLoad2, *vLoad4, *vLoad8; 
  Function *vStore2, *vStore4, *vStore8;
  enum CalleeType : unsigned {
    LOAD = 0,
    EXTRACT = 1,
    STORE = 2
  };

  ChainID getChainID(const Value *Ptr);
  std::pair<InstChainMap, InstChainMap> collectInstructions(BasicBlock *BB, TargetTransformInfo &TTI);
  Function* getVectorCallee(int dimension, LoopVectorizePass::CalleeType calleeType);
  void fillVectorArgument(Value *address, int64_t mask, SmallVector<Value*, 8> &Args);
  void makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM);
  bool vectorize(Loop *L, LoopInfo &LI, ScalarEvolution &SE, TargetTransformInfo &TTI, const DataLayout &DL, DominatorTree &DT);
  bool vectorizeMap(InstChainMap &instChainMap, ScalarEvolution &SE, const DataLayout &DL, DominatorTree &DT);
  bool vectorizeInstructions(InstChain &instChain, ScalarEvolution &SE, const DataLayout &DL, DominatorTree &DT);
  void vectorizeLoadInsts(InstChain &instChain, int dimension, int64_t mask, Instruction *first);
  void vectorizeStoreInsts(InstChain &instChain, int dimension, int64_t mask, Instruction *first);
public:
  LoopVectorizePass(Module &M);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


#endif
