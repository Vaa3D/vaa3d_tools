#include "zswcglobalfeatureanalyzer.h"
#include "zswctreenodearray.h"
#include "swctreenode.h"
#include "zswcdisttrunkanalyzer.h"

ZSwcGlobalFeatureAnalyzer::ZSwcGlobalFeatureAnalyzer()
{
}

double ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(
    const ZSwcTree &tree)
{
  ZCuboid box =tree.boundBox();

  return sqrt(box.width() * box.width() + box.height() * box.height()) /
      box.depth();
}

//Number of leaves, number of branch points,
//box volume, maximum segment length, maximum path length
//average radius
std::vector<double> ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
    ZSwcTree &tree, EFeatureSet setName)
{
  std::vector<double> featureSet;
  switch (setName) {
  case NGF1:
  {
    featureSet.resize(6, 0.0);
    if (tree.isEmpty()) {
      break;
    }
    int leafCount = 0;
    int branchPointCount = 0;
    double boxVolume = 0;
    double averageRadius = 0;
    double count = 0;
    double maxPathLength = 0.0;
    double maxSegmentLength = 0.0;

    ZSwcTreeNodeArray nodeArray = tree.getSwcTreeNodeArray();
    for (ZSwcTreeNodeArray::const_iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;
      if (SwcTreeNode::isRegular(tn)) {
        if (SwcTreeNode::isLeaf(tn)) {
          ++leafCount;
        }
        if (SwcTreeNode::isBranchPoint(tn)) {
          ++branchPointCount;
        }
        if (SwcTreeNode::isTerminal(tn)) {
          //Check root branch point
        }
        averageRadius += SwcTreeNode::radius(tn);
        ++count;
      }
    }
    averageRadius /= count;

    boxVolume = tree.boundBox().volume();

    ZSwcDistTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.setDistanceWeight(0.0, 1.0);
    ZSwcPath path = trunkAnalyzer.extractMainTrunk(&tree);
    maxPathLength = path.getLength();

    maxSegmentLength = tree.getMaxSegmentLenth();

    featureSet[0] = leafCount;
    featureSet[1] = branchPointCount;
    featureSet[2] = boxVolume;
    featureSet[3] = maxSegmentLength;
    featureSet[4] = maxPathLength;
    featureSet[5] = averageRadius;
  }
    break;
  default:
    break;
  }

  return featureSet;
}
