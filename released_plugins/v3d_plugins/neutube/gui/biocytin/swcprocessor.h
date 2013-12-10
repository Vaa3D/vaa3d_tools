#ifndef SWCPROCESSOR_H
#define SWCPROCESSOR_H

#include "c_stack.h"

class ZSwcTree;

namespace Biocytin {
class SwcProcessor
{
public:
  SwcProcessor();

public:
  static void assignZ(ZSwcTree *tree, const Stack &depthImage);
  static void removeZJump(ZSwcTree *tree, double minDeltaZ);
  static void breakZJump(ZSwcTree *tree, double minDeltaZ);
  static void removeOrphan(ZSwcTree *tree);
  static void smoothZ(ZSwcTree *tree);
  static void smoothRadius(ZSwcTree *tree);
};
}

#endif // SWCPROCESSOR_H
