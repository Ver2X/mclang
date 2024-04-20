#include "Block.h"
#include "FunctionIR.h"
#include "LoopInfo.h"

class Pass {
public:
  virtual bool runOnFunction();
  virtual bool runOnBasicBlock();
  virtual bool runOnLoop();
  virtual bool runOnModule();

  static int ID;
  std::string PassName;
};