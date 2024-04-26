#pragma once
#include "BasicBlock.h"
#include "Instruction.h"

class LoopInfo {
public:
  BasicBlockPtr Guard;

  BasicBlockPtr Header;
  BasicBlockPtr PreHeader;
  BasicBlockPtr Batch;

  BasicBlockPtr Exit;
  BasicBlockPtr Exiting;
};