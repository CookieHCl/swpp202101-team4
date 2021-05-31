#include "LivenessAnalysis.h"

using namespace llvm;
using namespace std;

namespace backend
{

//---------------------------------------------------------------
//class RegisterGraph
//---------------------------------------------------------------
set<unsigned int> RegisterGraph::DO_NOT_CONSIDER = {Instruction::Store,
                                                    Instruction::Alloca,
                                                    Instruction::Ret,
                                                    Instruction::Switch,
                                                    Instruction::Br};
set<unsigned int> RegisterGraph::SAME_CONSIDER = {Instruction::GetElementPtr,
                                                  Instruction::BitCast,
                                                  Instruction::PtrToInt,
                                                  Instruction::IntToPtr,
                                                  Instruction::SExt,
                                                  Instruction::ZExt};

RegisterGraph::RegisterGraph(Module &M)
{
    this->M = &M;

    //find all value-results
    SearchAllArgInst(M);
    
    //Find live interval for all values
    vector<vector<bool>> liveInterval;
    liveInterval = LiveInterval(M);
    
    //Initialize adjList
    RegisterAdjList(liveInterval);
    ColorGraph();
    
    InverseColorMap();
}

void RegisterGraph::SearchAllArgInst(Module &M)
{
    for (Function &F : M)
    {
        if(F.isDeclaration()) continue;
        valuesInFunction[&F] = vector<Value*>();
        for (BasicBlock &BB : F)
        {
            for (Instruction &I : BB)
            {
                if (!doNotConsider(I))
                {
                    values.push_back(&I);
                    valuesInFunction[&F].push_back(&I);
                }
            }
        }
    }
}

vector<vector<bool>> RegisterGraph::LiveInterval(Module &M)
{
    //Value v is live in instruction I iff...
    //live[I][index of v in values] = true
    map<Instruction *, vector<bool>> live;

    int N = values.size();

    //Initialize map 'live'
    for (Function &F : M)
    {
        for (BasicBlock &BB : F)
        {
            for (Instruction &I : BB)
            {
                live[&I] = vector<bool>(N);
            }
        }
    }

    //Search Liveness Interval for all values
    int i = 0;
    for (Value *V : values)
    {
        //'F': Function which 'V' is located.
        Function &F = *dyn_cast<Instruction>(V)->getFunction();

        //'start': Where to start the search.
        Instruction &def = *dyn_cast<Instruction>(V);

        //finds if V is live in the specific instruction.
        //recursively searches through the basic blocks and store result in live.
        for(auto& v : V->uses()) {
            if(isa<PHINode>(v.getUser())) {
                PHINode* phi = dyn_cast<PHINode>(v.getUser());
                LivenessSearch(*(phi->getIncomingBlock(v)->getTerminator()), *V, i, live);
            }
            else if(isa<Instruction>(v.getUser())) {
                LivenessSearch(*dyn_cast<Instruction>(v.getUser()), *V, i, live);
            }
        }

        if (V->hasNUses(0) && isa<Instruction>(V)) {
            // if it has no use
            Instruction *I = dyn_cast<Instruction>(V);
            Instruction *Nxt = I->getNextNode();
            if (Nxt != NULL) {
                live[Nxt][i] = true;
            }
        }

        if(isa<PHINode>(V)) {
            PHINode* phi = dyn_cast<PHINode>(V);
            //phinodes should be live from the beginning of the function.
            for(auto& I : phi->getParent()->phis()){
                live[&I][i] = true;
            }
        }
        i++;
    }

    vector<vector<bool>> result;

    for (Function &F : M)
    {
        for (BasicBlock &BB : F)
        {
            for (Instruction &I : BB)
            {
                result.push_back(live[&I]);
            }

            Instruction* term = BB.getTerminator();
            vector<bool> PhiTerm = live[term];

            // Condition for BranchInst or SwitchInst should be alive
            BranchInst *br;
            SwitchInst *swc;
            Value *cond = NULL;

            if ((br = dyn_cast<BranchInst>(term)) && br->isConditional()) {
                cond = br->getCondition();
            } else if ((swc = dyn_cast<SwitchInst>(term))) {
                cond = swc->getCondition();
            }

            if (cond != NULL) {
                unsigned fv = findValue(cond);
                if (fv != -1) PhiTerm[fv] = true;
            }

            for(BasicBlock* succ : successors(&BB)) {
                for(PHINode& phi : succ->phis()) {
                    assert(findValue(&phi) != -1 && "phi term should be live.");
                    PhiTerm[findValue(&phi)] = true;
                }
            }
            result.push_back(PhiTerm);
        }
    }

    return result;
}

void RegisterGraph::LivenessSearch(Instruction &curr, Value &find, int index, map<Instruction *, vector<bool>> &live)
{
    BasicBlock &BB = *(curr.getParent());

    bool isAlive = false;

    for (auto it = BB.rbegin(); it != BB.rend(); ++it)
    {
        Instruction &I = *it;
        if(&curr == &I) isAlive = true;
        //if already live in the position or reached def, stop.
        if(&find == &I || live[&I][index]) return;
        //if 'find' is alive in I, (if 'find' == 'I' it is not alive yet)
        if(isAlive) {
            live[&I][index] = true;
        }
    }
    for(auto it = pred_begin(&BB); it != pred_end(&BB); ++it) {
        LivenessSearch(*((*it)->rbegin()), find, index, live);
    }

}

void RegisterGraph::RegisterAdjList(vector<vector<bool>> &liveInterval)
{

    for (Value *value : values)
    {
        adjList[value] = set<Value *>();
    }

    for (vector<bool> &vec : liveInterval)
    {
        assert(vec.size() == values.size() && "values & liveInterval match");

        //if two registers are alive together in one instruction,
        for (int i = 0; i < vec.size(); ++i)
        {
            for (int j = i + 1; j < vec.size(); ++j)
            {
                if (vec[i] && vec[j])
                {
                    //add to each other's adjacency list
                    adjList[values[i]].insert(values[j]);
                    adjList[values[j]].insert(values[i]);
                }
            }
        }
    }
}

void RegisterGraph::ColorGraph()
{
    for(Function& F : *M) {
        //calculates PEO for values + adjList
        auto& valueF = valuesInFunction[&F];
        vector<Value *> PEO = PerfectEliminationOrdering(valueF);

        //colors the graph greedily with PEO;
        reverse(PEO.begin(), PEO.end());
        unsigned int colorCount;
        valueToColor[&F] = GreedyColoring(PEO, colorCount, F);
        assert(VerifyColoring(PEO, valueToColor[&F]));

        //update global color count
        numColors[&F] = colorCount;
    }
    coallocateIfPossible();
}

vector<Value *> RegisterGraph::PerfectEliminationOrdering(vector<Value *> &values)
{

    //FIXME: This is a typical disjoint set(union-find) with order problem;
    //I do not know how to apply the feature.
    //Time complexity can reduced from O(EV2) to O(EV).

    //if not defined function, do not consider anyway
    if(values.size() == 0) return vector<Value*>();

    map<int, Value*> initSigma;
    map<Value*,int> valueIdx;
    int idx = 0;
    for (auto *v : values) {
        initSigma[idx] = v;
        valueIdx[v] = idx;
        idx++;
    }

    //Initially, Sigma contains a single set of all value in values.
    list<map<int, Value *>> Sigma{initSigma};
    vector<Value *> PEO;

    while (Sigma.size() > 0)
    {
        //retrieve one value v from Sigma[0] and delete it.
        auto bg = Sigma.begin()->begin();
        Value *v = bg->second;
        Sigma.begin()->erase(bg);
        //if Sigma[0] is empty, remove it.
        if (Sigma.begin()->empty())
        {
            Sigma.erase(Sigma.begin());
        }
        //push the popped value to result PEO vector.
        //reserved colors are always located at last
        Instruction *inst = dyn_cast<Instruction>(v);
        //push it if v is not a ReservedColor node
        if (inst == NULL || ReservedColor.find(inst) == ReservedColor.end())
            PEO.push_back(v);

        //for all members in Sigma, insert empty set
        for (auto it = Sigma.begin(); it != Sigma.end(); ++it)
        {
            Sigma.insert(it, map<int, Value *>());
        }

        //for all nodes connected to v,
        for (Value *w : adjList[v])
        {
            //search if w is still in Sigma.
            for (auto it = Sigma.begin(); it != Sigma.end(); ++it)
            {
                int wIdx = valueIdx.find(w)->second;
                auto wItr = it->find(wIdx);
                //if w exists in *it,
                if (wItr != it->end())
                {
                    //move w to the set infront.
                    auto it_prev = it;
                    --it_prev;
                    it_prev->insert({wIdx, w});
                    it->erase(wItr);
                }
            }
        }

        Sigma.remove(map<int, Value *>());
    }

    // construct a temporary set for instruction owner check
    std::unordered_set<Value*> values_set(values.begin(), values.end());

    //reserved colors are always located at last;
    //because GreedyColoring() calls reverse(PEO.begin(), PEO.end()),
    //this reserved colors will be colored prior to other nodes
    for (auto &[I, c] : ReservedColor) {
        // push into PEO only if the 'reserving' instruction belongs to this function
        if (values_set.find(I) != values_set.end()) {
            PEO.push_back(I);
        }
    }
    assert(PEO.size() == values.size() && "PEO should be the same size as values");

    return PEO;
}

map<Value *, unsigned int> RegisterGraph::GreedyColoring(vector<Value *> &PEO, unsigned int& NUM_COLORS, Function &F)
{

    map<Value *, unsigned int> valueToColor;

    NUM_COLORS = 0;

    //fetch values by Perfect Elimination Order
    for (auto it = PEO.begin(); it != PEO.end(); ++it)
    {
        //strongly color for reserved color (from vload or vstore)
        Instruction *inst = dyn_cast<Instruction>(*it);
        if (inst != NULL && ReservedColor.find(inst) != ReservedColor.end()) {
            unsigned int c = ReservedColor[inst];
            valueToColor[inst] = c;
            if (noEvictNum[&F] < c) noEvictNum[&F] = c + 1;
            if (NUM_COLORS <= c) NUM_COLORS = c + 1;
            continue;
        }

        //mark all colors which vertex before *it has used
        bool colorUsed[NUM_COLORS];
        for(bool& b : colorUsed) b = false;
        for (auto jt = PEO.begin(); jt != it; ++jt)
        {
            if(adjList[*it].count(*jt) != 0) 
            {
                colorUsed[valueToColor[*jt]] = true;
            }
        }

        //c: color for the vertex *it
        //if every neighbour nodes use all colors, c = NUM_COLORS
        //else, it becomes the first non-used git color
        int c = 0;
        for (c = 0; c < NUM_COLORS; c++)
        {
            if (!colorUsed[c])
            {
                break;
            }
        }

        valueToColor[*it] = c;

        //if new color used, increment the number of colors used.
        if (c == NUM_COLORS)
            NUM_COLORS++;
    }

    return valueToColor;

}

bool RegisterGraph::VerifyColoring(vector<Value *> &PEO, map<Value*, unsigned int> &valueToColor)
{
    for (auto it = PEO.begin(); it != PEO.end(); ++it)
    {
        for (auto jt = PEO.begin(); jt != it; ++jt)
        {
            if(adjList[*it].count(*jt) != 0) 
            {
                if (valueToColor[*it] == valueToColor[*jt])
                    return false;
            }
        }
    }

    return true;
}

void RegisterGraph::coallocateIfPossible() {
    for(Function& F : *M) {
        for(auto it = inst_begin(&F); it!=inst_end(&F); ++it) {
            Instruction& I = *it;
            if(!I.hasOneUse() || findValue(&I) == -1) continue;

            //Fetch the only user which follows right after.
            Instruction* user = dyn_cast<Instruction>(I.use_begin()->getUser());

            int toChange = valueToColor[I.getFunction()][user], flag = 0;
            for (Value *v : adjList[&I]) {
                if (valueToColor[I.getFunction()][v] == toChange) {
                    flag = 1;
                    break;
                }
            }
            if (flag) continue;

            if(I.getNextNode() != user || SAME_CONSIDER.find(user->getOpcode()) == SAME_CONSIDER.end() || findValue(user) == -1) continue;

            valueToColor[I.getFunction()][&I] = valueToColor[I.getFunction()][user];
        }
    }
}

void RegisterGraph::InverseColorMap()
{
    for(Function& F : *M) {
        colorToValue[&F] = vector<vector<Value*>>(numColors[&F]);;
        for(int i = 0; i <numColors[&F]; i++) {
            colorToValue[&F][i] = vector<Value*>();
        }
        for(auto p : valueToColor[&F]) {
            colorToValue[&F][p.second].push_back(p.first);
        }
    }
}

unsigned RegisterGraph::findValue(Value* V) {
    for(int i = 0; i < values.size(); i++) {
        if(values[i] == V) return i;
    }
    return -1;
}

bool RegisterGraph::doNotConsider(Instruction &I)
{
    if (DO_NOT_CONSIDER.find(I.getOpcode())!=DO_NOT_CONSIDER.end()) return true;

    CallInst *CI = dyn_cast<CallInst>(&I);
    if (!CI) return false;

    string fn = CI->getCalledFunction()->getName().str();
    if (UnfoldVectorInstPass::VLOADS.find(fn) != UnfoldVectorInstPass::VLOADS.end()) return true;

    return false;
}

} // namespace backend
