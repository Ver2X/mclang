#include "chibicc.h"

Type *ty_int = &(Type){TY_INT};

/*! judge type	
*/
bool is_integer(Type * ty)
{
	return ty->kind == TY_INT;
}

/*! create a pointer of type, with base type
 * */
Type * pointer_to(Type * base)
{
	Type * ty = calloc(1, sizeof(Type));
	ty->kind = TY_PTR;
	ty->base = base;
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

	switch(node->kind)
	{
		// arithmetic operations
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_NEG:
		case ND_ASSIGN:
			// may need expand to support implict convert
			node->ty = node->lhs->ty;
			return ;

		// logic operations and int set as ty_int;
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
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
	}
}