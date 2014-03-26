#include <stdio.h>
#include "tz_math_lib.h"

/* obsolete file */

int minint(int* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minint\n");
    return 0;
  }
  int min_value;
  ARRAY_MIN(array,length,min_value);
  return min_value;
}

int maxint(int* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minint\n");
    return 0;
  }
  int max_value;
  ARRAY_MAX(array,length,max_value);
  return max_value;
}

double mindouble(double* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in mindouble\n");
    return 0;
  }
  double min_value;
  ARRAY_MIN(array,length,min_value);
  return min_value;
}

double maxdouble(double* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in mindouble\n");
    return 0;
  }
  double max_value;
  ARRAY_MAX(array,length,max_value);
  return max_value;
}

float minfloat(float* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minfloat\n");
    return 0;
  }
  float min_value;
  ARRAY_MIN(array,length,min_value);
  return min_value;
}

float maxfloat(float* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minfloat\n");
    return 0;
  }
  float max_value;
  ARRAY_MAX(array,length,max_value);
  return max_value;
}

long minlong(long* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minlong\n");
    return 0;
  }
  long min_value;
  ARRAY_MIN(array,length,min_value);
  return min_value;
}

long maxlong(long* array,long length) {
  if(length<=0) {
    fprintf(stderr,"Invalid array length in minlong\n");
    return 0;
  }
  long max_value;
  ARRAY_MAX(array,length,max_value);
  return max_value;
}

int signint(int x)
{
  if (x > 0) {
    return 1;
  } else if (x < 0) {
    return -1;
  } else {
    return 0;
  }
}

void switchint(int *x, int *y)
{
  int tmp = *x;
  *x = *y;
  *y = tmp;
}
