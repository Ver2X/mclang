#include "BasicBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>

void BasicBlock::Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
                        BasicBlockPtr targetTwo, IROpKind Op,
                        IRBuilder *Buider) {
  if (Op == IROpKind::Op_Branch) {
    Buider->lastResVar = indicateVariable;
    assert(indicateVariable != nullptr);
    auto branchInst = std::make_shared<BranchInst>(
        shared_from_this(), indicateVariable, targetOne, targetTwo, Op);
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(branchInst));
    }
    if (indicateVariable)
      indicateVariable->User.push_back(
          std::dynamic_pointer_cast<Instruction>(branchInst));
    return;
  } else {
    assert(Op == IROpKind::Op_UnConBranch);
    // Buider->lastResVar = indicateVariable;
    assert(indicateVariable == nullptr);
    assert(targetOne != nullptr);
    assert(targetTwo == nullptr);
    auto branchInst =
        std::make_shared<BranchInst>(shared_from_this(), targetOne, Op);
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(branchInst));
    }
    return;
  }
}

void BasicBlock::Insert(VarTypePtr VTy, VariablePtr Ptr,
                        std::vector<VariablePtr> IdxList, VariablePtr Res,
                        IROpKind Op, IRBuilder *Buider) {
  assert(Op == IROpKind::Op_GetElementPtr);
  Buider->lastResVar = Res;
  if (Res->getName() == "")
    Res->setName(Buider->getParent()->createName("%arrayidx"));
  auto GepInst = std::make_shared<GetElementPtrInst>(shared_from_this(), VTy,
                                                     Ptr, IdxList, Res);
  if (Buider->isOrdered()) {
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(GepInst));
  } else {
    InstInBB.insert(Buider->CurrentInsertBefore,
                    std::dynamic_pointer_cast<Instruction>(GepInst));
  }
  Res->Use = GepInst;
  for (auto &Id : IdxList) {
    Id->User.push_back(GepInst);
  }
  Ptr->User.push_back(GepInst);
  return;
}

void BasicBlock::Insert(VarTypePtr VTy, unsigned NumReservedValues,
                        VariablePtr Res, IROpKind Op, IRBuilder *Buider) {
  assert(Op == IROpKind::Op_PhiNode);
  Buider->lastResVar = Res;
  if (Res->getName() == "")
    Res->setName(Buider->getParent()->createName("%phi"));
  auto PHIInst = std::make_shared<PHINode>(shared_from_this(), VTy,
                                           NumReservedValues, Res);
  if (Buider->isOrdered()) {
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(PHIInst));
  } else {
    InstInBB.insert(Buider->CurrentInsertBefore,
                    std::dynamic_pointer_cast<Instruction>(PHIInst));
  }
  Res->Use = PHIInst;
  PHINodes.push_back(PHIInst);
  return;
}

void BasicBlock::Insert(IRFunctionPtr Func, std::vector<VariablePtr> Args,
                        VariablePtr Result, IROpKind Op, IRBuilder *Buider) {
  assert(Op == IROpKind::Op_FUNCALL);
  auto callInst =
      std::make_shared<CallInst>(shared_from_this(), Func, Args, Result);
  Buider->lastResVar = Result;
  if (Buider->isOrdered()) {
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(callInst));
  } else {
    InstInBB.insert(Buider->CurrentInsertBefore,
                    std::dynamic_pointer_cast<Instruction>(callInst));
  }
  Result->Use = callInst;
  for (auto Arg : Args) {
    if (Arg)
      Arg->User.push_back(std::dynamic_pointer_cast<Instruction>(callInst));
  }
  return;
}

void BasicBlock::Insert(VarTypePtr VTy, VariablePtr ArraySize,
                        VariablePtr Result, IROpKind Op, IRBuilder *Buider) {
  assert(Op == IROpKind::Op_Alloca);
  auto allocaInst =
      std::make_shared<AllocaInst>(shared_from_this(), VTy, Result, ArraySize);
  Buider->lastResVar = Result;
  Allocas.push_back(allocaInst);
  Result->Use = allocaInst;
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

void BasicBlock::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                        IROpKind Op, IRBuilder *Buider) {
#if DEBUG
  FileOut << "we going insert insert to blcok with Name :" << this->getName()
          << " Label :" << this->GetLabel() << std::endl;
#endif
  switch (Op) {
  // shouldn't change order
  case IROpKind::Op_Alloca: {
    assert(false);
    return;
  }
  case IROpKind::Op_Store: {
    auto storeInst =
        std::make_shared<StoreInst>(shared_from_this(), Left, Result);
    Buider->lastResVar = Result;
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(storeInst));
    }
    // Store don't have Result it is Dest
    Left->User.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    Result->User.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    return;
  }
  case IROpKind::Op_Load: {
    auto loadInst =
        std::make_shared<LoadInst>(shared_from_this(), Left, Result);
    Buider->lastResVar = Result;
    assert(Right == nullptr);
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(loadInst));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(loadInst));
    }
    Result->Use = loadInst;
    Left->User.push_back(std::dynamic_pointer_cast<Instruction>(loadInst));
    return;
  }
  case IROpKind::Op_Return: {
    // Buider->lastResVar = Result;

    // fix me: should not generate temp variable
    // assert(Left == nullptr);
    // assert(Right == nullptr);
    // std::shared_ptr<ReturnInst> returnInst;
    // if(Result == nullptr)
    //	returnInst = std::make_shared<ReturnInst>(Buider->lastResVar);
    // else
    auto returnInst = std::make_shared<ReturnInst>(shared_from_this(), Result);
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(returnInst));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(returnInst));
    }
    if (Result)
      Result->User.push_back(
          std::dynamic_pointer_cast<Instruction>(returnInst));
    return;
  }
  case IROpKind::Op_ADD:
  case IROpKind::Op_SUB:
  case IROpKind::Op_MUL:
  case IROpKind::Op_DIV:
  case IROpKind::Op_SLE:
  case IROpKind::Op_SLT:
  case IROpKind::Op_SGE:
  case IROpKind::Op_SGT:
  case IROpKind::Op_NE:
  case IROpKind::Op_EQ: {
    std::string s;
    switch (Op) {
    case IROpKind::Op_ADD:
      s = "%add";
      break;
    case IROpKind::Op_SUB:
      s = "%sub";
      break;
    case IROpKind::Op_MUL:
      s = "%mul";
      break;
    case IROpKind::Op_DIV:
      s = "%div";
      break;
    case IROpKind::Op_SLE:
    case IROpKind::Op_SLT:
    case IROpKind::Op_SGE:
    case IROpKind::Op_SGT:
      s = "%cmp";
      break;
    default:
      break;
    }
    // VariablePtr arithRes = std::make_shared<Variable>();
    // int nextcf = Buider->GetNextCountSuffix();
    // if (nextcf != 0)
    //   s += std::to_string(nextcf);
    Result->setName(Buider->getParent()->createName(s));
    auto instArith = std::make_shared<BinaryOperator>(shared_from_this(), Left,
                                                      Right, Result, Op);
    if (Buider->isOrdered()) {
      InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
    } else {
      InstInBB.insert(Buider->CurrentInsertBefore,
                      std::dynamic_pointer_cast<Instruction>(instArith));
    }
    Buider->lastResVar = Result;
    // Result->setName(std::move(s));
    Result->Use = instArith;
    Left->User.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
    Right->User.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
    return;
  }
  default: {
    assert(false);
  }
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