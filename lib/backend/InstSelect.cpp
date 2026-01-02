#include "backend/InstSelect.h"
#include "mir/MachineIR.h"

void codeGenMir(ModulePtr ModPtr) {
  auto &M = *ModPtr;
  for (auto Func : M.getFunctions()) {
    codeGenMirFunc(Func);
  }
}