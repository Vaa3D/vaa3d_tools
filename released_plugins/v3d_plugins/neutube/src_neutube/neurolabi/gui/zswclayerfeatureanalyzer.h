#ifndef ZSWCLAYERFEATUREANALYZER_H
#define ZSWCLAYERFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"
#include "swctreenode.h"
#include "zswcsizefeatureanalyzer.h"

class ZSwcLayerFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcLayerFeatureAnalyzer();

public:
  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);
  inline void setLayerScale(double scale) {
    m_layerScale = scale;
  }

private:
  double m_layerBaseFactor;
  double m_layerScale;
  ZSwcSizeFeatureAnalyzer m_sizeFeatureAnalyzer;
};

#endif // ZSWCLAYERFEATUREANALYZER_H
