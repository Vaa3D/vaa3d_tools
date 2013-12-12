/**@file tz_<1t>.h
 * @brief Routines for <1t>
 * @author Ting Zhao
 */

/* tz_fftw.h.t
 *
 * Initial write: Ting Zhao
 */

#ifndef TZ_<1T>_H_
#define TZ_<1T>_H_

#include "tz_fftw_header.h"

__BEGIN_DECLS

#ifndef R2C_LENGTH
#define R2C_LENGTH(length) (length/2+1)
#endif

/* <1t> fftw/fftwf <2t> double/float */

typedef <2t> <1t>_comp;

/* Additional utilites for fftw. */

/* 
 * <1t>_complex_comp() returns the real or imaginary component of each complex
 * number of <array>. The returned value is also an array and the ith element
 * is the:
 *   real component (when comp is 0);
 *   imaginary component (when comp is 1);
 *   amplitude (when comp is 2);
 * of the ith element of <array>. 
 */
<1t>_comp* <1t>_complex_comp(<1t>_complex* array,long length,int comp);

/*
 * <1t>_complete_2d() is obselete.
 */
void <1t>_complete_2d(<1t>_complex* array,int nx,int ny);

/*
 * <1t>_malloc_r2c_1d() allocates memory for output of 1D DFT a real array with
 * <nx> elements. <1t>_malloc_r2c_2d() and <1t>_malloc_r2c_3d() are similar
 * functions for 2D and 3D DFT.
 */
<1t>_complex* <1t>_malloc_r2c_1d(int nx);
<1t>_complex* <1t>_malloc_r2c_2d(int nx,int ny); 
<1t>_complex* <1t>_malloc_r2c_3d(int nx,int ny,int nz);

/*
 * <1t>_pack_c2r_result() removes holes in the c2r transform results. The holes
 * are caused by the padding reqirement of in-place transformation, i.e. a real
 * array needs to be padded by 1 or 2 elements in the last dimension to fit in 
 * the resulted complex transformation. <nlast> is the size of the last
 * dimension and <before> is the product of the all dimensions except the last
 * one.
 */
void <1t>_pack_c2r_result(<2t> *in,int nlast,int nbefore); 


__END_DECLS

#endif
