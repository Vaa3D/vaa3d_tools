#ifndef ZRANDOMGENERATOR_H
#define ZRANDOMGENERATOR_H

#include <vector>

class ZRandomGenerator
{
public:
  ZRandomGenerator();
  ZRandomGenerator(int seed);

public:
  std::vector<int> randperm(int n);
  int rndint(int maxValue);
  void setSeed(int seed);

  static int count;
};

#endif // ZRANDOMGENERATOR_H
