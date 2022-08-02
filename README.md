# chibicc

This is the reference implementation of https://www.sigbus.info/compilerbook.

link:  https://github.com/rui314/chibicc

## TODO LIST:

before write code self, create a branch to save old code

support **Doxygen**

move to C with STL , then move to C++



add free(), using tools to check or use shared pointer...


maybe create a function IRgen

push all of instruction in to IRgen Builder

and a symbol table

then travel again to generate IR code



add printAST function

like	

​		|

​		|

​		|

​				|

​				|

change locals to a nest symbol, to support multi function

may use the dic tree to boost string compare 

support CHANGE LOG

may using tree IR?  like gcc and open64



===> tree base instruction selection ==> DAG base

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
                   | <typedef-name>

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
                    | ( <type-name> ) <cast-expression>

<unary-expression> ::= <postfix-expression>
                     | ++ <unary-expression>
                     | -- <unary-expression>
                     | <unary-operator> <cast-expression>
                     | sizeof <unary-expression>
                     | sizeof <type-name>

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

<type-name> ::= {<specifier-qualifier>}+ {<abstract-declarator>}?

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

<typedef-name> ::= <identifier>

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
	Obj * next;
	char * name;
	int offset; 	// from rbp
	Type *ty;		// Type of local variable ,  new add
};

struct Type
{
	TypeKind kind;
	Type * base;

	// declaration
	Token * name;  // new add
};

```





for parser

declaration

type:

declspec = "int"

left of declaration

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
static Node * declaration(Token **rest, Token *tok)
{
	Type * basety = declspec(&tok, tok);

	Node head = {};
	Node * cur = &head;


	int i = 0; // label to check whether is the first declaration varibale
	while(!equal(tok, ";"))
	{
		if(i++ > 0)
			tok = skip(tok, ",");

		// define but not used, add it to variable list
		Type * ty = declarator(&tok, tok, basety);
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

```

and

```c++
	
// => primary =  "(" expr ")" | ident | num 
static Node *primary(Token ** rest, Token * tok)
    ...
if(tok->kind == TK_IDENT)
	{
		Obj * var = find_var(tok);
		if(!var){
			error_tok(tok, "undefined varibale");
		}
		*rest = tok->next;
		return new_var_node(var, tok);
	}
```



then change the type.c



```c++
void add_type(Node *node)
    ...

case ND_NUM:
			node->ty = ty_int;
			return;
		case ND_VAR:
			// int, int *, int ** ...
			node->ty = node->var->ty;
			return;

		// for '&', create a new type as TY_PTR, setup base type
		case ND_ADDR:
			node->ty = pointer_to(node->lhs->ty);
			return;
		case ND_DEREF:
			if(node->lhs->ty->kind != TY_PTR)
			{
				error_tok(node->tok, "expect dereference a pointer, but not");
				
			}
			// for '*', down a type level
			node->ty = node->lhs->ty->base;
			return;
```

`commit d5895ddaf3ebe4f90176295fb52f788d8446643b`



### Step 18 : Support  null argument function call, don't support function define

To support function call, first we need change parser, notice temp argument list is null

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
	Node * body;
	Obj * locals;
	int stack_size;

	Function * next;
	char * name;
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
	TypeKind kind;
	Type * base;

	// declaration
	Token * name;

	// Function type
	Type * return_ty;
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
// for now, func-params = NULL
```

besides, now we should parser function first, not statements

but for function definition. function also need a independent variable list

```c++
// function_declaration = declspec declarator "{" compound_stmt "}"
static Function * function(Token * tok)
{
	Type * ty = declspec(&tok, tok);
	ty = declarator(&tok, tok, ty);

	locals = NULL;

	Function *fn = calloc(1, sizeof(Function));
	fn->name = get_ident(ty->name);

	tok = skip(tok, "{");

	fn->body = compound_stmt(rest, tok);
	fn->locals = locals;
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
    Obj *params;
};

struct Type
{
	TypeKind kind;
	Type * base;

	// declaration
	Token * name;

	// Function type
	Type * return_ty;
	Type *params;
  	Type *next;
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
static void create_param_lvars(Type * param)
{
	if(param)
	{
        // recurse to last params
		create_param_lvars(param->next);
		// head insert, in variable list seq is same with seq decl variable
        new_lvar(get_ident(param->name, param));
	}
}
```

in codegen

```c++
void codegen(Function * prog)
{
	// first setup offset
	assign_lvar_offsets(prog);

	for(Function * fn = prog; fn; fn = fn->next)
	{
		printf("  .globl %s\n", fn->name);
		printf("%s:\n", fn->name);
		current_fn = fn;


		// prologue
		printf("  push %%rbp\n");
		printf("  mov %%rsp, %%rbp\n");
		printf("  sub $%d, %%rsp\n", fn->stack_size);

		// save passed-by-register arguments to the stack
		int i = 0;
		for(Obj * var = fn->params; var; var = var->next)
		{
			printf("  mov %s, %d(%%rbp)\n", argreg[i++], var->offset);
		}

		// emit code
		gen_stmt(fn->body);
		assert(depth == 0);

		// Epilogue
		printf(".L.return.%s:\n", fn->name);
		printf("  mov %%rbp, %%rsp\n");
		printf("  pop %%rbp\n");
		printf("  ret\n");
	}
}
```

`bfc8877e86be8d5b669a49b34b0e2f2990cc9dd6`

### Step 22: Support one dimensional arrays

in C , array is a lanuage sugar, it is a pointer with number of element, but is could be modify "not a lvalue"

for AST and size and len

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
	TypeKind kind;

	int size;      // sizeof() value

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

	// Function type
	Type * return_ty;
	Type *params;
  	Type *next;
};
```



for supoort arrays

we need to change the parser make it support "[ num ]", besides make int => size = 8, maintain size set right

```c++
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

		Type * basety = declspec(&tok, tok);
		Type * ty = declarator(&tok, tok, basety);
		cur = cur->next = copy_type(ty);
	}

	ty = func_type(ty);
	ty->params = head.next;
	*rest = tok->next;
	return ty;
}
// type-suffix = "(" func-params
//             | "[" num "]"
//             | ε

static Type * type_suffix(Token ** rest, Token *tok, Type *ty)
{
	if(equal(tok, "("))
		return func_params(rest, tok->next, ty);

	if(equal(tok, "["))
	{
		int sz = get_number(tok->next);
		*rest = skip(tok->next->next, "]");
		return array_of(ty, sz);
	}

	*rest = tok;
	return ty;
}
```

`bfc8877e86be8d5b669a49b34b0e2f2990cc9dd6`

### Step 23: Support sizeof()

in lexer, make it reconize "sizeof" as a keyword

because before we have add  "size" int Type, change parser just return the correspond number

```c++
// primary =  "(" expr ")" | ident func-args?| num 
//
// to
//
// primary = "(" expr ")" | "sizeof" unary | ident func-args? | num
static Node *primary(Token ** rest, Token * tok)
{
    // ...
    if(equal(tok, "sizeof"))
	{
		Node * node = unary(rest, tok->next);
		add_type(node);
		return new_num(node->ty->size, tok);
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
```

`3c7242041444a16919c2d85a0b939bdd94113cce`

### Step 25: Support [] operator

it is a post operator, just change parser

```c++
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

// postfix = primary ("[" expr "]")*
static Node *postfix(Token **rest, Token * tok)
{
	Node * node = primary(&tok, tok);

	while(equal(tok, "["))
	{
		// x[y] ==> *(x+y)
		Token * start = tok;
		Node * idx = expr(&tok, tok->next);
		tok = skip(tok, "]");
		node = new_unary(ND_DEREF, new_add(node, idx, start), start);
	}
	*rest = tok;
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
	Obj * next;
	char * name;
	int offset; 	// from rbp
	Type *ty;		// Type of local variable
-};

-// function
-typedef struct Function Function;
-struct Function
-{
	Node * body;
	Obj * locals;
	int stack_size;

+	bool is_function;
-	Function * next;
-	char * name;
	Obj *params;
};
```

`5962a3645688d81345bc938404a46549c32011ca`

### Step 27:Support global variables

for parser deal global varibale

```c++
// program = (function-definition | global-variable)*
Obj * parse(Token * tok)
{
	globals = NULL;


	while(tok->kind != TK_EOF){
		Type *basety = declspec(&tok, tok);

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
```

for codegen()

```c++
// emit global data
static void emit_data(Obj * prog)
{
	for(Obj * var = prog; var; var = var->next)
	{
		if(var->is_function)
			continue;
		printf("  .data\n");
		printf("  .globl %s\n", var->name);
		printf("%s:\n",var->name);
		printf("  .zero %d\n", var->ty->size);
	}
}
```

`7cb83c641af402c2f449178b9857e44605ba4e49`

### Step 28: Support char

for now, use think char is integer

```c++
// declspec = "char" | "int"
static Type * declspec(Token ** rest, Token *tok)
{
	if(equal(tok, "char"))
	{
		*rest = tok->next;
		return ty_char;
	}
	*rest = skip(tok, "int");
	return ty_int;
}
```

notice we need use 8bit reg, not 64bit, such as

```c++
static void emit_text(Obj * prog)
{
	for(Obj * fn = prog; fn; fn = fn->next)
	{
		if(!fn->is_function)
			continue;

		printf("  .globl %s\n", fn->name);
		printf("  .text\n");
		printf("%s:\n", fn->name);
		current_fn = fn;


		// prologue
		printf("  push %%rbp\n");
		printf("  mov %%rsp, %%rbp\n");
		printf("  sub $%d, %%rsp\n", fn->stack_size);

		// save passed-by-register arguments to the stack
		int i = 0;
		for(Obj * var = fn->params; var; var = var->next)
		{
			if(var->ty->size == 1)
				printf("  mov %s, %d(%%rbp)\n", argreg8[i++], var->offset);
			else
				printf("  mov %s, %d(%%rbp)\n", argreg64[i++], var->offset);
		}

		// emit code
		gen_stmt(fn->body);
		assert(depth == 0);

		// Epilogue
		printf(".L.return.%s:\n", fn->name);
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
	Type * ty;				// used if TK_STR
	char * str;				// string literal contents including terminating '\0'
};

// for local variable or function
typedef struct Obj Obj;
struct Obj
{
	// ...
    
	// Global variable
	char * init_data;

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

static Obj * new_anon_gvar(Type * ty)
{
	return new_gvar(new_unique_name(), ty);
}

static Obj * new_string_literal(char * p, Type * ty)
{
	Obj * var = new_anon_gvar(ty);
	var->init_data = p;
	return val;
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

change parser and codegen and type add_type() function

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
static Node *primary(Token ** rest, Token * tok)
{

	if(equal(tok, "(") && equal(tok->next, "{"))
	{
		// this is a GNU statement expresssion
		Node * node = new_node(ND_STMT_EXPR, tok);
		node->body = compound_stmt(&tok, tok->next->next)->body;
		*rest = skip(tok, ")");
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

### Step 37: Support line and block comments

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

	// skip block comments
	if(startswith(p, "/*"))
	{
		char * q = strstr(p + 2, "*/");
		if(!q)
			error_at(p, "unclosed block comment");
		p = q + 2;
	}
```

`da07308f95da1914d925eef8159198e84898d3aa`

### Step 38: Support block scope

remind how we deal variable before

a. every time call new_lvar() will add a varibale to locals varibale list 

b.

1. when call parse first add global variable by global_varibale()
2. when parse function(), wuk first add function name as global varibale and add function params (by create_param_lvars()) to local varibale list then call **compound_stmt()** which call declaration() , then declspec() and declarator() judge variable type and set right variable name, then return to declaration() call new_lvar() to create local variable

parse --- > global_varibale

parse --- > function ---> compound_stmt ---> declaration ---> new_lvar

then if variable have been define, change it

```c++
// stmt = "return" expr ";"
//		| "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//		| expr_stmt

// function_declaration = declspec declarator "{" compound_stmt "}"
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
// function_declaration = declspec declarator "{" compound_stmt "}"
```

the code of from scope,

struct Scope, present a block scope, init it present global variable scope,

and every time meet '{' enter a level scope, and meet '}' exit a level of scope, VarScope present variable list 

```c++
// scope for local or global variable
typedef struct VarScope VarScope;
struct VarScope{
  VarScope * next;
  char * name;
  Obj *var;
};

// represent a block scope
typedef struct Scope Scope;
struct Scope{
  Scope * next;
  VarScope * vars;
};

static Obj * find_var(Token * tok)
{
	for(Scope * sc = scope; sc; sc = sc->next)
	{
		for(VarScope * sc2 = sc->vars; sc2; sc2 = sc2->next)
			if(equal(tok, sc2->name))
				return sc2->var;
	}
	return NULL;
}

static void enter_scope(void)
{
	Scope *sc = calloc(1, sizeof(Scope));
	sc->next = scope;
	scope = sc;
}

static void leave_scope(void)
{
	scope = scope->next;
}

static VarScope * push_scope(char * name, Obj *var)
{
	VarScope * sc = calloc(1, sizeof(VarScope));
	sc->name = name;
	sc->var = var;
	sc->next = scope->vars;	// VarScope * vars, head insert here
	scope->vars = sc;
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
		find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

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

This patch allows writing a comma expression on the left-hand side of an assignment expression. This is called the "generalized lvalue" which is a deprecated GCC language extension, implementing it anyway because it's useful to implement other features.

```c++
// expr       = assign
// assign     = equality ("=" assign)?
//
// to
//
// expr = assign ("," expr)?
```

make below right

```c+
  ASSERT(3, (1,2,3));
  ASSERT(5, ({ int i=2, j=3; (i=5,j)=6; i; }));
  ASSERT(6, ({ int i=2, j=3; (i=5,j)=6; j; }));
```

which return a mutable variable

```c++
// expr       = assign ("," expr)
static Node * expr(Token ** rest, Token * tok)
{
	Node * node = assign(&tok, tok);
	if(equal(tok, ","))
		return new_binary(ND_COMMA, node, expr(rest, tok->next), tok);

	*rest = tok;
	return node;
}
```

`343df57590ee2b13bae5586e361cbf0c6245bb45`

### Step 41: Support struct 

for now, when meet struct's "{" we don't need enter scope, actually struct have its own scope

and struct just is a anonymous variable type, no tag

```c++
// declspec = "char" | "int" | struct-decl
// struct-decl = "{" struct-members
// struct-members = (declspec declarator (","  declarator)* ";")*


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
  Member *members;
  // ...
};

// Struct member
struct Member {
  Member *next;
  Type *ty;
  Token *name;
  int offset;
};

```

`e562ff5a90f5eee001622409ed8319c2f31ed158`

### Step 42: Align struct members

align of a struct is the max ocopied space align

add a `int align` to Type, and maintain it, besides align offset to `align`

which mean

```c
  ASSERT(16, ({ struct {char a; int b;} x; sizeof(x); }));
  ASSERT(16, ({ struct {int a; char b;} x; sizeof(x); }));
```

`74b5f74e3f4aa463670f2906f8f18d36ad45d3a0`

### Step 43: Align local variables

when set vriable offset in codegen, just align it.

```c++
offset = align_to(offset, var->ty->align);
```

`2733964ec177e24d31c7f6d2bb178bc7943a2466`

### Step 44: Support struct tags and scope

```c++
// struct-decl = "{" struct-members
//
// to
// struct-decl = ident? ("{" struct-members)?
static Type * struct_decl(Token **rest, Token *tok)
{
	// read a truct tag
	// define
	Token * tag = NULL;
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
	Type * ty = calloc(1, sizeof(Type));
	ty->kind = TY_STRUCT;
	struct_members(rest, tok->next, ty);	// skip "}"
	ty->align = 1;

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

	// register the struct type if a name was given.
	if(tag)
		push_tag_scope(tag, ty);
	return ty;
}
```

`55fef4513cb3c360b8a2f7a3d034559a161993ef`

### Step 45: Support -> operator

which is a postfix operator

```c++
		if(equal(tok, "->"))
		{
			// x->t is short for (*x).y
			node = new_unary(ND_DEREF, node, tok);
			node = struct_ref(node, tok->next);
			tok = tok->next->next; // skip "->" and ident
			continue;
		}
```

`601526fb1c19bff3f0eea5374a9532bd70562b54`

### Step 46: Support union

the way to handle union is like handle struct, for union, all offset is 0, but we need to compute the alignment and size.

all variable share the space

```c++
// struct-union-decl = ident? ("{" struct-members)?
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
			ty->size = mem->ty->size; // shred space, so set size as max elem size

	}
    // algin size, offset is always for all variable
	ty->size = align_to(ty->size, ty->align);

	return ty;
}
```

`f7e55d253fb8578a3e0ad652f73c48b31e5d00e2`

### Step 47: Support struct assignment

```c++
// store %rax to an address that the stack top is pointing to.
static void store(Type * ty) {
  pop("%rdi");
  if(ty->kind == TY_STRUCT || ty->kind == TY_UNION)
  {
  	for(int i = 0; i < ty->size; i++)
  	{
  		println("  mov %d(%%rax), %%r8b", i);
  		println("  mov %%r8b, %d(%%rdi)", i);
  	}
  	return;
  }

  if(ty->size == 1)
  	println("  mov %%al, (%%rdi)");
  else
  	println("  mov %%rax, (%%rdi)");
}
```

`cf39095e150df76e1c81679f997bcfee4922faf8`

### Step 48: Change size of int from 8 to 4

samplely change

```c++
Type *ty_int = &(Type){TY_INT, 8, 8};
Type *ty_int = &(Type){TY_INT, 4, 4};
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
```

`03b8718fdd6290ede0e5518fca3c77ac9259c369`

### Step52: Support function declaration

if detected a function declaration, not definition , just skip it.

```c++
// in Obg
bool is_definition;
// parse.c
// function_declaration = declspec declarator "{" compound_stmt "}"
static Token * function(Token *tok, Type *basety)
{// ...
    fn->is_definition = !consume(&tok, tok, ";"); // if consume(&tok, tok, ";") == true, is a declaration   
}

// codegen.c
    if (!fn->is_function || !fn->is_definition) // ===> declaration
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

