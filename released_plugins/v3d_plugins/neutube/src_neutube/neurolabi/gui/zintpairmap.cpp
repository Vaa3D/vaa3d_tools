#include "zintpairmap.h"

#include <iostream>

using namespace std;

void ZIntPairMap::incPairCount(int x, int y)
{
  std::pair<int, int> key(x, y);
  if (this->count(key) == 0) {
    (*this)[key] = 1;
  } else {
    (*this)[key]++;
  }
}

void ZIntPairMap::print() const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    cout << (iter->first).first << " " << (iter->first).second
         << ": " << iter->second << endl;
  }
}

void ZIntPairMap::printSummary() const
{
  cout << "Int pair map: " << size() << " entries" << endl;
}
