#include <stdio.h>
#include "tz_complex.h"

void fftwf_cadd(fftwf_complex c1,const fftwf_complex c2)
{
  c1[0] += c2[0];
  c1[1] += c2[1];
}

void fftwf_csub(fftwf_complex c1,const fftwf_complex c2)
{
  c1[0] -= c2[0];
  c1[1] -= c2[1];  
}

void fftwf_cmul(fftwf_complex c1,const fftwf_complex c2)
{
  float temp;
  temp = c1[0];
  c1[0] = c1[0]*c2[0] - c1[1]*c2[1];
  c1[1] = temp*c2[1] + c1[1]*c2[0];
}

void fftwf_conjg(fftwf_complex c)
{
  c[1] = -c[1];
}

/**
 * fftwf_cadd_array, fftwf_csub_array, fftwf_cmul_array, fftwf_conjg_array
 * Operations on complex number arrays. They are all done in place, i.e.
 * results will be stored in the first argument.
 */
fftwf_complex* fftwf_cadd_array(fftwf_complex* c1,fftwf_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftwf_cadd(c1[i],c2[i]);
  }
  return c1;
}

fftwf_complex* fftwf_csub_array(fftwf_complex* c1,fftwf_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftwf_csub(c1[i],c2[i]);
  }
  return c1;
}

fftwf_complex* fftwf_cmul_array(fftwf_complex* c1,fftwf_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftwf_cmul(c1[i],c2[i]);
  }
  return c1;
}

fftwf_complex* fftwf_conjg_array(fftwf_complex* c,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftwf_conjg(c[i]);
  }
  return c;
}

void fftwf_print(fftwf_complex *c,long length)
{
  long i;
  for(i=0;i<length;i++) {
    if(c[i][1]==0.0)
      printf("%f ",c[i][0]);
    else if(c[i][1]<0)
      printf("%.4f-%.4fi ",c[i][0],-c[i][1]);
    else
      printf("%.4f+%.4fi ",c[i][0],c[i][1]);
  }

  printf("\n");
}

