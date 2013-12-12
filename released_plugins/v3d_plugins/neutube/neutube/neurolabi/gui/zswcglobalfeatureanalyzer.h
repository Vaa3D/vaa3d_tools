#ifndef ZSWCGLOBALFEATUREANALYZER_H
#define ZSWCGLOBALFEATUREANALYZER_H

#include "zswctree.h"

class ZSwcGlobalFeatureAnalyzer
{
public:
  ZSwcGlobalFeatureAnalyzer();

public:
  static double computeLateralVerticalRatio(const ZSwcTree &tree);
};

#endif // ZSWCGLOBALFEATUREANALYZER_H
