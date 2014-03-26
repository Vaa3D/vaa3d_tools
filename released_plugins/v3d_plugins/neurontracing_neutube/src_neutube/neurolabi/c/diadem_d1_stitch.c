/**@file diadem_d1_stitch.c
 * @author Ting Zhao
 * @date 08-Apr-2010
 */

#include <stdlib.h>
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
  static char *Spec[] = {"[-root_id <int>] [<input:string>]",
    "[-a <string>] [-b <string>]", 
    "[-stitch_script] [-exclude <string>] [-tile_number <int>]",
    NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int *excluded = NULL;
  int nexc = 0;
  int *excluded_pair = NULL;
  int nexcpair = 0;
  if (Is_Arg_Matched("-exclude")) {
    String_Workspace *sw = New_String_Workspace();
    FILE *fp = fopen(Get_String_Arg("-exclude"), "r");
    char *line = Read_Line(fp, sw);
    excluded = String_To_Integer_Array(line, NULL, &nexc);
    line = Read_Line(fp, sw);
    if (line != NULL) {
      excluded_pair = String_To_Integer_Array(line, NULL, &nexcpair);
      nexcpair /= 2;
    }
    Kill_String_Workspace(sw);
    fclose(fp);
  }

  int n = 0;
  Graph *graph = Make_Graph(n + 1, n, TRUE);
  char filepath1[100];
  char filepath2[100];
  int i, j;
  Stack *stack1 = NULL;
  FILE *fp = NULL;
  if (Is_Arg_Matched("-stitch_script")) {
    Cuboid_I *boxes = read_tile_array(Get_String_Arg("-a"), &n);
    for (i = 0; i < n; i++) {
      for (j = i + 1; j < n; j++) {      
	BOOL is_excluded = FALSE;
	int k;
	for (k = 0; k < nexc; k++) {
	  if ((i == excluded[k] - 1) || (j == excluded[k] - 1)) {
	    is_excluded = TRUE;
	    break;
	  }
	}
	for (k = 0; k < nexcpair; k++) {
	  if (((i == excluded_pair[k*2]) && (j == excluded_pair[k*2+1])) ||
	      ((j == excluded_pair[k*2]) && (i == excluded_pair[k*2+1]))) {
	    is_excluded = TRUE;
	    break;
	  }
	}
	/*
	if ((i != 103) && (j != 103) && (i != 115) && (j != 115) && (i != 59) &&
	    (j != 59) && !(i == 116 && j == 116)) {
	    */
	if (is_excluded == FALSE) {
	  Cuboid_I_Overlap_Volume(boxes + i, boxes + j);
	  Cuboid_I ibox;
	  Cuboid_I_Intersect(boxes + i, boxes + j, &ibox);
	  int width, height, depth;
	  Cuboid_I_Size(&ibox, &width, &height, &depth);

	  if ((imax2(width, height) > 1024 / 3) && (imin2(width, height) > 0)) {
	    sprintf(filepath1, "%s/stack/%03d.xml", 
		Get_String_Arg("input"), i + 1);
	    sprintf(filepath2, "%s/stack/%03d.xml", 
		Get_String_Arg("input"), j + 1);
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

    Arrayqueue q = Graph_Traverse_B(graph, Get_Int_Arg("-root_id"), gw);

    Print_Arrayqueue(&q);

    int *grown = iarray_malloc(graph->nvertex);
    for (i = 0; i < graph->nvertex; i++) {
      grown[i] = 0;
    }

    int index  = Arrayqueue_Dequeue(&q);
    grown[index] = 1;

    char stitch_p_file[5][500];
    FILE *pfp[5];
    for (i = 0; i < 5; i++) {
      sprintf(stitch_p_file[i], "%s/stitch/stitch_%d.sh", Get_String_Arg("input"), i);
      pfp[i] = fopen(stitch_p_file[i], "w");
    }

    sprintf(filepath1, "%s/stitch/stitch_all.sh", Get_String_Arg("input"));

    fp = GUARDED_FOPEN(filepath1, "w");
    fprintf(fp, "#!/bin/bash\n");

    int count = 0;
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
	    char cmd[500];
	    sprintf(filepath2, "%s/stitch/%03d_%03d_pos.txt",
		Get_String_Arg("input"), index2, index);
	    sprintf(cmd, 
		"%s/stitchstack %s/stack/%03d.xml %s/stack/%03d.xml -o %s", 
		Get_String_Arg("-b"), Get_String_Arg("input"),
		index2, Get_String_Arg("input"), index, filepath2);
	    fprintf(fp, "%s\n", cmd);
	    count++;
	    fprintf(pfp[count%5], "%s\n", cmd);
	    /*
	    if (!fexist(filepath2)) {
	      system(cmd);
	    }
	    */
	    grown[index] = 1;
	    break;
	  }
	}
      }
    }

    fclose(fp);  
    for (i = 0; i < 5; i++) {
      fprintf(pfp[i], "touch %s/stitch/stitch_%d_done\n", 
	  Get_String_Arg("input"), i);
      fclose(pfp[i]);
    }
    return 0;
  }
  
  sprintf(filepath1, "%s/stitch/stitch_all.sh", Get_String_Arg("input"));
  fp = GUARDED_FOPEN(filepath1, "r");

//#define MAX_TILE_INDEX 153
  int tile_number = Get_Int_Arg("-tile_number");
  int max_tile_index = tile_number + 1;

  char *line = NULL;
  String_Workspace *sw = New_String_Workspace();
  int id[2];
  char filepath[100];
  int offset[max_tile_index][3];
  int relative_offset[max_tile_index][3];
  int array[max_tile_index];

  for (i = 0; i < max_tile_index; i++) {
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
    if (String_Ends_With(line, "stitchstack")) {
      String_To_Integer_Array(remain, id, &n);
      id[0] = id[1];
      id[1] = id[3];
      array[id[1]] = id[0];

      sprintf(filepath, "%s/stitch/%03d_%03d_pos.txt", Get_String_Arg("input"),
	      id[0], id[1]);
      if (!fexist(filepath)) {
	fprintf(stderr, "file %s does not exist\n", filepath);
	return 1;
      }
      FILE *fp2 = GUARDED_FOPEN(filepath, "r");
      line = Read_Line(fp2, sw);
      line = Read_Line(fp2, sw);
      int tmpoffset[8];
      String_To_Integer_Array(line, tmpoffset, &n);
      relative_offset[id[1]][0] = tmpoffset[2];
      relative_offset[id[1]][1] = tmpoffset[3];
      relative_offset[id[1]][2] = tmpoffset[4];

      fclose(fp2);
    }
  }
  
  for (i = 1; i < max_tile_index; i++) {
    BOOL is_excluded = FALSE;
    int k;
    for (k = 0; k < nexc; k++) {
      if (i == excluded[k]) {
	is_excluded = TRUE;
	break;
      }
    }
    /*if ((i == 104) || (i == 116) || (i == 60) || (i == 152)) {*/
    if (is_excluded) {
      printf("%d: (0, 0, 10000)\n", i);
    } else {
      int index = i;
      while (index >= 0) {
	offset[i][0] += relative_offset[index][0];
	offset[i][1] += relative_offset[index][1];
	offset[i][2] += relative_offset[index][2];
	index = array[index];
      }
      printf("%d: (%d, %d, %d)\n", i, offset[i][0], offset[i][1], offset[i][2]);
    }
  }

  fclose(fp);

  return 0;
}
