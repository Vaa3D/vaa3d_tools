#ifndef ZWEIGHTEDPOINT_H
#define ZWEIGHTEDPOINT_H

#include "zpoint.h"

class ZWeightedPoint : public ZPoint
{
public:
  ZWeightedPoint();
  ZWeightedPoint(double x, double y, double z, double w);

  inline double weight() const { return m_weight; }
  inline void setWeight(double weight) { m_weight = weight; }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const ZWeightedPoint &pt);

private:
  double m_weight;
};


#endif // ZWEIGHTEDPOINT_H
