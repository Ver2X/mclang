#pragma once
#include <memory>

typedef struct Type Type;
using TypePtr = std::shared_ptr<Type>;
struct Member;
typedef struct Token Token;
using TokenPtr = std::shared_ptr<Token>;
extern TypePtr TyChar;
extern TypePtr TyShort;
extern TypePtr TyInt;
extern TypePtr TyLong;
extern TypePtr TyVoid;
extern TypePtr TyDouble;
extern TypePtr TyBit;

enum class TypeKind {
  TY_INT,
  TY_SHORT,
  TY_LONG,
  TY_DOUBLE,
  TY_PTR,
  TY_FUNC,
  TY_ARRAY,
  TY_CHAR,
  TY_BIT,
  TY_STRUCT,
  TY_UNION,
  TY_VOID,
};

class Type {
public:
  Type(TypeKind _kind, int _size, int _align)
      : Kind(_kind), Size(_size), Align(_align) {}
  Type() = default;

  std::string CodeGen();
  TypeKind Kind;

  int Size; // sizeof() value

  int Align;

  // Pointer-to or array-of type. We intentionally use the same member
  // to represent pointer/array duality in C.
  //
  // In many contexts in which a pointer is expected, we examine this
  // member instead of "Kind" member to determine whether a type is a
  // pointer or not. That means in many contexts "array of T" is
  // naturally handled as if it were "pointer to T", as required by
  // the C spec.
  TypePtr Base;

  // declaration
  TokenPtr Name;

  // array
  int ArrayLen;

  // struct
  Member *Members;

  // Function type
  TypePtr ReturnTy;
  TypePtr Params;
  TypePtr Next;
};

TypePtr newType(TypeKind Kind, int Size, int Align);
bool isInteger(TypePtr Ty);
TypePtr pointerTo(TypePtr Base);
TypePtr baseTo(TypePtr Ptr);
TypePtr funcType(TypePtr ReturnTy);
TypePtr copyType(TypePtr Ty);
TypePtr arrayOf(TypePtr Base, int len);