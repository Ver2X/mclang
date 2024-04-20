#include "Block.h"
#include "Instruction.h"

class LoopInfo {
public:
  BlockPtr Guard;

  BlockPtr Header;
  BlockPtr PreHeader;
  BlockPtr Batch;

  BlockPtr Exit;
  BlockPtr Exiting;
};