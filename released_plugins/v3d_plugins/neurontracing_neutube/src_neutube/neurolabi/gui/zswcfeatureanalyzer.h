#ifndef ZSWCFEATUREANALYZER_H
#define ZSWCFEATUREANALYZER_H

#include <vector>
#include "tz_swc_tree.h"

//The strategy pattern for calculating and comparing features.
class ZSwcFeatureAnalyzer
{
public:
  ZSwcFeatureAnalyzer();
  virtual ~ZSwcFeatureAnalyzer();

  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn) = 0;
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2) = 0;

  virtual void setParameter(const std::vector<double> &parameterArray) = 0;

};

#endif // ZSWCFEATUREANALYZER_H
