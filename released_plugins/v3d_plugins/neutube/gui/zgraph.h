#ifndef ZGRAPH_H
#define ZGRAPH_H

#include "tz_graph.h"

#include <set>
#include <vector>
#include <string>
#include <map>

#include "zprogressreporter.h"

//! ZGraph class
/*!
 *This is a class of hosting a graph which is defined as (V, E), where V is the
 *set of vertices and E is the set of edges. There are four types of graphs
 *supported by this class:
 *  DIRECTED_WITH_WEIGHT: Directed with weighted edges (default)
 *  UNDIRECTED_WITH_WEIGHT: Undirected with weighted edges
 *  DIRECTED_WITHOUT_WEIGHT: Directed without weights
 *  UNDIRECTED_WITHOUT_WEIGHT: Undirected without weights
*/

class ZGraph
{
public:
  enum EGraphType {
    DIRECTED_WITH_WEIGHT, UNDIRECTED_WITH_WEIGHT, DIRECTED_WITHOUT_WEIGHT,
    UNDIRECTED_WITHOUT_WEIGHT
  };

  ZGraph(EGraphType type = UNDIRECTED_WITHOUT_WEIGHT);
  ZGraph(Graph *graph);
  ~ZGraph();

  ZGraph* clone() const;

  enum EComponent {
    PARENT_LIST, CHILD_LIST, WEIGHT_MATRIX, DEGREE,
    IN_DEGREE, OUT_DEGREE, EDGE_TABLE, EDGE_MAP, BFS_TREE, NEIGHBOR_LIST,
    CONNECTED_SUBGRAPH, ALL_COMPONENT
  };

  /*!
   * \brief Set the graph as a directed/undirected graph.
   *
   * \param isDirected true for directed graph; false for undirected graph
   */
  void setDirected(bool isDirected);

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  inline bool isDirected() const { return Graph_Is_Directed(m_graph); }
  inline bool isWeighted() const { return Graph_Is_Weighted(m_graph); }

  inline int getVertexNumber() const { return m_graph->nvertex; }
  inline int getEdgeNumber() const { return m_graph->nedge; }

  inline bool hasVertex(int v) const {
    return ((v >= 0) && (v < getVertexNumber()));
  }

  inline double edgeWeight(size_t index) const { return m_graph->weights[index]; }

  void addEdge(int v1, int v2, double weight = 1.0);

  //Do not check if the edge exists
  void addEdgeFast(int v1, int v2, double weight = 1.0);

  inline int edgeStart(size_t edgeIndex) const {
    return m_graph->edges[edgeIndex][0];
  }
  inline int edgeEnd(size_t edgeIndex) const {
    return m_graph->edges[edgeIndex][1];
  }

  int getEdgeIndex(int v1, int v2) const;

  void setEdge(int index, int v1, int v2);
  //void setEdge(int index, int v1, int v2, double weight);

  inline size_t size() const { return m_graph->nedge; }
  inline int order() const { return m_graph->nvertex; }

  std::set<int> getNeighborSet(int vertex) const;
  std::set<int> getNeighborSet(const std::vector<int> &vertexArray) const;

  void print() const;
  void exportDotFile(const std::string &filePath) const;
  void exportDotFile(const std::string &filePath,
                     const std::vector<bool> &labeled) const;
  void exportTxtFile(const std::string &filePath) const;
  void importTxtFile(const std::string &filePath);

  void clear();

  double getEdgeWeight(int edgeIndex) const;
  double getEdgeWeight(int v1, int v2);
  void setEdgeWeight(int v1, int v2, double weight);

  void toMst();

  int* getDegree() const;
  int getDegree(int v) const;

  //Remove all termini (any node with degree <= 1)
  void removeTerminal();

  //Extract the path (with smallest number of nodes) between two vertices.
  //It extracts a cycle if <v1> is the same as <v2> and the cycle exists.
  std::vector<int> getPath(int v1, int v2);

  std::vector<std::vector<int> > getCycle(int root) const;
  std::vector<std::vector<int> > getCycle() const;

  //Merge a set of vertex. the new vertex index is the minumum of vertexArray
  void mergeVertex(const std::vector<int> &vertexArray);



  void setVertexNumber(int n);

  inline int getEdgeBegin(int edgeIndex) const {
    return m_graph->edges[edgeIndex][0];
  }

  inline int getEdgeEnd(int edgeIndex) const {
    return m_graph->edges[edgeIndex][1];
  }

  void removeEdge(int index);
  void removeEdge(const std::vector<int> &edgeList);

  void removeSelfEdge();
  void removeDuplicatedEdge();

  int **getNeighborList() const;
  int getNeighborNumber(int v) const;
  int getNeighbor(int v, int index) const;

  const std::vector<ZGraph*>& getConnectedSubgraph() const;

  inline void setProgressReporter(ZProgressReporter *reporter) {
    m_progressReporter = reporter;
  }

  inline Graph* getRawGraph() { return m_graph; }
  inline const Graph* getRawGraph() const { return m_graph; }

  /*!
   * \brief Compute shortest path
   *
   * It computes the shortest path from \a start to \a end. The current version
   * treated a directed graph as undirected. It returns empty if there is no
   * path from start to end.
   *
   * \param start vertex
   * \param end vertex
   * \return the array of vertices on the path
   */
  std::vector<int> computeShortestPath(int start, int end);

  void expandEdgeTable(int v1, int v2);

  /*!
   * \brief Find matches among vertices
   *
   * This function does not garuantee a global optimization. The weight of each
   * edge must be small (< 100) to get good results.
   *
   * \return Map of the matches
   */
  std::map<int, int> runMinWeightSumMatch();

private:
  void initWorkspace();
  const Hash_Table* getEdgeTable() const;

private:
  Graph *m_graph;
  Graph_Workspace *m_workspace;
  mutable std::vector<ZGraph*> m_connectedSubgraph;
  ZProgressReporter *m_progressReporter;
  ZProgressReporter m_nullProgressReporter;
};

#endif // ZGRAPH_H
