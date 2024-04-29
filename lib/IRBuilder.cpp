#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
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

// extern SymbolTablePtr GlobalSymTable;
bool IRBuilder::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                       IROpKind Op, int Label, std::string Name,
                       SymbolTablePtr Table) {
  if (function->EntryLabel < 0)
    function->EntryLabel = Label;
  if (Blocks.count(Label) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Block->setName(Name);
    Block->SetLabel(Label);
    Blocks.insert(std::make_pair(Label, Block));
  }
  Blocks[Label]->Insert(Left, Right, Result, Op, this);
  return true;
}

VariablePtr IRBuilder::CreateCall(IRFunctionPtr func,
                                  std::vector<VariablePtr> args) {
  auto Res = std::make_shared<Variable>();
  Res->setName(getNextVarName());
  Res->setType(func->RetTy);
  if (function->EntryLabel < 0)
    function->EntryLabel = function->CacheLabel;
  Blocks[function->CacheLabel]->Insert(func, args, Res, IROpKind::Op_FUNCALL,
                                       this);
  return Res;
  // std::cout << "in ssss" << std::endl;
}

bool IRBuilder::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                       IROpKind Op, SymbolTablePtr Table) {
  return Insert(Left, Right, Result, Op, function->CacheLabel,
                function->CacheName, Table);
}

VariablePtr IRBuilder::CreateBinary(VariablePtr Left, VariablePtr Right,
                                    IROpKind Op) {
  auto Res = std::make_shared<Variable>();
  if (Op == IROpKind::Op_SLE || Op == IROpKind::Op_EQ ||
      Op == IROpKind::Op_SLT || Op == IROpKind::Op_SGE ||
      Op == IROpKind::Op_SGT || Op == IROpKind::Op_NE) {
    Res->setType(TyBit);
  } else {
    Res->setType(Right->getType());
  }
  auto rt = Insert(Left, Right, Res, Op, function->CacheLabel,
                   function->CacheName, function->GeTable());
  assert(rt);
  return Res;
}

VariablePtr IRBuilder::CreateLoad(VariablePtr Addr) {
  auto Res = std::make_shared<Variable>();
  Res->setName(getNextVarName());
  // std::cout << "Load Res is " << Res->CodeGen() << "\n";
  // std::cout << "Error Load: " << Addr->CodeGen() << "\n";
  // assert(Addr->getType()->Kind == TypeKind::TY_PTR);
  Res->setType(baseTo(Addr->getType()));
  auto rt = Insert(Addr, nullptr, Res, IROpKind::Op_Load, function->CacheLabel,
                   function->CacheName, function->GeTable());
  assert(rt);
  return Res;
}

void IRBuilder::CreateRet(VariablePtr value) {
  Insert(nullptr, nullptr, value, IROpKind::Op_Return, function->CacheLabel,
         function->CacheName, function->GeTable());
}

// void IRBuilder::CreateAlloca(VariablePtr Addr) {
//   Insert(nullptr, nullptr, Addr, IROpKind::Op_Alloca, function->GeTable());
// }

VariablePtr IRBuilder::CreateAlloca(VarTypePtr VTy,
                                    VariablePtr ArraySize = nullptr,
                                    std::string Name = "") {
  auto Res = std::make_shared<Variable>();
  Res->setName(Name);
  Res->setType(pointerTo(VTy));

  if (function->EntryLabel < 0)
    function->EntryLabel = function->CacheLabel;
  if (Blocks.count(function->CacheLabel) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Block->setName(function->CacheName);
    Block->SetLabel(function->CacheLabel);
    Blocks.insert(std::make_pair(function->CacheLabel, Block));
  }
  Blocks[function->EntryLabel]->Insert(VTy, ArraySize, Res, IROpKind::Op_Alloca,
                                       this);
  return Res;
}

VariablePtr IRBuilder::CreateGEP(VarTypePtr VTy, VariablePtr Ptr,
                                 std::vector<VariablePtr> IdxList,
                                 std::string Name) {
  auto Res = std::make_shared<Variable>();
  Res->setName(Name);
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
    Res->setType(ResTy);
  }

  if (function->EntryLabel < 0)
    function->EntryLabel = function->CacheLabel;
  if (Blocks.count(function->CacheLabel) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Block->setName(function->CacheName);
    Block->SetLabel(function->CacheLabel);
    Blocks.insert(std::make_pair(function->CacheLabel, Block));
  }

  Blocks[function->CacheLabel]->Insert(VTy, Ptr, IdxList, Res,
                                       IROpKind::Op_GetElementPtr, this);
  return Res;
}

VariablePtr IRBuilder::CreatePHI(VarTypePtr VTy, unsigned NumReservedValues,
                                 const std::string &Name) {
  auto Res = std::make_shared<Variable>();
  Res->setName(Name);
  Res->setType(pointerTo(baseTo(VTy)));

  if (function->EntryLabel < 0)
    function->EntryLabel = function->CacheLabel;
  if (Blocks.count(function->CacheLabel) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Block->setName(function->CacheName);
    Block->SetLabel(function->CacheLabel);
    Blocks.insert(std::make_pair(function->CacheLabel, Block));
  }

  Blocks[function->CacheLabel]->Insert(VTy, NumReservedValues, Res,
                                       IROpKind::Op_PhiNode, this);
  return Res;
}

void IRBuilder::CreateStore(VariablePtr value, VariablePtr Addr) {
  Insert(value, nullptr, Addr, IROpKind::Op_Store, function->GeTable());
}

// void IRBuilder::CreateStore(VariablePtr Addr) { CreateStore(nullptr, Addr); }

bool IRBuilder::Insert(VariablePtr Result, IROpKind Op, SymbolTablePtr Table) {
  // store num
  return Insert(nullptr, nullptr, Result, Op, function->CacheLabel,
                function->CacheName, Table);
}

bool IRBuilder::Insert(VariablePtr Source, VariablePtr Dest, IROpKind Op,
                       SymbolTablePtr Table) {
  // store identity
  return Insert(Source, nullptr, Dest, Op, function->CacheLabel,
                function->CacheName, Table);
}

// branch instruction
bool IRBuilder::Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
                       BasicBlockPtr targetTwo, IROpKind Op,
                       SymbolTablePtr Table) {
  if (function->EntryLabel < 0)
    function->EntryLabel = function->CacheLabel;
  if (Blocks.count(function->CacheLabel) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>(function);
    Block->setName(function->CacheName);
    Block->SetLabel(function->CacheLabel);
    Blocks.insert(std::make_pair(function->CacheLabel, Block));
  }
  assert(Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch);
  Blocks[function->CacheLabel]->Insert(indicateVariable, targetOne, targetTwo,
                                       Op, this);
  return true;
}

void IRBuilder::CreateCondBr(VariablePtr Cond, BasicBlockPtr True,
                             BasicBlockPtr False) {
  auto Curr = getCurrentBlock();
  Curr->SetSucc(True);
  Curr->SetSucc(False);
  True->SetPred(Curr);
  False->SetPred(Curr);
  Insert(Cond, True, False, IROpKind::Op_Branch, function->GeTable());
}

void IRBuilder::CreateBr(BasicBlockPtr Target) {
  auto Curr = getCurrentBlock();
  Curr->SetSucc(Target);
  Target->SetPred(Curr);
  Insert(nullptr, Target, nullptr, IROpKind::Op_UnConBranch,
         function->GeTable());
}

void IRBuilder::fixNonReturn(SymbolTablePtr Table) {
  BasicBlockPtr lastBlock;
  for (const auto &blk : Blocks) {
    lastBlock = blk.second;
  }
  // fix no return statement
  if (lastBlock->InstInBB.empty()) {
    if (function->RetTy == TyVoid) {
      // fix me: remove temp variables
      VariablePtr tempRetVoid = std::make_shared<Variable>();
      tempRetVoid->setName("void");
      auto t1 = std::make_shared<Variable>();
      auto t2 = std::make_shared<Variable>();
      this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, Table);
      // this->Insert(std::make_shared<Variable>(),
      // std::make_shared<Variable>(), tempRetVoid, IROpKind::Op_Return, Table);
    } else {
      auto t1 = std::make_shared<Variable>();
      auto t2 = std::make_shared<Variable>();
      this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, Table);
      // this->Insert(std::make_shared<Variable>(),
      // std::make_shared<Variable>(), std::make_shared<Variable>(lastResVar),
      // IROpKind::Op_Return, Table);
    }

  } else {
    InstructionPtr lastInst = lastBlock->InstInBB.back();
    if (auto d = std::dynamic_pointer_cast<ReturnInst>(lastInst);
        d == nullptr) {
      if (function->RetTy == TyVoid) {
        // fix me: remove temp variable
        VariablePtr tempRetVoid = std::make_shared<Variable>();
        tempRetVoid->setName("void");
        auto t1 = std::make_shared<Variable>();
        auto t2 = std::make_shared<Variable>();
        this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, Table);
      } else {
        auto t1 = std::make_shared<Variable>();
        auto t2 = std::make_shared<Variable>();
        this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, Table);
      }
    }
  }
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