#pragma once
#include "BasicBlock.h"
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
  // order by Label
  std::map<int, BasicBlockPtr> Blocks;
  std::map<Edge, EdgeKind> EdgeKinds;
  std::map<BasicBlockPtr, int> PreNum;
  std::map<BasicBlockPtr, int> PostNum;
  std::map<int, BasicBlockPtr> PreNumToBlock;
  std::map<int, BasicBlockPtr> PostNumToBlock;
  std::shared_ptr<DominTree> Domin;
  std::shared_ptr<DominTree> PostDomin;
  int CacheLabel;
  int EntryLabel;
  std::string CacheName;
  int count_suffix;
  int numofblock;
  bool Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
              IROpKind Op, int Label, std::string Name, SymbolTablePtr Table);
  bool Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
              IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
              BasicBlockPtr targetTwo, IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr dest, IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr source, VariablePtr dest, IROpKind Op,
              SymbolTablePtr Table);

public:
  IRBuilder() {
    CacheLabel = -1;
    EntryLabel = -1;
    globalVariable = nullptr;
    function = nullptr;
    count_suffix = 1;
  }
  IRBuilder(IRFunctionPtr func) {
    CacheLabel = -1;
    EntryLabel = -1;
    globalVariable = nullptr;
    function = func;
    count_suffix = 1;
  }
  VariablePtr lastResVar;
  int GetNextCountSuffix() { return count_suffix++; }
  void SetInsertPoint(int Label, std::string Name);
  void SetInsertPoint(BasicBlockPtr insertPoint);
  void SetFunc(IRFunctionPtr func) { function = func; }
  std::string CodeGen();
  void CreateAlloca(VariablePtr addr);
  void CreateStore(VariablePtr value, VariablePtr addr);
  void CreateStore(VariablePtr addr);
  VariablePtr CreateCall(IRFunctionPtr func, std::vector<VariablePtr> args);
  VariablePtr CreateBinary(VariablePtr Left, VariablePtr Right, IROpKind Op);
  VariablePtr CreateLoad(VariablePtr addr);
  int nextBlockLabelNum();
  int nextControlFlowNum();
  void CreateRet(VariablePtr value);
  void CreateCondBr(VariablePtr cond, BasicBlockPtr True, BasicBlockPtr False);
  void CreateBr(BasicBlockPtr Target);
  void fixNonReturn(SymbolTablePtr Table);
  void InsertBasicBlock(int Label, std::string Name, int Pred);
  std::string getNextVarName();
  BasicBlockPtr getCurrentBlock();
  void DepthFirstSearchPP(int &, int &, std::map<BasicBlockPtr, bool> &,
                          std::vector<BasicBlockPtr> &, BasicBlockPtr);
  void SetPredAndSuccNum();
  std::string DumpCFG();
};
