#include "zpoint.h"

#include <iostream>
#include <sstream>

#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include <stdio.h>

#include "tz_math.h"
#include "tz_geo3d_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_error.h"
#include <cstdio>

const double ZPoint::m_minimalDistance = 1e-5;

ZPoint::ZPoint()
{
  set(0, 0, 0);
}

ZPoint::ZPoint(double x, double y, double z)
{
  set(x, y, z);
}

ZPoint::ZPoint(const double *pt)
{
  set(pt[0], pt[1], pt[2]);
}

ZPoint::ZPoint(const ZPoint &pt) :
  ZInterface(pt), ZDocumentable(pt), ZStackDrawable(pt)
{
  set(pt.m_x, pt.m_y, pt.m_z);
}

void ZPoint::display(QPainter &painter, int n, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  if ((iround(m_z) == n) || (n == -1)) {
    painter.setPen(QPen(QColor(0, 0, 255, 255), .7));
    painter.drawPoint(m_x, m_y);
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(n);
  UNUSED_PARAMETER(style);
#endif
}

void ZPoint::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZPoint::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

double ZPoint::distanceTo(const ZPoint &pt) const
{
  return Geo3d_Dist(m_x, m_y, m_z, pt.x(), pt.y(), pt.z());
}

double ZPoint::distanceTo(double x, double y, double z) const
{
  return Geo3d_Dist(m_x, m_y, m_z, x, y, z);
}

double ZPoint::length() const
{
  return Geo3d_Orgdist(m_x, m_y, m_z);
}

ZPoint& ZPoint::operator +=(const ZPoint &pt)
{
  m_x += pt.m_x;
  m_y += pt.m_y;
  m_z += pt.m_z;

  return *this;
}

ZPoint& ZPoint::operator -=(const ZPoint &pt)
{
  m_x -= pt.m_x;
  m_y -= pt.m_y;
  m_z -= pt.m_z;

  return *this;
}

ZPoint& ZPoint::operator +=(double offset)
{
  m_x += offset;
  m_y += offset;
  m_z += offset;

  return *this;
}

ZPoint& ZPoint::operator -=(double offset)
{
  m_x -= offset;
  m_y -= offset;
  m_z -= offset;

  return *this;
}

ZPoint& ZPoint::operator *=(double scale)
{
  m_x *= scale;
  m_y *= scale;
  m_z *= scale;

  return *this;
}

ZPoint& ZPoint::operator /=(double scale)
{
  m_x /= scale;
  m_y /= scale;
  m_z /= scale;

  return *this;
}

ZPoint& ZPoint::operator /=(const ZPoint &pt)
{
  m_x /= pt.m_x;
  m_y /= pt.m_y;
  m_z /= pt.m_z;

  return *this;
}

ZPoint operator + (const ZPoint &pt1, const ZPoint &pt2)
{
  return ZPoint(pt1) += pt2;
}

ZPoint operator - (const ZPoint &pt1, const ZPoint &pt2)
{
  return ZPoint(pt1) -= pt2;
}

ZPoint operator * (const ZPoint &pt1, double scale)
{
  return ZPoint(pt1) *= scale;
}

void ZPoint::toArray(double *pt) const
{
  pt[0] = m_x;
  pt[1] = m_y;
  pt[2] = m_z;
}

void ZPoint::normalize()
{
  coordinate_3d_t coord;
  toArray(coord);
  Coordinate_3d_Unitize(coord);
  set(coord[0], coord[1], coord[2]);
}

const double& ZPoint::operator [](int index) const
{
  TZ_ASSERT(index >= 0 && index < 3, "Invalid index");

  switch (index) {
  case 0:
    return m_x;
  case 1:
    return m_y;
  case 2:
    return m_z;
  default:
    break;
  }

  std::cerr << "Index out of bound" << std::endl;

  return m_x;
}

double& ZPoint::operator[] (int index)
{
  return const_cast<double&>(static_cast<const ZPoint&>(*this)[index]);
}

ZPoint& ZPoint::operator= (const ZPoint &pt)
{
  m_x = pt.m_x;
  m_y = pt.m_y;
  m_z = pt.m_z;

  return *this;
}

double ZPoint::dot(const ZPoint &pt) const
{
  return m_x * pt.x() + m_y * pt.y() + m_z * pt.z();
}

double ZPoint::cosAngle(const ZPoint &pt) const
{
  if (this->isApproxOrigin() || pt.isApproxOrigin()) {
    return 1.0;
  }

  double value = dot(pt) / length() / pt.length();

  if (value > 1.0) {
    value = 1.0;
  } else if (value < -1.0) {
    value = -1.0;
  }

  return value;
}

bool ZPoint::isApproxOrigin() const
{
  return (length() < m_minimalDistance);
}

bool ZPoint::approxEquals(const ZPoint &pt) const
{
  return (distanceTo(pt) < m_minimalDistance);
}

std::string ZPoint::toString() const
{
  std::ostringstream stream;
  stream << "(" << x() << ", " << y() << ", " << z() << ")";

  return stream.str();
}

std::string ZPoint::toJsonString() const
{
  std::ostringstream stream;
  stream << "[" << x() << ", " << y() << ", " << z() << "]";

  return stream.str();
}

ZINTERFACE_DEFINE_CLASS_NAME(ZPoint)

