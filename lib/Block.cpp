#include "Block.h"
#include "IRBuilder.h"
#include "Variable.h"
#include "Instruction.h"
#include "Block.h"
#include <cassert>
#include <memory>


extern std::string next_variable_name();

void Block::Insert(VariablePtr indicateVariable, BlockPtr targetOne, BlockPtr targetTwo, IROpKind Op, IRBuilder * buider)
{
	if(Op == IROpKind::Op_Branch)
	{
		buider->lastResVar = indicateVariable;
		assert(indicateVariable != NULL);
		auto branchInst = std::make_shared<BranchInst>(indicateVariable, targetOne, targetTwo, Op);
		instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
		return;
	}else{
		assert(Op == IROpKind::Op_UnConBranch);
		// buider->lastResVar = indicateVariable;
		assert(indicateVariable == NULL);
		assert(targetOne != NULL);
		assert(targetTwo == NULL);
		auto branchInst = std::make_shared<BranchInst>(targetOne, Op);
		instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
		return;
	}
}
void Block::Insert(VariablePtr left, VariablePtr right, VariablePtr result, IROpKind Op, IRBuilder * buider)
{	
	#if DEBUG
		file_out << "we going insert insert to blcok with name :" << this->GetName() << " label :" << this->GetLabel() << std::endl; 
	#endif
	switch(Op)
	{
		// shouldn't change order
		case IROpKind::Op_Alloca:
		{
			assert(left == NULL && right == NULL);
			auto allocaInst = std::make_shared<AllocaInst>(result);
			// InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			buider->lastResVar = result;			
			allocas.push_back(std::dynamic_pointer_cast<Instruction>(allocaInst));
		}
		case IROpKind::Op_Store:
		{
			auto storeInst = std::make_shared<StoreInst>(left, result);
			
			if(left == NULL && right == NULL){				
				storeInst->Ival = result->Ival;
			}
			buider->lastResVar = result;
			instructinos.push_back(std::dynamic_pointer_cast<Instruction>(storeInst));	
			return;
		}
		case IROpKind::Op_Load:
		{
			auto loadInst = std::make_shared<LoadInst>(left,result);
			buider->lastResVar = result;
			assert(right == NULL);
			instructinos.push_back(std::dynamic_pointer_cast<Instruction>(loadInst));	
			return;
		}
		/*case IROpKind::Op_Branch:
		{
			assert(false);
			buider->lastResVar = result;
			assert(result != NULL);
			auto branchInst = std::make_shared<BranchInst>(result, left, right, Op);
			instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
			return;
		}
		case IROpKind::Op_UnConBranch:
		{
			assert(false);
			buider->lastResVar = result;
			assert(result != NULL);
			assert(left == NULL);
			assert(right == NULL);
			auto branchInst = std::make_shared<BranchInst>(left, result, Op);
			instructinos.push_back(std::dynamic_pointer_cast<Instruction>(branchInst));
			return;
		}*/
		case IROpKind::Op_Return:
		{
			buider->lastResVar = result;
			// fix me: should not generate temp variable
			// assert(left == NULL); 
			// assert(right == NULL);
			auto returnInst = std::make_shared<ReturnInst>(result);
			instructinos.push_back(std::dynamic_pointer_cast<Instruction>(returnInst));
			return;
		}
		case IROpKind::Op_ADD:
		case IROpKind::Op_SUB:
		case IROpKind::Op_MUL:
		case IROpKind::Op_DIV:
		case IROpKind::Op_Cmp:
		{
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
				case IROpKind::Op_Cmp:
					s = "%cmp";
					break;
				default:
					break;
			}
			VariablePtr arithRes = std::make_shared<Variable>();
			int nextcf = buider->GetNextCountSuffix();
			if(nextcf != 0)
				s += std::to_string(nextcf);
			arithRes->SetName(s);


			VariablePtr load1;
			VariablePtr load2;

			if(!left->isConst && !right->isConst){
				load1 = std::make_shared<Variable>();
				load1->SetName(next_variable_name());

				load2 = std::make_shared<Variable>();
				load2->SetName(next_variable_name());

				auto inst1 = std::make_shared<LoadInst>(left, load1);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(inst1));

				auto inst2 = std::make_shared<LoadInst>(right, load2);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(inst2));

				auto instArith = std::make_shared<BinaryOperator>(load1, load2, arithRes, Op);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
			}else if(!left->isConst)
			{
				load1 = std::make_shared<Variable>();
				load1->SetName(next_variable_name());
				auto inst1 = std::make_shared<LoadInst>(left, load1);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(inst1));

				auto instArith = std::make_shared<BinaryOperator>(load1, right, arithRes, Op);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
			}else{
				load2 = std::make_shared<Variable>();
				load2->SetName(next_variable_name());
				auto inst2 = std::make_shared<LoadInst>(right,load2);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(inst2));

				auto instArith = std::make_shared<BinaryOperator>(left, load2, arithRes, Op);
				instructinos.push_back(std::dynamic_pointer_cast<Instruction>(instArith));
			}

			buider->lastResVar = arithRes;
			//InstructionPtr store = std::make_shared<Instruction>(arithRes, nullptr, result, IROpKind::Op_Store);

			
			result->SetName(std::move(s));
			//instructinos.push_back(store);

			return;
		}
		default:{
			assert(false);
		}
	}		
}

std::string Block::CodeGen()
{
	//#ifdef DEBUG
	//	file_out << "dumping block " << this->GetName() << std::endl;
	//#endif
	std::string s;
	if(name != "entry")
		s += "\n";
	if(this->allocas.empty())
		s += name + ":\n";
	for(const auto & ins : instructinos)
	{
		s += ins->CodeGen();
	}
	return s;
}

std::string Block::AllocaCodeGen()
{
	std::string s;
	s += name + ":\n";
	for(const auto & ins : allocas)
	{
		s += ins->CodeGen();
	}
	return s;
}
