#include "FunctionIR.h"
#include "Block.h"
#include "LoopInfo.h"

class Pass
{
public:
    virtual bool runOnFunction();
    virtual bool runOnBasicBlock();
    virtual bool runOnLoop();
    virtual bool runOnModule();

    static int ID;
    std::string PassName;
};