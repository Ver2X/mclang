#pragma once
#include "ir/BasicBlock.h"
#include "ir/FunctionIR.h"
#include "ir/SymbolTable.h"
#include "front/Type.h"
#include "ir/Value.h"

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
class IRBuilder : std::enable_shared_from_this<IRBuilder> {

  VariablePtr globalVariable;
  IRFunctionPtr function;
  bool InOrderInsert;
  // order by Label
  std::map<int, BasicBlockPtr> &Blocks;
  std::map<Edge, EdgeKind> &EdgeKinds;
  std::map<BasicBlockPtr, int> &PreNum;
  std::map<BasicBlockPtr, int> &PostNum;
  std::map<int, BasicBlockPtr> &PreNumToBlock;
  std::map<int, BasicBlockPtr> &PostNumToBlock;
  void InsertIntoCacheBB(InstructionPtr Inst);

public:
  IRBuilder(IRFunctionPtr func)
      : function(func), globalVariable(nullptr), Blocks(func->Blocks),
        EdgeKinds(func->EdgeKinds), PreNum(func->PreNum),
        PostNum(func->PostNum), PreNumToBlock(func->PreNumToBlock),
        PostNumToBlock(func->PostNumToBlock), InOrderInsert(true) {}
  std::list<InstructionPtr>::iterator CurrentInsertBefore;
  bool isOrdered() { return InOrderInsert; }
  int GetNextCountSuffix() { return function->CountSuffix++; }
  void SetInsertPoint(int Label, std::string Name);
  void SetInsertPoint(BasicBlockPtr insertPoint);
  void SetInsertPoint(InstructionPtr InsertBefore);
  void SetFunc(IRFunctionPtr func) { function = func; }
  void CreateStore(VariablePtr value, VariablePtr addr);
  CallInstPtr CreateCall(IRFunctionPtr func, std::vector<VariablePtr> args);
  BinaryOperatorPtr CreateBinary(VariablePtr Left, VariablePtr Right,
                                 IROpKind Op);
  LoadInstPtr CreateLoad(VariablePtr addr);
  AllocaInstPtr CreateAlloca(VarTypePtr VTy, VariablePtr ArraySize,
                             std::string Name);
  GetElementPtrInstPtr CreateGEP(VarTypePtr VTy, VariablePtr Ptr,
                                 std::vector<VariablePtr> IdxList,
                                 std::string Name = "");
  PHINodePtr CreatePHI(VarTypePtr VTy, unsigned NumReservedValues,
                       const std::string &Name = "");
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
