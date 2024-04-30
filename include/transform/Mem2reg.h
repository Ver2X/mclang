#pragma once
#include "BasicBlock.h"
#include "FunctionIR.h"
#include <algorithm>
#include <map>
#include <memory>
#include <set>

struct PromoteMemoryToRegister {
  struct RenamePassData {
    using ValVector = std::vector<VariablePtr>;

    RenamePassData(BasicBlockPtr B, BasicBlockPtr P, ValVector V)
        : BB(B), Pred(P), Values(std::move(V)) {}

    BasicBlockPtr BB;
    BasicBlockPtr Pred;
    ValVector Values;
  };
  PromoteMemoryToRegister(IRFunctionPtr Func) : Func(Func) {}

  void promoteMem2Reg();
  void Rename(BasicBlockPtr Block);
  std::string NewName(VariablePtr N);
  bool QueuePhiNode(BasicBlockPtr BB, unsigned AllocaNo, unsigned &Version);
  void RenamePass(BasicBlockPtr BB, BasicBlockPtr Pred,
                  RenamePassData::ValVector &IncomingVals,
                  std::vector<RenamePassData> &Worklist);

private:
  IRFunctionPtr Func;
  std::map<int, BasicBlockPtr> Blocks;
  std::map<std::pair<unsigned, unsigned>, PHINodePtr> NewPhiNodes;
  std::map<BasicBlockPtr, int> BBNumbers;
  std::vector<int> Idxs;
  std::vector<AllocaInstPtr> Allocas;
  std::map<PHINodePtr, int> PhiToAllocaMap;
  std::set<BasicBlockPtr> Visited;
  std::map<AllocaInstPtr, unsigned> AllocaLookup;

  void RemoveFromAllocasList(unsigned &AllocaIdx) {
    Allocas[AllocaIdx] = Allocas.back();
    Allocas.pop_back();
    --AllocaIdx;
  }

  // std::map<PHINodePtr, VariablePtr> PHIToVar;
  // std::map<VariablePtr, int> Counter;
  // std::map<VariablePtr, std::stack<int>> Stack;
  // std::set<VariablePtr> Globals;
};