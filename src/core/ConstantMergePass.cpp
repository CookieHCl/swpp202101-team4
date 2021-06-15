#include "ConstantMergePass.h"

ConstantMergePass::ChainUnit ConstantMergePass::getChainUnit(Instruction *inst) {

  if (!inst->getType()->isIntegerTy()) return {NULL, NULL};

  Value *val;
  ConstantInt *con;
  const bool isCommutative = Instruction::isCommutative(inst->getOpcode());

  if (match(inst, m_c_BinOp(m_Value(val), m_ConstantInt(con)))) return {val, con};
  
  if (isCommutative && match(inst, m_c_BinOp(m_ConstantInt(con), m_Value(val)))) return {val, con};
  
  return {NULL, NULL};
}

bool ConstantMergePass::tryMergeConstant(Instruction *inst, BinaryOperator::BinaryOps ops) {
  bool isChanged = false;

  ChainUnit chainUnit = getChainUnit(inst);

  if (!chainUnit.first) return isChanged;

  SmallVector<User*> userVector(inst->user_begin(), inst->user_end());

  for (User *user : userVector) {
    Instruction *userInst = dyn_cast<Instruction>(user);
    if (!userInst) continue;

    if (userInst->getOpcode() != ops) continue;

    ChainUnit nextChainUnit = getChainUnit(userInst);
    if (!nextChainUnit.first) continue;

    if (nextChainUnit.second->getType() != chainUnit.second->getType()) continue;

    APInt currentInt = chainUnit.second->getValue();
    APInt nextInt = nextChainUnit.second->getValue();

    APInt newInt = (ops == Instruction::Mul) ? (currentInt * nextInt) : (currentInt + nextInt);

    Constant *newConst = ConstantInt::get(nextChainUnit.second->getType(), newInt);

    Instruction *newInst = BinaryOperator::CreateWithCopiedFlags(ops, chainUnit.first, newConst, userInst);
    ReplaceInstWithInst(userInst, newInst);

    isChanged = true;
  }

  return isChanged;
}

bool ConstantMergePass::MergeConstant(Function &F, BinaryOperator::BinaryOps ops) {
  bool isInstChanged = false;

  const bool isCommutative = Instruction::isCommutative(ops);

  while (true) {
    bool isChanged = false;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction *inst = &*I;
      if (inst->getType()->isIntegerTy() && inst->getOpcode() == ops && inst->getNumOperands() == 2)
        if ((isChanged = tryMergeConstant(inst, ops))) break;
    }

    isInstChanged |= isChanged;
    if (!isChanged) break;
  }

  return isInstChanged;
}

PreservedAnalyses ConstantMergePass::run(Function &F, FunctionAnalysisManager &FAM) {
  logs() << "[ConstantMergePass] @" << F.getName() << '\n';
  makeAllocaAsPHI(F, FAM);

  bool isChanged = false;

  vector<BinaryOperator::BinaryOps> opsVector = {Instruction::Add, Instruction::Mul, Instruction::Sub,
                                                 Instruction::Shl, Instruction::LShr, Instruction::AShr};
  
  for (BinaryOperator::BinaryOps ops : opsVector)
    isChanged |= this->MergeConstant(F, ops);

  logs() << "[ConstantMergePass] @" << F.getName() << " END\n";

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
