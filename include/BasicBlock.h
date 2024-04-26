#pragma once
#include "Instruction.h"

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
class BasicBlock;
class IRBuilder;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using Edge = std::pair<BasicBlockPtr, BasicBlockPtr>;

enum class EdgeKind {
  ForwardEdge,
  BackEdge,
  CrossEdge,
  TreeEdge,
};

class BasicBlock {

  std::vector<BasicBlockPtr> preds;
  std::vector<BasicBlockPtr> succes;
  int Label;
  std::string Name;
  // entry blck is a special BasicBlock
  std::vector<InstructionPtr> allocas;
  friend class IRBuilder;
  // std::vector<InstructionPtr> instructinos;
public:
  std::vector<InstructionPtr> instructinos;
  BasicBlock() { Label = 0; }
  BasicBlock(int Label, std::string Name) : Label(Label), Name(Name) {}
  void setName(std::string Name) { this->Name = Name; }
  std::string getName() { return Name; }
  void SetLabel(int Label) { this->Label = Label; }
  int GetLabel() { return Label; }
  void Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
              IROpKind Op, IRBuilder *buider);
  void Insert(IRFunctionPtr func, std::vector<VariablePtr> args,
              VariablePtr Result, IROpKind Op, IRBuilder *buider);
  void Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
              BasicBlockPtr targetTwo, IROpKind Op, IRBuilder *buider);
  void SetPred(BasicBlockPtr);
  void SetSucc(BasicBlockPtr);
  std::string CodeGen();
  std::string CodeGenCFG();
  std::string AllocaCodeGen();
  std::string EntryCodeGenCFG();
};