#include "zswclayertrunkanalyzer.h"

#include "zswctree.h"
#include "swctreenode.h"

using namespace std;

ZSwcLayerTrunkAnalyzer::ZSwcLayerTrunkAnalyzer() : m_step(10.0)
{
}

ZSwcPath ZSwcLayerTrunkAnalyzer::extractMainTrunk(ZSwcTree *tree)
{
  //Extract all nodes
  //vector<Swc_Tree_Node*> nodeArray = tree->toSwcTreeNodeArray(false);
  const vector<Swc_Tree_Node*> &nodeArray =
      tree->getSwcTreeNodeArray(ZSwcTree::Z_SORT_ITERATOR);

  //vector<Swc_Tree_Node*>::iterator firstIter = nodeArray->begin();

  size_t nodeIndex = 0;

  if (SwcTreeNode::isVirtual(nodeArray[0])) {
    ++nodeIndex;
  }

  double currentZ = SwcTreeNode::z(nodeArray[nodeIndex]);

  ZSwcPath path;

  //For each node
  while (nodeIndex < nodeArray.size()) {
    bool isValid = true;
    if (nodeIndex > 0) {
      //If the previous one is closer, increase Z without moving forward
      if (fabs(SwcTreeNode::z(nodeArray[nodeIndex]) - currentZ) >
          fabs(SwcTreeNode::z(nodeArray[nodeIndex - 1]) - currentZ)) {
        isValid = false;
        currentZ += m_step;
      }
    }

    if (isValid) {
      //If the next one is closer, move forward
      if (nodeIndex + 1 < nodeArray.size()) {
        if (fabs(SwcTreeNode::z(nodeArray[nodeIndex]) - currentZ) >=
            fabs(SwcTreeNode::z(nodeArray[nodeIndex + 1]) - currentZ)) {
          isValid = false;
          ++nodeIndex;
        }
      }
    }

    if (isValid) {
      path.append(nodeArray[nodeIndex]);
      ++nodeIndex;
      currentZ += m_step;
    }
  }

  path.setHostTree(tree);

  return path;
}

ZSwcPath ZSwcLayerTrunkAnalyzer::extractTrunk(
    ZSwcTree *tree, Swc_Tree_Node *start)
{
  UNUSED_PARAMETER(tree);
  UNUSED_PARAMETER(start);

  return ZSwcPath();
}
