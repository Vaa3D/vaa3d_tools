#include "zgraphcompressor.h"
#include "zgraph.h"

using namespace std;

ZGraphCompressor::ZGraphCompressor()
{
}

void ZGraphCompressor::setGraph(ZGraph *graph)
{
  m_graph = graph;
}

void ZGraphCompressor::compress()
{
  if (m_graph != NULL) {
    if (m_graph->getEdgeNumber() > 0) {
      m_compressMap.resize(m_graph->getVertexNumber(), -1);
      int *degree = m_graph->getDegree();
      for (int v = 0; v < m_graph->getVertexNumber(); ++v) {
        if (degree[v] > 0) {
          m_uncompressMap.push_back(v);
          m_compressMap[v] = m_uncompressMap.size() - 1;
        }
      }

      for (int i = 0; i < m_graph->getEdgeNumber(); ++i) {
        m_graph->setEdge(i, m_compressMap[m_graph->getEdgeBegin(i)],
            m_compressMap[m_graph->getEdgeEnd(i)]);
      }
      m_graph->setVertexNumber(m_uncompressMap.size());
    }
  }
}

void ZGraphCompressor::uncompress()
{
  if (m_graph != NULL) {
    for (int i = 0; i < m_graph->getEdgeNumber(); ++i) {
      m_graph->setEdge(i, m_uncompressMap[m_graph->getEdgeBegin(i)],
          m_uncompressMap[m_graph->getEdgeEnd(i)]);
    }
  }
}

int ZGraphCompressor::uncompress(int v)
{
  return m_uncompressMap[v];
}
