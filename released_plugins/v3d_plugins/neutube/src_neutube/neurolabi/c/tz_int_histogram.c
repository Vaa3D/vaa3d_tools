/* tz_int_histogram.c
 *
 * 20-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_int_histogram.h"

int Int_Histogram_Length(const int *hist)
{
  return hist[0];
}

int Int_Histogram_Min(const int *hist)
{
  return hist[1];
}

int Int_Histogram_Max(const int *hist)
{
  return hist[0] + hist[1] - 1;
}

int Int_Histogram_Quantile(const int *hist, double q)
{
  int length = hist[0];
  int hmin = hist[1];
  hist = Int_Histogram_Const_Array(hist);

  int all = 0;
  int i;
  for (i = 0; i < length; i++) {
    all += hist[i];
  }

  int thre = all * q;

  for (i = 0; i < length; i++) {
    thre -= hist[i];
    if (thre < 0) {
      break;
    }
  }

  return i + hmin;
}

int* Int_Histogram_Array(int *hist)
{
  return hist + 2;
}

const int* Int_Histogram_Const_Array(const int *hist)
{
  return hist + 2;
}

#define INT_HISTOGRAM_VALIDATE_RANGE(hist, low, high, length)	\
  {								\
    if (low < hist[1]) {					\
      low = hist[1];						\
    }								\
    if (high > hist[0] + hist[1] - 1) {				\
      high = hist[0] + hist[1] - 1;				\
    }								\
								\
    length = high - low + 1;					\
  }

void Print_Int_Histogram(const int *hist)
{
  int i;

  printf("Histogram:\n");

  printf("Value: ");
  for (i = 0; i < hist[0]; i++) {
    printf("%3d ", hist[1] + i);
  }
  printf("\n");

  printf("Count: ");
  for (i = 0; i < hist[0]; i++) {
    printf("%3d ", hist[2 + i]);
  }
  printf("\n");
}

void Int_Histogram_Range(const int *hist, int *low, int *high)
{
  ASSERT(hist != NULL, "NULL histogram");
  
  if (low != NULL) {
    *low = hist[1];
  } 

  if (high != NULL) {
    *high = hist[0] + hist[1] - 1;
  }
}

double Int_Histogram_Slope(const int *hist, int low, int high)
{
  /* recalculate the start and length */
  int length;
  INT_HISTOGRAM_VALIDATE_RANGE(hist, low, high, length);
  
  /* extract pure histogram*/
  hist = hist + low + 2;

  /* find the maximum of the pure histogram */
  size_t max_idx;
  int max = iarray_max(hist, length, &max_idx);

  /* find the minumum of the pure histogram */
  size_t min_idx;
  int min = iarray_min(hist, length, &min_idx);

  /* calculate and return the slope */
  double slope = 0.0;
  if (max_idx != min_idx) {
    slope = ((double) (max - min)) / (max_idx - min_idx);
  }

  return slope;
}

int Int_Histogram_Mode(const int *hist, int low, int high)
{
  if (low > high) {
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  size_t common;
  int *hist2 = (int *) (hist + 2);
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0] + hist[1] - 1;
  
  if(low > mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if ((high < maxgrey) && (high >= 0)) {
    maxgrey = high;
  }

  length = maxgrey - mingrey + 1;

  iarray_max(hist2, length, &common);

  common += mingrey;

  return common;
}

int* Int_Histogram_Diff(const int *hist, int *diff)
{
  int diff_length = hist[0] - 1;
   
  if (diff == NULL) {
    diff = (int *) malloc(sizeof(int) * diff_length);
  }


  iarraycpy(diff, hist + 3, 0, diff_length);
  iarray_sub(diff, (int *) (hist + 2), diff_length);

  return diff;
}

int Int_Histogram_Triangle_Threshold(const int *hist, int low, int high)
{
  /* Truncate the histogram and get range to search */
  int min_grey = hist[1];
  int max_grey = hist[0] + hist[1] - 1;
  
  const int *hist2 = hist + 2;

  if (min_grey < low) {
    hist2 += low - min_grey;
    min_grey = low;
  }
  
  if (max_grey > high) {
    max_grey = high;
  }

  int length = max_grey - min_grey + 1;
  
  /* Find the maximum of the histogram */
  size_t max_index;
  iarray_max(hist2, length, &max_index);
  
  /* Find the other end */
  size_t min_index;
  iarray_min_ml(hist2 + max_index, length - max_index, hist2 + max_index,
	       &min_index);
  min_index += max_index;

  /* If the two ends are the same, take all pixels in the range as foreground */
  if (max_index == min_index) {
    return min_grey - 1;
  }

  /* set length to the new range */
  length = min_index - max_index + 1;

  /* relocate histogram */
  hist2 += max_index;

  /* Normalize the histogram */
  double *dhist = (double *) malloc(sizeof(double) * length);
  int i;
  double norm_factor = (double) (length - 1) / 
    ((double) (hist2[0] - hist2[length - 1]));
  for (i = 0; i < length; i++) {
    dhist[i] = (double) (hist2[i] - hist2[length - 1]) * norm_factor;
  }

  /* Set threshold as the first value */
  int thre = 0;
  double best_score = dhist[0];
  double score;

  /* For each point on the histogram */
  for (i = 1; i < length; i++) {
    /* if there are some pixels with the value */
    if (hist2[i] > 0) {
      /* Calculate the score */
      score = dhist[i] + (double) i;
      /* If the score is smaller than the best score */
      if (score < best_score) {
	/* set the best score to score  */
	best_score = score;
	/* update threshold */
	thre = i;
      }
    }
  }

  /* Add offset to the threshold */
  thre += max_index + min_grey;

  free(dhist);
  
  return thre;
}

int Int_Histogram_Triangle_Threshold2(const int *hist, int low, int high,
				      double alpha)
{
  /* Truncate the histogram and get range to search */
  int min_grey = hist[1];
  int max_grey = hist[0] + hist[1] - 1;
  
  const int *hist2 = hist + 2;

  if (min_grey < low) {
    hist2 += low - min_grey;
    min_grey = low;
  }
  
  if (max_grey > high) {
    max_grey = high;
  }

  int length = max_grey - min_grey + 1;
  
  /* Find the maximum of the histogram */
  size_t max_index;
  iarray_max(hist2, length, &max_index);
  
  /* Find the other end */
  size_t min_index = length - 1;

  /* If the two ends are the same, take all pixels in the range as foreground */
  if (max_index == min_index) {
    return min_grey - 1;
  }

  /* set length to the new range */
  length = min_index - max_index + 1;

  /* relocate histogram */
  hist2 += max_index;

  /* Normalize the histogram */
  double *dhist = (double *) malloc(sizeof(double) * length);
  int i;
    /*
  double norm_factor = (double) (length - 1) / 
    ((double) (hist2[0] - hist2[length - 1]));
    */
  int min_value = iarray_min(hist2, length, NULL);
  double norm_factor = (double) (length - 1) / 
    ((double) (hist2[0] - min_value));
  for (i = 0; i < length; i++) {
    dhist[i] = (double) (hist2[i] - min_value) * norm_factor;
  }

  /* Set threshold as the first value */
  int thre = 0;
  double best_score = dhist[0];
  double score;

  /* For each point on the histogram */
  for (i = 1; i < length; i++) {
    /* if there are some pixels with the value */
    if (hist2[i] > 0) {
      /* Calculate the score */
      score = dhist[i] + alpha * (double) i;
      /* If the score is smaller than the best score */
      if (score < best_score) {
	/* set the best score to score  */
	best_score = score;
	/* update threshold */
	thre = i;
      }
    }
  }

#ifdef _DEBUG_2
  printf("%g\n", best_score);
#endif

  /* Add offset to the threshold */
  thre += max_index + min_grey;

  free(dhist);
  
  return thre;
}


#if 0
int Int_Histogram_Triangle_Threshold2(const int *hist, int low, int high)
{
  /* Truncate the histogram and get range to search */
  int min_grey = hist[1];
  int max_grey = hist[0] + hist[1] - 1;
  
  const int *hist2 = hist + 2;

  if (min_grey < low) {
    hist2 += low - min_grey;
    min_grey = low;
  }
  
  if (max_grey > high) {
    max_grey = high;
  }

  int length = max_grey - min_grey + 1;
  
  /* Find the maximum of the histogram */
  int max_index;
  iarray_max(hist2, length, &max_index);
  
  /* Find the other end */
  int min_index;
  iarray_min_ml(hist2 + max_index, length - max_index, hist2 + max_index,
	       &min_index);
  min_index += max_index;

  /* If the two ends are the same, take all pixels in the range as foreground */
  if (max_index == min_index) {
    return min_grey - 1;
  }

  /* set length to the new range */
  length = min_index - max_index + 1;

  /* relocate histogram */
  hist2 += max_index;

  /* Normalize the histogram */
  double *dhist = (double *) malloc(sizeof(double) * length);
  int i;
  double norm_factor = (double) (length - 1) / 
    ((double) (hist2[0] - hist2[length - 1]));
  for (i = 0; i < length; i++) {
    dhist[i] = (double) (hist2[i] - hist2[length - 1]) * norm_factor;
  }

  /* Set threshold as the first value */
  int thre = 0;
  double best_score = dhist[0];
  double score;

  /* For each point on the histogram */
  for (i = 1; i < length; i++) {
    /* if there are some pixels with the value */
    if (hist2[i] > 0) {
      /* Calculate the score */
      score = dhist[i] + (double) i;
      /* If the score is smaller than the best score */
      if (score < best_score) {
	/* set the best score to score  */
	best_score = score;
	/* update threshold */
	thre = i;
      }
    }
  }

  /* Add offset to the threshold */
  thre = max_index + min_grey + thre / 3;

  free(dhist);
  
  return thre;
}
#endif

int Int_Histogram_Rc_Threshold(const int *hist, int low, int high)
{
  const int *hist2 = hist+2;
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

  int prevthre;
  do {
    if (thre == length - 1) {
      break;
    }

    prevthre = thre;
 
    c1 = iarray_centroid_d(hist2,thre+1);
    c2 = iarray_centroid_d(hist2+thre+1,length-thre-1);

    c2 += thre+1;
    thre = (int) ((c1+c2) / 2);
  } while(thre!=prevthre);

  thre += mingrey;

  return thre;
}

int Int_Histogram_Stable_Point(const int *hist, int low, int high)
{
  const int *hist2 = hist+2;
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
  size_t idx;
  int hist_min = iarray_min(hist2, length, &idx);
  int hist_max = iarray_max(hist2, length, &idx);

  if (hist_min == hist_max) {
    thre = 0;
  } else {
    int i;
    int slope;
    for (i = 0; i < length; i++) {
      slope = (hist2[i] - hist_min) * (length - 1) / (hist_max - hist_min);
      if (slope < i) {
	thre = i - 1;
	break;
      }
    }
  }

  thre += mingrey;

  return thre;
}

int* Int_Histogram_Equalize(const int *hist, int min, int max)
{
  /* <map> allocated */
  int *map = iarray_malloc(Int_Histogram_Length(hist) + 2);
  
  map[1] = Int_Histogram_Min(hist);
  
  int i;
  map[0] = Int_Histogram_Length(hist);

  const int *hist_array = Int_Histogram_Const_Array(hist);
  int cumsum = 0;
  int sum = iarray_sum(hist_array, map[0]);

  for (i = 0; i < map[0]; i++) {
    cumsum += hist_array[i];
    map[i + 2] = min + 
      (max - min) * (cumsum - hist_array[0]) / (sum - hist_array[0]);
  }

  /* <map> returned */
  return map;
}

int* Int_Histogram_Equal_Map(const int *hist, int nbin, int *map)
{
  int length = Int_Histogram_Length(hist);

  if (map == NULL) {
    map = iarray_malloc(length);
  }

  const int *hist_array = Int_Histogram_Const_Array(hist);
  
  int i;
  int total_number = iarray_sum(hist_array, length);
  int current_index = 0;
  int threshold = total_number / nbin;
  int count = hist_array[0];

  map[0] = current_index;
  for (i = 1; i < length; i++) {
    if (nbin - map[i-1] - 1 >= length - i) {
      int j;
      current_index = map[i-1] + 1;
      for (j = i; j < length; j++) {
	map[j] = current_index++;
      }
      break;
    }

    count += hist_array[i];
    if (count <= threshold) {
      map[i] = current_index;
    } else {
      total_number -= count;
      if (count - threshold >= threshold + hist_array[i] - count) {
	total_number += hist_array[i];
	count = hist_array[i];
	map[i] = current_index + 1;
      } else {
	count = 0;
	map[i] = current_index;
      }

      current_index++;

      /* update threshold */
      threshold = total_number / (nbin - current_index);
    }
  }

  return map;
}

int* Int_Histogram_Equal_Info_Map(const int *hist, int nbin, int *map)
{
  int length = Int_Histogram_Length(hist);
  int min = Int_Histogram_Min(hist);

  if (map == NULL) {
    map = iarray_malloc(length);
  }

  const int *hist_array = Int_Histogram_Const_Array(hist);
  
  /* alloc <info> */
  double *info = darray_malloc(length);
  
  int i;

  for (i = 0; i < length; i++) {
    if (hist_array[i] == 0) {
      info[i] = 0;
    } else {
      if (min +i == 0) {
	info[i] = log(hist_array[i]);
      } else
	info[i] = log(hist_array[i]) + log(min + i);
    }
  }

  double total_number = darray_sum(info, length);
  int current_index = 0;
  double threshold = total_number / nbin;
  double count = info[0];

  map[0] = current_index;
  for (i = 1; i < length; i++) {
    if (nbin - map[i-1] - 1 >= length - i) {
      int j;
      current_index = map[i-1] + 1;
      for (j = i; j < length; j++) {
	map[j] = current_index++;
      }
      break;
    }

    count += info[i];
    if (count <= threshold) {
      map[i] = current_index;
    } else {
      total_number -= count;
      if (count - threshold >= threshold + info[i] - count) {
	total_number += info[i];
	count = info[i];
	map[i] = current_index + 1;
      } else {
	count = 0;
	map[i] = current_index;
      }

      current_index++;

      /* this case should not happen */
      if (current_index > 255) {
	current_index = 255;
      }

      /* update threshold */
      threshold = total_number / (nbin - current_index);
    }
  }

  /* free <info> */
  free(info);

  return map;
}
