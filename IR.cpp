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
#define RPINT_VALUE { if(Op != IROpKind::Op_Alloca) s += ";     left:" + std::to_string(left->Ival) + " right:" + std::to_string(right->Ival) + " result:" + std::to_string(result->Ival) + "\n";}

std::string Instruction::CodeGen()
{
	std::string s;
	assert(result != NULL);

	switch(Op)
	{		
		case IROpKind::Op_ADD:
			s += "  " + result->GetName() + " = " + "add ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";

			break;
		case IROpKind::Op_SUB:
			s += "  " + result->GetName() + " = " + "sub ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_MUL:
			s += "  " + result->GetName() + " = " + "mul ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_DIV:
			s += "  " + result->GetName() + " = " + "sdiv ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_Alloca:
			s += "  " + result->GetName() + " = " + "alloca i32 " + ", align " + std::to_string(result->align) + "\n";
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

bool SymbolTable::insert(Variable * var,int level)
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
	auto sVariable = std::make_shared<std::vector<Variable *>>();
	sVariable->push_back(var);
	table[var_name] = sVariable;
	return true;
}

bool SymbolTable::insert(Variable * var,Variable * newVar, int level)
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

bool SymbolTable::findVar(std::string & var_name, Variable * & result)
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
	Variable * head = args;
	//s += "argsNum is :";
	//s += std::to_string(argsNum);
	//s += "\n";
	for(int i = 0; i < argsNum; i++)
	{
		assert(head != NULL);
		s += head->CodeGen();
		if(i != argsNum - 1)
		{
			s += ", ";
		}
		head = head->next;
	}
	s += ")";
	return s;
}



std::string IRFunction::rename(){
	return functionName;
}
void IRFunction::AddArgs()
{
	// when enter function, need push varibale into symbol table
	// but when leave, destory it
}


int Instruction::getAlign(Variable * left, Variable * right, Variable * result)
{
	if(left != NULL && right != NULL)
		return std::max(std::max(left->align, right->align), result->align);
	return result->align;
}

// fix me: alloca need insert at font
void Block::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op)
{	

	Instruction * inst = new Instruction(left, right, result, Op);
	if(Op == IROpKind::Op_Alloca){
		allocas.push_back(inst);
	}
	else
		instructinos.push_back(inst);
}

void IRBuilder::SetInsertPoint(int label, std::string name)
{
	if(entry_label < 0)
		entry_label = label;
	if(blocks.count(label) == 0)
	{
		Block * block = new Block();
		block->SetName(name);
		block->SetLabel(label);
		blocks.insert(std::make_pair(label, block));
	}
	cache_label = label;
	cache_name = name;
}


// extern SymbolTablePtr symTable;
bool IRBuilder::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op, int label, std::string name, SymbolTablePtr table)
{
	if(entry_label < 0)
		entry_label = label;
	if(blocks.count(label) == 0)
	{
		Block * block = new Block();
		block->SetName(name);
		block->SetLabel(label);
		blocks.insert(std::make_pair(label, block));
	}

	if(Op == IROpKind::Op_Alloca){
		if(!table->findVar(result->GetName(), result)){
			blocks[entry_label]->Insert(left, right, result, Op);	
			return true;
		}else{
			return false;	
		}
	}
	else{
		blocks[label]->Insert(left, right, result, Op);
		return true;
	}
	// std::cout << "in ssss" << std::endl;
}

bool IRBuilder::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op, SymbolTablePtr table)
{
	return Insert(left, right, result, Op, cache_label, cache_name, table);
}

/*void Insert(Variable * left, Variable * right, Variable * result, IROpKind Op, Variable * AllocaResult)
{}*/

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
