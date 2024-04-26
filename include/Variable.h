#pragma once
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
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
#pragma once
enum class ReturnTypeKind {
  RTY_INT,
  RTY_SHORT,
  RTY_LONG,
  RTY_PTR,
  RTY_ARRAY,
  RTY_CHAR,
  RTY_STRUCT,
  RTY_UNION,
  RTY_VOID,
};

enum class VaribleKind {
  VAR_8,
  VAR_16,
  VAR_32,
  VAR_64,
  VAR_PRT,
  VAR_Void,
  VAR_Undefined = VAR_32,
};

class Operand;
using Variable = Operand;
using VariablePtr = std::shared_ptr<Variable>;
using VarList = std::shared_ptr<std::vector<VariablePtr>>;

class Operand {
private:
  std::string Name;

public:
  // void setName(std::string & Name) { this->Name = Name; }
  //  SSA?
  int Ival;
  double Fval;
  VaribleKind type;

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

  std::string &getName() { return Name; }
  void setName(std::string Name) { this->Name = Name; }

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
