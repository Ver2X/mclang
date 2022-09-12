#include "mclang.h"

static void gen_stmt_ir(Node * node, SymbolTablePtr);
static void gen_expr_ir(Node *node, VariablePtr*, SymbolTablePtr);
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
std::string Twine(std::string &l, std::string & r)
{
	return l + r;
}


std::string Twine(std::string l, std::string r)
{
	return l + r;
}

static int next_variable_name_v = 0;
static int next_variable_name_number()
{

	return next_variable_name_v++;
}

std::string next_variable_name()
{
	std::string name = Twine("%" , std::to_string(next_variable_name_number()));
	return name;
	//return std::to_string(next_variable_name_number());
}

static std::string getPreName(std::string name)
{
	return Twine("%", name);
}

IRBuilder InMemoryIR;
SymbolTablePtr symTable = std::make_shared<SymbolTable>();

// emit IR
void emit_ir(Obj * prog)
{
	

	for(Obj * fn = prog; fn; fn = fn->next)
	{
		next_variable_name_v = 0;
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
 				func->retTy = ReturnTypeKind::RTY_INT;
 				break;
 			case TY_CHAR:
 				func->retTy = ReturnTypeKind::RTY_CHAR;
 				break;
 			default:
 				func->retTy = ReturnTypeKind::RTY_PTR;
 				break;
 		}
		// save passed-by-register arguments to the stack
		//int i = 0;
		//VariablePtr head = NULL;
		VariablePtr arg_variable;
		VariablePtr arg_variable_addr;
		std::vector<std::tuple<VariablePtr, VariablePtr>> arg_variable_pair;
		// SymbolTable * loca_table = new SymbolTable(&symTable);
		auto loca_table = std::make_shared<SymbolTable>(symTable);

		/// Cache and release variable
		for(Obj * var = fn->params; var; var = var->next)
		{
			//file_out << "func args increase 1" << std::endl;
			arg_variable = std::make_shared<Variable>();
			// next_variable_name_number();
			switch(var->ty->size)
			{
				
				case 1:
				{
					// var->name
					arg_variable->type = VaribleKind::VAR_8;
					arg_variable->SetName(getPreName(var->name));					

					arg_variable_addr = std::make_shared<Variable>();
					arg_variable_addr->SetName(Twine(getPreName(var->name), ".addr"));
					arg_variable_pair.push_back(std::tuple<VariablePtr, VariablePtr>(arg_variable, arg_variable_addr));
					loca_table->insert(arg_variable_addr, 0);
					break;
				}
				case 2:
				{
					arg_variable->type = VaribleKind::VAR_16;
					arg_variable->SetName(getPreName(var->name));

					arg_variable_addr = std::make_shared<Variable>();
					arg_variable_addr->SetName(Twine(getPreName(var->name), ".addr"));
					arg_variable_pair.push_back(std::tuple<VariablePtr, VariablePtr>(arg_variable, arg_variable_addr));
					loca_table->insert(arg_variable_addr, 0);					
					break;
				}
				case 4:
				{
					arg_variable->type = VaribleKind::VAR_32;
					arg_variable->SetName(getPreName(var->name));
					loca_table->insert(arg_variable, 0);

					arg_variable_addr = std::make_shared<Variable>();
					arg_variable_addr->SetName(Twine(getPreName(var->name), ".addr"));
					arg_variable_pair.push_back(std::tuple<VariablePtr, VariablePtr>(arg_variable, arg_variable_addr));
					loca_table->insert(arg_variable_addr, 0);
					break;
				}
				case 8:
				{
					arg_variable->type = VaribleKind::VAR_64;
					arg_variable->SetName(getPreName(var->name));					
					loca_table->insert(arg_variable, 0);

					arg_variable_addr = std::make_shared<Variable>();
					arg_variable_addr->SetName(Twine(getPreName(var->name), ".addr"));
					arg_variable_pair.push_back(std::tuple<VariablePtr, VariablePtr>(arg_variable, arg_variable_addr));
					loca_table->insert(arg_variable_addr, 0);
					break;
				}
				default:
					break;
			} // end switch
			func->args.push_back(arg_variable);
			/*if(func->argsNum == 0)
			{
				if(arg_variable != NULL){
					func->args = arg_variable;
					head = func->args;
				}
				
			}
			else
			{
				if(head != NULL){
					head->next = arg_variable;
					head = head->next;
				}
			}
			*/
			(func->argsNum)++;
		}

		InMemoryIR.SetFunc(func);
		// emit code

		InMemoryIR.SetInsertPoint(next_variable_name_number(), "entry");
		for(auto p : arg_variable_pair)
		{
			InMemoryIR.Insert(NULL, NULL, std::get<1>(p), IROpKind::Op_Alloca, symTable);
			InMemoryIR.Insert(std::get<0>(p), NULL, std::get<1>(p), IROpKind::Op_Store, symTable);
		}
		////////// maybe need generate Block first
		//file_out << "arrive three 7" << std::endl;
		gen_stmt_ir(fn->body, loca_table);
		assert(depth == 0);

		// Epilogue
		//println(".L.return.%s:", fn->name);
		//println("  mov %%rbp, %%rsp");
		//println("  pop %%rbp");
		//println("  ret");
		file_out << InMemoryIR.CodeGen() << std::endl;
	}
	
}



static void gen_stmt_ir(Node * node, SymbolTablePtr table)
{
	//println("  .loc 1 %d", node->tok->line_no);
	switch(node->kind)
	{	
		case ND_IF:
		{
			int c = count();
			VariablePtr res;
			gen_expr_ir(node->cond, &res, table);
			//println("  cmp $0, %%rax");
			//println("  je .L.else.%d", c);
			gen_stmt_ir(node->then, table);
			//println("  jmp .L.end.%d", c);
			//println(".L.else.%d:", c);
			if(node->els)
				gen_stmt_ir(node->els, table);
			//println(".L.end.%d:", c);
			return; 
		}
		case ND_FOR: // or while
		{
			int c = count();

			// for handle while
			if(node->init)
				gen_stmt_ir(node->init, table);
			//println(".L.begin.%d:", c);
			if(node->cond)
			{
				VariablePtr res;
				gen_expr_ir(node->cond, &res, table);
				//println("  cmp $0, %%rax");
				//println("  je .L.end.%d", c);
			}
			gen_stmt_ir(node->then, table);

			if(node->inc){
				VariablePtr res;
				gen_expr_ir(node->inc, &res, table);
			}

			//println("  jmp .L.begin.%d", c);
			//println(".L.end.%d:", c);
			return; 
		}
		case ND_BLOCK:
		{
			// nest
			// int t = next_variable_name_number();
			// InMemoryIR.SetInsertPoint(t, std::to_string(t));
			for(Node * n = node->body; n; n = n->next){
				gen_stmt_ir(n, table);
			}
			return;
		}
		case ND_RETURN:
		{
			VariablePtr res;
			gen_expr_ir(node->lhs, &res, table);
			//println("  jmp .L.return.%s", current_fn->name);
			return;
		}
		case ND_EXPR_STMT:
		{
			//file_out << "arrive three 8" << std::endl;
			VariablePtr res = std::make_shared<Variable>();
			gen_expr_ir(node->lhs, &res, table);
			return;
		}
		default:
			return;
	}

	error("invalid statement");
}



static VariablePtr gen_variable_ir(Node *node, SymbolTablePtr table)
{
	switch(node->kind)
	{
		case ND_VAR:
			if(node->var->is_local)
			{
				// Local variable
				VariablePtr local_variable = std::make_shared<Variable>();
				local_variable->SetName(getPreName(node->var->name));
				local_variable->type = VaribleKind::VAR_32;
				if(InMemoryIR.Insert(NULL, NULL, local_variable, IROpKind::Op_Alloca, table))
				{
					table->insert(local_variable, 0);
				}
				table->findVar(local_variable->GetName(), local_variable);
				return local_variable;
				
				//println("  lea %d(%%rbp), %%rax", node->var->offset);
			}
			else
			{
				// Global variable
				//println("  lea %s(%%rip), %%rax", node->var->name);
			}
			return NULL;
		case ND_DEREF:
		{
			VariablePtr res;
			gen_expr_ir(node->lhs, &res, table);
			return NULL;
		}
		case ND_COMMA:
		{
			VariablePtr res;
			gen_expr_ir(node->lhs, &res, table);
			gen_variable_ir(node->rhs, table);			
			return NULL;
		}
		case ND_MEMBER:
			gen_variable_ir(node->lhs, table);
			//println("  add $%d, %%rax", node->member->offset);
			return NULL;
		default:
			return NULL;
	}

	error("not an lvalue");
}


// stack machine
static void gen_expr_ir(Node *node, VariablePtr* res, SymbolTablePtr table)
{
	//println("  .loc 1 %d", node->tok->line_no);
	// left = ...;
	// right = ...;
	// then deal them
	switch(node->kind)
	{
		case ND_NUM:
			// fix me:
			*res = std::make_shared<Variable>(node->val);
			//println("  mov $%ld, %%rax", node->val);
			return ;
		case ND_NEG:
			gen_expr_ir(node->lhs, res, table);
			//println("  neg %%rax");
			return ;
		case ND_VAR:
		case ND_MEMBER:
			gen_variable_ir(node, table);
			//load(node->ty);
			return ;
		case ND_DEREF:
			gen_expr_ir(node->lhs, res, table);
			//load(node->ty);
			return;
		case ND_ADDR:
			gen_variable_ir(node->lhs, table);
			return;
		case ND_ASSIGN:
		{
			VariablePtr left = gen_variable_ir(node->lhs, table);
			//push();
			gen_expr_ir(node->rhs, res, table);
			if(left != NULL){
				if(!(*res)->isConst){
					left->isConst = false;
					InMemoryIR.Insert(*res, left, IROpKind::Op_Store, table);
					// table->insert(left, *res, 0);
				}
				else
				{
					left->isConst = false;
					// here need const propagation
					left->isInitConst = true;
					left->Fval = (*res)->Fval;
					left->Ival = (*res)->Ival;
					left->type = (*res)->type;
					InMemoryIR.Insert(left, IROpKind::Op_Store, table);
					//file_out << "ASSIGN get value : "<< left->GetName() <<" = " << left->Ival <<" res is const?" << (*res)->isConst << std::endl;
				}
			}
			// need bind varibale at here
			//store(node->ty);
			return;
		}
		case ND_STMT_EXPR:
			for(Node * n = node->body; n; n = n->next)
			{
				gen_stmt_ir(n, table);
			}
			return;
		case ND_COMMA:
			gen_expr_ir(node->lhs, res, table);
			gen_expr_ir(node->rhs, res, table);
			return;
		case ND_FUNCALL:
			{
				int nargs = 0;
				for(Node *arg = node->args; arg; arg = arg->next)
				{
					gen_expr_ir(arg, res, table);
					//push();
					nargs++;
				}

				//for(int i = nargs - 1; i >= 0; i--)
				//	pop(argreg64[i]);

				//println("  mov $0, %%rax");
				//println("  call %s", node->funcname);
				return;
			}
	}

	// there must deal rhs first
	//file_out << "arrive three 6" << std::endl;
	VariablePtr right = std::make_shared<Variable>();
	gen_expr_ir(node->rhs, &right, table);
	//push();
	//file_out << "arrive three 5" << std::endl;
	VariablePtr left = std::make_shared<Variable>();
	gen_expr_ir(node->lhs, &left, table);
	//file_out << "arrive three 4" << std::endl;
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
	//file_out << "arrive three 3" << std::endl;

	assert(node->lhs != NULL && node->rhs != NULL);
	assert(table != NULL);
	switch(node->kind)
	{
		case ND_ADD:
			{
				
				
				// assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				//file_out << "arrive three 2" << std::endl;
				if((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM))
				{	
					assert(node->lhs != NULL);
					assert(node->rhs != NULL);
					if(node->lhs != NULL && node->rhs != NULL){
						// reload Variable '+/'-/'*/'/'
						node->kind = ND_NUM;
						*res = std::make_shared<Variable>(left->Ival + right->Ival);
						//*res =std::make_shared<Variable>(left->Ival + right->Ival);
						/*int constVaule = node->lhs->val + node->rhs->val;
						VariablePtr l, *r;
						(*res) =std::make_shared<Variable>();
						l =std::make_shared<Variable>(node->lhs->val);
						r =std::make_shared<Variable>(node->rhs->val);
						(*res)->SetName(next_variable_name());
						InMemoryIR.Insert(l, r, (*res), IROpKind::Op_ADD, table);	
						table->insert((*res), 0);
						*/		
					}
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						//assert(node->lhs->var == NULL);
						//assert(node->rhs->var != NULL);
						if(node->rhs->var != NULL){
							std::string s = getPreName(node->rhs->var->name);
							VariablePtr r;
							if(table->findVar(s, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = node->lhs->val + r->Ival;
								VariablePtr l;
								l = std::make_shared<Variable>(node->lhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_ADD, table);
								table->insert((*res), 0);
							}
						}
					}else if(node->rhs->kind == ND_NUM)
					{
						//assert(node->lhs->var != NULL);
						//assert(node->rhs->var == NULL);
						if(node->lhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							VariablePtr l;
							if(table->findVar(s, l)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival + node->rhs->val;
								VariablePtr r;
								r = std::make_shared<Variable>(node->rhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_ADD, table);
								table->insert((*res), 0);
							}
						}
					}else{
						// assert(node->lhs->var != NULL);
						// assert(node->rhs->var != NULL);
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							std::string s2 = getPreName(node->rhs->var->name);
							VariablePtr l , r;
							//file_out << "arrive three 1" << std::endl;
							if(table->findVar(s, l) && table->findVar(s2, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival + r->Ival;
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_ADD, table);
								table->insert((*res), 0);
							}
						}
					}
				}
				
				return;
			}
			//println("  add %s, %s", di, ax);
			
		case ND_SUB:
			{				
				// assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM))
				{
					if(node->lhs != NULL && node->rhs != NULL){
						node->kind = ND_NUM;
						*res = std::make_shared<Variable>(left->Ival - right->Ival);			
					}
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						if(node->rhs->var != NULL){
							std::string s = getPreName(node->rhs->var->name);
							VariablePtr r;
							if(table->findVar(s, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = node->lhs->val - r->Ival;
								VariablePtr l;
								l = std::make_shared<Variable>(node->lhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_SUB, table);
								table->insert((*res), 0);
							}
						}
					}else if(node->rhs->kind == ND_NUM)
					{
						if(node->lhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							VariablePtr l;
							if(table->findVar(s, l)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival - node->rhs->val;
								VariablePtr r;
								r = std::make_shared<Variable>(node->rhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_SUB, table);
								table->insert((*res), 0);
							}
						}
					}else{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							std::string s2 = getPreName(node->rhs->var->name);
							VariablePtr l, r;
							if(table->findVar(s, l) && table->findVar(s2, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival - r->Ival;
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_SUB, table);
								table->insert((*res), 0);
							}
						}
					}
				}
				
				return;
			}
		case ND_MUL:
			{				
				// assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM))
				{
					if(node->lhs != NULL && node->rhs != NULL){
						node->kind = ND_NUM;
						*res = std::make_shared<Variable>(left->Ival * right->Ival);
					}
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						if(node->rhs->var != NULL){
							std::string s = getPreName(node->rhs->var->name);
							VariablePtr r;
							if(table->findVar(s, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = node->lhs->val * r->Ival;
								VariablePtr l;
								l = std::make_shared<Variable>(node->lhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_MUL, table);
								table->insert((*res), 0);
							}
						}
					}else if(node->rhs->kind == ND_NUM)
					{
						if(node->lhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							VariablePtr l;
							if(table->findVar(s, l)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival * node->rhs->val;
								VariablePtr r;
								r = std::make_shared<Variable>(node->rhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_MUL, table);
								table->insert((*res), 0);
							}
						}
					}else{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							std::string s2 = getPreName(node->rhs->var->name);
							VariablePtr l, r;						
							if(table->findVar(s, l) && table->findVar(s2, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival * r->Ival;
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_MUL, table);
								table->insert((*res), 0);
							}
						}
					}
				}
				return;
			}
		case ND_DIV:
			{				
				// assert(node->lhs->kind == ND_VAR || node->lhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM))
				{
					if(node->lhs != NULL && node->rhs != NULL){
						node->kind = ND_NUM;
						*res = std::make_shared<Variable>(left->Ival / right->Ival);
					}
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						if(node->rhs->var != NULL){
							std::string s = getPreName(node->rhs->var->name);
							VariablePtr r;
							if(table->findVar(s, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = node->lhs->val * r->Ival;
								VariablePtr l;
								l = std::make_shared<Variable>(node->lhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_DIV, table);
								table->insert((*res), 0);
							}
						}
					}else if(node->rhs->kind == ND_NUM)
					{
						if(node->lhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							VariablePtr l;
							if(table->findVar(s, l)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival * node->rhs->val;
								VariablePtr r;
								r = std::make_shared<Variable>(node->rhs->val);
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_DIV, table);
								table->insert((*res), 0);
							}
						}
					}else{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							std::string s2 = getPreName(node->rhs->var->name);
							VariablePtr l, r;						
							if(table->findVar(s, l) && table->findVar(s2, r)){
								(*res) = std::make_shared<Variable>();
								//(*res)->Ival = l->Ival * r->Ival;
								(*res)->SetName(next_variable_name());
								InMemoryIR.Insert(l, r, (*res), IROpKind::Op_DIV, table);
								table->insert((*res), 0);
							}
						}
					}
				}
				return;
			}
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
			{
				return ;
				//(*res)->name = next_variable_name();
				assert(node->lhs->kind == ND_VAR || node->rhs->kind == ND_NUM);
				if((node->lhs->kind == ND_NUM) && (node->rhs->kind == ND_NUM))
				{
					if(node->lhs->var != NULL && node->rhs->var != NULL){
						
						/*if(node->kind == ND_EQ)
							(*res)->Ival = node->lhs->val == node->rhs->val;
						else if(node->kind == ND_NE)
							(*res)->Ival = node->lhs->val != node->rhs->val;
						else if(node->kind == ND_LT)
							(*res)->Ival = node->lhs->val < node->rhs->val;
						else if(node->kind == ND_LE)
							(*res)->Ival = node->lhs->val <= node->rhs->val;*/
					}
				}
				else
				{
					if(node->lhs->kind == ND_NUM)
					{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->rhs->var->name);
							VariablePtr r;
							table->findVar(s, r);
							std::string s2 = getPreName(node->rhs->var->name);
							/*if(node->kind == ND_EQ)
								(*res)->Ival = r->Ival == node->lhs->val;
							else if(node->kind == ND_NE)
								(*res)->Ival = r->Ival != node->lhs->val;
							else if(node->kind == ND_LT)
								(*res)->Ival = r->Ival < node->lhs->val;
							else if(node->kind == ND_LE)
								(*res)->Ival = r->Ival <= node->lhs->val;*/
						}
					}else if(node->rhs->kind == ND_NUM)
					{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);
							VariablePtr l;
							table->findVar(s, l);
							/*if(node->kind == ND_EQ)
								(*res)->Ival = l->Ival == node->rhs->val;
							else if(node->kind == ND_NE)
								(*res)->Ival = l->Ival != node->rhs->val;
							else if(node->kind == ND_LT)
								(*res)->Ival = l->Ival < node->rhs->val;
							else if(node->kind == ND_LE)
								(*res)->Ival = l->Ival <= node->rhs->val;*/
						}
					}else{
						if(node->lhs->var != NULL && node->rhs->var != NULL){
							std::string s = getPreName(node->lhs->var->name);					
							std::string s2 = getPreName(node->rhs->var->name);
							VariablePtr l, r;
							table->findVar(s, l);
							table->findVar(s2, r);
							/*if(node->kind == ND_EQ)
								(*res)->Ival = l->Ival == r->Ival;
							else if(node->kind == ND_NE)
								(*res)->Ival = l->Ival != r->Ival;
							else if(node->kind == ND_LT)
								(*res)->Ival = l->Ival < r->Ival;
							else if(node->kind == ND_LE)
								(*res)->Ival = l->Ival <= r->Ival;*/
						}
					}
				}
				return;
			}
			return;
		default:
			return;
	}

	error("invalid expression");
}
