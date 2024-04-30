#include "analysis/DominatorTree.h"
#include "analysis/Dominance.h"

DominatorTree::DominatorTreeNodePtr DominatorTree::buildDominatorTree(
    unsigned BBId, DominatorTree::DominatorTreeNodePtr Parent, unsigned Level) {
  auto Curr = Blocks[BBId];
  auto CurrNode = std::make_shared<DominatorTreeNode>(Curr, Parent, Level);
  QuickCheckMap[Curr] = CurrNode;
  for (auto Des : ParentToDes[Curr]) {
    CurrNode->Des.push_back(buildDominatorTree(Des, CurrNode, Level));
  }
  return CurrNode;
}

void DominatorTree::recalculate(IRFunctionPtr Func) {
  IDOM = getIDomOfFunction(Func);
  for (auto [Des, Parent] : IDOM) {
    ParentToDes[Blocks[Parent]].push_back(Des);
  }
  Blocks = Func->getBlocks();
  Root = buildDominatorTree(0, nullptr, 0);
}

bool DominatorTree::dominates(const BasicBlockPtr Def, const BasicBlockPtr BB) {
  return std::find_if(QuickCheckMap[Def]->Des.begin(),
                      QuickCheckMap[Def]->Des.end(),
                      [&BB](DominatorTreeNodePtr Node) {
                        return Node->Curr == BB;
                      }) != QuickCheckMap[Def]->Des.end();
}

bool DominatorTree::dominates(const InstructionPtr Def,
                              const InstructionPtr Use) {
  return dominates(Def->getParent(), Use->getParent());
}

bool DominatorTree::dominates(const VariablePtr Def, const InstructionPtr Use) {
  return dominates(Def->Use->getParent(), Use->getParent());
}
bool DominatorTree::dominates(const VariablePtr Def, const BasicBlockPtr BB) {
  return dominates(Def->Use->getParent(), BB);
}