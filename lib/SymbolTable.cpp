#include "SymbolTable.h"

bool SymbolTable::nameHaveUsed(std::string Name) {
  for (auto &[_, Var] : Table) {
    if (Var->getName() == Name) {
      return true;
    }
  }
  return false;
}
bool SymbolTable::insertFunc(IRFunctionPtr func) {
  for (auto f : funcs) {
    if (f->getName() == func->getName()) {
      return false;
    }
  }
  funcs.push_back(func);
  return true;
}

IRFunctionPtr SymbolTable::findFunc(std::string funcname) {
  for (auto func : funcs) {
    if (func->getName() == funcname) {
      return func;
    }
  }
  return nullptr;
}

bool SymbolTable::insert(Obj *node, VariablePtr Var) {
  if (!Table.count(node)) {
    Table[node] = Var;
    return true;
  } else {
    return false;
  }
}

bool SymbolTable::update(Obj *node, VariablePtr Var, VariablePtr newVar) {
  if (!Table.count(node)) {
    return false;
  } else {
    if (Table[node] != Var) {
      assert(false);
      return false;
    }
    Table[node] = newVar;
    return true;
  }
}

bool SymbolTable::erase(Obj *node, VariablePtr Var) {
  if (Table[node] != Var) {
    assert(false);
    return false;
  }
  Table.erase(node);
  return true;
}

VariablePtr SymbolTable::findVar(Obj *node) {
  if (Table.count(node)) {
    return Table[node];
  } else {
    return nullptr;
  }
}