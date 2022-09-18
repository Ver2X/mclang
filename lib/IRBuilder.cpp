#include "IRBuilder.h"
#include "Instruction.h"
#include "Variable.h"
#include <cassert>
#include <memory>
void IRBuilder::SetInsertPoint(int label, std::string name)
{
	if(entry_label < 0)
		entry_label = label;
	if(cache_label != label){
		if(blocks.count(label) == 0)
		{
			#if DEBUG
				file_out << "create a new blcok " << std::endl;
			#endif
			BlockPtr block = std::make_shared<Block>();
			block->SetName(name);
			block->SetLabel(label);
			blocks.insert(std::make_pair(label, block));
		}
		cache_label = label;
		cache_name = name;
	}
	
}

void IRBuilder::SetInsertPoint(BlockPtr insertPoint)
{
	if(entry_label < 0)
		entry_label = insertPoint->GetLabel();
	if(cache_label != insertPoint->GetLabel()){	
		if(blocks.count(insertPoint->GetLabel()) == 0)
		{
			#if DEBUG
				file_out << "create a new blcok " << std::endl;
			#endif
			blocks.insert(std::make_pair(insertPoint->GetLabel(), insertPoint));
		}
		cache_label = insertPoint->GetLabel();
		cache_name = insertPoint->GetName();
	}
	
}

void IRBuilder::InsertBasicBlock(int label, std::string name, int pred)
{
	// fix me :
	// keep pred succ right
	if(entry_label < 0)
		entry_label = label;
	if(blocks.count(label) == 0)
	{
		#if DEBUG
			file_out << "create a new blcok " << std::endl;
		#endif

		BlockPtr block = std::make_shared<Block>();
		blocks[pred]->succes.push_back(block);
		block->preds.push_back(blocks[pred]);


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
	return Insert(nullptr, nullptr, result, Op, cache_label, cache_name, table);
}


bool IRBuilder::Insert(VariablePtr source, VariablePtr dest, IROpKind Op, SymbolTablePtr table)
{
	// store identity
	return Insert(source, nullptr, dest, Op, cache_label, cache_name, table);
}
bool IRBuilder::Insert(VariablePtr indicateVariable, BlockPtr targetOne, BlockPtr targetTwo, IROpKind Op, SymbolTablePtr table)
{
	if(entry_label < 0)
		entry_label = cache_label;
	if(blocks.count(cache_label) == 0)
	{
		BlockPtr block = std::make_shared<Block>();
		block->SetName(cache_name);
		block->SetLabel(cache_label);
		blocks.insert(std::make_pair(cache_label, block));
	}
	assert(Op == IROpKind::Op_Branch || Op == IROpKind::Op_UnConBranch);
	blocks[cache_label]->Insert(indicateVariable, targetOne, targetTwo, Op, this);
	return true;
}

void IRBuilder::FixNonReturn(SymbolTablePtr table)
{
	BlockPtr lastBlock;
	for(const auto & blk: blocks)
	{
		lastBlock = blk.second;
	}
	// fix no return statement
	if(lastBlock->instructinos.empty())
	{
		if(function->retTy == ReturnTypeKind::RTY_VOID){
			// fix me: remove temp variables
			VariablePtr tempRetVoid = std::make_shared<Variable>();
			tempRetVoid->SetName("void");
			auto t1 = std::make_shared<Variable>();
			auto t2 = std::make_shared<Variable>();
			this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, table);
			//this->Insert(std::make_shared<Variable>(), std::make_shared<Variable>(), tempRetVoid, IROpKind::Op_Return, table);
		}
		else{
			auto t1 = std::make_shared<Variable>();
			auto t2 = std::make_shared<Variable>();
			this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, table);
			//this->Insert(std::make_shared<Variable>(), std::make_shared<Variable>(), std::make_shared<Variable>(lastResVar), IROpKind::Op_Return, table);
		}
			
	}else{
		InstructionPtr lastInst = *(lastBlock->instructinos.end() - 1);
	 	if(auto d = std::dynamic_pointer_cast<ReturnInst>(lastInst); d == nullptr)
		{
	 		if(function->retTy == ReturnTypeKind::RTY_VOID){
				// fix me: remove temp variable
				VariablePtr tempRetVoid = std::make_shared<Variable>();
				tempRetVoid->SetName("void");
				auto t1 = std::make_shared<Variable>();
				auto t2 = std::make_shared<Variable>();
				this->Insert(t1, t2, tempRetVoid, IROpKind::Op_Return, table);
			}else{
				auto t1 = std::make_shared<Variable>();
				auto t2 = std::make_shared<Variable>();
				this->Insert(t1, t2, lastResVar, IROpKind::Op_Return, table);
			}
	 	}
	}	
}

std::string IRBuilder::CodeGen()
{
	std::string s;
	if(function != nullptr)
	{
		s += function->CodeGen();
	}
	s += "{\n";
	BlockPtr lastBlock;
	for(const auto & blk: blocks)
	{
		#if DEBUG
			file_out << "dump instructinos in block , name: " << blk.second->GetName() <<  "label: " << blk.second->GetLabel()  << " size :" << blk.second->instructinos.size()  << std::endl;
		#endif
		if(blk.first == entry_label)
			s += blk.second->AllocaCodeGen();
		s += blk.second->CodeGen();
		lastBlock = blk.second;
	}
	s += "}\n";
	return s;
}