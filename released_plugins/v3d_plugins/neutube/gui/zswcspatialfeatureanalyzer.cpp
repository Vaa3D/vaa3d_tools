#include "zswcspatialfeatureanalyzer.h"

#include "tz_geo3d_utils.h"

using namespace std;

ZSwcSpatialFeatureAnalyzer::ZSwcSpatialFeatureAnalyzer()
{
}

vector<double> ZSwcSpatialFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  vector<double> feature(3);
  feature[0] = Swc_Tree_Node_X(tn);
  feature[1] = Swc_Tree_Node_Y(tn);
  feature[2] = Swc_Tree_Node_Z(tn);

  return feature;
}

double ZSwcSpatialFeatureAnalyzer::computeFeatureSimilarity(
    const vector<double> &featureArray1,
    const vector<double> &featureArray2)
{
  return 1.0 / (1.0 + Geo3d_Dist_Sqr(featureArray1[0], featureArray1[1],
                                     featureArray1[2], featureArray2[0],
                                     featureArray1[1], featureArray1[2]));
}

void ZSwcSpatialFeatureAnalyzer::setParameter(
    const vector<double> &parameterArray)
{
  UNUSED_PARAMETER(parameterArray[0]);
}
