#include "zellipsoid.h"

#include <iostream>

ZEllipsoid::ZEllipsoid() : m_rX(0.0), m_rY(0.0), m_rZ(0.0)
{
}

bool ZEllipsoid::containsPoint(double x, double y, double z) const
{
  if (m_rX <= 0 || m_rY <= 0 || m_rZ <= 0) {
    return false;
  }

  ZPoint pt(x, y, z);
  pt -= m_center;
  pt /= ZPoint(m_rX, m_rY, m_rZ);

  return pt.length() <= 1.0;
}

void ZEllipsoid::print() const
{
  std::cout << "Ellipsoid: center - " << m_center.toString() << " size - "
            << m_rX << " " << m_rY << " " << m_rZ << std::endl;
}
