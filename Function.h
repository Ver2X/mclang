#pragma once
#include "Variable.h"

#include <ctype.h> 
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#include <tuple>
#include <unordered_map>
class IRFunction{
public:
	IRFunction();

	std::string functionName;
	// VariablePtr args;
	std::vector<VariablePtr> args;
	int argsNum;
	VariablePtr ret;
	ReturnTypeKind retTy;

	std::string rename();
	void AddArgs();
	std::string CodeGen();
	
};
using IRFunctionPtr = std::shared_ptr<IRFunction>; 