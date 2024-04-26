#include "Instruction.h"
#include "BasicBlock.h"
#include <string>

int Instruction::getAlign(VariablePtr Left, VariablePtr Right,
                          VariablePtr Result) {
  if (Left != nullptr && Right != nullptr)
    return std::max(std::max(Left->Align, Right->Align), Result->Align);
  return Result->Align;
}

std::string BinaryOperator::CodeGen() {
  std::string s;

  switch (Op) {
  case IROpKind::Op_ADD:
    s += "  " + Result->getName() + " = " + "add nsw i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    // s += ", align " + std::to_string(Result->Align) + "\n";
    break;
  case IROpKind::Op_SUB:
    s += "  " + Result->getName() + " = " + "sub nsw i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    // s += ", align " + std::to_string(Result->Align) + "\n";
    break;
  case IROpKind::Op_MUL:
    s += "  " + Result->getName() + " = " + "mul nsw i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    // s += ", align " + std::to_string(Result->Align) + "\n";
    break;
  case IROpKind::Op_DIV:
    s += "  " + Result->getName() + " = " + "sdiv i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    // s += ", align " + std::to_string(Result->Align) + "\n";
    break;
  case IROpKind::Op_SLE:
    s += "  " + Result->getName() + " = " + "icmp sle i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    break;
  case IROpKind::Op_SLT:
    s += "  " + Result->getName() + " = " + "icmp slt i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    break;
  case IROpKind::Op_SGE:
    s += "  " + Result->getName() + " = " + "icmp gle i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    break;
  case IROpKind::Op_SGT:
    s += "  " + Result->getName() + " = " + "icmp glt i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    break;
  case IROpKind::Op_EQ:
    s += "  " + Result->getName() + " = " + "icmp eq i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
    // s += "\n";
    break;
  case IROpKind::Op_NE:
    s += "  " + Result->getName() + " = " + "icmp ne i32 ";
    s += Left->getName();
    s += ", ";
    s += Right->getName();
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
           dest->getName() + ", align " +
           std::to_string(dest->Align); // + "\n";
  else
    return "  store i32 " + source->getName() + ", i32* " + dest->getName() +
           ", align " + std::to_string(dest->Align); // + "\n";
}

std::string LoadInst::CodeGen() {
  return "  " + dest->getName() + " = load i32, " + "i32* " +
         source->getName() + ", align " +
         std::to_string(source->Align); // + "\n";
}

std::string AllocaInst::CodeGen() {
  return "  " + dest->getName() + " = " + "alloca i32 " + ", align " +
         std::to_string(dest->Align); // + "\n";
}

std::string CallInst::CodeGen() {
  std::string Res;
  // if (func->retTy == ReturnTypeKind::RTY_VOID) {
  //   Res += "  call void @";
  // } else {
  Res += "  " + dest->getName() + " =";
  Res += "  call i32 @";
  // }
  Res += func->FunctionName + "("; // + "\n";
  for (auto x : args) {
    Res += "i32 ";
    Res += x->getName();
    if (x != args[args.size() - 1]) {
      Res += ", ";
    }
  }
  Res += ")";
  return Res;
}

std::string BranchInst::CodeGen() {
  if (Op == IROpKind::Op_Branch) {
    std::string s;
    s += "  br i1 " + indicateVariable->getName();
    if (targetFirst != nullptr)
      s += ", label " + targetFirst->getName();
    if (targetSecond != nullptr)
      s += ", label " + targetSecond->getName(); // + "\n";
    return s;
  } else {
    return "  br label " + targetFirst->getName(); // + "\n";
  }
}

std::string ReturnInst::CodeGen() {
  if (returnValue != nullptr)
    return "  ret i32 " + returnValue->getName(); // + "\n";
  else
    return "  ret void\n";
}