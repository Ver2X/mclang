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
class Block;
class IRBuilder;
using BlockPtr = std::shared_ptr<Block>;
using Edge = std::pair<BlockPtr, BlockPtr>;

enum class EdgeKind {
  ForwardEdge,
  BackEdge,
  CrossEdge,
  TreeEdge,
};

class Block {

  std::vector<BlockPtr> preds;
  std::vector<BlockPtr> succes;
  int label;
  std::string name;
  // entry blck is a special block
  std::vector<InstructionPtr> allocas;
  friend class IRBuilder;
  // std::vector<InstructionPtr> instructinos;
public:
  std::vector<InstructionPtr> instructinos;
  Block() { label = 0; }
  Block(int label, std::string name) : label(label), name(name) {}
  void SetName(std::string name) { this->name = name; }
  std::string GetName() { return name; }
  void SetLabel(int label) { this->label = label; }
  int GetLabel() { return label; }
  void Insert(VariablePtr left, VariablePtr right, VariablePtr result,
              IROpKind Op, IRBuilder *buider);
  void Insert(VariablePtr indicateVariable, BlockPtr targetOne,
              BlockPtr targetTwo, IROpKind Op, IRBuilder *buider);
  void SetPred(BlockPtr);
  void SetSucc(BlockPtr);
  std::string CodeGen();
  std::string CodeGenCFG();
  std::string AllocaCodeGen();
  std::string EntryCodeGenCFG();
};