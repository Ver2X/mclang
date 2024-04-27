#include "Variable.h"
#include <string>

Operand::Operand() {
  Ival = INT_MIN;
  Next = nullptr;
  Align = 4;
  isConst = false;
  isGlobal = false;
  varIsArg = false;
  VarType = TyInt;
}
Operand::Operand(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}

Operand::Operand(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}

Operand::Operand(double v) {
  Fval = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Fval);
  VarType = TyDouble;
}
void Operand::setName(std::string InitName) { Name = InitName; }
bool Operand::isArg() { return varIsArg; }

void Operand::SetAddr(VariablePtr _Addr) { Addr = _Addr; }

VariablePtr Operand::getAddr() { return Addr; }

void Operand::SetConst(double v) {
  Fval = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Fval);
  VarType = TyDouble;
}

void Operand::SetConst(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}

void Operand::SetConst(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyLong;
}

void Operand::setGlobal() { isGlobal = true; }

void Operand::SetArg() { varIsArg = true; }

std::string Operand::CodeGen() {
  std::string s;
  if (isGlobal) {
    if (VarType == TyChar) {
      s += "@" + this->Name + " = dso_local global " + "i8 " +
           std::to_string(Ival) + ", align 4\n";
    } else if (VarType == TyShort) {
      s += "@" + this->Name + " = dso_local global " + "i16 " +
           std::to_string(Ival) + ", align 4\n";
    } else if (VarType == TyInt) {
      s += "@" + this->Name + " = dso_local global " + "i32 " +
           std::to_string(Ival) + ", align 4\n";
    } else if (VarType == TyLong) {
      s += "@" + this->Name + " = dso_local global " + "i64 " +
           std::to_string(Ival) + ", align 4\n";
    } else if (VarType == TyDouble) {
      s += "@" + this->Name + " = dso_local global " + "f64 " +
           std::to_string(Fval) + ", align 4\n";
    } else {
      assert(false);
    }
  } else {
    if (VarType == TyChar) {
      s += "i8 ";
    } else if (VarType == TyShort) {
      s += "i16 ";
    } else if (VarType == TyInt) {
      s += "i32 ";
    } else if (VarType == TyLong) {
      s += "i64 ";
    } else if (VarType == TyDouble) {
      s += "f64 ";
    } else if (VarType->Kind == TypeKind::TY_PTR) {
      s += VarType->Base->CodeGen() + "* ";
    } else {
      s += "??? ";
      assert(false);
    }
  }

  s += Name;
  return s;
}