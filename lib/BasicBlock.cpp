#include "BasicBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>

void BasicBlock::Insert(IRBuilder *Buider, InstructionPtr Inst) {
  if (Buider->isOrdered()) {
    InstInBB.push_back(Inst);
  } else {
    InstInBB.insert(Buider->CurrentInsertBefore, Inst);
  }
}

void BasicBlock::eraseFromParent(InstructionPtr Inst) {
  // delete
  if (auto Alloca = std::dynamic_pointer_cast<AllocaInst>(Inst)) {
    Allocas.erase(std::remove(Allocas.begin(), Allocas.end(), Alloca),
                  Allocas.end());
  } else {
    InstInBB.remove(Inst);
  }
}

std::string BasicBlock::CodeGen() {
  // #ifdef DEBUG
  //	FileOut << "dumping BasicBlock " << this->getName() << std::endl;
  // #endif
  std::string s;
  std::string LabelName = Name;
  if (!Name.empty() && Name[0] == '%') {
    LabelName = Name.substr(1);
  }
  if (LabelName != "entry")
    s += "\n";
  if (this->Allocas.empty())
    s += LabelName + ":\n";
  for (const auto &ins : InstInBB) {
    s += ins->CodeGen() + "\n";
  }
  return s;
}

std::string BasicBlock::AllocaCodeGen() {
  std::string s;
  s += Name + ":\n";
  for (const auto &ins : Allocas) {
    s += ins->CodeGen() + "\n";
  }
  return s;
}

std::string BasicBlock::EntryCodeGenCFG() {
  std::string s;
  s += Name + ":\\l";
  for (const auto &ins : Allocas) {
    s += ins->CodeGen() + "\\l";
  }
  for (const auto &ins : InstInBB) {
    s += ins->CodeGen() + "\\l";
  }
  return s;
}

std::string BasicBlock::CodeGenCFG() {
  std::string s;
  s += Name + ":\\l";
  for (const auto &ins : InstInBB) {
    s += ins->CodeGen() + "\\l";
  }
  return s;
}

void BasicBlock::SetPred(BasicBlockPtr Pred) { preds.push_back(Pred); }

void BasicBlock::SetSucc(BasicBlockPtr succ) { succes.push_back(succ); }