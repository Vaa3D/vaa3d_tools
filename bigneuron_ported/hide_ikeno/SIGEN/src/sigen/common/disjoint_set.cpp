#include "sigen/common/disjoint_set.h"
#include <algorithm>

namespace sigen {

DisjointSetInternal::DisjointSetInternal(int size) : data(size, -1) {}

int DisjointSetInternal::Root(int x) {
  return data[x] < 0 ? x : data[x] = Root(data[x]);
}

bool DisjointSetInternal::Same(int x, int y) {
  return Root(x) == Root(y);
}

int DisjointSetInternal::Size(int x) {
  return -data[Root(x)];
}

void DisjointSetInternal::Merge(int x, int y) {
  x = Root(x), y = Root(y);
  if (x != y) {
    if (Size(y) > Size(x))
      std::swap(x, y);
    data[x] += data[y];
    data[y] = x;
  }
}

} // namespace sigen
