#ifndef ZSWCWEIGHTTRUNKANALYZER_H
#define ZSWCWEIGHTTRUNKANALYZER_H

#include "zswctrunkanalyzer.h"

class ZSwcWeightTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcWeightTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);
};

#endif // ZSWCWEIGHTTRUNKANALYZER_H
