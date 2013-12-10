/**@file tz_mxutils.h
 * @brief utilities for matrix
 * @author Ting Zhao
 */

#ifndef _TZ_MXUTILS_H
#define _TZ_MXUTILS_H

#include "tz_cdefs.h"
#include "tz_utilities.h"

__BEGIN_DECLS

/**@addtogroup matrix_ Matrix operations
 * @{
 */

/**@addtogroup matrix_utils_ Matrix utilities
 * @{
 */

#define TZ_MATRIX_MAX_DIM 5 /* up to 5 dimensions */
#define TZ_MATRIX_MAX_DIM_I 6 /* It must be MAX_DIM + 1 */

/* Note: ndim_t and dim_t are preferred to use. ndim_type and dim_type are 
 *       named in an old coding style.
 *
 * The types of number of dimensions: ndim_type, ndim_t
 * The types of dimensions: dim_type, dim_t
 *
 * Note: since these numbers must be greater than 0 to be valid, they are not
 * graunteed to be able to store negative values. Cast them into a signed 
 * type to avoid the uncertainty.
 */
#define DIM_T_FORMAT "%d"
#define NDIM_T_FORMAT "%u"
typedef tz_uint8 ndim_type; /* obsolete type */
typedef int dim_type; /* obsolete type */
typedef ndim_type ndim_t;
typedef dim_type dim_t;

void Matrix_Error(const char *msg,const char *arg);
void Check_Matrix(const dim_type dim[],ndim_type ndim);
size_t Matrix_Size(const dim_type dim[],ndim_type ndim);
size_t Matrix_Subsize(const dim_type dim[],ndim_type start,ndim_type end);
size_t Sub_To_Ind(const dim_type dim[],ndim_type ndim,const dim_type sub[]);
void Ind_To_Sub(const dim_type dim[],ndim_type ndim,size_t index,
		dim_type sub[]);
void Translate_Intdim(const int idim[], dim_type dim[], int ndim);

/* For obsolete interface */
#define matrix_error Matrix_Error
#define check_matrix Check_Matrix 
#define matrix_size Matrix_Size
#define matrix_subsize Matrix_Subsize
#define sub2ind Sub_To_Ind
#define ind2sub Ind_To_Sub
#define translate_intdim Translate_Intdim

 
void Print_Dim(const dim_t dim[], ndim_t ndim);

void Matrix_Overlap(const dim_type dim[],const dim_type bdim[],const dim_type offset[],ndim_type ndim,dim_type op1[],dim_type op2[]);
int Compare_Dim(const dim_type dim1[],ndim_type ndim1,
		const dim_type dim2[],ndim_type ndim2);
dim_type* Get_Area(dim_type bdim[],dim_type dim[],ndim_type ndim);
dim_type* Get_Area_Part(dim_type bdim[],dim_type dim[],dim_type start[],dim_type end[],ndim_type ndim);

/**@}*/
/**@}*/

__END_DECLS

#endif
