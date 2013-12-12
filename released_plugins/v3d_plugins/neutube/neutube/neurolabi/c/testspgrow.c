/* file testspgrow.c
 * @author Ting Zhao
 * @date 25-Dec-2009
 */

#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_sp_grow.h"
#include "tz_stack_lib.h"
#include "tz_stack_utils.h"
#include "tz_locseg_chain.h"
#include "tz_stack_attribute.h"
#include "tz_stack_threshold.h"
#include "tz_int_histogram.h"
#include "tz_stack_relation.h"
#include "tz_image_io.h"
#include "tz_string.h"
#include "tz_stack_bwmorph.h"
#include "tz_u16array.h"
#include "tz_stack_stat.h"
#include "tz_stack_neighborhood.h"

#include "private/testspgrow.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
    Stack *stack = Make_Stack(GREY, 5, 5, 1);
    Zero_Stack(stack);
    size_t seeds[] = {0, 1, 2};
    size_t targets[] = {2, 3, 4};
    sgw->conn = 8;
    Int_Arraylist *path = Stack_Sp_Grow(stack, seeds, 1, targets, 1, sgw);

    if ((path->array[0] != 0) || (path->array[1] != 1) || 
	(sgw->dist[1] != 1.0)) {
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 2, targets, 1, sgw);

    if ((path->array[0] != 1) || (path->array[1] != 2) || 
	(sgw->dist[2] != 1.0)) {
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 2, targets, 2, sgw);

    if ((path->array[0] != 1) || (path->array[1] != 2) || 
	(sgw->dist[2] != 1.0)) {
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    seeds[0] = 6;
    targets[0] = 7;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 1, targets, 1, sgw);

    if ((path->array[0] != 6) || (path->array[1] != 7) || 
	(sgw->dist[7] != 1.0)) {
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    targets[0] = 8;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 1, targets, 1, sgw);

    if ((path->array[0] != 6) || (path->array[1] != 7) || 
	(path->array[2] != 8) || (sgw->dist[8] != 2.0)) {
      Print_Int_Arraylist(path, "%d");
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    seeds[0] = 0;
    targets[0] = 24;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 1, targets, 1, sgw);

    if ((path->array[0] != 0) || (path->array[1] != 6) || 
	(path->array[2] != 12) || (path->array[3] != 18) || 
	(path->array[4] != 24)) {
      Print_Int_Arraylist(path, "%d");
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    seeds[0] = 6;
    seeds[1] = 11;
    targets[0] = 12;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 2, targets, 1, sgw);

    if ((path->array[0] != 11) || (path->array[1] != 12) ||
	(sgw->dist[1] != 1.0)) {
      Print_Int_Arraylist(path, "%d");
      printf("dist: %g\n", sgw->dist[1]);
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    seeds[0] = 6;
    seeds[1] = 11;
    targets[0] = 8;
    targets[1] = 12;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 2, targets, 2, sgw);

    if ((path->array[0] != 11) || (path->array[1] != 12) ||
	(sgw->dist[1] != 1.0)) {
      Print_Int_Arraylist(path, "%d");
      printf("dist: %g\n", sgw->dist[1]);
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }


    seeds[0] = 6;
    seeds[1] = 7;
    seeds[2] = 8;
    targets[0] = 5;
    targets[1] = 7;
    targets[2] = 10;

    Kill_Int_Arraylist(path);

    path = Stack_Sp_Grow(stack, seeds, 3, targets, 3, sgw);

    if ((path->length != 1) || (path->array[0] != 7) || (sgw->dist[7] != 0.0) ||
	(sgw->value != 0.0)) {
      Print_Int_Arraylist(path, "%d");
      printf("dist: %g\n", sgw->dist[1]);
      PRINT_EXCEPTION("Bug?", "Wrong path.");
      return 1;
    }

    Kill_Stack(stack);
    Kill_Sp_Grow_Workspace(sgw);
    Kill_Int_Arraylist(path);

    if (golden_test() != 0) {
      PRINT_EXCEPTION("Bug?", "Golden test failed.");
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  Stack *stack = Make_Stack(GREY, 5, 5, 1);
  Zero_Stack(stack);
  Stack *mask = Copy_Stack(stack);
  /*
  mask->array[5] = 4;
  mask->array[6] = 4;
  mask->array[10] = 4;
  mask->array[11] = 4;
  mask->array[12] = 4;
  mask->array[15] = 4;
  mask->array[16] = 4;
  mask->array[17] = 4;
  mask->array[20] = 4;
  mask->array[21] = 4;
  mask->array[22] = 4;
  mask->array[23] = 4;
  */

  mask->array[4] = 4;
  mask->array[6] = 4;
  mask->array[7] = 4;
  mask->array[9] = 4;
  mask->array[13] = 4;
  mask->array[17] = 4;
  mask->array[21] = 4;
  mask->array[23] = 4;

  Print_Stack_Value(mask);

  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);

  size_t seeds[] = {0};
  size_t targets[] = {24};
  sgw->conn = 8;
  Int_Arraylist *path = Stack_Sp_Grow(stack, seeds, 1, targets, 0,
				      sgw);

  printf("Dist: %g\n", sgw->value);
  Print_Int_Arraylist(path, "%d");
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  
  tic();
  
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;

  int start[3] = {467, 337, 175};
  int end[3] = {464, 228, 148};

  size_t sources[1];
  size_t targets[1];
  sources[0] = Stack_Util_Offset(start[0], start[1], start[2], stack->width,
				 stack->height, stack->depth);

  targets[0] = Stack_Util_Offset(end[0], end[1], end[2], stack->width,
				 stack->height, stack->depth);
  Int_Arraylist *path = Stack_Sp_Grow(stack, sources, 1, targets, 1,
				      sgw);

  Kill_Sp_Grow_Workspace(sgw);

  printf("Time passed: %lld\n", toc());

#if 0
  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Image_Array ima;
  ima.array = canvas->array;

  int i;
  for (i = 0; i < path->length; i++) {
    int index = path->array[i];
    ima.arrayc[index][0] = 255;
    ima.arrayc[index][1] = 0;
    ima.arrayc[index][2] = 0;
  }

  Write_Stack("../data/test.tif", canvas);  

  Kill_Stack(canvas);
#endif

  Kill_Int_Arraylist(path);
  Kill_Stack(stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain0.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain1.tb");

  Locseg_Chain_Label_G(chain1, mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
  Locseg_Chain_Label_G(chain2, mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);

  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  //sgw->mask = mask->array;
  
  size_t source = Stack_Util_Offset(50, 22, 50, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
  size_t target = Stack_Util_Offset(79, 79, 50, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
				   
  
  Int_Arraylist *path = Stack_Sp_Grow(stack, &source, 1, &target, 1, sgw);

  //Print_Int_Arraylist(path, "%d");

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);  
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/mouse_neuron_single/stack.tif");
  /*
  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  */

  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_S;
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

  //sgw->size = Stack_Voxel_Number(stack);
  //sgw->mask = mask->array;
  
  size_t source = Stack_Util_Offset(12, 115, 22, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
  size_t target = Stack_Util_Offset(69, 455, 66, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
				   
  
  tic();
  Int_Arraylist *path = Stack_Sp_Grow(stack, &source, 1, &target, 1, sgw);
  printf("time passed: %llu\n", toc());

  //Print_Int_Arraylist(path, "%d");

  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);   
#endif

#if 0
  Stack *stack = Read_Stack("../data/10_slices_Oct_15.tif");
  /*
  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  */

  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  //sgw->mask = mask->array;
  
  size_t source = Stack_Util_Offset(1414, 862, 5, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
  size_t target = Stack_Util_Offset(1210, 1322, 5, Stack_Width(stack),
				    Stack_Height(stack), Stack_Depth(stack));
				   
  
  Int_Arraylist *path = Stack_Sp_Grow(stack, &source, 1, &target, 1, sgw);

  //Print_Int_Arraylist(path, "%d");

  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, NULL);

  Write_Stack("../data/test.tif", canvas);   
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/diadem/diadem_e1.tif");

  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  
  
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain24.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain35.tb");
  Locseg_Chain *chain3 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain0.tb");
  Locseg_Chain *chain4 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain14.tb");

  
  /*
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/cross/chain0.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/cross/chain1.tb");
  */
  Locseg_Chain_Label_G(chain1, mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
  Locseg_Chain_Label_G(chain2, mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);
  Locseg_Chain_Label_G(chain3, mask, 1.0, 0, -1, 1.0, 0.0, 0, 4);
  Locseg_Chain_Label_G(chain4, mask, 1.0, 0, -1, 1.0, 0.0, 0, 4);

  int *hist = Stack_Hist(mask);
  Print_Int_Histogram(hist);

  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.3296;
  sgw->resolution[1] = 0.3296;
  sgw->resolution[2] = 1.0;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  double inner = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
					    STACK_FIT_MEAN_SIGNAL);
  double outer = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
					    STACK_FIT_OUTER_SIGNAL);
  sgw->argv[3] = inner * 0.1 + outer * 0.9;
  sgw->argv[4] = (inner - outer) / 4.6 * 1.8;

  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);
  Locseg_Chain_Label(chain3, canvas, 1.0);
  Locseg_Chain_Label(chain4, canvas, 1.0);

  Write_Stack("../data/test/spgrow/tmp/diadem_e1_result.tif", canvas);   

  if (fcmp("../data/test/spgrow/tmp/diadem_e1_result.tif",
	   "../data/test/spgrow/golden/diadem_e1_result.tif") != 0) {
    printf("Conflict with golden standard.\n");
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/diadem_e1/chain12.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/diadem_e1/chain16.tb");

  Locseg_Chain_Label_G(chain1, mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
  Locseg_Chain_Label_G(chain2, mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);
 
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.3296;
  sgw->resolution[1] = 0.3296;
  sgw->resolution[2] = 1.0;
  sgw->sp_option = 1;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);
  /*
  double inner = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
					    STACK_FIT_MEAN_SIGNAL);
  double outer = Locseg_Chain_Average_Score(chain1, stack, 1.0, 
					    STACK_FIT_OUTER_SIGNAL);
  sgw->argv[3] = inner * 0.1 + outer * 0.9;
  sgw->argv[4] = (inner - outer) / 4.6 * 1.8;
  */

  tic();
  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
  printf("time: %llu\n", toc());
  
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);   
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");

  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);
  
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain1.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain0.tb");

  Locseg_Chain_Label_G(chain1, mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
  Locseg_Chain_Label_G(chain2, mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);
 
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  tic();
  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
  printf("time: %llu\n", toc());
  
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);    
#endif

#if 0
  Stack *stack1 = Read_Stack("../data/test/spgrow/tmp/diadem_e1_result.tif");
  Stack *stack2 = Read_Stack("../data/test/spgrow/golden/diadem_e1_result.tif");

  printf("%d\n", Stack_Identical(stack1, stack2));
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/diadem_d1_147.xml");
  
  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
			   Stack_Depth(stack));
  Zero_Stack(mask);

  int area = stack->width * stack->height;
  int i, k;
  int offset = 0;
  for (k = 0; k < stack->depth; k++) {
    for (i = 0; i < stack->width; i++) {
      mask->array[offset++] = SP_GROW_TARGET;
    }
    offset += area - stack->width;
  }

  /* set source */
  String_Workspace *sw = New_String_Workspace();
  char *line;
  int array[7];
  FILE *fp = fopen("../data/diadem_d1_root_z.swc", "r");
  int n;
  int count = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    String_To_Integer_Array(line, array, &n);
    if (n == 7) {
      printf("%d, %d, %d\n", array[2], array[3], array[4]);
      int index = Stack_Util_Offset(array[2], array[3], array[4],
				    stack->width, stack->height, stack->depth);
      if (count == 6) {
	mask->array[index] = SP_GROW_SOURCE;
      }
      count++;
    }
  }
  fclose(fp);
  Kill_String_Workspace(sw);

			
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.0375;
  sgw->resolution[1] = 0.0375;
  sgw->resolution[2] = 0.2;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->sp_option = 1;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  tic();
  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
  printf("time: %llu\n", toc());
  
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);   
  
  Kill_Stack(stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_d1_047_label.tif");
  Stack *mask = Copy_Stack(stack);

  size_t nvoxel = Stack_Voxel_Number(stack);
  size_t i;
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == 7) {
      stack->array[i] = 1;
    } else {
      stack->array[i] = 0;
    }
  }
  
  //Stack_Running_Min(stack, 0, stack);
  
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == 1) {
      mask->array[i] = 0;
    } else {
      mask->array[i] = SP_GROW_BARRIER;
    }
  }
  
  stack = Stack_Bwdist_L_U16(stack, NULL, 0);
  Translate_Stack(stack, GREY, 1);

  Set_Stack_Pixel(mask, 331, 11, 77, 1, SP_GROW_SOURCE);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->argv[3] = 3;
  sgw->argv[4] = 2.0 / 9.2;
  sgw->size = Stack_Voxel_Number(stack);
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->conn = 6;

  tic();
  Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
  printf("time: %llu\n", toc());

  int max_count;
  int idx = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  Int_Arraylist *path = NULL;

  
  if (max_count > 100) {
    path = New_Int_Arraylist();
    int cur_index = sgw->path[idx];
    while (cur_index >= 0) {
      Int_Arraylist_Insert(path, 0, cur_index);
      cur_index = sgw->path[cur_index];
    }

    for (i = 0; i < path->length; i++) {
      mask->array[path->array[i]] = SP_GROW_SOURCE;
    }
  }

  while (1) {
    tic();
    Kill_Int_Arraylist(path);
    Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
    printf("time: %llu\n", toc());
    idx = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
    printf("%d\n", max_count);

    if (max_count < 100) {
      break;
    }

    path = New_Int_Arraylist();
    int cur_index = sgw->path[idx];
    while (cur_index >= 0) {
      Int_Arraylist_Insert(path, 0, cur_index);
      cur_index = sgw->path[cur_index];
    }

    for (i = 0; i < path->length; i++) {
      mask->array[path->array[i]] = SP_GROW_SOURCE;
    }
  }
  
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  
  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);

  Write_Stack("../data/test.tif", canvas);   

  Kill_Stack(stack);
#endif

#if 1
  Stack *stack = Read_Stack("../data/test.tif");
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_I;
  int max_index;
  Stack_Max(stack, &max_index);

  Stack *mask = Copy_Stack(stack);
  Zero_Stack(mask);

  size_t nvoxel = Stack_Voxel_Number(stack);
  size_t i;
  for (i = 0; i < nvoxel; i++) {
    if (Stack_Array_Value(stack, i) == 0.0) {
      mask->array[i] = SP_GROW_BARRIER;
    }
  }
  mask->array[max_index] = SP_GROW_SOURCE;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);

  Int_Arraylist *path = NULL;

  int max_count;
  int cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  path = New_Int_Arraylist();
  //Int_Arraylist_Insert(path, 0, idx);
  //int cur_index = sgw->path[idx];
  while (cur_index >= 0) {
    Int_Arraylist_Insert(path, 0, cur_index);
    int tmp_index = cur_index;
    cur_index = sgw->path[cur_index];
    sgw->path[tmp_index] = -1;
  }

  int neighbor[26];
  int n_nbr = 18;
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  int is_in_bound[26];
  for (i = 1; i < path->length; i++) {
    int center = path->array[i];

    int n_in_bound = Stack_Neighbor_Bound_Test_I(n_nbr, stack->width, 
	stack->height, stack->depth, center, is_in_bound);
    int j;
    if (n_in_bound == n_nbr) {
      for (j = 0; j < n_nbr; j++) {
	sgw->path[center + neighbor[j]] = -1;
      }
    } else {
      for (j = 0; j < n_nbr; j++) {
	if (is_in_bound[j]) {
	  sgw->path[center + neighbor[j]] = -1;
	}
      }
    }
  }

  cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  //cur_index = sgw->path[idx];
  while (cur_index >= 0) {
    Int_Arraylist_Insert(path, 0, cur_index);
    int tmp_index = cur_index;
    cur_index = sgw->path[cur_index];
    sgw->path[tmp_index] = -1;
  }

  for (i = 0; i < path->length; i++) {
    mask->array[path->array[i]] = SP_GROW_SOURCE;
  }

  Stack *canvas = Translate_Stack(stack, COLOR, 1);

  Stack_Sp_Grow_Draw_Result(canvas, path, sgw);
  Write_Stack("../data/test2.tif", canvas);
#endif

  return 0;
}
