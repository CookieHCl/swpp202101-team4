#include "MemoryToStackPass.h"

const char* MemoryToStackPass::STACK_POINTER_NAME = "____sp";
const char* MemoryToStackPass::NEW_MALLOC_NAME = "____malloc";
const char* MemoryToStackPass::NEW_FREE_NAME = "____free";

#define NEW_FUNC(OldFunc, NewName, M) (OldFunc ? \
    Function::Create(OldFunc->getFunctionType(), Function::ExternalLinkage, \
    NewName, M) : nullptr)

// Code from Backend.cpp. Return sizeof(T) in bytes.
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

  // create new malloc & free
  // actual function definition is in Backend.cpp
  Function* NewMalloc = NEW_FUNC(OrigMalloc, NEW_MALLOC_NAME, M);
  Function* NewFree = NEW_FUNC(OrigFree, NEW_FREE_NAME, M);

  // replace original functions
  replaceAlloca(M, NewMalloc, I64Ty);
  replaceFunction(M, OrigMalloc, NewMalloc);
  replaceFunction(M, OrigFree, NewFree);

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
