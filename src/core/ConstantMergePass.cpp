#include "ConstantMergePass.h"

/*
 *                        Constant Merge Pass
 *
 *  This Pass merge consecutive constant binary operation.
 *  It regards typical binary operations like c = c + 1 or c = c * 2
 *  This Pass is recommended to place immediately after LoopVectorize
 *  or passes like merge multiple blocks contains typicall binary operations.
 *  
 */ 

Instruction* ConstantMergePass::ChainUnit::createInstruction(Instruction *flagInst) {
  Value *op1 = value;
  Value *op2 = constant;
  if (this->isReversed) std::swap(op1, op2);
  return BinaryOperator::CreateWithCopiedFlags(this->binOp, op1, op2, flagInst);
}

ConstantMergePass::ChainUnit ConstantMergePass::ChainUnit::merge(ConstantMergePass::ChainUnit &sourceUnit) {
  if (this->constant->getType() != sourceUnit.constant->getType()) return ChainUnit::blank();

  if (this->binOp != sourceUnit.binOp) return ChainUnit::blank();

  APInt sourceInt = sourceUnit.constant->getValue();
  APInt Int = this->constant->getValue();

  switch (this->binOp) {
    case Instruction::Sub: {
      // Modulo sub is distributive so do not have to care about overflow/underflow
      // There are four cases.
      //
      // Case 1.
      // x1 = x0 - c1
      // x2 = x1 - c2
      // => x2 = x0 - c1 - c2 = x0 - (c1 + c2)
      //
      // Case 2.
      // x1 = x0 - c1
      // x2 = c2 - x1
      // => x2 = c2 - (x0 - c1) = (c1 + c2) - x0
      //
      // Case 3.
      // x1 = c1 - x0
      // x2 = x1 - c2
      // => x2 = c1 - x0 - c2 = (c1 - c2) - x0
      //
      // Case 4.
      // x1 = c1 - x0
      // x2 = c2 - x1
      // => x2 = c2 - c1 + x0 = x0 - (c1 - c2)
      //
      // Therefore, each isReversed determines const and isReversed of mergeUnit
      //
      // if x1 = Value - Constant, the result constant is c1 + c2
      // else, the result consttant is c1 - c2
      //
      // if (x2 = V - C and x1 = V - C) or (x2 = C - V and x1 = C - V), the result is Value - Constant
      // else, the result is Constant - Value
      //
      APInt newInt = sourceUnit.isReversed ? (sourceInt - Int) : (sourceInt + Int);
      if (sourceUnit.isReversed) outs() << sourceInt << " - " << Int << " = " << newInt << "\n";
      Constant *newConst = ConstantInt::get(this->constant->getType(), newInt);
      ConstantInt *newConstInt = dyn_cast<ConstantInt>(newConst);
      bool newIsReversed = this->isReversed ^ sourceUnit.isReversed;
      return newConstInt ? ChainUnit(this->binOp, sourceUnit.value, newConstInt, newIsReversed) : ChainUnit::blank();
    }
    case Instruction::Mul:
    case Instruction::Add:
    case Instruction::Shl:
    case Instruction::AShr:
    case Instruction::LShr: {
      APInt newInt = (this->binOp == Instruction::Mul) ? (sourceInt * Int) : (sourceInt + Int);
      Constant *newConst = ConstantInt::get(this->constant->getType(), newInt);
      ConstantInt *newConstInt = dyn_cast<ConstantInt>(newConst);
      return newConstInt ? ChainUnit(this->binOp, sourceUnit.value, newConstInt, false) : ChainUnit::blank();
    }
    default: 
      return ChainUnit::blank();
  }
}

ConstantMergePass::ChainUnit ConstantMergePass::ChainUnit::createChainUnit(Instruction *inst) {
  if (!inst->getType()->isIntegerTy()) return ChainUnit::blank();

  BinaryOperator *binOperator = dyn_cast<BinaryOperator>(inst);

  if (!binOperator) return ChainUnit::blank();

  BinaryOperator::BinaryOps binOp = binOperator->getOpcode();
  Value *val;
  ConstantInt *con;

  switch (inst->getOpcode()) {
    case Instruction::Mul:
    case Instruction::Add:
    case Instruction::Sub: {
      if (match(inst, m_BinOp(m_Value(val), m_ConstantInt(con)))) return {binOp, val, con, false};
      if (match(inst, m_BinOp(m_ConstantInt(con), m_Value(val)))) return {binOp, val, con, true};
    }
    case Instruction::Shl:
    case Instruction::AShr:
    case Instruction::LShr:
      if (match(inst, m_BinOp(m_Value(val), m_ConstantInt(con)))) return {binOp, val, con, false};
    default:
      return ChainUnit::blank();
  }
  
  return ChainUnit::blank();
}

bool ConstantMergePass::tryMergeConstant(Instruction *inst, BinaryOperator::BinaryOps ops) {
  bool isChanged = false;

  logs() << "Target : " << *inst << "\n";

  ChainUnit chainUnit = ChainUnit::createChainUnit(inst);
  if (!chainUnit.isValid()) return isChanged;

  logs() << chainUnit << "\n";

  SmallVector<User*> userVector(inst->user_begin(), inst->user_end());

  for (User *user : userVector) {
    Instruction *userInst = dyn_cast<Instruction>(user);
    if (!userInst) continue;

    if (userInst->getOpcode() != ops) continue;

    ChainUnit nextChainUnit = ChainUnit::createChainUnit(userInst);
    logs() << "next : " << nextChainUnit << "\n";
    if (!nextChainUnit.isValid()) continue;

    ChainUnit newChainUnit = nextChainUnit.merge(chainUnit);
    logs() << "new : " << newChainUnit << "\n";
    if (!newChainUnit.isValid()) continue;

    Instruction *newInst = newChainUnit.createInstruction(userInst);
    ReplaceInstWithInst(userInst, newInst);
    logs() << "new inst : " << *newInst << "\n";

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

  for (BinaryOperator::BinaryOps ops : ChainUnit::getAvailableOps())
    isChanged |= this->MergeConstant(F, ops);

  logs() << "[ConstantMergePass] @" << F.getName() << " END\n";

  return isChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
