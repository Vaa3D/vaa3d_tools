#ifndef ZFLYEMNEURONRANGECOMPARE_H
#define ZFLYEMNEURONRANGECOMPARE_H

#include "zflyemneuronrange.h"

/*!
 * \brief The class for comparing two ranges
 */
class ZFlyEmNeuronRangeCompare
{
public:
  ZFlyEmNeuronRangeCompare();

  enum EMatchStatus {
    EXTRA_BRANCH, MISSING_BRANCH, GOOD_MATCH
  };

  /*!
   * \brief Get the radius difference between two range
   *
   * Computes \a range1( \a z ) - range2( \a z )
   */
  double getDifference(const ZFlyEmNeuronRange &range1,
                       const ZFlyEmNeuronRange &range2, double z) const;

  /*!
   * \brief Compare two ranges.
   *
   * \return The status indicating if there is any extra branch or missing
   *         branch, or neither.
   */
  EMatchStatus compare(const ZFlyEmNeuronRange &range1,
                       const ZFlyEmNeuronRange &range2,
                       double z) const;

private:
  double m_maxMainTrunkRadius;
};

#endif // ZFLYEMNEURONRANGECOMPARE_H
