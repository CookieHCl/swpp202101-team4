#ifndef LIVENESS_ANALYSIS_H
#define LIVENESS_ANALYSIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "UnfoldVectorInst.h"

#include "llvm/Analysis/PostDominators.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Support/raw_ostream.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace llvm;
using namespace std;

namespace backend
{

//RegisterGraph: stores the colored register graph information and provides interface
class RegisterGraph
{

public:
  //Constructors

  RegisterGraph(Module &);

  //Interfaces

  auto& getValues() {return values;}
  auto& getValues(Function* F) {return valuesInFunction[F];}

  unsigned findValue(Value* V);

  auto& getAdjList() {return adjList;}
  auto& getAdjList(Value* v) {return adjList[v];}

  auto& getNumColors() {return numColors;}
  int getNumColors(Function* F) {return numColors[F];}

  auto& getValueToColor() {return valueToColor;}
  auto& getValueToColor(Function* F) {return valueToColor[F];}
  unsigned int getValueToColor(Function* F, Value* v) {return valueToColor[F][v];}

  auto& getColorToValue() {return colorToValue;}
  auto& getColorToValue(Function* F) {return colorToValue[F];}

  auto& getNoEvictNum(Function *F) {return noEvictNum[F];}

  static set<unsigned int> DO_NOT_CONSIDER;
  static set<unsigned int> SAME_CONSIDER;

  //Do not consider this instruction when RA
  bool doNotConsider(Instruction &);

private:
  
  //Private Variables                          

  //M: Module which is analyzed
  Module* M;

  //values: result of SearchAllArgInst()
  vector<Value *> values;
  map<Function*, vector<Value *>> valuesInFunction;

  //adjList: result of LiveInterval() + RegisterAdjList()
  map<Value *, set<Value *>> adjList;

  //NUM_COLORS, valueToColor: result of ColorGraph();
  //valueToColor: Value=>color mapping
  map<Function*, unsigned int> numColors;
  map<Function*, map<Value *, unsigned int>> valueToColor;

  //colorToValue: result of InverseColorMap()
  //color->Value mapping
  map<Function*, vector<vector<Value *>>> colorToValue;

  //noEvictNum: maximum color number that cannot be evicted from vload.
  map<Function*, int> noEvictNum;

  //Functions for constructor RegisterGraph(Module&)

  //1. Construct liveness interval

  //Finds all instructions that can be alive in some point.
  //i.e. finds all Arguments & Instructions that contain valid values.
  //GVs and non-value Inst.(store, br, ...) are not considered.
  void SearchAllArgInst(Module &);

  //Recursively(post-order) searches through all instructions
  //mark liveness of each values in each instruction
  vector<vector<bool>> LiveInterval(Module &);
  //helper function for LiveInterval()
  //does the recursive search from use to def, making the value to be live
  void LivenessSearch(Instruction &, Value &, int, map<Instruction *, vector<bool>> &);

  //2. Construct live graph and assign different colors to
  //   values which are live together

  //Adjacency list of Argumetns & value-containing Instructions
  //two insts. are adjacent iff live inerval overlap.
  void RegisterAdjList(vector<vector<bool>> &);

  //Colors values so adjacent value have no same color
  //initializes NUM_COLORS, valueToColor
  void ColorGraph();
  //helper function for ColorGraph()
  //finds PEO via Lexicographic BFS algorithm
  vector<Value *> PerfectEliminationOrdering(vector<Value *> &);
  //helper function for ColorGraph()
  //colors the graph greedily(adjList always represents a chordal graph)
  map<Value *, unsigned int> GreedyColoring(vector<Value *> &, unsigned int&, Function &);
  //verifier function for ColorGraph()
  bool VerifyColoring(vector<Value *> &, map<Value *, unsigned int> &);
  //helper function for ColorGraph()
  //tries to coallocate insts. in SAME_CONSIDER if possible
  //If a value's only single use is SAME_CONSIDER right after it, coallocate.
  void coallocateIfPossible();

  //Makes colorToValue so easily retrieve all values with same color
  void InverseColorMap();
};

extern map<Instruction *, unsigned int> ReservedColor;

} // namespace backend


#endif