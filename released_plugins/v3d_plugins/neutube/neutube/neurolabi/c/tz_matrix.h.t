/**@file tz_<2t>.h
 * @brief Routines for <1t> matrix
 * @author Ting Zhao
 */


/* tz_matrix.h.t
 * 
 * 14-Aug-2007  Initial write:  Ting Zhao
 */
#ifndef _TZ_<2T>_H_
#define _TZ_<2T>_H_

#include <stdarg.h>
#include "tz_utilities.h"
#include "tz_mxutils.h"
#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup matrix_ Matrix operations
 * @{
 */

/**@addtogroup <2t>_ <1t> matrix operations (tz_<2t>.h)
 * @{
 */

/**
 * This structure defines a multidimensional matrix. The number of dimensions is ndim.
 * The dimensions are stored in the array dim and the matrix elements are stored
 * in the array in the column-major order, i.e. the lower dimension cycles 
 * faster.
 */ 
typedef struct {
  ndim_t ndim;
  dim_t dim[TZ_MATRIX_MAX_DIM];
  <1t> *array;
} <2T>;

/*utilities of <1t> matrix*/

<2T>* Make_<2T>(const dim_type dim[],ndim_type ndim);
<2T>* Copy_<2T>(const <2T>* dm);
void Free_<2T>(<2T> *<2t>);
void Kill_<2T>(<2T>* dm);

<2T>* Make_3d_<2T>(dim_type width, dim_type height, dim_type depth);

void <2T>_Copy(<2T> *des, const <2T> *src);

<2T>* Crop_<2T>(const <2T>* dm,const int offset[],const dim_type size[],
		<2T> *out);
void <2T>_Copy_Block(<2T> *des, const dim_type des_offset[], const <2T> *src,
		      const dim_type src_offset[]);

<2T>* <2T>_Blocksum(const <2T> *dm, const dim_type bdim[], <2T> *dm2);
<2T>* <2T>_Blocksum_Part(const <2T>* dm,const dim_type bdim[],const dim_type start[],const dim_type end[]);
<2T>* <2T>_Blocksum_Part2(const <2T>* dm,const dim_type bdim[],const dim_type start[],const dim_type end[]);
<2T>* <2T>_Blockmean(<2T>* dm,const dim_type bdim[],int itype);
<2T>* <2T>_Blockmean_Part(<2T>* dm,const dim_type dim[],const dim_type bdim[],const dim_type start[],const dim_type end[],int itype);
<1t> <2T>_Max(const <2T>* dm,dim_type* sub);
<1t> <2T>_Max_P(const <2T> *dm, const dim_type *start, const dim_type *end,
		dim_type *sub);
void <2T>_Clean_Edge(<2T>* dm);
<1t> <2T>_Scale(<2T>* dm);

#define <2T>_Print Print_<2T>
void <2T>_Print(const <2T>* dm);
void <2T>_Print_Part(const <2T>* dm,const dim_type start[],const dim_type end[]);
void <2T>_Write(const char* filename,const <2T>* dm);
<2T>* <2T>_Read(const char* filename);

<2T>* <3t>2<2t>(<1t>* array,ndim_type ndim,...);
<2t=fmatrix,imatrix>
<2T>* darray2<2t>(double *array,ndim_type ndim,...);
</t>

<2T> <2T>_View_Array(<1t>* array, ndim_type ndim, ...);

void <2T>_Set_Zero(<2T> *dm);

<2T>* Constant_<2T>(const dim_type dim[],ndim_type ndim,<1t> value);
<2T>* <2T>_Zeros(const dim_type dim[],ndim_type ndim);
<2T>* <2T>_Ones(const dim_type dim[],ndim_type ndim);

<2T>* <2T>_Add(<2T>* dm1,const <2T>*  dm2);
<2T>* <2T>_Addc(<2T>* dm1,<1t> d);
<2T>* <2T>_Sub(<2T>* dm1,const <2T>*  dm2);
<2T>* <2T>_Subc(<2T>* dm1,<1t> d);
<2T>* <2T>_Mul(<2T>* dm1,const <2T>*  dm2);
<2T>* <2T>_Div(<2T>* dm1,const <2T>*  dm2);
<2T>* <2T>_Sqr(<2T>* dm1);
<2T>* <2T>_Sqrt(<2T>* dm2);
<2T>* <2T>_Negative(<2T> *dm);
<2T>* <2T>_Max2(<2T> *dm1, const <2T> *dm2);
<2T>* <2T>_Min2(<2T> *dm1, const <2T> *dm2);

<2T>* <2T>_Partial_Diff(const <2T> *dm, ndim_type dim, <2T> *result);

void <2T>_Threshold(<2T> *dm, <1t> threshold);

void <2T>_Abs_Threshold(<2T> *dm, <1t> threshold);

<2T>* <2T>_Eigen3_Coeff2(<2T> *a, <2T> *b, <2T> *c, <2T> *result);
<2T>* <2T>_Eigen3_Coeff1(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result);
<2T>* <2T>_Eigen3_Coeff0(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result);
<2T>* <2T>_Eigen3_Coeffd(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result);
<2T>* <2T>_Eigen3_Solution_Score(<2T> *a, <2T> *b, <2T> *c, <2T> *d, 
				 <2T> *e, <2T> *f, <2T> *result);
<2T>* <2T>_Eigen3_Curvature(<2T> *xx, <2T> *yy, <2T> *zz, 
			    <2T> *x, <2T> *y, <2T> *z, <2T> *result);
/**@}*/
/**@}*/

__END_DECLS

#endif
