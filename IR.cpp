#include "chibicc.h"


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




void SymbolTable::insert(Variable * var,int level)
{
	// down to special level
	std::string var_name = var->name;
	if(auto iter = table.find(var_name) != table.end())
	{
		// Error, variable redefine define
		return ;
	}
	table.insert(	make_pair(var_name, var) );
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
Variable * SymbolTable::find_var(std::string & var_name)
{
	return table.find(var_name)->second;
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




void IRBuilder::Insert(Variable * left, Variable * right, Variable * result, IROpKind Op)
{

}


std::string IRBuilder::CodeGen()
{
	std::string s;
	if(function != NULL)
	{
		s += function->CodeGen();
	}
	return s;
}
