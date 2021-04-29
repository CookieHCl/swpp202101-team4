/*
    GEPUnpackPass
*/

#include "GEPUnpack.h"

using namespace llvm;
using namespace std;

namespace backend {

// Return sizeof(T) in bytes.
static
unsigned getAccessSize(Type *T) {
  if (isa<PointerType>(T))
    return 8;
  else if (isa<IntegerType>(T)) {
    return T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8);
  } else if (isa<ArrayType>(T)) {
    return getAccessSize(T->getArrayElementType()) * T->getArrayNumElements();
  }
  assert(false && "Unsupported access size type!");
}

PreservedAnalyses GEPUnpackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVMContext &Context = M.getContext();

  // Unpack GEPs.
  for (Function &F : M) {
    set<Instruction *> s;
    for (auto it = inst_begin(&F); it != inst_end(&F); ++it) {
      Instruction &I = *it;
      if (I.getOpcode() != Instruction::GetElementPtr) continue;

      // IRBuilder<> Builder(&I);
      GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(&I);

      Value *ptrOp = GI->getPointerOperand();
      Type *curr = ptrOp->getType();
      curr = curr->getPointerElementType();

      Instruction *pti = CastInst::CreateBitOrPointerCast(
          ptrOp, IntegerType::getInt64Ty(Context));
      pti->insertBefore(&I);

      vector<Instruction *> v;
      v.push_back(pti);
      for (auto opIt = GI->idx_begin(); opIt != GI->idx_end(); ++opIt) {
        Value *op = *opIt;
        unsigned size = getAccessSize(curr);
        Instruction *mul = BinaryOperator::CreateMul(
            op, ConstantInt::get(IntegerType::getInt64Ty(Context), size, true));
        mul->insertBefore(&I);
        Instruction *add = BinaryOperator::CreateAdd(v.back(), mul);
        v.push_back(add);
        add->insertBefore(&I);
        if (curr->isArrayTy()) curr = curr->getArrayElementType();
      }

      Instruction *itp =
          CastInst::CreateBitOrPointerCast(v.back(), I.getType());
      itp->insertBefore(&I);
      I.replaceAllUsesWith(itp);
      s.insert(&I);
    }
    for (auto &I : s) {
      I->eraseFromParent();
    }
  }

  return PreservedAnalyses::all();
}
}  // namespace backend