#include <set>

template <typename T> std::set<T> Union(std::set<T> &A, std::set<T> &B) {
  std::set<T> Res;
  for (auto E : A) {
    Res.insert(E);
  }
  for (auto E : B) {
    if (!A.count(E))
      Res.insert(E);
  }
  return Res;
}

template <typename T> std::set<T> Intersection(std::set<T> &A, std::set<T> &B) {
  std::set<T> Res;
  for (auto E : A) {
    if (B.count(E))
      Res.insert(E);
  }
  return Res;
}

template <typename T> bool EQ(std::set<T> &A, std::set<T> &B) {
  for (auto E : A) {
    if (!B.count(E)) {
      return false;
    }
  }
  for (auto E : B) {
    if (!A.count(E)) {
      return false;
    }
  }
  return true;
}

template <typename T> bool NEQ(std::set<T> &A, std::set<T> &B) {
  for (auto E : A) {
    if (!B.count(E)) {
      return true;
    }
  }
  for (auto E : B) {
    if (!A.count(E)) {
      return true;
    }
  }
  return false;
}