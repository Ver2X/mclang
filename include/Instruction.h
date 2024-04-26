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

enum class IROpKind {
  Op_ADD,         // +
  Op_SUB,         // -
  Op_MUL,         // *
  Op_DIV,         // /
  Op_EQ,          // ==
  Op_NE,          // !=
  Op_SLT,         // <
  Op_SLE,         // <=
  Op_SGT,         // >
  Op_SGE,         // >=
  Op_COMMA,       // ,
  Op_NEG,         // -, unary
  Op_ADDR,        // &, unary
  Op_DEREF,       // *, unary
  Op_Branch,      // branch
  Op_UnConBranch, // uncondition branch
  Op_FUNCALL,     // function call
  Op_Alloca,      // allcoa
  Op_Store,       // store
  // Op_Cmp,    	// icmp
  Op_Load,     // load
  Op_Return,   // return
  Op_MEMBER,   // . (struct member access)
  Op_RESERVED, // reserve for error
};

class Instruction;

class Instruction {
protected:
  IROpKind Op;
  // VariablePtr Left;
  // VariablePtr Right;
  // VariablePtr Result;
  int getAlign(VariablePtr Left, VariablePtr Right, VariablePtr Result);
  friend class BinaryOperator;

public:
  int Ival;
  Instruction(int _Ival) : Ival(_Ival) {}
  Instruction() : Ival(INT_MAX) {}
  IROpKind GetOp() { return Op; }
  virtual std::string CodeGen() = 0;
};
using InstructionPtr = std::shared_ptr<Instruction>;
class BasicBlock;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;

class BinaryOperator : public Instruction {
public:
  BinaryOperator(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                 IROpKind Op)
      : Left(Left), Right(Right), Result(Result) {
    this->Op = Op;
  }
  std::string CodeGen();

private:
  VariablePtr Left;
  VariablePtr Right;
  VariablePtr Result;
};

class UnaryOperator : public Instruction {
public:
  UnaryOperator(VariablePtr l, VariablePtr r) : Left(l), Result(r) {}
  std::string CodeGen();

private:
  VariablePtr Left;
  VariablePtr Result;
};

class ReturnInst : public Instruction {
public:
  ReturnInst(VariablePtr rv) : returnValue(rv) {}
  ReturnInst() {}
  std::string CodeGen();

private:
  VariablePtr returnValue;
};

class BranchInst : public Instruction {
public:
  BranchInst(VariablePtr iV, BasicBlockPtr tg1, BasicBlockPtr tg2, IROpKind Op)
      : indicateVariable(iV), targetFirst(tg1), targetSecond(tg2) {
    this->Op = Op;
  }
  BranchInst(BasicBlockPtr tg1, IROpKind Op) : targetFirst(tg1) {
    this->Op = Op;
  }
  std::string CodeGen();

private:
  VariablePtr indicateVariable;
  BasicBlockPtr targetFirst;
  BasicBlockPtr targetSecond;
};

class AllocaInst : public Instruction {
public:
  AllocaInst(VariablePtr dest) : dest(dest) {}
  std::string CodeGen();

private:
  VariablePtr dest;
};

class LoadInst : public Instruction {
public:
  LoadInst(VariablePtr source, VariablePtr dest) : source(source), dest(dest) {}
  std::string CodeGen();

private:
  VariablePtr source;
  VariablePtr dest;
};

class StoreInst : public Instruction {
public:
  StoreInst(VariablePtr source, VariablePtr dest)
      : source(source), dest(dest) {}
  std::string CodeGen();

private:
  VariablePtr source;
  VariablePtr dest;
};

class CallInst : public Instruction {
public:
  CallInst(IRFunctionPtr func, std::vector<VariablePtr> args, VariablePtr dest)
      : func(func), args(args), dest(dest) {}
  std::string CodeGen();

private:
  IRFunctionPtr func;
  std::vector<VariablePtr> args;
  VariablePtr dest;
};

class GetElementPtrInst : Instruction {};

class ICmpInst : Instruction {};

class PHINode : Instruction {};

class TruncInst : Instruction {};

class ZExtInst : Instruction {};

class SExtInst : Instruction {};

#define RPINT_VALUE                                                            \
  {                                                                            \
    if (Op != IROpKind::Op_Alloca && Op != IROpKind::Op_Store &&               \
        Op != IROpKind::Op_Load)                                               \
      s += ";     Left:" + std::to_string(Left->Ival) +                        \
           " Right:" + std::to_string(Right->Ival) +                           \
           " Result:" + std::to_string(Result->Ival) + "\n";                   \
  }
