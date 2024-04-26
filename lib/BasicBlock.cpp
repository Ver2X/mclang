#include "BasicBlock.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>

void BasicBlock::Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
                        BasicBlockPtr targetTwo, IROpKind Op,
                        IRBuilder *buider) {
  if (Op == IROpKind::Op_Branch) {
    buider->lastResVar = indicateVariable;
    assert(indicateVariable != nullptr);
    auto branchInst = std::make_shared<BranchInst>(indicateVariable, targetOne,
                                                   targetTwo, Op);
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
    return;
  } else {
    assert(Op == IROpKind::Op_UnConBranch);
    // buider->lastResVar = indicateVariable;
    assert(indicateVariable == nullptr);
    assert(targetOne != nullptr);
    assert(targetTwo == nullptr);
    auto branchInst = std::make_shared<BranchInst>(targetOne, Op);
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
    return;
  }
}

void BasicBlock::Insert(IRFunctionPtr func, std::vector<VariablePtr> args,
                        VariablePtr Result, IROpKind Op, IRBuilder *buider) {
  assert(Op == IROpKind::Op_FUNCALL);
  auto callInst = std::make_shared<CallInst>(func, args, Result);
  buider->lastResVar = Result;
  instructinos.push_back(std::dynamic_pointer_cast<Instruction>(callInst));
  return;
}

void BasicBlock::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                        IROpKind Op, IRBuilder *buider) {
#if DEBUG
  FileOut << "we going insert insert to blcok with Name :" << this->getName()
          << " Label :" << this->GetLabel() << std::endl;
#endif
  switch (Op) {
  // shouldn't change order
  case IROpKind::Op_Alloca: {
    assert(Left == nullptr && Right == nullptr);
    auto allocaInst = std::make_shared<AllocaInst>(Result);
    // InstructionPtr inst = std::make_shared<Instruction>(Left, Right, Result,
    // Op);
    buider->lastResVar = Result;
    allocas.push_back(std::dynamic_pointer_cast<Instruction>(allocaInst));
    return;
  }
  case IROpKind::Op_Store: {
    auto storeInst = std::make_shared<StoreInst>(Left, Result);

    if (Left == nullptr && Right == nullptr) {
      storeInst->Ival = Result->Ival;
    }
    buider->lastResVar = Result;
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));
    return;
  }
  case IROpKind::Op_Load: {
    auto loadInst = std::make_shared<LoadInst>(Left, Result);
    buider->lastResVar = Result;
    assert(Right == nullptr);
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(loadInst));
    return;
  }
  case IROpKind::Op_Return: {
    // buider->lastResVar = Result;

    // fix me: should not generate temp variable
    // assert(Left == nullptr);
    // assert(Right == nullptr);
    // std::shared_ptr<ReturnInst> returnInst;
    // if(Result == nullptr)
    //	returnInst = std::make_shared<ReturnInst>(buider->lastResVar);
    // else
    auto returnInst = std::make_shared<ReturnInst>(Result);
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(returnInst));
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
    VariablePtr arithRes = std::make_shared<Variable>();
    int nextcf = buider->GetNextCountSuffix();
    if (nextcf != 0)
      s += std::to_string(nextcf);
    arithRes->setName(s);

    auto instArith =
        std::make_shared<BinaryOperator>(Left, Right, arithRes, Op);
    instructinos.push_back(std::dynamic_pointer_cast<Instruction>(instArith));

    buider->lastResVar = arithRes;

    Result->setName(std::move(s));

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
  for (const auto &ins : instructinos) {
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
  for (const auto &ins : instructinos) {
    s += ins->CodeGen() + "\\l";
  }
  return s;
}

std::string BasicBlock::CodeGenCFG() {
  std::string s;
  s += Name + ":\\l";
  for (const auto &ins : instructinos) {
    s += ins->CodeGen() + "\\l";
  }
  return s;
}

void BasicBlock::SetPred(BasicBlockPtr Pred) { preds.push_back(Pred); }

void BasicBlock::SetSucc(BasicBlockPtr succ) { succes.push_back(succ); }