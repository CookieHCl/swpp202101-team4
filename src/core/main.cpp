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

enum Opts {
  Arithmetic,
  FunctionInline,
  LoopVectorize,
  RemoveUnused,
  SimplifyCFG,
};

static unsigned optOptimizationBits;
static cl::bits<Opts, unsigned> optOptimizations(
    "passes", cl::desc("Apply only selected optimizations:"),
    cl::location(optOptimizationBits), cl::CommaSeparated, cl::cat(optCategory),
    cl::values(
      clEnumVal(Arithmetic, "Replace with cheaper arithmetic operations"),
      clEnumVal(FunctionInline, "Inline functions if possible"),
      clEnumVal(LoopVectorize, "Vectorize load/store instruction in loop"),
      clEnumVal(RemoveUnused, "Remove unused BB & alloca & instruction"),
      clEnumVal(SimplifyCFG, "Simplify and canonicalize the CFG")
      ));

#define IFSET(enum, X) if (optOptimizations.isSet(enum)) { X; }

int main(int argc, char *argv[]) {
  //Parse command line arguments
  cl::HideUnrelatedOptions(optCategory);
  cl::ParseCommandLineOptions(argc, argv);
  if (!optOptimizationBits) {
    // if optimization is not specified, all optimizations should be enabled
    optOptimizationBits = -1;
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
  IFSET(Opts::SimplifyCFG, FPM.addPass(SimplifyCFGPass()))

  // Add IR passes
  IFSET(Opts::Arithmetic, FPM.addPass(ArithmeticPass()))
  IFSET(Opts::LoopVectorize, FPM.addPass(LoopVectorizePass(*M)))
  IFSET(Opts::RemoveUnused, FPM.addPass(RemoveUnusedPass()))

  // Add existing IR passes
  IFSET(Opts::SimplifyCFG, FPM.addPass(SimplifyCFGPass()))

  // Execute IR passes
  IFSET(Opts::FunctionInline, MPM.addPass(FunctionInlinePass()))
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
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
  LivenessAnalysis().run(*M, MAM);
  SpillCostAnalysis().run(*M, MAM);
  AddressArgCastPass().run(*M, MAM);
  ConstExprRemovePass().run(*M, MAM);
  GEPUnpackPass().run(*M, MAM);
  RegisterSpillPass().run(*M, MAM);

  // Execute backend to emit assembly
  Backend B(optOutput, optPrintProgress);
  B.run(*M, MAM);

  return 0;
}
