#ifndef ZSWCTRUNKSIZEFEATUREANALYZER_H
#define ZSWCTRUNKSIZEFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcTrunkAnalyzer;

class ZSwcTrunkSizeFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcTrunkSizeFeatureAnalyzer();
  virtual ~ZSwcTrunkSizeFeatureAnalyzer();

public:
  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  inline void setTrunkAnalyzer(ZSwcTrunkAnalyzer *trunkAnalyzer) {
    m_trunkAnalyzer = trunkAnalyzer;
  }

private:
  ZSwcTrunkAnalyzer *m_trunkAnalyzer;
};

#endif // ZSWCTRUNKSIZEFEATUREANALYZER_H
