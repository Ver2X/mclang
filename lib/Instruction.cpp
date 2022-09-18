#include "Instruction.h"
#include "Block.h"
#include <string>



int Instruction::getAlign(VariablePtr left, VariablePtr right, VariablePtr result)
{
	if(left != NULL && right != NULL)
		return std::max(std::max(left->align, right->align), result->align);
	return result->align;
}



std::string BinaryOperator::CodeGen()
{
	std::string s;

	switch(Op)
	{		
		case IROpKind::Op_ADD:
			s += "  " + result->GetName() + " = " + "add nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_SUB:
			s += "  " + result->GetName() + " = " + "sub nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_MUL:
			s += "  " + result->GetName() + " = " + "mul nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_DIV:
			s += "  " + result->GetName() + " = " + "sdiv i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_SLE:
			s += "  " + result->GetName() + " = " + "icmp sle ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;	
		case IROpKind::Op_SLT:
			s += "  " + result->GetName() + " = " + "icmp slt ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;
		case IROpKind::Op_SGE:
			s += "  " + result->GetName() + " = " + "icmp gle ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;
		case IROpKind::Op_SGT:
			s += "  " + result->GetName() + " = " + "icmp glt ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;
		case IROpKind::Op_EQ:
			s += "  " + result->GetName() + " = " + "icmp eq ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;
		case IROpKind::Op_NE:
			s += "  " + result->GetName() + " = " + "icmp ne ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			break;
		default:
			break;
	}
	return s;
}


std::string StoreInst::CodeGen()
{

	if(source == NULL)
		return "  store i32 " +  std::to_string(this->Ival) + ", i32* " + dest->GetName()  + ", align " + std::to_string(dest->align) + "\n";
	else
		return "  store i32 " +  source->GetName() + ", i32* " + dest->GetName()  + ", align " + std::to_string(dest->align) + "\n";
			
}

std::string LoadInst::CodeGen()
{
	return "  " + dest->GetName() + " = load i32, " + "i32* " + source->GetName()  + ", align " + std::to_string(source->align) + "\n";
}

std::string AllocaInst::CodeGen()
{
	return "  " + dest->GetName() + " = " + "alloca i32 " + ", align " + std::to_string(dest->align) + "\n";
}

std::string BranchInst::CodeGen()
{
	if(Op == IROpKind::Op_Branch)
	{	
		std::string s;
		s += "  br i1 " + indicateVariable->GetName();
		if(targetFirst != NULL)
			s += ", label " + targetFirst->GetName();
		if(targetSecond != NULL)
			s += ", label " + targetSecond->GetName() + "\n";
		return s;
	}else{
		return "  br label " + targetFirst->GetName() + "\n";
	}	
}

std::string ReturnInst::CodeGen()
{
	if(returnValue != NULL)
		return "  ret " + returnValue->GetName() + "\n";
	else
		return "  ret void\n";
}

/*
std::string Instruction::CodeGen()
{
	std::string s;

	switch(Op)
	{		
		case IROpKind::Op_ADD:
			s += "  " + result->GetName() + " = " + "add nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_SUB:
			s += "  " + result->GetName() + " = " + "sub nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_MUL:
			s += "  " + result->GetName() + " = " + "mul nsw i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_DIV:
			s += "  " + result->GetName() + " = " + "sdiv i32 ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
			// s += ", align " + std::to_string(result->align) + "\n";
			break;
		case IROpKind::Op_Cmp:
			s += "  " + result->GetName() + " = " + "icmp sgt ";
			s += left->GetName();
			s += ", ";
			s += right->GetName();
			s += "\n";
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
		case IROpKind::Op_Branch:
			{	
				s += "  br i1 " + result->GetName();
				if(left != NULL)
					s += ", label " + left->GetName();
				if(right != NULL)
					s += ", label " + right->GetName() + "\n";;
				break;
			}
		case IROpKind::Op_UnConBranch:
		{	
			s += "  br label " + result->GetName() + "\n";
			break;
		}
		case IROpKind::Op_Return:
		{	
			if(result != NULL)
				s += "  ret " + result->GetName() + "\n";
			else
				s += "  ret void\n";
			break;
		}
		default:
			break;
	}
	#if DEBUG
		RPINT_VALUE		
	#endif
	return s;
}
*/