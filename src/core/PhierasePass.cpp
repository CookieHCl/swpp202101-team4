#include "PhierasePass.h"

PreservedAnalyses PhierasePass::run(Function &F, FunctionAnalysisManager &FAM){

  for(auto B_itr = F.begin(),B_en = F.end(); B_itr!=B_en; ){

    auto tmp_B_itr = B_itr++;
    BasicBlock &BB = (*tmp_B_itr);

    Instruction* Cur_Terminator = BB.getTerminator();

    //now, phinode erase for terminator is ret. because other case( for exampe, loop ), it is not safe to change.
    if((&(*BB.begin()))==BB.getFirstNonPHI()||(!isa<ReturnInst>(Cur_Terminator))){
      continue;
    }

    for(auto pre_itr = pred_begin(&BB), pre_en = pred_end(&BB); pre_itr!=pre_en;){
      auto tmp_pre_itr = pre_itr++;
      BasicBlock* pre_BB = (*tmp_pre_itr);

      ValueToValueMapTy VM;

      BasicBlock* C_BB = CloneBasicBlock(&BB,VM); //cloning basicblock.

      for(auto itr = C_BB->begin(), e = C_BB->end(); itr!=e;){
        auto tmp_itr = itr++;
        Instruction* I = &(*tmp_itr);
        PHINode* Phi = dyn_cast<PHINode> (I);

        if(Phi){
          for(unsigned int i=0; i<Phi->getNumOperands(); ++i){

            BasicBlock* Cur_BB = Phi->getIncomingBlock(i);
            if(Cur_BB==pre_BB){
              Value* CurVal = Phi->getOperand(i);
              ReplaceInstWithValue(C_BB->getInstList(),tmp_itr,CurVal); //replace original instruction to modfied value.
            }
          }
        }
        else {
          for(auto OI = I->op_begin(), OE = I->op_end(); OI != OE; ++OI){
            Value *CurVal = (*OI);
            Value *CurVal2 = VM[CurVal];
            if(CurVal2){
              (*OI) = CurVal2;
            }    
          }
        }
      }

      C_BB->insertInto(&F,&BB);

      Instruction* pre_terminator = pre_BB->getTerminator();

      for(unsigned int i = 0; i<pre_terminator->getNumSuccessors(); ++i){
        BasicBlock* cur_successor = pre_terminator->getSuccessor(i);
        if(cur_successor==(&BB)){//change successor.
          pre_terminator->setSuccessor(i,C_BB);
        }
      }

    }

    BB.eraseFromParent(); //old basicblock should be deleted.
  }

  return PreservedAnalyses::none();
}