#ifndef ZSWCBRANCHINGTRUNKANALYZER_H
#define ZSWCBRANCHINGTRUNKANALYZER_H

#include "zswctrunkanalyzer.h"

class ZSwcBranchingTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcBranchingTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);

private:
  int compareSwcNode(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
};

#endif // ZSWCBRANCHINGTRUNKANALYZER_H
