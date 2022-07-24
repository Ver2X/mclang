# chibicc

## TODO



This is the reference implementation of https://www.sigbus.info/compilerbook.
TODO LIST:



support **Doxygen**

move to C with STL , then move to C++



add free(), using tools to check


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





