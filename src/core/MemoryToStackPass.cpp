#include "MemoryToStackPass.h"

PreservedAnalyses MemoryToStackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  logs() << "---------- Start MemoryToStackPass ----------\n";

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
