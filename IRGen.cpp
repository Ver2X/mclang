#include "chibicc.h"

static void gen_stmt_ir(Node * node);
static void gen_expr_ir(Node *node, Variable *);
static void gen_func_arg_ir(Obj * var, int r, int offset, int sz);
extern std::fstream file_out;
extern int depth;

extern int count_diff_if;
// use to identify diff "if" statements
static int count()
{
	
	return count_diff_if++;
}

// for now, define as a fucntion, then change to rope data structure.
static std::string Twine(std::string &l, std::string & r)
{
	return r + r;
}


static std::string Twine(std::string l, std::string r)
{
	return r + r;
}


static std::string next_variable_name()
{
	int next_variable_name_v = 0;
	std::string name = Twine("%" , std::to_string(next_variable_name_v++));
	return name;
}

IRBuilder InMemoryIR;
SymbolTable symTable;

// emit IR
void emit_ir(Obj * prog)
{
	

	for(Obj * fn = prog; fn; fn = fn->next)
	{
		if(!fn->is_function || !fn->is_definition)
			continue;
		IRFunction * func = new IRFunction();
		func->functionName = fn->name;

		// prologue
		//println("  push %%rbp");
		//println("  mov %%rsp, %%rbp");
		//println("  sub $%d, %%rsp", fn->stack_size);
 
 		switch(fn->ty->return_ty->kind)
 		{
 			case TY_INT:
 				func->retTy = RTY_INT;
 				break;
 			case TY_CHAR:
 				func->retTy = RTY_CHAR;
 				break;
 			default:
 				func->retTy = RTY_PTR;
 				break;
 		}
		// save passed-by-register arguments to the stack
		//int i = 0;
		Variable * head;
		for(Obj * var = fn->params; var; var = var->next)
		{
			//file_out << "func args increase 1" << std::endl;
			
			
			Variable * arg_variable = new Variable();
			switch(var->ty->size)
			{
				
				case 1:
					// var->name
					arg_variable->type = VAR_8;
					arg_variable->name = std::string(var->name);
					symTable.insert(arg_variable, 0);
					break;
				case 2:
					arg_variable->type = VAR_16;
					arg_variable->name = std::string(var->name);
					symTable.insert(arg_variable, 0);
					break;
				case 4:
					arg_variable->type = VAR_32;
					arg_variable->name = std::string(var->name);
					symTable.insert(arg_variable, 0);
					break;
				case 8:
					arg_variable->type = VAR_64;
					arg_variable->name = std::string(var->name);
					symTable.insert(arg_variable, 0);
					break;
				default:
					break;
			} // end switch
			if(func->argsNum == 0)
			{
				func->args = arg_variable;
				head = func->args;
			}
			else
			{
				head->next = arg_variable;
				head = head->next;
			}
			(func->argsNum)++;
		}
		InMemoryIR.function = func;
		// emit code
		gen_stmt_ir(fn->body);
		assert(depth == 0);

		// Epilogue
		//println(".L.return.%s:", fn->name);
		//println("  mov %%rbp, %%rsp");
		//println("  pop %%rbp");
		//println("  ret");
		file_out << InMemoryIR.CodeGen() << std::endl;
	}
	
}



static void gen_stmt_ir(Node * node)
{
	//println("  .loc 1 %d", node->tok->line_no);
	switch(node->kind )
	{	
		case ND_IF:
		{
			int c = count();
			gen_expr_ir(node->cond, NULL);
			//println("  cmp $0, %%rax");
			//println("  je .L.else.%d", c);
			gen_stmt_ir(node->then);
			//println("  jmp .L.end.%d", c);
			//println(".L.else.%d:", c);
			if(node->els)
				gen_stmt_ir(node->els);
			//println(".L.end.%d:", c);
			return; 
		}
		case ND_FOR: // or while
		{
			int c = count();

			// for handle while
			if(node->init)
				gen_stmt_ir(node->init);
			//println(".L.begin.%d:", c);
			if(node->cond)
			{
				gen_expr_ir(node->cond, NULL);
				//println("  cmp $0, %%rax");
				//println("  je .L.end.%d", c);
			}
			gen_stmt_ir(node->then);

			if(node->inc)
				gen_expr_ir(node->inc, NULL);

			//println("  jmp .L.begin.%d", c);
			//println(".L.end.%d:", c);
			return; 
		}
		case ND_BLOCK:
			for(Node * n = node->body; n; n = n->next)
				gen_stmt_ir(n);
			return;
		case ND_RETURN:
			gen_expr_ir(node->lhs, NULL);
			//println("  jmp .L.return.%s", current_fn->name);
			return;
		case ND_EXPR_STMT:
			gen_expr_ir(node->lhs, NULL);
			return;
		default:
			return;
	}

	error("invalid statement");
}



static void gen_variable_ir(Node *node)
{
	switch(node->kind)
	{
		case ND_VAR:
			if(node->var->is_local)
			{
				// Local variable
				Variable * local_variable = new Variable();
				local_variable->name = node->var->name;
				local_variable->type = VAR_32;
				symTable.insert(local_variable, 0);
				//println("  lea %d(%%rbp), %%rax", node->var->offset);
			}
			else
			{
				// Global variable
				//println("  lea %s(%%rip), %%rax", node->var->name);
			}
			return;
		case ND_DEREF:
			gen_expr_ir(node->lhs, NULL);
			return;
		case ND_COMMA:
			gen_expr_ir(node->lhs, NULL);
			//gen_addr(node->rhs);
			return;
		case ND_MEMBER:
			//gen_addr(node->lhs);
			//println("  add $%d, %%rax", node->member->offset);
			return;
		default:
			return;
	}

	error("not an lvalue");
}


// stack machine
static void gen_expr_ir(Node *node, Variable * res)
{

	//println("  .loc 1 %d", node->tok->line_no);
	switch(node->kind)
	{
		case ND_NUM:
			//println("  mov $%ld, %%rax", node->val);
			return ;
		case ND_NEG:
			gen_expr_ir(node->lhs, NULL);
			//println("  neg %%rax");
			return ;
		case ND_VAR:
		case ND_MEMBER:
			//gen_addr(node);
			//load(node->ty);
			return ;
		case ND_DEREF:
			gen_expr_ir(node->lhs, NULL);
			//load(node->ty);
			return;
		case ND_ADDR:
			//gen_addr(node->lhs);
			return;
		case ND_ASSIGN:
			gen_variable_ir(node->lhs);
			//push();
			gen_expr_ir(node->rhs, NULL);
			//store(node->ty);
			return;
		case ND_STMT_EXPR:
			for(Node * n = node->body; n; n = n->next)
			{
				gen_stmt_ir(n);
			}
			return;
		case ND_COMMA:
			gen_expr_ir(node->lhs, NULL);
			gen_expr_ir(node->rhs, NULL);
			return;
		case ND_FUNCALL:
			{
				int nargs = 0;
				for(Node *arg = node->args; arg; arg = arg->next)
				{
					gen_expr_ir(arg, NULL);
					//push();
					nargs++;
				}

				//for(int i = nargs - 1; i >= 0; i--)
				//	pop(argreg64[i]);

				//println("  mov $0, %%rax");
				//println("  call %s", node->funcname);
				return;
			}
		default:
			return;
	}

	// there must deal rhs first
	gen_expr_ir(node->rhs, NULL);
	//push();
	gen_expr_ir(node->lhs, NULL);
	//pop("%rdi");

	const char * ax, *di;
	if(node->lhs->ty->kind == TY_LONG || node->lhs->ty->base)
	{
		//ax = "%rax";
		//di = "%rdi";
	}
	else
	{
		//ax = "%eax";
		//di = "%edi";
	}

	switch(node->kind)
	{
		case ND_ADD:
			{
				res = new Variable();
				res->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->lhs->kind == ND_NUM))
				{
					
					res->Ival = node->lhs->val + node->rhs->val;
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						std::string s = node->rhs->var->name;
						Variable * r = symTable.find_var(s);
						res->Ival = node->lhs->val + r->Ival;
					}else if(node->rhs->kind == ND_NUM)
					{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						res->Ival = l->Ival + node->rhs->val;
					}else{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						std::string s2 = node->rhs->var->name;
						Variable * r = symTable.find_var(s2);
						res->Ival = l->Ival + r->Ival;
					}
				}
				symTable.insert(res, 0);
				return;
			}
			//println("  add %s, %s", di, ax);
			
		case ND_SUB:
			{
				res = new Variable();
				res->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->lhs->kind == ND_NUM))
				{
					res->Ival = node->lhs->val - node->rhs->val;
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						std::string s = node->rhs->var->name;
						Variable * r = symTable.find_var(s);
						res->Ival = node->lhs->val - r->Ival;
					}else if(node->rhs->kind == ND_NUM)
					{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						res->Ival = l->Ival - node->rhs->val;
					}else{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						std::string s2 = node->rhs->var->name;
						Variable * r = symTable.find_var(s2);
						res->Ival = l->Ival - r->Ival;
					}
				}
				symTable.insert(res, 0);
				return;
			}
		case ND_MUL:
			{
				res = new Variable();
				res->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->lhs->kind == ND_NUM))
				{
					res->Ival = node->lhs->val * node->rhs->val;
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						std::string s = node->rhs->var->name;
						Variable * r = symTable.find_var(s);
						res->Ival = node->lhs->val * r->Ival;
					}else if(node->rhs->kind == ND_NUM)
					{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						res->Ival = l->Ival * node->rhs->val;
					}else{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						std::string s2 = node->rhs->var->name;
						Variable * r = symTable.find_var(s2);
						res->Ival = l->Ival * r->Ival;
					}
				}
				symTable.insert(res, 0);
				return;
			}
		case ND_DIV:
			{
				res = new Variable();
				res->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->lhs->kind == ND_NUM))
				{
					res->Ival = node->lhs->val / node->rhs->val;
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						std::string s = node->rhs->var->name;
						Variable * r = symTable.find_var(s);
						res->Ival = node->lhs->val / r->Ival;
					}else if(node->rhs->kind == ND_NUM)
					{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						res->Ival = l->Ival / node->rhs->val;
					}else{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						std::string s2 = node->rhs->var->name;
						Variable * r = symTable.find_var(s2);
						res->Ival = l->Ival / r->Ival;
					}
				}
				symTable.insert(res, 0);
				return;
			}
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
			{
				res = new Variable();
				res->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->lhs->kind == ND_NUM))
				{
					if(node->kind == ND_EQ)
						res->Ival = node->lhs->val == node->rhs->val;
					else if(node->kind == ND_NE)
						res->Ival = node->lhs->val != node->rhs->val;
					else if(node->kind == ND_LT)
						res->Ival = node->lhs->val < node->rhs->val;
					else if(node->kind == ND_LE)
						res->Ival = node->lhs->val <= node->rhs->val;
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						std::string s = node->rhs->var->name;
						Variable * r = symTable.find_var(s);
						if(node->kind == ND_EQ)
							res->Ival = r->Ival == node->lhs->val;
						else if(node->kind == ND_NE)
							res->Ival = r->Ival != node->lhs->val;
						else if(node->kind == ND_LT)
							res->Ival = r->Ival < node->lhs->val;
						else if(node->kind == ND_LE)
							res->Ival = r->Ival <= node->lhs->val;
						
					}else if(node->rhs->kind == ND_NUM)
					{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						if(node->kind == ND_EQ)
							res->Ival = l->Ival == node->rhs->val;
						else if(node->kind == ND_NE)
							res->Ival = l->Ival != node->rhs->val;
						else if(node->kind == ND_LT)
							res->Ival = l->Ival < node->rhs->val;
						else if(node->kind == ND_LE)
							res->Ival = l->Ival <= node->rhs->val;
						
					}else{
						std::string s = node->lhs->var->name;
						Variable * l = symTable.find_var(s);
						std::string s2 = node->rhs->var->name;
						Variable * r = symTable.find_var(s2);
						if(node->kind == ND_EQ)
							res->Ival = l->Ival == r->Ival;
						else if(node->kind == ND_NE)
							res->Ival = l->Ival != r->Ival;
						else if(node->kind == ND_LT)
							res->Ival = l->Ival < r->Ival;
						else if(node->kind == ND_LE)
							res->Ival = l->Ival <= r->Ival;
					}
				}
				symTable.insert(res, 0);
				return;
			}
			return;
		default:
			return;
	}

	error("invalid expression");
}
