#include "zswccurvaturefeatureanalyzer.h"
#include "swctreenode.h"

ZSwcCurvatureFeatureAnalyzer::ZSwcCurvatureFeatureAnalyzer() :
  m_neighborRange(100.0)
{
}

std::vector<double> ZSwcCurvatureFeatureAnalyzer::computeFeature(
    Swc_Tree_Node *tn)
{
  std::vector<double> featureArray(1, 0.0);

  if (!SwcTreeNode::isContinuation(tn)) {
    return featureArray;
  } else {
    Swc_Tree_Node *preTn = SwcTreeNode::parent(tn);
    Swc_Tree_Node *postTn = SwcTreeNode::firstChild(tn);
    double preLength = SwcTreeNode::distance(preTn, tn);
    while (preLength < m_neighborRange) {
      if (!SwcTreeNode::isBranchPoint(preTn) && !SwcTreeNode::isRoot(tn)) {
        preLength += SwcTreeNode::length(preTn);
        preTn = SwcTreeNode::parent(preTn);
      } else {
        break;
      }
    }

    double postLength = SwcTreeNode::distance(postTn, tn);
    while (postLength < m_neighborRange) {
      if (SwcTreeNode::isContinuation(postTn)) {
        postLength += SwcTreeNode::length(postTn);
        postTn = SwcTreeNode::firstChild(postTn);
      } else {
        break;
      }
    }
    featureArray[0] = SwcTreeNode::computeCurvature(preTn, tn, postTn);
  }

  return featureArray;
}

double ZSwcCurvatureFeatureAnalyzer::computeFeatureSimilarity(
    const std::vector<double> &featureArray1,
    const std::vector<double> &featureArray2)
{
  return fabs(featureArray1[0] - featureArray2[0]);
}

void ZSwcCurvatureFeatureAnalyzer::ZSwcCurvatureFeatureAnalyzer::setParameter(
    const std::vector<double> &parameterArray)
{
  m_neighborRange = parameterArray[0];
}
