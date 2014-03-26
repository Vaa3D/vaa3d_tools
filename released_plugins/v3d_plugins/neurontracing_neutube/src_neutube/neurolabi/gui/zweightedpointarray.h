#ifndef ZWEIGHTEDPOINTARRAY_H
#define ZWEIGHTEDPOINTARRAY_H

#include <vector>
#include "tz_geo3d_scalar_field.h"
#include "zweightedpoint.h"

class ZWeightedPointArray : public std::vector<ZWeightedPoint>
{
public:
  ZWeightedPointArray();

public:
  Geo3d_Scalar_Field *toScalarField();
  ZPoint principalDirection();
};

#endif // ZWEIGHTEDPOINTARRAY_H
