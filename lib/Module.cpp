#include "Module.h"
#include "IRBuilder.h"
#include "FunctionIR.h"
#include "Variable.h"
#include <string>

void Module::InsertGlobalVariable(VariablePtr global)
{
    globalVariables.push_back(global);
}
std::string Module::GlobalVariableCodeGen()
{
    std::string s;
    for(auto v : globalVariables)
    {
        s += v->CodeGen();
    }
    return s;
}