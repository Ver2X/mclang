#include "Block.h"
#include "Instruction.h"
#include <vector>

class DominTree
{
    int num;
    std::map<BlockPtr, std::vector<BlockPtr> > domins;
public:
    DominTree(int num) : num(num) { }
    void BuildDominTree();
    void BuildPostDominTree();
};