/*****************************************************************************************\
*                                                                                         *
*  FFT algorithms including convolution and correlation and multiple dimensions           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2007                                                                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef FAST_FOURIER_TRANSFORMS

#define FAST_FOURIER_TRANSFORMS

typedef struct      //  Complex number data type
  { double  real;
    double  imag;
  } Complex;

//  All the FFT routines assume n is a power of 2.  As a convenience to help you pad
//    your vectors to this size, Power_Of_2_Pad returns the smallest power of 2 greater
//    than or equal to m.

int Next_Power_Of_2(int m);


/*****************************************************************************************\
*                                                                                         *
*  1-dimensional FFT algorithms                                                           *
*                                                                                         *
\*****************************************************************************************/

//  Basic 1-dimenstional FFT-algorithm.  The FFT is performed in-place within 'data' and
//    for convenience a pointer to data is returned by FFT.  If invert is non-zero then
//    the inverse Fourier Transform is performed.

Complex *FFT(int n, Complex *data, int invert);

//  FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The forward transform, Real_FFT, takes a double array of length n, and *in-place* produces
//    a Complex array c of length n/2 that is the first half of the conjugate symmetric FFT
//    of the real data with the exception that F_(n/2) (which is real) is tucked into c[0].imag
//    (which works as F_0 is also real).  Again, the pointer returned by FFT is equal to
//    rdata, the FFT is performed in-place.
//
//    The inverse transform, Real_FFT_Inverse, takes a complex half-matrix as produced by
//    Real_FFT, and produces a real-valued result *in-place*.  That is, the pointer returned is
//    exactly cdata (coerced to be double *), the real and imaginary parts of cdata *must be
//    contiguous in memory.  Note carefully that n is the length of the resulting real array
//    and is twice the length of cdata.

Complex *Real_FFT(int n, double *rdata);

double  *Real_FFT_Inverse(int n, Complex *cdata);

//  FFT-algorithms for convolution and correlation.  The second argument, signal and seq1,
//    respectively, are overwritten with the result and returned as the result of the function
//    for your convenience.  Both input arrays are overwitten in the process.  The code works
//    even if the two arrays are the same.

double *FFT_Convolution(int n, double *signal, double *filter);
double *FFT_Correlation(int n, double *seq1, double *seq2);


/*****************************************************************************************\
*                                                                                         *
*  Multi-dimensional FFT algorithms                                                       *
*                                                                                         *
\*****************************************************************************************/

//  Basic n-dimensional FFT-algorithm.  The FFT is performed in-place within 'data' and
//    for convenience a pointer to data is returned by FFT.  If invert is non-zero then
//    the inverse Fourier Transform is performed.

Complex *FFT_n(int ndim, int *dims, Complex *data, int invert);

//  Multi-dimensional FFT-algorithms optimized for the case of a real-valued time-domain data.
//
//    The forward transform, Real_FFT_n, takes a double array of size s, and *in-place* produces
//    a Complex array c of size s/2 that is the conjugate symmetric FFT of the real data for
//    the first half of the lowest dimension [0..dims[0]/2-1].  In order to make it fully
//    invertible F[0,...,0,dims[0]/2] is tucked into c[0,...,0,0].imag (which works as both are
//    real values.)  Again, the pointer returned by FFT is really the same as rdata, the
//    FFT is performed in-place.
//
//    The inverse transform, Real_FFT_Inverse_n, takes a complex half-matrix as produced by
//    Real_FFT_n, and produces a real-valued result *in-place*.  That is, the pointer returned is
//    exactly cdata (coerced to be double *).  Note carefully that dims[0] is the length of the
//    0th dimension of the result double array and twice that of the 0th dimension of cdata.

Complex *Real_FFT_n(int ndim, int *dims, double *rdata);

double  *Real_FFT_Inverse_n(int ndim, int *dims, Complex *cdata);

//  Multi-dimensional FFT-algorithms for convolution and correlation.  The 3rd argument,
//    signal and seq1, respectively, are overwritten with the result and returned as the
//    result of the function for your convenience.  Both input arrays are overwitten in the
//    process, but the code works in-place and even when both arrays are the same.

double *FFT_Convolution_n(int ndim, int *dims, double *signal, double *filter);
double *FFT_Correlation_n(int ndim, int *dims, double *seq1, double *seq2);

#endif
