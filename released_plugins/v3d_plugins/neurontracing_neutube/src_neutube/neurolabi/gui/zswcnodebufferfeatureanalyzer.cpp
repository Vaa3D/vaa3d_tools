#include "zswcnodebufferfeatureanalyzer.h"
#include "swctreenode.h"
#include "zerror.h"

ZSwcNodeBufferFeatureAnalyzer::ZSwcNodeBufferFeatureAnalyzer() : m_helper(NULL)
{
}

void ZSwcNodeBufferFeatureAnalyzer::setHelper(ZSwcFeatureAnalyzer *helper)
{
  m_helper = helper;
}

std::vector<double> ZSwcNodeBufferFeatureAnalyzer::computeFeature(
    Swc_Tree_Node *tn)
{
  std::vector<double> feature(2);

  if (m_helper != NULL) {
    if (SwcTreeNode::feature(tn) == 0.0) {
      feature = m_helper->computeFeature(tn);
      if (feature.size() < 2) {
        RECORD_WARNING_UNCOND("Incompatible feature size");
      } else {
        SwcTreeNode::setFeature(tn, feature[1]);
        SwcTreeNode::setWeight(tn, feature[0]);
      }
    }
  } else {
    RECORD_WARNING_UNCOND("Null helper");
  }

  feature[1] = SwcTreeNode::feature(tn);
  feature[0] = SwcTreeNode::weight(tn);

  return feature;
}

double ZSwcNodeBufferFeatureAnalyzer::computeFeatureSimilarity(
    const std::vector<double> &featureArray1,
    const std::vector<double> &featureArray2)
{
  if (m_helper != NULL) {
    return m_helper->computeFeatureSimilarity(featureArray1, featureArray2);
  } else {
    RECORD_WARNING_UNCOND("Null helper");
  }

  return 0.0;
}

void ZSwcNodeBufferFeatureAnalyzer::setParameter(
    const std::vector<double> &/*parameterArray*/)
{

}
