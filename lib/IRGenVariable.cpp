#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "mclang.h"

void genExprIR(Node *ExpNode, VariablePtr *, SymbolTablePtr);
void genFuncArgIR(Obj *Var, int r, int Offset, int sz);
std::string Twine(std::string &l, std::string &r);
std::string Twine(std::string l, std::string r);
std::string Twine(std::string l, int r);

extern std::shared_ptr<IRBuilder> InMemoryIR;
extern SymbolTablePtr GlobalSymTable;
extern std::shared_ptr<Module> ProgramModule;

VariablePtr genVariableIR(Node *ExpNode, SymbolTablePtr Table) {
  switch (ExpNode->Kind) {
  case ND_VAR: {
    if (ExpNode->Var->IsLocal) {
      // Local variable
      VariablePtr Var = Table->findVar(ExpNode->Var);
      assert(Var);
      if (Var->isArg()) {
        Var = Var->getAddr();
      }
      return Var;
    } else {
      // Global variable find in global symbol Table
      VariablePtr Var = GlobalSymTable->findVar(ExpNode->Var);
      assert(Var);
      return Var;
    }
    return nullptr;
  }
  case ND_DEREF: {
    VariablePtr Res;
    genExprIR(ExpNode->Lhs, &Res, Table);
    return nullptr;
  }
  case ND_COMMA: {
    VariablePtr Res;
    genExprIR(ExpNode->Lhs, &Res, Table);
    genVariableIR(ExpNode->Rhs, Table);
    return nullptr;
  }
  case ND_MEMBER:
    genVariableIR(ExpNode->Lhs, Table);
    // println("  add $%d, %%rax", ExpNode->member->Offset);
    return nullptr;
  default:
    return nullptr;
  }

  error("not an lvalue");
}
