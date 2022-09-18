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

// scope for local, global variable or typedefs
typedef struct VarScope VarScope;
struct VarScope{
  VarScope * next;
  char * name;
  Obj *var;

  Type *type_def;
};

// scope for struct or union tags 
typedef struct TagScope TagScope;
struct TagScope
{
	TagScope * next;
	char * name;
	Type * ty;
};


// represent a block scope
typedef struct Scope Scope;
struct Scope{
  Scope * next;
  
  VarScope * vars;	// vars

  TagScope * tags;	// structs
};

typedef struct{
	bool is_typedef;
}VarAttr;

// All locals variable, head insert method 
static Obj * locals;
static Obj * globals;

Scope scope__t = (Scope){};
static Scope * scope = &scope__t;

// rest use to skip symbol tokens, make it akways pointer to first token will deal.
// use rest, for 'cur = cur->next = stmt(&tok, tok);' right
//
// tok, make it akways pointer to first un-symbol token will deal.
static Node *expr(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);
static Node *new_node(NodeKind kind, Token * tok);
static Node *compound_stmt(Token **rest, Token *tok);
static Node *stmt(Token ** rest, Token * tok);
static Node *declaration(Token **rest, Token *tok, Type *basety);
static Type *declspec(Token **rest, Token *tok, VarAttr *attr);
static Type *declarator(Token **rest, Token *tok, Type *ty);
static Node *postfix(Token **rest, Token *tok);
static Type *struct_decl(Token **rest, Token *tok);
static Type *union_decl(Token **rest, Token *tok);
static bool is_typename(Token *tok);
static Token *parse_typedef(Token * tok, Type *basety);

static void enter_scope(void)
{
	Scope *sc = (Scope *)calloc(1, sizeof(Scope));
	sc->next = scope;
	scope = sc;
}

static void leave_scope(void)
{
	scope = scope->next;
}


static VarScope * find_var(Token * tok)
{
	for(Scope * sc = scope; sc; sc = sc->next)
	{
		for(VarScope * sc2 = sc->vars; sc2; sc2 = sc2->next)
			if(equal(tok, sc2->name))
				return sc2;
	}
	return nullptr;
}

static Type * find_tag(Token *tok)
{
	for(Scope * sc = scope; sc; sc = sc->next)
	{
		for(TagScope * sc2 = sc->tags; sc2; sc2 = sc2->next)
		{
			if(equal(tok, sc2->name))
				return sc2->ty;
		}
	}

	return nullptr;
}


static VarScope * push_scope(char * name)
{
	VarScope * sc = (VarScope *)calloc(1, sizeof(VarScope));
	sc->name = name;
	sc->next = scope->vars;	// VarScope * vars, head insert here
	scope->vars = sc;
	return sc;
}

static void push_tag_scope(Token *tok, Type *ty)
{
	TagScope * sc = (TagScope *)calloc(1, sizeof(TagScope));
	sc->name = strndup(tok->loc, tok->len);
	sc->ty = ty;
	sc->next = scope->tags;
	scope->tags = sc;
}


// head insert
static Obj * new_var(char * name, Type *ty)
{
	Obj * var = (Obj *)calloc(1, sizeof(Obj));
	var->name = name;
	var->ty = ty;
	push_scope(name)->var = var;
	return var;
}

// head insert, local variable
static Obj * new_lvar(char * name, Type *ty)
{
	Obj * var = new_var(name, ty);
	var->is_local = true;
	var->next = locals;
	locals = var;
	return var;
}

// head insert, global variable
static Obj * new_gvar(char * name, Type *ty)
{
	Obj * var = new_var(name, ty);
	var->next = globals;
	globals = var;
	return var;
}

static char * new_unique_name(void)
{
	static int id = 0;
	return format(".L..%d", id++);
}

static Obj * new_anon_gvar(Type * ty)
{
	return new_gvar(new_unique_name(), ty);
}

static Obj * new_string_literal(char * p, Type * ty)
{
	Obj * var = new_anon_gvar(ty);
	var->init_data = p;
	return var;
}

// create a new node by special kind
static Node *new_node(NodeKind kind, Token * tok) {
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

// binary operator
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token * tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// unary operator
static Node *new_unary(NodeKind kind, Node *expr, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

// create a number
static Node * new_num(int64_t val, Token * tok)
{
	Node *node = new_node(ND_NUM, tok);
	node->val = val;
	return node;
}

// create a variable
static Node * new_var_node(Obj * var, Token * tok)
{
	Node *node = new_node(ND_VAR, tok);
	node->var = var;
	return node;
}


static char * get_ident(Token *tok)
{
	if(tok->kind != TK_IDENT)
	{
		error_tok(tok, "expected an identifier");
	}

	return strndup(tok->loc, tok->len);
}

static Type *find_typedef(Token *tok)
{
	if(tok->kind == TK_IDENT)
	{
		VarScope * sc = find_var(tok);
		if(sc)
			return sc->type_def;
	}
	return nullptr;
}


static int get_number(Token * tok)
{
	if(tok->kind != TK_NUM)
	{
		error_tok(tok, "expected a number");
	}

	return tok->val;
}

// declspec = ("void" | "char" | "short" | "int" | "long"
//             | struct-decl | union-decl)+
//             | "typedef"
//             | struct-decl | union-decl | typedef-name)+
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
static Type * declspec(Token ** rest, Token *tok, VarAttr *attr)
{
	// We use a single integer as counters for all typenames.
	// For example, bits 0 and 1 represents how many times we saw the
	// keyword "void" so far. With this, we can use a switch statement
	// as you can see below.
	enum{
		VOID 	= 1 << 0,
		CHAR 	= 1 << 2,
		SHORT = 1 << 4,
		INT 	= 1 << 6,
		LONG 	= 1 << 8,
		OTHER = 1 << 10,
	};
	Type * ty = ty_int;
	int counter = 0;

	while(is_typename(tok))
	{
		if(equal(tok, "typedef"))
		{
			if(!attr)
				error_tok(tok, "storage class specifier is not allowed in this context");
			attr->is_typedef = true;
			tok = tok->next;
			continue;
		}

		// handle user-defined types
		Type * ty2 = find_typedef(tok);
		if(equal(tok, "struct") || equal(tok, "union") || ty2)
		{
			if(counter)
				break;

			if(equal(tok, "struct"))
				ty = struct_decl(&tok, tok->next);
			else if(equal(tok, "union"))
				ty = union_decl(&tok, tok->next);
			else
			{
				ty = ty2;
				tok = tok->next;
			}
			counter += OTHER;
			continue;
		}

		// handle built-in types
		if(equal(tok, "void"))
			counter	+= VOID;
		else if(equal(tok, "char"))
			counter += CHAR;
		else if(equal(tok, "short"))
			counter += SHORT;
		else if(equal(tok, "int"))
			counter += INT;
		else if(equal(tok, "long"))
			counter += LONG;
		else
			unreachable();

		switch(counter)
		{
			case VOID:
				ty = ty_void;
				break;
			case CHAR:
				ty = ty_char;
				break;
			case SHORT:
			case SHORT + INT:
				ty = ty_short;
				break;
			case INT:
				ty = ty_int;
				break;
			case LONG:
			case LONG + INT:
			case LONG + LONG:
			case LONG + LONG + INT:
				ty = ty_long;
				break;
			default:
				error_tok(tok, "invalid type");
		}
		tok = tok->next;
	}

	*rest = tok;
	return ty; // unreachable
}


static bool is_typename(Token *tok)
{
	static const char * kw[] = {"void", "char", "short", "int", "long", \
																					"struct", "union", "typedef"};
	for(int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
	{
		if(equal(tok, kw[i]))
			return true;
	}
	return find_typedef(tok);
}

// func-params = (param ("," param)*)? ")"
// param       = declspec declarator
static Type * func_params(Token **rest, Token*tok, Type * ty)
{
	Type head = {};
	Type * cur = &head;

	while(!equal(tok, ")"))
	{
		if(cur != &head)
		{
			tok = skip(tok, ",");			
		}

		Type * basety = declspec(&tok, tok, nullptr);
		Type * ty = declarator(&tok, tok, basety);
		cur = cur->next = copy_type(ty);
	}

	ty = func_type(ty);
	ty->params = head.next;
	*rest = tok->next;
	return ty;
}

// type-suffix = "(" func-params
//             | "[" num "]" type-suffix
//             | ε
static Type * type_suffix(Token ** rest, Token *tok, Type *ty)
{
	if(equal(tok, "("))
		return func_params(rest, tok->next, ty);

	if(equal(tok, "["))
	{
		int sz = get_number(tok->next);
		tok = skip(tok->next->next, "]");
		ty = type_suffix(rest, tok, ty);
		return array_of(ty, sz);
	}

	*rest = tok;
	return ty;
}

// declarator = "*"* ("(" ident ")" | "(" declarator ")" | ident) type-suffix
static Type * declarator(Token **rest, Token *tok, Type * ty)
{
	while(consume(&tok, tok, "*"))
	{
		ty = pointer_to(ty);
	}

	if(equal(tok, "("))
	{
		Token * start = tok;
		Type dummy = {};
		declarator(&tok, start->next, & dummy);
		tok = skip(tok, ")");
		ty = type_suffix(rest, tok, ty);
		return declarator(&tok, start->next, ty);
	}


	if(tok->kind != TK_IDENT)
		error_tok(tok, "expected a varibale name");

	ty = type_suffix(rest, tok->next, ty);
	ty->name = tok;
	return ty;
}

// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
static Type *abastract_declarator(Token **rest, Token *tok, Type *ty)
{
	while(equal(tok, "*"))
	{
		ty = pointer_to(ty);
		tok = tok->next;
	}
	if(equal(tok, "("))
	{
		Token *start = tok;
		Type dummy = {};
		abastract_declarator(&tok, start->next, &dummy);
		tok = skip(tok, ")");
		ty = type_suffix(rest, tok, ty);
		return abastract_declarator(&tok, start->next, ty);
	}

	return type_suffix(rest, tok, ty);
}

// type-name = declspec abstract-declarator
static Type * type_name(Token **rest, Token *tok)
{
	Type * ty = declspec(&tok, tok, nullptr);
	return abastract_declarator(rest, tok, ty);
}

// declaration = (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;
static Node * declaration(Token **rest, Token *tok, Type *basety)
{

	Node head = {};
	Node * cur = &head;


	int i = 0; // label to check whether is the first declaration varibale
	while(!equal(tok, ";"))
	{
		if(i++ > 0)
			tok = skip(tok, ",");

		// define but not used, add it to variable list
		Type * ty = declarator(&tok, tok, basety);
		if(ty->kind == TY_VOID)
			error_tok(tok, "varibale declared as void type");
		Obj * var = new_lvar(get_ident(ty->name), ty);

		if(!equal(tok, "="))
			continue;

		Node * lhs = new_var_node(var, ty->name);
		Node * rhs = assign(&tok, tok->next);
		Node * node = new_binary(ND_ASSIGN, lhs, rhs, tok);
		cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
	}

	Node * node = new_node(ND_BLOCK, tok);
	node->body = head.next;
	*rest = tok->next;
	return node;
}


// stmt = "return" expr ";"
//			| "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//			| expr_stmt
static Node * stmt(Token ** rest, Token * tok)
{
	if(equal(tok, "return"))
	{
		Node * node =  new_node(ND_RETURN, tok);
		node->lhs = expr(&tok, tok->next);
		*rest = skip(tok, ";");
		return node;
	}

	if(equal(tok, "if"))
	{
		Node * node = new_node(ND_IF, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(&tok, tok);
		if(equal(tok, "else"))
			node->els = stmt(&tok, tok->next);
		*rest = tok;
		return node;
	}

	if(equal(tok, "for"))
	{
		Node * node = new_node(ND_FOR, tok);
		tok = skip(tok->next, "(");

		node->init = expr_stmt(&tok, tok);	

		// here use !equal to judge whether cond is nullptr, e.x. for(init; ; inc)
		if(!equal(tok, ";"))
			node->cond = expr(&tok, tok);
		tok = skip(tok, ";");

		if(!equal(tok, ")"))
			node->inc = expr(&tok, tok);
		tok = skip(tok, ")");

		node->then = stmt(rest, tok);

		return node;
	}

	if(equal(tok, "while"))
	{
		Node * node = new_node(ND_FOR, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(rest, tok);
		return node;
	}

	if(equal(tok, "{"))
	{
		return compound_stmt(rest, tok->next);
	}
	return expr_stmt(rest, tok);
}

// compound-stmt = (typedef | declaration |stmt )* "}"
static Node * compound_stmt(Token ** rest, Token * tok)
{
	Node * node = new_node(ND_BLOCK, tok);

	Node head = {};
	Node * cur = &head;

	enter_scope();


	while(!equal(tok, "}")){
		if(is_typename(tok))
		{
			VarAttr attr = {};
			Type * basety = declspec(&tok, tok, &attr);

			if(attr.is_typedef)
			{
				tok = parse_typedef(tok, basety);
				continue;
			}

			cur = cur->next = declaration(&tok, tok, basety);
		}
		else
		{
			cur = cur->next = stmt(&tok, tok);
		}

		// here add type
		add_type(cur);
	}
	
	leave_scope();

	node->body = head.next;
	*rest = tok->next;
	return node;
}

// must expr_stmt could be nullptr, for support for init , e.x. for(; ; ;)
// expr_stmt       = expr? ";"
static Node * expr_stmt(Token ** rest, Token * tok)
{
	if(equal(tok, ";"))
		{
			*rest = tok->next;
			return new_node(ND_BLOCK, tok);
		}

	// expr(&tok, tok)), beacuse in parse() :  cur = cur->next = stmt(&tok, tok);
	Node * node = new_node(ND_EXPR_STMT, tok);
	node->lhs = expr(&tok, tok);
	*rest = skip(tok, ";"); 
	return node;
}

// expr       = assign ("," expr)
static Node * expr(Token ** rest, Token * tok)
{
	Node * node = assign(&tok, tok);
	if(equal(tok, ","))
		return new_binary(ND_COMMA, node, expr(rest, tok->next), tok);

	*rest = tok;
	return node;
}


// assign     = equality ("=" assign)?
Node * assign(Token ** rest, Token * tok)
{
	Node * node = equality(&tok, tok);
	if(equal(tok, "="))
		node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next), tok);
	// upward code tok->next, will skip token("=") in future call
	*rest = tok;
	return node;
}


// equality   = relational ("==" relational | "!=" relational)*
static Node * equality(Token ** rest, Token * tok)
{
	Node * node = relational(&tok, tok);
	
	for(;;)
	{
		Token * start = tok;
		if(equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relational(&tok, tok->next), start);
			continue;
		}

		if(equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relational(&tok, tok->next), start);
			continue;
		}
		
		*rest = tok;
		return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node * relational(Token ** rest, Token * tok)
{
	Node * node = add(&tok, tok);

	for(;;)
	{
		Token * start = tok;

		if(equal(tok, "<"))
		{
			node = new_binary(ND_LT, node, add(&tok, tok->next), start);
			continue;
		}
		
		if(equal(tok, "<="))
		{
			node = new_binary(ND_LE, node, add(&tok, tok->next), start);
			continue;
		}
		
		if(equal(tok, ">"))
		{
			node = new_binary(ND_LT, add(&tok, tok->next), node, start);
			continue;
		}
		
		if(equal(tok, ">="))
		{
			node = new_binary(ND_LE, add(&tok, tok->next), node, start);
			continue;
		}
		
		*rest = tok;
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
static Node * new_add(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);


	// num + num
	if(is_integer(lhs->ty) && is_integer(rhs->ty))
	{
		return new_binary(ND_ADD, lhs, rhs, tok);
	}

	if(lhs->ty->base && rhs->ty->base)
		error_tok(tok, "invalid operands");

	// canonicalize `num + ptr` to `ptr + num`
	if(!lhs->ty->base && rhs->ty->base)
	{
		Node * tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	// ptr + num
	rhs = new_binary(ND_MUL, rhs, new_num(lhs->ty->base->size, tok), tok);
	return new_binary(ND_ADD, lhs, rhs, tok);
}

// Like `+`, `-` is overloaded for the pointer type
static Node * new_sub(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);


	// num - num
	if(is_integer(lhs->ty) && is_integer(rhs->ty))
	{
		return new_binary(ND_SUB, lhs, rhs, tok);
	}
	
	// ptr - num
	if(lhs->ty->base && is_integer(rhs->ty))
	{
		rhs = new_binary(ND_MUL, rhs, new_num(lhs->ty->base->size, tok), tok);
		add_type(rhs);
		Node * node = new_binary(ND_SUB, lhs, rhs, tok);
		node->ty = lhs->ty;
		return node;
	}

	// ptr - ptr
	if(lhs->ty->base && rhs->ty->base)
	{
		Node * node = new_binary(ND_SUB, lhs, rhs, tok);
		node->ty = ty_int;
		return new_binary(ND_DIV, node, new_num(lhs->ty->base->size, tok), tok);
	}


	error_tok(tok, "invalid operands");

	return nullptr; // unreachable
}


// add        = mul ("+" mul | "-" mul)*
static Node * add(Token ** rest, Token * tok)
{
	Node * node = mul(&tok, tok);
	
	for(;;)
	{
		Token * start = tok;

		if(equal(tok, "+"))
		{
			node = new_add(node, mul(&tok, tok->next), start);
			continue;
		}
		
		if(equal(tok, "-"))
		{
			node = new_sub(node, mul(&tok, tok->next), start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

// mul     = unary ("*" unary | "/" unary)*
static Node *mul(Token ** rest, Token * tok)
{
	Node *node = unary(&tok, tok);

	for(;;)
	{
		Token * start = tok;

		if(equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, unary(&tok, tok->next), start);
			continue;
		}
		
		if(equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, unary(&tok, tok->next), start);
			continue;
		}
		

		*rest = tok;
		return node;		
	}
}


// unary   = ("+" | "-" | "&" | "*")? unary
//         | postfix
static Node* unary(Token ** rest, Token * tok)
{
	if(equal(tok, "+"))
		return unary(rest, tok->next);

	if(equal(tok, "-"))
		return new_unary(ND_NEG, unary(rest, tok->next), tok);

	if(equal(tok, "&"))
		return new_unary(ND_ADDR, unary(rest, tok->next), tok);

	if(equal(tok, "*"))
		return new_unary(ND_DEREF, unary(rest, tok->next), tok);

	return postfix(rest, tok);
}

// struct-members = (declspec declarator (","  declarator)* ";")*
static void struct_members(Token **rest, Token * tok, Type *ty)
{
	Member head = {};
	Member * cur = &head;

	while(!equal(tok, "}"))
	{
		Type * basety = declspec(&tok, tok, nullptr);
		int i = 0;

		while(!consume(&tok, tok, ";"))
		{
			if(i++)
				tok = skip(tok, ",");

			Member * mem = (Member *)calloc(1, sizeof(Member));
			mem->ty = declarator(&tok, tok, basety);
			mem->name = mem->ty->name;
			cur = cur->next = mem;
		}
	}

	*rest = tok->next;
	ty->members = head.next;
}

// struct-union-decl = ident? ("{" struct-union-members)?
static Type * struct_union_decl(Token **rest, Token *tok)
{
	// read a tag
	Token * tag = nullptr;
	if(tok->kind == TK_IDENT)
	{
		tag = tok;
		tok = tok->next;
	}

	// use to define a struct variable
	if(tag && !equal(tok, "{"))
	{
		Type * ty = find_tag(tag);
		if(!ty)
			error_tok(tag, "unknown struct type");
		*rest = tok;
		return ty;
	}



	// construct a struct object
	Type * ty = (Type *)calloc(1, sizeof(Type));
	ty->kind = TY_STRUCT;
	struct_members(rest, tok->next, ty);	// skip "}"
	ty->align = 1;

	// register the struct type if a name was give
	if(tag)
	{
		push_tag_scope(tag, ty);
	}
	return ty;
}
// struct-decl = struct-union-decl
static Type * struct_decl(Token **rest, Token *tok)
{
	Type * ty = struct_union_decl(rest, tok);
	ty->kind = TY_STRUCT;

	// assgin offsets
	int offset = 0;
	for(Member * mem = ty->members; mem; mem = mem->next)
	{
		offset = align_to(offset, mem->ty->align);
		mem->offset = offset;
		offset += mem->ty->size;

		if(ty->align < mem->ty->align)
		{
			ty->align = mem->ty->align;
		}

	}

	ty->size = align_to(offset, ty->align);
	return ty;
}


// union-decl = struct-union-decl
static Type * union_decl(Token **rest, Token *tok)
{
	Type * ty = struct_union_decl(rest, tok);
	ty->kind = TY_UNION;

	// for union, all offset is 0, but we need to compute the alignment
	// and size 

	for(Member * mem = ty->members; mem; mem = mem->next)
	{

		if(ty->align < mem->ty->align)
		{
			ty->align = mem->ty->align;
		}
		if(ty->size < mem->ty->size)
			ty->size = mem->ty->size;

	}
	ty->size = align_to(ty->size, ty->align);

	return ty;
}


static Member * get_struct_member(Type *ty, Token * tok)
{
	for(Member * mem = ty->members; mem; mem = mem->next)
	{
		if(mem->name->len == tok->len && !strncmp(mem->name->loc, tok->loc, tok->len))
		{
			return mem;
		}
	}

	error_tok(tok, "no such member");

	return nullptr;
}


static Node * struct_ref(Node * lhs, Token * tok)
{
	add_type(lhs);
	if(lhs->ty->kind != TY_STRUCT && lhs->ty->kind != TY_UNION)
		error_tok(lhs->tok, "not a struct nor union");

	Node * node =  new_unary(ND_MEMBER, lhs, tok);
	node->member = get_struct_member(lhs->ty, tok);
	return node;
}


// postfix = primary ("[" expr "]" | "." ident | "->" ident)*
static Node *postfix(Token **rest, Token * tok)
{
	Node * node = primary(&tok, tok);

	for(;;)
	{
		if(equal(tok, "["))
		{
			// x[y] ==> *(x+y)
			Token * start = tok;
			Node * idx = expr(&tok, tok->next);
			tok = skip(tok, "]");
			node = new_unary(ND_DEREF, new_add(node, idx, start), start);
			continue;
		}

		if(equal(tok, "."))
		{
			node = struct_ref(node, tok->next);
			tok = tok->next->next;
			continue;
		}

		if(equal(tok, "->"))
		{
			// x->t is short for (*x).y
			node = new_unary(ND_DEREF, node, tok);
			node = struct_ref(node, tok->next);
			tok = tok->next->next; // skip "->" and ident
			continue;
		}
		
		*rest = tok;
		
		return node;
	}


}


// funcall = ident "(" (assign ("," assign)*)? ")"
static Node * funcall(Token **rest, Token *tok)
{
	Token * start = tok; 		// point to ident
	tok = tok->next->next;	// point to first argument

	Node head = {};
	Node * cur = &head;

	while(!equal(tok, ")"))
	{
		if(cur != &head)
			tok = skip(tok, ",");

		cur = cur->next = assign(&tok, tok);
	} 

	*rest = skip(tok, ")");

	Node * node = new_node(ND_FUNCALL, start);
	node->funcname = strndup(start->loc, start->len);
	node->args = head.next;
	return node;
}


// primary = "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" "(" type-name ")"
//         | "sizeof" unary
//         | ident func-args?
//         | str
//         | num
static Node *primary(Token ** rest, Token * tok)
{
	Token * start = tok;

	if(equal(tok, "(") && equal(tok->next, "{"))
	{
		// this is a GNU statement expresssion
		Node * node = new_node(ND_STMT_EXPR, tok);
		node->body = compound_stmt(&tok, tok->next->next)->body;
		*rest = skip(tok, ")");
		return node;
	}

	if(equal(tok, "("))
	{
		// here is &tok, beacause expr change *rest = (frist node un deal)
		Node * node = expr(&tok, tok->next);
		*rest = skip(tok, ")");
		return node;
	}
	
	if(equal(tok, "sizeof") && equal(tok->next, "(") && 
													is_typename(tok->next->next))
	{
		Type * ty = type_name(&tok, tok->next->next);
		*rest = skip(tok, ")");
		return new_num(ty->size, start);
	}

	if(equal(tok, "sizeof"))
	{
		Node * node = unary(rest, tok->next);
		add_type(node);
		return new_num(node->ty->size, tok);
	}


	if(tok->kind == TK_IDENT)
	{
		// function call
		if(equal(tok->next, "("))
		{
			return funcall(rest, tok);
		}


		// variable
		VarScope * sc = find_var(tok);
		if(!sc || !sc->var){
			error_tok(tok, "undefined varibale");
		}
		*rest = tok->next;
		return new_var_node(sc->var, tok);
	}

	if(tok->kind == TK_STR)
	{
		Obj * var = new_string_literal(tok->str, tok->ty);
		*rest = tok->next;
		return new_var_node(var, tok);
	}

	if(tok->kind == TK_NUM)
	{
		Node * node = new_num(tok->val, tok);
		*rest = tok->next;
		return node;
	}

	error_tok(tok, "expected an expression");

	return nullptr;
}

static Token *parse_typedef(Token *tok, Type * basety)
{
	bool first = true;
	while(!consume(&tok, tok, ";"))
	{
		if(!first)
			tok = skip(tok, ",");
		first =false;

		Type * ty = declarator(&tok, tok, basety);
		push_scope(get_ident(ty->name))->type_def = ty;
	}
	return tok;
}

static void create_param_lvars(Type * param)
{
	if(param)
	{
    // recurse to last params
		create_param_lvars(param->next);
		// head insert, in variable list seq is same with seq decl variable
    new_lvar(get_ident(param->name), param);
	}
}


// function_declaration = declspec declarator "{" compound_stmt "}"
static Token * function(Token *tok, Type *basety)
{
	Type * ty = declarator(&tok, tok, basety);



	Obj *fn = new_gvar(get_ident(ty->name), ty);
	fn->is_function = true;
	fn->is_definition = !consume(&tok, tok, ";");

	if(!fn->is_definition)
		return tok;
	locals = nullptr;

	enter_scope();

	create_param_lvars(ty->params);
	fn->params = locals;

	tok = skip(tok, "{");

	fn->body = compound_stmt(&tok, tok);
	fn->locals = locals;
	leave_scope();
	return tok;
}

static Token * global_varibale(Token *tok, Type * basety)
{
	bool first = true;
	while(!consume(&tok, tok, ";"))
	{
		if(!first)
			tok = skip(tok, ",");
		first =false;

		Type * ty = declarator(&tok, tok, basety);
		new_gvar(get_ident(ty->name), ty);
	}
	return tok;
}

// Lookahead tokens and returns true if a given token is a start
// of a function definition or declaration.
static bool	is_function(Token * tok)
{
	if(equal(tok, ";"))
		return false;

		Type dummy = {};
		Type * ty = declarator(&tok, tok, &dummy);
		return ty->kind == TY_FUNC;
}

// program = (typedef | function-definition | global-variable)*
Obj * parse(Token * tok)
{
	globals = nullptr;


	while(tok->kind != TK_EOF){
		VarAttr attr = {};
		Type *basety = declspec(&tok, tok, &attr);

		// typedef
		if(attr.is_typedef)
		{
			tok = parse_typedef(tok, basety);
			continue;
		}
		// function 
		if(is_function(tok))
		{
			tok = function(tok, basety);
			continue;
		}

		// global variable
		tok = global_varibale(tok, basety);
	}
	return globals;
}
