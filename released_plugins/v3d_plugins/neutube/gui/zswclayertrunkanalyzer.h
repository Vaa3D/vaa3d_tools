#ifndef ZSWCLAYERTRUNKANALYZER_H
#define ZSWCLAYERTRUNKANALYZER_H

#include "zswctrunkanalyzer.h"

class ZSwcLayerTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcLayerTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);

  inline void setStep(double step) {
    m_step = step;
  }

  inline double getStep() { return m_step; }

private:
  double m_step;
};

#endif // ZSWCLAYERTRUNKANALYZER_H
