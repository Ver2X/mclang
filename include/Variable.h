#pragma once
#include "Type.h"
#include "Use.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// enum class ReturnTypePtr {
//   RTY_INT,
//   RTY_SHORT,
//   RTY_LONG,
//   RTY_PTR,
//   RTY_ARRAY,
//   RTY_CHAR,
//   RTY_STRUCT,
//   RTY_UNION,
//   RTY_VOID,
// };

// enum class VariablePtr {
//   VAR_8,
//   VAR_16,
//   VAR_32,
//   VAR_64,
//   VAR_PRT,
//   VAR_Void,
//   Var_Array,
//   VAR_Undefined = VAR_32,
// };

using VarTypePtr = TypePtr;
using ReturnTypePtr = TypePtr;

class Value;
using Variable = Value;
using VariablePtr = std::shared_ptr<Variable>;
using VarList = std::shared_ptr<std::vector<VariablePtr>>;
class Instruction;
using InstructionPtr = std::shared_ptr<Instruction>;
class Use;
using UsePtr = std::shared_ptr<Use>;
class Value {
private:
  std::string Name;
  std::list<Use *> UseList;

public:
  int Ival;
  double Fval;
  int *Pval;
  VarTypePtr VarType;
  int Align;
  Value *Next;
  bool isConst;
  bool isInitConst;
  bool isGlobal;
  bool varIsArg;
  VariablePtr Addr;
  Value();
  Value(int64_t v);
  Value(int v);
  Value(double v);
  Value(int *v);

  /// This method should only be used by the Use class.
  void addUse(Use *U) { U->addToList(UseList); }

  std::list<Use *> users() { return UseList; }
  bool use_empty() { return UseList.empty(); }

  virtual void replaceAllUsesWith(VariablePtr V) {
    for (auto U : this->users()) {
      U->set(V);
    }
  }

  std::string &getName() { return Name; }
  void setName(std::string InitName);
  void setType(VarTypePtr Ty) { VarType = Ty; }
  VarTypePtr getType() { return VarType; }
  void SetConst(double v);
  void SetConst(int v);
  void SetConst(int64_t v);
  void setGlobal();
  void SetArg();
  void SetAddr(VariablePtr _Addr);
  VariablePtr getAddr();
  bool isArg();

  std::string CodeGen();
};
