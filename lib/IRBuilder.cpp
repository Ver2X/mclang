#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>
#include <string>

std::string IRBuilder::GetNextVarName() {
  return "%" + std::to_string(function->NextVarNameNum());
}

int IRBuilder::NextBlockLabelNum() { return function->NextBlockLabelNum(); }

int IRBuilder::NextControlFlowNum() { return function->NextControlFlowNum(); }

void IRBuilder::SetInsertPoint(int label, std::string name) {
  if (entry_label < 0)
    entry_label = label;
  if (cache_label != label) {
    if (blocks.count(label) == 0) {
#if DEBUG
      file_out << "create a new blcok " << std::endl;
#endif
      BlockPtr block = std::make_shared<Block>();
      block->SetName(name);
      block->SetLabel(label);
      blocks.insert(std::make_pair(label, block));
    }
    cache_label = label;
    cache_name = name;
  }
}

void IRBuilder::SetInsertPoint(BlockPtr insertPoint) {
  if (entry_label < 0)
    entry_label = insertPoint->GetLabel();
  if (cache_label != insertPoint->GetLabel()) {
    if (blocks.count(insertPoint->GetLabel()) == 0) {
#if DEBUG
      file_out << "create a new blcok " << std::endl;
#endif
      blocks.insert(std::make_pair(insertPoint->GetLabel(), insertPoint));
    }
    cache_label = insertPoint->GetLabel();
    cache_name = insertPoint->GetName();
  }
}

void IRBuilder::InsertBasicBlock(int label, std::string name, int pred) {
  // fix me :
  // keep pred succ right
  if (entry_label < 0)
    entry_label = label;
  if (blocks.count(label) == 0) {
#if DEBUG
    file_out << "create a new blcok " << std::endl;
#endif

    BlockPtr block = std::make_shared<Block>();
    blocks[pred]->succes.push_back(block);
    block->preds.push_back(blocks[pred]);

    block->SetName(name);
    block->SetLabel(label);
    blocks.insert(std::make_pair(label, block));
  }
  cache_label = label;
  cache_name = name;
}

// extern SymbolTablePtr globalSymTable;
bool IRBuilder::Insert(VariablePtr left, VariablePtr right, VariablePtr result,
                       IROpKind Op, int label, std::string name,
                       SymbolTablePtr table) {
  if (entry_label < 0)
    entry_label = label;
  if (blocks.count(label) == 0) {
    BlockPtr block = std::make_shared<Block>();
    block->SetName(name);
    block->SetLabel(label);
    blocks.insert(std::make_pair(label, block));
  }

  if (Op == IROpKind::Op_Alloca) {
    blocks[entry_label]->Insert(left, right, result, Op, this);
    return true;
  } else {
    blocks[label]->Insert(left, right, result, Op, this);
    return true;
  }
  // std::cout << "in ssss" << std::endl;
}

VariablePtr IRBuilder::CreateCall(IRFunctionPtr func,
                                  std::vector<VariablePtr> args) {
  auto res = std::make_shared<Variable>();
  res->SetName(GetNextVarName());
  if (entry_label < 0)
    entry_label = cache_label;
  blocks[cache_label]->Insert(func, args, res, IROpKind::Op_FUNCALL, this);
  return res;
  // std::cout << "in ssss" << std::endl;
}

// for Op_Alloca
//
bool IRBuilder::Insert(VariablePtr left, VariablePtr right, VariablePtr result,
                       IROpKind Op, SymbolTablePtr table) {
  return Insert(left, right, result, Op, cache_label, cache_name, table);
}

VariablePtr IRBuilder::CreateBinary(VariablePtr left, VariablePtr right,
                                    IROpKind Op) {
  auto res = std::make_shared<Variable>();
  auto rt = Insert(left, right, res, Op, cache_label, cache_name,
                   function->GeTable());
  assert(rt);
  return res;
}

VariablePtr IRBuilder::CreateLoad(VariablePtr addr) {
  auto res = std::make_shared<Variable>();
  res->SetName(GetNextVarName());
  auto rt = Insert(addr, nullptr, res, IROpKind::Op_Load, cache_label,
                   cache_name, function->GeTable());
  assert(rt);
  return res;
}

void IRBuilder::CreateRet(VariablePtr value) {
  Insert(nullptr, nullptr, value, IROpKind::Op_Return, cache_label, cache_name,
         function->GeTable());
}

void IRBuilder::CreateAlloca(VariablePtr addr) {
  Insert(nullptr, nullptr, addr, IROpKind::Op_Alloca, function->GeTable());
}

void IRBuilder::CreateStore(VariablePtr value, VariablePtr addr) {
  Insert(value, nullptr, addr, IROpKind::Op_Store, function->GeTable());
}

void IRBuilder::CreateStore(VariablePtr addr) { CreateStore(nullptr, addr); }

bool IRBuilder::Insert(VariablePtr result, IROpKind Op, SymbolTablePtr table) {
  // store num
  return Insert(nullptr, nullptr, result, Op, cache_label, cache_name, table);
}

bool IRBuilder::Insert(VariablePtr source, VariablePtr dest, IROpKind Op,
                       SymbolTablePtr table) {
  // store identity
  return Insert(source, nullptr, dest, Op, cache_label, cache_name, table);
}

// branch instruction
bool IRBuilder::Insert(VariablePtr indicateVariable, BlockPtr targetOne,
                       BlockPtr targetTwo, IROpKind Op, SymbolTablePtr table) {
  if (entry_label < 0)
    entry_label = cache_label;
  if (blocks.count(cache_label) == 0) {
    BlockPtr block = std::make_shared<Block>();
    block->SetName(cache_name);
    block->SetLabel(cache_label);
    blocks.insert(std::make_pair(cache_label, block));
  }
  assert(Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch);
  blocks[cache_label]->Insert(indicateVariable, targetOne, targetTwo, Op, this);
  return true;
}

void IRBuilder::CreateCondBr(VariablePtr Cond, BlockPtr True, BlockPtr False) {
  auto Curr = GetCurrentBlock();
  Curr->SetSucc(True);
  Curr->SetSucc(False);
  True->SetPred(Curr);
  False->SetPred(Curr);
  Insert(Cond, True, False, IROpKind::Op_Branch, function->GeTable());
}

void IRBuilder::CreateBr(BlockPtr Target) {
  auto Curr = GetCurrentBlock();
  Curr->SetSucc(Target);
  Target->SetPred(Curr);
  Insert(nullptr, Target, nullptr, IROpKind::Op_UnConBranch,
         function->GeTable());
}

void IRBuilder::FixNonReturn(SymbolTablePtr table) {
  BlockPtr lastBlock;
  for (const auto &blk : blocks) {
    lastBlock = blk.second;
  }
  // fix no return statement
  if (lastBlock->instructinos.empty()) {
    if (function->retTy == ReturnTypeKind::RTY_VOID) {
      // fix me: remove temp variables
      VariablePtr tempRetVoid = std::make_shared<Variable>();
      tempRetVoid->SetName("void");
      auto t1 = std::make_shared<Variable>();
      auto t2 = std::make_shared<Variable>();
      this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, table);
      // this->Insert(std::make_shared<Variable>(),
      // std::make_shared<Variable>(), tempRetVoid, IROpKind::Op_Return, table);
    } else {
      auto t1 = std::make_shared<Variable>();
      auto t2 = std::make_shared<Variable>();
      this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, table);
      // this->Insert(std::make_shared<Variable>(),
      // std::make_shared<Variable>(), std::make_shared<Variable>(lastResVar),
      // IROpKind::Op_Return, table);
    }

  } else {
    InstructionPtr lastInst = *(lastBlock->instructinos.end() - 1);
    if (auto d = std::dynamic_pointer_cast<ReturnInst>(lastInst);
        d == nullptr) {
      if (function->retTy == ReturnTypeKind::RTY_VOID) {
        // fix me: remove temp variable
        VariablePtr tempRetVoid = std::make_shared<Variable>();
        tempRetVoid->SetName("void");
        auto t1 = std::make_shared<Variable>();
        auto t2 = std::make_shared<Variable>();
        this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, table);
      } else {
        auto t1 = std::make_shared<Variable>();
        auto t2 = std::make_shared<Variable>();
        this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, table);
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
  BlockPtr lastBlock;
  for (const auto &blk : blocks) {
#if DEBUG
    file_out << "dump instructinos in block , name: " << blk.second->GetName()
             << "label: " << blk.second->GetLabel()
             << " size :" << blk.second->instructinos.size() << std::endl;
#endif
    if (blk.first == entry_label && !blk.second->allocas.empty())
      s += blk.second->AllocaCodeGen();
    s += blk.second->CodeGen();
    lastBlock = blk.second;
  }
  s += "}\n";
  return s;
}

BlockPtr IRBuilder::GetCurrentBlock() {
  if (cache_label == -1)
    return nullptr;
  return blocks[cache_label];
}

void IRBuilder::SetPredAndSuccNum() {
  std::map<BlockPtr, bool> Visit;
  BlockPtr current;
  for (auto pr : blocks) {
    current = pr.second;
    Visit[current] = false;
  }
  int i, j;
  i = 0;
  j = 0;
  DepthFirstSearchPP(i, j, Visit, blocks[entry_label]->succes,
                     blocks[entry_label]);
  assert(i == j);
  numofblock = i;
}

void IRBuilder::DepthFirstSearchPP(int &i, int &j,
                                   std::map<BlockPtr, bool> &Visit,
                                   std::vector<BlockPtr> &succ, BlockPtr x) {
  // BlockPtr y;
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
          2 [shape=polygon,sides=4,label="hello world"]
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
      s += "  " + std::to_string(i) + " [shape = polygon, label = \" " +
           PreNumToBlock[i]->CodeGenCFG() + "\" ];";
    else
      s += "  " + std::to_string(i) + " [shape = polygon, label = \" " +
           PreNumToBlock[i]->EntryCodeGenCFG() + "\" ];";
  }
  s += "\n}\n";
  return s;
}