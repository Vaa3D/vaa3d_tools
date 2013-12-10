#include "zflyemneuronrangecompare.h"

ZFlyEmNeuronRangeCompare::ZFlyEmNeuronRangeCompare() :
  m_maxMainTrunkRadius(0.0)
{
}

double ZFlyEmNeuronRangeCompare::getDifference(
    const ZFlyEmNeuronRange &range1, const ZFlyEmNeuronRange &range2, double z) const
{
  return range1.getRadius(z) - range2.getRadius(z);
}

ZFlyEmNeuronRangeCompare::EMatchStatus ZFlyEmNeuronRangeCompare::compare(
    const ZFlyEmNeuronRange &range1, const ZFlyEmNeuronRange &range2, double z) const
{
  double r1 = range1.getRadius(z);
  double r2 = range2.getRadius(z);

  if (r1 == 0.0 && r2 == 0.0) {
    return GOOD_MATCH;
  }

  if (r1 == 0.0) {
    return MISSING_BRANCH;
  }

  if (r2 == 0.0) {
    return EXTRA_BRANCH;
  }

  if (r1 <= m_maxMainTrunkRadius && r2 > m_maxMainTrunkRadius) {
    return MISSING_BRANCH;
  }

  if (r1 > m_maxMainTrunkRadius && r2 <= m_maxMainTrunkRadius) {
    return EXTRA_BRANCH;
  }

  if (r1 <= m_maxMainTrunkRadius && r2 <= m_maxMainTrunkRadius) {
    return GOOD_MATCH;
  }

  double scale = 2.0;
  if (r1 >= r2 * 1.5) {
    return EXTRA_BRANCH;
  }

  if (r2 >= r1 * scale) {
    return MISSING_BRANCH;
  }

  return GOOD_MATCH;
}
