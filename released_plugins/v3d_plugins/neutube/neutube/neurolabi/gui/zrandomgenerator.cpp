#include "zrandomgenerator.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

int ZRandomGenerator::count = 0;

ZRandomGenerator::ZRandomGenerator()
{
  srand((unsigned)time(0) + count);
  count++;
}

ZRandomGenerator::ZRandomGenerator(int seed)
{
  srand(seed);
}

void ZRandomGenerator::setSeed(int seed)
{
  srand(seed);
}

int ZRandomGenerator::rndint(int maxValue)
{
  return rand() % maxValue + 1;
}

struct IntLessThan {
    bool operator() (const pair<int, int> &p1, const pair<int, int> &p2)
    {
        if (p1.first < p2.first) {
            return true;
        }

        return false;
    }
};

vector<int> ZRandomGenerator::randperm(int n)
{
  vector<pair<int, int> > randArray(n);
  for (int i = 0; i < n; i++) {
    randArray[i].first = rndint(n * 5);
    randArray[i].second = i + 1;
  }

  sort(randArray.begin(), randArray.end(), IntLessThan());

  vector<int> permArray(n);
  for (int i = 0; i < n; i++) {
    permArray[i] = randArray[i].second;
  }

  return permArray;
}
