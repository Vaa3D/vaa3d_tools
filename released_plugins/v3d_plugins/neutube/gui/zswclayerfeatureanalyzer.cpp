#include "zswclayerfeatureanalyzer.h"

#include "tz_error.h"
#include "swctreenode.h"
#include "tz_unipointer_linked_list.h"

using namespace std;

ZSwcLayerFeatureAnalyzer::ZSwcLayerFeatureAnalyzer() : m_layerBaseFactor(1.0),
  m_layerScale(10.0)
{
}

double ZSwcLayerFeatureAnalyzer::computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2)
{
  double layerDiff = fabs(featureArray1[0] - featureArray2[0]);

  double s2 = max(featureArray1[1], featureArray2[1]);
  double s1 = min(featureArray1[1], featureArray2[1]);

  TZ_ASSERT(s1 > 0.0, "Invalid number");

  //return maxSize - ((s2 - s1) + s1 * (log(s1 / s2)));

  return sqrt(s1) * s1 / s2 /
      (layerDiff / m_layerScale + m_layerBaseFactor);
}

std::vector<double> ZSwcLayerFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  std::vector<double> featureArray(2, 0);


  if (tn != NULL) {
    featureArray[0] = SwcTreeNode::z(tn);
    std::vector<double> sizeFeature = m_sizeFeatureAnalyzer.computeFeature(tn);
    featureArray[1] = sizeFeature[0];
  }

  return featureArray;
}

void ZSwcLayerFeatureAnalyzer::setParameter(const std::vector<double> &parameterArray)
{
  TZ_ASSERT(parameterArray.size() >= 4, "Too few parameters");

  m_sizeFeatureAnalyzer.setParameter(parameterArray);
  m_layerBaseFactor = parameterArray[2];
  m_layerScale = parameterArray[3];
}
