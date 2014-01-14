#ifndef ZSWCSHOLLFEATUREANALYZER_H
#define ZSWCSHOLLFEATUREANALYZER_H

#include <vector>
#include "zswcfeatureanalyzer.h"
#include "zswctree.h"

class ZSwcShollFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcShollFeatureAnalyzer();

public:
  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);
  inline void setShollStart(double start) {
    m_shollStart = start;
  }

  inline void setShollEnd(double end) {
    m_shollEnd = end;
  }

  inline void setShollRadius(double radius) {
    m_shollRadius = radius;
  }

private:
  double m_shollStart;
  double m_shollEnd;
  double m_shollRadius;
};

#endif // ZSWCSHOLLFEATUREANALYZER_H
