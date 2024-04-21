#define _POSIX_C_SOURCE                                                        \
  200809L // https://xy2401.com/local-docs/gnu/manual.zh/libc/Feature-Test-Macros.html
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
typedef struct Node Node;
typedef struct Type Type;
typedef struct Member Member;
typedef struct Token Token;
using TokenPtr = std::shared_ptr<Token>;
using TypePtr = std::shared_ptr<Type>;

// kinds of token
typedef enum {
  TK_IDENT,
  TK_NUM,
  TK_EOF,
  TK_PUNCT,
  TK_KEYWORD,
  TK_STR,
} TokenKind;

struct Token {
  TokenKind kind;
  TokenPtr next;
  int64_t val; // if kind == TK_NUM, val is the number
  char *loc;   // token location
  int len;     // token len
  TypePtr ty;  // used if TK_STR
  char *str;   // string literal contents including terminating '\0'

  int line_no; // line number
};

void error(const char *fmt, ...);
void error_at(char *loc, const char *fmt, ...);
void error_tok(TokenPtr tok, const char *fmt, ...);
bool equal(TokenPtr tok, const char *op);
TokenPtr skip(TokenPtr tok, const char *op);
TokenPtr tokenize_file(char *filename);
bool consume(TokenPtr *rest, TokenPtr tok, const char *str);

#define unreachable() error("internal error at %s:%d", __FILE__, __LINE__)

char *format(const char *fmt, ...);

// for local variable or function
typedef struct Obj Obj;
struct Obj {
  Obj *next;

  char *name;
  bool is_local; // local or global/function
  TypePtr ty;    // Type

  // Local variable
  int offset; // from rbp

  // Global variable or function
  bool is_function;
  bool is_definition;

  // Global variable
  char *init_data;

  // Function
  Obj *params;

  // Block or statement expression
  Node *body;
  Obj *locals;
  int stack_size;
};

// Node type of AST
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_COMMA,     // ,
  ND_NEG,       // -, unary
  ND_ADDR,      // &, unary
  ND_DEREF,     // *, unary
  ND_NUM,       // integer
  ND_VAR,       // local variable
  ND_EXPR_STMT, // statement
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for" or "while"
  ND_BLOCK,
  ND_FUNCALL,   // function call
  ND_STMT_EXPR, // statement expression
  ND_MEMBER,    // . (struct member access)
} NodeKind;

// Node of AST
struct Node {
  NodeKind kind;

  Node *lhs;
  Node *rhs;
  Node *next;
  TypePtr ty;   // Type of AST varible node
  TokenPtr tok; // representative token

  // "if" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  Node *body;

  int64_t val; // when kind == ND_NUM
  Obj *var;    // when kind == ND_VAR

  // struct member access
  Member *member;

  // function call
  char *funcname;
  Node *args;
};

Obj *parse(TokenPtr tok);

typedef enum {
  TY_INT,
  TY_SHORT,
  TY_LONG,
  TY_PTR,
  TY_FUNC,
  TY_ARRAY,
  TY_CHAR,
  TY_STRUCT,
  TY_UNION,
  TY_VOID,
} TypeKind;

struct Type {
  Type(TypeKind _kind, int _size, int _align)
      : kind(_kind), size(_size), align(_align) {}
  Type() = default;
  TypeKind kind;

  int size; // sizeof() value

  int align;

  // Pointer-to or array-of type. We intentionally use the same member
  // to represent pointer/array duality in C.
  //
  // In many contexts in which a pointer is expected, we examine this
  // member instead of "kind" member to determine whether a type is a
  // pointer or not. That means in many contexts "array of T" is
  // naturally handled as if it were "pointer to T", as required by
  // the C spec.
  TypePtr base;

  // declaration
  TokenPtr name;

  // array
  int array_len;

  // struct
  Member *members;

  // Function type
  TypePtr return_ty;
  TypePtr params;
  TypePtr next;
};

// struct member
struct Member {
  Member *next;
  TypePtr ty;
  TokenPtr name;
  int offset;
};

extern TypePtr ty_char;
extern TypePtr ty_short;
extern TypePtr ty_int;
extern TypePtr ty_long;

extern TypePtr ty_void;

bool is_integer(TypePtr ty);
void add_type(Node *node);
TypePtr pointer_to(TypePtr base);
TypePtr func_type(TypePtr return_ty);
TypePtr copy_type(TypePtr ty);
TypePtr array_of(TypePtr base, int size);

void codegen(Obj *prog, FILE *out, std::string file);
int align_to(int n, int align);

void emit_ir(Obj *prog, std::string file_name);
void emit_global_data_ir(Obj *prog);
std::string Twine(std::string &l, std::string &r);
std::string Twine(std::string l, std::string r);
std::string next_variable_name();

/*
class IR {
public:
        int opcode;
        Operand * left;
        Operand * right;
        Operand * result;
        int align;
};*/
