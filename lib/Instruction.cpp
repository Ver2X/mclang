#include "Instruction.h"
#include "BasicBlock.h"
#include "Module.h"
#include <string>

extern std::shared_ptr<Module> ProgramModule;

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

  if (Source == nullptr)
    return "  store i32 " + std::to_string(this->Ival) + ", i32* " +
           Dest->getName() + ", align " +
           std::to_string(Dest->Align); // + "\n";
  else
    return "  store i32 " + Source->getName() + ", i32* " + Dest->getName() +
           ", align " + std::to_string(Dest->Align); // + "\n";
}

std::string LoadInst::CodeGen() {
  return "  " + Dest->getName() + " = load i32, " + "i32* " +
         Source->getName() + ", align " +
         std::to_string(Source->Align); // + "\n";
}

std::string AllocaInst::CodeGen() {
  if (ArraySize) {
    return "  " + Dest->getName() + " = " + "alloca [" +
           std::to_string(ArraySize->Ival) + " x i32] " + ", align " +
           std::to_string(Dest->Align); // + "\n";
  } else if (VTy->Kind == TypeKind::TY_STRUCT) {
    return "  " + Dest->getName() + " = " + "alloca " +
           ProgramModule->GetType(VTy)->getName() + ", align " +
           std::to_string(Dest->Align);
  } else {
    return "  " + Dest->getName() + " = " + "alloca i32 " + ", align " +
           std::to_string(Dest->Align); // + "\n";
  }
}

std::string CallInst::CodeGen() {
  std::string Res;
  // if (func->RetTy == ReturnTypePtr::RTY_VOID) {
  //   Res += "  call void @";
  // } else {
  Res += "  " + Dest->getName() + " =";
  Res += "  call i32 @";
  // }
  Res += func->getName() + "("; // + "\n";
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

std::string GetElementPtrInst::CodeGen() {
  // %arrayidx? = getelementptr inbounds [2 x i32], [2 x i32]* %a, i64 0, i64 0
  std::string s = "  " + Result->getName() + " = getelementptr inbounds ";
  // assert(baseTo(PtrTy)->Kind == TypeKind::TY_STRUCT);
  if (baseTo(PtrTy)->Kind == TypeKind::TY_ARRAY) {
    s += baseTo(PtrTy)->CodeGen() + ", ";
    s += BasePtr->CodeGen();
  } else if (baseTo(PtrTy)->Kind == TypeKind::TY_STRUCT) {
    s += ProgramModule->GetType(baseTo(PtrTy))->getName() + ", ";
    // assert(false);
    s += ProgramModule->GetType(baseTo(PtrTy))->getName() + "* " +
         BasePtr->getName();
  }

  for (auto Idx : IdxList) {
    s += ", ";
    s += Idx->CodeGen();
  }
  return s;
}