#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "mclang.h"

void genExprIR(Node *ExprNode, VariablePtr *, SymbolTablePtr);
void genFuncArgIR(Obj *Var, int r, int Offset, int sz);
VariablePtr genVariableIR(Node *ExprNode, SymbolTablePtr Table);
void genStmtIR(Node *ExprNode, SymbolTablePtr Table);

std::string Twine(std::string &l, std::string &r);
std::string Twine(std::string l, std::string r);
std::string Twine(std::string l, int r);

extern std::shared_ptr<IRBuilder> InMemoryIR;
extern SymbolTablePtr GlobalSymTable;
extern std::shared_ptr<Module> ProgramModule;
extern std::vector<VariablePtr> argVariableCached;

IROpKind NKindToIRKind(NodeKind Op) {
  switch (Op) {
  case ND_ADD:
    return IROpKind::Op_ADD;
  case ND_SUB:
    return IROpKind::Op_SUB;
  case ND_MUL:
    return IROpKind::Op_MUL;
  case ND_DIV:
    return IROpKind::Op_DIV;
  case ND_EQ:
    return IROpKind::Op_EQ;
  case ND_NE:
    return IROpKind::Op_NE;
  case ND_LT:
    return IROpKind::Op_SLT;
  case ND_LE:
    return IROpKind::Op_SLE;
  default:
    return IROpKind::Op_RESERVED;
  }
}

bool binaryOperatorExpression(Node *ExprNode, VariablePtr *&Res,
                              SymbolTablePtr Table, VariablePtr Left,
                              VariablePtr Right, NodeKind Op) {
  IROpKind InstructionOp = NKindToIRKind(Op);
  if ((ExprNode->Lhs->Kind == ND_NUM) && (ExprNode->Rhs->Kind == ND_NUM)) {
    assert(ExprNode->Lhs != nullptr);
    assert(ExprNode->Rhs != nullptr);
    if (ExprNode->Lhs != nullptr && ExprNode->Rhs != nullptr) {
      ExprNode->Kind = ND_NUM;
      switch (Op) {
      case ND_ADD:
        *Res = std::make_shared<Variable>(Left->Ival + Right->Ival);
        break;
      case ND_SUB:
        *Res = std::make_shared<Variable>(Left->Ival - Right->Ival);
        break;
      case ND_MUL:
        *Res = std::make_shared<Variable>(Left->Ival * Right->Ival);
        break;
      case ND_DIV:
        *Res = std::make_shared<Variable>(Left->Ival / Right->Ival);
        break;
      case ND_EQ:
        *Res = std::make_shared<Variable>(Left->Ival == Right->Ival);
        break;
      case ND_NE:
        *Res = std::make_shared<Variable>(Left->Ival != Right->Ival);
        break;
      case ND_LT:
        *Res = std::make_shared<Variable>(Left->Ival < Right->Ival);
        break;
      case ND_LE:
        *Res = std::make_shared<Variable>(Left->Ival <= Right->Ival);
        break;
      default:
        return false;
      }
    }
  } else if (ExprNode->Lhs->Kind == ND_NUM) {
    VariablePtr R = Right;
    if (ExprNode->Rhs->Var != nullptr) {
      R = InMemoryIR->CreateLoad(Right);
    }

    VariablePtr L = std::make_shared<Variable>(ExprNode->Lhs->Val);
    *Res = InMemoryIR->CreateBinary(L, R, InstructionOp);
  } else if (ExprNode->Rhs->Kind == ND_NUM) {
    VariablePtr L = Left;
    if (ExprNode->Lhs->Var != nullptr) {
      L = InMemoryIR->CreateLoad(Left);
    }

    VariablePtr R = std::make_shared<Variable>(ExprNode->Rhs->Val);
    *Res = InMemoryIR->CreateBinary(L, R, InstructionOp);
  } else {
    VariablePtr L = Left;
    if (ExprNode->Lhs->Var != nullptr) {
      L = InMemoryIR->CreateLoad(Left);
    }
    VariablePtr R = Right;
    if (ExprNode->Rhs->Var != nullptr) {
      R = InMemoryIR->CreateLoad(Right);
    }
    *Res = InMemoryIR->CreateBinary(L, R, InstructionOp);
  }
  // }
  return true;
}

// stack machine
void genExprIR(Node *ExprNode, VariablePtr *Res, SymbolTablePtr Table) {
  // println("  .loc 1 %d", ExprNode->Tok->line_no);
  //  Left = ...;
  //  Right = ...;
  //  then deal them
  // if(ExprNode->Var)
  //	FileOut << "local variable Name :" << ExprNode->Var->Name << std::endl;
  switch (ExprNode->Kind) {
  case ND_NUM:
    // fix me:
    // FileOut << "arrive three 2: " << ExprNode->Val << std::endl;
    *Res = std::make_shared<Variable>(ExprNode->Val);
    // println("  mov $%ld, %%rax", ExprNode->Val);
    return;
  case ND_NEG:
    genExprIR(ExprNode->Lhs, Res, Table);
    // println("  neg %%rax");
    return;
  case ND_VAR:
  case ND_MEMBER:
    *Res = genVariableIR(ExprNode, Table);
    // load(ExprNode->Ty);
    return;
  case ND_DEREF:
    genExprIR(ExprNode->Lhs, Res, Table);
    // load(ExprNode->Ty);
    return;
  case ND_ADDR:
    genVariableIR(ExprNode->Lhs, Table);
    return;
  case ND_ASSIGN: {

    VariablePtr Left = genVariableIR(ExprNode->Lhs, Table);
    // push();
    std::cout << "meet assign : " << "Right is: " << ExprNode->Rhs->Kind
              << "\n";
    genExprIR(ExprNode->Rhs, Res, Table);
    VariablePtr LocalVar;
    if (Left != nullptr) {
      if (!(*Res)->isConst) {
        Left->isConst = false;
        // fix me: need Store?
        // No

        // insert a Load for single variable assign
        assert(ExprNode->Rhs != nullptr);
        if (ExprNode->Rhs->Kind == ND_VAR) {

          // FileOut << "meeting a single Variable assign " << std::endl;
          LocalVar = *Res;

          auto checkExistInCahced = [&](std::string Name) -> bool {
            for (auto Var : argVariableCached) {
              if (Var->getName() == Name) {
                return true;
              }
            }
            return false;
          };
          VariablePtr Load = InMemoryIR->CreateLoad(LocalVar);
          InMemoryIR->CreateStore(Load, Left);
        } else {
          InMemoryIR->CreateStore(*Res, Left);
        }
      } else {
        Left->isConst = false;
        // here need const propagation
        Left->isInitConst = true;
        Left->Fval = (*Res)->Fval;
        Left->Ival = (*Res)->Ival;
        Left->type = (*Res)->type;

        // fix me
        InMemoryIR->CreateStore(Left);
      }
    }
    // need bind varibale at here
    // store(ExprNode->Ty);
    return;
  }
  case ND_STMT_EXPR:
    for (Node *n = ExprNode->Body; n; n = n->Next) {
      genStmtIR(n, Table);
    }
    return;
  case ND_COMMA:
    genExprIR(ExprNode->Lhs, Res, Table);
    genExprIR(ExprNode->Rhs, Res, Table);
    return;
  case ND_FUNCALL: {
    int Nargs = 0;
    std::vector<VariablePtr> Args;
    for (Node *Arg = ExprNode->args; Arg; Arg = Arg->Next) {
      std::cout << "meet Arg: " << Nargs << "\n";
      genExprIR(Arg, Res, Table);
      auto NArg = *Res;
      if (Arg->Kind == ND_VAR) {
        std::cout << "ND_VAR:\n";
        VariablePtr load = InMemoryIR->CreateLoad(NArg);
        NArg = load;
      }
      Args.push_back(NArg);
      Nargs++;
    }

    auto FunctionName = ExprNode->funcname;
    IRFunctionPtr Func = GlobalSymTable->findFunc(FunctionName);
    VariablePtr Call = nullptr;
    if (Func) {
      Call = InMemoryIR->CreateCall(Func, Args);
    } else {
      // TODO:
      // generate declare?
      Func = std::make_shared<IRFunction>(FunctionName);
      auto insertFuncRes = GlobalSymTable->insertFunc(Func);
      assert(insertFuncRes);
      if (ExprNode->Ty == TyInt) {
        Func->retTy = ReturnTypeKind::RTY_INT;
      } else {
        Func->retTy = ReturnTypeKind::RTY_VOID;
      }
      Call = InMemoryIR->CreateCall(Func, Args);
    }
    *Res = Call;
    return;
  }
  default:
    break;
  }

  // there must deal Rhs first
  // FileOut << "arrive three 6" << std::endl;
  VariablePtr Right = std::make_shared<Variable>();
  genExprIR(ExprNode->Rhs, &Right, Table);
  std::cout << "Right is: " << Right->getName() << "\n";
  // push();
  // FileOut << "arrive three 5" << std::endl;
  VariablePtr Left = std::make_shared<Variable>();
  genExprIR(ExprNode->Lhs, &Left, Table);
  std::cout << "Left is: " << Left->getName() << "\n";
  // FileOut << "arrive three 4" << std::endl;
  // pop("%rdi");

  assert(Left != nullptr && Right != nullptr);
  assert(ExprNode->Lhs != nullptr && ExprNode->Rhs != nullptr);
  assert(Table != nullptr);

  if (!binaryOperatorExpression(ExprNode, Res, Table, Left, Right,
                                ExprNode->Kind)) {
    error("invalid expression");
  }
}
