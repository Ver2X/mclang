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
  bool Insert(VariablePtr left, VariablePtr right, VariablePtr result,
              IROpKind Op, int label, std::string name, SymbolTablePtr table);
  bool Insert(VariablePtr left, VariablePtr right, VariablePtr result,
              IROpKind Op, SymbolTablePtr table);
  bool Insert(VariablePtr indicateVariable, BlockPtr targetOne,
              BlockPtr targetTwo, IROpKind Op, SymbolTablePtr table);
  bool Insert(VariablePtr dest, IROpKind Op, SymbolTablePtr table);
  bool Insert(VariablePtr source, VariablePtr dest, IROpKind Op,
              SymbolTablePtr table);

public:
  IRBuilder() {
    cache_label = -1;
    entry_label = -1;
    globalVariable = nullptr;
    function = nullptr;
    count_suffix = 1;
  }
  IRBuilder(IRFunctionPtr func) {
    cache_label = -1;
    entry_label = -1;
    globalVariable = nullptr;
    function = func;
    count_suffix = 1;
  }
  VariablePtr lastResVar;
  int GetNextCountSuffix() { return count_suffix++; }
  void SetInsertPoint(int label, std::string name);
  void SetInsertPoint(BlockPtr insertPoint);
  void SetFunc(IRFunctionPtr func) { function = func; }
  std::string CodeGen();
  void CreateAlloca(VariablePtr addr);
  void CreateStore(VariablePtr value, VariablePtr addr);
  void CreateStore(VariablePtr addr);
  VariablePtr CreateCall(IRFunctionPtr func, std::vector<VariablePtr> args);
  VariablePtr CreateBinary(VariablePtr left, VariablePtr right, IROpKind Op);
  VariablePtr CreateLoad(VariablePtr addr);
  int NextBlockLabelNum();
  int NextControlFlowNum();
  void CreateRet(VariablePtr value);
  void CreateCondBr(VariablePtr cond, BlockPtr True, BlockPtr False);
  void CreateBr(BlockPtr Target);
  void FixNonReturn(SymbolTablePtr table);
  void InsertBasicBlock(int label, std::string name, int pred);
  std::string GetNextVarName();
  BlockPtr GetCurrentBlock();
  void DepthFirstSearchPP(int &, int &, std::map<BlockPtr, bool> &,
                          std::vector<BlockPtr> &, BlockPtr);
  void SetPredAndSuccNum();
  std::string DumpCFG();
};
