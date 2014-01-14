/**@file tz_graph_utils.h
 * @brief utilities for graph
 * @author Ting Zhao
 * @date 21-May-2008
 */

#ifndef _TZ_GRAPH_UTILS_H_
#define _TZ_GRAPH_UTILS_H_ 

#include "tz_cdefs.h"
#include "tz_graph_defs.h"

__BEGIN_DECLS

void Graph_Wmat_Neighbor_List(double **weight, int **neighbors, int nvertex);
void Graph_Edge_Neighbor_List(int nvertex, Graph_Edge_t *edges, int nedge,
			      int **neighbors, int **edge_idx);

void Graph_Edge_Child_List(int nvertex, Graph_Edge_t *edges, int nedge,
			   int **child_list, int **edge_idx);

void Graph_Shortest_Path_From_Adjmat(double **weight, int nvertex,
				     int **neighbors,
				     int start, double *dist, int *path,
				     int8_t *checked);

void Graph_Print_Path(int *path, int nvertex, int end);

int Graph_Mst_From_Adjmat(double **weight, Graph_Edge_t *min_span_tree, int n,
			  int *p_num_of_edges, double *p_sum_of_edges);

BOOL Is_Adjmat_Connected(int **conn, int n);

int* Adjmat_Isolate(int **conn, int n, int *nnbr);

/* assign n jobs to m workers */
double Adjmat_Hungarian(double **weight, int m, int n, BOOL **result);

__END_DECLS

#endif
