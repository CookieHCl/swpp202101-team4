#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"
#include "../backend/SplitSelfLoop.h"

#include "Team4Header.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"

#include <string>

using namespace std;
using namespace llvm;

int main(int argc, char *argv[]) {
  //Parse command line arguments
  if(argc!=3) return -1;
  string optInput = argv[1];
  string optOutput = argv[2];
  bool optPrintProgress = false;

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

  // Add IR passes
  //FPM.addPass(MyPass());

  // Execute IR passes
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  MPM.run(*M, MAM);

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
