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

VariablePtr genAddrIR(Node *ExpNode, SymbolTablePtr Table) {
  switch (ExpNode->Kind) {
  case ND_VAR: {
    if (ExpNode->Var->IsLocal) {
      // Local variable
      VariablePtr Var = Table->findVar(ExpNode->Var);
      assert(Var);
      if (Var->isArg()) {
        Var = Var->getAddr();
      }
      // std::cout << "find local var pair: " << ExpNode->Var << " to: " << Var
      //           << "\n";
      // std::cout << "find local var: " << Var->CodeGen() << "\n";
      assert(Var->getType()->Kind == TypeKind::TY_PTR);
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
    // array also is dereference
    // std::cout << "start handle defref \n";
    VariablePtr Res;
    assert(ExpNode->Lhs->Kind == ND_POINTER_OFFSET);
    genExprIR(ExpNode->Lhs, &Res, Table);
    // std::cout << "meet deref at: " << Res->CodeGen() << "\n";
    return Res;
  }
  case ND_COMMA: {
    VariablePtr Res;
    genExprIR(ExpNode->Lhs, &Res, Table);
    genAddrIR(ExpNode->Rhs, Table);
    return nullptr;
  }
  case ND_MEMBER: {
    // println("  add $%d, %%rax", ExpNode->member->Offset);
    auto BasePointer = genAddrIR(ExpNode->Lhs, Table);
    std::cout << "BasePointer Ty is: " << BasePointer->CodeGen() << "\n";
    assert(baseTo(BasePointer->getType())->Kind == TypeKind::TY_STRUCT);
    auto Res = InMemoryIR->CreateGEP(
        BasePointer->getType(), BasePointer,
        {std::make_shared<Variable>(0),
         std::make_shared<Variable>(ExpNode->member->Idx)});
    return Res;
  }
  default:
    return nullptr;
  }

  error("not an lvalue");
}
