#include "zswcnodedistselector.h"

#include "zswctree.h"

ZSwcNodeDistSelector::ZSwcNodeDistSelector()
{
}

ZSwcTreeNodeArray ZSwcNodeDistSelector::select(const ZSwcTree &tree) const
{
  tree.setLabel(0);
  ZSwcTreeNodeArray nodeArray;

  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree.begin();
  if (SwcTreeNode::isVirtual(tn)) {
    tn = tree.next();
  }

  if (SwcTreeNode::isRegular(tn)) {
    nodeArray.push_back(tn);
    SwcTreeNode::setLabel(tn, 1);
  }

  for (; tn != NULL; tn = tree.next()) {
    double dist = 0.0;
    Swc_Tree_Node *parent = tn;
    while (parent != NULL) {
      dist += SwcTreeNode::length(parent);
      if (dist > m_minDistance) {
        nodeArray.push_back(tn);
        SwcTreeNode::setLabel(tn, 1);
      }
      parent = SwcTreeNode::parent(parent);
      if ((parent == NULL) || SwcTreeNode::label(parent) > 0) {
        break;
      }
    }
  }

#if 0
#ifdef _DEBUG_2
  tree.print();
#endif

  tree.updateIterator(SWC_TREE_ITERATOR_LEAF);

  for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
    nodeArray.push_back(tn);
    SwcTreeNode::setLabel(tn, 2);
    double dist = 0;
    Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
    if (SwcTreeNode::label(parent) > 0 || SwcTreeNode::isVirtual(parent)) {
      continue;
    }

    dist += SwcTreeNode::length(tn);
    while (SwcTreeNode::isRegular(parent)) {
      SwcTreeNode::setLabel(parent, 1);
      if (dist >= m_minDistance) {
        nodeArray.push_back(parent);
        dist = 0;
        SwcTreeNode::setLabel(parent, 2);
      }
      dist += SwcTreeNode::length(parent);

      parent = SwcTreeNode::parent(parent);
      if (SwcTreeNode::label(parent) > 0) {
        break;
      }
    }
  }
#endif

  return nodeArray;
}
