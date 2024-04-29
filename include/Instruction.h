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
  Op_PhiNode,     // phi node
  // Op_Cmp,    	// icmp
  Op_Load,          // load
  Op_GetElementPtr, // gep
  Op_Return,        // return
  Op_MEMBER,        // . (struct member access)
  Op_RESERVED,      // reserve for error
};

class Instruction;
// Other class public inheritance "Instruction" , so than auto
// cover from "std::shared_ptr<xxx>"" to "std::shared_ptr<Instruction>""
class Instruction : public std::enable_shared_from_this<Instruction> {
protected:
  IROpKind Op;
  // VariablePtr Left;
  // VariablePtr Right;
  // VariablePtr Result;
  BasicBlockPtr Parent;
  int getAlign(VariablePtr Left, VariablePtr Right, VariablePtr Result);
  friend class BinaryOperator;

public:
  int Ival;
  Instruction(BasicBlockPtr P, int _Ival) : Parent(P), Ival(_Ival) {}
  Instruction(BasicBlockPtr P) : Parent(P), Ival(INT_MAX) {}
  IROpKind GetOp() { return Op; }
  BasicBlockPtr getParent() { return Parent; }
  virtual VariablePtr getResult() = 0;
  void setOperand(int Idx, VariablePtr NewOp) { getOperand(Idx) = NewOp; };
  virtual VariablePtr &getOperand(int Idx) = 0;
  virtual unsigned getNumOperands() = 0;
  void eraseFromParent();
  virtual std::string CodeGen() = 0;
  bool isTerminator(IROpKind Op) {
    return Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch ||
           Op == IROpKind::Op_Return;
  }
  bool isTerminator() { return isTerminator(GetOp()); }

  void replaceAllUsesWith(VariablePtr V) {
    auto Res = this->getResult();
    for (auto Inst : Res->User) {
      for (unsigned Index = 0; Index < Inst->getNumOperands(); ++Index) {
        if (Inst->getOperand(Index) == Res) {
          Inst->setOperand(Index, V);
        }
      }
    }
  }
};
using InstructionPtr = std::shared_ptr<Instruction>;
class BasicBlock;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;

class BinaryOperator : public Instruction {
public:
  BinaryOperator(BasicBlockPtr BB, VariablePtr Left, VariablePtr Right,
                 VariablePtr Result, IROpKind Op)
      : Instruction(BB), Left(Left), Right(Right), Result(Result) {
    this->Op = Op;
  }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    if (Idx == 0) {
      return Left;
    } else {
      assert(Idx == 1);
      return Right;
    }
  }
  VariablePtr getResult() { return Result; }
  unsigned getNumOperands() { return 2; }

private:
  VariablePtr Left;
  VariablePtr Right;
  VariablePtr Result;
};

// class UnaryOperator : public Instruction {
// public:
//   UnaryOperator(BasicBlockPtr BB, VariablePtr l, VariablePtr r) :
//   Instruction(BB), Left(l), Result(r) {} std::string CodeGen(); unsigned
//   getNumOperands() { return 1;}

// private:
//   VariablePtr Left;
//   VariablePtr Result;
// };

class ReturnInst : public Instruction {
public:
  ReturnInst(BasicBlockPtr BB, VariablePtr rv)
      : Instruction(BB), returnValue(rv) {
    Op = IROpKind::Op_Return;
  }
  ReturnInst(BasicBlockPtr BB) : Instruction(BB) { Op = IROpKind::Op_Return; }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    assert(Idx == 0);
    return returnValue;
  }
  unsigned getNumOperands() { return 1; }
  VariablePtr getResult() { return nullptr; }

private:
  VariablePtr returnValue;
};

class BranchInst : public Instruction {
public:
  BranchInst(BasicBlockPtr BB, VariablePtr iV, BasicBlockPtr tg1,
             BasicBlockPtr tg2, IROpKind Op)
      : Instruction(BB), indicateVariable(iV), targetFirst(tg1),
        targetSecond(tg2) {
    this->Op = Op;
  }
  BranchInst(BasicBlockPtr BB, BasicBlockPtr tg1, IROpKind Op)
      : Instruction(BB), targetFirst(tg1) {
    this->Op = Op;
  }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    if (Idx == 0) {
      return indicateVariable;
    }
    // TODO: make VariablePtr -> ReturnValuePtr
    // and make ReturnValuePtr could cast to VariablePtr
    // and BlockPtr
    assert(false);
  }
  unsigned getNumOperands() { return 1; }
  VariablePtr getResult() { return nullptr; }

private:
  VariablePtr indicateVariable;
  BasicBlockPtr targetFirst;
  BasicBlockPtr targetSecond;
};

class AllocaInst : public Instruction {
public:
  AllocaInst(BasicBlockPtr BB, VarTypePtr VTy, VariablePtr Dest,
             VariablePtr ArraySize = nullptr)
      : Instruction(BB), VTy(VTy), Dest(Dest), ArraySize(ArraySize) {
    Op = IROpKind::Op_Alloca;
  }
  std::string CodeGen();
  VariablePtr getResult() { return Dest; }
  VarTypePtr getAllocatedType() { return VTy; }
  VariablePtr &getOperand(int Idx) {
    assert(false);
    return Dest;
  }
  unsigned getNumOperands() { return 0; }

private:
  VariablePtr ArraySize;
  VarTypePtr VTy;
  VariablePtr Dest;
};
using AllocaInstPtr = std::shared_ptr<AllocaInst>;

class LoadInst : public Instruction {
public:
  LoadInst(BasicBlockPtr BB, VariablePtr Source, VariablePtr Dest)
      : Instruction(BB), Source(Source), Dest(Dest) {
    Op = IROpKind::Op_Load;
  }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    assert(Idx == 0);
    return Source;
  }
  VariablePtr getResult() { return Dest; }
  unsigned getNumOperands() { return 1; }
  VariablePtr getPointerOperand() { return Source; }

private:
  VariablePtr Source;
  VariablePtr Dest;
};

class GetElementPtrInst : public Instruction {
public:
  GetElementPtrInst(BasicBlockPtr BB, VarTypePtr PtrTy, VariablePtr BasePtr,
                    std::vector<VariablePtr> IdxList, VariablePtr Result)
      : Instruction(BB), PtrTy(PtrTy), BasePtr(BasePtr), IdxList(IdxList),
        Result(Result) {
    Op = IROpKind::Op_GetElementPtr;
  }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    assert(Idx == 0);
    if (Idx == 0) {
      return BasePtr;
    } else {
      assert(Idx - 1 >= 0 && Idx - 1 < IdxList.size());
      return IdxList[Idx - 1];
    }
  }
  VariablePtr getIndex(int Idx) {
    assert(Idx >= 0 && Idx < IdxList.size());
    return IdxList[Idx];
  }
  std::vector<VariablePtr> getIndexs() { return IdxList; }
  unsigned getNumOperands() { return 1 + IdxList.size(); }
  VariablePtr getResult() { return Result; }

private:
  VarTypePtr PtrTy;
  VariablePtr BasePtr;
  std::vector<VariablePtr> IdxList;
  VariablePtr Result;
};

class StoreInst : public Instruction {
public:
  StoreInst(BasicBlockPtr BB, VariablePtr Source, VariablePtr Dest)
      : Instruction(BB), Source(Source), Dest(Dest) {
    Op = IROpKind::Op_Store;
  }
  std::string CodeGen();
  VariablePtr &getOperand(int Idx) {
    if (Idx == 0) {
      return Source;
    } else if (Idx == 1) {
      return Dest;
    }
    assert(false);
  }
  unsigned getNumOperands() { return 2; }
  VariablePtr getResult() { return nullptr; }
  VariablePtr getPointerOperand() { return Dest; }

private:
  VariablePtr Source;
  VariablePtr Dest;
};

class CallInst : public Instruction {
public:
  CallInst(BasicBlockPtr BB, IRFunctionPtr func, std::vector<VariablePtr> args,
           VariablePtr Dest)
      : Instruction(BB), func(func), args(args), Dest(Dest) {
    Op = IROpKind::Op_FUNCALL;
  }
  std::string CodeGen();
  VariablePtr getArg(int Idx) {
    assert(Idx >= 0 && Idx < args.size());
    return args[Idx];
  }
  std::vector<VariablePtr> getArgs() { return args; }
  VariablePtr getResult() { return Dest; }
  VariablePtr &getOperand(int Idx) {
    assert(Idx < args.size());
    return args[Idx];
  }
  unsigned getNumOperands() { return args.size(); }

private:
  IRFunctionPtr func;
  std::vector<VariablePtr> args;
  VariablePtr Dest;
};

class PHINode : public Instruction {
public:
  PHINode(BasicBlockPtr BB, VarTypePtr VTy, unsigned int NumOfIncoming,
          VariablePtr Res)
      : Instruction(BB), ValueTy(VTy), NumOfIncomingValues(NumOfIncoming),
        CurNumOfIncomingValues(0), Result(Res) {
    Op = IROpKind::Op_PhiNode;
  }
  std::string CodeGen();
  VariablePtr getResult() { return Result; };
  VariablePtr &getOperand(int Idx) {
    assert(Idx < InComingValues.size());
    return InComingValues[Idx];
  }
  unsigned getNumOperands() { return CurNumOfIncomingValues; }
  void addIncoming(VariablePtr InComingValue, BasicBlockPtr InComingBB);

private:
  TypePtr ValueTy;
  std::vector<VariablePtr> InComingValues;
  std::vector<BasicBlockPtr> InComingBlocks;
  unsigned int NumOfIncomingValues;
  unsigned int CurNumOfIncomingValues;
  VariablePtr Result;
};
using PHINodePtr = std::shared_ptr<PHINode>;

// class TruncInst : Instruction {};

// class ZExtInst : Instruction {};

// class SExtInst : Instruction {};

#define RPINT_VALUE                                                            \
  {                                                                            \
    if (Op != IROpKind::Op_Alloca && Op != IROpKind::Op_Store &&               \
        Op != IROpKind::Op_Load)                                               \
      s += ";     Left:" + std::to_string(Left->Ival) +                        \
           " Right:" + std::to_string(Right->Ival) +                           \
           " Result:" + std::to_string(Result->Ival) + "\n";                   \
  }
