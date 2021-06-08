#include "MemoryToStackPass.h"

// make new function declaration with same type
#define NEW_FUNC(OldFunc, NewName, M) (!OldFunc ? nullptr : Function::Create( \
    OldFunc->getFunctionType(), Function::ExternalLinkage, NewName, M))

// return sizeof(T) in bytes.
// code from Backend.cpp
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

// stack can conflict; replace alloca with new malloc
// manually replacing alloca with new malloc is cheaper than sp with ____malloc
// because we don't need to add/sub to calculate address from sp
void MemoryToStackPass::replaceAlloca(Module &M, Function* NewMalloc) {
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

        IntegerType* Int64Type = Type::getInt64Ty(M.getContext());
        auto* CallMalloc = CallInst::Create(NewMalloc->getFunctionType(),
            NewMalloc, ConstantInt::get(Int64Type, ptrSize), Twine(), Alloca);
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
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (auto* CB = dyn_cast<CallBase>(&*I)) {
        if (CB->getCalledFunction() == OrigFun) {
          CB->setCalledFunction(NewFun);
        }
      }
    }
  }
}

// remove unnessary free from main;
// only remove free when malloc won't appear later
//
// algorithm:
// check functions have malloc to determine call inst
// then topological sort BBs and iterate only when malloc doesn't appear
void MemoryToStackPass::removeUnnessaryFree(Module &M, Function* NewMalloc,
    Function* NewFree) {
  // store functions with malloc
  SmallPtrSet<Function*, 4> functionsWithMalloc = {NewMalloc};
  for (Function &F : M) {
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (auto* CB = dyn_cast<CallBase>(&*I)) {
        if (CB->getCalledFunction() == NewMalloc) {
          functionsWithMalloc.insert(&F);
        }
      }
    }
  }

  // used for topological sort; count successors with possible malloc
  DenseMap<BasicBlock*, unsigned> mallocSuccessors;
  std::queue<BasicBlock*> q;

  for (BasicBlock &BB : *(M.getFunction("main"))) {
    unsigned succSize = succ_size(&BB);
    if (succSize == 0) {
      q.push(&BB);
    } else {
      mallocSuccessors[&BB] = succ_size(&BB);
    }
  }

  while (!q.empty()) {
    BasicBlock* BB = q.front(); q.pop();
    bool hasNoMalloc = true;

    // iterate backwards to check malloc doesn't appear after free
    for (auto I = BB->rbegin(), E = BB->rend(); I != E;) {
      // increase iterator first to avoid invalidation
      if (auto* CB = dyn_cast<CallBase>(&*(I++))) {
        Function* calledFunction = CB->getCalledFunction();
        if (calledFunction == NewFree) {
          CB->eraseFromParent();
        } else if (functionsWithMalloc.contains(calledFunction)) {
          hasNoMalloc = false;
          break;
        }
      }
    }

    logs() << BB->getName() << (hasNoMalloc ? " has no" : "has") << " malloc\n";

    // if BB doesn't have malloc, decrese successor count from predecessors
    if (hasNoMalloc) {
      for (BasicBlock *Pred : predecessors(BB)) {
        if (--mallocSuccessors[Pred] == 0) {
          q.push(Pred);
        }
      }
    }
  }
}

PreservedAnalyses MemoryToStackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  logs() << "---------- Start MemoryToStackPass ----------\n";

  Function* OrigMalloc = M.getFunction("malloc");
  // if we don't have malloc, we don't need to change to stack at all
  if (!OrigMalloc) {
    logs() << "Module doesn't have malloc;\n"
              "---------- End MemoryToStackPass ----------\n";
    return PreservedAnalyses::all();
  }

  // it's normal to not have free; don't check it
  Function* OrigFree = M.getFunction("free");

  // create new malloc & free
  // actual function definition is in Backend.cpp
  Function* NewMalloc = NEW_FUNC(OrigMalloc, "____malloc", M);
  Function* NewFree = NEW_FUNC(OrigFree, "____free", M);

  // replace original functions
  replaceAlloca(M, NewMalloc);
  replaceFunction(M, OrigMalloc, NewMalloc);
  replaceFunction(M, OrigFree, NewFree);

  // remove unnessary free from main
  removeUnnessaryFree(M, NewMalloc, NewFree);

  logs() << "---------- End MemoryToStackPass ----------\n";
  return PreservedAnalyses::none();
}
