#ifndef ZSWCSPATIALFEATUREANALYZER_H
#define ZSWCSPATIALFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcSpatialFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcSpatialFeatureAnalyzer();

  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);

};

#endif // ZSWCSPATIALFEATUREANALYZER_H
