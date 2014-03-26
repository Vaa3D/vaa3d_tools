#include "tubemodel.h"
#include "swctreenode.h"

using namespace std;

Swc_Tree_Node* TubeModel::createSwc(Locseg_Chain *chain, int type)
{
  int n;
  Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array(chain, NULL, &n);

  int start = 0;
  int end = n;
  int step = 1;

  Swc_Tree_Node *head = NULL;
  Swc_Tree_Node *parent = NULL;
  for (int i = start; i != end; i += step) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(
          i, type, circles[i].center[0],
        circles[i].center[1], circles[i].center[2], circles[i].radius, i - 1);

    SwcTreeNode::setParent(tn, parent);
    if (parent == NULL) {
      head = tn;
    }
    parent = tn;
  }

  free(circles);

  return head;
}

void TubeModel::removeRedundantEdge(Neuron_Structure *ns)
{
  Process_Neuron_Structure(ns);
}

Neuron_Structure *createNeuronStructure(
    const vector<Locseg_Chain*> &chainArray,
    Stack *stack,
    Connection_Test_Workspace *ctw)
{
  if (chainArray.empty()) {
    return NULL;
  }

  Neuron_Structure *ns = Make_Neuron_Structure(chainArray.size());
  Graph_Workspace *gw = New_Graph_Workspace();

  int index = 0;
  for (vector<Locseg_Chain*>::const_iterator iter = chainArray.begin();
       iter != chainArray.end(); ++iter, ++index) {
    Neuron_Component comp;
    Set_Neuron_Component(&comp, NEUROCOMP_TYPE_LOCSEG_CHAIN, *iter);
    Neuron_Structure_Set_Component(ns, index, &comp);
  }

  for (size_t i = 0; i < chainArray.size(); ++i) {
    for (size_t j = 0; j < chainArray.size(); ++j) {
      if (i != j) {
        Neurocomp_Conn conn;
        conn.mode = NEUROCOMP_CONN_HL;
        Locseg_Chain *chain_i = NEUROCOMP_LOCSEG_CHAIN(ns->comp+ i);
        Locseg_Chain *chain_j = NEUROCOMP_LOCSEG_CHAIN(ns->comp + j);

        if (Locseg_Chain_Connection_Test(
              chain_i, chain_j, stack, 1.0, &conn, ctw)
            == TRUE) {
          Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(chain_j),
                                        &conn);
          BOOL conn_existed = FALSE;
          if (i > j) {
            if (ns->graph->nedge > 0) {
              int edge_idx = Graph_Edge_Index(j, i, gw);
              if (edge_idx >= 0) {
                if (conn.mode == NEUROCOMP_CONN_LINK) {
                  if (ns->conn[edge_idx].info[0] == conn.info[1]) {
                    conn_existed = TRUE;
                  }
                } else if (ns->conn[edge_idx].mode == NEUROCOMP_CONN_LINK) {
                  if (ns->conn[edge_idx].info[1] == conn.info[0]) {
                    conn_existed = TRUE;
                  }
                }
                if (conn_existed == TRUE) {
                  if (ns->conn[edge_idx].cost > conn.cost) {
                    Neurocomp_Conn_Copy(ns->conn + edge_idx, &conn);
                    ns->graph->edges[edge_idx][0] = i;
                    ns->graph->edges[edge_idx][1] = j;
                    Graph_Update_Edge_Table(ns->graph, gw);
                  }
                }
              }
            }
          }

          if (conn_existed == FALSE) {
            Neuron_Structure_Add_Conn(ns, i, j, &conn);
            Graph_Expand_Edge_Table(i, j, ns->graph->nedge - 1, gw);
          }
        }
      }
    }
  }

  Kill_Graph_Workspace(gw);

  return ns;
}
