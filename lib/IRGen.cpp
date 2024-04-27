#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "mclang.h"
// #include <bits/types/FILE.h>
#include <algorithm>
#include <cassert>
#include <memory>
void genStmtIR(Node *Node, SymbolTablePtr);
void genExprIR(Node *Node, VariablePtr *, SymbolTablePtr);
void genFuncArgIR(Obj *Var, int R, int Offset, int Sz);
extern std::fstream FileOut;
extern int Depth;

// for now, define as a fucntion, then change to rope data structure.
std::string Twine(std::string &L, std::string &R) { return L + R; }

std::string Twine(std::string L, std::string R) { return L + R; }

std::string Twine(std::string L, int R) {
  if (R == 0)
    return L;
  else
    return L + std::to_string(R);
}

std::string getPreName(std::string Name) { return Twine("%", Name); }

// VariablePtr nodeTypeToVarType(TypeKind Ty) {
//   if (Ty == TY_CHAR) {
//     return VariablePtr::VAR_8;
//   } else if (Ty == TY_SHORT) {
//     return VariablePtr::VAR_16;
//   } else if (Ty == TY_INT) {
//     return VariablePtr::VAR_32;
//   } else if (Ty == TY_LONG) {
//     return VariablePtr::VAR_64;
//   } else if (Ty == TY_VOID) {
//     return VariablePtr::VAR_Void;
//   } else if (Ty == TY_ARRAY) {
//     return VariablePtr::Var_Array;
//   } else {
//     // TODO: make assert do
//     assert(false && "not support now");
//     return VariablePtr::VAR_Undefined;
//   }
// }

std::string NodeKindStrings[] = {
    "ND_ADD",       // +
    "ND_SUB",       // -
    "ND_MUL",       // *
    "ND_DIV",       // /
    "ND_EQ",        // ==
    "ND_NE",        // !=
    "ND_LT",        // <
    "ND_LE",        // <=
    "ND_ASSIGN",    // =
    "ND_COMMA",     // ,
    "ND_NEG",       // -, unary
    "ND_ADDR",      // &, unary
    "ND_DEREF",     // *, unary
    "ND_NUM",       // integer
    "ND_VAR",       // local variable
    "ND_EXPR_STMT", // statement
    "ND_RETURN",    // "return"
    "ND_IF",        // "if"
    "ND_FOR",       // "for" or "while"
    "ND_BLOCK",
    "ND_FUNCALL",   // function call
    "ND_STMT_EXPR", // statement expression
    "ND_MEMBER"     // . (struct member access)
};
std::shared_ptr<IRBuilder> InMemoryIR;
SymbolTablePtr GlobalSymTable;
std::shared_ptr<Module> ProgramModule = std::make_shared<Module>();

std::vector<VariablePtr> ArgVariableCached;

static std::string getRealPreName(Obj *Var) {
  auto checkExistInCahced = [&](std::string Name) -> bool {
    for (auto Var : ArgVariableCached) {
      if (Var->getName() == Name) {
        return true;
      }
    }
    return false;
  };

  if (checkExistInCahced(Twine(getPreName(Var->Name), ".addr"))) {
    return Twine(getPreName(Var->Name), ".addr");
  }
  return Twine("%", Var->Name);
}
// emit IR
void emitIR(Obj *Prog, std::string FileName) {
  ProgramModule->setName(FileName);
  GlobalSymTable = std::make_shared<SymbolTable>();
  emitGlobalDataIR(Prog);
  for (Obj *FuncNode = Prog; FuncNode; FuncNode = FuncNode->Next) {
    if (!FuncNode->IsFunction || !FuncNode->IsDefinition)
      continue;
    IRFunctionPtr Func = GlobalSymTable->findFunc(FuncNode->Name);
    if (!Func) {
      Func = std::make_shared<IRFunction>();
      Func->setName(FuncNode->Name);
    }

    auto LocalTable = std::make_shared<SymbolTable>(GlobalSymTable);
    Func->setTable(LocalTable);
    InMemoryIR = std::make_shared<IRBuilder>(Func);
    InMemoryIR->SetInsertPoint(InMemoryIR->nextBlockLabelNum(), "entry");

    // switch (FuncNode->Ty->ReturnTy->Kind) {
    // case TY_INT:
    //   Func->RetTy = ReturnTypePtr::RTY_INT;
    //   break;
    // case TY_CHAR:
    //   Func->RetTy = ReturnTypePtr::RTY_CHAR;
    //   break;
    // default:
    //   Func->RetTy = ReturnTypePtr::RTY_PTR;
    //   break;
    // }
    Func->RetTy = FuncNode->Ty->ReturnTy;
    // save passed-by-register arguments to the stack
    // VariablePtr ArgVar;
    // VariablePtr ArgVarAddr;
    // arg, addr of arg
    // std::vector<std::pair<VariablePtr, VariablePtr>> ArgVarPair;

    /// Cache and release variable
    for (Obj *Var = FuncNode->Params; Var; Var = Var->Next) {

      // FileOut << "Func args increase 1" << std::endl;
      auto ArgVar = std::make_shared<Variable>();
      // std::cout << "meet type: " << Var->Ty->Kind << "\n";
      // assert(Var->Ty);
      // Var->Ty->Kind;
      // assert(Var->Ty == TyInt);
      if (Var->Ty->Kind == TypeKind::TY_INT) {
        ArgVar->VarType = TyInt;
      } else {
        assert(false);
      }
      // ArgVar->VarType = Var->Ty;
      ArgVar->setName(getPreName(Var->Name));
      ArgVar->SetArg();
      Func->addParamTy(ArgVar->VarType);
      LocalTable->insert(Var, ArgVar);
      // ArgVarAddr = std::make_shared<Variable>();
      // ArgVarAddr->setName(Twine(getPreName(Var->Name), ".addr"));
      // ArgVarPair.push_back(
      //     std::tuple<VariablePtr, VariablePtr>(ArgVar, ArgVarAddr));

      // Func->Args.push_back(ArgVar);
      Func->addArg(ArgVar);
      ArgVariableCached.push_back(ArgVar);
      (Func->argsNum)++;
      auto ArgVarAddr = InMemoryIR->CreateAlloca(
          ArgVar->VarType, nullptr, Twine(getPreName(Var->Name), ".addr"));
      ArgVar->SetAddr(ArgVarAddr);
      InMemoryIR->CreateStore(ArgVar, ArgVarAddr);
    }

    // InMemoryIR->SetFunc(Func);
    Func->setBody(InMemoryIR);
    Func->setModule(ProgramModule);

    int NumOfLocal = 0;
    for (Obj *Var = FuncNode->Locals; Var; Var = Var->Next) {
      std::string InitName = getPreName(Var->Name);
      while (LocalTable->nameHaveUsed(InitName)) {
        InitName += ".1";
      }
      auto LocalVarTy = Var->Ty;
      VariablePtr ArraySize = nullptr;
      if (LocalVarTy->Kind == TypeKind::TY_ARRAY) {
        // std::cout << "array len is: " << Var->Ty->ArrayLen << "\n";
        ArraySize = std::make_shared<Variable>(Var->Ty->ArrayLen);
      } else if (LocalVarTy->Kind == TypeKind::TY_STRUCT) {
        auto StructName = "%struct." + std::string(LocalVarTy->Tag->Name);
        std::cout << "StructName: " << StructName << "\n";
        ProgramModule->CreateType(LocalVarTy, StructName);
      }
      // std::cout << "local var " << InitName
      //           << " Ty is: " << LocalVarTy->CodeGen() << "\n";

      auto LocalVar = InMemoryIR->CreateAlloca(LocalVarTy, ArraySize, InitName);

      // std::cout << "local var " << InitName
      //           << " ptr Ty is: " << LocalVar->getType()->CodeGen() << "\n";
      // std::cout << "Table insert: " << Var << " to: " << LocalVar << "\n";
      LocalTable->insert(Var, LocalVar);

      NumOfLocal++;
    }
    // std::cout << "NumOfLocal: " << NumOfLocal << "\n";

    genStmtIR(FuncNode->Body, LocalTable);
    assert(Depth == 0);

    // fix no return statement
    InMemoryIR->fixNonReturn(LocalTable);

    if (FuncNode == Prog) {
      std::cout << ProgramModule->GlobalVariableCodeGen() << std::endl;
      FileOut << ProgramModule->GlobalVariableCodeGen() << std::endl;
    }

    // FileOut << "end global" << std::endl;
    FileOut << InMemoryIR->CodeGen() << std::endl;
    std::cout << InMemoryIR->CodeGen() << std::endl;
    InMemoryIR->SetPredAndSuccNum();
    std::fstream CfgOut;
    CfgOut.open(FileName + "_" + FuncNode->Name + ".dot", std::ios_base::out);
    CfgOut << InMemoryIR->DumpCFG() << std::endl;
    std::string GenPNGCmd = "dot -Tpng " + FileName + "_" + FuncNode->Name +
                            ".dot -o " + FileName + "_" + FuncNode->Name +
                            ".png";
    std::cout << "do cmd:\n" << GenPNGCmd << "\n";
    system(GenPNGCmd.c_str());
  }
}

// emit global data
void emitGlobalDataIR(Obj *Prog) {
  std::vector<Obj *> vars;
  for (Obj *Var = Prog; Var; Var = Var->Next) {
    if (Var->IsFunction)
      continue;
    vars.push_back(Var);
  }
  auto handleGlobalVariable = [&](Obj *Var) {
    VariablePtr GlobalValue;
    if (Var->InitData) {
      GlobalValue = std::make_shared<Variable>(Var->InitData[0]);
      GlobalValue->setGlobal();
      GlobalValue->setName("@" + std::string(Var->Name));
      ProgramModule->insertGlobalVariable(GlobalValue);
      // for(int i = 0; i < Var->Ty->Size;i++)
      //	println("  .byte %d", Var->InitData[i]);
    } else if (Var->InitScala) {
      GlobalValue = std::make_shared<Variable>(Var->InitScala->Val);
      GlobalValue->setGlobal();
      GlobalValue->setName("@" + std::string(Var->Name));
      ProgramModule->insertGlobalVariable(GlobalValue);
    } else {
      GlobalValue = std::make_shared<Variable>(0);
      GlobalValue->setGlobal();
      GlobalValue->setName("@" + std::string(Var->Name));
      ProgramModule->insertGlobalVariable(GlobalValue);
      // println("  .zero %d", Var->Ty->Size);
    }
    GlobalSymTable->insert(Var, GlobalValue);
  };
  std::for_each(vars.rbegin(), vars.rend(), handleGlobalVariable);
}