#ifndef ZFLYEMNEURONRANGE_H
#define ZFLYEMNEURONRANGE_H

#include <map>
#include "zpoint.h"

class ZSwcTree;

/*!
 * \brief The class specifying the range of a neuron
 *
 * The shape of the range is defined by a set of planar circles. Any point
 * between two adajcent circles are defined by linear interpolation.
 */

class ZFlyEmNeuronRange
{
public:
  ZFlyEmNeuronRange();

  /*!
   * \brief Test if the range is empty.
   *
   * \return true iff the range is empty.
   */
  bool isEmpty() const;

  /*!
   * \brief Set the range of a certain plane
   *
   * \param z The plane to set
   * \param r Radius of the circle of the plane
   */
  void setPlaneRange(double z, double r);

  /*!
   * \brief Test if a point is within the range.
   *
   * \param x X coordinate of the point
   * \param y Y coordinate of the point
   * \param z Z coordinate of the point
   *
   * \return true iff the point is within the range.
   */
  bool contains(double x, double y, double z) const;

  /*!
   * \brief Test if a point is within the range.
   *
   * \return true iff \a pt is within the range.
   */
  bool contains(const ZPoint &pt) const;


  /*!
   * \brief Get Z minimum
   *
   * \return The minimal Z. Returns 0 iff the range is empty.
   */
  double getMinZ() const;

  /*!
   * \brief Get Z maximum
   *
   * \return The maximal Z. Returns 0 iff the range is empty.
   */
  double getMaxZ() const;

  /*!
   * \brief Get the range radius at a certain plane
   *
   * \param z The Z coordinate of the target plane.
   *
   * \return The radius of the plane range. It returns -1 if \a z is out of
   *         range.
   */
  double getRadius(double z) const;

  /*!
   * \brief Transform the range
   *
   * Transform the range by setting the radius r at each plane as
   * r * scale + offset.
   */
  void transform(double offset, double scale);

  /*!
   * \brief Union of two ranges
   *
   * Set the current range as the union of itself and \a range.
   */
  void unify(const ZFlyEmNeuronRange &range);

  typedef std::map<double, double> RangeMap;

  static inline double getRadius(const std::pair<double, double> &rmap) {
    return rmap.second;
  }
  static inline double getZ(const std::pair<double, double> &rmap) {
    return rmap.first;
  }
  static inline double setRadius(std::pair<double, double> &rmap, double r) {
    return rmap.second = r;
  }
  static inline double setRadius(std::pair<const double, double> &rmap, double r) {
    return rmap.second = r;
  }
  static inline double setZ(std::pair<double, double> &rmap, double z) {
    return rmap.first = z;
  }

  /*!
   * \brief Export the range as a CSV file
   *
   * \return true iff the export is successful.
   */
  bool exportCsvFile(const std::string &filePath) const;

  /*!
   * \brief Import from a CSV file
   *
   * The range is cleared before importing.
   *
   * \return true iff the export is successful.
   */
  bool importCsvFile(const std::string &filePath);

private:
  RangeMap m_planeRange;
};

#endif // ZFLYEMNEURONRANGE_H
