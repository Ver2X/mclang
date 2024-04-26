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
  std::shared_ptr<IRBuilder> Body;
  SymbolTablePtr Table;

public:
  IRFunction();
  IRFunction(std::string Name);

  std::string FunctionName;
  // VariablePtr args;
  std::vector<VariablePtr> args;
  int argsNum;
  VariablePtr ret;
  ReturnTypeKind retTy;

  std::string rename();
  std::string getName() { return FunctionName; };
  SymbolTablePtr GeTable() { return Table; };
  int nextVarNameNum();
  int nextBlockLabelNum();
  int nextControlFlowNum();
  void AddArgs();
  std::string CodeGen();
  void setBody(std::shared_ptr<IRBuilder> Body);
  void setTable(SymbolTablePtr local_table) { Table = local_table; };
};
using IRFunctionPtr = std::shared_ptr<IRFunction>;