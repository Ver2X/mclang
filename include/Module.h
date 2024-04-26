#pragma once
#include "FunctionIR.h"
#include "Twine.h"
#include "Variable.h"

#include <vector>

class Module {
public:
  Module() {}
  void insertGlobalVariable(VariablePtr global);
  std::string GlobalVariableCodeGen();
  std::string getName() { return Name; }
  void setName(std::string _Name) { Name = _Name; }
  std::vector<VariablePtr> globalVariables;

private:
  std::vector<IRFunctionPtr> Functions;
  std::string Name;
};