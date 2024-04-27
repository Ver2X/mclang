#include "analysis/Dominance.h"

void dump(std::map<int, std::set<int>> DOM, IRFunctionPtr Func,
          std::string Action) {
  auto Blocks = Func->getBlocks();
  for (auto &[Idx, BBs] : DOM) {
    std::cout << Blocks[Idx]->getName() << " " + Action + " is:\n";
    std::cout << "{";
    int n = BBs.size();
    int i = 0;
    for (auto D : BBs) {
      auto BB = Blocks[D];
      std::cout << BB->getName();
      if (i != n - 1)
        std::cout << ", ";
      i++;
    }
    std::cout << "}\n";
  }
}

std::map<int, std::set<int>> getDominanceOfFunction(IRFunctionPtr Func) {
  auto Blocks = Func->getBlocks();
  int n = Blocks.size();
  std::map<int, std::set<int>> DOM;
  std::set<int> Universe;
  std::vector<int> Idxs;
  for (auto [Idx, _] : Blocks) {
    Idxs.push_back(Idx);
    Universe.insert(Idx);
    DOM[Idx] = std::set<int>{};
  }
  auto FindIdx = [&Blocks](BasicBlockPtr BB) {
    for (auto [Idx, Cur] : Blocks) {
      if (Cur == BB) {
        return Idx;
      }
    }
    return -1;
  };
  // PDO sort
  sort(Idxs.begin(), Idxs.end());
  assert(Blocks[0]->getName() == "entry");
  DOM[0] = std::set<int>{0};
  for (int i = 1; i < n; ++i) {
    DOM[Idxs[i]] = Universe;
  }
  bool Changed = true;
  while (Changed) {
    Changed = false;
    for (int i = 1; i < n; ++i) {
      auto Idx = Idxs[i];
      auto Insect = Universe;
      for (auto Pred : Blocks[Idx]->GetPred()) {
        Insect = Intersection(Insect, DOM.at(FindIdx(Pred)));
      }

      Insect.insert(Idx);
      if (NEQ(Insect, DOM[Idx])) {
        DOM[Idx] = Insect;
        Changed = true;
      }
    }
  }
  dump(DOM, Func, "DOM");
  return DOM;
}

std::map<int, int> getIDomOfFunction(IRFunctionPtr Func) {
  auto Blocks = Func->getBlocks();
  auto GenSDOM = [](std::map<int, std::set<int>> &&DOM) {
    for (auto &[Source, Doms] : DOM) {
      Doms.erase(Source);
    }
    return DOM;
  };
  auto SDOM = GenSDOM(getDominanceOfFunction(Func));

  std::cout << "\n\n dump SDOM:\n";
  dump(SDOM, Func, "SDOM");
  std::map<int, int> IDOM;
  for (auto &[Dest, Doms] : SDOM) {
    for (auto Source : Doms) {
      bool IsIdom = true;
      for (auto Other : Doms) {
        if (SDOM[Other].count(Source)) {
          IsIdom = false;
          break;
        }
      }
      if (IsIdom) {
        IDOM[Dest] = Source;
      }
    }
  }
  std::cout << "\n\n dump IDOM:\n";
  for (auto [Dest, Source] : IDOM) {
    std::cout << "IDOM of " << Blocks[Dest]->getName() << " is "
              << Blocks[Source]->getName() << "\n";
  }
  return IDOM;
}

std::map<int, std::set<int>> getDomFrontierOfFunction(IRFunctionPtr Func) {
  auto IDOM = getIDomOfFunction(Func);
  auto Blocks = Func->getBlocks();
  auto FindIdx = [&Blocks](BasicBlockPtr BB) {
    for (auto [Idx, Cur] : Blocks) {
      if (Cur == BB) {
        return Idx;
      }
    }
    return -1;
  };
  std::map<int, std::set<int>> DF;
  std::vector<int> Idxs;
  for (auto [Idx, _] : Blocks) {
    Idxs.push_back(Idx);
  }
  for (auto N : Idxs) {
    // has multiple predecessors
    if (Blocks[N]->GetPred().size() > 1) {
      for (auto Pred : Blocks[N]->GetPred()) {
        auto Runner = FindIdx(Pred);
        while (Runner != IDOM[N]) {
          DF[Runner].insert(N);
          Runner = IDOM[Runner];
        }
      }
    }
  }
  std::cout << "\n\n dump DF:\n";
  dump(DF, Func, "DF");
  return DF;
}