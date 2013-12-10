#ifndef ZSWCGENERATOR_H
#define ZSWCGENERATOR_H

#include "zcuboid.h"
#include "flyem/zflyemneuronrange.h"
#include "flyem/zflyemneuronaxis.h"
#include "zvoxelarray.h"

class ZSwcTree;

class ZSwcGenerator
{
public:
  ZSwcGenerator();

  enum EPostProcess {
    OPTIMAL_SAMPLING, NO_PROCESS
  };

  static ZSwcTree* createVirtualRootSwc();
  static ZSwcTree* createCircleSwc(double cx, double cy, double cz, double r);
  static ZSwcTree* createBoxSwc(const ZCuboid &box);
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range);
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range,
                             const ZFlyEmNeuronAxis &axis);
  static ZSwcTree* createRangeCompareSwc(
      const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference);

  static ZSwcTree* createSwc(const ZVoxelArray &voxelArray,
                             EPostProcess option = NO_PROCESS);
};

#endif // ZSWCGENERATOR_H
