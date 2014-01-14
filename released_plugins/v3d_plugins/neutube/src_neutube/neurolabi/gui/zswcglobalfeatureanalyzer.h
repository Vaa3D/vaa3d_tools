#ifndef ZSWCGLOBALFEATUREANALYZER_H
#define ZSWCGLOBALFEATUREANALYZER_H

#include "zswctree.h"

class ZSwcGlobalFeatureAnalyzer
{
public:
  ZSwcGlobalFeatureAnalyzer();

  enum EFeatureSet{
    NGF1, //Number of leaves, number of branch points,
          //box volume, maximum segment length, maximum path length
          //average radius, radius variance, lateral/vertical ratio
          //Average curvature
    NGF2,
    NGF3,
    UNDEFINED_NGF
  };

public:
  static double computeLateralVerticalRatio(const ZSwcTree &tree);
  static std::vector<double> computeFeatureSet(ZSwcTree &tree,
                                               EFeatureSet setName);
};

#endif // ZSWCGLOBALFEATUREANALYZER_H
