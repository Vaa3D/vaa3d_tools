#include "zswcnetwork.h"

#include <iostream>
#include <map>

#include "tz_error.h"
#include "zstring.h"
#include "tz_utilities.h"
#include "zcuboid.h"
#include "tz_darray.h"
#include "zpointnetwork.h"
#include "swctreenode.h"

using namespace std;

ZSwcNetwork::ZSwcNetwork()
{
}

void ZSwcNetwork::importTxtFile(const std::string &filePath)
{
  m_swcTreeArray.clear();
  m_connectionArray.clear();
  m_offsetArray.clear();
  m_treeIndexMap.clear();

  ZString str;

  FILE *fp = fopen(filePath.c_str(), "r");
  while (str.readLine(fp)) {
    if (str.contains(".swc")) {
      if (!str.isAbsolutePath()) {
        str = str.absolutePath(ZString(filePath).dirPath());
      }
      ZSwcTree *tree = new ZSwcTree;
      tree->load(str.c_str());
      addSwcTree(tree, ZPoint(0, 0, 0));
    } else {
      std::vector<double> connection = str.toDoubleArray();
      if (connection.size() == 5) {
        addConnection(connection[0], connection[1],
            connection[2], connection[3],
            connection[4]);
      }
    }
  }

  fclose(fp);
}

void ZSwcNetwork::exportTxtFile(const string &filePath)
{
  FILE *fp = fopen(filePath.c_str(), "w");

  //Save swc trees
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    ZString swcFilePath = filePath;
    swcFilePath.appendNumber(i, 3);
    swcFilePath += ".swc";
    m_swcTreeArray[i]->save(swcFilePath.c_str());

    fprintf(fp, "%s\n", swcFilePath.c_str());
  }

#ifdef _DEBUG_
  cout << "swc tree saved" << endl;
#endif

  //Save connections
  for (size_t i = 0; i < m_connectionArray.size(); i++) {
    int treeIndex[2] = {-1, -1};
    for (size_t j = 0; j < m_swcTreeArray.size(); j++) {
      if (treeIndex[0] < 0) {
        if (m_swcTreeArray[j]->contains(m_connectionArray[i].first)) {
          treeIndex[0] = j;
        }
      }

      if (treeIndex[1] < 0) {
        if (m_swcTreeArray[j]->contains(m_connectionArray[i].second)) {
          treeIndex[1] = j;
        }
      }

      if (treeIndex[0] >= 0 && treeIndex[1] >= 0) {
        break;
      }
    }

    fprintf(fp, "%d %d %d %d %g\n", treeIndex[0],
            SwcTreeNode::id(m_connectionArray[i].first), treeIndex[1],
            SwcTreeNode::id(m_connectionArray[i].second),
            m_connectionArray[i].weight);
  }

#ifdef _DEBUG_
  cout << "connection saved" << endl;
#endif

  fclose(fp);
}

void ZSwcNetwork::addSwcTree(ZSwcTree *tree, const ZPoint &offset)
{
  m_swcTreeArray.push_back(tree);
  m_offsetArray.push_back(offset);
}

void ZSwcNetwork::layoutSwc()
{
  Graph *graph = toGraph();

  vector<double> xArray(graph->nvertex);
  vector<double> yArray(graph->nvertex);

  /* Initialize the positions */
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    double z;
    Swc_Tree_Centroid(m_swcTreeArray[i]->data(), &(xArray[i]), &(yArray[i]), &z);
  }

  Graph_Workspace *gw = New_Graph_Workspace();

  vector<double> cxArray = xArray;
  vector<double> cyArray = yArray;

  Graph_Layout(graph, &(xArray[0]), &(yArray[0]), gw);

  Kill_Graph_Workspace(gw);
  Kill_Graph(graph);

  m_offsetArray.resize(m_swcTreeArray.size());

  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    m_offsetArray[i].set(xArray[i] * 5.0 - cxArray[i],
                       yArray[i] * 5.0 - cyArray[i], 0.0);
  }
}

Graph* ZSwcNetwork::toGraph()
{
  Graph *graph = Make_Graph(0, 1, TRUE);

  for (size_t i = 0; i < m_connectionArray.size(); i++) {
    Graph_Add_Weighted_Edge(graph, m_treeIndexMap[m_connectionArray[i].first],
        m_treeIndexMap[m_connectionArray[i].second],
        m_connectionArray[i].weight);
  }

  Graph_Normalize_Edge(graph);
  Graph_Remove_Duplicated_Edge(graph);

  return graph;
}

ZPointNetwork* ZSwcNetwork::toPointNetwork()
{
  ZPointNetwork *network = new ZPointNetwork;

  map<Swc_Tree_Node*, int> indexMap;

  for (size_t i = 0; i < m_connectionArray.size(); i++) {
    Swc_Tree_Node *tn1 = m_connectionArray[i].first;
    Swc_Tree_Node *tn2 = m_connectionArray[i].second;

    size_t s = indexMap.size();
    if (indexMap.count(tn1) == 0) {
      indexMap[tn1] = s;
      network->addPoint(SwcTreeNode::pos(tn1) +
                        m_offsetArray[m_treeIndexMap[tn1]] +
          m_connectionArray[i].first_offset, 1.0);
    }

    s = indexMap.size();
    if (indexMap.count(tn2) == 0) {
      indexMap[tn2] = s;
      network->addPoint(SwcTreeNode::pos(tn2) +
                        m_offsetArray[m_treeIndexMap[tn2]] +
          m_connectionArray[i].second_offset, 1.0);
    }

#ifdef _DEBUG_2
    cout << s << endl;
#endif

#ifdef _DEBUG_2
    cout << "edge: " << SwcTreeNode::toString(tn1) << " "
         << SwcTreeNode::toString(tn2) << endl;
#endif

    network->addEdge(indexMap[tn1], indexMap[tn2], m_connectionArray[i].weight);
  }

  return network;
}

void ZSwcNetwork::addConnection(int tree_index1, int node_id1,
                                int tree_index2, int node_id2,
                                double weight)
{
  Swc_Tree_Node *tn1 = m_swcTreeArray[tree_index1]->queryNode(node_id1);
  Swc_Tree_Node *tn2 = m_swcTreeArray[tree_index2]->queryNode(node_id2);

#ifdef _DEBUG_2
  cout << "added connection: " << SwcTreeNode::id(tn1) << " "
       << SwcTreeNode::id(tn2) << endl;
#endif

  m_treeIndexMap[tn1] = tree_index1;
  m_treeIndexMap[tn2] = tree_index2;

  addConnection(tn1, tn2, weight);
}

void ZSwcNetwork::addConnection(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2,
                                double weight)
{
#ifdef _DEBUG_2
  cout << "connection weight: " << weight << endl;
#endif

  m_connectionArray.push_back(SwcNodeConnection(tn1, tn2, weight));
}

void ZSwcNetwork::addConnection(Swc_Tree_Node *tn1, const ZPoint &offset1,
                                Swc_Tree_Node *tn2, const ZPoint &offset2,
                                double weight)
{
  m_connectionArray.push_back(SwcNodeConnection(tn1, offset1, tn2, offset2,
                                                weight));
}

void ZSwcNetwork::addConnection(Swc_Tree_Node *tn1, const ZPoint &offset1,
                                int tree_index1, Swc_Tree_Node *tn2,
                                const ZPoint &offset2, int tree_index2,
                                double weight)
{
  m_connectionArray.push_back(SwcNodeConnection(tn1, offset1, tn2, offset2,
                                                weight));
  m_treeIndexMap[tn1] = tree_index1;
  m_treeIndexMap[tn2] = tree_index2;
}

void ZSwcNetwork::boundBox(double *corner)
{
  for (int i = 0; i < 3; i++) {
    corner[i] = Infinity;
    corner[i + 3] = -Infinity;
  }

  double tmpCorner[6];

  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    m_swcTreeArray[i]->boundBox(tmpCorner);
    for (int j = 0; j < 3; j++) {
      corner[j] = min(corner[j], tmpCorner[j]);
      corner[j + 3] = max(corner[j + 3], tmpCorner[j + 3]);
    }
  }
}

ZSwcTree* ZSwcNetwork::getTree(int index)
{
  return m_swcTreeArray[index];
}

void ZSwcNetwork::merge(const ZSwcNetwork &network)
{
  m_swcTreeArray.insert(m_swcTreeArray.end(), network.m_swcTreeArray.begin(),
                        network.m_swcTreeArray.end());
  m_connectionArray.insert(m_connectionArray.end(),
                           network.m_connectionArray.begin(),
                           network.m_connectionArray.end());
  m_offsetArray.insert(m_offsetArray.end(), network.m_offsetArray.begin(),
                       network.m_offsetArray.end());

  int newIndex = m_treeIndexMap.size();
  for (map<Swc_Tree_Node*, int>::const_iterator
       iter = network.m_treeIndexMap.begin();
       iter != network.m_treeIndexMap.end(); ++iter) {
    m_treeIndexMap[iter->first] = iter->second + newIndex;
  }
}
