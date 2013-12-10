#ifndef ZFLYEMNEURONAXIS_H
#define ZFLYEMNEURONAXIS_H

#include <map>
#include <utility>
#include "zpoint.h"

class ZSwcTree;

/*!
 * \brief The class of representing the medial axis along Z
 */
class ZFlyEmNeuronAxis
{
public:
  ZFlyEmNeuronAxis();

  /*!
   * \brief Test if the axis is empty.
   * \return true iff the axis is empty.
   */
  bool isEmpty() const;

  /*!
   * \brief Get Z minimum
   * \return The minimal Z
   */
  double getMinZ() const;

  /*!
   * \brief Get Z maximum
   * \return The maximal Z
   */
  double getMaxZ() const;

  /*!
   * \brief Get the axis center at a certain plane
   *
   * \return The center at plane \a z. The Z coordinate of the point is \a z.
   */
  ZPoint getCenter(double z) const;

  /*!
   * \brief Set a center point on the axis
   */
  void setCenter(const ZPoint &center);

  typedef std::pair<double, double> AxisPoint;
  typedef std::map<double, AxisPoint> AxisMap;

  /*!
   * \brief Convert the axis into an SWC tree
   *
   * Converts the axis into an swc tree. \a sampleRate specifies the Z
   * distance between to neighboring nodes. If \a sampleRate is not greater
   * than 0, the conversion uses the raw axis represenation. The first and last
   * point will always be sampled.
   *
   * \param sampleRate The sampling rate.
   * \return An swc tree. It returns NULL if the axis is empty.
   */
  ZSwcTree* toSwc(double radius, double sampleRate = 0.0);

private:
  AxisMap m_axis;
};

#endif // ZFLYEMNEURONAXIS_H
