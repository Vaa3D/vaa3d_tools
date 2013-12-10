/**@file zpoint.h
 * @brief 3D point class
 * @author Ting Zhao
 */
#ifndef ZPOINT_H
#define ZPOINT_H

#include <iostream>
#include <string>

#include "zqtheader.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"

class ZPoint : public ZDocumentable, public ZStackDrawable {
public:
  ZPoint();
  ZPoint(double x, double y, double z);
  ZPoint(const double *pt);
  ZPoint(const ZPoint &pt);

  virtual const std::string& className() const;

public:
  inline void set(double x, double y, double z) {
    m_x = x;
    m_y = y;
    m_z = z;
  }
  inline void set(const ZPoint &pt) {
    set(pt.x(), pt.y(), pt.z());
  }
  inline double x() const { return m_x; }
  inline double y() const { return m_y; }
  inline double z() const { return m_z; }

  inline double* xRef() { return &m_x; }
  inline double* yRef() { return &m_y; }
  inline double* zRef() { return &m_z; }

  const double& operator[] (int index) const;
  double& operator[] (int index);
  ZPoint& operator= (const ZPoint &pt);

  inline void setX(double x) { m_x = x; }
  inline void setY(double y) { m_y = y; }
  inline void setZ(double z) { m_z = z; }

  double distanceTo(const ZPoint &pt) const;
  double distanceTo(double x, double y, double z) const;
  double length() const;

  ZPoint& operator += (const ZPoint &pt);
  ZPoint& operator -= (const ZPoint &pt);
  ZPoint& operator /= (const ZPoint &pt);
  ZPoint& operator += (double offset);
  ZPoint& operator -= (double offset);
  ZPoint& operator *= (double scale);
  ZPoint& operator /= (double scale);

  friend ZPoint operator + (const ZPoint &pt1, const ZPoint &pt2);
  friend ZPoint operator - (const ZPoint &pt1, const ZPoint &pt2);
  friend ZPoint operator * (const ZPoint &pt1, double scale);

  void toArray(double *pt) const;

  void normalize();
  double dot(const ZPoint &pt) const;
  double cosAngle(const ZPoint &pt) const;

  bool isApproxOrigin() const;
  bool approxEquals(const ZPoint &pt) const;

  std::string toString() const;
  std::string toJsonString() const;

  inline void translate(double dx, double dy, double dz) {
    m_x += dx;
    m_y += dy;
    m_z += dz;
  }

public:
  virtual void display(QPainter &painter, int n = 0, Display_Style style = NORMAL) const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  static inline double minimalDistance() { return m_minimalDistance; }

private:
  double m_x;
  double m_y;
  double m_z;

  const static double m_minimalDistance;
};

#endif // ZPOINT_H
