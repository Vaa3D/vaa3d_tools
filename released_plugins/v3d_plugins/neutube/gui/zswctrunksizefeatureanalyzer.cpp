#include "zswctrunksizefeatureanalyzer.h"

#include <vector>
#include "zswctree.h"
#include "swctreenode.h"

using namespace std;

ZSwcTrunkSizeFeatureAnalyzer::ZSwcTrunkSizeFeatureAnalyzer()
{
}

ZSwcTrunkSizeFeatureAnalyzer::~ZSwcTrunkSizeFeatureAnalyzer()
{

}

std::vector<double> ZSwcTrunkSizeFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  ZSwcTree tree;
  tree.setDataFromNodeRoot(tn);

  tree.labelTrunkLevel(m_trunkAnalyzer);

  vector<double> featureArray(1);
  featureArray[0] =
      SwcTreeNode::downstreamSize(tn, SwcTreeNode::labelDifference);

  return featureArray;
}

double ZSwcTrunkSizeFeatureAnalyzer::computeFeatureSimilarity(
    const vector<double> &featureArray1, const vector<double> &featureArray2)
{
  //double maxSize = 10000;

  double s2 = max(featureArray1[0], featureArray2[0]);
  double s1 = min(featureArray1[0], featureArray2[0]);

  //return maxSize - ((s2 - s1) + s1 * (log(s1 / s2)));

  return sqrt(s1) * s1 / s2;
}
