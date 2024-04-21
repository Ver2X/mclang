#include "Instruction.h"
#include "Block.h"
#include <string>

int Instruction::getAlign(VariablePtr left, VariablePtr right,
                          VariablePtr result) {
  if (left != nullptr && right != nullptr)
    return std::max(std::max(left->align, right->align), result->align);
  return result->align;
}

std::string BinaryOperator::CodeGen() {
  std::string s;

  switch (Op) {
  case IROpKind::Op_ADD:
    s += "  " + result->GetName() + " = " + "add nsw i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    // s += ", align " + std::to_string(result->align) + "\n";
    break;
  case IROpKind::Op_SUB:
    s += "  " + result->GetName() + " = " + "sub nsw i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    // s += ", align " + std::to_string(result->align) + "\n";
    break;
  case IROpKind::Op_MUL:
    s += "  " + result->GetName() + " = " + "mul nsw i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    // s += ", align " + std::to_string(result->align) + "\n";
    break;
  case IROpKind::Op_DIV:
    s += "  " + result->GetName() + " = " + "sdiv i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    // s += ", align " + std::to_string(result->align) + "\n";
    break;
  case IROpKind::Op_SLE:
    s += "  " + result->GetName() + " = " + "icmp sle i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  case IROpKind::Op_SLT:
    s += "  " + result->GetName() + " = " + "icmp slt i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  case IROpKind::Op_SGE:
    s += "  " + result->GetName() + " = " + "icmp gle i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  case IROpKind::Op_SGT:
    s += "  " + result->GetName() + " = " + "icmp glt i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  case IROpKind::Op_EQ:
    s += "  " + result->GetName() + " = " + "icmp eq i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  case IROpKind::Op_NE:
    s += "  " + result->GetName() + " = " + "icmp ne i32 ";
    s += left->GetName();
    s += ", ";
    s += right->GetName();
    // s += "\n";
    break;
  default:
    break;
  }
  return s;
}

std::string StoreInst::CodeGen() {

  if (source == nullptr)
    return "  store i32 " + std::to_string(this->Ival) + ", i32* " +
           dest->GetName() + ", align " +
           std::to_string(dest->align); // + "\n";
  else
    return "  store i32 " + source->GetName() + ", i32* " + dest->GetName() +
           ", align " + std::to_string(dest->align); // + "\n";
}

std::string LoadInst::CodeGen() {
  return "  " + dest->GetName() + " = load i32, " + "i32* " +
         source->GetName() + ", align " +
         std::to_string(source->align); // + "\n";
}

std::string AllocaInst::CodeGen() {
  return "  " + dest->GetName() + " = " + "alloca i32 " + ", align " +
         std::to_string(dest->align); // + "\n";
}

std::string BranchInst::CodeGen() {
  if (Op == IROpKind::Op_Branch) {
    std::string s;
    s += "  br i1 " + indicateVariable->GetName();
    if (targetFirst != nullptr)
      s += ", label " + targetFirst->GetName();
    if (targetSecond != nullptr)
      s += ", label " + targetSecond->GetName(); // + "\n";
    return s;
  } else {
    return "  br label " + targetFirst->GetName(); // + "\n";
  }
}

std::string ReturnInst::CodeGen() {
  if (returnValue != nullptr)
    return "  ret i32" + returnValue->GetName(); // + "\n";
  else
    return "  ret void\n";
}