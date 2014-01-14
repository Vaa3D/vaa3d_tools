/* tz_graph_utils.c
 *
 * 21-May-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include <stdlib.h>
#include "tz_stdint.h"
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_arrayqueue.h"
#include "tz_graph_utils.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_math.h"

void Graph_Wmat_Neighbor_List(double **weight, int **neighbors, int nvertex)
{
  int i, j;
  int n;
  for (i = 0; i < nvertex; i++) {
    n = 0;
    for (j = 0; j < nvertex; j++) {
      if ((weight[i][j] != Infinity) || (weight[j][i] != Infinity)) {
	if (i != j) {
	  n++;
	  GRAPH_NEIGHBOR_OF(i, n, neighbors) = j;
	}
      }
    }
    GRAPH_SET_NUMBER_OF_NEIGHBORS(i, neighbors, n)
  }
}

void Graph_Edge_Neighbor_List(int nvertex, Graph_Edge_t *edges, int nedge,
			      int **neighbors, int **edge_idx)
{
  int i;
  for (i = 0; i < nvertex; i++) {
    GRAPH_SET_NUMBER_OF_NEIGHBORS(i, neighbors, 0)
  }

  for (i = 0; i < nedge; i++) {
    int v1 = edges[i][0];
    int v2 = edges[i][1];
    GRAPH_NUMBER_OF_NEIGHBORS_REF(v1, neighbors)++;
    GRAPH_NUMBER_OF_NEIGHBORS_REF(v2, neighbors)++;
    GRAPH_NEIGHBOR_OF(v1, GRAPH_NUMBER_OF_NEIGHBORS(v1, neighbors),
		      neighbors) = v2;
    GRAPH_NEIGHBOR_OF(v2, GRAPH_NUMBER_OF_NEIGHBORS(v2, neighbors),
		      neighbors) = v1;
    if (edge_idx != NULL) {
      edge_idx[v1][GRAPH_NUMBER_OF_NEIGHBORS(v1, neighbors)]
	= i;
      edge_idx[v2][GRAPH_NUMBER_OF_NEIGHBORS(v2, neighbors)] 
	= i;
    }
  }
}

void Graph_Edge_Child_List(int nvertex, Graph_Edge_t *edges, int nedge,
			   int **neighbors, int **edge_idx)
{
  int i;
  for (i = 0; i < nvertex; i++) {
    GRAPH_NUMBER_OF_NEIGHBORS_REF(i, neighbors) = 0;
  }

  for (i = 0; i < nedge; i++) {
    GRAPH_NUMBER_OF_NEIGHBORS_REF(edges[i][0], neighbors)++;
    GRAPH_NEIGHBOR_OF(edges[i][0], 
		      GRAPH_NUMBER_OF_NEIGHBORS(edges[i][0], neighbors),
		      neighbors) = edges[i][1];
    if (edge_idx != NULL) {
      edge_idx[edges[i][0]][GRAPH_NUMBER_OF_NEIGHBORS(edges[i][0], neighbors)] 
	= i;
    }
  }
}

static int extract_min_b(double *dist, int8_t *checked, int nvertex)
{
  int min_idx;
  double min;

  int i;
  min = Infinity;
  min_idx = -1;
  for (i = 0; i < nvertex; i++) {
    if (checked[i] != 1) {
      if (min > dist[i]) {
	min = dist[i];
	min_idx = i;
      }
    }
  }


  if (min == Infinity) {
    min_idx = -1;
  } else {
    checked[min_idx] = 1;
  }

  return min_idx;
}

void Graph_Shortest_Path_From_Adjmat(double **weight, int nvertex,
				     int **neighbors,
				     int start, double *dist, int *path,
				     int8_t *checked)
{
  int i, j;

  for (i = 1; i < nvertex; i++) {
    dist[i] = Infinity;
    path[i] = -1;
  }

  dist[start] = 0;
  path[start] = -1;
  checked[start] = 1;

  Graph_Wmat_Neighbor_List(weight, neighbors, nvertex);
  
  int prev_vertex = start;
  int cur_vertex = start;
  int updating_vertex;
  double tmpdist;

  for (i = 1; i < nvertex; i++) {
    prev_vertex = cur_vertex;
    for (j = 1; j <= GRAPH_NUMBER_OF_NEIGHBORS(cur_vertex, neighbors); j++) {
      updating_vertex = GRAPH_NEIGHBOR_OF(cur_vertex, j, neighbors);

      if (checked[updating_vertex] == 0) {
	tmpdist = weight[cur_vertex][updating_vertex] + dist[cur_vertex];
	if (dist[updating_vertex] > tmpdist) {
	  dist[updating_vertex] = tmpdist;
	  path[updating_vertex] = cur_vertex;
	}
      }
    }
    cur_vertex = extract_min_b(dist, checked, nvertex);
    if (cur_vertex < 0) {
      break;
    }
  }
}

void Graph_Print_Path(int *path, int nvertex, int end)
{
  Arrayqueue aq;
  aq.head = end;
  aq.capacity = nvertex;
  aq.array = path;
  
  Print_Arrayqueue(&aq);
}

/* Modified from http://www.ngs.noaa.gov/gps-toolbox/span-c.txt */
int Graph_Mst_From_Adjmat(double **graph, Graph_Edge_t *min_span_tree, int n,
			  int *p_num_of_edges, double *p_sum_of_edges)
{
  int i,best_node,new_node,num_nodes_outside, num_of_edges;
  int *closest_existing_node,*not_in_tree,outside_node;
  int all_nodes_in_tree = 0;
  double *smallest_edges, edge_weight, best_edge, sum_of_edges;

  /* allocate memory for arrays */
  closest_existing_node = (int*)malloc( (n)*sizeof(int) );
  if (!closest_existing_node) {
    printf(" In spantree, memory request failed for closest_existing_node matrix ! \n");
    return(1);
  }

  not_in_tree = (int*)malloc( (n)*sizeof(int) );
  if (!not_in_tree) {
    printf(" In spantree, memory request failed for not_in_tree matrix ! \n");
    return(2);
  }

  smallest_edges = (double*)malloc( (n)*sizeof(double) );
  if (!smallest_edges) {
    printf(" In spantree, memory request failed for smallest_edges matrix ! \n");
    return(3);
  }

  /* initialize the node label arrays */
  sum_of_edges = 0.0;
  num_nodes_outside = n - 1;
  new_node = n - 1;  /* the last node is the first node in the MST */
  num_of_edges = 0;
  
  for( i = 0; i < num_nodes_outside; i++) {
    not_in_tree[i] = i;
    smallest_edges[i] = graph[i][new_node];
    closest_existing_node[i] = new_node;
  }

  /* update labels of nodes not yet in tree */
  while( !all_nodes_in_tree ) {
    for( i = 0; i < num_nodes_outside; i++ ) {
      outside_node = not_in_tree[i];
      edge_weight = graph[outside_node][new_node];
      if( smallest_edges[i] > edge_weight ) { 
	smallest_edges[i] = edge_weight;
	closest_existing_node[i] = new_node;
      }
    }
    
    /* find node outside tree nearest to tree */
    best_edge = smallest_edges[0];  /* to start, assume 1st edge is closest */
    for( i = 0; i < num_nodes_outside; i++ ) {
      if( smallest_edges[i] <= best_edge ) {
	best_edge = smallest_edges[i];
	best_node = i;
      }
    }
    
    /* put nodes of appropriate edge into array mst */
    min_span_tree[num_of_edges][0] = not_in_tree[best_node];
    min_span_tree[num_of_edges][1] = closest_existing_node[best_node];
    sum_of_edges = sum_of_edges + best_edge;
    new_node = not_in_tree[best_node];
    num_of_edges++;

    /* delete new tree node from array not_in_tree by replacing it with
       the last node in not_in_tree[], then decrement the number of
       nodes not yet in the tree */
    smallest_edges[best_node] = smallest_edges[num_nodes_outside - 1];
    not_in_tree[best_node] = not_in_tree[num_nodes_outside - 1];
    closest_existing_node[best_node] = 
      closest_existing_node[num_nodes_outside - 1];
    num_nodes_outside--;
    if( num_nodes_outside == 0 ) {
      all_nodes_in_tree = num_of_edges;
    }
  } /* finish while loop when all nodes are in tree */
  
  if (p_num_of_edges != NULL) {
    *p_num_of_edges = num_of_edges;
  }

  if (p_sum_of_edges != NULL) {
    *p_sum_of_edges = sum_of_edges;
  }

  /* free memory */
  free( closest_existing_node );
  free( not_in_tree );
  free( smallest_edges );
  
  return(0);
}

BOOL Is_Adjmat_Connected(int **conn, int n)
{
  int i;
  int *mask = iarray_malloc(n);
  for (i = 0; i < n; i++) {
    mask[i] = 0;
  }

  int conn_num = 0;
  int cur_node = 0;
  Arrayqueue *aq = Make_Arrayqueue(n);
  Arrayqueue_Add_Last(aq, cur_node);
  mask[0] = 1;
  do {
    for (i = 0; i < n; i++) {
      if ((conn[cur_node][i] == 1) && (mask[i] == 0)) {
	Arrayqueue_Add_Last(aq, i);
	mask[i] = 1;
      }
    }
    conn_num++;
    cur_node = aq->array[cur_node];
  } while (cur_node >= 0);
  
  free(mask);
  Kill_Arrayqueue(aq);

  return conn_num == n;  
}

int* Adjmat_Isolate(int **conn, int n, int *nnbr)
{
  /* number of neighbors, including itself */
  /* alloc <nnbr> */
  if (nnbr == NULL) {
    nnbr = iarray_malloc(n);
  }

  int i, j;
  
  for (i = 0; i < n; i++) {
    nnbr[i] = 0;
    for (j = 0; j < n; j++) {
      if (i != j) {
	nnbr[i] += conn[i][j];
      }
    }
  }

  size_t idx;
  int max_nbr = 0;
  
  while ((max_nbr = iarray_max(nnbr, n, &idx)) > 0) {
    for (i = 0; i < n; i++) {
      if (conn[i][idx] == 1) {
	nnbr[i]--;
      }
      conn[idx][i] = 0;
    }
    nnbr[idx] = -1;
  }
  
  /* return <nnbr> */
  return nnbr;
}

/* from http://reptar.uta.edu/NOTES5311/hungarian.c */
#define ADJMAT_HUNGARIAN_IS_ZERO(x) ((x) < 1e-5)
#define ADJMAT_HUNGARIAN_IS_EQUAL(x, y) (Compare_Float(x, y, 1e-5)==0)
#define ADJMAT_HUNGARIAN_COMPARE(x, y) (Compare_Float(x, y, 1e-5))
double Adjmat_Hungarian(double **weight, int m, int n, BOOL **result)
{
  int i,j;
  int size1 = m;
  int size2 = n;

  int k;
  int l;
  double s;
  int t;
  int q;
  int unmatched;
  double cost=0;

  int *col_mate = iarray_calloc(size1);
  int *row_mate = iarray_calloc(size2);
  int *parent_row = iarray_calloc(size2);
  int *unchosen_row = iarray_calloc(size1);
  double *row_dec = darray_calloc(size1);
  double *col_inc = darray_calloc(size2);
  double *slack = darray_calloc(size2);
  double *slack_row = darray_calloc(size2);

  for (i=0;i<size1;++i)
    for (j=0;j<size2;++j)
      result[i][j]=FALSE;

  // Begin subtract column minima in order to start with lots of zeroes 12
  printf("Using heuristic\n");
  for (l=0;l<n;l++) {
    s=weight[0][l];
    for (k=1;k<m;k++)
      if (weight[k][l]< s)
	s=weight[k][l];
    cost+=s;
    if (s!=0)
      for (k=0;k<m;k++)
	weight[k][l]-=s;
  }
  // End subtract column minima in order to start with lots of zeroes 12

#ifdef _DEBUG_2
  for (j = 0; j < size1; j++) {
    for (i = 0; i < size2; i++) {
      printf("%g ", weight[i][j]);
    }
    printf("\n");
  }
#endif

  // Begin initial state 16
  t=0;
  for (l=0;l<n;l++) {
    row_mate[l]= -1;
    parent_row[l]= -1;
    col_inc[l]=0;
    slack[l]=Infinity;
  }
  for (k=0;k<m;k++) {
    s=weight[k][0];
    for (l=1;l<n;l++)
      if (weight[k][l] < s)
	s=weight[k][l];
    row_dec[k]=s;
    for (l=0;l<n;l++)
      if ((s == weight[k][l]) && row_mate[l]<0) {
	col_mate[k]=l;
	row_mate[l]=k;
	goto row_done;
      }
    col_mate[k]= -1;
    unchosen_row[t++]=k;
row_done:
    ;
  }
  // End initial state 16

  // Begin Hungarian algorithm 18
  if (t==0)
    goto done;
  unmatched=t;
  while (1) {
    q=0;
    while (1) {
      while (q<t) {
	// Begin explore node q of the forest 19
	{
	  k=unchosen_row[q];
	  s=row_dec[k];
	  for (l=0;l<n;l++)
	    if (!ADJMAT_HUNGARIAN_IS_EQUAL(slack[l],0)) {
	      double del;
	      del=weight[k][l]-s+col_inc[l];
	      if (ADJMAT_HUNGARIAN_COMPARE(del,slack[l])<0) {
		if (ADJMAT_HUNGARIAN_IS_EQUAL(del, 0)) {
		  if (row_mate[l]<0)
		    goto breakthru;
		  slack[l]=0;
		  parent_row[l]=k;
		  unchosen_row[t++]=row_mate[l];
		} else {
		  slack[l]=del;
		  slack_row[l]=k;
		}
	      }
	    }
	}
	// End explore node q of the forest 19
	q++;
      }

      // Begin introduce a new zero into the matrix 21
      s=Infinity;
      for (l=0;l<n;l++)
	if (!ADJMAT_HUNGARIAN_IS_EQUAL(slack[l],0) && slack[l]<s)
	  s=slack[l];
      for (q=0;q<t;q++)
	row_dec[unchosen_row[q]]+=s;
      for (l=0;l<n;l++)
	if (!ADJMAT_HUNGARIAN_IS_EQUAL(slack[l],0)) {
	  slack[l]-=s;
	  if (ADJMAT_HUNGARIAN_IS_EQUAL(slack[l],0)) {
	    // Begin // look // at // a // new // zero // 22
	    k=slack_row[l];
	    if (row_mate[l]<0) {
	      for (j=l+1;j<n;j++)
		if (ADJMAT_HUNGARIAN_IS_EQUAL(slack[j],0))
		  col_inc[j]+=s;
	      goto breakthru;
	    }
	    else {
	      parent_row[l]=k;
	      unchosen_row[t++]=row_mate[l];
	    }
	    // End // look // at // a // new // zero // 22
	  }
	}
	else
	  col_inc[l]+=s;
      // End introduce a new zero into the
      // matrix 21
    }
breakthru:
    // Begin update the matching 20
    while (1) {
      j=col_mate[k];
      col_mate[k]=l;
      row_mate[l]=k;
      if (j<0)
	break;
      k=parent_row[j];
      l=j;
    }
    // End update the matching 20
    if (--unmatched==0)
      goto done;
    // Begin get ready for another stage 17
    t=0;
    for (l=0;l<n;l++) {
      parent_row[l]= -1;
      slack[l]=Infinity;
    }
    for (k=0;k<m;k++)
      if (col_mate[k]<0) {
	unchosen_row[t++]=k;
      }
    // End get ready for another stage 17
  }
done:

  // Begin doublecheck the solution 23
  for (k=0;k<m;k++)
    for (l=0;l<n;l++)
      if (ADJMAT_HUNGARIAN_COMPARE(weight[k][l], row_dec[k]-col_inc[l]) < 0) {
	cost = Infinity;
	goto cleanup;
      }
  for (k=0;k<m;k++) {
    l=col_mate[k];
    if (l<0 || 
	!ADJMAT_HUNGARIAN_IS_EQUAL(weight[k][l],row_dec[k]-col_inc[l])) {
      cost = Infinity;
      goto cleanup;
    }
  }
  k=0;
  for (l=0;l<n;l++)
    if (!ADJMAT_HUNGARIAN_IS_EQUAL(col_inc[l],0))
      k++;
  if (k>m) {
    cost = Infinity;
    goto cleanup;
  }

  // End doublecheck the solution 23
  // End Hungarian algorithm 18

  for (i=0;i<m;++i) {
    result[i][col_mate[i]]=TRUE;
    /*TRACE("%d - %d\n", i, col_mate[i]);*/
  }
  for (k=0;k<m;++k) {
    for (l=0;l<n;++l) {
      /*TRACE("%d ",weight[k][l]-row_dec[k]+col_inc[l]);*/
      weight[k][l]=weight[k][l]-row_dec[k]+col_inc[l];
    }
    /*TRACE("\n");*/
  }
  for (i=0;i<m;i++)
    cost+=row_dec[i];
  for (i=0;i<n;i++)
    cost-=col_inc[i];

cleanup:
  free(col_mate);
  free(row_mate);
  free(parent_row);
  free(unchosen_row);
  free(row_dec);
  free(col_inc);
  free(slack);
  free(slack_row);

#ifdef _DEBUG_
  printf("cost: %g\n", cost);
#endif

  return cost;
}
