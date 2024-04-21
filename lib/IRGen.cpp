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

extern int count_diff_if;
// use to identify diff "if" statements
static int count() { return count_diff_if++; }

// for now, define as a fucntion, then change to rope data structure.
std::string Twine(std::string &l, std::string &r) { return l + r; }

std::string Twine(std::string l, std::string r) { return l + r; }

std::string Twine(std::string l, int r) {
  if (r == 0)
    return l;
  else
    return l + std::to_string(r);
}

static int next_variable_name_v = 0;
static int next_variable_name_number() { return next_variable_name_v++; }

// use to count number of if, same group have the same number
static int next_label_name_v = 0;
static int next_label_name_number() { return next_label_name_v++; }

// numbering label, all blocks have different label
static int next_label_num_v = 0;
static int next_label_num_number() { return next_label_num_v++; }

std::string next_variable_name() {
  std::string name = Twine("%", std::to_string(next_variable_name_number()));
  return name;
  // return std::to_string(next_variable_name_number());
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

// IRBuilder InMemoryIR;
std::shared_ptr<IRBuilder> InMemoryIR; // = std::make_shared<IRBuilder>();
SymbolTablePtr symTable;               // = std::make_shared<SymbolTable>();
std::shared_ptr<Module> ProgramModule = std::make_shared<Module>();

// emit IR
void emit_ir(Obj *prog, std::string file_name) {
  symTable = std::make_shared<SymbolTable>();
  emit_global_data_ir(prog);
  for (Obj *fn = prog; fn; fn = fn->next) {
    InMemoryIR = std::make_shared<IRBuilder>();
    next_variable_name_v = 0;
    if (!fn->is_function || !fn->is_definition)
      continue;
    IRFunctionPtr func = std::make_shared<IRFunction>();
    func->functionName = fn->name;

    // prologue
    // println("  push %%rbp");
    // println("  mov %%rsp, %%rbp");
    // println("  sub $%d, %%rsp", fn->stack_size);

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
    // int i = 0;
    // VariablePtr head = nullptr;
    VariablePtr arg_variable;
    VariablePtr arg_variable_addr;
    // arg, addr of arg
    std::vector<std::tuple<VariablePtr, VariablePtr>> arg_variable_pair;
    auto local_table = std::make_shared<SymbolTable>(symTable);

    /// Cache and release variable
    for (Obj *var = fn->params; var; var = var->next) {
      // file_out << "func args increase 1" << std::endl;
      arg_variable = std::make_shared<Variable>();
      // next_variable_name_number();
      switch (var->ty->size) {

      case 1: {
        // var->name
        arg_variable->type = VaribleKind::VAR_8;
        break;
      }
      case 2: {
        arg_variable->type = VaribleKind::VAR_16;
        break;
      }
      case 4: {
        arg_variable->type = VaribleKind::VAR_32;
        break;
      }
      case 8: {
        arg_variable->type = VaribleKind::VAR_64;
        break;
      }
      default:
        break;
      } // end switch
      arg_variable->SetName(getPreName(var->name));
      arg_variable->SetArg();
      local_table->insert(arg_variable, 0);
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
    // emit code

    InMemoryIR->SetInsertPoint(next_label_num_number(), "entry");

    for (auto p : arg_variable_pair) {
      // std::get<1>(p) = arg.addr
      bool insertRes = InMemoryIR->Insert(nullptr, nullptr, std::get<1>(p),
                                          IROpKind::Op_Alloca, local_table);
      assert(insertRes == true);
      // std::get<0>(p) = arg
      insertRes = InMemoryIR->Insert(std::get<0>(p), nullptr, std::get<1>(p),
                                     IROpKind::Op_Store, local_table);
      assert(insertRes == true);
      local_table->insert(std::get<1>(p), 0);
    }
    ////////// maybe need generate Block first
    // file_out << "arrive three 7" << std::endl;
    //  std::stack<Obj *> local_variables;
    std::vector<Obj *> local_variables;
    for (Obj *var = fn->locals; var; var = var->next) {
      // local_variables.push(var);
      local_variables.push_back(var);
    }
    auto handleVariableDeclaration = [&](Obj *var) {
      VariablePtr local_variable = std::make_shared<Variable>();
      local_variable->SetName(getPreName(var->name));
      local_variable->type = VaribleKind::VAR_32;
      if (InMemoryIR->Insert(nullptr, nullptr, local_variable,
                             IROpKind::Op_Alloca, local_table)) {
        local_table->insert(local_variable, 0);
      }
    };
    std::for_each(local_variables.rbegin(), local_variables.rend(),
                  handleVariableDeclaration);
    // handleVariableDeclarationRes();

    gen_stmt_ir(fn->body, local_table);
    assert(depth == 0);

    // fix no return statement
    InMemoryIR->FixNonReturn(local_table);
    // file_out << "begin global: " << std::endl;
    // file_out << "ProgramModule size: "<<
    // ProgramModule->globalVariables.size() << std::endl;
    ProgramModule->SetName(file_name);
    file_out << ProgramModule->GlobalVariableCodeGen() << std::endl;
    // file_out << "end global" << std::endl;
    file_out << InMemoryIR->CodeGen() << std::endl;
    std::cout << InMemoryIR->CodeGen() << std::endl;
    InMemoryIR->SetPredAndSuccNum();
    std::fstream cfg_out;
    cfg_out.open(file_name + ".dot", std::ios_base::out);
    cfg_out << InMemoryIR->DumpCFG();
  }
}

static void gen_stmt_ir(Node *node, SymbolTablePtr table) {
  // println("  .loc 1 %d", node->tok->line_no);
  switch (node->kind) {
  case ND_IF: {
    int c = count();
    int loop_id = next_label_name_number();
    VariablePtr res;

    gen_expr_ir(node->cond, &res, table);
    // insert icmp

    // br i1 %cmp, label %if.then, label %if.else
    BlockPtr before = InMemoryIR->GetCurrentBlock();

    BlockPtr label1 = std::make_shared<Block>(next_label_num_number(),
                                              Twine("%if.then", loop_id));
    BlockPtr label2 = std::make_shared<Block>(next_label_num_number(),
                                              Twine("%if.else", loop_id));

    before->SetSucc(label1);
    before->SetSucc(label2);
    label1->SetPred(before);
    label2->SetPred(before);

    assert(InMemoryIR->lastResVar != nullptr);

    InMemoryIR->Insert(InMemoryIR->lastResVar, label1, label2,
                       IROpKind::Op_Branch, table);

    InMemoryIR->SetInsertPoint(label1);

    gen_stmt_ir(node->then, table);

    BlockPtr dest = std::make_shared<Block>(next_label_num_number(),
                                            Twine("%if.end", loop_id));

    label1->SetSucc(dest);
    label2->SetSucc(dest);
    dest->SetPred(label1);
    dest->SetPred(label2);

    InMemoryIR->Insert(nullptr, dest, nullptr, IROpKind::Op_UnConBranch, table);
    // println("  jmp .L.end.%d", c);
    // println(".L.else.%d:", c);
    if (node->els) {
      // file_out << "set new if.else" << std::endl;
      InMemoryIR->SetInsertPoint(label2);
      gen_stmt_ir(node->els, table);
      InMemoryIR->Insert(nullptr, dest, nullptr, IROpKind::Op_UnConBranch,
                         table);
    }
    // file_out << "set new if.end" << std::endl;
    InMemoryIR->SetInsertPoint(dest);
    // println(".L.end.%d:", c);
    return;
  }
  case ND_FOR: // or while
  {
    int c = count();

    // for handle while
    if (node->init)
      gen_stmt_ir(node->init, table);
    // println(".L.begin.%d:", c);
    if (node->cond) {
      VariablePtr res;
      gen_expr_ir(node->cond, &res, table);
      // println("  cmp $0, %%rax");
      // println("  je .L.end.%d", c);
    }
    gen_stmt_ir(node->then, table);

    if (node->inc) {
      VariablePtr res;
      gen_expr_ir(node->inc, &res, table);
    }

    // println("  jmp .L.begin.%d", c);
    // println(".L.end.%d:", c);
    return;
  }
  case ND_BLOCK: {
    // nest
    // int t = next_variable_name_number();
    // InMemoryIR->SetInsertPoint(t, std::to_string(t));
    for (Node *n = node->body; n; n = n->next) {
      gen_stmt_ir(n, table);
    }
    return;
  }
  case ND_RETURN: {
    VariablePtr res;
    gen_expr_ir(node->lhs, &res, table);
    VariablePtr load;
    load = std::make_shared<Variable>();
    load->SetName(next_variable_name());
    bool assertCheck =
        InMemoryIR->Insert(res, nullptr, load, IROpKind::Op_Load, table);
    assert(assertCheck == true);
    InMemoryIR->Insert(nullptr, nullptr, load, IROpKind::Op_Return, table);
    // println("  jmp .L.return.%s", current_fn->name);
    return;
  }
  case ND_EXPR_STMT: {
    // file_out << "generating ND_EXPR_STMT" << std::endl;
    // file_out << "arrive three 8" << std::endl;
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
    std::cout << "gen_variable_ir: " << std::string(node->var->name) << "\n";
    std::cout << "is_local: " << node->var->is_local << "\n";
    if (node->var->is_local) {
      // Local variable
      VariablePtr Var;
      auto VarHaveDefined = table->findVar(getPreName(node->var->name), Var);
      std::cout << "is arg: " << Var->isArg() << "\n";
      if (Var->isArg()) {
        Var = Var->GetAddr();
        // std::cout << "Var: " << Var->GetName() << " addr: " <<
        // Var->GetAddr()->GetName() << "\n";
      }

      assert(VarHaveDefined);
      assert(Var);
      return Var;

      // VariablePtr local_variable = std::make_shared<Variable>();
      // auto checkExistInCahced = [&](std::string name) -> bool {
      //   for (auto var : argVariableCached) {
      //     if (var->GetName() == name) {
      //       return true;
      //     }
      //   }
      //   return false;
      // };

      // // if (checkExistInCahced(Twine(getPreName(node->var->name), ".addr")))
      // { std::cout << "pos 0:\n" << std::string(node->var->name) << "\n";

      // if (!(table->findVar(getPreName(node->var->name), Var)
      //         && Var->isArg())) {
      //   local_variable->SetName(getPreName(node->var->name));
      //   local_variable->type = VaribleKind::VAR_32;
      //   bool assertCheck = InMemoryIR->Insert(nullptr, nullptr,
      //   local_variable,
      //                                         IROpKind::Op_Alloca, table);
      //   assert(assertCheck == false);

      //   /*VariablePtr load1 = std::make_shared<Variable>();
      //   load1->SetName(next_variable_name());

      //   // inst1 = std::make_shared<Instruction>(local_variable, nullptr,
      //   load1, IROpKind::Op_Load);
      //   // instructinos.push_back(inst1);

      //   if(InMemoryIR->Insert(local_variable, nullptr, load1,
      //   IROpKind::Op_Load, table)) file_out << "have successfull insert load
      //   when meeting use" << std::endl;*/

      // } else {
      //   local_variable->SetName(getPreName(node->var->name));
      //   local_variable->type = VaribleKind::VAR_32;
      //   bool assertCheck = InMemoryIR->Insert(nullptr, nullptr,
      //   local_variable,
      //                                         IROpKind::Op_Alloca, table);
      //   assert(assertCheck == false);

      //   /*VariablePtr load1 = std::make_shared<Variable>();
      //   load1->SetName(next_variable_name());

      //   if(InMemoryIR->Insert(local_variable, nullptr, load1,
      //   IROpKind::Op_Load, table)) file_out << "have successfull insert load
      //   when meeting use" << std::endl;*/
      // }
      // return local_variable;

      // println("  lea %d(%%rbp), %%rax", node->var->offset);
    } else {
      // Global variable
      // println("  lea %s(%%rip), %%rax", node->var->name);
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
  // assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
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
  } else {
    if (node->lhs->kind == ND_NUM) {
      // assert(node->lhs->var == nullptr);
      // assert(node->rhs->var != nullptr);
      if (node->rhs->var != nullptr) {
        std::string s = getRealPreName(node->rhs->var);
        VariablePtr r;
        if (table->findVar(s, r)) {
          (*res) = std::make_shared<Variable>();
          //(*res)->Ival = node->lhs->val + r->Ival;
          VariablePtr l;
          l = std::make_shared<Variable>(node->lhs->val);
          //(*res)->SetName(next_variable_name());
          InMemoryIR->Insert(l, r, (*res), InstructionOp, table);
          table->insert((*res), 0);
        }
      }
    } else if (node->rhs->kind == ND_NUM) {
      // assert(node->lhs->var != nullptr);
      // assert(node->rhs->var == nullptr);
      if (node->lhs->var != nullptr) {
        std::string s = getRealPreName(node->lhs->var);
        VariablePtr l;
        if (table->findVar(s, l)) {
          (*res) = std::make_shared<Variable>();
          //(*res)->Ival = l->Ival + node->rhs->val;
          VariablePtr r;
          r = std::make_shared<Variable>(node->rhs->val);
          //(*res)->SetName(next_variable_name());
          InMemoryIR->Insert(l, r, (*res), InstructionOp, table);
          table->insert((*res), 0);
        }
      }
    } else {
      // assert(node->lhs->var != nullptr);
      // assert(node->rhs->var != nullptr);
      if (node->lhs->var != nullptr && node->rhs->var != nullptr) {
        std::string s = getRealPreName(node->lhs->var);
        std::string s2 = getRealPreName(node->rhs->var);
        VariablePtr l, r;
        // file_out << "arrive three 1" << std::endl;
        if (table->findVar(s, l) && table->findVar(s2, r)) {
          (*res) = std::make_shared<Variable>();
          //(*res)->Ival = l->Ival + r->Ival;
          //(*res)->SetName(next_variable_name());
          InMemoryIR->Insert(l, r, (*res), InstructionOp, table);
          table->insert((*res), 0);
        }
      }
    }
  }
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
          VariablePtr load1;
          load1 = std::make_shared<Variable>();
          load1->SetName(next_variable_name());

          bool assertCheck = InMemoryIR->Insert(local_variable, nullptr, load1,
                                                IROpKind::Op_Load, table);
          assert(assertCheck == true);
          // VariablePtr Var;
          // auto VarHaveDefined =
          // table->findVar(getPreName(node->rhs->var->name), Var); std::cout <<
          // "is arg: " << Var->isArg() << "\n"; if (Var->isArg()) {
          //   load1 = local_variable;
          //   // Var = Var->GetAddr();
          //   // std::cout << "Var: " << Var->GetName() << " addr: " <<
          //   Var->GetAddr()->GetName() << "\n";
          // } else {
          //   load1 = std::make_shared<Variable>();
          //   load1->SetName(next_variable_name());

          //   bool assertCheck = InMemoryIR->Insert(
          //         local_variable, nullptr, load1, IROpKind::Op_Load, table);
          //   assert(assertCheck == true);
          // }

          // if (checkExistInCahced(
          //         Twine(getPreName(node->rhs->var->name), ".addr"))) {
          //   local_variable->SetName(
          //       Twine(getPreName(node->rhs->var->name), ".addr"));
          //   local_variable->type = VaribleKind::VAR_32;
          //   // bool assertCheck = InMemoryIR->Insert(nullptr, nullptr,
          //   // local_variable, IROpKind::Op_Alloca, table);
          //   assert(assertCheck
          //   // == false);

          //   load1 = std::make_shared<Variable>();
          //   load1->SetName(next_variable_name());

          //   bool assertCheck = InMemoryIR->Insert(
          //       local_variable, nullptr, load1, IROpKind::Op_Load, table);
          //   assert(assertCheck == true);
          //   // if(InMemoryIR->Insert(local_variable, nullptr, load1,
          //   // IROpKind::Op_Load, table))
          //   //	file_out << "have successfull insert load when meeting
          //   use" <<
          //   // std::endl;

          // } else {
          //   local_variable->SetName(getPreName(node->rhs->var->name));
          //   local_variable->type = VaribleKind::VAR_32;

          //   // bool assertCheck = InMemoryIR->Insert(nullptr, nullptr,
          //   // local_variable, IROpKind::Op_Alloca, table);
          //   assert(assertCheck
          //   // == false);

          //   load1 = std::make_shared<Variable>();
          //   load1->SetName(next_variable_name());

          //   bool assertCheck = InMemoryIR->Insert(
          //       local_variable, nullptr, load1, IROpKind::Op_Load, table);
          //   assert(assertCheck == true);
          //   //	file_out << "have successfull insert load when meeting
          //   use" <<
          //   // std::endl;
          // }

          // InMemoryIR->Insert(local_variable, left, IROpKind::Op_Store,
          // table);
          InMemoryIR->Insert(load1, left, IROpKind::Op_Store, table);
        } else {
          InMemoryIR->Insert(*res, left, IROpKind::Op_Store, table);
        }

        // table->insert(left, *res, 0);
      } else {
        left->isConst = false;
        // here need const propagation
        left->isInitConst = true;
        left->Fval = (*res)->Fval;
        left->Ival = (*res)->Ival;
        left->type = (*res)->type;

        // fix me
        // assert(false);
        InMemoryIR->Insert(left, IROpKind::Op_Store, table);

        // file_out << "ASSIGN get value : "<< left->GetName() <<" = " <<
        // left->Ival <<" res is const?" << (*res)->isConst << std::endl;
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
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen_expr_ir(arg, res, table);
      nargs++;
    }
    return;
  }
  default:
    break;
  }

  // there must deal rhs first
  // file_out << "arrive three 6" << std::endl;
  VariablePtr right = std::make_shared<Variable>();
  gen_expr_ir(node->rhs, &right, table);
  // push();
  // file_out << "arrive three 5" << std::endl;
  VariablePtr left = std::make_shared<Variable>();
  gen_expr_ir(node->lhs, &left, table);
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
    if (var->init_data) {
      VariablePtr Zero = std::make_shared<Variable>(var->init_data[0]);
      Zero->SetGlobal();
      Zero->SetName(var->name);
      ProgramModule->InsertGlobalVariable(Zero);
      // for(int i = 0; i < var->ty->size;i++)
      //	println("  .byte %d", var->init_data[i]);
    } else {
      VariablePtr Zero = std::make_shared<Variable>(0);
      Zero->SetGlobal();
      Zero->SetName(var->name);
      ProgramModule->InsertGlobalVariable(Zero);
      // println("  .zero %d", var->ty->size);
    }
  };
  auto handle = std::for_each(vars.rbegin(), vars.rend(), handleGlobalVariable);
}