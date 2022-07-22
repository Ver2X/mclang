#include <ctype.h> 
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// kinds of token
typedef enum
{
	TK_IDENT,
	TK_NUM,
	TK_EOF,
	TK_PUNCT,
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
}NodeKind;

typedef struct Node Node;

// Node of AST
struct Node
{
	NodeKind kind;
	Node * lhs;
	Node * rhs;
	int val; 		// when kind == ND_NUM
	char name; 		// when kind == ND_VAR
	Node * next;
};

Node *parse(Token *tok);





void codegen(Node *node);
