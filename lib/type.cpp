#include "Module.h"
#include "mclang.h"

TypePtr TyBit = std::make_shared<Type>(TypeKind::TY_BIT, 0, 1);

TypePtr TyChar = std::make_shared<Type>(TypeKind::TY_CHAR, 1, 1);

TypePtr TyShort = std::make_shared<Type>(TypeKind::TY_SHORT, 2, 2);

TypePtr TyInt = std::make_shared<Type>(TypeKind::TY_INT, 4, 4);

TypePtr TyLong = std::make_shared<Type>(TypeKind::TY_LONG, 8, 8);

TypePtr TyDouble = std::make_shared<Type>(TypeKind::TY_DOUBLE, 8, 8);

TypePtr TyVoid = std::make_shared<Type>(TypeKind::TY_VOID, 1, 1);

extern std::shared_ptr<Module> ProgramModule;

std::string Type::CodeGen() {
  if (Kind == TypeKind::TY_INT) {
    return "i32";
  } else if (Kind == TypeKind::TY_PTR) {
    return Base->CodeGen() + "*";
  } else if (Kind == TypeKind::TY_ARRAY) {
    return "[" + std::to_string(ArrayLen) + " x " + Base->CodeGen() + "]";
  } else {
    return "???";
  }
}
TypePtr newType(TypeKind Kind, int Size, int Align) {
  TypePtr Ty = std::make_shared<Type>();
  Ty->Kind = Kind;
  Ty->Size = Size;
  Ty->Align = Align;
  return Ty;
}

/*! judge type
 */
bool isInteger(TypePtr Ty) {
  return Ty->Kind == TypeKind::TY_CHAR || Ty->Kind == TypeKind::TY_SHORT ||
         Ty->Kind == TypeKind::TY_INT || Ty->Kind == TypeKind::TY_LONG;
}

/*! create a pointer of type, with Base type
 * */
TypePtr pointerTo(TypePtr Base) {
  TypePtr Ty = newType(TypeKind::TY_PTR, 8, 8);
  Ty->Base = Base;
  return Ty;
}

TypePtr baseTo(TypePtr Ptr) {
  TypePtr Ty = Ptr->Base;
  return Ty;
}

TypePtr funcType(TypePtr ReturnTy) {
  TypePtr Ty = std::make_shared<Type>();
  Ty->Kind = TypeKind::TY_FUNC;
  Ty->ReturnTy = ReturnTy;
  return Ty;
}

TypePtr copyType(TypePtr Ty) {
  TypePtr ret = std::make_shared<Type>();
  *ret = *Ty;
  return ret;
}

TypePtr arrayOf(TypePtr Base, int len) {
  TypePtr Ty = newType(TypeKind::TY_ARRAY, Base->Size * len, Base->Align);
  Ty->Base = Base;
  Ty->ArrayLen = len;
  return Ty;
}

/*! recursive setup the type of given AST node
 * */
void addType(Node *node) {
  if (!node || node->Ty)
    return;

  addType(node->Lhs);
  addType(node->Rhs);
  addType(node->cond);
  addType(node->then);
  addType(node->els);
  addType(node->init);
  addType(node->inc);

  for (Node *n = node->Body; n; n = n->Next) {
    addType(n);
  }
  for (Node *n = node->args; n; n = n->Next) {
    addType(n);
  }

  switch (node->Kind) {
  // arithmetic operations
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_NEG:
  case ND_POINTER_OFFSET:
    // may need expand to support implict convert
    node->Ty = node->Lhs->Ty;
    return;
  case ND_ASSIGN:
    if (node->Lhs->Ty->Kind == TypeKind::TY_ARRAY)
      error_tok(node->Lhs->Tok, "not an lvalue");
    node->Ty = node->Lhs->Ty;
    return;

  // logic operations and int set as TyInt;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
  case ND_FUNCALL:
    node->Ty = TyInt;
    return;
  case ND_VAR:
    // int, int *, int ** ...
    node->Ty = node->Var->Ty;
    return;
  case ND_COMMA:
    node->Ty = node->Rhs->Ty;
    return;
  case ND_MEMBER:
    node->Ty = node->member->Ty;
    return;
  // for '&', create a new type as TY_PTR, setup Base type
  case ND_ADDR:
    if (node->Lhs->Ty->Kind == TypeKind::TY_ARRAY)
      node->Ty = pointerTo(node->Lhs->Ty->Base);
    else
      node->Ty = pointerTo(node->Lhs->Ty);
    return;
  case ND_DEREF:
    if (!node->Lhs->Ty->Base) {
      error_tok(node->Tok,
                "expect dereference a pointer or array pointer, but not");
    }
    if (node->Lhs->Ty->Base->Kind == TypeKind::TY_VOID) {
      error_tok(node->Tok, "dereferencing a void pointer");
    }

    // for '*', down a type level
    node->Ty = node->Lhs->Ty->Base;
    return;
  case ND_STMT_EXPR:
    if (node->Body) {
      Node *stmt = node->Body;
      while (stmt->Next)
        stmt = stmt->Next;
      if (stmt->Kind == ND_EXPR_STMT) {
        node->Ty = stmt->Lhs->Ty;
        return;
      }
    }
    error_tok(node->Tok,
              "statement expression returning void is not supported");
    return;
  default:
    return;
  }
}