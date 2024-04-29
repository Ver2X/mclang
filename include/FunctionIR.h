#pragma once
#include "Variable.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// do include "IRBuilder.h" incase cycle include
class SymbolTable;
class BasicBlock;
using SymbolTablePtr = std::shared_ptr<SymbolTable>;
using BasicBlockPtr = std::shared_ptr<BasicBlock>;
using Edge = std::pair<BasicBlockPtr, BasicBlockPtr>;
enum class EdgeKind;
class IRBuilder;
class Module;
using ModulePtr = std::shared_ptr<Module>;

class IRFunction {
  friend class IRBuilder;
  int varNameNum;
  int blockLabelNum;
  int controlFlowNum;
  std::shared_ptr<IRBuilder> Body;
  SymbolTablePtr Table;
  int CacheLabel;
  int EntryLabel;
  std::string CacheName;
  int CountSuffix;
  int numofblock;
  std::map<int, BasicBlockPtr> Blocks;
  std::map<Edge, EdgeKind> EdgeKinds;
  std::map<BasicBlockPtr, int> PreNum;
  std::map<BasicBlockPtr, int> PostNum;
  std::map<int, BasicBlockPtr> PreNumToBlock;
  std::map<int, BasicBlockPtr> PostNumToBlock;
  std::string FunctionName;
  std::vector<TypePtr> ParamTys;
  std::vector<VariablePtr> Args;
  ModulePtr ParentMod;

public:
  IRFunction();
  IRFunction(std::string Name);

  // VariablePtr args;
  int argsNum;
  VariablePtr ret;
  ReturnTypePtr RetTy;
  std::map<std::string, int> CachedVarNames;

  std::string rename();
  std::string getName() { return FunctionName; };
  std::map<int, BasicBlockPtr> getBlocks() { return Blocks; }
  void setModule(ModulePtr Mod) { ParentMod = Mod; }
  ModulePtr getParent() { return ParentMod; }
  std::string createName(std::string Name);
  void setName(std::string Name) { FunctionName = Name; };
  SymbolTablePtr GeTable() { return Table; };
  int nextVarNameNum();
  int nextBlockLabelNum();
  int nextControlFlowNum();
  void AddArgs();
  void setParamTys(std::vector<TypePtr> &Tys);
  void addParamTy(TypePtr Ty);
  void addArg(VariablePtr Arg);
  std::string CodeGen();
  void setBody(std::shared_ptr<IRBuilder> Body);
  void setTable(SymbolTablePtr local_table) { Table = local_table; };
};
using IRFunctionPtr = std::shared_ptr<IRFunction>;