#pragma once
#include "BasicBlock.h"
#include "DominTree.h"
#include "FunctionIR.h"
#include "SymbolTable.h"
#include "Type.h"
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
  std::map<int, BasicBlockPtr> &Blocks;
  std::map<Edge, EdgeKind> &EdgeKinds;
  std::map<BasicBlockPtr, int> &PreNum;
  std::map<BasicBlockPtr, int> &PostNum;
  std::map<int, BasicBlockPtr> &PreNumToBlock;
  std::map<int, BasicBlockPtr> &PostNumToBlock;
  std::shared_ptr<DominTree> Domin;
  std::shared_ptr<DominTree> PostDomin;
  bool Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
              IROpKind Op, int Label, std::string Name, SymbolTablePtr Table);
  bool Insert(VariablePtr Left, VariablePtr Right, VariablePtr Result,
              IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr indicateVariable, BasicBlockPtr targetOne,
              BasicBlockPtr targetTwo, IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr Dest, IROpKind Op, SymbolTablePtr Table);
  bool Insert(VariablePtr Source, VariablePtr Dest, IROpKind Op,
              SymbolTablePtr Table);

public:
  IRBuilder(IRFunctionPtr func)
      : function(func), globalVariable(nullptr), Blocks(func->Blocks),
        EdgeKinds(func->EdgeKinds), PreNum(func->PreNum),
        PostNum(func->PostNum), PreNumToBlock(func->PreNumToBlock),
        PostNumToBlock(func->PostNumToBlock) {}
  VariablePtr lastResVar;
  int GetNextCountSuffix() { return function->CountSuffix++; }
  void SetInsertPoint(int Label, std::string Name);
  void SetInsertPoint(BasicBlockPtr insertPoint);
  void SetFunc(IRFunctionPtr func) { function = func; }
  std::string CodeGen();
  // void CreateAlloca(VariablePtr addr);
  // VariablePtr CreateAlloca(VariablePtr VTy, VariablePtr ArraySize,
  // std::string Name);
  void CreateStore(VariablePtr value, VariablePtr addr);
  void CreateStore(VariablePtr addr);
  VariablePtr CreateCall(IRFunctionPtr func, std::vector<VariablePtr> args);
  VariablePtr CreateBinary(VariablePtr Left, VariablePtr Right, IROpKind Op);
  VariablePtr CreateLoad(VariablePtr addr);
  VariablePtr CreateAlloca(VarTypePtr VTy, VariablePtr ArraySize,
                           std::string Name);
  VariablePtr CreateGEP(VarTypePtr VTy, VariablePtr Ptr,
                        std::vector<VariablePtr> IdxList,
                        std::string Name = "");
  int nextBlockLabelNum();
  int nextControlFlowNum();
  void CreateRet(VariablePtr value);
  void CreateCondBr(VariablePtr cond, BasicBlockPtr True, BasicBlockPtr False);
  void CreateBr(BasicBlockPtr Target);
  void fixNonReturn(SymbolTablePtr Table);
  void InsertBasicBlock(int Label, std::string Name, int Pred);
  std::string getNextVarName();
  IRFunctionPtr getParent() { return function; };
  BasicBlockPtr getCurrentBlock();
  void DepthFirstSearchPP(int &, int &, std::map<BasicBlockPtr, bool> &,
                          std::list<BasicBlockPtr> &, BasicBlockPtr);
  void SetPredAndSuccNum();
  std::string DumpCFG();
};
