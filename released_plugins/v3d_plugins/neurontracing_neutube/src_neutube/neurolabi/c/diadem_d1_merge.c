/**@file regionmerge.c
 * @author Ting Zhao
 * @date 09-Jul-2010
 */

#include <string.h>
#include "tz_utilities.h"
#include "tz_stack_lib.h"
#include "tz_int_histogram.h"
#include "tz_graph.h"
#include "tz_iarray.h"
#include "tz_stack_attribute.h"
#include "tz_color.h"
#include "tz_string.h"
#include "tz_stack_threshold.h"
#include "tz_image_io.h"

int Detect_Threshold(int **m, int pn, int min_thre){
  //int min_thre = 100;
  int size_thre = min_thre;

  int tn = 0;
  int *ts = (int*)malloc(sizeof(int)*pn*pn);

  for (int i=0; i<(pn-1); ++i)
    for(int j=(i+1); j<pn; ++j)

      if(m[i][j]>0){
	ts[tn++] = m[i][j];
      }

  int *ind = (int*)malloc(sizeof(int)*tn);
  iarray_qsort(ts, ind, tn); // in non-descreasing order

  double max_diff = 0;
  for(int i=0; i<(tn-1); ++i){
    if ((ts[i+1] > min_thre) && (ts[i+1] < 1000)) {
      double ld = ((double) ts[i+1]) / ts[i];
      if(max_diff<ld){
	max_diff = ld;
	size_thre = ts[i+1];
      }
    }
  }

  return size_thre;
}

Graph* Count_Region_Touching_Size(Stack *stack, int objects_n,
    char *stack_name, int size_thre, const char *tm_file)
{
  // find the touching region sizes

  printf("there are %d objects regions.\n", objects_n-1); fflush(stdout);

  int **neighbors_count = (int**)malloc(sizeof(int*)*objects_n);
  for(int i=0; i<objects_n; ++i){
    neighbors_count[i] = (int*)malloc(sizeof(int)*objects_n);
    for(int j=0; j<objects_n; ++j)
      neighbors_count[i][j] = 0;
  }

  int i, j, k, xx, yy, zz, x_low, x_high, y_low, y_high, z_low, z_high;
  int dim[3] = {stack->width, stack->height, stack->depth};

  int winx = 1;
  int winy = 1;
  int winz = 1;

  for(i=0; i<dim[0]; i++)
    for(j=0; j<dim[1]; j++)
      for(k=0; k<dim[2]; k++){
	int c_index = Get_Stack_Pixel(stack,i,j,k,0);
	if(c_index>0){
	  x_low = imax2(0, i-winx);
	  x_high = imin2(dim[0]-1, i+winx);

	  y_low = imax2(0, j-winy);
	  y_high = imin2(dim[1]-1, j+winy);

	  z_low = imax2(0, k-winz);
	  z_high = imin2(dim[2]-1, k+winz);

	  zz = k;
	  for(xx=x_low; xx<=x_high; xx++) {
	    for(yy=y_low; yy<=y_high; yy++) {
	      for(zz=z_low; zz<=z_high; zz++){

		int neigh_index = Get_Stack_Pixel(stack,xx,yy,zz,0);
		// use // original // stack or smoothed stack

		if(neigh_index>0 && neigh_index > c_index)
		  neighbors_count[c_index][neigh_index] ++;
	      }
	    }
	  }
	}
      }

  if (size_thre == -1) {
    int min_thre = 100;
    if (Is_Arg_Matched("-t")) {
      min_thre = Get_Int_Arg("-t");
    }
    size_thre = Detect_Threshold(neighbors_count, objects_n, min_thre);
  }

  printf("Size threshold: %d\n", size_thre);

  /*
  char *file_name = (char*)malloc(sizeof(char)*200);

  strcpy(file_name, stack_name);
  strcat(file_name, "_tm.txt");
*/
  if (tm_file != NULL) {
    FILE * pFile = fopen(tm_file, "w");

    for(int i=1; i<objects_n; ++i){
      for(int j=1; j<objects_n; ++j){
	fprintf(pFile, "%d, ", neighbors_count[i][j]);
	/*
	   if(neighbors_count[i][j] >= size_thre)
	   printf("region %d and %d with touching size %d.\n",
	   i,j,neighbors_count[i][j]);
	 */
      }
      fprintf(pFile, "\n");
    }

    fclose(pFile);
  }

  fflush(stdout);


  Graph *graph = Make_Graph(objects_n, 1, TRUE);

  for(int i=1; i<objects_n; ++i){
    for(int j=1; j<objects_n; ++j){
      if(neighbors_count[i][j] >= size_thre) {
	Graph_Add_Weighted_Edge(graph, i, j, neighbors_count[i][j]);
      }
    }
  }

  for(int i=0; i<objects_n; ++i)
    free(neighbors_count[i]);

  return graph;
}


int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> [-o <string>]", 
    "[-r <string>] [-m <string>] [-t <int>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  //char *stack_name = "../data/diadem_d1_072_label.tif";
  char *stack_name = Get_String_Arg("input");
  Stack *stack = Read_Stack(stack_name);
  int nvoxel = Stack_Voxel_Number(stack);

  Stack *signal = NULL;
  Stack *stack2 = stack;
  if (Is_Arg_Matched("-r")) {
    signal = Read_Stack_U(Get_String_Arg("-r"));
    double thre1 = Stack_Find_Threshold_Locmax_L2(signal, 0, 65535, 1.0);
    printf("threshold: %g\n", thre1);
    Stack_Threshold_Binarize(signal, thre1);
    Stack_Binarize(signal);
    Translate_Stack(signal, GREY, 1);
    stack2 = Copy_Stack(stack);
    int i;
    for (i = 0; i < nvoxel; i++) {
      if (signal->array[i] == 0) {
	stack2->array[i] = 0;
      }
    }
  }

  int *hist = Stack_Hist(stack);

  int hist_max = Int_Histogram_Max(hist);
  //int hist_min = Int_Histogram_Min(hist);

  int touching_size_thre = -1;
  int i;

  char *tm_file = NULL;
  if (Is_Arg_Matched("-m")) {
    tm_file = Get_String_Arg("-m");
  }
    
  /* If there is no more than one region, no merge required. */
  if (hist_max > 1) {
    Graph *graph = Count_Region_Touching_Size(stack2, hist_max+1, stack_name,
	touching_size_thre, tm_file); // include the background
    if (tm_file != NULL) {
      printf("Generated touching matrix. Done.\n");
      return 0;
    }

    Print_Graph(graph);

    int *region_map = iarray_calloc(graph->nvertex);

    int k = 1;
    int j;
    Graph_Workspace *gw = New_Graph_Workspace();
    Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 
	GRAPH_EDGE_NODE(graph, 0, 0));
    for (j = 0;  j < subgraph->nedge; j++) {
      region_map[GRAPH_EDGE_NODE(subgraph, j, 0)] = k;
      region_map[GRAPH_EDGE_NODE(subgraph, j, 1)] = k;
    }
    k++;

    Print_Graph(subgraph);

    /* This is necessary to extract subgraph one by one */
    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);

    for (i = 0; i < graph->nedge; i++) {
      if (gw->elist[i] == 0) {
	subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
	for (j = 0;  j < subgraph->nedge; j++) {
	  region_map[GRAPH_EDGE_NODE(subgraph, j, 0)] = k;
	  region_map[GRAPH_EDGE_NODE(subgraph, j, 1)] = k;
	}
	k++;
	Print_Graph(subgraph);
      }
    }
    Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);

    for (i = 1; i < graph->nvertex; i++) {
      if (region_map[i] == 0) {
	region_map[i] = k++;
      }
    }
    iarray_print2(region_map, graph->nvertex, 1);

    for (i = 0; i < nvoxel; i++) {
      stack->array[i] = region_map[stack->array[i]];
      /*
	 if ((stack->array[i] != 26) && (stack->array[i] != 27)) {
	 stack->array[i] = 0;
	 }
       */
    }
  }

  //int filenum = String_Last_Integer(stack_name);
  /*
  char filepath[500];
  sprintf(filepath, "/Volumes/myerslab/zhaot/DIADEM_NM/label/%03d.tif", filenum);
  */
  char *filepath = Get_String_Arg("-o");
  Write_Stack(filepath, stack);

  return 0;
}
