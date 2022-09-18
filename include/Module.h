#pragma once
#include "FunctionIR.h"
#include "Variable.h"
#include "FunctionIR.h"

#include <vector>


class Module
{
public:
    
private:
    std::vector<VariablePtr>  globalVariables;
    std::vector<IRFunctionPtr> Functions;
};