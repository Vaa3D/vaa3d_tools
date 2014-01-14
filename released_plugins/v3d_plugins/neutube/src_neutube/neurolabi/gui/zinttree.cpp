#include "zinttree.h"

#include <iostream>

using namespace std;

ZIntTree::ZIntTree(size_t size) : vector<int>(size, -1)
{
}

void ZIntTree::addLink(int i, int j)
{
  if (i >= 0) {
    (*this)[i] = j;
  }
}

void ZIntTree::setParent(int child, int parent)
{
  addLink(child, parent);
}

void ZIntTree::detach(int i)
{
  if (i >= 0) {
    (*this)[i] = -1;
  }
}

vector<int> ZIntTree::traceBack(int i)
{
  vector<int> trace;

  int index = i;
  while (index >= 0) {
    trace.push_back(index);
    index = (*this)[index];
  }

  return trace;
}

void ZIntTree::print()
{
  cout << "Int tree: " << endl;

  std::vector<bool> isLeaf(size(), true);
  for (size_t i = 0; i < size(); i++) {
    vector<int> trace = traceBack(i);
    for (size_t j = 1; j < trace.size(); j++) {
      isLeaf[trace[j]] = false;
    }
  }

  for (size_t i = 0; i < size(); i++) {
    if (isLeaf[i]) {
      vector<int> trace = traceBack(i);
      if (trace.size() > 1) {
        cout << trace[0];
        for (size_t j = 1; j < trace.size(); j++) {
          cout << "-->" << trace[j];
        }
        cout << endl;
      }
    }
  }
}

int ZIntTree::getCommonAncestor(int i, int j)
{
  if (i < 0 || j < 0) {
    return -1;
  }

  if (i == j) {
    return i;
  }

  vector<bool> checked(size(), false);
  int p = i;
  while (p >= 0) {
    checked[p] = true;
    p = getParent(p);
  }

  p = j;
  while (p >= 0) {
    if (checked[p]) {
      return p;
    }
    p = getParent(p);
  }

  return -1;
}
