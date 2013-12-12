/* tz_stack_threshold.c
 *
 * 11-Nov-2007 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_image_lib.h"
#include "tz_int_histogram.h"
#include "tz_stack_attribute.h"
#include "tz_stack_threshold.h"

/*
 * Thresholding. Any intesity no greater than <thre> will be set to 0.
 * It returns 1 if the input stack is changed, otherwise it returns 0.
 */
#define STACK_THRESHOLD(stack_array)		\
  for (i = 0; i < voxel_number; i++) {		\
    if (stack_array[i] <= thre) {		\
      stack_array[i] = 0;			\
      is_changed = 1;				\
    }						\
  }

int Stack_Threshold(Stack *stack,int thre)
{
  if (stack == NULL) {
    return 0;
  }

  int is_changed = 0;
  int voxel_number = Stack_Voxel_Number(stack);

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  int i;  
  switch(Stack_Kind(stack)) {
  case GREY:
    if (thre > 0) {
      STACK_THRESHOLD(array_grey);
    }
    break;
  case GREY16:
    if (thre > 0) {
      STACK_THRESHOLD(array_grey16);
    }
    break;
  case FLOAT32:
    STACK_THRESHOLD(array_float32);
    break;
  case FLOAT64:
    STACK_THRESHOLD(array_float64);
    break;
  default:
    perror("Unsupported stack kind.");
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return is_changed;
}

#undef STACK_THRESHOLD

#define STACK_THRESHOLD_BINARIZE(stack_array)	\
  for (i = 0; i < voxel_number; i++) {		\
    stack_array[i] = stack_array[i] > thre;	\
  }

void Stack_Threshold_Binarize(Stack *stack, int thre)
{
  if (stack == NULL) {
    return;
  }

  int voxel_number = Stack_Voxel_Number(stack);

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  int i;  
  switch(Stack_Kind(stack)) {
  case GREY:
    STACK_THRESHOLD_BINARIZE(array_grey);
    break;
  case GREY16:
    STACK_THRESHOLD_BINARIZE(array_grey16);
    break;
  case FLOAT32:
    STACK_THRESHOLD_BINARIZE(array_float32);
    break;
  case FLOAT64:
    STACK_THRESHOLD_BINARIZE(array_float64);
    break;
  default:
    perror("Unsupported stack kind.");
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

#define STACK_THRESHOLD_BINARIZE_S(stack_array)			\
  for (k = 0; k < stack->depth; k++) {				\
    for (j = 0; j < stack->height; j++) {			\
      for (i = 0; i < stack->width; i++) {			\
	stack_array[offset] = stack_array[offset] > thre[k];	\
	offset++;						\
      }								\
    }								\
  }

void Stack_Threshold_Binarize_S(Stack *stack, const int *thre)
{
  int i, j, k;
  int offset = 0;
  
  Image_Array ima;
  ima.array = stack->array;
  
  switch(Stack_Kind(stack)) {
  case GREY:
    STACK_THRESHOLD_BINARIZE_S(ima.array);
    break;
  case GREY16:
    STACK_THRESHOLD_BINARIZE_S(ima.array16);
    break;
  case FLOAT32:
    STACK_THRESHOLD_BINARIZE_S(ima.array32);
    break;
  case FLOAT64:
    STACK_THRESHOLD_BINARIZE_S(ima.array64);
    break;
  default:
    perror("Unsupported stack kind.");
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

int Stack_Find_Threshold_A(Stack *stack, int meth)
{
  if (meth == THRESHOLD_DEFAULT) {
    meth = THRESHOLD_LOCMAX_TRIANGLE;
  }

  int *hist = NULL;

  hist = Stack_Hist(stack);
  int low = Int_Histogram_Min(hist);
  int high = Int_Histogram_Max(hist);
  int thre = 0;

  switch (meth) {
    case THRESHOLD_LOCMAX_TRIANGLE:
      thre = Stack_Find_Threshold_Locmax(stack, low, high);
      break;

    default:
      PRINT_EXCEPTION("Unexpeced value.", "Unrecognized thresholding method");
      Stack_Binarize(stack);
  }

  printf("Threshold: %d\n", thre);

  free(hist);

  return thre;
}

void Stack_Threshold_Binarize_A(Stack *stack, int meth)
{
  Stack_Threshold_Binarize(stack, Stack_Find_Threshold_A(stack, meth));
}

int Stack_Threshold_Common(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  /* int thre = Hist_Most_Common(hist,low,high); */
  int thre = Int_Histogram_Mode(hist, low, high);
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

int Stack_Threshold_Median(Stack* stack)
{
  int *hist = Stack_Hist(stack);
  /* int thre = Hist_Most_Common(hist,low,high); */
  int thre = Int_Histogram_Quantile(hist, 0.5);
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

int Stack_Threshold_Quantile(Stack* stack, double q)
{
  int *hist = Stack_Hist(stack);
  /* int thre = Hist_Most_Common(hist,low,high); */
  int thre = Int_Histogram_Quantile(hist, q);
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

/* Hist_Rcthre(): RC thresholding.
 */
int Hist_Rcthre(int *hist,int low,int high)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  int thre = 0;
  double c1,c2;

#if 0 
  FILE *fp = fopen("/tmp/mextest.txt", "w+");
#endif

  int prevthre;
  do {
    if (thre == length - 1) {
      break;
    }

    prevthre = thre;
 
    c1 = iarray_centroid_d(hist2,thre+1);
    c2 = iarray_centroid_d(hist2+thre+1,length-thre-1);

#if 0
    fprintf(fp, "%.4f : %.4f\n", c1, c2);
#endif

    c2 += thre+1;
    thre = (int) ((c1+c2) / 2);
  } while(thre!=prevthre);
  
  /*
  c1 = iarray_centroid_d(hist2, thre+1);
  c2 = iarray_centroid_d(hist2 + thre + 1, length - thre - 1);
  double c = (c2 + c2) / 2.0;

  while (thre < c) {
    thre++;
    if(thre == length-1)
      break;

    c1 = iarray_centroid_d(hist2, thre+1);
    c2 = iarray_centroid_d(hist2+thre+1, length-thre-1);
    c2 += thre+1;

    c = (c1+c2) / 2;
  } while((double) thre < c);
  */
#if 0
  fclose(fp);
#endif

  thre += mingrey;

  return thre;
}

int Hist_Rcthre_R(int *hist,int low,int high, double *c1, double *c2)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  int thre = (mingrey + maxgrey) / 2.0 - mingrey;
  /* use the median as the initial value */
  //int thre = Int_Histogram_Quantile(hist, 0.5);

  *c1 = 0.0;
  *c2 = 0.0;
  int prevthre;
  do {
    if (thre == length - 1) {
      break;
    }

    prevthre = thre;
 
    *c1 = iarray_centroid_d(hist2, thre + 1);
    *c2 = iarray_centroid_d(hist2 + thre + 1, length - thre - 1);

    *c2 += thre + 1;
    thre = (int) ((*c1 + *c2) / 2);
  } while (thre != prevthre);

  thre += mingrey;

  return thre;
}

/* Hist_Spthre: Stable point thresholding.
 */
int Hist_Spthre(int *hist,int low,int high)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  //int thre = (length-1)/2;
  int thre = 0;
  size_t idx;
  int hist_min = iarray_min(hist2, length, &idx);
  int hist_max = iarray_max(hist2, length, &idx);

  if (hist_min == hist_max) {
    thre = 0;
  } else {
    int i;
    for (i = 0; i < length; i++) {
      hist2[i] = (hist2[i] - hist_min) * (length - 1) / (hist_max - hist_min);
      if (hist2[i] < i) {
	thre = i - 1;
	break;
      }
    }
  }

  thre += mingrey;

  return thre;
}

/* Hist_Tpthre: Turning point thresholding.
 */
int Hist_Tpthre(int *hist,int low,int high)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  //int thre = (length-1)/2;
  int thre = 0;
  size_t idx;
  int hist_min = iarray_min(hist2, length, &idx);
  int hist_max = iarray_max(hist2, length, &idx);

  if (hist_min == hist_max) {
    thre = 0;
  } else {
    int i;
    for (i = 0; i < length; i++) {
      hist2[i] = (hist2[i] - hist_min) * (length - 1) / (hist_max - hist_min);
    }

    for (i = 1; i < length; i++) {
      if (hist[i] - hist[i-1] <= 1) {
	thre = i - 1;
	break;
      }
    }
  }

  thre += mingrey;

  return thre;
}

/*
 */
int Hist_Tpthre2(int *hist, int low, int high)
{
  int threshold = 0;
  
  int *raw_hist = hist + 2;

  double *curve = (double *) malloc(sizeof(double) * hist[0]);
  double *curvature = (double *) malloc(sizeof(double) * (hist[0]));
  int *curvature_index = (int *) malloc(sizeof(int) * (hist[0]));
  int *signal_index = (int *) malloc(sizeof(int) * (hist[0]));
  int signal_length = 0;

  int i;
  for (i = 0; i < hist[0]; i++) {
    curve[i] = (double) raw_hist[i];
    if (raw_hist[i] > 0) {
      signal_index[signal_length++] = i;
    }
  }

  if (signal_length > 3) {
    size_t index;
    double curve_min = darray_min(curve, hist[0], &index);
    double curve_max = darray_max(curve, hist[0], &index);
  
    for (i = 0; i < hist[0]; i++) {
      curve[i] = (curve[i] - curve_min) / (curve_max - curve_min) * 
	(double) hist[0];
    }
    
    double v1[2], v2[2];
    double d1_sq, d2_sq;
    double d12;
    int curvature_length = 0;
  
    if ((high < 0) || (high > hist[1] + hist[0] - 1)) {
      high = hist[0] + hist[1] - 1;
    }
    
    if (low < hist[1]) {
      low = hist[1];
    }
    
    low -= hist[1];
    high -= hist[1];

    int left, center, right;

    for (i = 1; i < signal_length - 1; i++) {
      if ((signal_index[i] >= low) && (signal_index[i] <= high)) {
	left = signal_index[i - 1];
        center = signal_index[i];
	right = signal_index[i + 1];
	/*
	printf("%d, %d, %d\n", 
	       raw_hist[left], raw_hist[center], raw_hist[right]);
	*/
	if ((curve[center] <= curve[left]) && (curve[right] <= curve[center])) {
	  v1[0] = ((double) (left - center)) / 2.0;
	  v1[1] = (curve[left] - curve[center]) / 2.0;
	  v2[0] = ((double) (right - center)) / 2.0;
	  v2[1] = (curve[right] - curve[center]) / 2.0;
	  d1_sq = v1[0] * v1[0] + v1[1] * v1[1];
	  d2_sq = v2[0] * v2[0] + v2[1] * v2[1];
	  d12 = v1[0] * v2[0] + v1[1] * v2[1];
	  curvature_index[curvature_length] = i;
	  curvature[curvature_length++] = 
	    sqrt((d1_sq + d2_sq + 2 * fabs(d12)) / 
		 (d1_sq * d2_sq - d12 * d12)) * 
	    sqrt(d1_sq) * sqrt(d2_sq);  
	}
      }
    }

    if (curvature_length > 0) {
      /*
      darray_print2(curvature, curvature_length, 1);
      iarray_print2(curvature_index, curvature_length, 1);
      */
      darray_min(curvature, curvature_length, &index);
      
      threshold = signal_index[curvature_index[index]] + hist[1];
    } else {
      threshold = 0;
    }
  } else {
    threshold = 0;
  }
 
  free(curve);
  free(curvature);
  free(curvature_index);
  free(signal_index);

  return threshold;
}

/* Hist_Dpthre: Droping point thresholding.
 *
 * Note: It finds a point where the histogram drops less than half.
 */
int Hist_Dpthre(int *hist,int low,int high)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  //int thre = (length-1)/2;
  int thre = 0;

  int i;
  for (i = 1; i < length; i++) {
    if (hist2[i - 1] / hist2[i] == 1) {
      thre = i - 1;
      break;
    }
  }

  thre += mingrey;

  return thre;
}

/* Hist_Dpthre2(): Another dropping point thresholding.
 *
 * Args: hist - histogram
 *       low - lower bound;
 *       high - upper bound.
 *
 * Note: It finds a point where the slope is smaller than the slope of the line
 *       connecting the point and the minimal point.
 */
int Hist_Dpthre2(int *hist,int low,int high)
{
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey  = high;

  length = maxgrey - mingrey + 1;

  //int thre = (length-1)/2;
  int thre = 0;
  size_t max_idx, min_idx;
  int hist_min = iarray_min(hist2, length, &min_idx);
  int hist_max = iarray_max(hist2, length, &max_idx);

  if ((hist_min == hist_max) || (min_idx == 0)) {
    thre = 0;
  } else {
    int i;
    for (i = 1; i < length; i++) {
      if (hist2[i] * min_idx - hist_min >= hist2[i-1] * (min_idx - 1)) {
	thre = i - 1;
	break;
      }
    }
  }

  thre += mingrey;

  return thre;
}

int Stack_Threshold_RC(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Rcthre(hist,low,high);
#ifdef _DEBUG_2
  printf("%d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

int Stack_Threshold_Triangle(Stack* stack, int low, int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Int_Histogram_Triangle_Threshold(hist,low,high);
#ifdef _DEBUG_2
  printf("%d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

int Stack_Threshold_RC_R(Stack* stack, int low, int high, 
			 double *c1, double *c2)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Rcthre_R(hist,low,high, c1, c2);
  Stack_Threshold(stack,thre);
  free(hist);

  return thre;
}

void Stack_Threshold_Sp(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Spthre(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

void Stack_Threshold_Tp(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Tpthre(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

void Stack_Threshold_Tp2(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Tpthre2(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

void Stack_Threshold_Tp3(Stack* stack,int low,int high)
{
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
  int *hist = Stack_Hist_M(stack, locmax);
  Kill_Stack(locmax);
  int thre = Hist_Tpthre2(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

int Stack_Find_Threshold_Locmax(Stack *stack, int low, int high)
{
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_SINGLE);
  int *hist = Stack_Hist_M(stack, locmax);
  Kill_Stack(locmax);
  
  int thre = 0;

  if (hist != NULL) {
#if 0
    TRACE("Info");
    Print_Int_Histogram(hist);
#endif
    
    thre = Int_Histogram_Triangle_Threshold(hist,low,high);
    
    free(hist);
  }

  return thre;
}

int Stack_Find_Threshold_Locmax2(Stack *stack, int low, int high, double alpha)
{
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
  int *hist = Stack_Hist_M(stack, locmax);
  Kill_Stack(locmax);

  int thre = 0;

  if (hist != NULL) {
#if 0
    TRACE("Info");
    Print_Int_Histogram(hist);
#endif

    thre = Int_Histogram_Triangle_Threshold2(hist, low, high, alpha);

    free(hist);
  }

  return thre;
}

int Stack_Find_Threshold_Locmax_L2(Stack *stack, int low, int high, 
    double alpha)
{
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
  int *hist = Stack_Hist_M(stack, locmax);
  Kill_Stack(locmax);

  int thre = 0;

  if (hist != NULL) {
    thre = Int_Histogram_Triangle_Threshold2(hist, low, high, alpha);
    thre = Int_Histogram_Triangle_Threshold2(hist, thre, high, alpha);
    free(hist);
  }

  return thre;
}

void Stack_Threshold_Tp4(Stack* stack,int low,int high)
{
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);
  int *hist = Stack_Hist_M(stack, locmax);
#if 0
  Print_Int_Histogram(hist);
#endif
  Kill_Stack(locmax);
  int thre = Int_Histogram_Triangle_Threshold(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

void Stack_Threshold_Dp(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Dpthre(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}

void Stack_Threshold_Dp2(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Dpthre2(hist,low,high);
#if 1
  printf("Threshold: %d\n", thre);
#endif
  Stack_Threshold(stack,thre);
  free(hist);
}
