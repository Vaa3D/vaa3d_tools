#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tz_error.h"
#include "tz_<1t>_lib.h"
<3t=double,float>
#include <math.h>
#include "tz_complex.h"
#include "tz_image_lib.h"
#include "tz_<5t>.h"
#include "tz_iarray.h"
</t>
#include "tz_<7t>.h"
#include "tz_arrayview.h"
#include "tz_image_lib_defs.h"
#include "tz_math.h"

<3t=double,float>
/**
 * Cumulative sum of an image.
 */
<3t>* Cumsum_Image_<4T>(const Image *image)
{
  if(image == NULL) {
    return NULL;
  }

  <3t>* cumsum = (<3t>*)malloc(sizeof(<3t>)*image->width*image->height);
  <3t> cur_state = 0;
  int x,y,offset=1;
  uint16* array16 = (uint16*)image->array;
  float32* array32 = (float32*)image->array;

  switch(image->kind) {
  case GREY:
    cumsum[0] = (<3t>)image->array[0];

    /*The first row*/
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + (<3t>)image->array[offset];
      offset++;
    }

    
    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	//Cumulate row sum of the current row
	cur_state += (<3t>)image->array[offset];
	//Sum the cumsum of the up neighbor and current row sum
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  case GREY16:
    cumsum[0] = (<3t>)array16[0];
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + (<3t>)array16[offset];
      offset++;
    }

    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	cur_state += (<3t>)array16[offset];
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  case FLOAT32:
    cumsum[0] = (<3t>)array32[0];
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + (<3t>)array32[offset];
      offset++;
    }

    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	cur_state += (<3t>)array32[offset];
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  default:
    fprintf(stderr,"Unsupported image kind in Cumsum_Image()\n");
  }

  return cumsum;
}


/**
 * FFT of an image. Since the fourier transform of an image is symmetric, it only returns half 
 * of the transformed image. See the tutorial of fftw for more details.
 * Notice: The output should be freed by fftw_free after being used.
 */
<5t>_complex* Image_FFT_<4T>(Image *image)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  <5t>_complex *out = NULL;
  if(image) {
    <3t>* in = (<3t> *) Get_<6T>_Array(image);

    out = <5t>_malloc_r2c_2d(image->height,image->width);
    <5t>_plan p = <5t>_plan_dft_r2c_2d(image->height,image->width,in,out,FFTW_ESTIMATE);
    <5t>_execute(p);
    <5t>_destroy_plan(p);
    <5t>_free(in);
  }

  return out;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

/**
 * IFFT of the FFT of an image. It takes the output of Image_FFT as the input.
 */
<3t> *Image_IFFT_<4T>(<5t>_complex* fimage,int width,int height)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  <3t>* out = NULL;
  if(fimage) {
    long length = (long)width*height;
    out = (<3t>*)malloc(length*sizeof(<3t>));
    <5t>_plan p = <5t>_plan_dft_c2r_2d(height,width,fimage,out,FFTW_ESTIMATE);
    <5t>_execute(p);
    <5t>_destroy_plan(p);
    <7t>_divc(out,length,length);
  }

  return out;
<3t=double,float>
#else
  TZ_ERROR(ERROR_NA_FUNC);
</t>
  return NULL;
<3t=double,float>
#endif
</t>
}

/**
 * Image correlation. image1 and image2 must have the same size.
 * If reflect is not 0, image2 will be rotated by 180 degree before 
 * convolution. This will result in correlation.
 */
<3t>* Convolve_Image_<4T>(Image *image1, Image *image2,int reflect)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  if( (image1->width!=image2->width) || (image1->height!=image2->height) ) {
    fprintf(stderr,"Unmatched image size in Convolve_Image_<4T>()\n");
    exit(1);
  }

  long length = (long)(image1->width/2+1)*image1->height;
  <5t>_complex* ft1 = Image_FFT_<4T>(image1);
  <5t>_complex* ft2 = Image_FFT_<4T>(image2);


  if(reflect)
    <5t>_conjg_array(ft2,length);

  /*
  <3t>* test = Image_IFFT_<4T>(ft2,image1->width,image1->height);
  <7t>_print2(test,image2->width,image2->height);
  free(test);
  */

  //printf("%d\n",length);

  <5t>_cmul_array(ft1,ft2,length);


  <5t>_free(ft2);
  <3t>* result = Image_IFFT_<4T>(ft1,image1->width,image1->height);

  //<7t>_print2(result,image2->width,1);

  <5t>_free(ft1);

  return result;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

/**
 * Image convolution. image1 and image2 must have the same size.
 */
<3t>* Correlate_Image_<4T>(Image* image1,Image* image2)
{
  return Convolve_Image_<4T>(image1,image2,1);
}

/**
 * Normalized correlation between 2 images.
 * http://www.idiom.com/~zilla/Work/nvisionInterface/nip.html
 */
<3t>* Normcorr_Image_<4T>(Image *image1, Image *image2)
{
  <3t> *corr_image = NULL;
  long length1,length2,length;
  int corr_width,corr_height;
  int width1,height1,width2,height2;

  width1 = image1->width;
  height1 = image1->height;
  width2 = image2->width;
  height2 = image2->height;
  length1 = Get_Pixel_Number(image1);
  length2 = Get_Pixel_Number(image2);

  corr_width = width1+width2-1;
  corr_height = height1+height2-1;
  Image* ref_image1 = Reflect_Image(image1,0);
  Image* padded_image1 = Crop_Image(ref_image1,-width2,-height2,corr_width+1,corr_height+1, NULL);
  Image* padded_image2 = Crop_Image(image2,-width1,-height1,corr_width+1,corr_height+1, NULL);

  Write_Image("data/out3.tif",padded_image1);
  Write_Image("data/out4.tif",padded_image2);

  Kill_Image(ref_image1);

  //Print_Image_Value(padded_image1);
  Write_Image("data/out.tif",padded_image2);
  
  <3t>* temp_corr_image = Convolve_Image_<4T>(padded_image1,padded_image2,0);
  corr_image = <7t>_malloc2(corr_width,corr_height);

  long offset1=0,offset2=0;
  size_t row_size = sizeof(<3t>)*corr_width;
  int i;
  for(i=0;i<corr_height;i++) {
    memcpy(corr_image+offset1,temp_corr_image+offset2,row_size);
    offset1 += corr_width;
    offset2 += corr_width+1;
  }

  //<7t>_print2(corr_image,corr_width,corr_height);

  free(temp_corr_image);

  //<7t>_print2(corr_image,corr_width,corr_height);

  length = (long)corr_width*corr_height;
  Kill_Image(padded_image1);
  Kill_Image(padded_image2);
  
  <3t>* dimage2 = Get_<6T>_Array(image2);
  <3t>* fsuv = <7t>_blocksum(dimage2,width2,height2,width1,height1);
  <3t>* fmuv = <7t>_copy(fsuv,length);
  <7t>_blockmean(fmuv,width2,height2,width1,height1,1);

  <7t>_sqr(dimage2,length2);
  <3t>* fsuv2 = <7t>_blocksum(dimage2,width2,height2,width1,height1);

  free(dimage2);

  Reflect_Image(image1,1);
  <3t>* dimage1 = Get_<6T>_Array(image1);
  <3t>* tsuv = <7t>_blocksum(dimage1,width1,height1,width2,height2);
  <3t>* tmuv = <7t>_copy(tsuv,length);
  <7t>_blockmean(tmuv,width1,height1,width2,height2,1);
  <7t>_sqr(dimage1,length1);
  <3t>* tsuv2 = <7t>_blocksum(dimage1,width1,height1,width2,height2);
  free(dimage1);
  Reflect_Image(image1,1);

  //summed standard deviation of image2
  <7t>_mul(fsuv,fmuv,length);
  <7t>_sub(fsuv2,fsuv,length);
  <7t>_sqrt(fsuv2,length);
  free(fsuv);

  //summed standard deviation of image1
  <7t>_mul(tmuv,tsuv,length);
  <7t>_sub(tsuv2,tmuv,length);
  <7t>_sqrt(tsuv2,length);
  free(tmuv);

  //the product of the two deviations
  <7t>_mul(fsuv2,tsuv2,length);
  free(tsuv2);

  <7t>_mul(tsuv,fmuv,length);
  <7t>_sub(corr_image,tsuv,length);

  //<7t>_print2(corr_image,corr_width,corr_height);
  //<7t>_print2(fsuv2,corr_width,corr_height);

  <7t>_div(corr_image,fsuv2,length);
  
  //<7t>_print2(corr_image,corr_width,corr_height);

  free(fsuv2);
  free(fmuv);
  free(tsuv);

  return corr_image;
}


<2T=DMatrix,FMatrix>

/* Gaussian_2D_Filter_<4T>(): 2D Gaussian filter.
 *
 * Args: sigma - standard deviation of the filter;
 *       filter - output filter. If it is NULL, a new filter will be created.
 *
 * Return: the filter.
 */
<2T>* Gaussian_2D_Filter_<4T>(const double *sigma, <2T> *filter)
{
  dim_type i, j;
  dim_type offset = 0;;
  double r;
  double coord[2];
  double weight = 0.0;

  dim_type wndsize[2];
  double sigma2[2];

  for (i = 0; i < 2; i++) {
    wndsize[i] = (dim_type) (sigma[i] + 0.5) * 2;
    sigma2[i] = sigma[i] * sigma[i];
  }

  if (filter == NULL) {
    dim_type dim[2];
    for (i = 0; i < 2; i++) {
      dim[i] = wndsize[i] * 2 + 1;
    }
    filter = Make_<2T>(dim, 2);
  }
  
  for (j = 0; j < filter->dim[1]; j++) {
    for (i = 0; i < filter->dim[0]; i++) {
      coord[0] = ((double) i) - wndsize[0];
      coord[1] = ((double) j) - wndsize[1];
      r = coord[0] * coord[0] / sigma2[0] + coord[1] * coord[1] / sigma2[1];
      filter->array[offset] = exp(-r / 2); 
      weight += filter->array[offset];
      offset ++;
    }
  }

  for (i = 0; i < offset; i++) {
    filter->array[i] /= weight;
  }

  return filter;  
}

<2T>* Mexihat_3D1_<4T>(double sigma, <2T> *filter, ndim_t dt)
{
  dim_t wndsize = (dim_t) (sigma * 3);
  if (filter == NULL) {
    dim_t dim[3];
    dim[0] = wndsize * 2 + 1;
    dim[1] = 1;
    dim[2] = 1;
    filter = Make_<2T>(dim, 3);
  }

  dim_t i;
  double r;
  double coord;
  double sigma2 = sigma * sigma * 2.0;
  double weight = 0.0;
  double mean = 0.0;

  for (i = 0; i < filter->dim[0]; i++) {
    coord = ((double) i) - wndsize;
    r = coord * coord / sigma2;
    filter->array[i] = (1.0 - r) * exp(-r);
    weight += filter->array[i] * filter->array[i];
    mean += filter->array[i];
  }

  mean /= filter->dim[0];

  for (i = 0; i < filter->dim[0]; i++) {
    filter->array[i] -= mean;
    filter->array[i] /= sqrt(weight);
  }

  if (dt != 0) {
    filter->dim[dt] = filter->dim[0];
    filter->dim[0] = 1;
  }

  return filter;  
}

/* Mexhihat_2D(): 2D Mexican hat filter.
 *
 * Args: sigma - standard deviation of the filter;
 *       filter - output filter. If it is NULL, a new filter will be created.
 *
 * Return: the filter.
 */
<2T>* Mexihat_2D_<4T>(double sigma, <2T> *filter)
{
  dim_type wndsize = (dim_type) (sigma * 3);
  if (filter == NULL) {
    dim_type dim[2];
    dim[0] = wndsize * 2 + 1;
    dim[1] = dim[0];
    filter = Make_<2T>(dim, 2);
  }

  dim_type i, j;
  dim_type offset = 0;;
  double r;
  double coord[2];
  double sigma2 = sigma * sigma * 2;
  double weight = 0.0;
  
  for (j = 0; j < filter->dim[1]; j++) {
    for (i = 0; i < filter->dim[0]; i++) {
      coord[0] = ((double) i) - wndsize;
      coord[1] = ((double) j) - wndsize;
      r = (coord[0] * coord[0] + coord[1] * coord[1]) / sigma2;
      filter->array[offset] = (1.0 - r) * exp(-r);
      weight += fabs(filter->array[offset]);
      offset++;
    }
  }

  for (i = 0; i < offset; i++) {
    filter->array[i] /= weight;
  }

  return filter;  
}

#define FILTER_IMAGE_<4T>(subimage_array)				\
   for (j = 0; j < image->height; j++) {				\
     top = j - filter_offset[1];					\
     for (i = 0; i < image->width; i++) {				\
       left = i - filter_offset[0];					\
       Crop_Image(image, left, top, filter->dim[0], filter->dim[1], subimage); \
       out->array[offset] = 0;						\
       for (m = 0; m < filter_length; m++) {				\
	 out->array[offset] += filter->array[m] * (<3t>) (subimage_array[m]); \
       }								\
       offset++;							\
     }									\
   }

/* Filter_Image_<4T>(): Image filtering.
 *
 * Notice: the caller is responsible for clearing up the output.
 *
 * Args: image - input stack;
 *       filter - stack filter, which is a 2D <3t> matrix;
 *       out - filtered stack, which is a 2D <3t> matrix. If it is NULL, a new
 *             <3t> matrix will be created.
 *
 * Return: filtered stack.
 */
<2T>* Filter_Image_<4T>(Image *image, const <2T> *filter, <2T> *out)
{
  int i, j, m;
  int left, top;
  int filter_offset[2];
  int filter_length = matrix_size(filter->dim, filter->ndim);
  int offset = 0;

  dim_type dim[2];
  dim[0] = image->width;
  dim[1] = image->height;

  if (out == NULL) {
    out = Make_<2T>(dim, 2);
  }

  for (i = 0; i < 2; i++) {
    filter_offset[i] = (filter->dim[i] - 1) / 2;
  }

  Image *subimage = Make_Image(image->kind, filter->dim[0], filter->dim[1]);
  DEFINE_SCALAR_ARRAY_ALL(subimage, subimage);

  switch (image->kind) {
  case GREY:
    FILTER_IMAGE_<4T>(subimage_grey);
    break;
  case GREY16:
    FILTER_IMAGE_<4T>(subimage_grey16);
    break;
  case FLOAT32:
    FILTER_IMAGE_<4T>(subimage_float32);
    break;
  case FLOAT64:
    FILTER_IMAGE_<4T>(subimage_float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
    
  Kill_Image(subimage);
  return out;
}
</t>


/**
 * Copy the values of a stack to a double array, which is padded along width
 * to be suitable for fftw application.
 */
<3t>* Get_<6T>_Array_Pad(Stack *stack)
{
  int pad_width = (stack->width/2+1)*2;
  int pad_offset = pad_width - stack->width;
  int hd_length = stack->height*stack->depth;
  int i,j;
  int offset1,offset2;
  <3t>* array = (<3t> *) malloc(pad_width*hd_length*sizeof(<3t>));

  uint16 *array16 = (uint16 *) stack->array;
  float32 *array32 = (float32 *) stack->array;

  offset1 = 0;
  offset2 = 0;

  switch(stack->kind) {
  case GREY:
    for(i=0;i<hd_length;i++) {
      for(j=0;j<stack->width;j++) {
	array[offset2] = stack->array[offset1];
	offset1++;
	offset2++;
      }
      offset2 += pad_offset;
    }
    break;
  case GREY16:
    for(i=0;i<hd_length;i++) {
      for(j=0;j<stack->width;j++) {
	array[offset2] = array16[offset1];
	offset1++;
	offset2++;
      }
      offset2 += pad_offset;
    }
    break;
  case FLOAT32:
    for(i=0;i<hd_length;i++) {
      for(j=0;j<stack->width;j++) {
	array[offset2] = array32[offset1];
	offset1++;
	offset2++;
      }
      offset2 += pad_offset;
    }
    break;
  }
  return array;
}

/**
 * FFT of an satck. Since the fourier transform of an image is symmetric, it only returns half 
 * of the transformed image. See the tutorial of fftw for more details.
 */
<5t>_complex* Stack_FFT_<4T>(Stack *stack)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  <5t>_complex *out = NULL;
  if(stack) {
    out = (<5t>_complex *) Get_<6T>_Array_Pad(stack);
    <5t>_plan p = <5t>_plan_dft_r2c_3d(stack->depth,stack->height,stack->width,(<3t> *)out,out,FFTW_ESTIMATE);
    <5t>_execute(p);
    <5t>_destroy_plan(p);
  }

  return out;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

/**
 * IFFT of the FFT of an stack. It takes the output of Image_FFT as the input.
 */
<3t>* Stack_IFFT_<4T>(<5t>_complex* fstack,int width,int height,int depth)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  <3t>* out = NULL;
  if(fstack) {
    long length = (long)width*height*depth;

    out = (<3t> *)fstack;

    <5t>_plan p = <5t>_plan_dft_c2r_3d(depth,height,width,fstack,out,FFTW_ESTIMATE);
    <5t>_execute(p);

    <5t>_destroy_plan(p);
    <5t>_pack_c2r_result(out,width,depth*height);
    <7t>_divc(out,length,length);

  }

  return out;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

/**
 * Stack convolution. stack1 and stack2 must have the same size.
 * If reflect is not 0, stack2 will be reflected around the origin
 * point before correlation.
 */
<3t>* Convolve_Stack_<4T>(Stack* stack1,Stack* stack2,int reflect)
{
  if( (stack1->width!=stack2->width) || (stack1->height!=stack2->height) 
      || (stack1->depth!=stack2->depth) ) {
    fprintf(stderr,"Unmatched stack size in Convolve_Stack_<4T>\n");
    exit(1);
  }

  long length = (long)(stack1->width/2+1)*stack1->height*stack1->depth;
  <5t>_complex* ft1 = Stack_FFT_<4T>(stack1);

  <5t>_complex* ft2 = Stack_FFT_<4T>(stack2);

  if(reflect)
    <5t>_conjg_array(ft2,length);

  <5t>_cmul_array(ft1,ft2,length);

  free(ft2);
  
  <3t>* result = Stack_IFFT_<4T>(ft1,stack1->width,stack1->height,stack1->depth);

  return result;
}

<3t>* Correlate_Padstack_<4T>(Stack* stack1,Stack* stack2)
{
  int corr_width,corr_height,corr_depth;

  int width1 = stack1->width;
  int height1 = stack1->height;
  int depth1 = stack1->depth;

  corr_width = stack1->width+stack2->width-1;
  corr_height = stack1->height+stack2->height-1;
  corr_depth = stack1->depth+stack2->depth-1;

  /*
  Stack* padded_stack1 = Crop_Stack(stack1,0,0,0,corr_width,corr_height,corr_depth, NULL);
  Stack* padded_stack2 = Crop_Stack(stack2,-stack1->width+1,-stack1->height+1,-stack1->depth+1,corr_width,corr_height,corr_depth, NULL);

  <3t>* corr_stack = Convolve_Stack_<4T>(padded_stack2,padded_stack1,1);
  */
#if 0
  Stack* padded_stack1 = Crop_Stack(stack1,0,0,0,corr_width,corr_height,corr_depth, NULL);
  Stack* padded_stack2 = Crop_Stack(stack2,-width1+1,-height1+1,-depth1+1,corr_width,corr_height,corr_depth, NULL);

  <3t>* corr_stack = Convolve_Stack_<4T>(padded_stack2,padded_stack1,1);
#else /* faster version for 2^n-size stacks */
  Stack* padded_stack1 = Crop_Stack(stack1, 0, 0, 0,
				    corr_width + 1, corr_height + 1, 
				    corr_depth, NULL);
  Stack* padded_stack2 = Crop_Stack(stack2, -width1+1, -height1+1, -depth1+1,
				    corr_width + 1,corr_height + 1, 
				    corr_depth, NULL);

  <3t>* corr_stack = Convolve_Stack_<4T>(padded_stack2,padded_stack1,1);

  Stack src, dst;
  src.width = padded_stack1->width;
  src.height = padded_stack1->height;
  src.depth = padded_stack1->depth;
  src.array = (uint8*) corr_stack;

  dst.width = corr_width;
  dst.height = corr_height;
  dst.depth = corr_depth;
  dst.array = (uint8*) corr_stack;

<3t=double>
  src.kind = FLOAT64;
  dst.kind = FLOAT64;
</t>

<3t=float>
  src.kind = FLOAT32;
  dst.kind = FLOAT32;
</t>

  Crop_Stack(&src, 0, 0, 0, corr_width, corr_height, corr_depth, &dst);
#endif
 
  Kill_Stack(padded_stack1);
  Kill_Stack(padded_stack2);

  return corr_stack;
}

#define CORRELATE_STACK_PART(array1, array2)		\
   for(k=start[2];k<=end[2];k++) {			\
     tmpoffset[1] = offset;				\
     start1[2] = imax2(stack1->depth-1-k,0);		\
     end1[2] = imin2(corr_depth-1-k,stack1->depth-1);	\
     start2[2] = imax2(k-stack1->depth+1,0);		\
     /*end2[2] = imin2(k,stack2->depth-1);*/		\
     for(j=start[1];j<=end[1];j++) {			\
       tmpoffset[0] = offset;					\
       start1[1] = imax2(stack1->height-1-j,0);			\
       end1[1] = imin2(corr_height-1-j,stack1->height-1);	\
       start2[1] = imax2(j-stack1->height+1,0);			\
       /*end2[1] = imin2(j,stack2->height-1);*/			\
       for(i=start[0];i<=end[0];i++) {				\
	 start1[0] = imax2(stack1->width-1-i,0);		\
	 end1[0] = imin2(corr_width-1-i,stack1->width-1);		\
	 start2[0] = imax2(i-stack1->width+1,0);			\
	 /*end2[0] = imin2(i,stack2->width-1);*/				\
	 offset1 = start1[0]+start1[1]*stack1->width+start1[2]*plane_offset1; \
	 offset2 = start2[0]+start2[1]*stack2->width+start2[2]*plane_offset2; \
	 corr_stack[offset] = 0;					\
	 for(k1=start1[2];k1<=end1[2];k1++) {				\
	   tmpoffset1[1] = offset1;					\
	   tmpoffset2[1] = offset2;					\
	   inc = 0;							\
	   for(j1=start1[1];j1<=end1[1];j1++) {				\
	     tmpoffset1[0] = offset1;					\
	     tmpoffset2[0] = offset2;					\
	     for(i1=start1[0];i1<=end1[0];i1++) {			\
	       inc += (((<3t>)array1[offset1])) * (((<3t>)array2[offset2])); \
	       offset1++;						\
	       offset2++;						\
	     }								\
	     offset1 = tmpoffset1[0] +  stack1->width;			\
	     offset2 = tmpoffset2[0] + stack2->width;			\
	     								\
	     }								\
	   corr_stack[offset] += inc;					\
	   offset1 = tmpoffset1[1] + plane_offset1;			\
	   offset2 = tmpoffset2[1] + plane_offset2;			\
	 }								\
	 offset++;							\
       }								\
       offset = tmpoffset[0]+width2;					\
     }									\
     offset = tmpoffset[1]+plane_offset;				\
   }
   
<3t>* Correlate_Stack_Part_<4T>(Stack* stack1,Stack* stack2,int start[],int end[])
{
  int i,j,k,i1,j1,k1;
  int start1[3],start2[3],end1[3]/*,end2[3]*/;
  int width2,height2,depth2;
  int corr_width,corr_height,corr_depth;
  int offset,offset1,offset2,tmpoffset[2],tmpoffset1[2],tmpoffset2[2];
  int plane_offset,plane_offset1,plane_offset2;
  <3t> inc;
  
  width2 = end[0]-start[0]+1;
  height2 = end[1]-start[1]+1;
  depth2 = end[2]-start[2]+1;

  <3t>* corr_stack = <7t>_malloc3(width2,height2,depth2);

  corr_width = stack1->width+stack2->width-1;
  corr_height = stack1->height+stack2->height-1;
  corr_depth = stack1->depth+stack2->depth-1;

  plane_offset = width2*height2;
  plane_offset1 = stack1->width*stack1->height;
  plane_offset2 = stack2->width*stack2->height;
  
  offset = 0;
  
  DEFINE_SCALAR_ARRAY_ALL(stack1_array, stack1);
  DEFINE_SCALAR_ARRAY_ALL(stack2_array, stack2);

#define STACK2_CORRELATE(array1)					\
  switch (stack2->kind) {						\
  case GREY:								\
    CORRELATE_STACK_PART(array1, stack2_array_grey);			\
    break;								\
  case GREY16:								\
    CORRELATE_STACK_PART(array1, stack2_array_grey16);			\
    break;								\
  case FLOAT32:								\
    CORRELATE_STACK_PART(array1, stack2_array_float32);			\
    break;								\
  case FLOAT64:								\
    CORRELATE_STACK_PART(array1, stack2_array_float64);			\
    break;								\
  default:								\
    THROW(ERROR_DATA_TYPE);						\
    break;								\
  }

  switch (stack1->kind) {
  case GREY:
    STACK2_CORRELATE(stack1_array_grey);
    break;
  case GREY16:
    STACK2_CORRELATE(stack1_array_grey16);
    break;
  case FLOAT32:
    STACK2_CORRELATE(stack1_array_float32);
    break;
  case FLOAT64:
    STACK2_CORRELATE(stack1_array_float64);
    break;
  default:
    THROW(ERROR_DATA_TYPE);
    break;  
  }

#undef STACK2_CORRELATE
  return corr_stack;
}

#undef CORRELATE_STACK_PART

/**
 * Normalized correlation between 2 stacks. std is a flag for
 * standarizing the results or not. stack1 is the moving template.
 */
<2T>* Normcorr_Stack_<4T>(Stack *stack1, Stack *stack2, int std, <3t> *max_corr)
{
  <3t> *corr_stack = NULL;
  size_t length1,length2,length;
  int corr_width,corr_height,corr_depth;
  int width1,height1,depth1,width2,height2,depth2;

  width1 = stack1->width;
  height1 = stack1->height;
  depth1 = stack1->depth;
  width2 = stack2->width;
  height2 = stack2->height;
  depth2 = stack2->depth;

  length1 = width1*height1*depth1;
  length2 = width2*height2*depth2;

  corr_width = width1+width2-1;
  corr_height = height1+height2-1;
  corr_depth = depth1+depth2-1;

  length = (size_t) corr_width*corr_height*corr_depth;

  
  dim_type bdim[3];
  <2T>* dstack2 = Get_<6T>_Matrix3(stack2);
  bdim[0] = (dim_type)width1;
  bdim[1] = (dim_type)height1;
  bdim[2] = (dim_type)depth1;

  dim_type sub[3];

#ifdef _DEBUG_
  printf("Calculating fsuv ...\n");
#endif

  <2T>* fsuv = <2T>_Blocksum(dstack2, bdim, NULL);

#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv,sub));
  printf("Calculating tsuv ...\n");
#endif

  <2T>* fmuv = Copy_<2T>(fsuv);
  <2T>_Blockmean(fmuv,bdim,1);

#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fmuv,sub));
#endif

  <7t>_sqr(dstack2->array, length2);

#ifdef _DEBUG_
  printf("Calculating fsuv2 ...\n");
#endif

  <2T>* fsuv2 = <2T>_Blocksum(dstack2, bdim, NULL);

#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv2,sub));
#endif

  Kill_<2T>(dstack2);

  //summed standard deviation of stack2
#ifdef _DEBUG_
  printf("summed standard deviation of stack2 ...\n");
#endif
  <7t>_fun2(fsuv2->array,fsuv->array,fmuv->array,length);
  Kill_<2T>(fsuv);

  Reflect_Stack(stack1,1);
  <2T>* dstack1 = Get_<6T>_Matrix3(stack1);
  bdim[0] = width2;
  bdim[1] = height2;
  bdim[2] = depth2;
#ifdef _DEBUG_
  printf("Calculating tsuv ...\n");
#endif
  <2T>* tsuv = <2T>_Blocksum(dstack1, bdim, NULL);
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tsuv,sub));
#endif

  <2T>* tmuv = Copy_<2T>(tsuv);
#ifdef _DEBUG_
  printf("Calculating tmuv ...\n");
#endif
  <2T>_Blockmean(tmuv,bdim,1);
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tmuv,sub));
#endif
  <7t>_sqr(dstack1->array,length1);
#ifdef _DEBUG_
  printf("Calculating tsuv2 ...\n");
#endif
  <2T>* tsuv2 = <2T>_Blocksum(dstack1, bdim, NULL);
  Kill_<2T>(dstack1);
  Reflect_Stack(stack1,1);

  //summed standard deviation of stack1
#ifdef _DEBUG_
  printf("summed standard deviation of stack1 ...\n");
#endif
  <7t>_fun2(tsuv2->array,tsuv->array,tmuv->array,length);
  Kill_<2T>(tmuv);

  //the product of the two deviations
#ifdef _DEBUG_
  printf("the product of the two deviations ...\n");
#endif
  <7t>_mul(fsuv2->array,tsuv2->array,length);
  Kill_<2T>(tsuv2);

#ifdef _DEBUG_
  printf("Correlate stacks ...\n");
#endif
  corr_stack = Correlate_Padstack_<4T>(stack1, stack2);

  <7t>_fun3(corr_stack,tsuv->array,fmuv->array,fsuv2->array,length);

#ifdef _DEBUG_
  size_t tmp_idx;
#endif

#ifdef _DEBUG_
  printf("max corrcoef: %g\n", <7t>_max(corr_stack, length, &tmp_idx));
  printf("mu: %g\n", fmuv->array[tmp_idx]);  
  printf("std: %g\n", fsuv2->array[tmp_idx]);
#endif
  Kill_<2T>(fsuv2);
  Kill_<2T>(fmuv);
  Kill_<2T>(tsuv);

  /*
  <2T> corr_matrix = 
     <2T>_View_Array(corr_stack,3,corr_width,corr_height,corr_depth);
  Stack_View sv = Stack_View_<2T>(&corr_matrix);
  Stack_Locmax_Enhance(&(sv.stack), &(sv.stack));
  */

  //free(result);
#ifdef _DEBUG_
  printf("max corrcoef: %g\n", <7t>_max(corr_stack, length, &tmp_idx));
#endif

  if(std > 0) {
    dim_type dim[3];
    dim[0] = width2;
    dim[1] = height2;
    dim[2] = depth2;

    bdim[0] = width1;
    bdim[1] = height1;
    bdim[2] = depth1;
    

    <2T>* Ns = <2T>_Ones(dim,3);
    <2T>* Ns2 = <2T>_Blocksum(Ns, bdim, NULL);
    Kill_<2T>(Ns);
   
    //int minarea = 512;
    //int minarea = length / 1000 + 128;
    int minarea = std;
    if (std < 10) {
      switch (std) {
      case 1:
	minarea = imin2(imax3(stack1->width, stack1->height, stack1->depth),
			imax3(stack2->width, stack2->height, stack2->depth));
	break;
      case 2:
	minarea = imin2(imax3(stack1->width, stack1->height, stack1->depth),
			imax3(stack2->width, stack2->height, stack2->depth));
	minarea *= minarea;
	break;
      case 3:
	minarea = imin2(imin2(stack1->width, stack1->height) * stack1->depth,
			imin2(stack2->width, stack2->height) * stack2->depth);
	break;
      default:
	minarea = 128;
      }
    }

    Stack *bstack1 = Copy_Stack(stack1);
    Stack_Binarize(bstack1);
    Stack * bstack2 = Copy_Stack(stack2);
    Stack_Binarize(bstack2);

    IMatrix *area = Stack_Foreoverlap(bstack1,bstack2);
    int max_overlap = IMatrix_Max(area, NULL);

    minarea = imin2(max_overlap/5, minarea);

    Kill_Stack(bstack1);
    Kill_Stack(bstack2);

    size_t i;
    for(i=0;i<length;i++)
      if(area->array[i]<=minarea) {
	Ns2->array[i] = 0;
	corr_stack[i] = 0;
      }
    Kill_IMatrix(area);

#ifdef _DEBUG_
    printf("minarea: %d\n", minarea);
#endif

#ifdef _DEBUG_2
    size_t tmpidx;
    <7t>_max(corr_stack, length, &tmpidx);
    printf("%d, %d, %g, %g\n", tmpidx, length, corr_stack[tmpidx], Ns2->array[tmpidx]); 
#endif

    if(max_corr!=NULL)
      <7t>_fun1_max(corr_stack,Ns2->array,bdim[0],length,max_corr);
    else
      <7t>_fun1(corr_stack,Ns2->array,bdim[0],length);
#ifdef _DEBUG_
    printf("max_corr: %g\n", *max_corr);
#endif
    Kill_<2T>(Ns2);
  }

  <2T> *result = <7t>2<2t>(corr_stack,3,corr_width,corr_height,corr_depth);

  return result;
}


<2T>* Normcorr_Stack_White_<4T>(Stack *stack1, Stack *stack2,int std,<3t> *max_corr)
{
  <3t> *corr_stack = NULL;
  size_t length1,length2,length;
  int corr_width,corr_height,corr_depth;
  int width1,height1,depth1,width2,height2,depth2;

  width1 = stack1->width;
  height1 = stack1->height;
  depth1 = stack1->depth;
  width2 = stack2->width;
  height2 = stack2->height;
  depth2 = stack2->depth;

  length1 = (long)width1*height1*depth1;
  length2 = (long)width2*height2*depth2;

  corr_width = width1+width2-1;
  corr_height = height1+height2-1;
  corr_depth = depth1+depth2-1;

  length = (size_t)corr_width*corr_height*corr_depth;

  
  dim_type bdim[3];
  <2T>* dstack2 = Get_<6T>_Matrix3(stack2);
  bdim[0] = (dim_type)width1;
  bdim[1] = (dim_type)height1;
  bdim[2] = (dim_type)depth1;

  Stack* bstack1 = Copy_Stack(stack1);
  Stack_Binarize(bstack1);
  Stack* bstack2 = Copy_Stack(stack2);
  Stack_Binarize(bstack2);

  IMatrix *area = Stack_Foreunion(bstack1,bstack2);

  Kill_Stack(bstack1);
  Kill_Stack(bstack2);

  printf("Calculating fsuv ...\n");
  <2T>* fsuv = <2T>_Blocksum(dstack2, bdim, NULL);
  printf("Calculating fmuv ...\n");
  <2T>* fmuv = Copy_<2T>(fsuv);
  <7t>_div_i(fmuv->array,area->array,length);
  //<2T>_Blockmean(fmuv,bdim,1);

  <7t>_sqr(dstack2->array,length2);
  printf("Calculating fsuv2 ...\n");
  <2T>* fsuv2 = <2T>_Blocksum(dstack2, bdim, NULL);

  Kill_<2T>(dstack2);

  //summed standard deviation of stack2
  printf("summed standard deviation of stack2 ...\n");
  <7t>_fun2(fsuv2->array,fsuv->array,fmuv->array,length);
  Kill_<2T>(fsuv);

  Reflect_Stack(stack1,1);
  <2T>* dstack1 = Get_<6T>_Matrix3(stack1);
  bdim[0] = width2;
  bdim[1] = height2;
  bdim[2] = depth2;
  printf("Calculating tsuv ...\n");
  <2T>* tsuv = <2T>_Blocksum(dstack1, bdim, NULL);

  <2T>* tmuv = Copy_<2T>(tsuv);
  printf("Calculating tmuv ...\n");
  //<2T>_Blockmean(tmuv,bdim,1);
  <7t>_div_i(tmuv->array,area->array,length);
  Kill_IMatrix(area);
  <7t>_sqr(dstack1->array,length1);
  printf("Calculating tsuv2 ...\n");
  <2T>* tsuv2 = <2T>_Blocksum(dstack1, bdim, NULL);
  Kill_<2T>(dstack1);
  Reflect_Stack(stack1,1);

  //summed standard deviation of stack1
  printf("summed standard deviation of stack1 ...\n");
  <7t>_fun2(tsuv2->array,tsuv->array,tmuv->array,length);
  Kill_<2T>(tmuv);

  //the product of the two deviations
  printf("the product of the two deviations ...\n");
  <7t>_mul(fsuv2->array,tsuv2->array,length);
  Kill_<2T>(tsuv2);

  corr_stack = Correlate_Padstack_<4T>(stack1,stack2);

  <7t>_fun3(corr_stack,tsuv->array,fmuv->array,fsuv2->array,length);

  Kill_<2T>(fsuv2);
  Kill_<2T>(fmuv);
  Kill_<2T>(tsuv);

  if(std) {
    
    /*
    dim_type dim[3];
    dim[0] = width2;
    dim[1] = height2;
    dim[2] = depth2;

    bdim[0] = width1;
    bdim[1] = height1;
    bdim[2] = depth1;

    <2T>* Ns = <2T>_Ones(dim,3);
    <2T>* Ns2 = <2T>_Blocksum(Ns,bdim);
    Kill_<2T>(Ns);

    if(max_corr!=NULL)
      <7t>_fun1_max(corr_stack,Ns2->array,bdim[0],length,max_corr);
    else
      <7t>_fun1(corr_stack,Ns2->array,bdim[0],length);

    Kill_<2T>(Ns2);
    */


    bstack1 = Copy_Stack(stack1);
    Stack_Binarize(bstack1);
    bstack2 = Copy_Stack(stack2);
    Stack_Binarize(bstack2);
    int minarea = 128;
    area = Stack_Foreunion_cthr(bstack1,bstack2,32);
    
    Kill_Stack(bstack1);
    Kill_Stack(bstack2);

    if(max_corr!=NULL)
      <7t>_fun1_i2_max(corr_stack,area->array,minarea,length,max_corr);
    else
      <7t>_fun1_i2(corr_stack,area->array,minarea,length);

    Kill_IMatrix(area);
  }

  return <7t>2<2t>(corr_stack,3,corr_width,corr_height,corr_depth);
}


/**
 * Partial normalized correlation.
 */
<2T>* Normcorr_Stack_Part_<4T>(Stack *stack1,Stack *stack2,int std,int start[],int end[])
{
  <3t> *corr_stack = NULL;
  size_t length1,length2,length;
  int corr_width,corr_height,corr_depth;
  int width1,height1,depth1,width2,height2,depth2;
  dim_type bdim[3];
  dim_type dim[3];

  width1 = stack1->width;
  height1 = stack1->height;
  depth1 = stack1->depth;
  width2 = stack2->width;
  height2 = stack2->height;
  depth2 = stack2->depth;

  bdim[0] = (dim_type)width1;
  bdim[1] = (dim_type)height1;
  bdim[2] = (dim_type)depth1;

  dim[0] = width2;
  dim[1] = height2;
  dim[2] = depth2;

  length1 = (size_t)width1*height1*depth1;
  length2 = (size_t)width2*height2*depth2;

  corr_width = end[0]-start[0]+1;
  corr_height = end[1]-start[1]+1;
  corr_depth = end[2]-start[2]+1;
  length = (size_t)corr_width*corr_height*corr_depth;


  <2T>* dstack2 = Get_<6T>_Matrix3(stack2);
  //tz+ 27-Aug-2007
  //<3t> max2 = <2T>_Scale(dstack2);
  //tz++
#ifdef _DEBUG_
  printf("Calculating fsuv ...\n");
#endif
  <2T>* fsuv = <2T>_Blocksum_Part(dstack2,bdim,start,end);

  dim_type sub[3];
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv,sub));
  printf("Calculating fmuv ...\n");
#endif
  <2T>* fmuv = Copy_<2T>(fsuv);
  <2T>_Blockmean_Part(fmuv,dim,bdim,start,end,1);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fmuv,sub));
#endif
  <7t>_sqr(dstack2->array,length2);
#ifdef _DEBUG_
  printf("Calculating fsuv2 ...\n");
#endif
  <2T>* fsuv2 = <2T>_Blocksum_Part(dstack2,bdim,start,end);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv2,sub));
#endif
  Kill_<2T>(dstack2);
#ifdef _DEBUG_
  printf("summed standard deviation of stack2 ...\n");
#endif
  <7t>_fun2(fsuv2->array,fsuv->array,fmuv->array,length);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv2,sub));
#endif
  Kill_<2T>(fsuv);

  Reflect_Stack(stack1,1);
  <2T>* dstack1 = Get_<6T>_Matrix3(stack1);
  //tz+ 27-Aug-2007
  //<3t> max1 = <2T>_Scale(dstack1);
  //tz++
#ifdef _DEBUG_
  printf("Calculating tsuv ...\n");
#endif
  <2T>* tsuv = <2T>_Blocksum_Part(dstack1,dim,start,end);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tsuv,sub));
#endif
  <2T>* tmuv = Copy_<2T>(tsuv);
#ifdef _DEBUG_
  printf("Calculating tmuv ...\n");
#endif
  <2T>_Blockmean_Part(tmuv,bdim,dim,start,end,1);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tmuv,sub));
#endif
  <7t>_sqr(dstack1->array,length1);
#ifdef _DEBUG_
  printf("Calculating tsuv2 ...\n");
#endif
  <2T>* tsuv2 = <2T>_Blocksum_Part(dstack1,dim,start,end);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tsuv2,sub));
#endif
  Kill_<2T>(dstack1);
  Reflect_Stack(stack1,1);
#ifdef _DEBUG_
  printf("summed standard deviation of stack1 ...\n");
#endif
  <7t>_fun2(tsuv2->array,tsuv->array,tmuv->array,length);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(tsuv2,sub));
#endif
  Kill_<2T>(tmuv);
#ifdef _DEBUG_
  printf("the product of the two deviations ...\n");
#endif
  <7t>_mul(fsuv2->array,tsuv2->array,length);
  //debug
#ifdef _DEBUG_
  printf("%f\n",<2T>_Max(fsuv2,sub));
#endif
  Kill_<2T>(tsuv2);
#ifdef _DEBUG_
  printf("Correlate stacks ...");
#endif
  corr_stack = Correlate_Stack_Part_<4T>(stack1,stack2,start,end);

  //debug
#ifdef _DEBUG_
  size_t idx;
#endif

#ifdef _DEBUG_
  printf("%f\n",<7t>_max(corr_stack,corr_width*corr_height*corr_depth,&idx));
#endif
  <7t>_fun3(corr_stack,tsuv->array,fmuv->array,fsuv2->array,length);
#ifdef _DEBUG_
  printf("%f\n",<7t>_max(corr_stack,corr_width*corr_height*corr_depth,&idx));
#endif
  //debug
  //printf("%f\n",<2T>_Max(corr_stack,sub));

  Kill_<2T>(fsuv2);
  Kill_<2T>(fmuv);
  Kill_<2T>(tsuv);

  //std = 0;
  if(std) {
    //<2T>* Ns = <2T>_Ones(dim,3);
    //<2T>* Ns2 = <2T>_Blocksum_Part(Ns,bdim,start,end);
    //Kill_<2T>(Ns);
    int* Ns = Get_Area_Part(bdim,dim,start,end,3);
    //<7t>_fun1(corr_stack,Ns2->array,32,length);
    <7t>_fun1_i2(corr_stack,Ns,32,length);
    free(Ns);
    //Kill_<2T>(Ns2);
  }

  <2T> *result = <7t>2<2t>(corr_stack,3,corr_width,corr_height,corr_depth);
  
  return result;
}

/**
 * Get the best alignment of two stacks. The result is store in offset, which is
 * the offset of stack1 to have the best alignement. It will return the maximum
 * correlation value.
 */
<3t> Align_Stack_<4T>(Stack* stack1, Stack* stack2, int offset[],
		      <3t> *unnorm_maxcorr)
{
  <2T>* corr_stack = Normcorr_Stack_<4T>(stack1, stack2, 3, unnorm_maxcorr);

  dim_t sub[3];
  <3t> max_corr = <2T>_Max(corr_stack,sub);

#ifdef _DEBUG_
  printf("%g\n", max_corr);
  printf("%f\n",*unnorm_maxcorr);
#endif

  int i;
  for(i=0; i<3; i++) {
    offset[i] = (int)(sub[i]);
  }

  Kill_<2T>(corr_stack);

  return max_corr; 
}

<3t> Align_Stack_C_<4T>(Stack* stack1, Stack* stack2, const int config[],
			int offset[], <3t> *unnorm_maxcorr)
{
  <2T>* corr_stack = Normcorr_Stack_<4T>(stack1, stack2, 3, unnorm_maxcorr);

  dim_t sub[3];
  dim_t start[3];
  dim_t end[3];
  int dim1[3];
  int dim2[3];

  dim1[0] = stack1->width;
  dim1[1] = stack1->height;
  dim1[2] = stack1->depth;

  dim2[0] = stack2->width;
  dim2[1] = stack2->height;
  dim2[2] = stack2->depth;

  int k;
  for (k = 0; k < 3; k++) {
    switch (config[k]) {
    case -1:
      start[k] = 0;
      end[k] = imin2(dim1[k]/2, dim2[k]/2);
      break;
    case 0:
      start[k] = imin2(dim1[k] / 2, dim2[k] / 2);
      end[k] = imax2(dim2[k] - 1 + dim1[k] / 2, dim1[k] - 1 + dim2[k] / 2);
      break;
    case 1:
      start[k] = imax2(dim2[k] - 1 + dim1[k] / 2, dim1[k] - 1 + dim2[k] / 2);
      end[k] = dim1[k] + dim2[k] - 2;
      break;
    default:
      start[k] = 0;
      end[k] = dim1[k] + dim2[k] - 2;
      break;
    }
  }

  <3t> max_corr = <2T>_Max_P(corr_stack, start, end, sub);
  
#ifdef _DEBUG_
  printf("%g\n", max_corr);
  printf("%f\n",*unnorm_maxcorr);
#endif

  int i;
  for(i=0; i<3; i++) {
    offset[i] = (int)(sub[i]);
  }

  Kill_<2T>(corr_stack);

  return max_corr;   
}

/**
 * Similar to Align_Stack. But it did alignment by downsampling stack1
 * and stack 2 first. intv is the downsampling interval and fine is the 
 * flag for fine tuning (greater than 0) or not (0 or less). Here is the 
 * detail:
 *   0 : only coarse registration is tried. The stacks will be downsampled.
 *   -1: only coarse registration is tried. The stacks should be downsampled 
 *       and preprocessed already.
 *   1 : both coarse and fine registration will be tried.
 *   2 : offset should be ready from coarse alignment. Only fine registration
 *       is tried.
 */
<3t> Align_Stack_MR_<4T>(Stack* stack1,Stack* stack2,int intv[],int fine,
			 const int *config, int offset[],<3t> *unnorm_maxcorr)
{
  int i;
  int sub[3];
  int stack1_dim[3],stack2_dim[3];
  int start1[3],start2[3],end1[3],end2[3];
  Stack* substack1;
  Stack* substack2;
  <3t> max_value = 0.0;
  Stack *downstack1,*downstack2;
  int sub_dim[3];

  if(fine != 2) { /* coarse alignment specified */
    if(fine < 0) { /* stacks already downsampled */
      downstack1 = stack1;
      downstack2 = stack2;
    } else { /* downsample the stacks */
      downstack1 = Downsample_Stack_Mean(stack1,intv[0],intv[1],intv[2], NULL);
      downstack2 = Downsample_Stack_Mean(stack2,intv[0],intv[1],intv[2], NULL);
      /*
      Pixel_Range* pr = Stack_Range(stack1,0);
      Stack_Sub_Common(downstack1, 0, (int)((pr->minval+pr->maxval)/2));
      pr = Stack_Range(stack2,0);
      Stack_Sub_Common(downstack2, 0, (int)((pr->minval+pr->maxval)/2));
      */
    }

    if (config == NULL) {
      max_value = Align_Stack_<4T>(downstack1, downstack2, sub, unnorm_maxcorr);
    } else {
      max_value = Align_Stack_C_<4T>(downstack1, downstack2, config, sub, 
				     unnorm_maxcorr);
    }

#ifdef _DEBUG_2
    Print_Stack_Info(downstack1);
    Print_Stack_Info(downstack2);
    printf("(%d %d %d)\n",sub[0],sub[1],sub[2]);


    printf("max_value: %g\n", max_value);

    //downstack1 = Downsample_Stack_Mean(stack1,intv[0],intv[1],intv[2]);
    //downstack2 = Downsample_Stack_Mean(stack2,intv[0],intv[1],intv[2]);    
    Write_Stack("../data/downstack1.tif",downstack1);
    Write_Stack("../data/downstack2.tif",downstack2);
    
    stack1_dim[0] = downstack1->width;
    stack1_dim[1] = downstack1->height;
    stack1_dim[2] = downstack1->depth;

    stack2_dim[0] = downstack2->width;
    stack2_dim[1] = downstack2->height;
    stack2_dim[2] = downstack2->depth;
    for(i=0;i<3;i++)
      offset[i] = sub[i];

    //offset[0] = 255;
    //offset[1] = 67;
    //offset[2] = 24;
    printf("(%d %d %d)\n",offset[0],offset[1],offset[2]);

    Stack_Overlap(stack2_dim,stack1_dim,offset,start1,end1,start2,end2);
    for(i=0;i<3;i++) {
      sub_dim[i] = end1[i]-start1[i]+1;
    }
    substack1 = Crop_Stack(downstack1,start1[0],start1[1],start1[2],
			   end1[0]-start1[0]+1,sub_dim[1],end1[2]-start1[2]+1, 
			   NULL);
    substack2 = Crop_Stack(downstack2,start2[0],start2[1],start2[2],
			   end2[0]-start2[0]+1,sub_dim[1],end2[2]-start2[2]+1, 
			   NULL);
    printf("corrcoef: %f\n",Stack_Corrcoef(substack1,substack2));

    Print_Stack_Info(substack1);
    Print_Stack_Info(substack2);

    Write_Stack("../data/substack1.tif",substack1);
    Write_Stack("../data/substack2.tif",substack2);
   
    Kill_Stack(substack1);
    Kill_Stack(substack2);
    exit(1);
#endif

    
#ifdef _DEBUG_2
    stack1_dim[0] = stack1->width;
    stack1_dim[1] = stack1->height;
    stack1_dim[2] = stack1->depth;

    stack2_dim[0] = stack2->width;
    stack2_dim[1] = stack2->height;
    stack2_dim[2] = stack2->depth;

    for(i=0;i<3;i++)
      offset[i] = sub[i]*(intv[i]+1)+(stack1_dim[i]-1)%(intv[i]+1);
    Stack_Overlap(stack2_dim,stack1_dim,offset,start1,end1,start2,end2);

    for(i=0;i<3;i++) {
      sub_dim[i] = end1[i]-start1[i]+1;
    }
   
    if(sub_dim[0]*sub_dim[1]*sub_dim[2]/sizeof(double)*sizeof(<3t>)>134217728) {
      printf("Warning: size too large\n");
      sub_dim[1] /= 2;
    }

    substack1 = Crop_Stack(stack1,start1[0],start1[1],start1[2],
			   end1[0]-start1[0]+1,sub_dim[1],end1[2]-start1[2]+1, 
			   NULL);
    substack2 = Crop_Stack(stack2,start2[0],start2[1],start2[2],
			   end2[0]-start2[0]+1,sub_dim[1],end2[2]-start2[2]+1, 
			   NULL);
    printf("corrcoef: %f\n",Stack_Corrcoef(substack1,substack2));
    Kill_Stack(substack1);
    Kill_Stack(substack2);
    exit(1);
#endif

    if (downstack1 != stack1) {
      Kill_Stack(downstack1);
    }

    if (downstack2 != stack2) {
      Kill_Stack(downstack2);
    }
  }

  stack1_dim[0] = stack1->width;
  stack1_dim[1] = stack1->height;
  stack1_dim[2] = stack1->depth;

  stack2_dim[0] = stack2->width;
  stack2_dim[1] = stack2->height;
  stack2_dim[2] = stack2->depth;

  if(fine != 2) {
    for(i=0; i<3; i++) {
      /* scale the offset to the original space */
      offset[i] = sub[i] * (intv[i]+1);//+(stack1_dim[i]-1)%(intv[i]+1);
    }
  }
 
  if(fine > 0) { /* fine alignment */
    if ((intv[0] == 0) && (intv[1] == 0) && (intv[2] == 0)) {
      /* no fine alignment required */
      return 0.0;
    }

#ifdef _DEBUG_
    printf("Fine alignment ...\n");
#endif

    Stack_Overlap(stack2_dim,stack1_dim,offset,start1,end1,start2,end2);

    /*
    for (i = 0; i < 3; i++) {
      start1[i] -= margin[i];
      end1[i] += margin[i];
      start2[i] -= margin[i];
      end2[i] += margin[i];
    }
    */

    for(i=0; i<3; i++) {
      sub_dim[i] = end1[i] - start1[i] + 1;
    }

    /*
    if(sub_dim[0]*sub_dim[1]*sub_dim[2]/sizeof(double)*sizeof(<3t>)>134217728) {
      printf("Warning: size too large\n");
      sub_dim[1] /= 2;
    }
    */

    substack1 = Crop_Stack(stack1,start1[0],start1[1],start1[2],
			   end1[0]-start1[0]+1,sub_dim[1],end1[2]-start1[2]+1,
			   NULL);
    substack2 = Crop_Stack(stack2,start2[0],start2[1],start2[2],
			   end2[0]-start2[0]+1,sub_dim[1],end2[2]-start2[2]+1,
			   NULL);

#ifdef _DEBUG_2
    Write_Stack("data/substack1.tif",substack1);
    Write_Stack("data/substack2.tif",substack2);
#endif

    int margin[3];
    for (i = 0; i < 3; i++) {
      margin[i] = intv[i] + (intv[i] > 0);
    }

    start1[0] = substack1->width-1-margin[0];
    start1[1] = substack1->height-1-margin[1];
    start1[2] = substack1->depth-1-margin[2];

    for(i=0;i<3;i++) {
      end1[i] = start1[i]+margin[i]*2;
      if (start1[i] < 0) {
	fprintf(stderr, "align failed\n");
	Kill_Stack(substack1);
	Kill_Stack(substack2);
	return 0.0;
      }
    }

    <2T>* corr_stack = Normcorr_Stack_Part_<4T>(substack1,substack2,1,start1,end1);
    
    //<2T>* corr_stack = Normcorr_Stack_<4T>(substack1,substack2,1,NULL);

    Kill_Stack(substack1);
    Kill_Stack(substack2);

    <3t> max_value = <2T>_Max(corr_stack,sub);

#ifdef _DEBUG_
    <2T>_Print(corr_stack);
#endif
    
    Kill_<2T>(corr_stack);

    for(i=0; i<3; i++) { /* adjust the offset */
      offset[i] += sub[i] - margin[i];
    }

#ifdef _DEBUG_    
    printf("%f\n",max_value);

    for(i=0;i<3;i++)
      printf("%d ",offset[i]);
    printf("\n");
#endif

#ifdef _DEBUG_2
    Stack_Overlap(stack2_dim,stack1_dim,offset,start1,end1,start2,end2);
    substack1 = Crop_Stack(stack1,start1[0],start1[1],start1[2],
			   end1[0]-start1[0]+1,end1[1]-start1[1]+1,
			   end1[2]-start1[2]+1, NULL);
    substack2 = Crop_Stack(stack2,start2[0],start2[1],start2[2],
			   end2[0]-start2[0]+1,end2[1]-start2[1]+1,
			   end2[2]-start2[2]+1, NULL);
    Write_Stack("data/substack1.tif",substack1);
    Write_Stack("data/substack2.tif",substack2);
    Kill_Stack(substack1);
    Kill_Stack(substack2);
#endif
  }

  return max_value;
}
</t>

<2T=DMatrix,FMatrix>

<2T>* Ring_Filter_<4T>(double r1, double r2, <2T> *filter)
{
  dim_type wndsize = (dim_type) (r2 + 0.5);
  if (filter == NULL) {
    dim_type dim[3];
    dim[0] = wndsize * 2 + 1;
    dim[1] = dim[0];
    dim[2] = dim[0];
    filter = Make_<2T>(dim, 3);
  }

  dim_type i, j, k;
  size_t offset = 0;;
  double d;
  double coord[3];
  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	coord[0] = i - wndsize;
	coord[1] = j - wndsize;
	coord[2] = k - wndsize;
	d = sqrt(coord[0] * coord[0] + coord[1] * coord[1] + 
		 coord[2] * coord[2]);
	if (d < r1) {
	  filter->array[offset] = -0.5;
	} else if (d <= r2) {
	  filter->array[offset] = 1;
	} else {
	  filter->array[offset] = 0;
	}
	offset++;
      }
    }
  }
  return filter;
}

/* Mexhihat_3D(): 3D Mexican hat filter.
 *
 * Args: sigma - standard deviation of the filter;
 *       filter - output filter. If it is NULL, a new filter will be created.
 *
 * Return: the filter.
 */
<2T>* Mexihat_3D_<4T>(double sigma, <2T> *filter)
{
   dim_type wndsize = (dim_type) sigma * 3;
  if (filter == NULL) {
    dim_type dim[3];
    dim[0] = wndsize * 2 + 1;
    dim[1] = dim[0];
    dim[2] = dim[0];
    filter = Make_<2T>(dim, 3);
  }

  dim_type i, j, k;
  size_t offset = 0;;
  double r;
  double coord[3];
  double sigma2 = sigma * sigma;
  double weight = 0.0;
  
  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	coord[0] = ((double) i) - wndsize;
	coord[1] = ((double) j) - wndsize;
	coord[2] = ((double) k) - wndsize;
	r = coord[0] * coord[0] + coord[1] * coord[1] + 
		 coord[2] * coord[2];
	r = r / sigma2;
	filter->array[offset] = (1.0 - r / 3) * exp(-r / 2);
	weight += fabs(filter->array[offset]);
	offset++;
      }
    }
  }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
    filter->array[idx] /= weight;
  }

  return filter;  
}

/* Gaussian_3D_Filter_<4T>(): 3D Gaussian filter.
 *
 * Args: sigma - standard deviation of the filter;
 *       filter - output filter. If it is NULL, a new filter will be created.
 *
 * Return: the filter.
 */
<2T>* Gaussian_3D_Filter_<4T>(const double *sigma, <2T> *filter)
{
  dim_type i, j, k;
  size_t offset = 0;;
  double r;
  double coord[3];
  double weight = 0.0;

  dim_type wndsize[3];
  double sigma2[3];

  for (i = 0; i < 3; i++) {
    wndsize[i] = (dim_type) (sigma[i] + 0.5) * 3.0;
    sigma2[i] = sigma[i] * sigma[i];
  }
  

  //double cum_sigma = sigma[0] * sigma[1] * sigma[2];

  if (filter == NULL) {
    dim_type dim[3];
    for (i = 0; i < 3; i++) {
      dim[i] = wndsize[i] * 2 + 1;
    }
    if (sigma[2] == 0.0) {
      dim[2] = 1;
    }

    filter = Make_<2T>(dim, 3);
  }
  
  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	coord[0] = ((double) i) - wndsize[0];
	coord[1] = ((double) j) - wndsize[1];
	coord[2] = ((double) k) - wndsize[2];
	r = coord[0] * coord[0] / sigma2[0] + coord[1] * coord[1] / sigma2[1];
        if (sigma[2] > 0.0) {
          r += coord[2] * coord[2] / sigma2[2];
        }
	filter->array[offset] = exp(-r / 2) /*/ cum_sigma*/; 
	if (filter->array[offset] >= 0) {
	  weight += filter->array[offset];
	} 
	offset ++;
      }
    }
  }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
    filter->array[idx] /= weight;
  }

  return filter;  
}

<2T>* Gaussian_3D_Filter_2x_<4T>(const double *sigma, <2T> *filter)
{
  dim_type i, j, k;
  size_t offset = 0;;
  double r;
  double coord[3];
  double weight = 0.0;

  dim_type wndsize[3];
  double sigma2[3];

  for (i = 0; i < 3; i++) {
    wndsize[i] = (dim_type) (sigma[i] + 0.5) * 2.0;
    sigma2[i] = sigma[i] * sigma[i];
  }
  

  //double cum_sigma = sigma[0] * sigma[1] * sigma[2];

  if (filter == NULL) {
    dim_type dim[3];
    for (i = 0; i < 3; i++) {
      dim[i] = wndsize[i] * 2 + 1;
    }
    if (sigma[2] == 0.0) {
      dim[2] = 1;
    }

    filter = Make_<2T>(dim, 3);
  }
  
  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	coord[0] = ((double) i) - wndsize[0];
	coord[1] = ((double) j) - wndsize[1];
	coord[2] = ((double) k) - wndsize[2];
	r = coord[0] * coord[0] / sigma2[0] + coord[1] * coord[1] / sigma2[1];
        if (sigma[2] > 0.0) {
          r += coord[2] * coord[2] / sigma2[2];
        }
	filter->array[offset] = exp(-r / 2) /*/ cum_sigma*/; 
	if (filter->array[offset] >= 0) {
	  weight += filter->array[offset];
	} 
	offset ++;
      }
    }
  }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
    filter->array[idx] /= weight;
  }

  return filter;  
}

static inline double gauss_d2_factor(const int *dim, const double *sigma,
				     double *coord) {
  if (dim[0] == dim[1]) {
    double s = sigma[dim[0]] * sigma[dim[0]];
    return ((coord[dim[0]] * coord[dim[0]]) / s - 1.0) / s;
  } else {
    return coord[dim[0]] * coord[dim[1]] / (sigma[dim[0]] * sigma[dim[0]]) /
      (sigma[dim[1]] * sigma[dim[1]]);
  }
}

<2T>* Gaussian_3D_D2_Filter_<4T>(const double *sigma, int dim[2], <2T> *filter)
{
  dim_type i, j, k;
  size_t offset = 0;;
  double r;
  double coord[3];
  double weight = 0.0;

  dim_type wndsize[3];
  double sigma2[3];

  for (i = 0; i < 3; i++) {
    wndsize[i] = (dim_type) (sigma[i] + 0.5) * 3.0;
    sigma2[i] = sigma[i] * sigma[i];
  }

  double cum_sigma = sigma[0] * sigma[1] * sigma[2];

  if (filter == NULL) {
    dim_type tdim[3];
    for (i = 0; i < 3; i++) {
      tdim[i] = wndsize[i] * 2 + 1;
    }
    filter = Make_<2T>(tdim, 3);
  }

  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	coord[0] = ((double) i) - wndsize[0];
	coord[1] = ((double) j) - wndsize[1];
	coord[2] = ((double) k) - wndsize[2];
	
	r = coord[0] * coord[0] / sigma2[0] + coord[1] * coord[1] / sigma2[1] + 
		 coord[2] * coord[2] / sigma2[2];
	filter->array[offset] = exp(-r / 2) / cum_sigma * 
	  gauss_d2_factor(dim, sigma, coord); 
	weight += fabs(filter->array[offset]);
	offset++;
      }
    }
  }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
    filter->array[idx] /= weight;
  }

  return filter;  
}

<2T>* Geo3d_Scalar_Field_To_Filter_<4T>(const Geo3d_Scalar_Field *field)
{
  coordinate_3d_t corners[2];
  Geo3d_Scalar_Field_Boundbox(field, corners); 
  dim_type dim[3];
  int i;
  for (i = 0; i < 3; i++) {
    dim[i] = corners[1][i] - corners[0][i] + 1;
  }

  <2T> *filter = Make_<2T>(dim, 3);

  Geo3d_Scalar_Field *field2 = Copy_Geo3d_Scalar_Field(field);
  Geo3d_Scalar_Field_Translate(field2, -corners[0][0], -corners[0][1], 
			       -corners[0][2]);

  Stack stack = Stack_View_<2T>(filter);
  Geo3d_Scalar_Field_Draw_Stack(field2, &(stack), NULL, NULL);

  Kill_Geo3d_Scalar_Field(field2);

  return filter;
}

#define FILTER_STACK_<4T>(substack_array)				\
   for (k = 0; k < stack->depth; k++) {					\
     printf("%3d", k);							\
     fflush(stdout);							\
     front = k - filter_offset[2];					\
     for (j = 0; j < stack->height; j++) {				\
       top = j - filter_offset[1];					\
       for (i = 0; i < stack->width; i++) {				\
	 left = i - filter_offset[0];					\
	 Crop_Stack(stack, left, top, front,				\
		    filter->dim[0], filter->dim[1], filter->dim[2],	\
		    substack);						\
	 out->array[offset] = 0;					\
	 for (m = 0; m < filter_length; m++) {				\
	   out->array[offset] += filter->array[m] * (<3t>) (substack_array[m]); \
	 }								\
	 offset++;							\
       }								\
     }									\
									\
     printf("\b\b\b");							\
   }

/* Filter_Stack_<4T>(): Stack filtering.
 *
 * Notice: the caller is responsible for clearing up the output.
 *
 * Args: stack - input stack;
 *       filter - stack filter, which is a 3D <3t> matrix;
 *       out - filtered stack, which is a 3D <3t> matrix. If it is NULL, a new
 *             <3t> matrix will be created.
 *
 * Return: filtered stack.
 */
<2T>* Filter_Stack_<4T>(const Stack *stack, const <2T> *filter, <2T> *out)
{
  int i, j, k, m;
  int left, top, front;
  int filter_offset[3];
  int filter_length = matrix_size(filter->dim, filter->ndim);
  size_t offset = 0;

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  if (out == NULL) {
    out = Make_<2T>(dim, 3);
  }

  for (i = 0; i < 3; i++) {
    filter_offset[i] = (filter->dim[i] - 1) / 2;
  }

  Stack *substack = Make_Stack(stack->kind, filter->dim[0], filter->dim[1],
			       filter->dim[2]);
  DEFINE_SCALAR_ARRAY_ALL(substack, substack);

  switch (stack->kind) {
  case GREY:
    FILTER_STACK_<4T>(substack_grey);
    break;
  case GREY16:
    FILTER_STACK_<4T>(substack_grey16);
    break;
  case FLOAT32:
    FILTER_STACK_<4T>(substack_float32);
    break;
  case FLOAT64:
    FILTER_STACK_<4T>(substack_float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
    
  Kill_Stack(substack);

  return out;
}

#define FILTER_STACK_FAST_<4T>(stack_array)				\
   for (k = pad_depth_offset; k < pad_depth; k++) {			\
     for (j = 0; j < pad_height_offset; j++) {				\
       for (i = 0; i < fftw_real_width; i++) {				\
	 out->array[offset2++] = 0.0;					\
       }								\
     }									\
									\
     for (j = pad_height_offset; j < pad_height; j++) {			\
       for (i = 0; i < pad_width_offset; i++) {				\
	 out->array[offset2++] = 0.0;					\
       }								\
									\
       for (i = pad_width_offset; i < pad_width; i++) {			\
	 out->array[offset2++] = (<6t>) (stack_array[offset++]);	\
       }								\
									\
       for (i = pad_width; i < fftw_real_width; i++) {			\
	 out->array[offset2++] = 0.0;					\
       }								\
     }									\
   }

/* Filter_Stack_Fast_<4T>(): Fast stack filtering.
 *
 * Notice: This function does the almost same thing as Filter_Stack(), but it is
 *         supposed to be faster. The disadvantage is that it requires more
 *         memory. Another difference is that the output may be padded.
 *
 * Args: stack - input stack;
 *       filter - stack filter, which is a 3D <3t> matrix;
 *       out - filtered stack, which is a 3D <3t> matrix. If it is NULL, a new
 *             <3t> matrix will be created.
 *       pad - [out] has the same size as [stack] if it is 0 and padded size if
 *             it is 1. Other values are undefined.
 *      
 * Return: filtered stack.
 */   
<2T>* Filter_Stack_Fast_<4T>(const Stack *stack, const <2T> *filter, <2T> *out, int pad)
{
<3t=double>
#if defined(HAVE_LIBFFTW3)
</t>
<3t=float>
#if defined(HAVE_LIBFFTW3F)
</t>
  if (stack == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (filter == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  int pad_width = stack->width + filter->dim[0] -1; /* padded width */
  int pad_height = stack->height + filter->dim[1] -1; /* padded height */
  int pad_depth = stack->depth+ filter->dim[2] - 1; /* padded depth*/

  int fftw_width = R2C_LENGTH(pad_width); /* width of the fft result */
  int fftw_real_width = fftw_width * 2; /* length of the real array */
  int i, j, k;

  <6t> *filter_in = NULL;

  dim_type dim[3];
  dim[0] = fftw_real_width;
  dim[1] = pad_height;
  dim[2] = pad_depth;

  if (out == NULL) {
    out = Make_<2T>(dim, 3);
  }
  out->dim[0] = pad_width;
  
  <5t>_complex *stack_ft = (<5t>_complex *) out->array;
  <5t>_complex *filter_ft = <5t>_malloc_r2c_3d(pad_depth, pad_height, pad_width);

  filter_in = (<6t> *) filter_ft;

  size_t offset = 0;
  size_t offset2 = 0;

  int pad_width_offset = pad_width - stack->width;
  int pad_height_offset = pad_height - stack->height;
  int pad_depth_offset = pad_depth - stack->depth;

  /* initialize padded stack */
  for (k = 0; k < pad_depth_offset; k++) {
    for (j = 0; j < pad_height; j++) {
      for (i = 0; i < fftw_real_width; i++) {
	out->array[offset2++] = 0.0;
      }
    }
  }

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  switch (stack->kind) {
  case GREY:
    FILTER_STACK_FAST_<4T>(array_grey);
    break;
  case GREY16:
    FILTER_STACK_FAST_<4T>(array_grey16);
    break;
  case FLOAT32:
    FILTER_STACK_FAST_<4T>(array_float32);
    break;
  case FLOAT64:
    FILTER_STACK_FAST_<4T>(array_float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

#if 0
  Stack *test_stack = Scale_<6T>_Stack(out->array, fftw_real_width, out->dim[1],
					 out->dim[2], GREY);
  Write_Stack("../data/test_stack_filter.tif", test_stack);
  Kill_Stack(test_stack);
#endif

  /* initialize padded filter */
  offset = 0;
  offset2 = 0;

  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
	filter_in[offset2++] = (<6t>) (filter->array[offset++]);
      }
      
      for (i = filter->dim[0]; i < fftw_real_width; i++) {
	filter_in[offset2++] = 0.0;
      }	
    }
    for (j = filter->dim[1]; j < pad_height; j++) {
      for (i = 0; i < fftw_real_width; i++) {
	filter_in[offset2++] = 0.0;
      }
    }
  }

  for (k = filter->dim[2]; k < pad_depth; k++) {
    for (j = 0; j < pad_height; j++) {
      for (i = 0; i < fftw_real_width; i++) {
	filter_in[offset2++] = 0.0;
      }
    }
  }

#if 0  
  Stack *test_stack = Scale_<6T>_Stack(filter_in, out->dim[0], out->dim[1],
					 out->dim[2], GREY);
  Write_Stack("../data/test_stack_filter.tif", test_stack);
  Kill_Stack(test_stack);
#endif

  <5t>_plan p = <5t>_plan_dft_r2c_3d(pad_depth, pad_height, pad_width, 
				     out->array, stack_ft, FFTW_ESTIMATE);
  <5t>_execute(p);
  <5t>_destroy_plan(p);

  if (pad_depth == 1) {
    p = <5t>_plan_dft_r2c_2d(pad_height, pad_width,
        filter_in, filter_ft, FFTW_ESTIMATE);
  } else {
    p = <5t>_plan_dft_r2c_3d(pad_depth, pad_height, pad_width,
        filter_in, filter_ft, FFTW_ESTIMATE);
  }

  <5t>_execute(p);
  <5t>_destroy_plan(p);

  int length = fftw_width * pad_height * pad_depth;

  <5t>_conjg_array(filter_ft,length);

  <5t>_cmul_array(stack_ft,filter_ft,length);

  <5t>_free(filter_ft);
  

  length = pad_width * pad_height * pad_depth;

  if (pad_depth == 1) {
    p = <5t>_plan_dft_c2r_2d(pad_height, pad_width, 
        stack_ft, out->array, FFTW_ESTIMATE);
  } else {
    p = <5t>_plan_dft_c2r_3d(pad_depth, pad_height, pad_width, 
        stack_ft, out->array, FFTW_ESTIMATE);
  }
  <5t>_execute(p);
  <5t>_destroy_plan(p);

  <5t>_pack_c2r_result(out->array, pad_width, pad_depth * pad_height);
  <7t>_divc(out->array, length, length);

#if 0 
  Stack *test_stack = Scale_<6T>_Stack(out->array, out->dim[0], out->dim[1],
					 out->dim[2], GREY);
  Write_Stack("../data/test_stack_filter.tif", test_stack);
  Kill_Stack(test_stack);
#endif

  /* crop result */
  if (pad == 0) {
    size_t copy_width = sizeof(<6t>) * stack->width;
    int copy_width_offset = pad_width_offset / 2;
    int copy_height_offset = pad_height_offset / 2;
    int copy_depth_offset = pad_depth_offset / 2;
    
    offset2 = 0;
    offset = (size_t) pad_width * pad_height * copy_depth_offset + 
      pad_width * copy_height_offset + copy_width_offset;

    for (k = 0; k < stack->depth; k++) {
      for (j = 0;  j < stack->height; j++) {
	memmove(&(out->array[offset2]), &(out->array[offset]), copy_width);
	offset2 += stack->width;
	offset += pad_width;
      }
      offset += pad_width * pad_height_offset;
    }

    out->dim[0] = stack->width;
    out->dim[1] = stack->height;
    out->dim[2] = stack->depth;
  }

  return out;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

<2T>* Filter_Stack_Slice_<4T>(const Stack *stack, const <2T> *filter, <2T> *out
)
{
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;

  if (out == NULL) {
    out = Make_<2T>(dim, 3);
  }

  int z;
  size_t area = stack->width * stack->height;
  Stack image;
  image.depth = 1;
  image.width = stack->width;
  image.height = stack->height;
  image.kind = stack->kind;
  image.array = NULL;
  /*
  <2T> out_slice;
  out_slice.ndim = 3;
  out_slice.dim[0] = stack->width;
  out_slice.dim[1] = stack->height;
  out_slice.dim[2] = 1;
  */

  for (z = 0; z < stack->depth; ++z) {
    image.array = stack->array + area * z;
    //out_slice.array = out->array + area * z;
    //Filter_Image_<4T>(&image, filter, &out_slice);
    <2T>* out_slice = Filter_Stack_Fast_<4T>(&image, filter, NULL, 0);
    memcpy(out->array + area * z, out_slice->array, area * sizeof(<6t>));
    Kill_<2T>(out_slice);
  }

  return out;
}

<2T>* Smooth_Stack_Fast_<4T>(const Stack *stack, int wx, int wy, int wz, 
    <2T> *out)
{
  /*
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  <2T> *dm = Make_<2T>(dim, 3);
  size_t volume = Stack_Voxel_Number(stack);
  Image_Array ima;
  ima.array = stack->array;
  switch (stack->kind) {
    case GREY:
      for (size_t i = 0; i < volume; ++i) {
        dm->array[i] = stack->array[i];
      }
      break;
    case GREY16:
      for (size_t i = 0; i < volume; ++i) {
        dm->array[i] = ima.array16[i];
      }
      break;
    default:
      break;
  }
  */
  <2T> *dm = Get_<6T>_Matrix3(stack);

  dim_type bdim[3];
  bdim[0] = wx;
  bdim[1] = wy;
  bdim[2] = wz;
  <2T> *sm1 = <2T>_Blockmean(dm, bdim, 0);
  Kill_<2T>(dm);
  <2T> *sm2 = <2T>_Blockmean(sm1, bdim, 0);
  Kill_<2T>(sm1);

  return sm2;
}

<2T>* Filter_Stack_Block_<4T>(const Stack *stack, const <2T> *filter, 
			     <2T> *out)
{
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;

  if (out == NULL) {
    out = Make_<2T>(dim, 3);
    <2T>_Set_Zero(out);
  }
  
  int margin[3];
  int i;
  for (i = 0; i < 3; i++) {
    margin[i] = filter->dim[i] / 2;
  }
  int block_width = stack->width / 2 + margin[0] + 1;
  int block_height = stack->height / 2 + margin[1] + 1;

  Stack *substack = Make_Stack(stack->kind, block_width, block_height, 
			       stack->depth);

  Crop_Stack(stack, 0, 0, 0, block_width, block_height, stack->depth, 
	     substack);

  dim_type src_offset[3] = {0, 0, 0};
  dim_type des_offset[3] = {0, 0, 0};
  
  <2T> *subout = Filter_Stack_Fast_<4T>(substack, filter, NULL, 0);

  <2T>_Copy_Block(out, des_offset, subout, src_offset);

  
  Crop_Stack(stack, stack->width - block_width, 0,
	     0, block_width, block_height, stack->depth, substack);
  Filter_Stack_Fast_<4T>(substack, filter, subout, 0);
  des_offset[0] = stack->width - block_width + margin[0];
  des_offset[1] = 0;
  src_offset[0] = margin[0];
  src_offset[1] = 0;
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
  
  Crop_Stack(stack, 0, stack->height - block_height, 
	     0, block_width, block_height, stack->depth, substack);
  Filter_Stack_Fast_<4T>(substack, filter, subout, 0);
  des_offset[0] = 0;
  des_offset[1] = stack->height - block_height + margin[1];
  src_offset[0] = 0;
  src_offset[1] = margin[1];
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
    
  Crop_Stack(stack, stack->width - block_width, stack->height - block_height, 
	     0, block_width, block_height, stack->depth, substack);
  Filter_Stack_Fast_<4T>(substack, filter, subout, 0);
  des_offset[0] = stack->width - block_width + margin[0];
  des_offset[1] = stack->height - block_height + margin[1];
  src_offset[0] = margin[0];
  src_offset[1] = margin[1];
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
  
  Kill_<2T>(subout);
  Kill_Stack(substack);

  return out;
}

void Correct_Filter_Stack_<4T>(<2T> *filter, <2T> *stack)
{
  size_t length = Matrix_Size(filter->dim, filter->ndim);

  /* only odd window size is allowed */
  ASSERT(length % 2 != 0, "Unsupported matrix size.");
  ASSERT((filter->dim[0] <= stack->dim[0]) && 
	 (filter->dim[1] <= stack->dim[1]) &&
	 (filter->dim[2] <= stack->dim[2]), "Stack too small.");

  int edge_offset[3];
  edge_offset[0] = filter->dim[0] / 2;
  edge_offset[1] = filter->dim[1] / 2;
  edge_offset[2] = filter->dim[2] / 2;

  size_t idx;
  for (idx = 0; idx < length; idx++) {
    if (filter->array[idx] < 0.0) {
      filter->array[idx] = -filter->array[idx];
    }
  }

  /* alloc <tmp_stack> */
  Stack *tmp_stack = Make_Stack(GREY, filter->dim[0], filter->dim[1], 
				filter->dim[2]);
  One_Stack(tmp_stack);

  /* alloc <corr_tmpl> */
  <2T> *corr_tmpl = Filter_Stack_Fast_<4T>(tmp_stack, filter, NULL, 0);

  <3t> center_response = 
     corr_tmpl->array[edge_offset[2] * filter->dim[0] * filter->dim[1] +
		      edge_offset[1] * filter->dim[0] + edge_offset[0]];
  for (idx = 0; idx < length; idx++) {
    corr_tmpl->array[idx] /= center_response;
  }

  int i, j, k;
  i = j = k = 0;

  int tmpl_offset = 0;
  int offset = 0;
  BOOL safe_x, safe_y, safe_z;
  safe_y = FALSE;
  safe_z = FALSE;
  while (k < stack->dim[2]) {
    j = 0;
    while (j < stack->dim[1]) {     
      i = 0;
      while (i < stack->dim[0]) {
	if (corr_tmpl->array[tmpl_offset] != 0.0) {
	  stack->array[offset] /= corr_tmpl->array[tmpl_offset];
	}
	i++;
	safe_x = (i > edge_offset[0]) && (i < stack->dim[0] - edge_offset[0]);

	/* skip safe zone */
	if ((i == edge_offset[0]) && safe_y && safe_z) {
	  i = stack->dim[0] - edge_offset[0];
	  offset += stack->dim[0] - edge_offset[0] - edge_offset[0] + 1;
	  tmpl_offset += 2;
	} else {
	  offset++;
	  if (!safe_x) {
	    tmpl_offset++;
	  }
	}
      }
      j++;
      /* safe zone along y (j==edge_offset[1] is not included for programming 
	 simplicity) */
      safe_y = (j > edge_offset[1]) && (j < stack->dim[1] - edge_offset[1]);
      if (safe_y) {
	tmpl_offset -= corr_tmpl->dim[0];
      }
    }

    k++;
    /* safe zone along z (k==edge_offset[2] is not included for programming 
       simplicity) */
    safe_z = (k > edge_offset[2]) && (k < stack->dim[2] - edge_offset[2]);
    if (safe_z) {
      tmpl_offset -= corr_tmpl->dim[0] * corr_tmpl->dim[1];
    }
  }
  
  /* free <tmp_stack> */
  Kill_Stack(tmp_stack);

  /* free <corr_tmpl> */
  Kill_<2T>(corr_tmpl);
}


#if 0
<2T>* El_Stack_<4T>(const Stack *stack, const double *scale)
{
  <2T> *filter = Gaussian_3D_Filter_<4T>(scale, NULL);
  <2T> *f = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);

  Kill_<2T>(filter);

  <2T> *result2 = <2T>_Partial_Diff(f, 0, NULL);

  <2T> *result = <2T>_Partial_Diff(result2, 0, NULL);

  <2T>_Partial_Diff(f, 1, result2);
  <2T> *result3 = <2T>_Partial_Diff(result2, 1, NULL);

  <2T>_Add(result, result3);

  <2T>_Partial_Diff(f, 2, result2);
  <2T>_Partial_Diff(result2, 2, result3);

  <2T>_Add(result, result3);

  <2T>_Negative(result);

  <2T> *result5 = Copy_<2T>(result);

  <2T>_Partial_Diff(f, 0, result2);
  <2T>_Partial_Diff(result2, 0, result);
  <2T>_Partial_Diff(f, 1, result2);
  <2T> *result4 = <2T>_Partial_Diff(result2, 1, NULL);
  <2T>_Mul(result, result4);

  <2T>_Partial_Diff(f, 2, result2);
  <2T>_Partial_Diff(result2, 2, result3);
  <2T>_Mul(result3, result4); 

  <2T>_Add(result, result3);

  <2T>_Partial_Diff(f, 0, result2);
  <2T>_Partial_Diff(result2, 0, result3);
  <2T>_Partial_Diff(f, 2, result2);
  <2T>_Partial_Diff(result2, 2, result4);
  <2T>_Mul(result3, result4); 

  <2T>_Add(result, result3);

  <2T>_Partial_Diff(f, 0, result2);
  <2T>_Partial_Diff(result2, 1, result3);
  <2T>_Sqr(result3);

  <2T>_Sub(result, result3);

  <2T>_Partial_Diff(f, 1, result2);
  <2T>_Partial_Diff(result2, 2, result3);
  <2T>_Sqr(result3);

  <2T>_Sub(result, result3);

  <2T>_Partial_Diff(f, 0, result2);
  <2T>_Partial_Diff(result2, 2, result3);
  <2T>_Sqr(result3);

  <2T>_Sub(result, result3);

  <2T>_Sqrt(result);

  <2T>_Div(result5, result);

  //<2T>_Max2(result, result5);
  
  <2T>_Threshold(result5, 0.0); 

  Kill_<2T>(f);
  Kill_<2T>(result2);
  Kill_<2T>(result3);
  Kill_<2T>(result4);
  //  Kill_<2T>(result5);

  return result5;
}
#endif

#if 1 /* differential version */
<2T>* El_Stack_<4T>(const Stack *stack, const double *scale, <2T> *result)
{
  <2T> *filter = Gaussian_3D_Filter_<4T>(scale, NULL);
  <2T> *f = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  
  Correct_Filter_Stack_<4T>(filter, f);

  Kill_<2T>(filter);

  <2T> *Ix = <2T>_Partial_Diff(f, 0, result);
  <2T> *Ixx = <2T>_Partial_Diff(Ix, 0, NULL);
  <2T> *Ixy = <2T>_Partial_Diff(Ix, 1, NULL);
  <2T> *Ixz = <2T>_Partial_Diff(Ix, 2, NULL);

  <2T> *Iy = <2T>_Partial_Diff(f, 1, Ix);
  <2T> *Iyy = <2T>_Partial_Diff(Iy, 1, NULL);
  <2T> *Iyz = <2T>_Partial_Diff(Iy, 2, NULL);

  <2T> *Iz = <2T>_Partial_Diff(f, 2, Iy);
  <2T> *Izz = <2T>_Partial_Diff(Iz, 2, f);

  if (result == NULL) {
    result = Ix;
  }
  <2T>_Eigen3_Solution_Score(Ixx, Iyy, Izz, Ixy, Ixz, Iyz, result);

  /* normalize */
  /*
  dim_t length = Matrix_Size(result->dim, result->ndim);
  dim_t i;
 
  for (i = 0; i < length; i++) {
    if (f->array[i] <= 0.0) {
      result->array[i] = 0.0;
    } else {
      result->array[i] /= f->array[i];
    }
  }
  */
  Kill_<2T>(Ixx);
  Kill_<2T>(Iyy);
  Kill_<2T>(Izz);
  Kill_<2T>(Ixy);
  Kill_<2T>(Ixz);
  Kill_<2T>(Iyz);
  //Kill_<2T>(f);

  return result;
}
#endif

#if 0 /* filter version */
<2T>* El_Stack_<4T>(const Stack *stack, const double *scale, <2T> *result)
{
  <2T> *corr_filter = Gaussian_3D_Filter_<4T>(scale, NULL);

  int dim[2];
  dim[0] = 0;
  dim[1] = 0;
  <2T> *filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Ixx = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Ixx);    
  Kill_<2T>(filter);

  dim[0] = 0;
  dim[1] = 1;
  filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Ixy = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Ixy);    

#ifdef _DEBUG_2
  Stack *out = Scale_<6T>_Stack(filter->array, filter->dim[0], filter->dim[1],
				filter->dim[2], GREY16);
  Write_Stack("../data/test2.tif", out);
  Kill_Stack(out);
#endif


  Kill_<2T>(filter);

  dim[0] = 0;
  dim[1] = 2;
  filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Ixz = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Ixz);    
  Kill_<2T>(filter);

  dim[0] = 1;
  dim[1] = 1;
  filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Iyy = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Iyy);    
  Kill_<2T>(filter);

  dim[0] = 1;
  dim[1] = 2;
  filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Iyz = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Iyz);    
  Kill_<2T>(filter);

  dim[0] = 2;
  dim[1] = 2;
  filter = Gaussian_3D_D2_Filter_<4T>(scale, dim, NULL);
  <2T> *Izz = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(corr_filter, Izz);    
  Kill_<2T>(filter);

  Kill_<2T>(corr_filter);

  result = <2T>_Eigen3_Solution_Score(Ixx, Iyy, Izz, Ixy, Ixz, Iyz, result);

  Kill_<2T>(Ixx);
  Kill_<2T>(Iyy);
  Kill_<2T>(Izz);
  Kill_<2T>(Ixy);
  Kill_<2T>(Ixz);
  Kill_<2T>(Iyz);

  return result;
}
#endif

<2T>* El_Stack_L_<4T>(const Stack *stack, const double *scale, <2T> *out)
{
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;

  if (out == NULL) {
    out = Make_<2T>(dim, 3);
    <2T>_Set_Zero(out);
  }
  
  /* estimate margin size */
  /* alloc <filter> */
  <2T> *filter = Gaussian_3D_Filter_<4T>(scale, NULL);
  int margin[3];
  int i;
  for (i = 0; i < 3; i++) {
    margin[i] = filter->dim[i] / 2;
  }
  int block_width = stack->width / 2 + margin[0] + 1;
  int block_height = stack->height / 2 + margin[1] + 1;
  /* free <filter> */
  Kill_<2T>(filter);

  /* alloc <substack> */
  Stack *substack = Make_Stack(stack->kind, block_width, block_height, 
			       stack->depth);

  Crop_Stack(stack, 0, 0, 0, block_width, block_height, stack->depth, 
	     substack);

  dim_type src_offset[3] = {0, 0, 0};
  dim_type des_offset[3] = {0, 0, 0};
  
  /* alloc <subout> */
  <2T> *subout = El_Stack_<4T>(substack, scale, NULL);
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
  
  Crop_Stack(stack, stack->width - block_width, 0,
	     0, block_width, block_height, stack->depth, substack);
  El_Stack_<4T>(substack, scale, subout);
  des_offset[0] = stack->width - block_width + margin[0];
  des_offset[1] = 0;
  src_offset[0] = margin[0];
  src_offset[1] = 0;
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
  
  Crop_Stack(stack, 0, stack->height - block_height, 
	     0, block_width, block_height, stack->depth, substack);
  El_Stack_<4T>(substack, scale, subout);
  des_offset[0] = 0;
  des_offset[1] = stack->height - block_height + margin[1];
  src_offset[0] = 0;
  src_offset[1] = margin[1];
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
    
  Crop_Stack(stack, stack->width - block_width, stack->height - block_height, 
	     0, block_width, block_height, stack->depth, substack);
  El_Stack_<4T>(substack, scale, subout);
  des_offset[0] = stack->width - block_width + margin[0];
  des_offset[1] = stack->height - block_height + margin[1];
  src_offset[0] = margin[0];
  src_offset[1] = margin[1];
  <2T>_Copy_Block(out, des_offset, subout, src_offset);
  
  /* free <subout> */
  Kill_<2T>(subout);

  /* free <substack> */
  Kill_Stack(substack);

  return out;  
}

#if 0
<2T>* El_Stack_<4T>(const Stack *stack, const double *scale)
{
  <2T> *filter = Gaussian_3D_Filter_<4T>(scale, NULL);
  <2T> *f = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);

  Kill_<2T>(filter);

  <2T> *Ix = <2T>_Partial_Diff(f, 0, NULL);
  <2T> *Ixx = <2T>_Partial_Diff(Ix, 0, NULL);
  <2T> *Ixy = <2T>_Partial_Diff(Ix, 1, NULL);
  <2T> *Ixz = <2T>_Partial_Diff(Ix, 2, NULL);

  <2T> *Iy = <2T>_Partial_Diff(f, 1, NULL);
  <2T> *Iyy = <2T>_Partial_Diff(Iy, 1, NULL);
  <2T> *Iyz = <2T>_Partial_Diff(Iy, 2, NULL);

  <2T> *Iz = <2T>_Partial_Diff(f, 2, NULL);
  <2T> *Izz = <2T>_Partial_Diff(Iz, 2, f);

  <2T> *result = <2T>_Eigen3_Curvature(Ixx, Iyy, Izz, Ix, Iy, Iz, NULL);

  Kill_<2T>(Ix);
  Kill_<2T>(Iy);
  Kill_<2T>(Iz);
  Kill_<2T>(Ixx);
  Kill_<2T>(Iyy);
  Kill_<2T>(Izz);
  Kill_<2T>(Ixy);
  Kill_<2T>(Ixz);
  Kill_<2T>(Iyz);

  return result;
}
#endif

Stack *Stack_Line_Paint_<4T>(const Stack *stack, double *sigma, int option)
{
  double default_sigma[3] = {1.0, 1.0, 1.0};
  if (sigma == NULL) {
    sigma = default_sigma;
  }

  <2T> *filter = Gaussian_3D_Filter_<4T>(sigma, NULL);
  <2T> *f = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  Correct_Filter_Stack_<4T>(filter, f);

  Kill_<2T>(filter);

  <2T> *Ix = <2T>_Partial_Diff(f, 0, NULL);
  <2T> *Ixx = <2T>_Partial_Diff(Ix, 0, NULL);
  <2T> *Ixy = <2T>_Partial_Diff(Ix, 1, NULL);
  <2T> *Ixz = <2T>_Partial_Diff(Ix, 2, NULL);

  <2T> *Iy = <2T>_Partial_Diff(f, 1, Ix);
  <2T> *Iyy = <2T>_Partial_Diff(Iy, 1, NULL);
  <2T> *Iyz = <2T>_Partial_Diff(Iy, 2, NULL);

  <2T> *Iz = <2T>_Partial_Diff(f, 2, Iy);
  <2T> *Izz = <2T>_Partial_Diff(Iz, 2, f);

  Stack *color_stack = Make_Stack(FCOLOR, f->dim[0], f->dim[1], f->dim[2]);
  
  <4t>color_t *stack_array = (<4t>color_t*) color_stack->array;

  size_t length = Matrix_Size(Ixx->dim, Ixx->ndim);
  size_t i;
 
  for (i = 0; i < length; i++) {
    if (Ixx->array[i] + Iyy->array[i] + Izz->array[i] < 0.0) {
      /* for steerable optimization (turned out to be wrong) */
      if (option == 2) {
	double alpha = -0.5;
	Ixy->array[i] *= 1-alpha;
	Iyz->array[i] *= 1-alpha;
	Ixz->array[i] *= 1-alpha;
	double fxx = Ixx->array[i] + 
	  (Iyy->array[i] + Izz->array[i]) * alpha / 2.0;
	double fyy = Iyy->array[i] + 
	  (Ixx->array[i] + Izz->array[i]) * alpha / 2.0;
	Izz->array[i] += (Iyy->array[i] + Ixx->array[i]) * alpha / 2.0;
	Ixx->array[i] = fxx;
	Iyy->array[i] = fyy;  
      }    
      /******************/

      double coeff[3];
      coeff[0] = -Ixx->array[i] - Iyy->array[i] - Izz->array[i];
      coeff[1] = Ixx->array[i] * Iyy->array[i] + 
	Iyy->array[i] * Izz->array[i] +
	Ixx->array[i] * Izz->array[i] - Iyz->array[i] * Iyz->array[i] - 
	Ixy->array[i] * Ixy->array[i] - Ixz->array[i] * Ixz->array[i];
      coeff[2] = -Ixx->array[i] * Iyy->array[i] * Izz->array[i] - 
	2.0 * Ixy->array[i] * Iyz->array[i] * Ixz->array[i] +
	Ixx->array[i] * Ixz->array[i] * Ixz->array[i] + 
	Izz->array[i] * Ixy->array[i] * Ixy->array[i] + 
	Iyy->array[i] * Iyz->array[i] * Iyz->array[i];

      /*
      stack_array[i][0] = coeff[0];
      stack_array[i][1] = coeff[1];
      stack_array[i][2] = coeff[2];
      */
      
      if (Solve_Cubic(1.0, coeff[0], coeff[1], coeff[2], coeff) > 0){
	double tmp;
	if (coeff[0] < coeff[1]) {
	  SWAP2(coeff[0], coeff[1], tmp);
	  if (coeff[0] < coeff[2]) {
	    SWAP2(coeff[0], coeff[2], tmp);
	  }
	}

	if (coeff[1] < coeff[2]) {
	  SWAP2(coeff[1], coeff[2], tmp);
	}
	
	if (option == 2) {
	  stack_array[i][0] = 0.0;
	} else {
	  if (coeff[0] >= 0) {
	    stack_array[i][0] = coeff[0];
	  } else {
	    stack_array[i][0] = -coeff[0];
	  }
	}
	  
	stack_array[i][1] = -coeff[1];
	stack_array[i][2] = -coeff[2];
	
      } else {
	stack_array[i][0] = 0;
	stack_array[i][1] = 0;
	stack_array[i][2] = 0;
      }
      
    } else {
      stack_array[i][0] = 0;
      stack_array[i][1] = 0;
      stack_array[i][2] = 0;
    }
  }

  Stack* out = Scale_<6T>_Stack((<6t>*) stack_array, stack->width, 
				stack->height, stack->depth * 3, GREY);

  out->depth = stack->depth;
  out->kind = COLOR;

  Kill_Stack(color_stack);

  Kill_<2T>(Ixx);
  Kill_<2T>(Iyy);
  Kill_<2T>(Izz);
  Kill_<2T>(Ixy);
  Kill_<2T>(Ixz);
  Kill_<2T>(Iyz);

  return out;
}

<2T>* Stack_Pixel_Feature_<4T>(const Stack *stack, const double *scale, 
			       const Object_3d *pts, <2T> *result)
{
  dim_type dim[2];
  dim[0] = 13;
  dim[1] = pts->size;

  if (result == NULL) {
    result = Make_<2T>(dim, 2);
  }
  
  size_t *indices = (size_t*) malloc(sizeof(size_t) * pts->size);
  Object_3d_Indices(pts, stack->width, stack->height, stack->depth, indices);

  /* alloc <filter> */
  <2T> *filter = Gaussian_3D_Filter_<4T>(scale, NULL);
  /* alloc <f> */
  <2T> *f = Filter_Stack_Fast_<4T>(stack, filter, NULL, 0);
  
  Correct_Filter_Stack_<4T>(filter, f);

  int i;
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i] = f->array[indices[i]];
  }

  /* free <filter> */
  Kill_<2T>(filter);

  /* alloc <Ix> */
  <2T> *Ix = <2T>_Partial_Diff(f, 0, NULL);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 1] = Ix->array[indices[i]];
  }

  /* alloc <Ixx> */
  <2T> *Ixx = <2T>_Partial_Diff(Ix, 0, NULL);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 4] = Ixx->array[indices[i]];
  }

  <2T> *Ixy = <2T>_Partial_Diff(Ix, 1, Ixx);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 7] = Ixy->array[indices[i]];
  }

  <2T> *Ixz = <2T>_Partial_Diff(Ix, 2, Ixx);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 8] = Ixz->array[indices[i]];
  }

  <2T> *Iy = <2T>_Partial_Diff(f, 1, Ix);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 2] = Iy->array[indices[i]];
  }

  <2T> *Iyy = <2T>_Partial_Diff(Iy, 1, Ixx);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 5] = Iyy->array[indices[i]];
  }

  <2T> *Iyz = <2T>_Partial_Diff(Iy, 2, Ixx);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 9] = Iyz->array[indices[i]];
  }

  <2T> *Iz = <2T>_Partial_Diff(f, 2, Iy);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 3] = Iz->array[indices[i]];
  }

  <2T> *Izz = <2T>_Partial_Diff(Iz, 2, f);
  for (i = 0; i < result->dim[1]; i++) {
    result->array[result->dim[0] * i + 6] = Izz->array[indices[i]];
  }

  for (i = 0; i < result->dim[1]; i++) {
    double eigen_value[3];
    Matrix_Eigen_Value_Cs(result->array[result->dim[0] * i + 4], 
			  result->array[result->dim[0] * i + 5],
			  result->array[result->dim[0] * i + 6], 
			  result->array[result->dim[0] * i + 7],
			  result->array[result->dim[0] * i + 8],
			  result->array[result->dim[0] * i + 9], 
			  eigen_value);
    result->array[result->dim[0] * i + 10] = eigen_value[0];
    result->array[result->dim[0] * i + 11] = eigen_value[1];
    result->array[result->dim[0] * i + 12] = eigen_value[2];
  }
  
  //<2T>_Eigen3_Solution_Score(Ixx, Iyy, Izz, Ixy, Ixz, Iyz, result);

  /* normalize */
  /*
  dim_t length = Matrix_Size(result->dim, result->ndim);
  dim_t i;
 
  for (i = 0; i < length; i++) {
    if (f->array[i] <= 0.0) {
      result->array[i] = 0.0;
    } else {
      result->array[i] /= f->array[i];
    }
  }
  */

  /* free <f> */
  Kill_<2T>(f);
  /* free <Ix> */
  Kill_<2T>(Ix);
  /* free <Ixx> */
  Kill_<2T>(Ixx);
  
  //Kill_<2T>(Iyy);
  //Kill_<2T>(Izz);
  //Kill_<2T>(Ixy);
  //Kill_<2T>(Ixz);
  //Kill_<2T>(Iyz);
  //Kill_<2T>(f);
  free(indices);

  return result;
}
</t>

<3t=double,float,int>
<2T>* Get_<6T>_Matrix3(const Stack *stack)
{
  <2T>* dm = NULL;

  dim_type dim[3];
  dim[0] = (dim_type)stack->width;
  dim[1] = (dim_type)stack->height;
  dim[2] = (dim_type)stack->depth;

  dm = Make_<2T>(dim,3);
  
  uint16* array16 = (uint16*) (stack->array);
  float32* array32 = (float32*) (stack->array);
  float64* array64 = (float64*) (stack->array);
  color_t *array_color = (color_t*) (stack->array);

  size_t i,length;
  length = (size_t) dim[0]*dim[1]*dim[2];//stack->width*stack->height*stack->depth;

  switch(stack->kind) {
  case GREY:
    for(i=0;i<length;i++)
      dm->array[i] = (<3t>) (stack->array[i]);
    break;
  case GREY16:
    for(i=0;i<length;i++)
      dm->array[i] = (<3t>) (array16[i]);   
    break;
  case FLOAT32:
    for(i=0;i<length;i++)
      dm->array[i] = (<3t>) (array32[i]);
    break;
  case FLOAT64:
    for(i=0;i<length;i++)
      dm->array[i] = (<3t>) (array64[i]);
    break;
  case COLOR:
    for(i=0;i<length;i++) {
      dm->array[i] = (<3t>) (array_color[i][0]) + (<3t>) (array_color[i][1]) + 
	(<3t>) (array_color[i][2]);
    }
    break;
  default:
    fprintf(stderr,"Unrecongnzied stack kind in Get_Float_Matrix3");
  }

  return dm;  
}
</t>

