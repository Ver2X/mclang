#include "FunctionIR.h"
#include "BasicBlock.h"
#include <algorithm>

IRFunction::IRFunction() {
  argsNum = 0;
  RetTy = TyVoid;
  varNameNum = 0;
  blockLabelNum = 0;
  controlFlowNum = 0;
  CacheLabel = -1;
  EntryLabel = -1;
  CountSuffix = 1;
  ParentMod = nullptr;
}

IRFunction::IRFunction(std::string Name) {
  argsNum = 0;
  RetTy = TyVoid;
  FunctionName = Name;
  varNameNum = 0;
  blockLabelNum = 0;
  controlFlowNum = 0;
  CacheLabel = -1;
  EntryLabel = -1;
  CountSuffix = 1;
  ParentMod = nullptr;
}

int IRFunction::nextVarNameNum() { return varNameNum++; }

int IRFunction::nextBlockLabelNum() { return blockLabelNum++; }

int IRFunction::nextControlFlowNum() { return controlFlowNum++; }

void IRFunction::setBody(std::shared_ptr<IRBuilder> _body) { Body = _body; }

std::string IRFunction::createName(std::string Name) {
  if (CachedVarNames.count(Name)) {
    return Name + std::to_string(CachedVarNames[Name]++);
  } else {
    CachedVarNames[Name] = 1;
    return Name;
  }
}
void IRFunction::addParamTy(TypePtr Ty) { ParamTys.push_back(Ty); }

void IRFunction::setParamTys(std::vector<TypePtr> &Tys) { ParamTys = Tys; }

void IRFunction::addArg(VariablePtr Arg) { Args.push_back(Arg); }

std::string IRFunction::CodeGen() {
  // if Body non-null
  std::string s;
  s += "define dso_local ";
  if (RetTy == TyVoid) {
    s += "void ";
  } else if (RetTy == TyInt) {
    s += "i32 ";
  } else if (RetTy == TyLong) {
    s += "i64 ";
  } else if (RetTy == TyDouble) {
    s += "f64 ";
  } else if (RetTy == TyChar) {
    s += "i8 ";
  } else if (RetTy == TyShort) {
    s += "i16 ";
  }

  s += "@";
  s += rename();
  s += "(";
  // VariablePtr head = args;
  // s += "argsNum is :";
  // s += std::to_string(argsNum);
  // s += "\n";
  for (auto Begin = Args.begin(), End = Args.end(); Begin != End; Begin++) {
    s += (*Begin)->CodeGen();
    if (Begin != End - 1) {
      s += ", ";
    }
  }

  s += ")";

  s += "{\n";

  std::vector<int> Idxs;
  for (auto [Idx, _] : Blocks) {
    Idxs.push_back(Idx);
  }
  sort(Idxs.begin(), Idxs.end());
  BasicBlockPtr lastBlock;
  for (auto Idx : Idxs) {
    auto blk = Blocks[Idx];
#if DEBUG
    FileOut << "dump InstInBB in BasicBlock , Name: " << blk->getName()
            << "Label: " << blk->GetLabel() << " Size :" << blk->InstInBB.Size()
            << std::endl;
#endif
    if (Idx == this->EntryLabel && !blk->getAllocas().empty())
      s += blk->AllocaCodeGen();
    s += blk->CodeGen();
    lastBlock = blk;
  }
  s += "}\n";
  return s;
}

std::string IRFunction::rename() {
  // // _ + return type + Name + arg
  // std::string s = "_";
  // if (RetTy == ReturnTypePtr::RTY_INT)
  //   s += "Z";
  // s += FunctionName;

  // for (auto arg_iter : args) {
  //   if (arg_iter->type == VariablePtr::VAR_32)
  //     s += "i";
  // }
  return FunctionName;
}
void IRFunction::AddArgs() {
  // when enter function, need push varibale into symbol Table
  // but when leave, destory it
}
