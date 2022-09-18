#pragma once
#include "FunctionIR.h"
#include "Variable.h"
#include "FunctionIR.h"

#include <vector>


class Module
{
public:
    Module() {}
    void InsertGlobalVariable(VariablePtr global);
    std::string GlobalVariableCodeGen();

    std::vector<VariablePtr>  globalVariables;
private:
    std::vector<IRFunctionPtr> Functions;
};