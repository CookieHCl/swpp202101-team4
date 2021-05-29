#include "PhierasePass.h"

PreservedAnalyses PhierasePass::run(Function &F, FunctionAnalysisManager &FAM){
  
  queue<BasicBlock*> que;

  for(auto B_itr = F.begin(),B_en = F.end(); B_itr!=B_en; ) {//initialize que.

    auto tmp_B_itr = B_itr++;
    BasicBlock &BB = (*tmp_B_itr);

    Instruction* Cur_Terminator = BB.getTerminator();

    //now, phinode erase for terminator is ret. because other case( for exampe, loop ), it is not safe to change.
    if((&(*BB.begin())) == BB.getFirstNonPHI()||(!isa<ReturnInst> (Cur_Terminator))) {
      continue;
    }

    que.push(&BB);
  }

  while(!que.empty()){//iterate blocks using queue.

    BasicBlock *Cur_BB = que.front();
    que.pop();

    for(auto pre_itr = pred_begin(Cur_BB), pre_en = pred_end(Cur_BB); pre_itr!=pre_en;) {

      auto tmp_pre_itr = pre_itr++;
      BasicBlock *pre_BB = (*tmp_pre_itr);

      ValueToValueMapTy VM;

      BasicBlock* Cloned_BB = CloneBasicBlock(Cur_BB,VM); //cloning basicblock.

      for(auto itr = Cloned_BB->begin(), e = Cloned_BB->end(); itr!=e;){
        auto tmp_itr = itr++;
        Instruction *I = &(*tmp_itr);
        PHINode *Phi = dyn_cast<PHINode> (I);

        if(Phi) {
          for(unsigned int i=0; i<Phi->getNumOperands(); ++i){

            BasicBlock *Cur_pre_BB = Phi->getIncomingBlock(i);
            if(Cur_pre_BB == pre_BB){
              Value *CurVal = Phi->getOperand(i);
              ReplaceInstWithValue(Cloned_BB->getInstList(), tmp_itr, CurVal); //replace original instruction to modfied value.
            }
          }
        } else {
          for(auto OI = I->op_begin(), OE = I->op_end(); OI != OE; ++OI){
            Value *CurVal = *OI;
            Value *CurVal2 = VM[CurVal];
            if(CurVal2){
              *OI = CurVal2;
            }
          }
        }
      }

      Cloned_BB->insertInto(&F,Cur_BB);

      Instruction *pre_terminator = pre_BB->getTerminator();
      pre_terminator->replaceSuccessorWith(Cur_BB,Cloned_BB);

      if(MergeBlockIntoPredecessor(Cloned_BB) && ((&(*pre_BB->begin())) != pre_BB->getFirstNonPHI())) {
        que.push(pre_BB); //if merged block satisfying condition, insert block into que.
      }

    }

    Cur_BB->eraseFromParent(); //old basicblock should be deleted.

  }


  return PreservedAnalyses::none();
}