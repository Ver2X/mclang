#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "mclang.h"
// #include <bits/types/FILE.h>
#include <algorithm>
#include <cassert>
#include <memory>
static void gen_stmt_ir(Node *node, SymbolTablePtr);
static void gen_expr_ir(Node *node, VariablePtr *, SymbolTablePtr);
static void gen_func_arg_ir(Obj *var, int r, int offset, int sz);
extern std::fstream file_out;
extern int depth;

// for now, define as a fucntion, then change to rope data structure.
std::string Twine(std::string &l, std::string &r) { return l + r; }

std::string Twine(std::string l, std::string r) { return l + r; }

std::string Twine(std::string l, int r) {
  if (r == 0)
    return l;
  else
    return l + std::to_string(r);
}

static std::string getPreName(std::string name) { return Twine("%", name); }
std::vector<VariablePtr> argVariableCached;
static std::string getRealPreName(Obj *var) {
  auto checkExistInCahced = [&](std::string name) -> bool {
    for (auto var : argVariableCached) {
      if (var->GetName() == name) {
        return true;
      }
    }
    return false;
  };

  if (checkExistInCahced(Twine(getPreName(var->name), ".addr"))) {
    return Twine(getPreName(var->name), ".addr");
  }
  return Twine("%", var->name);
}

static VaribleKind nodeTypeToVarType(TypeKind ty) {
  if (ty == TY_CHAR) {
    return VaribleKind::VAR_8;
  } else if (ty == TY_SHORT) {
    return VaribleKind::VAR_16;
  } else if (ty == TY_INT) {
    return VaribleKind::VAR_32;
  } else if (ty == TY_LONG) {
    return VaribleKind::VAR_64;
  } else if (ty == TY_VOID) {
    return VaribleKind::VAR_Void;
  } else {
    // TODO: make assert do
    assert(false && "not support now");
    return VaribleKind::VAR_Undefined;
  }
}

std::string NodeKindStrings[] = {
    "ND_ADD",       // +
    "ND_SUB",       // -
    "ND_MUL",       // *
    "ND_DIV",       // /
    "ND_EQ",        // ==
    "ND_NE",        // !=
    "ND_LT",        // <
    "ND_LE",        // <=
    "ND_ASSIGN",    // =
    "ND_COMMA",     // ,
    "ND_NEG",       // -, unary
    "ND_ADDR",      // &, unary
    "ND_DEREF",     // *, unary
    "ND_NUM",       // integer
    "ND_VAR",       // local variable
    "ND_EXPR_STMT", // statement
    "ND_RETURN",    // "return"
    "ND_IF",        // "if"
    "ND_FOR",       // "for" or "while"
    "ND_BLOCK",
    "ND_FUNCALL",   // function call
    "ND_STMT_EXPR", // statement expression
    "ND_MEMBER"     // . (struct member access)
};
std::shared_ptr<IRBuilder> InMemoryIR;
SymbolTablePtr globalSymTable;
std::shared_ptr<Module> ProgramModule = std::make_shared<Module>();

// emit IR
void emit_ir(Obj *prog, std::string file_name) {
  ProgramModule->SetName(file_name);
  globalSymTable = std::make_shared<SymbolTable>();
  emit_global_data_ir(prog);
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_function || !fn->is_definition)
      continue;
    IRFunctionPtr func = globalSymTable->findFunc(fn->name);
    if (!func) {
      func = std::make_shared<IRFunction>();
      func->functionName = fn->name;
    }

    auto local_table = std::make_shared<SymbolTable>(globalSymTable);
    func->setTable(local_table);
    InMemoryIR = std::make_shared<IRBuilder>(func);

    switch (fn->ty->return_ty->kind) {
    case TY_INT:
      func->retTy = ReturnTypeKind::RTY_INT;
      break;
    case TY_CHAR:
      func->retTy = ReturnTypeKind::RTY_CHAR;
      break;
    default:
      func->retTy = ReturnTypeKind::RTY_PTR;
      break;
    }
    // save passed-by-register arguments to the stack
    VariablePtr arg_variable;
    VariablePtr arg_variable_addr;
    // arg, addr of arg
    std::vector<std::tuple<VariablePtr, VariablePtr>> arg_variable_pair;

    /// Cache and release variable
    for (Obj *var = fn->params; var; var = var->next) {
      // file_out << "func args increase 1" << std::endl;
      arg_variable = std::make_shared<Variable>();
      std::cout << "meet type: " << var->ty->kind << "\n";
      arg_variable->type = nodeTypeToVarType(var->ty->kind);
      arg_variable->SetName(getPreName(var->name));
      arg_variable->SetArg();
      local_table->insert(var, arg_variable);
      arg_variable_addr = std::make_shared<Variable>();
      arg_variable_addr->SetName(Twine(getPreName(var->name), ".addr"));
      arg_variable_pair.push_back(std::tuple<VariablePtr, VariablePtr>(
          arg_variable, arg_variable_addr));
      arg_variable->SetAddr(arg_variable_addr);
      func->args.push_back(arg_variable);
      argVariableCached.push_back(arg_variable);
      (func->argsNum)++;
    }

    InMemoryIR->SetFunc(func);

    func->setBody(InMemoryIR);

    // emit code
    InMemoryIR->SetInsertPoint(InMemoryIR->NextBlockLabelNum(), "entry");

    for (auto p : arg_variable_pair) {
      InMemoryIR->CreateAlloca(std::get<1>(p));
      InMemoryIR->CreateStore(std::get<0>(p), std::get<1>(p));
    }

    int NumOfLocal = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      VariablePtr local_variable = std::make_shared<Variable>();
      std::string InitName = getPreName(var->name);
      while (local_table->nameHaveUsed(InitName)) {
        InitName += ".1";
      }
      local_variable->SetName(InitName);
      local_variable->type = nodeTypeToVarType(var->ty->kind);
      InMemoryIR->CreateAlloca(local_variable);
      local_table->insert(var, local_variable);

      NumOfLocal++;
    }
    std::cout << "NumOfLocal: " << NumOfLocal << "\n";

    gen_stmt_ir(fn->body, local_table);
    assert(depth == 0);

    // fix no return statement
    InMemoryIR->FixNonReturn(local_table);
    if (fn == prog)
      file_out << ProgramModule->GlobalVariableCodeGen() << std::endl;
    // file_out << "end global" << std::endl;
    file_out << InMemoryIR->CodeGen() << std::endl;
    std::cout << InMemoryIR->CodeGen() << std::endl;
    InMemoryIR->SetPredAndSuccNum();
    std::fstream cfg_out;
    cfg_out.open(file_name + "_" + fn->name + ".dot", std::ios_base::out);
    cfg_out << InMemoryIR->DumpCFG() << std::endl;
    std::string GenPNGCmd = "dot -Tpng " + file_name + "_" + fn->name +
                            ".dot -o " + file_name + "_" + fn->name + ".png";
    std::cout << "do cmd:\n" << GenPNGCmd << "\n";
    system(GenPNGCmd.c_str());
  }
}

static void gen_stmt_ir(Node *node, SymbolTablePtr table) {
  // println("  .loc 1 %d", node->tok->line_no);
  switch (node->kind) {
  case ND_IF: {
    int loop_id = InMemoryIR->NextControlFlowNum();
    VariablePtr res;

    gen_expr_ir(node->cond, &res, table);
    // insert icmp

    // br i1 %cmp, label %if.then, label %if.else
    BlockPtr InIf = InMemoryIR->GetCurrentBlock();

    BlockPtr Then = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                            Twine("%if.then", loop_id));
    BlockPtr Else = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                            Twine("%if.else", loop_id));

    InMemoryIR->CreateCondBr(InMemoryIR->lastResVar, Then, Else);

    InMemoryIR->SetInsertPoint(Then);

    gen_stmt_ir(node->then, table);

    BlockPtr End = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                           Twine("%if.end", loop_id));
    InMemoryIR->CreateBr(End);
    if (node->els) {
      InMemoryIR->SetInsertPoint(Else);
      gen_stmt_ir(node->els, table);
      InMemoryIR->CreateBr(End);
    }
    InMemoryIR->SetInsertPoint(End);
    return;
  }
  case ND_FOR: // or while
  {
    int loop_id = InMemoryIR->NextControlFlowNum();
    std::string PreName = "%while.";
    if (node->inc) {
      PreName = "%for.";
    }
    BlockPtr InBB = InMemoryIR->GetCurrentBlock();
    // for handle while
    if (node->init) {
      BlockPtr PreHeader =
          std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                  Twine(PreName + "preheader", loop_id));
      InMemoryIR->CreateBr(PreHeader);
      InMemoryIR->SetInsertPoint(PreHeader);
      InBB = PreHeader;
      gen_stmt_ir(node->init, table);
    }

    BlockPtr CondBB = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                              Twine(PreName + "cond", loop_id));
    BlockPtr Body = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                            Twine(PreName + "body", loop_id));
    BlockPtr Exit = std::make_shared<Block>(InMemoryIR->NextBlockLabelNum(),
                                            Twine(PreName + "exit", loop_id));

    InMemoryIR->CreateBr(CondBB);

    InMemoryIR->SetInsertPoint(CondBB);

    if (node->cond) {
      VariablePtr res;
      gen_expr_ir(node->cond, &res, table);
      assert(InMemoryIR->lastResVar != nullptr);
      InMemoryIR->CreateCondBr(InMemoryIR->lastResVar, Body, Exit);
    } else {
      InMemoryIR->CreateBr(Body);
    }

    InMemoryIR->SetInsertPoint(Body);
    gen_stmt_ir(node->then, table);

    if (node->inc) {
      BlockPtr Latch = std::make_shared<Block>(
          InMemoryIR->NextBlockLabelNum(), Twine(PreName + "latch", loop_id));

      InMemoryIR->CreateBr(Latch);
      InMemoryIR->SetInsertPoint(Latch);
      VariablePtr res;
      gen_expr_ir(node->inc, &res, table);
    }

    InMemoryIR->CreateBr(CondBB);

    InMemoryIR->SetInsertPoint(Exit);
    return;
  }
  case ND_BLOCK: {
    // nest
    for (Node *n = node->body; n; n = n->next) {
      gen_stmt_ir(n, table);
    }
    return;
  }
  case ND_RETURN: {
    VariablePtr res;
    gen_expr_ir(node->lhs, &res, table);
    if (node->lhs->kind == ND_VAR) {
      std::cout << "return is a var" << "\n";
      VariablePtr load = InMemoryIR->CreateLoad(res);
      InMemoryIR->CreateRet(load);
    } else {
      std::cout << "return not a var" << "\n";
      InMemoryIR->CreateRet(res);
    }
    return;
  }
  case ND_EXPR_STMT: {
    VariablePtr res = std::make_shared<Variable>();
    gen_expr_ir(node->lhs, &res, table);
    return;
  }
  default:
    return;
  }

  error("invalid statement");
}

static VariablePtr gen_variable_ir(Node *node, SymbolTablePtr table) {
  switch (node->kind) {
  case ND_VAR: {
    if (node->var->is_local) {
      // Local variable
      VariablePtr Var = table->findVar(node->var);
      assert(Var);
      if (Var->isArg()) {
        Var = Var->GetAddr();
      }
      return Var;
    } else {
      // Global variable find in global symbol table
      VariablePtr Var = globalSymTable->findVar(node->var);
      assert(Var);
      return Var;
    }
    return nullptr;
  }
  case ND_DEREF: {
    VariablePtr res;
    gen_expr_ir(node->lhs, &res, table);
    return nullptr;
  }
  case ND_COMMA: {
    VariablePtr res;
    gen_expr_ir(node->lhs, &res, table);
    gen_variable_ir(node->rhs, table);
    return nullptr;
  }
  case ND_MEMBER:
    gen_variable_ir(node->lhs, table);
    // println("  add $%d, %%rax", node->member->offset);
    return nullptr;
  default:
    return nullptr;
  }

  error("not an lvalue");
}

static IROpKind NKindToIRKind(NodeKind Op) {
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

static bool BinaryOperatorExpression(Node *node, VariablePtr *&res,
                                     SymbolTablePtr table, VariablePtr left,
                                     VariablePtr right, NodeKind Op) {
  IROpKind InstructionOp = NKindToIRKind(Op);
  if ((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM)) {
    assert(node->lhs != nullptr);
    assert(node->rhs != nullptr);
    if (node->lhs != nullptr && node->rhs != nullptr) {
      node->kind = ND_NUM;
      switch (Op) {
      case ND_ADD:
        *res = std::make_shared<Variable>(left->Ival + right->Ival);
        break;
      case ND_SUB:
        *res = std::make_shared<Variable>(left->Ival - right->Ival);
        break;
      case ND_MUL:
        *res = std::make_shared<Variable>(left->Ival * right->Ival);
        break;
      case ND_DIV:
        *res = std::make_shared<Variable>(left->Ival / right->Ival);
        break;
      case ND_EQ:
        *res = std::make_shared<Variable>(left->Ival == right->Ival);
        break;
      case ND_NE:
        *res = std::make_shared<Variable>(left->Ival != right->Ival);
        break;
      case ND_LT:
        *res = std::make_shared<Variable>(left->Ival < right->Ival);
        break;
      case ND_LE:
        *res = std::make_shared<Variable>(left->Ival <= right->Ival);
        break;
      default:
        return false;
      }
    }
  } else if (node->lhs->kind == ND_NUM) {
    VariablePtr r = right;
    if (node->rhs->var != nullptr) {
      r = InMemoryIR->CreateLoad(right);
    }

    VariablePtr l = std::make_shared<Variable>(node->lhs->val);
    *res = InMemoryIR->CreateBinary(l, r, InstructionOp);
  } else if (node->rhs->kind == ND_NUM) {
    VariablePtr l = left;
    if (node->lhs->var != nullptr) {
      l = InMemoryIR->CreateLoad(left);
    }

    VariablePtr r = std::make_shared<Variable>(node->rhs->val);
    *res = InMemoryIR->CreateBinary(l, r, InstructionOp);
  } else {
    VariablePtr l = left;
    if (node->lhs->var != nullptr) {
      l = InMemoryIR->CreateLoad(left);
    }
    VariablePtr r = right;
    if (node->rhs->var != nullptr) {
      r = InMemoryIR->CreateLoad(right);
    }
    *res = InMemoryIR->CreateBinary(l, r, InstructionOp);
  }
  // }
  return true;
}

// stack machine
static void gen_expr_ir(Node *node, VariablePtr *res, SymbolTablePtr table) {
  // println("  .loc 1 %d", node->tok->line_no);
  //  left = ...;
  //  right = ...;
  //  then deal them
  // if(node->var)
  //	file_out << "local variable name :" << node->var->name << std::endl;
  switch (node->kind) {
  case ND_NUM:
    // fix me:
    // file_out << "arrive three 2: " << node->val << std::endl;
    *res = std::make_shared<Variable>(node->val);
    // println("  mov $%ld, %%rax", node->val);
    return;
  case ND_NEG:
    gen_expr_ir(node->lhs, res, table);
    // println("  neg %%rax");
    return;
  case ND_VAR:
  case ND_MEMBER:
    *res = gen_variable_ir(node, table);
    // load(node->ty);
    return;
  case ND_DEREF:
    gen_expr_ir(node->lhs, res, table);
    // load(node->ty);
    return;
  case ND_ADDR:
    gen_variable_ir(node->lhs, table);
    return;
  case ND_ASSIGN: {

    VariablePtr left = gen_variable_ir(node->lhs, table);
    // push();
    std::cout << "meet assign : " << "right is: " << node->rhs->kind << "\n";
    gen_expr_ir(node->rhs, res, table);
    VariablePtr local_variable;
    if (left != nullptr) {
      if (!(*res)->isConst) {
        left->isConst = false;
        // fix me: need Store?
        // No

        // insert a load for single variable assign
        assert(node->rhs != nullptr);
        if (node->rhs->kind == ND_VAR) {

          // file_out << "meeting a single Variable assign " << std::endl;
          local_variable = *res;

          auto checkExistInCahced = [&](std::string name) -> bool {
            for (auto var : argVariableCached) {
              if (var->GetName() == name) {
                return true;
              }
            }
            return false;
          };
          VariablePtr load = InMemoryIR->CreateLoad(local_variable);
          InMemoryIR->CreateStore(load, left);
        } else {
          InMemoryIR->CreateStore(*res, left);
        }
      } else {
        left->isConst = false;
        // here need const propagation
        left->isInitConst = true;
        left->Fval = (*res)->Fval;
        left->Ival = (*res)->Ival;
        left->type = (*res)->type;

        // fix me
        InMemoryIR->CreateStore(left);
      }
    }
    // need bind varibale at here
    // store(node->ty);
    return;
  }
  case ND_STMT_EXPR:
    for (Node *n = node->body; n; n = n->next) {
      gen_stmt_ir(n, table);
    }
    return;
  case ND_COMMA:
    gen_expr_ir(node->lhs, res, table);
    gen_expr_ir(node->rhs, res, table);
    return;
  case ND_FUNCALL: {
    int nargs = 0;
    std::vector<VariablePtr> Args;
    for (Node *arg = node->args; arg; arg = arg->next) {
      std::cout << "meet arg: " << nargs << "\n";
      gen_expr_ir(arg, res, table);
      auto NArg = *res;
      if (arg->kind == ND_VAR) {
        std::cout << "ND_VAR:\n";
        VariablePtr load = InMemoryIR->CreateLoad(NArg);
        NArg = load;
      }
      Args.push_back(NArg);
      nargs++;
    }

    auto FunctionName = node->funcname;
    IRFunctionPtr Func = globalSymTable->findFunc(FunctionName);
    VariablePtr call = nullptr;
    if (Func) {
      call = InMemoryIR->CreateCall(Func, Args);
    } else {
      // TODO:
      // generate declare?
      Func = std::make_shared<IRFunction>(FunctionName);
      auto insertFuncRes = globalSymTable->insertFunc(Func);
      assert(insertFuncRes);
      if (node->ty == ty_int) {
        Func->retTy = ReturnTypeKind::RTY_INT;
      } else {
        Func->retTy = ReturnTypeKind::RTY_VOID;
      }
      call = InMemoryIR->CreateCall(Func, Args);
    }
    *res = call;
    return;
  }
  default:
    break;
  }

  // there must deal rhs first
  // file_out << "arrive three 6" << std::endl;
  VariablePtr right = std::make_shared<Variable>();
  gen_expr_ir(node->rhs, &right, table);
  std::cout << "right is: " << right->GetName() << "\n";
  // push();
  // file_out << "arrive three 5" << std::endl;
  VariablePtr left = std::make_shared<Variable>();
  gen_expr_ir(node->lhs, &left, table);
  std::cout << "left is: " << left->GetName() << "\n";
  // file_out << "arrive three 4" << std::endl;
  // pop("%rdi");

  assert(left != nullptr && right != nullptr);
  assert(node->lhs != nullptr && node->rhs != nullptr);
  assert(table != nullptr);

  if (!BinaryOperatorExpression(node, res, table, left, right, node->kind)) {
    error("invalid expression");
  }
}

// emit global data
void emit_global_data_ir(Obj *prog) {
  std::vector<Obj *> vars;
  for (Obj *var = prog; var; var = var->next) {
    if (var->is_function)
      continue;
    vars.push_back(var);
  }
  auto handleGlobalVariable = [&](Obj *var) {
    VariablePtr GlobalValue;
    if (var->init_data) {
      GlobalValue = std::make_shared<Variable>(var->init_data[0]);
      GlobalValue->SetGlobal();
      GlobalValue->SetName(var->name);
      ProgramModule->InsertGlobalVariable(GlobalValue);
      // for(int i = 0; i < var->ty->size;i++)
      //	println("  .byte %d", var->init_data[i]);
    } else {
      GlobalValue = std::make_shared<Variable>(0);
      GlobalValue->SetGlobal();
      GlobalValue->SetName(var->name);
      ProgramModule->InsertGlobalVariable(GlobalValue);
      // println("  .zero %d", var->ty->size);
    }
    globalSymTable->insert(var, GlobalValue);
  };
  auto handle = std::for_each(vars.rbegin(), vars.rend(), handleGlobalVariable);
}