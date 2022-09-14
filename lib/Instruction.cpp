#include "Instruction.h"



int Instruction::getAlign(VariablePtr left, VariablePtr right, VariablePtr result)
{
	if(left != NULL && right != NULL)
		return std::max(std::max(left->align, right->align), result->align);
	return result->align;
}
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
		{	if(result != NULL)
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