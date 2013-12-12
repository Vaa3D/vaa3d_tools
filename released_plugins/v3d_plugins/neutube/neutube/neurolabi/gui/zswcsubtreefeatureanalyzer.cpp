#include "zswcsubtreefeatureanalyzer.h"
#include "zswctree.h"
#include "swctreenode.h"

ZSwcSubTreeFeatureAnalyzer::ZSwcSubTreeFeatureAnalyzer()
{
}

//thickness, overall length, volume of bound box
std::vector<double> ZSwcSubTreeFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  std::vector<double> featureArray(3);

  ZSwcTree tree;
  tree.setDataFromNodeRoot(tn);

  tree.updateIterator(0, tn, SWC_TREE_ITERATOR_DEPTH_FIRST);

  int count = 0;
  for (tn = tree.begin(); tn != NULL; tn = tree.next()) {
    featureArray[0] += SwcTreeNode::radius(tn);
    ++count;
  }

  featureArray[0] /= count;

  return featureArray;
}

double ZSwcSubTreeFeatureAnalyzer::computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2)
{
  double dist = 0.0;
  for (size_t i = 0; i < featureArray1.size(); ++i) {
    double df = featureArray1[i] - featureArray2[i];
    dist += df * df;
  }

  return sqrt(dist);
}

void ZSwcSubTreeFeatureAnalyzer::setParameter(
    const std::vector<double> &/*parameterArray*/)
{
}
