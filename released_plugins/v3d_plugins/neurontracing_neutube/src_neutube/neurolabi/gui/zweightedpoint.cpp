#include "zweightedpoint.h"

ZWeightedPoint::ZWeightedPoint()
{
}

ZWeightedPoint::ZWeightedPoint(double x, double y, double z, double w)
{
  set(x, y, z);
  setWeight(w);
}

std::ostream &operator<<(std::ostream &stream,
                         const ZWeightedPoint &pt)
{
  stream << "(" << pt.x() << ", " << pt.y() << ", " << pt.z() << ", "
         << pt.weight() << ")";

  return stream;
}
