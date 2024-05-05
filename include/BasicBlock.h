#pragma once
#include "Instruction.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <list>
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
class BasicBlock;
class IRBuilder;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using IRBuilderPtr = std::shared_ptr<IRBuilder>;
using Edge = std::pair<BasicBlockPtr, BasicBlockPtr>;

enum class EdgeKind {
  ForwardEdge,
  BackEdge,
  CrossEdge,
  TreeEdge,
};

class BasicBlock : public std::enable_shared_from_this<BasicBlock> {

  std::list<BasicBlockPtr> preds;
  std::list<BasicBlockPtr> succes;
  int Label;
  std::string Name;
  // entry blck is a special BasicBlock
  std::vector<AllocaInstPtr> Allocas;
  std::list<PHINodePtr> PHINodes;
  friend class IRBuilder;
  friend class Instruction;
  IRFunctionPtr Parent;
  void eraseFromParent(InstructionPtr Inst);

public:
  std::list<InstructionPtr> InstInBB;
  BasicBlock(IRFunctionPtr Parent) : Parent(Parent) { Label = 0; }
  BasicBlock(IRFunctionPtr Parent, int Label, std::string Name)
      : Parent(Parent), Label(Label), Name(Name) {}
  InstructionPtr getFront() { return InstInBB.front(); }
  void setName(std::string Name) { this->Name = Name; }
  IRFunctionPtr getParent() { return Parent; }
  std::vector<AllocaInstPtr> getAllocas() { return Allocas; }
  std::string getName() { return Name; }
  void SetLabel(int Label) { this->Label = Label; }
  int GetLabel() { return Label; }
  void Insert(IRBuilder *Buider, InstructionPtr Inst);
  void SetPred(BasicBlockPtr);
  void SetSucc(BasicBlockPtr);
  const std::list<BasicBlockPtr> &GetPred() { return preds; };
  const std::list<BasicBlockPtr> &GetSucc() { return succes; };
  std::string CodeGen();
  std::list<PHINodePtr> phis() { return PHINodes; };
  std::string CodeGenCFG();
  std::string AllocaCodeGen();
  std::string EntryCodeGenCFG();
};