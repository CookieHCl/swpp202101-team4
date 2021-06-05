#include "MemoryToStackPass.h"

const char* MemoryToStackPass::STACK_COUNTER_NAME = "____stacksize";
const char* MemoryToStackPass::NEW_MALLOC_NAME = "____malloc";
const char* MemoryToStackPass::NEW_FREE_NAME = "____free";
const char* MemoryToStackPass::STACK_ALLOC_NAME = "____stackalloc";

#define CREATE_FUNCTION(name, type, M) \
  Function::Create(type, Function::ExternalLinkage, name, M)

void MemoryToStackPass::replaceMalloc(Module &M, Value* StackCounter) {
  logs() << "Replacing malloc...\n";
  Function* OrigMalloc = M.getFunction("malloc");
  if (!OrigMalloc) {
    logs() << "no malloc\n";
    return;
  }

  FunctionType* MallocType = OrigMalloc->getFunctionType();
  Function* NewMalloc = CREATE_FUNCTION(NEW_MALLOC_NAME, MallocType, M);
  Function* StackAlloc = CREATE_FUNCTION(STACK_ALLOC_NAME, MallocType, M);

  /*
  // Note that global variable is pointer
  ____malloc(size) {
    if (size + *StackCounter > STACK_SIZE) { // CondBB
      return malloc(size); // MallocBB
    }
    else {
      *StackCounter += size;
      return ____stackalloc(size); // StackBB
    }
  }
  */

  BasicBlock* CondBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* MallocBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* StackBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);

  // Instructions for CondBB
  auto* OldSize = new LoadInst(Type::getInt64Ty(M.getContext()),
      StackCounter, Twine(), CondBB);
  auto* NewSize = BinaryOperator::Create(BinaryOperator::Add,
      NewMalloc->getArg(0), OldSize, Twine(), CondBB);
  auto* HasOverflow = new ICmpInst(*CondBB, ICmpInst::ICMP_SGT,
      NewSize, ConstantInt::getSigned(Type::getInt64Ty(M.getContext()), STACK_SIZE));
  BranchInst::Create(MallocBB, StackBB, HasOverflow, CondBB);

  // Instructions for MallocBB
  auto* CallMalloc= CallInst::Create(MallocType, OrigMalloc,
      NewMalloc->getArg(0), Twine(), MallocBB);
  ReturnInst::Create(M.getContext(), CallMalloc, MallocBB);

  // Instructions for StackBB
  new StoreInst(NewSize, StackCounter, StackBB);
  auto* CallStackAlloc= CallInst::Create(MallocType, StackAlloc,
      NewMalloc->getArg(0), Twine(), StackBB);
  ReturnInst::Create(M.getContext(), CallStackAlloc, StackBB);
}

void MemoryToStackPass::replaceFree(Module &M) {
  logs() << "Replacing free...\n";
  Function* OrigFree = M.getFunction("free");
  if (!OrigFree) {
    logs() << "no free\n";
    return;
  }

  FunctionType* FreeType = OrigFree->getFunctionType();
  Function* NewFree = CREATE_FUNCTION(NEW_FREE_NAME, FreeType, M);

  /*
  // Note that pointer is in heap if pointer >= 204800
  ____free(p) {
    if (p > 123456) { // CondBB
      free(p); // FreeBB
    }
    else {
      return; // VoidBB
    }
  }
  */

  BasicBlock* CondBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);
  BasicBlock* FreeBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);
  BasicBlock* VoidBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);

  // Instructions for CondBB
  auto* ConstantPtr = ConstantExpr::getIntToPtr(ConstantInt::get(Type::getInt64Ty(M.getContext()), 123456),
      NewFree->getArg(0)->getType());
  auto* IsHeap = new ICmpInst(*CondBB, ICmpInst::ICMP_UGT,
      NewFree->getArg(0), ConstantPtr);
  BranchInst::Create(FreeBB, VoidBB, IsHeap, CondBB);

  // Instructions for FreeBB
  auto* CallMalloc= CallInst::Create(FreeType, OrigFree,
      NewFree->getArg(0), Twine(), FreeBB);
  ReturnInst::Create(M.getContext(), FreeBB);

  // Instructions for VoidBB
  ReturnInst::Create(M.getContext(), VoidBB);
}

PreservedAnalyses MemoryToStackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  logs() << "---------- Start MemoryToStackPass ----------\n";

  auto* ConstZero = ConstantInt::getSigned(Type::getInt64Ty(M.getContext()), 0);
  auto* StackCounter = new GlobalVariable(M, Type::getInt64Ty(M.getContext()),
      false, GlobalVariable::PrivateLinkage, ConstZero, STACK_COUNTER_NAME);

  replaceMalloc(M, StackCounter);
  replaceFree(M);

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
