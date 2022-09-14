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


enum class IROpKind
{
	Op_ADD,  		// +
	Op_SUB,  		// -
	Op_MUL,  		// *
	Op_DIV,  		// /
	Op_EQ, 	 		// ==
	Op_NE,   		// !=
	Op_LT,   		// <
	Op_LE,   		// <=
	Op_COMMA,		// ,
	Op_NEG,         // -, unary
	Op_ADDR,		// &, unary
	Op_DEREF,		// *, unary
	Op_Branch,  // branch
	Op_UnConBranch,  // uncondition branch
	Op_FUNCALL,	// function call
	Op_Alloca,  // allcoa
	Op_Store,   // store
	Op_Cmp,     // icmp
	Op_Load,    // load
	Op_Return,  // return
	Op_MEMBER,	// . (struct member access)
};

class Instruction{

	IROpKind Op;
	VariablePtr left;
	VariablePtr right;
	VariablePtr result;
	int getAlign(VariablePtr left, VariablePtr right, VariablePtr result);

public:
	int Ival;
	Instruction(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op)
	{
		this->Op = Op;
		this->left = left;
		this->right = right;
		// result may null ("ret")
		// result->align = getAlign(left, right, result);
		this->result = result;

	}
	IROpKind GetOp() { return Op; }
	std::string CodeGen();
 };
using InstructionPtr = std::shared_ptr<Instruction>;