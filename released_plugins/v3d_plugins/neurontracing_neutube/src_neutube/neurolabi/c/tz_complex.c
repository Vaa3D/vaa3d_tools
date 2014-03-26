#include <stdio.h>
#include "tz_complex.h"

void fftw_cadd(fftw_complex c1,const fftw_complex c2)
{
  c1[0] += c2[0];
  c1[1] += c2[1];
}

void fftw_csub(fftw_complex c1,const fftw_complex c2)
{
  c1[0] -= c2[0];
  c1[1] -= c2[1];  
}

void fftw_cmul(fftw_complex c1,const fftw_complex c2)
{
  double temp;
  temp = c1[0];
  c1[0] = c1[0]*c2[0] - c1[1]*c2[1];
  c1[1] = temp*c2[1] + c1[1]*c2[0];
}

void fftw_conjg(fftw_complex c)
{
  c[1] = -c[1];
}

/**
 * fftw_cadd_array, fftw_csub_array, fftw_cmul_array, fftw_conjg_array
 * Operations on complex number arrays. They are all done in place, i.e.
 * results will be stored in the first argument.
 */
fftw_complex* fftw_cadd_array(fftw_complex* c1,fftw_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftw_cadd(c1[i],c2[i]);
  }
  return c1;
}

fftw_complex* fftw_csub_array(fftw_complex* c1,fftw_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftw_csub(c1[i],c2[i]);
  }
  return c1;
}

fftw_complex* fftw_cmul_array(fftw_complex* c1,fftw_complex* c2,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftw_cmul(c1[i],c2[i]);
  }
  return c1;
}

fftw_complex* fftw_conjg_array(fftw_complex* c,long length)
{
  long i;
  for(i=0;i<length;i++) {
    fftw_conjg(c[i]);
  }
  return c;
}

void fftw_print(fftw_complex *c,long length)
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
