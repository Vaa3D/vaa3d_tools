#ifndef ZSWCRANGEANALYZER_H
#define ZSWCRANGEANALYZER_H

#include <vector>
#include "zswctree.h"
#include "zellipsoid.h"

class ZSwcRangeAnalyzer
{
public:
  ZSwcRangeAnalyzer();

  std::vector<ZEllipsoid> computeLayerRange(const ZSwcTree &tree);
  std::vector<ZPoint> computerLayerCentroid(const ZSwcTree &tree);
  std::vector<Swc_Tree_Node*> getOutsideNode(const ZSwcTree &host,
                                             const ZSwcTree &targetTree);

  inline void setZStep(double step) { m_zStep = step; }
  inline void setZMargin(double margin) { m_zMargin = margin; }

private:
  double m_zStep;
  double m_zMargin;
};

#endif // ZSWCRANGEANALYZER_H
