#pragma once
#include "FunctionIR.h"
#include "Variable.h"
#include "Twine.h"

#include <vector>

class Module {
public:
  Module() {}
  void InsertGlobalVariable(VariablePtr global);
  std::string GlobalVariableCodeGen();
  std::string GetName() { return Name;}
  void SetName(std::string _Name) { Name = _Name; }
  std::vector<VariablePtr> globalVariables;

private:
  std::vector<IRFunctionPtr> Functions;
  std::string Name;
};