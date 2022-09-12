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
#include <memory>
#include <vector>
#include <tuple>
#include <unordered_map>
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
	ND_BLOCK,	
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




enum class ReturnTypeKind
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
};


enum class VaribleKind
{
	VAR_8,
	VAR_16,
	VAR_32,
	VAR_64,
	VAR_PRT,
};


class Operand {
private:
		std::string name;
public:
	std::string& GetName() { return name; }
	void SetName(std::string name) { this->name = name; }
	void SetName(std::string & name) { this->name = name; }
	// SSA?
	int Ival;
	double Fval;
	VaribleKind type;

	int align;
	Operand * next;
	bool isConst;
	bool isInitConst;
	Operand()
	{
		next = NULL;
		align = 4;
		isConst = false;
		isInitConst = false;
	}

	Operand(int64_t v)
	{
		Ival = v;
		next = NULL;
		align = 8;
		isConst = true;
		name = std::to_string(Ival);
		type = VaribleKind::VAR_64;
	}	

	Operand(int v)
	{
		Ival = v;
		next = NULL;
		align = 4;
		isConst = true;
		name = std::to_string(Ival);
		type = VaribleKind::VAR_32;
	}	

	Operand(double v)
	{
		Fval = v;
		next = NULL;
		align = 8;
		isConst = true;
		name = std::to_string(Fval);
		type = VaribleKind::VAR_64;
	}	

	void SetConst(double v)
	{
		Fval = v;
		next = NULL;
		align = 8;
		isConst = true;
		name = std::to_string(Fval);
		type = VaribleKind::VAR_64;
	}	

	void SetConst(int v)
	{
		Ival = v;
		next = NULL;
		align = 4;
		isConst = true;
		name = std::to_string(Ival);
		type = VaribleKind::VAR_32;
	}	

	void SetConst(int64_t v)
	{
		Ival = v;
		next = NULL;
		align = 8;
		isConst = true;
		name = std::to_string(Ival);
		type = VaribleKind::VAR_64;
	}	

	std::string CodeGen();
};



class SymbolTable;
using Variable = Operand;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;
using VariablePtr = std::shared_ptr<Variable>;
using VarList = std::shared_ptr<std::vector<VariablePtr>>;

class SymbolTable{
public:
	// std::string name;
	// VariablePtr variables;
	int level;
	//SymbolTable * symb_list;
	//SymbolTable * symb_list_back_level;
	
	SymbolTablePtr symb_list;
	SymbolTablePtr symb_list_back_level;
	
	std::map<std::string,  VarList > table; // global

	SymbolTable()
	{

	}

	SymbolTable(SymbolTablePtr fa)
	{
		symb_list_back_level = fa;
		symb_list = std::make_shared<SymbolTable>();
	}

	bool insert(VariablePtr var,int level);
	bool insert(VariablePtr var,VariablePtr newVar, int level);
	// use a cache save inserted varibale, when leaving function, delete
	// it from symbol table
	void erase(std::string var_name,int level);
	bool findVar(std::string & var_name, VariablePtr &);
};

/*
class IR {
public:
	int opcode;
	Operand * left;
	Operand * right;
	Operand * result;
	int align;
};*/

class IRFunction{
public:
	IRFunction()
	{
		argsNum = 0;
		retTy = ReturnTypeKind::RTY_VOID; 
	}


	std::string functionName;
	// VariablePtr args;
	std::vector<VariablePtr> args;
	int argsNum;
	VariablePtr ret;
	ReturnTypeKind retTy;

	std::string rename();
	void AddArgs();
	std::string CodeGen();
	
};


enum class IROpKind
{
	Op_ADD,  		// +
	Op_SUB,  		// -
	Op_MUL,  		// *
	Op_DIV,  		// /
	Op_EQ, 	 		// ==
	Op_NE,   		// !=
	Op_LT,   		// <
	Op_LE,   		// <=
	Op_COMMA,		// ,
	Op_NEG,         // -, unary
	Op_ADDR,		// &, unary
	Op_DEREF,		// *, unary
	Op_barnch,  // branch
	Op_FUNCALL,	// function call
	Op_Alloca,  // allcoa
	Op_Store,   // store
	Op_Load,    // load futher use ??
	Op_STMT_EXPR,   // statement expression
	Op_MEMBER,			// . (struct member access)
};




class Instruction{

	IROpKind Op;
	VariablePtr left;
	VariablePtr right;
	VariablePtr result;
	int getAlign(VariablePtr left, VariablePtr right, VariablePtr result);

public:
	int Ival;
	Instruction(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op)
	{
		this->Op = Op;
		this->left = left;
		this->right = right;
		result->align = getAlign(left, right, result);
		this->result = result;

	}

	std::string CodeGen();
 };

class IRBuilder;

class Block{

	Block * preds;
	Block * succes;
	int label;
	std::string name;
	// entry blck is a special block
	std::vector<Instruction *> allocas;

	std::vector<Instruction *> instructinos;
public:
	Block()
	{
		preds = NULL;
		succes = NULL;
		label = 0;
	}

	void SetName(std::string name)
	{
		this->name = name;
	}
	void SetLabel(int label)
	{
		this->label = label;
	}
	void Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, IRBuilder * buider);
	void SetPred(Block *);
	void SetSucc(Block *);
	std::string CodeGen();
	std::string AllocaCodeGen();
};

class IRBuilder{

	VariablePtr globalVariable;
	IRFunction * function;
	// order by label
	std::map<int, Block *> blocks;
	int cache_label;
	int entry_label;
	std::string cache_name;
	int count_suffix;
public:
	IRBuilder()
	{
		cache_label = -1;
		entry_label = -1;
		globalVariable = NULL;
		function = NULL;
		count_suffix = 1;
	}
	int GetNextCountSuffix() { return count_suffix++; }
	void SetInsertPoint(int label, std::string name);
	void SetFunc(IRFunction * func) { function = func; }
	std::string CodeGen();
	// using label to index Blocks
	bool Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, int label, std::string name, SymbolTablePtr table);
	bool Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, SymbolTablePtr table);
	bool Insert(VariablePtr dest, IROpKind Op, SymbolTablePtr table);
	bool Insert(VariablePtr source, VariablePtr dest, IROpKind Op, SymbolTablePtr table);
};


void emit_ir(Obj * prog);
std::string Twine(std::string &l, std::string & r);
std::string Twine(std::string l, std::string r);
std::string next_variable_name();