// https://www.topcoder.com/community/data-science/data-science-tutorials/disjoint-set-data-structures/
#pragma once
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <cassert>
#include <map>
#include <vector>

namespace sigen {

class DisjointSetInternal {
  std::vector<int> data;
  int Root(int x);

public:
  explicit DisjointSetInternal(int size);
  int Size(int x);
  bool Same(int x, int y);
  void Merge(int x, int y);
};

template <class T>
class DisjointSet : boost::noncopyable {
  std::map<T, int> forward;
  boost::shared_ptr<DisjointSetInternal> U;

public:
  void Add(T x) {
    int next_id = forward.size();
    forward[x] = next_id;
  }

  void SetUp() {
    U = boost::make_shared<DisjointSetInternal>(forward.size());
  }

  int Size(T x) {
    assert((bool)U);
    assert(forward.count(x) > 0);
    return U->Size(forward[x]);
  }

  bool IsSame(T x, T y) {
    assert((bool)U);
    assert(forward.count(x) > 0);
    assert(forward.count(y) > 0);
    return U->Same(forward[x], forward[y]);
  }

  void Merge(T x, T y) {
    assert((bool)U);
    assert(forward.count(x) > 0);
    assert(forward.count(y) > 0);
    U->Merge(forward[x], forward[y]);
  }
};

} // namespace sigen
