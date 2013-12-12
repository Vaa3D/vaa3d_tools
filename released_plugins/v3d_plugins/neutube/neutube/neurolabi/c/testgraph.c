/* testgraph.h
 *
 * 21-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_graph.h"
#include "tz_arrayqueue.h"
#include "tz_graph_utils.h"
#include "tz_unipointer_linked_list.h"
#include "tz_stack_graph.h"
#include "tz_stack_utils.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_lib.h"
#include "tz_utilities.h"
#include "tz_stack_attribute.h"
#include "tz_stack_draw.h"
#include "tz_random.h"
#include "tz_math.h"
#include "tz_testdata.h"

int main(int argc, char *argv[])
{

  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    /* Make an unweighted graph with 10 nodes with edge capacity 20 */
    /* The vertices are represented by contiguous intergers starting from 0 */
    Graph *graph = Make_Graph(10, 20, FALSE);
    
    /* Check if the graph is unweighted */
    if (Graph_Is_Weighted(graph) == TRUE) {
      PRINT_EXCEPTION("Bug?", "The graph is weighted.");
      return 1;
    }

    /* Check the number of vertices */
    if (GRAPH_VERTEX_NUMBER(graph) != 10) {
      PRINT_EXCEPTION("Bug?", "The vertex number is not 10.");
      return 1;
    }

    /* The graph is undirected by default */
    if (Graph_Is_Directed(graph) == TRUE) {
      PRINT_EXCEPTION("Bug?", "The graph is not undirected.");
      return 1;
    }

    /* Right after creation, there is no edge in the graph */
    if (GRAPH_EDGE_NUMBER(graph) != 0) {
      PRINT_EXCEPTION("Bug?", "The edge number is not 0.");
      return 1;
    }

    /* We can add edges */
    Graph_Add_Edge(graph, 0, 2);
    Graph_Add_Edge(graph, 1, 2);
    Graph_Add_Edge(graph, 3, 2);
    Graph_Add_Edge(graph, 3, 4);
    Graph_Add_Edge(graph, 4, 5);
    Graph_Add_Edge(graph, 2, 4);
    Graph_Add_Edge(graph, 0, 4);
    Graph_Add_Edge(graph, 0, 6);
    Graph_Add_Edge(graph, 1, 7);
    Graph_Add_Edge(graph, 0, 8);
    Graph_Add_Edge(graph, 4, 9);

    /* Now we should have 11 edges */
    if (GRAPH_EDGE_NUMBER(graph) != 11) {
      PRINT_EXCEPTION("Bug?", "The edge number is not 11.");
      return 1;
    }
    
    /* Write the graph to a DOT file, which can be visualized by Graphviz*/
    Graph_To_Dot_File(graph, "../data/test.dot");

    /* For more complicated operations or attributes, the graph workspace is 
     * required */
    Graph_Workspace *gw = New_Graph_Workspace();

    /* Calculate the degrees of a graph */
    int *degree = Graph_Degree(graph, gw);
    
    if ((degree[0] != 4) || (degree[1] != 2) || (degree[2] != 4) || 
	(degree[3] != 2) || (degree[4] != 5) || (degree[5] != 1) || 
	(degree[6] != 1) || (degree[7] != 1) || (degree[8] != 1) ||
	(degree[9] != 1)) {
      PRINT_EXCEPTION("Bug?", "Incorrect degree.");
      return 1;
    }
    
    /* Build adjacency matrix for the graph */
    int **adjmat2 = Graph_Adjmat(graph, 0, gw);

    int i, j;
    for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
      if ((adjmat2[GRAPH_EDGE_NODE(graph, i, 0)][GRAPH_EDGE_NODE(graph, i, 1)] 
	   != 1) || 
	  (adjmat2[GRAPH_EDGE_NODE(graph, i, 1)][GRAPH_EDGE_NODE(graph, i, 0)]
	   != 1)) {
	PRINT_EXCEPTION("Bug?", "Inconsistent adjacency matrix.");
	return 1;
      }
    }

    /* Copy adjacency matrix */
    int **adjmat;
    MALLOC_2D_ARRAY(adjmat, GRAPH_VERTEX_NUMBER(graph), 
		    GRAPH_VERTEX_NUMBER(graph), int, i);

    for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
      for (j = 0; j < GRAPH_VERTEX_NUMBER(graph); j++) {
	adjmat[i][j] = adjmat2[i][j];
      }
    }

    /* Build neighbor list */
    /* Note: Graph_Adjmat() and Graph_Neighbor_List() are using the same 
     * reference in the same workspace, so calculating either of them will 
     * destroy the other if they are using the same workspace. */

    int **nbrs = Graph_Neighbor_List(graph, gw);
    for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
      for (j = 1; j <= GRAPH_NUMBER_OF_NEIGHBORS(i, nbrs); j++) {
	if (adjmat[i][GRAPH_NEIGHBOR_OF(i, j, nbrs)] != 1) {
	  PRINT_EXCEPTION("Bug?", "Unexpected neighbors.");
	  return 1;
	}
      }
    }
    

    /* Build an edge table to get the edge index given two nodes */
    Graph_Update_Edge_Table(graph, gw);
    
    /* Get the edge_index of an edge connecting two given nodes. */
    /* It returns -1 if the edge does not exist */
    int edge_index = Graph_Edge_Index(2, 4, gw);
    
    /* We can access the nodes of an edge via edge_index easily*/
    if ((GRAPH_EDGE_NODE(graph, edge_index, 0) != 2) ||
	(GRAPH_EDGE_NODE(graph, edge_index, 1) != 4)) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }

    for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
      edge_index = Graph_Edge_Index(GRAPH_EDGE_NODE(graph, i, 0), 
				    GRAPH_EDGE_NODE(graph, i, 1), gw);
      if (edge_index != i) {
	PRINT_EXCEPTION("Bug?", "Inconsistent edge index.");
	return 1;
      }
    }

    /* Graph_Edge_Index_U() is another way to get the index. This routine 
     * ignores the order of the input vertices. */
    for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
      edge_index = Graph_Edge_Index_U(GRAPH_EDGE_NODE(graph, i, 0), 
				      GRAPH_EDGE_NODE(graph, i, 1), gw);
      if (edge_index != i) {
	PRINT_EXCEPTION("Bug?", "Inconsistent edge index.");
	return 1;
      }

      edge_index = Graph_Edge_Index_U(GRAPH_EDGE_NODE(graph, i, 1), 
				      GRAPH_EDGE_NODE(graph, i, 0), gw);
      if (edge_index != i) {
	PRINT_EXCEPTION("Bug?", "Inconsistent edge index.");
	return 1;
      }
    }

    /* check if all the edges are correct */
    for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
      for (j = 0; j < GRAPH_VERTEX_NUMBER(graph); j++) {
	edge_index = Graph_Edge_Index(i, j, gw);
	if (edge_index >= 0) {
	  if (!(((GRAPH_EDGE_NODE(graph, edge_index, 0) == i) &&
		 (GRAPH_EDGE_NODE(graph, edge_index, 1) == j)) || 
		((GRAPH_EDGE_NODE(graph, edge_index, 0) == j) &&
		 (GRAPH_EDGE_NODE(graph, edge_index, 1) == i)))) {
	    PRINT_EXCEPTION("Bug?", "The edge is not expected.");
	    return 1;
	  }
	} else {
	  if (Graph_Edge_Index(j, i, gw) < 0) {
	    if (adjmat[i][j] == 1) {
	      printf("%d, %d\n", i, j);
	      PRINT_EXCEPTION("Bug?", "Inconsistent adjacency matrix.");
	      return 1;
	    }
	  }
	}
      }
    }

    /* The edge table should be expanded or updated again to deal with a new 
       edge */
    Graph_Add_Edge(graph, 0, 5);

    /* The table does not contain the new edge yet */
    if (Graph_Edge_Index(0, 5, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }

    /* Expanding one is more efficient than updating all */
    Graph_Expand_Edge_Table(0, 5, GRAPH_EDGE_NUMBER(graph) - 1, gw);
    
    /* Now the edge is in the table */
    if (Graph_Edge_Index(0, 5, gw) != GRAPH_EDGE_NUMBER(graph) - 1) {
      PRINT_EXCEPTION("Bug?", "The edge index is wrong.");
      return 1;
    }

    int old_edge_number = GRAPH_EDGE_NUMBER(graph);

    /* We can remove an edge */
    Graph_Remove_Edge(graph, 3);

    if (GRAPH_EDGE_NUMBER(graph) != old_edge_number - 1) {
      PRINT_EXCEPTION("Bug?", "The edge number is wrong.");
      return 1;
    }

    /* We can remove multiple edges at the same time */
    /* Edge 15 does not exist, so it will be ignored. */
    int remove_edge_indices[] = {0, 3, 5, 15};

    old_edge_number = GRAPH_EDGE_NUMBER(graph);
    Graph_Remove_Edge_List(graph, remove_edge_indices, 4);

    if (GRAPH_EDGE_NUMBER(graph) != old_edge_number - 3) {
      PRINT_EXCEPTION("Bug?", "The edge number is wrong.");
      return 1;
    }

    /* Now the edges [0, 3, 4, 6] in the original graph should be gone */
    Graph_Update_Edge_Table(graph, gw);
    
    /* 0-2, 3-4, 4-5, 0-4 were removed */
    if (Graph_Edge_Index(0, 2, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }
    if (Graph_Edge_Index(3, 4, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }
    if (Graph_Edge_Index(4, 5, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }
    if (Graph_Edge_Index(0, 4, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "The edge is not expected.");
      return 1;
    }

    /* For undirected graph, edges i-j and j-i are same. */
    /* We can make sure i <= j to make the edge representation unique. */
    Graph_Normalize_Edge(graph);
    
    for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
      if (GRAPH_EDGE_NODE(graph, i, 0) > GRAPH_EDGE_NODE(graph, i, 1)) {
	PRINT_EXCEPTION("Bug?", "Unnormalized edge.");
	return 1;
      }
    }

    Graph *old_graph = Copy_Graph(graph);

    /* Graph_Add_Edge() does not check if the edge has existed. So an edge can
     * have duplicates. Graph_Remove_Duplicated_Edge() is a routine to solve
     * this problem. */
    Graph_Add_Edge(graph, 4, 9);
    Graph_Remove_Duplicated_Edge(graph);
    
    for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
      if ((GRAPH_EDGE_NODE(graph, i, 0) != GRAPH_EDGE_NODE(old_graph, i, 0)) ||
	   (GRAPH_EDGE_NODE(graph, i, 1) != GRAPH_EDGE_NODE(old_graph, i, 1))) {
	PRINT_EXCEPTION("Bug?", "Unmatched edge.");
	return 1;
      }
    }

    Kill_Graph(old_graph);
    old_graph = Copy_Graph(graph);

    /* We can insert a node between two nodes. */
    int v1 = GRAPH_EDGE_NODE(graph, 0, 0);
    int v2 = GRAPH_EDGE_NODE(graph, 0, 1);
    Graph_Insert_Node(graph, v1, v2, GRAPH_VERTEX_NUMBER(graph), gw);
    
    if (GRAPH_VERTEX_NUMBER(graph) != GRAPH_VERTEX_NUMBER(old_graph) + 1) {
      PRINT_EXCEPTION("Bug?", "Unexpected vertex number.");
      return 1;      
    }

    Graph_Update_Edge_Table(graph, gw);
    
    if (Graph_Edge_Index_U(v1, GRAPH_VERTEX_NUMBER(old_graph), gw) < 0) {
      PRINT_EXCEPTION("Bug?", "Edge missing.");
      return 1;
    }
    
    if (Graph_Edge_Index_U(v2, GRAPH_VERTEX_NUMBER(old_graph), gw) < 0) {
      PRINT_EXCEPTION("Bug?", "Edge missing.");
      return 1;
    }
    
    /* the old edge should be gone */
    if (Graph_Edge_Index_U(v1, v2, gw) >= 0) {
      PRINT_EXCEPTION("Bug?", "Unexpected edge.");
      return 1;
    }

    /* Extract subgraphs from a graph */
    Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 
					       GRAPH_EDGE_NODE(graph, 0, 0));

    if (GRAPH_EDGE_NUMBER(subgraph) != 6) {
      PRINT_EXCEPTION("Bug?", "Unexpected graph.");
      return 1;
    }

    Unipointer_List *graph_list = NULL;
    Unipointer_List_Add(&graph_list, subgraph);

    /* This is necessary to extract subgraph one by one */
    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);

    for (i = 0; i < graph->nedge; i++) {
      if (gw->elist[i] == 0) {
	subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
	if (GRAPH_EDGE_NUMBER(subgraph) != 3) {
	  PRINT_EXCEPTION("Bug?", "Unexpected graph.");
	  return 1;
	}
	Unipointer_List_Add(&graph_list, subgraph);
      }
    }

    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);

    /* Extract the largest connected subgraph */
    subgraph = Graph_Main_Subgraph(graph, gw);
    
    if (GRAPH_EDGE_NUMBER(subgraph) != 6) {
      PRINT_EXCEPTION("Bug?", "Unexpected graph.");
      return 1;
    }

    /* Use Kill_Graph() to free all memory associated with the graph */
    Kill_Graph(graph);

    /* Create a weighted graph */
    graph = Make_Graph(10, 20, TRUE);
    
    if (Graph_Is_Weighted(graph) == FALSE) {
      PRINT_EXCEPTION("Bug?", "The graph has no weights.");
      return 1;      
    }

    Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
    Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
    Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
    Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
    Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
    Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
    Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
    Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);
    Graph_Add_Weighted_Edge(graph, 1, 7, 1.0);
    Graph_Add_Weighted_Edge(graph, 0, 8, 3.0);
    Graph_Add_Weighted_Edge(graph, 4, 9, 1.0);

    Graph_Update_Edge_Table(graph, gw);

    /* Find shortest path */
    int *path = Graph_Shortest_Path(graph, 0, gw);
    
    /* the distances are stored in gw->dlist */
    for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
      int end = i;
      double dist = gw->dlist[i];
      
      if (tz_isinf(dist)) {
	if (path[end] >= 0) {
	  PRINT_EXCEPTION("Bug?", "Inconsistent distance.");
	  return 1; 
	}
      } else {
	double d = 0.0;
	while (end >= 0) {
	  if (path[end] >= 0) {
	    int index = Graph_Edge_Index_U(end, path[end], gw);
	    d += GRAPH_EDGE_WEIGHT(graph, index);
	  }
	  end = path[end];
	}
	
	if (dist != d) {
	  PRINT_EXCEPTION("Bug?", "Inconsistent distance.");
	  return 1; 	
	}
      }
    }

    { /* unit test*/
      Graph *graph = New_Graph();
      if ((graph->nvertex > 0) || (graph->nedge > 0)) {
	PRINT_EXCEPTION("Bug?", "Non-empty graph.");
	return 1;
      }
      Delete_Graph(graph);

      graph = New_Graph();
      Construct_Graph(graph, 10, 20, FALSE);
      Construct_Graph(graph, 10, 20, TRUE);
      Construct_Graph(graph, 10, 20, FALSE);
      if (graph->weights != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected weights.");
	return 1;
      }
      
      Clean_Graph(graph);
    }

    { /* random data test */
      Graph_Workspace *gw = New_Graph_Workspace();

      int ncycle = 1000;

      /* test unweighted graph */
      while (ncycle > 0) {
	Random_Seed(time(NULL) - getpid());
	int nvertex = Unifrnd_Int(100) + 1;

	if (nvertex <= 1) {
	  continue;
	}

	int nedge = Unifrnd_Int(nvertex * (nvertex - 1) / 2);
	Graph *graph = Make_Graph(nvertex, nedge, FALSE);
	int i;
	for (i = 0; i < nedge; i++) {
	  int v1 = Unifrnd_Int(nvertex - 1);
	  int v2 = Unifrnd_Int(nvertex - 1);
	  if (v1 != v2) {
	    Graph_Add_Edge(graph, v1, v2);
	  }
	}

	Graph_Normalize_Edge(graph);
	Graph_Remove_Duplicated_Edge(graph);

	/* Calculate the degrees of a graph */
	Graph_Degree(graph, gw);
      
	/* Build adjacency matrix for the graph */
	int **adjmat2 = Graph_Adjmat(graph, 0, gw);

	int j;
	for (i = 0; i < GRAPH_EDGE_NUMBER(graph); i++) {
	  if ((adjmat2[GRAPH_EDGE_NODE(graph, i, 0)][GRAPH_EDGE_NODE(graph, i, 1)] 
	       != 1) || 
	      (adjmat2[GRAPH_EDGE_NODE(graph, i, 1)][GRAPH_EDGE_NODE(graph, i, 0)]
	       != 1)) {
	    PRINT_EXCEPTION("Bug?", "Inconsistent adjacency matrix.");
	    return 1;
	  }
	}

	/* Copy adjacency matrix */
	int **adjmat;
	MALLOC_2D_ARRAY(adjmat, GRAPH_VERTEX_NUMBER(graph), 
			GRAPH_VERTEX_NUMBER(graph), int, i);
      
	for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
	  for (j = 0; j < GRAPH_VERTEX_NUMBER(graph); j++) {
	    adjmat[i][j] = adjmat2[i][j];
	    if (adjmat2[i][j] != adjmat2[j][i]) {
	      PRINT_EXCEPTION("Bug?", "Unsymmetric adjacency matrix.");
	      return 1;
	    }
	  }
	}

	/* Build neighbor list */
	int **nbrs = Graph_Neighbor_List(graph, gw);
	for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
	  for (j = 1; j <= GRAPH_NUMBER_OF_NEIGHBORS(i, nbrs); j++) {
	    if (adjmat[i][GRAPH_NEIGHBOR_OF(i, j, nbrs)] != 1) {
	      PRINT_EXCEPTION("Bug?", "Unexpected neighbors.");
	      return 1;
	    }
	  }
	}
	FREE_2D_ARRAY(adjmat, GRAPH_VERTEX_NUMBER(graph));

	Kill_Graph(graph);
	ncycle--;
      }

      /* test weighted graph */
      Graph_Workspace *gw2 = New_Graph_Workspace();

      ncycle = 1000;
      while (ncycle > 0) {
	Random_Seed(time(NULL) - getpid());
	int nvertex = Unifrnd_Int(100) + 1;

	if (nvertex <= 1) {
	  continue;
	}

	int nedge = Unifrnd_Int(nvertex * (nvertex - 1) / 2);
	Graph *graph = Make_Graph(nvertex, nedge, TRUE);
	int i;
	for (i = 0; i < nedge; i++) {
	  int v1 = Unifrnd_Int(nvertex - 1);
	  int v2 = Unifrnd_Int(nvertex - 1);
	  double w = Unifrnd() + 1.0;
	  if (v1 != v2) {
	    Graph_Add_Weighted_Edge(graph, v1, v2, w);
	  }
	}

	Graph_Normalize_Edge(graph);
	Graph_Remove_Duplicated_Edge(graph);

	Graph_Update_Edge_Table(graph, gw);

	/* Find shortest path */
	int *path = Graph_Shortest_Path(graph, 0, gw);
    
	/* the distances are stored in gw->dlist */
	for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
	  int end = i;
	  double dist = gw->dlist[i];
      
	  if (tz_isinf(dist)) {
	    if (path[end] >= 0) {
	      PRINT_EXCEPTION("Bug?", "Inconsistent distance.");
	      return 1; 
	    }
	  } else {
	    double d = 0.0;
	    while (end >= 0) {
	      if (path[end] >= 0) {
		int index = Graph_Edge_Index_U(end, path[end], gw);
		d += GRAPH_EDGE_WEIGHT(graph, index);
	      }
	      end = path[end];
	    }

	    if (Compare_Float(dist, d, 1e-3) != 0) {
	      PRINT_EXCEPTION("Bug?", "Inconsistent distance.");
	      return 1; 	
	    }
	  }
	}

	Graph_Remove_Edge(graph, 0);

	Graph_Shortest_Path(graph, 0, gw2);
	for (i = 0; i < GRAPH_VERTEX_NUMBER(graph); i++) {
	  if (gw->dlist[i] > gw2->dlist[i]) {
	     PRINT_EXCEPTION("Bug?", "Inconsistent distance.");
	     return 1; 
	  }
	}

	Kill_Graph(graph);

	ncycle--;
      }
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  int i, j;

  Graph *graph = Make_Graph(10, 20, TRUE);
  graph->directed = FALSE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);
  Graph_Add_Weighted_Edge(graph, 1, 7, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 8, 3.0);
  Graph_Add_Weighted_Edge(graph, 4, 9, 1.0);

  Graph_Normalize_Edge(graph);
  
  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_Update_Edge_Table(graph, gw);

  Graph_To_Dot_File(graph, "../data/test.dot");

  Graph_To_Mst2(graph, gw);

  Graph_To_Dot_File(graph, "../data/test2.dot");


  int *path = Graph_Shortest_Path(graph, 0, gw);

  for (i = 0; i < gw->nvertex; i++) {
    Graph_Print_Path(path, gw->nvertex, i);
    printf("dist: %g\n", gw->dlist[i]);
  }


  /*
  Graph_Prune(graph, gw);
  Print_Graph(graph);
  */

  printf("%d\n", Graph_Edge_Index(1, 2, gw));

  Graph_Weight_Matrix(graph, gw);

  for (i = 0; i < gw->nvertex; i++) {
    for (j = 0; j < gw->nvertex; j++) {
      printf("%3g ", gw->weight[i][j]);
    }
    printf("\n");
  }

  int **neighbors;

  MALLOC_2D_ARRAY(neighbors, gw->nvertex, gw->nvertex, int, i);
  double *dist = darray_malloc(gw->nvertex);
  path = iarray_malloc(gw->nvertex);

  int8_t *checked = (int8_t *) malloc(sizeof(int8_t) * gw->nvertex);
  for (i = 0; i < gw->nvertex; i++) {
    checked[i] = 0;
  }
  Graph_Shortest_Path_From_Adjmat(gw->weight, gw->nvertex, neighbors, 0, dist,
				  path, checked);

  for (i = 0; i < gw->nvertex; i++) {
    Graph_Print_Path(path, gw->nvertex, i);
    printf("dist: %g\n", dist[i]);
  }

  Kill_Graph_Workspace(gw);
  Kill_Graph(graph);
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, TRUE);
  graph->directed = FALSE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);
  //Graph_Add_Weighted_Edge(graph, 1, 7, 1.0);
  Graph_Add_Weighted_Edge(graph, 7, 8, 3.0);
  Graph_Add_Weighted_Edge(graph, 8, 9, 1.0);

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 9);

  Print_Graph(subgraph);

  Kill_Graph(graph);
  Kill_Graph(subgraph);
  Kill_Graph_Workspace(gw);
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, TRUE);
  Graph_Workspace *gw = New_Graph_Workspace();

  graph->directed = FALSE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);

  int i;
  for (i = 0; i < graph->nedge; i++) {
    Graph_Expand_Edge_Table(graph->edges[i][0], graph->edges[i][1], i, gw);
  }

  for (i = 0; i < graph->nedge; i++) {
    printf("%d, %d\n", Graph_Edge_Index(graph->edges[i][0], graph->edges[i][1],
					gw), i);
  }

  printf("%d\n", Graph_Edge_Index(30, 29, gw));

  Kill_Graph_Workspace(gw);
  Kill_Graph(graph);
#endif

#if 0 /* find all subgraph */
  Graph *graph = Make_Graph(10, 20, TRUE);
  graph->directed = FALSE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 6, 1.0);
  Graph_Add_Weighted_Edge(graph, 7, 8, 3.0);
  Graph_Add_Weighted_Edge(graph, 3, 0, 5.0);
  //Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  //  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  //  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 8, 9, 1.0);

  Graph_Workspace *gw = New_Graph_Workspace();

  Unipointer_List *graph_list = NULL;

  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[0][0]);
  Print_Graph(subgraph);

  Unipointer_List_Add(&graph_list, subgraph);

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);

  int i;
  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == 0) {
      subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
      Print_Graph(subgraph);
      Unipointer_List_Add(&graph_list, subgraph);
    }
  }

  iarray_print2(gw->elist, graph->nedge, 1);

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);
 
  printf("%d\n", Unipointer_List_Length(graph_list));
#endif

#if 0
  //Stack *stack = Read_Stack("../data/sphere_data.tif");
  //Stack *stack2 = Read_Stack("../data/fly_brain3.tif");
  Stack *stack2 = Read_Stack("../data/fly_brain_tmp.tif");
  stack2 = Downsample_Stack_Max(stack2, 1, 1, 0);

  stack2->array += stack2->width * (stack2->height / 2) * stack2->kind;

  Stack *stack = Make_Stack(stack2->kind, stack2->width, stack2->depth, 1);
  int area = stack->width * stack->height;

  int k;
  for (k = 0; k < stack2->depth; k++) {
    memcpy(stack->array + stack->kind * stack->width * k, stack2->array, stack2->kind * stack2->width);
    stack2->array += stack2->kind * stack2->width * stack2->height;
  }

  
  //stack->array += area * 124;
  //stack->depth = 1;

  Stretch_Stack_Value(stack);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;
    
  int slice_number = stack->depth;


  //ima.arrayc += area * 124;
  tic();

  int slice_index = 0;
  for (slice_index = 0; slice_index < slice_number; slice_index++) {
    printf("slice %d\n", slice_index);

    stack->depth = 1;
    //int range[6] ={50, 62, 38, 63, 51, 51};
    //int range[6] = {0, 500, 0, 580, 0, 0};
    //int range[6] = {500, 900, 0, 580, 0, 0};
    int range[6];
    range[0] = stack->width / 8;
    range[1] = stack->width / 2 - stack->width / 8;
    range[2] = 0;
    range[3] = stack->height - 1;
    range[4] = 0;
    range[5] = 0;
    
    Graph *graph = Stack_Graph(stack, 8, range, Stack_Voxel_Weight);
    
    Print_Graph_Info(graph);
    
    Graph_Workspace *gw = New_Graph_Workspace();
    
    int swidth = range[1] - range[0] + 1;
    int sheight = range[3] - range[2] + 1;
    int sarea = swidth * sheight;
    
    
    int start = 0;
    int *path = Graph_Shortest_Path(graph, start, gw);
  
    
    int i;
    /*
      for (i = 0; i < gw->nvertex; i++) {
      Graph_Print_Path(path, gw->nvertex, i);
      printf("dist: %g\n", gw->dlist[i]);
      }
    */

    //Stack *canvas = Translate_Stack(stack, COLOR, 0);

    int end = sarea * (range[5] - range[4] + 1) - 1;
    
    //Graph_Print_Path(path, start, end);
    //printf("%.4f\n", gw->dlist[i]);
    
    i = end;
    while (i >= 0) {
      int index = Stack_Subindex(i, range[0], range[2], range[4], swidth, sarea, 
				 stack->width, area);
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
      i = path[i];
    }
    
    Kill_Graph(graph);
    Kill_Graph_Workspace(gw);
    
    range[0] = stack->width / 2 + stack->width / 8;
    range[1] = stack->width - stack->width / 8;
    
    graph = Stack_Graph(stack, 8, range, Stack_Voxel_Weight);
    
    swidth = range[1] - range[0] + 1;
    start = swidth - 1;
    sarea = swidth * sheight;
    end = sarea * (range[5] - range[4] + 1) - swidth;

    gw = New_Graph_Workspace();
    path = Graph_Shortest_Path(graph, start, gw);
    
    i = end;
       
    while (i >= 0) {
      int index = Stack_Subindex(i, range[0], range[2], range[4], 
				 swidth, sarea, 
				 stack->width, area);
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
      i = path[i];
    }
    
    Kill_Graph(graph);
    Kill_Graph_Workspace(gw);

    stack->array += area;
    ima.arrayc += area;
  }

  printf("Time passed: %lld\n", toc());

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  //Stack *stack = Read_Stack("../data/sphere_data.tif");
  Stack *stack2 = Read_Stack("../data/fly_brain3.tif");

  /*
  stack2->array += stack2->width * (stack2->height / 2) * stack2->kind;

  Stack *stack = Make_Stack(stack2->kind, stack2->width, stack2->depth, 1);
  int area = stack->width * stack->height;

  int k;
  for (k = 0; k < stack2->depth; k++) {
    memcpy(stack->array + stack->kind * stack->width * k, stack2->array, stack2->kind * stack2->width);
    stack2->array += stack2->kind * stack2->width * stack2->height;
  }
  */

  Stack *stack = Crop_Stack(stack2, 0, stack2->height / 2, 0, stack2->width,
			    1, stack2->depth, NULL);
  stack->height = stack->depth;
  stack->depth = 1;
  
  Stretch_Stack_Value(stack);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;
    
  int slice_number = stack->depth;


  //ima.arrayc += area * 124;
  tic();

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  Stack_Graph_Workspace_Set_Range(sgw, stack->width / 8, 
				  stack->width / 2 - stack->width / 8, 0,
				  stack->height - 1, 0, 0);    
  Graph *graph = Stack_Graph(stack, 8, sgw->range, Stack_Voxel_Weight);
    
  Print_Graph_Info(graph);
    
  sgw->gw = New_Graph_Workspace();
    
  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sarea = swidth * sheight;
    
    
  int start = 0;
  int *path = Graph_Shortest_Path(graph, start, sgw->gw);
    
  int end = sarea * (sgw->range[5] - sgw->range[4] + 1) - 1;

  Int_Arraylist *offset_path = 
    Parse_Stack_Shortest_Path(path, start, end, stack->width, stack->height,
			      sgw);
							 
  int i;
  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    ima.arrayc[index][0] = 255;
    ima.arrayc[index][1] = 0;
    ima.arrayc[index][2] = 0;
  }
    
  Kill_Graph(graph);
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  printf("Time passed: %lld\n", toc());

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *stack2 = Read_Stack("../data/fly_neuron_Crop.tif");
  Stack *stack =  Copy_Stack(stack2);
  //Crop_Stack(stack2, 0, 0, 142, stack2->width,
  //			     stack2->height, 1, NULL);
  //Stretch_Stack_Value_Q(stack, 0.999);
  Stack_Invert_Value(stack);  
  tic();

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  Stack_Graph_Workspace_Set_Range(sgw, 0, 
				  stack->width - 1, 0,
				  stack->height - 1, 142, 143);    
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight;

  Graph *graph = Stack_Graph(stack, sgw->conn, sgw->range, sgw->wf);
    
  Print_Graph_Info(graph);
    
  sgw->gw = New_Graph_Workspace();
    
  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sarea = swidth * sheight;
  int sdepth = sgw->range[5] - sgw->range[4] + 1;
    
  int start = Stack_Util_Offset(141, 89, 0, swidth, sheight, sdepth);
  int end =  Stack_Util_Offset(123, 151, 1, swidth, sheight, sdepth);

  if (start > end) {
    int tmp;
    SWAP2(start, end, tmp);
  }
  int *path = Graph_Shortest_Path(graph, start, sgw->gw);

  Int_Arraylist *offset_path = 
    Parse_Stack_Shortest_Path(path, start, end, stack->width, stack->height,
			      sgw);
							 
  int i;

  Write_Stack("../data/test2.tif", stack);
  Stack *canvas = Translate_Stack(stack2, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;

  printf("%g\n", sgw->gw->dlist[end] / offset_path->length);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    ima.arrayc[index][0] = 255;
    ima.arrayc[index][1] = 0;
    ima.arrayc[index][2] = 0;
  }
    
  Kill_Graph(graph);
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  printf("Time passed: %lld\n", toc());

  Write_Stack("../data/test.tif", canvas);  
#endif

#if 0
  Stack *stack2 = Read_Stack("../data/fly_neuron.tif");
  Stack *stack =  Copy_Stack(stack2);
  //Stack_Invert_Value(stack);  

  tic();

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;

  /*
  int start[3] = {467, 337, 175};
  int end[3] =  {464, 228, 148};  
  */

  int start[3] = {457, 309, 133};
  int end[3] = {467, 176, 170};
  Stack_Graph_Workspace_Set_Range_M(sgw, start[0], end[0], start[1], end[1], 
				    start[2], end[2], 10, 10, 10, 10, 10, 10);

  sgw->signal_mask = stack;

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start, end, sgw);

  sgw->signal_mask = NULL;

  int i;

  Kill_Int_Arraylist(offset_path);
  offset_path = Stack_Route(stack, start, end, sgw);

#  if 1							 
  Write_Stack("../data/test2.tif", stack);
  Stack *canvas = Translate_Stack(stack2, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    printf("%d ", index);
    ima.arrayc[index][0] = 255;
    ima.arrayc[index][1] = 0;
    ima.arrayc[index][2] = 0;
  }
    
  printf("Time passed: %lld\n", toc());

  Write_Stack("../data/test.tif", canvas);  

  Kill_Stack(canvas);
#  endif

  Kill_Int_Arraylist(offset_path);
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Stack(stack);
  Kill_Stack(stack2);
#endif

#if 0
  int conn[][5] = {
    { 0, 1, 0, 1, 1 },
    { 1, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 1 },
    { 1, 1, 0, 0, 1 },
    { 1, 0, 1, 1, 0 }
  };

  int *pconn[5];
  int i;
  for (i = 0; i < 5; i++) {
    pconn[i] = conn[i];
  }

  if(Is_Adjmat_Connected(pconn, 5) == TRUE) {
    printf("connected\n");
  } else {
    printf("not connected\n");
  }

  int *nnbr = Adjmat_Isolate(pconn, 5, NULL);
  iarray_print2(nnbr, 5, 1);
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, TRUE);
  Graph_Workspace *gw = New_Graph_Workspace();

  graph->directed = FALSE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);

  int **conn = Graph_Adjmat(graph, 0, gw);
  int i;
  for (i = 0; i < 10; i++) {
    iarray_print2(conn[i], 10, 1);
  }

  if(Is_Adjmat_Connected(conn,10) == TRUE) {
    printf("connected\n");
  } else {
    printf("not connected\n");
  }

  int *nnbr = Adjmat_Isolate(conn, 10, NULL);
  iarray_print2(nnbr, 10, 1);
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, TRUE);
  Graph_Workspace *gw = New_Graph_Workspace();

  //graph->directed = TRUE;
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);
  Graph_Add_Weighted_Edge(graph, 11, 6, 1.0);
  Graph_Add_Weighted_Edge(graph, 11, 12, 1.0);
  Graph_Add_Weighted_Edge(graph, 13, 12, 5.0);
  Graph_Add_Weighted_Edge(graph, 13, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 14, 15, 2.0);
  Graph_Add_Weighted_Edge(graph, 12, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 10, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 10, 16, 1.0);
  
  Graph_To_Mst2(graph, gw);
  
  Graph_To_Dot_File(graph, "../data/test.dot");

  Graph_Traverse_Direct(graph, 1, gw);
  graph->directed = TRUE;
  Graph_To_Dot_File(graph, "../data/test2.dot");
#endif
  
#if 0
  Graph *graph = Make_Graph(10, 20, TRUE);
  Graph_Workspace *gw = New_Graph_Workspace();

  graph->directed = TRUE;
  //Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 1.0);
  Graph_Add_Weighted_Edge(graph, 3, 2, 5.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 4, 5, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 1.0);
  //Graph_Add_Weighted_Edge(graph, 0, 4, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 1.0);

  Graph_Add_Weighted_Edge(graph, 11, 12, 1.0);
  Graph_Add_Weighted_Edge(graph, 13, 12, 5.0);
  Graph_Add_Weighted_Edge(graph, 13, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 14, 15, 2.0);
  Graph_Add_Weighted_Edge(graph, 12, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 10, 14, 1.0);
  Graph_Add_Weighted_Edge(graph, 10, 16, 1.0);
  
  //Graph_To_Mst2(graph, gw);
  
  Graph_To_Dot_File(graph, "../data/test2.dot");
 
  int *in_degree = Graph_In_Degree(graph, gw);
  iarray_print2(in_degree, gw->nvertex, 1);

  int *out_degree = Graph_Out_Degree(graph, gw);
  iarray_print2(out_degree, gw->nvertex, 1);

  int **child = Graph_Child_List(graph, gw);
  int i;
  for (i = 0; i < graph->nvertex; i++) {
    if (child[i][0] > 0) {
      printf("%d: ", i);
      int j;
      for (j = 1; j <= child[i][0]; j++) {
	printf("%d ", child[i][j]);
      }
      printf("\n");
    }
  }

  int *index = Graph_Toposort(graph, gw);
  
  iarray_print2(index, graph->nvertex, 1);

  Graph_To_Dot_File(graph, "../data/test.dot");

  Arrayqueue queue = Graph_Traverse_B(graph, 1, gw);

  Print_Arrayqueue(&queue);
  /*
  Graph *subgraph = Graph_Main_Subgraph(graph, gw);

  Arrayqueue aq = Graph_Traverse_B(graph, 0, gw);
  Print_Arrayqueue(&aq);
  */
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, FALSE);
  Graph_Workspace *gw = New_Graph_Workspace();

  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 3, 2);
  Graph_Add_Edge(graph, 3, 0);
  Graph_Add_Edge(graph, 4, 5);
  Graph_Add_Edge(graph, 2, 4);
  Graph_Add_Edge(graph, 0, 6);
  Graph_Add_Edge(graph, 0, 7);
  Graph_Add_Edge(graph, 7, 8);
 
  Graph_To_Dot_File(graph, "../data/test.dot");

  /*
  Graph_Clean_Root(graph, 0, gw);

  Graph_To_Dot_File(graph, "../data/test2.dot");
  */

  Graph_Insert_Node(graph, 0, 5, 9, gw);
  Graph_To_Dot_File(graph, "../data/test2.dot");
#endif

#if 0
  Graph *graph = Make_Graph(10, 20, FALSE);
  Graph_Workspace *gw = New_Graph_Workspace();

  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 3, 2);
  Graph_Add_Edge(graph, 2, 1);
  Graph_Add_Edge(graph, 3, 0);
  Graph_Add_Edge(graph, 4, 5);
  Graph_Add_Edge(graph, 2, 1);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 4, 6);
  Graph_Add_Edge(graph, 2, 4);

  Print_Graph(graph);
  
  Graph_Normalize_Edge(graph);
  Graph_Remove_Duplicated_Edge(graph);

  Graph *graph2 = Make_Graph(10, 20, FALSE);
  Graph_Add_Edge(graph2, 1, 2);
  Graph_Add_Edge(graph2, 3, 2);
  Graph_Normalize_Edge(graph2);

  printf("%d\n", Graph_Edge_Count(graph, graph2->edges, graph2->nedge, gw));

#endif

#if 1
  Stack *stack2 = Read_Stack("../data/benchmark/line.tif");
  Stack *stack =  Copy_Stack(stack2);
  
  //Stack_Invert_Value(stack);  

  tic();

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;
  sgw->group_mask = Read_Stack("../data/benchmark/stack_graph/mask1.tif");

  int start[3] = {22, 14, 7};
  //int start[3] = {20, 20, 7};
  //int start[3] = {20, 14, 5};
  //int end[3] =  {70, 50, 10};
  int end[3] = {45, 35, 9};
  //int end[3] =  {25, 25, 7};
  Stack_Graph_Workspace_Set_Range(sgw, start[0], end[0], 
				  start[1], end[1], 
				  start[2], end[2]);

  int i;

  int nvoxel = Stack_Voxel_Number(stack);
  
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] > 0) {
      stack->array[i] = 255;
    }
  }

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start, end, sgw);

  //Kill_Int_Arraylist(offset_path);
  //offset_path = Stack_Route(stack, start, end, sgw);

  //Write_Stack("../data/test2.tif", sgw->group_mask);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
    
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  printf("Time passed: %lld\n", toc());

  Write_Stack("../data/test.tif", canvas);  
  //Write_Stack("../data/presentation9/line_mask_path.tif", canvas);  
#endif

#if 0
  Graph *graph = Make_Graph(10, 14, FALSE);
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 5);
  Graph_Add_Edge(graph, 0, 8);
  Graph_Add_Edge(graph, 5, 6);
  Graph_Add_Edge(graph, 1, 7);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 9);
  Graph_Add_Edge(graph, 8, 4);
  Graph_Add_Edge(graph, 2, 7);
  Graph_Add_Edge(graph, 2, 3);
  Graph_Add_Edge(graph, 9, 3);
  Graph_Add_Edge(graph, 3, 4);
  Graph_Add_Edge(graph, 7, 4);
  Graph_Add_Edge(graph, 6, 2);

  double *value = darray_malloc(graph->nvertex);
  int i;
  for (i = 0; i < graph->nvertex; i++) {
    value[i] = 20 - i;
  }
  //Graph_To_Dot_File(graph, "../data/test.dot");
  
  Graph_Workspace *gw = New_Graph_Workspace();
  int end = 9;
  int *path = Graph_Shortest_Path_Maxmin(graph, 0, end, value, gw);
  Graph_Print_Path(path, graph->nvertex, end);
  printf("%g\n", -gw->dlist[end]);
#endif

#if 0
  Graph *graph = Make_Graph(10, 14, FALSE);
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 5);
  Graph_Add_Edge(graph, 0, 8);
  Graph_Add_Edge(graph, 5, 6);
  Graph_Add_Edge(graph, 1, 7);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 9);
  Graph_Add_Edge(graph, 8, 4);
  Graph_Add_Edge(graph, 2, 7);
  Graph_Add_Edge(graph, 2, 3);
  Graph_Add_Edge(graph, 9, 3);
  Graph_Add_Edge(graph, 3, 4);
  Graph_Add_Edge(graph, 7, 4);
  Graph_Add_Edge(graph, 6, 2);
  Print_Graph(graph);
  int nedge = Graph_Remove_Vertex(graph, 3);
  printf("%d edges removed\n", nedge);
  Print_Graph(graph);
#endif

#if 0
  Graph *graph = Make_Graph(0, 1, TRUE);
  Graph_Add_Weighted_Edge(graph, 0, 1, 1.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 2, 3, 3.0);
  Graph_Add_Weighted_Edge(graph, 2, 4, 3.0);
  Graph_Add_Weighted_Edge(graph, 0, 5, 3.0);
  Graph_Add_Weighted_Edge(graph, 0, 6, 2.0);
  Graph_Add_Weighted_Edge(graph, 0, 7, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 8, 3.0);
  Graph_Add_Weighted_Edge(graph, 7, 9, 1.0);
  Graph_Workspace *gw = New_Graph_Workspace();
  Print_Graph(graph);
  Graph_To_Dot_File(graph, "../data/test.dot");
  Graph_Mwt(graph, gw);
  Print_Graph(graph);
  Graph_To_Dot_File(graph, "../data/test2.dot");
#endif
  
#if 0
  Stack *stack = Index_Stack(GREY, 6, 6, 1);
  Set_Stack_Pixel(stack, 2, 2, 0, 0, 0);
  Print_Stack_Value(stack);

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_A;
  sgw->signal_mask = stack;
  //Stack_Graph_Workspace_Set_Range(sgw, 1, 2, 1, 2, 0, 0);
  Graph *graph = Stack_Graph_W(stack, sgw);
  sgw->signal_mask = NULL;
  
  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_To_Mst2(graph, gw);
  Graph_To_Dot_File(graph, "../data/test.dot");
  Graph_Mwt(graph, gw);
  Graph_To_Dot_File(graph, "../data/test2.dot");
  
  Print_Graph(graph);
  

#endif

#if 0
  int i;
  double **weight;
  MALLOC_2D_ARRAY(weight, 5, 5, double, i);
  /*
  weight[0][0]=7;
  weight[0][1]=2;
  weight[0][2]=1;
  weight[0][3]=9;
  weight[0][4]=4;
  weight[1][0]=9;
  weight[1][1]=6;
  weight[1][2]=9;
  weight[1][3]=5;
  weight[1][4]=5;
  weight[2][0]=3;
  weight[2][1]=8;
  weight[2][2]=3;
  weight[2][3]=1;
  weight[2][4]=8;
  weight[3][0]=7;
  weight[3][1]=9;
  weight[3][2]=4;
  weight[3][3]=2;
  weight[3][4]=2;
  weight[4][0]=8;
  weight[4][1]=4;
  weight[4][2]=7;
  weight[4][3]=4;
  weight[4][4]=8;
*/
  weight[0][1]=3;
  weight[0][2]=7;
  weight[0][3]=9;
  weight[0][4]=4;
  weight[1][2]=9;
  weight[1][3]=5;
  weight[1][4]=5;
  weight[2][3]=1;
  weight[2][4]=8;
  weight[3][4]=2;
  
  int j;
  for (j= 0; j < 5; j++) {
    for (i = j; i < 5; i++) {
      if (i == j) {
	weight[i][j] = 9999;
      } else {
	weight[i][j] = weight[j][i];
      }
    }
  }
  BOOL **result;
  MALLOC_2D_ARRAY(result, 5, 5, BOOL, i);

  tic();
  double cost = Adjmat_Hungarian(weight, 4, 4, result);
  ptoc();
  
  printf("cost: %g\n", cost);

  int x;
  int y;
  for (y=0;y<4;++y)
    for (x=0;x<4;++x)
      if (result[y][x])
	printf("%d and %d are connected in the assignment\n",y,x);


#endif

#if 0
  Graph *graph = Make_Graph(0, 1, TRUE);
  Graph_Add_Weighted_Edge(graph, 0, 1, 1.1);
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.3);
  Graph_Add_Weighted_Edge(graph, 0, 3, 2.3);
  Graph_Add_Weighted_Edge(graph, 1, 2, Infinity);
  Graph_Add_Weighted_Edge(graph, 1, 3, 1.2);
  Graph_Add_Weighted_Edge(graph, 2, 3, 3.1);
  Print_Graph(graph);
  
  Graph_Workspace *gw = New_Graph_Workspace();
  BOOL **conn = Graph_Hungarian_Match(graph, gw);
  
  int x,y; 
  for (y=0;y<graph->nvertex;++y)
    for (x=0;x<graph->nvertex;++x)
      if (conn[y][x])
	printf("%d and %d are connected in the assignment\n",y,x);
#endif

#if 0
  Graph *graph = Make_Graph(0, 1, TRUE);
  Graph_Add_Weighted_Edge(graph, 0, 1, 1.1);
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.3);
  Graph_Add_Weighted_Edge(graph, 1, 2, 2.3);

  Random_Seed(time(NULL) - getpid());
  double *x = Unifrnd_Double_Array(graph->nvertex, NULL);
  double *y = Unifrnd_Double_Array(graph->nvertex, NULL);
  
  /*
  x[0] = 1.0;
  x[1] = 2.0;
  x[2] = 3.0;
 
  y[0] = 1.0;
  y[1] = 2.0;
  y[2] = 1.0;
  */

  Graph_Workspace *gw = New_Graph_Workspace();

  Graph_Layout(graph, x, y, gw);

  darray_print2(x, graph->nvertex, 1);
  darray_print2(y, graph->nvertex, 1);

  Kill_Graph_Workspace(gw);
#endif

#if 0
  Graph *graph = Make_Graph(0, 1, FALSE);
  /*
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 2);
  Graph_Add_Edge(graph, 1, 3);
  Graph_Add_Edge(graph, 2, 3);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 4, 3);
  Graph_Add_Edge(graph, 5, 3);
  Graph_Add_Edge(graph, 5, 6);
  Graph_Add_Edge(graph, 6, 0);
  Graph_Add_Edge(graph, 5, 0);
  */

  /*
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 0, 4);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 4);
  Graph_Add_Edge(graph, 2, 3);
  Graph_Add_Edge(graph, 2, 4);
  Graph_Add_Edge(graph, 3, 4);
  */

  /*
  Graph_Add_Edge(graph, 0, 5);
  Graph_Add_Edge(graph, 0, 2);
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 4);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 2, 5);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 4);
  Graph_Add_Edge(graph, 3, 4);
  */


  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 7);
  Graph_Add_Edge(graph, 0, 8);
  Graph_Add_Edge(graph, 0, 2);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 2, 9);
  Graph_Add_Edge(graph, 2, 3);
  Graph_Add_Edge(graph, 3, 4);
  Graph_Add_Edge(graph, 3, 9);
  Graph_Add_Edge(graph, 4, 5);
  Graph_Add_Edge(graph, 4, 9);
  Graph_Add_Edge(graph, 5, 6);
  Graph_Add_Edge(graph, 5, 8);
  Graph_Add_Edge(graph, 6, 7);
  Graph_Add_Edge(graph, 6, 8);
  Graph_Add_Edge(graph, 7, 8);

  Graph_Workspace *gw = New_Graph_Workspace();

  Arrayqueue aq = Graph_Traverse_Lbfs(graph, gw);

  Print_Arrayqueue(&aq);
#endif

#if 0
  Graph *graph = Make_Graph(0, 1, FALSE);
  /*
  Graph_Add_Edge(graph, 0, 5);
  Graph_Add_Edge(graph, 0, 2);
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 4);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 2, 5);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 4);
  Graph_Add_Edge(graph, 3, 4);
  */

  /*
  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 3);
  Graph_Add_Edge(graph, 0, 4);
  Graph_Add_Edge(graph, 1, 2);
  Graph_Add_Edge(graph, 1, 3);
  Graph_Add_Edge(graph, 1, 4);
  Graph_Add_Edge(graph, 1, 5);
  Graph_Add_Edge(graph, 2, 4);
  Graph_Add_Edge(graph, 2, 5);
  Graph_Add_Edge(graph, 4, 5);
  Graph_Add_Edge(graph, 3, 4);
  Graph_Add_Edge(graph, 3, 6);
  Graph_Add_Edge(graph, 3, 7);
  Graph_Add_Edge(graph, 4, 7);
  Graph_Add_Edge(graph, 5, 7);
  Graph_Add_Edge(graph, 6, 7);
  Graph_Add_Edge(graph, 5, 8);
  Graph_Add_Edge(graph, 4, 8);
  Graph_Add_Edge(graph, 7, 8);
  */

  Graph_Add_Edge(graph, 0, 1);
  Graph_Add_Edge(graph, 0, 2);
  Graph_Add_Edge(graph, 3, 1);
  Graph_Add_Edge(graph, 3, 2);
  Graph_Add_Edge(graph, 3, 0);

  Graph_To_Dot_File(graph, "../data/test.dot");

  Graph_Workspace *gw = New_Graph_Workspace();
  Arrayqueue aq = Graph_Traverse_Lbfs(graph, gw);
  Print_Arrayqueue(&aq);

  if (Graph_Has_Hole(graph) == TRUE) {
    printf("The graph has a hole.\n");
  }
#endif

#if 1
  Graph *graph = Make_Graph(0, 1, TRUE);
  graph->directed = TRUE;
  Graph_Add_Weighted_Edge(graph, 0, 5, 1.0);
  Graph_Add_Weighted_Edge(graph, 0, 2, 2.0);
  Graph_Add_Weighted_Edge(graph, 0, 1, 3.0);
  Graph_Add_Weighted_Edge(graph, 0, 4, 3.0);
  Graph_Add_Weighted_Edge(graph, 0, 3, 3.0);
  Graph_Add_Weighted_Edge(graph, 2, 5, 3.0);
  Graph_Add_Weighted_Edge(graph, 1, 2, 3.0);
  Graph_Add_Weighted_Edge(graph, 1, 4, 3.0);
  Graph_Add_Weighted_Edge(graph, 3, 4, 5.5);

  Print_Graph(graph);

  Write_Graph("../data/test.gr", graph); 
  
  Graph *graph2 = Read_Graph("../data/test.gr");
  Print_Graph(graph2);
#endif

  return 0;
}
