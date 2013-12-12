/**@file tz_graph_defs.h
 * @brief definitions for graph
 * @author Ting Zhao
 * @date 21-May-2008
 */

#ifndef _TZ_GRAPH_DEFS_H_
#define _TZ_GRAPH_DEFS_H_

#include "tz_cdefs.h"

__BEGIN_DECLS
#include <utilities.h>
#include <hash.h>
__END_DECLS

#include "tz_stdint.h"
#include "tz_int_arraylist.h"
#include "tz_bitmask.h"

typedef enum {
  GENERAL_GRAPH = 0, TREE_GRAPH, COMPLETE_GRAPH
} Graph_Structure_e;

typedef int graph_edge_t[2];

#define Graph_Edge_t graph_edge_t /* for obsolete name convention */
const static int Max_Edge_Capacity = 1073741824; /* 1G */

/**@struct _Graph tz_graph_defs.h
 * 
 * Graph data structure
 */
typedef struct _Graph {
  BOOL directed;          /**< TRUE for directed graph, FALSE for undirected*/
  Graph_Structure_e type; /**< Type of the graph */
  int nvertex;            /**< Number of vertices */
  int nedge;              /**< Number of edges */
  int edge_capacity;      /**< How many edges can it hold */
  graph_edge_t *edges;    /**< Edge array */
  double *weights;        /**< Edge weights (NULL for unweighted graph) */
} Graph;

#define GRAPH_VERTEX_NUMBER(graph) (graph->nvertex)

#define GRAPH_EDGE(graph, i) (graph->edges[i])
#define GRAPH_EDGE_NODE(graph, i, v) (GRAPH_EDGE(graph, i)[v])
#define GRAPH_EDGE_NUMBER(graph) ((graph)->nedge)
#define GRAPH_EDGE_WEIGHT(graph, i) ((graph)->weights[i])

#define GRAPH_NUMBER_OF_NEIGHBORS(v, neighbors) ((neighbors[v] == NULL) ? 0 : neighbors[v][0])
#define GRAPH_NUMBER_OF_NEIGHBORS_REF(v, neighbors) neighbors[v][0]
#define GRAPH_SET_NUMBER_OF_NEIGHBORS(v, neighbors, n) if (neighbors[v] != NULL) { GRAPH_NUMBER_OF_NEIGHBORS_REF(v, neighbors) = n; }
#define GRAPH_INC_NUMBER_OF_NEIGHBORS(v, neighbors) if (neighbors[v] != NULL) { GRAPH_NUMBER_OF_NEIGHBORS_REF(v, neighbors)++; }
#define GRAPH_NEIGHBOR_OF(v, n, neighbors) neighbors[v][n]

/*
 * The data structure Graph_Workspace is designed to accommodate intermediate
 * results or provide allocated memory space for graph rountines. It has four
 * categories of fields:
 *   Status mask: Each bit of the mask indicates the status of the corresponding
 *                field;
 *   Attribute: Attributes of the graph to work on;
 *   Dynamic space: any pointer type field is a dynamic space. Its creation is
 *                  affected by the status masks. ON (1) in the mask 
 *                  <allocated> means the space has been allocated and ON in 
 *                  the mask <ready> means meaningful values can be retrieved
 *                  from the space. This saves time by reducing replicated 
 *                  operations;
 *                  Note: for those 2D pointers, there is a corresponding
 *                        field for recording their sizes (how many pointers
 *                        to pointer are there).
 *   Static space: It is used to store intermediate result temporarily. There is
 *                 no need to allocate them explicitly. It is not associated 
 *                 with any status mask.
 */

/* the filed values must be continuous */
typedef enum _Graph_Workspace_Field_e {
  GRAPH_WORKSPACE_FIELD_BEGIN = 0,
  GRAPH_WORKSPACE_CONNECTION = 1,
  GRAPH_WORKSPACE_PARENT,
  GRAPH_WORKSPACE_CHILD,
  GRAPH_WORKSPACE_WEIGHT,
  GRAPH_WORKSPACE_MINDIST,
  GRAPH_WORKSPACE_IDX,
  GRAPH_WORKSPACE_DEGREE,
  GRAPH_WORKSPACE_IN_DEGREE,
  GRAPH_WORKSPACE_OUT_DEGREE,
  GRAPH_WORKSPACE_VLIST,
  GRAPH_WORKSPACE_ELIST,
  GRAPH_WORKSPACE_DLIST,
  GRAPH_WORKSPACE_EDGE_TABLE,
  GRAPH_WORKSPACE_EDGE_MAP,
  GRAPH_WORKSPACE_STATUS,
  GRAPH_WORKSPACE_FIELD_END
} Graph_Workspace_Field_e;

typedef struct _Graph_Workspace {
  Bitmask_t allocated; /* Allocation mask (One bit for one field) */
  Bitmask_t ready;     /* Readiness mask  (One bit for one field) */
  int nvertex;         /* Number of vertices */
  int nedge;           /* Number of edges */
  int **connection;
  int connection_psize;
  int **parent;
  int parent_psize;
  int **child;
  int child_psize;
  double **weight;
  int weight_psize;
  double **mindist;
  int mindist_psize;
  int **idx;
  int idx_psize;
  int *degree;     
  int *in_degree;  
  int *out_degree; 
  int *vlist;      
  int *elist;      
  double *dlist;
  Hash_Table *edge_table;
  Int_Arraylist *edge_map;  
  uint8_t *status;
  char edge_entry[50];
} Graph_Workspace;

typedef struct _Graph_Path {
  int length;
  int *path;  
} Graph_Path;

#endif 
