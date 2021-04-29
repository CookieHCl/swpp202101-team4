#ifndef REGISTER_ALLOC_H
#define REGISTER_ALLOC_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Support/raw_ostream.h"

#include "LivenessAnalysis.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace llvm;
using namespace std;

namespace backend
{

//For stack-loaded variables, cost needed per color is the sum of:
//spilled variables are proven to reside in 
static const float INF_COST = 1e16,
                   ALLOCA_COST = 0.0,
                   LOAD_COST = 12,
                   STORE_COST = 12;

class SpillCostAnalysis : public AnalysisInfoMixin<SpillCostAnalysis>
{
  friend AnalysisInfoMixin<SpillCostAnalysis>;
  static AnalysisKey Key;

public:

  //For undecided iterations of loops, the default iteration count.
  static const unsigned DEFAULT_LOOP = 50;

  //1-1 mapping between colors(unsigned int) and spill cost(float)
  using Result = map<Function*, vector<float>>;

  //Calculates the estimated spill cost of each set of equally-colored registers.
  //Note that this pass performs just a heuristic method; it may not be accurate
  Result run(Module &M, ModuleAnalysisManager &MAM);

private:

  //Counts the loop trip count considering all layers of loops
  unsigned countLoopTripCount(Instruction*, ScalarEvolution&, LoopInfo&);
  
};

class RegisterSpillPass : public PassInfoMixin<RegisterSpillPass>
{
  //Module which this analysis runs.
  Module* M;
  //Register graph for the module.
  RegisterGraph* RG;

public:

  //Register file size of the target machine.
  //static const unsigned REGISTER_CAP = 16;
  static const unsigned REGISTER_CAP = 32;

  //Finds the registers that need to be spilled
  //and actually spills them by adding alloca, store, and load.
  //Requirement: MUST execute Dead Store Elimination & GVN after running this.
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

private:

  //Searches through all functions and check if the spilling is enough.
  bool spilledEnough(unsigned, vector<bool>, Function*);

  //Pre-order traverses through the function's basic blocks.
  //In this way, if a BB has a single predecessor, the register state is already calculated.
  //Spills the registers by adding store and load instructions.
  void spillRegister(unsigned, const vector<bool>&, const vector<AllocaInst*>&, BasicBlock&);

  //Finds the most suitable color to discard from register memory.
  //Looks forward within the BB(scope), and finds the latest-used color and discard.
  unsigned findReplaced(unsigned, Instruction*, vector<bool>, vector<bool>, BasicBlock&);

  //Inserts the load instruction and corresponding type conversions.
  // %temp0 = load %loadFrom
  // %temp1 = (proper type conversion from i64) %temp0
  // %insertBefore (may be same as the target)
  //----------------------------------------------
  // %.. = uses %target.getUser() ..
  //           => uses %temp1 ..
  Value* insertLoad(Use& target, AllocaInst* loadFrom, Instruction* insertBefore, set<Value*>& skip);

  //Inserts the store instruction and corresponding type conversions
  // %storeVal = ..
  //----------------------------------------------
  // %temp = (proper type conversion to i64) %storeVal
  // store %temp, %storeAt
  // %insertBefore
  Value* insertStore(Value* storeVal, AllocaInst* storeAt, Instruction* insertBefore, set<Value*>& skip);

};
}

#endif