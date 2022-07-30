#define _POSIX_C_SOURCE 200809L // https://xy2401.com/local-docs/gnu/manual.zh/libc/Feature-Test-Macros.html
#include <ctype.h> 
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

typedef struct Node Node;
typedef struct Type Type;
typedef struct Member Member;

// kinds of token
typedef enum
{
	TK_IDENT,
	TK_NUM,
	TK_EOF,
	TK_PUNCT,
	TK_KEYWORD,
	TK_STR,
}TokenKind;

typedef struct Token Token;

struct Token
{
	TokenKind 	kind;
	Token * 	next;
	int64_t		val;		// if kind == TK_NUM, val is the number
	char * 		loc;		// token location
	int 		len; 		// token len
	Type * ty;				// used if TK_STR
	char * str;				// string literal contents including terminating '\0'


	int line_no;			// line number
};


void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize_file(char *filename);
bool consume(Token **rest, Token *tok, char *str);


#define unreachable() \
  error("internal error at %s:%d", __FILE__, __LINE__)

char * format(char * fmt, ...);



// for local variable or function
typedef struct Obj Obj;
struct Obj
{
	Obj * next;

	char * name;
	bool is_local;	// local or global/function
	Type *ty;		// Type 

	// Local variable
	int offset; 	// from rbp

	// Global variable or function
	bool is_function;

	// Global variable
	char * init_data;

	// Function
	Obj * params;

	// Block or statement expression
	Node * body;
	Obj * locals;
	int stack_size;
};

// Node type of AST
typedef enum
{
	ND_ADD,  		// +
	ND_SUB,  		// -
	ND_MUL,  		// *
	ND_DIV,  		// /
	ND_EQ, 	 		// ==
	ND_NE,   		// !=
	ND_LT,   		// <
	ND_LE,   		// <=
	ND_ASSIGN, 		// =
	ND_COMMA,		// ,
	ND_NEG,         // -, unary
	ND_ADDR,		// &, unary
	ND_DEREF,		// *, unary
	ND_NUM,  		// integer
	ND_VAR, 		// local variable
	ND_EXPR_STMT,	// statement
	ND_RETURN,		// "return"
	ND_IF,			// "if"
	ND_FOR, 		// "for" or "while"
	ND_BLOCK,		// { }
	ND_FUNCALL,		// function call
	ND_STMT_EXPR,   // statement expression
	ND_MEMBER,		// . (struct member access)
}NodeKind;



// Node of AST
struct Node
{
	NodeKind kind;
	
	Node * lhs;
	Node * rhs;
	Node * next;
	Type * ty;		// Type of AST varible node
	Token * tok; 	// representative token

	// "if" or "for" statement
	Node * cond;
	Node * then;
	Node * els;
	Node * init;
	Node * inc;

	Node * body;

	int64_t val; 		// when kind == ND_NUM
	Obj *var; 		// when kind == ND_VAR
	

	// struct member access
	Member * member;

	// function call
	char * funcname;
	Node * args;
};

Obj *parse(Token *tok);



typedef enum
{
	TY_INT,
	TY_LONG,
	TY_PTR,
	TY_FUNC,
	TY_ARRAY,
	TY_CHAR,
	TY_STRUCT,
	TY_UNION,
}TypeKind;

struct Type
{
	TypeKind kind;

	int size;      // sizeof() value

	int align;

  	// Pointer-to or array-of type. We intentionally use the same member
	// to represent pointer/array duality in C.
	//
	// In many contexts in which a pointer is expected, we examine this
	// member instead of "kind" member to determine whether a type is a
	// pointer or not. That means in many contexts "array of T" is
	// naturally handled as if it were "pointer to T", as required by
	// the C spec.
	Type * base;

	// declaration
	Token * name;

	// array
	int array_len;

	// struct
	Member * members;

	// Function type
	Type * return_ty;
	Type *params;
  	Type *next;
};


// struct member
struct Member
{
	Member * next;
	Type * ty;
	Token  * name;
	int offset;
};



extern Type * ty_int;
extern Type * ty_char;
extern Type * ty_long;

bool is_integer(Type * ty);
void add_type(Node *node);
Type * pointer_to(Type * base);
Type * func_type(Type *return_ty);
Type * copy_type(Type *ty);
Type * array_of(Type *base, int size);

void codegen(Obj *prog, FILE *out);
int align_to(int n, int align);