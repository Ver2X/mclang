#include "ir/StructTypeTag.h"

std::string StructTypeTag::CodeGen() {
  std::string Gen = Name + " = type {";
  std::vector<TypePtr> ElemTys;
  for (auto Mem = StructType->Members; Mem; Mem = Mem->Next) {
    ElemTys.push_back(Mem->Ty);
  }
  for (auto Bg = ElemTys.begin(), Ed = ElemTys.end(); Bg != Ed; ++Bg) {
    Gen += " " + (*Bg)->CodeGen();
    if (Bg != Ed - 1) {
      Gen += ", ";
    }
  }
  Gen += " }";
  return Gen;
}