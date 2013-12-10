/**@file tz_<2t>.h
 * @brief routines for <1t> array
 * @author Ting Zhao
 */

#ifndef _TZ_<2T>_H_
#define _TZ_<2T>_H_

<2t=darray,farray>
#include "tz_<3t>.h"
</t>

<2t=darray>
/*double array*/
</t>
<2t=farray>
/*float array*/
</t>
<2t=iarray>
/*int array*/
</t>
<2t=u8array>
/*uint8 array*/
</t>

#include <stdlib.h>
#include "tz_cdefs.h"
#include "tz_utilities.h"

__BEGIN_DECLS
  
/**@addtogroup array_opr_ Array operations
 * @{
 */

/**@addtogroup array_opr_<1t> <1t> array (tz_<2t>.h)
 * @{
 */

/*
 * <2t>_max() and <2t>_min() find the maximum and minimum in an array 
 * respectively. The index of the first occurrence maximum or minimum is 
 * stored in idx if it is not NULL.
 * <2t>_max_m() and <2t>_min_m() support masked operation.
 * <2t>_max_l() and <2t>_min_l() find the maximum and mininum too, but <idx>
 * is the last occurrence. Their masked verions are <2t>_max_ml and <2t>_min_ml.
 * The masked versions returns 0 and sets <idx> to <length> if no element is 
 * available.
 *
 <1t=double>
 *
 *<2t>_max_n() ignores NaN values. It returns 0.0 and sets <idx> to the invalid
 * array index if all * values are NaN. 
 </t>
 */
<1t> <2t>_max(const <1t>* d1, size_t length, size_t* idx);
<1t> <2t>_min(const <1t>* d1, size_t length, size_t* idx);
<1t> <2t>_max_m(const <1t> *d1, size_t length, const int *mask, size_t *idx);
<1t> <2t>_min_m(const <1t> *d1, size_t length, const int *mask, size_t *idx);
<1t> <2t>_max_l(const <1t>* d1, size_t length, size_t* idx);
<1t> <2t>_min_l(const <1t>* d1, size_t length, size_t* idx);
<1t> <2t>_max_ml(const <1t> *d1, size_t length, const int *mask, size_t *idx);
<1t> <2t>_min_ml(const <1t> *d1, size_t length, const int *mask, size_t *idx);
<1t=double>
<1t> <2t>_max_n(const <1t> *d1, size_t length, size_t *idx);
</t>

/*
 * <2t>_abs() turns <d1> to its absolute values and <2t>_neg() reverse the signs
 * of <d1>.
 */
<1t=float,double,int>
<1t>* <2t>_abs(<1t>* d1, size_t length);
<1t>* <2t>_neg(<1t>* d1, size_t length);
</t>

/*
 * <2t>_add() stores the sum of <d1> and <d2> into <d1>. <2t>_addc stores the
 * sum of <d1> and a scalar <d2> into <d1>. Other arithmetic operations have
 * the same interfaces.
 */
<1t>* <2t>_add(<1t>* d1,const <1t>* d2, size_t length);
<1t>* <2t>_addc(<1t>*d1,<1t> d2, size_t length);
<1t>* <2t>_cadd(<1t> d2, <1t>*d1, size_t length);
<1t>* <2t>_sub(<1t>* d1,<1t>* d2, size_t length);
<1t>* <2t>_csub(<1t> d2, <1t>*d1, size_t length);
<1t>* <2t>_subc(<1t>*d1,<1t> d2, size_t length);
<1t>* <2t>_mul(<1t>* d1,<1t>* d2, size_t length);
<1t>* <2t>_mulc(<1t>*d1,<1t> d2, size_t length);
<1t>* <2t>_div(<1t>* d1,<1t>* d2, size_t length);
<1t>* <2t>_div_i(<1t>* d1,int* d2, size_t length);
<1t>* <2t>_divc(<1t>*d1,<1t> d2, size_t length);

/*
 * <2t>_dot() returns the dot product of <d1> and <d2>.
 */
double <2t>_dot(const <1t> *d1, const <1t> *d2, size_t length);
<1t=double>
double <2t>_dot_n(const <1t> *d1, const <1t> *d2, size_t length);
<1t> <2t>_sum_n(const <1t>* d1, size_t length);
<1t> <2t>_mean_n(const <1t>* d1, size_t length);
double <2t>_corrcoef_n(const <1t> *d1, const <1t> *d2, size_t length);
double <2t>_dot_nw(const <1t> *d1, const <1t> *d2, size_t length);
</t>
/*
 * <2t>_sqr() calculates the square of <d1>. <2t>_sqrt() calculates the squre 
 * roots of <d1>.
 */
<1t>* <2t>_sqr(<1t>* d1, size_t length);
<1t>* <2t>_sqrt(<1t>* d1, size_t length);

/*
 * <2t>_scale() scales <d1> so that its minimal value is <min> and maxinum value
 * is <max>.
 */
<1t>* <2t>_scale(<1t> *d1, size_t length, <1t> min, <1t> max);

<1t>* <2t>_max2(<1t>* d1,const <1t>* d2, size_t length);
<1t>* <2t>_min2(<1t>* d1,const <1t>* d2, size_t length);

void <2t>_threshold(<1t> *d, size_t length, const <1t> *min, const <1t> *max);

/*
 * These are routines for special usage.
 */
<1t>* <2t>_fun1(<1t>* d1,<1t> *d2,<1t> thr,size_t length);
<1t>* <2t>_fun1_max(<1t>* d1,<1t> *d2,<1t> thr,size_t length,<1t> *maxv);
<1t>* <2t>_fun1_i2(<1t>* d1,int *d2,<1t> thr,size_t length);
<1t>* <2t>_fun1_i2_max(<1t>* d1,int *d2,<1t> thr,size_t length,<1t> *maxv);
<1t>* <2t>_fun2(<1t> *d1,<1t> *d2,<1t> *d3,size_t length);
<1t>* <2t>_fun3(<1t> *d1,<1t> *d2,<1t> *d3,<1t> *d4,size_t length);

<1t>* <2t>_cumsum(<1t>* d1,size_t length);
<1t>* <2t>_cumsum_m(<1t>* d1,size_t length,const int *mask);
<1t>* <2t>_cumsum2(<1t>* d1,int width,int height);

<1t> <2t>_sum(const <1t>* d1,size_t length);
<1t> <2t>_abssum(<1t>* d1,size_t length);
<1t> <2t>_sum_h(<1t>* d1,size_t length);
<1t> <2t>_centroid(const <1t>* d1,size_t length);
double <2t>_centroid_d(const <1t>* d1,size_t length);
void <2t>_linsum1(<1t>* d1,<1t>* d2,int width,int height,int bwidth,int bheight);
void <2t>_linsum2(<1t>* d2,int width2,int height,int bheight);
void <2t>_linsum1_part(const <1t>* d1,<1t>* d2,int width,int bwidth,
    int start, int end);
void <2t>_linsum2_part(const <1t>* d1,<1t>* d2,int width2,int height,int bheight, int start, int end);

<1t=double>
double <2t>_sqsum(const <1t> *d1, size_t length);
double <2t>_norm(const <1t> *d1, size_t length);
double <2t>_simscore(<1t> *d1, <1t> *d2, size_t length);
</t>
/*
 * <2t>cpy() copies an array from <src> to <dst> + <offset> with <length> 
 * elements. <2t>cpy2 copies an array from <src> to <dst> as an 2d array. The
 * size of <src> <width2> and <height2> must not be greater than the size of
 * <dst> <width1> and <height1>. It starts from (<row_offset>, <col_offset>)
 * iin dst.
 */
void <2t>cpy(<1t>* dst, const <1t>* src,size_t offset,size_t length);
void <2t>cpy2(<1t>* dst,<1t>* src,int width1,int height1,int width2,int height2,
	      int row_offset,int col_offset);
void <2t>cpy3(<1t>* d1,<1t>* d2,int width1,int height1,int depth1,int width2,
	      int height2,int depth2,int row_offset,int col_offset,
	      int dep_offset);
 
//void <2t>_linsum2(<1t>* d2,int width,int height,int bwidth,int bheight);
<1t>* <2t>_blocksum(<1t> *d1,int width,int height,int bwidth,int bheight);
<1t>* <2t>_blockmean(<1t> *d1,int width,int height,int bwidth,int bheight,int itype);
<1t>* <2t>_shiftdim2(<1t>* d1,int width,int height);
double <2t>_sum_d(const <1t> *d1,size_t length);
<1t> <2t>_mean(<1t>* d1,size_t length);
double <2t>_mean_d(const <1t> *d1,size_t length);
double <2t>_mean_d_m(const <1t> *d1,size_t length, const int *mask);
double <2t>_var(<1t> *d1, size_t length);
double <2t>_cov(<1t> *d1, <1t> *d2, size_t length);
<1t>* <2t>_malloc(size_t length);
<1t>* <2t>_calloc(size_t length);
<1t>* <2t>_malloc2(int width,int height);
<1t>* <2t>_malloc3(int width,int height,int depth);
<1t>* <2t>_copy(<1t>* d1,size_t length);
void <2t>_clean_edge3(<1t>* d1,int width,int height,int depth,int margin);

void <2t>_printf(<1t> *d1, size_t length, const char *format);
void <2t>_print2(<1t>* d1,int width,int height);
void <2t>_print(<1t>* d1, size_t length);
void <2t>_printf2(FILE *fp, <1t>* d1,int width,int height);
void <2t>_print3(<1t> *d1,int width,int height,int depth);

/**
 * Those binary reading and writing function operate on array size with integer
 * type.
 */
int <2t>_write(const char* filename,const <1t> *d1, int length);
<1t>* <2t>_read(const char *filename, int *length);
int <2t>_read2(const char* filename, <1t> *d1, int *length);

<1t=double,float,int>
size_t <2t>_fscanf(FILE *fp, <1t> *d1, size_t length);
<1t>* <2t>_load_matrix(const char *filepath, <1t> *d, int *m, int *n);
<1t>* <2t>_load_csv(const char *filepath, <1t> *d, int *m, int *n);
</t>

/**
 * <2t>_qsort(), <2t>_myqsort() and <2t>_binsearch() only supports array size 
 * with integer type. <idx> does not have to be initialized when it is not NULL.
 */
void <2t>_qsort(<1t> *d1,int *idx,int length);
void <2t>_myqsort(<1t> *d1, int length);
int <2t>_binsearch(<1t> *d1, int length, <1t> value);

double <2t>_corrcoef(const <1t> *d1, const <1t> *d2, size_t length);
double <2t>_corrcoef_m(const <1t> *d1, const <1t> *d2, size_t length, 
		       const int *mask);
double <2t>_uint8_corrcoef(const <1t> *d1, const tz_uint8 *d2, size_t length);

<2t=darray,farray>
/**
 * fft related routines only supports array size with integer type.
 */
<3t>_complex* <2t>_fft(<1t> *d1, int length,int in_place,int preserve);
<1t>* <2t>_ifft(<3t>_complex *c, int length,int in_place,int preserve,int normalize);
<1t>* <2t>_convolve(<1t> *d1, int length1,<1t> *d2, int length2,int reflect,
		    <1t> *convolution);
<1t>* <2t>_avgsmooth(const <1t>* in, size_t length, int wndsize, <1t> *out);
<1t>* <2t>_curvature(const <1t>* in, size_t length, <1t> *out);
<1t>* <2t>_cendiff(const <1t>* in, size_t length, <1t> *out);
</t>

<1t>* <2t>_medfilter(const <1t> *in, size_t length, int wndsize, <1t> *out);

<1t=double,float,int>
int <2t>_iszero(const <1t> *d, size_t length);
<1t>* <2t>_contarray(size_t start, size_t end, <1t> *d);
</t>

void <2t>_reverse(<1t> *d, size_t length);

/**@}*/

/**@}*/

__END_DECLS

#endif
