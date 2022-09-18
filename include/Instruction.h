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
	Op_SLT,   		// <
	Op_SLE,   		// <=
	Op_SGT,   		// >
	Op_SGE,   		// >=
	Op_COMMA,		// ,
	Op_NEG,         // -, unary
	Op_ADDR,		// &, unary
	Op_DEREF,		// *, unary
	Op_Branch,  	// branch
	Op_UnConBranch, // uncondition branch
	Op_FUNCALL,		// function call
	Op_Alloca,  	// allcoa
	Op_Store,   	// store
	// Op_Cmp,    	// icmp
	Op_Load,    	// load
	Op_Return,  	// return
	Op_MEMBER,		// . (struct member access)
	Op_RESERVED,	// reserve for error
};

class Instruction;




class Instruction{
protected:
	IROpKind Op;
	// VariablePtr left;
	// VariablePtr right;
	// VariablePtr result;
	int getAlign(VariablePtr left, VariablePtr right, VariablePtr result);
friend class BinaryOperator;
public:
	int Ival;
	Instruction(int _Ival) : Ival(_Ival) { }
	Instruction() : Ival(0) { }
	IROpKind GetOp() { return Op; }
	virtual std::string CodeGen() = 0;
 };
using InstructionPtr = std::shared_ptr<Instruction>;
class Block;
using BlockPtr = std::shared_ptr<Block>;

class BinaryOperator : public Instruction{
public:
	// https://stackoverflow.com/questions/1847纸上得9295/member-initializer-does-not-name-a-non-static-data-member-or-base-class
	BinaryOperator (VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op) : left(left), right(right), result(result) { 
		this->Op = Op;
	}
	std::string CodeGen();
private:
	VariablePtr left;
	VariablePtr right;
	VariablePtr result;
};


class UnaryOperator : public Instruction{
public:
	UnaryOperator(VariablePtr l, VariablePtr r) : left(l), result(r) {}
	std::string CodeGen();
private:
	VariablePtr left;
	VariablePtr result;
};



class ReturnInst : public Instruction{
public:
	ReturnInst(VariablePtr rv) :returnValue(rv) {}
	ReturnInst() {}
	std::string CodeGen();
private:
	VariablePtr returnValue;
};

class BranchInst : public Instruction{
public:
	BranchInst(VariablePtr iV, BlockPtr tg1, BlockPtr tg2, IROpKind Op) : indicateVariable(iV), targetFirst(tg1), targetSecond(tg2) {
		this->Op = Op;
	}
	BranchInst(BlockPtr tg1, IROpKind Op) : targetFirst(tg1){
		this->Op = Op;
	}
	std::string CodeGen();
private:
	VariablePtr indicateVariable;
	BlockPtr targetFirst;
	BlockPtr targetSecond;
};

class AllocaInst : public Instruction{
public:
	AllocaInst(VariablePtr dest) : dest(dest) {}
	std::string CodeGen();
private:
	VariablePtr dest;
};

class LoadInst : public Instruction{
public:
	LoadInst(VariablePtr source, VariablePtr dest) : source(source), dest(dest) {}
	std::string CodeGen();
private:
	VariablePtr source;
	VariablePtr dest;
};

class StoreInst : public Instruction{
public:
	StoreInst(VariablePtr source, VariablePtr dest) : source(source), dest(dest) {}
	std::string CodeGen();
private:
	VariablePtr source;
	VariablePtr dest;
};

class GetElementPtrInst : Instruction{

};

class ICmpInst : Instruction{

};

class PHINode : Instruction{

};

class TruncInst : Instruction{

};

class ZExtInst : Instruction{

};

class SExtInst : Instruction{

};


#define RPINT_VALUE { if(Op != IROpKind::Op_Alloca && Op !=  IROpKind::Op_Store && Op != IROpKind::Op_Load) s += ";     left:" + std::to_string(left->Ival) + " right:" + std::to_string(right->Ival) + " result:" + std::to_string(result->Ival) + "\n";}
