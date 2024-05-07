#include "transform/DCE.h"
#include "ir/BasicBlock.h"
#include <set>

bool isInstructionTriviallyDead(InstructionPtr I) {
  if (!I->use_empty())
    return false;
  if (I->isTerminator())
    return false;

  if (!I->mayHaveSideEffects()) {
    return true;
  }

  return false;
}

bool DCEInstruction(InstructionPtr Inst, std::set<InstructionPtr> &WorkList) {
  if (isInstructionTriviallyDead(Inst)) {
    // Null out all of the instruction's operands to see if any operand becomes
    // dead as we go.
    std::cout << "will remove : " << Inst->CodeGen() << "\n";
    for (unsigned i = 0, e = Inst->getNumOperands(); i != e; ++i) {
      auto OpV = Inst->getOperand(i).getValPtr();

      Inst->setOperand(i, nullptr);

      if (!OpV || !OpV->users().empty())
        continue;

      if (auto OpI = std::dynamic_pointer_cast<Instruction>(OpV))
        if (isInstructionTriviallyDead(OpI))
          WorkList.insert(OpI);
    }
    Inst->eraseFromParent();
    return true;
  }
  return false;
}

bool eliminateDeadCode(IRFunctionPtr Func) {
  bool MadeChange = false;
  std::set<InstructionPtr> WorkList;

  for (auto [_, BB] : Func->getBlocks()) {
    for (auto II = BB->InstInBB.begin(), IE = BB->InstInBB.end(); II != IE;) {
      auto CurInst = *(II++);
      if (!WorkList.count(CurInst))
        MadeChange |= DCEInstruction(CurInst, WorkList);
    }
  }

  while (!WorkList.empty()) {
    auto I = WorkList.begin();
    auto Inst = *I;
    WorkList.erase(I);
    MadeChange |= DCEInstruction(Inst, WorkList);
  }
  return MadeChange;
}