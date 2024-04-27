#pragma once
#include "BasicBlock.h"
#include "Instruction.h"
#include <vector>

class DominTree {
  int num;
  std::map<BasicBlockPtr, std::vector<BasicBlockPtr>> domins;

public:
  DominTree(int num) : num(num) {}
  void BuildDominTree();
  void BuildPostDominTree();
};