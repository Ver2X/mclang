#pragma once
#include "ir/FunctionIR.h"
#include "ir/StructTypeTag.h"
#include "utils/Twine.h"
#include "ir/Value.h"

#include <vector>

class Module {
public:
  Module() {}
  void insertGlobalVariable(VariablePtr global);
  std::string GlobalVariableCodeGen();
  std::string getName() { return Name; }
  void setName(std::string _Name) { Name = _Name; }
  void insertFunction(IRFunctionPtr Func) { Functions.push_back(Func); }
  std::vector<IRFunctionPtr> getFunctions() { return Functions; }
  void CreateType(TypePtr Ty, std::string N);
  StructTypeTagPtr GetType(TypePtr Ty) { return CompoundTypeTags[Ty]; }

  std::vector<VariablePtr> globalVariables;

private:
  std::vector<IRFunctionPtr> Functions;
  std::map<TypePtr, StructTypeTagPtr> CompoundTypeTags;
  std::string Name;
};

using ModulePtr = std::shared_ptr<Module>;