#include "zswcdeepanglemetric.h"
#include "zerror.h"
#include "tz_error.h"
#include "zswctree.h"

ZSwcDeepAngleMetric::ZSwcDeepAngleMetric() : m_level(1), m_minDist(100.0)
{
}

double ZSwcDeepAngleMetric::computeAngle(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
    const Swc_Tree_Node *tn3, const Swc_Tree_Node *tn4)
{
  if (tn1 == NULL || tn2 == NULL || tn3 == NULL || tn4 == NULL) {
    RECORD_WARNING_UNCOND("Null pointer");
    return 0.0;
  }

  double pos1[3] = {0, 0, 0};
  double pos2[3] = {0, 0, 0};
  double pos3[3] = {0, 0, 0};
  double pos4[3] = {0, 0, 0};

  Swc_Tree_Node_Pos(tn1, pos1);
  Swc_Tree_Node_Pos(tn2, pos2);
  Swc_Tree_Node_Pos(tn3, pos3);
  Swc_Tree_Node_Pos(tn4, pos4);

  double a1 = acos(Coordinate_3d_Cos3(pos1, pos2, pos3));
  double a2 = acos(Coordinate_3d_Cos3(pos2, pos3, pos4));

  return a1 + a2;
}

double ZSwcDeepAngleMetric::computeDeepAngle(
    const std::vector<const Swc_Tree_Node *> &nodeArray1,
    const std::vector<const Swc_Tree_Node *> &nodeArray2)
{
  if (nodeArray1.empty() || nodeArray2.empty()) {
    return Infinity;
  }

  if (nodeArray1.size() <= 1 || nodeArray2.size() <= 1) {
    if (SwcTreeNode::distance(
          nodeArray1[0], nodeArray2[0], SwcTreeNode::EUCLIDEAN_SURFACE) <=
        m_minDist) {
      return 0.0;
    } else {
      return Infinity;
    }
  }

  size_t s1 = nodeArray1.size() - 1;
  size_t s2 = nodeArray2.size() - 1;

  double minDist = Infinity;

  for (size_t i = 0; i < s1; ++i) {
    for (size_t j = 0; j < s2; ++j) {
      if (SwcTreeNode::distance(
            nodeArray1[i], nodeArray2[i], SwcTreeNode::EUCLIDEAN_SURFACE) <=
          m_minDist) {
        double dist = computeAngle(nodeArray1[i + 1], nodeArray1[i],
            nodeArray2[i], nodeArray2[i + 1]);
        if (dist < minDist) {
          minDist = dist;
        }
      }
    }
  }

  return minDist;
}

double ZSwcDeepAngleMetric::measureDistance(
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
    std::vector<const Swc_Tree_Node*> seg1 = extractLeafSegment(*iter1);
    for (std::vector<Swc_Tree_Node*>::const_iterator iter2 = leafArray2.begin();
         iter2 != leafArray2.end(); ++iter2) {
      std::vector<const Swc_Tree_Node*> seg2 = extractLeafSegment(*iter2);

      double dist = computeDeepAngle(seg1, seg2);
      if (dist < minDist) {
        minDist = dist;
        m_closestNodePair.first = *iter1;
        m_closestNodePair.second = *iter2;
      }
    }
  }

  return minDist;
}

std::vector<const Swc_Tree_Node*> ZSwcDeepAngleMetric::extractLeafSegment(
    const Swc_Tree_Node *tn)
{
  TZ_ASSERT(SwcTreeNode::isRegular(tn), "Invalid node");

  std::vector<const Swc_Tree_Node*> nodeArray;
  nodeArray.push_back(tn);
  if (SwcTreeNode::isRoot(tn)) {
    for (int i = 0; i < m_level; ++i) {
      tn = SwcTreeNode::firstChild(tn);
      if (tn == NULL) {
        break;
      }
      nodeArray.push_back(tn);
    }
  } else {
    for (int i = 0; i < m_level; ++i) {
      tn = SwcTreeNode::parent(tn);
      if (!SwcTreeNode::isRegular(tn)) {
        break;
      }
      nodeArray.push_back(tn);
    }
  }

  return nodeArray;
}
