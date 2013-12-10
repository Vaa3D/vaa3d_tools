#include "zswcterminalsurfacemetric.h"
#include <vector>
#include "zswctree.h"
#include "swctreenode.h"

ZSwcTerminalSurfaceMetric::ZSwcTerminalSurfaceMetric()
{
}

double ZSwcTerminalSurfaceMetric::measureDistance(
    const ZSwcTree *tree1, const ZSwcTree *tree2)
{
  if (tree1 == NULL || tree2 == NULL) {
    return 0.0;
  }

  if (tree1->isEmpty() || tree2->isEmpty()) {
    return 0.0;
  }

  if (tree1 == tree2) {
    return 0.0;
  }

  const std::vector<Swc_Tree_Node*> &leafArray1 =
      tree1->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  const std::vector<Swc_Tree_Node*> &leafArray2 =
      tree2->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  double minDist = Infinity;

  for (std::vector<Swc_Tree_Node*>::const_iterator iter1 = leafArray1.begin();
       iter1 != leafArray1.end(); ++iter1) {
    for (std::vector<Swc_Tree_Node*>::const_iterator iter2 = leafArray2.begin();
         iter2 != leafArray2.end(); ++iter2) {
      double dist = SwcTreeNode::distance(
            *iter1, *iter2, SwcTreeNode::EUCLIDEAN_SURFACE);
      if (dist < minDist) {
        minDist = dist;
        m_closestNodePair.first = *iter1;
        m_closestNodePair.second = *iter2;
      }
    }
  }

  return minDist;
}
