#pragma once
#include "Type.h"
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

class Operand;
using Variable = Operand;
using VariablePtr = std::shared_ptr<Variable>;
using VarList = std::shared_ptr<std::vector<VariablePtr>>;
class Instruction;
using InstructionPtr = std::shared_ptr<Instruction>;
class Operand {
private:
  std::string Name;

public:
  // void setName(std::string & Name) { this->Name = Name; }
  //  SSA?
  int Ival;
  double Fval;
  int *Pval;
  VarTypePtr VarType;
  InstructionPtr Use;
  std::list<InstructionPtr> User;
  int Align;
  Operand *Next;
  bool isConst;
  bool isInitConst;
  bool isGlobal;
  bool varIsArg;
  VariablePtr Addr;
  Operand();
  Operand(int64_t v);
  Operand(int v);
  Operand(double v);
  Operand(int *v);

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
