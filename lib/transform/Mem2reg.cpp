#include "transform/Mem2reg.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "analysis/Dominance.h"
#include <stack>

/// Queue a phi-node to be added to a basic-block for a specific Alloca.
///
/// Returns true if there wasn't already a phi-node for that variable
bool PromoteMemoryToRegister::QueuePhiNode(BasicBlockPtr BB, unsigned AllocaNo,
                                           unsigned &Version) {
  // Look up the basic-block in question.
  PHINodePtr &PN = NewPhiNodes[std::make_pair(BBNumbers[BB], AllocaNo)];

  // If the BB already has a phi node added for the i'th alloca then we're done!
  if (PN)
    return false;

  // Create a PhiNode using the dereferenced type... and add the phi-node to the
  // BasicBlock.
  IRBuilder Buider(Func);
  Buider.SetInsertPoint(BB->getFront());
  auto NewPHIV = Buider.CreatePHI(Allocas[AllocaNo]->getAllocatedType(),
                                  BB->GetPred().size(),
                                  Allocas[AllocaNo]->getResult()->getName() +
                                      "." + std::to_string(Version++));
  PN = std::dynamic_pointer_cast<PHINode>(NewPHIV->Use);
  PhiToAllocaMap[PN] = AllocaNo;
  return true;
}

void PromoteMemoryToRegister::promoteMem2Reg() {
  Blocks = Func->getBlocks();
  auto IDF = getIteratedDomFrontierOfFunction(Func);
  for (auto [Idx, Block] : Blocks) {
    Idxs.push_back(Idx);
    BBNumbers[Block] = Idx;
  }

  std::map<VariablePtr, std::set<int>> VarBlocks;
  Allocas = Blocks[Idxs[0]]->getAllocas();

  for (unsigned AllocaNum = 0; AllocaNum != Allocas.size(); ++AllocaNum) {
    auto AI = Allocas[AllocaNum];
    assert(AI->getResult()->Use);

    // If there is only a single store to this value, replace any loads of
    // it that are directly dominated by the definition with the value stored.
    InstructionPtr SingleStore = nullptr;
    for (auto Inst : AI->getResult()->User) {
      if (auto Store = std::dynamic_pointer_cast<StoreInst>(Inst)) {
        if (!SingleStore) {
          SingleStore = Store;
        } else {
          SingleStore = nullptr;
          break;
        }
      }
    }
    if (SingleStore) {
      for (auto Inst : AI->getResult()->User) {
        if (auto Load = std::dynamic_pointer_cast<LoadInst>(Inst)) {
          Load->replaceAllUsesWith(SingleStore->getOperand(0));
          Load->eraseFromParent();
        }
      }
      SingleStore->eraseFromParent();
      AI->eraseFromParent();
      continue;
    }

    // If the alloca is only read and written in one basic block, just perform a
    // linear sweep over the block to eliminate it.

    AllocaLookup[Allocas[AllocaNum]] = AllocaNum;

    // auto PHIBlockNums = IDF[BBNumbers[AI->getParent()]];
    std::set<int> PHIBlockNums;
    std::vector<BasicBlockPtr> PHIBlocks;
    for (auto Inst : AI->getResult()->User) {
      if (std::dynamic_pointer_cast<StoreInst>(Inst))
        PHIBlockNums = Union(PHIBlockNums, IDF[BBNumbers[Inst->getParent()]]);
    }
    for (auto Num : PHIBlockNums) {
      PHIBlocks.push_back(Blocks[Num]);
    }

    unsigned CurrentVersion = 0;
    for (auto BB : PHIBlocks)
      QueuePhiNode(BB, AllocaNum, CurrentVersion);
  }

  // Set the incoming values for the basic block to be null values for all of
  // the alloca's.  We do this in case there is a load of a value that has not
  // been stored yet.  In this case, it will get this null value.
  RenamePassData::ValVector Values(Allocas.size());
  for (unsigned i = 0, e = Allocas.size(); i != e; ++i) {
    if (Allocas[i]->getAllocatedType() == TyInt) {
      Values[i] = std::make_shared<Variable>(0);
    } else {
      Values[i] = std::make_shared<Variable>(nullptr);
    }
  }

  // Walks all basic blocks in the function performing the SSA rename algorithm
  // and inserting the phi nodes we marked as necessary
  std::vector<RenamePassData> RenamePassWorkList;
  RenamePassWorkList.emplace_back(Blocks[Idxs[0]], nullptr, std::move(Values));
  do {
    RenamePassData RPD = std::move(RenamePassWorkList.back());
    RenamePassWorkList.pop_back();

    // RenamePass may add new worklist entries.
    RenamePass(RPD.BB, RPD.Pred, RPD.Values, RenamePassWorkList);
  } while (!RenamePassWorkList.empty());
  Visited.clear();
}

/// Recursively traverse the CFG of the function, renaming loads and
/// stores to the allocas which we are promoting.
///
void PromoteMemoryToRegister::RenamePass(
    BasicBlockPtr BB, BasicBlockPtr Pred,
    RenamePassData::ValVector &IncomingVals,
    std::vector<RenamePassData> &Worklist) {
NextIteration:
  // If we are inserting any phi nodes into this BB, they will already be in the
  // block.
  ///
  if (auto APN = std::dynamic_pointer_cast<PHINode>(*BB->InstInBB.begin())) {
    // If we have PHI nodes to update, compute the number of edges from Pred to
    // BB.
    if (PhiToAllocaMap.count(APN)) {
      // We want to be able to distinguish between PHI nodes being inserted by
      // this invocation of mem2reg from those phi nodes that already existed in
      // the IR before mem2reg was run.  We determine that APN is being inserted
      // because it is missing incoming edges.  All other PHI nodes being
      // inserted by this pass of mem2reg will have the same number of incoming
      // operands so far.  Remember this count.
      unsigned NewPHINumOperands = APN->getNumOperands();

      unsigned NumEdges = Pred->GetSucc().size();
      assert(NumEdges && "Must be at least one edge from Pred to BB!");

      // Add entries for all the phis.
      auto PNI = BB->InstInBB.begin();
      do {
        unsigned AllocaNo = PhiToAllocaMap[APN];

        // Add N incoming values to the PHI node.
        // IncomingVals是构建时的RenamePass(RPD.BB, RPD.Pred, RPD.Values,
        // RPD.Locations, RenamePassWorkList) 中的RPD.Values
        // 也就是一堆UndefValue（初始时）
        //
        // APN = phi ... , [IncomingVals[AllocaNo], Pred]
        for (unsigned i = 0; i != NumEdges; ++i)
          APN->addIncoming(IncomingVals[AllocaNo], Pred);

        // The currently active variable for this block is now the PHI.
        IncomingVals[AllocaNo] = APN->getResult();

        // Get the next phi node.
        ++PNI;
        APN = std::dynamic_pointer_cast<PHINode>(*PNI);
        if (!APN)
          break;
      } while (APN->getNumOperands() == NewPHINumOperands);
    }
  }

  // Don't revisit blocks.
  // IF多个BB指向同一个BB，或者循环反向边等多前驱情况
  if (!Visited.insert(BB).second)
    return;

  for (auto II = BB->InstInBB.begin(); !(*II)->isTerminator();) {
    auto I = *II++; // get the instruction, increment iterator
    if (auto LI = std::dynamic_pointer_cast<LoadInst>(I)) {
      auto Src =
          std::dynamic_pointer_cast<AllocaInst>(LI->getPointerOperand()->Use);
      if (!Src)
        continue;

      auto AI = AllocaLookup.find(Src);
      if (AI == AllocaLookup.end())
        continue;

      auto V = IncomingVals[AI->second];

      // Anything using the load now uses the current value.
      LI->replaceAllUsesWith(V);
      LI->eraseFromParent();
    } else if (auto SI = std::dynamic_pointer_cast<StoreInst>(I)) {
      // Delete this instruction and mark the name as the current holder of the
      // value
      auto Dest =
          std::dynamic_pointer_cast<AllocaInst>(SI->getPointerOperand()->Use);
      if (!Dest) {
        continue;
      }

      auto ai = AllocaLookup.find(Dest);
      if (ai == AllocaLookup.end())
        continue;

      // what value were we writing?
      // store kill了旧值，所以需要更新当前变量的活跃值和低点
      unsigned AllocaNo = ai->second;
      // upate IncomingVals
      IncomingVals[AllocaNo] = SI->getOperand(0);

      SI->eraseFromParent();
    }
  }

  // 'Recurse' to our successors.
  auto I = BB->GetSucc().begin();
  auto E = BB->GetSucc().end();
  if (I == E)
    return;

  // Keep track of the successors so we don't visit the same successor twice
  std::set<BasicBlockPtr> VisitedSuccs;

  // Handle the first successor without using the worklist.
  // 这里执行DFS序列遍历
  VisitedSuccs.insert(*I);
  Pred = BB;
  BB = *I;
  ++I;

  // 初始的时候 RenamePassWorkList.emplace_back(&F.front(), nullptr,
  // std::move(Values),
  //                 std::move(Locations));
  // 注意这里把incomming value也存下来了 dfs 回来的时候就可以取到了
  for (; I != E; ++I)
    if (VisitedSuccs.insert(*I).second)
      Worklist.emplace_back(*I, Pred, IncomingVals);

  goto NextIteration;
}