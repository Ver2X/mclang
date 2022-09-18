#include "Variable.h"
Operand::Operand()
{
	Ival = 0;
	next = nullptr;
	align = 4;
	isConst = false;
	type = VaribleKind::VAR_32;
}
Operand::Operand(int64_t v)
{
	Ival = v;
	next = nullptr;
	align = 8;
	isConst = true;
	name = std::to_string(Ival);
	type = VaribleKind::VAR_64;
}

Operand::Operand(int v)
{
	Ival = v;
	next = nullptr;
	align = 4;
	isConst = true;
	name = std::to_string(Ival);
	type = VaribleKind::VAR_32;
}	

Operand::Operand(double v)
{
	Fval = v;
	next = nullptr;
	align = 8;
	isConst = true;
	name = std::to_string(Fval);
	type = VaribleKind::VAR_64;
}

void Operand::SetConst(double v)
{
	Fval = v;
	next = nullptr;
	align = 8;
	isConst = true;
	name = std::to_string(Fval);
	type = VaribleKind::VAR_64;
}	

void Operand::SetConst(int v)
{
	Ival = v;
	next = nullptr;
	align = 4;
	isConst = true;
	name = std::to_string(Ival);
	type = VaribleKind::VAR_32;
}	

void Operand::SetConst(int64_t v)
{
	Ival = v;
	next = nullptr;
	align = 8;
	isConst = true;
	name = std::to_string(Ival);
	type = VaribleKind::VAR_64;
}	





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