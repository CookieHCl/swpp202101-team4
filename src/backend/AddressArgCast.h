#ifndef ADDRESS_ARG_CAST_H
#define ADDRESS_ARG_CAST_H

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

namespace backend {
class AddressArgCastPass : public PassInfoMixin<AddressArgCastPass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};
}

#endif 