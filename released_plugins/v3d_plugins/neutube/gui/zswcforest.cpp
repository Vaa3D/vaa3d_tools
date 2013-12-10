#include "zswcforest.h"
#include <iostream>

#include "tz_error.h"
#include "zswctree.h"

using namespace std;

ZSwcForest::ZSwcForest()
{
  m_graph = NULL;
  m_workspace = NULL;
}

ZSwcForest::~ZSwcForest()
{

  for (ZSwcForest::iterator iter = begin(); iter != end(); ++iter) {
    delete (*iter);
  }

  if (m_graph != NULL) {
    Kill_Graph(m_graph);
  }

  if (m_workspace != NULL) {
    Kill_Graph_Workspace(m_workspace);
  }
}

//Disabled for unidentified bugs
#if 0
Graph* ZSwcForest::buildConnectionGraph(bool mst, double distThre)
{
  if (m_graph != NULL) {
    Kill_Graph(m_graph);
  }
  if (m_workspace != NULL) {
    Kill_Graph_Workspace(m_workspace);
  }
  m_workspace = New_Graph_Workspace();
  m_connection.clear();
  m_graph = Make_Graph(size(), 1, TRUE);

  for (size_t i = 0; i < size(); i++) {
    for (size_t j = 0; j < size(); j++) {
      if (i != j) {
        Swc_Tree_Node *source = NULL;
        Swc_Tree_Node *target = NULL;
        double dist = (*this)[i]->distanceTo((*this)[j], &source, &target);
        if (distThre < 0.0 || dist <= distThre) {
          Graph_Add_Weighted_Edge(m_graph, i, j, dist);
          m_connection.push_back(
                pair<Swc_Tree_Node*, Swc_Tree_Node*>(source, target));
        }
      }
    }
  }

  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > tmpConnection;
  if (mst) {
    Graph_To_Mst2(m_graph, m_workspace);
    vector<bool> keep(m_connection.size(), false);
    for (size_t i = 0; i < m_connection.size(); i++) {
      keep[i] = (m_workspace->status[i] == 1);
    }
    Graph_Traverse_Direct(m_graph, 0, m_workspace);
    for (size_t i = 0; i < m_connection.size(); i++) {
      if (keep[i]) {
        tmpConnection.push_back(m_connection[i]);
      }
    }
    m_connection = tmpConnection;

    for (size_t i = 0; i < m_connection.size(); i++) {
      if (m_workspace->status[i] == 1) { //edge swapped
        Swc_Tree_Node *tmp;
        tmp = m_connection[i].first;
        m_connection[i].first = m_connection[i].second;
        m_connection[i].second = tmp;
      }
    }
  }

  return m_graph;
}

ZSwcTree* ZSwcForest::merge()
{
  vector<bool> isMerged(m_connection.size(), false);

  ZSwcTree *result = new ZSwcTree;

  for (size_t i = 0; i < m_connection.size(); i++) {
    Swc_Tree_Node *tn1 = m_connection[i].first;
    Swc_Tree_Node *tn2 = m_connection[i].second;

    int mergedIndex = m_graph->edges[i][1];
    if (isMerged[mergedIndex]) {
      tn1 = m_connection[i].second;
      tn2 = m_connection[i].first;
      mergedIndex = m_graph->edges[i][0];
    }

    //If the second has been merged, then merge the first
    /*
    if (isMerged[m_graph->edges[i][1]]) {
      tn1 = m_connection[i].second;
      tn2 = m_connection[i].first;
      mergedIndex = m_graph->edges[i][0];
    }
    */
#ifdef _DEBUG_
    if (tn1->node.y == 157 || tn2->node.y == 157) {
      cout << "tn1: ";
      Print_Swc_Tree_Node(tn1);
      Print_Swc_Tree_Node(tn2);
    }
#endif

    Swc_Tree_Node_Set_Root(tn2);
    Swc_Tree_Node_Set_Parent(tn2, tn1);

    TZ_ASSERT(tn1 != NULL, "Null node");
    TZ_ASSERT(tn2 != NULL, "Null node");

    isMerged[mergedIndex] = true;
    (*this)[mergedIndex]->setData(NULL, ZSwcTree::LEAVE_ALONE);
  }

  size_t index;
  for (index = 0; index < size(); index++) {
    if (!isMerged[index]) {
      ZSwcTree *tree = at(index);
      result->merge(tree->data(), false);
    }
  }

  return result;
}
#endif

void ZSwcForest::print()
{
    cout << size() << " trees" << endl;
    Print_Graph(m_graph);
}
