#ifndef ZSWCSUBTREEFEATUREANALYZER_H
#define ZSWCSUBTREEFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcSubTreeFeatureAnalyzer : ZSwcFeatureAnalyzer
{
public:
  ZSwcSubTreeFeatureAnalyzer();

  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);
};

#endif // ZSWCSUBTREEFEATUREANALYZER_H
