/* testmatrix.c
 *
 * 23-Sep-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_imatrix.h"
#include "tz_iarray.h"
#include "tz_matlabio.h"
#include "tz_arrayview.h"
#include "tz_error.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    const char *mat_file = "../data/benchmark/3darray.mat";
    if (fexist(mat_file)) {
      Matlab_Array *ma = Read_Matlab_Array(mat_file);

      DMatrix dm = DMatrix_View_Matlab_Array(ma);
      //Print_DMatrix(&dm);

      dim_t start[] = {0, 0, 2};
      dim_t end[] = {4, 4, 2};  
      dim_t sub[TZ_MATRIX_MAX_DIM];

      double maxv = DMatrix_Max_P(&dm, start, end, sub);

      dim_t subdim[TZ_MATRIX_MAX_DIM];
      ndim_t k;
      for (k = 0; k < dm.ndim; k++) {
	subdim[k] = end[k] - start[k] + 1;
      }

      //iarray_print2(subdim, dm.ndim, 1);

      DMatrix *mt2 = Crop_DMatrix(&dm, start, subdim, NULL);
      //Print_DMatrix(mt2);
      dim_t sub2[TZ_MATRIX_MAX_DIM];
      double maxv2 = DMatrix_Max(mt2, sub2);

      if (maxv != maxv2) {
	PRINT_EXCEPTION("Bug?", "Unmatched maximum.");
	return 1;
      }

      if ((sub[0] != sub2[0] + start[0]) || (sub[1] != sub2[1] + start[1]) ||
	  (sub[2] != sub2[2] + start[2])) {
	PRINT_EXCEPTION("Bug?", "Unmatched maximum.");
	return 1;
      }

      Kill_DMatrix(mt2);
      Kill_Matlab_Array(ma);
    } else {
      printf("%s cannot be found\n", mat_file);
    }

    int *array = iarray_malloc(6);
    int *array2 = iarray_malloc(6);
    size_t i;
    for (i = 0; i < 6; ++i) {
      array[i] = i;
      array2[i] = i * i;
    }
    IMatrix *mt3 = iarray2imatrix(array, 2, 2, 3);
    IMatrix *mt5 = iarray2imatrix(array2, 2, 2, 3);

    dim_type des_offset[] = {1, 2};
    dim_type src_offset[] = {1, 1};
    IMatrix_Copy_Block(mt5, des_offset, mt3, src_offset);
    if (mt5->array[5] != 3) {
      PRINT_EXCEPTION("Bug?", "Unexpected value.");
      return 1;
    }

    dim_type bdim[] = { 2, 2 };
    IMatrix *mt6 = IMatrix_Blocksum(mt3, bdim, NULL);

    if (mt6->array[7] != 14) {
      PRINT_EXCEPTION("Bug?", "Unexpected value.");
      return 1;
    }

    int offset[] = {1, 1};
    dim_type size[] = {1, 2};

    IMatrix *mt4 = Crop_IMatrix(mt3, offset, size, NULL);
    if (mt4->array[0] != 3 || mt4->array[1] != 5) {
      PRINT_EXCEPTION("Bug?", "Unexpected value.");
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  ndim_type ndim = 3;
  dim_type dim[] = {5, 5, 3};
  IMatrix *im = Make_IMatrix(dim, ndim);
  
  int i;
  for (i = 0; i < 75; i++) {
    im->array[i] = i * i;
  }

  IMatrix_Print(im);

  IMatrix *im2 = Make_IMatrix(dim, ndim);
  IMatrix_Partial_Diff(im, 2, im2);

  IMatrix_Print(im2);
#endif
  
#if 0
  ndim_t ndim = 5;
  dim_t dim[5] = {10, 23, 12, 31, 21};
  dim_t sub[5];
  ind2sub(dim, ndim, 30102, sub);
  Print_Dim(sub, ndim);

  printf(DIM_T_FORMAT, sub2ind(dim, ndim, sub));
  printf("\n");
#endif

#if 0
  ndim_type ndim = 3;
  dim_type dim[] = {5, 5, 3};
  IMatrix *im = Make_IMatrix(dim, ndim);
  
  int i;
  for (i = 0; i < 75; i++) {
    im->array[i] = -i;
  }

  IMatrix_Print(im);
  
  dim[0] = 3;
  dim[1] = 3;
  dim[2] = 3;
  IMatrix *im2 = Make_IMatrix(dim, ndim);
  for (i = 0; i < dim[0] * dim[1] * dim[2]; i++) {
    im2->array[i] = i;
  }

  dim_t des_offset[] = {1, 1, 1};
  dim_t src_offset[] = {1, 1, 1};
  IMatrix_Copy_Block(im, des_offset, im2, src_offset);
  IMatrix_Print(im);
#endif

#if 0
  ndim_t ndim = 3;
  dim_t dim[] = {5, 3, 3};
  dim_t start[] = {0, 0, 1};
  dim_t end[] = {2, 2, 2};

  IMatrix *mt = Make_IMatrix(dim, ndim);
  dim_t sub[MAX_DIM];

  size_t i;
  size_t n = Matrix_Size(mt->dim, mt->ndim);
  for (i = 0; i < n; i++) {
    mt->array[i] = i * i % 15;
  }

  IMatrix_Print(mt);

  int maxv = IMatrix_Max_P(mt, start, end, sub);
  iarray_print2(sub, mt->ndim, 1);
  printf("%d\n", maxv);

  dim_t subdim[MAX_DIM];
  ndim_t k;
  for (k = 0; k < mt->ndim; k++) {
    subdim[k] = end[k] - start[k] + 1;
  }

  iarray_print2(subdim, mt->ndim, 1);

  IMatrix *mt2 = Crop_IMatrix(mt, start, subdim, NULL);
  Print_IMatrix(mt2);
  dim_t sub2[MAX_DIM];
  int maxv2 = IMatrix_Max(mt2, sub2);
  iarray_print2(sub2, mt2->ndim, 1);
  printf("%d\n", maxv2);
#endif

#if 1
  Matlab_Array *ma = Read_Matlab_Array("../data/benchmark/3darray.mat");
  
  DMatrix dm = DMatrix_View_Matlab_Array(ma);
  Print_DMatrix(&dm);

  dim_t start[] = {0, 0, 1};
  dim_t end[] = {2, 2, 2};  
  dim_t sub[TZ_MATRIX_MAX_DIM];

  double maxv = DMatrix_Max_P(&dm, start, end, sub);
  iarray_print2(sub, dm.ndim, 1);
  printf("%g\n", maxv);

  dim_t subdim[TZ_MATRIX_MAX_DIM];
  ndim_t k;
  for (k = 0; k < dm.ndim; k++) {
    subdim[k] = end[k] - start[k] + 1;
  }

  iarray_print2(subdim, dm.ndim, 1);

  DMatrix *mt2 = Crop_DMatrix(&dm, start, subdim, NULL);
  Print_DMatrix(mt2);
  dim_t sub2[TZ_MATRIX_MAX_DIM];
  double maxv2 = DMatrix_Max(mt2, sub2);
  iarray_print2(sub2, mt2->ndim, 1);
  printf("%g\n", maxv2);

  Kill_Matlab_Array(ma);
#endif

  return 0;
}
