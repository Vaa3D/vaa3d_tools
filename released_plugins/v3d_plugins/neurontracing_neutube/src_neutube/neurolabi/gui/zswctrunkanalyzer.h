#ifndef ZSWCTRUNKANALYZER_H
#define ZSWCTRUNKANALYZER_H

#include <set>
#include "zswcpath.h"

class ZSwcTree;

class ZSwcTrunkAnalyzer
{
public:
  ZSwcTrunkAnalyzer();
  virtual ~ZSwcTrunkAnalyzer();

  enum ETrafficRule {
    FURTEST_PAIR, LONGEST_PAIR, REACH_ROOT
  };

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree) = 0;
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start) = 0;
  virtual void labelTraffic(ZSwcTree *tree, ETrafficRule rule = FURTEST_PAIR);

  inline void setDistanceWeight(double euclideanWeight, double geodesicWeight) {
    m_euclideanWeight = euclideanWeight;
    m_geodesicWeight = geodesicWeight;
  }

  void addBlocker(Swc_Tree_Node *tn);
  void clearBlocker();
  inline void setBlocker(const std::set<Swc_Tree_Node*> &blocker) {
    m_blocker = blocker;
  }

protected:
  double m_geodesicWeight;
  double m_euclideanWeight;
  std::set<Swc_Tree_Node*> m_blocker;
};

#endif // ZSWCTRUNKANALYZER_H
