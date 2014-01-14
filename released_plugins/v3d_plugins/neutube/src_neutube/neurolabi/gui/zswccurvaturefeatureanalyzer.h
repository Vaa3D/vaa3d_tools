#ifndef ZSWCCURVATUREFEATUREANALYZER_H
#define ZSWCCURVATUREFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcCurvatureFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcCurvatureFeatureAnalyzer();

  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);

private:
  double m_neighborRange;
};

#endif // ZSWCCURVATUREFEATUREANALYZER_H
