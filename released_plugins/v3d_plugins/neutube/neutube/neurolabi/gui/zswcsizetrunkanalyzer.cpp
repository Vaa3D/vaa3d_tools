#include "zswcsizetrunkanalyzer.h"

#include "zswctree.h"
#include "swctreenode.h"

ZSwcSizeTrunkAnalyzer::ZSwcSizeTrunkAnalyzer()
{
}

ZSwcPath ZSwcSizeTrunkAnalyzer::extractMainTrunk(ZSwcTree *tree)
{
  return extractTrunk(tree, tree->firstRegularRoot());
}

ZSwcPath ZSwcSizeTrunkAnalyzer::extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start)
{
  UNUSED_PARAMETER(tree);

  Swc_Tree_Node *end = start;
  Swc_Tree_Node *tn = start;

  while (tn != NULL) {
    Swc_Tree_Node *child = tn->first_child;
    Swc_Tree_Node *result = NULL;
    while (child != NULL) {
      if (m_blocker.count(child) == 0) {
        if (result == NULL ||
            SwcTreeNode::radius(result) < SwcTreeNode::radius(child)) {
          result = child;
        }
      }
      child = child->next_sibling;
    }

    if (result != NULL) {
      end = result;
    }

    tn = result;
  }

  return ZSwcPath(start, end);
}
