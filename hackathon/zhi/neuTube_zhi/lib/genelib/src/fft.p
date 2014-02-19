/*****************************************************************************************\
*                                                                                         *
*  FFT algorithms including convolution and correlation and multiple dimensions           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2007                                                                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "utilities.h"
#include "fft.h"

#define TPI 6.28318530717959

#undef  DEBUG
#undef  DEBUGn
#undef  DEBUG_COMPn

#ifdef DEBUG

static void print_complex(int n, Complex *array, char *title)
{ int i;

  printf("\n%s:\n",title);
  for (i = 0; i < n; i++)
    { printf("   %4d",i);
      if (fabs(array[i].real) < 1e-13)
        printf(": 0 + i ");
      else
        printf(": %.5g + i ",array[i].real);
      if (fabs(array[i].imag) < 1e-13)
        printf("0\n");
      else
        printf("%.5g\n",array[i].imag);
    }
}

static void print_double(int n, double *array, char *title)
{ int i;

  printf("\n%s:\n",title);
  for (i = 0; i < n; i++)
    { printf("   %4d",i);
      if (fabs(array[i]) < 1e-13)
        printf(": 0\n");
      else
        printf(": %.5g\n",array[i]);
    }
}

#endif

//  All the FFT routines assume n is a power of 2.  As a convenience to help the user pad
//    their vectors to this size, Power_Of_2_Pad returns the smallest power of 2 greater
//    than or equal to m.

int Next_Power_Of_2(int m)
{ int n;

  n = 2;
  while (n < m)
    n <<= 1;
  return (n);
}


//  Basic 1-dimenstional FFT-algorithm.  The FFT is performed in-place within 'data' and
//    for convenience a pointer to data is returned by FFT.  If invert is non-zero then
//    the inverse Fourier Transform is performed.

Complex *FFT(int n, Complex *data, int invert)
{
  { int     i, j, b;        //  Reorder data w.r.t. bit reversal of index
    Complex x;              //  The inductive invariant is j = rev(i) 
    int    n2 = (n >> 1);

    j = 0;
    for (i = 1; i < n; i++)
      { b = n2;         //  Add '1' to j in bit reversal order
        while (j >= b)
          { j -= b;     //  Carry propogation
            b >>= 1; 
          }
        j += b;

        if (i < j)            //  Swap each (i,j) pair once!
          { x = data[i];
            data[i] = data[j];
            data[j] = x;
          }  
      }
  }

  if (n >= 2)    //  Special case h=2 iteration (see below).  Roots of unity are 1, -1
    { int j;

      for (j = 0; j < n; j += 2)      // data[j]   = data[j] + data[j+1]
                                      // data[j+1] = data[j] - data[j+1]
        { int    i  = j+1;
          double jr = data[j].real;
          double ji = data[j].imag;
          double ir = data[i].real;
          double ii = data[i].imag;

          data[j].real = jr + ir;
          data[j].imag = ji + ii;
          data[i].real = jr - ir;
          data[i].imag = ji - ii;
        }
    }

  if (n >= 4)    //  Special case h=4 iteration (see below).  Roots of unity are 1, i, -1, -i
    { int j;

      if (invert)                  //  Sneaky: if inverting this is equivalent to (0,-1) below
        for (j = 3; j < n; j += 4)
          { data[j].real = -data[j].real;
            data[j].imag = -data[j].imag;
          }

      for (j = 0; j < n; j += 4)      // data[j]   = data[j]   +       data[j+2]
                                      // data[j+2] = data[j]   -       data[j+2]
        { int    k  = j+1;            // data[j+1] = data[j+1] + (0,1)*data[j+3]
          int    i  = j+2;            // data[j+3] = data[j+1] - (0,1)*data[j+3]
          double jr = data[j].real;
          double ji = data[j].imag;
          double ir = data[i].real;
          double ii = data[i].imag;

          data[j].real = jr + ir;
          data[j].imag = ji + ii;
          data[i].real = jr - ir;
          data[i].imag = ji - ii;

          i  = k+2;
          jr = data[k].real;
          ji = data[k].imag;
          ir = data[i].real;
          ii = data[i].imag;

          data[k].real = jr - ii;
          data[k].imag = ji + ir;
          data[i].real = jr + ii;
          data[i].imag = ji - ir;
        }
    }

  { int  s, h;    //  General FFT iterations starting with h = 8.

    s = 4;
    for (h = 8; h <= n; h <<= 1)
      { double  uhr, uhi;
        double  cos0, sin0;
        int     k, j;

        { double  theta;

          if (invert)        // Need cos & sin of 2pi/h to compute roots incrementally
            theta = -TPI/h;
          else
            theta =  TPI/h;
          cos0 = cos(theta);
          sin0 = sin(theta);
        }

        uhr = 1.0;         // uhr + i*uhi is the h'th principle root of unity to the power k
        uhi = 0.0;         //   as the outer loop below progresses through values of k

        for (k = 0; k < s; k++)
          { for (j = k; j < n; j += h)      // data[j]   = data[j] + (uhr,uhi)*data[j+s]
                                            // data[j+s] = data[j] - (uhr,uhi)*data[j+s]
              { int    i  = j+s;
                double jr = data[j].real;
                double ji = data[j].imag;
                double ir = data[i].real;
                double ii = data[i].imag;
                double pr = uhr * ir - uhi * ii;
                double pi = uhr * ii + uhi * ir;

                data[j].real = jr + pr;
                data[j].imag = ji + pi;
                data[i].real = jr - pr;
                data[i].imag = ji - pi;
              }

            { double x;                         //  Update (uhr,uhi) to the next h'th root of
                                                //     unity: (uhr,uhi) *= (cos0,sin0)
              x   = uhr * cos0 - uhi * sin0;
              uhi = uhr * sin0 + uhi * cos0;
              uhr = x;
            }
          }

        s = h;
      }
  }

  if (invert)       //  Normalize terms by n if an inverse fft
    { int    k;
      double no = n;

      for (k = 0; k < n; k++)
        { data[k].real /= no;
          data[k].imag /= no;
        }
    }

  return (data);
}


//  FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The forward transform, Real_FFT, takes a double array of length n, and *in-place* produces 
//    a Complex array c of length n/2 that is the first half of the conjugate symmetric FFT
//    of the real data with the exception that F_(n/2) (which is real) is tucked into c[0].imag
//    (which works as F_0 is also real).  Again, the pointer returned by FFT is really the same
//    as rdata, the FFT is performed in-place. 

Complex *Real_FFT(int n, double *rdata)
{ int    k;
  double unr, uni;

  Complex *cdata = (Complex *) rdata;
  int      n2    = n/2;

  double theta = TPI / n;      // Need cos & sin of 2pi/n to compute roots incrementally
  double cos0  = cos(theta);
  double sin0  = sin(theta);
  
  FFT(n2,cdata,0);

  unr = cos0;         // unr + i*uni is the n'th principle root of unity to the power k
  uni = sin0;         //   as the outer loop below progresses through values of k

  { double zr = cdata[0].real;   //  Special case for the 0th element whose x'forms are real 
    double zi = cdata[0].imag;

    cdata[0].real = zr + zi;     //  Put F_0 in .real and F_n2 in .imag of 0th element
    cdata[0].imag = zr - zi;     //  Works as both are real!
  }

  for (k = 1; k <= n/4; k++)
    { int j = n2-k;

      double kr = cdata[k].real;    //  Forward xform:
      double ki = cdata[k].imag;    //    Ck = .5 ((Ck+Cj*) - i(w^n)(Ck-Cj*)) for Cx = cdata[x]
      double jr = cdata[j].real;
      double ji = cdata[j].imag;

      double f0r = .5 * (kr + jr);
      double f0i = .5 * (ki - ji);
      double f1r = .5 * (ji + ki);
      double f1i = .5 * (jr - kr);

      cdata[k].real =  f0r + unr*f1r - uni*f1i;
      cdata[k].imag =  f0i + unr*f1i + uni*f1r;
      cdata[j].real =  f0r - unr*f1r + uni*f1i;
      cdata[j].imag = -f0i + unr*f1i + uni*f1r;

      { double x;                         //  Update (unr,uni) to the next n'th root of
                                          //     unity: (unr,uni) *= (cos0,sin0)
        x   = unr * cos0 - uni * sin0;
        uni = unr * sin0 + uni * cos0;
        unr = x;
      }
    }

  return (cdata);
} 


//  FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The inverse transform, Real_FFT_Inverse, takes a complex half-matrix as produced by
//    Real_FFT, and produces a real-valued result *in-place*.  That is, the pointer returned is
//    exactly cdata (coerced to be double *).  Note carefully that n is the length of the
//    resulting real array and is twice the length of cdata.

double *Real_FFT_Inverse(int n, Complex *cdata)
{ int    k;
  double unr, uni;

  int    n2    = n/2;
  double theta = TPI / n;      // Need cos & sin of 2pi/n to compute roots incrementally
  double cos0  = cos(theta);
  double sin0  = sin(theta);

  unr = cos0;         // unr + i*uni is the n'th principle root of unity to the power k
  uni = sin0;         //   as the outer loop below progresses through values of k

  { double zr = cdata[0].real;   //  Special case for the 0th element whose x'forms are real 
    double zi = cdata[0].imag;

    cdata[0].real = .5*(zr + zi);
    cdata[0].imag = .5*(zr - zi);
  }

  for (k = 1; k <= n2/2; k++)
    { int j = n2-k;

      double kr = cdata[k].real;    //  Reverse xform:
      double ki = cdata[k].imag;    //    Ck = .5 ((Ck+Cj*) + i(w^n)*(Ck-Cj*)) for Cx = cdata[x]
      double jr = cdata[j].real;
      double ji = cdata[j].imag;

      double f0r = .5 * (kr + jr);
      double f0i = .5 * (ki - ji);
      double f1r = .5 * (ji + ki);
      double f1i = .5 * (jr - kr);

      cdata[k].real =  f0r - unr*f1r - uni*f1i;
      cdata[k].imag =  f0i - unr*f1i + uni*f1r;
      cdata[j].real =  f0r + unr*f1r + uni*f1i;
      cdata[j].imag = -f0i - unr*f1i + uni*f1r;

      { double x;                         //  Update (unr,uni) to the next n'th root of
                                          //     unity: (unr,uni) *= (cos0,sin0)
        x   = unr * cos0 - uni * sin0;
        uni = unr * sin0 + uni * cos0;
        unr = x;
      }
    }

  return ((double *) FFT(n2,cdata,1));
} 


//  FFT-algorithm for convolution.  The second argument signal is overwritten with the result
//    and returned as the result of the function for the users convenience.  Both inputs are
//    destroyed in the process.  The code works if signal and filter are the same array.

double *FFT_Convolution(int n, double *signal, double *filter)
{ Complex *s_fft, *f_fft;
  int      k;
  int      n2 = n/2;

  s_fft = Real_FFT(n,signal);
  if (signal != filter)
    f_fft = Real_FFT(n,filter);
  else
    f_fft = s_fft;
    
  //  In one step decode the first-half of 2 real-valued FFT's in data
  //    and pairwise multiply them.  0 is a special case, pack F_(n/2) into
  //    the imaginary part of data[0] in preparation for a real fft inversion.

  s_fft[0].real *= f_fft[0].real;
  s_fft[0].imag *= f_fft[0].imag;

  for (k = 1; k < n2; k++)
    { double kr = s_fft[k].real;
      double ki = s_fft[k].imag;
      double jr = f_fft[k].real;
      double ji = f_fft[k].imag;

      s_fft[k].real = kr*jr - ki*ji;
      s_fft[k].imag = ki*jr + kr*ji;
    }

  return (Real_FFT_Inverse(n,s_fft));
}


//  FFT-algorithm for correlation.  The second argument seq1 is overwritten with the result
//    and returned as the result of the function for the users convenience.  Both inputs
//    are stored in the process.  The code works if seq1 and seq2 are the same array.

double *FFT_Correlation(int n, double *seq1, double *seq2)
{ Complex *s_fft, *f_fft;
  int      k;
  int      n2 = n/2;

  s_fft = Real_FFT(n,seq1);
  if (seq1 != seq2)
    f_fft = Real_FFT(n,seq2);
  else
    f_fft = s_fft;

  //  In one step decode the first-half of 2 real-valued FFT's in data
  //    and pairwise multiply them.  0 is a special case, pack F_(n/2) into
  //    the imaginary part of data[0] in preparation for a real fft inversion.

  s_fft[0].real *= f_fft[0].real;
  s_fft[0].imag *= f_fft[0].imag;

  for (k = 1; k < n2; k++)
    { double kr = s_fft[k].real;
      double ki = s_fft[k].imag;
      double jr = f_fft[k].real;
      double ji = f_fft[k].imag;

      s_fft[k].real = kr*jr + ki*ji;
      s_fft[k].imag = ki*jr - kr*ji;
    }

  return (Real_FFT_Inverse(n,s_fft));
}

#ifdef DEBUG

#define DSIZE 1048576

int main(int argc, char *argv[])
{ static double rata[DSIZE], fata[DSIZE];
  int i, k;

  for (k = 0; k < 100; k++)

    { for (i = 0; i < DSIZE; i++)
        { rata[i] = i;
          fata[i] = DSIZE-i;
        }

      FFT_Convolution(DSIZE,rata,rata);
    }

  exit (0);
}

#endif

#ifdef DEBUGn

void print_index(int ndim,int *dims, int k)
{ int d;

  if (ndim > 1)
    { printf("%4d",k/dims[ndim-2]);
      for (d = ndim-2; d > 0; d--)
        printf(",%4d",(k%dims[d])/dims[d-1]);
      printf(",%4d",k%dims[0]);
    }
  else
    printf("%4d",k);
}

static void print_complex_n(int ndim, int *dims, Complex *array, char *title)
{ int i, d;

  for (d = 1; d < ndim; d++)
    dims[d] *= dims[d-1];

  printf("\n%s:\n",title);
  for (i = 0; i < dims[ndim-1]; i++)
    { printf("   ");
      print_index(ndim,dims,i);
      if (fabs(array[i].real) < 1e-13)
        printf(": 0 + i ");
      else
        printf(": %.5g + i ",array[i].real);
      if (fabs(array[i].imag) < 1e-13)
        printf("0\n");
      else
        printf("%.5g\n",array[i].imag);
    }

  for (d = ndim-1; d > 0; d--)
    dims[d] /= dims[d-1];
}

static void print_double_n(int ndim, int *dims, double *array, char *title)
{ int i, d;

  for (d = 1; d < ndim; d++)
    dims[d] *= dims[d-1];

  printf("\n%s:\n",title);
  for (i = 0; i < dims[ndim-1]; i++)
    { printf("   ");
      print_index(ndim,dims,i);
      if (fabs(array[i]) < 1e-13)
        printf(": 0\n",array[i]);
      else
        printf(": %.5g\n",array[i]);
    }

  for (d = ndim-1; d > 0; d--)
    dims[d] /= dims[d-1];
}

#endif

//  Basic n-dimenstional FFT-algorithm.  The FFT is performed in-place within 'data' and
//    for convenience a pointer to data is returned by FFT.  If invert is non-zero then
//    the inverse Fourier Transform is performed.

Complex *FFT_n(int ndim, int *dims, Complex *data, int invert)
{ int size;
  int area;
  int base;
  int n, d;

  size = 1;                     //  Get total size of array
  for (d = 0; d < ndim; d++)
    size *= dims[d];

  area = 1;
  for (d = 0; d < ndim; d++)     //  For each dimension do
    { n     = dims[d];  
      base  = area;
      area *= n;

      if (n == 1) continue;     //  Trivial dimension?

      { int     i, j, k, b;     //  Reorder data w.r.t. bit reversal of index
        int    n2 = (n >> 1);   //  The inductive invariant is j = rev(i/base) 
    
        j = 0;
        for (i = base; i < area; i += base)
          { b = n2;         //  Add '1' to j in bit reversal order
            while (j >= b)
              { j -= b;     //  Carry propogation
                b >>= 1; 
              }
            j += b;

            k = j*base;
            if (i < k)                     //  Swap each (i,k) pair once!
              { int     x, y;
                Complex c;
                for (x = 0; x < base; x++)
                  for (y = x; y < size; y += area)
                    { Complex *dp = data+y;
                      c     = dp[i];
                      dp[i] = dp[k];
                      dp[k] = c;
                    }  
              }  
          }
      }
    
      //  Special case h=2 iteration (see below).  Roots of unity are 1, -1

      { int j, x, y;
        int base2 = 2*base;
    
        for (j = 0; j < area; j += base2)      // data[j]   = data[j] + data[j+1]
                                               // data[j+1] = data[j] - data[j+1]
          for (x = 0; x < base; x++)
            for (y = x; y < size; y += area)
              { Complex *dp = data+y;
                int       i = j+base;

                double jr = dp[j].real;
                double ji = dp[j].imag;
                double ir = dp[i].real;
                double ii = dp[i].imag;
  
                dp[j].real = jr + ir;
                dp[j].imag = ji + ii;
                dp[i].real = jr - ir;
                dp[i].imag = ji - ii;
              }
      }

      if (n == 2) continue;
    
      //  Special case h=4 iteration (see below).  Roots of unity are 1, i, -1, -i

      { int j, x, y;
        int base2 = 2*base;
        int base4 = 4*base;
    
        if (invert)                  //  Sneaky: if inverting this is equivalent to (0,-1) below
          for (j = 3*base; j < area; j += base4)
            for (x = 0; x < base; x++)
              for (y = x; y < size; y += area)
                { Complex *dp = data+y;
                  dp[j].real = -dp[j].real;
                  dp[j].imag = -dp[j].imag;
                }
   
        for (j = 0; j < area; j += base4)      // data[j]   = data[j]   +       data[j+2]
                                               // data[j+2] = data[j]   -       data[j+2]
          for (x = 0; x < base; x++)           // data[j+1] = data[j+1] + (0,1)*data[j+3]
            for (y = x; y < size; y += area)   // data[j+3] = data[j+1] - (0,1)*data[j+3]
              { Complex *dp = data+y;
                int      k  = j+base;  
                int      i  = j+base2;

                double jr = dp[j].real;
                double ji = dp[j].imag;
                double ir = dp[i].real;
                double ii = dp[i].imag;
      
                dp[j].real = jr + ir;
                dp[j].imag = ji + ii;
                dp[i].real = jr - ir;
                dp[i].imag = ji - ii;
      
                i  = k+base2;
                jr = dp[k].real;
                ji = dp[k].imag;
                ir = dp[i].real;
                ii = dp[i].imag;
      
                dp[k].real = jr - ii;
                dp[k].imag = ji + ir;
                dp[i].real = jr + ii;
                dp[i].imag = ji - ir;
              }
      }

      if (n == 4) continue;

      { int  s, h;    //  General FFT iterations starting with h = 8.
    
        s = 4;
        for (h = 8; h <= n; h <<= 1)
          { double  uhr, uhi;
            double  cos0, sin0;
            int     k, j;
            int     x, y;
            int     sbase = s*base;
            int     hbase = h*base;
    
            { double  theta;
    
              if (invert)        // Need cos & sin of 2pi/h to compute roots incrementally
                theta = -TPI/h;
              else
                theta =  TPI/h;
              cos0 = cos(theta);
              sin0 = sin(theta);
            }
    
            uhr = 1.0;         // uhr + i*uhi is the h'th principle root of unity to the power k
            uhi = 0.0;         //   as the outer loop below progresses through values of k
    
            for (k = 0; k < sbase; k += base)
              { for (j = k; j < area; j += hbase)  // data[j]   = data[j] + (uhr,uhi)*data[j+s]
                                                   // data[j+s] = data[j] - (uhr,uhi)*data[j+s]
                  for (x = 0; x < base; x++)
                    for (y = x; y < size; y += area)
                      { Complex *dp = data+y;
                        int      i  = j+sbase;

                        double jr = dp[j].real;
                        double ji = dp[j].imag;
                        double ir = dp[i].real;
                        double ii = dp[i].imag;

                        double pr = uhr * ir - uhi * ii;
                        double pi = uhr * ii + uhi * ir;
    
                        dp[j].real = jr + pr;
                        dp[j].imag = ji + pi;
                        dp[i].real = jr - pr;
                        dp[i].imag = ji - pi;
                      }
    
                { double x;                         //  Update (uhr,uhi) to the next h'th root of
                                                    //     unity: (uhr,uhi) *= (cos0,sin0)
                  x   = uhr * cos0 - uhi * sin0;
                  uhi = uhr * sin0 + uhi * cos0;
                  uhr = x;
                }
              }
    
            s = h;
          }
      }
    }

  if (invert)       //  Normalize terms by n if an inverse fft
    { int    k;
      double no = size;

      for (k = 0; k < size; k++)
        { data[k].real /= no;
          data[k].imag /= no;
        }
    }

  return (data);
}


//  Multi-dimensional FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The forward transform, Real_FFT_n, takes a double array of size s, and *in-place* produces
//    a Complex array c of size s/2 that is the conjugate symmetric FFT of the real data for
//    the first half of the lowest dimension [0..dims[0]/2-1].  In order to make it fully
//    invertible F[0,...,0,dims[0]/2] is tucked into c[0,...,0,0].imag (which works as both are
//    real values.)  Again, the pointer returned by FFT is really the same as rdata, the
//    FFT is performed in-place.

Complex *Real_FFT_n(int ndim, int *dims, double *rdata)
{ int    d0;
  int    base[10], cntr[10];
  int    size, nsub, ncmp;

  Complex *cdata = (Complex *) rdata;

  double theta = TPI / dims[0];   // Need cos & sin of 2pi/n to compute roots incrementally
  double cos0  = cos(theta);
  double sin0  = sin(theta);

  dims[0] = d0 = dims[0] / 2;

  if (ndim > 10)
    { fprintf(stderr,"Real_FFT_n: Dimension of array is greater than 10!\n");
      exit (1);
    }
  
  FFT_n(ndim,dims,cdata,0);

  { double zr = cdata[0].real;   //  Special case for the 0th element whose x'forms are real 
    double zi = cdata[0].imag;

    cdata[0].real = zr + zi;   //  Put F_(0,...,0) in .real and F_(0,...,n0/2) in .imag of 0th
    cdata[0].imag = zr - zi;   //    element.  Works as both are real!
  }

//  To compute F(in,...,i1,i0) for ik in [0,nk-1] and i0 in [0,n0/2-1] you need both
//    H(in,...,i1,i0) and H(in*,...i1*,i0*) where ik* = nk-ik, and you need w^i0 and
//    w^(io*).  Unfortunately you don't have the ik* term available if ik == 0.
//    Fortunately, its the same as the term with ik* = 0 by cyclicity of the transform.
//    w^(i0*) = w^(i0)* for i0 > 0 but not for i0 = 0!  So we break out the i0 = 0 case
//    immediately below and then handle i0 > 0 in the loop that follows.
//   
//  Given the integer index k corresponding to the entry (in,...,i1,i0) we have the
//    interesting problem of computing the index k* for the entry (in*,...,i1*,i0*)
//    given the cyclic definition above.   A little thought gives:
//         k* = nsub - SUM nj for ij == 0  if i0 != 0
//         k* = ncmp - SUM nj for ij == 0  if i0 == 0
//             where nsub = SUM_j=0^n ( n0/2 * PI_k=1^j nk )
//               and ncmp = nsub - n0/2.

  { int d;                     //  Get total size of array and set up dimensional terms needed
                               //    to compute complement indices
    size = 1;
    nsub = 0;
    for (d = 0; d < ndim; d++)
      { size   *= dims[d];
        nsub   += size;
        base[d] = size;
      }
    ncmp = nsub - d0;
  }

  { int k, d;                       //  Handle 0-row as a special case

    for (d = 1; d < ndim; d++)
      cntr[d] = 0;

    for (k = d0; k <= size/2; k += d0)
      { int      j,  c;
        double  kr,  ki;
        double  jr,  ji;
        double f0r, f0i;
        double f1r, f1i;

        j = ncmp - k;                //  Compute j* (cntr[ndim-1..0] is k in dimension digits)
        c = 1;
        for (d = 1; d < ndim; d++)
          { cntr[d] += c;
            if (cntr[d] >= dims[d])
              cntr[d] = 0;
            else
              c = 0;
            if (cntr[d] == 0)
              j -= base[d];
          }

#ifdef DEBUG_COMPn
        printf("  ");
        print_index(ndim,base,k);
        printf("<->");
        print_index(ndim,base,j);
        printf(" Symmetry");
        if (k > j) printf(" **");
        printf("\n");
#endif

        if (k <= j)

          { kr = cdata[k].real;    //  Forward xform for 0 row:
            ki = cdata[k].imag;    //    Ck = .5 ((Ck+Cj*) - i(Ck-Cj*)) for Cx = cdata[x]
            jr = cdata[j].real;    //    Cj = .5 ((Cj+Ck*) + i(Cj-Ck*)) for Cx = cdata[x]
            ji = cdata[j].imag;

            f0r = .5 * (kr + jr);
            f0i = .5 * (ki - ji);
            f1r = .5 * (ji + ki);
            f1i = .5 * (jr - kr);

            cdata[k].real =  f0r + f1r;
            cdata[k].imag =  f0i + f1i;
            cdata[j].real =  f0r + f1r;
            cdata[j].imag = -f0i - f1i;
          }
      }
  }

  { int    h, k, d;
    double unr, uni;

    unr = cos0;         // unr + i*uni is the n'th principle root of unity to the power k
    uni = sin0;         //   as the outer loop below progresses through values of k

    for (h = 1; h < d0; h++)
      { int d;

        cntr[1] = -1;
        for (d = 2; d < ndim; d++)
          cntr[d] = 0;

        for (k = h; k <= nsub/2; k += d0) 
          { int      j,  c;
            double  kr,  ki;
            double  jr,  ji;
            double f0r, f0i;
            double f1r, f1i;

            j = nsub - k;            //  Compute j* (cntr[ndim-1..0] is k in dimension digits)
            c = 1;
            for (d = 1; d < ndim; d++)
              { cntr[d] += c;
                if (cntr[d] >= dims[d])
                  cntr[d] = 0;
                else
                  c = 0;
                if (cntr[d] == 0)
                  j -= base[d];
              }

#ifdef DEBUG_COMPn
            printf("  ");
            print_index(ndim,base,k);
            printf("<->");
            print_index(ndim,base,j);
            printf(" %g + i%g",unr,uni);
            if (k > j) printf(" **");
            printf("\n");
#endif

            if (k <= j)
    
              { kr = cdata[k].real;    //  Forward xform:
                ki = cdata[k].imag;    //    Ck = .5 ((Ck+Cj*) - i(w^n)(Ck-Cj*)) for Cx = cdata[x]
                jr = cdata[j].real;    //    Cj = .5 ((Cj+Ck*) - i(w^n)*(Cj-Ck*))
                ji = cdata[j].imag;
      
                f0r = .5 * (kr + jr);
                f0i = .5 * (ki - ji);
                f1r = .5 * (ji + ki);
                f1i = .5 * (jr - kr);
      
                cdata[k].real =  f0r + unr*f1r - uni*f1i;
                cdata[k].imag =  f0i + unr*f1i + uni*f1r;
                cdata[j].real =  f0r - unr*f1r + uni*f1i;
                cdata[j].imag = -f0i + unr*f1i + uni*f1r;
              }
          }

        { double x;                         //  Update (unr,uni) to the next n'th root of
                                            //     unity: (unr,uni) *= (cos0,sin0)
          x   = unr * cos0 - uni * sin0;
          uni = unr * sin0 + uni * cos0;
          unr = x;
        }
      }
  }

  dims[0] *= 2;

  return (cdata);
} 

//  Multi-dimensional FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The inverse transform, Real_FFT_Inverse_n, takes a complex half-matrix as produced by
//    Real_FFT_n, and produces a real-valued result *in-place*.  That is, the pointer returned is
//    exactly cdata (coerced to be double *).  Note carefully that dims[0] is the length of the
//    0th dimension of the result double array and twice that of the 0th dimension of cdata.

double *Real_FFT_Inverse_n(int ndim, int *dims, Complex *cdata)
{ int    d0;
  int    base[10], cntr[10];
  int    size, nsub, ncmp;

  double theta = TPI / dims[0];   // Need cos & sin of 2pi/n to compute roots incrementally
  double cos0  = cos(theta);
  double sin0  = sin(theta);

  d0 = dims[0] /= 2;

  if (ndim > 10)
    { fprintf(stderr,"Real_FFT_Inverse_n: Dimension of array is greater than 10!\n");
      exit (1);
    }

  { double zr = cdata[0].real;   //  Special case for the 0th element whose x'forms are real 
    double zi = cdata[0].imag;

    cdata[0].real = .5*(zr+zi);
    cdata[0].imag = .5*(zr-zi);
  }

  { int d;                     //  Get total size of array and set up dimensional terms needed
                               //    to compute complement indices
    size = 1;
    nsub = 0;
    for (d = 0; d < ndim; d++)
      { size   *= dims[d];
        nsub   += size;
        base[d] = size;
      }
    ncmp = nsub - d0;
  }

  { int k, d;                  //  Handle 0-row as a special case

    for (d = 1; d < ndim; d++)
      cntr[d] = 0;

    for (k = d0; k <= size/2; k += d0)
      { int      j,  c;
        double  kr,  ki;
        double  jr,  ji;
        double f0r, f0i;
        double f1r, f1i;

        j = ncmp - k;            //  Compute j* (cntr[ndim-1..0] is k in dimension digits)
        c = 1;
        for (d = 1; d < ndim; d++)
          { cntr[d] += c;
            if (cntr[d] >= dims[d])
              cntr[d] = 0;
            else
              c = 0;
            if (cntr[d] == 0)
              j -= base[d];
          }

#ifdef DEBUG_COMPn
        printf("  (%d) ",k);
        print_index(ndim,base,k);
        printf("<->");
        print_index(ndim,base,j);
        printf(" (%d) Symmetry",j);
        if (k > j) printf(" **");
        printf("\n");
#endif

        if (k <= j)

          { kr = cdata[k].real;    //  Forward xform for 0 row
            ki = cdata[k].imag;
            jr = cdata[j].real;
            ji = cdata[j].imag;

            cdata[k].real = .5*(jr + ji);
            cdata[k].imag = .5*(jr - ji);
            cdata[j].real = .5*(kr + ki);
            cdata[j].imag = .5*(kr - ki);
          }
      }
  }

  { int    h, k, d;
    double unr, uni;

    unr = cos0;         // unr + i*uni is the n'th principle root of unity to the power k
    uni = sin0;         //   as the outer loop below progresses through values of k

    for (h = 1; h < d0; h++)
      { int d;

        cntr[1] = -1;
        for (d = 2; d < ndim; d++)
          cntr[d] = 0;

        for (k = h; k <= nsub/2; k += d0) 
          { int      j,  c;
            double  kr,  ki;
            double  jr,  ji;
            double f0r, f0i;
            double f1r, f1i;

            j = nsub - k;            //  Compute j* (cntr[ndim-1..0] is k in dimension digits)
            c = 1;
            for (d = 1; d < ndim; d++)
              { cntr[d] += c;
                if (cntr[d] >= dims[d])
                  cntr[d] = 0;
                else
                  c = 0;
                if (cntr[d] == 0)
                  j -= base[d];
              }

#ifdef DEBUG_COMPn
            printf("  (%d) ",k);
            print_index(ndim,base,k);
            printf("<->");
            print_index(ndim,base,j);
            printf(" (%d) %g + i%g",j,unr,uni);
            if (k > j) printf(" **");
            printf("\n");
#endif

            if (k <= j)
    
              { kr = cdata[k].real;    //  Forward xform:
                ki = cdata[k].imag;    //    Ck = .5 ((Ck+Cj*) + i(w^n)(Ck-Cj*)) for Cx = cdata[x]
                jr = cdata[j].real;    //    Cj = .5 ((Cj+Ck*) + i(w^n)*(Cj-Ck*))
                ji = cdata[j].imag;
      
                f0r = .5 * (kr + jr);
                f0i = .5 * (ki - ji);
                f1r = .5 * (ji + ki);
                f1i = .5 * (jr - kr);
      
                cdata[k].real =  f0r - unr*f1r - uni*f1i;
                cdata[k].imag =  f0i - unr*f1i + uni*f1r;
                cdata[j].real =  f0r + unr*f1r + uni*f1i;
                cdata[j].imag = -f0i - unr*f1i + uni*f1r;
              }
          }

        { double x;                         //  Update (unr,uni) to the next n'th root of
                                            //     unity: (unr,uni) *= (cos0,sin0)
          x   = unr * cos0 - uni * sin0;
          uni = unr * sin0 + uni * cos0;
          unr = x;
        }
      }
  }

  FFT_n(ndim,dims,cdata,1);

  dims[0] *= 2;

  return ((double *) cdata);
} 


//  Multi-dimensional FFT-algorithm for convolution.  The 3rd argument, signal is overwritten
//    with the result and returned as the result of the function for your convenience.  Both
//    input arrays are overwitten in the process, but the code works in-place and even when
//    both arrays are the same.

double *FFT_Convolution_n(int ndim, int *dims, double *signal, double *filter)
{ Complex *s_fft, *f_fft;
  int      k;
  int      size;

  s_fft = Real_FFT_n(ndim,dims,signal);
  if (signal != filter)
    f_fft = Real_FFT_n(ndim,dims,filter);
  else
    f_fft = s_fft;
    
  //  In one step decode the first-half of 2 real-valued FFT's in data
  //    and pairwise multiply them.  0 is a special case, pack F_(n/2) into
  //    the imaginary part of data[0] in preparation for a real fft inversion.

  size = 1;
  for (k = 0; k < ndim; k++)
    size *= dims[k];
  size /= 2;

  s_fft[0].real *= f_fft[0].real;
  s_fft[0].imag *= f_fft[0].imag;

  for (k = 1; k < size; k++)
    { double kr = s_fft[k].real;
      double ki = s_fft[k].imag;
      double jr = f_fft[k].real;
      double ji = f_fft[k].imag;

      s_fft[k].real = kr*jr - ki*ji;
      s_fft[k].imag = ki*jr + kr*ji;
    }

  return (Real_FFT_Inverse_n(ndim,dims,s_fft));
}


//  Multi-dimensional FFT-algorithm for correlation.  The 3rd argument, signal is overwritten
//    with the result and returned as the result of the function for your convenience.  Both
//    input arrays are overwitten in the process, but the code works in-place and even when
//    both arrays are the same.

double *FFT_Correlation_n(int ndim, int *dims, double *seq1, double *seq2)
{ Complex *s_fft, *f_fft;
  int      k;
  int      size;

  s_fft = Real_FFT_n(ndim,dims,seq1);
  if (seq1 != seq2)
    f_fft = Real_FFT_n(ndim,dims,seq2);
  else
    f_fft = s_fft;

  //  In one step decode the first-half of 2 real-valued FFT's in data
  //    and pairwise multiply them.  0 is a special case, pack F_(n/2) into
  //    the imaginary part of data[0] in preparation for a real fft inversion.

  size = 1;
  for (k = 0; k < ndim; k++)
    size *= dims[k];
  size /= 2;

  s_fft[0].real *= f_fft[0].real;
  s_fft[0].imag *= f_fft[0].imag;

  for (k = 1; k < size; k++)
    { double kr = s_fft[k].real;
      double ki = s_fft[k].imag;
      double jr = f_fft[k].real;
      double ji = f_fft[k].imag;

      s_fft[k].real = kr*jr + ki*ji;
      s_fft[k].imag = ki*jr - kr*ji;
    }

  return (Real_FFT_Inverse_n(ndim,dims,s_fft));
}

#ifdef DEBUGn

#define DSIZE 512

int main(int argc, char *argv[])
{ static double  rata[DSIZE*DSIZE*DSIZE];
  static double  fata[DSIZE*DSIZE*DSIZE];
  int            dims[3];
  int            i, j, k, r, d;

  dims[0] = DSIZE;
  dims[1] = DSIZE;
  dims[2] = DSIZE;

  for (r = 0; r < 1; r++)

    { int base2 = dims[0]*dims[1];
      int base1 = dims[0];

      for (k = 0; k < dims[2]; k++)
        for (i = 0; i < dims[1]; i++)
          for (j = 0; j < dims[0]; j++)
            { d = k*base2+i*base1+j;
              rata[k*base2+i*base1+j] = d;
              fata[k*base2+i*base1+j] = base2-d;
            }

      FFT_Correlation_n(3,dims,rata,fata);
    }

  exit (0);
}

#endif
