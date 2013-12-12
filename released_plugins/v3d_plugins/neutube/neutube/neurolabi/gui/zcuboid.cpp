#include "zcuboid.h"

#include <math.h>
#include <cstddef>
#include <iostream>

#include "zpoint.h"
#include "tz_error.h"

#ifndef NULL
#  define NULL 0x0
#endif

using namespace std;

ZCuboid::ZCuboid() : m_firstCorner(), m_lastCorner()
{
}

ZCuboid::ZCuboid(double x1, double y1, double z1,
                 double x2, double y2, double z2)
{
  set(x1, y1, z1, x2, y2, z2);
}

ZCuboid::ZCuboid(const ZCuboid &cuboid) : m_firstCorner(cuboid.m_firstCorner),
  m_lastCorner(cuboid.m_lastCorner)
{
}

void ZCuboid::set(double x1, double y1, double z1,
                  double x2, double y2, double z2)
{
  m_firstCorner.set(x1, y1, z1);
  m_lastCorner.set(x2, y2, z2);
}

void ZCuboid::set(const double *corner)
{
  set(corner[0], corner[1], corner[2], corner[3], corner[4], corner[5]);
}

bool ZCuboid::isValid()
{
  return (m_lastCorner.x() > m_firstCorner.x()) &&
      (m_lastCorner.y() > m_firstCorner.y()) &&
      (m_lastCorner.z() > m_firstCorner.z());
}

double ZCuboid::width()
{
  return (m_lastCorner.x() - m_firstCorner.x());
}

double ZCuboid::height()
{
  return (m_lastCorner.y() - m_firstCorner.y());
}

double ZCuboid::depth()
{
  return (m_lastCorner.z() - m_firstCorner.z());
}

double ZCuboid::volume()
{
  return width() * height() * depth();
}

void ZCuboid::intersect(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = max(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = min(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }
}

void ZCuboid::bind(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = min(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = max(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }
}

double ZCuboid::moveOutFrom(ZCuboid &cuboid, double margin)
{
  double minOffset = -1;
  int movingDim = -1;

  for (int i = 0; i < 3; i++) {
    double offset = cuboid.m_firstCorner[i] - m_lastCorner[i];
    if (movingDim < 0) {
      movingDim = 0;
      minOffset = offset;
    } else {
      if (fabs(minOffset) > fabs(offset)) {
        minOffset = offset;
        movingDim = i;
      }
    }

    offset = cuboid.m_lastCorner[i] - m_firstCorner[i];
    if (fabs(minOffset) > fabs(offset)) {
      minOffset = offset;
      movingDim = i;
    }
  }

  if (minOffset >= 0.0) {
    minOffset += margin;
  } else {
    minOffset -= margin;
  }

  m_firstCorner[movingDim] += minOffset;
  m_lastCorner[movingDim] += minOffset;

  return fabs(minOffset);
}

void ZCuboid::layout(std::vector<ZCuboid> *cuboidArray, double margin)
{
  if (cuboidArray == NULL) {
    return;
  }

  ZCuboid b0 = *this;
  size_t numberOfBoxMoved = 0;

  vector<bool> moved(cuboidArray->size(), false);

  while (numberOfBoxMoved < cuboidArray->size()) {
    double minOffset = -1.0;
    int movingBoxIndex = -1;

    for (size_t i = 0; i < cuboidArray->size(); i++) {
      if (!moved[i]) {
        ZCuboid currentBox = (*cuboidArray)[i];
        double offset = currentBox.moveOutFrom(b0, margin);
        if ((minOffset < 0.0) || (offset < minOffset)) {
          minOffset = offset;
          movingBoxIndex = i;
        }
      }
    }

    (*cuboidArray)[movingBoxIndex].moveOutFrom(b0, margin);
    b0.bind((*cuboidArray)[movingBoxIndex]);
    moved[movingBoxIndex] = true;
    numberOfBoxMoved++;
  }
}

ZCuboid& ZCuboid::operator= (const ZCuboid &cuboid)
{
  m_firstCorner = cuboid.m_firstCorner;
  m_lastCorner = cuboid.m_lastCorner;

  return *this;
}

double& ZCuboid::operator [](int index)
{
  return const_cast<double&>(static_cast<const ZCuboid&>(*this)[index]);
}

const double& ZCuboid::operator [](int index) const
{
  if (index < 3) {
    return m_firstCorner[index];
  }

  return m_lastCorner[index - 3];
}

double ZCuboid::estimateSeparateScale(const ZCuboid &cuboid, const ZPoint &vec)
const
{
  double bestScale = 0.0;

  if (!vec.isApproxOrigin()) {
    //For each dimension
    for (int i = 0; i < 3; i++) {
      //If its moving step is positive
      if (vec[i] > ZPoint::minimalDistance()) {
        //Calculate forwared moving scale
        double diff = cuboid[i] - (*this)[i];
        if (fabs(diff) > ZPoint::minimalDistance()) {
          double scale = 0.0;
          if (diff > 0.0) {
            scale = ((*this)[i+3] - (*this)[i]) / diff / vec[i];
          } else {
            scale = (cuboid[i] - cuboid[i+3]) / diff / vec[i];
          }

          if ((scale < bestScale) || (bestScale == 0.0)) {
            bestScale = scale;
          }
        }
      }
    }
  }

  if (bestScale == 0.0) {
    bestScale = 1.0;
  }

  return bestScale;
}

void ZCuboid::print()
{
  cout << "(" << m_firstCorner.x() << "," << m_firstCorner.y() << "," <<
          m_firstCorner.z() << ")" << " -> (" << m_lastCorner.x() << "," <<
          m_lastCorner.y() << "," << m_lastCorner.z() << ")" << endl;
}

void ZCuboid::scale(double s)
{
  m_firstCorner *= s;
  m_lastCorner *= s;
}

void ZCuboid::expand(double margin)
{
  m_firstCorner -= margin;
  m_lastCorner += margin;
}

ZPoint ZCuboid::corner(int index) const
{
  TZ_ASSERT(index >= 0 && index <= 7, "invalid index.");

  switch (index) {
  case 0:
    return m_firstCorner;
  case 7:
    return m_lastCorner;
  case 1:
    return ZPoint(m_lastCorner.x(), m_firstCorner.y(), m_firstCorner.z());
  case 2:
    return ZPoint(m_firstCorner.x(), m_lastCorner.y(), m_firstCorner.z());
  case 3:
    return ZPoint(m_lastCorner.x(), m_lastCorner.y(), m_firstCorner.z());
  case 4:
    return ZPoint(m_firstCorner.x(), m_firstCorner.y(), m_lastCorner.z());
  case 5:
    return ZPoint(m_lastCorner.x(), m_firstCorner.y(), m_lastCorner.z());
  case 6:
    return ZPoint(m_firstCorner.x(), m_lastCorner.y(), m_lastCorner.z());
  default:
    break;
  }

  return ZPoint(0, 0, 0);
}

ZPoint ZCuboid::center() const
{
  return ZPoint((m_firstCorner.x() + m_lastCorner.x()) / 2.0,
                (m_firstCorner.y() + m_lastCorner.y()) / 2.0,
                (m_firstCorner.z() + m_lastCorner.z()) / 2.0);
}

void ZCuboid::joinX(double x)
{
  if (m_firstCorner.x() > x) {
    m_firstCorner.setX(x);
  } else if (m_lastCorner.x() < x) {
    m_lastCorner.setX(x);
  }
}

void ZCuboid::joinY(double y)
{
  if (m_firstCorner.y() > y) {
    m_firstCorner.setY(y);
  } else if (m_lastCorner.y() < y) {
    m_lastCorner.setY(y);
  }
}

void ZCuboid::joinZ(double z)
{
  if (m_firstCorner.z() > z) {
    m_firstCorner.setZ(z);
  } else if (m_lastCorner.z() < z) {
    m_lastCorner.setZ(z);
  }
}
