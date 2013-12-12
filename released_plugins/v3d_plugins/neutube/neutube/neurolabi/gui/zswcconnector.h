#ifndef ZSWCCONNECTOR_H
#define ZSWCCONNECTOR_H

#include <utility>
#include <vector>
#include "swctreenode.h"
#include "zswctree.h"
#include "zswcpath.h"

class ZGraph;

class ZSwcConnector
{
public:
  ZSwcConnector();

  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcPath &hook, const ZSwcTree &loop);
  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcPath &hook, const std::vector<ZSwcTree*> &loop);

  inline double getConnDist() const { return m_dist; }
  inline void setConnDist(double dist) { m_dist = dist; }

  static ZGraph* buildConnection(const std::vector<Swc_Tree_Node *> &nodeArray);
  static ZGraph* buildConnection(const std::set<Swc_Tree_Node *> &nodeSet);

private:
  double m_minDist;
  double m_dist;
};

#endif // ZSWCCONNECTOR_H
