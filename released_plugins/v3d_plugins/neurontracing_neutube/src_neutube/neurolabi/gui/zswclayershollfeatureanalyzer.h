#ifndef ZSWCLAYERSHOLLFEATUREANALYZER_H
#define ZSWCLAYERSHOLLFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcTree;

class ZSwcLayerShollFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcLayerShollFeatureAnalyzer();

public:
  virtual std::vector<double> computeFeature(Swc_Tree_Node *tn);
  virtual double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  virtual void setParameter(const std::vector<double> &parameterArray);
  inline void setLayerScale(double scale) {
    m_layerScale = scale;
  }
  inline double getLayerScale() {
    return m_layerScale;
  }

  inline void setLayerMargin(double margin) {
    m_layerMargin = margin;
  }
  inline double getLayerMargin() {
    return m_layerMargin;
  }



private:
  double m_layerBaseFactor;
  double m_layerScale;
  double m_layerMargin;
};

#endif // ZSWCLAYERSHOLLFEATUREANALYZER_H
