#ifndef ZSWCDEEPANGLEMETRIC_H
#define ZSWCDEEPANGLEMETRIC_H

#include <vector>
#include "swctreenode.h"
#include "zswcmetric.h"

class ZSwcTree;

class ZSwcDeepAngleMetric : ZSwcMetric
{
public:
  ZSwcDeepAngleMetric();

  virtual double measureDistance(const ZSwcTree *tree1, const ZSwcTree *tree2);

  inline void setLevel(int level) { m_level = level; }
  inline void setMinDist(double dist) { m_minDist = dist; }

  inline int getLevel() const { return m_level; }
  inline double getMinDist() const { return m_minDist; }

private:

  /*!
   * \brief Compute angle formed by four nodes
   *
   * <\a tn1 - \a tn2 - \a tn3> + <\a tn2, \a tn3, \a tn4>.
   *
   * It returns 0 if any of the input pointers is NULL.
   */
  double computeAngle(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                      const Swc_Tree_Node *tn3, const Swc_Tree_Node *tn4);

  /*!
   * \brief Compute deep angle between two array of nodes
   *
   * nodeArray1[1] - nodeArray1[0] -- nodeArray2[0] - nodeArray2[1]
   */
  double computeDeepAngle(const std::vector<const Swc_Tree_Node*> &nodeArray1,
                          const std::vector<const Swc_Tree_Node*> &nodeArray2);

  std::vector<const Swc_Tree_Node *>
  extractLeafSegment(const Swc_Tree_Node *tn);

private:
  int m_level;
  double m_minDist;
};

#endif // ZSWCDEEPANGLEMETRIC_H
