#include "Block.h"
#include "IRBuilder.h"
#include "Variable.h"
#include "Instruction.h"
#include "Block.h"


extern std::string next_variable_name();


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
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			buider->lastResVar = result;
			allocas.push_back(inst);
		}
		case IROpKind::Op_Store:
		{
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			
			if(left == NULL && right == NULL){				
				inst->Ival = result->Ival;
			}
			buider->lastResVar = result;
			instructinos.push_back(inst);	
			return;
		}
		case IROpKind::Op_Load:
		{
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			buider->lastResVar = result;
			assert(right == NULL);
			instructinos.push_back(inst);	
			return;
		}
		case IROpKind::Op_Branch:
		{
			buider->lastResVar = result;
			assert(result != NULL);
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			instructinos.push_back(inst);
			return;
		}
		case IROpKind::Op_UnConBranch:
		{
			buider->lastResVar = result;
			assert(result != NULL);
			assert(left == NULL);
			assert(right == NULL);
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			instructinos.push_back(inst);
			return;
		}
		case IROpKind::Op_Return:
		{
			buider->lastResVar = result;
			assert(left == NULL);
			assert(right == NULL);
			InstructionPtr inst = std::make_shared<Instruction>(left, right, result, Op);
			instructinos.push_back(inst);
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
			InstructionPtr inst1;
			InstructionPtr inst2;

			InstructionPtr instArith;

			if(!left->isConst && !right->isConst){
				load1 = std::make_shared<Variable>();
				load1->SetName(next_variable_name());

				load2 = std::make_shared<Variable>();
				load2->SetName(next_variable_name());

				inst1 = std::make_shared<Instruction>(left, nullptr, load1, IROpKind::Op_Load);
				instructinos.push_back(inst1);

				inst2 = std::make_shared<Instruction>(right, nullptr, load2, IROpKind::Op_Load);
				instructinos.push_back(inst2);

				instArith = std::make_shared<Instruction>(load1, load2, arithRes, Op);
				instructinos.push_back(instArith);
			}else if(!left->isConst)
			{
				load1 = std::make_shared<Variable>();
				load1->SetName(next_variable_name());
				inst1 = std::make_shared<Instruction>(left, nullptr, load1, IROpKind::Op_Load);
				instructinos.push_back(inst1);

				instArith = std::make_shared<Instruction>(load1, right, arithRes, Op);
				instructinos.push_back(instArith);
			}else{
				load2 = std::make_shared<Variable>();
				load2->SetName(next_variable_name());
				inst2 = std::make_shared<Instruction>(right, nullptr, load2, IROpKind::Op_Load);
				instructinos.push_back(inst2);

				instArith = std::make_shared<Instruction>(left, load2, arithRes, Op);
				instructinos.push_back(instArith);
			}

			buider->lastResVar = arithRes;
			//InstructionPtr store = std::make_shared<Instruction>(arithRes, nullptr, result, IROpKind::Op_Store);

			
			result->SetName(std::move(s));
			//instructinos.push_back(store);

			return;
		}
		default:
			return; 
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
