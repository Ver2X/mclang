#pragma once
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

// do include "IRBuilder.h" incase cycle include
class SymbolTable;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;
class IRBuilder;
class IRFunction {
  int varNameNum;
  int blockLabelNum;
  int controlFlowNum;
  std::shared_ptr<IRBuilder> body;
  SymbolTablePtr table;

public:
  IRFunction();
  IRFunction(std::string name);

  std::string functionName;
  // VariablePtr args;
  std::vector<VariablePtr> args;
  int argsNum;
  VariablePtr ret;
  ReturnTypeKind retTy;

  std::string rename();
  std::string GetName() { return functionName; };
  SymbolTablePtr GeTable() { return table; };
  int NextVarNameNum();
  int NextBlockLabelNum();
  int NextControlFlowNum();
  void AddArgs();
  std::string CodeGen();
  void setBody(std::shared_ptr<IRBuilder> body);
  void setTable(SymbolTablePtr local_table) { table = local_table; };
};
using IRFunctionPtr = std::shared_ptr<IRFunction>;