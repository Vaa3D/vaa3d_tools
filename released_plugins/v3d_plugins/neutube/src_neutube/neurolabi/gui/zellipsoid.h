#ifndef ZELLIPSOID_H
#define ZELLIPSOID_H

#include "zpoint.h"

/*!
 * \brief The class of non-oriented ellipsoid
 *
 * A ellipsoid is specified by six parameters: the coordinates of its center and
 * the radii along three dimensions.
 */
class ZEllipsoid
{
public:
  ZEllipsoid();

  /*!
   * \brief Test if point is contained by the ellipsoid
   *
   * It returns true iff (\a x, \a y, \a z) is inside or on the ellipsoid.
   *
   * \param x X coordinate.
   * \param y Y coordinate.
   * \param z Z coordinate.
   */
  bool containsPoint(double x, double y, double z) const;

  inline const ZPoint& getCenter() const { return m_center; }
  inline double getXRadius() const { return m_rX; }
  inline double getYRadius() const { return m_rY; }
  inline double getZRadius() const { return m_rZ; }

  inline void setCenter(const ZPoint &center) { m_center = center; }
  inline void setCenter(double x, double y, double z) {
    m_center.set(x, y, z);
  }
  inline void setCenterX(double x) {
    m_center.setX(x);
  }
  inline void setCenterY(double y) {
    m_center.setY(y);
  }

  inline void setSize(double rx, double ry, double rz) {
    m_rX = rx; m_rY = ry; m_rZ = rz;
  }

  void print() const;

private:
  ZPoint m_center;
  double m_rX;
  double m_rY;
  double m_rZ;
};

#endif // ZELLIPSOID_H
