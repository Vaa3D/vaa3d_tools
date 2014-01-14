#ifndef ZSWCNODEBUFFERFEATUREANALYZER_H
#define ZSWCNODEBUFFERFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcNodeBufferFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcNodeBufferFeatureAnalyzer();
  virtual ~ZSwcNodeBufferFeatureAnalyzer() {}

  std::vector<double> computeFeature(Swc_Tree_Node *tn);
  double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  void setParameter(const std::vector<double> &parameterArray);

  void setHelper(ZSwcFeatureAnalyzer *helper);

private:
  ZSwcFeatureAnalyzer *m_helper;
};

#endif // ZSWCNODEBUFFERFEATUREANALYZER_H
