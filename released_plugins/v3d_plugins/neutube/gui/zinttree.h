#ifndef ZINTTREE_H
#define ZINTTREE_H

#include <vector>

class ZIntTree : public std::vector<int>
{
public:
  ZIntTree(std::size_t size);

  void addLink(int i, int j);
  void setParent(int child, int parent);
  inline int getParent(int i){ return (*this)[i]; }
  void detach(int i);
  std::vector<int> traceBack(int i);

  int getCommonAncestor(int i, int j);

  void print();
};

#endif // ZINTTREE_H
