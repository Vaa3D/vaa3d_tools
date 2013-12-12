/**@file tz_complex.h
 * @brief complex number
 * @author Ting Zhao
 */

#ifndef _TZ_COMPLEX_H_
#define _TZ_COMPLEX_H_

#include "tz_fftw_header.h"

/* Complex operation routines */

/* 
 * The functions include:
 *
 * Add, subtract or multiply two an array and a constant; 
 * Add, subtrace or multiply two arrays;
 * Conjugate of a complex number or array;
 * print an array.
 */

#define COMPLEX_DEFINE_API(X, C)					\
void X(add)(C c1,const C c2);						\
void X(csub)(C c1,const C c2);						\
void X(cmul)(C c1,const C c2);						\
void X(conjg)(C c);							\
C* X(cadd_array)(C* c1,C* c2,long length);				\
C* X(csub_array)(C* c1,C* c2,long length);				\
C* X(cmul_array)(C* c1,C* c2,long length);				\
C* X(conjg_array)(C* c,long length);					\
void X(print)(C *c,long length);					

/* end of COMPLEX_DEFINE_API macro */

COMPLEX_DEFINE_API(FFTW_MANGLE_DOUBLE,fftw_complex)
COMPLEX_DEFINE_API(FFTW_MANGLE_FLOAT,fftwf_complex)

#endif
