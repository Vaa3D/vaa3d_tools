#include "zcloudnetwork.h"

ZCloudNetwork::ZCloudNetwork()
{
  m_pointCloud = new ZWeightedPointArray;
  m_graph = New_Graph();
}

ZCloudNetwork::~ZCloudNetwork()
{
  delete m_pointCloud;
  Kill_Graph(m_graph);
}

void ZCloudNetwork::addEdge(int vs, int vt)
{
  Graph_Add_Edge(m_graph, vs, vt);
}

void ZCloudNetwork::addPoint(const ZWeightedPoint &pt)
{
  m_pointCloud->push_back(pt);
}

void ZCloudNetwork::addPoint(const ZPoint &pt, double weight)
{
  ZWeightedPoint wpt;
  wpt.set(pt.x(), pt.y(), pt.z());
  wpt.setWeight(weight);
}
