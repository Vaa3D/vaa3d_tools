#ifndef ZSWCDISTTRUNKANALYZER_H
#define ZSWCDISTTRUNKANALYZER_H

#include "zswctrunkanalyzer.h"
#include "swctreenode.h"

class ZSwcDistTrunkAnalyzer : public ZSwcTrunkAnalyzer
{
public:
  ZSwcDistTrunkAnalyzer();

public:
  virtual ZSwcPath extractMainTrunk(ZSwcTree *tree);
  virtual ZSwcPath extractTrunk(ZSwcTree *tree, Swc_Tree_Node *start);
  //virtual void labelTraffic(ZSwcTree *tree);

  inline void setDistanceWeight(double euclideanWeight, double geodesicWeight) {
    m_euclideanWeight = euclideanWeight;
    m_geodesicWeight = geodesicWeight;
  }

private:
  double m_geodesicWeight;
  double m_euclideanWeight;
};

#endif // ZSWCDISTTRUNKANALYZER_H
