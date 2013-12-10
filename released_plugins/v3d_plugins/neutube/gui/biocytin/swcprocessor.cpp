#include "swcprocessor.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "tz_math.h"

Biocytin::SwcProcessor::SwcProcessor()
{
}

void Biocytin::SwcProcessor::assignZ(ZSwcTree *tree, const Stack &depthImage)
{
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
    if (SwcTreeNode::isRegular(tn)) {
      int x = iround(SwcTreeNode::x(tn));
      int y = iround(SwcTreeNode::y(tn));

      CLIP_VALUE(x, 0, C_Stack::width(&depthImage) - 1);
      CLIP_VALUE(y, 0, C_Stack::height(&depthImage) - 1);

      SwcTreeNode::setZ(tn, C_Stack::value(&depthImage, x, y, 0, 0));
    }
  }
}

void Biocytin::SwcProcessor::removeZJump(ZSwcTree *tree, double minDeltaZ)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      double deltaZ = SwcTreeNode::z(tn) - SwcTreeNode::z(SwcTreeNode::parent(tn));
      if (deltaZ > minDeltaZ) {
        SwcTreeNode::detachParent(tn);
        SwcTreeNode::adoptChildren(tree->root(), tn);
        SwcTreeNode::kill(tn);
      }
    }
    tn = next;
  }
}

void Biocytin::SwcProcessor::breakZJump(ZSwcTree *tree, double minDeltaZ)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      double deltaZ =
          fabs(SwcTreeNode::z(tn) - SwcTreeNode::z(SwcTreeNode::parent(tn)));
      if (deltaZ > minDeltaZ) {
        SwcTreeNode::setParent(tn, tree->root());
      }
    }
    tn = next;
  }
}

void Biocytin::SwcProcessor::removeOrphan(ZSwcTree *tree)
{
  if (SwcTreeNode::isVirtual(tree->root())) {
    Swc_Tree_Node *root = SwcTreeNode::firstChild(tree->root());
    while (root != NULL) {
      Swc_Tree_Node *next = SwcTreeNode::nextSibling(root);
      if (!SwcTreeNode::hasChild(root)) {
        SwcTreeNode::detachParent(root);
        SwcTreeNode::kill(root);
      }
      root = next;
    }
  }
}

void Biocytin::SwcProcessor::smoothZ(ZSwcTree *tree)
{
  const std::vector<Swc_Tree_Node*> &leafArray =
      tree->getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR);
  tree->setLabel(0);
  for (std::vector<Swc_Tree_Node*>::const_iterator iter =
       leafArray.begin(); iter != leafArray.end(); ++iter) {
    Swc_Tree_Node *endTn = *iter;
    while (endTn != NULL) {
      if (SwcTreeNode::isRegularRoot(endTn) ||
          SwcTreeNode::label(endTn) == 1) {
        break;
      }
      endTn = endTn->parent;
    }
    ZSwcPath path(*iter, endTn);
    path.smoothZ();
    path.label(1);
  }
}

void Biocytin::SwcProcessor::smoothRadius(ZSwcTree *tree)
{
  const std::vector<Swc_Tree_Node*> &leafArray =
      tree->getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR);
  tree->setLabel(0);
  for (std::vector<Swc_Tree_Node*>::const_iterator iter =
       leafArray.begin(); iter != leafArray.end(); ++iter) {
    Swc_Tree_Node *endTn = *iter;
    while (endTn != NULL) {
      if (SwcTreeNode::isRegularRoot(endTn) ||
          SwcTreeNode::label(endTn) == 1) {
        break;
      }
      endTn = endTn->parent;
    }
    ZSwcPath path(*iter, endTn);
    path.smoothRadius(true);
    path.label(1);
  }
}
