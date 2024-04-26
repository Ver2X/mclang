#include "Variable.h"
#include <string>
Operand::Operand() {
  Ival = INT_MIN;
  Next = nullptr;
  Align = 4;
  isConst = false;
  isGlobal = false;
  varIsArg = false;
  type = VaribleKind::VAR_32;
}
Operand::Operand(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  type = VaribleKind::VAR_64;
}

Operand::Operand(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  type = VaribleKind::VAR_32;
}

Operand::Operand(double v) {
  Fval = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Fval);
  type = VaribleKind::VAR_64;
}

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
  type = VaribleKind::VAR_64;
}

void Operand::SetConst(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  type = VaribleKind::VAR_32;
}

void Operand::SetConst(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  type = VaribleKind::VAR_64;
}

void Operand::setGlobal() { isGlobal = true; }

void Operand::SetArg() { varIsArg = true; }

std::string Operand::CodeGen() {
  std::string s;
  if (isGlobal) {
    switch (type) {

    case VaribleKind::VAR_8:
      s += "@" + this->Name + " = dso_local global " + "i8 " +
           std::to_string(Ival) + ", Align 4\n";
      break;
    case VaribleKind::VAR_16:
      s += "@" + this->Name + " = dso_local global " + "i16 " +
           std::to_string(Ival) + ", Align 4\n";
      break;
    case VaribleKind::VAR_32:
      s += "@" + this->Name + " = dso_local global " + "i32 " +
           std::to_string(Ival) + ", Align 4\n";
      break;
    case VaribleKind::VAR_64:
      s += "@" + this->Name + " = dso_local global " + "i64 " +
           std::to_string(Ival) + ", Align 4\n";
      break;
    case VaribleKind::VAR_PRT:
      s += "@" + this->Name + " = dso_local global " + "i32* " +
           std::to_string(Ival) + ", Align 4\n";
      break;
    default:
      break;
    }
    return s;
  } else {
    switch (type) {
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
  }

  // s += "%";
  s += Name;
  return s;
}