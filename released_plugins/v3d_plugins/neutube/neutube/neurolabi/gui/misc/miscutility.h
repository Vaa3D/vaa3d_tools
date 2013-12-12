#ifndef MISCUTILITY_H
#define MISCUTILITY_H

#include <vector>
#include "zhistogram.h"
#include "zstack.hxx"

namespace misc {

void paintRadialHistogram(const ZHistogram hist, double cx, double cy, int z,
                          Stack *stack);
void paintRadialHistogram2D(const std::vector<ZHistogram> hist,
                            double cx, int startZ, Stack *stack);

}

namespace {
// generic solution
template <class T>
int numDigits(T number)
{
  int digits = 0;
  if (number < 0) digits = 1; // remove this line if '-' counts as a digit
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}

// partial specialization optimization for 32-bit numbers
template<>
int numDigits(int32_t x)
{
  if (x == std::numeric_limits<int>::min()) return 10 + 1;
  if (x < 0) return numDigits(-x) + 1;

  if (x >= 10000) {
    if (x >= 10000000) {
      if (x >= 100000000) {
        if (x >= 1000000000)
          return 10;
        return 9;
      }
      return 8;
    }
    if (x >= 100000) {
      if (x >= 1000000)
        return 7;
      return 6;
    }
    return 5;
  }
  if (x >= 100) {
    if (x >= 1000)
      return 4;
    return 3;
  }
  if (x >= 10)
    return 2;
  return 1;
}

//// partial-specialization optimization for 8-bit numbers
//template <>
//int numDigits(char n)
//{
//  // if you have the time, replace this with a static initialization to avoid
//  // the initial overhead & unnecessary branch
//  static char x[256] = {0};
//  if (x[0] == 0) {
//    for (char c = 1; c != 0; c++)
//      x[static_cast<int>(c)] = numDigits((int32_t)c);
//    x[0] = 1;
//  }
//  return x[static_cast<int>(n)];
//}
}


#endif // MISCUTILITY_H
