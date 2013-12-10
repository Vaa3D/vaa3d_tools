/**@file diadem_d1.c
 * @author Ting Zhao
 * @date 20-Feb-2010
 */

#include <stdio.h>
#include <string.h>
#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_stack_io.h"
#include "tz_fimage_lib.h"
#include "tz_string.h"
#include "tz_stack_utils.h"
#include "tz_stack_watershed.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_stack_neighborhood.h"
#include "tz_error.h"
#include "tz_stack.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_color.h"

/*
static const uint8 Color_Map[][3] = { 
  {0, 0, 0},
  {0, 224, 64}, {32, 64, 128}, {64, 64, 0}, {64, 128, 64},
  {96, 64, 128}, {128, 0, 0}, {128, 200, 64}, {160, 128, 128},
  {192, 0, 0}, {192, 160, 64}, {224, 64, 128}, {224, 224, 192}};
*/
static int load_align(const char *filepath, char ***stack_file, int ***offset)
{
  FILE *fp = fopen(filepath, "r");
  
  String_Workspace *sw = New_String_Workspace();
  
  char *line = NULL;
  
  int nstack = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      nstack++;
    }
  }

  if (nstack > 0) {
    GUARDED_MALLOC_ARRAY(*stack_file, nstack, char*);
    GUARDED_MALLOC_ARRAY(*offset, nstack, int*);

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while ((line = Read_Line(fp, sw)) != NULL) {
      if (Is_Space(line) == FALSE) {
	GUARDED_MALLOC_ARRAY((*stack_file)[i], strlen(line)+1, char);
	strcpy((*stack_file)[i], line);
	strrmspc((*stack_file)[i]);
	strsplit((*stack_file)[i], '(', 1);
	int n;
	
	line = strsplit(line, '(', 1);
	(*offset)[i] = String_To_Integer_Array(line, NULL, &n);
	
	i++;
      }
    }
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return nstack;
}
static void parse_align(int * const *offset, int n, int *corner) 
{
  int i;
  int j;
  for (j = 0; j < 3; j++) {
    corner[j] = offset[0][j];
    corner[j+3] = offset[0][3+j] + offset[0][j] - 1;
  }

  for (j = 0; j < 3; j++) {
    for (i = 1; i < n; i++) {
      if (corner[j] > offset[i][j]) {
	corner[j] = offset[i][j];
      }

      if (corner[3+j] < offset[i][3+j] + offset[i][j] - 1) {
	corner[3+j] = offset[i][3+j] + offset[i][j] - 1;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-root] [-grow <string>] [-nw]",
    "[-root_tile <string>]", "[-root_file <string>]",
    "[-align <string> <string> <string>] -data_id <int>", 
    "[-first_tile_offset <int> <int>] [-flag]", 
    "[-o <string>]", NULL};

  Process_Arguments(argc, argv, Spec, 1);

  int data_id = Get_Int_Arg("-data_id");
  
  if (Is_Arg_Matched("-align")) {
    char *tag1 = Get_String_Arg("-align", 1);
    char *tag2 = Get_String_Arg("-align", 2);
    char *tag3 = Get_String_Arg("-align", 3);

    char filepath1[100];
    char filepath2[100];
    char filepath3[100];

    sprintf(filepath1, "../data/diadem_d%d_%s_pos.txt", data_id, tag1);
    sprintf(filepath2, "../data/diadem_d%d_%s_pos.txt", data_id, tag2);
    sprintf(filepath3, "../data/diadem_d%d_%s_pos.txt", data_id, tag3);

    int **offset1, **offset2, **offset3;
    char **stack_file1, **stack_file2, **stack_file3;
    int n1 = load_align(filepath1, &stack_file1, &offset1);
    int n2 = load_align(filepath2, &stack_file2, &offset2);
    load_align(filepath3, &stack_file3, &offset3);

    int size1[3], size2[3];
    int corner1[6], corner2[6];

    parse_align(offset1, n1, corner1);
    parse_align(offset2, n2, corner2);

    int i; 
    for (i = 0; i < 3; i++) {
      size1[i] = corner1[i+3] - corner1[i] + 1;
      size2[i] = corner2[i+3] - corner2[i] + 1;
    }

    int offset[3];
    int *x1 = NULL;
    int *x2 = NULL;
    for (i = 0; i < n1; i++) {
      if (strcmp(stack_file1[i], stack_file3[0]) == 0) {
	x1 = offset1[i];
	break;
      }
    }
    for (i = 0; i < n2; i++) {
      if (strcmp(stack_file2[i], stack_file3[1]) == 0) {
	x2 = offset2[i];
	break;
      }
    }

    if ((x1 == NULL) || (x2 ==NULL)) {
      PRINT_EXCEPTION("No connection", "Invalid alignment found.");
    }

    for (i = 0; i < 3; i++) {
      offset[i] = corner2[i] - corner1[i] + x1[i] - x2[i] + offset3[1][i];
    }

    char filepath[500];
    sprintf(filepath, "../data/diadem_d%d_%s__%s_pos.txt", data_id, tag1, tag2);
    
    FILE *fp = fopen(filepath, "w");
    sprintf(filepath1, "../data/diadem_d%d_%s.xml", data_id, tag1);
    if (!fexist(filepath1)) {
      sprintf(filepath1, "../data/diadem_d%d_%s.tif", data_id, tag1);      
    }
    fprintf(fp, "%s (0, 0, 0) (%d, %d, %d)\n", filepath1,
	    size1[0], size1[1], size1[2]);
    sprintf(filepath2, "../data/diadem_d%d_%s.xml", data_id, tag2);
    if (!fexist(filepath2)) {
      sprintf(filepath2, "../data/diadem_d%d_%s.tif", data_id, tag2);      
    }
    fprintf(fp, "%s (%d, %d, %d) (%d, %d, %d)\n", filepath2,
	    offset[0], offset[1], offset[2], size2[0], size2[1], size2[2]);
    
    fclose(fp);

    return 0;
  }

  char tmp_filepath[500];
  if (Is_Arg_Matched("-root")) {
    Stack *stack = Read_Stack_U(Get_String_Arg("-root_tile"));

    Print_Stack_Info(stack);

    double sigma[3] = {2.0, 2.0, 1.0};

    FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
    FMatrix *out = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  
    int roots[12][3];
  
    sprintf(tmp_filepath, "../data/diadem_d%d_rootinfo.txt", data_id);
    FILE *fp = fopen(tmp_filepath, "r");

    int n = 0;
    char *line = NULL;
    String_Workspace *sw = New_String_Workspace();
    int array[4];

    while ((line = Read_Line(fp, sw)) != NULL) {
      if (Is_Space(line) == FALSE) {
	int m;
	String_To_Integer_Array(line, array, &m);
	roots[n][0] = array[1] - Get_Int_Arg("-first_tile_offset", 1);
	roots[n][1] = array[2] - Get_Int_Arg("-first_tile_offset", 2);
	roots[n][2] = 0;
	n++;
      }
    }

    fclose(fp);

    int area = stack->width * stack->height;
    int i, k;
    float maxv[12];
    int offset[12];
    for (i = 0; i < 12; i++) {
      offset[i] = Stack_Util_Offset(roots[i][0], roots[i][1], 0,
				    stack->width, stack->height, stack->depth);
      maxv[i] = out->array[offset[i]];
    }

    for (k = 1; k < stack->depth; k++) {
      for (i = 0; i < 12; i++) {
	if (maxv[i] < out->array[offset[i]]) {
	  maxv[i] = out->array[offset[i]];
	  roots[i][2] = k;
	}
	offset[i] += area;
      }
    }


    sprintf(tmp_filepath, "../data/diadem_d%d_root_z.swc", data_id);
    FILE *fp2 = fopen(tmp_filepath, "w");
    sprintf(tmp_filepath, "../data/diadem_d%d_root_z.txt", data_id);
    FILE *fp3 = fopen(tmp_filepath, "w");
    for (i = 0; i < 12; i++) {
      fprintf(fp2, "%d 3 %d %d %d 3 -1\n", i+1, 
	      roots[i][0], roots[i][1], roots[i][2]);
      fprintf(fp3, "%d %d %d\n", 
	      roots[i][0], roots[i][1], roots[i][2]);
    }
    fclose(fp2);
    fclose(fp3);
  }

  if (Is_Arg_Matched("-grow")) {
    Stack *stack = NULL;
    Stack *mask = NULL;
    char stack_path[100];
    stack_path[0] = '\0';
    char mask_path[500];

    int stack_id = -1;
    if (strcmp("start", Get_String_Arg("-grow")) == 0) {
      if (Is_Arg_Matched("-root_tile")) {
	strcpy(stack_path, Get_String_Arg("-root_tile"));
      } else {
	strcpy(stack_path, "../data/diadem_d1_147.xml");
      }

      stack_id = String_Last_Integer(stack_path);
      /*
      strcpy(mask_path, stack_path);
      strsplit(mask_path, '.', -1);
      */
      sprintf(mask_path, "%s/%03d.tif", Get_String_Arg("-o"), stack_id);

      if (Is_Arg_Matched("-nw")) {
	if (fexist(mask_path)) {
	  printf("%s is already there.\n", mask_path);
	  return 0;
	}
      }

      if (!fexist(stack_path)) {
	fprintf(stderr, "Cannot find %s\n", stack_path);
	return 1;
      }
      printf("Processing %s\n", stack_path);
      stack = Read_Stack_U(stack_path);
      mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
      Zero_Stack(mask);

      int conn = 26;
      const int *dx = Stack_Neighbor_X_Offset(conn);
      const int *dy = Stack_Neighbor_X_Offset(conn);
      const int *dz = Stack_Neighbor_X_Offset(conn);
  
      int seed[3];
      String_Workspace *sw = New_String_Workspace();
      char *line = NULL;
      FILE *fp = NULL;
      BOOL one_indexed = FALSE;
      
      if (Is_Arg_Matched("-root_file")) {
	fp = fopen(Get_String_Arg("-root_file"), "r");
	if (fhasext(Get_String_Arg("-root_file"), "marker")) {
	  one_indexed = TRUE;
	}
      } else {
	fp = fopen("../data/diadem_d1_root_z.txt", "r");
      }
      int k = 1;

      /* label seeds */
      while ((line = Read_Line(fp, sw)) != NULL) {
	int n;
	String_To_Integer_Array(line, seed, &n);
	if (one_indexed) {
	  seed[0]--;
	  seed[1]--;
	  seed[2]--;
	}
	double maxv = -1;
	if (n >= 3) {
	  maxv = Get_Stack_Pixel(stack, seed[0], seed[1], seed[2], 0);
	  int i;
	  for (i = 0; i < conn; i++) {
	    if (maxv < Get_Stack_Pixel(stack, seed[0] + dx[i], seed[1] + dy[i], 
				       seed[2] + dz[i], 0)) {
	      maxv = Get_Stack_Pixel(stack, seed[0] + dx[i], seed[1] + dy[i], 
				     seed[2] + dz[i], 0);
	    }
	  }
	  
	  Set_Stack_Pixel(mask, seed[0], seed[1], seed[2], 0, k);
	  for (i = 0; i < conn; i++) {
	    Set_Stack_Pixel(mask, seed[0] + dx[i], seed[1] + dy[i], 
			    seed[2] + dz[i], 0, k);	
	  }
	  k++;
	}
      }
      fclose(fp);
      Kill_String_Workspace(sw);
    } else {
      char **filepath;
      int **final_offset = NULL;
      int nstack = load_align(Get_String_Arg("-grow", 0), &filepath, 
			      &final_offset);

      stack_id = String_Last_Integer(filepath[1]);
      /*
      strcpy(mask_path, filepath[1]);
      strsplit(mask_path, '.', -1);
      sprintf(mask_path, "%s_label.tif", mask_path);
      */
      sprintf(mask_path, "%s/%03d.tif", Get_String_Arg("-o"), stack_id);

      if (Is_Arg_Matched("-nw")) {
	if (fexist(mask_path)) {
	  printf("%s is already there.\n", mask_path);
	  return 0;
	}
      }

      if (nstack == 2) {
	strsplit(filepath[0], '.', -1);
	sprintf(mask_path, "%s_label.tif", filepath[0]);
	/*
	  Stack *mask2 = Read_Stack(mask_path);
	  Stack_Watershed_Workspace *ws2 = Make_Stack_Watershed_Workspace(mask2);
	  Stack *mask = Stack_Region_Border_Shrink(mask2, ws2);
	  Kill_Stack(mask2);
	  Kill_Stack_Watershed_Workspace(ws2);
	*/
	if (!fexist(mask_path)) {
	  fprintf(stderr, "Cannot find %s\n", mask_path);
	  return 1;
	}

	Stack *mask2 = Read_Stack(mask_path);

	strcpy(stack_path, filepath[1]);
	printf("Processing %s\n", stack_path);

	if (!fexist(stack_path)) {
	  fprintf(stderr, "Cannot find %s\n", stack_path);
	  return 1;
	}

	stack = Read_Stack_U(stack_path);

	mask = Crop_Stack(mask2, final_offset[1][0], final_offset[1][1], 
			  final_offset[1][2], Stack_Width(stack), 
			  Stack_Height(stack), Stack_Depth(stack), NULL);

	Kill_Stack(mask2);
      }
    }

    if (stack == NULL) {
      PRINT_EXCEPTION("No data found", "No stack is loaded for growing.");
      return 1;
    }

    tic();
    /* can be faster by combining the two steps */
    double thre1 = Stack_Find_Threshold_Locmax2(stack, 0, 65535, 1.0);
    printf("first level threshold: %g\n", thre1);
    double thre2 = Stack_Find_Threshold_Locmax2(stack, thre1, 65535, 1.0);
    double thre = (thre1 + thre2) / 2.0;
    printf("threshold: %g\n", thre);

    //Stack_Running_Median(stack, 0, stack);
    //Stack_Running_Median(stack, 1, stack);
    Stack *median_stack = Stack_Median_Filter_N(stack, 8, NULL);
    Kill_Stack(stack);
    stack = median_stack;

    Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
    ws->mask = mask;
    
    Filter_3d *filter = Gaussian_Filter_3d(2.0, 2.0, 1.5);
    Stack *filtered_stack  = Filter_Stack(stack, filter);
    Stack_Watershed_Zgap_Barrier(filtered_stack, ws->mask);
    Stack_Running_Max(ws->mask, 0, ws->mask);
    Stack_Running_Max(ws->mask, 1, ws->mask);
    //Write_Stack("../data/test.tif", ws->mask);
    
    Kill_Stack(filtered_stack);
    filtered_stack = NULL;

    
    FMatrix *dm = Mexihat_3D1_F(2.0, NULL, 2);
    //FMatrix *dm = Mexihat_3D_F(2.0, NULL);
    FMatrix_Negative(dm);

    filtered_stack = Filter_Stack(stack, dm);
  
    Stack_Threshold_Common(filtered_stack, 0, 65535);
    Stack_Binarize(filtered_stack);
    Translate_Stack(filtered_stack, GREY, 1);
    
    {
      int i, j, k;
      int offset = 0;
      uint16 *array = (uint16*) stack->array;
      for (k = 0; k < stack->depth; k++) {
	for (j = 0; j < stack->height; j++) {
	  for (i = 0; i < stack->width; i++) {
	    if (filtered_stack != NULL) {
	      if (filtered_stack->array[offset] == 1) {
		ws->mask->array[offset] = STACK_WATERSHED_BARRIER;
	      }
	    }
	    if (array[offset] > thre/* - k / 2*/) {
	      array[offset] += k * 2;
	    }
	    offset++;
#ifdef _DEBUG_2
	    if (ws->mask->array[offset-1] == STACK_WATERSHED_BARRIER) {
	      array[offset-1] = 0;
	    }
#endif
	  }
	}
      }
#ifdef _DEBUG_2
      Write_Stack("../data/test.tif", stack);
      return 1;
#endif
    }

    //Write_Stack("../data/test.tif", ws->mask);
    Kill_Stack(filtered_stack);
    
    Stack_Watershed_Infer_Parameter(stack, ws);
    printf("Start level: %d\n", ws->start_level);
    if (ws->min_level < thre) {
      ws->min_level = thre;
    }
    ws->conn = 6;

    /*
    double weights[26] = {0.5, 0.5, 1.0, 1.0, 0.2, 0.2, 0.75, 0.75, 0.75, 0.75,
			  0.35, 0.35, 0.35, 0.35, 0.6, 0.6, 0.6, 0.6, 
			  0.45, 0.45, 0.45, 0.45,
			  0.45, 0.45, 0.45, 0.45};
    */

    /*
    double weights[26] = {0.0, 0.8, 0.1, 1, 0.0, 0.0, 
			  0.0, 0.9, 0.0, 0.0, 
			  0.0, 0.3, 0.3, 0.0, 0.0, 0.3, 0.3, 0.0,
			  0.0, 0.3, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0};
    */

    double weights[26] = {0.5, 0.5, 1.0, 1.0, 0.2, 0.2, 0.75, 0.75, 0.75, 0.75,
			  0.35, 0.35, 0.35, 0.35, 0.6, 0.6, 0.6, 0.6, 
			  0.45, 0.45, 0.45, 0.45,
			  0.45, 0.45, 0.45, 0.45};

    ws->weights = weights;
    ws->weights = NULL;
    
    if (ws->weights != NULL) {
      ws->min_level /= 3;
    }

    //ws->min_level = 1300;
        
    Stack *out = Stack_Watershed(stack, ws);

    /*
    strcpy(mask_path, stack_path);
    strsplit(mask_path, '.', -1);
    sprintf(mask_path, "%s_label.tif", mask_path);
    */
    sprintf(mask_path, "%s/%03d.tif", Get_String_Arg("-o"), stack_id);
    
    if (Is_Arg_Matched("-nw")) {
      if (fexist(mask_path)) {
	printf("%s is already there.\n", mask_path);
	return 0;
      }
    }
    ptoc();

    Write_Stack(mask_path, out);
    printf("%s saved.\n", mask_path);
    char tmp_cmd[500];
    sprintf(tmp_cmd, "touch %s.done", mask_path);
    system(tmp_cmd);
    

    /*
    Stack *out2 = Upsample_Stack(out, 0, 0, 3, NULL);
    Kill_Stack(out);

    Stack_Binarize(out2);
    Stack *stack3 = Stack_Bwdist_L_U16(out2, NULL, 0);
    Kill_Stack(out2);

    Stack *stack2 = Downsample_Stack(stack3, 0, 0, 3);
    Kill_Stack(stack3);
    
    Stack_Watershed_Infer_Parameter(stack2, ws);
    ws->conn = 6;
    ws->min_level = 1;
    out = Stack_Watershed(stack2, ws);
    */

#if 0    
    Translate_Stack(out, COLOR, 1);
    size_t nvoxel = Stack_Voxel_Number(out);
    size_t i;
    color_t *arrayc = (color_t*) out->array;
    Rgb_Color color;
    for (i = 0; i < nvoxel; i++) {
      if (arrayc[i][0] > 0) {
	Set_Color_Jet(&color, arrayc[i][0] * 5);
	arrayc[i][2] = color.b;
	arrayc[i][1] = color.g;
	arrayc[i][0] = color.r;
      }
    }

    strcpy(mask_path, stack_path);
    strsplit(mask_path, '.', -1);
    sprintf(mask_path, "%s_paint.tif", mask_path);
    Write_Stack(mask_path, out);
#endif
  }

  return 0;
}
