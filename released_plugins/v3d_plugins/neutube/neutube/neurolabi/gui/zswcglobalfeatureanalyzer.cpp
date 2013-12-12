#include "zswcglobalfeatureanalyzer.h"

ZSwcGlobalFeatureAnalyzer::ZSwcGlobalFeatureAnalyzer()
{
}

double ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(
    const ZSwcTree &tree)
{
  ZCuboid box =tree.boundBox();

  return sqrt(box.width() * box.width() + box.height() * box.height()) /
      box.depth();
}
