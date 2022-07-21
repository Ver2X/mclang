#include <ctype.h> 
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// kinds of token
typedef enum
{
	TK_RESERVED, // Keywords or punctuators
	TK_NUM,
	TK_EOF,
}TokenKind;

typedef struct Token Token;

struct Token
{
	TokenKind kind;
	Token * next;
	int val;
	char * str;
	int len; // for string (a op b)
};

// current token
extern Token * token;

// inputs
extern char *user_input;


void error_at(char * loc, char * fmt, ...) ;
bool consume(char *op);
void expect(char *op);
int expect_number();
static bool at_eof();
static Token * new_token(TokenKind kind, Token * cur, char * str, int len);
static bool startswith(char *p, char *q);
Token * tokenize();




// Node type of AST
typedef enum
{
	ND_ADD,  // +
	ND_SUB,  // -
	ND_MUL,  // *
	ND_DIV,  // /
	ND_EQ, 	 // ==
	ND_NE,   // !=
	ND_LT,   // <
	ND_LE,   // <=
	ND_NUM,  // integer
}NodeKind;

typedef struct Node Node;

// Node of AST
struct Node
{
	NodeKind kind;
	Node * lhs;
	Node * rhs;
	int val; // when kind == ND_NUM
};




Node * expr();
static Node * equality();
static Node * relational();
static Node * add();
static Node * mul();
static Node * primary();
static Node * unary();




void gen(Node *node);