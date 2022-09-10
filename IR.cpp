#include "chibicc.h"
extern std::fstream file_out;

//////////////////////////////////////////////////////////////////////////     //////////////////////////////////////////////////////////////////////////////////
std::string Operand::CodeGen()
{
	std::string s;
	switch (type)
	{
		
		case VAR_8:
			s += "i8 ";
			break;
		case VAR_16:
			s += "i16 ";
			break;
		case VAR_32:
			s += "i32 ";
			break;
		case VAR_64:
			s += "i64 ";
			break;
		case VAR_PRT:
			s += "i32* ";
			break;
		default:
			break;
	}
	s += "%";
	s += name;
	return s;
}

std::string Instruction::CodeGen()
{
	std::string s;
	assert(result != NULL);
	switch(Op)
	{		
		case Op_ADD:
			s += "  " + result->name + " = " + "add ";
			if(left->isConst)
				s += left->name;
			else
				s += Twine("%", left->name);
			s += ", ";
			if(right->isConst)
				s += right->name;
			else
				s += Twine("%", right->name);
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case Op_SUB:
			s += "  " + result->name + " = " + "sub ";
			if(left->isConst)
				s += left->name;
			else
				s += Twine("%", left->name);
			s += ", ";
			if(right->isConst)
				s += right->name;
			else
				s += Twine("%", right->name);
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case Op_MUL:
			s += "  " + result->name + " = " + "mul ";
			if(left->isConst)
				s += left->name;
			else
				s += Twine("%", left->name);
			s += ", ";
			if(right->isConst)
				s += right->name;
			else
				s += Twine("%", right->name);
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case Op_DIV:
			s += "  " + result->name + " = " + "sdiv ";
			if(left->isConst)
				s += left->name;
			else
				s += Twine("%", left->name);
			s += ", ";
			if(right->isConst)
				s += right->name;
			else
				s += Twine("%", right->name);
			s += ", align " + std::to_string(result->align) + "\n";
			break;
		case Op_Alloca:
			s += "  %" + result->name + " = " + "alloca i32 " + ", align " + std::to_string(result->align) + "\n";
			break;
		default:
			break;
	}
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
	std::string var_name = var->name;
	if(auto iter = table.find(var_name) != table.end())
	{
		return false;
	}
	table.insert(make_pair(var_name, var));
	return true;
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
	result = table.find(var_name)->second;
	if(result == NULL)
		return false;
	else{
		assert(result != NULL);
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
		case RTY_VOID:
			s += "void ";
			break;
		case RTY_INT:
			s += "i32 ";
			break;
		case RTY_CHAR:
			s += "signext i8 ";
			break;
		case RTY_PTR:
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
	if(Op == Op_Alloca){
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

void IRBuilder::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op, int label, std::string name)
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
	if(Op == Op_Alloca){
		blocks[entry_label]->Insert(left, right, result, Op);
	}
	else
		blocks[label]->Insert(left, right, result, Op);
	// std::cout << "in ssss" << std::endl;
}

void IRBuilder::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op)
{
	Insert(left, right, result, Op, cache_label, cache_name);
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
