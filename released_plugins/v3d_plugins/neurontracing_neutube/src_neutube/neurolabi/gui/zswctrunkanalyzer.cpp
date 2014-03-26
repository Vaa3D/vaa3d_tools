#include "zswctrunkanalyzer.h"

#include <vector>

#include "zswctree.h"
#include "swctreenode.h"

using namespace std;

ZSwcTrunkAnalyzer::ZSwcTrunkAnalyzer()
{
  m_geodesicWeight = 1.0;
  m_euclideanWeight = 15.0;
}

ZSwcTrunkAnalyzer::~ZSwcTrunkAnalyzer()
{

}

void ZSwcTrunkAnalyzer::addBlocker(Swc_Tree_Node *tn)
{
  m_blocker.insert(tn);
}

void ZSwcTrunkAnalyzer::clearBlocker()
{
  m_blocker.clear();
}

void ZSwcTrunkAnalyzer::labelTraffic(ZSwcTree *tree, ETrafficRule rule)
{
  tree->setLabel(0);

  switch (rule) {
  case FURTEST_PAIR:
  {
    vector<Swc_Tree_Node*> leafArray = tree->terminalArray();

    //Calculate the distance of all the node to the regular root
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, true);

    double *distanceArray = NULL;

    if (m_geodesicWeight != 0.0) {
      distanceArray = Swc_Tree_Accm_Length(tree->data(), NULL);
    }

    //Calculate distances for each pair of leaves
    for (size_t i = 0; i < leafArray.size(); i++) {
      Swc_Tree_Node *leaf1 = leafArray[i];
      Swc_Tree_Node *leaf2 = NULL;

      double maxLength = -1.0;

      for (size_t j = 0; j < leafArray.size(); j++) {
        if (leaf1 != leafArray[j]) {
          double geodesicDistance = 0.0;
          if (distanceArray != NULL) {
            //Find the common ancestor of the leaves
            Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(leaf1,
                                                                  leafArray[j]);
            geodesicDistance = distanceArray[SwcTreeNode::index(leaf1)] +
                distanceArray[SwcTreeNode::index(leafArray[j])] -
                2.0 * distanceArray[SwcTreeNode::index(ancestor)];
          }

          double length = geodesicDistance * m_geodesicWeight +
              SwcTreeNode::distance(leaf1, leafArray[j]) * m_euclideanWeight;

          if (length >= maxLength) {
            maxLength = length;
            leaf2 = leafArray[j];
          }
        }
      }

      if (leaf2 != NULL) {
#ifdef _DEBUG_2
        cout << SwcTreeNode::toString(leaf1) << endl;
        cout << SwcTreeNode::toString(leaf2) << endl;
        cout << "accum dist: " << distanceArray[SwcTreeNode::index(leaf1)]
             << " " << distanceArray[SwcTreeNode::index(leaf2)]
             << " "
             << distanceArray[SwcTreeNode::index(
                  SwcTreeNode::commonAncestor(leaf1, leaf2))] << endl;
        cout << "Length: " << maxLength << " "
             << SwcTreeNode::pathLength(leaf1, leaf2) << endl;
        if (leaf2 != leaf3) {
          cout << "debug here" << endl;
        }
#endif

        SwcTreeNode::addPathLabel(leaf1, leaf2, 1);
      }
    }

    if (distanceArray != NULL) {
      free(distanceArray);
    }
  }
    break;
  case REACH_ROOT:
  {
    tree->updateIterator(SWC_TREE_ITERATOR_LEAF);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      Swc_Tree_Node *parent = tn;
      while (parent != NULL) {
        SwcTreeNode::addLabel(parent, 1);
        parent = SwcTreeNode::parent(parent);
      }
    }
  }
    break;
  default:
    break;
  }
}
