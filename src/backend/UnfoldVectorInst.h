#ifndef UNFOLD_VECTOR_H
#define UNFOLD_VECTOR_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

const static string MOV_FN = "__backend_mov__";

namespace backend {
class UnfoldVectorInstPass : public PassInfoMixin<UnfoldVectorInstPass> {
public:
    static set<string> VLOADS, EXTRACT_ELEMENTS, VSTORES;

    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

extern map<Instruction*, unsigned int> ReservedColor;
}

#endif 