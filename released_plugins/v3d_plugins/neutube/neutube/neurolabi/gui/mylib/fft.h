/*****************************************************************************************\
*                                                                                         *
*  FFT algorithms including convolution and correlation                                   *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2007                                                                   *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _FFT_MODULE

#define _FFT_MODULE

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

Dimn_Type Power_Of_2_Pad(Dimn_Type m);

Complex_Array *FFT(Complex_Array *R(M(data)), boolean invert);

Complex_Array *FFT_Convolution(Complex_Array *R(M(fft1)), Complex_Array *fft2);
Complex_Array *FFT_Correlation(Complex_Array *R(M(fft1)), Complex_Array *fft2);

Numeric_Array *Normalize_FFT_Correlation(Numeric_Array *ref1, Numeric_Array *ref2,
                                         Numeric_Array *R(M(cor)));

#ifdef __cplusplus
}
#endif

#endif
