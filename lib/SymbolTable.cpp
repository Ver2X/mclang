#include "SymbolTable.h"

bool SymbolTable::nameHaveUsed(std::string name) {
  for (auto &[_, Var] : table) {
    if (Var->GetName() == name) {
      return true;
    }
  }
  return false;
}
bool SymbolTable::insertFunc(IRFunctionPtr func) {
  for (auto f : funcs) {
    if (f->GetName() == func->GetName()) {
      return false;
    }
  }
  funcs.push_back(func);
  return true;
}

IRFunctionPtr SymbolTable::findFunc(std::string funcname) {
  for (auto func : funcs) {
    if (func->GetName() == funcname) {
      return func;
    }
  }
  return nullptr;
}

bool SymbolTable::insert(Obj *node, VariablePtr var) {
  if (!table.count(node)) {
    table[node] = var;
    return true;
  } else {
    return false;
  }
}

bool SymbolTable::update(Obj *node, VariablePtr var, VariablePtr newVar) {
  if (!table.count(node)) {
    return false;
  } else {
    if (table[node] != var) {
      assert(false);
      return false;
    }
    table[node] = newVar;
    return true;
  }
}

bool SymbolTable::erase(Obj *node, VariablePtr var) {
  if (table[node] != var) {
    assert(false);
    return false;
  }
  table.erase(node);
  return true;
}

VariablePtr SymbolTable::findVar(Obj *node) {
  if (table.count(node)) {
    return table[node];
  } else {
    return nullptr;
  }
}