#include "MemoryToStackPass.h"

const char* MemoryToStackPass::STACK_POINTER_NAME = "____sp";
const char* MemoryToStackPass::NEW_MALLOC_NAME = "____malloc";
const char* MemoryToStackPass::NEW_FREE_NAME = "____free";

/*
// Note that in llvm, global variable is pointer
____malloc(size) {
  if (size > *StackPointer) { // CondBB
    return malloc(size); // MallocBB
  }
  else {
    return (*StackPointer -= size); // StackBB
  }
}
*/
Function* MemoryToStackPass::createNewMalloc(Module &M, Function* OrigMalloc,
    Value* StackPointer, IntegerType* I64Ty) {
  if (!OrigMalloc) {
    llvm_unreachable("There must be malloc to use stack");
  }

  FunctionType* MallocType = OrigMalloc->getFunctionType();
  Function* NewMalloc = Function::Create(MallocType, Function::ExternalLinkage,
      NEW_MALLOC_NAME, M);
  NewMalloc->setAttributes(OrigMalloc->getAttributes());

  BasicBlock* CondBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* MallocBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);
  BasicBlock* StackBB = BasicBlock::Create(M.getContext(), Twine(), NewMalloc);

  // Instructions for CondBB
  auto* RemainingSize = new LoadInst(I64Ty, StackPointer, Twine(), CondBB);
  auto* HasOverflow = new ICmpInst(*CondBB, ICmpInst::ICMP_UGT,
      NewMalloc->getArg(0), RemainingSize);
  BranchInst::Create(MallocBB, StackBB, HasOverflow, CondBB);

  // Instructions for MallocBB
  auto* CallMalloc = CallInst::Create(MallocType, OrigMalloc,
      NewMalloc->getArg(0), Twine(), MallocBB);
  ReturnInst::Create(M.getContext(), CallMalloc, MallocBB);

  // Instructions for StackBB
  auto* NewSize = BinaryOperator::Create(BinaryOperator::Sub,
      RemainingSize, NewMalloc->getArg(0), Twine(), StackBB);
  new StoreInst(NewSize, StackPointer, StackBB);
  auto* NewPointer = new IntToPtrInst(NewSize, MallocType->getReturnType(),
      Twine(), StackBB);
  ReturnInst::Create(M.getContext(), NewPointer, StackBB);

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
  NewFree->setAttributes(OrigFree->getAttributes());

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

// Return sizeof(T) in bytes.
// Code from Backend.cpp
static uint64_t getAccessSize(Type *T) {
  if (isa<PointerType>(T))
    return 8;
  else if (isa<IntegerType>(T)) {
    return T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8);
  } else if (isa<ArrayType>(T)) {
    return getAccessSize(T->getArrayElementType()) * T->getArrayNumElements();
  }
  llvm_unreachable("Unsupported access size type!");
}

// stack can conflict; replace alloca with new malloc so nobody use stack
void MemoryToStackPass::replaceAlloca(Module &M, Function* NewMalloc,
    IntegerType* I64Ty) {
  for (Function &F : M) {
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;) {
      // increase iterator first to avoid invalidation
      if (auto* Alloca = dyn_cast<AllocaInst>(&*(I++))) {
        uint64_t ptrSize = getAccessSize(Alloca->getAllocatedType());
        // malloc size should be a multiple of 8
        // we don't do this in getAccessSize because array size will become huge
        ptrSize = (ptrSize + 7) / 8 * 8;

        logs() << "Alloca type: " << *(Alloca->getType()) << '\n';
        logs() << "Alloca size: " << ptrSize << '\n';

        auto* CallMalloc = CallInst::Create(NewMalloc->getFunctionType(),
            NewMalloc, ConstantInt::get(I64Ty, ptrSize), Twine(), Alloca);
        auto* CastMalloc = new BitCastInst(CallMalloc, Alloca->getType());

        ReplaceInstWithInst(Alloca, CastMalloc);
        // log after replace to print properly assigned instruction
        logs() << "Replace with " << *CallMalloc << '\n' << *CastMalloc << '\n';
      }
    }
  }
}

// replace all calls calling OrigFun except those inside NewFun
void MemoryToStackPass::replaceFunction(Module &M, Function* OrigFun,
    Function* NewFun) {
  for (Function &F : M) {
    if (&F == NewFun) {
      continue;
    }

    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (auto* CB = dyn_cast<CallBase>(&*I)) {
        if (CB->getCalledFunction() == OrigFun) {
          CB->setCalledFunction(NewFun);
        }
      }
    }
  }
}

PreservedAnalyses MemoryToStackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  logs() << "---------- Start MemoryToStackPass ----------\n";

  IntegerType* I64Ty = Type::getInt64Ty(M.getContext());

  Function* OrigMalloc = M.getFunction("malloc");
  // If we don't have malloc, we don't need to change to stack at all
  if (!OrigMalloc) {
    logs() << "Module doesn't have malloc;\n"
              "---------- End MemoryToStackPass ----------\n";
    return PreservedAnalyses::all();
  }

  // It's normal to not have free; don't check it
  Function* OrigFree = M.getFunction("free");

  // global variable to count stack's size; sp = left size
  auto* StackPointer = new GlobalVariable(M, I64Ty, false,
      GlobalVariable::PrivateLinkage, ConstantInt::get(I64Ty, STACK_SIZE),
      STACK_POINTER_NAME);

  // create new malloc & free
  Function* NewMalloc = createNewMalloc(M, OrigMalloc, StackPointer, I64Ty);
  Function* NewFree = createNewFree(M, OrigFree, I64Ty);

  // replace original functions
  replaceAlloca(M, NewMalloc, I64Ty);
  replaceFunction(M, OrigMalloc, NewMalloc);
  replaceFunction(M, OrigFree, NewFree);

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
