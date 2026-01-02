#pragma once
#include <memory>
#include <string>
#include <variant>

enum class MachineOpKind {
  M_Add,
  M_Sub,
  M_Div,
  M_Mul,
  M_Call, // call inst use val are in hint register
  M_Load,
  M_Store,
  M_Copy,
  M_Mov,
};

// consider a load + gep
// will become (arithm) * 4 => addr
// load addr
// or
// loadr base, offset

// for $1 = $0 + 3
//     $2 = $1 + 4
//
// after RA coalase
//
// $0 = $0 + 3
// $0 = $0 + 4

// class DAGNode;
// using DAGNodePtr = std::shared_ptr<DAGNode>;
// class DAGNode {
//     MachineOpKind Op;
//     DAGNodePtr Res;
//     DAGNodePtr Left;
//     DAGNodePtr Right;
// };

class MachineInstruction {
  MachineOpKind Op;
};

class VirtualRegister {
  int VirtualRegiterNumber;
  VirtualRegister() = default;

public:
  int getVReg() { return VirtualRegiterNumber; }
  std::string getName() { return "$" + std::to_string(VirtualRegiterNumber); }
};

// union MachineValue {
//     VirtualRegister VReg;
//     int ConstVal;
// };
// using MachineValue = std::variant<VirtualRegister, int>;
class MachineValue {
public:
  MachineValue(int ConstInt) : Data(ConstInt) {}
  bool isConst() { return Data.index() == 1; }

private:
  std::variant<VirtualRegister, int> Data;
};

class MachineLoadInst : public MachineInstruction {
  MachineValue BaseAddr;
  MachineValue Offset;
  MachineValue Dest;
};

class MachineStoreInst : public MachineInstruction {
  MachineValue Value;
  MachineValue BaseAddr;
  MachineValue Offset;
};

class MachineBinaryInst : public MachineInstruction {
  MachineValue Left;
  MachineValue Rigth;
  MachineValue Dest;
};
