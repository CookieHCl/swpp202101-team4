#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"
#include "../backend/SplitSelfLoop.h"

#include "FixedCommandLine.h"
#include "Team4Header.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"

#include <string>

using namespace std;
using namespace llvm;

static cl::OptionCategory optCategory("SWPP Compiler options");

static cl::opt<string> optInput(
    cl::Positional, cl::desc("<input file>"), cl::Required,
    cl::value_desc("filename"), cl::cat(optCategory));

static cl::opt<string> optOutput(
    cl::Positional, cl::desc("[output file]"), cl::init("-"),
    cl::value_desc("filename"), cl::cat(optCategory));

static cl::opt<bool> optPrintProgress(
    "verbose", cl::desc("Print detailed progress"),
    cl::cat(optCategory));
static cl::alias optPrintProgressA(
    "v", cl::desc("Alias for --verbose"),
    cl::aliasopt(optPrintProgress), cl::cat(optCategory));

static cl::opt<bool> optEmitLLVM(
    "emit-llvm", cl::desc("Write output as LLVM IR"),
    cl::cat(optCategory));

enum class Opts {
  Arithmetic,
  BranchPredict,
  FunctionInline,
  GVN,
  LoopUnroll,
  LoopVectorize,
  MatmulTranspose,
  MemoryToStack,
  Phierase,
  RemoveUnused,
  SCCP,
  SimplifyCFG,
};

#define OPT_ENUM_VAL(enum, desc) clEnumValN(Opts::enum, #enum, desc)

static unsigned optOptimizationBits;
static cl::bits<Opts, unsigned> optOptimizations(
    "passes", cl::desc("Apply only selected optimizations:"),
    cl::location(optOptimizationBits), cl::CommaSeparated, cl::cat(optCategory),
    cl::values(
      OPT_ENUM_VAL(Arithmetic, "Replace with cheaper arithmetic operations"),
      OPT_ENUM_VAL(BranchPredict, "Set most used branch to false branch"),
      OPT_ENUM_VAL(FunctionInline, "Inline functions if possible"),
      OPT_ENUM_VAL(GVN, "Constant folding & eliminate fully redundant instructions and dead load"),
      OPT_ENUM_VAL(LoopUnroll, "Unroll for loop"),
      OPT_ENUM_VAL(LoopVectorize, "Vectorize load/store instruction in loop"),
      OPT_ENUM_VAL(MatmulTranspose, "LoopInterchange for more effective vectorize"),
      OPT_ENUM_VAL(MemoryToStack, "Use stack instead of heap"),
      OPT_ENUM_VAL(Phierase, "Erase phi node by copying basicblock"),
      OPT_ENUM_VAL(RemoveUnused, "Remove unused BB & alloca & instruction"),
      OPT_ENUM_VAL(SCCP, "Sparse Conditinal Constant Propagation"),
      OPT_ENUM_VAL(SimplifyCFG, "Simplify and canonicalize the CFG")
    ));

static cl::opt<bool> optInvertOptimization(
    "off", cl::desc("Instead of applying optimizations, exclude selected ones"),
    cl::cat(optCategory));

#define IFSET(enum, X) if (optOptimizations.isSet(Opts::enum)) { X; }

int main(int argc, char *argv[]) {
  //Parse command line arguments
  cl::HideUnrelatedOptions(optCategory);
  cl::ParseCommandLineOptions(argc, argv);
  if (!optOptimizationBits) {
    // if optimization is not specified, all optimizations should be enabled
    optOptimizationBits = -1;
  }
  if (optInvertOptimization) {
    optOptimizationBits = ~optOptimizationBits;
  }

  //Parse input LLVM IR module
  LLVMContext Context;
  unique_ptr<Module> M;

  SMDiagnostic Error;
  M = parseAssemblyFile(optInput, Error, Context);

  //If loading file failed:
  string errMsg;
  raw_string_ostream os(errMsg);
  Error.print("", os);

  if (!M)
    return 1;

  // Init managers
  FunctionPassManager FPM;
  FunctionPassManager FPM1;
  ModulePassManager MPM;

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  PassBuilder PB;

  // Register all the basic analyses with the managers
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // Add existing IR passes
  IFSET(GVN, FPM.addPass(GVN({true, true, true, true, true})))
  IFSET(SCCP, FPM.addPass(SCCPPass()))

  // matmul
  IFSET(MatmulTranspose, FPM1.addPass(MatmulTransposePass(optPrintProgress)))

  // Add IR passes
  IFSET(LoopUnroll, FPM.addPass(LoopUnrollPass(optPrintProgress)))
  IFSET(LoopVectorize, FPM.addPass(LoopVectorizePass(*M, optPrintProgress)))

  IFSET(SimplifyCFG, FPM.addPass(SimplifyCFGPass()))
  IFSET(Arithmetic, FPM.addPass(ArithmeticPass()))
  IFSET(Phierase, FPM.addPass(PhierasePass()))
  IFSET(RemoveUnused, FPM.addPass(RemoveUnusedPass()))
  IFSET(BranchPredict, FPM.addPass(BranchPredictPass(optPrintProgress)))

  // Add existing IR passes
  IFSET(SimplifyCFG, FPM.addPass(SimplifyCFGPass()))
  IFSET(GVN, FPM.addPass(GVN()))
  IFSET(SCCP, FPM.addPass(SCCPPass()))

  // Execute IR passes
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));
  if (optOptimizations.isSet(Opts::FunctionInline) && !optOptimizations.isSet(Opts::LoopUnroll))
    MPM.addPass(FunctionInlinePass());
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  // IFSET(FunctionInline, MPM.addPass(FunctionInlinePass()));
  IFSET(MemoryToStack, MPM.addPass(MemoryToStackPass(optPrintProgress)))
  MPM.run(*M, MAM);

  // If flag is set, write output as LLVM assembly
  if (optEmitLLVM) {
    error_code EC;
    raw_ostream *os =
      optOutput == "-" ? &outs() : new raw_fd_ostream(optOutput, EC);

    if (EC) {
      errs() << "Cannot open file: " << optOutput << "\n";
      return 1;
    }

    *os << *M;
    if (os != &outs()) delete os;

    return 0;
  }

  // Execute backend passes
  SplitSelfLoopPass().run(*M, MAM);
  UnfoldVectorInstPass().run(*M, MAM);
  AddressArgCastPass().run(*M, MAM);
  ConstExprRemovePass().run(*M, MAM);
  GEPUnpackPass().run(*M, MAM);
  RegisterSpillPass().run(*M, MAM);

  // Execute backend to emit assembly
  Backend B(optOutput, optPrintProgress);
  B.run(*M, MAM);

  return 0;
}
