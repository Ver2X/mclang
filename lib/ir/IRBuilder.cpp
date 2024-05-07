#include "ir/IRBuilder.h"
#include "ir/Instruction.h"
#include "ir/Value.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>

std::string IRBuilder::getNextVarName() {
  return "%" + std::to_string(function->nextVarNameNum());
}

int IRBuilder::nextBlockLabelNum() { return function->nextBlockLabelNum(); }

int IRBuilder::nextControlFlowNum() { return function->nextControlFlowNum(); }

void IRBuilder::SetInsertPoint(int Label, std::string Name) {
  if (function->EntryLabel < 0)
    function->EntryLabel = Label;
  if (function->CacheLabel != Label) {
    if (Blocks.count(Label) == 0) {
#if DEBUG
      FileOut << "create a new blcok " << std::endl;
#endif
      BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
      Block->setName(Name);
      Block->SetLabel(Label);
      Blocks.insert(std::make_pair(Label, Block));
    }
    function->CacheLabel = Label;
    function->CacheName = Name;
  }
}

void IRBuilder::SetInsertPoint(BasicBlockPtr insertPoint) {
  if (function->EntryLabel < 0)
    function->EntryLabel = insertPoint->GetLabel();
  if (function->CacheLabel != insertPoint->GetLabel()) {
    if (Blocks.count(insertPoint->GetLabel()) == 0) {
#if DEBUG
      FileOut << "create a new blcok " << std::endl;
#endif
      Blocks.insert(std::make_pair(insertPoint->GetLabel(), insertPoint));
    }
    function->CacheLabel = insertPoint->GetLabel();
    function->CacheName = insertPoint->getName();
  }
}

void IRBuilder::SetInsertPoint(InstructionPtr InsertBeforeInst) {
  SetInsertPoint(InsertBeforeInst->getParent());
  InOrderInsert = false;
  CurrentInsertBefore = InsertBeforeInst->getParent()->InstInBB.begin();
}

void IRBuilder::InsertBasicBlock(int Label, std::string Name, int Pred) {
  // fix me :
  // keep Pred succ Right
  if (function->EntryLabel < 0)
    function->EntryLabel = Label;
  if (Blocks.count(Label) == 0) {
#if DEBUG
    FileOut << "create a new blcok " << std::endl;
#endif

    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Blocks[Pred]->succes.push_back(Block);
    Block->preds.push_back(Blocks[Pred]);

    Block->setName(Name);
    Block->SetLabel(Label);
    Blocks.insert(std::make_pair(Label, Block));
  }
  function->CacheLabel = Label;
  function->CacheName = Name;
}

void IRBuilder::InsertIntoCacheBB(InstructionPtr Inst) {
  Blocks[function->CacheLabel]->Insert(this, Inst);
}

CallInstPtr IRBuilder::CreateCall(IRFunctionPtr func,
                                  std::vector<VariablePtr> args) {
  auto Res =
      std::make_shared<CallInst>(Blocks[function->CacheLabel], func, args);
  Res->setName(getNextVarName());
  InsertIntoCacheBB(Res);
  return Res;
  // std::cout << "in ssss" << std::endl;
}

BinaryOperatorPtr IRBuilder::CreateBinary(VariablePtr Left, VariablePtr Right,
                                          IROpKind Op) {
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
  auto Res = std::make_shared<BinaryOperator>(Blocks[function->CacheLabel],
                                              Left, Right, Op);
  if (Op == IROpKind::Op_SLE || Op == IROpKind::Op_EQ ||
      Op == IROpKind::Op_SLT || Op == IROpKind::Op_SGE ||
      Op == IROpKind::Op_SGT || Op == IROpKind::Op_NE) {
    Res->setType(TyBit);
  } else {
    Res->setType(Right->getType());
  }
  Res->setName(this->getParent()->createName(s));
  InsertIntoCacheBB(Res);
  return Res;
}

LoadInstPtr IRBuilder::CreateLoad(VariablePtr Addr) {
  auto Res = std::make_shared<LoadInst>(Blocks[function->CacheLabel], Addr);
  Res->setName(getNextVarName());
  // std::cout << "Load Res is " << Res->CodeGen() << "\n";
  // std::cout << "Error Load: " << Addr->CodeGen() << "\n";
  // assert(Addr->getType()->Kind == TypeKind::TY_PTR);
  assert(baseTo(Addr->getType()));
  Res->setType(baseTo(Addr->getType()));

  InsertIntoCacheBB(Res);
  return Res;
}

void IRBuilder::CreateRet(VariablePtr value) {
  auto Res = std::make_shared<ReturnInst>(Blocks[function->CacheLabel], value);

  InsertIntoCacheBB(Res);
}

// void IRBuilder::CreateAlloca(VariablePtr Addr) {
//   Insert(nullptr, nullptr, Addr, IROpKind::Op_Alloca, function->GeTable());
// }

AllocaInstPtr IRBuilder::CreateAlloca(VarTypePtr VTy,
                                      VariablePtr ArraySize = nullptr,
                                      std::string Name = "") {
  auto Res = std::make_shared<AllocaInst>(Blocks[function->CacheLabel], VTy,
                                          ArraySize);
  Res->setName(Name);
  Res->setType(pointerTo(VTy));

  // InsertIntoCacheBB(Res);
  Blocks[function->CacheLabel]->Allocas.push_back(Res);
  return Res;
}

GetElementPtrInstPtr IRBuilder::CreateGEP(VarTypePtr VTy, VariablePtr Ptr,
                                          std::vector<VariablePtr> IdxList,
                                          std::string Name) {
  auto Res = std::make_shared<GetElementPtrInst>(Blocks[function->CacheLabel],
                                                 VTy, Ptr, IdxList);
  Res->setName(Name);
  if (Res->getName() == "")
    Res->setName(this->getParent()->createName("%arrayidx"));
  auto ElementTy = baseTo(VTy);
  assert(ElementTy->Kind == TypeKind::TY_ARRAY ||
         ElementTy->Kind == TypeKind::TY_STRUCT);
  assert(VTy == Ptr->getType());
  if (ElementTy->Kind == TypeKind::TY_ARRAY)
    Res->setType(pointerTo(baseTo(ElementTy)));
  else {
    // it is decide by witch member
    auto Idx = IdxList[1]->Ival;
    auto Head = ElementTy->Members;
    while (Idx--) {
      Head = Head->Next;
    }
    auto ResTy = Head->Ty;
    Res->setType(pointerTo(ResTy));
  }

  InsertIntoCacheBB(Res);
  return Res;
}

PHINodePtr IRBuilder::CreatePHI(VarTypePtr VTy, unsigned NumReservedValues,
                                const std::string &Name) {
  auto Res = std::make_shared<PHINode>(Blocks[function->CacheLabel], VTy,
                                       NumReservedValues);
  Res->setName(Name);
  Res->setType(VTy);

  InsertIntoCacheBB(Res);
  return Res;
}

void IRBuilder::CreateStore(VariablePtr V, VariablePtr Addr) {
  auto Res = std::make_shared<StoreInst>(Blocks[function->CacheLabel], V, Addr);

  InsertIntoCacheBB(Res);
}

void IRBuilder::CreateCondBr(VariablePtr Cond, BasicBlockPtr True,
                             BasicBlockPtr False) {
  assert(Cond);
  auto Res = std::make_shared<BranchInst>(Blocks[function->CacheLabel], Cond,
                                          True, False, IROpKind::Op_Branch);
  auto Curr = getCurrentBlock();
  Curr->SetSucc(True);
  Curr->SetSucc(False);
  True->SetPred(Curr);
  False->SetPred(Curr);
  InsertIntoCacheBB(Res);
}

void IRBuilder::CreateBr(BasicBlockPtr Target) {
  auto Res = std::make_shared<BranchInst>(Blocks[function->CacheLabel], Target,
                                          IROpKind::Op_UnConBranch);
  auto Curr = getCurrentBlock();
  Curr->SetSucc(Target);
  Target->SetPred(Curr);

  InsertIntoCacheBB(Res);
}

// std::string IRBuilder::CodeGen() {

// }

BasicBlockPtr IRBuilder::getCurrentBlock() {
  if (function->CacheLabel == -1)
    return nullptr;
  return Blocks[function->CacheLabel];
}

void IRBuilder::SetPredAndSuccNum() {
  std::map<BasicBlockPtr, bool> Visit;
  BasicBlockPtr current;
  for (auto pr : Blocks) {
    current = pr.second;
    Visit[current] = false;
  }
  int i, j;
  i = 0;
  j = 0;
  DepthFirstSearchPP(i, j, Visit, Blocks[function->EntryLabel]->succes,
                     Blocks[function->EntryLabel]);
  assert(i == j);
  function->numofblock = i;
}

void IRBuilder::DepthFirstSearchPP(int &i, int &j,
                                   std::map<BasicBlockPtr, bool> &Visit,
                                   std::list<BasicBlockPtr> &succ,
                                   BasicBlockPtr x) {
  // BasicBlockPtr y;
  Visit[x] = true;
  PreNum[x] = j;
  PreNumToBlock[j] = x;
  j++;
  for (auto y : succ) {
    if (!Visit[y]) {
      DepthFirstSearchPP(i, j, Visit, y->succes, y);
      EdgeKinds[std::make_pair(x, y)] = EdgeKind::TreeEdge;
    } else if (PreNum[x] < PreNum[y]) {
      EdgeKinds[std::make_pair(x, y)] = EdgeKind::ForwardEdge;
    } else if (PostNum[y] == 0) {
      EdgeKinds[std::make_pair(x, y)] = EdgeKind::BackEdge;
    } else {
      EdgeKinds[std::make_pair(x, y)] = EdgeKind::CrossEdge;
    }
  }
  PostNum[x] = i;
  PostNumToBlock[i] = x;
  i++;
}

std::string IRBuilder::DumpCFG() {
  /*
  digraph CFG {
          0 -> 1-> 2
          1 -> 3;
          0 [shape=polygon,sides=5,peripheries=3,color=lightblue,style=filled];
          2 [shape=polygon,sides=4,Label="hello world"]
          3 [shape=invtriangle];
          4 [shape=polygon,sides=4,distortion=.7];
  }
  */
  std::string s = "digraph CFG {\n";
  for (auto edge : EdgeKinds) {
    auto From = edge.first.first;
    auto To = edge.first.second;
    s += "  " + std::to_string(PreNum[From]) + " -> " +
         std::to_string(PreNum[To]) + ";\n";
  }
  for (int i = 0; i < function->numofblock; i++) {
    s += "\n";
    if (i != 0)
      s += "  " + std::to_string(i) + " [shape = polygon, label = \" " +
           PreNumToBlock[i]->CodeGenCFG() + "\" ];";
    else
      s += "  " + std::to_string(i) + " [shape = polygon, label = \" " +
           PreNumToBlock[i]->EntryCodeGenCFG() + "\" ];";
  }
  s += "\n}\n";
  return s;
}