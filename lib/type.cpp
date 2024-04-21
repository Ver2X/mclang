#include "mclang.h"

TypePtr ty_char = std::make_shared<Type>(TY_CHAR, 1, 1);

TypePtr ty_short = std::make_shared<Type>(TY_SHORT, 2, 2);

TypePtr ty_int = std::make_shared<Type>(TY_INT, 4, 4);

TypePtr ty_long = std::make_shared<Type>(TY_LONG, 8, 8);

TypePtr ty_void = std::make_shared<Type>(TY_VOID, 1, 1);

static TypePtr new_type(TypeKind kind, int size, int align) {
  TypePtr ty = std::make_shared<Type>();
  ty->kind = kind;
  ty->size = size;
  ty->align = align;
  return ty;
}

/*! judge type
 */
bool is_integer(TypePtr ty) {
  return ty->kind == TY_CHAR || ty->kind == TY_SHORT || ty->kind == TY_INT ||
         ty->kind == TY_LONG;
}

/*! create a pointer of type, with base type
 * */
TypePtr pointer_to(TypePtr base) {
  TypePtr ty = new_type(TY_PTR, 8, 8);
  ty->base = base;
  return ty;
}

TypePtr func_type(TypePtr return_ty) {
  TypePtr ty = std::make_shared<Type>();
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;
  return ty;
}

TypePtr copy_type(TypePtr ty) {
  TypePtr ret = std::make_shared<Type>();
  *ret = *ty;
  return ret;
}

TypePtr array_of(TypePtr base, int len) {
  TypePtr ty = new_type(TY_ARRAY, base->size * len, base->align);
  ty->base = base;
  ty->array_len = len;
  return ty;
}

/*! recursive setup the type of given AST node
 * */
void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next) {
    add_type(n);
  }
  for (Node *n = node->args; n; n = n->next) {
    add_type(n);
  }

  switch (node->kind) {
  // arithmetic operations
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_NEG:
    // may need expand to support implict convert
    node->ty = node->lhs->ty;
    return;
  case ND_ASSIGN:
    if (node->lhs->ty->kind == TY_ARRAY)
      error_tok(node->lhs->tok, "not an lvalue");
    node->ty = node->lhs->ty;
    return;

  // logic operations and int set as ty_int;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
  case ND_FUNCALL:
    node->ty = ty_long;
    return;
  case ND_VAR:
    // int, int *, int ** ...
    node->ty = node->var->ty;
    return;
  case ND_COMMA:
    node->ty = node->rhs->ty;
    return;
  case ND_MEMBER:
    node->ty = node->member->ty;
    return;
  // for '&', create a new type as TY_PTR, setup base type
  case ND_ADDR:
    if (node->lhs->ty->kind == TY_ARRAY)
      node->ty = pointer_to(node->lhs->ty->base);
    else
      node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF:
    if (!node->lhs->ty->base) {
      error_tok(node->tok,
                "expect dereference a pointer or array pointer, but not");
    }
    if (node->lhs->ty->base->kind == TY_VOID) {
      error_tok(node->tok, "dereferencing a void pointer");
    }

    // for '*', down a type level
    node->ty = node->lhs->ty->base;
    return;
  case ND_STMT_EXPR:
    if (node->body) {
      Node *stmt = node->body;
      while (stmt->next)
        stmt = stmt->next;
      if (stmt->kind == ND_EXPR_STMT) {
        node->ty = stmt->lhs->ty;
        return;
      }
    }
    error_tok(node->tok,
              "statement expression returning void is not supported");
    return;
  default:
    return;
  }
}