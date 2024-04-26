#include "Module.h"
#include "FunctionIR.h"
#include "IRBuilder.h"
#include "Variable.h"
#include <string>

void Module::insertGlobalVariable(VariablePtr global) {
  globalVariables.push_back(global);
}
std::string Module::GlobalVariableCodeGen() {
  std::string s;
  for (auto v : globalVariables) {
    s += v->CodeGen();
  }
  return s;
}