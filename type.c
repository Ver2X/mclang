#include "chibicc.h"

Type *ty_int = &(Type){TY_INT, 4, 4};
Type *ty_char = &(Type){TY_INT, 1, 1};

static Type * new_type(TypeKind kind, int size, int align)
{
	Type * ty = calloc(1, sizeof(Type));
	ty->kind = kind;
	ty->size = size;
	ty->align = align;
	return ty;
}


/*! judge type	
*/
bool is_integer(Type * ty)
{
	return ty->kind == TY_INT || ty->kind == TY_INT;
}

/*! create a pointer of type, with base type
 * */
Type * pointer_to(Type * base)
{
	Type * ty = new_type(TY_PTR, 8, 8);
	ty->base = base;
	return ty;
}

Type * func_type(Type * return_ty)
{
	Type * ty = calloc(1, sizeof(Type));
	ty->kind = TY_FUNC;
	ty->return_ty = return_ty;
	return ty;
}

Type * copy_type(Type * ty)
{
	Type * ret = calloc(1, sizeof(Type));
	*ret = *ty;
	return ret;
}

Type * array_of(Type * base, int len)
{
	Type * ty = new_type(TY_ARRAY, base->size * len, base->align);
	ty->base = base;
	ty->array_len = len;
	return ty;
}

/*! recursive setup the type of given AST node
 * */
void add_type(Node *node)
{
	if(!node || node->ty)
		return ;

	add_type(node->lhs);
	add_type(node->rhs);
	add_type(node->cond);
	add_type(node->then);
	add_type(node->els);
	add_type(node->init);
	add_type(node->inc);

	for(Node * n = node->body; n; n = n->next)
	{
		add_type(n);
	}
	for(Node * n = node->args; n; n = n->next)
	{
		add_type(n);
	}

	switch(node->kind)
	{
		// arithmetic operations
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_NEG:
			// may need expand to support implict convert
			node->ty = node->lhs->ty;
			return ;
		case ND_ASSIGN:
			if(node->lhs->ty->kind == TY_ARRAY)
				error_tok(node->lhs->tok, "not an lvalue");
			node->ty = node->lhs->ty;
			return ;

		// logic operations and int set as ty_int;
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
		case ND_NUM:
		case ND_FUNCALL:
			node->ty = ty_int;
			return;
		case ND_VAR:
			// int, int *, int ** ...
			node->ty = node->var->ty;
			return;
		case ND_COMMA:
			node->ty = node->rhs->ty;
			return;
		case ND_MEMBER:
			node->ty = node->member->ty;
			return;
		// for '&', create a new type as TY_PTR, setup base type
		case ND_ADDR:
			if(node->lhs->ty->kind == TY_ARRAY)
				node->ty = pointer_to(node->lhs->ty->base);
			else
				node->ty = pointer_to(node->lhs->ty);
			return;
		case ND_DEREF:
			if(!node->lhs->ty->base)
			{
				error_tok(node->tok, "expect dereference a pointer or array pointer, but not");
				
			}
			// for '*', down a type level
			node->ty = node->lhs->ty->base;
			return;
			case ND_STMT_EXPR:
			if(node->body)
			{
				Node *stmt = node->body;
				while(stmt->next)
					stmt = stmt->next;
				if(stmt->kind == ND_EXPR_STMT)
				{
					node->ty = stmt->lhs->ty;
					return;
				}
			}
			error_tok(node->tok, "statement expression returning void is not supported");
			return;
	}
}