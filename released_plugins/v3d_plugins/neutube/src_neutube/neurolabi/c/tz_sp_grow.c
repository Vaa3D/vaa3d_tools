#include "tz_sp_grow.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <string.h>
#include "tz_utilities.h"
#include "tz_stack_attribute.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_lib.h"
#include "tz_error.h"
#include "tz_stack_utils.h"
#include "tz_stack_threshold.h"
#include "tz_int_histogram.h"

#include "private/tz_graph_utils.c"

DEFINE_ZOBJECT_INTERFACE(Sp_Grow_Workspace)

void Default_Sp_Grow_Workspace(Sp_Grow_Workspace *sgw)
{
  sgw->size = 0;
  sgw->dist = NULL;
  sgw->path = NULL;
  sgw->length = NULL;
  sgw->checked = NULL;
  sgw->flag = NULL;
  sgw->mask = NULL;
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;
  sgw->resolution[0] = 1.0;
  sgw->resolution[1] = 1.0;
  sgw->resolution[2] = 1.0;
  sgw->value = 0.0;
  sgw->fgratio = 0.0;
  sgw->sp_option = 0;
  sgw->width = 0;
  sgw->height = 0;
  sgw->depth = 0;
}

void Clean_Sp_Grow_Workspace(Sp_Grow_Workspace *sgw)
{
  OBJECT_SAFE_FREE(sgw->dist, free);
  OBJECT_SAFE_FREE(sgw->path, free);
  OBJECT_SAFE_FREE(sgw->checked, free);
  OBJECT_SAFE_FREE(sgw->flag, free);
  OBJECT_SAFE_FREE(sgw->length, free);
}

void Print_Sp_Grow_Workspace(Sp_Grow_Workspace *sgw)
{
  printf("Workspace for shortest path growing workspace: \n");
  printf("Size: %lld", (int64_t) sgw->size);
}

void Sp_Grow_Workspace_Set_Mask(Sp_Grow_Workspace *sgw, uint8_t *mask)
{
  sgw->mask = mask;
}

/* check the neighbor and update the heap if necessary */
static void update_waiting(const Stack *stack, size_t r, size_t nbr_index, 
			   double weight, Int_Arraylist *heap, 
			   Int_Arraylist *result, Sp_Grow_Workspace *sgw)
{
  if ((sgw->checked[nbr_index] != 1) && 
      (sgw->flag[nbr_index] != SP_GROW_BARRIER)) {
    double tmpdist = sgw->dist[r];
    double eucdist = weight;

    if ((sgw->wf != NULL) && (sgw->sp_option == 0)) { /* calculate weight */
      sgw->argv[0] = weight;
      sgw->argv[1] = Stack_Array_Value(stack, r);
      sgw->argv[2] = Stack_Array_Value(stack, nbr_index);
      weight = sgw->wf(sgw->argv);
    }

    if (sgw->sp_option == 1) {
      tmpdist = imax2(tmpdist, -Stack_Array_Value(stack, r));
    } else {
      tmpdist += weight;
    }


    if (tmpdist < sgw->dist[nbr_index]) { /* update geodesic distance */
      sgw->dist[nbr_index] = tmpdist;
      sgw->path[nbr_index] = r;
      if (sgw->length != NULL) {
        sgw->length[nbr_index] = sgw->length[r] + eucdist;
      }	
    }

    if (sgw->sp_option == 1) {
      if (tmpdist == sgw->dist[nbr_index]) {
        if (sgw->path[nbr_index] >= 0) {
          double v1 = Stack_Array_Value(stack, sgw->path[nbr_index]);
          double v2 = Stack_Array_Value(stack, r);
          if (v2 > v1) {
            sgw->path[nbr_index] = r;
          }
        }
      }
    }

    if (sgw->checked[nbr_index] > 1) { /* update heap */
      Int_Heap_Update_I(heap, nbr_index, sgw->dist, sgw->checked);
    } else if (sgw->checked[nbr_index] <= 0) { /* add to heap */
      Int_Heap_Add_I(heap, nbr_index, sgw->dist, sgw->checked);
    }
  }
}

/* add a neighbor to the flood filling queue for super conductor */
static size_t update_neighbor(size_t cur_index, size_t tail_index,
			      size_t nbr_index, Sp_Grow_Workspace *sgw)
{
  if ((sgw->checked[nbr_index] == 0) && 
      (sgw->flag[nbr_index] == 4)) {
    sgw->dist[nbr_index] = sgw->dist[cur_index];
    sgw->path[nbr_index] = tail_index;
    sgw->checked[nbr_index] = -1; /* make sure it is not enqueued again */
    tail_index = nbr_index;
  }

  return tail_index;
}

Int_Arraylist* Stack_Sp_Grow(const Stack *stack, const size_t *seeds, 
			     int nseed, const size_t *targets, int ntarget, 
			     Sp_Grow_Workspace *sgw)
{
  size_t nvoxel = Stack_Voxel_Number(stack);
  sgw->width = stack->width;
  sgw->height = stack->height;
  sgw->depth = stack->depth;

  /* allocate workspace */
  if (sgw->size < nvoxel) {
    sgw->dist = (double*) Guarded_Realloc(sgw->dist, sizeof(double) * nvoxel,
					  "Stack_Sp_Grow");
    sgw->path = (int*) Guarded_Realloc(sgw->path, sizeof(int) * nvoxel,
				       "Stack_Sp_Grow");
    sgw->checked = (int*) Guarded_Realloc(sgw->checked, sizeof(int) * nvoxel,
					  "Stack_Sp_Grow");
    sgw->flag = (uint8_t*) Guarded_Realloc(sgw->flag, sizeof(uint8_t) * nvoxel,
					  "Stack_Sp_Grow");
  } else {
    if (sgw->dist == NULL) {
      sgw->dist = (double*) Guarded_Malloc(sizeof(double) * nvoxel,
					   "Stack_Sp_Grow");
    }
    if (sgw->path == NULL) {
      sgw->path = (int*) Guarded_Malloc(sizeof(int) * nvoxel,
					"Stack_Sp_Grow");
    }
    if (sgw->checked == NULL) {
      sgw->checked = (int*) Guarded_Malloc(sizeof(int) * nvoxel,
					   "Stack_Sp_Grow");
    }
    if (sgw->flag == NULL) {
      sgw->flag = (uint8_t*) Guarded_Malloc(sizeof(uint8_t) * nvoxel,
					   "Stack_Sp_Grow");    
    }
  }

  sgw->size = nvoxel;
  
  /* initialize */
  size_t s;
  for (s = 0; s < nvoxel; s++) {
    sgw->dist[s] = Infinity;
    if (sgw->length != NULL) {
      sgw->length[s] = 0.0;
    }
    sgw->path[s] = -1;
    sgw->checked[s] = 0;
    sgw->flag[s] = 0;
  }

  if (sgw->mask != NULL) {
    memcpy(sgw->flag, sgw->mask, nvoxel);
  }

  /* Recruit seeds (source) */
  int i;
  for (i = 0; i < nseed; i++) {
    if (sgw->sp_option == 1) {
      sgw->dist[seeds[i]] = -Stack_Array_Value(stack, seeds[i]);
    } else {
      sgw->dist[seeds[i]] = 0.0;
    }
    sgw->checked[seeds[i]] = 1;
    sgw->flag[seeds[i]] = SP_GROW_SOURCE;
  }
  
  if (sgw->mask != NULL) {
    for (s = 0; s < nvoxel; s++) {
      if (sgw->flag[s] == SP_GROW_SOURCE) {
        if (sgw->sp_option == 1) {
          sgw->dist[s] = -Stack_Array_Value(stack, s);
        } else {
          sgw->dist[s] = 0.0;
        }
        sgw->checked[s] = 1;	
      }
    }
  }

  Int_Arraylist *result = New_Int_Arraylist();

  for (i = 0; i < ntarget; i++) {
    if (sgw->flag[targets[i]] == 2) { /* overlap of source and target */
      Int_Arraylist_Add(result, targets[i]);
      sgw->value = 0.0;
      return result;
    }
    sgw->flag[targets[i]] = SP_GROW_TARGET;
  }

  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);

  double dist[26];
  Stack_Neighbor_Dist_R(sgw->conn, sgw->resolution, dist);

  int is_in_bound[26];
  int neighbors[26];
  Stack_Neighbor_Offset(sgw->conn, Stack_Width(stack), Stack_Height(stack), 
			neighbors);			

  BOOL stop = FALSE;

  /* Check neighbors of seeds */
  int j;
  //  for (i = 0; i < nseed; i++) {
  // size_t r = seeds[i];

  /* alloc <heap> */
  Int_Arraylist *heap = New_Int_Arraylist();

  size_t r;
  for (r = 0; r < nvoxel; r++) {
    if (sgw->flag[r] == SP_GROW_SOURCE) { /* seeds */
      int nbound = Stack_Neighbor_Bound_Test_I(sgw->conn, width, height, depth, 
          r, is_in_bound);

      if (nbound == sgw->conn) { /* all neighbors are in bound */
        for (j = 0; j < sgw->conn; j++) {
          size_t nbr_index = r + neighbors[j];
          if (sgw->checked[nbr_index] != 1) {
            update_waiting(stack, r, nbr_index, dist[j], heap, result, sgw);
          }
        }
      } else if (nbound > 0) {
        for (j = 0; j < sgw->conn; j++) {
          if (is_in_bound[j]) {
            size_t nbr_index = r + neighbors[j];
            if (sgw->checked[nbr_index] != 1) {
              update_waiting(stack, r, nbr_index, dist[j], heap, result, sgw);
            }
          }
        }
      }
    }
  }

  //Verify_Int_Heap_I(heap, sgw->dist);

  ssize_t last_r = -1;

  while (stop == FALSE) {
    ssize_t r = extract_min(sgw->dist, sgw->checked, sgw->size, heap);

    if (r >= 0) {
      last_r = r;
      if (sgw->flag[r] == 0) { /* normal voxel */
        int nbound = Stack_Neighbor_Bound_Test_I(sgw->conn, width, height, 
            depth, r, is_in_bound);

        if (nbound == sgw->conn) { /* all neighbors are in bound */
          for (j = 0; j < sgw->conn; j++) {
            size_t nbr_index = r + neighbors[j];
            if (sgw->checked[nbr_index] != 1) {
              update_waiting(stack, r, nbr_index, dist[j], heap, result, sgw);
            }
          }
        } else if (nbound > 0) {
          for (j = 0; j < sgw->conn; j++) {
            if (is_in_bound[j]) {
              size_t nbr_index = r + neighbors[j];
              if (sgw->checked[nbr_index] != 1) {
                update_waiting(stack, r, nbr_index, dist[j], heap, result, sgw);
              }
            }
          }
        }
        //Print_Int_Heap(heap, "%d");
        //Verify_Int_Heap_I(heap, sgw->dist);
      } else if (sgw->flag[r] == SP_GROW_TARGET) { /* target reached */
        //Int_Arraylist_Add(result, r);
        stop = TRUE;
      } else if (sgw->flag[r] == SP_GROW_CONDUCTOR) { 
        /* 0-distance region (super conductor) */
        int nbound = Stack_Neighbor_Bound_Test_I(sgw->conn, width, height, 
            depth, r, is_in_bound);
        size_t tail_index = r;
        size_t old_tail_index = tail_index;
        size_t cur_index = r;
#ifdef _DEBUG_2
        printf("r: %lu\n", r);
#endif

#define UPDATE_SUPER_CONDUCTOR				\
        size_t nbr_index = cur_index + neighbors[j];	\
        if (sgw->flag[nbr_index] == 4) {				\
          if (sgw->checked[nbr_index] > 1) {				\
            Int_Heap_Remove_I_At(heap, nbr_index, sgw->dist,		\
                sgw->checked);				\
          }								\
          tail_index = update_neighbor(cur_index, tail_index, nbr_index,sgw); \
        } else {							\
          update_waiting(stack, cur_index, nbr_index, dist[j], heap, result,sgw); \
        }

        if (nbound == sgw->conn) { /* all neighbors are in bound */
          for (j = 0; j < sgw->conn; j++) {
            UPDATE_SUPER_CONDUCTOR
          }
        } else if (nbound > 0) {
          for (j = 0; j < sgw->conn; j++) {
            if (is_in_bound[j]) {
              UPDATE_SUPER_CONDUCTOR
            }
          }
        }

        int count = 0;
        while (tail_index != old_tail_index) {
          old_tail_index = tail_index;

          size_t cur_index = tail_index;
          while (sgw->checked[cur_index] != 1) {
            int nbound = Stack_Neighbor_Bound_Test_I(sgw->conn, width, height, 
                depth, cur_index, 
                is_in_bound);
            if (nbound == sgw->conn) { /* all neighbors are in bound */
              for (j = 0; j < sgw->conn; j++) {
                UPDATE_SUPER_CONDUCTOR
              }
            } else if (nbound > 0) {
              for (j = 0; j < sgw->conn; j++) {
                if (is_in_bound[j]) {
                  UPDATE_SUPER_CONDUCTOR
                }
              }
            }

            sgw->checked[cur_index] = 1;
            size_t tmp_index = cur_index;
            cur_index = sgw->path[cur_index];
            sgw->path[tmp_index] = r;
            count++;
          }
        }
#ifdef _DEBUG_2
        int x, y, z;
        printf("count: %d\n", count);
        Stack_Util_Coord(tail_index, stack->width, stack->height, &x, &y, &z);
        printf("Stopped at: %ld: %d %d %d\n", tail_index, x, y, z);
#endif
      }
    } else {
      stop = TRUE;
    }
  }

  if (last_r >= 0) {
    Int_Arraylist_Add(result, last_r);
  }

  if (result->length > 0) { 
    /* extract path*/
    sgw->value = sgw->dist[result->array[0]];
    int cur_index = sgw->path[result->array[0]];
    while (cur_index >= 0) {
      Int_Arraylist_Insert(result, 0, cur_index);
      cur_index = sgw->path[cur_index];
    }
  }
  /* free <heap> */
  Kill_Int_Arraylist(heap);

  return result;
}

void Stack_Sp_Grow_Draw_Result(Stack *canvas, const Int_Arraylist *path,
			       const Sp_Grow_Workspace *sgw)
{
  TZ_ASSERT(Stack_Kind(canvas) == COLOR, "Unsupported stack kind.");
  
  Image_Array ima;
  ima.array = canvas->array;

  if (sgw != NULL) {
    size_t nvoxel = Stack_Voxel_Number(canvas);
    size_t s;
    for (s = 0; s < nvoxel; s++) {
      if (sgw->checked[s] == 1) {
	ima.arrayc[s][0] = 128;
	ima.arrayc[s][1] = 0;
      }
      if (sgw->flag[s] == 2) {
	ima.arrayc[s][0] = 0;
	ima.arrayc[s][1] = 128;
      }
      if (sgw->flag[s] == 1) {
	ima.arrayc[s][0] = 0;
	ima.arrayc[s][2] = 128;
      }
    }
  }

  if (path != NULL) {
    int i;
    for (i = 0; i < path->length; i++) {
      int index = path->array[i];
      BOOL drawit = TRUE;
      if (sgw != NULL) {
	if ((sgw->flag[index] == SP_GROW_SOURCE) || 
	    (sgw->flag[index] == SP_GROW_TARGET)) {
	  drawit = FALSE;
	}
      }
      if (drawit) {
	ima.arrayc[index][0] = 255;
	ima.arrayc[index][1] = 255;
	ima.arrayc[index][2] = 0;
      }
    }
  }
}

void Stack_Sp_Grow_Infer_Parameter(Sp_Grow_Workspace *sgw, const Stack *stack)
{
  if (sgw->wf == Stack_Voxel_Weight_S) {
    int *hist = Stack_Hist(stack);

    double c1, c2;
    int thre = Hist_Rcthre_R(hist, Int_Histogram_Min(hist), 
			     Int_Histogram_Max(hist), &c1, &c2);
    free(hist);
    sgw->argv[3] = thre;
    sgw->argv[4] = c2 - c1;
    if (sgw->argv[4] < 1.0) {
      sgw->argv[4] = 1.0;
    }
    sgw->argv[4] /= 9.2;
  } 
}

static int path_count(Sp_Grow_Workspace *sgw, int idx)
{
  int count = 0;
  while (idx >= 0) {
    count++;
    if (sgw->mask[idx] == SP_GROW_SOURCE) {
      break;
    }
    idx = sgw->path[idx];
  }

  if (idx < 0) {
    count = 0;
  }

  return count;
}

ssize_t Stack_Sp_Grow_Furthest_Point(Sp_Grow_Workspace *sgw, int *max_count)
{
  ssize_t idx = -1;
  *max_count = 0;
  size_t i;
  for (i = 0; i < sgw->size; i++) {
    int count = path_count(sgw, i);
    if (count > *max_count) {
      *max_count = count;
      idx = i;
    }
  }

  return idx;
}
