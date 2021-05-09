#include "Team4Header.h"

PreservedAnalyses ArithmeticPass::run(Function &F, FunctionAnalysisManager &FAM) {

  Value* FirstVal; //FirstOperand Value
  ConstantInt* ConsVal; //Constant Value

  vector<Instruction *> InstRemove;// Inst to Remove

  for(BasicBlock &BB: F){
     
    //vectors for store change instructions.
    vector<Instruction *> InstAdd,InstShl,InstLShr,InstAShr,InstAnd,InstAndMx;

    for(Instruction &I : BB){
            
      //Case one: add X X -> mul X 2
      if(match(&I,m_Add(m_Value(FirstVal),m_Deferred(FirstVal)))){
        InstAdd.push_back(&I);
      }

      //Case two: shl X C -> mul X (2^C)
      if(match(&I, m_Shl(m_Value(FirstVal),m_ConstantInt(ConsVal)))){
        InstShl.push_back(&I);
      }

      //Case three: lshr X C -> Udiv X (2^C)
      if(match(&I, m_LShr(m_Value(FirstVal),m_ConstantInt(ConsVal)))){
        InstLShr.push_back(&I);
      }

      //Case four: ashr X C -> Sdiv X (2^C)
      if(match(&I, m_AShr(m_Value(FirstVal),m_ConstantInt(ConsVal)))){
        InstAShr.push_back(&I);
      }

      //Case five: and X (2^C-1) -> udiv X (2^C)
      if(match(&I, m_And(m_Value(FirstVal), m_ConstantInt(ConsVal)))){
        uint64_t cons = ConsVal->getZExtValue(),bi = 0;

        bool chk = false;

        //check cons is 2^C-1 form.
        for(unsigned i=0;i<63;i++){
          bi = (bi<<1)|1;
          if(bi==cons){
            chk = true;
            break;
          }
        }
                
        if(chk){
          InstAnd.push_back(&I);
        } else if(((bi<<1)|1)==cons){//if cons is unsigned max, push it InstAndMx
          InstAndMx.push_back(&I);
        }
      }
    }

    //Change Instruction to mul x 2
    for(Instruction *AddI : InstAdd){
      Value* FirstOperand = AddI->getOperand(0);
      Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul,FirstOperand,ConstantInt::get(FirstOperand->getType(),2));
      ReplaceInstWithInst(AddI,NewInst);
    }

    //Change Instruction to mul x (1<<c)
    for(Instruction *ShlI : InstShl){
      Value* FirstOperand = ShlI->getOperand(0);
      ConstantInt* ShlVal = dyn_cast<ConstantInt> (ShlI->getOperand(1));
      uint32_t ushlval = ShlVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul,FirstOperand,ConstantInt::get(FirstOperand->getType(),(1ull<<ushlval)));
      ReplaceInstWithInst(ShlI,NewInst);
    }

    //Change Instruction to udiv x (1<<c)
    for(Instruction *LShrI : InstLShr){
      Value* FirstOperand = LShrI->getOperand(0);
      ConstantInt* LShrVal = dyn_cast<ConstantInt> (LShrI->getOperand(1));
      uint32_t ushrval = LShrVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::UDiv,FirstOperand,ConstantInt::get(FirstOperand->getType(),(1ull<<ushrval)));
      ReplaceInstWithInst(LShrI,NewInst);
    }

    //Change Instruction to sdiv x (1<<c)
    for(Instruction *AShrI : InstAShr){
      Value* FirstOperand = AShrI->getOperand(0);
      ConstantInt* AShrVal = dyn_cast<ConstantInt> (AShrI->getOperand(1));
      uint32_t ashrval = AShrVal->getZExtValue();
      Instruction*  NewInst = BinaryOperator::Create(Instruction::SDiv,FirstOperand,ConstantInt::get(FirstOperand->getType(),(1ull<<ashrval)));
      ReplaceInstWithInst(AShrI,NewInst);
    }

    //Change Instruction to urem x (1<<c)
    for(Instruction *AndI : InstAnd){
      Value* FirstOperand = AndI->getOperand(0);
      ConstantInt* AndVal = dyn_cast<ConstantInt> (AndI->getOperand(1));
      uint64_t andval = AndVal->getZExtValue();
      Instruction* NewInst = BinaryOperator::Create(Instruction::URem,FirstOperand,ConstantInt::get(FirstOperand->getType(),(andval+1)));
      ReplaceInstWithInst(AndI,NewInst);
    }

    //Change Instruction to FirstOperand
    for(Instruction *AndMxI : InstAndMx){
      Value* FirstOperand = AndMxI->getOperand(0);
      AndMxI->replaceAllUsesWith(FirstOperand);
      InstRemove.push_back(AndMxI);
    }

  }
  
  //Remove Instructions from Parent.
  for(Instruction *RemoveI : InstRemove){
    RemoveI->removeFromParent();
  }

  return PreservedAnalyses::all();
}
