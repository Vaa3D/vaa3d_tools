#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <image_filters.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_dimage_lib.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_image_lib.h"
#include "tz_int_histogram.h"
#include "tz_local_neuroseg.h"
#include "tz_fimage_lib.h"
#include "tz_constant.h"
#include "tz_stack_threshold.h"
#include "tz_stack_math.h"
#include "tz_object_3d.h"
#include "tz_stack_draw.h"
#include "tz_darray.h"
#include "tz_image_io.h"
#include "tz_stack.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{

#if 0
  Image *image = Read_Image("../data/threshold.tif");

#if 0
  Image *filter = Mexican_Hat_2D_Filter(3);
  Filter_Image(filter, image, 1);
  Translate_Image(image, GREY, 1);
#endif

#if 0
  DMatrix *filter = Mexihat_2D_D(1, NULL);
  DMatrix_Negative(filter);
  DMatrix_Print(filter);
  DMatrix *out = Filter_Image_D(image, filter, NULL);
  /*
  DMatrix *in = Get_Double_Matrix3(stack);
  darray_sub(out->array, in->array, matrix_size(out->dim, out->ndim));
  */

  Kill_Image(image);
  image = Scale_Double_Image(out->array, out->dim[0], out->dim[1], GREY);
#endif

  Write_Image("../data/test.tif", image);
#endif

#if 0
  Stack *stack = Read_Stack("../data/threshold.tif");
  DMatrix *filter = Mexihat_3D_D(3, NULL);
  DMatrix_Negative(filter);
  //DMatrix_Print(filter);
  DMatrix *out = Filter_Stack_Fast_D(stack, filter, NULL, 0);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1], out->dim[2],
			     GREY);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Local_Neuroseg locseg;
  Set_Local_Neuroseg(&locseg, 2, 2, 12, TZ_PI_2, 0, 0, 3, 3, 3);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(&locseg, 0.5, NULL);
  
  /*
  Stack *stack = Make_Stack(FLOAT32, 20, 20, 20);
  coordinate_3d_t corners[2];
  Geo3d_Scalar_Field_Boundbox(field, corners); 

  Geo3d_Scalar_Field *field2 = Copy_Geo3d_Scalar_Field(field);
  Geo3d_Scalar_Field_Translate(field2, -corners[0][0], -corners[0][1], 
			       -corners[0][2]);

  Geo3d_Scalar_Field_Draw_Stack(field2, stack, NULL, NULL);
  Stack *result = Scale_Float_Stack((float *)stack->array, 
				    stack->width,
				    stack->height,
				    stack->depth,
				    GREY);
  */
  
  
  FMatrix *filter = Geo3d_Scalar_Field_To_Filter_F(field);
  
  Stack *stack = Read_Stack("../data/mouse_neuron.tif");
  FMatrix *filtered_stack = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *result = Scale_Float_Stack(filtered_stack->array, 
				    filtered_stack->dim[0],
				    filtered_stack->dim[1],
				    filtered_stack->dim[2],
				    GREY);

  Write_Stack("../data/test.tif", result);
  
#endif

#if 0
  double sigma[] = {1.5, 1.5, 1.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);

  FMatrix *result2 = FMatrix_Partial_Diff(f, 0, NULL);
  FMatrix *result = FMatrix_Partial_Diff(result2, 0, NULL);

  FMatrix_Partial_Diff(f, 1, result2);
  FMatrix *result3 = FMatrix_Partial_Diff(result2, 1, NULL);
  
  FMatrix_Add(result, result3);

  FMatrix_Partial_Diff(f, 2, result2);
  FMatrix_Partial_Diff(result2, 2, result3);

  FMatrix_Add(result, result3);

  FMatrix_Negative(result);

  FMatrix *result5 = Copy_FMatrix(result);

  FMatrix_Partial_Diff(f, 0, result2);
  FMatrix_Partial_Diff(result2, 0, result);
  FMatrix_Partial_Diff(f, 1, result2);
  FMatrix *result4 = FMatrix_Partial_Diff(result2, 1, NULL);
  FMatrix_Mul(result, result4);

  FMatrix_Partial_Diff(f, 2, result2);
  FMatrix_Partial_Diff(result2, 2, result3);
  FMatrix_Mul(result3, result4); 

  FMatrix_Add(result, result3);

  FMatrix_Partial_Diff(f, 0, result2);
  FMatrix_Partial_Diff(result2, 0, result3);
  FMatrix_Partial_Diff(f, 2, result2);
  FMatrix_Partial_Diff(result2, 2, result4);
  FMatrix_Mul(result3, result4); 

  FMatrix_Add(result, result3);

  FMatrix_Partial_Diff(f, 0, result2);
  FMatrix_Partial_Diff(result2, 1, result3);
  FMatrix_Sqr(result3);

  FMatrix_Sub(result, result3);

  FMatrix_Partial_Diff(f, 1, result2);
  FMatrix_Partial_Diff(result2, 2, result3);
  FMatrix_Sqr(result3);

  FMatrix_Sub(result, result3);

  FMatrix_Partial_Diff(f, 0, result2);
  FMatrix_Partial_Diff(result2, 2, result3);
  FMatrix_Sqr(result3);

  FMatrix_Sub(result, result3);

  FMatrix_Sqrt(result);
  
  FMatrix_Add(result, result5);

  FMatrix_Threshold(result, 0.0);

  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
				 result->dim[2], GREY16);

  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  uint16 *out_array = (uint16*) out->array;
  uint16 *stack_array = (uint16*) stack->array;
  for (i = 0; i < nvoxel; i++) {
    int sum = (int) out_array[i] + (int) stack_array[i];
    if (sum > 65535) {
      sum = 65535;
    }
    out_array[i] = sum;
  }

  Write_Stack("../data/test.tif", out);
#endif

#if 0
  double sigma[] = {1.0, 1.0, 0.5};
  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  //Stack *stack = Read_Stack("../data/test/greyline.tif");
  FMatrix *result = El_Stack_F(stack, sigma);
  
  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
				 result->dim[2], GREY16);
#  if 0
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  uint16 *out_array = (uint16*) out->array;
  uint16 *stack_array = (uint16*) stack->array;
  for (i = 0; i < nvoxel; i++) {
    int sum = (int) out_array[i] + (int) stack_array[i];
    if (sum > 65535) {
      sum = 65535;
    }
    out_array[i] = sum;
  }
#  endif
  
  int *hist = Stack_Hist(out);
  int thre = Int_Histogram_Triangle_Threshold2(hist, Int_Histogram_Min(hist),
					       Int_Histogram_Max(hist), 0.005);
  printf("%d\n", thre);
  Stack_Threshold(out, thre);
  Stack_Binarize(out);

  Translate_Stack(out, GREY, 1);

  //Stack *thremask = Read_Stack("../data/mouse_neuron4_org_crop/mask.tif");
  //Stack_Or(out, thremask, out );

  Write_Stack("../data/test.tif", out);
#endif

#if 0
  double sigma[3] = {1.5, 2.0, 0.5};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  Stack *stack = Make_Stack(GREY, 9,10,7);
  One_Stack(stack);

  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  //Correct_Filter_Stack_F(filter, f);

  FMatrix_Print(f);
#endif

#if 0
  double sigma[3] = {1.5, 2.0, 0.5};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  Stack *stack = Make_Stack(GREY, 11,10,7);
  One_Stack(stack);

  FMatrix *f = Filter_Stack_Block_F(stack, filter, NULL);
  
  Correct_Filter_Stack_F(filter, f);

  FMatrix_Print(f);
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  double scale[] = {3.0, 3.0, 3.0};
  Stack *canvas = Stack_Line_Paint_F(stack, scale, 1);
  Write_Stack("../data/test.tif", canvas);
   Write_Stack("../data/labmeeting5/mouse_neuron4_org_crop/line_s3_color.tif", 
  	      canvas);
#endif

#if 0

  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  double scale[] = {4.0, 4.0, 4.0};
  FMatrix *result = El_Stack_F(stack, scale, NULL);
  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
				 result->dim[2], GREY);
   Write_Stack("../data/labmeeting5/mouse_neuron4_org_crop/line_s4.tif", 
	       out);
#endif
  
#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  double scale[] = {1.0, 1.0, 1.0};
  FMatrix *result = El_Stack_F(stack, scale, NULL);

  int i;
  for (i = 0; i < 3; i++) {
    scale[i] *= sqrt(10.0);
  }

  FMatrix *result2 = El_Stack_F(stack, scale, NULL);
  FMatrix_Min2(result, result2);

  /*
  int j;
  for (j = 0; j < 3; j++) {
    for (i = 0; i < 3; i++) {
      scale[i] *= sqrt(2.0);
    }
    El_Stack_F(stack, scale, result2);
    FMatrix_Max2(result, result2);
  }
  */
  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
				 result->dim[2], GREY);
  
  Write_Stack("../data/test.tif", out);   
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");
  double scale[] = {1.5, 1.5, 1.5};
  
  int m;
  int n;
  int *d = iarray_load_csv("../data/fly_neuron_n1_branch.csv", NULL, &m, &n);

  Object_3d *pts = Make_Object_3d(n, 0);
  int i;
  for (i = 0; i < n; i++) {
    pts->voxels[i][0] = d[i * m] - 1;
    pts->voxels[i][1] = stack->height - d[i * m + 1];
    pts->voxels[i][2] = d[i * m + 2] - 1;
  }

  //DMatrix *result = Stack_Pixel_Feature_D(stack, scale, pts, NULL);
  
  //Print_DMatrix(result);

  Translate_Stack(stack, COLOR, 1);
  Rgb_Color color;
  color.r = 255;
  color.g = 0;
  color.b = 0;
  Stack_Draw_Object_Bwc(stack, pts, color);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0 /* test the speed of fft */
  Stack *stack1 = Make_Stack(1, 1024, 1024, 512);
  Stack *stack2 = Make_Stack(1, 1024, 1024, 512);
  int i;
  for (i = 0; i < Stack_Voxel_Number(stack1); i++) {
    stack1->array[i] = i % 255;
    stack2->array[i] = i % 255;
  }

  /* 2^n can be 5 times faster */
  tic();
  Convolve_Stack_D(stack1, stack2, 1);
  printf("time passed: %lld\n", toc());
  
  /*
  int offset[3];
  float maxcorr;

  tic();
  Align_Stack_F(stack1, stack2, offset, &maxcorr);
  printf("time passed: %lld\n", toc());
  */
#endif

#if 0
  FMatrix *dm = Mexihat_3D1_F(2.0, NULL, 2);
  FMatrix_Negative(dm);

  Stack *stack = Read_Stack_U("../data/diadem_d1_047.xml");
  Stack *out = Filter_Stack(stack, dm);
  
  Stack_Threshold_Common(out, 0, 65535);
  Stack_Binarize(out);

  Translate_Stack(out, GREY, 1);

  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/diadem/diadem_e1.tif");
  double sigma[2] = {3, 3};
  DMatrix *filter = Gaussian_2D_Filter_D(sigma, NULL);
  DMatrix *out = Filter_Stack_Slice_D(stack, filter, NULL);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1], out->dim[2],
			     GREY);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  double sigma[3] = {3, 3, 0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  Print_FMatrix(filter);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/mouse_neuron_big/slice15_3to33ds_C2.tif");
  tic();
  double sigma[3] = {3, 3, 0};
  FMatrix *filter = Gaussian_3D_Filter_2x_F(sigma, NULL);
  Filter_Stack_Slice_F(stack, filter, NULL);
  ptoc();
#endif

#if 1
  Stack *stack = Read_Stack("../data/benchmark/mouse_neuron_big/slice15_3to33ds_C2.tif");
  tic();
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  FMatrix *dm = Make_FMatrix(dim, 3);
  size_t volume = Stack_Voxel_Number(stack);
  for (size_t i = 0; i < volume; ++i) {
    dm->array[i] = stack->array[i];
  }
  dim_type bdim[3] = {3, 3, 1};
  FMatrix_Blockmean(dm, bdim, 0);
  FMatrix_Blockmean(dm, bdim, 0);

  ptoc();
#endif


  return 0;
}
