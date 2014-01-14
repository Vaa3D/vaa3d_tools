#include "zsynapselocationmetric.h"

FlyEm::ZSynapseLocationMetric::ZSynapseLocationMetric()
{
}

double FlyEm::ZSynapseLocationEuclideanMetric::distance(
    const FlyEm::SynapseLocation &loc1,
    const FlyEm::SynapseLocation &loc2)
{
  return loc1.pos().distanceTo(loc2.pos());
}

double FlyEm::ZSynapseLocationAngleMetric::distance(
    const FlyEm::SynapseLocation &loc1,
    const FlyEm::SynapseLocation &loc2)
{
  ZPoint vec1, vec2;
  vec1 = loc1.pos() - m_refPoint;
  vec2 = loc2.pos() - m_refPoint;

  return 1.0 - vec1.cosAngle(vec2);
}
