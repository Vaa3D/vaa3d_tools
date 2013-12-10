#include "zswclayershollfeatureanalyzer.h"

#include <iostream>
#include "swctreenode.h"
#include "zswctree.h"
#include "tz_error.h"

using namespace std;

ZSwcLayerShollFeatureAnalyzer::ZSwcLayerShollFeatureAnalyzer() :
  m_layerBaseFactor(1.0), m_layerScale(10.0), m_layerMargin(0.0)
{
}

std::vector<double> ZSwcLayerShollFeatureAnalyzer::computeFeature(
    Swc_Tree_Node *tn)
{
  TZ_ASSERT(tn != NULL, "null pointer");

  std::vector<double> featureArray(2, 0);

  ZSwcTree tree;
  tree.setDataFromNodeRoot(tn);
  tree.updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);

  double baseZ = SwcTreeNode::z(tn);
  featureArray[0] = baseZ;

  Swc_Tree_Node *iter = tree.begin();

  while (SwcTreeNode::isRoot(iter)) {
    iter = tree.next();
  }

  double upZ = baseZ - m_layerMargin;
  double downZ = baseZ + m_layerMargin;
  for (; iter != NULL; iter = iter->next) {
    Swc_Tree_Node *parent = iter->parent;
    double minZ = iter->node.z;
    double maxZ = parent->node.z;
    if (minZ > maxZ) {
      minZ = parent->node.z;
      maxZ = iter->node.z;
    }
    /*
    double r1 = SwcTreeNode::radius(iter);
    double r2 = SwcTreeNode::radius(parent);
    */
    if (minZ <= downZ && maxZ >= upZ) {
      featureArray[1] += 1.0;
    }
  }

#ifdef _DEBUG_
  if (featureArray[1] == 0.0) {
    cout << "debug here" << endl;
  }
#endif

  tree.setData(NULL, ZSwcTree::FREE_WRAPPER);

  return featureArray;
}

double ZSwcLayerShollFeatureAnalyzer::computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2)
{
  double layerDiff = fabs(featureArray1[0] - featureArray2[0]);

  double s1 = featureArray1[1];
  double s2 = featureArray2[1];
  if (s1 > s2) {
    double tmp;
    SWAP2(s1, s2, tmp);
  }
  /*
  double s2 = std::max(featureArray1[1], featureArray2[1]);
  double s1 = std::min(featureArray1[1], featureArray2[1]);
*/
  TZ_ASSERT(s1 > 0.0, "Invalid number");

  return sqrt(s1) * s1 / s2 /
      (layerDiff / m_layerScale + m_layerBaseFactor);
}

void ZSwcLayerShollFeatureAnalyzer::setParameter(
    const std::vector<double> &parameterArray)
{
  TZ_ASSERT(parameterArray.size() >= 2, "Too few parameters");

  m_layerBaseFactor = parameterArray[0];
  m_layerScale = parameterArray[1];
}
