#ifndef TEAM4HEADER_H
#define TEAM4HEADER_H

// Existing passes
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

// IR passes
#include "ArithmeticPass.h"
#include "BranchPredictPass.h"
#include "FunctionInlinePass.h"
#include "PhierasePass.h"
#include "RemoveUnusedPass.h"

#include "LoopUnrollPass.h"
#include "LoopVectorizePass.h"

#endif
