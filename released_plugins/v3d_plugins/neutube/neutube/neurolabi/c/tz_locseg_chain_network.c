#include <stdlib.h>

#include "tz_error.h"
#include "tz_int_arraylist.h"
#include "tz_iarray.h"
#include "tz_locseg_chain_network.h"

void Locseg_Chain_Network_Simlify(Locseg_Chain_Network *net, Graph *graph,
				  Int_Arraylist *cidx, Int_Arraylist *sidx)
{
  int nchain = net->ns->graph->nvertex;
  int *chain_length = iarray_malloc(nchain);
  int i;
  int j;
  int **status = (int**) malloc(sizeof(int*) * nchain);
  for (i = 0; i < nchain; i++) {
    chain_length[i] = Locseg_Chain_Length(net->chain_array + i);
    status[i] = (int*) malloc(sizeof(int) * chain_length[i]);
    for (j = 0; j < chain_length[i]; j++) {
      status[i][j] = -1;
    }
  }

  int cur_id = 0;
  int chain_index[2];
  int seg_index[2];
  for (i = 0; i < net->ns->graph->nedge; i++) {
    chain_index[0] = GRAPH_EDGE_NODE(net->ns->graph, i, 0);
    chain_index[1] = GRAPH_EDGE_NODE(net->ns->graph, i, 1);
    if (net->ns->conn[i].mode == NEUROCOMP_CONN_HL) {
      seg_index[0] = net->ns->conn[i].info[0];
      seg_index[1] = net->ns->conn[i].info[1];
    } else if (net->ns->conn[i].mode == NEUROCOMP_CONN_LINK) {
      if (net->ns->conn[i].info[0] == 0) {
	seg_index[0] = 0;
      } else {
	seg_index[1] = chain_length[i] - 1;
      }
    } else {
      continue;
    }

    int id1 = status[chain_index[0]][seg_index[0]];
    if (id1 < 0) {
      id1 = cur_id;
      status[chain_index[0]][seg_index[0]] = id1;
      Int_Arraylist_Add(cidx, chain_index[0]);
      Int_Arraylist_Add(sidx, seg_index[0]);
      cur_id++;
    }
    int id2 = status[chain_index[1]][seg_index[1]];
    if (id2 < 0) {
      id2 = cur_id;
      status[chain_index[0]][seg_index[0]] = id2;
      Int_Arraylist_Add(cidx, chain_index[1]);
      Int_Arraylist_Add(sidx, seg_index[1]);
      cur_id++;
    }

    Graph_Add_Edge(graph, id1, id2);
  }

  free(chain_length);
  FREE_2D_ARRAY(status, nchain);
}

Locseg_Chain* Locseg_Chain_Network_Find_Branch(const Neuron_Structure *net)
{
  /* alloc <branches> */
  Locseg_Chain *branches = New_Locseg_Chain();
  
  int nchain = Neuron_Structure_Component_Number(net);

  /* alloc <chain_length> */
  int *chain_length = iarray_malloc(nchain);
  int i;
  /*alloc <status> */
  int *status = (int*) malloc(sizeof(int) * nchain * 2);
  int *head_link = status;
  int *tail_link = status + nchain;

  for (i = 0; i < nchain; i++) {
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(net, i));
    TZ_ASSERT(chain != NULL, "Incorrect neuron component.");
    chain_length[i] = Locseg_Chain_Length(chain);
    head_link[i] = 0;
    tail_link[i] = 0;
  }

  int chain_index[2];

  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(net); i++) {
    chain_index[0] = GRAPH_EDGE_NODE(net->graph, i, 0);
    chain_index[1] = GRAPH_EDGE_NODE(net->graph, i, 1);
    Neurocomp_Conn *conn = Neuron_Structure_Get_Conn(net, i);
    
    if (conn->mode == NEUROCOMP_CONN_HL) { /* branch point */
      Locseg_Chain *chain = 
	NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(net, chain_index[1]));

      Local_Neuroseg *locseg = 
	Locseg_Chain_Peek_Seg_At(chain, conn->info[1]);
      Locseg_Chain_Add(branches, Copy_Local_Neuroseg(locseg), NULL, DL_TAIL);
    } else if (conn->mode == NEUROCOMP_CONN_LINK) {   
      /* calculate degree of each node */
      if (conn->info[0] == 0) {
	head_link[chain_index[0]]++;
      } else {
	tail_link[chain_index[0]]++;
      }
      if (conn->info[1] == 0) {
	head_link[chain_index[1]]++;
      } else {
	tail_link[chain_index[1]]++;
      }
    } else {
      continue;
    }
  }

  /* */
  for (i = 0; i < nchain; i++) {
    Local_Neuroseg *locseg = NULL;
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(net, i));
    if (head_link[i] > 1) {
      locseg = Locseg_Chain_Head_Seg(chain);
    }
    if (locseg != NULL) {
      Locseg_Chain_Add(branches, Copy_Local_Neuroseg(locseg), NULL, DL_TAIL);
    }
    locseg = NULL;
    if (tail_link[i] > 1) {
      locseg = Locseg_Chain_Tail_Seg(chain);
    }
    if (locseg != NULL) {
      Locseg_Chain_Add(branches, Copy_Local_Neuroseg(locseg), NULL, DL_TAIL);
    }
  }

  /* free <chain_length> */
  free(chain_length);
  /* free <status> */
  free(status);

  /* return <branches> */
  return branches;
}
