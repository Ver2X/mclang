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
class SymbolTable;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;

class SymbolTable {
  // std::string name;
  // VariablePtr variables;
  int level;

  // SymbolTablePtr symb_list;
  SymbolTablePtr prev_level;
  SymbolTablePtr next_level;

  std::map<std::string, VarList> table; // global
public:
  SymbolTable() {
    prev_level = nullptr;
    next_level = nullptr;
  }

  SymbolTable(SymbolTablePtr fa) {
    // symb_list = std::make_shared<SymbolTable>();
    prev_level = fa;
    // fa->next_level = this; ???
  }

  bool insert(VariablePtr var, int level);
  bool insert(VariablePtr var, VariablePtr newVar, int level);
  // use a cache save inserted varibale, when leaving function, delete
  // it from symbol table
  void erase(std::string var_name, int level);
  // bool findVar(std::string &var_name, VariablePtr &);
  bool findVar(std::string var_name, VariablePtr &);
};