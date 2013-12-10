/* testgeo3d.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tz_stdint.h"
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_stack_utils.h"
#include "tz_iarray.h"
#include "tz_objdetect.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_draw.h"
#include "tz_stack_relation.h"
#include "tz_stack_bwmorph.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_attribute.h"
#include "tz_graph.h"
#include "tz_voxel_linked_list_ext.h"
#include "tz_stack_math.h"
#include "tz_stack_threshold.h"
#include "tz_stack_stat.h"
#include "tz_stack_code.h"
#include "tz_image_io.h"
#include "tz_int_histogram.h"
#include "tz_object_3d_linked_list_ext.h"
#include "tz_image_trans.h"

INIT_EXCEPTION_MAIN(e)

int main()
{
#if 0
  int s[26];
  int n_nbr = 26;
  int x = 148;
  int y = 186;
  int z = 144;

  Stack *stack = Read_Stack("../data/fly_neuron.tif");

  Stack_Neighbor_Sampling(stack, n_nbr, x, y, z, -1, s);
  Print_Neighborhood(s, n_nbr);

  int seed = z * stack->width *stack->height + y * stack->width +x;

  Stack_Threshold(stack, 100);
  Stack_Binarize(stack);

  IMatrix *chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);

  tic();
  Stack_Label_Object_Dist_N(stack, chord, seed, -1, 255, 20, n_nbr);
  //Stack_Label_Object(stack, chord, seed, 1, 255);
  //Stack_Find_Object_N(stack, NULL, 1, 0, 26);
  //Stack_Find_Object(stack, 1, 0);
  printf("%lld ms\n", toc());

  Kill_IMatrix(chord);
  Write_Stack("../data/test.tif", stack);
  Kill_Stack(stack);
#endif

#if 0 /* test object_N functions*/
  //Stack *stack = Read_Stack("../data/sphere_data.tif");
  Stack *stack = Read_Stack("../data/fly_neuron/clear_mask.tif");

  //Stack *stack2 = Stack_Perimeter(stack, NULL, 18);

  //Stack_Label_Background_N(stack, 2, 4, NULL);

  Object_3d_List *objs = Stack_Find_Hole_N(stack, 8, NULL);
  Print_Object_3d_List_Compact(objs);

  //Stack_Draw_Objects_Bw(stack, objs, 3);
  
  /*
  Object_3d_List *objs = Stack_Find_Object_N(stack, NULL, 1, 0, 26);
  Print_Object_3d_List_Compact(objs);
  */
  /* 
  Stack *stack2 = Make_Stack(stack->kind, stack->width, stack->height,
			     stack->depth);
  Zero_Stack(stack2);
  Stack_Draw_Objects_Bw(stack2, objs, 255);
  */

  //Write_Stack("../data/test.tif", stack);
  

  //Stack *stack3 = Read_Stack("../data/test2.tif");
  //printf("%d\n", Stack_Identical(stack2, stack3));

#endif

#if 0 /* test Stack_Label_Large_Object_N */
  Stack *stack = Read_Stack("../data/mouse_neuron/mask.tif");
  Stack *stack2 = Copy_Stack(stack);
  IMatrix *chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth);
  //Stack_Label_Large_Objects_N(stack, chord, 1, 2, 5000, 26);
  Object_3d *obj = Stack_Find_Largest_Object_N(stack2, chord, 1, 26);
  Print_Object_3d_Info(obj);

  Graph *mst = New_Graph();
  Object_3d *centers = New_Object_3d();
  Object_3d_To_Mst_S(obj, mst, centers, stack);

  iarray_write("../data/mst.bn", (int *) mst->edges, mst->nedge * 2);
  iarray_write("../data/obj.bn", (int *) centers->voxels, centers->size * 3);

#  if 0
  Voxel_List *list = Object_3d_Central_Points(obj);
  
  printf("%d\n", Voxel_List_Length(list));

  Translate_Stack(stack, COLOR, 1);
  Voxel_List *tmp = list;
  while (tmp != NULL) {
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 0, 1.0);
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 1, 0.0);
    Set_Stack_Pixel(stack, tmp->data->x, tmp->data->y, tmp->data->z, 2, 0.0);
    tmp = tmp->next;
  }

  //printf("%d objects found\n", Stack_Label_Objects_N(stack, chord, 1, 2, 26));
  Write_Stack("../data/test.tif", stack);
#  endif

  Kill_Stack(stack2);
  Kill_Stack(stack);
  Kill_IMatrix(chord);
#endif

#if 0 /* draw a tree in a stack */
  Graph *mst = New_Graph();
  mst->edges = (Graph_Edge_t *) iarray_read("../data/mst.bn", &(mst->nedge));
  mst->nedge /= 2;
  mst->nvertex = mst->nedge + 1;
  mst->edge_capacity = mst->nedge;
  mst->directed = FALSE;
  mst->type = TREE_GRAPH;
  
  Print_Graph_Info(mst);

  int nvoxel;
  Voxel_t *voxels = (Voxel_t *) iarray_read("../data/obj.bn", &nvoxel);
  nvoxel /= 3;

  int i;

  Stack *stack = Read_Stack("../data/mouse_neuron.tif");
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  Rgb_Color color;
  color.r = 255;
  color.g = 0;
  color.b = 0;
  
  for (i = 0; i < mst->nedge; i++) {
    Object_3d *obj = Line_To_Object_3d(voxels[mst->edges[i][0]],
				       voxels[mst->edges[i][1]]);
    Stack_Draw_Object_Bwc(canvas, obj, color);
    Kill_Object_3d(obj);
  }
  
  Write_Stack("../data/label.tif", canvas);
#endif


#if 0 /* create branch points */
  Graph *mst = New_Graph();
  mst->edges = (Graph_Edge_t *) iarray_read("../data/mst.bn", &(mst->nedge));
  mst->nedge /= 2;
  mst->nvertex = mst->nedge + 1;
  mst->edge_capacity = mst->nedge;
  mst->directed = FALSE;
  mst->type = TREE_GRAPH;


  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_Prune(mst, 2, gw);
  //Graph_Prune(mst, gw);

  iarray_write("../data/mst2.bn", (int *) mst->edges, mst->nedge * 2);

  gw->ready = 0;
  gw->allocated = 0;
  Graph_Degree(mst, gw);
  iarray_print2(gw->degree, gw->nvertex, 1);
  iarray_write("../data/degree.bn", gw->degree, mst->nvertex);

  //Stack *stack = Read_Stack("../data/label.tif");
  Stack *stack = Read_Stack("../data/mouse_neuron.tif");
  Translate_Stack(stack, GREY, 1);
  Stack *canvas = Translate_Stack(stack, COLOR, 0);
 
  Stack *painter = Make_Stack(GREY, stack->width, stack->height, stack->depth);

  int nvoxel;
  Voxel_t *voxels = (Voxel_t *) iarray_read("../data/obj.bn", &nvoxel);
  nvoxel /= 3;
  
  int i;
  for (i = 0; i < nvoxel; i++) {
    if (gw->degree[i] > 2) {
      Set_Stack_Pixel(painter, voxels[i][0], voxels[i][1], voxels[i][2], 0, 1);
      //Set_Stack_Pixel(stack, voxels[i][0], voxels[i][1], voxels[i][2], 1, 255.0);
      /*
      Set_Stack_Pixel(stack, voxels[i][0]+1, voxels[i][1], voxels[i][2], 1, 255.0);
      Set_Stack_Pixel(stack, voxels[i][0]-1, voxels[i][1], voxels[i][2], 1, 255.0);
      Set_Stack_Pixel(stack, voxels[i][0], voxels[i][1]+1, voxels[i][2], 1, 255.0);
      Set_Stack_Pixel(stack, voxels[i][0], voxels[i][1]-1, voxels[i][2], 1, 255.0);
      */
    }
  }
  
  Struct_Element *se = Make_Ball_Se(3);
  Stack *out = Stack_Dilate(painter, NULL, se);
  
  Rgb_Color color;
  Set_Color(&color, 255, 0 ,0);
  Stack_Label_Color(canvas, out, 5.0, 1.0, stack);
  
  //printf("%d objects found\n", Stack_Label_Objects_N(stack, chord, 1, 2, 26));
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *stack = Read_Stack("../data/sphere_data.tif");
  Stack *stack16 = Make_Stack(GREY16, 
			      stack->width, stack->height, stack->depth);
  uint16 *array16 = (uint16 *) stack16->array;
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  for (i = 0; i < nvoxel; i++) {
    array16[i] = stack->array[i];
  }
  
  int nseed = 5;
  Objlabel_Workspace ow;
  ow.init_chord = TRUE;
  ow.conn = 26;
  ow.chord = NULL;
  
  int seed[5];
  seed[0] = Stack_Util_Offset(45, 50, 50, stack->width, stack->height, 
			      stack->depth);
  seed[1] = Stack_Util_Offset(51, 45, 50, stack->width, stack->height, 
			      stack->depth);
  seed[2] = Stack_Util_Offset(57, 50, 50, stack->width, stack->height, 
			      stack->depth);
  seed[3] = Stack_Util_Offset(50, 56, 50, stack->width, stack->height, 
			      stack->depth);
  seed[4] = Stack_Util_Offset(55, 46, 50, stack->width, stack->height, 
			      stack->depth);

  uint8_t **connmat;
  MALLOC_2D_ARRAY(connmat, nseed, nseed, uint8_t, i);

  int j;
  for (i = 0; i < nseed; i++) {
    for (j = 0; j < nseed; j++) {
      connmat[i][j] = 0;
    }
  }

  Stack_Build_Seed_Graph(stack16, seed, nseed, connmat, &ow);
  
  for (i = 0; i < nseed; i++) {
    u8array_print2(connmat[i], nseed, 1);
  }

  Write_Stack("../data/test.tif", stack16);

#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron/mask.tif");
  Stack *stack16 = Make_Stack(GREY16, 
			      stack->width, stack->height, stack->depth);
  uint16 *array16 = (uint16 *) stack16->array;
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  for (i = 0; i < nvoxel; i++) {
    array16[i] = stack->array[i];
  }

  
  Object_3d *obj = Stack_Find_Largest_Object_N(stack, NULL, 1, 26);
  Print_Object_3d_Info(obj);

  Voxel_List *list = Object_3d_Central_Points(obj);

  printf("central points done.\n");

  int nseed = Voxel_List_Length(list);

  Objlabel_Workspace ow;
  ow.init_chord = TRUE;
  ow.conn = 26;
  ow.chord = NULL;

  int *seed = iarray_malloc(nseed);
  int offset = 0;

  Voxel_List *tmp = list;
  while (tmp != NULL) {
    seed[offset++] = 
      Stack_Util_Offset(tmp->data->x, tmp->data->y, tmp->data->z,
			stack->width, stack->height, stack->depth);
    tmp = tmp->next;
    //printf("%d, %p\n", offset, tmp);
  }

  ASSERT(offset == nseed, "Wrong number of seeds.");

  /*
  uint8_t **connmat;
  MALLOC_2D_ARRAY(connmat, nseed, nseed, uint8_t, i);
  
  int j;
  for (i = 0; i < nseed; i++) {
    for (j = 0; j < nseed; j++) {
      connmat[i][j] = 0;
    }
  }
  */
  printf("Building graph...\n");

  Graph *graph = Stack_Build_Seed_Graph_G(stack16, seed, nseed, TRUE, &ow);
  
  printf("done\n");

/*
  Stack_Build_Seed_Graph(stack16, seed, nseed, connmat, &ow);
  
  //Write_Stack("../data/test2.tif", stack16);

  int nedge = 0;
  for (i = 0; i < nseed; i++) {
    for (j = 0; j < nseed; j++) {
      nedge += connmat[i][j];
    }
  }

  printf("#nodes: %d, #edges: %d\n", nseed, nedge);
  
  Graph *graph = Make_Graph(nseed, nedge, TRUE);

  int dv[3];
  offset = 0;
  for (i = 0; i < nseed; i++) {
    for (j = 0; j < nseed; j++) {
      if (connmat[i][j] == 1) {
	graph->edges[offset][0] = i;
	graph->edges[offset][1] = j;
	graph->nedge++;
	dv[0] = centers->voxels[i][0] - centers->voxels[j][0];
	dv[1] = centers->voxels[i][1] - centers->voxels[j][1];
	dv[2] = centers->voxels[i][2] - centers->voxels[j][2];
	graph->weights[offset] = 
	  sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
	offset++;
      }
    }
  }
  */
  Print_Graph_Info(graph);

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_To_Mst2(graph, gw);

  Print_Graph_Info(graph);

  iarray_write("../data/mst.bn", (int *) graph->edges, graph->nedge * 2);
  Object_3d *centers = Voxel_List_To_Object_3d(list, NULL);
  iarray_write("../data/obj.bn", (int *) centers->voxels, centers->size * 3);

  Write_Stack("../data/test.tif", stack16);
#endif

#if 0
  Pixel_Array pa = Pixel_Array_Read("../data/mouse_neuron/seeds.pa");
  Graph *graph = Stack_Build_Seed_Graph_G(stack16, seed, nseed, TRUE, &ow);

#endif

#if 0
  Stack *seeds = Read_Stack("../data/fly_neuron2/soma.tif");
  Stack_Threshold_Binarize(seeds, 2);
  Stack *mask = Read_Stack("../data/fly_neuron2/mask.tif");
  Stack *trace_mask = Read_Stack("../data/fly_neuron2/trace_mask.tif");
  Stack_And(mask, trace_mask, mask);

  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  ow->conn = 8;
  Stack_Grow_Object_S(seeds, mask, ow);
  Write_Stack("../data/test.tif", seeds);
  Write_Stack("../data/test2.tif", mask);
#endif

#if 0
  Stack *stack = Read_Stack("../data/rn003/cross_15_0/mask2.tif");
  int seed = Stack_Util_Offset(51, 50, 51, stack->width, stack->height,
			       stack->depth);
  Stack *code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);
  Zero_Stack(code);
  Stack_Label_Object_Level_N(stack, NULL, seed, 1, 2, code, 10, 26);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/pollen_bw.tif");
  printf("%d\n", Stack_Label_Objects_Ns(stack, NULL, 1, 2, 3, 26));

  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == 3) {
      printf("%d\n", i);
    }
  }

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Object_3d *obj = Make_Object_3d(10, 26);
  Print_Object_3d(obj);
  obj = Make_Object_3d(8, 26);
  Print_Object_3d(obj);
  obj = Make_Object_3d(sizeof(Voxel_t), 26);
  Print_Object_3d(obj);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/pollen_bw.tif");
  Object_3d_List *objs = Stack_Find_Object_N(stack, NULL, 1, 0, 26);
  Print_Object_3d_List_Compact(objs);
  while (objs != NULL) {
    Object_3d *obj = objs->data;
    Print_Object_3d(obj);
    objs = objs->next;
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/pollen_bw.tif");
  /*
  int seed = Stack_Util_Offset(222, 61, 42, stack->width, stack->height,
			       stack->depth);
  */

  printf("%g\n", Stack_Sum(stack));

  Voxel_t seed;
  seed[0] = 228;
  seed[1] = 337;
  seed[2] = 43;

  Object_3d *obj = Stack_Grow_Object_N(stack, NULL, 1, seed, 26);
  Print_Object_3d_Info(obj);
  
  Stack_Draw_Object_Bw(stack, obj, 0);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/pollen_bw.tif");
  Object_3d_List *objs = Stack_Find_Object_N(stack, NULL, 1, 10000, 26);
  Print_Object_3d_List_Compact(objs);
#endif

#if 0
  Object_3d *obj = Make_Object_3d(2, 6);
  Object_3d_Set_Voxel(obj, 0, 1, 2, 3);
  Object_3d_Set_Voxel(obj, 1, 2, 3, 4);
  Object_3d_Csv_Fprint(obj, stdout);
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/benchmark/binary/2d/disk_n1.tif");
  int seed[10];
  seed[0] = Stack_Util_Offset(30, 31, 0, Stack_Width(stack), 
      Stack_Height(stack), Stack_Depth(stack));
  int nseed = 1;

  Stack *code= Stack_Level_Code(stack, NULL, NULL, seed, nseed);

  Write_Stack("../data/test.tif", code);

  int *hist = Stack_Hist(code);
  Print_Int_Histogram(hist);
#endif

#if 0
  tic();
  Stack *stack = Read_Stack_U("../data/flyem/segmentation/assignments/assignment_2/mask.tif");
  ptoc();

  size_t offset;
  size_t volume = Stack_Voxel_Number(stack);
  //color_t s1 = {3, 222, 0};
  //color_t s2 = {199, 209, 0};
  //color_t s2 = {181, 229, 0};

  //color_t s1 = {245, 71, 0};
  //color_t s2 = {14, 197, 0};

  //color_t s1 = {159, 174, 0};
  //color_t s2 = {172, 9, 1};
  
  //color_t s1 = {179, 235, 0};
  //color_t s2 = {56, 233, 0};
  
  color_t s1 = {151, 67, 0};
  //color_t s2 = {168, 35, 0};
  color_t s2 = { 244, 64, 0};

  Int_Arraylist *seed_array[2];
  seed_array[0] = Make_Int_Arraylist(0, 1);
  seed_array[1] = Make_Int_Arraylist(0, 1);

  color_t *arrayc = (color_t*) stack->array;

  int neighbor_offset[26];
  int is_in_bound[26];

  Stack_Neighbor_Offset(26, Stack_Width(stack), 
      Stack_Height(stack), neighbor_offset);

  Stack *objStack[2];
  objStack[0] = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
      Stack_Depth(stack));
  Zero_Stack(objStack[0]);

  objStack[1] = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
      Stack_Depth(stack));
  Zero_Stack(objStack[1]);

  BOOL is_seed[2] = {FALSE, FALSE};
  for (offset = 0; offset < volume; ++offset) {
    is_seed[0] = FALSE;
    is_seed[1] = FALSE;
    if (SAME_COLOR(s1, arrayc[offset])) {
      objStack[0]->array[offset] = 1;
      int nnbr = Stack_Neighbor_Bound_Test_I(26, Stack_Width(stack),
          Stack_Height(stack), Stack_Depth(stack), offset, is_in_bound);

      for (int i = 0; i < 26; ++i) {
        if (nnbr == 26 || is_in_bound[i]) {
          if (SAME_COLOR(s2, arrayc[offset + neighbor_offset[i]])) {
            is_seed[0] = TRUE;
            break;
          }
        }
      }
    } else if (SAME_COLOR(s2, arrayc[offset])) {
      objStack[1]->array[offset] = 1;
      int nnbr = Stack_Neighbor_Bound_Test_I(26, Stack_Width(stack),
          Stack_Height(stack), Stack_Depth(stack), offset, is_in_bound);

      for (int i = 0; i < 26; ++i) {
        if (nnbr == 26 || is_in_bound[i]) {
          if (SAME_COLOR(s1, arrayc[offset + neighbor_offset[i]])) {
            is_seed[1] = TRUE;
            break;
          }
        }
      }
    } 

    
    for (int i = 0; i < 2; i++) {
      if (is_seed[i] == TRUE) {
        Int_Arraylist_Add(seed_array[i], offset);
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    Stack *code= Stack_Level_Code(objStack[i], NULL, NULL, seed_array[i]->array, 
        seed_array[i]->length);

    Write_Stack("../data/test.tif", code);

    int *hist = Stack_Hist(code);
    Kill_Stack(code);

    Print_Int_Histogram(hist);
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/rice_label.tif");
  Translate_Stack(stack, GREY16, 1);
  
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Default_Objlabel_Workspace(ow);
  Graph *graph = Stack_Label_Field_Neighbor_Graph(stack, 10, ow);

  Print_Graph(graph);
#endif

#if 0
  Stack *stack = Make_Stack(GREY16, 3, 3, 3);
  Zero_Stack(stack);
  Set_Stack_Pixel(stack, 0, 0, 0, 0, 1);
  //Set_Stack_Pixel(stack, 1, 0, 0, 0, 2);
  Set_Stack_Pixel(stack, 2, 0, 0, 0, 3);
  Set_Stack_Pixel(stack, 2, 2, 2, 0, 2);
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Default_Objlabel_Workspace(ow);
  Graph *graph = Stack_Label_Field_Neighbor_Graph(stack, 10, ow);

  Print_Graph(graph);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/Lee_Lab/pipeline/0027/SeparationResultUnmapped.nsp", 2);
  Print_Neurons(neu);

  Stack *stack = Neuron_To_Stack(neu, NULL);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/Lee_Lab/pipeline/0027/SeparationResultUnmapped.nsp", 2);
  Print_Neurons(neu);

  int width, height, depth;
  Region_Stack_Size(neu->neuron[0], &width, &height, &depth);

  printf("%d x %d x %d\n", width, height, depth);

  Stack *stack = Make_Stack(GREY16, width, height, depth);

  int i;
  for (i = 0; i < 16; ++i) {
    Object_3d *obj = Region_To_Object_3d(neu->neuron[i]);
    Stack_Draw_Object_Bw(stack, obj, i+1);

    Kill_Object_3d(obj);
  }

  Stack *out = stack;
    //Stack *out = Stack_Bound_Crop(stack, 1);

  Write_Stack("../data/test.tif", out);

  /*
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Default_Objlabel_Workspace(ow);
  Graph *graph = Stack_Label_Field_Neighbor_Graph(out, 100, ow);

  Print_Graph(graph);
  */

  Mc_Stack mstack = Mc_Stack_View_Stack(stack);
  Rgb_Color color = Region_Average_Color(neu->neuron[3], &mstack);
  Print_Rgb_Color(&color);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/Lee_Lab/pipeline/0027/SeparationResultUnmapped.nsp", 2);
  Print_Neurons(neu);

  Mc_Stack *mstack = Read_Mc_Stack("/Users/zhaot/Work/neutube/neurolabi/data/neurosep/Lee_Lab/13C01_BLM00027_A1.v3dpbd", -1);
  mstack->nchannel = 2;

  Stack *canvas = 
    Make_Stack(COLOR, mstack->width, mstack->height, mstack->depth);
  Zero_Stack(canvas);

  int i;
  for (i = 0; i < neu->numneu; ++i) {
    Rgb_Color color = Region_Average_Color(neu->neuron[i], mstack);
    Print_Rgb_Color(&color);
    double h, s, v;
    Rgb_Color_To_Hsv(&color, &h, &s, &v);
    Set_Color_Hsv(&color, h, s, 1.0);

    Region_Draw_Stack(neu->neuron[i], canvas, color.r, color.g, color.b);
  }

  Write_Stack_U("../data/test.raw", canvas, NULL);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);

  Object_3d *obj = Region_To_Object_3d(neu->neuron[23]);
  Print_Object_3d(obj);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);
  //Print_Neurons(neu);

  Mc_Stack *mstack = Read_Mc_Stack("../data/neurosep/aljosha/pipeline/7513/ConsolidatedSignal.v3dpbd", -1);
  Mc_Stack_Flip_Y(mstack, mstack);

  /*
  Stack *canvas = 
    Make_Stack(COLOR, mstack->width, mstack->height, mstack->depth);
  Zero_Stack(canvas);
  */
  Mc_Stack *canvas = 
    Make_Mc_Stack(mstack->kind, mstack->width, mstack->height, mstack->depth,
        mstack->nchannel);
  Mc_Stack_Set_Zero(canvas);

  Stack *mask = Make_Stack(GREY, mstack->width, mstack->height,
      mstack->depth);
  Zero_Stack(mask);

  int i;
  int list[] = {16, 107, 142, 215};
  for (i = 0; i < sizeof(list) / sizeof(int); ++i) {
    mylib_Region *neuron = neu->neuron[list[i]];
    Rgb_Color color = Region_Average_Color(neuron, mstack);
    printf("Size: %lu\n", (unsigned long) Region_Size(neuron));
    Print_Rgb_Color(&color);
    double h, s, v;
    Rgb_Color_To_Hsv(&color, &h, &s, &v);
    Set_Color_Hsv(&color, h, s, 0.2 * (i + 1));

    Region_Label_Stack(neuron, 1, mask);
    //Region_Draw_Stack(neu->neuron[list[i]], canvas, color.r, color.g, color.b);
  }

  Mc_Stack_Mask(mstack, mask, canvas);

  Mc_Stack_Flip_Y(canvas, canvas);
  //Write_Stack_U("../data/test.raw", canvas, NULL);
  Write_Mc_Stack("../data/test.raw", canvas, NULL);
#endif

#if 0
  FILE *fp = fopen("../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", "rb");

  mylib_Prolog *prolog = Read_Prolog(fp, 3, 1, NULL);
  Print_Prolog(prolog);
  fclose(fp);
  /*
  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);
      */
  //Print_Neurons(neu);
  FILE *output = fopen("../data/test.nsp", "w");
  Write_Prolog(prolog, output);
  fclose(output);

  Kill_Prolog(prolog);

  fp = fopen("../data/test.nsp", "rb");

  prolog = Read_Prolog(fp, 3, 1, NULL);
  Print_Prolog(prolog);
  fclose(fp);
#endif

#if 0
  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);
  Print_Neurons(neu);

  int width, height, depth;
  Region_Stack_Size(neu->neuron[0], &width, &height, &depth);

  printf("%d x %d x %d\n", width, height, depth);


  Mc_Stack *mstack = Read_Mc_Stack(
      "../data/neurosep/aljosha/pipeline/7513/ConsolidatedSignal.v3dpbd", -1);
  Mc_Stack_Flip_Y(mstack, mstack);

  Stack *canvas = 
    Make_Stack(COLOR, mstack->width, mstack->height, mstack->depth);
  Zero_Stack(canvas);

  int i;
  Rgb_Color all_color[neu->numneu];
  for (i = 0; i < neu->numneu; ++i) {
    Rgb_Color color = Region_Average_Color(neu->neuron[i], mstack);
    all_color[i] = color;
    Print_Rgb_Color(&color);
    double h, s, v;
    Rgb_Color_To_Hsv(&color, &h, &s, &v);
    Set_Color_Hsv(&color, h, s, 1.0);

    Region_Draw_Stack(neu->neuron[i], canvas, color.r, color.g, color.b);
  }

  Stack *stack = Make_Stack(GREY16, width, height, depth);

  for (i = 0; i < neu->numneu; ++i) {
    Object_3d *obj = Region_To_Object_3d(neu->neuron[i]);
    Stack_Draw_Object_Bw(stack, obj, i+1);

    Kill_Object_3d(obj);
  }

  Stack *out = stack;

  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Default_Objlabel_Workspace(ow);
  Graph *graph = Stack_Label_Field_Neighbor_Graph(out, 10, ow);

  Print_Graph(graph);

  int nedge = 0;
  int *edge_index = (int*) malloc(sizeof(int) * graph->nedge);

  for (i = 0; i < graph->nedge; ++i) {
    int v1 = graph->edges[i][0] - 1;
    int v2 = graph->edges[i][1] - 1;
    if (Rgb_Color_Hue_Diff(all_color + v1, all_color + v2) < 0.05) {
      printf("%d - %d: (%u, %u, %u), (%u, %u, %u)\n", v1, v2, 
          all_color[v1].r, all_color[v1].g, all_color[v1].b,
          all_color[v2].r, all_color[v2].g, all_color[v2].b);
    } else {
      edge_index[nedge++] = i;
    }
  }

  Graph_Remove_Edge_List(graph, edge_index, nedge);
  Print_Graph(graph);

  Write_Graph("../data/test.gr", graph);
#endif

#if 0
  int i;
  Graph *graph = Read_Graph("../data/test.gr");
  Print_Graph(graph);

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 
      GRAPH_EDGE_NODE(graph, 0, 0));
  Print_Graph(subgraph);

  /* This is necessary to extract subgraph one by one */
  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);

  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == 0) {
      subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
      Print_Graph(subgraph);
      break;
    }
  }

  Graph_Workspace *gw2 = New_Graph_Workspace();
  Arrayqueue array = Graph_Traverse_B(subgraph, GRAPH_EDGE_NODE(subgraph, 0, 0), gw2);
  Print_Arrayqueue(&array);
  int *index = (int*) malloc(sizeof(int) * graph->nvertex);
  int nvertex = Arrayqueue_To_Array(&array, index);

  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);
  mylib_Region **merge_region = 
    (mylib_Region**) malloc(sizeof(mylib_Region*) * graph->nvertex);

  for (i = 0; i < nvertex; ++i) {
    merge_region[i] = neu->neuron[index[i] - 1];
  }
  Region_Merge_M(merge_region[0], merge_region + 1, nvertex - 1);
  printf("merged size: %d\n", (int) Region_Size(merge_region[0]));
  
  Mc_Stack *mstack = Read_Mc_Stack("../data/neurosep/aljosha/pipeline/7513/ConsolidatedSignal.v3dpbd", -1);
  Mc_Stack_Flip_Y(mstack, mstack);

  Mc_Stack *canvas = 
    Make_Mc_Stack(mstack->kind, mstack->width, mstack->height, mstack->depth,
        mstack->nchannel);
  Mc_Stack_Set_Zero(canvas);

  Stack *mask = Make_Stack(GREY, mstack->width, mstack->height,
      mstack->depth);
  Zero_Stack(mask);

  mylib_Region *neuron = merge_region[0];
  Region_Label_Stack(neuron, 1, mask);
  Mc_Stack_Mask(mstack, mask, canvas);

  Mc_Stack_Flip_Y(canvas, canvas);
  Write_Mc_Stack("../data/test.raw", canvas, NULL);

#endif

#if 0
  Graph *graph = Read_Graph("../data/test.gr");
  Print_Graph(graph);

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 
      GRAPH_EDGE_NODE(graph, 0, 0));
  Print_Graph(subgraph);

  Graph_Workspace *gw2 = New_Graph_Workspace();
  Arrayqueue array = Graph_Traverse_B(subgraph, GRAPH_EDGE_NODE(graph, 0, 0), gw2);
  Print_Arrayqueue(&array);

  Neurons *neu = Read_Neurons(
      "../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);
  Mc_Stack *mstack = Read_Mc_Stack("../data/neurosep/aljosha/pipeline/7513/ConsolidatedSignal.v3dpbd", -1);
  Mc_Stack_Flip_Y(mstack, mstack);

  /*
  Mc_Stack *out = Region_Mip(neu->neuron[29], mstack);

  Mc_Stack_Flip_Y(out, out);
  Write_Mc_Stack("../data/test.tif", out, NULL);
*/
  int *index = (int*) malloc(sizeof(int) * graph->nvertex);
  int nvertex = Arrayqueue_To_Array(&array, index);

  mylib_Region **merge_region = 
    (mylib_Region**) malloc(sizeof(mylib_Region*) * graph->nvertex);

  int i;
  for (i = 0; i < nvertex; ++i) {
    merge_region[i] = neu->neuron[index[i] - 1];
  }

  for (i = 0; i < nvertex; ++i) {
    Rgb_Color color = Region_Average_Color(merge_region[i], mstack);
    printf("%d: ", index[i] - 1);
    Print_Rgb_Color(&color);

    double h, s, v;
    Rgb_Color_To_Hsv(&color, &h, &s, &v);
    printf("Hue: %g\n", h);
  }
#endif

#if 0
  int numchan = Infer_Neuron_File_Channel_Number("../data/test.nsp");
  printf("channel number: %d\n", numchan);
#endif

#if 0
  Neurons *neu = Read_Neurons("../data/test.nsp", 3);
  //neu->numneu = 1;
  Print_Neurons(neu);
#endif

#if 0
  //Neurons *neu = Read_Neurons("../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", 3);

  FILE *fp = Guarded_Fopen("../data/neurosep/aljosha/pipeline/7513/SeparationResultUnmapped.nsp", "rb", "Read_Neurons");

  int channel_number = 3;
  mylib_Prolog *prolog = Read_Prolog(fp, channel_number, 1, NULL);

  Neurons *neu = read_neurons(fp, channel_number);

  fclose(fp);

  Region_Merge(neu->neuron[0], neu->neuron[1]);
  //Print_Region(neu->neuron[0]);
  neu->numneu = 1;
  Write_Neurons("../data/test.nsp", neu, prolog, channel_number);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/rice_bw.tif");
  Translate_Stack(stack, GREY16, 1);
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Stack_Label_Object_W(stack, 25, 1, 2, ow);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/rice_bw.tif");
  //Translate_Stack(stack, GREY16, 1);
  Stack_Label_Objects_N(stack, NULL, 1, 255, 8);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/rice_bw.tif");
  Translate_Stack(stack, GREY16, 1);
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Stack_Label_Large_Objects_W(stack, 1, 254, 5, ow);
  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
