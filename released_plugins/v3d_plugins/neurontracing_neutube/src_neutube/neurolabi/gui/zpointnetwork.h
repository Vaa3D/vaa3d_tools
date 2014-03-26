#ifndef ZPOINTNETWORK_H
#define ZPOINTNETWORK_H

#include "zweightedpointarray.h"
#include "tz_graph.h"

class ZPointNetwork
{
public:
  ZPointNetwork();
  ~ZPointNetwork();

  inline ZWeightedPointArray* pointArray() { return &m_pointArray; }
  inline Graph* graph() { return m_graph; }

  void addPoint(const ZWeightedPoint &pt);
  void addPoint(const ZPoint &pt, double weight);
  void addEdge(int vs, int vt, double weight);

  ZWeightedPoint getPoint(int index) const;
  int getEdgeNodeStart(int index) const;
  int getEdgeNodeEnd(int index) const;
  double getEdgeWeight(int index) const;

  inline size_t pointNumber() const { return m_pointArray.size(); }
  inline size_t edgeNumber() const { return m_graph->nedge; }

private:
  ZWeightedPointArray m_pointArray;
  Graph *m_graph;
};

#endif // ZPOINTNETWORK_H
