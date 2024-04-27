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
  for (auto [_, v] : CompoundTypeTags) {
    s += v->CodeGen();
  }
  return s;
}

void Module::CreateType(TypePtr Ty, std::string N) {
  CompoundTypeTags[Ty] = std::make_shared<StructTypeTag>(Ty, N);
}