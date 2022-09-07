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
#include <string>
#include <map>
#include <fstream>
#include <iostream>
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


void error(const char *fmt, ...);
void error_at(char *loc, const char *fmt, ...);
void error_tok(Token *tok, const char *fmt, ...);
bool equal(Token *tok, const char *op);
Token *skip(Token *tok, const char *op);
Token *tokenize_file(char *filename);
bool consume(Token **rest, Token *tok, const char *str);


#define unreachable() \
  error("internal error at %s:%d", __FILE__, __LINE__)

char * format(const char * fmt, ...);



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
	bool is_definition;

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
	TY_SHORT,
	TY_LONG,
	TY_PTR,
	TY_FUNC,
	TY_ARRAY,
	TY_CHAR,
	TY_STRUCT,
	TY_UNION,
	TY_VOID,
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


extern Type * ty_char;
extern Type * ty_short;
extern Type * ty_int;
extern Type * ty_long;

extern Type * ty_void;

bool is_integer(Type * ty);
void add_type(Node *node);
Type * pointer_to(Type * base);
Type * func_type(Type *return_ty);
Type * copy_type(Type *ty);
Type * array_of(Type *base, int size);

void codegen(Obj *prog, FILE *out);
int align_to(int n, int align);




typedef enum
{
	RTY_INT,
	RTY_SHORT,
	RTY_LONG,
	RTY_PTR,
	RTY_ARRAY,
	RTY_CHAR,
	RTY_STRUCT,
	RTY_UNION,
	RTY_VOID,
}ReturnTypeKind;


typedef enum
{
	VAR_8,
	VAR_16,
	VAR_32,
	VAR_64,
	VAR_PRT,
}VaribleKind;


class Operand {
public:
	int Ival;
	double Fval;
	VaribleKind type;
	std::string name;
	Operand * next;
	Operand()
	{
		next = NULL;
	}

	std::string CodeGen()
	{

		std::string s;
		switch (type)
		{
			
			case VAR_8:
				s += "i8 ";
				break;
			case VAR_16:
				s += "i16 ";
				break;
			case VAR_32:
				s += "i32 ";
				break;
			case VAR_64:
				s += "i64 ";
				break;
			case VAR_PRT:
				s += "i32* ";
				break;
			default:
				break;
		}

		s += "%";
		s += name;
		return s;
	}

};

using Variable = Operand;

class SymbolTable{
public:
	std::string name;
	Variable * variables;
	int level;
	SymbolTable * next_level;
	std::map<std::string, Variable *> table;
	void insert(std::string var_name, Variable * var,int level)
	{
		// down to special level
		if(auto iter = table.find(var_name) != table.end())
		{
			// Error, variable redefine define
			return ;
		}

		table.insert(	make_pair(var_name, var) );

	}

	// use a cache save inserted varibale, when leaving function, delete
	// it from symbol table
	void erase(std::string var_name,int level)
	{
		auto iter = table.find(var_name);

		if(iter != table.end())
		{
			table.erase(iter);
		}
	}

	void find_var()
	{

	}
};


class IR {
public:
	int opcode;
	Operand * left;
	Operand * right;
	Operand * result;
	int align;
};

class IRFunction{
public:
	IRFunction()
	{
		argsNum = 0;
		retTy = RTY_VOID; 
	}


	std::string functionName;
	Variable * args;
	int argsNum;
	Variable * ret;
	ReturnTypeKind retTy;

	std::string rename(){
		return functionName;
	}
	void AddArgs()
	{
		// when enter function, need push varibale into symbol table
		// but when leave, destory it
	}
	std::string CodeGen()
	{
		// if body non-null
		std::string s;
		s += "define dso_local ";
		switch (retTy)
		{
			case RTY_VOID:
				s += "void ";
				break;
			case RTY_INT:
				s += "i32 ";
				break;
			case RTY_CHAR:
				s += "signext i8 ";
				break;
			case RTY_PTR:
				// dump de type
				s += "i32";
				s+="* ";
				break;	
			default:
				s += "void ";
				break;
		}
		s += "@";
		s += rename();
		s += "(";
		Variable * head = args;
		//s += "argsNum is :";
		//s += std::to_string(argsNum);
		//s += "\n";
		for(int i = 0; i < argsNum; i++)
		{
			assert(head != NULL);
			s += head->CodeGen();
			if(i != argsNum - 1)
			{
				s += ", ";
			}
			head = head->next;
		}
		s += ")";
		return s;
	}
};


class IRBuilder{
public:
	Variable * globalVariable;
	IRFunction * function;

	std::string CodeGen()
	{
		std::string s;
		if(function != NULL)
		{
			s += function->CodeGen();
		}
		return s;
	}
};

