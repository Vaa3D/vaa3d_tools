#include "zswcterminalanglemetric.h"
#include "zswctree.h"
#include "swctreenode.h"

ZSwcTerminalAngleMetric::ZSwcTerminalAngleMetric() : m_useDistWeight(true)
{
}

void ZSwcTerminalAngleMetric::extractLeafDirection(
    const Swc_Tree_Node *tn, double *pos1, double *pos2)
{
  Swc_Tree_Node_Pos(tn, pos1);
  if (SwcTreeNode::isRoot(tn)) {
    Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);
    if (child == NULL) {
      Swc_Tree_Node_Pos(tn, pos2);
    } else {
      Swc_Tree_Node *grandChild = SwcTreeNode::firstChild(child);
      if (grandChild != NULL) {
        Swc_Tree_Node_Pos(grandChild, pos2);
      } else {
        Swc_Tree_Node_Pos(child, pos2);
      }
    }
  } else {
    Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
    Swc_Tree_Node *grandParent = SwcTreeNode::parent(parent);

    if (SwcTreeNode::isRegular(grandParent)) {
      Swc_Tree_Node_Pos(grandParent, pos2);
    } else {
      Swc_Tree_Node_Pos(parent, pos2);
    }
  }
}

double ZSwcTerminalAngleMetric::measureDistance(
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

  double pos1[3] = {0, 0, 0};
  double pos2[3] = {0, 0, 0};
  double pos3[3] = {0, 0, 0};
  double pos4[3] = {0, 0, 0};


  for (std::vector<Swc_Tree_Node*>::const_iterator iter1 = leafArray1.begin();
       iter1 != leafArray1.end(); ++iter1) {
    extractLeafDirection(*iter1, pos1, pos2);

    for (std::vector<Swc_Tree_Node*>::const_iterator iter2 = leafArray2.begin();
         iter2 != leafArray2.end(); ++iter2) {
      extractLeafDirection(*iter2, pos3, pos4);

      double a1 = acos(Coordinate_3d_Cos3(pos2, pos1, pos3));
      double a2 = acos(Coordinate_3d_Cos3(pos1, pos3, pos4));

      double dist = a1 + a2;

      if (m_useDistWeight) {
        double surfDist = SwcTreeNode::distance(*iter1, *iter2) + 1.0;
        dist *= surfDist;
      }
      if (dist < minDist) {
        minDist = dist;
        m_closestNodePair.first = *iter1;
        m_closestNodePair.second = *iter2;
      }
    }
  }

  return minDist;
}

void ZSwcTerminalAngleMetric::setDistanceWeight(bool enable)
{
  m_useDistWeight = enable;
}
