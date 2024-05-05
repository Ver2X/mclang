#pragma once
#include "Use.h"
#include "Variable.h"
#include <memory>
class Value;
using ValuePtr = std::shared_ptr<Value>;
class User : public Value {
public:
  virtual Use &getValue(int Idx) = 0;
  void setValue(int Idx, ValuePtr NewOp) { getValue(Idx) = NewOp; };
};
using UserPtr = std::shared_ptr<User>;