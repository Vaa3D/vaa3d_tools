#include "zswcbranchingtrunkanalyzer.h"

#include "zswctree.h"
#include "swctreenode.h"
#include "tz_error.h"

ZSwcBranchingTrunkAnalyzer::ZSwcBranchingTrunkAnalyzer()
{
}

int ZSwcBranchingTrunkAnalyzer::compareSwcNode(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  if (SwcTreeNode::label(tn1) > SwcTreeNode::label(tn2)) {
    return 2;
  } else if (SwcTreeNode::label(tn1) < SwcTreeNode::label(tn2)) {
    return -2;
  } else {
    if (SwcTreeNode::radius(tn1) > SwcTreeNode::radius(tn2)) {
      return 1;
    } else if (SwcTreeNode::radius(tn1) < SwcTreeNode::radius(tn2)) {
      return -1;
    }
  }

  return 0;
}

ZSwcPath ZSwcBranchingTrunkAnalyzer::extractMainTrunk(ZSwcTree *tree)
{
  labelTraffic(tree);

  //Find the node with maximum label
  Swc_Tree_Node *tn = tree->maxLabelNode();

  SwcTreeNode::setAsRoot(tn);

  //Grow from the node with label / thickness preference
  Swc_Tree_Node* growingEnd[2] = {NULL, NULL};
  Swc_Tree_Node *child = tn->first_child;

  while (child != NULL) {
    if (growingEnd[0] == NULL) {
      growingEnd[0] = child;
    } else {
      if (compareSwcNode(child, growingEnd[0]) > 0) { //child is bigger
        growingEnd[1] = growingEnd[0];
        growingEnd[0] = child;
      } else {
        if (growingEnd[1] == NULL) {
          growingEnd[1] = child;
        } else if (compareSwcNode(child, growingEnd[1]) > 0) {
          growingEnd[1] = child;
        }
      }
    }

    child = child->next_sibling;
  }

  for (int i = 0; i < 2; i++) {
    if (growingEnd[i] != NULL) {
      while (growingEnd[i]->first_child != NULL) {
        child = growingEnd[i]->first_child;
        growingEnd[i] = child;
        while (child != NULL) {
          if (compareSwcNode(child, growingEnd[i]) > 0) {
            growingEnd[i] = child;
          }
          child = child->next_sibling;
        }

#ifdef _DEBUG_2
        if (SwcTreeNode::label(growingEnd[i]) * 2 < SwcTreeNode::label(tn)) {
          break;
        }
#endif
      }
    }
  }

  return ZSwcPath(growingEnd[0], growingEnd[1]);
}

ZSwcPath ZSwcBranchingTrunkAnalyzer::extractTrunk(ZSwcTree *tree,
                                                  Swc_Tree_Node *start)
{
  TZ_ERROR(ERROR_PART_FUNC);

  UNUSED_PARAMETER(tree);
  UNUSED_PARAMETER(start);

  return ZSwcPath(NULL, NULL);
}
