#pragma once
#include "Block.h"
#include "DominTree.h"
#include "FunctionIR.h"
#include "SymbolTable.h"
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
class IRBuilder {

  VariablePtr globalVariable;
  IRFunctionPtr function;
  // order by label
  std::map<int, BlockPtr> blocks;
  std::map<Edge, EdgeKind> EdgeKinds;
  std::map<BlockPtr, int> PreNum;
  std::map<BlockPtr, int> PostNum;
  std::map<int, BlockPtr> PreNumToBlock;
  std::map<int, BlockPtr> PostNumToBlock;
  std::shared_ptr<DominTree> Domin;
  std::shared_ptr<DominTree> PostDomin;
  int cache_label;
  int entry_label;
  std::string cache_name;
  int count_suffix;
  int numofblock;

public:
  IRBuilder() {
    cache_label = -1;
    entry_label = -1;
    globalVariable = nullptr;
    function = nullptr;
    count_suffix = 1;
  }
  VariablePtr lastResVar;
  int GetNextCountSuffix() { return count_suffix++; }
  void SetInsertPoint(int label, std::string name);
  void SetInsertPoint(BlockPtr insertPoint);
  void SetFunc(IRFunctionPtr func) { function = func; }
  std::string CodeGen();
  // using label to index Blocks
  bool Insert(VariablePtr left, VariablePtr right, VariablePtr result,
              IROpKind Op, int label, std::string name, SymbolTablePtr table);
  bool Insert(VariablePtr left, VariablePtr right, VariablePtr result,
              IROpKind Op, SymbolTablePtr table);
  bool CreateAlloca(VariablePtr addr, SymbolTablePtr table);
  bool CreateLoad(VariablePtr addr, SymbolTablePtr table);
  bool CreateStore(VariablePtr value, VariablePtr addr, SymbolTablePtr table);
  bool Insert(VariablePtr indicateVariable, BlockPtr targetOne,
              BlockPtr targetTwo, IROpKind Op, SymbolTablePtr table);
  bool Insert(VariablePtr dest, IROpKind Op, SymbolTablePtr table);
  bool Insert(VariablePtr source, VariablePtr dest, IROpKind Op,
              SymbolTablePtr table);
  void FixNonReturn(SymbolTablePtr table);
  void InsertBasicBlock(int label, std::string name, int pred);
  BlockPtr GetCurrentBlock();
  void DepthFirstSearchPP(int &, int &, std::map<BlockPtr, bool> &,
                          std::vector<BlockPtr> &, BlockPtr);
  void SetPredAndSuccNum();
  std::string DumpCFG();
};
