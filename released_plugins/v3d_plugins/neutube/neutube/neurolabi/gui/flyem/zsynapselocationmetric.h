#ifndef ZSYNAPSELOCATIONMETRIC_H
#define ZSYNAPSELOCATIONMETRIC_H

#include "zsynapseannotation.h"

namespace FlyEm {
class ZSynapseLocationMetric
{
public:
  ZSynapseLocationMetric();

  virtual double distance(const SynapseLocation &loc1,
                          const SynapseLocation &loc2) = 0;

  inline void setRefPoint(const ZPoint &pt) { m_refPoint = pt; }

protected:
  ZPoint m_refPoint;
};


class ZSynapseLocationEuclideanMetric : public ZSynapseLocationMetric
{
public:
  virtual double distance(const SynapseLocation &loc1,
                          const SynapseLocation &loc2);
};

class ZSynapseLocationAngleMetric : public ZSynapseLocationMetric
{
public:
  virtual double distance(const SynapseLocation &loc1,
                          const SynapseLocation &loc2);
};

}
#endif // ZSYNAPSELOCATIONMETRIC_H
