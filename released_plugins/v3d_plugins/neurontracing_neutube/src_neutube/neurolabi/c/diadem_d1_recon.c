/**@file diadem_d1_recon.c
 * @author Ting Zhao
 * @date 20-Mar-2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tz_utilities.h"
#include "tz_constant.h"
#include "tz_swc_cell.h"
#include "tz_geo3d_utils.h"
#include "tz_darray.h"
#include "tz_swc_arraylist.h"
#include "tz_swc.h"
#include "tz_iarray.h"
#include "tz_swc_tree.h"
#include "tz_string.h"
#include "tz_error.h"
#include "tz_stack_tile_i.h"
#include "tz_image_io.h"
#include "tz_u16array.h"
#include "tz_int_histogram.h"
#include "tz_stack_attribute.h"
#include "tz_stack_threshold.h"

#include "private/diadem_d1_recon.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-test]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_11.swc");
  
  double root[3] = {288, 54, 77};
  Swc_Tree *tree2 = Swc_Tree_Pull_R(tree, root);
  Swc_Tree_Node *tn = Swc_Tree_Outmost_Leaf(tree2);

  Swc_Tree_Node_Data(tn)->type = 5;

  Write_Swc_Tree("../data/test.swc", tree2);

  /* Load other trees. */
  Kill_Swc_Tree(tree);
  tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_1.swc");
  
  Swc_Tree *tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_3.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
 
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_5.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_6.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_7.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_9.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_10.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_12.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
 
 /* Look for continuation. */
  Swc_Tree_Node *target = swc_tree_continuation(tn, tree);
  Swc_Tree_Node_Data(target)->d = 10;

  Print_Swc_Tree_Node(target);

  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test2.swc", tree);
  
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_1.swc");
  
  Swc_Tree *tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_3.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_5.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_6.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_7.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_9.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_10.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_11.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_12.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  swc_tree_down_sample(tree, 15.0);
  swc_tree_label_color(tree);
  Swc_Tree_Resort_Id(tree);

  swc_tree_set_up_root(tree);

  Swc_Tree_Remove_Terminal_Branch(tree, 30.0);

  Write_Swc_Tree("../data/test2.swc", tree);

  swc_tree_set_type(tree, 2);

  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Resort_Id(tree);

  Swc_Tree *tree2 = Copy_Swc_Tree(tree);
  int n = Swc_Tree_Iterator_Start(tree2, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);

  Swc_Tree_Node **tn_array2;
  GUARDED_MALLOC_ARRAY(tn_array2, n+1, Swc_Tree_Node*);

  while ((tn = Swc_Tree_Next(tree2)) != NULL) {
    tn_array2[Swc_Tree_Node_Id(tn)] = tn;
  }

  Swc_Tree_Node *best_pair[2];
  
  BOOL stop = FALSE;

  while (stop == FALSE) {
    best_pair[0] = NULL;
    best_pair[1] = NULL;
    double max_conn = 0.0;
    stop = TRUE;

    /* Find matches. */
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Leaf(tn)) {
	coordinate_3d_t vec;
	swc_tree_node_vec(tn, 30.0, vec);
	Coordinate_3d_Unitize(vec);
	if (Swc_Tree_Node_Data(tn)->x == 124 && Swc_Tree_Node_Data(tn)->y == 22) {
	    printf("here.\n");
	  }
	if (vec[1] > 0.0) {
	  //Swc_Tree_Node_Data(tn)->type = 5;	  
	  Swc_Tree_Node *match = swc_tree_continuation(tn, tree2);

	  tn = tn_array2[Swc_Tree_Node_Id(tn)];

	  if (swc_tree_node_sharing_regular_root(match, tn) == FALSE) {
	    if (match != NULL) {
	      double conn = swc_tree_node_connect_strength(tn, match);
	      if (conn > max_conn) {
		max_conn = conn;
		best_pair[0] = tn;
		best_pair[1] = match;
	      }
	    
	      /*
		if (Swc_Tree_Node_Dist(tn, match) < 200.0) {
		Swc_Tree_Node_Data(match)->type = 5;
		Swc_Tree_Node_Set_Root(match);
		Swc_Tree_Node_Set_Parent(match, tn);
		}
	      */
	    }
	  }
	}
      }
    }

    if (best_pair[0] != NULL) {
      /*
      Print_Swc_Tree_Node(best_pair[0]);
      printf("%g\n", Swc_Tree_Node_Dist(best_pair[0], best_pair[1]));
      */
      if (Swc_Tree_Node_Dist(best_pair[0], best_pair[1]) < 200.0) {
	Swc_Tree_Node_Data(best_pair[1])->type = 5;
	Swc_Tree_Node_Set_Root(best_pair[1]);
	Swc_Tree_Node_Set_Parent(best_pair[1], best_pair[0]);
	Kill_Swc_Tree(tree);
	tree = Copy_Swc_Tree(tree2);
	stop = FALSE;
      }
    }
  }

  //swc_tree_break_cross(tree2);

  while (swc_tree_break_cross(tree2));
  Swc_Tree_Remove_Terminal_Branch(tree2, 30.0);

  swc_tree_label_color(tree2);
  Swc_Tree_Resort_Id(tree2);
  Write_Swc_Tree("../data/test.swc", tree2);

#endif

#if 0
  Stack_Tile_I_Arraylist *tiles = NULL;

  Stack_Tile_I *tile1 = Stack_Tile_Arraylist_At(tiles, 0);
  Stack_Tile_I *tile2 = Stack_Tile_Arraylist_At(tiles, 1);

  Cuboid_I r1, r2;
  Stack_Tile_I_Boundbox(tile1, &r1);
  Stack_Tile_I_Boundbox(tile2, &r2);
  
  Cuboid_I br1, br2;
  Stack_Tile_I_Overlap(tile1, tile2, &br1);
  Stack_Tile_I_Overlap(tile1, tile2, &br2);

  Cuboid_I_Translate_Back(br1, Stack_Tile_Offset(tile1));
  Cuboid_I_Translate_Back(br2, Stack_Tile_Offset(tile2));
  
  swc_tree_seed_mask(tree, stack, offset);
 
  Stack *out = Stack_Watershed(stack, ws);
  
  Swc_Tree *tree = Stack_Label_To_Swc(stack);
  
  swc_tree_reset_root(tree, seed_region);
  swc_tree_reconnect(tree);
#endif

#if 0
  int n;
  Cuboid_I *boxes = read_tile_array("../data/diadem_d1_align.txt", &n);
  Graph *graph = Make_Graph(n, n, TRUE);
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = i; j < n; j++) {
      if ((i != 103) && (j != 103) && (i != 115) && (j != 115) && 
	  !(i == 116 && j == 117)) {
	if (i != j) {
	  int s = Cuboid_I_Overlap_Volume(boxes + i, boxes + j);
	  if (s > 1000) {
	    /* recalculate weights */
	    Graph_Add_Weighted_Edge(graph, i + 1, j + 1, 1000.0 / s);
	  }
	}
      }
    }
  }

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_To_Mst2(graph, gw);

  Print_Graph(graph);
  Graph_To_Dot_File(graph, "../data/test.dot");

  FILE *fp = GUARDED_FOPEN("../shell/diadem_d1_stitch_all.sh", "w");
  fprintf(fp, "#!/bin/bash\n");
  for (i = 0; i < graph->nedge; i++) {
    fprintf(fp, "./diadem_d1_stitch %03d %03d\n", 
	    graph->edges[i][0], graph->edges[i][1]);
  }
  fclose(fp);
#endif

#if 0
  int n;
  Cuboid_I *boxes = read_tile_array("../data/diadem_d1_align.txt", &n);
  Graph *graph = Make_Graph(n + 1, n, TRUE);
  char filepath1[100];
  char filepath2[100];
  int i, j;
  Stack *stack1 = NULL;
  for (i = 0; i < n; i++) {
    for (j = i + 1; j < n; j++) {
      /*
      if ((i != 70) || (j != 82)) {
	continue;
      }
      */
      
      if ((i != 103) && (j != 103) && (i != 115) && (j != 115) && (i != 59) &&
	  (j != 59) && !(i == 116 && j == 117)) {
	int s = Cuboid_I_Overlap_Volume(boxes + i, boxes + j);
	Cuboid_I ibox;
	Cuboid_I_Intersect(boxes + i, boxes + j, &ibox);
	int width, height, depth;
	Cuboid_I_Size(&ibox, &width, &height, &depth);

	if ((imax2(width, height) > 1024 / 3) && (imin2(width, height) > 0)) {
	//if (s > 10000) {
	  sprintf(filepath1, "../data/diadem_d1_%03d.xml", i + 1);
	  sprintf(filepath2, "../data/diadem_d1_%03d.xml", j + 1);
	  if (stack1 == NULL) {
	    stack1 = Read_Stack_U(filepath1);
	  }
	  Stack *stack2 = Read_Stack_U(filepath2);
	  
	  Stack *substack1= Crop_Stack(stack1, ibox.cb[0] - boxes[i].cb[0], 
				       ibox.cb[1] - boxes[i].cb[1], 0,
				       width, height, stack1->depth, NULL);
	  Stack *substack2 = Crop_Stack(stack2, ibox.cb[0] - boxes[j].cb[0], 
					ibox.cb[1] - boxes[j].cb[1], 0,
					width, height, stack2->depth, NULL);
	  
	  Image *img1 = Proj_Stack_Zmax(substack1);	
	  Image *img2 = Proj_Stack_Zmax(substack2);
	  double w = u16array_corrcoef((uint16_t*) img1->array, 
				       (uint16_t*) img2->array, 
				       img1->width * img1->height);
	  
	  Kill_Stack(stack2);
	  Kill_Stack(substack1);
	  Kill_Stack(substack2);
	  Kill_Image(img1);
	  Kill_Image(img2);
	  printf("%d, %d : %g\n", i + 1, j + 1, w);
	  Graph_Add_Weighted_Edge(graph, i + 1, j + 1, 1000.0 / (w + 1.0));
	}
      }
      if (stack1 != NULL) {
	Kill_Stack(stack1);
	stack1 = NULL;
      }
    }
  }

  Graph_Workspace *gw = New_Graph_Workspace();
  Graph_To_Mst2(graph, gw);

  Graph_To_Dot_File(graph, "../data/test.dot");

  Arrayqueue q = Graph_Traverse_B(graph, 147, gw);
  
  Print_Arrayqueue(&q);

  int *grown = iarray_malloc(graph->nvertex);
  for (i = 0; i < graph->nvertex; i++) {
    grown[i] = 0;
  }
  
  int index  = Arrayqueue_Dequeue(&q);
  grown[index] = 1;

  FILE *fp = GUARDED_FOPEN("../shell/diadem_d1_grow_all.sh", "w");
  fprintf(fp, "#!/bin/bash\n");

  while ((index = Arrayqueue_Dequeue(&q)) > 0) {
    for (i = 0; i < graph->nedge; i++) {
      int index2 = -1;
      if (index == graph->edges[i][0]) {
	index2 = graph->edges[i][1];
      } else if (index == graph->edges[i][1]) {
	index2 = graph->edges[i][0];
      }

      if (index2 > 0) {
	if (grown[index2] == 1) {
	  fprintf(fp, "./diadem_d1_stitch %03d %03d\n", index2, index);
	  fprintf(fp, 
		  "../c/bin/diadem_d1 -grow ../data/diadem_d1_%03d_%03d_pos.txt\n", 
		  index2, index);
	  grown[index] = 1;
	  break;
	}
      }
    }
  }

  fclose(fp);  
#endif

#if 1
  FILE *fp = GUARDED_FOPEN("../shell/diadem_d1_grow_all.sh", "r");

  //Graph *graph = Make_Graph(0, 0, FALSE);
  //Graph_Set_Directed(TRUE);
#define MAX_TILE_NUMBER 152

  char *line = NULL;
  String_Workspace *sw = New_String_Workspace();
  int id[2];
  int n;
  char filepath[100];
  int offset[MAX_TILE_NUMBER][3];
  int relative_offset[MAX_TILE_NUMBER][3];
  int array[MAX_TILE_NUMBER];
  int i;
  for (i = 0; i < MAX_TILE_NUMBER; i++) {
    array[i] = -1;
    offset[i][0] = 0;
    offset[i][1] = 0;
    offset[i][2] = 0;
    relative_offset[i][0] = 0;
    relative_offset[i][1] = 0;
    relative_offset[i][2] = 0;
  }

  while ((line = Read_Line(fp, sw)) != NULL) {
    char *remain = strsplit(line, ' ', 1);
    if (strcmp(line, "./diadem_d1_stitch") == 0) {
      String_To_Integer_Array(remain, id, &n);
      //Graph_Add_Edge(id[0], id[1]);
      array[id[1]] = id[0];

      if (id[1] == 1) {
	printf("stop here\n");
      }

      sprintf(filepath, 
	      "/Users/zhaot/Work/neurolabi/data/diadem_d1_%03d_%03d_pos.txt", 
	      id[0], id[1]);
      FILE *fp2 = GUARDED_FOPEN(filepath, "r");
      line = Read_Line(fp2, sw);
      line = Read_Line(fp2, sw);
      int tmpoffset[8];
      String_To_Integer_Array(line, tmpoffset, &n);
      relative_offset[id[1]][0] = tmpoffset[2];
      relative_offset[id[1]][1] = tmpoffset[3];
      relative_offset[id[1]][2] = tmpoffset[4];

      if (relative_offset[1][0] == 150) {
	printf("stop here\n");
      }

      fclose(fp2);
    }
  }
  
  for (i = 1; i < MAX_TILE_NUMBER; i++) {
    int index = i;
    while (index >= 0) {
      //printf("%d->", index);
      offset[i][0] += relative_offset[index][0];
      offset[i][1] += relative_offset[index][1];
      offset[i][2] += relative_offset[index][2];
      index = array[index];
    }
    //printf("end (%d, %d, %d)\n", offset[i][0], offset[i][1], offset[i][2]);
    printf("%d: (%d, %d, %d)\n", i, offset[i][0], offset[i][1], offset[i][2]);
  }

  fclose(fp);
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_d1_047_label.tif");
  stack_label_collapse(stack);
  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
