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

