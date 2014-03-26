#ifndef ZSWCTERMINALANGLEMETRIC_H
#define ZSWCTERMINALANGLEMETRIC_H

#include "zswcmetric.h"

/*!
 * Computer terminal distance weighted by terminal angles
 */
class ZSwcTerminalAngleMetric : public ZSwcMetric
{
public:
  ZSwcTerminalAngleMetric();

  virtual double measureDistance(const ZSwcTree *tree1, const ZSwcTree *tree2);

  /*!
   * \brief Enable or disable distance weight
   */
  void setDistanceWeight(bool enable);

private:
  void extractLeafDirection(const Swc_Tree_Node *tn, double *pos1, double *pos2);

private:
  bool m_useDistWeight;
};

#endif // ZSWCTERMINALANGLEMETRIC_H
