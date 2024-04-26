# mclang

This is the reference implementation of https://www.sigbus.info/compilerbook.

link:  https://github.com/rui314/chibicc
## IR idea
ir is a struct

struct irbuilder;

struct Block;

struct inst
{
	node * Left;
	node * Right;
	emum op;
	int Align;

	Label * lb;
}

node * create(op, Left, Right)
{
	%3 = add %1 , %2
	node * rt = new node(3);
	rt->Left = %1;
	rt->Right = %2;
	rt->op = op;

	// here save instruction to a instruction list,array, generate linear ir
	// temp generate llvm ir, then use clang to compile it
	
	return *rt;
}

%1 , %2, ...


write a AST_traveler passing to a function operator
make it reuse, like ir gen, printer 

## TODO LIST:


support **Doxygen**

all move to C++

bind ir 'Res' to 'instruction'

IR gen function return a Result, don't passing by arg

index use slot

cfg pre-succ is set with create of branch inst automatically

support array

support void function

support type

add printAST function

like	

​		|

​		|

​		|

​				|

​				|

symbol Table using a 'idenity'

change Locals to a nest symbol, to support multi function

twine

support CHANGE LOG


Opt?:

SSA

DCE

CodeGen?:

## The syntax of C in Backus-Naur Form

```c
<translation-unit> ::= {<external-declaration>}*

<external-declaration> ::= <function-definition>
                         | <declaration>

<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>

<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>

<storage-class-specifier> ::= auto
                            | register
                            | static
                            | extern
                            | typedef

<type-specifier> ::= void
                   | char
                   | short
                   | int
                   | long
                   | float
                   | double
                   | signed
                   | unsigned
                   | <struct-or-union-specifier>
                   | <enum-specifier>
                   | <typedef-Name>

<struct-or-union-specifier> ::= <struct-or-union> <identifier> { {<struct-declaration>}+ }
                              | <struct-or-union> { {<struct-declaration>}+ }
                              | <struct-or-union> <identifier>

<struct-or-union> ::= struct
                    | union

<struct-declaration> ::= {<specifier-qualifier>}* <struct-declarator-list>

<specifier-qualifier> ::= <type-specifier>
                        | <type-qualifier>

<struct-declarator-list> ::= <struct-declarator>
                           | <struct-declarator-list> , <struct-declarator>

<struct-declarator> ::= <declarator>
                      | <declarator> : <constant-expression>
                      | : <constant-expression>

<declarator> ::= {<pointer>}? <direct-declarator>

<pointer> ::= * {<type-qualifier>}* {<pointer>}?

<type-qualifier> ::= const
                   | volatile

<direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )

<constant-expression> ::= <conditional-expression>

<conditional-expression> ::= <logical-or-expression>
                           | <logical-or-expression> ? <expression> : <conditional-expression>

<logical-or-expression> ::= <logical-and-expression>
                          | <logical-or-expression> || <logical-and-expression>

<logical-and-expression> ::= <inclusive-or-expression>
                           | <logical-and-expression> && <inclusive-or-expression>

<inclusive-or-expression> ::= <exclusive-or-expression>
                            | <inclusive-or-expression> | <exclusive-or-expression>

<exclusive-or-expression> ::= <and-expression>
                            | <exclusive-or-expression> ^ <and-expression>

<and-expression> ::= <equality-expression>
                   | <and-expression> & <equality-expression>

<equality-expression> ::= <relational-expression>
                        | <equality-expression> == <relational-expression>
                        | <equality-expression> != <relational-expression>

<relational-expression> ::= <shift-expression>
                          | <relational-expression> < <shift-expression>
                          | <relational-expression> > <shift-expression>
                          | <relational-expression> <= <shift-expression>
                          | <relational-expression> >= <shift-expression>

<shift-expression> ::= <additive-expression>
                     | <shift-expression> << <additive-expression>
                     | <shift-expression> >> <additive-expression>

<additive-expression> ::= <multiplicative-expression>
                        | <additive-expression> + <multiplicative-expression>
                        | <additive-expression> - <multiplicative-expression>

<multiplicative-expression> ::= <cast-expression>
                              | <multiplicative-expression> * <cast-expression>
                              | <multiplicative-expression> / <cast-expression>
                              | <multiplicative-expression> % <cast-expression>

<cast-expression> ::= <unary-expression>
                    | ( <type-Name> ) <cast-expression>

<unary-expression> ::= <postfix-expression>
                     | ++ <unary-expression>
                     | -- <unary-expression>
                     | <unary-operator> <cast-expression>
                     | sizeof <unary-expression>
                     | sizeof <type-Name>

<postfix-expression> ::= <primary-expression>
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++
                       | <postfix-expression> --

<primary-expression> ::= <identifier>
                       | <constant>
                       | <string>
                       | ( <expression> )

<constant> ::= <integer-constant>
             | <character-constant>
             | <floating-constant>
             | <enumeration-constant>

<expression> ::= <assignment-expression>
               | <expression> , <assignment-expression>

<assignment-expression> ::= <conditional-expression>
                          | <unary-expression> <assignment-operator> <assignment-expression>

<assignment-operator> ::= =
                        | *=
                        | /=
                        | %=
                        | +=
                        | -=
                        | <<=
                        | >>=
                        | &=
                        | ^=
                        | |=

<unary-operator> ::= &
                   | *
                   | +
                   | -
                   | ~
                   | !

<type-Name> ::= {<specifier-qualifier>}+ {<abstract-declarator>}?

<parameter-type-list> ::= <parameter-list>
                        | <parameter-list> , ...

<parameter-list> ::= <parameter-declaration>
                   | <parameter-list> , <parameter-declaration>

<parameter-declaration> ::= {<declaration-specifier>}+ <declarator>
                          | {<declaration-specifier>}+ <abstract-declarator>
                          | {<declaration-specifier>}+

<abstract-declarator> ::= <pointer>
                        | <pointer> <direct-abstract-declarator>
                        | <direct-abstract-declarator>

<direct-abstract-declarator> ::=  ( <abstract-declarator> )
                               | {<direct-abstract-declarator>}? [ {<constant-expression>}? ]
                               | {<direct-abstract-declarator>}? ( {<parameter-type-list>}? )

<enum-specifier> ::= enum <identifier> { <enumerator-list> }
                   | enum { <enumerator-list> }
                   | enum <identifier>

<enumerator-list> ::= <enumerator>
                    | <enumerator-list> , <enumerator>

<enumerator> ::= <identifier>
               | <identifier> = <constant-expression>

<typedef-Name> ::= <identifier>

<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;

<init-declarator> ::= <declarator>
                    | <declarator> = <initializer>

<initializer> ::= <assignment-expression>
                | { <initializer-list> }
                | { <initializer-list> , }

<initializer-list> ::= <initializer>
                     | <initializer-list> , <initializer>

<compound-statement> ::= { {<declaration>}* {<statement>}* }

<statement> ::= <labeled-statement>
              | <expression-statement>
              | <compound-statement>
              | <selection-statement>
              | <iteration-statement>
              | <jump-statement>

<labeled-statement> ::= <identifier> : <statement>
                      | case <constant-expression> : <statement>
                      | default : <statement>

<expression-statement> ::= {<expression>}? ;

<selection-statement> ::= if ( <expression> ) <statement>
                        | if ( <expression> ) <statement> else <statement>
                        | switch ( <expression> ) <statement>

<iteration-statement> ::= while ( <expression> ) <statement>
                        | do <statement> while ( <expression> ) ;
                        | for ( {<expression>}? ; {<expression>}? ; {<expression>}? ) <statement>

<jump-statement> ::= goto <identifier> ;
                   | continue ;
                   | break ;
                   | return {<expression>}? ;
```

This grammar was adapted from Section A13 of *The C programming language*, 2nd edition, by Brian W. Kernighan and Dennis M. Ritchie,Prentice Hall, 1988.



## CODE



### Step 17 : Support  declaration variable

for lexcer

```c++
// for local variable
typedef struct Obj Obj;
struct Obj
{
	Obj * Next;
	char * Name;
	int Offset; 	// from rbp
	TypePtr Ty;		// Type of local variable ,  new add
};

struct Type
{
	TypeKind Kind;
	TypePtr  Base;

	// declaration
	TokenPtr  Name;  // new add
};

```





for parser

declaration

type:

declspec = "int"

Left of declaration

declarator = "*"* ident

 declaration = declspec (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;

after ",",  >= 0 or more times, before >= 1 times

```c++
// declaration = declspec (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;
// compound-stmt = (declaration |stmt )* "}"
// => primary =  "(" expr ")" | ident | num 
```



```c++
// declaration = declspec (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;
static Node * declaration(TokenPtr *rest, TokenPtr Tok)
{
	TypePtr  basety = declspec(&Tok, Tok);

	Node head = {};
	Node * cur = &head;


	int i = 0; // Label to check whether is the first declaration varibale
	while(!equal(Tok, ";"))
	{
		if(i++ > 0)
			Tok = skip(Tok, ",");

		// define but not used, add it to variable list
		TypePtr  Ty = declarator(&Tok, Tok, basety);
		Obj * Var = new_lvar(get_ident(Ty->Name), Ty);

		if(!equal(Tok, "="))
			continue;

		Node * Lhs = new_var_node(Var, Ty->Name);
		Node * Rhs = assign(&Tok, Tok->Next);
		Node * node = new_binary(ND_ASSIGN, Lhs, Rhs, Tok);
		cur = cur->Next = new_unary(ND_EXPR_STMT, node, Tok);
	}

	Node * node = createNewNode(ND_BLOCK, Tok);
	node->Body = head.Next;
	*rest = Tok->Next;
	return node;
}

```

and

```c++
	
// => primary =  "(" expr ")" | ident | num 
static Node *primary(TokenPtr * rest, TokenPtr  Tok)
    ...
if(Tok->Kind == TK_IDENT)
	{
		Obj * Var = find_var(Tok);
		if(!Var){
			error_tok(Tok, "undefined varibale");
		}
		*rest = Tok->Next;
		return new_var_node(Var, Tok);
	}
```



then change the type.c



```c++
void addType(Node *node)
    ...

case ND_NUM:
			node->Ty = TyInt;
			return;
		case ND_VAR:
			// int, int *, int ** ...
			node->Ty = node->Var->Ty;
			return;

		// for '&', create a new type as TY_PTR, setup Base type
		case ND_ADDR:
			node->Ty = pointerTo(node->Lhs->Ty);
			return;
		case ND_DEREF:
			if(node->Lhs->Ty->Kind != TY_PTR)
			{
				error_tok(node->Tok, "expect dereference a pointer, but not");
				
			}
			// for '*', down a type level
			node->Ty = node->Lhs->Ty->Base;
			return;
```

`commit d5895ddaf3ebe4f90176295fb52f788d8446643b`



### Step 18 : Support  nullptr argument function call, don't support function define

To support function call, first we need change parser, notice temp argument list is nullptr

```c++
// before: primary =  "(" expr ")" | ident | num 
// after: primary =  "(" expr ")" | ident args? | num 
//           args = "(" ")"
```

for lexcer, add

```c++
// ND_FUNCALL
//
// and
//
// char *funcname;
```

just a call, assemby add  a "call" instruction 

for test

```c++
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
EOF

```

`69968644141c1a3c427950f0f26a42d8a268e881`

### Step 19 : Support function call with up to 6 arguments, don't support function define

for AST

need 

```c++
Node * args;
```

for parser

```c++
// before : primary =  "(" expr ")" | ident args?| num 
//			   args = "(" ")"
//
// primary = "(" expr ")" | ident func-args? | num
//
// funcall = ident "(" (assign ("," assign)*)? ")"  ---------- aux fucntion
```

for codegem, deal reg, pass argument use %rdi ~ %r9

`8aa1e08253cfed02c57c780fc05a5441f5f56ed0`

### Step 20: Support zero-arity function definition

for function definition

```c++
// function
typedef struct Function Function;
struct Function
{
	Node * Body;
	Obj * Locals;
	int StackSize;

	Function * Next;
	char * Name;
};
// add FUNC type
typedef enum
{
	TY_INT,
	TY_PTR,
	TY_FUNC,
}TypeKind;

struct Type
{
	TypeKind Kind;
	TypePtr  Base;

	// declaration
	TokenPtr  Name;

	// Function type
	TypePtr  ReturnTy;
};

```

for parser, before we only could declaration a varibale, now we also could declaration functions

```c++
//before:
// declspec = "int"
// declarator = "*"* ident
//

// declarator = "*"* ident type-suffix
// type-suffix = ("(" func-params ")")?
//
// for now, func-params = nullptr
```

besides, now we should parser function first, not statements

but for function definition. function also need a independent variable list

```c++
// function_declaration = declspec declarator "{" compoundStmt "}"
static Function * function(TokenPtr  Tok)
{
	TypePtr  Ty = declspec(&Tok, Tok);
	Ty = declarator(&Tok, Tok, Ty);

	Locals = nullptr;

	Function *fn = calloc(1, sizeof(Function));
	fn->Name = get_ident(Ty->Name);

	Tok = skip(Tok, "{");

	fn->Body = compoundStmt(rest, Tok);
	fn->Locals = Locals;
	return fn;
}
```

`27588a57a5233867f3fae96ee6355bd3b2d3f7bf`

### Step 21: Support function definition up to 6 parameters

for struct

```c++
// function
typedef struct Function Function;
struct Function
{	// ...
    Obj *Params;
};

struct Type
{
	TypeKind Kind;
	TypePtr  Base;

	// declaration
	TokenPtr  Name;

	// Function type
	TypePtr  ReturnTy;
	TypePtr Params;
  	TypePtr Next;
};
```

for parser

```c++
// type-suffix = ("(" func-params ")")?
//
// to
//
// type-suffix = ("(" func-params? ")")?
// func-params = param ("," param)*
// param       = declspec declarator
```

notice the sequence  of varibale created

```c++
static void create_param_lvars(TypePtr  param)
{
	if(param)
	{
        // recurse to last params
		create_param_lvars(param->Next);
		// head insert, in variable list seq is same with seq decl variable
        new_lvar(get_ident(param->Name, param));
	}
}
```

in codegen

```c++
void codegen(Function * prog)
{
	// first setup Offset
	assign_lvar_offsets(prog);

	for(Function * fn = prog; fn; fn = fn->Next)
	{
		printf("  .globl %s\n", fn->Name);
		printf("%s:\n", fn->Name);
		current_fn = fn;


		// prologue
		printf("  push %%rbp\n");
		printf("  mov %%rsp, %%rbp\n");
		printf("  sub $%d, %%rsp\n", fn->StackSize);

		// save passed-by-register arguments to the stack
		int i = 0;
		for(Obj * Var = fn->Params; Var; Var = Var->Next)
		{
			printf("  mov %s, %d(%%rbp)\n", argreg[i++], Var->Offset);
		}

		// emit code
		gen_stmt(fn->Body);
		assert(Depth == 0);

		// Epilogue
		printf(".L.return.%s:\n", fn->Name);
		printf("  mov %%rbp, %%rsp\n");
		printf("  pop %%rbp\n");
		printf("  ret\n");
	}
}
```

`bfc8877e86be8d5b669a49b34b0e2f2990cc9dd6`

### Step 22: Support one dimensional arrays

in C , array is a lanuage sugar, it is a pointer with number of element, but is could be modify "not a lvalue"

for AST and Size and len

```c++
typedef enum
{
	TY_INT,
	TY_PTR,
	TY_FUNC,
	TY_ARRAY,
}TypeKind;

struct Type
{
	TypeKind Kind;

	int Size;      // sizeof() value

  	// Pointer-to or array-of type. We intentionally use the same member
	// to represent pointer/array duality in C.
	//
	// In many contexts in which a pointer is expected, we examine this
	// member instead of "Kind" member to determine whether a type is a
	// pointer or not. That means in many contexts "array of T" is
	// naturally handled as if it were "pointer to T", as required by
	// the C spec.
	TypePtr  Base;

	// declaration
	TokenPtr  Name;

	// array
	int ArrayLen;

	// Function type
	TypePtr  ReturnTy;
	TypePtr Params;
  	TypePtr Next;
};
```



for supoort arrays

we need to change the parser make it support "[ num ]", besides make int => Size = 8, maintain Size set Right

```c++
// func-params = (param ("," param)*)? ")"
// param       = declspec declarator
static TypePtr  func_params(TokenPtr *rest, Token*Tok, TypePtr  Ty)
{
	Type head = {};
	TypePtr  cur = &head;

	while(!equal(Tok, ")"))
	{
		if(cur != &head)
		{
			Tok = skip(Tok, ",");			
		}

		TypePtr  basety = declspec(&Tok, Tok);
		TypePtr  Ty = declarator(&Tok, Tok, basety);
		cur = cur->Next = copyType(Ty);
	}

	Ty = funcType(Ty);
	Ty->Params = head.Next;
	*rest = Tok->Next;
	return Ty;
}
// type-suffix = "(" func-params
//             | "[" num "]"
//             | ε

static TypePtr  type_suffix(TokenPtr * rest, TokenPtr Tok, TypePtr Ty)
{
	if(equal(Tok, "("))
		return func_params(rest, Tok->Next, Ty);

	if(equal(Tok, "["))
	{
		int sz = get_number(Tok->Next);
		*rest = skip(Tok->Next->Next, "]");
		return arrayOf(Ty, sz);
	}

	*rest = Tok;
	return Ty;
}
```

`bfc8877e86be8d5b669a49b34b0e2f2990cc9dd6`

### Step 23: Support sizeof()

in lexer, make it reconize "sizeof" as a keyword

because before we have add  "Size" int Type, change parser just return the correspond number

```c++
// primary =  "(" expr ")" | ident func-args?| num 
//
// to
//
// primary = "(" expr ")" | "sizeof" unary | ident func-args? | num
static Node *primary(TokenPtr * rest, TokenPtr  Tok)
{
    // ...
    if(equal(Tok, "sizeof"))
	{
		Node * node = unary(rest, Tok->Next);
		addType(node);
		return new_num(node->Ty->Size, Tok);
	}
    // ...
}
```

`3c7242041444a16919c2d85a0b939bdd94113cce`

### Step 24: Support arrays of arrays

recursive parser

```c++
// type-suffix = "(" func-params
//             | "[" num "]" type-suffix
//             | ε
static TypePtr  type_suffix(TokenPtr * rest, TokenPtr Tok, TypePtr Ty)
{
	if(equal(Tok, "("))
		return func_params(rest, Tok->Next, Ty);

	if(equal(Tok, "["))
	{
		int sz = get_number(Tok->Next);
		Tok = skip(Tok->Next->Next, "]");
		Ty = type_suffix(rest, Tok, Ty);
		return arrayOf(Ty, sz);
	}

	*rest = Tok;
	return Ty;
}
```

`3c7242041444a16919c2d85a0b939bdd94113cce`

### Step 25: Support [] operator

it is a post operator, just change parser

```c++
// unary   = ("+" | "-" | "&" | "*")? unary
//         | postfix
static Node* unary(TokenPtr * rest, TokenPtr  Tok)
{
	if(equal(Tok, "+"))
		return unary(rest, Tok->Next);

	if(equal(Tok, "-"))
		return new_unary(ND_NEG, unary(rest, Tok->Next), Tok);

	if(equal(Tok, "&"))
		return new_unary(ND_ADDR, unary(rest, Tok->Next), Tok);

	if(equal(Tok, "*"))
		return new_unary(ND_DEREF, unary(rest, Tok->Next), Tok);

	return postfix(rest, Tok);
}

// postfix = primary ("[" expr "]")*
static Node *postfix(TokenPtr *rest, TokenPtr  Tok)
{
	Node * node = primary(&Tok, Tok);

	while(equal(Tok, "["))
	{
		// x[y] ==> *(x+y)
		TokenPtr  start = Tok;
		Node * idx = expr(&Tok, Tok->Next);
		Tok = skip(Tok, "]");
		node = new_unary(ND_DEREF, new_add(node, idx, start), start);
	}
	*rest = Tok;
	return node;
}
```



### Step26: Merge Function with Var

(reoder , not real diff)

```diff
// for local variable or function
typedef struct Obj Obj;
struct Obj
{
	Obj * Next;
	char * Name;
	int Offset; 	// from rbp
	TypePtr Ty;		// Type of local variable
-};

-// function
-typedef struct Function Function;
-struct Function
-{
	Node * Body;
	Obj * Locals;
	int StackSize;

+	bool IsFunction;
-	Function * Next;
-	char * Name;
	Obj *Params;
};
```

`5962a3645688d81345bc938404a46549c32011ca`

### Step 27:Support global variables

for parser deal global varibale

```c++
// program = (function-definition | global-variable)*
Obj * parse(TokenPtr  Tok)
{
	globals = nullptr;


	while(Tok->Kind != TK_EOF){
		TypePtr basety = declspec(&Tok, Tok);

		// function 
		if(IsFunction(Tok))
		{
			Tok = function(Tok, basety);
			continue;
		}

		// global variable
		Tok = global_varibale(Tok, basety);
	}
	return globals;
}
```

for codegen()

```c++
// emit global data
static void emit_data(Obj * prog)
{
	for(Obj * Var = prog; Var; Var = Var->Next)
	{
		if(Var->IsFunction)
			continue;
		printf("  .data\n");
		printf("  .globl %s\n", Var->Name);
		printf("%s:\n",Var->Name);
		printf("  .zero %d\n", Var->Ty->Size);
	}
}
```

`7cb83c641af402c2f449178b9857e44605ba4e49`

### Step 28: Support char

for now, use think char is integer

```c++
// declspec = "char" | "int"
static TypePtr  declspec(TokenPtr * rest, TokenPtr Tok)
{
	if(equal(Tok, "char"))
	{
		*rest = Tok->Next;
		return TyChar;
	}
	*rest = skip(Tok, "int");
	return TyInt;
}
```

notice we need use 8bit reg, not 64bit, such as

```c++
static void emit_text(Obj * prog)
{
	for(Obj * fn = prog; fn; fn = fn->Next)
	{
		if(!fn->IsFunction)
			continue;

		printf("  .globl %s\n", fn->Name);
		printf("  .text\n");
		printf("%s:\n", fn->Name);
		current_fn = fn;


		// prologue
		printf("  push %%rbp\n");
		printf("  mov %%rsp, %%rbp\n");
		printf("  sub $%d, %%rsp\n", fn->StackSize);

		// save passed-by-register arguments to the stack
		int i = 0;
		for(Obj * Var = fn->Params; Var; Var = Var->Next)
		{
			if(Var->Ty->Size == 1)
				printf("  mov %s, %d(%%rbp)\n", argreg8[i++], Var->Offset);
			else
				printf("  mov %s, %d(%%rbp)\n", argreg64[i++], Var->Offset);
		}

		// emit code
		gen_stmt(fn->Body);
		assert(Depth == 0);

		// Epilogue
		printf(".L.return.%s:\n", fn->Name);
		printf("  mov %%rbp, %%rsp\n");
		printf("  pop %%rbp\n");
		printf("  ret\n");
	}
}
```

`b1847c506ae01823ef44c5fa59d93b03296f5ea2`

### Step29: Support string literal

for lexcer

```c++

struct Token
{
	// ...
	TypePtr  Ty;				// used if TK_STR
	char * str;				// string literal contents including terminating '\0'
};

// for local variable or function
typedef struct Obj Obj;
struct Obj
{
	// ...
    
	// Global variable
	char * InitData;

	// ...
};
```



for parser

```c++
static char * new_unique_name(void)
{
	static int id = 0;
	char * buf = calloc(1, 20);
	sprintf(buf, ".L..%d", id++);
	return buf;
}

static Obj * new_anon_gvar(TypePtr  Ty)
{
	return new_gvar(new_unique_name(), Ty);
}

static Obj * new_string_literal(char * p, TypePtr  Ty)
{
	Obj * Var = new_anon_gvar(Ty);
	Var->InitData = p;
	return Val;
}



// primary = "(" expr ")" | "sizeof" unary | ident func-args? |  str | num
```

`f8cf408c2bfd42faf2c7cd065a47caeff68ef89c`

### string aux function

```c++
// takes a printf-style format string and returns a formatted string.
char * format(char * fmt, ...)
{
	char * buf;
	size_t buflen;
	FILE * out = open_memstream(&buf, &buflen);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);
	fclose(out);
	return buf;
}
```

`5f01c23b6bcec6458f13f2adc9529926a7e48340`

###  Step 30:  Support \a, \b, \t, \n \v, \f, \r and \e

```c++
static int read_escaped_char(char *p) {
  // Escape sequences are defined using themselves here. E.g.
  // '\n' is implemented using '\n'. This tautological definition
  // works because the compiler that compiles our compiler knows
  // what '\n' actually is. In other words, we "inherit" the ASCII
  // code of '\n' from the compiler that compiles our compiler,
  // so we don't have to teach the actual code here.
  //
  // This fact has huge implications not only for the correctness
  // of the compiler but also for the security of the generated code.
  // For more info, read "Reflections on Trusting Trust" by Ken Thompson.
  // https://github.com/rui314/chibicc/wiki/thompson1984.pdf
  switch (*p) {
  case 'a': return '\a';
  case 'b': return '\b';
  case 't': return '\t';
  case 'n': return '\n';
  case 'v': return '\v';
  case 'f': return '\f';
  case 'r': return '\r';
  // [GNU] \e for the ASCII escape character is a GNU C extension.
  case 'e': return 27;
  default: return *p;
  }
}
```

`0b7f19c9850ac1bcd10881de64d3661888200035`

### Step 31:  Support  \<octal-sequence>

chang the function in tokenize, we need change the pos of token

```c++
static int read_escaped_char(char ** new_pos, char *p) {
	if('0' <= *p && *p <= '7')
	{
		// reand an octalnumber.
		int c = *p++ - '0';
		if('0' <= *p && *p <= '7')
		{
			c = (c <<3 ) + (*p++ - '0');
			if('0' <= *p && *p <= '7')
				c = (c << 3) + (*p++ - '0');
		}
		*new_pos = p;
		return c;
	}
	// ...
}
```

`2805f768e2605029bd1819ac00974630ee38e78b`

### Step 32:  Support   \x<hexadecimal-sequence>

```c++
static int from_hex(char c)
{
	if('0' <= c && c <= '9')
		return c - '0';
	if('a' <= c && c <= 'f')
		return c - 'a' + 10;
	return c - 'A' + 10;
}
```

`bd66b632f4d73cfced4a97c3a45dee879a847ebf`

### Step 33: Support [GNU] Add statement expression

GUN allow write epxression like

```c++
int a = ({int c = 1; c + 2;});
```

change parser and codegen and type addType() function

```c++
// primary = "(" expr ")" | "sizeof" unary | ident func-args? |  str | num
//
// to
//
// primary = "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" unary
//         | ident func-args?
//         | str
//         | num
static Node *primary(TokenPtr * rest, TokenPtr  Tok)
{

	if(equal(Tok, "(") && equal(Tok->Next, "{"))
	{
		// this is a GNU statement expresssion
		Node * node = createNewNode(ND_STMT_EXPR, Tok);
		node->Body = compoundStmt(&Tok, Tok->Next->Next)->Body;
		*rest = skip(Tok, ")");
		return node;
	}
	// ...
}
```

`274d26227af23175e57a1f9e7127149375126961`

### Step 34: Support read from file

read from file, need redirection

`c9b848b2f8878a2a10699f7c876739ab88fd23ca`

### Step 35: little refactor

printf  + '\n' to println

```c++
static void println(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}
```

`b90d71f9436683397e0726082dcf8f8f12298549`

### Step 36: Support -o and --help options

```c++
static char * opt_o;

static char * input_path;

static void usage(int status)
{
	fprintf(stderr, "chibicc [ -o <path>] <file> \n");
	exit(status);
}

static void parse_args(int argc, char ** argv)
{
	for(int i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--help"))
			usage(0);

		if(!strcmp(argv[i], "-o"))
		{
			if(!argv[++i])
				usage(1);
			opt_o = argv[i];
			continue;
		}

		if(!strncmp(argv[i], "-o", 2))
		{
			opt_o = argv[i] + 2;
			continue;
		}

		if(argv[i][0] == '-' && argv[i][1] != '\0')
			error("unknown argument: %s", argv[i]);

		input_path = argv[i];
	}

	if(!input_path)
		error("no input files");
}
```

`4d32c17c2acd5d27a9a42c5607e3ef2eb6062ae8`

### Step 37: Support line and Block comments

just jum it when tokenize

The C library function **char \*strstr(const char \*haystack, const char \*needle)** function finds the first occurrence of the substring **needle** in the string **haystack**. The terminating '\0' characters are not compared.

```c++
	// skip line comments
	if(startswith(p, "//"))
	{
		p += 2;
		while(*p != '\n')
			p++;
		continue;
	}

	// skip Block comments
	if(startswith(p, "/*"))
	{
		char * q = strstr(p + 2, "*/");
		if(!q)
			error_at(p, "unclosed Block comment");
		p = q + 2;
	}
```

`da07308f95da1914d925eef8159198e84898d3aa`

### Step 38: Support Block Scope

remind how we deal variable before

a. every time call new_lvar() will add a varibale to Locals varibale list 

b.

1. when call parse first add global variable by global_varibale()
2. when parse function(), wuk first add function Name as global varibale and add function params (by create_param_lvars()) to local varibale list then call **compoundStmt()** which call declaration() , then declspec() and declarator() judge variable type and set Right variable Name, then return to declaration() call new_lvar() to create local variable

parse --- > global_varibale

parse --- > function ---> compoundStmt ---> declaration ---> new_lvar

then if variable have been define, change it

```c++
// stmt = "return" expr ";"
//		| "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//		| exprStmt

// function_declaration = declspec declarator "{" compoundStmt "}"
```

but actually in C, the varibale save as a nest list

```c++
// level 0
{
    // level 1
    {
        // level 2
        {
            // level 3
            {
                // level 4
                
            }
        }
        // level 2
        {
            // level 3
            
        }
    }
    // level 1
    {
        // level 2
        
    }
}
```



```c++
// compound-stmt = (declaration | stmt)* "}"
// function_declaration = declspec declarator "{" compoundStmt "}"
```

the code of from Scope,

struct BlockScope, present a Block Scope, init it present global variable Scope,

and every time meet '{' enter a level Scope, and meet '}' exit a level of Scope, VarScope present variable list 

```c++
// Scope for local or global variable
typedef struct VarScope VarScope;
struct VarScope{
  VarScope * Next;
  char * Name;
  Obj *Var;
};

// represent a Block Scope
typedef struct BlockScope BlockScope;
struct BlockScope{
  BlockScope * Next;
  VarScope * vars;
};

static Obj * find_var(TokenPtr  Tok)
{
	for(BlockScope * sc = Scope; sc; sc = sc->Next)
	{
		for(VarScope * sc2 = sc->vars; sc2; sc2 = sc2->Next)
			if(equal(Tok, sc2->Name))
				return sc2->Var;
	}
	return nullptr;
}

static void enterScope(void)
{
	BlockScope *sc = calloc(1, sizeof(BlockScope));
	sc->Next = Scope;
	Scope = sc;
}

static void leaveScope(void)
{
	Scope = Scope->Next;
}

static VarScope * push_scope(char * Name, Obj *Var)
{
	VarScope * sc = calloc(1, sizeof(VarScope));
	sc->Name = Name;
	sc->Var = Var;
	sc->Next = Scope->vars;	// VarScope * vars, head insert here
	Scope->vars = sc;
}
```

`6631123631ea2ab164927aebcb1af77d221d3e48`

### Step 39: Rewrite tests in shell script in C

```makefile
CFLAGS=-std=c11 -g -fno-common

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)


chibicc: $(OBJS)
		$(CC) -o chibicc $(OBJS) $(LDFLAGS)	

$(OBJS): chibicc.h

test/%.exe: chibicc test/%.c
		$(CC) -o- -E -P -C test/$*.c | ./chibicc -o test/$*.s -
		$(CC) -o $@ test/$*.s -xc test/common

test: $(TESTS)
		for i in $^; do echo $$i; ./$$i || exit 1; echo; done	
		test/driver.sh
	

clean:
		rm -f chibicc tmp*  $(TESTS) test/*.s test/*.exe
		find * -type f '(' -Name '*~' -o -Name '*.o' ')' -exec rm {} ';'

.PHONU: test clean

```

`0bca9f751b2969e71dd599420b121133e730151a`

### Precompute line number for each token

$O(n^2) => O(n)$

`1dd304f3c8196f7b6da57f91b44a892d0d3a02ee`

### Emit .file and .loc assembler directives

With these directives, gdb can print out an error location when a compiled program crashes.

`88684ce9e881c1ba893530646f1d0f75d85dfa3e`

### Step 40: Support comma operator

This patch allows writing a comma expression on the Left-hand side of an assignment expression. This is called the "generalized lvalue" which is a deprecated GCC language extension, implementing it anyway because it's useful to implement other features.

```c++
// expr       = assign
// assign     = equality ("=" assign)?
//
// to
//
// expr = assign ("," expr)?
```

make below Right

```c+
  ASSERT(3, (1,2,3));
  ASSERT(5, ({ int i=2, j=3; (i=5,j)=6; i; }));
  ASSERT(6, ({ int i=2, j=3; (i=5,j)=6; j; }));
```

which return a mutable variable

```c++
// expr       = assign ("," expr)
static Node * expr(TokenPtr * rest, TokenPtr  Tok)
{
	Node * node = assign(&Tok, Tok);
	if(equal(Tok, ","))
		return new_binary(ND_COMMA, node, expr(rest, Tok->Next), Tok);

	*rest = Tok;
	return node;
}
```

`343df57590ee2b13bae5586e361cbf0c6245bb45`

### Step 41: Support struct 

for now, when meet struct's "{" we don't need enter Scope, actually struct have its own Scope

and struct just is a anonymous variable type, no tag

```c++
// declspec = "char" | "int" | struct-decl
// struct-decl = "{" struct-Members
// struct-Members = (declspec declarator (","  declarator)* ";")*


// postfix = primary ("[" expr "]")*
//
// to
//
// postfix = primary ("[" expr "]" | "." ident)*
```



```c++
struct Type {
@@ -169,12 +175,23 @@ struct Type {
  // ...
  // Struct
  Member *Members;
  // ...
};

// Struct member
struct Member {
  Member *Next;
  TypePtr Ty;
  TokenPtr Name;
  int Offset;
};

```

`e562ff5a90f5eee001622409ed8319c2f31ed158`

### Step 42: Align struct Members

Align of a struct is the max ocopied space Align

add a `int Align` to Type, and maintain it, besides Align Offset to `Align`

which mean

```c
  ASSERT(16, ({ struct {char a; int b;} x; sizeof(x); }));
  ASSERT(16, ({ struct {int a; char b;} x; sizeof(x); }));
```

`74b5f74e3f4aa463670f2906f8f18d36ad45d3a0`

### Step 43: Align local variables

when set vriable Offset in codegen, just Align it.

```c++
Offset = alignTo(Offset, Var->Ty->Align);
```

`2733964ec177e24d31c7f6d2bb178bc7943a2466`

### Step 44: Support struct tags and Scope

```c++
// struct-decl = "{" struct-Members
//
// to
// struct-decl = ident? ("{" struct-Members)?
static TypePtr  structDecl(TokenPtr *rest, TokenPtr Tok)
{
	// read a truct tag
	// define
	TokenPtr  tag = nullptr;
	if(Tok->Kind == TK_IDENT)
	{
		tag = Tok;
		Tok = Tok->Next;
	}

	// use to define a struct variable
	if(tag && !equal(Tok, "{"))
	{
		TypePtr  Ty = find_tag(tag);
		if(!Ty)
			error_tok(tag, "unknown struct type");
		*rest = Tok;
		return Ty;
	}



	// construct a struct object
	TypePtr  Ty = calloc(1, sizeof(Type));
	Ty->Kind = TY_STRUCT;
	struct_members(rest, Tok->Next, Ty);	// skip "}"
	Ty->Align = 1;

	int Offset = 0;
	for(Member * mem = Ty->Members; mem; mem = mem->Next)
	{
		Offset = alignTo(Offset, mem->Ty->Align);
		mem->Offset = Offset;
		Offset += mem->Ty->Size;

		if(Ty->Align < mem->Ty->Align)
		{
			Ty->Align = mem->Ty->Align;
		}

	}
	Ty->Size = alignTo(Offset, Ty->Align);

	// register the struct type if a Name was given.
	if(tag)
		push_tag_scope(tag, Ty);
	return Ty;
}
```

`55fef4513cb3c360b8a2f7a3d034559a161993ef`

### Step 45: Support -> operator

which is a postfix operator

```c++
		if(equal(Tok, "->"))
		{
			// x->t is short for (*x).y
			node = new_unary(ND_DEREF, node, Tok);
			node = struct_ref(node, Tok->Next);
			Tok = Tok->Next->Next; // skip "->" and ident
			continue;
		}
```

`601526fb1c19bff3f0eea5374a9532bd70562b54`

### Step 46: Support union

the way to handle union is like handle struct, for union, all Offset is 0, but we need to compute the alignment and Size.

all variable share the space

```c++
// struct-union-decl = ident? ("{" struct-Members)?
// union-decl = struct-union-decl
static TypePtr  unionDecl(TokenPtr *rest, TokenPtr Tok)
{
	TypePtr  Ty = struct_union_decl(rest, Tok);
	Ty->Kind = TY_UNION;

	// for union, all Offset is 0, but we need to compute the alignment
	// and Size 

	for(Member * mem = Ty->Members; mem; mem = mem->Next)
	{

		if(Ty->Align < mem->Ty->Align)
		{
			Ty->Align = mem->Ty->Align;
		}
		if(Ty->Size < mem->Ty->Size)
			Ty->Size = mem->Ty->Size; // shred space, so set Size as max elem Size

	}
    // algin Size, Offset is always for all variable
	Ty->Size = alignTo(Ty->Size, Ty->Align);

	return Ty;
}
```

`f7e55d253fb8578a3e0ad652f73c48b31e5d00e2`

### Step 47: Support struct assignment

```c++
// store %rax to an address that the stack top is pointing to.
static void store(TypePtr  Ty) {
  pop("%rdi");
  if(Ty->Kind == TY_STRUCT || Ty->Kind == TY_UNION)
  {
  	for(int i = 0; i < Ty->Size; i++)
  	{
  		println("  mov %d(%%rax), %%r8b", i);
  		println("  mov %%r8b, %d(%%rdi)", i);
  	}
  	return;
  }

  if(Ty->Size == 1)
  	println("  mov %%al, (%%rdi)");
  else
  	println("  mov %%rax, (%%rdi)");
}
```

`cf39095e150df76e1c81679f997bcfee4922faf8`

### Step 48: Change Size of int from 8 to 4

samplely change

```c++
TypePtr TyInt = &(Type){TY_INT, 8, 8};
TypePtr TyInt = &(Type){TY_INT, 4, 4};
```

and related code

`c914abc8901a23e172d9551f55b7acc29447f6b2`

### Step 49: Support long type

add long to keyword

```c++
// declspec = "char" | "short" | "int" | "long" | struct-decl | union-decl
```

`7813a650342b59f33b24df93035bd74ead7f293c`

### Step 50: Support short type

same as long

`456a93585f176dfd75eea55086a3e68776d9ad5c`

### Navigate source code files to Cpp

`f0d44e6c271892ee7efa26a128a4091d752ab077`

### Step 51:  Supoport nested type declarators

```c++
// declarator = "*"* ident type-suffix
//
//
// declarator = "*"* ("(" ident ")" | "(" declarator ")" | ident) type-suffix
static TypePtr  declarator(TokenPtr *rest, TokenPtr Tok, TypePtr  Ty)
{
	while(consume(&Tok, Tok, "*"))
	{
		Ty = pointerTo(Ty);
	}

	if(equal(Tok, "("))
	{
		TokenPtr  start = Tok;
		Type dummy = {};
		declarator(&Tok, start->Next, & dummy);
		Tok = skip(Tok, ")");
		Ty = type_suffix(rest, Tok, Ty);
		return declarator(&Tok, start->Next, Ty);
	}


	if(Tok->Kind != TK_IDENT)
		error_tok(Tok, "expected a varibale Name");

	Ty = type_suffix(rest, Tok->Next, Ty);
	Ty->Name = Tok;
	return Ty;
}
```

`03b8718fdd6290ede0e5518fca3c77ac9259c369`

### Step52: Support function declaration

if detected a function declaration, not definition , just skip it.

```c++
// in Obg
bool IsDefinition;
// parse.c
// function_declaration = declspec declarator "{" compoundStmt "}"
static TokenPtr  function(TokenPtr Tok, TypePtr basety)
{// ...
    fn->IsDefinition = !consume(&Tok, Tok, ";"); // if consume(&Tok, Tok, ";") == true, is a declaration   
}

// codegen.c
    if (!fn->IsFunction || !fn->IsDefinition) // ===> declaration
      continue;
```

`07246dd0d629034407aa36b364a63fcc20dffcf9`

### Step53: Support function void type

new keyowrd , type

but only function may return as void type, not variable, we need check in `declaration` 

besides also couldn't be dereference.

```c++
// declspec = "char" | "short" | "int" | "long" |struct-decl |union-decl
//
// declspec = "void" | "char" | "short" | "int" | "long"
//          | struct-decl | union-decl
```

`34493bd616627d0746176bbf34e80d4bd5b13698`

### Step54: Support complex type declarations correctly

 can now read complex type declarations such as below.

  long x;
  long int x;
  int long x;
  short x;
  short int x;
  int short x;
  long long x;
  long long int x;

```c++
// declspec = "void" | "char" | "short" | "int" | "long"
//          | struct-decl | union-decl
//
// declspec = ("void" | "char" | "short" | "int" | "long"
//             | struct-decl | union-decl)+
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
```

the idea is change to a big switch

`17329c7115e2f8f3e8724af898f0891312376ca1`

### Step55: using long long as an alias for long

because in Step54 have implemention a bitint, just add to switch

`39837604bef4d47421274254c2a83bda60fcc758`

### Step56: Support typedef

typedef is a special keyword, it create a type, typically

In the following example, `x` is defined as an alias for `int`.

  typedef x;

Below is valid C code where the second `t` is a local variable
of type int having value 3.

  typedef int t;
  t t = 3;

make typedef also is a typename, invoke complex type, beside every time judg whether is a type, try to find typedef.

before variable just a variable, but now is maybe a typedef

**detial:**

when meeting a "typedef" 

1. first set attr 'is_typedef' to true , tag it is a typedef
2. since `declspec`actually return two value, first is TypePtr  , second is attr return by pointer. the caller of `declspec` change from `declaration` to `compoundStmt` , whjch create a `VarAttr attr` variable and pass to `declspec`. and `declspec` fill the is_typedef field. 
3. if the is_typedef is true, then call `parseTypedef` to create a typedef variable, save to variable Scope, otherwise same as ordinary variable declaration.
4. Next time, function `declspec` also need to check whether a type is a typedef type.
5. since, function `declaration` no longer call  `declspec`, so it need a `basety` ,passing by function argument
6. last thing need be mention, since now variable maybe a typedef, so every time call `find_var` no longer return the Obj *Var, but VarScope *, need extract variable from VarScope if need.

```c++
static Node * declaration(TokenPtr *rest, TokenPtr Tok)
{
	TypePtr  basety = declspec(&Tok, Tok);
}
// now
static Node * declaration(TokenPtr *rest, TokenPtr Tok, TypePtr basety)
{
    
}
// compound-stmt = (declaration |stmt )* "}"
// compound-stmt = (typedef | declaration |stmt )* "}"

// declaration = declspec (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;
// declaration = (declarator ("=" expr)? ("," declarator  ("=" expr)?)* )? ;

```

code:

```c++
// compound-stmt = (typedef | declaration |stmt )* "}"
static Node * compoundStmt(TokenPtr * rest, TokenPtr  Tok)
{
	Node * node = createNewNode(ND_BLOCK, Tok);

	Node head = {};
	Node * cur = &head;

	enterScope();


	while(!equal(Tok, "}")){
		if(isTypename(Tok))
		{
			VarAttr attr = {};
			TypePtr  basety = declspec(&Tok, Tok, &attr);

			if(attr.is_typedef)
			{
				Tok = parseTypedef(Tok, basety);
				continue;
			}

			cur = cur->Next = declaration(&Tok, Tok, basety);
		}
		else
		{
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
```



```c++
static TokenPtr parseTypedef(TokenPtr Tok, TypePtr  basety)
{
	bool first = true;
	while(!consume(&Tok, Tok, ";"))
	{
		if(!first)
			Tok = skip(Tok, ",");
		first =false;

		TypePtr  Ty = declarator(&Tok, Tok, basety);
		push_scope(get_ident(Ty->Name))->type_def = Ty;
	}
	return Tok;
}

```

`ccfc6a29d386a285015f50f30952ca76aa82885a`

### Step57: Support sizeof() a typename

the Base idea is expand the `primary` to support a special expression

replace the expression in sizeof() to a num node.

```c++
// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
// type-Name = declspec abstract-declarator

// primary = "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" "(" type-Name ")"
//         | "sizeof" unary
//         | ident func-args?
//         | str
//         | num
```

`588154350f1d39fac80de859b7d0b954f7632c43`

### Use 32 bit registers for char, short and int

change codegen()

`3500d5c84ee41725df211c0fc8ac68402c6e3a51`

### Add Symbol Table and function argument, but all return type is void
