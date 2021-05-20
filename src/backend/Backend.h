#ifndef BACKEND_H
#define BACKEND_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "LivenessAnalysis.h"
#include "UnfoldVectorInst.h"

#include <string>
#include <queue>
#include <set>

using namespace llvm;
using namespace std;
using namespace backend;

namespace backend {

//---------------------------------------------------------------
//backend/TargetMachine.cpp
//---------------------------------------------------------------

class Memory;
class Register;

class Symbol{
protected:
  string name;
  Symbol() = default;
  virtual ~Symbol() = default;
  Symbol(string name): name(name) {}
public:
  string getName();
  virtual Register* castToRegister() = 0;
  virtual Memory* castToMemory() = 0;
};
//Hardware registers
class Register : public virtual Symbol {
public:
  Register(string name): Symbol(name) {}
  ~Register() {}
  Register* castToRegister() {return this;}
  Memory* castToMemory() {return NULL;}
};
//Memory addresses(GV, )
class Memory : public virtual Symbol {
  Register* base;
  int64_t offset;
public:
  Memory(Register* base, int64_t offset);
  ~Memory() {}
  Register* getBase();
  int64_t getOffset();
  Register* castToRegister() {return NULL;}
  Memory* castToMemory() {return this;}
};
class Func : public Symbol {
public:
  Func(string name): Symbol(name) {}
  ~Func() {}
  Register* castToRegister() {return NULL;}
  Memory* castToMemory() {return NULL;}
};
class Block : public Symbol {
public:
  Block(string name): Symbol(name) {}
  ~Block() {}
  Register* castToRegister() {return NULL;}
  Memory* castToMemory() {return NULL;}
};

class TargetMachine {
  Register* regfile[32];
  Register* argfile[16];
  Register* spreg;
  Register* gvpreg;
  Register* fakereg;

public:
  //Read-only getters for the register data stored in TargetMachine object.
  Register* reg(unsigned);
  Register* arg(unsigned);
  Register* sp();
  Register* gvp();
  Register* fakeReg();

  unsigned regNo(Symbol*);
  unsigned argNo(Symbol*);

  //Checks the validity of the given symbol within the machine.
  bool valid(Symbol*);
  
  //Initializes the target machine.
  TargetMachine();
};

//---------------------------------------------------------------
//backend/AssemblyEmitter.cpp
//---------------------------------------------------------------

class SymbolMap;
class AssemblyEmitter : public InstVisitor<AssemblyEmitter> {
  raw_ostream *fout;
  TargetMachine* TM;

  //Input IR characteristics
  SymbolMap* SM;
  map<Function*, unsigned> spOffset;

  //interface from values to string names of assigned symbols.
  //references SM to find the assignee.
  string name(Value*);

  //functions that emit assembly
  string emitInst(vector<string>);
  string emitBinary(Instruction*, string, string, string);
  string emitCopy(Instruction*, Value*);

  //updates the bandwidth and returns the value.
  string stringBandWidth(Value*);

public:
  AssemblyEmitter(raw_ostream *fout, TargetMachine& TM, SymbolMap& SM, map<Function*, unsigned>& spOffset);

  //Visit functions; should statically override.
  void visitFunction(Function&);
  void visitBasicBlock(BasicBlock&);

  void visitICmpInst(ICmpInst&);
  void visitAllocaInst(AllocaInst&);
  void visitLoadInst(LoadInst&);
  void visitStoreInst(StoreInst&);
  void visitPHINode(PHINode&);
  void visitTruncInst(TruncInst&);
  void visitZExtInst(ZExtInst&);
  void visitSExtInst(SExtInst&);
  void visitPtrToIntInst(PtrToIntInst&);
  void visitIntToPtrInst(IntToPtrInst&);
  void visitBitCastInst(BitCastInst&);
  void visitSelectInst(SelectInst&);
  void visitCallInst(CallInst&);
  void visitReturnInst(ReturnInst&);
  void visitBranchInst(BranchInst&);
  void visitSwitchInst(SwitchInst&);
  void visitBinaryOperator(BinaryOperator&);
};

//---------------------------------------------------------------
//backend/Backend.cpp
//---------------------------------------------------------------

class SymbolMap {
  //The target module to rename.
  Module* M;
  //Maps every value(GV, arg, reg) to a new symbol defined in the TargetMachine.
  map<Value*, Symbol*> symbolTable;
  //Target Machine model of our target.
  TargetMachine TM;

public:
  //Initializes the symbolTable.
  //Every register/memory stored values(GV, insts, args) are mapped to register.
  //Allocas are allocated a bit after.
  SymbolMap(Module*, TargetMachine&, RegisterGraph&);

  //interface for indirect access to symbolTable.
  void set(Value*, Symbol*);
  Symbol* get(Value*);
};

class Backend : public PassInfoMixin<Backend> {
  //File to print assembly (.s).
  string outputFile;
  //if true, prints the intermediate steps.
  bool printProcess;
  //Model for the target machine of our project.
  //Contains information about register files and 
  TargetMachine TM;  

public:

  Backend(string outputFile, bool printProcess = false) :
      outputFile(outputFile), printProcess(printProcess), TM() {}
  
  //runs the backend, which emits the assembly to given .s file.
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  //SymbolMap initially does not mark alloca and its derivatives.
  //processing should be seperately done.
  map<Function*, unsigned> processAlloca(Module&, SymbolMap&);
  void SSAElimination(Module &, SymbolMap &, RegisterGraph &);
  void ElimBackendMovInst(Module &, SymbolMap &);
  void addEdges(BasicBlock &, BasicBlock &, SymbolMap &, vector<vector<Symbol *>> &);
  Value *findLeastReg(Symbol *, BasicBlock &, SymbolMap &);
};

//Function which returns the byte size of a Type.
//brought from SimpleBackend(from repo swpp202001-compiler).
unsigned getAccessSize(Type *T);

unsigned getBitWidth(Type* T);

} //end namespace backend

#endif