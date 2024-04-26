#pragma once
#include "FunctionIR.h"
#include "Variable.h"
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
class SymbolTable;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;
typedef struct Obj Obj;

class SymbolTable {
  SymbolTablePtr global_table;

  std::map<Obj *, VariablePtr> Table;
  std::vector<IRFunctionPtr> funcs;

public:
  SymbolTable() : global_table(nullptr) {}
  SymbolTable(SymbolTablePtr fa) : global_table(fa) {}
  bool nameHaveUsed(std::string name);
  bool insertFunc(IRFunctionPtr func);
  bool insert(Obj *node, VariablePtr var);
  bool update(Obj *node, VariablePtr var, VariablePtr newVar);
  bool erase(Obj *node, VariablePtr var);
  VariablePtr findVar(Obj *node);
  IRFunctionPtr findFunc(std::string funcname);
};