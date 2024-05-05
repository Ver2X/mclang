#include "Variable.h"
#include <string>

Value::Value() {
  Ival = INT_MIN;
  Next = nullptr;
  Align = 4;
  isConst = false;
  isGlobal = false;
  varIsArg = false;
  VarType = TyInt;
}
Value::Value(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}

Value::Value(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}
Value::Value(int *v) {
  assert(v == nullptr);
  Pval = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = "nullptr";
  VarType = TyInt;
}

Value::Value(double v) {
  Fval = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Fval);
  VarType = TyDouble;
}
void Value::setName(std::string InitName) { Name = InitName; }
bool Value::isArg() { return varIsArg; }

void Value::SetAddr(VariablePtr _Addr) { Addr = _Addr; }

VariablePtr Value::getAddr() { return Addr; }

void Value::SetConst(double v) {
  Fval = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Fval);
  VarType = TyDouble;
}

void Value::SetConst(int v) {
  Ival = v;
  Next = nullptr;
  Align = 4;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyInt;
}

void Value::SetConst(int64_t v) {
  Ival = v;
  Next = nullptr;
  Align = 8;
  isConst = true;
  isGlobal = false;
  varIsArg = false;
  Name = std::to_string(Ival);
  VarType = TyLong;
}

void Value::setGlobal() { isGlobal = true; }

void Value::SetArg() { varIsArg = true; }

void Use::set(ValuePtr V) {
  if (Val)
    removeFromList();
  Val = V;
  if (V)
    V->addUse(this);
}

std::string Value::CodeGen() {
  std::string s;
  if (isGlobal) {
    if (baseTo(VarType) == TyChar) {
      s += this->Name + " = dso_local global " + "i8 " + std::to_string(Ival) +
           ", align 4\n";
    } else if (baseTo(VarType) == TyShort) {
      s += this->Name + " = dso_local global " + "i16 " + std::to_string(Ival) +
           ", align 4\n";
    } else if (baseTo(VarType) == TyInt) {
      s += this->Name + " = dso_local global " + "i32 " + std::to_string(Ival) +
           ", align 4\n";
    } else if (baseTo(VarType) == TyLong) {
      s += this->Name + " = dso_local global " + "i64 " + std::to_string(Ival) +
           ", align 4\n";
    } else if (baseTo(VarType) == TyDouble) {
      s += this->Name + " = dso_local global " + "f64 " + std::to_string(Fval) +
           ", align 4\n";
    } else {
      assert(false);
    }
    return s;
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