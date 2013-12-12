#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "tz_error.h"
#include "tz_<2t>.h"
#include "tz_string.h"
#if defined(HAVE_LIBGSL)
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_cblas.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include "tz_utilities.h"
<2t=darray,farray>
#include "tz_complex.h"
</t>

void <2t>_error(const char *msg,const char *arg)
{
  fprintf(stderr,"\nError in tz_<2t>.c:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

void <2t>_warning(const char *msg,const char *arg)
{
  fprintf(stderr,"\nWarning in tz_<2t>.c:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
}

<1t> <2t>_max(const <1t>* d1,size_t length, size_t* idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  size_t max_idx = 0;
  size_t i;
  for(i = 1; i < length; i++) {
    if(d1[i] > d1[max_idx]) {
      max_idx = i;
    }
  }

  if (idx != NULL) {
    *idx = max_idx;
  }

  return d1[max_idx];
}

/* find max. <idx> is the last occurrence. */
<1t> <2t>_max_l(const <1t>* d1,size_t length,size_t *idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  size_t max_idx = length - 1;

  size_t i;
  for(i = length - 1; i > 0; i--) {
    if(d1[i - 1] > d1[max_idx]) {
      max_idx = i - 1;
    }
  }

  if (idx != NULL) {
    *idx = max_idx;
  }

  return d1[max_idx];
}

<1t> <2t>_min(const <1t>* d1, size_t length, size_t* idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  size_t min_idx = 0;
  size_t i;
  for(i = 1; i < length; i++) {
    if(d1[i] < d1[min_idx]) {
      min_idx = i;
    }
  }

  if (idx != NULL) {
    *idx = min_idx;
  }

  return d1[min_idx];
}

/* find min. <idx> is the last occurrence. */
<1t> <2t>_min_l(const <1t>* d1, size_t length, size_t *idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  size_t min_idx = length - 1;

  size_t i;
  for(i = length - 1; i > 0; i--) {
    if(d1[i - 1] < d1[min_idx]) {
      min_idx = i - 1;
    }
  }

  if (idx != NULL) {
    *idx = min_idx;
  }

  return d1[min_idx];
}

/* find max from elements of interest. */
<1t> <2t>_max_m(const <1t> *d1, size_t length, const int *mask, size_t *idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  BOOL found_first = FALSE;

  size_t max_idx = INVALID_ARRAY_INDEX;
  size_t i;
  for (i = 0; i < length; i++) {
    if (mask[i]) {
      max_idx = i;
      found_first = TRUE;
      break;
    }
  }

  if (found_first == TRUE) {    
    for (i = max_idx; i < length; i++) {
      if (mask[i]) {
	if(d1[i] > d1[max_idx]) {
	  max_idx = i;
	}
      }
    }
  }
  
  if (idx != NULL) {
    *idx = max_idx;
  }
  
  if (found_first == FALSE) {
    return 0;
  } else {
    return d1[max_idx];
  }
}

<1t=double>
<1t> <2t>_max_n(const <1t> *d1, size_t length, size_t *idx)
{
  TZ_ASSERT(d1 != NULL, "Null pointer");
  TZ_ASSERT(length > 0, "Array length is zero");

  BOOL found_first = FALSE;
  size_t max_idx = INVALID_ARRAY_INDEX;
  size_t i;
  for (i = 0; i < length; i++) {
    if (!isnan(d1[i])) {
      max_idx = i;
      found_first = TRUE;
      break;
    }
  }

  if (found_first == TRUE) {
    for (i = max_idx; i < length; i++) {
      if (!isnan(d1[i])) {
	if(d1[i] > d1[max_idx]) {
	  max_idx = i;
	}
      }
    }
  }
  
  if (idx != NULL) {
    *idx = max_idx;
  }
  
  if (found_first == FALSE) {
    return 0.0;
  } else {
    return d1[max_idx];
  }
}
</t>


/* last occurrence */
<1t> <2t>_max_ml(const <1t> *d1, size_t length, const int *mask, size_t *idx)
{
  BOOL found_first = FALSE;

  size_t max_idx = length;
  size_t i;
  for (i = length; i > 0; i--) {
    if (mask[i - 1] != 0) {
      found_first = TRUE;
      max_idx = i - 1;
      break;
    }
  }

  if (found_first == TRUE) {    
    for (i = max_idx + 1; i > 0; i--) {
      if (mask[i - 1] != 0) {
	if(d1[i - 1] > d1[max_idx]) {
	  max_idx = i - 1;
	}
      }
    }
  }
  
  if (idx != NULL) {
    *idx = max_idx;
  }
  
  if (found_first == FALSE) {
    return 0;
  } else {
    return d1[max_idx];
  }
}

<1t> <2t>_min_m(const <1t> *d1, size_t length, const int *mask, size_t *idx)
{
  BOOL found_first = FALSE;
  size_t min_idx = INVALID_ARRAY_INDEX;
  size_t i;
  for (i = 0; i < length; i++) {
    if (mask[i] != 0) {
      found_first = TRUE;
      min_idx = i;
      break;
    }
  }

  if (found_first == TRUE) {    
    for (i = min_idx; i < length; i++) {
      if (mask[i] != 0) {
	if(d1[i] < d1[min_idx]) {
	  min_idx = i;
	}
      }
    }
  }
  
  if (idx != NULL) {
    *idx = min_idx;
  }
  
  if (found_first == FALSE) {
    return 0.0;
  } else {
    return d1[min_idx];
  }
}

<1t> <2t>_min_ml(const <1t> *d1, size_t length, const int *mask, size_t *idx)
{
  size_t min_idx = length;
  size_t i;
  for (i = length; i > 0; i--) {
    if (mask[i - 1] != 0) {
      min_idx = i - 1;
      break;
    }
  }

  if (min_idx < length) {    
    for (i = min_idx + 1; i > 0; i--) {
      if (mask[i - 1] != 0) {
	if(d1[i - 1] < d1[min_idx]) {
	  min_idx = i - 1;
	}
      }
    }
  }
  
  if (idx != NULL) {
    *idx = min_idx;
  }
  
  if (min_idx >= length) {
    return 0;
  } else {
    return d1[min_idx];
  }
}

<1t=float,double,int>
<1t>* <2t>_abs(<1t>* d1, size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    d1[i] = <2t=darray,farray>f</t>abs(d1[i]);
  }

  return d1;
}

<1t>* <2t>_neg(<1t>* d1,size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    d1[i] = -d1[i];
  }

  return d1;
}
</t>

<1t>* <2t>_add(<1t>* d1, const <1t>* d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] += d2[i];
  return d1;
}

<1t>* <2t>_addc(<1t>*d1,<1t> d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] += d2;
  return d1;
}

<1t>* <2t>_cadd(<1t> d2, <1t>*d1, size_t length)
{
  return <2t>_addc(d1,d2,length);
}


<1t>* <2t>_sub(<1t>* d1,<1t>* d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] -= d2[i];
  return d1;
}

<1t>* <2t>_subc(<1t>*d1,<1t> d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] -= d2;
  return d1;
}

<1t>* <2t>_csub(<1t> d2, <1t>*d1, size_t length)
{
  return <2t>_subc(d1,d2,length);
}

<1t>* <2t>_mul(<1t>* d1,<1t>* d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    d1[i] *= d2[i];
  }
  return d1;
}

<1t>* <2t>_mulc(<1t>* d1,<1t> d2, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] *= d2;
  return d1;
}

<1t>* <2t>_div(<1t>* d1,<1t>* d2, size_t length)
{
  size_t i;
  int warned = 0;

  for(i=0;i<length;i++) {
    //if(fabs(d1[i])>MIN_<1T> && fabs(d2[i])>MIN_<1T>)
<1t=float,double>
    if(d2[i] == 0.0) {
</t>
<1t=int,tz_uint8,tz_uint16,int64_t>
    if(d2[i]==0) {
</t>
      if (!warned) {
        <2t>_warning("Divide by zero: %s\n","<2t>_div");
        warned = 1;
      }
      d1[i] = 0;
    } else {
      d1[i] /= d2[i];
    }
  }
  
  return d1;
}

<1t>* <2t>_div_i(<1t>* d1,int* d2, size_t length)
{
  size_t i;
  int warned = 0;

  for(i=0;i<length;i++) {
    //if(fabs(d1[i])>MIN_<1T> && fabs(d2[i])>MIN_<1T>)
    if(d2[i]==0 && !warned) {
      <2t>_warning("Divide by zero: %s\n","<2t>_div");
      warned = 1;
    }

    if(d2[i]!=0)
      d1[i] /= d2[i];
  }
  
  return d1;
}

<1t>* <2t>_divc(<1t>* d1,<1t> d2, size_t length)
{
  size_t i;

<1t=float,double>
  if(d2==0.0)
</t>
    <1t=int,tz_uint8,tz_uint16>
  if(d2==0)
</t>
    <2t>_warning("Divide by zero: %s\n","<2t>_divc");

  for(i=0;i<length;i++)
<1t=float,double>
    if(d1[i]!=0.0)
</t>
<1t=int,tz_uint8,tz_uint16>
    if(d1[i]==0)
</t>
      d1[i] /= d2;

  return d1;
}

double <2t>_dot(const <1t> *d1, const <1t> *d2,  size_t length)
{
  size_t i;
  double d = 0.0;
  for (i = 0; i < length; i++) {
    d += d1[i] * d2[i];
  }

  return d;
}

<1t=double>
double <2t>_dot_n(const <1t> *d1, const <1t> *d2, size_t length)
{
  size_t i;
  double d = 0.0;
  for (i = 0; i < length; i++) {
    if (!(isnan(d1[i]) || isnan(d2[i]))) {
      d += d1[i] * d2[i];
    }
  }

  return d;
}

double <2t>_dot_nw(const <1t> *d1, const <1t> *d2, size_t length)
{
  double w1 = 0.0;
  double w2 = 0.0;
  double nw1 = 0.0;
  double nw2 = 0.0;

  size_t i;

  for (i = 0; i < length; i++) {
    if (d1[i] > 0.0) {
      w1 += d1[i];
    } else {
      w2 -= d1[i];
    }
    if (!isnan(d2[i])) {
      if (d1[i] > 0.0) {
	nw1 += d1[i];
      } else {
	nw2 -= d1[i];
      }      
    }
  }

  if ((nw1 > 0.0) && (nw2 > 0.0)) {
    w1 /= nw1;
    w2 /= nw2;
  }

  double d = 0.0;
  for (i = 0; i < length; i++) {
    if (!(isnan(d1[i]) || isnan(d2[i]))) {
      if (d1[i] > 0.0) {
	d += d1[i] * d2[i] * w1;
      } else {
	d += d1[i] * d2[i] * w2;
      }
    }
  }

  return d;
}
</t>

<1t>* <2t>_sqr(<1t>* d1, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] *= d1[i];
  return d1;
}

<1t>* <2t>_sqrt(<1t>* d1, size_t length)
{
  size_t i;
  int warned = 0;
  for(i=0;i<length;i++) {
    if(d1[i]<0.0) {
      warned++;
      d1[i] = -sqrt(-d1[i]);
    } else {
      d1[i] =sqrt(d1[i]);
    }
  }

  if (warned > 0) {
    <2t>_warning("%s \n","<2t>_sqrt");
    fprintf(stderr, "Square root of %d negative numbers\n", warned);
  }

  return d1;
}

<1t>* <2t>_scale(<1t> *d1, size_t length, <1t> min, <1t> max)
{
  size_t i;

  <1t> array_min = <2t>_min(d1, length, &i);
  <1t> array_max = <2t>_max(d1, length, &i);
  double coef = (double) (max - min) / (array_max - array_min);
  for (i = 0; i < length; i++) {
    d1[i] = (<1t>) ((double) (d1[i] - array_min) * coef + min);
  }

  return d1;
}

<1t>* <2t>_max2(<1t>* d1,const <1t>* d2, size_t length)
{
  size_t i;
  for (i = 0; i < length; i++) {
    if (d1[i] < d2[i]) {
      d1[i] = d2[i];
    }
  }

  return d1;
}


<1t>* <2t>_min2(<1t>* d1,const <1t>* d2, size_t length)
{
  size_t i;
  for (i = 0; i < length; i++) {
    if (d1[i] > d2[i]) {
      d1[i] = d2[i];
    }
  }

  return d1;
}

void <2t>_threshold(<1t> *d, size_t length, const <1t> *min, const <1t> *max)
{
  size_t i;
  for (i = 0; i < length; i++) {
    TZ_ASSERT(max[i] >= min[i], "Invalid threshold");
    if (d[i] < min[i]) {
      d[i] = min[i];
    } else if (d[i] > max[i]) {
      d[i] = max[i];
    }
  }
}

/*calculate d1*sqrt((d2-2)./(1-d1.^2)). The result is 0 if d2 is 
  less than thr. The result is stored in d1.*/
<1t>* <2t>_fun1(<1t>* d1,<1t> *d2,<1t> thr, size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    if(d2[i]<thr)
      d1[i] = 0;
    else {
      if (d1[i] > 0) {
	d1[i] *= log(d1[i]*sqrt((d2[i]-2)/(1.0001-d1[i]*d1[i])));
      }
    }     
  }
  return d1;  
}

<1t>* <2t>_fun1_max(<1t>* d1,<1t> *d2,<1t> thr, size_t length,<1t> *maxv)
{
  size_t i;
  <1t> tmpv = d1[0];
  *maxv = tmpv;

  if(d2[0]<thr)
      d1[0] = 0;
  else {
    if (d1[0] > 0) {
      d1[0] *= log(d1[0]*sqrt((d2[0]-2)/(1.0001-d1[0]*d1[0])));
    }
  }
  <1t> tmpmaxv = d1[0];
  
  for(i=1;i<length;i++) {
    tmpv = d1[i];
    if(d2[i]<thr)
      d1[i] = 0;
    else {
      if (d1[i] > 0) {
	d1[i] *= log(d1[i]*sqrt((d2[i]-2)/(1.01-d1[i]*d1[i])));
      }
    }
    if(d1[i] > tmpmaxv) {
      tmpmaxv = d1[i];
      *maxv = tmpv;
    }
  }

  return d1;  
}

<1t>* <2t>_fun1_i2(<1t>* d1,int *d2,<1t> thr,size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    if(d2[i]<thr)
      d1[i] = 0;
    else
      d1[i] = d1[i]*sqrt(((<1t>)d2[i]-2)/(1.0001-d1[i]*d1[i]));
  }
  return d1; 
}

<1t>* <2t>_fun1_i2_max(<1t>* d1,int *d2,<1t> thr,size_t length,<1t> *maxv)
{
  size_t i;
  <1t> tmpv = d1[0];
  *maxv = tmpv;

  if(d2[0]<thr)
      d1[0] = 0;
    else
      d1[0] = d1[0]*sqrt((d2[0]-2)/(1.0001-d1[0]*d1[0]));
  
  <1t> tmpmaxv = d1[0];
  
  for(i=1;i<length;i++) {
    tmpv = d1[i];
    if(d2[i]<thr)
      d1[i] = 0;
    else
      d1[i] = d1[i]*sqrt((d2[i]-2)/(1.01-d1[i]*d1[i]));

    if(d1[i] > tmpmaxv) {
      tmpmaxv = d1[i];
      *maxv = tmpv;
    }
  }

  return d1;  
}

/**
 *  sqrt(d1-d2*d3). results stored in d1.
 */
<1t>* <2t>_fun2(<1t> *d1,<1t> *d2,<1t> *d3, size_t length)
{
  size_t i;
  for(i=0;i<length;i++)
    d1[i] = sqrt(d1[i]-d2[i]*d3[i]);
 
  return d1;
}
 
/**
 * (d1-d2*d3)/d4. results stored in d1.
 */
<1t>* <2t>_fun3(<1t> *d1,<1t> *d2,<1t> *d3,<1t> *d4, size_t length)
{
  size_t i;
  for(i=0;i<length;i++) {
    if (d4[i] == 0.0) {
      d1[i] = 0.0;
    } else {
      //d1[i] = (d1[i]-d2[i]*d3[i])/d4[i];
      d1[i] = d1[i]/d4[i] - (d2[i]/d4[i])*d3[i];
    }

    /* calibrate invalid values */
    if (d1[i] > 1.01) {
      d1[i] = 0.0;
    } else if (d1[i] > 1.0) {
      d1[i] = 1.0;
    }
    
  }
  
  return d1;
}

<1t>* <2t>_cumsum(<1t>* d1, size_t length)
{
  size_t i;
  for(i=1;i<length;i++)
    d1[i] += d1[i-1];
  return d1;
}

<1t>* <2t>_cumsum_m(<1t>* d1,size_t length,const int *mask)
{
  size_t i, j;
  j = 0;
  for(i=0;i<length;i++) {
    if (mask[i] > 0) {
      if (j > 0) {
	d1[i] += d1[j - 1];
      }
      j = i + 1;
    }
  }
  return d1;
}

<1t>* <2t>_cumsum2(<1t>* d1,int width,int height)
{
  <1t> cur_state = 0;
  int x,y;
  size_t offset=1;
  for(x=1;x<width;x++) {
    d1[offset] += d1[offset-1];
    offset++;
  }

  for(y=1;y<height;y++) {
    cur_state = 0;
    for(x=0;x<width;x++) {
      cur_state += d1[offset];
      d1[offset] = d1[offset-width]+cur_state;
      offset++;
    }
  }

  return d1;
}

<1t> <2t>_sum(const <1t>* d1, size_t length)
{
  size_t i;
  <1t> sum=0;
  for(i=0;i<length;i++)
    sum += d1[i];
  return sum;
}

<1t=double>
<1t> <2t>_sum_n(const <1t>* d1, size_t length)
{
  size_t i;
  <1t> sum=0;
  for(i=0;i<length;i++) {
    if (!isnan(d1[i])) {
      sum += d1[i];
    }
  }
  return sum;
}
</t>

<1t> <2t>_abssum(<1t>* d1, size_t length)
{
  size_t i;
  <1t> sum=0;
  for(i=0;i<length;i++) {
    sum += <2t=darray,farray>f</t>abs(d1[i]);
  }
  return sum;
}

/**
 * same as <2t>_sum. but it might be more precise for summing
 * large amount of floating data.
 */
<1t> <2t>_sum_h(<1t>* d1, size_t length)
{
  if(length==1)
    return d1[0];

  if(length==2)
    return d1[0]+d1[1];

  size_t sublen = length/2;
  return <2t>_sum_h(d1,sublen)+<2t>_sum_h(d1+sublen,length-sublen);
}


<1t> <2t>_centroid(const <1t>* d1, size_t length)
{
  size_t i;
  <1t> totalWeight,totalPos;
  totalWeight = totalPos = 0;

  for(i=0;i<length;i++) {
    totalWeight += d1[i];
    totalPos += d1[i]*i;
  }

  <1t> c = totalPos/totalWeight;

<1t=int,tz_uint8,tz_uint16>
   if( (totalPos % totalWeight) * 2 >= totalWeight )
     c++;
</t>

  return c;
}

double <2t>_centroid_d(const <1t>* d1, size_t length)
{
  size_t i;
  double totalWeight,totalPos;
  totalWeight = totalPos = 0;

  for(i=0;i<length;i++) {
    totalWeight += d1[i];
    totalPos += ((double) d1[i]) * i;
  }

  if (totalWeight == 0.0) {
    return (double) length / 2.0;
  } else {
    return totalPos/totalWeight;
  }
}

/**
 * Local sum of an array. It assumes that the array has 2D subscripts with
 * column-major order and processes the first dimension. The results will be 
 * stored in d2, which should be long enough to hold the data.
 */
void <2t>_linsum1(<1t>* d1,<1t>* d2,int width,int height,int bwidth,int bheight)
{
  int width2 = width+bwidth-1;
  int height2 = height+bheight-1;
  int i,j;
  size_t offset = 0,offset1=0;

  /* rowwise */
  for(j=0;j<bheight-1;j++)
    for(i=0;i<width2;i++) {
      d2[offset] = 0;
      offset++;
    }

  if(bwidth==1) {
    size_t row_offset = ((size_t) bheight - 1) * width;
    size_t src_offset = 0;
    size_t row_size = sizeof(<1t>) * width;
    for(j = 0; j < height; j++) {
      memcpy(d2 + row_offset, d1 + src_offset, row_size);
      row_offset += width;
      src_offset += width;
    }
    return;
  }

  if(width>=bwidth) {
    for(j=bheight-1;j<height2;j++) {
      d2[offset] = d1[offset1];
      offset++;
      offset1++;
      for(i=1;i<bwidth;i++) {
	d2[offset] = d2[offset-1]+d1[offset1];
	offset++;
	offset1++;
      }
      for(i=bwidth;i<width;i++) {
	d2[offset] = d2[offset-1]+d1[offset1]-d1[offset1-bwidth];
	offset++;
	offset1++;
      }
      offset1 -= bwidth;
      for(i=width;i<width2;i++) {
	d2[offset] = d2[offset-1]-d1[offset1];
	offset++;
	offset1++;
      }
      offset1++;
    }
  } else {
    for(j=bheight-1;j<height2;j++) {
      d2[offset] = d1[offset1];
      offset++;
      offset1++;
      for(i=1;i<width;i++) {
	d2[offset] = d2[offset-1]+d1[offset1];
	offset++;
	offset1++;
      }
      for(i=width;i<bwidth;i++) {
	d2[offset] = d2[offset-1];
	offset++;
      }
      offset1 -= width;
      for(i=bwidth;i<width2;i++) {
	d2[offset] = d2[offset-1]-d1[offset1];
	offset++;
	offset1++;
      }
      offset1++;
    }
  } 
}

/**
 * Local sum of an array. It assumes that the array has 2D subscripts with
 * column-major order and processes the second dimension. It's done in place.
 */
void <2t>_linsum2(<1t>* d2,int width2,int height,int bheight)
{
  if(bheight==1)
    return;

  //int i,j,offset,offset1;
  size_t i,j,offset,offset1;
  int height2 = height+bheight-1;
  <1t>* cur_row = <2t>_malloc(width2);
  <1t>* prev_row = <2t>_malloc(width2);
  size_t row_size = (size_t)width2*sizeof(<1t>);

  offset1 = width2*(bheight-1);

  for(i=0;i<width2;i++) {
    d2[i] = d2[offset1];
    offset1++;
  }

  offset = width2;

  if(bheight<=height) {    
    for(j=1;j<bheight;j++){
      if(j==bheight-1)
	memcpy((void*)prev_row,(void*)(d2+offset),row_size);
      for(i=0;i<width2;i++) {
	d2[offset] = d2[offset-width2]+d2[offset1];
	offset++;
	offset1++;
      }
    }
  
    for(j=bheight;j<height;j++) {
      memcpy((void*)cur_row,(void*)(d2+offset),row_size);
      for(i=0;i<width2;i++) {
	d2[offset] = d2[offset1]+d2[offset-width2]-prev_row[i];
	offset++;
	offset1++;
      }
      memcpy((void*)prev_row,(void*)cur_row,row_size);
    }
    
    for(j=height;j<height2;j++) {
      memcpy((void*)cur_row,(void*)(d2+offset),row_size);
      for(i=0;i<width2;i++) {
	d2[offset] = d2[offset-width2] - prev_row[i];
	offset++;
      }
      memcpy((void*)prev_row,(void*)cur_row,row_size);
    }
  } else {
    for(j=1;j<height;j++){
      for(i=0;i<width2;i++) {
	d2[offset] = d2[offset-width2]+d2[offset1];
	offset++;
	offset1++;
      }
    }
      
    memcpy((void*)prev_row,(void*)(d2+width2*(bheight-1)),row_size);

    for(j=height;j<bheight;j++) {
      memcpy((void*)(d2+offset),(void*)(d2+offset-width2),row_size);
      offset += width2;
    }

    for(j=bheight;j<height2;j++) {
      memcpy((void*)cur_row,(void*)(d2+offset),row_size);
      for(i=0;i<width2;i++) {
	d2[offset] = d2[offset-width2]-prev_row[i];
	offset++;
      }	
      memcpy((void*)prev_row,(void*)cur_row,row_size);
    }
  }

  free(prev_row);
  free(cur_row);  
}

/** 
 * Calculate  the sum of each block defined by the template size: 
 * bwidth,bheight. The calculation is performed out place.
 * d1 is supposed to have size width*height
 * The return matrix has size (width+bwidth-1)*(height+bheight-1)
 */
<1t>* <2t>_blocksum(<1t> *d1,int width,int height,int bwidth,int bheight)
{
  int width2 = width+bwidth-1;
  int height2 = height+bheight-1;
  <1t>* d2 = <2t>_malloc2(width2,height2);

  /* sum up rows */
  <2t>_linsum1(d1,d2,width,height,bwidth,bheight);

  /* sum columns */
  <2t>_linsum2(d2,width2,height,bheight);

  
  #ifdef _DEBUG_
 
  printf("%d\n",width2);
  printf("%d\n",height2);
  <2t>_print2(d2,width2,height2);

  #endif
  
  return d2;
}


/**
 * The size of d2 must be at least end-start+1
 */
void <2t>_linsum1_part(const <1t>* d1,<1t>* d2,int width,int bwidth,
    int start, int end)
{
  int width2 = end-start+1;
  int i,minend;
  size_t offset = 0,offset1=start;

  if(bwidth==1) {
    for(i=0;i<width2;i++) {
      d2[i] = d1[offset1++];
    }
    return;
  }

  d2[offset] = 0;

  if(width>=bwidth) {
    if(start<width)
      for(i=imax2(0,start-bwidth+1);i<=start;i++)
	d2[offset] += d1[i];
    else 
      for(i=start-bwidth+1;i<width;i++)
	d2[offset] += d1[i];

    offset++;
    offset1++;
    
    minend = imin2(bwidth,end+1);
    for(i=start+1;i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1];
      offset++;
      offset1++;
    }
    minend = imin2(width,end+1);
    for(i=imax2(bwidth,start+1);i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1]-d1[offset1-bwidth];
      offset++;
      offset1++;
    }
    offset1 -= bwidth;
    for(i=imax2(width,start+1);i<end+1;i++) {
      d2[offset] = d2[offset-1]-d1[offset1];
      offset++;
      offset1++;
    }
  } else {
    if(start<bwidth) {
      minend = imin2(width-1,start);
      for(i=0;i<=minend;i++)
	d2[offset] += d1[i];
    }
    else 
      for(i=start-bwidth+1;i<width;i++)
	d2[offset] += d1[i];

    offset++;
    offset1++;

    minend = imin2(width,end+1);
    for(i=start+1;i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1];
      offset++;
      offset1++;
    }
    minend = imin2(bwidth,end+1);
    for(i=imax2(width,start+1);i<minend;i++) {
      d2[offset] = d2[offset-1];
      offset++;
      offset1++;
    }
    offset1 -= bwidth;
    for(i=imax2(bwidth,start+1);i<end+1;i++) {
      d2[offset] = d2[offset-1]-d1[offset1];
      offset++;
      offset1++;
    }
  } 
}

/**
 * Get essential row sums.
 */
/*
static void <2t>_linsum1_part1(const <1t>*d1,<1t> *d2,int width,int bwidth,int height,int bheight,int col_start,int col_end,int row_start,int row_end)
{
  int i;
  int width2 = col_start+col_end-1;
  int height2 = row_start+row_end-1;
  int offset1,offset2;
  offset1 = row_start*width;
  offset2 = 0;
  for(i=0;i<height2;i++) {
    <2t>_linsum1_part(d1+offset1,d2+offset2,width,bwidth,col_start,col_end);
    offset1 += width;
    offset2 += width2;
  }
  
}
*/

void <2t>_linsum2_part(const <1t>* d1,<1t>* d2,int width2,int height,int bheight, int start, int end)
{
  int height2 = end-start+1;
  int i,minend;
  size_t offset = 0,offset1=start*width2;

  if(bheight==1) {
    for(i=0;i<height2;i++) {
      d2[offset] = d1[offset1];
      offset += width2;
      offset1 += width2;
    }
    return;
  }

  d2[offset] = 0;

  if(height>=bheight) {
    if(start<height)
      for(i=imax2(0,start-bheight+1);i<=start;i++)
	d2[offset] += d1[i*width2];
    else 
      for(i=start-bheight+1;i<height;i++)
	d2[offset] += d1[i*width2];

    offset += width2;
    offset1 += width2;
    
    minend = imin2(bheight,end+1);
    for(i=start+1;i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1];
      offset += width2;
      offset1 += width2;
    }
    minend = imin2(height,end+1);
    for(i=imax2(bheight,start+1);i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1]-d1[offset1-bheight];
      offset += width2;
      offset1 += width2;
    }
    offset1 -= bheight;
    for(i=imax2(height,start+1);i<end+1;i++) {
      d2[offset] = d2[offset-1]-d1[offset1];
      offset += width2;
      offset1 += width2;
    }
  } else {
    if(start<bheight) {
      minend = imin2(height-1,start);
      for(i=0;i<=minend;i++)
	d2[offset] += d1[i*width2];
    }
    else 
      for(i=start-bheight+1;i<height;i++)
	d2[offset] += d1[i*width2];

    offset += width2;
    offset1 += width2;

    minend = imin2(height,end+1);
    for(i=start+1;i<minend;i++) {
      d2[offset] = d2[offset-1]+d1[offset1];
      offset += width2;
      offset1 += width2;
    }
    minend = imin2(bheight,end+1);
    for(i=imax2(height,start+1);i<minend;i++) {
      d2[offset] = d2[offset-1];
      offset += width2;
      offset1 += width2;
    }
    offset1 -= bheight;
    for(i=imax2(bheight,start+1);i<end+1;i++) {
      d2[offset] = d2[offset-1]-d1[offset1];
      offset += width2;
      offset1 += width2;
    }
  }   
}

<1t=double>
double <2t>_sqsum(const <1t> *d1, size_t length)
{
  size_t i;
  double result = 0.0;
  for (i = 0; i < length; i++) {
    result += d1[i] * d1[i];
  }

  return result;
}

double <2t>_norm(const <1t> *d1, size_t length)
{
  return sqrt(<2t>_sqsum(d1, length));
}

double <2t>_simscore(<1t> *d1, <1t> *d2, size_t length)
{
  size_t idx;
  <1t> min1 = <2t>_min(d1, length, &idx);
  <1t> min2 = <2t>_min(d2, length, &idx);
  
  <2t>_subc(d1, min1, length);
  <2t>_subc(d2, min2, length);

  <1t> sum1 = <2t>_sum(d1, length);
  <1t> sum2 = <2t>_sum(d2, length);
  
  <2t>_divc(d1, sum1, length);
  <2t>_divc(d2, sum2, length);

  <2t>_sub(d1, d2, length);

  double score = <2t>_abssum(d1, length);

  return -score;
}
</t>

/*
 * Calculate  the mean of each block defined by the template size: 
 * bwidth,bheight. The first row and first column will be set to 0.
 * The calculation is performed out place if itype is 0. If itype is 
 * not 0, d1 is already the block sum and the calculation will be done
 * in place.
 * The return matrix has size (width+bwidth-1)*(height+bheight-1).
 */
<1t>* <2t>_blockmean(<1t> *d1,int width,int height,int bwidth,int bheight,int itype)
{
  <1t>* d2;
  if(itype)
    d2 = d1;
  else
    d2 = <2t>_blocksum(d1,width,height,bwidth,bheight);

  int i,j;
  size_t offset=0;
  int minwidth = imin2(width,bwidth);
  int minheight = imin2(height,bheight);
  int maxwidth = imax2(width,bwidth);
  int maxheight = imax2(height,bheight);

  int width2 = width+bwidth-1;
  int height2 = height+bheight-1;

  int* cx = (int*)malloc((size_t)width2*sizeof(int));

  for(i=0;i<=width2;i++) {
    if(i<minwidth)
      cx[i] = i+1;
    else { 
      if(i>=maxwidth)
	cx[i] = width2-i;
      else
	cx[i] = minwidth;
    }
  }

  int* cy = (int*)malloc((size_t)height2*sizeof(int));

  for(j=0;j<=height2;j++) {
    if(j<minheight)
      cy[j] = j+1;
    else {
      if(j>=maxheight)
	cy[j] = height2-j;
      else
	cy[j] = minheight;
    }
  }

  for(j=0;j<height2;j++) {
    for(i=0;i<width2;i++) {
      d2[offset] /= cx[i]*cy[j];
      offset++;
    }
  }

  free(cx);
  free(cy);

  return d2;
}

<1t>* <2t>_shiftdim2(<1t>* d1,int width,int height)
{
  int i,j;
  <1t> temp;
  size_t offset1 = 0;
  size_t offset2 = 0;
  for(j=0;j<height;j++) {
    for(i=0;i<width;i++) {
      temp = d1[offset1];
      d1[offset1] = d1[offset1];
      d1[offset2] = temp;
      offset1++;
      offset2 += width;
    }
    offset2 = j;
  }

  return d1;
}

<1t> <2t>_mean(<1t>* d1,size_t length)
{
  return <2t>_sum(d1,length)/((<1t>)length);
}

<1t=double>
<1t> <2t>_mean_n(const <1t>* d1,size_t length)
{
  return <2t>_sum_n(d1,length)/((<1t>)length);
}
</t>

double <2t>_sum_d(const <1t> *d1,size_t length)
{
  double mu=0;
  int i;
  for(i=0;i<length;i++)
    mu += (double)d1[i];

  return mu;
}

double <2t>_mean_d(const <1t> *d1,size_t length)
{
  double mu=0;
  int i;
  for(i=0;i<length;i++)
    mu += (double)d1[i];

  return mu/(double)length;
}

double <2t>_mean_d_m(const <1t> *d1,size_t length, const int *mask)
{
  double mu=0;
  int i;
  size_t length2 = 0;
  for(i=0;i<length;i++) {
    if (mask[i] == 1) {
      mu += (double) d1[i];
      length2++;
    }
  }
  
  if (length2 == 0) {
    return 0.0;
  } else {
    return mu/(double)length2;
  }
}

double <2t>_var(<1t> *d1, size_t length)
{
  if (length <= 1) {
    return 0.0;
  }

  double v = 0.0;
  double mu = 0.0;
  size_t i;
  for(i=0; i<length; i++) {
    v += d1[i] * d1[i];
    mu += d1[i];
  }
  
  v /= (length - 1);

  return v - mu * mu / length / (length - 1);
}

double <2t>_cov(<1t> *d1, <1t> *d2, size_t length)
{
  if (length <= 1) {
    return 0.0;
  }

  double v= 0.0;
  double mu1 = 0.0;
  double mu2 = 0.0;
  size_t i;
  for(i=0; i<length; i++) {
    v += d1[i] * d2[i];
    mu1 += d1[i];
    mu2 += d2[i];
  }
  
  v /= (length - 1);

  return v - mu1* mu2/ length / (length - 1);
}

<1t>* <2t>_malloc(size_t length)
{
  if (length <= 0) {
    return NULL;
  }

  size_t array_size = length * sizeof(<1t>);
  return (<1t>*) malloc(array_size);
}

<1t>* <2t>_calloc(size_t length)
{
  if (length <= 0) {
    return NULL;
  }

  return (<1t>*) calloc(length, sizeof(<1t>));
}

<1t>* <2t>_malloc2(int width,int height)
{
  size_t array_size = (size_t) width * height;
  return <2t>_malloc(array_size);
}

<1t>* <2t>_malloc3(int width,int height,int depth)
{
  size_t array_size = (size_t) width * height * depth;
  return <2t>_malloc(array_size);
}

<1t>* <2t>_copy(<1t>*d1, size_t length)
{
  size_t array_size = length * sizeof(<1t>);
  <1t>* d2 = (<1t>*) malloc(array_size);
  memcpy(d2,d1,array_size);
  return d2;
}

void <2t>_clean_edge3(<1t>* d1,int width,int height,int depth,int margin)
{
  size_t offset = 0;
  int i,j,k;
  for(k=0;k<depth;k++)
    for(j=0;j<height;j++)
      for(i=0;i<width;i++) {
	if(i<=margin || j<=margin || i>=width-margin || j>=height-margin )
	  d1[offset] = 0;
	offset++;
      }
}

void <2t>_printf(<1t> *d1, size_t length, const char *format)
{
  size_t i;
  for (i = 0; i < length; i++) {
    printf(format, d1[i]);
    printf(" ");
  }
  printf("\n");
}

void <2t>_print2(<1t>* d1,int width,int height)
{
  if (d1 == NULL) {
    printf("Null array.\n");
  }

  int i,j;
  size_t offset = 0;
  for(j=0;j<height;j++) {
    for(i=0;i<width;i++) {
<1t=float,double>
      printf("%.4f  ",d1[offset]);
</t>
<1t=int>
      printf("%d  ",d1[offset]);
</t>
<1t=tz_uint8,tz_uint16>
      printf("%u  ",d1[offset]);
</t>	 
      offset++;
    }
    printf("\n");
  } 
  printf("\n");
}

void <2t>_print(<1t>* d1,size_t length)
{
  <2t>_print2(d1, length, 1);
}

void <2t>_fprint2(FILE *fp, <1t>* d1,int width,int height)
{
  int i,j;
  size_t offset = 0;
  for(j=0;j<height;j++) {
    for(i=0;i<width;i++) {
<1t=float,double>
      fprintf(fp, "%.4f  ",d1[offset]);
</t>
<1t=int>
      fprintf(fp, "%d  ",d1[offset]);
</t>
<1t=tz_uint8,tz_uint16>
      fprintf(fp, "%u  ",d1[offset]);
</t>	 
      offset++;
    }
    fprintf(fp, "\n");
  } 
  fprintf(fp, "\n");
}

void <2t>_print3(<1t> *d1,int width,int height,int depth)
{
  int k;
  size_t offset=0;
  int plane_offset=width*height;

  for(k=0;k<depth;k++){
    printf("plane %d:\n",k);
    <2t>_print2(d1+offset,width,height);
    offset += plane_offset;
  }
}

int <2t>_write(const char* filename,const <1t> *d1, int length)
{
  FILE* fp;
  
  if( !(fp=fopen(filename,"wb")) ) {
    perror(strerror(errno));
    return 0;
  }
  
  fwrite(&length, sizeof(int), 1, fp);
  fwrite(d1,sizeof(<1t>),length,fp);

  fclose(fp);

  return 1;
}

<1t>* <2t>_read(const char *filename, int *length)
{
  FILE* fp;
  
  if( !(fp=fopen(filename,"rb")) ) {
    perror(strerror(errno));
    return NULL;
  }
  
  fread(length, sizeof(int), 1, fp);
  <1t> *array = <2t>_malloc(*length);
  fread(array, sizeof(<1t>), *length, fp);

  fclose(fp);

  return array;
}

int <2t>_read2(const char* filename, <1t> *d1,int *length)
{
  FILE* fp;
  
  if( !(fp=fopen(filename,"rb")) ) {
    perror(strerror(errno));
    return 0;
  }
  
  fread(length, sizeof(int), 1, fp);
  fread(d1, sizeof(<1t>), *length, fp);

  fclose(fp);

  return 1;
}

<1t=double,float,int>
size_t <2t>_fscanf(FILE *fp, <1t> *d1, size_t length)
{
<1t=double>
#define <2T>_FSCANF(arg) fscanf(fp, "%lf", arg)
</t>
<1t=float>
#define <2T>_FSCANF(arg) fscanf(fp, "%f", arg)
</t>
<1t=int>
#define <2T>_FSCANF(arg) fscanf(fp, "%d", arg)
</t>

  int read_length = 0;
  char buffer;
  while (!feof(fp)) {
    if (<2T>_FSCANF(d1 + read_length) == 1) {
      read_length++;
    } else {
      fread(&buffer, 1, 1, fp);
    }
    if (read_length >= length) {
      break;
    }
  }

  return read_length;
}

<1t>* <2t>_load_matrix(const char *filepath, <1t> *d, int *m, int *n)
{
  FILE *fp = fopen (filepath, "r");

  String_Workspace *sw = New_String_Workspace();

  *m = 0;
  *n = 0;
  int c = 0;
  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if ((c = Count_Word_D(line, tz_isdlm)) > 0) {
      *m += c;
      (*n)++;
      if (c * (*n) != *m) {
	PRINT_EXCEPTION("Wrong file", "Unable to read the file");
      }
    }
  }
  
  *m /= *n;

  Kill_String_Workspace(sw);
    /*
    int length = 0;
    char str[100];
    while (Read_Word_D(fp, str, 100, tz_isdlm) > 0) {
      length++;
    }
    */

  if (d == NULL) {
    d = <2t>_malloc((*m) * (*n));
  }

  fseek(fp, 0, SEEK_SET);
  <2t>_fscanf(fp, d, (*m) * (*n));

  fclose(fp);

  return d;
}

<1t>* <2t>_load_csv(const char *filepath, <1t> *d, int *m, int *n)
{
  FILE *fp = fopen (filepath, "r");

  String_Workspace *sw = New_String_Workspace();

  *m = 0;
  *n = 0;
  int c = 0;
  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (line[0] == '#') {
      continue;
    }

<1t=double,float>
    if ((c = Count_Number_D(line, tz_isdlm)) > 0) {
</t>
<1t=int>
    if ((c = Count_Integer_D(line, tz_isdlm)) > 0) {
</t>
      *m += c;
      (*n)++;
      if (c * (*n) != *m) {
	PRINT_EXCEPTION("Wrong file", "Unable to read the file");
      }
    }
  }
  
  *m /= *n;

  Kill_String_Workspace(sw);

  if (d == NULL) {
    d = <2t>_malloc((*m) * (*n));
  }

  fseek(fp, 0, SEEK_SET);
  <2t>_fscanf(fp, d, (*m) * (*n));

  fclose(fp);

  return d;
}

</t>


void <2t>cpy(<1t>* d1, const <1t>* d2, size_t offset, size_t length)
{
  memcpy(d1 + offset,d2,length*sizeof(<1t>));
}

void <2t>cpy2(<1t>* d1,<1t>* d2,int width1,int height1,int width2,int height2,int col_offset,int row_offset)
{
  int i;
  size_t offset1,offset2;
  int copy_length = width2*sizeof(<1t>);

  /* offset for d1 */
  offset1 = col_offset+ (size_t) row_offset * width1;

  /* offset for d2 */
  offset2 = 0;

  for(i=0;i<height2;i++) {
    memcpy(d1+offset1,d2+offset2,copy_length);
    offset1 += width1;
    offset2 += width2;
  }
}

void <2t>cpy3(<1t>* d1,<1t>* d2,int width1,int height1,int depth1,int width2,int height2,int depth2,int col_offset,int row_offset,int dep_offset)
{
  int i,k;
  size_t offset1,offset2;
  int copy_length = width2*sizeof(<1t>);

  int plane_offset1 = width1*height1;
  int plane_offset2 = width2*height2;

  //offset for d1
  offset1 = col_offset+row_offset*width1+dep_offset*plane_offset1;

  //offset for d2
  offset2 = 0;

  int tmpoffset1,tmpoffset2;

  for(k=0;k<depth2;k++) {
    tmpoffset1 = offset1;
    tmpoffset2 = offset2;
    for(i=0;i<height2;i++) {
      memcpy(d1+offset1,d2+offset2,copy_length);
      offset1 += width1;
      offset2 += width2;
    }
    offset1 = tmpoffset1 + plane_offset1;
    offset2 = tmpoffset2 + plane_offset2;
  }  
}

/*Modified from http://alienryderflex.com/quicksort/*/
void <2t>_qsort(<1t> *d1, int *idx, int length)
{
  #define  MAX_LEVELS  100
  
  if (idx != NULL) {
    int i;
    for (i = 0; i < length; i++) {
      idx[i] = i;
    }
  }

  <1t> piv;
  int  beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap, pivL ;

  beg[0]=0; end[0]=length;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=d1[L];
      if (idx != NULL) {
	pivL = idx[L];
      }
      while (L<R) {
        while (d1[R]>=piv && L<R) R--; 
	if (L<R) {
	  if (idx != NULL) {
	    idx[L] = idx[R];
	  }
	  d1[L++]=d1[R];
	}
        while (d1[L]<=piv && L<R) L++; 
	if (L<R) {
	  if (idx != NULL) {
	    idx[R] = idx[L];
	  }
	  d1[R--]=d1[L]; 
	}
      }
      d1[L]=piv; 
      if (idx != NULL) {
	idx[L]=pivL;
      }
      beg[i+1]=L+1; 
      end[i+1]=end[i]; end[i++]=L;
      if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
        swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
        swap=end[i]; end[i]=end[i-1]; end[i-1]=swap; 
      }
    } else {
      i--;
    }
  }

  #undef MAX_LEVELS
}

int <2t>_binsearch(<1t> *d1, int length, <1t> value)
{
  if (length == 1) {
    if (d1[0] == value) {
      return 0;
    }
  } else {
    int begin = 0;
    int end = length - 1;
    int index = 0;
    do {
      index = (begin + end) / 2;
      if (value == d1[index]) {
	return index;
      } else if (value < d1[index]) {
	end = index - 1;
      } else {
	begin = index + 1;
      }
    } while (begin < end);

    if (value == d1[begin]) {
      return begin;
    }
  }

  return -1;
}


void <2t>_myqsort(<1t> *d1, int length)
{
  if ((length <= 1) || (d1 == NULL)){
    return;
  }

  <1t> tmp;

  if (length == 2) {
    if (d1[0] > d1[1]) {
      tmp = d1[0];
      d1[0] = d1[1];
      d1[1] = tmp;
    }

    return;
  }

  int pivot_pos = (length - 1)/ 2;
  int left = 0;
  int right = length - 2;
  <1t> pivot;

  /* remove the pivot to last*/
  pivot = d1[pivot_pos];
  d1[pivot_pos] = d1[length - 1];
  d1[length - 1] = pivot;

  while (left <= right) {
    if ((d1[right] < pivot) && (d1[left] > pivot)) {
      tmp = d1[left];
      d1[left] = d1[right];
      d1[right] = tmp;
      left++;
      right--;
    } else {
      if (d1[left] <= pivot) {
	left++;
      } 
    
      if (d1[right] >= pivot) {
	right--;
      }
    }
  }

  pivot_pos = right + 1;
  d1[length - 1] = d1[pivot_pos];
  d1[pivot_pos] = pivot;

  <2t>_myqsort(d1, pivot_pos);
  <2t>_myqsort(d1 + pivot_pos + 1, length - pivot_pos - 1);
}


double <2t>_uint8_corrcoef(const <1t> *filter, const tz_uint8 *signal, 
    size_t length)
{
  <1t> *dsignal  = (<1t> *) malloc(sizeof(<1t>) * length);
  size_t i;
  int is_constant = 1;

  for (i = 0; i < length-1; i++) {
    if (signal[i] != signal[i+1]) {
      is_constant = 0;
      break;
    }
  }

  if (is_constant == 1) {
    return 0.0;
  }

  for (i = 0; i < length; i++) {
    dsignal[i] = (<1t>) signal[i];
  }

  double c = <2t>_corrcoef(filter, dsignal, length);
  free(dsignal);

  return c;
}

double <2t>_corrcoef_m(const <1t> *d1, const <1t> *d2, size_t length,
		       const int *mask)
{
  double mu1 = <2t>_mean_d_m(d1,length,mask);
  double mu2 = <2t>_mean_d_m(d2,length,mask);
  double r,v1,v2;
  double sd1,sd2; 
  size_t i;

  r = v1 = v2 = 0.0;
  
  for(i=0;i<length;i++) {
    if (mask[i] == 1) {
      sd1 = (double)d1[i] - mu1;
      sd2 = (double)d2[i] - mu2;
      r += sd1*sd2;
      v1 += sd1*sd1;
      v2 += sd2*sd2;
    }
  }
  
  if ((v1 == 0.0) || (v2 == 0.0)) {
    return 0.0;
  }

  return r/sqrt(v1*v2);
}

double <2t>_corrcoef(const <1t> *d1, const <1t> *d2, size_t length)
{
  double mu1 = <2t>_mean_d(d1,length);
  double mu2 = <2t>_mean_d(d2,length);
  double r,v1,v2;
  double sd1,sd2; 
  size_t i;

  r = v1 = v2 = 0.0;
  
  for(i=0;i<length;i++) {
    sd1 = (double)d1[i] - mu1;
    sd2 = (double)d2[i] - mu2;
    r += sd1*sd2;
    v1 += sd1*sd1;
    v2 += sd2*sd2;
  }
  
  if ((v1 == 0.0) || (v2 == 0.0)) {
    return 0.0;
  }

  return r/sqrt(v1*v2);
}

<1t=double>
double <2t>_corrcoef_n(const <1t> *d1, const <1t> *d2, size_t length)
{
  double mu1 = <2t>_mean_n(d1,length);
  double mu2 = <2t>_mean_n(d2,length);
  double r,v1,v2;
  double sd1,sd2; 
  int i;

  r = v1 = v2 = 0.0;
  
  for(i=0;i<length;i++) {
    if (!(isnan(d1[i]) || isnan(d2[i]))) {
      sd1 = d1[i] - mu1;
      sd2 = d2[i] - mu2;
      r += sd1*sd2;
      v1 += sd1*sd1;
      v2 += sd2*sd2;
    }
  }
  
  if ((v1 == 0.0) || (v2 == 0.0)) {
    return 0.0;
  }

  return r/sqrt(v1*v2);
}
</t>

<2t=darray,farray>
<2t=darray>
#if defined(HAVE_LIBFFTW3)
</t>
<2t=farray>
#if defined(HAVE_LIBFFTW3F)
</t>
/**
 * DFT of an array. It returns the DFT of the <1t> array <d1> which has logical 
 * length <length>.  The transformation will be done in place if <in_place> is 
 * 1 and out place if <in_place> is 0. Other values of <in_place> has undefined
 * behaviour. For in place transformation, the memory length of <d1> must be at
 * least (length/2+1)*2, which is a little longer than the logical length. 
 * More details can be found in fftw manual. If <preserve> is 1, the value in 
 * <d1> will not changed for out place transformation. If <preserve> is 0, 
 * <d1> might be changed. Other values of <preserve> has undefined behaviour.
 */
<3t>_complex* <2t>_fft(<1t> *d1, int length,int in_place,int preserve)
{
  int flag = FFTW_ESTIMATE;
  <3t>_complex *out;

  if(in_place)
    out = (<3t>_complex*)d1;
  else
    out = <3t>_malloc_r2c_1d(length);

  if(preserve && !in_place)
    flag = flag | FFTW_PRESERVE_INPUT;

  <3t>_plan p = <3t>_plan_dft_r2c_1d(length,d1,out,flag);
  <3t>_execute(p);
  <3t>_destroy_plan(p);

  return out;
}

/**
 * IDFT of the DFT of a <1t> array. <c> should be the result from 
 * <2t>_fft and <length> is the logical length of the origina array. 
 * <in_place> and <preserve> have the same meaning as those for <2t>_fft.
 * If <normalize> is 1, the result will be normalized. If <normalize> is 0,
 * the result will be kept unnormalized. Other values of <normalize> have
 * undefined behaviour.
 */
<1t>* <2t>_ifft(<3t>_complex *c, int length, int in_place,
		int preserve, int normalize)
{
  int flag = FFTW_ESTIMATE;
  <1t> *out;

  if(in_place)
    out = (<1t> *) c;
  else
    out = (<1t> *) <3t>_malloc(sizeof(<1t>)*length);
    
  if(preserve && !in_place)
    flag = flag | FFTW_PRESERVE_INPUT;

  <3t>_plan p = <3t>_plan_dft_c2r_1d(length,c,out,flag);
  <3t>_execute(p);
  <3t>_destroy_plan(p);

  if(normalize) {
    <2t>_divc(out, (<1t>)length, length);
  }

  return out;
}

/**
 * Convolve two arrays. The array convolution must have at least 
 * ((<length1>+<length2>-1)/2+1)*2 elements if it is not NULL. If <convolution>
 * is NULL, the returned array must be freed by <3t>_free after using. 
 */
<1t>* <2t>_convolve(<1t> *d1,int length1,<1t> *d2,int length2,int reflect,
		    <1t> *convolution)
{
  if(d1==NULL || d2==NULL || length1<=0 || length2<=0) {
    //add warning here
    return NULL;
  }

  <1t> *pd1;  //padded array of d1
  <1t> *pd2;  //padded array of d2
  int i;
  int padded_length = length1+length2-1; //length of the padded array
  int fft_length = R2C_LENGTH(padded_length); //length for in place trasnformation

  if(convolution==NULL)
    pd1 = (<1t> *) <3t>_malloc(sizeof(<3t>_complex)*fft_length);
  else
    pd1 = convolution;

  pd2 = (<1t> *) <3t>_malloc(sizeof(<3t>_complex)*fft_length);
  
  for(i=0;i<padded_length;i++) { //initialize the padded arrays
    pd1[i] = 0;
    pd2[i] = 0;
  }

  if(reflect)
      memcpy(pd1+length2-1,d1,sizeof(<1t>)*length1);
  else
    memcpy(pd1,d1,sizeof(<1t>)*length1);
  memcpy(pd2,d2,sizeof(<1t>)*length2);

  /*fft*/
  <2t>_fft(pd1,padded_length,1,0);
  <2t>_fft(pd2,padded_length,1,0);

  if(reflect)  //to calculate correlation
    <3t>_conjg_array( (<3t>_complex *) pd2, fft_length );


  <3t>_cmul_array( (<3t>_complex *) pd1, (<3t>_complex *) pd2, fft_length );
  <3t>_free(pd2);
  <2t>_ifft( (<3t>_complex *) pd1, padded_length, 1, 0, 1 );

  return pd1;
}

#endif

<1t>* <2t>_avgsmooth(const <1t>* in, size_t length, int wndsize, <1t> *out)
{
  size_t i, j;
  int right_span = wndsize / 2;
  int left_span = wndsize - right_span - 1;

  for (i = 0; i < length; i++) {
    size_t left_side = 0;
    if (i > left_span) {
      left_side = i - left_span;
    }
    //imax2(i - left_span, 0);
    size_t right_side = imin2(i + right_span, length - 1);
    size_t size = right_side - left_side + 1;
    //printf("%d\n", right_side - left_side + 1);
    out[i] = 0.0;
    for (j = left_side; j <= right_side; j++) {
      out[i] += in[j];
    }
    out[i] /= size;
  }

  return out;
}

<1t>* <2t>_curvature(const <1t>* curve, size_t length, <1t> *out)
{
  size_t i;

  <1t> *array = <2t>_malloc(length);

  <2t>_cendiff(curve, length, array);
  <2t>_cendiff(array, length, out);

  for (i = 0; i < length; i++) {
    array[i] = 1.0 + array[i] * array[i];
    array[i] = sqrt(array[i] * array[i] * array[i]);
    out[i] /= -array[i];
  }

  free(array);

  return out;
}

<1t>* <2t>_cendiff(const <1t>* in, size_t length, <1t> *out)
{
  out[0] = in[1] - in[0];
  out[length - 1] = in[length - 1] - in[length - 2];
  size_t i;
  for (i = 1; i < length - 1; i++) {
    out[i] = (in[i + 1] - in[i - 1]) / 2.0;
  }

  return out;
}

</t>

<1t>* <2t>_medfilter(const <1t> *in, size_t length, int wndsize, <1t> *out)
{
  if (out == NULL) {
    out = <2t>_malloc(length);
  }

  <1t> *buffer = <2t>_malloc(wndsize);

  size_t i;
  int right_span = wndsize / 2;
  int left_span = wndsize - right_span - 1;

  for (i = 0; i < length; i++) {
    size_t left_side = 0;
    if (i > left_span) {
      left_side = i - left_span;
    }
    //imax2(i - left_span, 0);
    size_t right_side = imin2(i + right_span, length - 1);
    size_t size = right_side - left_side + 1;
    memcpy(buffer, in + left_side, sizeof(<1t>) * size);
    //printf("%d\n", right_side - left_side + 1);
    <2t>_qsort(buffer, NULL, size);

    size_t medpos = size / 2;
    if (size % 2 == 0) {
      if (buffer[medpos - 1] == in[i]) {
        out[i] = in[i];
      } else {
        out[i] = buffer[medpos];
      }
    } else {
      out[i] = buffer[medpos];
    }
  }

  free(buffer);

  return out;
}


<1t=double,float,int>
int <2t>_iszero(const <1t> *d, size_t length)
{
#ifdef HAVE_LIBGSL
  <1t=double>
  gsl_vector_const_view v = gsl_vector_const_view_array(d, length);
  return gsl_vector_isnull(&(v.vector));
  </t>
  
  <1t=int,float>
  gsl_vector_<1t>_const_view v = gsl_vector_<1t>_const_view_array(d, length);
  return gsl_vector_<1t>_isnull(&(v.vector));
  </t>
#else
  size_t i;
  for (i = 0; i < length; i++) {
    if (d[i] != 0) {
      return 0;
    }
  }

  return 1;
#endif
}

<1t>* <2t>_contarray(size_t start, size_t end, <1t> *d)
{
  if (start > end) {
    return NULL;
  }

  if (d == NULL) {
    d = <2t>_malloc(end - start + 1);
  }

  int i;
  int offset = 0;

  for (i = start; i <= end; i++) {
    d[offset++] = i;
  }

  return d;
}
</t>

void <2t>_reverse(<1t> *d, size_t length)
{
  if (length <= 1) {
    return;
  }

  size_t length2 = length / 2;
  size_t i, j;
  <1t> temp;
  for (i = 0, j = length - 1; i < length2; i++, j--) {
    temp = d[i]; 
    d[i] = d[j];
    d[j] = temp;
  }
}
