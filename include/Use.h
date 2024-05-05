#pragma once
#include <iostream>
#include <list>
#include <memory>

class Value;
class User;
class Instruction;

using ValuePtr = std::shared_ptr<Value>;

using UserPtr = std::shared_ptr<User>;
class Use {
public:
  friend class Value;
  friend class User;
  ValuePtr getValPtr() const { return Val; }
  bool isValid(ValuePtr V) { return V.get(); }

  void set(ValuePtr V);
  // Use(const Use &U) = delete;
  unsigned getIndex() { return Idx; };
  Instruction *getUser() { return Parent; }

  Use(Instruction *Parent, unsigned Idx) : Parent(Parent), Idx(Idx) {}
  ValuePtr operator=(ValuePtr RHS) {
    std::cout << "call set !!\n";
    set(RHS);
    return RHS;
  }

private:
  unsigned Idx;
  ValuePtr Val;
  Instruction *Parent;
  std::list<Use *> *CacheList;

  void addToList(std::list<Use *> &List) {
    CacheList = &List;
    List.push_back(this);
  }

  void removeFromList() { CacheList->remove(this); }
};
