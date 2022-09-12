#include "mclang.h"
extern std::fstream file_out;
//////////////////////////////////////////////////////////////////////////     //////////////////////////////////////////////////////////////////////////////////
std::string Operand::CodeGen()
{
	std::string s;
	switch (type)
	{
		
		case VaribleKind::VAR_8:
			s += "i8 ";
			break;
		case VaribleKind::VAR_16:
			s += "i16 ";
			break;
		case VaribleKind::VAR_32:
			s += "i32 ";
			break;
		case VaribleKind::VAR_64:
			s += "i64 ";
			break;
		case VaribleKind::VAR_PRT:
			s += "i32* ";
			break;
		default:
			break;
	}
	// s += "%";
	s += name;
	return s;
}
#define DEBUG 1
#define RPINT_VALUE { if(Op != IROpKind::Op_Alloca && Op !=  IROpKind::Op_Store && Op != IROpKind::Op_Load) s += ";     left:" + std::to_string(left->Ival) + " right:" + std::to_string(right->Ival) + " result:" + std::to_string(result->Ival) + "\n";}

std::string Instruction::CodeGen()
{
	std::string s;
	assert(result != NULL);

	switch(Op)
	{		
		case IROpKind::Op_ADD:
			s += "  " + result->GetName() + " = " + "add nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";

			break;
		case IROpKind::Op_SUB:
			s += "  " + result->GetName() + " = " + "sub nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_MUL:
			s += "  " + result->GetName() + " = " + "mul nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_DIV:
			s += "  " + result->GetName() + " = " + "sdiv i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_Alloca:
			s += "  " + result->GetName() + " = " + "alloca i32 " + ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_Store:
			{
				if(left == NULL)
					s += "  store i32 " +  std::to_string(this->Ival) + ", i32* " + result->GetName()  + ", align " + std::to_string(result->align) + "\n";
				else
					s += "  store i32 " +  left->GetName() + ", i32* " + result->GetName()  + ", align " + std::to_string(result->align) + "\n";
				break;
			}
			
		case IROpKind::Op_Load:
			s += "  " + result->GetName() + " = load i32, " + "i32* " + left->GetName()  + ", align " + std::to_string(left->align) + "\n";
			break;	
		default:
			break;
	}
	#if DEBUG
		RPINT_VALUE		
	#endif
	return s;
}


std::string Block::CodeGen()
{
	std::string s;
	// s += name + ":\n";
	for(const auto & ins : instructinos)
	{
		s += ins->CodeGen();
	}
	return s;
}

std::string Block::AllocaCodeGen()
{
	std::string s;
	// s += name + ":\n";
	for(const auto & ins : allocas)
	{
		s += ins->CodeGen();
	}
	return s;
}

bool SymbolTable::insert(VariablePtr var,int level)
{
	// down to special level
	std::string var_name = var->GetName();
	auto iter = table.find(var_name);

	if(iter != table.end())
	{
		
					
		auto finout = [=](VarList vars){
			auto res = std::find(vars->begin(), vars->end(), var);
			if(res != vars->end())
			{
				return false;
			}
			else
			{
				vars->push_back(var);
				return true;
			}	
		};
		return finout(iter->second);
	}
	auto sVariable = std::make_shared<std::vector<VariablePtr>>();
	sVariable->push_back(var);
	table[var_name] = sVariable;
	return true;
}

bool SymbolTable::insert(VariablePtr var,VariablePtr newVar, int level)
{
	// down to special level
	std::string var_name = var->GetName();
	auto iter = table.find(var_name);
	if(iter == table.end()){
		return false;
	}
	assert(iter != table.end());
	auto finout = [=](VarList vars){
		auto res = std::find(vars->begin(), vars->end(), newVar);
		if(res != vars->end())
		{
			return false;
		}
		else
		{
			vars->push_back(newVar);
			return true;
		}	
	};
	return finout(iter->second);
}
// use a cache save inserted varibale, when leaving function, delete
// it from symbol table
void SymbolTable::erase(std::string var_name,int level)
{
	auto iter = table.find(var_name);
	if(iter != table.end())
	{
		table.erase(iter);
	}
}

bool SymbolTable::findVar(std::string & var_name, VariablePtr & result)
{
	//result = table.find(var_name)->second->back();
	auto it = table.find(var_name);
	if(it == table.end())
	{
		// shouldn't flush result !!!
		// it will make insert alloca instruction error
		return false;
	}else{
		result = it->second->back();
		return true;
	}
}




std::string IRFunction::CodeGen()
{
	// if body non-null
	std::string s;
	s += "define dso_local ";
	switch (retTy)
	{
		case ReturnTypeKind::RTY_VOID:
			s += "void ";
			break;
		case ReturnTypeKind::RTY_INT:
			s += "i32 ";
			break;
		case ReturnTypeKind::RTY_CHAR:
			s += "signext i8 ";
			break;
		case ReturnTypeKind::RTY_PTR:
			// dump de type
			s += "i32";
			s+="* ";
			break;	
		default:
			s += "void ";
			break;
	}
	s += "@";
	s += rename();
	s += "(";
	//VariablePtr head = args;
	//s += "argsNum is :";
	//s += std::to_string(argsNum);
	//s += "\n";
	for(auto Begin = args.begin(), End = args.end(); Begin != End; Begin++)
	{
		s += (*Begin)->CodeGen();
		if(Begin != End - 1)
		{
			s += ", ";
		}
	}
	/*for(int i = 0; i < argsNum; i++)
	{
		assert(head != NULL);
		s += head->CodeGen();
		if(i != argsNum - 1)
		{
			s += ", ";
		}
		head = head->next;
	}*/
	s += ")";
	return s;
}



std::string IRFunction::rename(){
	// _ + return type + name + arg
	std::string s = "_";
	if(retTy == ReturnTypeKind::RTY_INT)
		s += "Z";
	s += functionName;

	for(auto arg_iter : args)
	{
		if(arg_iter->type == VaribleKind::VAR_32)
			s += "i";
	}
	return s;
}
void IRFunction::AddArgs()
{
	// when enter function, need push varibale into symbol table
	// but when leave, destory it
}


int Instruction::getAlign(VariablePtr left, VariablePtr right, VariablePtr result)
{
	if(left != NULL && right != NULL)
		return std::max(std::max(left->align, right->align), result->align);
	return result->align;
}

// fix me: alloca need insert at font

void Block::Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, IRBuilder * buider)
{	

	
	switch(Op)
	{
		// shouldn't change order
		case IROpKind::Op_Alloca:
		{
			Instruction * inst = new Instruction(left, right, result, Op);
			allocas.push_back(inst);
		}
		case IROpKind::Op_Store:
		{
			Instruction * inst = new Instruction(left, right, result, Op);
			if(Op == IROpKind::Op_Store){
				if(left == NULL && right == NULL){
					if(Op == IROpKind::Op_Store)
						inst->Ival = result->Ival;
				}
				instructinos.push_back(inst);
			}
			return;
		}
		case IROpKind::Op_ADD:
		case IROpKind::Op_SUB:
		case IROpKind::Op_MUL:
		case IROpKind::Op_DIV:
		{
			VariablePtr load1 = std::make_shared<Variable>();
			load1->SetName(next_variable_name());
			VariablePtr load2 = std::make_shared<Variable>();
			load2->SetName(next_variable_name());
			Instruction * inst1 = new Instruction(left, NULL, load1, IROpKind::Op_Load);
			Instruction * inst2 = new Instruction(right, NULL, load2, IROpKind::Op_Load);
			std::string s;
			switch(Op)
			{
				case IROpKind::Op_ADD:
					s = "%add";
					break;
				case IROpKind::Op_SUB:
					s = "%sub";
					break;
				case IROpKind::Op_MUL:
					s = "%mul";
					break;
				case IROpKind::Op_DIV:
					s = "%div";
					break;
				default:
					break;
			}
			VariablePtr arithRes = std::make_shared<Variable>();
			s += std::to_string(buider->GetNextCountSuffix());
			arithRes->SetName(std::move(s));

			Instruction * instArith = new Instruction(load1, load2, arithRes, Op);

			Instruction * store = new Instruction(arithRes, NULL, result, IROpKind::Op_Store);
			instructinos.push_back(inst1);
			instructinos.push_back(inst2);
			instructinos.push_back(instArith);
			instructinos.push_back(store);
			

			return;
		}
		default:
			return; 
	}		
}

void IRBuilder::SetInsertPoint(int label, std::string name)
{
	if(entry_label < 0)
		entry_label = label;
	if(blocks.count(label) == 0)
	{
		BlockPtr block = std::make_shared<Block>();
		block->SetName(name);
		block->SetLabel(label);
		blocks.insert(std::make_pair(label, block));
	}
	cache_label = label;
	cache_name = name;
}


// extern SymbolTablePtr symTable;
bool IRBuilder::Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, int label, std::string name, SymbolTablePtr table)
{
	if(entry_label < 0)
		entry_label = label;
	if(blocks.count(label) == 0)
	{
		BlockPtr block = std::make_shared<Block>();
		block->SetName(name);
		block->SetLabel(label);
		blocks.insert(std::make_pair(label, block));
	}

	if(Op == IROpKind::Op_Alloca){
		if(!table->findVar(result->GetName(), result)){
			blocks[entry_label]->Insert(left, right, result, Op, this);	
			return true;
		}else{
			return false;	
		}
	}
	else{
		blocks[label]->Insert(left, right, result, Op, this);
		return true;
	}
	// std::cout << "in ssss" << std::endl;
}

// for Op_Alloca
// 
bool IRBuilder::Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, SymbolTablePtr table)
{
	return Insert(left, right, result, Op, cache_label, cache_name, table);
}

bool IRBuilder::Insert(VariablePtr result, IROpKind Op, SymbolTablePtr table)
{
	// store num
	return Insert(NULL, NULL, result, Op, cache_label, cache_name, table);
}

bool IRBuilder::Insert(VariablePtr source, VariablePtr dest, IROpKind Op, SymbolTablePtr table)
{
	// store identity
	return Insert(source, NULL, dest, Op, cache_label, cache_name, table);
}


std::string IRBuilder::CodeGen()
{
	std::string s;
	if(function != NULL)
	{
		s += function->CodeGen();
	}
	s += "{\n";
	for(const auto & blk: blocks)
	{
		// *blk.second;
		if(blk.first == entry_label)
			s += blk.second->AllocaCodeGen();
		s += blk.second->CodeGen();
	}
	s += "}\n";
	return s;
}
