/* tz_stack_watershed.c
 *
 * 31-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_stack_neighborhood.h"
#include "tz_stack_watershed.h"
#include "tz_iarray.h"
#include "tz_stack_utils.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_int_arraylist.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_int_histogram.h"
#include "tz_stack_stat.h"
#include "tz_error.h"
#include "tz_math.h"

#if 0
Watershed_3d* New_Watershed_3d()
{
  Watershed_3d *watershed = (Watershed_3d *) 
    Guarded_Malloc(sizeof(Watershed_3d),
		   "New_Watershed_3d");
  watershed->nlabels = 0;
  watershed->labels = NULL;
  watershed->nbasins = 0;
  watershed->seeds = NULL;
  watershed->conn = 0;

  return watershed;
}

void Free_Watershed_3d(Watershed_3d *watershed)
{
  free(watershed);
}

void Kill_Watershed_3d(Watershed_3d *watershed)
{
  Kill_Stack(watershed->labels);
  free(watershed->seeds);
  free(watershed);
}

Watershed_3d_Workspace * New_Watershed_3d_Workspace()
{
  Watershed_3d_Workspace *ws = (Watershed_3d_Workspace *)
    Guarded_Malloc(sizeof(Watershed_3d_Workspace),
		   "New_Watershed_3d_Workspace");
  ws->array = NULL;
  ws->max_area = 0;
  ws->conn = 26;
  ws->mask = NULL;

  return ws;
}

void Free_Watershed_3d_Workspace(Watershed_3d_Workspace *ws)
{
  free(ws);
}

void Kill_Watershed_3d_Workspace(Watershed_3d_Workspace *ws)
{
  if (ws->mask != NULL) {
    Kill_Stack(ws->mask);
  }
  free(ws->array);
  free(ws);
}

/* Modified from Gene's watershed functions */

static int *get_chord(int area, Watershed_3d_Workspace *ws)
{ 
  if (area < 0) {
    free(ws->array);
    ws->max_area = 0;
    ws->array = NULL;
  } else if (area > ws->max_area) {
    ws->max_area = area;
    ws->array = (int *) 
      Guarded_Realloc(ws->array, sizeof(int) * ws->max_area, "get_chord");
  }

  return ws->array;
}


Watershed_3d *Build_3D_Watershed(const Stack *stack, Watershed_3d *result, 
				 Watershed_3d_Workspace *ws)
{
  FILE *fp = fopen("../data/out2.txt", "w");
  int nlabels;
  int nbasins;
  int *seeds;

  int index[0x10001];
  int maxval;

  int neighbor[26];
  int n_nbrs = ws->conn;

  uint8  *value8;
  uint16 *value16;

  int   *chord;
  int   *out;

  if (stack->kind == COLOR)
    { fprintf(stderr,"Build_3D_Watershed: Cannot apply to COLOR images\n");
      exit (1);
    }
  if (stack->kind == FLOAT32)
    { fprintf(stderr,"Build_3D_Watershed: Cannot apply to FLOAT32 images\n");
      exit (1);
    }

  
  Stack_Neighbor_Offset(ws->conn, stack->width, stack->height, neighbor);
  int tmpindex[26];
  int tmpi;
  for (tmpi = 0; tmpi < 26; tmpi++) {
    tmpindex[tmpi] = tmpi;
  }
  iarray_qsort(neighbor, tmpindex, ws->conn);
  iarray_print2(neighbor, ws->conn, 1);

  int cwidth   = stack->width;
  int cheight  = stack->height;
  int cdepth   = stack->depth;
  int carea    = cwidth * cheight;
  int cvolume  = carea * cdepth;

  int chk_width = stack->width - 1;
  int chk_height = stack->height - 1;
  int chk_depth = stack->depth - 1;

  int xx, yy, zz;
  int b[26];
  int nbound;

  chord = get_chord(2*cvolume, ws);
  out   = chord + cvolume;

  if (stack->kind == GREY16)
    { maxval  = 0x10000;
      value8  = NULL;
      value16 = (uint16 *) (stack->array);
    }
  else
    { maxval  = 0x100;
      value8  = stack->array;
      value16 = NULL;
    }

  // Establish lists of pixels with each value


  { int p, v;

    for (p = 0; p < cvolume; p++) {
      if (ws->mask != NULL) {
	if (ws->mask->array[p] > 0) {
	  out[p] = MINIM;
	} else {
	  out[p] = IGNORED;
	}
      } else {
	out[p] = MINIM;
      }
    }


    for (v = 0; v <= maxval; v++)
      index[v] = -1;

    if (value8 != NULL)
      for (p = 0; p < cvolume; p++)
        { 
	  if (out[p] != IGNORED) {
	    v        = value8[p];
	    chord[p] = index[v];
	    index[v] = p;
	  } else {
	    chord[p] = -1;
	  }
        }
    else  // value16 != NULL
      for (p = 0; p < cvolume; p++)
        { 
	  if (out[p] != IGNORED) {
	    v        = value16[p];
	    chord[p] = index[v];
	    index[v] = p;
	  } else {
	    chord[p] = -1;
	  }
        }
  }
  

#define PUSH(p)			\
{ if (qbot < 0)			\
    qtop = qbot = (p);		\
  else				\
    qtop = chord[qtop] = (p);	\
  chord[qtop] = -1;		\
}

#define POP(p)  { qbot = chord[p = qbot]; }

  //  Label all pixels that are not part of a catchment basin minimum with INIT
  //    and those that are with MINIM.
#define PUSH_ALL_VOXEL(test, value)					\
  for (j = 0; j < n_nbrs; j++){						\
    if (test && value[p+neighbor[j]] < v && (out[p+neighbor[j]]) != IGNORED) \
      { out[p] = INIT;							\
	/*fprintf(fp, "%d\n", p);*/					\
	PUSH(p);							\
	break;								\
      }									\
  }
  
#define PUSH_ALL_VOXEL2(test, value)					\
  for (j = 0; j < n_nbrs; j++) {					\
    q = p + neighbor[j];						\
    if (test  && out[q] != IGNORED)					\
      {									\
	if (out[q] != INIT && value[q] == v && out[q] != IGNORED)	\
	  { out[q] = INIT;						\
	    /*fprintf(fp, "%d\n", q);*/					\
	    PUSH(q);							\
	  }								\
      }									\
  }				
  

  { int v, p;


    if (value8 != NULL) {
      for (v = 0; v < maxval; v++)
        { int qtop, qbot, t;
  
          //  Push all c-pixels that are adjacent to a pixel of lesser value and mark INIT

          qbot = -1;
          for (p = index[v]; p >= 0; p = t)
            { int j;
	      
              t = chord[p];
	      Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz);
	      nbound = Stack_Neighbor_Bound_Test(ws->conn, chk_width, 
						 chk_height,
						 chk_depth, xx, yy, zz, b);
              if (nbound > 0) {
		if (nbound == n_nbrs) {		
		  PUSH_ALL_VOXEL(1, value8);
		} else {
		  PUSH_ALL_VOXEL(b[tmpindex[j]], value8);
		}
	      }
	      //fprintf(fp, "%d\n", p);
            }

          //  Find all c-pixels connected to another labeled INIT and mark as INIT
  
          while (qbot >= 0)
            { int p, j, q;
  
              POP(p);
              //t = chord[p];
	      Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz);
	      nbound = Stack_Neighbor_Bound_Test(ws->conn, chk_width, 
						 chk_height,
						 chk_depth, xx, yy, zz, b);

	      if (nbound > 0) {
		if (nbound == n_nbrs) {
		  PUSH_ALL_VOXEL2(1, value8);
		} else {
		  PUSH_ALL_VOXEL2(b[tmpindex[j]], value8);
		}
	      }
	    }
        }
    } else {
      for (v = 0; v < maxval; v++)
        { int qtop, qbot, t;
  
          //  Push all c-pixels that are adjacent to a pixel of lesser value and mark INIT

          qbot = -1;
          for (p = index[v]; p >= 0; p = t)
            { int j;
	      
              t = chord[p];
	      Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz);
	      nbound = Stack_Neighbor_Bound_Test(ws->conn, chk_width, 
						 chk_height,
						 chk_depth, xx, yy, zz, b);
              if (nbound > 0) {
		if (nbound == n_nbrs) {
		  PUSH_ALL_VOXEL(1, value16);
		} else {
		  PUSH_ALL_VOXEL(b[tmpindex[j]], value16);
		}
	      }
            }

          //  Find all c-pixels connected to another labeled INIT and mark as INIT
  
          while (qbot >= 0)
            { int p, j, q;
  
              POP(p);
              //t = chord[p];
	      Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz);
	      nbound = Stack_Neighbor_Bound_Test(ws->conn, chk_width, 
						 chk_height,
						 chk_depth, xx, yy, zz, b);
	     
	      if (nbound > 0) {
		if (nbound == n_nbrs) {
		  PUSH_ALL_VOXEL2(1, value16);
		} else {
		  PUSH_ALL_VOXEL2(b[tmpindex[j]], value16);
		}
	      }
	    }
        }
    }
  }
  iarray_print2(index, maxval, 1);
  // Build pixel value lists again but now with only INIT pixels in each list

  { int p, v;

    for (v = 0; v <= maxval; v++)
      index[v] = -1;

    if (value8 != NULL)
      { for (p = 0; p < cvolume; p++)
          if (out[p] == INIT)
            { v        = value8[p];
              chord[p] = index[v];
              index[v] = p;
            }
       }

     else // value16 != NULL
      { for (p = 0; p < cvolume; p++)
          if (out[p] == INIT)
            { v        = value16[p];
              chord[p] = index[v];
              index[v] = p;
            }
      }
  }

  // Find connected components of MINIM pixels and label with a unique catchment basin #.

#define FIND_COMP_MIN(test)			\
  for (j = 0; j < n_nbrs; j++)	{		\
    r = q+neighbor[j];				\
    if (test && out[r] != IGNORED)		\
      {						\
	if (out[r] == MINIM)			\
	  { PUSH(r);				\
	    out[r] = nbasins;			\
	  }					\
      }						\
  }

  { int p;
    int qtop, qbot;

    nbasins = 0;
  
    qbot = -1;
    for (p = 0; p < cvolume; p++)
      if (out[p] == MINIM)
        { PUSH(p);
          out[p] = ++nbasins;
	  fprintf(fp, "%d %d\n", p, out[p]);
          while (qbot >= 0)
            { int q, j, r;

              POP(q);
              //b = boundary_pixels(q);
	      Stack_Util_Coord(q, cwidth, cheight, &xx, &yy, &zz); 
	      nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
						 chk_depth, xx, yy, zz, b);
	      if (nbound > 0) {
		if (nbound == n_nbrs) {
		  FIND_COMP_MIN(1);
		} else {
		  FIND_COMP_MIN(b[tmpindex[j]]);
		}
	      }
            }
        }
  }

#define PUSH_ALL_INIT_VOXEL(test)			\
  for (j = 0; j < n_nbrs; j++)				\
    if (test && out[p+neighbor[j]] >= 0)		\
      { out[p] = ONQUEUE;				\
	PUSH(p);					\
	break;						\
      }

#define LABEL_C_PIXEL(test)					\
  for (j = 0; j < n_nbrs; j++) {				\
    q = p + neighbor[j];					\
    if (test && out[q] != IGNORED)				\
      {								\
	o = out[q];						\
	if (o == MASK)						\
	  { out[q] = ONQUEUE;					\
	    PUSH(q);						\
	  }							\
	else if (o != ONQUEUE && o != INIT && o != IGNORED)	\
	  { if (out[p] == ONQUEUE)				\
	      out[p] = o;					\
	    else if (o != out[p] && o != WSHED && o != IGNORED) {	\
	      if (out[p] == WSHED)				\
		out[p] = o;					\
	      else						\
		out[p] = WSHED;					\
	    }							\
	  }							\
      }								\
  }
  
  
  { int v, p;

    for (v = 0; v < maxval; v++)
      { int qbot, qtop, t;

        // Push all INIT c-pixels that are adjacent to a previously labeled pixel and
        //   label ONQUEUE, label all others MASK

        qbot = -1;
        for (p = index[v]; p >= 0; p = t)
          { int j;

            t      = chord[p];
            out[p] = MASK;
	    
            //b      = boundary_pixels(p);
	    Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz); 
	    nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
					       chk_depth, xx, yy, zz, b);
	    if (nbound > 0) {
		if (nbound == n_nbrs) {
		  PUSH_ALL_INIT_VOXEL(1);
		} else {
		  PUSH_ALL_INIT_VOXEL(b[tmpindex[j]]);
		}
	    }
	  }

        //  In BFS, reach all c-pixels (have MASK set) in order of distance, and label
        //    with catchment basin or WSHED

        while (qbot >= 0)
          { int p, j, q, o;

            POP(p);
            //b = boundary_pixels(p);
	    Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz); 
	    nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
					       chk_depth, xx, yy, zz, b);
	    
	    if (nbound > 0) {
	      if (nbound == n_nbrs) {
		LABEL_C_PIXEL(1);
	      } else {
		LABEL_C_PIXEL(b[tmpindex[j]]);
	      }
	    }
          }
      }
  }

  //  Find all WSHED pixels in order of distance from a non-WSHED pixel and randomly
  //    assign to one of the nearest catchment basins

#define FIND_ALL_WSHED(test)			\
  for (j = 0; j < n_nbrs; j++)			\
    if (test && out[p+neighbor[j]] > 0 &&  out[p+neighbor[j]] != IGNORED) \
      { PUSH(p);				\
	out[p] = ONQUEUE;			\
	break;					\
      }

#define ASSIGN_ALL_WSHED(test)			\
  for (j = 0; j < n_nbrs; j++)	{		\
    q = p + neighbor[j];			\
    if (test && out[q] != IGNORED)		\
      {						\
	o = out[q];				\
	if (o > 0)				\
	  out[p] = o;				\
	else if (out[q] != ONQUEUE)		\
	  { PUSH(q);				\
	    out[q] = ONQUEUE;			\
	  }					\
      }						\
  }

  { int p, qbot, qtop;

    qbot = -1;
    for (p = 0; p < cvolume; p++)
      if (out[p] == WSHED)
        { int j;

          //b = boundary_pixels(p);
	  Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz); 
	  nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
					     chk_depth, xx, yy, zz, b);
	  
	  if (nbound > 0) {
	    if (nbound == n_nbrs) {
	      FIND_ALL_WSHED(1);
	    } else {
	      FIND_ALL_WSHED(b[tmpindex[j]]);
	    }
	  }
        }

    while (qbot >= 0)
      { int j, q, o;

        POP(p);
        //b = boundary_pixels(p);
	Stack_Util_Coord(p, cwidth, cheight, &xx, &yy, &zz); 
	nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
					   chk_depth, xx, yy, zz, b);
	
	if (nbound > 0) {
	  if (nbound == n_nbrs) {
	    ASSIGN_ALL_WSHED(1);
	  } else {
	    ASSIGN_ALL_WSHED(b[tmpindex[j]]);
	  }
	}
      }
  }
  //iarray_fprint2(fp, out, cwidth, cheight);
  //  Make the data structure now

  //result = new_watershed(sizeof(int)*(nbasins+1),"Build_3D_Watershed");
  result->seeds = Guarded_Malloc(sizeof(int)*(nbasins+1),"Build_3D_Watershed");
  result->nbasins = nbasins;
  result->conn  = ws->conn;

  //  Greedily assign labels to catchment basins (CBs) so that no basin is adjacent to
  //    another with the same number.  Typicall the number of labels required is very,
  //    very small, 6 or 7 in my experience.  At the end of this segment, out[p] is
  //    the negative of its label reassignment, seeds[b] is the new label for CB b,
  //    and nlabels is the number of unique labels used.

#define ASSIGN_BASIN_LABEL(test)			\
  for (j = 0; j < n_nbrs; j++){				\
    r = q + neighbor[j];				\
    if (test && out[r] != IGNORED)			\
      {							\
	o = out[r];					\
	if (o == olabel)				\
	  { PUSH(r);					\
	    out[r] = 0;					\
	  }						\
	else						\
	  { o = -o;					\
	    if (o > 0 && o != olabel)			\
	      { o = seeds[o];				\
		adjacent[o] = 1;			\
		if (o > alargest)			\
		  alargest = o;				\
	      }						\
	  }						\
      }							\
  }

  { int   qbot, qtop;
    int   olabel;
    int   adjacent[256];
    int   alargest;
    int   p;

    seeds   = result->seeds;
    nlabels = 0;

    for (p = 1; p < 256; p++)
      adjacent[p] = 0;

    qbot = -1;
    for (p = 0; p < cvolume; p++)
      if (out[p] > 0)

        { PUSH(p);
          olabel   = out[p];
          out[p]   = 0;
          alargest = 0;

          while (qbot >= 0)
            { int j, q, r, o;

              POP(q);
              out[q] = -olabel;
      
              //b = boundary_pixels(q);
	      Stack_Util_Coord(q, cwidth, cheight, &xx, &yy, &zz); 
	      nbound = Stack_Neighbor_Bound_Test(n_nbrs, chk_width, chk_height,
						 chk_depth, xx, yy, zz, b);
	      
	      if (nbound > 0) {
		if (nbound == n_nbrs) {
		  ASSIGN_BASIN_LABEL(1);
		} else {
		  ASSIGN_BASIN_LABEL(b[tmpindex[j]]);
		}
	      }
            }

          { int j;

            for (j = 1; j <= alargest; j++)
              if (! adjacent[j])
                break;

            seeds[olabel] = j;
            if (j > nlabels)
              { nlabels = j;
                if (j > 255)
                  { fprintf(stderr,"Warning: More than 255 colors needed to label watershed\n");
                    fprintf(stderr,"         Returning NULL result\n");
                    fflush(stderr);
                    return (NULL);
                  }
              }

            for (j = 1; j <= alargest; j++)
              adjacent[j] = 0;
          }
        }
  }

  //  Make an image with the assigned labels, and make seeds contain the lexicographically
  //    smalles pixel in each region.

  { int    p, b;
    uint16 *iarray;

    result->nlabels = nlabels;
    result->labels  = Make_Stack(GREY16,cwidth,cheight, cdepth);
    iarray = (uint16 *) (result->labels->array);

    for (p = 0; p < cvolume; p++)
      { 
	if (out[p] == IGNORED) {
	  iarray[p] = 0;
	} else {
	  out[p] = b = -out[p];
	  iarray[p] = seeds[b];
	}
      }
    for (p = 0; p < cvolume; p++)
      if (seeds[b = out[p]]  > 0) 
        seeds[b] = -(p+1); 
    for (b = 1; b <= nbasins; b++)
      seeds[b] = -seeds[b];
  }

  /*
  { int    p;
    int   *color;
    Image *view;

    view = Make_Image(COLOR,frame->width,frame->height);

    for (p = 0; p < carea; p++)
      { if (out[p] <= 0)
          { printf("Unassigned pix\n");
            continue;
          }
        color = palette[(out[p]-1)%8];
        view->array[3*p]   = color[0];
        view->array[3*p+1] = color[1];
        view->array[3*p+2] = color[2];
      }

    Write_Image("watershed.tif",view);

    Kill_Image(view);
  }
  */
  fclose(fp);
  return (result);
}
#endif

DEFINE_ZOBJECT_INTERFACE(Stack_Watershed_Workspace)

void Default_Stack_Watershed_Workspace(Stack_Watershed_Workspace *ws)
{
  ws->conn = 26;
  ws->array = NULL;
  ws->mask = NULL;
  ws->min_level = 0;
  ws->start_level = 65535;
  ws->weights = NULL;
}

void Clean_Stack_Watershed_Workspace(Stack_Watershed_Workspace *ws)
{
  if (ws->array != NULL) {
    free(ws->array);
    ws->array = NULL;
  }

  if (ws->mask != NULL) {
    Kill_Stack(ws->mask);
    ws->mask = NULL;
  }

  ws->weights = NULL;
}

void Print_Stack_Watershed_Workspace(Stack_Watershed_Workspace *ws)
{
  printf("Connection: %d\n", ws->conn);
  if (ws->mask != NULL) {
    printf("Mask size: %d x %d x %d\n", Stack_Width(ws->mask),
	   Stack_Height(ws->mask), Stack_Depth(ws->mask));
  }
}

Stack_Watershed_Workspace* Make_Stack_Watershed_Workspace(Stack *stack)
{
  Stack_Watershed_Workspace *ws = New_Stack_Watershed_Workspace();
  size_t nvoxel = Stack_Voxel_Number(stack);
  GUARDED_MALLOC_ARRAY(ws->array, nvoxel, int);

  return ws;
}

#define STACK_WATERSHED_IS_SEED(mask, i)			\
  (((mask)->array[i] >= 1) && ((mask)->array[i] <= STACK_WATERSHED_MAX_SEED))

void Stack_Watershed_Infer_Parameter(const Stack *stack, 
				     Stack_Watershed_Workspace *ws)
{
  int conn = 18;
  Stack *locmax = Stack_Locmax_Region(stack, conn);
  Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
  int nvoxel = Stack_Voxel_Number(locmax);
  int i;

  for (i = 0; i < nvoxel; i++) {
    if (locmax->array[i] < 3) {
      locmax->array[i] = 0;
    } else {
      locmax->array[i] = 1;
    }
  }

  int *hist = Stack_Hist_M(stack, locmax);

  Kill_Stack(locmax);
  
  int low, high;
  Int_Histogram_Range(hist, &low, &high);
  
  ws->min_level = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
  printf("min level: %d\n", ws->min_level);

  free(hist);
  
  ws->start_level = 0;

  for (i = 0; i < nvoxel; i++) {
    if (STACK_WATERSHED_IS_SEED(ws->mask, i)) {
      int value = (int) Stack_Array_Value(stack, i);
      if (ws->start_level < value) {
	ws->start_level = value;
      }
    }
  }
}

#define STACK_WATERSHED_ZGAP_BARRIER(stack_array)			\
  for (j = 0; j < Stack_Height(stack); j++) {				\
    for (i = 0; i < Stack_Width(stack); i++) {				\
      start = -1;							\
      for (k = 1; k < Stack_Depth(stack); k++) {			\
	int offset2 = offset + area * k;				\
	if (stack_array[offset2] > stack_array[offset2 - area]) {	\
	  if (start >= 0) {						\
	    end = offset2;						\
	    int u;							\
	    for (u = start; u < end; u += area) {			\
	      if (mask->array[u] == 0) {				\
		mask->array[u] = STACK_WATERSHED_BARRIER;		\
	      }								\
	    }								\
	  }								\
	  start = -1;							\
	} else if (stack_array[offset2] < stack_array[offset2 - area]) { \
	  start = offset2;						\
	}								\
      }									\
      offset++;								\
    }									\
  }
  
void Stack_Watershed_Zgap_Barrier(const Stack *stack, Stack *mask)
{
  int i, j, k;
  int area = Stack_Width(stack) * Stack_Height(stack);

  /* scanning status */
  int offset = 0;
  int start, end;
  
  SCALAR_STACK_OPERATION(stack, STACK_WATERSHED_ZGAP_BARRIER);
}

#define STACK_WATERSHED_UNLABELED(i) (out->array[i] == 0)

#define STACK_WATERSHED_ENQUEUE(level, i)	\
  if (queue_head[level] == -1) {		\
    queue_head[level] = i;			\
    queue_tail[level] = i;			\
  } else {					\
    level_queue[queue_tail[level]] = i;		\
    queue_tail[level] = i;			\
  }

#define STACK_WATERSHED_DEQUEUE(level, v)	\
  v = queue_head[level];			\
  if (v >= 0) {					\
    queue_head[level] = level_queue[v];		\
    if (queue_head[level] == -1) {		\
      queue_tail[level] = -1;			\
    }						\
  }

Stack* Stack_Watershed(const Stack *stack, Stack_Watershed_Workspace *ws)
{
  if (ws->mask == NULL) {
    return NULL;
  }

  if (Stack_Voxel_Number(stack) == 1) {
    return NULL;
  }

  Stack *out = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack), 
			  Stack_Depth(stack));
  Zero_Stack(out);
  
  size_t nvoxel = Stack_Voxel_Number(stack);

  int is_in_bound[26];
  int neighbors[26];
  Stack_Neighbor_Offset(ws->conn, Stack_Width(stack), Stack_Height(stack), 
			neighbors);			

  int i;
  
  int max_level = 65535;
  int queue_head[65536];
  int queue_tail[65536];

  for (i = 0; i <= max_level; i++) {
    queue_head[i] = -1;
    queue_tail[i] = -1;
  }

  int j;

  for (i = 0; i < nvoxel; i++) {
    ws->array[i] = -1;
  }

  int water_level = ws->start_level;
  int nseed = 0;
  int *level_queue = ws->array;

  for (i = 0; i < nvoxel; i++) {
    /* If the voxel is an aviable seed */
    if (STACK_WATERSHED_IS_SEED(ws->mask, i)) {
      int basin = ws->mask->array[i];
      if (nseed < basin) {
	nseed = basin;
      }
      out->array[i] = basin;
      int level = (int) Stack_Array_Value(stack, i);
      if (level >= water_level) {
	STACK_WATERSHED_ENQUEUE(water_level, i);
      } else {
	STACK_WATERSHED_ENQUEUE(level, i);
      }
    }
  }


  /* Now start watershed. */
  while (water_level >= ws->min_level) {
#ifdef _DEBUG_2
    printf("water level: %d\n", water_level);
#endif
    int cur_index;
    STACK_WATERSHED_DEQUEUE(water_level, cur_index);
    
    while (cur_index >= 0) {
      int basin = out->array[cur_index];
      int nbound = Stack_Neighbor_Bound_Test_I(ws->conn, 
					       Stack_Width(stack),
					       Stack_Height(stack),
					       Stack_Depth(stack), 
					       cur_index, 
					       is_in_bound);
      
#define STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j)			\
      int nbr = cur_index + neighbors[j];				\
      if (STACK_WATERSHED_UNLABELED(nbr) && (ws->mask->array[nbr] == 0)) { \
	int level;							\
	if (ws->weights == NULL) {					\
	  level = (int) Stack_Array_Value(stack, nbr);			\
	} else {							\
	  level = iround(Stack_Array_Value(stack, nbr) * ws->weights[j]); \
	}								\
	if (level >= ws->min_level) {					\
	  if (level >= water_level) {					\
	    STACK_WATERSHED_ENQUEUE(water_level, nbr);			\
	    out->array[nbr] = basin;					\
	  } else {							\
	    STACK_WATERSHED_ENQUEUE(level, nbr);			\
	    out->array[nbr] = basin;					\
	  }								\
	}								\
      }

      /* Enqueue the neighbors. */
      if (nbound == ws->conn) {
	for (j = 0; j < ws->conn; j++) {
	  STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j);
	}
      } else {
	for (j = 0; j < ws->conn; j++) {
	  if (is_in_bound[j]) {
	    STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j);
	  }
	}
      }

      STACK_WATERSHED_DEQUEUE(water_level, cur_index);
    }

    water_level--;
  }

  return out;
}

#define STACK_REGION_BORDER_SHRINK_ENQUEUE(basin, i)	\
  if (basin_head->array[basin] == -1) {			\
    basin_head->array[basin] = i;			\
    basin_tail->array[basin] = i;			\
  } else {						\
    queue[basin_tail->array[basin]] = i;		\
    basin_tail->array[basin] = i;			\
  }

#define STACK_REGION_BORDER_SHRINK_DEQUEUE(basin, v)	\
  v = basin_head->array[basin];				\
  if (v >= 0) {						\
    basin_head->array[basin] = queue[v];		\
    queue[v] = -1;					\
    if (basin_head->array[basin] == -1) {		\
      basin_tail->array[basin] = -1;			\
    }							\
  }

Stack* Stack_Region_Border_Shrink(const Stack *stack,
				  Stack_Watershed_Workspace *ws)
{
  int nregion = (int) Stack_Max(stack, NULL);
  
  Int_Arraylist *basin_head = Make_Int_Arraylist(nregion, 0);
  Int_Arraylist *basin_tail = Make_Int_Arraylist(nregion, 0);

  int *queue = ws->array;

  int i, j;
  for (i = 0; i < nregion; i++) {
    basin_head->array[i] = -1;
    basin_tail->array[i] = -1;
  }

  int nvoxel = (int) Stack_Voxel_Number(stack);
  int is_in_bound[26];
  int neighbors[26];
  Stack_Neighbor_Offset(ws->conn, Stack_Width(stack), Stack_Height(stack), 
			neighbors);			

  /* Sweep to initialize each basin. */
  for (i = 0; i < nvoxel; i++) {
    queue[i] = -1;
    int basin = stack->array[i];
    if (basin > 0) {
      basin--;

      /* Bound test */
      int nbound = Stack_Neighbor_Bound_Test_I(ws->conn, Stack_Width(stack),
					       Stack_Height(stack),
					       Stack_Depth(stack), 
					       i, is_in_bound);
      if (nbound == ws->conn) {
	for (j = 0; j < ws->conn; j++) {
	  size_t nbr = i + neighbors[j];
	  if (stack->array[nbr] != stack->array[i]) {
	    STACK_REGION_BORDER_SHRINK_ENQUEUE(basin, i);
	    break;
	  }
	}
      } else {
	for (j = 0; j < ws->conn; j++) {
	  if (is_in_bound[j]) {
	    size_t nbr = i + neighbors[j];
	    if (stack->array[nbr] != stack->array[i]) {
	      STACK_REGION_BORDER_SHRINK_ENQUEUE(basin, i);
	      break;
	    }
	  }
	}
      }
    }
  }

  Stack *out = Copy_Stack((Stack *) stack);

  int ridx;

  for (ridx = 0; ridx < nregion; ridx++) {
    int cur_idx = basin_head->array[ridx];
    while (cur_idx >= 0) {
      out->array[cur_idx] = 0;
      cur_idx = queue[cur_idx];
    }
  }

  int niter = 5;
  for (i = 0; i < niter; i++) {
    for (ridx = 0; ridx < nregion; ridx++) {
      int tail_idx = basin_tail->array[ridx];
      int cur_idx;
      
      if (tail_idx >= 0) {
	do {
	  STACK_REGION_BORDER_SHRINK_DEQUEUE(ridx, cur_idx);

	  int nbound = Stack_Neighbor_Bound_Test_I(ws->conn, 
						   Stack_Width(stack),
						   Stack_Height(stack),
						   Stack_Depth(stack), 
						   cur_idx, is_in_bound);
	  if (nbound == ws->conn) {
	    for (j = 0; j < ws->conn; j++) {
	      size_t nbr = cur_idx + neighbors[j];
	      if (out->array[nbr] == ridx + 1) {
		STACK_REGION_BORDER_SHRINK_ENQUEUE(ridx, nbr);
		out->array[nbr] = 0;
		break;
	      }
	    }
	  } else {
	    for (j = 0; j < ws->conn; j++) {
	      if (is_in_bound[j]) {
		size_t nbr = cur_idx + neighbors[j];
		if (out->array[nbr] == ridx + 1) {
		  STACK_REGION_BORDER_SHRINK_ENQUEUE(ridx, nbr);
		  out->array[nbr] = 0;
		  break;
		}
	      }
	    }
	  }	  
	} while (tail_idx != cur_idx);
      }
    }
  }

  Kill_Int_Arraylist(basin_head);
  Kill_Int_Arraylist(basin_tail);

  return out;
}
