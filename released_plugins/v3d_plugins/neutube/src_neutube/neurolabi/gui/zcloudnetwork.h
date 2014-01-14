#ifndef ZCLOUDNETWORK_H
#define ZCLOUDNETWORK_H

#include "zweightedpointarray.h"
#include "tz_graph.h"
#include "zpoint.h"

class ZCloudNetwork
{
public:
  ZCloudNetwork();
  ~ZCloudNetwork();

  inline ZWeightedPointArray* pointCloud() { return m_pointCloud; }
  inline Graph* graph() { return m_graph; }

  void addPoint(const ZWeightedPoint &pt);
  void addPoint(const ZPoint &pt, double weight);
  void addEdge(int vs, int vt);

private:
  ZWeightedPointArray *m_pointCloud;
  Graph *m_graph;
};

#endif // ZCLOUDNETWORK_H
