#define _POSIX_C_SOURCE 200809L // https://xy2401.com/local-docs/gnu/manual.zh/libc/Feature-Test-Macros.html
#include <ctype.h> 
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Node Node;

// kinds of token
typedef enum
{
	TK_IDENT,
	TK_NUM,
	TK_EOF,
	TK_PUNCT,
	TK_KEYWORD,
}TokenKind;

typedef struct Token Token;

struct Token
{
	TokenKind 	kind;
	Token * 	next;
	int 		val;
	char * 		loc;		// token location
	int 		len; 		// token len
};


void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);


// for local variable
typedef struct Obj Obj;
struct Obj
{
	Obj * next;
	char * name;
	int offset; // from rbp
};

// function
typedef struct Function Function;
struct Function
{
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
	ND_NEG,         // - , unary
	ND_NUM,  		// integer
	ND_VAR, 		// local variable
	ND_EXPR_STMT,	// statement
	ND_RETURN,		// "return"
	ND_BLOCK,		// { }
}NodeKind;



// Node of AST
struct Node
{
	NodeKind kind;
	
	Node * lhs;
	Node * rhs;
	Node * next;

	Node * body;

	int val; 		// when kind == ND_NUM
	Obj *var; 		// when kind == ND_VAR
	
};

Function *parse(Token *tok);





void codegen(Function *prog);
