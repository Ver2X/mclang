#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>
#include <string>

std::string IRBuilder::getNextVarName() {
  return "%" + std::to_string(function->nextVarNameNum());
}

int IRBuilder::nextBlockLabelNum() { return function->nextBlockLabelNum(); }

int IRBuilder::nextControlFlowNum() { return function->nextControlFlowNum(); }

void IRBuilder::SetInsertPoint(int Label, std::string Name) {
  if (EntryLabel < 0)
    EntryLabel = Label;
  if (CacheLabel != Label) {
    if (Blocks.count(Label) == 0) {
#if DEBUG
      FileOut << "create a new blcok " << std::endl;
#endif
      BasicBlockPtr Block = std::make_shared<BasicBlock>();
      Block->setName(Name);
      Block->SetLabel(Label);
      Blocks.insert(std::make_pair(Label, Block));
    }
    CacheLabel = Label;
    CacheName = Name;
  }
}

void IRBuilder::SetInsertPoint(BasicBlockPtr insertPoint) {
  if (EntryLabel < 0)
    EntryLabel = insertPoint->GetLabel();
  if (CacheLabel != insertPoint->GetLabel()) {
    if (Blocks.count(insertPoint->GetLabel()) == 0) {
#if DEBUG
      FileOut << "create a new blcok " << std::endl;
#endif
      Blocks.insert(std::make_pair(insertPoint->GetLabel(), insertPoint));
    }
    CacheLabel = insertPoint->GetLabel();
    CacheName = insertPoint->getName();
  }
}

void IRBuilder::InsertBasicBlock(int Label, std::string Name, int Pred) {
  // fix me :
  // keep Pred succ Right
  if (EntryLabel < 0)
    EntryLabel = Label;
  if (Blocks.count(Label) == 0) {
#if DEBUG
    FileOut << "create a new blcok " << std::endl;
#endif

    BasicBlockPtr Block = std::make_shared<BasicBlock>();
    Blocks[Pred]->succes.push_back(Block);
    Block->preds.push_back(Blocks[Pred]);

    Block->setName(Name);
    Block->SetLabel(Label);
    Blocks.insert(std::make_pair(Label, Block));
  }
  CacheLabel = Label;
  CacheName = Name;
}

// extern SymbolTablePtr GlobalSymTable;
bool IRBuilder::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                       IROpKind Op, int Label, std::string Name,
                       SymbolTablePtr Table) {
  if (EntryLabel < 0)
    EntryLabel = Label;
  if (Blocks.count(Label) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>();
    Block->setName(Name);
    Block->SetLabel(Label);
    Blocks.insert(std::make_pair(Label, Block));
  }

  if (Op == IROpKind::Op_Alloca) {
    Blocks[EntryLabel]->Insert(Left, Right, Result, Op, this);
    return true;
  } else {
    Blocks[Label]->Insert(Left, Right, Result, Op, this);
    return true;
  }
  // std::cout << "in ssss" << std::endl;
}

VariablePtr IRBuilder::CreateCall(IRFunctionPtr func,
                                  std::vector<VariablePtr> args) {
  auto Res = std::make_shared<Variable>();
  Res->setName(getNextVarName());
  if (EntryLabel < 0)
    EntryLabel = CacheLabel;
  Blocks[CacheLabel]->Insert(func, args, Res, IROpKind::Op_FUNCALL, this);
  return Res;
  // std::cout << "in ssss" << std::endl;
}

// for Op_Alloca
//
bool IRBuilder::Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
                       IROpKind Op, SymbolTablePtr Table) {
  return Insert(Left, Right, Result, Op, CacheLabel, CacheName, Table);
}

VariablePtr IRBuilder::CreateBinary(VariablePtr Left, VariablePtr Right,
                                    IROpKind Op) {
  auto Res = std::make_shared<Variable>();
  auto rt =
      Insert(Left, Right, Res, Op, CacheLabel, CacheName, function->GeTable());
  assert(rt);
  return Res;
}

VariablePtr IRBuilder::CreateLoad(VariablePtr addr) {
  auto Res = std::make_shared<Variable>();
  Res->setName(getNextVarName());
  auto rt = Insert(addr, nullptr, Res, IROpKind::Op_Load, CacheLabel, CacheName,
                   function->GeTable());
  assert(rt);
  return Res;
}

void IRBuilder::CreateRet(VariablePtr value) {
  Insert(nullptr, nullptr, value, IROpKind::Op_Return, CacheLabel, CacheName,
         function->GeTable());
}

void IRBuilder::CreateAlloca(VariablePtr addr) {
  Insert(nullptr, nullptr, addr, IROpKind::Op_Alloca, function->GeTable());
}

void IRBuilder::CreateStore(VariablePtr value, VariablePtr addr) {
  Insert(value, nullptr, addr, IROpKind::Op_Store, function->GeTable());
}

void IRBuilder::CreateStore(VariablePtr addr) { CreateStore(nullptr, addr); }

bool IRBuilder::Insert(VariablePtr Result, IROpKind Op, SymbolTablePtr Table) {
  // store num
  return Insert(nullptr, nullptr, Result, Op, CacheLabel, CacheName, Table);
}

bool IRBuilder::Insert(VariablePtr source, VariablePtr dest, IROpKind Op,
                       SymbolTablePtr Table) {
  // store identity
  return Insert(source, nullptr, dest, Op, CacheLabel, CacheName, Table);
}

// branch instruction
bool IRBuilder::Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
                       BasicBlockPtr targetTwo, IROpKind Op,
                       SymbolTablePtr Table) {
  if (EntryLabel < 0)
    EntryLabel = CacheLabel;
  if (Blocks.count(CacheLabel) == 0) {
    BasicBlockPtr Block = std::make_shared<BasicBlock>();
    Block->setName(CacheName);
    Block->SetLabel(CacheLabel);
    Blocks.insert(std::make_pair(CacheLabel, Block));
  }
  assert(Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch);
  Blocks[CacheLabel]->Insert(indicateVariable, targetOne, targetTwo, Op, this);
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
  if (lastBlock->instructinos.empty()) {
    if (function->retTy == ReturnTypeKind::RTY_VOID) {
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
    InstructionPtr lastInst = *(lastBlock->instructinos.end() - 1);
    if (auto d = std::dynamic_pointer_cast<ReturnInst>(lastInst);
        d == nullptr) {
      if (function->retTy == ReturnTypeKind::RTY_VOID) {
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

std::string IRBuilder::CodeGen() {
  std::string s;
  if (function != nullptr) {
    s += function->CodeGen();
  }
  s += "{\n";
  BasicBlockPtr lastBlock;
  for (const auto &blk : Blocks) {
#if DEBUG
    FileOut << "dump instructinos in BasicBlock , Name: "
            << blk.second->getName() << "Label: " << blk.second->GetLabel()
            << " Size :" << blk.second->instructinos.Size() << std::endl;
#endif
    if (blk.first == EntryLabel && !blk.second->allocas.empty())
      s += blk.second->AllocaCodeGen();
    s += blk.second->CodeGen();
    lastBlock = blk.second;
  }
  s += "}\n";
  return s;
}

BasicBlockPtr IRBuilder::getCurrentBlock() {
  if (CacheLabel == -1)
    return nullptr;
  return Blocks[CacheLabel];
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
  DepthFirstSearchPP(i, j, Visit, Blocks[EntryLabel]->succes,
                     Blocks[EntryLabel]);
  assert(i == j);
  numofblock = i;
}

void IRBuilder::DepthFirstSearchPP(int &i, int &j,
                                   std::map<BasicBlockPtr, bool> &Visit,
                                   std::vector<BasicBlockPtr> &succ,
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
  for (int i = 0; i < numofblock; i++) {
    s += "\n";
    if (i != 0)
      s += "  " + std::to_string(i) + " [shape = polygon, Label = \" " +
           PreNumToBlock[i]->CodeGenCFG() + "\" ];";
    else
      s += "  " + std::to_string(i) + " [shape = polygon, Label = \" " +
           PreNumToBlock[i]->EntryCodeGenCFG() + "\" ];";
  }
  s += "\n}\n";
  return s;
}