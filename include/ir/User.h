#pragma once
#include "ir/Use.h"
#include "ir/Value.h"
#include <memory>
class Value;
using ValuePtr = std::shared_ptr<Value>;
class User : public Value {
public:
  virtual Use &getOperand(int Idx) = 0;
  void setOperand(int Idx, ValuePtr NewOp) { getOperand(Idx) = NewOp; };
};
using UserPtr = std::shared_ptr<User>;