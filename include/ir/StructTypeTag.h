#pragma once
#include "front/Type.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <list>
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

class StructTypeTag {
private:
  std::string Name;

public:
  TypePtr StructType;
  StructTypeTag(TypePtr Ty, std::string N) : StructType(Ty), Name(N) {}
  std::string getName() { return Name; };
  std::string CodeGen();
};

using StructTypeTagPtr = std::shared_ptr<StructTypeTag>;