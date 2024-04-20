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
};

class Operand {
private:
  std::string name;

public:
  std::string &GetName() { return name; }
  void SetName(std::string name) { this->name = name; }
  // void SetName(std::string & name) { this->name = name; }
  //  SSA?
  int Ival;
  double Fval;
  VaribleKind type;

  int align;
  Operand *next;
  bool isConst;
  bool isInitConst;
  bool isGlobal;

  Operand();
  Operand(int64_t v);
  Operand(int v);
  Operand(double v);

  void SetConst(double v);
  void SetConst(int v);
  void SetConst(int64_t v);
  void SetGlobal();

  std::string CodeGen();
};

using Variable = Operand;
using VariablePtr = std::shared_ptr<Variable>;
using VarList = std::shared_ptr<std::vector<VariablePtr>>;