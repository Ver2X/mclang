#include "FunctionIR.h"

IRFunction::IRFunction() {
  argsNum = 0;
  retTy = ReturnTypeKind::RTY_VOID;
  varNameNum = 0;
  blockLabelNum = 0;
  controlFlowNum = 0;
}

IRFunction::IRFunction(std::string Name) {
  argsNum = 0;
  retTy = ReturnTypeKind::RTY_VOID;
  FunctionName = Name;
  varNameNum = 0;
  blockLabelNum = 0;
  controlFlowNum = 0;
}

int IRFunction::nextVarNameNum() { return varNameNum++; }

int IRFunction::nextBlockLabelNum() { return blockLabelNum++; }

int IRFunction::nextControlFlowNum() { return controlFlowNum++; }

void IRFunction::setBody(std::shared_ptr<IRBuilder> _body) { Body = _body; }

std::string IRFunction::CodeGen() {
  // if Body non-null
  std::string s;
  s += "define dso_local ";
  switch (retTy) {
  case ReturnTypeKind::RTY_VOID:
    s += "void ";
    break;
  case ReturnTypeKind::RTY_INT:
    s += "i32 ";
    break;
  case ReturnTypeKind::RTY_CHAR:
    s += "signext i8 ";
    break;
  case ReturnTypeKind::RTY_PTR:
    // dump de type
    s += "i32";
    s += "* ";
    break;
  default:
    s += "void ";
    break;
  }
  s += "@";
  s += rename();
  s += "(";
  // VariablePtr head = args;
  // s += "argsNum is :";
  // s += std::to_string(argsNum);
  // s += "\n";
  for (auto Begin = args.begin(), End = args.end(); Begin != End; Begin++) {
    s += (*Begin)->CodeGen();
    if (Begin != End - 1) {
      s += ", ";
    }
  }
  /*for(int i = 0; i < argsNum; i++)
  {
          assert(head != nullptr);
          s += head->CodeGen();
          if(i != argsNum - 1)
          {
                  s += ", ";
          }
          head = head->Next;
  }*/
  s += ")";
  return s;
}

std::string IRFunction::rename() {
  // // _ + return type + Name + arg
  // std::string s = "_";
  // if (retTy == ReturnTypeKind::RTY_INT)
  //   s += "Z";
  // s += FunctionName;

  // for (auto arg_iter : args) {
  //   if (arg_iter->type == VaribleKind::VAR_32)
  //     s += "i";
  // }
  return FunctionName;
}
void IRFunction::AddArgs() {
  // when enter function, need push varibale into symbol Table
  // but when leave, destory it
}
