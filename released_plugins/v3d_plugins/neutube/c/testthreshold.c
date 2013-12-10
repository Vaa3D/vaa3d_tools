#include <stdio.h>
#include "tz_stack_threshold.h"
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_neuroseg.h"
#include "tz_stack_draw.h"
#include "tz_objdetect.h"
#include "tz_iarray.h"
#include "tz_stack_utils.h"
#include "tz_optimize_utils.h"
#include "tz_image_io.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
#if 0
  //Stack *stack = Read_Stack("../data/lobster_neuron.tif");
  //Stack *stack = Read_Stack("../data/run_w1_t2.tif");
  Stack *stack = Read_Stack("../data/tutorial0128.tif");
  //Stack *stack2 = Copy_Stack(stack);
  Stack *stack2 = Stack_Channel_Extraction(stack, 0, NULL);
  Stack *stack4 = Copy_Stack(stack2);
  Write_Stack("../data/tutorial0128_r.tif", stack2);

  /*
  DMatrix *filter = Gaussian_2D_Filter_D(1.0, NULL);
  DMatrix *result = Filter_Stack_Fast_D(stack2, filter, NULL, 0);
  */

  Stack *stack3 = Read_Stack("../data/tutorial0128_n.tif");
  int thre = Stack_Find_Threshold_Locmax(stack3, 0, 65535);
  printf("%d\n", thre);
  
  Stack_Threshold_Binarize(stack2, thre);

  Write_Stack("../data/tutorial0128_bin.tif", stack2);
  
  Stack_Label_Color(stack, stack2, 5.0, 1, stack4);
  Write_Stack("../data/tutorial0128_c.tif", stack);

  Kill_Stack(stack);
  Kill_Stack(stack2);
  Kill_Stack(stack3);
  Kill_Stack(stack4);
#endif

#if 0
  Stack *stack = Read_Stack("../data/test/mono1_n.tif");
  int thre = Stack_Find_Threshold_Locmax(stack, 0, 65535);
  printf("%d\n", thre);
  Kill_Stack(stack);
#endif

#if 0
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
  int *hist = Stack_Hist_M(stack, locmax);
  //int thre = Int_Histogram_Triangle_Threshold(hist, 0, hist[0] + hist[1] - 1);
  int thre = Hist_Tpthre2(hist, 0, hist[0] + hist[1] - 1);
  printf("%d\n", thre);
#endif

#if 0
  int common = Hist_Most_Common(hist, 0, 255);
  
  printf("%d\n", common);

  Stack_Threshold(stack, common);

  Stack_Binarize(stack);
  
  Scale_Stack(stack, 0, 255, 0);

  Write_Stack("../data/threshold/test1.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/ext/TransportOfEndosomalVirus00000026_grey.tif");
  Stack_Threshold_RC(stack, 1, 65535);
  
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron2/dist.tif");
  Stack *result = Read_Stack("../data/fly_neuron2/bundle_seed.tif");

#  if 0
  Stack *result = Stack_Local_Max(stack, NULL, STACK_LOCMAX_FLAT);
  Stack_Clean_Locmax(stack, result);
#  endif

  
  Stretch_Stack_Value(stack);
  
  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  
#  if 0
  Object_3d_List *objs = Stack_Find_Object_N(result, NULL, 1, 0, 26);
  int n = 0;
  while (objs != NULL) {
    Object_3d *obj = objs->data;
    Voxel_t center;
    Object_3d_Central_Voxel(obj, center);

    //double centroid[3];
    //Object_3d_Centroid(obj, centroid);
    /*    
    Stack_Draw_Object_C(canvas, obj, 0,
			Get_Stack_Pixel(stack, obj->voxels[0][0], 
					obj->voxels[0][1], 
					obj->voxels[0][2], 0), 
			0);
    */
    Stack_Draw_Voxel_C(canvas, center[0], center[1], center[2], 4,
		       255, 0, 0);
    /*
    Stack_Draw_Voxel_C(canvas, centroid[0], centroid[1], centroid[2], 4,
		       0, 255, 0);
    */
    objs = objs->next;
    n++;
  }

  //Stack_Draw_Voxel_Mc(canvas, result, 0, color);

  printf("%d\n", n);
#  endif

  Stack_Draw_Voxel_Mc(canvas, result, 4, 255, 0, 0);
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_a2_org.tif");
  Stack *stack2 = Crop_Stack(stack, 0, 0, stack->depth /2, 
			     stack->width, stack->height,
			     1, NULL);

  int *thre = iarray_malloc(stack->depth);

  int i;
  for (i = 0; i < stack->depth; i++) {
    Crop_Stack(stack, 0, 0, i, 
	       stack->width, stack->height,
	       1, stack2);
    thre[i] = Stack_Find_Threshold_Locmax2(stack2, 0, 255, 1.0);
    printf("%d\n", thre[i]);
  }

  iarray_write("../data/test.bn", thre, stack->depth);
  
  double *y = darray_malloc(stack->depth);
  for (i = 0; i < stack->depth; i++) {
    y[i] = thre[i];
  }
  double *x = darray_contarray(1, stack->depth, NULL);
  
  double c[3];
  Polyfit(x, y, stack->depth, 2, c, y);
  
  darray_write("../data/test2.bn", y, stack->depth);

  for (i = 0; i < stack->depth; i++) {
    thre[i] = iround(y[i]);
  }
  
  Stack_Threshold_Binarize_S(stack, thre);
  
  Write_Stack("../data/fly_neuron_a2_org/threshold_s.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_a2_org.tif");
  int thre = Stack_Find_Threshold_Locmax(stack, 0, 255);
  Stack_Threshold_Binarize(stack, thre);
  Write_Stack("../data/test.tif", stack);
#endif

#if 1
  Stack *stack = Read_Stack_U("../data/diadem_d1_023.xml");

  //Stack *out = Stack_Neighbor_Median(stack, 8, NULL);
  Stack_Running_Median(stack, 0, stack);
  Stack *out = Stack_Median_Filter_N(stack, 1, stack);

  Write_Stack("../data/test.tif", out);
#endif

  return 0;
}
