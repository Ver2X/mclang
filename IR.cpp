#include "Variable.h"
#include "Function.h"
#include "Block.h"
#include "Instruction.h"
#include "SymbolTable.h"
#include "IRBuilder.h"
extern std::fstream file_out;
#define DEBUG 0
//////////////////////////////////////////////////////////////////////////     //////////////////////////////////////////////////////////////////////////////////



#define RPINT_VALUE { if(Op != IROpKind::Op_Alloca && Op !=  IROpKind::Op_Store && Op != IROpKind::Op_Load) s += ";     left:" + std::to_string(left->Ival) + " right:" + std::to_string(right->Ival) + " result:" + std::to_string(result->Ival) + "\n";}















// fix me: alloca need insert at font

