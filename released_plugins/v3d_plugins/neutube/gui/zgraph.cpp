#include "zgraph.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "tz_utilities.h"
#include "zstring.h"
#include "tz_math.h"
#include "tz_u8array.h"
#include "zinttree.h"
#include "tz_error.h"
#include "zerror.h"

using namespace std;

ZGraph::ZGraph(EGraphType type)
{
  switch (type) {
  case DIRECTED_WITH_WEIGHT:
  case UNDIRECTED_WITH_WEIGHT:
    m_graph = Make_Graph(0, 1, TRUE);
    break;
  case DIRECTED_WITHOUT_WEIGHT:
  case UNDIRECTED_WITHOUT_WEIGHT:
    m_graph = Make_Graph(0, 1, FALSE);
    break;
  default:
    break;
  }

  if (type == DIRECTED_WITH_WEIGHT || type == DIRECTED_WITHOUT_WEIGHT) {
    Graph_Set_Directed(m_graph, TRUE);
  }

  initWorkspace();
}

ZGraph::ZGraph(Graph *graph)
{
  m_graph = graph;

  initWorkspace();
}

ZGraph::~ZGraph()
{
  if (m_graph != NULL) {
    Kill_Graph(m_graph);
    m_graph = NULL;
  }

  if (m_workspace != NULL) {
    Kill_Graph_Workspace(m_workspace);
    m_workspace = NULL;
  }
}

ZGraph* ZGraph::clone() const
{
  Graph *graph = NULL;
  if (m_graph != NULL) {
    graph = Copy_Graph(m_graph);
  }

  return new ZGraph(graph);
}

void ZGraph::initWorkspace()
{
  m_workspace = New_Graph_Workspace();
  Graph_Workspace_Load(m_workspace, m_graph);
  m_progressReporter = &m_nullProgressReporter;
}

void ZGraph::deprecateDependent(EComponent /*component*/)
{
}


void ZGraph::deprecate(EComponent component)
{
  deprecateDependent(component);
  switch (component) {
  case PARENT_LIST:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_PARENT, FALSE);
    break;
  case CHILD_LIST:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_CHILD, FALSE);
    break;
  case WEIGHT_MATRIX:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_WEIGHT, FALSE);
    break;
  case DEGREE:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_DEGREE, FALSE);
    break;
  case IN_DEGREE:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_IN_DEGREE, FALSE);
    break;
  case OUT_DEGREE:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_OUT_DEGREE, FALSE);
    break;
  case EDGE_TABLE:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_EDGE_TABLE, FALSE);
    break;
  case EDGE_MAP:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_EDGE_MAP, FALSE);
    break;
  case NEIGHBOR_LIST:
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_CONNECTION, FALSE);
    break;
  case BFS_TREE:
    break;
  case CONNECTED_SUBGRAPH:
    if (!m_connectedSubgraph.empty()) {
      for (std::vector<ZGraph*>::iterator iter = m_connectedSubgraph.begin();
           iter != m_connectedSubgraph.end(); ++iter) {
        delete (*iter);
      }
      m_connectedSubgraph.clear();
    }
    break;
  case ALL_COMPONENT:
    deprecate(NEIGHBOR_LIST);
    deprecate(PARENT_LIST);
    deprecate(CHILD_LIST);
    deprecate(WEIGHT_MATRIX);
    deprecate(DEGREE);
    deprecate(IN_DEGREE);
    deprecate(OUT_DEGREE);
    deprecate(EDGE_TABLE);
    deprecate(EDGE_MAP);
    deprecate(BFS_TREE);
    deprecate(CONNECTED_SUBGRAPH);
    break;
  }
}

bool ZGraph::isDeprecated(EComponent component) const
{
  switch (component) {
  case NEIGHBOR_LIST:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_CONNECTION)
        == FALSE;
    break;
  case PARENT_LIST:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_PARENT)
        == FALSE;
    break;
  case CHILD_LIST:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_CHILD)
        == FALSE;
    break;
  case WEIGHT_MATRIX:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_WEIGHT)
        == FALSE;
    break;
  case DEGREE:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_DEGREE)
        == FALSE;
    break;
  case IN_DEGREE:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_IN_DEGREE)
        == FALSE;
    break;
  case OUT_DEGREE:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_OUT_DEGREE)
        == FALSE;
    break;
  case EDGE_TABLE:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_EDGE_TABLE)
        == FALSE;
    break;
  case EDGE_MAP:
    return Graph_Workspace_Ready(m_workspace, GRAPH_WORKSPACE_EDGE_MAP)
        == FALSE;
    break;
  case BFS_TREE:
    return FALSE;
    break;
  case CONNECTED_SUBGRAPH:
    return m_connectedSubgraph.empty();
  case ALL_COMPONENT:
    return TRUE;
    break;
  }

  return FALSE;
}

double ZGraph::getEdgeWeight(int edgeIndex) const
{
  TZ_ASSERT(edgeIndex >= 0 && edgeIndex < getEdgeNumber(), "Invalide edge index");

  if (isWeighted()) {
    return edgeWeight(edgeIndex);
  }

  return 1.0;
}

double ZGraph::getEdgeWeight(int v1, int v2)
{
  if (isWeighted()) {
    return edgeWeight(Graph_Edge_Index(v1, v2, m_workspace));
  }

  return 1.0;
}

void ZGraph::setEdgeWeight(int v1, int v2, double weight)
{
  if (isWeighted()) {
    int index = Graph_Edge_Index(v1, v2, m_workspace);
    if (index >= 0) {
      m_graph->weights[index] = weight;
    }
  }
}

int** ZGraph::getNeighborList() const
{
  if (isDeprecated(NEIGHBOR_LIST)) {
    Graph_Workspace_Load(m_workspace, m_graph);
  }
  int** list = Graph_Neighbor_List(m_graph, m_workspace);
  Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_CONNECTION, TRUE);

  return list;
}

int ZGraph::getNeighborNumber(int v) const
{
  int **neighborList = getNeighborList();
  if (neighborList == NULL) {
    return 0;
  }

  if (neighborList[v] == NULL) {
    return 0;
  }

  return neighborList[v][0];
}

int ZGraph::getNeighbor(int v, int index) const
{
  int neighborNumber = getNeighborNumber(v);
  if (index < 0 || index >= neighborNumber) {
    return -1;
  }

  int **neighborList = getNeighborList();

  return neighborList[v][index + 1];
}

void ZGraph::addEdge(int v1, int v2, double weight)
{
  if (v1 == v2) {
    std::cout << "Self loop added: " << v1 << " " << v2 << std::endl;
  }

  bool edgeExisted = false;

  if (!isDirected()) {
    if (v1 > v2) {
      int tmp;
      SWAP2(v1, v2, tmp);
    }
  }

  if (getEdgeIndex(v1, v2) >= 0) {
    edgeExisted = true;
  }

  if (!edgeExisted) {
    if (isWeighted()) {
      Graph_Add_Weighted_Edge(m_graph, v1, v2, weight);
    } else {
      Graph_Add_Edge(m_graph, v1, v2);
    }
    deprecate(ALL_COMPONENT);
    //Graph_Expand_Edge_Table(v1, v2, size() - 1, m_workspace);
  } else {
    if (isWeighted()) {
      setEdgeWeight(v1, v2, getEdgeWeight(v1, v2) + weight);
    }
  }
}

void ZGraph::addEdgeFast(int v1, int v2, double weight)
{
  if (!isDirected()) {
    if (v1 > v2) {
      int tmp;
      SWAP2(v1, v2, tmp);
    }
  }

  if (isWeighted()) {
    Graph_Add_Weighted_Edge(m_graph, v1, v2, weight);
  } else {
    Graph_Add_Edge(m_graph, v1, v2);
  }
  deprecate(ALL_COMPONENT);
  //Graph_Expand_Edge_Table(v1, v2, size() - 1, m_workspace);
}

set<int> ZGraph::getNeighborSet(int vertex) const
{
  set<int> neighborSet;

  for (size_t edgeIndex = 0; edgeIndex < size(); ++edgeIndex) {
    if (edgeStart(edgeIndex) == vertex) {
      neighborSet.insert(edgeEnd(edgeIndex));
    } else if (edgeEnd(edgeIndex) == vertex) {
      neighborSet.insert(edgeStart(edgeIndex));
    }
  }

  return neighborSet;
}

set<int> ZGraph::getNeighborSet(const vector<int> &vertexArray) const
{
  set<int> neighborSet;
  for (size_t i = 0; i < vertexArray.size(); ++i) {
    set<int> subset = getNeighborSet(vertexArray[i]);
    neighborSet.insert(subset.begin(), subset.end());
  }

  for (size_t i = 0; i < vertexArray.size(); ++i) {
    neighborSet.erase(vertexArray[i]);
  }

  return neighborSet;
}

void ZGraph::print() const
{
  if (isDirected()) {
    cout << "Directed graph:";
  } else {
    cout << "Undirected graph:";
  }

  cout << order() << " vertices; ";
  cout << size() << " edges" << endl;

  for (size_t i = 0; i < size(); i++) {
    cout << edgeStart(i) << " -- ";
    if (isDirected()) {
      cout << "> ";
    }
    cout << edgeEnd(i);

    if (isWeighted()) {
      cout << ": " << edgeWeight(i);
    }
    cout << endl;
  }
}

void ZGraph::exportDotFile(const string &filePath) const
{
  Graph_To_Dot_File(m_graph, filePath.c_str());
}

void ZGraph::exportDotFile(const string &filePath,
                           const std::vector<bool> &labeled) const
{
  ofstream stream(filePath.c_str());

  if (stream.is_open()) {

    if (isDirected()) {
      stream << "digraph G {" << endl;
    } else {
      stream << "graph G {" << endl;
    }

    for (int i = 0; i < getEdgeNumber(); ++i) {
      stream << getEdgeBegin(i);
      if (isDirected()) {
        stream << " -> ";
      } else {
        stream << " -- ";
      }
      stream << getEdgeEnd(i);

      if (isWeighted()) {
        stream << " [len=" << getEdgeWeight(i) << ", " << "label=\""
               << getEdgeWeight(i) << "\"]";
      }
      stream << ";" << endl;
    }

    for (int i = 0; i < getVertexNumber(); ++i) {
      if (labeled[i]) {
        if (getDegree(i) > 0) {
          stream << i << " [style=filled];" << endl;
        }
      }
    }

#ifdef _DEBUG_
    cout << filePath << " saved." << endl;
#endif
    stream.close();
  } else {
    cerr << "Unable to write " << filePath << endl;
  }
}

void ZGraph::exportTxtFile(const string &filePath) const
{
  ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); i++) {
    stream << edgeStart(i) << " " << edgeEnd(i) ;

    if (isWeighted()) {
      stream << " " << edgeWeight(i);
    }
    stream << endl;
  }
}

void ZGraph::importTxtFile(const string &filePath)
{
  clear();
  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    ZString str;
    while (str.readLine(fp)) {
      vector<double> edge = str.toDoubleArray();
      if (edge[0] != edge[1] && edge[0] >= 0 && edge[1] >= 0) {
        if (edge.size() == 3) {
          addEdge(iround(edge[0]), iround(edge[1]), edge[2]);
        } else {
          addEdge(iround(edge[0]), iround(edge[1]));
        }
      } else {
        RECORD_WARNING_UNCOND("Skip invalid edge");
      }
    }
  }

  fclose(fp);
}

void ZGraph::clear()
{
  m_graph->nvertex = 0;
  m_graph->nedge = 0;
  deprecate(ALL_COMPONENT);
}

void ZGraph::toMst()
{
  Graph_To_Mst2(m_graph, m_workspace);
}

const Hash_Table *ZGraph::getEdgeTable() const
{
  if (isDeprecated(EDGE_TABLE)) {
    Graph_Workspace_Load(m_workspace, m_graph);
    Graph_Update_Edge_Table(m_graph, m_workspace);
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_EDGE_TABLE, TRUE);
  }

  return m_workspace->edge_table;
}

void ZGraph::expandEdgeTable(int v1, int v2)
{
  Graph_Expand_Edge_Table(v1, v2, size() - 1, m_workspace);
}

int ZGraph::getEdgeIndex(int v1, int v2) const
{
  if (size() == 0) {
    return -1;
  }

  const Hash_Table* edgeTable = getEdgeTable();
  if (edgeTable != NULL) {
    if (isDirected()) {
      return Graph_Edge_Index(v1, v2, m_workspace);
    } else {
      return Graph_Edge_Index_U(v1, v2, m_workspace);
    }
  }

  return -1;
}

int* ZGraph::getDegree() const
{
  if (isDeprecated(DEGREE)) {
    Graph_Workspace_Load(m_workspace, m_graph);
  }
  int* degree = Graph_Degree(m_graph, m_workspace);
  Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_DEGREE, TRUE);

  return degree;
}

int ZGraph::getDegree(int v) const
{
  if (v < 0 || v >= getVertexNumber()) {
    return 0;
  }

  int *degree = getDegree();

  if (degree == NULL) {
    return 0;
  }

  return degree[v];
}

//At least one edge of any cycle is unique
std::vector<std::vector<int> > ZGraph::getCycle(int root) const
{
  TZ_ASSERT(!isDirected(), "Directed graph not supported yet");

  std::vector<std::vector<int> > cycleArray;

  //Bfs traversal from v1
  Arrayqueue *aq = Make_Arrayqueue(getVertexNumber());
  //Initialize_Arrayqueue(aq);

  int vertexNumber = getVertexNumber();
  ZIntTree traversalTree(vertexNumber);
  if (getNeighborNumber(root) > 0) {
    /* alloc <mask> */
    uint8_t *mask = u8array_malloc(vertexNumber);
    for (int i = 0; i < vertexNumber; i++) {
      mask[i] = 0;
      aq->array[i] = -1;
    }

    vector<bool> edgeChecked(getEdgeNumber(), false);

    Arrayqueue_Add_Last(aq, root);
    mask[root] = 1;
    int next = root;
    do {
      //For each neighbor
      for (int i = 0; i < getNeighborNumber(next); i++) {
        int node = getNeighbor(next, i);
        if (mask[node] == 0) { //If the neighbor has not been visited
          Arrayqueue_Add_Last(aq, node);
          traversalTree.setParent(node, next);
          mask[node] = 1;
        } else { //If the neighbor has been visited
          if (node != traversalTree[next]) {
            //If the traversal hits a labeled vertex that is not the parent
            //Retrive the path
            int ancestor = traversalTree.getCommonAncestor(next, node);
            int currentNode = next;
            std::vector<int> path;
            bool isPathValid = false;
            while (currentNode != ancestor) {
              path.insert(path.begin(), currentNode);
              currentNode = traversalTree[currentNode];
              if (!edgeChecked[getEdgeIndex(path[0], currentNode)]) {
                isPathValid = true;
                edgeChecked[getEdgeIndex(path[0], currentNode)] = true;
              }
            }
            currentNode = node;
            while (currentNode != ancestor) {
              if (!edgeChecked[getEdgeIndex(path.back(), currentNode)]) {
                isPathValid = true;
                edgeChecked[getEdgeIndex(path.back(), currentNode)] = true;
              }
              path.push_back(currentNode);
              currentNode = traversalTree[currentNode];
            }
            if (!edgeChecked[getEdgeIndex(path.back(), ancestor)]) {
              isPathValid = true;
              edgeChecked[getEdgeIndex(path.back(), ancestor)] = true;
            }
            path.push_back(ancestor);
            if (isPathValid) {
              cycleArray.push_back(path);
            }
          }
        }
      }
      next = aq->array[next];
    } while (next >= 0);

    /* free <mask> */
    free(mask);
  }

  Kill_Arrayqueue(aq);

  return cycleArray;
}

std::vector<std::vector<int> > ZGraph::getCycle() const
{
  m_progressReporter->start();
  const vector<ZGraph*>& subgraphArray = getConnectedSubgraph();
  std::vector<std::vector<int> > allPath;

  m_progressReporter->advance(0.1);
  m_progressReporter->startSubprogress(0.9);

  for (size_t i = 0; i < subgraphArray.size(); ++i) {
    m_progressReporter->advance(1.0 / subgraphArray.size());
    std::vector<std::vector<int> > path = subgraphArray[i]->getCycle(
          subgraphArray[i]->getEdgeBegin(0));
    allPath.insert(allPath.end(), path.begin(), path.end());
  }

  m_progressReporter->endSubprogress(0.9);

  m_progressReporter->end();

  return allPath;
}


std::vector<int> ZGraph::getPath(int v1, int v2)
{
  std::vector<int> path;
  if (v1 == v2 && getDegree(v1) < 2) {
    path.push_back(v1);
  } else {
    //Bfs traversal from v1
    Arrayqueue *aq = Make_Arrayqueue(getVertexNumber());
    //Initialize_Arrayqueue(aq);

    int root = v1;
    int vertexNumber = getVertexNumber();
    ZIntTree traversalTree(vertexNumber);
    if (getNeighborNumber(root) > 0) {
      /* alloc <mask> */
      uint8_t *mask = u8array_malloc(vertexNumber);
      for (int i = 0; i < vertexNumber; i++) {
        mask[i] = 0;
        aq->array[i] = -1;
      }

      Arrayqueue_Add_Last(aq, root);
      mask[root] = 1;
      int next = root;
      bool pathFound = false;
      do {
        //For each neighbor
        for (int i = 0; i < getNeighborNumber(next); i++) {
          int node = getNeighbor(next, i);
          if (mask[node] == 0) { //If the neighbor has not been visited
            Arrayqueue_Add_Last(aq, node);
            traversalTree.setParent(node, next);
            mask[node] = 1;
          } else { //If the neighbor has been visited
            if (v1 == v2) { //Get cycle
              if (node != traversalTree[next]) {
                //If the traversal hits a labeled vertex that is not the parent
                //Retrive the path
                if (traversalTree.getCommonAncestor(next, node) == root) {
                  int currentNode = next;
                  while (currentNode >= 0) {
                    path.insert(path.begin(), currentNode);
                    currentNode = traversalTree[currentNode];
                  }
                  currentNode = node;
                  while (currentNode >= 0) {
                    path.push_back(currentNode);
                    currentNode = traversalTree[currentNode];
                  }
                  pathFound = true;
                }
              }
            }
          }

          if (v1 != v2) { // Regular path
            if (node == v2) {
              int currentNode = next;
              while (currentNode >= 0) {
                path.insert(path.begin(), currentNode);
                currentNode = traversalTree[currentNode];
              }
              path.push_back(node);
              pathFound = true;
            }
          }
          if (pathFound) {
            break;
          }
        }
        next = aq->array[next];
      } while (next >= 0 && !pathFound);

      /* free <mask> */
      free(mask);
    }

    Kill_Arrayqueue(aq);
  }

  return path;
}

void ZGraph::removeEdge(int index)
{
  Graph_Remove_Edge(m_graph, index);
  deprecate(ALL_COMPONENT);
}

void ZGraph::removeEdge(const std::vector<int> &edgeList)
{
  if (Graph_Remove_Edge_List(m_graph, &(edgeList[0]), edgeList.size()) > 0) {
    deprecate(ALL_COMPONENT);
  }
}

void ZGraph::removeTerminal()
{
  int *degree = getDegree();

  std::vector<int> removableEdgeIndex;

  if (degree != NULL) {
    for (int i = 0; i < getEdgeNumber(); ++i) {
      if (degree[getEdgeBegin(i)] == 1 || degree[getEdgeEnd(i)] == 1) {
        removableEdgeIndex.push_back(i);
      }
    }

    removeEdge(removableEdgeIndex);
  }
}

void ZGraph::mergeVertex(const std::vector<int> &vertexArray)
{
  vector<int> vertexMap(getVertexNumber());

  //Initialize vertex map
  for (size_t i = 0; i < vertexMap.size(); ++i) {
    vertexMap[i] = i;
  }

  int minVertex = *(std::min_element(vertexArray.begin(), vertexArray.end()));


  for (vector<int>::const_iterator iter = vertexArray.begin();
       iter != vertexArray.end(); ++iter) {
    vertexMap[*iter] = minVertex;
  }

  //Update edges
  vector<int> removableEdgeList;
  for (int i = 0; i < getEdgeNumber(); ++i) {
    m_graph->edges[i][0] = vertexMap[m_graph->edges[i][0]];
    m_graph->edges[i][1] = vertexMap[m_graph->edges[i][1]];
    if (m_graph->edges[i][0] == m_graph->edges[i][1]) {
      removableEdgeList.push_back(i);
    }
  }

  removeEdge(removableEdgeList);

  removeDuplicatedEdge();

  deprecate(ALL_COMPONENT);
}

void ZGraph::removeDuplicatedEdge()
{
  if (Graph_Remove_Duplicated_Edge(m_graph) > 0) {
    deprecate(ALL_COMPONENT);
  }
}

void ZGraph::setEdge(int index, int v1, int v2)
{
  TZ_ASSERT(index >= 0 && index < getEdgeNumber(), "Invalide edge index");

  if (m_graph->edges[index][0] != v1 ||
      m_graph->edges[index][1] != v2) {
    m_graph->edges[index][0] = v1;
    m_graph->edges[index][1] = v2;
    if (v1 >= getVertexNumber()) {
      setVertexNumber(v1 + 1);
    }
    if (v2 >= getVertexNumber()) {
      setVertexNumber(v2 + 1);
    }
    deprecate(ALL_COMPONENT);
  }
}

void ZGraph::setVertexNumber(int n)
{
   m_graph->nvertex = n;
   deprecate(ALL_COMPONENT);
}

const std::vector<ZGraph*>& ZGraph::getConnectedSubgraph() const
{
  if (isDeprecated(CONNECTED_SUBGRAPH) && getEdgeNumber() > 0) {
    Graph_Workspace_Load(m_workspace, m_graph);
    Graph *subgraph = Graph_Connected_Subgraph(
          m_graph, m_workspace, getEdgeBegin(0));

    m_connectedSubgraph.push_back(new ZGraph(subgraph));

    /* This is necessary to extract subgraph one by one */
    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_ELIST, TRUE);

    for (int i = 1; i < getEdgeNumber(); i++) {
      if (m_workspace->elist[i] == 0) {
        subgraph = Graph_Connected_Subgraph(
              m_graph, m_workspace, getEdgeBegin(i));
        m_connectedSubgraph.push_back(new ZGraph(subgraph));
      }
    }

    Graph_Workspace_Set_Readiness(m_workspace, GRAPH_WORKSPACE_ELIST, FALSE);
  }

  return m_connectedSubgraph;
}

void ZGraph::setDirected(bool isDirected)
{
  if (m_graph->directed != isDirected) {
    deprecate(ALL_COMPONENT);
  }

  Graph_Set_Directed(m_graph, isDirected);
}


std::vector<int> ZGraph::computeShortestPath(int start, int end)
{
  std::vector<int> path;

  if (hasVertex(start) && hasVertex(end)) {
    if (start == end) {
      path.push_back(start);
    } else {
      int *pathList = Graph_Shortest_Path_E(m_graph, start, end, m_workspace);

      if (pathList[end] >= 0) {
        int v = end;
        while (v >= 0) {
          path.push_back(v);
          v = pathList[v];
        }

        std::reverse(path.begin(), path.end());
      }
    }
  }

  return path;
}

std::map<int, int> ZGraph::runMinWeightSumMatch()
{
  std::map<int, int> matchMap;

  if (!isDirected() && isWeighted()) {
    if (getEdgeNumber() > 0) {
      int nvertex = getVertexNumber();

      BOOL **match = Graph_Hungarian_Match(m_graph, m_workspace);

      for (int i = 0; i < nvertex; ++i) {
        for (int j = i + 1; j < nvertex; ++j) {
          if (match[i][j] == TRUE || match[j][i] == TRUE) {
            if (getEdgeIndex(i, j) >= 0) {
              matchMap[i] = j;
            }
          }
        }
      }

      FREE_2D_ARRAY(match, nvertex);
    }
  }

  return matchMap;
}
