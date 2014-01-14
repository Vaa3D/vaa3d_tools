#include <iostream>
#include "zpointnetwork.h"

using namespace std;

ZPointNetwork::ZPointNetwork()
{
  m_graph = Make_Graph(0, 1, TRUE);
}

ZPointNetwork::~ZPointNetwork()
{
  Kill_Graph(m_graph);
}

void ZPointNetwork::addEdge(int vs, int vt, double weight)
{
  //cout << "weight pointer: " << m_graph->weights << endl;

  Graph_Add_Weighted_Edge(m_graph, vs, vt, weight);

#ifdef _DEBUG_
  cout << "Added edge: " << getEdgeNodeStart(m_graph->nedge - 1) << " "
       << getEdgeNodeEnd(m_graph->nedge - 1) << " "
       << getEdgeWeight(m_graph->nedge - 1) << endl;
  cout << getPoint(getEdgeNodeStart(m_graph->nedge - 1)) << endl;
  cout << getPoint(getEdgeNodeEnd(m_graph->nedge - 1)) << endl;
#endif
}

void ZPointNetwork::addPoint(const ZWeightedPoint &pt)
{
  m_pointArray.push_back(pt);
}

void ZPointNetwork::addPoint(const ZPoint &pt, double weight)
{
  ZWeightedPoint wpt;
  wpt.set(pt.x(), pt.y(), pt.z());
  wpt.setWeight(weight);

  addPoint(wpt);
}

ZWeightedPoint ZPointNetwork::getPoint(int index) const
{
  return m_pointArray[index];
}

int ZPointNetwork::getEdgeNodeStart(int index) const
{
  return m_graph->edges[index][0];
}

int ZPointNetwork::getEdgeNodeEnd(int index) const
{
  return m_graph->edges[index][1];
}

double ZPointNetwork::getEdgeWeight(int index) const
{
  return m_graph->weights[index];
}
