#include "swc/zswcresampler.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "tz_error.h"

ZSwcResampler::ZSwcResampler()
{
}

int ZSwcResampler::suboptimalDownsample(ZSwcTree *tree)
{
  int count = 0;
  int treeCount = tree->updateIterator();
  int checked = 0;

  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;

    //std::cout << "before: " << next <<std::endl;

    ++checked;
    if (SwcTreeNode::isContinuation(tn)) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
      Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);

      bool redundant = false;
      //bool mergingToChild = false;
      if (SwcTreeNode::isWithin(tn, parent) || SwcTreeNode::isWithin(tn, child)) {
        redundant = true;
        /*
        if (SwcTreeNode::isWithin(tn, child)) {
          mergingToChild = true;
        }
        */
      }

      if (!redundant) {
        redundant = isInterRedundant(tn, parent);
      }

      if (redundant) {
        next = next->next;
        /*
        if (mergingToChild) {
          SwcTreeNode::copyProperty(tn, parent);
        }
        */

        SwcTreeNode::mergeToParent(tn);
        ++count;
      }
    }

    tn = next;
  }

  std::cout << count << " removed" << std::endl;
  std::cout << checked << " " << treeCount << std::endl;

  return count;
}

void ZSwcResampler::optimalDownsample(ZSwcTree *tree)
{
  int n = 0;
  while (suboptimalDownsample(tree) > 0) {
    std::cout << "iter: " << ++n << std::endl;
  }
  optimizeCriticalParent(tree);
}

int ZSwcResampler::optimizeCriticalParent(ZSwcTree *tree)
{
  //Check nodes connected to leaves
  tree->updateIterator();

  Swc_Tree_Node *tn = tree->begin();

  int count = 0;

  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;

    if ((SwcTreeNode::isBranchPoint(tn) || SwcTreeNode::isLeaf(tn)) &&
        !SwcTreeNode::isRoot(tn)) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);

      bool redundant = false;

      if (SwcTreeNode::isContinuation(parent)) {
        //Set leaf to be the same as its parent if it's covered by the parent
        if (SwcTreeNode::isWithin(tn, parent)) {
          SwcTreeNode::copyProperty(parent, tn);
          redundant = true;
        }

        if (!redundant) {
          if (SwcTreeNode::isWithin(parent, tn)) {
            redundant = true;
          }
        }

        if (!redundant) {
          redundant = isInterRedundant(parent, tn);
        }
      }

      if (redundant) {
        TZ_ASSERT(!SwcTreeNode::isRoot(parent), "Invalid node");
        SwcTreeNode::mergeToParent(parent);
        ++count;
      }
    }

    tn = next;
  }

  std::cout << count << " critical removed" << std::endl;

  return count;
}

bool ZSwcResampler::isInterRedundant(
    const Swc_Tree_Node *tn, const Swc_Tree_Node *master) const
{
  bool redundant = false;

  Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
  Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);

  if (parent == master || child == master) {
    if (SwcTreeNode::isContinuation(tn) && SwcTreeNode::hasOverlap(tn, master)) {
      double d1 = SwcTreeNode::distance(tn, parent);
      double d2 = SwcTreeNode::distance(tn, child);
      double lambda = d2 / (d1 + d2);
      Swc_Tree_Node tmpNode;
      SwcTreeNode::setDefault(&tmpNode);
      SwcTreeNode::interpolate(parent, child, lambda, &tmpNode);

      TZ_ASSERT(SwcTreeNode::isRegular(&tmpNode), "Unexpected virtual node");

      double sizeScale = 1.2;
      if (SwcTreeNode::distance(tn, &tmpNode) * 2.0 <
          SwcTreeNode::radius(&tmpNode)) { //not too far away
        if ((SwcTreeNode::radius(tn) * sizeScale > SwcTreeNode::radius(&tmpNode)) &&
            (SwcTreeNode::radius(tn) < SwcTreeNode::radius(&tmpNode) * sizeScale)) {
          redundant = true;
        }
      }
    }
  }

  return redundant;
}
