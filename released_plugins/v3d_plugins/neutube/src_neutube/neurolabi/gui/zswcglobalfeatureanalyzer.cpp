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
    featureSet.resize(9, 0.0);
    if (tree.isEmpty()) {
      break;
    }
    int leafCount = 0;
    int branchPointCount = 0;
    double boxVolume = 0;
    double averageRadius = 0;
    double radiusVariance = 0;
    double count = 0;
    double maxPathLength = 0.0;
    double maxSegmentLength = 0.0;
    double averageCurvature = 0.0;
    int curvatureCount = 0;

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
        radiusVariance += SwcTreeNode::radius(tn) * SwcTreeNode::radius(tn);
        ++count;

        if (SwcTreeNode::isContinuation(tn)) {
          double a = SwcTreeNode::distance(tn, SwcTreeNode::parent(tn));
          double b = SwcTreeNode::distance(tn, SwcTreeNode::firstChild(tn));
          double c = SwcTreeNode::distance(
                SwcTreeNode::parent(tn), SwcTreeNode::firstChild(tn));
          if (a > 0.0 && b > 0.0 && c > 0.0) {
            ++curvatureCount;
            double u = b + c - a;
            double v = a + c - b;
            double w = a + b - c;
            if (u > 0.0 && v > 0.0 && w > 0.0) {
              averageCurvature +=
                  sqrt(a + b + c) / a * sqrt(u) / b * sqrt(v) / c * sqrt(w);
            }
          }
        }
      }
    }
    averageRadius /= count;
    radiusVariance = radiusVariance / count - averageRadius * averageRadius;

    ZCuboid box = tree.boundBox();
    boxVolume = box.volume();

    ZSwcDistTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.setDistanceWeight(0.0, 1.0);
    ZSwcPath path = trunkAnalyzer.extractMainTrunk(&tree);
    maxPathLength = path.getLength();

    maxSegmentLength = tree.getMaxSegmentLenth();

    double lvRatio = sqrt(box.width() * box.width() + box.height() * box.height()) /
        box.depth();

    if (curvatureCount > 0) {
      averageCurvature /= curvatureCount;
    }

    featureSet[0] = leafCount;
    featureSet[1] = branchPointCount;
    featureSet[2] = boxVolume;
    featureSet[3] = maxSegmentLength;
    featureSet[4] = maxPathLength;
    featureSet[5] = averageRadius;
    featureSet[6] = radiusVariance;
    featureSet[7] = lvRatio;
    featureSet[8] = averageCurvature;
  }
    break;
  default:
    break;
  }

  return featureSet;
}
