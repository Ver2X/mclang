#include "ir/Instruction.h"
#include "ir/BasicBlock.h"
#include "ir/Module.h"
#include <map>
#include <string>

extern std::shared_ptr<Module> ProgramModule;

void Instruction::eraseFromParent() {
  this->getParent()->eraseFromParent(shared_from_this());
}

int Instruction::getAlign(VariablePtr Left, VariablePtr Right,
                          VariablePtr Result) {
  if (Left != nullptr && Right != nullptr)
    return std::max(std::max(Left->Align, Right->Align), Result->Align);
  return Result->Align;
}

void PHINode::addIncoming(VariablePtr InComingValue, BasicBlockPtr InComingBB) {
  InComingValues.emplace_back(this, CurNumOfIncomingValues);
  InComingValues.back() = InComingValue;
  InComingBlocks.push_back(InComingBB);
  CurNumOfIncomingValues++;
}
std::string BinaryOperator::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  std::string s;
  std::map<IROpKind, std::string> MapOpToStr;
  MapOpToStr[IROpKind::Op_ADD] = "add";
  MapOpToStr[IROpKind::Op_SUB] = "sub";
  MapOpToStr[IROpKind::Op_MUL] = "mul";
  MapOpToStr[IROpKind::Op_DIV] = "sdiv";
  MapOpToStr[IROpKind::Op_SLE] = "icmp sle";
  MapOpToStr[IROpKind::Op_SLT] = "icmp slt";
  MapOpToStr[IROpKind::Op_SGE] = "icmp sge";
  MapOpToStr[IROpKind::Op_SGT] = "icmp sgt";
  MapOpToStr[IROpKind::Op_EQ] = "icmp eq";
  MapOpToStr[IROpKind::Op_NE] = "icmp ne";
  s += "  " + this->getName() + " = ";
  s += MapOpToStr[Op] + " ";
  assert(Left.getValPtr());
  std::cout << "op is : " << MapOpToStr[Op] << "   "
            << " for : " << this->getName() << "\n";
  assert(Left.getValPtr()->getType());
  s += Left.getValPtr()->getType()->CodeGen();
  s += " ";
  s += Left.getValPtr()->getName();
  s += ", ";
  s += Right.getValPtr()->getName();
  return s;
}

std::string StoreInst::CodeGen() {
  if (Source.getValPtr() == nullptr)
    return "  store i32 " + std::to_string(this->Ival) + ", i32* " +
           Dest.getValPtr()->getName() + ", align " +
           std::to_string(Dest.getValPtr()->Align); // + "\n";
  else
    return "  store i32 " + Source.getValPtr()->getName() + ", i32* " +
           Dest.getValPtr()->getName() + ", align " +
           std::to_string(Dest.getValPtr()->Align); // + "\n";
}

std::string LoadInst::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    assert(this->getParent());
    assert(this->getParent()->getParent());
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  return "  " + this->getName() + " = load i32, " + "i32* " +
         Source.getValPtr()->getName() + ", align " +
         std::to_string(Source.getValPtr()->Align); // + "\n";
}

std::string AllocaInst::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  if (ArraySize.getValPtr()) {
    return "  " + this->getName() + " = " + "alloca [" +
           std::to_string(ArraySize.getValPtr()->Ival) + " x i32] " +
           ", align " + std::to_string(this->Align); // + "\n";
  } else if (VTy->Kind == TypeKind::TY_STRUCT) {
    return "  " + this->getName() + " = " + "alloca " +
           ProgramModule->GetType(VTy)->getName() + ", align " +
           std::to_string(this->Align);
  } else {
    return "  " + this->getName() + " = " + "alloca i32 " + ", align " +
           std::to_string(this->Align); // + "\n";
  }
}

std::string CallInst::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  std::string Res;
  // if (func->RetTy == ReturnTypePtr::RTY_VOID) {
  //   Res += "  call void @";
  // } else {
  Res += "  " + this->getName() + " =";
  Res += "  call i32 @";
  // }
  Res += func->getName() + "("; // + "\n";
  for (int i = 0; i < args.size(); ++i) {
    Res += "i32 ";
    Res += args[i].getValPtr()->getName();
    if (i != args.size() - 1) {
      Res += ", ";
    }
  }
  Res += ")";
  return Res;
}

std::string PHINode::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  std::string s = "  " + this->getName() + " = phi ";
  s += ValueTy->CodeGen();
  for (int i = 0; i < CurNumOfIncomingValues; ++i) {
    s += "[ ";
    s += InComingValues[i].getValPtr()->getName() + ", ";
    if (InComingBlocks[i]->getName() == "entry") {
      s += "%entry ]";
    } else {
      s += InComingBlocks[i]->getName() + " ]";
    }

    if (i != CurNumOfIncomingValues - 1) {
      s += ", ";
    }
  }
  return s;
}

std::string BranchInst::CodeGen() {
  if (Op == IROpKind::Op_Branch) {
    std::string s;
    s += "  br i1 " + indicateVariable.getValPtr()->getName();
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
  if (returnValue.getValPtr() != nullptr)
    return "  ret i32 " + returnValue.getValPtr()->getName(); // + "\n";
  else
    return "  ret void\n";
}

std::string GetElementPtrInst::CodeGen() {
  if (this->getName()[1] >= '0' && this->getName()[1] <= '9') {
    this->setName(
        "%" +
        std::to_string(this->getParent()->getParent()->getNextSlotIndex()));
  }
  // %arrayidx? = getelementptr inbounds [2 x i32], [2 x i32]* %a, i64 0, i64 0
  std::string s = "  " + this->getName() + " = getelementptr inbounds ";
  // assert(baseTo(PtrTy)->Kind == TypeKind::TY_STRUCT);
  if (baseTo(PtrTy)->Kind == TypeKind::TY_ARRAY) {
    s += baseTo(PtrTy)->CodeGen() + ", ";
    s += BasePtr.getValPtr()->CodeGen();
  } else if (baseTo(PtrTy)->Kind == TypeKind::TY_STRUCT) {
    s += ProgramModule->GetType(baseTo(PtrTy))->getName() + ", ";
    // assert(false);
    s += ProgramModule->GetType(baseTo(PtrTy))->getName() + "* " +
         BasePtr.getValPtr()->getName();
  }

  for (auto &Idx : IdxList) {
    s += ", ";
    s += Idx.getValPtr()->CodeGen();
  }
  return s;
}