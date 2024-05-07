#pragma once
#include "ir/BasicBlock.h"
#include "ir/FunctionIR.h"
#include "analysis/utils/SetCompute.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>

std::map<int, std::set<int>> getDominanceOfFunction(IRFunctionPtr Func);
std::map<int, int> getIDomOfFunction(IRFunctionPtr Func);
std::map<int, std::set<int>> getDomFrontierOfFunction(IRFunctionPtr Func);
std::map<int, std::set<int>>
getIteratedDomFrontierOfFunction(IRFunctionPtr Func);