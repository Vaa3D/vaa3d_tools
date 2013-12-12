#ifndef ZSWCMETRIC_H
#define ZSWCMETRIC_H

#include <utility>
#include "tz_swc_tree.h"

class ZSwcTree;

class ZSwcMetric
{
public:
  ZSwcMetric();

  /*!
   * \brief Measure spatial distance between \a tree1 and \a tree2
   *
   * Always returns 0 if tree1 or tree2 is null or empty.
   */
  virtual double measureDistance(const ZSwcTree *tree1, const ZSwcTree *tree2) = 0;

  inline const Swc_Tree_Node* getFirstNode() const {
    return m_closestNodePair.first;
  }

  inline const Swc_Tree_Node* getSecondNode() const {
    return m_closestNodePair.second;
  }

protected:
  std::pair<const Swc_Tree_Node*, const Swc_Tree_Node*> m_closestNodePair;
};

#endif // ZSWCMETRIC_H
