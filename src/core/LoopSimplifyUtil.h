#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"

using namespace llvm;


void rotateLoop(Function &F, FunctionAnalysisManager &FAM);
void makeSimplifyLCSSA(Function &F, FunctionAnalysisManager &FAM);
void makeAllocaAsPHI(Function &F, FunctionAnalysisManager &FAM);
void insertPreheader(Function &F, FunctionAnalysisManager &FAM);
