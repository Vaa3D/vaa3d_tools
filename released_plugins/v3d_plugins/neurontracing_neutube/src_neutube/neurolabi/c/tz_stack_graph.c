/* tz_stack_graph.c
 *
 * 06-Aug-08 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <math.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_stack_utils.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_graph.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_geo3d_utils.h"
#include "tz_math.h"
#include "private/tz_graph_utils.c"

Stack_Graph_Workspace* New_Stack_Graph_Workspace()
{
  Stack_Graph_Workspace *sgw = (Stack_Graph_Workspace*) 
    Guarded_Malloc(sizeof(Stack_Graph_Workspace), "New_Stack_Graph_Workspace");

  Default_Stack_Graph_Workspace(sgw);

  return sgw;
}

void Default_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw)
{
  if (sgw != NULL) {
    sgw->conn = 26;
    sgw->range = NULL;
    sgw->wf = Stack_Voxel_Weight;
    int i;
    for (i = 0; i < STACK_GRAPH_WORKSPACE_ARGC; i++) {
      sgw->argv[i] = NaN;
    }
    sgw->gw = NULL;
    sgw->group_mask = NULL;
    sgw->signal_mask = NULL;
    sgw->resolution[0] = 1.0;
    sgw->resolution[1] = 1.0;
    sgw->resolution[2] = 1.0;
    sgw->sp_option = 0;
    sgw->intensity = NULL;
  }
}

void Delete_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw)
{
  free(sgw);
}

void Clean_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw)
{
  free(sgw->range);
  Kill_Graph_Workspace(sgw->gw);
  sgw->range = NULL;
  sgw->gw = NULL;
  if (sgw->group_mask != NULL) {
    Kill_Stack(sgw->group_mask);
  }
  if (sgw->signal_mask != NULL) {
    Kill_Stack(sgw->signal_mask);
  }

  if (sgw->intensity != NULL) {
    free(sgw->intensity);
  }
}

void Kill_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw)
{
  if (sgw == NULL) {
    return;
  }
  Clean_Stack_Graph_Workspace(sgw);
  Delete_Stack_Graph_Workspace(sgw);
}

void Stack_Graph_Workspace_Set_Range(Stack_Graph_Workspace *sgw, int x0, 
				     int x1, int y0, int y1, int z0, int z1)
{
  if (sgw->range == NULL) {
    sgw->range = iarray_malloc(6);
  }
  int tmp;
  sgw->range[0] = x0;
  sgw->range[1] = x1;
  if (sgw->range[0] > sgw->range[1]) {
    SWAP2(sgw->range[0], sgw->range[1], tmp);
  }
  sgw->range[2] = y0;
  sgw->range[3] = y1;
  if (sgw->range[2] > sgw->range[3]) {
    SWAP2(sgw->range[2], sgw->range[3], tmp);
  }
  sgw->range[4] = z0;
  sgw->range[5] = z1;
  if (sgw->range[4] > sgw->range[5]) {
    SWAP2(sgw->range[4], sgw->range[5], tmp);
  }
}

void Stack_Graph_Workspace_Update_Range(Stack_Graph_Workspace *sgw,
					int x, int y, int z)
{
  if (x < sgw->range[0]) {
    sgw->range[0] = x;
  } else if (x > sgw->range[1]) {
    sgw->range[1] = x;
  }

  if (y < sgw->range[2]) {
    sgw->range[2] = y;
  } else if (y > sgw->range[3]) {
    sgw->range[3] = y;
  }

  if (z < sgw->range[4]) {
    sgw->range[4] = z;
  } else if (z > sgw->range[5]) {
    sgw->range[5] = z;
  }
}

void Stack_Graph_Workspace_Set_Range_M(Stack_Graph_Workspace *sgw, int x0, 
				       int x1, int y0, int y1, int z0, int z1,
				       int mx0, int mx1, int my0, int my1,
				       int mz0, int mz1)
{
  Stack_Graph_Workspace_Set_Range(sgw, x0, x1, y0, y1, z0, z1);
  sgw->range[0] -= mx0;
  sgw->range[1] += mx1;
  sgw->range[2] -= my0;
  sgw->range[3] += my1;
  sgw->range[4] -= mz0;
  sgw->range[5] += mz1;
}

void Stack_Graph_Workspace_Expand_Range(Stack_Graph_Workspace *sgw, 
					int mx0, int mx1, 
					int my0, int my1,
					int mz0, int mz1)
{
  sgw->range[0] -= mx0;
  sgw->range[1] += mx1;
  sgw->range[2] -= my0;
  sgw->range[3] += my1;
  sgw->range[4] -= mz0;
  sgw->range[5] += mz1;  
}

void Stack_Graph_Workspace_Validate_Range(Stack_Graph_Workspace *sgw, 
					  int width, int height, int depth)
{
  if (sgw->range[0] < 0) {
    sgw->range[0] = 0;
  }

  if (sgw->range[1] > --width) {
    sgw->range[1] = width;
  }

  if (sgw->range[2] < 0) {
    sgw->range[2] = 0;
  }

  if (sgw->range[3] > --height) {
    sgw->range[3] = height;
  }

  if (sgw->range[4] < 0) {
    sgw->range[4] = 0;
  }

  if (sgw->range[5] > --depth) {
    sgw->range[5] = depth;
  }
}

double Stack_Graph_Workspace_Dist(const Stack_Graph_Workspace *sgw, int index)
{
  return sgw->gw->dlist[index];
}

double Stack_Voxel_Weight(void *argv)
{
  double v1 = ((double*) argv)[1];
  double v2 = ((double*) argv)[2];
  double d = ((double*) argv)[0];
  
  return (v1 + v2) * (v1 + v2) * d;
}

double Stack_Voxel_Weight_I(void *argv)
{
  double v1 = ((double*) argv)[1];
  double v2 = ((double*) argv)[2];
  double d = ((double*) argv)[0];
  
  return d / (1.0 + v1) + d / (v2 + 1.0);
}

double Stack_Voxel_Weight_R(void *argv)
{
  double v1 = ((double*) argv)[1];
  double v2 = ((double*) argv)[2];
  double d = ((double*) argv)[0];
  
  double weight = (v1 == v2) ? d / sqrt(1.0 + v1) :
    2.0 * d * (sqrt(1.0 + v1) - sqrt(1.0 + v2)) / (v1 - v2);
  
  return weight;
}

double Stack_Voxel_Weight_A(void *argv)
{
  double v1 = ((double*) argv)[1];
  double v2 = ((double*) argv)[2];
  //double d = ((double*) argv)[0];
  
  return 1.0 / (1.0 + v1 + v2);
}

double Stack_Voxel_Weight_S(void *argv)
{
  double v1 = ((double*) argv)[1];
  double v2 = ((double*) argv)[2];
  double d = ((double*) argv)[0];

  double thre = ((double*) argv)[3];
  if (tz_isnan(thre)) {
    thre = 60.0;
  }
  
  double scale = ((double*) argv)[4];
  if (tz_isnan(scale)) {
    scale = 5.0;
  }

  double weight = d * 
    (1.0 / (1.0 + exp((v1 - thre)/scale)) 
     + 1.0 / (1.0 + exp((v2 - thre)/scale)) 
     + 0.00001);

  return weight;
}

Graph* Stack_Graph(const Stack *stack, int conn, const int *range, 
		   Weight_Func_t *wf)
{
  int x, y, z;
  int offset = 0;
  int is_in_bound[26];
  int nbound;
  int i;
  int stack_range[6];

  if (range == NULL) {
    stack_range[0] = 0;
    stack_range[1] = stack->width - 1;
    stack_range[2] = 0;
    stack_range[3] = stack->height - 1;
    stack_range[4] = 0;
    stack_range[5] = stack->depth - 1;
  } else {
    stack_range[0] = imax2(0, range[0]);
    stack_range[1] = imin2(stack->width - 1, range[1]);
    stack_range[2] = imax2(0, range[2]);
    stack_range[3] = imin2(stack->height - 1, range[3]);
    stack_range[4] = imax2(0, range[4]);
    stack_range[5] = imin2(stack->depth - 1, range[5]);
  }
  
  int cdepth = stack_range[5] - stack_range[4];
  int cheight = stack_range[3] - stack_range[2];
  int cwidth = stack_range[1] - stack_range[0];
  
  int nvertex = (cwidth + 1) * (cheight + 1) * (cdepth + 1);

  Graph *graph = Make_Graph(nvertex, nvertex, TRUE);

  int neighbor[26];
  int scan_mask[26];
  Stack_Neighbor_Offset(conn, cwidth + 1, cheight + 1, neighbor);

  const double *dist = Stack_Neighbor_Dist(conn);
  const int *x_offset = Stack_Neighbor_X_Offset(conn);
  const int *y_offset = Stack_Neighbor_Y_Offset(conn);
  const int *z_offset = Stack_Neighbor_Z_Offset(conn);

  double args[3];

  for (i = 0; i < conn; i++) {
    scan_mask[i] = (neighbor[i] > 0);
  }

  for (z = 0; z <= cdepth; z++) {
    for (y = 0; y <= cheight; y++) {
      for (x = 0; x <= cwidth; x++) {
	nbound = Stack_Neighbor_Bound_Test(conn, cwidth, cheight, cdepth, 
					   x, y, z, is_in_bound);
	if (nbound == conn) {
	  for (i = 0; i < conn; i++) {
	    if (scan_mask[i] == 1) {
	      double weight = dist[i];
	      if (wf != NULL) {
		args[0] = dist[i];
		args[1] = Get_Stack_Pixel((Stack *)stack, x + stack_range[0], 
					  y + stack_range[2], 
					  z + stack_range[4], 0);
		args[2] = Get_Stack_Pixel((Stack *)stack, 
					  x + stack_range[0] + x_offset[i],
					  y + stack_range[2] + y_offset[i],
					  z + stack_range[4] + z_offset[i], 0);
		weight = wf(args);
	      }
	      Graph_Add_Weighted_Edge(graph, offset, offset + neighbor[i], 
				      weight);
	    }
	  }
	} else {
	  for (i = 0; i < conn; i++) {
	    if ((scan_mask[i] == 1) && is_in_bound[i]){
	      double weight = dist[i];
	      if (wf != NULL) {
		args[0] = dist[i];
		args[1] = Get_Stack_Pixel((Stack *)stack, x + stack_range[0], 
					  y + stack_range[2], 
					  z + stack_range[4], 0);
		args[2] = Get_Stack_Pixel((Stack *)stack, 
					  x + stack_range[0] + x_offset[i],
					  y + stack_range[2] + y_offset[i],
					  z + stack_range[4] + z_offset[i], 0);
		weight = wf(args);
	      }
	      Graph_Add_Weighted_Edge(graph, offset, offset + neighbor[i], 
				      weight);
	    }
	  }
	}
	offset++;
      }
    }
  }

  return graph;
}

Graph* Stack_Graph_W(const Stack *stack, Stack_Graph_Workspace *sgw)
{
  int x, y, z;
  int offset = 0;
  int is_in_bound[26];
  int nbound;
  int i;
  int stack_range[6];

  int *range = sgw->range;

  if (range == NULL) {
    stack_range[0] = 0;
    stack_range[1] = stack->width - 1;
    stack_range[2] = 0;
    stack_range[3] = stack->height - 1;
    stack_range[4] = 0;
    stack_range[5] = stack->depth - 1;
  } else {
    stack_range[0] = imax2(0, range[0]);
    stack_range[1] = imin2(stack->width - 1, range[1]);
    stack_range[2] = imax2(0, range[2]);
    stack_range[3] = imin2(stack->height - 1, range[3]);
    stack_range[4] = imax2(0, range[4]);
    stack_range[5] = imin2(stack->depth - 1, range[5]);
  }
  
  int cdepth = stack_range[5] - stack_range[4];
  int cheight = stack_range[3] - stack_range[2];
  int cwidth = stack_range[1] - stack_range[0];
  
  int nvertex = (cwidth + 1) * (cheight + 1) * (cdepth + 1);
  sgw->virtualVertex = nvertex;

  BOOL weighted = TRUE;
  if (sgw->sp_option == 1) {
    weighted = FALSE;
    sgw->intensity = darray_malloc(nvertex + 1);
    sgw->intensity[nvertex] = Infinity;
  }
  Graph *graph = Make_Graph(nvertex, nvertex, weighted);

  int neighbor[26];
  int scan_mask[26];
  Stack_Neighbor_Offset(sgw->conn, cwidth + 1, cheight + 1, neighbor);

  int org_neighbor[26];
  Stack_Neighbor_Offset(sgw->conn, Stack_Width(stack), Stack_Height(stack), 
      org_neighbor);

  double dist[26];
  Stack_Neighbor_Dist_R(sgw->conn, sgw->resolution, dist);
  //const double *dist = Stack_Neighbor_Dist(sgw->conn);
  const int *x_offset = Stack_Neighbor_X_Offset(sgw->conn);
  const int *y_offset = Stack_Neighbor_Y_Offset(sgw->conn);
  const int *z_offset = Stack_Neighbor_Z_Offset(sgw->conn);

  /* go forward */
  for (i = 0; i < sgw->conn; i++) {
    scan_mask[i] = (neighbor[i] > 0);
  }

#define STACK_GRAPH_ADD_EDGE(cond) \
  for (i = 0; i < sgw->conn; i++) { \
    if (cond) { \
      int nx = x + stack_range[0]; \
      int ny = y + stack_range[2]; \
      int nz = z + stack_range[4]; \
      if (Graph_Is_Weighted(graph)) { \
	double weight = dist[i]; \
	if (sgw->wf != NULL) { \
	  sgw->argv[0] = dist[i]; \
	  \
	  sgw->argv[1] = Get_Stack_Pixel((Stack *)stack, nx, ny, nz, 0); \
	  sgw->argv[2] = \
	  Get_Stack_Pixel((Stack *)stack, nx + x_offset[i], \
	      ny + y_offset[i], nz + z_offset[i], 0); \
	  weight = sgw->wf(sgw->argv); \
	} \
	Graph_Add_Weighted_Edge(graph, offset, offset + neighbor[i], \
	    weight); \
      } else { \
	Graph_Add_Edge(graph, offset, offset + neighbor[i]); \
	sgw->intensity[offset] = Get_Stack_Pixel((Stack*) stack, \
	      nx, ny, nz, 0); \
      } \
    } \
  }

  int groupVertexMap[256];
  for (i = 0; i < 256; ++i) {
    groupVertexMap[i] = 0;
  }

  int swidth = cwidth + 1;
  int sarea =  (cwidth + 1) * (cheight + 1);
  int area = stack->width * stack->height;
  for (z = 0; z <= cdepth; z++) {
    for (y = 0; y <= cheight; y++) {
      for (x = 0; x <= cwidth; x++) {
	nbound = Stack_Neighbor_Bound_Test_S(sgw->conn, cwidth, cheight, 
					     cdepth, 
					     x, y, z, is_in_bound);
	size_t offset2 = Stack_Subindex((size_t) offset, stack_range[0],
	    stack_range[2], stack_range[4],
	    swidth, sarea, stack->width, area);

#ifdef _DEBUG_2
        if (offset == 36629) {
          printf("debug here\n");
        }
#endif

	if (nbound == sgw->conn) {
	  STACK_GRAPH_ADD_EDGE((scan_mask[i] == 1) && 
              (sgw->signal_mask == NULL ? 1 : 
               ((sgw->signal_mask->array[offset2] > 0) &&
                (sgw->signal_mask->array[offset2+org_neighbor[i]] > 0))))
	} else {
	  STACK_GRAPH_ADD_EDGE((scan_mask[i] == 1) && is_in_bound[i] && 
	      (sgw->signal_mask == NULL ? 1 : 
	       ((sgw->signal_mask->array[offset2] > 0) &&
		(sgw->signal_mask->array[offset2+org_neighbor[i]]) >
		0)))
	}
	if (sgw->group_mask != NULL) {
          int groupId = sgw->group_mask->array[offset2];
	  if (groupId > 0) {
#ifdef _DEBUG_2
	    sgw->group_mask->array[offset2] = 2;
#endif
            int groupVertex = groupVertexMap[groupId];
            if (groupVertex <= 0) {
              groupVertex = nvertex++;
              groupVertexMap[groupId] = groupVertex;
            }

            Graph_Add_Weighted_Edge(graph, groupVertex, offset, 0.0);
	  }
	}

	offset++;
      }
    }
  }

  return graph;
}

int* Stack_Shortest_Path(const Stack *stack, int start, 
			 Stack_Graph_Workspace *sgw)
{
  ASSERT(sgw->range != NULL, "null pointer");
  Graph *graph = Stack_Graph(stack, sgw->conn, sgw->range, sgw->wf);

  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sarea = swidth * sheight;
  int substart = Stack_Subindex(start, -sgw->range[0], -sgw->range[2], 
				-sgw->range[4], stack->width, 
				stack->width * stack->height, swidth, sarea);
  
  int *path = Graph_Shortest_Path(graph, substart, sgw->gw);
  
  Kill_Graph(graph);

  return path;
}

int* Stack_Shortest_Path_E(const Stack *stack, int start, int end,
			 Stack_Graph_Workspace *sgw)
{
  ASSERT(sgw->range != NULL, "null pointer");
  Graph *graph = Stack_Graph(stack, sgw->conn, sgw->range, sgw->wf);

  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sarea = swidth * sheight;
  int substart = Stack_Subindex(start, -sgw->range[0], -sgw->range[2], 
				-sgw->range[4], stack->width, 
				stack->width * stack->height, swidth, sarea);
  int subend = Stack_Subindex(end, -sgw->range[0], -sgw->range[2], 
			      -sgw->range[4], stack->width, 
			      stack->width * stack->height, swidth, sarea);
  
  int *path = Graph_Shortest_Path_E(graph, substart, subend, sgw->gw);
  
  Kill_Graph(graph);

  return path;
}

Int_Arraylist* Parse_Stack_Shortest_Path(int *path, int start, int end,
					 int width, int height,
					 Stack_Graph_Workspace *sgw)
{
  int area = width * height;
  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sdepth = sgw->range[5] - sgw->range[4] + 1;
  int sarea = swidth * sheight;
  int svolume = sarea * sdepth;
  Int_Arraylist *list = Make_Int_Arraylist(0, 1);
  while (end >= 0) {
    int index = -1;
    if (end < svolume) {
      index = Stack_Subindex(end, sgw->range[0], sgw->range[2], sgw->range[4],
			     swidth, sarea, width, area);
    }
    Int_Arraylist_Add(list, index);
    end = path[end];
  }

  return list;
}

int* Stack_Graph_Shortest_Path(const Stack *stack, int start[], int end[],
			       Stack_Graph_Workspace *sgw)
{
  int start_index = Stack_Util_Offset(start[0], start[1], start[2], 
				      stack->width, stack->height, 
				      stack->depth);
  int end_index = Stack_Util_Offset(end[0], end[1], end[2], 
				    stack->width, stack->height, 
				    stack->depth);

  int nvoxel = Stack_Voxel_Number(stack);
  int *path = iarray_malloc(nvoxel);
  double *dist = darray_malloc(nvoxel);
  int *checked = iarray_malloc(nvoxel);

  int i, j;

  for (i = 0; i < nvoxel; i++) {
    dist[i] = Infinity;
    path[i] = -1;
    checked[i] = 0;
  }

  dist[start_index] = 0;
  path[start_index] = -1;
  checked[start_index] = 1;
  
  int prev_vertex = start_index;
  int cur_vertex = start_index;
  int updating_vertex;
  double tmpdist;

  int neighbor[26];
  Stack_Neighbor_Offset(sgw->conn, stack->width, stack->height, neighbor);
  const double *neighbor_dist = Stack_Neighbor_Dist(sgw->conn);

  int is_in_bound[26];
  int nbound;

  Int_Arraylist *heap = Int_Arraylist_New(1, 0);
  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  void *argv[3];
  double v1;
  double v2;
  double d;
  argv[0] = &v1;
  argv[1] = &v2;
  argv[2] = &d;

  for (i = 1; i < nvoxel; i++) {
    prev_vertex = cur_vertex;
    nbound = Stack_Neighbor_Bound_Test_I(sgw->conn, cwidth, cheight, cdepth, 
					 cur_vertex, is_in_bound);
    
    if (nbound == sgw->conn) {
      for (j = 0; j < sgw->conn; j++) {
	updating_vertex = cur_vertex + neighbor[j];
	if (checked[updating_vertex] != 1) {
	  v1 = Stack_Array_Value(stack, cur_vertex);
	  v2 = Stack_Array_Value(stack, updating_vertex);
	  d = neighbor_dist[j];
	  double weight = sgw->wf(argv);
	  tmpdist = weight + dist[cur_vertex];
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
    } else {
      for (j = 0; j < sgw->conn; j++) {
	if (is_in_bound[j]) {
	  updating_vertex = cur_vertex + neighbor[j];
	  if (checked[updating_vertex] != 1) {
	    v1 = Stack_Array_Value(stack, cur_vertex);
	    v2 = Stack_Array_Value(stack, updating_vertex);
	    d = neighbor_dist[j];
	    double weight = sgw->wf(argv);
	    tmpdist = weight + dist[cur_vertex];
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
      }
    }

    cur_vertex = extract_min(dist, checked, nvoxel, heap);

    if (cur_vertex == end_index) {
      break;
    }

    if (cur_vertex < 0) {
      break;
    }
  }

  Kill_Int_Arraylist(heap);
  free(checked);
  free(dist);

  return path;  
}

Int_Arraylist *Stack_Route(const Stack *stack, int start[], int end[],
			   Stack_Graph_Workspace *sgw)
{
  if (sgw->gw == NULL) {
    sgw->gw = New_Graph_Workspace();
  }
  if (sgw->range == NULL) {
    double dist = Geo3d_Dist(start[0], start[1], start[2], end[0], end[1],
        end[2]);
    int margin[3];
    int i = 0;
    for (i = 0; i < 3; ++i) {
      margin[i] = iround(dist - abs(end[i] - start[i] + 1));
      if (margin[i] < 0) {
        margin[i] = 0;
      }
    }

    Stack_Graph_Workspace_Set_Range(sgw, start[0], end[0], start[1], end[1],
				    start[2], end[2]);
    Stack_Graph_Workspace_Expand_Range(sgw, margin[0], margin[0],
        margin[1], margin[1], margin[2], margin[2]);
    Stack_Graph_Workspace_Validate_Range(sgw, stack->width, stack->height,
        stack->depth);
  }

  int swidth = sgw->range[1] - sgw->range[0] + 1;
  int sheight = sgw->range[3] - sgw->range[2] + 1;
  int sdepth = sgw->range[5] - sgw->range[4] + 1;

  int start_index = Stack_Util_Offset(start[0] - sgw->range[0], 
				      start[1] - sgw->range[2], 
				      start[2] - sgw->range[4], 
				      swidth, sheight, sdepth);
  int end_index =  Stack_Util_Offset(end[0] - sgw->range[0], 
				     end[1] - sgw->range[2], 
				     end[2] - sgw->range[4],
				     swidth, sheight, sdepth);

  if (start_index > end_index) {
    int tmp;
    SWAP2(start_index, end_index, tmp);
  }

  ASSERT(start_index >= 0, "Invalid starting index.");
  ASSERT(end_index >= 0, "Invalid ending index.");

  tic();
  Graph *graph = Stack_Graph_W(stack, sgw);
  ptoc();

  tic();
  int *path = NULL;
  
  switch (sgw->sp_option) {
    case 0:
      path = Graph_Shortest_Path_E(graph, start_index, end_index, sgw->gw);
      break;
    case 1:
      {
	//printf("%g\n", sgw->intensity[start_index]);
	sgw->intensity[end_index] = 4012;
	sgw->intensity[start_index] = 4012;
	path = Graph_Shortest_Path_Maxmin(graph, start_index, end_index, 
	    sgw->intensity, sgw->gw);
      }
      break;
  }

  sgw->value = sgw->gw->dlist[end_index];

  Kill_Graph(graph);

  if (isinf(sgw->value)) {
    return NULL;
  }

#ifdef _DEBUG_2
  {
    Graph_Update_Edge_Table(graph, sgw->gw);
    Stack *stack = Make_Stack(GREY, swidth, sheight, sdepth);
    Zero_Stack(stack);
    int nvoxel = (int) Stack_Voxel_Number(stack);
    int index = end_index;
    printf("%d -> %d\n", start_index, end_index);
    while (index >= 0) {
      if (index < nvoxel) {
	stack->array[index] = 255;
      }
      int x, y, z;
      Stack_Util_Coord(index, swidth, sheight, &x, &y, &z);
      printf("%d (%d, %d, %d), %g\n", index, x, y, z, sgw->gw->dlist[index]);
      index = path[index];
    }
    Write_Stack("../data/test2.tif", stack);
    Kill_Stack(stack);
  }
#endif

  Int_Arraylist *offset_path = 
    Parse_Stack_Shortest_Path(path, start_index, end_index, 
			      stack->width, stack->height, sgw);
  

  int org_start = Stack_Util_Offset(start[0], start[1], start[2], stack->width,
				    stack->height, stack->depth);
  if (org_start != offset_path->array[0]) {
    iarray_reverse(offset_path->array, offset_path->length);
  }
  
  int org_end = Stack_Util_Offset(end[0], end[1], end[2], stack->width,
				  stack->height, stack->depth);

  //printf("%d, %d\n", org_end, offset_path->array[offset_path->length -]);
  ASSERT(org_start == offset_path->array[0], "Wrong path head.");
  if (org_end != offset_path->array[offset_path->length - 1]) {
    printf("debug here\n");
  }
  ASSERT(org_end == offset_path->array[offset_path->length - 1], 
  	 "Wrong path tail.");

  ptoc();

  return offset_path;
}
