#ifndef ZSWCTYPETRUNKANALYZER_H
#define ZSWCTYPETRUNKANALYZER_H

#include "zswctrunkanalyzer.h"

class ZSwcTypeTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcTypeTrunkAnalyzer();
  virtual ~ZSwcTypeTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);

  inline void setTrunType(int type) {
    m_trunkType = type;
  }

  inline void setTrunkAnalyzer(ZSwcTrunkAnalyzer *analyzer) {
    m_trunkAnalyzer = analyzer;
  }

private:
  int m_trunkType;
  ZSwcTrunkAnalyzer *m_trunkAnalyzer; //composition
};

#endif // ZSWCTYPETRUNKANALYZER_H
