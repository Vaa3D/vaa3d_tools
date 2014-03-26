/*tz_graph_workspace.c
 *
 * 21-May-2008 Initial write: Ting Zhao
 */
#include <stdlib.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_graph.h"

/*
 * The following functions need to be changed while adding a field:
 * New_Graph_Workspace(), Clean_Graph_Workspace(), Graph_Workspace_Prepare()
 */
Graph_Workspace* New_Graph_Workspace()
{
  Graph_Workspace *gw = (Graph_Workspace *) 
    Guarded_Malloc(sizeof(Graph_Workspace), "New_Graph_Workspace");
  gw->allocated = ZERO_BIT_MASK;
  gw->ready = ZERO_BIT_MASK;
  gw->nvertex = 0;
  gw->nedge = 0;
  gw->connection = NULL;
  gw->connection_psize = 0;
  gw->parent = NULL;
  gw->parent_psize = 0;
  gw->child = NULL;
  gw->child_psize = 0;
  gw->weight = NULL;
  gw->weight_psize = 0;
  gw->mindist = NULL;
  gw->mindist_psize = 0;
  gw->idx = NULL;
  gw->idx_psize = 0;
  gw->degree = NULL;
  gw->in_degree = NULL;
  gw->out_degree = NULL;
  gw->vlist = NULL;
  gw->elist = NULL;
  gw->dlist = NULL;
  gw->edge_table = NULL;
  gw->edge_map = NULL;
  gw->status = NULL;

  return gw;
}

void Free_Graph_Workspace(Graph_Workspace *gw)
{
  free(gw);
}

#define CLEAN_GRAPH_WORKSPACE_FIELD(field, free_func)	\
  if (field != NULL) {					\
    free_func(field);					\
    field = NULL;					\
  }

void Clean_Graph_Workspace(Graph_Workspace *gw)
{
  gw->allocated = ZERO_BIT_MASK;
  gw->ready = ZERO_BIT_MASK;

#ifdef _MSC_VER
  int field_id;
#else
  Graph_Workspace_Field_e field_id;
#endif
  for (field_id = GRAPH_WORKSPACE_FIELD_BEGIN + 1; 
       field_id < GRAPH_WORKSPACE_FIELD_END; field_id++) {
    Graph_Workspace_Prepare(gw, field_id);
  }

  gw->nvertex = 0;
  gw->nedge = 0;
  gw->connection_psize = 0;
  gw->parent_psize = 0;
  gw->child_psize = 0;
  gw->idx_psize = 0;
  gw->weight_psize = 0;
  gw->mindist_psize = 0;
}

void Kill_Graph_Workspace(Graph_Workspace *gw)
{
  if (gw == NULL) {
    return;
  }

  Clean_Graph_Workspace(gw);
  free(gw);
}

void Graph_Workspace_Load(Graph_Workspace *gw, const Graph *graph)
{
  gw->nvertex = graph->nvertex;
  gw->nedge = graph->nedge;
}

void Graph_Workspace_Reload(Graph_Workspace *gw, const Graph *graph)
{
  Clean_Graph_Workspace(gw);

  gw->nvertex = graph->nvertex;
  gw->nedge = graph->nedge;
}

void Graph_Workspace_Prepare(Graph_Workspace *gw, int field_id)
{
  if ((Graph_Workspace_Ready(gw, field_id) == FALSE) && 
      (Graph_Workspace_Allocated(gw, field_id) == FALSE)) {
    switch (field_id) {
    case GRAPH_WORKSPACE_CONNECTION:
      FREE_2D_ARRAY(gw->connection, gw->connection_psize);
      gw->connection_psize = 0;
      break;
    case GRAPH_WORKSPACE_PARENT:
      FREE_2D_ARRAY(gw->parent, gw->parent_psize);
      gw->parent_psize = 0;
      break;
    case GRAPH_WORKSPACE_CHILD:
      FREE_2D_ARRAY(gw->child, gw->child_psize);
      gw->child_psize = 0;
      break;
    case GRAPH_WORKSPACE_WEIGHT:
      FREE_2D_ARRAY(gw->weight, gw->weight_psize); 
      gw->weight_psize = 0;
      break;
    case GRAPH_WORKSPACE_MINDIST:
      FREE_2D_ARRAY(gw->mindist, gw->mindist_psize);
      gw->mindist_psize = 0;
      break;
    case GRAPH_WORKSPACE_IDX:
      FREE_2D_ARRAY(gw->idx, gw->idx_psize);
      gw->idx_psize = 0;
      break;
    case GRAPH_WORKSPACE_DEGREE:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->degree, free);
      break;
    case GRAPH_WORKSPACE_IN_DEGREE:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->in_degree, free);
      break;
    case GRAPH_WORKSPACE_OUT_DEGREE:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->out_degree, free);
      break;
    case GRAPH_WORKSPACE_ELIST:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->elist, free);
      break;
    case GRAPH_WORKSPACE_VLIST:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->vlist, free);
      break;
    case GRAPH_WORKSPACE_DLIST:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->dlist, free);
      break;  
    case GRAPH_WORKSPACE_EDGE_TABLE:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->edge_table, Kill_Hash_Table);
      break;
    case GRAPH_WORKSPACE_EDGE_MAP:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->edge_map, Kill_Int_Arraylist);
      break;
    case GRAPH_WORKSPACE_STATUS:
      CLEAN_GRAPH_WORKSPACE_FIELD(gw->status, free);
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
    }
  }
}

void Graph_Workspace_Alloc(Graph_Workspace *gw, int field_id)
{
  switch(field_id) {
  case GRAPH_WORKSPACE_VLIST:
    if (gw->vlist == NULL) {
      gw->vlist = (int *) Guarded_Malloc(sizeof(int) * gw->nvertex,
					 "Graph_Workspace_Alloc");
    }
    break;
  case GRAPH_WORKSPACE_ELIST:
    if (gw->elist == NULL) {
      gw->elist = (int *) Guarded_Malloc(sizeof(int) * gw->nedge,
					 "Graph_Workspace_Alloc");
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
}

BOOL Graph_Workspace_Allocated(const Graph_Workspace *gw, int field_id)
{
  return Bitmask_Get_Bit(gw->allocated, field_id);
}

BOOL Graph_Workspace_Ready(const Graph_Workspace *gw, int field_id)
{
  return Bitmask_Get_Bit(gw->ready, field_id);
}

void Graph_Workspace_Set_Readiness(Graph_Workspace *gw, int field_id, 
				   BOOL status)
{
  Bitmask_Set_Bit(field_id, status, &(gw->ready));
}

void Graph_Workspace_Set_Allocation(Graph_Workspace *gw, int field_id, 
				    BOOL status)
{
  Bitmask_Set_Bit(field_id, status, &(gw->allocated));
}
