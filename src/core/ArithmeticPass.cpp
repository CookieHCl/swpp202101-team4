#include "ArithmeticPass.h"

PreservedAnalyses ArithmeticPass::run(Function &F, FunctionAnalysisManager &FAM) {

  Value* FirstVal; //FirstOperand Value
  ConstantInt* ConsVal; //Constant Value

  for(BasicBlock &BB: F){

    //vectors for store change instructions.
    vector<Instruction *> InstAdd, InstShl, InstLShr, InstAShr, InstAnd;

    for(auto itr = BB.begin(), en = BB.end(); itr!=en;){

      auto tmp_itr = itr++;
      Instruction &I = *tmp_itr;

      //Case one: add X X -> mul X 2
      if(match(&I, m_Add(m_Value(FirstVal), m_Deferred(FirstVal)))){
        InstAdd.push_back(&I);
      }

      //Case two: shl X C -> mul X (2^C)
      if(match(&I, m_Shl(m_Value(FirstVal), m_ConstantInt(ConsVal)))){
        InstShl.push_back(&I);
      }

      //Case three: lshr X C -> Udiv X (2^C)
      if(match(&I, m_LShr(m_Value(FirstVal), m_ConstantInt(ConsVal)))){
        InstLShr.push_back(&I);
      }

      //Case four: ashr X C -> Sdiv X (2^C)
      if(match(&I, m_AShr(m_Value(FirstVal), m_ConstantInt(ConsVal)))){
        InstAShr.push_back(&I);
      }

      //Case five: and X (2^C-1) -> udiv X (2^C)
      if(match(&I, m_And(m_Value(FirstVal), m_ConstantInt(ConsVal))) || match(&I, m_And(m_ConstantInt(ConsVal), m_Value(FirstVal)))){

        uint64_t cons = ConsVal->getZExtValue(), width = FirstVal->getType()->getIntegerBitWidth();

        if(cons==UINT64_MAX){
          ReplaceInstWithValue(BB.getInstList(),tmp_itr,FirstVal);
        } else if(width==64){
          InstAnd.push_back(&I);
        } else if((cons+1) == (1ull<<width)){
          ReplaceInstWithValue(BB.getInstList(),tmp_itr,FirstVal);
        } else if (cons && (! (cons & (cons+1)))) {
          InstAnd.push_back(&I);
        }
      }

    }

    //Change Instruction to mul x 2
    for(Instruction *AddI : InstAdd){
      Value* FirstOperand = AddI->getOperand(0);
      Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul, FirstOperand, ConstantInt::get(FirstOperand->getType(),2));
      ReplaceInstWithInst(AddI, NewInst);
    }

    //Change Instruction to mul x (1<<c)
    for(Instruction *ShlI : InstShl){
      Value* FirstOperand = ShlI->getOperand(0);
      ConstantInt* ShlVal = dyn_cast<ConstantInt> (ShlI->getOperand(1));
      uint32_t ushlval = ShlVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul, FirstOperand, ConstantInt::get(FirstOperand->getType(),(1ull<<ushlval)));
      ReplaceInstWithInst(ShlI, NewInst);
    }

    //Change Instruction to udiv x (1<<c)
    for(Instruction *LShrI : InstLShr){
      Value* FirstOperand = LShrI->getOperand(0);
      ConstantInt* LShrVal = dyn_cast<ConstantInt> (LShrI->getOperand(1));
      uint32_t ushrval = LShrVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::UDiv, FirstOperand,ConstantInt::get(FirstOperand->getType(),(1ull<<ushrval)));
      ReplaceInstWithInst(LShrI, NewInst);
    }

    //Change Instruction to sdiv x (1<<c)
    for(Instruction *AShrI : InstAShr){
      Value* FirstOperand = AShrI->getOperand(0);
      ConstantInt* AShrVal = dyn_cast<ConstantInt> (AShrI->getOperand(1));
      uint32_t ashrval = AShrVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::SDiv, FirstOperand, ConstantInt::get(FirstOperand->getType(),(1ull<<ashrval)));
      ReplaceInstWithInst(AShrI, NewInst);
    }

    //Change Instruction to urem x (1<<c)
    for(Instruction *AndI : InstAnd){
      Value* FirstOperand = AndI->getOperand(0);
      Value* SecondOperand = AndI->getOperand(1);
      Value* XOperand;
      ConstantInt* AndVal = dyn_cast<ConstantInt> (FirstOperand);
      if(AndVal==NULL){
        AndVal = dyn_cast<ConstantInt> (SecondOperand);
        XOperand = FirstOperand;
      } else {
        XOperand = SecondOperand;
      }
      uint64_t andval = AndVal->getZExtValue();
      Instruction* NewInst = BinaryOperator::Create(Instruction::URem, XOperand, ConstantInt::get(XOperand->getType(),(andval+1)));
      ReplaceInstWithInst(AndI, NewInst);
    }
  }

  return PreservedAnalyses::none();
}
