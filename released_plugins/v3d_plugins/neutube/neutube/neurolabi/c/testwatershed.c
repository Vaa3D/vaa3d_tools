/* testwatershed.c
 *
 * 04-Jun-2008
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tz_string.h"
#include "tz_error.h"
#include "tz_stack_watershed.h"
#include "tz_stack_attribute.h"
#include "tz_arrayview.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_string.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_relation.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_stat.h"
#include "tz_object_3d_linked_list.h"
#include "tz_objdetect.h"
#include "tz_stack_draw.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    /* Example test */
    Stack *stack = Make_Stack(GREY, 7, 7, 1);
    One_Stack(stack);
    Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
    ws->conn = 26;
    ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			  Stack_Depth(stack));
    Zero_Stack(ws->mask);

    /* set seeds */
    Set_Stack_Pixel(ws->mask, 1, 1, 0, 0, 1.0);
    Set_Stack_Pixel(ws->mask, 1, 5, 0, 0, 2.0);
    Set_Stack_Pixel(ws->mask, 3, 3, 0, 0, 3.0);
    Set_Stack_Pixel(ws->mask, 5, 1, 0, 0, 4.0);
    Set_Stack_Pixel(ws->mask, 5, 5, 0, 0, 5.0);

    /* set stack values */
    Set_Stack_Pixel(stack, 1, 1, 0, 0, 3.0);
    Set_Stack_Pixel(stack, 1, 5, 0, 0, 3.0);
    Set_Stack_Pixel(stack, 3, 3, 0, 0, 3.0);
    Set_Stack_Pixel(stack, 5, 1, 0, 0, 3.0);
    Set_Stack_Pixel(stack, 5, 5, 0, 0, 3.0);
    
    Stack *out = Stack_Watershed(stack, ws);

    //Write_Stack("../data/test/watershed/golden/watershed1.tif", out);

    char *golden_file = "../data/test/watershed/golden/watershed1.tif";
    if (fexist(golden_file)) {
      Stack *golden = Read_Stack(golden_file);
      if (Stack_Identical(out, golden) == FALSE) {
	Print_Stack_Value(stack);
	Print_Stack_Value(out);
	Print_Stack_Value(golden);
	PRINT_EXCEPTION("Bug?", "Conflict with golden.");
	return 1;
      }

      Kill_Stack(stack);
      Kill_Stack(out);
      Kill_Stack(golden);
      Kill_Stack_Watershed_Workspace(ws);
    } else {
      printf("%s cannot be found.\n", golden_file);
    }

    char *test_file = "../data/benchmark/rice_label.tif";
    if (fexist(test_file)) {
      stack = Read_Stack_U(test_file);

      ws = Make_Stack_Watershed_Workspace(stack);
      ws->mask = Copy_Stack(stack);
      ws->conn = 26;
      One_Stack(stack);

      out = Stack_Watershed(stack, ws);

      //Write_Stack("../data/test/watershed/golden/watershed2.tif", out);

      Stack *golden = Read_Stack("../data/test/watershed/golden/watershed2.tif");
      if (Stack_Identical(out, golden) == FALSE) {
	PRINT_EXCEPTION("Bug?", "Conflict with golden.");
	return 1;
      }
    } else {
      printf("%s cannot be found.\n", test_file);
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  /* Initialize */
  Watershed_3d *watershed = New_Watershed_3d();
  Watershed_3d_Workspace *ws = New_Watershed_3d_Workspace();
  ws->conn = 26;

  /* Initialize stack */
  Stack *stack = Read_Stack("../data/fly_neuron/dist.tif");
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  uint16 *array16 = (uint16 *)stack->array;
  for (i = 0; i < nvoxel; i++) {
    array16[i] = 0xFFFF - array16[i];
  }
  
  /* Add mask to ignore dark voxels  */
  ws->mask = Copy_Stack(stack); 
  

  //Translate_Stack(stack, GREY, 1);

  //stack->array = stack->array + stack->width * stack->height * 100;
  //stack->depth = 1;
  // ws->mask = Copy_Stack(stack);
  //Stack_Binarize(ws->mask);
  // ws->mask = NULL;

  //Write_Stack("../data/dist.tif", stack);


  //Stack *stack2 = Copy_Stack(stack);


#  if 0
  Build_3D_Watershed(stack, watershed, ws);
  Write_Stack("../data/test.tif", watershed->labels);
#  endif

#  if 0
  
  Image_View iv = Image_View_Stack(stack2);
  Watershed_Test *watershed2 = Build_2D_Watershed_Test(&(iv.image), 0);
  Write_Image("../data/test2.tif", watershed2->labels);
#  endif
#endif

#if 0
  Image *image = Read_Image("../data/Ring15.tif_Sub1200_Original_inv.tif");
  Watershed_2D *ws = Build_2D_Watershed(image, 0);
  Image *result = Color_Watersheds(ws, image);
  Write_Image("../data/test.tif", result);
  Kill_Image(image);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 7, 7, 1);
  One_Stack(stack);
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->conn = 26;
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
  Zero_Stack(ws->mask);
  Set_Stack_Pixel(ws->mask, 1, 1, 0, 0, 1.0);
  Set_Stack_Pixel(ws->mask, 1, 5, 0, 0, 2.0);
  Set_Stack_Pixel(ws->mask, 3, 3, 0, 0, 3.0);
  Set_Stack_Pixel(ws->mask, 5, 1, 0, 0, 4.0);
  Set_Stack_Pixel(ws->mask, 5, 5, 0, 0, 5.0);

  Set_Stack_Pixel(stack, 1, 1, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 1, 5, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 3, 3, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 5, 1, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 5, 5, 0, 0, 3.0);
  
  Stack *out = Stack_Watershed(stack, ws);

  Print_Stack_Value(out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 3, 3, 1);
  One_Stack(stack);
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->conn = 26;
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
  Zero_Stack(ws->mask);

  Set_Stack_Pixel(ws->mask, 0, 0, 0, 0, 1.0);
  Set_Stack_Pixel(ws->mask, 2, 2, 0, 0, 2.0);

  Set_Stack_Pixel(stack, 0, 0, 0, 0, 5.0);
  Set_Stack_Pixel(stack, 2, 2, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 0, 2, 0, 0, 2.0);
  //Set_Stack_Pixel(stack, 1, 1, 0, 0, 2.0);
  Set_Stack_Pixel(stack, 1, 2, 0, 0, 2.0);
  Set_Stack_Pixel(stack, 2, 0, 0, 0, 2.0);
  Set_Stack_Pixel(stack, 2, 1, 0, 0, 2.0);

  Print_Stack_Value(stack);

  ws->start_level = 6;

  Stack *out = Stack_Watershed(stack, ws);

  Print_Stack_Value(out);
#endif
 
#if 0
  Stack *stack = Read_Stack_U("../data/benchmark/rice_label.tif");

  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->mask = Copy_Stack(stack);
  ws->conn = 26;
  One_Stack(stack);

  tic();
  Stack *out = Stack_Watershed(stack, ws);
  printf("%lld\n", toc());

  Write_Stack("../data/test.tif", out); 
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/diadem_d1_147.xml");
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
  Zero_Stack(ws->mask);
  ws->conn = 26;

  const int *dx = Stack_Neighbor_X_Offset(ws->conn);
  const int *dy = Stack_Neighbor_X_Offset(ws->conn);
  const int *dz = Stack_Neighbor_X_Offset(ws->conn);
  
  int seed[3];
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  FILE *fp = fopen("../data/diadem_d1_root_z.txt", "r");
  int k = 1;
    
  while ((line = Read_Line(fp, sw)) != NULL) {
    int n;
    String_To_Integer_Array(line, seed, &n);
    double maxv = -1;
    if (n == 3) {
      maxv = Get_Stack_Pixel(stack, seed[0], seed[1], seed[2], 0);
      printf("%g\n", maxv);
      int i;
      for (i = 0; i < ws->conn; i++) {
	if (maxv < Get_Stack_Pixel(stack, seed[0] + dx[i], seed[1] + dy[i], 
				   seed[2] + dz[i], 0)) {
	  maxv = Get_Stack_Pixel(stack, seed[0] + dx[i], seed[1] + dy[i], 
				 seed[2] + dz[i], 0);
	}
      }

      //Set_Stack_Pixel(stack, seed[0], seed[1], seed[2], 0, maxv);
      Set_Stack_Pixel(ws->mask, seed[0], seed[1], seed[2], 0, k);
      for (i = 0; i < ws->conn; i++) {
	//Set_Stack_Pixel(stack, seed[0] + dx[i], seed[1] + dy[i], 
	//		seed[2] + dz[i], 0, maxv);
	Set_Stack_Pixel(ws->mask, seed[0] + dx[i], seed[1] + dy[i], 
			seed[2] + dz[i], 0, k);	
      }
      k++;
    }
  }
  fclose(fp);
  Kill_String_Workspace(sw);
  
  /*
  Set_Stack_Pixel(ws->mask, 19, 605, 112, 0, 1.0);
  Set_Stack_Pixel(ws->mask, 28, 565, 112, 0, 1.0);
  */
  Stack_Watershed_Infer_Parameter(stack, ws);

  tic();
  Stack *out = Stack_Watershed(stack, ws);
  printf("%lld\n", toc());

  Write_Stack("../data/diadem_d1_147_label.tif", out);

  static const uint8 Color_Map[][3] = { 
    {0, 0, 0},
    {0, 224, 64}, {32, 64, 128}, {64, 64, 0}, {64, 128, 64},
    {96, 64, 128}, {64, 0, 0}, {128, 200, 64}, {160, 128, 128},
    {192, 0, 0}, {192, 160, 64}, {224, 64, 128}, {224, 224, 192}};
  
  Translate_Stack(out, COLOR, 1);
  size_t nvoxel = Stack_Voxel_Number(out);
  size_t i;
  color_t *arrayc = (color_t*) out->array;
  for (i = 0; i < nvoxel; i++) {
    arrayc[i][2] = Color_Map[arrayc[i][0]][2];
    arrayc[i][1] = Color_Map[arrayc[i][0]][1];
    arrayc[i][0] = Color_Map[arrayc[i][0]][0];
  }

  Write_Stack("../data/diadem_d1_147_paint.tif", out);
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/diadem_d1_146.xml");
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  Stack *mask = Read_Stack("../data/test3.tif");
  ws->mask = Crop_Stack(mask, 252, -937, 2, 1024, 1024, 63, NULL);
  ws->conn = 26;

  Stack_Watershed_Infer_Parameter(stack, ws);

  tic();
  Stack *out = Stack_Watershed(stack, ws);
  printf("%lld\n", toc());
  
  Write_Stack("../data/test2.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 7, 7, 1);
  One_Stack(stack);
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->conn = 26;
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
  Zero_Stack(ws->mask);
  Set_Stack_Pixel(ws->mask, 1, 1, 0, 0, 1.0);
  Set_Stack_Pixel(ws->mask, 1, 5, 0, 0, 2.0);
  Set_Stack_Pixel(ws->mask, 3, 3, 0, 0, 3.0);
  Set_Stack_Pixel(ws->mask, 5, 1, 0, 0, 4.0);
  Set_Stack_Pixel(ws->mask, 5, 5, 0, 0, 5.0);

  Set_Stack_Pixel(stack, 1, 1, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 1, 5, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 3, 3, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 5, 1, 0, 0, 3.0);
  Set_Stack_Pixel(stack, 5, 5, 0, 0, 3.0);
  
  Stack *out = Stack_Watershed(stack, ws);
  Print_Stack_Value(out);

  Stack *out2 = Stack_Region_Border_Shrink(out, ws);
  
  Print_Stack_Value(out2);

#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_d1_013_label.tif");
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  //ws->conn = 26;

  Stack *out = Stack_Region_Border_Shrink(stack, ws);
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 1, 1, 19);
  One_Stack(stack);
  
  stack->array[3] = 5;
  stack->array[4] = 5;
  stack->array[7] = 5;
  stack->array[8] = 5;
  stack->array[10] = 4;
  stack->array[11] = 5;
  stack->array[12] = 5;
  stack->array[13] = 4;
  stack->array[16] = 5;
  stack->array[17] = 5;

  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->conn = 26;
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			Stack_Depth(stack));
  Zero_Stack(ws->mask);

  
  Print_Stack_Value(stack);
  Stack_Watershed_Zgap_Barrier(stack, ws->mask);
  Print_Stack_Value(ws->mask);
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_d1_047_label.tif");
  Stack_Binarize(stack);

  Stack *stack2 = Stack_Bwdist_L_U16(stack, NULL, 0);
  Write_Stack("../data/test.tif", stack2);  
#endif

#if 0
  char stack_path[100];
  char mask_path[100];

  strcpy(stack_path, "../data/diadem_d1_064.xml");

  strcpy(mask_path, stack_path);
  strsplit(mask_path, '.', -1);
  sprintf(mask_path, "%s_label.tif", mask_path);

  if (!fexist(stack_path)) {
    fprintf(stderr, "Cannot find %s\n", stack_path);
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
  
  int seed[3];
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  FILE *fp = fopen("../data/064.seeds.txt", "r");
  int k = 1;

  /* label seeds */
  while ((line = Read_Line(fp, sw)) != NULL) {
    int n;
    String_To_Integer_Array(line, seed, &n);
    double maxv = -1;
    if (n == 3) {
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

  Stack_Running_Median(stack, 0, stack);
  Stack_Running_Median(stack, 1, stack);
    
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
	  array[offset++] += k * 2;
	}
      }
    }
  }

  Kill_Stack(filtered_stack);
  
  Stack_Watershed_Infer_Parameter(stack, ws);
  ws->conn = 6;

  double weights[26] = {0.5, 0.5, 1.0, 1.0, 0.2, 0.2, 0.75, 0.75, 0.75, 0.75,
			0.35, 0.35, 0.35, 0.35, 0.6, 0.6, 0.6, 0.6, 
			0.45, 0.45, 0.45, 0.45,
			0.45, 0.45, 0.45, 0.45};

  ws->weights = weights;
  ws->weights = NULL;
  
  if (ws->weights != NULL) {
    ws->min_level /= 3;
  }
    
  Stack_Running_Median(stack, 0, stack);
  Stack_Running_Median(stack, 1, stack);
  
  Stack *out = Stack_Watershed(stack, ws);
  
  strcpy(mask_path, stack_path);
  strsplit(mask_path, '.', -1);
  sprintf(mask_path, "%s_label.tif", mask_path);
  
  Write_Stack("../data/test.tif", out);

#endif

#if 0
  Stack *stack = Read_Stack("../data/test/soma2.tif");

  int thre = Stack_Find_Threshold_A(stack, THRESHOLD_LOCMAX_TRIANGLE);
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 0.5);
  Stack *out = Filter_Stack(stack, filter);
  stack = Copy_Stack(out);
  Stack_Threshold_Binarize(out, thre);
  Stack *out2 = Stack_Bwdist_L_U16P(out, NULL, 0);
  int nvoxel = Stack_Voxel_Number(out);
  uint16_t *dist_array = (uint16_t*) out2->array;
  int i;
  for (i = 0; i < nvoxel; i++) {
    dist_array[i] = sqrt(dist_array[i]);
  }

  Write_Stack("../data/labmeeting13/distp.tif", out2);
#endif

#if 0
  //Stack *stack = Read_Stack("../data/test/soma2.tif");

  Stack *stack = Read_Stack("/Users/zhaot/Data/jinny/slice7_2to34ds_soma_c2.tif");
  int thre = Stack_Find_Threshold_A(stack, THRESHOLD_LOCMAX_TRIANGLE);
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 0.5);
  Stack *out = Filter_Stack(stack, filter);
  stack = Copy_Stack(out);
  Stack_Threshold_Binarize(out, thre);
  Stack *out2 = Stack_Bwdist_L_U16P(out, NULL, 0);

  /*
  out = Stack_Locmax_Region(out2, 26);
  */
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
      Stack_Depth(stack));
  Zero_Stack(ws->mask);
  size_t nvoxel = Stack_Voxel_Number(stack);
  size_t offset;
  uint16_t *dist_array = (uint16_t*) out2->array;
  for (offset = 0; offset < nvoxel; offset++) {
    if (/*(out->array[offset] == 1) && */(dist_array[offset] > 1000)) {
      ws->mask->array[offset] = 1;
    }
  }

  //Objlabel_Workspace *ow = New_Objlabel_Workspace();
  //Stack_Label_Large_Objects_W(ws->mask, 1, 2, 10, ow);
  //Stack_Label_Objects_N(ws->mask, NULL, 1, 2, 26);
  Object_3d_List *objs = Stack_Find_Object(ws->mask, 1, 100);
  Zero_Stack(ws->mask);
  Stack_Draw_Objects_Bw(ws->mask, objs, -255);
  printf("%g\n", Stack_Max(ws->mask, NULL));

  /*
  Write_Stack("../data/test.tif", ws->mask);
  return 1;
*/

  ws->min_level = thre;
  ws->start_level = 65535;

  Stack *out3 = Stack_Watershed(stack, ws);
  /*
  Write_Stack("../data/labmeeting13/watershed.tif", out3);
  return 1;
*/
  for (offset = 0; offset < nvoxel; offset++) {
    if (dist_array[offset] < 300) {
      out3->array[offset] = 0;
    }
  }

  int nregion = Stack_Max(out3, NULL);

  Kill_Stack(out2);
  Stack *filtered = Copy_Stack(stack);
  Kill_Stack(stack);
  
  ws->conn = 8;
  stack = Stack_Region_Border_Shrink(out3, ws);
  out2 = Stack_Region_Expand(stack, 4, 30, NULL);

  for (offset = 0; offset < nvoxel; offset++) {
    if (out->array[offset] == 0) {
      out2->array[offset] = 0;
    }
  }

  Write_Stack("../data/test2.tif", out2);

  Kill_Stack(stack);
  //stack = Read_Stack("../data/test/soma2.tif");
  stack = Read_Stack("/Users/zhaot/Data/jinny/slice7_2to34ds_soma_c2.tif");
  Translate_Stack(stack, COLOR, 1);
  int i;
  double h = 0.0;
  double s = 1.0;
  for (i = 0; i < nregion; i++) {
    Stack_Label_Color_L(stack, out2, i+1, h+=0.35, s, filtered);
    /*
    Rgb_Color color;
    Set_Color_Jet(&color, i*3);
    Stack_Label_Level(stack, out2, i+1, color);
    */
  }
  Write_Stack("../data/test.tif", stack);
#endif

#if 1
  Stack *stack = Read_Stack("../data/leaktest/leak3.tif");
  Stack *distmap = Stack_Bwdist_L_U16P(stack, NULL, 0);

  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->mask = Copy_Stack(distmap);
  Stack_Threshold_Binarize(ws->mask, 10);
  Translate_Stack(ws->mask, GREY, 1);
  Object_3d_List *objs = Stack_Find_Object(ws->mask, 1, 100);
  Zero_Stack(ws->mask);
  Stack_Draw_Objects_Bw(ws->mask, objs, -255);

  ws->min_level = 1;
  ws->start_level = 65535;
  Stack *out3 = Stack_Watershed(distmap, ws);
  Write_Stack("../data/test.tif", out3);

#endif
  return 0;
}
