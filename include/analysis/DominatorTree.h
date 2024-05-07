#pragma once
#include "ir/FunctionIR.h"

class DominatorTree {
public:
  DominatorTree() = default;
  DominatorTree(IRFunctionPtr Func) { recalculate(Func); }
  struct DominatorTreeNode;
  using DominatorTreeNodePtr = std::shared_ptr<DominatorTreeNode>;
  struct DominatorTreeNode {
    DominatorTreeNode(BasicBlockPtr Curr, DominatorTreeNodePtr Parent,
                      unsigned Level)
        : Curr(Curr), Parent(Parent), Level(Level) {}
    BasicBlockPtr Curr;
    DominatorTreeNodePtr Parent;
    unsigned Level;
    std::vector<DominatorTreeNodePtr> Des;
  };

  void recalculate(IRFunctionPtr F);
  bool dominates(const BasicBlockPtr Def, const BasicBlockPtr BB);
  bool dominates(const InstructionPtr Def, const InstructionPtr Use);
  bool dominates(const InstructionPtr Def, const BasicBlockPtr Use);
  bool dominates(const BasicBlockPtr Def, const InstructionPtr BB);

private:
  DominatorTreeNodePtr Root;
  DominatorTreeNodePtr buildDominatorTree(unsigned BBId,
                                          DominatorTreeNodePtr Parent,
                                          unsigned Level);
  std::map<int, int> IDOM;
  std::map<BasicBlockPtr, std::vector<int>> ParentToDes;
  std::map<int, BasicBlockPtr> Blocks;
  std::map<BasicBlockPtr, DominatorTreeNodePtr> QuickCheckMap;
};