/**@file diadem_d1_watershed.c
 * @author Ting Zhao
 * @date 23-Aug-2010
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
#include "tz_int_histogram.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>",
    "-seed <string> -range <int> <int>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int start = Get_Int_Arg("-range", 1);
  int end = Get_Int_Arg("-range", 2);

  int stack_id;
  printf("Processing from %d to %d\n", start, end);
  for (stack_id = start; stack_id <= end; stack_id++) {
    tic();
    char stack_path[500];
    char seed_path[500];
    char mask_path[500];

    sprintf(stack_path, "%s/%03d.xml", Get_String_Arg("input"), stack_id);
    sprintf(seed_path, "%s/%03d.seeds.marker", Get_String_Arg("-seed"), stack_id);
    sprintf(mask_path, "%s/%03d.tif", Get_String_Arg("-o"), stack_id);

    if (!fexist(seed_path)) {
      fprintf(stderr, "seed_path does not exist.\n");
      return 1;
    }

    printf("Processing %s\n", stack_path);
    Stack *stack = Read_Stack_U(stack_path);
    Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
	Stack_Depth(stack));
    Zero_Stack(mask);

    int conn = 26;
    const int *dx = Stack_Neighbor_X_Offset(conn);
    const int *dy = Stack_Neighbor_X_Offset(conn);
    const int *dz = Stack_Neighbor_X_Offset(conn);
      
    FILE *fp = fopen(seed_path, "r");

    BOOL one_indexed = FALSE;
    if (fhasext(seed_path, "marker")) {
      one_indexed = TRUE;
    }

    int k = 1;

    /* label seeds */
    String_Workspace *sw = New_String_Workspace();
    char *line = NULL;
    int seed[10];
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

    /*
    double thre1 = Stack_Find_Threshold_Locmax2(stack, 0, 65535, 1.0);
    printf("first level threshold: %g\n", thre1);
    double thre2 = Stack_Find_Threshold_Locmax2(stack, thre1, 65535, 1.0);
    double thre = (thre1 + thre2) / 2.0;
    printf("threshold: %g\n", thre);
*/

    Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
    int *hist = Stack_Hist_M(stack, locmax);
    Kill_Stack(locmax);

    double alpha = 1.0;
    int thre1 = Int_Histogram_Triangle_Threshold2(hist, 0, 65535, alpha);
    printf("first level threshold: %d\n", thre1);
    int thre2 = Int_Histogram_Triangle_Threshold2(hist, thre1, 65535, alpha);
    free(hist);
    int thre = (thre1 + thre2) / 2;
    printf("threshold: %d\n", thre);

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
    
    Kill_Stack(filtered_stack);
    Kill_FMatrix(filter);
    filtered_stack = NULL;

    
    FMatrix *dm = Mexihat_3D1_F(2.0, NULL, 2);
    FMatrix_Negative(dm);

    filtered_stack = Filter_Stack(stack, dm);
    Kill_FMatrix(dm);
  
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
	  }
	}
      }
    }

    Kill_Stack(filtered_stack);
    
    Stack_Watershed_Infer_Parameter(stack, ws);
    printf("Start level: %d\n", ws->start_level);
    if (ws->min_level < thre) {
      ws->min_level = thre;
    }
    ws->conn = 6;
    ws->weights = NULL;
        
    Stack *out = Stack_Watershed(stack, ws);
    Kill_Stack(stack);

    Write_Stack(mask_path, out);
    Kill_Stack(out);
    printf("%s saved.\n", mask_path);
    char tmp_cmd[500];
    sprintf(tmp_cmd, "touch %s.done", mask_path);
    system(tmp_cmd);

    Kill_Stack_Watershed_Workspace(ws);
    ptoc();
  }

  return 0;
}
