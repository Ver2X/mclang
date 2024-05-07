#pragma once
#include "ir/BasicBlock.h"
#include "ir/Instruction.h"

class LoopInfo {
public:
  BasicBlockPtr Guard;

  BasicBlockPtr Header;
  BasicBlockPtr PreHeader;
  BasicBlockPtr Batch;

  BasicBlockPtr Exit;
  BasicBlockPtr Exiting;
};