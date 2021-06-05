#include "MemoryToStackPass.h"

const char* MemoryToStackPass::STACK_COUNTER_NAME = "____stacksize";
const char* MemoryToStackPass::NEW_MALLOC_NAME = "____malloc";
const char* MemoryToStackPass::NEW_FREE_NAME = "____free";
const char* MemoryToStackPass::STACK_ALLOC_NAME = "____stackalloc";

/*
// Note that in llvm, global variable is pointer
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
Function* MemoryToStackPass::createNewMalloc(Module &M, Function* OrigMalloc,
    Value* StackCounter, IntegerType* I64Ty) {
  if (!OrigMalloc) {
    llvm_unreachable("There must be malloc to use stack");
  }

  FunctionType* MallocType = OrigMalloc->getFunctionType();
  Function* NewMalloc = Function::Create(MallocType, Function::ExternalLinkage,
      NEW_MALLOC_NAME, M);
  Function* StackAlloc = Function::Create(MallocType, Function::ExternalLinkage,
      STACK_ALLOC_NAME, M);

  BasicBlock* CondBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* MallocBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* StackBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);

  // Instructions for CondBB
  auto* OldSize = new LoadInst(I64Ty, StackCounter, Twine(), CondBB);
  auto* NewSize = BinaryOperator::Create(BinaryOperator::Add,
      NewMalloc->getArg(0), OldSize, Twine(), CondBB);
  auto* HasOverflow = new ICmpInst(*CondBB, ICmpInst::ICMP_SGT,
      NewSize, ConstantInt::getSigned(I64Ty, STACK_SIZE));
  BranchInst::Create(MallocBB, StackBB, HasOverflow, CondBB);

  // Instructions for MallocBB
  auto* CallMalloc = CallInst::Create(MallocType, OrigMalloc,
      NewMalloc->getArg(0), Twine(), MallocBB);
  ReturnInst::Create(M.getContext(), CallMalloc, MallocBB);

  // Instructions for StackBB
  new StoreInst(NewSize, StackCounter, StackBB);
  auto* CallStackAlloc = CallInst::Create(MallocType, StackAlloc,
      NewMalloc->getArg(0), Twine(), StackBB);
  ReturnInst::Create(M.getContext(), CallStackAlloc, StackBB);

  return NewMalloc;
}

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
Function* MemoryToStackPass::createNewFree(Module &M, Function* OrigFree,
    IntegerType* I64Ty) {
  if (!OrigFree) {
    return nullptr;
  }

  FunctionType* FreeType = OrigFree->getFunctionType();
  Function* NewFree = Function::Create(FreeType, Function::ExternalLinkage,
      NEW_FREE_NAME, M);

  BasicBlock* CondBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);
  BasicBlock* FreeBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);
  BasicBlock* VoidBB = BasicBlock::Create(M.getContext(), Twine(), NewFree);

  // Instructions for CondBB
  auto* ConstantPtr = ConstantExpr::getIntToPtr(ConstantInt::get(I64Ty, 123456),
      NewFree->getArg(0)->getType());
  auto* IsHeap = new ICmpInst(*CondBB, ICmpInst::ICMP_UGT,
      NewFree->getArg(0), ConstantPtr);
  BranchInst::Create(FreeBB, VoidBB, IsHeap, CondBB);

  // Instructions for FreeBB
  auto* CallMalloc = CallInst::Create(FreeType, OrigFree, NewFree->getArg(0),
      Twine(), FreeBB);
  ReturnInst::Create(M.getContext(), FreeBB);

  // Instructions for VoidBB
  ReturnInst::Create(M.getContext(), VoidBB);

  return NewFree;
}

PreservedAnalyses MemoryToStackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  logs() << "---------- Start MemoryToStackPass ----------\n";

  IntegerType* I64Ty = Type::getInt64Ty(M.getContext());

  Function* OrigMalloc = M.getFunction("malloc");
  // If we don't have malloc, we don't need to change to stack at all
  if (!OrigMalloc) {
    logs() << "Module doesn't have malloc;\n"
        << "---------- End MemoryToStackPass ----------";
    return PreservedAnalyses::all();
  }

  // It's normal to not have free; don't check it
  Function* OrigFree = M.getFunction("free");

  // global variable to count stack's size
  auto* StackCounter = new GlobalVariable(M, I64Ty, false,
      GlobalVariable::PrivateLinkage, ConstantInt::getNullValue(I64Ty),
      STACK_COUNTER_NAME);

  // create new malloc & free
  Function* NewMalloc = createNewMalloc(M, OrigMalloc, StackCounter, I64Ty);
  Function* NewFree = createNewFree(M, OrigFree, I64Ty);

  // replace malloc & free
  //replaceFunction(M, OrigMalloc, NewMalloc);
  //replaceFunction(M, OrigFree, NewFree);

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
