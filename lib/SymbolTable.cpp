#include "SymbolTable.h"

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