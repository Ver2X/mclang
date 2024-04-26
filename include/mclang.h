#pragma once
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
  TokenKind Kind;
  TokenPtr Next;
  int64_t Val; // if Kind == TK_NUM, Val is the number
  char *loc;   // token location
  int len;     // token len
  TypePtr Ty;  // used if TK_STR
  char *str;   // string literal contents including terminating '\0'

  int line_no; // line number
};

void error(const char *fmt, ...);
void error_at(char *loc, const char *fmt, ...);
void error_tok(TokenPtr Tok, const char *fmt, ...);
bool equal(TokenPtr Tok, const char *op);
TokenPtr skip(TokenPtr Tok, const char *op);
TokenPtr tokenize_file(char *filename);
bool consume(TokenPtr *rest, TokenPtr Tok, const char *str);

#define unreachable() error("internal error at %s:%d", __FILE__, __LINE__)

char *format(const char *fmt, ...);

// for local variable or function
typedef struct Obj Obj;
struct Obj {
  Obj *Next;

  char *Name;
  bool IsLocal; // local or global/function
  TypePtr Ty;   // Type

  // Local variable
  int Offset; // from rbp

  // Global variable or function
  bool IsFunction;
  bool IsDefinition;

  // Global variable
  char *InitData;

  // Function
  Obj *Params;

  // Block or statement expression
  Node *Body;
  Obj *Locals;
  int StackSize;
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
  NodeKind Kind;

  Node *Lhs;
  Node *Rhs;
  Node *Next;
  TypePtr Ty;   // Type of AST varible node
  TokenPtr Tok; // representative token

  // "if" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  Node *Body;

  int64_t Val; // when Kind == ND_NUM
  Obj *Var;    // when Kind == ND_VAR

  // struct member access
  Member *member;

  // function call
  char *funcname;
  Node *args;
};

Obj *parse(TokenPtr Tok);

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
      : Kind(_kind), Size(_size), Align(_align) {}
  Type() = default;
  TypeKind Kind;

  int Size; // sizeof() value

  int Align;

  // Pointer-to or array-of type. We intentionally use the same member
  // to represent pointer/array duality in C.
  //
  // In many contexts in which a pointer is expected, we examine this
  // member instead of "Kind" member to determine whether a type is a
  // pointer or not. That means in many contexts "array of T" is
  // naturally handled as if it were "pointer to T", as required by
  // the C spec.
  TypePtr Base;

  // declaration
  TokenPtr Name;

  // array
  int ArrayLen;

  // struct
  Member *Members;

  // Function type
  TypePtr ReturnTy;
  TypePtr Params;
  TypePtr Next;
};

// struct member
struct Member {
  Member *Next;
  TypePtr Ty;
  TokenPtr Name;
  int Offset;
};

extern TypePtr TyChar;
extern TypePtr TyShort;
extern TypePtr TyInt;
extern TypePtr TyLong;

extern TypePtr TyVoid;

bool isInteger(TypePtr Ty);
void addType(Node *node);
TypePtr pointerTo(TypePtr Base);
TypePtr funcType(TypePtr ReturnTy);
TypePtr copyType(TypePtr Ty);
TypePtr arrayOf(TypePtr Base, int Size);

void codegen(Obj *prog, FILE *out, std::string file);
int alignTo(int n, int Align);

void emitIR(Obj *prog, std::string file_name);
void emitGlobalDataIR(Obj *prog);
std::string Twine(std::string &l, std::string &r);
std::string Twine(std::string l, std::string r);

/*
class IR {
public:
        int opcode;
        Operand * Left;
        Operand * Right;
        Operand * Result;
        int Align;
};*/
