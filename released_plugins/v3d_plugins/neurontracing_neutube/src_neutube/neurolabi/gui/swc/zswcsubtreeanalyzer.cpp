#include "zswcsubtreeanalyzer.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zswctreenodearray.h"
#include "tz_error.h"

ZSwcSubtreeAnalyzer::ZSwcSubtreeAnalyzer() :
  m_minLength(100.0), m_maxLength(1000.0)
{
}

int ZSwcSubtreeAnalyzer::labelSubtree(ZSwcTree *tree) const
{
  tree->setLabel(0);
  tree->computeBackTraceLength();

  ZSwcTreeNodeArray nodeArray =
      tree->getSwcTreeNodeArray(ZSwcTree::DEPTH_FIRST_ITERATOR);

  //Sort the node by weight
  nodeArray.sortByWeight();

  int subtreeId = 0;
  for (size_t i = 0; i < nodeArray.size(); ++i) {
    Swc_Tree_Node *tn = nodeArray[i];
#ifdef _DEBUG_2
    std::cout << "Weight of node " << SwcTreeNode::id(tn) << ": "
              << SwcTreeNode::weight(tn) << std::endl;
#endif
    if (SwcTreeNode::weight(tn) >= m_minLength && SwcTreeNode::isRegular(tn)) {
      TZ_ASSERT(SwcTreeNode::label(tn) == 0, "Invalid label");
      //tree->addLabelSubtree(tn, 1);
       ++subtreeId;
#ifdef _DEBUG_2
      std::cout << "Subtree root: " << SwcTreeNode::id(tn) << std::endl;
#endif
      tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, tn, 0);
      for (Swc_Tree_Node *iter = tree->begin(); iter != NULL; iter = tree->next()) {
        if (SwcTreeNode::label(iter) == 0) {
          SwcTreeNode::setLabel(iter, subtreeId);
#ifdef _DEBUG_2
          std::cout << "Label node " << SwcTreeNode::id(iter) << " by "
                    << subtreeId << std::endl;
#endif
        }
      }

      //Subtract weight from upstream nodes
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
      while (parent != NULL) {
        SwcTreeNode::addWeight(
              parent, -SwcTreeNode::weight(tn) - SwcTreeNode::length(tn));
        parent = SwcTreeNode::parent(parent);
      }
    }
  }

  return subtreeId;
}

int ZSwcSubtreeAnalyzer::decompose(ZSwcTree *tree) const
{
  tree->forceVirtualRoot();
  tree->setLabel(0);
  tree->computeBackTraceLength();

  ZSwcTreeNodeArray nodeArray =
      tree->getSwcTreeNodeArray(ZSwcTree::DEPTH_FIRST_ITERATOR);

  //Sort the node by weight
  nodeArray.sortByWeight();

  int subtreeId = 0;
  for (size_t i = 0; i < nodeArray.size(); ++i) {
    Swc_Tree_Node *tn = nodeArray[i];
#ifdef _DEBUG_2
    std::cout << "Weight of node " << SwcTreeNode::id(tn) << ": "
              << SwcTreeNode::weight(tn) << std::endl;
#endif
    if (SwcTreeNode::weight(tn) >= m_minLength && SwcTreeNode::isRegular(tn)) {
      TZ_ASSERT(SwcTreeNode::label(tn) == 0, "Invalid label");
      //tree->addLabelSubtree(tn, 1);
       ++subtreeId;
#ifdef _DEBUG_2
      std::cout << "Subtree root: " << SwcTreeNode::id(tn) << std::endl;
#endif
      tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, tn, 0);
      for (Swc_Tree_Node *iter = tree->begin(); iter != NULL; iter = tree->next()) {
        if (SwcTreeNode::label(iter) == 0) {
          SwcTreeNode::setLabel(iter, subtreeId);
#ifdef _DEBUG_2
          std::cout << "Label node " << SwcTreeNode::id(iter) << " by "
                    << subtreeId << std::endl;
#endif
        }
      }

      //Subtract weight from upstream nodes
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
      while (parent != NULL) {
        SwcTreeNode::addWeight(
              parent, -SwcTreeNode::weight(tn) - SwcTreeNode::length(tn));
        parent = SwcTreeNode::parent(parent);
      }

      //Detach
      SwcTreeNode::setParent(tn, tree->root());
    }
  }

  return subtreeId;
}
