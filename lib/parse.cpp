// This file contains a recursive descent parser for C.
//
// Most functions in this file are named after the symbols they are
// supposed to read from an input token list. For example, stmt() is
// responsible for reading a statement from a token list. The function
// then construct an AST node representing a statement.
//
// Each function conceptually returns two values, an AST node and
// remaining part of the input tokens. Since C doesn't support
// multiple return values, the remaining tokens are returned to the
// caller via a pointer argument.
//
// Input tokens are represented by a linked list. Unlike many recursive
// descent parsers, we don't have the notion of the "input token stream".
// Most parsing functions don't change the global state of the parser.
// So it is very easy to lookahead arbitrary number of tokens in this
// parser.

#include "mclang.h"

// Scope for local, global variable or typedefs
typedef struct VarScope VarScope;
struct VarScope {
  VarScope *Next;
  char *Name;
  Obj *Var;

  TypePtr type_def;
};

// Scope for struct or union tags
typedef struct TagScope TagScope;
struct TagScope {
  TagScope *Next;
  char *Name;
  TypePtr Ty;
};

// represent a Block Scope
typedef struct BlockScope BlockScope;
struct BlockScope {
  BlockScope *Next;

  VarScope *vars; // vars

  TagScope *tags; // structs
};

typedef struct {
  bool is_typedef;
} VarAttr;

// All Locals variable, head insert method
static Obj *Locals;
static Obj *globals;

BlockScope scope__t = (BlockScope){};
static BlockScope *Scope = &scope__t;

// rest use to skip symbol tokens, make it akways pointer to first token will
// deal. use rest, for 'cur = cur->Next = stmt(&Tok, Tok);' Right
//
// Tok, make it akways pointer to first un-symbol token will deal.
static Node *expr(TokenPtr *rest, TokenPtr Tok);
static Node *exprStmt(TokenPtr *rest, TokenPtr Tok);
static Node *assign(TokenPtr *rest, TokenPtr Tok);
static Node *equality(TokenPtr *rest, TokenPtr Tok);
static Node *relational(TokenPtr *rest, TokenPtr Tok);
static Node *add(TokenPtr *rest, TokenPtr Tok);
static Node *mul(TokenPtr *rest, TokenPtr Tok);
static Node *unary(TokenPtr *rest, TokenPtr Tok);
static Node *primary(TokenPtr *rest, TokenPtr Tok);
static Node *createNewNode(NodeKind Kind, TokenPtr Tok);
static Node *compoundStmt(TokenPtr *rest, TokenPtr Tok);
static Node *stmt(TokenPtr *rest, TokenPtr Tok);
static Node *declaration(TokenPtr *rest, TokenPtr Tok, TypePtr basety);
static TypePtr declspec(TokenPtr *rest, TokenPtr Tok, VarAttr *attr);
static TypePtr declarator(TokenPtr *rest, TokenPtr Tok, TypePtr Ty);
static Node *postfix(TokenPtr *rest, TokenPtr Tok);
static TypePtr structDecl(TokenPtr *rest, TokenPtr Tok);
static TypePtr unionDecl(TokenPtr *rest, TokenPtr Tok);
static bool isTypename(TokenPtr Tok);
static TokenPtr parseTypedef(TokenPtr Tok, TypePtr basety);

static void enterScope(void) {
  auto *sc = (BlockScope *)calloc(1, sizeof(BlockScope));
  sc->Next = Scope;
  Scope = sc;
}

static void leaveScope(void) { Scope = Scope->Next; }

static VarScope *find_var(TokenPtr Tok) {
  for (BlockScope *sc = Scope; sc; sc = sc->Next) {
    for (VarScope *sc2 = sc->vars; sc2; sc2 = sc2->Next)
      if (equal(Tok, sc2->Name))
        return sc2;
  }
  return nullptr;
}

static TypePtr find_tag(TokenPtr Tok) {
  for (BlockScope *sc = Scope; sc; sc = sc->Next) {
    for (TagScope *sc2 = sc->tags; sc2; sc2 = sc2->Next) {
      if (equal(Tok, sc2->Name))
        return sc2->Ty;
    }
  }

  return nullptr;
}

static VarScope *push_scope(char *Name) {
  auto *sc = (VarScope *)calloc(1, sizeof(VarScope));
  sc->Name = Name;
  sc->Next = Scope->vars; // VarScope * vars, head insert here
  Scope->vars = sc;
  return sc;
}

static void push_tag_scope(TokenPtr Tok, TypePtr Ty) {
  auto *sc = (TagScope *)calloc(1, sizeof(TagScope));
  sc->Name = strndup(Tok->loc, Tok->len);
  sc->Ty = Ty;
  sc->Next = Scope->tags;
  Scope->tags = sc;
}

// head insert
static Obj *new_var(char *Name, TypePtr Ty) {
  Obj *Var = (Obj *)calloc(1, sizeof(Obj));
  Var->Name = Name;
  Var->Ty = Ty;
  push_scope(Name)->Var = Var;
  return Var;
}

// head insert, local variable
static Obj *new_lvar(char *Name, TypePtr Ty) {
  Obj *Var = new_var(Name, Ty);
  Var->IsLocal = true;
  Var->Next = Locals;
  Locals = Var;
  return Var;
}

// head insert, global variable
static Obj *new_gvar(char *Name, TypePtr Ty) {
  Obj *Var = new_var(Name, Ty);
  Var->Next = globals;
  globals = Var;
  return Var;
}

static char *new_unique_name(void) {
  static int id = 0;
  return format(".L..%d", id++);
}

static Obj *new_anon_gvar(TypePtr Ty) {
  return new_gvar(new_unique_name(), Ty);
}

static Obj *new_string_literal(char *p, TypePtr Ty) {
  Obj *Var = new_anon_gvar(Ty);
  Var->InitData = p;
  return Var;
}

// create a new node by special Kind
static Node *createNewNode(NodeKind Kind, TokenPtr Tok) {
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->Kind = Kind;
  node->Tok = Tok;
  return node;
}

// binary operator
static Node *new_binary(NodeKind Kind, Node *Lhs, Node *Rhs, TokenPtr Tok) {
  Node *node = createNewNode(Kind, Tok);
  node->Lhs = Lhs;
  node->Rhs = Rhs;
  return node;
}

// unary operator
static Node *new_unary(NodeKind Kind, Node *expr, TokenPtr Tok) {
  Node *node = createNewNode(Kind, Tok);
  node->Lhs = expr;
  return node;
}

// create a number
static Node *new_num(int64_t Val, TokenPtr Tok) {
  Node *node = createNewNode(ND_NUM, Tok);
  node->Val = Val;
  return node;
}

// create a variable
static Node *new_var_node(Obj *Var, TokenPtr Tok) {
  Node *node = createNewNode(ND_VAR, Tok);
  node->Var = Var;
  return node;
}

static char *get_ident(TokenPtr Tok) {
  if (Tok->Kind != TK_IDENT) {
    error_tok(Tok, "expected an identifier");
  }

  return strndup(Tok->loc, Tok->len);
}

static TypePtr find_typedef(TokenPtr Tok) {
  if (Tok->Kind == TK_IDENT) {
    VarScope *sc = find_var(Tok);
    if (sc)
      return sc->type_def;
  }
  return nullptr;
}

static int get_number(TokenPtr Tok) {
  if (Tok->Kind != TK_NUM) {
    error_tok(Tok, "expected a number");
  }

  return Tok->Val;
}

// declspec = ("void" | "char" | "short" | "int" | "long"
//             | struct-decl | union-decl)+
//             | "typedef"
//             | struct-decl | union-decl | typedef-Name)+
//
// The order of typenames in a type-specifier doesn't matter. For
// example, `int long static` means the same as `static long int`.
// That can also be written as `static long` because you can omit
// `int` if `long` or `short` are specified. However, something like
// `char int` is not a valid type specifier. We have to accept only a
// limited combinations of the typenames.
//
// In this function, we count the number of occurrences of each typename
// while keeping the "current" type object that the typenames up
// until that point represent. When we reach a non-typename token,
// we returns the current type object.
static TypePtr declspec(TokenPtr *rest, TokenPtr Tok, VarAttr *attr) {
  // We use a single integer as counters for all typenames.
  // For example, bits 0 and 1 represents how many times we saw the
  // keyword "void" so far. With this, we can use a switch statement
  // as you can see below.
  enum {
    VOID = 1 << 0,
    CHAR = 1 << 2,
    SHORT = 1 << 4,
    INT = 1 << 6,
    LONG = 1 << 8,
    OTHER = 1 << 10,
  };
  TypePtr Ty = TyInt;
  int counter = 0;

  while (isTypename(Tok)) {
    if (equal(Tok, "typedef")) {
      if (!attr)
        error_tok(Tok,
                  "storage class specifier is not allowed in this context");
      attr->is_typedef = true;
      Tok = Tok->Next;
      continue;
    }

    // handle user-defined types
    TypePtr ty2 = find_typedef(Tok);
    if (equal(Tok, "struct") || equal(Tok, "union") || ty2) {
      if (counter)
        break;

      if (equal(Tok, "struct"))
        Ty = structDecl(&Tok, Tok->Next);
      else if (equal(Tok, "union"))
        Ty = unionDecl(&Tok, Tok->Next);
      else {
        Ty = ty2;
        Tok = Tok->Next;
      }
      counter += OTHER;
      continue;
    }

    // handle built-in types
    if (equal(Tok, "void"))
      counter += VOID;
    else if (equal(Tok, "char"))
      counter += CHAR;
    else if (equal(Tok, "short"))
      counter += SHORT;
    else if (equal(Tok, "int"))
      counter += INT;
    else if (equal(Tok, "long"))
      counter += LONG;
    else
      unreachable();

    switch (counter) {
    case VOID:
      Ty = TyVoid;
      break;
    case CHAR:
      Ty = TyChar;
      break;
    case SHORT:
    case SHORT + INT:
      Ty = TyShort;
      break;
    case INT:
      Ty = TyInt;
      break;
    case LONG:
    case LONG + INT:
    case LONG + LONG:
    case LONG + LONG + INT:
      Ty = TyLong;
      break;
    default:
      error_tok(Tok, "invalid type");
    }
    Tok = Tok->Next;
  }

  *rest = Tok;
  return Ty; // unreachable
}

static bool isTypename(TokenPtr Tok) {
  static const char *kw[] = {"void", "char",   "short", "int",
                             "long", "struct", "union", "typedef"};
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    if (equal(Tok, kw[i]))
      return true;
  }
  return find_typedef(Tok) != nullptr;
}

// func-Params = (param ("," param)*)? ")"
// param       = declspec declarator
static TypePtr func_params(TokenPtr *rest, TokenPtr Tok, TypePtr Ty) {
  TypePtr head = std::make_shared<Type>();
  TypePtr cur = head;

  while (!equal(Tok, ")")) {
    if (cur != head) {
      Tok = skip(Tok, ",");
    }

    TypePtr basety = declspec(&Tok, Tok, nullptr);
    TypePtr Ty = declarator(&Tok, Tok, basety);
    cur = cur->Next = copyType(Ty);
  }

  Ty = funcType(Ty);
  Ty->Params = head->Next;
  *rest = Tok->Next;
  return Ty;
}

// type-suffix = "(" func-Params
//             | "[" num "]" type-suffix
//             | ε
static TypePtr type_suffix(TokenPtr *rest, TokenPtr Tok, TypePtr Ty) {
  if (equal(Tok, "("))
    return func_params(rest, Tok->Next, Ty);

  if (equal(Tok, "[")) {
    int sz = get_number(Tok->Next);
    Tok = skip(Tok->Next->Next, "]");
    Ty = type_suffix(rest, Tok, Ty);
    return arrayOf(Ty, sz);
  }

  *rest = Tok;
  return Ty;
}

// declarator = "*"* ("(" ident ")" | "(" declarator ")" | ident) type-suffix
static TypePtr declarator(TokenPtr *rest, TokenPtr Tok, TypePtr Ty) {
  while (consume(&Tok, Tok, "*")) {
    Ty = pointerTo(Ty);
  }

  if (equal(Tok, "(")) {
    TokenPtr start = Tok;
    TypePtr dummy = std::make_shared<Type>();
    declarator(&Tok, start->Next, dummy);
    Tok = skip(Tok, ")");
    Ty = type_suffix(rest, Tok, Ty);
    return declarator(&Tok, start->Next, Ty);
  }

  if (Tok->Kind != TK_IDENT)
    error_tok(Tok, "expected a varibale Name");

  Ty = type_suffix(rest, Tok->Next, Ty);
  Ty->Name = Tok;
  return Ty;
}

// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
static TypePtr abastract_declarator(TokenPtr *rest, TokenPtr Tok, TypePtr Ty) {
  while (equal(Tok, "*")) {
    Ty = pointerTo(Ty);
    Tok = Tok->Next;
  }
  if (equal(Tok, "(")) {
    TokenPtr start = Tok;
    TypePtr dummy = std::make_shared<Type>();
    abastract_declarator(&Tok, start->Next, dummy);
    Tok = skip(Tok, ")");
    Ty = type_suffix(rest, Tok, Ty);
    return abastract_declarator(&Tok, start->Next, Ty);
  }

  return type_suffix(rest, Tok, Ty);
}

// type-Name = declspec abstract-declarator
static TypePtr type_name(TokenPtr *rest, TokenPtr Tok) {
  TypePtr Ty = declspec(&Tok, Tok, nullptr);
  return abastract_declarator(rest, Tok, Ty);
}

// declaration = (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;
static Node *declaration(TokenPtr *rest, TokenPtr Tok, TypePtr basety) {

  Node head = {};
  Node *cur = &head;

  int i = 0; // Label to check whether is the first declaration varibale
  while (!equal(Tok, ";")) {
    if (i++ > 0)
      Tok = skip(Tok, ",");

    // define but not used, add it to variable list
    TypePtr Ty = declarator(&Tok, Tok, basety);
    if (Ty->Kind == TypeKind::TY_VOID)
      error_tok(Tok, "varibale declared as void type");
    Obj *Var = new_lvar(get_ident(Ty->Name), Ty);

    if (!equal(Tok, "="))
      continue;

    Node *Lhs = new_var_node(Var, Ty->Name);
    Node *Rhs = assign(&Tok, Tok->Next);
    Node *node = new_binary(ND_ASSIGN, Lhs, Rhs, Tok);
    cur = cur->Next = new_unary(ND_EXPR_STMT, node, Tok);
  }

  Node *node = createNewNode(ND_BLOCK, Tok);
  node->Body = head.Next;
  *rest = Tok->Next;
  return node;
}

// stmt = "return" expr ";"
//			| "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//			| exprStmt
static Node *stmt(TokenPtr *rest, TokenPtr Tok) {
  if (equal(Tok, "return")) {
    Node *node = createNewNode(ND_RETURN, Tok);
    node->Lhs = expr(&Tok, Tok->Next);
    *rest = skip(Tok, ";");
    return node;
  }

  if (equal(Tok, "if")) {
    Node *node = createNewNode(ND_IF, Tok);
    Tok = skip(Tok->Next, "(");
    node->cond = expr(&Tok, Tok);
    Tok = skip(Tok, ")");
    node->then = stmt(&Tok, Tok);
    if (equal(Tok, "else"))
      node->els = stmt(&Tok, Tok->Next);
    *rest = Tok;
    return node;
  }

  if (equal(Tok, "for")) {
    Node *node = createNewNode(ND_FOR, Tok);
    Tok = skip(Tok->Next, "(");

    node->init = exprStmt(&Tok, Tok);

    // here use !equal to judge whether cond is nullptr, e.x. for(init; ; inc)
    if (!equal(Tok, ";"))
      node->cond = expr(&Tok, Tok);
    Tok = skip(Tok, ";");

    if (!equal(Tok, ")"))
      node->inc = expr(&Tok, Tok);
    Tok = skip(Tok, ")");

    node->then = stmt(rest, Tok);

    return node;
  }

  if (equal(Tok, "while")) {
    Node *node = createNewNode(ND_FOR, Tok);
    Tok = skip(Tok->Next, "(");
    node->cond = expr(&Tok, Tok);
    Tok = skip(Tok, ")");
    node->then = stmt(rest, Tok);
    return node;
  }

  if (equal(Tok, "{")) {
    return compoundStmt(rest, Tok->Next);
  }
  return exprStmt(rest, Tok);
}

// compound-stmt = (typedef | declaration |stmt )* "}"
static Node *compoundStmt(TokenPtr *rest, TokenPtr Tok) {
  Node *node = createNewNode(ND_BLOCK, Tok);

  Node head = {};
  Node *cur = &head;

  enterScope();

  while (!equal(Tok, "}")) {
    if (isTypename(Tok)) {
      VarAttr attr = {};
      TypePtr basety = declspec(&Tok, Tok, &attr);

      if (attr.is_typedef) {
        Tok = parseTypedef(Tok, basety);
        continue;
      }

      cur = cur->Next = declaration(&Tok, Tok, basety);
    } else {
      cur = cur->Next = stmt(&Tok, Tok);
    }

    // here add type
    addType(cur);
  }

  leaveScope();

  node->Body = head.Next;
  *rest = Tok->Next;
  return node;
}

// must exprStmt could be nullptr, for support for init , e.x. for(; ; ;)
// exprStmt       = expr? ";"
static Node *exprStmt(TokenPtr *rest, TokenPtr Tok) {
  if (equal(Tok, ";")) {
    *rest = Tok->Next;
    return createNewNode(ND_BLOCK, Tok);
  }

  // expr(&Tok, Tok)), beacuse in parse() :  cur = cur->Next = stmt(&Tok, Tok);
  Node *node = createNewNode(ND_EXPR_STMT, Tok);
  node->Lhs = expr(&Tok, Tok);
  *rest = skip(Tok, ";");
  return node;
}

// expr       = assign ("," expr)
static Node *expr(TokenPtr *rest, TokenPtr Tok) {
  Node *node = assign(&Tok, Tok);
  if (equal(Tok, ","))
    return new_binary(ND_COMMA, node, expr(rest, Tok->Next), Tok);

  *rest = Tok;
  return node;
}

// assign     = equality ("=" assign)?
Node *assign(TokenPtr *rest, TokenPtr Tok) {
  Node *node = equality(&Tok, Tok);
  if (equal(Tok, "="))
    node = new_binary(ND_ASSIGN, node, assign(&Tok, Tok->Next), Tok);
  // upward code Tok->Next, will skip token("=") in future call
  *rest = Tok;
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
static Node *equality(TokenPtr *rest, TokenPtr Tok) {
  Node *node = relational(&Tok, Tok);

  for (;;) {
    TokenPtr start = Tok;
    if (equal(Tok, "==")) {
      node = new_binary(ND_EQ, node, relational(&Tok, Tok->Next), start);
      continue;
    }

    if (equal(Tok, "!=")) {
      node = new_binary(ND_NE, node, relational(&Tok, Tok->Next), start);
      continue;
    }

    *rest = Tok;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(TokenPtr *rest, TokenPtr Tok) {
  Node *node = add(&Tok, Tok);

  for (;;) {
    TokenPtr start = Tok;

    if (equal(Tok, "<")) {
      node = new_binary(ND_LT, node, add(&Tok, Tok->Next), start);
      continue;
    }

    if (equal(Tok, "<=")) {
      node = new_binary(ND_LE, node, add(&Tok, Tok->Next), start);
      continue;
    }

    if (equal(Tok, ">")) {
      node = new_binary(ND_LT, add(&Tok, Tok->Next), node, start);
      continue;
    }

    if (equal(Tok, ">=")) {
      node = new_binary(ND_LE, add(&Tok, Tok->Next), node, start);
      continue;
    }

    *rest = Tok;
    return node;
  }
}

/*! In C, `+` operator is overloaded to perform the pointer arithmetic.
 * If p is a pointer, p+n adds not n but sizeof(*p)*n to the value of p,
 * so that p+n points to the location n elements (not bytes) ahead of p.
 * In other words, we need to scale an integer value before adding to a
 * pointer value. This function takes care of the scaling.
 *
 * */
static Node *new_add(Node *Lhs, Node *Rhs, TokenPtr Tok) {
  addType(Lhs);
  addType(Rhs);

  // num + num
  if (isInteger(Lhs->Ty) && isInteger(Rhs->Ty)) {
    return new_binary(ND_ADD, Lhs, Rhs, Tok);
  }

  if (Lhs->Ty->Base && Rhs->Ty->Base)
    error_tok(Tok, "invalid operands");

  // canonicalize `num + ptr` to `ptr + num`
  if (!Lhs->Ty->Base && Rhs->Ty->Base) {
    Node *tmp = Lhs;
    Lhs = Rhs;
    Rhs = tmp;
  }

  // ptr + num
  return new_binary(ND_POINTER_OFFSET, Lhs, Rhs, Tok);
}

// Like `+`, `-` is overloaded for the pointer type
static Node *new_sub(Node *Lhs, Node *Rhs, TokenPtr Tok) {
  addType(Lhs);
  addType(Rhs);

  // num - num
  if (isInteger(Lhs->Ty) && isInteger(Rhs->Ty)) {
    return new_binary(ND_SUB, Lhs, Rhs, Tok);
  }

  // ptr - num
  if (Lhs->Ty->Base && isInteger(Rhs->Ty)) {
    Rhs = new_binary(ND_MUL, Rhs, new_num(Lhs->Ty->Base->Size, Tok), Tok);
    addType(Rhs);
    Node *node = new_binary(ND_SUB, Lhs, Rhs, Tok);
    node->Ty = Lhs->Ty;
    return node;
  }

  // ptr - ptr
  if (Lhs->Ty->Base && Rhs->Ty->Base) {
    Node *node = new_binary(ND_SUB, Lhs, Rhs, Tok);
    node->Ty = TyInt;
    return new_binary(ND_DIV, node, new_num(Lhs->Ty->Base->Size, Tok), Tok);
  }

  error_tok(Tok, "invalid operands");

  return nullptr; // unreachable
}

// add        = mul ("+" mul | "-" mul)*
static Node *add(TokenPtr *rest, TokenPtr Tok) {
  Node *node = mul(&Tok, Tok);

  for (;;) {
    TokenPtr start = Tok;

    if (equal(Tok, "+")) {
      node = new_add(node, mul(&Tok, Tok->Next), start);
      continue;
    }

    if (equal(Tok, "-")) {
      node = new_sub(node, mul(&Tok, Tok->Next), start);
      continue;
    }

    *rest = Tok;
    return node;
  }
}

// mul     = unary ("*" unary | "/" unary)*
static Node *mul(TokenPtr *rest, TokenPtr Tok) {
  Node *node = unary(&Tok, Tok);

  for (;;) {
    TokenPtr start = Tok;

    if (equal(Tok, "*")) {
      node = new_binary(ND_MUL, node, unary(&Tok, Tok->Next), start);
      continue;
    }

    if (equal(Tok, "/")) {
      node = new_binary(ND_DIV, node, unary(&Tok, Tok->Next), start);
      continue;
    }

    *rest = Tok;
    return node;
  }
}

// unary   = ("+" | "-" | "&" | "*")? unary
//         | postfix
static Node *unary(TokenPtr *rest, TokenPtr Tok) {
  if (equal(Tok, "+"))
    return unary(rest, Tok->Next);

  if (equal(Tok, "-"))
    return new_unary(ND_NEG, unary(rest, Tok->Next), Tok);

  if (equal(Tok, "&"))
    return new_unary(ND_ADDR, unary(rest, Tok->Next), Tok);

  if (equal(Tok, "*"))
    return new_unary(ND_DEREF, unary(rest, Tok->Next), Tok);

  return postfix(rest, Tok);
}

// struct-Members = (declspec declarator (","  declarator)* ";")*
static void struct_members(TokenPtr *rest, TokenPtr Tok, TypePtr Ty) {
  Member head = {};
  Member *cur = &head;

  while (!equal(Tok, "}")) {
    TypePtr basety = declspec(&Tok, Tok, nullptr);
    int i = 0;

    while (!consume(&Tok, Tok, ";")) {
      if (i++)
        Tok = skip(Tok, ",");

      auto *mem = (Member *)calloc(1, sizeof(Member));
      mem->Ty = declarator(&Tok, Tok, basety);
      mem->Name = mem->Ty->Name;
      cur = cur->Next = mem;
    }
  }

  *rest = Tok->Next;
  Ty->Members = head.Next;
}

// struct-union-decl = ident? ("{" struct-union-Members)?
static TypePtr struct_union_decl(TokenPtr *rest, TokenPtr Tok) {
  // read a tag
  TokenPtr tag = nullptr;
  if (Tok->Kind == TK_IDENT) {
    tag = Tok;
    Tok = Tok->Next;
  }

  // use to define a struct variable
  if (tag && !equal(Tok, "{")) {
    TypePtr Ty = find_tag(tag);
    if (!Ty)
      error_tok(tag, "unknown struct type");
    *rest = Tok;
    return Ty;
  }

  // construct a struct object
  TypePtr Ty = std::make_shared<Type>();
  Ty->Kind = TypeKind::TY_STRUCT;
  struct_members(rest, Tok->Next, Ty); // skip "}"
  Ty->Align = 1;

  // register the struct type if a Name was give
  if (tag) {
    push_tag_scope(tag, Ty);
  }
  return Ty;
}
// struct-decl = struct-union-decl
static TypePtr structDecl(TokenPtr *rest, TokenPtr Tok) {
  TypePtr Ty = struct_union_decl(rest, Tok);
  Ty->Kind = TypeKind::TY_STRUCT;

  // assgin offsets
  int Offset = 0;
  for (Member *mem = Ty->Members; mem; mem = mem->Next) {
    Offset = alignTo(Offset, mem->Ty->Align);
    mem->Offset = Offset;
    Offset += mem->Ty->Size;

    if (Ty->Align < mem->Ty->Align) {
      Ty->Align = mem->Ty->Align;
    }
  }

  Ty->Size = alignTo(Offset, Ty->Align);
  return Ty;
}

// union-decl = struct-union-decl
static TypePtr unionDecl(TokenPtr *rest, TokenPtr Tok) {
  TypePtr Ty = struct_union_decl(rest, Tok);
  Ty->Kind = TypeKind::TY_UNION;

  // for union, all Offset is 0, but we need to compute the alignment
  // and Size

  for (Member *mem = Ty->Members; mem; mem = mem->Next) {

    if (Ty->Align < mem->Ty->Align) {
      Ty->Align = mem->Ty->Align;
    }
    if (Ty->Size < mem->Ty->Size)
      Ty->Size = mem->Ty->Size;
  }
  Ty->Size = alignTo(Ty->Size, Ty->Align);

  return Ty;
}

static Member *get_struct_member(TypePtr Ty, TokenPtr Tok) {
  for (Member *mem = Ty->Members; mem; mem = mem->Next) {
    if (mem->Name->len == Tok->len &&
        !strncmp(mem->Name->loc, Tok->loc, Tok->len)) {
      return mem;
    }
  }

  error_tok(Tok, "no such member");

  return nullptr;
}

static Node *struct_ref(Node *Lhs, TokenPtr Tok) {
  addType(Lhs);
  if (Lhs->Ty->Kind != TypeKind::TY_STRUCT &&
      Lhs->Ty->Kind != TypeKind::TY_UNION)
    error_tok(Lhs->Tok, "not a struct nor union");

  Node *node = new_unary(ND_MEMBER, Lhs, Tok);
  node->member = get_struct_member(Lhs->Ty, Tok);
  return node;
}

// postfix = primary ("[" expr "]" | "." ident | "->" ident)*
static Node *postfix(TokenPtr *rest, TokenPtr Tok) {
  Node *node = primary(&Tok, Tok);

  for (;;) {
    if (equal(Tok, "[")) {
      // x[y] ==> *(x+y)
      TokenPtr start = Tok;
      Node *idx = expr(&Tok, Tok->Next);
      Tok = skip(Tok, "]");
      // node->Lhs->Ty->Base
      node = new_unary(ND_DEREF, new_add(node, idx, start), start);
      // node = new_unary(ND_DEREF, idx, start);
      continue;
    }

    if (equal(Tok, ".")) {
      node = struct_ref(node, Tok->Next);
      Tok = Tok->Next->Next;
      continue;
    }

    if (equal(Tok, "->")) {
      // x->t is short for (*x).y
      node = new_unary(ND_DEREF, node, Tok);
      node = struct_ref(node, Tok->Next);
      Tok = Tok->Next->Next; // skip "->" and ident
      continue;
    }

    *rest = Tok;

    return node;
  }
}

// funcall = ident "(" (assign ("," assign)*)? ")"
static Node *funcall(TokenPtr *rest, TokenPtr Tok) {
  TokenPtr start = Tok;  // point to ident
  Tok = Tok->Next->Next; // point to first argument

  Node head = {};
  Node *cur = &head;

  while (!equal(Tok, ")")) {
    if (cur != &head)
      Tok = skip(Tok, ",");

    cur = cur->Next = assign(&Tok, Tok);
  }

  *rest = skip(Tok, ")");

  Node *node = createNewNode(ND_FUNCALL, start);
  node->funcname = strndup(start->loc, start->len);
  node->args = head.Next;
  return node;
}

// primary = "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" "(" type-Name ")"
//         | "sizeof" unary
//         | ident func-args?
//         | str
//         | num
static Node *primary(TokenPtr *rest, TokenPtr Tok) {
  TokenPtr start = Tok;

  if (equal(Tok, "(") && equal(Tok->Next, "{")) {
    // this is a GNU statement expresssion
    Node *node = createNewNode(ND_STMT_EXPR, Tok);
    node->Body = compoundStmt(&Tok, Tok->Next->Next)->Body;
    *rest = skip(Tok, ")");
    return node;
  }

  if (equal(Tok, "(")) {
    // here is &Tok, beacause expr change *rest = (frist node un deal)
    Node *node = expr(&Tok, Tok->Next);
    *rest = skip(Tok, ")");
    return node;
  }

  if (equal(Tok, "sizeof") && equal(Tok->Next, "(") &&
      isTypename(Tok->Next->Next)) {
    TypePtr Ty = type_name(&Tok, Tok->Next->Next);
    *rest = skip(Tok, ")");
    return new_num(Ty->Size, start);
  }

  if (equal(Tok, "sizeof")) {
    Node *node = unary(rest, Tok->Next);
    addType(node);
    return new_num(node->Ty->Size, Tok);
  }

  if (Tok->Kind == TK_IDENT) {
    // function call
    if (equal(Tok->Next, "(")) {
      return funcall(rest, Tok);
    }

    // variable
    VarScope *sc = find_var(Tok);
    if (!sc || !sc->Var) {
      error_tok(Tok, "undefined varibale");
    }
    *rest = Tok->Next;
    return new_var_node(sc->Var, Tok);
  }

  if (Tok->Kind == TK_STR) {
    Obj *Var = new_string_literal(Tok->str, Tok->Ty);
    *rest = Tok->Next;
    return new_var_node(Var, Tok);
  }

  if (Tok->Kind == TK_NUM) {
    Node *node = new_num(Tok->Val, Tok);
    *rest = Tok->Next;
    return node;
  }

  error_tok(Tok, "expected an expression");

  return nullptr;
}

static TokenPtr parseTypedef(TokenPtr Tok, TypePtr basety) {
  bool first = true;
  while (!consume(&Tok, Tok, ";")) {
    if (!first)
      Tok = skip(Tok, ",");
    first = false;

    TypePtr Ty = declarator(&Tok, Tok, basety);
    push_scope(get_ident(Ty->Name))->type_def = Ty;
  }
  return Tok;
}

static void create_param_lvars(TypePtr param) {
  if (param) {
    // recurse to last Params
    create_param_lvars(param->Next);
    // head insert, in variable list seq is same with seq decl variable
    new_lvar(get_ident(param->Name), param);
  }
}

// function_declaration = declspec declarator "{" compoundStmt "}"
static TokenPtr function(TokenPtr Tok, TypePtr basety) {
  TypePtr Ty = declarator(&Tok, Tok, basety);

  Obj *fn = new_gvar(get_ident(Ty->Name), Ty);
  fn->IsFunction = true;
  fn->IsDefinition = !consume(&Tok, Tok, ";");

  if (!fn->IsDefinition)
    return Tok;
  Locals = nullptr;

  enterScope();

  create_param_lvars(Ty->Params);
  fn->Params = Locals;

  Tok = skip(Tok, "{");

  fn->Body = compoundStmt(&Tok, Tok);
  fn->Locals = Locals;
  leaveScope();
  return Tok;
}

static TokenPtr global_variable(TokenPtr Tok, TypePtr basety) {
  bool first = true;
  while (!consume(&Tok, Tok, ";")) {
    if (!first)
      Tok = skip(Tok, ",");
    first = false;

    TypePtr Ty = declarator(&Tok, Tok, basety);
    Obj *var = new_gvar(get_ident(Ty->Name), Ty);
    if (equal(Tok, "=")) {
      var->InitScala = primary(&Tok, Tok->Next);
    }
  }
  return Tok;
}

// Lookahead tokens and returns true if a given token is a start
// of a function definition or declaration.
static bool IsFunction(TokenPtr Tok) {
  if (equal(Tok, ";"))
    return false;

  TypePtr dummy = std::make_shared<Type>();
  TypePtr Ty = declarator(&Tok, Tok, dummy);
  return Ty->Kind == TypeKind::TY_FUNC;
}

// program = (typedef | function-definition | global-variable)*
Obj *parse(TokenPtr Tok) {
  globals = nullptr;

  while (Tok->Kind != TK_EOF) {
    VarAttr attr = {};
    TypePtr basety = declspec(&Tok, Tok, &attr);

    // typedef
    if (attr.is_typedef) {
      Tok = parseTypedef(Tok, basety);
      continue;
    }
    // function
    if (IsFunction(Tok)) {
      Tok = function(Tok, basety);
      continue;
    }

    // global variable
    Tok = global_variable(Tok, basety);
  }
  return globals;
}
