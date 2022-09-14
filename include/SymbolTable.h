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
class SymbolTable;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;

class SymbolTable{
public:
	// std::string name;
	// VariablePtr variables;
	int level;
	//SymbolTable * symb_list;
	//SymbolTable * symb_list_back_level;
	
	SymbolTablePtr symb_list;
	SymbolTablePtr symb_list_back_level;
	
	std::map<std::string,  VarList > table; // global

	SymbolTable()
	{

	}

	SymbolTable(SymbolTablePtr fa)
	{
		symb_list_back_level = fa;
		symb_list = std::make_shared<SymbolTable>();
	}

	bool insert(VariablePtr var,int level);
	bool insert(VariablePtr var,VariablePtr newVar, int level);
	// use a cache save inserted varibale, when leaving function, delete
	// it from symbol table
	void erase(std::string var_name,int level);
	bool findVar(std::string & var_name, VariablePtr &);
};