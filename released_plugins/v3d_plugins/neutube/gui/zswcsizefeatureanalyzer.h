#ifndef ZSWCSIZEFEATUREANALYZER_H
#define ZSWCSIZEFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcSizeFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcSizeFeatureAnalyzer();

public:
  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);

  inline void setExcludedLabel(int label) {
    m_excludedLabel = label;
  }

  inline void setIncludedLabel(int label) {
    m_includedLabel = label;
  }

private:
  int m_excludedLabel;
  int m_includedLabel;
};

#endif // ZSWCSIZEFEATUREANALYZER_H
