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
class IRBuilder;
class IRFunction {

  std::shared_ptr<IRBuilder> body;

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
  void AddArgs();
  std::string CodeGen();
  void setBody(std::shared_ptr<IRBuilder> body);
};
using IRFunctionPtr = std::shared_ptr<IRFunction>;