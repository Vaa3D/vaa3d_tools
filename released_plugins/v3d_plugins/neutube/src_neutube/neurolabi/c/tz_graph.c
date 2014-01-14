/* tz_graph.c
 *
 * 21-May-2008 Initial write: Ting Zhao
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_utilities.h"
#include "tz_graph_utils.h"
#include "tz_graph.h"
#include "tz_intpair_map.h"
#include "tz_interface.h"
#include "tz_unipointer_linked_list.h"

#include "private/tz_graph_utils.c"

#define GROWTH_FACTOR 2

Graph* New_Graph()
{
  Graph *graph = (Graph *) Guarded_Malloc(sizeof(Graph), "New_Graph");
  graph->directed = FALSE;
  graph->type = GENERAL_GRAPH;
  graph->nvertex = 0;
  graph->nedge = 0;
  graph->edge_capacity = 0;
  graph->edges = NULL;
  graph->weights = NULL;

  return graph;
}

void Delete_Graph(Graph *graph)
{
  free(graph);
}

void Construct_Graph(Graph *graph, int nvertex, int edge_capacity, 
		     BOOL weighted)
{
  ASSERT(edge_capacity <= Max_Edge_Capacity, "Too many edges.");

  graph->nvertex = nvertex;
  graph->edge_capacity = edge_capacity;
  if (graph->edges != NULL) {
    free(graph->edges);
  }
  graph->edges = (graph_edge_t *) 
    Guarded_Malloc(sizeof(graph_edge_t) * edge_capacity, "Construct_Graph");

  if (graph->weights != NULL) {
    free(graph->weights);
    graph->weights = NULL;
  }
  
  if (weighted == TRUE) {
      graph->weights = (double *) Guarded_Malloc(sizeof(double) * edge_capacity,
              "Construct_Graph");
  }
}

void Clean_Graph(Graph *graph)
{
  if (graph->edges != NULL) {
    free(graph->edges);
    graph->edges = NULL;
  }

  if (graph->weights != NULL) {
    free(graph->weights);
    graph->weights = NULL;
  }

  graph->edge_capacity = 0;
  graph->nvertex = 0;
  graph->nedge = 0;
}

Graph* Make_Graph(int nvertex, int edge_capacity, BOOL weighted)
{
  Graph *graph = New_Graph();
  Construct_Graph(graph, nvertex, edge_capacity, weighted);
  return graph;
}

void Kill_Graph(Graph *graph)
{						
  Clean_Graph(graph);
  free(graph);
}

Graph* Copy_Graph(const Graph *graph)
{
  Graph *graph_copy = Make_Graph(GRAPH_VERTEX_NUMBER(graph), 
      graph->edge_capacity, Graph_Is_Weighted(graph));
  int i;
  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    graph_copy->edges[i][0] = graph->edges[i][0];
    graph_copy->edges[i][1] = graph->edges[i][1];
    if (Graph_Is_Weighted(graph)) {
      graph_copy->weights[i] = graph->weights[i];
    }
  }

  graph_copy->type = graph->type;
  graph_copy->nedge = graph->nedge;

  return graph_copy;
}

void Print_Graph(const Graph *graph)
{
  if (graph == NULL) {
    printf("Null Graph pointer\n");
    return;
  }

  if (graph->directed == TRUE) {
    printf("Directed graph:");
  } else {
    printf("Undirected graph:");
  }

  printf(" %d vertices; ", graph->nvertex);
  printf(" %d edges\n", graph->nedge);

  if (graph->nedge > 0) {
    int i;
    for (i = 0; i < graph->nedge; i++) {
      printf("%d", graph->edges[i][0]);
      if (graph->directed == TRUE) {
	printf(" --> ");
      } else {
	printf(" -- ");
      }
      printf("%d", graph->edges[i][1]);
      if (graph->weights != NULL) {
	printf(": %g", graph->weights[i]);
      }
      printf("\n");   
    }
  }
}

void Print_Graph_Info(const Graph *graph)
{
  if (graph->directed == TRUE) {
    printf("Directed graph:");
  } else {
    printf("Undirected graph:");
  }

  printf(" %d vertices; ", graph->nvertex);
  printf(" %d edges\n", graph->nedge);
}

BOOL Graph_Is_Directed(const Graph *graph) 
{
  return graph->directed;
}

void Graph_Set_Directed(Graph *graph, BOOL directed)
{
  BOOL remove_edge = FALSE;
  if (graph->directed && (directed == FALSE)) {
    remove_edge = TRUE;
  }
  graph->directed = directed;
  if (remove_edge) {
    Graph_Remove_Duplicated_Edge(graph);
  }
}

BOOL Graph_Is_Weighted(const Graph *graph)
{
  if (graph->weights == NULL) {
    return FALSE;
  }

  return TRUE;
}

void Graph_Set_Weighted(Graph *graph, BOOL weighted)
{
  if (weighted == FALSE) {
    if (graph->weights != NULL) {
      free(graph->weights);
      graph->weights = NULL;
    }
  } else {
    if (graph->edge_capacity > 0) {
      GUARDED_CALLOC_ARRAY(graph->weights, graph->edge_capacity, double);
    }
  }
}

static void graph_edge_grow(Graph *graph)
{
  if (graph->edge_capacity == 0) {
    graph->edge_capacity = GROWTH_FACTOR;    
    graph->edges = (graph_edge_t *) 
      Guarded_Malloc(sizeof(graph_edge_t) *graph->edge_capacity,
		     "graph_edge_grow");
  } else {
    graph->edge_capacity = GROWTH_FACTOR * graph->nedge;

    ASSERT(graph->edge_capacity <= Max_Edge_Capacity, 
	   "Reach the edge capacity limit. Edge growing failed");
    /*
    graph_edge_t *new_edge = (graph_edge_t *) 
      Guarded_Malloc(sizeof(graph_edge_t) *graph->edge_capacity,
		     "graph_edge_grow");
    memcpy(new_edge, graph->edges, sizeof(graph_edge_t) * graph->nedge);
    free(graph->edges);
    graph->edges = new_edge;
    */    
    graph->edges = (graph_edge_t *) 
      Guarded_Realloc(graph->edges, sizeof(graph_edge_t) *graph->edge_capacity,
    		      "graph_edge_grow");
  }

  if (graph->weights != NULL) {
    /*
    double *new_weight = (double *) 
      Guarded_Malloc(sizeof(double) *graph->edge_capacity,
		     "graph_edge_grow");
    memcpy(new_weight, graph->weights, sizeof(double) * graph->nedge);
    free(graph->weights);
    graph->weights = new_weight;
    */
    graph->weights = (double *) 
      Guarded_Realloc(graph->weights, sizeof(double) *graph->edge_capacity,
		      "graph_edge_grow");
  }
}

void Graph_Add_Edge(Graph *graph, int v1, int v2)
{
  if (graph->nedge >= graph->edge_capacity) {
    graph_edge_grow(graph);
  }
  graph->nedge++;

  graph->edges[graph->nedge - 1][0] = v1;
  if (graph->nvertex < v1 + 1) {
    graph->nvertex = v1 + 1;
  }
  graph->edges[graph->nedge - 1][1] = v2;
  if (graph->nvertex < v2 + 1) {
    graph->nvertex = v2 + 1;
  }

  if (graph->weights != NULL) {
    graph->weights[graph->nedge - 1] = 0.0;
  }
}

void Graph_Add_Weighted_Edge(Graph *graph, int v1, int v2, double weight)
{
  Graph_Add_Edge(graph, v1, v2);
  
  if (graph->weights != NULL) {
    graph->weights[graph->nedge - 1] = weight;
  }
}

void Graph_Remove_Edge(Graph *graph, int edge_index)
{
  if ((edge_index >= graph->nedge) || (edge_index < 0)){
    return;
  }

  memmove(graph->edges + edge_index, graph->edges + edge_index + 1,
	  sizeof(graph_edge_t) * (graph->nedge - edge_index - 1));
  
  if (graph->weights != NULL) {
    memmove(graph->weights + edge_index, graph->weights + edge_index + 1,
	  sizeof(double) * (graph->nedge - edge_index - 1));
  }
  graph->nedge--;
}

int Graph_Remove_Edge_List(Graph *graph, const int *edge_index, int nedge)
{
  uint8_t *mask = u8array_calloc(GRAPH_EDGE_NUMBER(graph));
  int i;
  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    mask[i] = 0;
  }

  int nrm = 0;

  for (i = 0; i < nedge; i++) {
    if ((edge_index[i] < graph->nedge) && (edge_index[i] >= 0)){
      mask[edge_index[i]] = 1;
      nrm++;
    }
  }

  int end = 0;

  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    if (mask[i] == 0) {
      graph->edges[end][0] = graph->edges[i][0];
      graph->edges[end][1] = graph->edges[i][1];
      if (Graph_Is_Weighted(graph)) {
	graph->weights[end] = graph->weights[i];
      }
      end++;
    }
  }

  graph->nedge -= nrm;

  free(mask);

  return nrm;
}

int Graph_Remove_Vertex(Graph *graph, int vertex)
{
  if (graph->nedge == 0) {
    return 0;
  }

  int nedge = 0;
  int *edge_index = iarray_malloc(graph->nedge);
  int k = 0;
  int i;
  for (i = 0; i < graph->nedge; i++) {
    if ((GRAPH_EDGE_NODE(graph, i, 0) == vertex) ||
	(GRAPH_EDGE_NODE(graph, i, 1) == vertex)) {
      nedge++;
      edge_index[k++] = i;
    }
  }
  Graph_Remove_Edge_List(graph, edge_index, nedge);
  free(edge_index);

  return nedge;
}

/* change gw->edge_table and edge_map */
void Graph_Update_Edge_Table(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_EDGE_TABLE) == TRUE) {
    return;
  }

  gw->nedge = graph->nedge;

  if (gw->edge_table != NULL) {
    Kill_Hash_Table(gw->edge_table);
    Kill_Int_Arraylist(gw->edge_map);
  }

  gw->edge_table = New_Hash_Table(graph->nedge);
  gw->edge_map = Int_Arraylist_New(graph->nedge, 0);

  int i;
  for (i = 0; i < gw->nedge; i++) {
    sprintf(gw->edge_entry, "%d_%d", graph->edges[i][0], graph->edges[i][1]);
    int map = Hash_Add(gw->edge_table, gw->edge_entry);
    gw->edge_map->array[map] = i;
  }
}

/* change edge table and edge map */
void Graph_Expand_Edge_Table(int v1, int v2, int edge_idx, Graph_Workspace *gw)
{
  if (Graph_Edge_Index(v1, v2, gw) == -1) {
    if (gw->edge_table == NULL) {
      gw->edge_table = New_Hash_Table(1);
      ASSERT(gw->edge_map == NULL, "Wrong edge map.");
      gw->edge_map = Int_Arraylist_New(1, 0);
    }

    gw->nedge++;
    sprintf(gw->edge_entry, "%d_%d", v1, v2);
    int map = Hash_Add(gw->edge_table, gw->edge_entry);
    Int_Arraylist_Set(gw->edge_map, map, edge_idx);
  }
}

/* use edge_table and edge_map */
int Graph_Edge_Index(int v1, int v2, Graph_Workspace *gw)
{
  if (gw->edge_table == NULL) {
    return -1;
  }

  sprintf(gw->edge_entry, "%d_%d", v1, v2);
  int index = Hash_Lookup(gw->edge_table, gw->edge_entry);
  if (index >= 0) {
    return gw->edge_map->array[index];
  }

  return -1;
}

int Graph_Edge_Index_U(int v1, int v2, Graph_Workspace *gw)
{
  int idx = Graph_Edge_Index(v1, v2, gw);
  if (idx < 0) {
    idx = Graph_Edge_Index(v2, v1, gw);
  }
  
  return idx;
}

void Graph_Insert_Node(Graph *graph, int v1, int v2, int v, 
		       Graph_Workspace *gw)
{
  Graph_Update_Edge_Table(graph, gw);

  int idx = Graph_Edge_Index_U(v1, v2, gw);

  if (idx >= 0) {
    GRAPH_EDGE_NODE(graph, idx, 0) = v1;
    GRAPH_EDGE_NODE(graph, idx, 1) = v;
  } else {
    Graph_Add_Edge(graph, v1, v);
  }
  Graph_Add_Edge(graph, v, v2);
}

void Graph_Normalize_Edge(Graph *graph)
{
  if (graph->directed == FALSE) {
    int i;
    int tmp;
    for (i = 0; i < graph->nedge; i++) {
      if (graph->edges[i][0] > graph->edges[i][1]) {
	tmp = graph->edges[i][0];
	graph->edges[i][0] = graph->edges[i][1];
	graph->edges[i][1] = tmp;
      }
    }
  }
}

int Graph_Remove_Duplicated_Edge(Graph *graph)
{
  Intpair_Map *edge_map = New_Intpair_Map();
  Int_Arraylist *remove_list = Make_Int_Arraylist(0, 1);

  int i;
  for (i = 0; i < graph->nedge; i++) {
    if (Intpair_Map_Value(edge_map, graph->edges[i][0], graph->edges[i][1])
	< 0) {
      Intpair_Map_Add(edge_map, graph->edges[i][0], graph->edges[i][1], i);
    } else {
      Int_Arraylist_Add(remove_list, i);
    }
  }

  int nremoved = remove_list->length;

  Graph_Remove_Edge_List(graph, remove_list->array, remove_list->length);

  Kill_Int_Arraylist(remove_list);
  Kill_Intpair_Map(edge_map);

  return nremoved;
}

int Graph_Edge_Count(const Graph *graph, graph_edge_t* const edges, int n,
		     Graph_Workspace *gw)
{
  Graph_Update_Edge_Table(graph, gw);
  int count = 0;
  int i;
  for (i = 0; i < n; i++) {
    if (Graph_Edge_Index(edges[i][0], edges[i][1], gw) >= 0) {
      count++;
    }
  }

  return count;
}

/* change gw->weight */
double** Graph_Weight_Matrix(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_WEIGHT)) {
    return gw->weight;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_WEIGHT);
  Graph_Workspace_Load(gw, graph);

  int i, j;
  if (gw->weight == NULL) {
    MALLOC_2D_ARRAY(gw->weight, gw->nvertex, gw->nvertex, double, i);
  }

  
  for (i = 0; i < graph->nvertex; i++) {
    for (j = 0; j < graph->nvertex; j++) {
      if (i == j) {
	gw->weight[i][j] = 0;
      } else {
	gw->weight[i][j] = Infinity;
      }
    }
  }

  double weight = 0.0;

  for (i = 0; i < graph->nedge; i++) {
    if (graph->weights == NULL) {
      weight = 1.0;
    } else {
      weight = graph->weights[i];
    }

    gw->weight[graph->edges[i][0]][graph->edges[i][1]] = weight;
    if (graph->directed == FALSE) {
      gw->weight[graph->edges[i][1]][graph->edges[i][0]] = weight;
    }
  }

  return gw->weight;
}

/* change gw->degree */
int* Graph_Degree(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_DEGREE) == TRUE) {
    return gw->degree;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);
  Graph_Workspace_Load(gw, graph);

  if (gw->degree == NULL) {
    gw->degree = (int *) Guarded_Malloc(sizeof(int) * graph->nvertex,
					"Graph_Degree");
  }

  int i;
  for (i = 0; i < graph->nvertex; i++) {
    gw->degree[i] = 0;
  }
  for (i = 0; i < graph->nedge; i++) {
    (gw->degree[graph->edges[i][0]])++;
    (gw->degree[graph->edges[i][1]])++;
  }

  return gw->degree;
}

int* Graph_In_Degree(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Is_Directed(graph) == FALSE) {
    return Graph_Degree(graph, gw);
  }

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_IN_DEGREE) == TRUE) {
    return gw->in_degree;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_IN_DEGREE);
  Graph_Workspace_Load(gw, graph);

  if (gw->in_degree == NULL) {
    gw->in_degree = (int*) Guarded_Malloc(sizeof(int) * graph->nvertex,
					  "Graph_In_Degree");
  }

  int i;
  for (i = 0; i < graph->nvertex; i++) {
    gw->in_degree[i] = 0;
  }
  for (i = 0; i < graph->nedge; i++) {
    (gw->in_degree[graph->edges[i][1]])++;
  }

  return gw->in_degree;
}

int* Graph_Out_Degree(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Is_Directed(graph) == FALSE) {
    return Graph_Degree(graph, gw);
  }

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_OUT_DEGREE) == TRUE) {
    return gw->out_degree;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_OUT_DEGREE);
  Graph_Workspace_Load(gw, graph);

  if (gw->out_degree == NULL) {
    gw->out_degree = (int*) Guarded_Malloc(sizeof(int) * graph->nvertex,
					   "Graph_In_Degree");
  }

  int i;
  for (i = 0; i < graph->nvertex; i++) {
    gw->out_degree[i] = 0;
  }
  for (i = 0; i < graph->nedge; i++) {
    (gw->out_degree[graph->edges[i][0]])++;
  }

  return gw->out_degree;
}

/* change gw->connection */
int** Graph_Adjmat(const Graph *graph, int diag, Graph_Workspace *gw)
{
  TZ_ASSERT((diag == 0) || (diag == 1), "Invalid diag value");

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_CONNECTION) == TRUE) {
    return gw->connection;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);
  Graph_Workspace_Load(gw, graph);

  int i, j;
  if (gw->connection == NULL) {
    MALLOC_2D_ARRAY(gw->connection, graph->nvertex, graph->nvertex, int, i);
    gw->connection_psize = graph->nvertex;
  }
  
  for (i = 0; i < graph->nvertex; i++) {
    for (j = 0; j < graph->nvertex; j++) {
      if (i == j) {
	gw->connection[i][j] = diag;
      } else {
	gw->connection[i][j] = 0;
      }
    }
  }

  if (Graph_Is_Directed(graph)) {
    for (i = 0; i < graph->nedge; i++) {
      gw->connection[GRAPH_EDGE_NODE(graph, i, 0)]
	[GRAPH_EDGE_NODE(graph, i, 1)] = 1;
    }
  } else {
    for (i = 0; i < graph->nedge; i++) {
      gw->connection[GRAPH_EDGE_NODE(graph, i, 0)]
	[GRAPH_EDGE_NODE(graph, i, 1)] = 1;
      gw->connection[GRAPH_EDGE_NODE(graph, i, 1)]
	[GRAPH_EDGE_NODE(graph, i, 0)] = 1;
    }
  }

  return gw->connection;
}

/* change gw->degree, gw->connection, gw->idx */
int** Graph_Neighbor_List(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_CONNECTION) == TRUE) {
    return gw->connection;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_IDX);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);

  Graph_Workspace_Load(gw, graph);

  int* degree = Graph_Degree(graph, gw);

  int i;
  if (gw->connection == NULL) {
    gw->connection = (int **) 
      Guarded_Malloc(sizeof(int*) * graph->nvertex, "Graph_Neighor_List");
    for (i = 0; i < graph->nvertex; i++) {
      if (degree[i] > 0) {
        gw->connection[i] = (int *) 
          Guarded_Malloc(sizeof(int) * (degree[i] + 1), "Graph_Neighor_List");
      } else {
        gw->connection[i] = NULL;
      }
    }
    gw->connection_psize = graph->nvertex;
  }

  if (gw->idx == NULL) {
    gw->idx = (int **) 
      Guarded_Malloc(sizeof(int*) * graph->nvertex, "Graph_Neighor_List");
    for (i = 0; i < graph->nvertex; i++) {
      if (degree[i] > 0) {
        gw->idx[i] = (int *) 
          Guarded_Malloc(sizeof(int) * (degree[i] + 1), "Graph_Neighor_List");
      } else {
        gw->idx[i] = NULL;
      }
    }
    gw->idx_psize = graph->nvertex;
  }

  Graph_Edge_Neighbor_List(graph->nvertex, graph->edges, graph->nedge, 
			   gw->connection, gw->idx);

  return gw->connection;
}

/* change gw->out_degree, gw->connection, gw->idx */
int** Graph_Child_List(const Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_CONNECTION) == TRUE) {
    return gw->connection;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CHILD);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_IDX);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_OUT_DEGREE);

  Graph_Workspace_Load(gw, graph);

  int* out_degree = Graph_Out_Degree(graph, gw);

  int i;
  if (gw->child == NULL) {
    gw->child = (int **) 
      Guarded_Malloc(sizeof(int*) * graph->nvertex, "Graph_Child_List");
    for (i = 0; i < graph->nvertex; i++) {
      gw->child[i] = (int *) 
	Guarded_Malloc(sizeof(int) * (out_degree[i] + 1), "Graph_Child_List");
    }
    gw->child_psize = graph->nvertex;
  }

  if (gw->idx == NULL) {
    gw->idx = (int **) 
      Guarded_Malloc(sizeof(int*) * graph->nvertex, "Graph_Child_List");
    for (i = 0; i < graph->nvertex; i++) {
      gw->idx[i] = (int *) 
	Guarded_Malloc(sizeof(int) * (out_degree[i] + 1), "Graph_Child_List");
    }
    gw->idx_psize = graph->nvertex;
  }

  Graph_Edge_Child_List(graph->nvertex, graph->edges, graph->nedge, 
			gw->child, gw->idx);

  return gw->child;
}

/* change gw->degree */
void Graph_Cut_Leaves(Graph *graph, Graph_Workspace *gw)
{
  Graph_Degree(graph, gw);

  int i, j;
  j = 0;
  for (i = 0; i < gw->nedge; i++) {
    if ((gw->degree[graph->edges[i][0]] > 1) &&
	(gw->degree[graph->edges[i][1]] > 1)) {
      graph->edges[j][0] = graph->edges[i][0];
      graph->edges[j][1] = graph->edges[i][1];
      if (graph->weights != NULL) {
	graph->weights[j] = graph->weights[i];
      }
      j++;
    }
  }

  graph->nedge = j;
}

/* change gw->degree */
void Graph_Prune(Graph *graph, int n, Graph_Workspace *gw)
{
  Graph_Degree(graph, gw);

  int i, j, k;
  
  for (k = 1; k < n; k++) {
    for (i = 0; i < gw->nedge; i++) {
      if (gw->degree[graph->edges[i][0]] == 1) {
	  if (gw->degree[graph->edges[i][1]] > 1) {
	    gw->degree[graph->edges[i][1]]--;
	  }
      } else if (gw->degree[graph->edges[i][1]] == 1) {
	gw->degree[graph->edges[i][0]]--;
      }
    }
  }
  
  j = 0;
  for (i = 0; i < gw->nedge; i++) {
    if ((gw->degree[graph->edges[i][0]] > 1) &&
	(gw->degree[graph->edges[i][1]] > 1)) {
      graph->edges[j][0] = graph->edges[i][0];
      graph->edges[j][1] = graph->edges[i][1];
      if (graph->weights != NULL) {
	graph->weights[j] = graph->weights[i];
      }
      j++;
    }
  }

  graph->nedge = j;
}

/* change gw->vlist, gw->status, gw->dlist, gw->connection */
int* Graph_Shortest_Path(const Graph *graph, int start, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_VLIST) == TRUE) {
    return gw->vlist;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  //Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DLIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);

  Graph_Workspace_Load(gw, graph);

  if (gw->vlist == NULL) {
    gw->vlist = iarray_malloc(gw->nvertex);
  }

  if (gw->dlist == NULL) {
    gw->dlist = darray_malloc(gw->nvertex);
  }

  Graph_Neighbor_List(graph, gw);

  int **neighbors = gw->connection;
  //int8_t *checked = (int8_t *) gw->status;
  int *checked = NULL;
  double *dist = gw->dlist;
  int *path = gw->vlist;

  int i, j;

  if (checked == NULL) {
    checked = iarray_malloc(gw->nvertex);
  }

  for (i = 0; i < gw->nvertex; i++) {
    dist[i] = Infinity;
    path[i] = -1;
    checked[i] = 0;
  }

  dist[start] = 0;
  path[start] = -1;
  checked[start] = 1;
  
  int prev_vertex = start;
  int cur_vertex = start;
  int updating_vertex;
  double tmpdist;

  PROGRESS_BEGIN("Finding shortest path");

  Int_Arraylist *heap = Int_Arraylist_New(1, 0);

  for (i = 1; i < gw->nvertex; i++) {
    PROGRESS_STATUS(i * 100 / gw->nvertex);
    prev_vertex = cur_vertex;
    for (j = 1; j <= NUMBER_OF_NEIGHBORS(cur_vertex, neighbors); j++) {
      updating_vertex = NEIGHBOR_OF(cur_vertex, j, neighbors);
      if (checked[updating_vertex] != 1) {
	if (graph->weights != NULL) {
	  tmpdist = graph->weights[gw->idx[cur_vertex][j]] + dist[cur_vertex];
	} else {
	  tmpdist = 1.0 + dist[cur_vertex];
	}
	if (dist[updating_vertex] > tmpdist) {
	  dist[updating_vertex] = tmpdist;
	  path[updating_vertex] = cur_vertex;
	  
	  if (checked[updating_vertex] > 1) {
#ifdef _DEBUG_2
	    printf("Verify after updating: \n");
#endif
	    Int_Heap_Update_I(heap, updating_vertex, dist, checked);
	  } else {
#ifdef _DEBUG_2
	    printf("Verify after adding: \n");
#endif
	    Int_Heap_Add_I(heap, updating_vertex, dist, checked);
	  }
#ifdef _DEBUG_2

	  Print_Int_Heap_I(heap, dist);
	  if (Verify_Int_Heap_I(heap, dist) > 0) {
	    printf("%d, %d, %g\n", cur_vertex, updating_vertex,
		   dist[updating_vertex]);
	    exit(1);
	  }
#endif
	}
      }
    }

#ifdef _DEBUG_2
    printf("Verify before min extract: \n");
    Print_Int_Heap_I(heap, dist);
    if (Verify_Int_Heap_I(heap, dist) > 0) {
      printf("invalid heap\n");
      exit(1);
    }
#endif
    cur_vertex = extract_min(dist, checked, gw->nvertex, heap);
#ifdef _DEBUG_2
    printf("Verify: \n");
    Print_Int_Heap_I(heap, dist);
    if (Verify_Int_Heap_I(heap, dist) > 0) {
      exit(1);
    }
#endif

    PROGRESS_REFRESH;

    if (cur_vertex < 0) {
      break;
    }
  }

  Kill_Int_Arraylist(heap);
  free(checked);

  PROGRESS_END("done");

  return gw->vlist;
}

/* change gw->vlist, gw->status, gw->dlist, gw->connection */
int* Graph_Shortest_Path_E(const Graph *graph, int start, int end,
			   Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_VLIST) == TRUE) {
    return gw->vlist;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  //Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DLIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);

  Graph_Workspace_Load(gw, graph);

  if (gw->vlist == NULL) {
    gw->vlist = iarray_malloc(gw->nvertex);
  }

  if (gw->dlist == NULL) {
    gw->dlist = darray_malloc(gw->nvertex);
  }

  Graph_Neighbor_List(graph, gw);

  int **neighbors = gw->connection;
  //int8_t *checked = (int8_t *) gw->status;
  int *checked = NULL;
  double *dist = gw->dlist;
  int *path = gw->vlist;

  int i, j;

  if (checked == NULL) {
    checked = iarray_malloc(gw->nvertex);
  }

  for (i = 0; i < gw->nvertex; i++) {
    dist[i] = Infinity;
    path[i] = -1;
    checked[i] = 0;
  }

  dist[start] = 0;
  path[start] = -1;
  checked[start] = 1;
  
  int prev_vertex = start;
  int cur_vertex = start;
  int updating_vertex;
  double tmpdist;

  PROGRESS_BEGIN("Finding shortest path");

  Int_Arraylist *heap = Int_Arraylist_New(1, 0);

  for (i = 1; i < gw->nvertex; i++) {
    PROGRESS_STATUS(i * 100 / gw->nvertex);
    prev_vertex = cur_vertex;

    for (j = 1; j <= NUMBER_OF_NEIGHBORS(cur_vertex, neighbors); j++) {
      updating_vertex = NEIGHBOR_OF(cur_vertex, j, neighbors);
      if (checked[updating_vertex] != 1) {
        TZ_ASSERT(dist[cur_vertex] >= 0.0, "Negative distance");

	if (graph->weights != NULL) {
	  tmpdist = graph->weights[gw->idx[cur_vertex][j]] + dist[cur_vertex];
#ifdef _DEBUG_2
          printf("tmpdist: %g\n", tmpdist);
#endif
	} else {
	  tmpdist = 1.0 + dist[cur_vertex];
	}
	if (dist[updating_vertex] > tmpdist) {
	  dist[updating_vertex] = tmpdist;
	  path[updating_vertex] = cur_vertex;
	  
	  if (checked[updating_vertex] > 1) {
	    Int_Heap_Update_I(heap, updating_vertex, dist, checked);
	  } else {
	    Int_Heap_Add_I(heap, updating_vertex, dist, checked);
#ifdef _DEBUG_2
            if (Verify_Int_Heap_I(heap, dist) > 0) {
              printf("Wrong heap.\n");
              exit(1);
            }
#endif
	  }
	}
      }
    }

    cur_vertex = extract_min(dist, checked, gw->nvertex, heap);

#ifdef _DEBUG_2
    if (cur_vertex >= 0) {
      printf("V: %d %g\n",cur_vertex, dist[cur_vertex]);
    }
#endif

    if (cur_vertex == end) {
      break;
    }

    PROGRESS_REFRESH;

    if (cur_vertex < 0) {
      break;
    }
  }

  Kill_Int_Arraylist(heap);
  free(checked);

  PROGRESS_END("done");

  return path;
}

int* Graph_Shortest_Path_Maxmin(const Graph *graph, int start, int end,
    double *value, Graph_Workspace *gw)
{
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_VLIST) == TRUE) {
    return gw->vlist;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  //Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DLIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);

  Graph_Workspace_Load(gw, graph);

  if (gw->vlist == NULL) {
    gw->vlist = iarray_malloc(gw->nvertex);
  }

  if (gw->dlist == NULL) {
    gw->dlist = darray_malloc(gw->nvertex);
  }

  Graph_Neighbor_List(graph, gw);

  int **neighbors = gw->connection;
  //int8_t *checked = (int8_t *) gw->status;
  int *checked = NULL;
  double *dist = gw->dlist;
  int *path = gw->vlist;

  int i, j;

  if (checked == NULL) {
    checked = iarray_malloc(gw->nvertex);
  }

  for (i = 0; i < gw->nvertex; i++) {
    dist[i] = Infinity;
    path[i] = -1;
    checked[i] = 0;
  }

  dist[start] = -value[start];
  path[start] = -1;
  checked[start] = 1;
  
  int prev_vertex = start;
  int cur_vertex = start;
  int updating_vertex;
  double tmpdist;

  PROGRESS_BEGIN("Finding shortest path");

  Int_Arraylist *heap = Int_Arraylist_New(1, 0);

  for (i = 1; i < gw->nvertex; i++) {
    PROGRESS_STATUS(i * 100 / gw->nvertex);
    prev_vertex = cur_vertex;
    for (j = 1; j <= NUMBER_OF_NEIGHBORS(cur_vertex, neighbors); j++) {
      updating_vertex = NEIGHBOR_OF(cur_vertex, j, neighbors);
      if (checked[updating_vertex] != 1) {
        tmpdist = dmax2(-value[updating_vertex], dist[cur_vertex]);
        /*
	if (graph->weights != NULL) {
	  tmpdist = graph->weights[gw->idx[cur_vertex][j]] + dist[cur_vertex];
	} else {
	  tmpdist = 1.0 + dist[cur_vertex];
	}
        */
	if (dist[updating_vertex] > tmpdist) {
	  dist[updating_vertex] = tmpdist;
	  path[updating_vertex] = cur_vertex;
	  
	  if (checked[updating_vertex] > 1) {
	    Int_Heap_Update_I(heap, updating_vertex, dist, checked);
	  } else {
	    Int_Heap_Add_I(heap, updating_vertex, dist, checked);
	  }
	}
      }
    }

    cur_vertex = extract_min(dist, checked, gw->nvertex, heap);

    if (cur_vertex== end) {
      break;
    }

    PROGRESS_REFRESH;

    if (cur_vertex < 0) {
      break;
    }
  }

  Kill_Int_Arraylist(heap);
  free(checked);

  PROGRESS_END("done");

  return path;
}

void Graph_To_Mst(Graph *graph, Graph_Workspace *gw)
{
  if (graph->directed == TRUE) {
    PRINT_EXCEPTION("Unsuppoted graph","The graph must be directed.");
    return;
  }

  if (graph->weights == NULL) {
    PRINT_EXCEPTION("Unsuppoted graph","The graph must be weighted.");
    return;
  }

  Graph_Weight_Matrix(graph, gw);
  Graph_Mst_From_Adjmat(gw->weight, graph->edges, graph->nvertex,
			&(graph->nedge), NULL);
  int i;
  for (i = 0; i < graph->nedge; i++) {
    graph->weights[i] = gw->weight[graph->edges[i][0]][graph->edges[i][1]];
  }
}

#define GRAPH_MST_CHANGE_TREE_ID(id2, id1, next)		\
  next = id2;							\
  while (next >=0 ) {						\
    tree_id[next] = id1;					\
    next = connection[next];					\
  }								\
  next = id1;							\
  while (connection[next] >= 0) {				\
    next = connection[next];					\
  }								\
  connection[next] = id2;
  

void Graph_To_Mst2(Graph *graph, Graph_Workspace *gw)
{
  if (GRAPH_EDGE_NUMBER(graph) == 0) {
    return;
  }

  TZ_ASSERT(Graph_Is_Weighted(graph), "The graph is not weighted.");

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DEGREE);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_ELIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_STATUS);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_DLIST);

  Graph_Workspace_Load(gw, graph);

  if (gw->degree == NULL) { /* store tree connection */
    gw->degree = iarray_malloc(gw->nvertex);
  }

  if (gw->vlist == NULL) { /* store tree identification */
    gw->vlist = iarray_malloc(gw->nvertex);
  }

  if (gw->elist == NULL) { /* store sorted edges */
    gw->elist = iarray_malloc(gw->nedge);
  }

  if (gw->dlist == NULL) {
    gw->dlist = darray_malloc(gw->nedge);
  }
    
  if (gw->status == NULL) { /* store added edges */
    gw->status = u8array_malloc(gw->nedge);
  }
  
  uint8_t *edge_in = gw->status;
  int *tree_id = gw->vlist;
  int *sorted_edge_idx = gw->elist;
  int *connection = gw->degree;
  double *weights = gw->dlist;
  
  int i;
  for (i = 0; i < graph->nedge; i++) {
    edge_in[i] = 0;
    sorted_edge_idx[i] = i;
  }
  for (i = 0; i < graph->nvertex; i++) {
    tree_id[i] = i;
    connection[i] = -1;
  }

  darraycpy(weights, graph->weights, 0, gw->nedge);
  darray_qsort(weights, sorted_edge_idx, graph->nedge);

  int v1, v2;
  int tmpid;
  int next;
  for (i = 0; i < graph->nedge; i++) {
    v1 = graph->edges[sorted_edge_idx[i]][0];
    v2 = graph->edges[sorted_edge_idx[i]][1];

    if (tree_id[v1] != tree_id[v2]) {
      tmpid = tree_id[v2]; /* save id of v2 */
      /* change ids of v2-tree to v1*/
      next = tree_id[v2];
      while (next >=0 ) {
	ASSERT(connection[next] != next, "self loop");
	tree_id[next] = tree_id[v1];
	next = connection[next];
      }				
      /******************************/

      /* connect v1-tree and v2-tree*/
      next = tree_id[v1];			
      while (connection[next] >= 0) {
	ASSERT(connection[next] != next, "self loop");
	next = connection[next];		
      }						
      connection[next] = tmpid;
      /******************************/

      edge_in[sorted_edge_idx[i]] = 1;
    }
  }

  int j = 0;
  for (i = 0; i < graph->nedge; i++) {
    if (edge_in[i] == 1) {
      graph->edges[j][0] = graph->edges[i][0];
      graph->edges[j][1] = graph->edges[i][1];
      graph->weights[j] = graph->weights[i];
      j++;
    }
  }
  
  graph->nedge = j;
}


Graph* Graph_Connected_Subgraph(Graph *graph, Graph_Workspace *gw, int seed)
{
  if ((seed < 0) || (seed > graph->nvertex)) {
    return NULL;
  }

  if (graph->nedge == 0) {
    return NULL;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_STATUS);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_ELIST);

  Graph_Workspace_Load(gw, graph);

  int **neighbor_list = Graph_Neighbor_List(graph, gw);
  


  if (gw->vlist == NULL) {
    gw->vlist = (int *) Guarded_Malloc(sizeof(int) * gw->nvertex,
				       "Graph_Connected_Subgraph");
  }

  if (gw->elist == NULL) {
    gw->elist = (int *) Guarded_Malloc(sizeof(int) * (gw->nedge + 1),
				       "Graph_Connected_Subgraph");
  }
  
  if (gw->status == NULL) {
    gw->status = (uint8_t *) Guarded_Malloc(sizeof(uint8_t) * gw->nvertex,
					    "Graph_Connected_Subgraph");
  }

  int i;

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_VLIST) == FALSE) {
    for (i = 0; i < gw->nvertex; i++) {
      gw->vlist[i] = -1;
    }
  }

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_STATUS) == FALSE) {
    for (i = 0; i < gw->nvertex; i++) {
      gw->status[i] = 0;
    }
  }

  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_ELIST) == FALSE) {
    gw->elist[gw->nedge] = 1;
    for (i = 0; i < gw->nedge; i++) {
      gw->elist[i] = 0;
    }
  }

  Graph *subgraph = New_Graph();
  subgraph->directed = graph->directed;
  subgraph->type = graph->type;
  subgraph->nvertex = graph->nvertex;
  subgraph->nedge = 0;

  int checking_vertex = seed;
  int tail = seed;
  gw->status[checking_vertex] = 1;

  while (checking_vertex >= 0) {
    int j;
    for (j = 1; j <= NUMBER_OF_NEIGHBORS(checking_vertex, neighbor_list); j++) {
      int neighbor = neighbor_list[checking_vertex][j];
      if (gw->elist[gw->idx[checking_vertex][j]] == 0) {
	gw->elist[gw->idx[checking_vertex][j]] = gw->elist[gw->nedge]; /* add the edge */
	subgraph->nedge++;
      }

      if (gw->status[neighbor] == 0) {
	gw->vlist[tail] = neighbor; /* enqueue the neighbor */
	tail = neighbor;
	gw->status[neighbor] = 1;
      }
    }

    checking_vertex = gw->vlist[checking_vertex]; /* next */
  }

  BOOL weighted = (graph->weights != NULL);

  Construct_Graph(subgraph, graph->nvertex, subgraph->nedge, weighted);
  subgraph->nedge = 0;
  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == gw->elist[gw->nedge]) {
      subgraph->edges[subgraph->nedge][0] = graph->edges[i][0];
      subgraph->edges[subgraph->nedge][1] = graph->edges[i][1];
      if (weighted == TRUE) {
	subgraph->weights[subgraph->nedge] = graph->weights[i];
      }
       subgraph->nedge++;
    }
  }

  gw->elist[gw->nedge]++;

  return subgraph;
}

Graph* Graph_Main_Subgraph(Graph *graph, Graph_Workspace *gw)
{
  if (graph == NULL) {
    return NULL;
  }

  if (graph->nedge == 0) {
    return NULL;
  }

  Graph *subgraph = NULL;  
  Graph *main_subgraph = 
    Graph_Connected_Subgraph(graph, gw, graph->edges[0][0]);
  
  if (main_subgraph != NULL) {
    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);
  
    int i;
    for (i = 0; i < graph->nedge; i++) {
      if (gw->elist[i] == 0) {
	subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
	if (subgraph->nedge > main_subgraph->nedge) {
	  Kill_Graph(main_subgraph);
	  main_subgraph = subgraph;
	} else {
	  Kill_Graph(subgraph);
	}
      }
    }

    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);
  }

#ifdef _DEBUG_2
  printf("---------------->main graph:\n");
  Print_Graph(main_subgraph);
#endif

  return main_subgraph;
}

Arrayqueue Graph_Traverse_B(Graph *graph, int root, Graph_Workspace *gw)
{
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Load(gw, graph);  
  Graph_Workspace_Alloc(gw, GRAPH_WORKSPACE_VLIST);

  Arrayqueue aq;
  Arrayqueue_Attach(&aq, gw->vlist, gw->nvertex);
  Initialize_Arrayqueue(&aq);
  
  int **neighbor_list = Graph_Neighbor_List(graph, gw);
  if (NUMBER_OF_NEIGHBORS(neighbor_list, root) > 0) {
    /* alloc <mask> */
    uint8_t *mask = u8array_malloc(gw->nvertex);
    int i;
    for (i = 0; i < gw->nvertex; i++) {
      mask[i] = 0;
    }

    Arrayqueue_Add_Last(&aq, root);
    mask[root] = 1;
    int next = root;
    
    do {
      for (i = 1; i <= NUMBER_OF_NEIGHBORS(next, neighbor_list); i++) {
	int node = NEIGHBOR_OF(next, i, neighbor_list);
	if (mask[node] == 0) {
	  Arrayqueue_Add_Last(&aq, node);
	  mask[node] = 1;
	}
      }
      next = aq.array[next];
    } while (next >= 0);

    /* free <mask> */
    free(mask);
  }
  
  return aq;
}

Arrayqueue Graph_Traverse_Lbfs(Graph *graph, Graph_Workspace *gw)
{
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Load(gw, graph);  
  Graph_Workspace_Alloc(gw, GRAPH_WORKSPACE_VLIST);

  Arrayqueue aq;
  Arrayqueue_Attach(&aq, gw->vlist, gw->nvertex);
  Initialize_Arrayqueue(&aq);
  
  Graph_Update_Edge_Table(graph, gw);

  /* Initialize the set sequence S */
  Unipointer_List *sigma = Unipointer_List_New();
  Int_Arraylist *start_set = Make_Int_Arraylist(graph->nvertex, 0);
  int i;
  for (i = 0; i < graph->nvertex; ++i) {
    start_set->array[i] = i;
  }
  sigma->data = (unipointer_t) start_set;

  int count = 0;

  /* While the sequence Sigma is not empty */
  while (sigma != NULL) {
#ifdef _DEBUG_
    Unipointer_List *head = sigma;
    printf("Round:  \n");
    while (head != NULL) {
      Print_Int_Arraylist(head->data, "%d");
      head = head->next;
    }
#endif

    /* Take v from the first set in Sigma */
    Unipointer_List *set_node = sigma;
    Int_Arraylist *s = (Int_Arraylist*) sigma->data;

    int v = Int_Arraylist_Take_Last(s);
    Arrayqueue_Enqueue(&aq, v);
#ifdef _DEBUG_
    printf("%d / %d\n", ++count, graph->nvertex);
#endif
  

    Unipointer_List *prior_set = NULL;
    /* For the each set s of sigma */
    while (set_node != NULL) {
      Int_Arraylist *s = (Int_Arraylist *) set_node->data;

      /* For each node in the set*/
      int i;
      BOOL is_replaced = FALSE;
      int *iter = s->array;
      int length = s->length;
      int current_index = 0;
      for (i = 0; i < length; ++i) {
        /* If the node is a neighbor of s */
        if (Graph_Edge_Index_U(v, *iter, gw) >= 0) {
          /* if the s is not replaced */
          if (is_replaced == FALSE) {
            /* Create a set prior to s */
            Int_Arraylist *news = Make_Int_Arraylist(0, 1);
            Unipointer_List *newnode = Unipointer_List_New();
            newnode->data = news;
            if (prior_set != NULL) {
              prior_set->next = newnode;
              newnode->next = set_node;
            } else {
              newnode->next = set_node;
              sigma = newnode;
            }
            prior_set = newnode;

            /* set s to replaced */
            is_replaced = TRUE;
          }
          /* Move the node the set prior to s */
          int w = Int_Arraylist_Take(s, current_index);
          Int_Arraylist_Add(prior_set->data, w);
        } else {
          ++iter;
          ++current_index;
        }
      }

      Unipointer_List *next_set = set_node->next;
      if (s->length == 0) {
        if (prior_set != NULL) {
          prior_set->next = set_node->next;
        }
        if (set_node == sigma) {
          sigma = set_node->next;
        }

        Kill_Int_Arraylist(set_node->data);
        free(set_node);
        set_node = NULL;

        if (is_replaced == TRUE) {
          set_node = prior_set;
        }
      }
      prior_set = set_node;
      set_node = next_set;
    }
  }

  return aq;
}

void Graph_Traverse_Direct(Graph *graph, int root, Graph_Workspace *gw)
{
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_STATUS);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Load(gw, graph);  
  Graph_Workspace_Alloc(gw, GRAPH_WORKSPACE_VLIST);

  if (gw->status == NULL) { 
    gw->status = u8array_malloc(gw->nedge);
  }
  
  if (Graph_Workspace_Ready(gw, GRAPH_WORKSPACE_STATUS) == FALSE) {
    int i;
    for (i = 0; i < gw->nedge; i++) {
      gw->status[i] = 0;
    }
  }

  Arrayqueue aq;
  Arrayqueue_Attach(&aq, gw->vlist, gw->nvertex);
  Initialize_Arrayqueue(&aq);
  
  int **neighbor_list = Graph_Neighbor_List(graph, gw);
  if (NUMBER_OF_NEIGHBORS(neighbor_list, root) > 0) {
    /* alloc <mask> */
    uint8_t *mask = u8array_malloc(gw->nvertex);
    int i;
    for (i = 0; i < gw->nvertex; i++) {
      mask[i] = 0;
    }

    Arrayqueue_Add_Last(&aq, root);
    mask[root] = 1;
    int next = root;
    
    do {
      for (i = 1; i <= NUMBER_OF_NEIGHBORS(next, neighbor_list); i++) {
	int node = NEIGHBOR_OF(next, i, neighbor_list);
	if (mask[node] == 0) {
          /* next is the parent of node */
	  Arrayqueue_Add_Last(&aq, node);
	  mask[node] = 1;
          int edge_index = gw->idx[next][i];
          if (graph->edges[edge_index][0] != next) {
            TZ_ASSERT(graph->edges[edge_index][0] == node &&
                graph->edges[edge_index][1] == next, "Inconsistent edge");
            int tmp;
            SWAP2(graph->edges[edge_index][0], graph->edges[edge_index][1], 
                tmp);
            gw->status[edge_index] = 1;
          }
	}
      }
      next = aq.array[next];
    } while (next >= 0);

    /* free <mask> */
    free(mask);
  }
}

void Graph_To_Dot_File(Graph *graph, const char *file_path)
{
  FILE *fp = Guarded_Fopen((char *) file_path, "w", "Graph_To_Dot_File");

  if (Graph_Is_Directed(graph)) {
    fprintf(fp, "digraph G {");
  } else {
    fprintf(fp, "graph G {");
  }
  
  if (graph->nedge > 0) {
    int i;
    for (i = 0; i < graph->nedge; i++) {
      fprintf(fp, "%d", graph->edges[i][0]);
      if (graph->directed == TRUE) {
	fprintf(fp, " -> ");
      } else {
	fprintf(fp, " -- ");
      }
      fprintf(fp, "%d", graph->edges[i][1]);
      
      if (graph->weights != NULL) {
	fprintf(fp, " [len=%g, label=\"%g\"]", 
		graph->weights[i], graph->weights[i]);
      }
      
      fprintf(fp, ";\n");   
    }
  }

  fprintf(fp, "}");
  fclose(fp);
}

int* Graph_Toposort(Graph *graph, Graph_Workspace *gw)
{
  if (Graph_Is_Directed(graph) == FALSE) {
    return NULL;
  }

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
  Graph_Workspace_Load(gw, graph);

  if (gw->vlist == NULL) {
    gw->vlist = iarray_malloc(gw->nvertex);
  }

  int *in_degree = Graph_In_Degree(graph, gw);
  int **child_list = Graph_Child_List(graph, gw);
  /* alloc <queue> */
  Arrayqueue *queue = Make_Arrayqueue(gw->nvertex);
  
  int i;

  for (i = 0; i < gw->nvertex; i++) {
    if (in_degree[i] == 0) {
      Arrayqueue_Enqueue(queue, i);
    }
  }

  i = 0;
  while (Arrayqueue_Is_Empty(queue) == FALSE) {
    int cur_node = Arrayqueue_Dequeue(queue);
    gw->vlist[i++] = cur_node;
    int j;
    for (j = 1; j <= child_list[cur_node][0]; j++) {
      if (--in_degree[child_list[cur_node][j]] == 0) {
	Arrayqueue_Enqueue(queue, child_list[cur_node][j]);
      }
    }
  }

  /* free <queue> */
  Kill_Arrayqueue(queue);

  /* in_degree is destroyed */
  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_IN_DEGREE, FALSE);
 
  return gw->vlist;
}

void Graph_Clean_Root(Graph *graph, int root, Graph_Workspace *gw)
{
  int **neighbors = Graph_Neighbor_List(graph, gw);
  if (GRAPH_NUMBER_OF_NEIGHBORS(root, neighbors) <=1) {
    return;
  }

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_CONNECTION, TRUE);

  int *path = Graph_Shortest_Path(graph, root, gw);
  
  int i;
  for (i = 0; i < gw->nvertex; i++) {
    if (gw->dlist[i] == Infinity) {
      gw->dlist[i] = -1.0;
    }
  }

  size_t index;
  darray_max(gw->dlist, gw->nvertex, &index);
  
  while (index != root) {
    if (path[index] == root) {
      break;
    }
    index = path[index];
  }

  Graph_Update_Edge_Table(graph, gw);

  int *remove_list = 
    iarray_malloc(GRAPH_NUMBER_OF_NEIGHBORS(root, neighbors) - 1);
  int j = 0;

  for (i = 1; i <= GRAPH_NUMBER_OF_NEIGHBORS(root, neighbors); i++) {
    int node = GRAPH_NEIGHBOR_OF(root, i, neighbors);
    if (node != index) {
      int edge_index = Graph_Edge_Index(root, node, gw);
      if (edge_index < 0) {
	edge_index = Graph_Edge_Index(node, root, gw);
      }
      remove_list[j++] = edge_index;
    }
  }

  Graph_Remove_Edge_List(graph, remove_list, j);
  
  free(remove_list);

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_CONNECTION, FALSE);
}

void Graph_Move_Edge(Graph *graph, int dst, int src)
{
  TZ_ASSERT(dst <= graph->nedge, "Invalid index.");
  TZ_ASSERT(src <= graph->nedge, "Invalid index.");

  graph->edges[dst][0] = graph->edges[src][0];
  graph->edges[dst][1] = graph->edges[src][1];
  
  if (Graph_Is_Weighted(graph)) {
    graph->weights[dst] = graph->weights[src];
  }

  graph->edges[src][0] = -1;
  graph->edges[src][1] = -1;
}

void Graph_Mwt(Graph *graph, Graph_Workspace *gw)
{
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_CONNECTION);
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_ELIST);

  Graph_Workspace_Load(gw, graph);
  Graph_Workspace_Alloc(gw, GRAPH_WORKSPACE_ELIST);
  
  int **conn = Graph_Neighbor_List(graph, gw);
  Graph_Update_Edge_Table(graph, gw);

  int i;
  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    gw->elist[i] = 0;
  }

  size_t min_index;
  darray_min(graph->weights, GRAPH_EDGE_NUMBER(graph), &min_index);
  gw->elist[min_index] = 1;
  
  int current_vertex = GRAPH_EDGE_NODE(graph, min_index, 0);
  double min_weight = Infinity;
  int next_vertex = -1;
  do {
    double min_weight = Infinity;
    int min_edge_index = -1;
    int nnbr = GRAPH_NUMBER_OF_NEIGHBORS(current_vertex, conn);
    next_vertex = -1;
    if (nnbr > 1) {
      int k;
      for (k = 1; k <= nnbr; k++) {
	int nbr = GRAPH_NEIGHBOR_OF(current_vertex, k, conn);
	int edge_index = Graph_Edge_Index_U(current_vertex, nbr, gw);
	if (gw->elist[edge_index] == 0) {
	  if (graph->weights[edge_index] < min_weight) {
	    min_weight = graph->weights[edge_index];
	    next_vertex = nbr;
	    min_edge_index = edge_index;
	  }
	}
      }
    }
    if (min_edge_index >= 0) {
      gw->elist[min_edge_index] = 1;
    }
    current_vertex = next_vertex;
  } while (next_vertex >= 0);


  current_vertex = GRAPH_EDGE_NODE(graph, min_index, 1);
  do {
    min_weight = Infinity;
    int min_edge_index = -1;
    int nnbr = GRAPH_NUMBER_OF_NEIGHBORS(current_vertex, conn);
    next_vertex = -1;
    if (nnbr > 1) {
      int k;
      for (k = 1; k <= nnbr; k++) {
	int nbr = GRAPH_NEIGHBOR_OF(current_vertex, k, conn);
	int edge_index = Graph_Edge_Index_U(current_vertex, nbr, gw);
	if (gw->elist[edge_index] == 0) {
	  if (graph->weights[edge_index] < min_weight) {
	    min_weight = graph->weights[edge_index];
	    next_vertex = nbr;
	    min_edge_index = edge_index;
	  }
	}
      }
    }
    if (min_edge_index >= 0) {
      gw->elist[min_edge_index] = 1;
    }
    current_vertex = next_vertex;
  } while (next_vertex >= 0);

  int nedge = 0;
  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    if (gw->elist[i] == 0) {
      gw->elist[nedge++] = i;
    }
  } 

  if (nedge > 0) {
    Graph_Remove_Edge_List(graph, gw->elist, nedge);
  }
}

#define GHM_SUB_TO_IND(i, j) (nvertex * (i) + (j))
#define GHM_MATRIX_VALUE_FROM_IND(x, idx) ((x)[(idx)/nvertex][(idx)%nvertex])

int construct_edge_loop(BOOL **conn, int nvertex, int i, int j, int *loop)
{
  int loop_length = 1;
  loop[0] = GHM_SUB_TO_IND(i, j);
  int k;
  BOOL opening = TRUE;
  while (opening) {
    for (k = 0; k < nvertex; k++) {
      opening = FALSE;
      if (conn[j][k] == TRUE) {
#ifdef _DEBUG_
	if (j == 202 && k == 1110) {
	  printf("debug here\n");
	}
#endif
	opening = TRUE;
	loop[loop_length++] = GHM_SUB_TO_IND(j, k);
	if (k == i) {
	  opening = FALSE;
	} else {
	  j = k;
	}
	break;
      }
    }
  }

  return loop_length;
}

double loop_match_score(int *loop, int loop_length, int start, double **wm, int
    nvertex)
{
  double w = 0.0;
  int i;
  for (i = start; i < loop_length; i += 2) {
    if ((start != 0) || (i != loop_length - 1)) {
      w += GHM_MATRIX_VALUE_FROM_IND(wm, loop[i]);
    }
  }

  return w;
}

void correct_match(int **conn, int nvertex, int *loop, int loop_length, int start)
{
  int i;
  for (i = 0; i < start; i++) {
    GHM_MATRIX_VALUE_FROM_IND(conn, loop[i]) = FALSE;
  }
  for (i = start+1; i < loop_length; i += 2) {
    GHM_MATRIX_VALUE_FROM_IND(conn, loop[i]) = FALSE;
  }
  if (start == 0) {
    if (loop_length % 2 == 1) {
      GHM_MATRIX_VALUE_FROM_IND(conn, loop[loop_length - 1]) = FALSE;
    }
  }
    
}

#define LARGE_WEIGHT 10000.0

/* limited to 1000 nodes. Must be complete graph. */
BOOL** Graph_Hungarian_Match(Graph *graph, Graph_Workspace *gw)
{
  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_WEIGHT);

  Graph_Workspace_Load(gw, graph);
  
  double **wm = Graph_Weight_Matrix(graph, gw);
  int i, j;
  int nvertex = GRAPH_VERTEX_NUMBER(graph);
  /*
  double maxw = 0.0;
  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      if (maxw < wm[i][j]) {
	maxw = wm[i][j];
      }
    }
  }
  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      wm[i][j] /= maxw;
    }
  }
*/
  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_WEIGHT, FALSE);
  /*
  for (i = 0; i < nvertex; i++) {
    wm[i][i] = Infinity;
  }
  */
  for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
    wm[graph->edges[i][0]][graph->edges[i][0]] = LARGE_WEIGHT;
    wm[graph->edges[i][1]][graph->edges[i][1]] = LARGE_WEIGHT;
  }

  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      if (isinf(wm[i][j])) {
	wm[i][j] = LARGE_WEIGHT;
      }
    }
  }

  BOOL **conn;
  MALLOC_2D_ARRAY(conn, nvertex, nvertex, BOOL, i);
  Adjmat_Hungarian(wm, nvertex, nvertex, conn);

  wm = Graph_Weight_Matrix(graph, gw);
  for (i = 0; i < nvertex; i++) {
    wm[i][i] = Infinity;
    conn[i][i] = FALSE;
  }
#ifdef _DEBUG_2
  return conn;
#endif

  int *loop = iarray_malloc(nvertex);
  for (i = 0; i < nvertex; i++) {
    loop[i] = -1;
  }
  int loop_length = 0;
  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      if (conn[i][j] == TRUE) {
	if (conn[j][i] == TRUE) {
	  conn[j][i] = FALSE;
	} else {
	  loop_length = construct_edge_loop(conn, nvertex, i, j, loop);
	  if (loop_length > 1) {
	    double minw;
	    int best_start = 0;
	    double tmpw;
	    minw = loop_match_score(loop, loop_length, 0, wm, nvertex);
	    tmpw = loop_match_score(loop, loop_length, 1, wm, nvertex);
	    if (tmpw < minw) {
	      minw = tmpw;
	      best_start = 1;
	    }
	    if (loop_length % 2 == 1) {
	      tmpw = loop_match_score(loop, loop_length, 2, wm, nvertex);
	      if (tmpw < minw) {
		minw = tmpw;
		best_start = 2;
	      }
	    }
	    correct_match(conn, nvertex, loop, loop_length, best_start);
	  }
	}
      }
    }
  }

  free(loop);

  wm = Graph_Weight_Matrix(graph, gw);
  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      if (isinf(wm[i][j])) {
	conn[i][j] = FALSE;
      }
    }
  }

  return conn;
}

/* http://en.wikipedia.org/wiki/Force-based_algorithms_(graph_drawing) */
void Graph_Layout(Graph *graph, double *x, double *y, Graph_Workspace *gw)
{
  /* Initialize the velocities */
  double *vx = darray_calloc(graph->nvertex);
  double *vy = darray_calloc(graph->nvertex);

  const static int Max_Iter = 200;
  const static double Min_Energy = 0.01;

  double energy = Min_Energy + 1.0;
  int iter = 0;

  int **neighbor = Graph_Neighbor_List(graph, gw);

  /* While the iteration number hasn't bee reached and the energe is large */
  while ((iter < Max_Iter) && (energy > Min_Energy)) {
    /* Set the energy to 0 */
    energy = 0.0;

    /* For each vertex*/
    int i;
    for (i = 0; i < graph->nvertex; i++) {
      /* Calculate force */
      double fx = 0.0;
      double fy = 0.0;

      /* repulsion force */
      int j;
      for (j = 0; j < graph->nvertex; j++) {
        if (i != j) {
          double dx = x[i] - x[j];
          double dy = y[i] - y[j];
          double dsq = dx * dx + dy * dy;
          fx += 400.0 * dx / dsq;
          fy += 400.0 * dy / dsq;
        }
      }

#ifdef _DEBUG_2
      printf("repulsion force: %g %g\n", fx, fy);
#endif

      /* attraction force */
      for (j = 1; j <= neighbor[i][0]; j++) {
        double dx = x[neighbor[i][j]] - x[i];
        double dy = y[neighbor[i][j]] - y[i];
        fx += dx * 0.06;
        fy += dy * 0.06;
      }

#ifdef _DEBUG_2
      printf("net force: %g %g\n", fx, fy);
#endif

      /* update velocity */
      vx[i] = (vx[i] + fx) * 0.85;
      vy[i] = (vy[i] + fy) * 0.85;

#ifdef _DEBUG_2
      printf("velocity: %g %g\n", vx[i], vy[i]);
#endif

      energy += vx[i] * vx[i] + vy[i] * vy[i];
    }

#ifdef _DEBUG_2
    printf("%d: %g\n", iter, energy);
#endif

    /* Update position */
    darray_add(x, vx, graph->nvertex);
    darray_add(y, vy, graph->nvertex);

    iter++;
  }
}

BOOL Graph_Has_Hole(Graph *graph)
{
  BOOL hasHole = FALSE;

  Graph_Workspace *gw = New_Graph_Workspace();

  int *order = iarray_malloc(graph->nvertex);

  Arrayqueue aq = Graph_Traverse_Lbfs(graph, gw);
#ifdef _DEBUG_
  Print_Arrayqueue(&aq);
#endif
  Arrayqueue_To_Index(&aq, order);

  int **nbr = Graph_Neighbor_List(graph, gw); 

  Int_Arraylist *neighborOrder = Make_Int_Arraylist(0, 1);
  Int_Arraylist *neighborIndex = Make_Int_Arraylist(0, 1);

  int i;
  for (i = 0; i < graph->nvertex; ++i) {
    int neighborNumber= NUMBER_OF_NEIGHBORS(i, nbr);
    if (neighborNumber > 1) {
      int j;
      Int_Arraylist_Set_Length(neighborOrder, 0);
      Int_Arraylist_Set_Length(neighborIndex, neighborNumber);

      /* sort the neighbors */
      for (j = 1; j <= neighborNumber; ++j) {
        Int_Arraylist_Add(neighborOrder, order[nbr[i][j]]);
      }

      iarray_qsort(neighborOrder->array, neighborIndex->array, neighborNumber);

      for (j = neighborNumber - 1; j > 0; --j) {
        if (neighborOrder->array[j] < order[i]) {
          int nbr1 = nbr[i][neighborIndex->array[j] + 1];
          int nbr2 = nbr[i][neighborIndex->array[j - 1] + 1];
          if (Graph_Edge_Index_U(nbr1, nbr2, gw) < 0) {
            if (neighborNumber == 2) {
              hasHole = TRUE;
            } else {
              hasHole = TRUE;
              if (j - 2 >= 0) {
                int nbr3 = nbr[i][neighborIndex->array[j - 2] + 1];
                printf("nbr3: %d\n", nbr3);

                if (Graph_Edge_Index_U(nbr1, nbr3, gw) >= 0 && 
                    Graph_Edge_Index_U(nbr2, nbr3, gw) >= 0) {
                  hasHole = FALSE;
                }
              }
              if (hasHole == TRUE) {
                if (j + 1 < neighborNumber) {
                  int nbr3 = nbr[i][neighborIndex->array[j + 1] + 1];
                  printf("nbr3: %d\n", nbr3);
                  if (Graph_Edge_Index_U(nbr1, nbr3, gw) >= 0 && 
                      Graph_Edge_Index_U(nbr2, nbr3, gw) >= 0) {
                    hasHole = FALSE;
                  }
                }
              }
            }
          }
#ifdef _DEBUG_
          if (hasHole == TRUE) {
            printf("Hole at: %d - %d, %d\n", i, nbr1, nbr2);
            break;
          }
#endif
        }
      }
    }

    if (hasHole == TRUE) {
      break;
    }
  }

  Kill_Int_Arraylist(neighborOrder);
  Kill_Int_Arraylist(neighborIndex);

  Kill_Graph_Workspace(gw);

  return hasHole;
}

void Write_Graph(const char *file_path, const Graph *graph)
{
  if (graph != NULL) {
    FILE *fp = fopen(file_path, "wb");

    if (fp == NULL) {
      printf("Cannot open %s\n", file_path);
      return; 
    }

    BOOL has_weight = FALSE;
    if (graph->weights != NULL) {
      has_weight = TRUE;
    }
    fwrite(&has_weight, sizeof(BOOL), 1, fp);
    fwrite(&(graph->nvertex), sizeof(int), 1, fp);
    fwrite(&(graph->directed), sizeof(BOOL), 1, fp);
    fwrite(&(graph->nedge), sizeof(int), 1, fp);

    fwrite(graph->edges, sizeof(graph_edge_t), graph->nedge, fp);
    if (graph->weights != NULL) {
      fwrite(graph->weights, sizeof(double), graph->nedge, fp);
    }
    
    fclose(fp);
  }
}

Graph* Read_Graph(const char *file_path)
{
  FILE *fp = fopen(file_path, "rb");

  if (fp == NULL) {
    printf("Cannot open %s\n", file_path);
    return NULL; 
  }
  
  int nvertex;
  int nedge;
  BOOL is_directed;
  BOOL is_weighted;
  fread(&is_weighted, sizeof(BOOL), 1, fp);
  fread(&nvertex, sizeof(int), 1, fp);
  fread(&is_directed, sizeof(BOOL), 1, fp);
  fread(&nedge, sizeof(int), 1, fp);

  Graph *graph = Make_Graph(nvertex, nedge, is_weighted);

  graph->directed = is_directed;
  graph->nedge = nedge;
  fread(graph->edges, sizeof(graph_edge_t), nedge, fp);
  if (is_weighted == TRUE){
    fread(graph->weights, sizeof(double), nedge, fp);
  }

  fclose(fp);

  return graph;
}
