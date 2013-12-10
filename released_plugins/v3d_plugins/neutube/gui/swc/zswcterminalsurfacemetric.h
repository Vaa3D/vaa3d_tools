#ifndef ZSWCTERMINALSURFACEMETRIC_H
#define ZSWCTERMINALSURFACEMETRIC_H

#include "zswcmetric.h"

class ZSwcTerminalSurfaceMetric : public ZSwcMetric
{
public:
  ZSwcTerminalSurfaceMetric();

  virtual double measureDistance(const ZSwcTree *tree1, const ZSwcTree *tree2);
};

#endif // ZSWCTERMINALSURFACEMETRIC_H
