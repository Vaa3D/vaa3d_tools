/**@file tz_graph.h
 * @brief graph theory
 * @author Ting Zhao
 * @date 21-May-2008
 */

#ifndef _TZ_GRAPH_H_
#define _TZ_GRAPH_H_

#include "tz_cdefs.h"
#include "tz_graph_defs.h"
#include "tz_arrayqueue.h"

__BEGIN_DECLS

/* Note: 
 * For those routines taking Graph_Workspace as an argument, there is no
 * necessary to initialize the workspace. You can simply use 
 * New_Graph_Workspace() to create one and pass it. When the workspace is no
 * longer in use, use Kill_Graph_Workspace() to remove it to avoid memory 
 * leaks.
 */

/**@addtogroup graph_ Graph utilities (tz_graph.h)
 * @{
 */

/**@addtogroup graph_man_ General Graph object management
 * @{
 * Constructors and destructors.
 */
Graph* New_Graph();
void Delete_Graph(Graph *graph);

void Construct_Graph(Graph *graph, int nvertex, int edge_capacity, 
		     BOOL weighted);
void Clean_Graph(Graph *graph);

Graph* Make_Graph(int nvertex, int edge_capacity, BOOL weighted);
void Kill_Graph(Graph *graph);

Graph* Copy_Graph(const Graph *graph);

/**@}*/

/**@brief print a graph
 *
 * Print_Graph() prints <graph> to the standard output.
 */
void Print_Graph(const Graph *graph);

/**@brief print a graph briefly
 *
 * Print_Graph() prints the brief information of <graph> to the standard output.
 */
void Print_Graph_Info(const Graph *graph);

/**@brief test if a graph is directed
 *
 * Graph_Is_Directed() returns TRUE if <graph> is directed; otherwise, it returns 
 * FALSE.
 */
BOOL Graph_Is_Directed(const Graph *graph);

/**@brief Set a graph to be directed or undirected.
 *
 * The routine will perform redundant edge removal when a directed graph is set
 * to undirected graph.
 */
void Graph_Set_Directed(Graph *graph, BOOL directed);

/**@brief Test if a graph is weighted.
 * 
 * Graph_Is_Weighted() return TRUE if <graph> is weighted; otherwise it returns
 * FALSE.
 */
BOOL Graph_Is_Weighted(const Graph *graph);

/**@brief Set a graph to weighted or unweighted.
 *
 * Graph_Set_Weighted() sets <graph> to weighted (<weighted> is TRUE) or
 * unweighted (<weighted> is FALSE).
 */
void Graph_Set_Weighted(Graph *graph, BOOL weighted);

/**@brief add an edge
 *
 * Graph_Add_Edge() adds an edge between <v1> and <v2>. If <graph> is a weighted
 * graph, the weight of the edge edge is set to 0. Note that it does not check
 * if the edge is duplcated.
 */
void Graph_Add_Edge(Graph *graph, int v1, int v2);

/**@brief add a weighted edge
 *
 * Graph_Add_Weighted_Edge() adds an edge with weight <weight> bewteen <v1>
 * and <v2>. When <graph> is unweighted, <weight> is ignored.
 */
void Graph_Add_Weighted_Edge(Graph *graph, int v1, int v2, double weight);

/**@brief remove an edge
 *
 * Graph_Remove_Edge() removes the edge with the index <edge_index> from 
 * <graph>. Nothing will be done if <edge_index> is not a valid index.
 */
void Graph_Remove_Edge(Graph *graph, int edge_index);

/**@brief Remove a set of edges.
 *
 * Graph_Remove_Edge_List() removes a set of edges from <graph>. <edge_index>
 * is an array to specify the edges to remove and <nedge> is the number of
 * edges to remove. All invalid indices will be ignored.
 */
int Graph_Remove_Edge_List(Graph *graph, const int *edge_index, int nedge);

/**@brief Remove all edges associated with a vertex.
 *
 * Graph_Remove_Vertex() removes all edges associated with <vertex> in
 * <graph>. It returns the number of edges that are removed.
 */
int Graph_Remove_Vertex(Graph *graph, int vertex);

/**@brief Move an edge.
 * 
 * Graph_Move_Edge() moves an edge with index <src> to the index <dst> in 
 * <graph>. The edge <src> becomes invalid after the move.
 */
void Graph_Move_Edge(Graph *graph, int dst, int src);

/*
 * Graph_Update_Edge_Table() updates the edge table so that Graph_Edge_Index()
 * is able to obtain the index of an edge in <graph>. <gw> can be used 
 * repeatedly.
 */
void Graph_Update_Edge_Table(const Graph *graph, Graph_Workspace *gw);

/**@brief Expand an edge table.
 *
 * Graph_Expand_Edge_Table() expands the edges table in <gw> by adding the 
 * edge of (<v1>, <v2>) with the edge index <edge_idx>.
 */
void Graph_Expand_Edge_Table(int v1, int v2, int edge_idx, 
    Graph_Workspace *gw);

/**@brief The index of an edge.
 *
 * Graph_Edge_Index() returns the index of the edge (<v1>, <v2>). It returns -1
 * if the edge does not exist or has not been indexed.
 */
int Graph_Edge_Index(int v1, int v2, Graph_Workspace *gw);

/**@brief The index of an edge.
 *
 * Graph_Edge_Index() returns the index of the edge (<v1>, <v2>) or (<v2>,
 * <v2>). It returns -1 if the edge does not exist or has not been indexed.
 */
int Graph_Edge_Index_U(int v1, int v2, Graph_Workspace *gw);

/**@brief Insert a node.
 *
 * Graph_Insert_Node() interts node <v> between <v1> and <v2> and forms the
 * structure <v1>-<v>-<v2>.
 */
void Graph_Insert_Node(Graph *graph, int v1, int v2, int v, 
		       Graph_Workspace *gw);

/*
 * Graph_Weight_Matrix() returns the weight matrix (weight set to 1 for 
 * unweighted graph) of <graph>. Its a 2D array of the weights and the element 
 * at (i, j) is the weight between i and j. The returned pointer is assoicated 
 * with <gw>.
 */
double** Graph_Weight_Matrix(const Graph *graph, Graph_Workspace *gw);

/**@brief Degrees of a graph.
 *
 * Graph_Degree() calculates the degree of each vertex of <graph> and stores it
 * in <gw->degree>. It also returns an integer array and the ith element in the
 * array is the degree of the vertex i. The array is associated with <gw>, so
 * the caller does not need to free it separately.
 */
int* Graph_Degree(const Graph *graph, Graph_Workspace *gw);

/**@brief In degrees of a graph.
 */
int* Graph_In_Degree(const Graph *graph, Graph_Workspace *gw);

/**@brief Out degrees of a graph.
 */
int* Graph_Out_Degree(const Graph *graph, Graph_Workspace *gw);

/*
 * Graph_Adjmat() returns the adjacency matrix of <graph>. The returned pointer
 * is owned by <gw>. <diag> specfies the value on the diagonal and should be
 * 0 or 1.
 *
 * Note: Graph_Adjmat() and Graph_Neighbor_List() are using the same reference
 *       in <gw>, so calculating either of them will destroy the other.
 */
int** Graph_Adjmat(const Graph *graph, int diag, Graph_Workspace *gw);

/**@brief Neighbor list of graph.
 *
 * Grap_Neighbor_List() builds the neighbor list for each vertex in graph and
 * stores the result in <gw>. It returns also a 2D array, of which the ith 
 * element points to a 1D array containing the neighbors of the vertex i. The
 * first element of a 1D array is the number of neighbors of the corresponding
 * vertex and it is flollowed by the vertex indices if the neighbors. <gw->idx>
 * contains the edge indices corresponding to the neighbor list, i.e.
 * <gw->idx[i][j]> is the index of the edge between i and its jth neighbor.
 *
 * The returned pointer is associated with <gw>.
 *
 * Note: Graph_Adjmat() and Graph_Neighbor_List() are using the same reference
 *       in <gw>, so calculating either of them will destroy the other.
 */
int** Graph_Neighbor_List(const Graph *graph, Graph_Workspace *gw);

/**@brief Child list of a graph.
 */
int** Graph_Child_List(const Graph *graph, Graph_Workspace *gw);

/**@brief Build shortest path on a graph.
 * 
 * Graph_Shortest_Path() finds the shortest path from any vertex in <graph>
 * to the vertex <start> and stores the results in <gw>. It returns an array
 * with linked indices, i.e. there is always a connection between array[i] and
 * i if i is a valid index (>=0). <gw->dlist> stores the weights of the
 * shortest paths. <gw->dlist[i]> is how far away from start to i.
 *
 * The returned pointer is associated with <gw>.
 */
int* Graph_Shortest_Path(const Graph *graph, int start, Graph_Workspace *gw);

/**@brief Find the shortest path betwee two nodes.
 * 
 * Graph_Shortest_Path_E() finds the shortest path from <start> to <end>. All
 * other parameters and return values are the same as those in
 * Graph_Shortest_Path().
 */
int* Graph_Shortest_Path_E(const Graph *graph, int start, int end,
			   Graph_Workspace *gw);

/**@brief Find the max min path between two nodes.
 *
 * Graph_Shortest_Path_Maxmin() finds the minmax path between <start> and 
 * <end>. Here the minmax path is defined as the path along which the minimal
 * value is greater than minimal values along other possible paths.
 */
int* Graph_Shortest_Path_Maxmin(const Graph *graph, int start, int end,
    double *value, Graph_Workspace *gw);

/**@brief Cut leaves of a graph.
 *
 * Graph_Cut_Leaves() cuts leaves off in <graph>.
 */
void Graph_Cut_Leaves(Graph *graph, Graph_Workspace *gw);


/**@brief Prune a graph
 *
 * Graph_Prune() cuts leaves of a graph <n> times.
 */
void Graph_Prune(Graph *graph, int n, Graph_Workspace *gw);

/* 
 * Graph_To_Mst() turns a graph to a minimal spanning tree. <graph> must be
 * a connected-undirected-weighted graph. 
 */
void Graph_To_Mst(Graph *graph, Graph_Workspace *gw);

/**@brief Minimal spanning tree.
 *
 * Graph_To_Mst2() turns a graph into a minimal spanning tree. It is more
 * effiecient than Graph_To_Mst() in terms of memory space and speed. So it
 * is preferred for building an MST. 
 */
void Graph_To_Mst2(Graph *graph, Graph_Workspace *gw);

/**@brief Normalize the edges.
 *
 * Graph_Normalize_Edge() makes all edges in an undirected <graph> start from 
 * smaller vertex index. Nothing will be done if <graph> is directed.
 */
void Graph_Normalize_Edge(Graph *graph);

/**@brief Remove duplicated edges.
 *
 * Graph_Remove_Duplicated_Edge() removes all duplicated edges in <graph>.
 * It returns the number of removed edges.
 */
int Graph_Remove_Duplicated_Edge(Graph *graph);

/**@brief Count edges.
 *
 * Graph_Edge_Count() return the number of edges that exist in both <graph> and
 * the edge list <edges>.
 */
int Graph_Edge_Count(const Graph *graph, graph_edge_t* const edges, int n,
		     Graph_Workspace *gw);

/*
 * Graph_Connected_Subgraph() returns a sub-graph of <graph>. The sub-graph
 * is the maximum connected subset containing the vertex <seed>.
 */
Graph* Graph_Connected_Subgraph(Graph *graph, Graph_Workspace *gw, int seed);

/*
 * Graph_Main_Subgraph() returns the largest connected subgraph of \a graph.
 */
Graph* Graph_Main_Subgraph(Graph *graph, Graph_Workspace *gw);

/* 
 * Breadth-first traversal.
 */
Arrayqueue Graph_Traverse_B(Graph *graph, int root, Graph_Workspace *gw);

/**@brief lexicographic breadth-first search
 *
 */
Arrayqueue Graph_Traverse_Lbfs(Graph *graph, Graph_Workspace *gw);

BOOL Graph_Has_Hole(Graph *graph);

/**@brief Turn a graph to a directed graph by bread-first traversal
 */
void Graph_Traverse_Direct(Graph *graph, int root, Graph_Workspace *gw);

void Graph_To_Dot_File(Graph *graph, const char *file_path);

void Graph_Mwt(Graph *graph, Graph_Workspace *gw);

/*
 * Topological sort.
 */
int* Graph_Toposort(Graph *graph, Graph_Workspace *gw);

void Graph_Clean_Root(Graph *graph, int root, Graph_Workspace *gw);

BOOL** Graph_Hungarian_Match(Graph *graph, Graph_Workspace *gw);

/**@brief Layout a graph in a 2d plane
 *
 * Graph_Layout() computes positions of the nodes in <graph> and stores the
 * coordinates in <x> and <y>, which should be initialized before passed to the
 * function. The initial positions should not have overlap.
 */
void Graph_Layout(Graph *graph, double *x, double *y, Graph_Workspace *gw);

void Write_Graph(const char *file_path, const Graph *graph);
Graph* Read_Graph(const char *file_path);

/*************** Workspace rountines *****************/
Graph_Workspace* New_Graph_Workspace();
void Free_Graph_Workspace(Graph_Workspace *gw);

void Construct_Graph_Workspace(Graph_Workspace *gw, int nvertex, int nedge);
void Clean_Graph_Workspace(Graph_Workspace *gw);

Graph_Workspace* Make_Graph_Workspace(int nvertex, int nedge);
void Kill_Graph_Workspace(Graph_Workspace *);

/*
 * Graph_Workspace_Load() loads attributes of <graph> into <gw> so that they
 * are associated. Graph_Workspace_Reload() loads the attributes too, but it
 * will clean up all previously assigned spaces in <gw>.
 *
 */
void Graph_Workspace_Load(Graph_Workspace *gw, const Graph *graph);
void Graph_Workspace_Reload(Graph_Workspace *gw, const Graph *graph);

/**@brief workspace preparation
 *
 * Graph_Workspace_Prepare() cleans up a field for next step if necessary.
 * See tz_graph_defs.h for field ID definitions. This function should not be 
 * called after any attribute in <gw> is changed.
 */
void Graph_Workspace_Prepare(Graph_Workspace *gw, int field_id);

/* 
 * Alloc space for the field \a field_id for \a gw if it is NULL. The function 
 * does nothing if the field is not NULL. 
 */
void Graph_Workspace_Alloc(Graph_Workspace *gw, int field_id);

/*
 * The following 4 routines are designed for reuse of previous settings. Be 
 * cautious while using them, because the mis-usage of them may generate bugs 
 * hard to track.
 */
BOOL Graph_Workspace_Ready(const Graph_Workspace *gw, int field_id);
void Graph_Workspace_Set_Readiness(Graph_Workspace *gw, int field_id, 
				   BOOL status);

BOOL Graph_Workspace_Allocated(const Graph_Workspace *gw, int field_id);
void Graph_Workspace_Set_Allocation(Graph_Workspace *gw, int field_id, 
				    BOOL status);

/**@}*/

__END_DECLS

#endif
