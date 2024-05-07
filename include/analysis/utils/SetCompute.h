#include <set>

template <typename T>
std::set<T> Union(const std::set<T> &A, const std::set<T> &B) {
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

template <typename T>
std::set<T> Intersection(const std::set<T> &A, const std::set<T> &B) {
  std::set<T> Res;
  for (auto E : A) {
    if (B.count(E))
      Res.insert(E);
  }
  return Res;
}

template <typename T> bool EQ(const std::set<T> &A, const std::set<T> &B) {
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

template <typename T> bool NEQ(const std::set<T> &A, const std::set<T> &B) {
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