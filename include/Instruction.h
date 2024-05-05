#pragma once
#include "FunctionIR.h"
#include "User.h"
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
class Instruction : public User,
                    public std::enable_shared_from_this<Instruction> {
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
  Instruction(BasicBlockPtr P, IROpKind Op)
      : Parent(P), Ival(INT_MAX), Op(Op) {}
  IROpKind GetOp() { return Op; }
  BasicBlockPtr getParent() { return Parent; }

  virtual unsigned getNumOperands() = 0;

  bool isValidIdx(int Idx) { return Idx >= 0 && Idx < getNumOperands(); }

  void eraseFromParent();
  virtual std::string CodeGen() = 0;
  bool isTerminator(IROpKind Op) {
    return Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch ||
           Op == IROpKind::Op_Return;
  }
  bool isTerminator() { return isTerminator(GetOp()); }
  bool mayHaveSideEffects() {
    return Op == IROpKind::Op_FUNCALL || Op == IROpKind::Op_Load ||
           Op == IROpKind::Op_Store;
  }
};
using InstructionPtr = std::shared_ptr<Instruction>;
class BasicBlock;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;

class BinaryOperator : public Instruction {
public:
  BinaryOperator(BasicBlockPtr BB, VariablePtr Left, VariablePtr Right,
                 IROpKind Op)
      : Instruction(BB, Op), Left(this, 0), Right(this, 1) {
    this->Left = Left;
    this->Right = Right;
  }
  std::string CodeGen();

  unsigned getNumOperands() { return 2; }

  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    if (Idx == 0) {
      return Left;
    } else {
      return Right;
    }
  }

private:
  Use Left;
  Use Right;
};
using BinaryOperatorPtr = std::shared_ptr<BinaryOperator>;

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
      : Instruction(BB, IROpKind::Op_Return), returnValue(this, 0) {
    this->returnValue = rv;
  }
  // ReturnInst(BasicBlockPtr BB) : Instruction(BB, IROpKind::Op_Return) {  }
  std::string CodeGen();
  unsigned getNumOperands() { return 1; }

  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    return returnValue;
  }

private:
  Use returnValue;
};
using ReturnInstPtr = std::shared_ptr<ReturnInst>;

class BranchInst : public Instruction {
public:
  BranchInst(BasicBlockPtr BB, VariablePtr iV, BasicBlockPtr tg1,
             BasicBlockPtr tg2, IROpKind Op)
      : Instruction(BB, Op), indicateVariable(this, 0), targetFirst(tg1),
        targetSecond(tg2) {
    this->indicateVariable = iV;
  }
  BranchInst(BasicBlockPtr BB, BasicBlockPtr tg1, IROpKind Op)
      : Instruction(BB, Op), targetFirst(tg1), indicateVariable(this, 0) {}
  std::string CodeGen();
  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    return indicateVariable;
  }
  unsigned getNumOperands() { return 1; }
  VariablePtr getResult() { return nullptr; }

private:
  Use indicateVariable;
  BasicBlockPtr targetFirst;
  BasicBlockPtr targetSecond;
};
using BranchInstPtr = std::shared_ptr<BranchInst>;

class AllocaInst : public Instruction {
public:
  AllocaInst(BasicBlockPtr BB, VarTypePtr VTy, VariablePtr ArraySize = nullptr)
      : Instruction(BB, IROpKind::Op_Alloca), VTy(VTy), ArraySize(this, 0) {
    this->ArraySize = ArraySize;
  }
  std::string CodeGen();
  VarTypePtr getAllocatedType() { return VTy; }
  unsigned getNumOperands() { return 1; }
  Use &getOperand(int Idx) { return ArraySize; };

private:
  Use ArraySize;
  VarTypePtr VTy;
};
using AllocaInstPtr = std::shared_ptr<AllocaInst>;

class LoadInst : public Instruction {
public:
  LoadInst(BasicBlockPtr BB, VariablePtr Source)
      : Instruction(BB, IROpKind::Op_Load), Source(this, 0) {
    this->Source = Source;
  }
  std::string CodeGen();
  unsigned getNumOperands() { return 1; }

  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    return Source;
  }
  VariablePtr getPointerOperand() { return Source.getValPtr(); }

private:
  Use Source;
};
using LoadInstPtr = std::shared_ptr<LoadInst>;

class GetElementPtrInst : public Instruction {
public:
  GetElementPtrInst(BasicBlockPtr BB, VarTypePtr PtrTy, VariablePtr BasePtr,
                    std::vector<VariablePtr> IdxList)
      : Instruction(BB, IROpKind::Op_GetElementPtr), PtrTy(PtrTy),
        BasePtr(this, 0) {
    this->BasePtr = BasePtr;
    for (int i = 1; i <= IdxList.size(); ++i) {
      this->IdxList.emplace_back(this, i);
      this->IdxList.back() = IdxList[i - 1];
    }
  }
  std::string CodeGen();
  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    if (Idx == 0) {
      return BasePtr;
    } else {
      assert(Idx - 1 >= 0 && Idx - 1 < IdxList.size());
      return IdxList[Idx - 1];
    }
  }
  VariablePtr getIndex(int Idx) {
    assert(isValidIdx(Idx));
    return IdxList[Idx].getValPtr();
  }
  std::vector<Use> &getIndexs() { return IdxList; }
  unsigned getNumOperands() { return 1 + IdxList.size(); }

private:
  VarTypePtr PtrTy;
  Use BasePtr;
  std::vector<Use> IdxList;
};
using GetElementPtrInstPtr = std::shared_ptr<GetElementPtrInst>;

class StoreInst : public Instruction {
public:
  StoreInst(BasicBlockPtr BB, VariablePtr Source, VariablePtr Dest)
      : Instruction(BB, IROpKind::Op_Store), Source(this, 0), Dest(this, 0) {
    this->Source = Source;
    this->Dest = Dest;
  }
  std::string CodeGen();
  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    if (Idx == 0) {
      return Source;
    } else if (Idx == 1) {
      return Dest;
    }
    assert(false);
  }
  unsigned getNumOperands() { return 2; }
  VariablePtr getPointerOperand() { return Dest.getValPtr(); }
  VariablePtr getOperandOperand() { return Source.getValPtr(); }

private:
  Use Source;
  Use Dest;
};
using StoreInstPtr = std::shared_ptr<StoreInst>;

class CallInst : public Instruction {
public:
  CallInst(BasicBlockPtr BB, IRFunctionPtr func, std::vector<VariablePtr> args)
      : Instruction(BB, IROpKind::Op_FUNCALL), func(func) {
    for (int i = 0; i < args.size(); ++i) {
      this->args.emplace_back(this, i);
      this->args[i] = args[i];
    }
  }
  std::string CodeGen();
  VariablePtr getArg(int Idx) {
    assert(Idx >= 0 && Idx < args.size());
    return args[Idx].getValPtr();
  }
  std::vector<Use> &getArgs() { return args; }
  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    return args[Idx];
  }
  unsigned getNumOperands() { return args.size(); }

private:
  IRFunctionPtr func;
  std::vector<Use> args;
};
using CallInstPtr = std::shared_ptr<CallInst>;

class PHINode : public Instruction {
public:
  PHINode(BasicBlockPtr BB, VarTypePtr VTy, unsigned int NumOfIncoming)
      : Instruction(BB, IROpKind::Op_PhiNode), ValueTy(VTy),
        NumOfIncomingValues(NumOfIncoming), CurNumOfIncomingValues(0) {}
  std::string CodeGen();
  Use &getOperand(int Idx) {
    assert(isValidIdx(Idx));
    return InComingValues[Idx];
  }
  unsigned getNumOperands() { return CurNumOfIncomingValues; }
  void addIncoming(VariablePtr InComingValue, BasicBlockPtr InComingBB);

private:
  TypePtr ValueTy;
  std::vector<Use> InComingValues;
  std::vector<BasicBlockPtr> InComingBlocks;
  unsigned int NumOfIncomingValues;
  unsigned int CurNumOfIncomingValues;
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
