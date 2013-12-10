#ifndef ZSWCSIZETRUNKANALYZER_H
#define ZSWCSIZETRUNKANALYZER_H

#include "zswctrunkanalyzer.h"

class ZSwcSizeTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcSizeTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);
};

#endif // ZSWCSIZETRUNKANALYZER_H
