# chibicc

This is the reference implementation of https://www.sigbus.info/compilerbook.

link:  https://github.com/rui314/chibicc

## TODO LIST:



support **Doxygen**

move to C with STL , then move to C++



add free(), using tools to check or use shared pointer...


maybe create a function IRgen

push all of instruction in to IRgen Builder

and a symbol table

then travel again to generate IR code



add printAST function





change locals to a nest symbol, to support multi function

may use the dic tree to boost string compare 



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







































































