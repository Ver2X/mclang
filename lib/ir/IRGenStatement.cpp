#include "front/mclang.h"
#include "ir/IRBuilder.h"
#include "ir/Instruction.h"
#include "ir/Module.h"
#include "ir/SymbolTable.h"
#include "ir/Value.h"

void genExprIR(Node *ExpNode, VariablePtr *, SymbolTablePtr);
void genFuncArgIR(Obj *Var, int r, int Offset, int sz);
std::string Twine(std::string &l, std::string &r);
std::string Twine(std::string l, std::string r);
std::string Twine(std::string l, int r);

extern std::shared_ptr<IRBuilder> InMemoryIR;
extern SymbolTablePtr GlobalSymTable;
extern std::shared_ptr<Module> ProgramModule;

void genStmtIR(Node *ExpNode, SymbolTablePtr Table) {
  // println("  .loc 1 %d", ExpNode->Tok->line_no);
  switch (ExpNode->Kind) {
  case ND_IF: {
    int LoopID = InMemoryIR->nextControlFlowNum();
    VariablePtr Cond;

    genExprIR(ExpNode->cond, &Cond, Table);
    // insert icmp

    // br i1 %cmp, Label %if.then, Label %if.else
    BasicBlockPtr InIf = InMemoryIR->getCurrentBlock();

    BasicBlockPtr Then = std::make_shared<BasicBlock>(
        InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
        Twine("%if.then", LoopID));
    BasicBlockPtr Else = nullptr;
    if (ExpNode->els) {
      Else = std::make_shared<BasicBlock>(InMemoryIR->getParent(),
                                          InMemoryIR->nextBlockLabelNum(),
                                          Twine("%if.else", LoopID));
    }
    BasicBlockPtr End = std::make_shared<BasicBlock>(
        InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
        Twine("%if.end", LoopID));
    if (ExpNode->els) {
      InMemoryIR->CreateCondBr(Cond, Then, Else);
    } else {
      InMemoryIR->CreateCondBr(Cond, Then, End);
    }
    InMemoryIR->SetInsertPoint(Then);

    genStmtIR(ExpNode->then, Table);

    InMemoryIR->CreateBr(End);
    if (ExpNode->els) {
      InMemoryIR->SetInsertPoint(Else);
      genStmtIR(ExpNode->els, Table);
      InMemoryIR->CreateBr(End);
    }
    InMemoryIR->SetInsertPoint(End);
    return;
  }
  case ND_FOR: // or while
  {
    int LoopID = InMemoryIR->nextControlFlowNum();
    std::string PreName = "%while.";
    if (ExpNode->inc) {
      PreName = "%for.";
    }
    BasicBlockPtr InBB = InMemoryIR->getCurrentBlock();
    // for handle while
    if (ExpNode->init) {
      BasicBlockPtr PreHeader = std::make_shared<BasicBlock>(
          InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
          Twine(PreName + "preheader", LoopID));
      InMemoryIR->CreateBr(PreHeader);
      InMemoryIR->SetInsertPoint(PreHeader);
      InBB = PreHeader;
      genStmtIR(ExpNode->init, Table);
    }

    BasicBlockPtr CondBB = std::make_shared<BasicBlock>(
        InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
        Twine(PreName + "cond", LoopID));
    BasicBlockPtr Body = std::make_shared<BasicBlock>(
        InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
        Twine(PreName + "body", LoopID));
    BasicBlockPtr Latch = nullptr;
    if (ExpNode->inc) {
      Latch = std::make_shared<BasicBlock>(InMemoryIR->getParent(),
                                           InMemoryIR->nextBlockLabelNum(),
                                           Twine(PreName + "latch", LoopID));
    }
    BasicBlockPtr Exit = std::make_shared<BasicBlock>(
        InMemoryIR->getParent(), InMemoryIR->nextBlockLabelNum(),
        Twine(PreName + "exit", LoopID));

    InMemoryIR->CreateBr(CondBB);

    InMemoryIR->SetInsertPoint(CondBB);

    if (ExpNode->cond) {
      VariablePtr Cond;
      genExprIR(ExpNode->cond, &Cond, Table);
      InMemoryIR->CreateCondBr(Cond, Body, Exit);
    } else {
      InMemoryIR->CreateBr(Body);
    }

    InMemoryIR->SetInsertPoint(Body);
    genStmtIR(ExpNode->then, Table);

    if (ExpNode->inc) {
      //   BasicBlockPtr Latch = std::make_shared<BasicBlock>(
      //       InMemoryIR->nextBlockLabelNum(), Twine(PreName + "latch",
      //       LoopID));

      InMemoryIR->CreateBr(Latch);
      InMemoryIR->SetInsertPoint(Latch);
      VariablePtr Res;
      genExprIR(ExpNode->inc, &Res, Table);
    }

    InMemoryIR->CreateBr(CondBB);

    InMemoryIR->SetInsertPoint(Exit);
    return;
  }
  case ND_BLOCK: {
    // nest
    for (Node *N = ExpNode->Body; N; N = N->Next) {
      genStmtIR(N, Table);
    }
    return;
  }
  case ND_RETURN: {
    VariablePtr Res;
    genExprIR(ExpNode->Lhs, &Res, Table);
    if (ExpNode->Lhs->Kind == ND_VAR) {
      //   std::cout << "return is a Var" << "\n";
      VariablePtr load = InMemoryIR->CreateLoad(Res);
      InMemoryIR->CreateRet(load);
    } else {
      //   std::cout << "return not a Var" << "\n";
      InMemoryIR->CreateRet(Res);
    }
    return;
  }
  case ND_EXPR_STMT: {
    VariablePtr Res = nullptr;
    genExprIR(ExpNode->Lhs, &Res, Table);
    assert(Res);
    return;
  }
  default:
    return;
  }

  error("invalid statement");
}
