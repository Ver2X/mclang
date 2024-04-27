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
    auto branchInst = std::make_shared<BranchInst>(indicateVariable, targetOne,
                                                   targetTwo, Op);
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
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
    auto branchInst = std::make_shared<BranchInst>(targetOne, Op);
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
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
  auto GepInst = std::make_shared<GetElementPtrInst>(VTy, Ptr, IdxList, Res);
  InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(GepInst));
  Res->Use = GepInst;
  for (auto &Id : IdxList) {
    Id->User.push_back(GepInst);
  }
  Ptr->User.push_back(GepInst);
  return;
}

void BasicBlock::Insert(IRFunctionPtr Func, std::vector<VariablePtr> Args,
                        VariablePtr Result, IROpKind Op, IRBuilder *Buider) {
  assert(Op == IROpKind::Op_FUNCALL);
  auto callInst = std::make_shared<CallInst>(Func, Args, Result);
  Buider->lastResVar = Result;
  InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(callInst));
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
  auto allocaInst = std::make_shared<AllocaInst>(VTy, Result, ArraySize);
  // InstructionPtr inst = std::make_shared<Instruction>(Left, Right, Result,
  // Op);
  Buider->lastResVar = Result;
  allocas.push_back(std::dynamic_pointer_cast<Instruction>(allocaInst));
  Result->Use = allocaInst;
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
    // auto allocaInst = std::make_shared<AllocaInst>(Result);
    // // InstructionPtr inst = std::make_shared<Instruction>(Left, Right,
    // Result,
    // // Op);
    // Buider->lastResVar = Result;
    // allocas.push_back(std::dynamic_pointer_cast<Instruction>(allocaInst));
    // Result->Use = allocaInst;
    return;
  }
  case IROpKind::Op_Store: {
    auto storeInst = std::make_shared<StoreInst>(Left, Result);

    if (Left == nullptr && Right == nullptr) {
      storeInst->Ival = Result->Ival;
    }
    Buider->lastResVar = Result;
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    Result->Use = storeInst;
    if (Left)
      Left->User.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    if (Result)
      Result->User.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    return;
  }
  case IROpKind::Op_Load: {
    auto loadInst = std::make_shared<LoadInst>(Left, Result);
    Buider->lastResVar = Result;
    assert(Right == nullptr);
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(loadInst));
    Result->Use = loadInst;
    if (Left)
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
    auto returnInst = std::make_shared<ReturnInst>(Result);
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(returnInst));
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
    auto instArith = std::make_shared<BinaryOperator>(Left, Right, Result, Op);
    InstInBB.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
    Buider->lastResVar = Result;
    // Result->setName(std::move(s));
    Result->Use = instArith;
    if (Left)
      Left->User.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
    if (Right)
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
  if (this->allocas.empty())
    s += LabelName + ":\n";
  for (const auto &ins : InstInBB) {
    s += ins->CodeGen() + "\n";
  }
  return s;
}

std::string BasicBlock::AllocaCodeGen() {
  std::string s;
  s += Name + ":\n";
  for (const auto &ins : allocas) {
    s += ins->CodeGen() + "\n";
  }
  return s;
}

std::string BasicBlock::EntryCodeGenCFG() {
  std::string s;
  s += Name + ":\\l";
  for (const auto &ins : allocas) {
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