#include "zswcdisttrunkanalyzer.h"

#include <vector>
#include "zswctree.h"

using namespace std;

ZSwcDistTrunkAnalyzer::ZSwcDistTrunkAnalyzer() : m_geodesicWeight(0.2),
  m_euclideanWeight(0.8)
{
}

ZSwcPath ZSwcDistTrunkAnalyzer::extractMainTrunk(ZSwcTree *tree)
{
  tree->updateIterator(SWC_TREE_ITERATOR_LEAF);

  vector<Swc_Tree_Node*> leafArray;
  leafArray.push_back(tree->firstRegularRoot());

  for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end();
       tn = tree->next()) {
    leafArray.push_back(tn);
  }

  Swc_Tree_Node *leaf1 = NULL;
  Swc_Tree_Node *leaf2 = NULL;
  double maxLength = -1.0;

  //Calculate the distance of all the node to the regular root
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, true);

  double *distanceArray = NULL;

  if (m_geodesicWeight != 0.0) {
    distanceArray = Swc_Tree_Accm_Length(tree->data(), NULL);
  }

  //Calculate distances for each pair of leaves
  for (size_t i = 0; i < leafArray.size(); i++) {
#ifdef _DEBUG_2
    cout << i << " / " << leafArray.size() << endl;
#endif
    for (size_t j = 0; j < leafArray.size(); j++) {
      if (leafArray[i] != leafArray[j]) {
        double geodesicDistance = 0.0;
        if (distanceArray != NULL) {
          //Find the common ancestor of the leaves
          Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(leafArray[i],
                                                                leafArray[j]);
          geodesicDistance = distanceArray[SwcTreeNode::index(leafArray[i])] +
              distanceArray[SwcTreeNode::index(leafArray[j])] -
              2.0 * distanceArray[SwcTreeNode::index(ancestor)];
        }

        double length = geodesicDistance * m_geodesicWeight +
            SwcTreeNode::distance(leafArray[i], leafArray[j]) * m_euclideanWeight;
        if (length > maxLength) {
          maxLength = length;
          leaf1 = leafArray[i];
          leaf2 = leafArray[j];
        }
      }
    }
  }

  if (distanceArray != NULL) {
    free(distanceArray);
  }

  /*
  if (SwcTreeNode::z(leaf1) > SwcTreeNode::z(leaf2)) {
    return ZSwcPath(leaf2, leaf1);
  }
  */

  return ZSwcPath(leaf1, leaf2);
}

ZSwcPath ZSwcDistTrunkAnalyzer::extractTrunk(ZSwcTree *tree,
                                             Swc_Tree_Node *start)
{
  tree->updateIterator(SWC_TREE_ITERATOR_LEAF, start, m_blocker);

  vector<Swc_Tree_Node*> leafArray;

  for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end();
       tn = tree->next()) {
    leafArray.push_back(tn);
  }

  Swc_Tree_Node *leaf1 = start;
  Swc_Tree_Node *leaf2 = NULL;
  double maxLength = -1.0;

  for (size_t i = 0; i < leafArray.size(); i++) {
    double length = 0.0;
    Swc_Tree_Node *tn = leafArray[i];
    while (tn != start) {
      if (m_blocker.count(tn) > 0) {
        tn = NULL;
        break;
      } else {
        length += Swc_Tree_Node_Length(tn);
      }

      tn = tn->parent;
    }

    length = length * m_geodesicWeight +
        SwcTreeNode::distance(start, leafArray[i]) * m_euclideanWeight;

    if (tn != NULL) {
      if (length > maxLength) {
        leaf2 = leafArray[i];
        maxLength = length;
      }
    }
  }

  return ZSwcPath(leaf1, leaf2);
}
/*
void ZSwcDistTrunkAnalyzer::labelTraffic(ZSwcTree *tree)
{
  ZSwcTrunkAnalyzer::labelTraffic(tree);
}
*/
