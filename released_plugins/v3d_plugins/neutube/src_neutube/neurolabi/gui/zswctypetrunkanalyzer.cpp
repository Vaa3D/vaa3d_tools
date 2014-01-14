#include "zswctypetrunkanalyzer.h"
#include "zswctree.h"
#include "swctreenode.h"

ZSwcTypeTrunkAnalyzer::ZSwcTypeTrunkAnalyzer() :
  m_trunkType(5), m_trunkAnalyzer(NULL)
{
}

ZSwcTypeTrunkAnalyzer::~ZSwcTypeTrunkAnalyzer()
{
  delete m_trunkAnalyzer;
}

ZSwcPath ZSwcTypeTrunkAnalyzer::extractMainTrunk(ZSwcTree *tree)
{
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  Swc_Tree_Node *beginTn = NULL;
  Swc_Tree_Node *endTn = NULL;
  for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
    if (SwcTreeNode::isRegular(tn)) {
      bool isTerminal = false;
      if (SwcTreeNode::type(tn) == m_trunkType) {
        int neighborCount = 0;
        Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
        if (SwcTreeNode::isRegular(parent)) {
          if (SwcTreeNode::type(parent) == m_trunkType) {
            ++neighborCount;
          }
        }
        Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);
        while (child != NULL) {
          if (SwcTreeNode::type(child) == m_trunkType) {
            ++neighborCount;
          }
          child = SwcTreeNode::nextSibling(child);
        }

        if (neighborCount <= 1) {
          isTerminal = true;
        }
      }

      if (isTerminal) {
        if (beginTn == NULL) {
          beginTn = tn;
        } else {
          endTn = tn;
          break;
        }
      }
    }
  }

  return ZSwcPath(beginTn, endTn);
}

ZSwcPath ZSwcTypeTrunkAnalyzer::extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start)
{
  if (m_trunkAnalyzer != NULL) {
    m_trunkAnalyzer->setBlocker(m_blocker);
    return m_trunkAnalyzer->extractTrunk(tree, start);
  }

  return ZSwcPath();
}
