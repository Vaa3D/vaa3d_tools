/* testtarray.c
 *
 * 04-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_int_linked_list.h"

INIT_EXCEPTION_MAIN(e)

int main()
{
#if 0
  double d1[] = {2.0, 3.1, 1.2, 3.0, 1.1, 2.3};
  double d2[6];
  int idx[6] = {0, 1, 2, 3, 4, 5};

  darraycpy(d2, d1, 0, 6);
  
  darray_printf(d2, 6, "%g");
  darray_qsort(d2, idx, 6);
  darray_printf(d2, 6, "%g");
  iarray_print2(idx, 6, 1);
  int i;
  char separator[3];
  separator[0] = '\0';
  for (i = 0; i < 6; i++) {
    printf("%s%g", separator, d1[idx[i]]);
    if (separator[0] == '\0') {
      strcpy(separator, ", ");
    }
  }
  printf("\n");

  return 1;

  int a[] = {5, 8, 1, 4, 3, 7, 6, 9, 11, 10, 12, 2};
  iarray_myqsort(a, 12);
  iarray_print2(a, 12, 1);

  int a2[] = {5, 6, 1, 4, 3, 2};
  iarray_myqsort(a2, 6);
  iarray_print2(a2, 6, 1);

  /*
  FILE *fp = OpenDec("../data/test.dec");
  fclose(fp);
  */
  
#endif

#if 0
  double d[] = {2.0, 3.1, 1.2, 3.0, 1.1, 2.3};
  int length = sizeof(d) / sizeof(double);

  double *out = darray_malloc(length);

  darray_avgsmooth(d, length, 3, out);

  darray_print2(d, length, 1);
  darray_print2(out, length, 1);
  
  free(out);
#endif

#if 1
  int length;
  double *d = darray_read("../data/rx_array.bn", &length);
  double *d2 = darray_read("../data/ry_array.bn", &length);
  darray_mul(d, d2, length);
  double *avg = darray_malloc(length);
  double *diff = darray_malloc(length);
  darray_avgsmooth(d, length, 3, avg);
  darray_scale(avg, length, 1, length);
  darray_curvature(avg, length, diff);
  darray_write("../data/rxy_array.bn", avg, length);
  darray_write("../data/test.bn", diff, length);
#endif

#if 0
  Int_List *p = NULL;
  
  int i;
  for (i = 0; i < 10; i++) {
    Int_List_Add(&p, i);
  }

  Print_Int_List_F(p, "%d");

  int length;
  int *array = Int_List_To_Array(p, -1, NULL, &length);
  iarray_print2(array, length, 1);
#endif

#if 0
  printf("Auto testing ...\n");

  double a[] = {-1.0, 0.0, 1.0, 3.0, 2.0, 1.0, -1.0, 3.3, 4.2, 2.1};
  int mask[] = {0, 1, 0, 1, 0, 1, 1, 1, 0, 1};
  int mask2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int length = 10;
  
  int idx;					
  int i;
  double max = darray_max(a, length, NULL);

  darray_max(a, length, &idx);
  if (a[idx] != max) {
    TRACE("possible bug in darray_max");
  }

  for (i = 0; i < length; i++) {
    if (a[i] > max) {
      TRACE("possible bug in darray_max");
    }
  }

  darray_max_l(a, length, &idx);
  if (a[idx] != max) {
    TRACE("possible bug in darray_max_l");
  }

  for (i = 0; i < length; i++) {
    if (a[i] > max) {
      TRACE("possible bug in darray_max_l");
    }
  }

  max = darray_max_m(a, length, mask, &idx);
  if (a[idx] != max) {
    TRACE("possible bug in darray_max_m");
  }

  for (i = 0; i < length; i++) {
    if ((a[i] > max) && (mask[i] == 1)) {
      TRACE("possible bug in darray_max_m");
    }
  }

  max = darray_max_ml(a, length, mask, &idx);
  if (a[idx] != max) {
    TRACE("possible bug in darray_max");
  }

  for (i = 0; i < length; i++) {
    if ((a[i] > max) && (mask[i] == 1)) {
      TRACE("possible bug in darray_max");
    }
  }

  max = darray_max_m(a, length, mask2, &idx);
  if (idx >= 0) {
    TRACE("possible bug in darray_max_m");
  }

  if (max != 0.0) {
    TRACE("possible bug in darray_max_m");
  }

  max = darray_max_ml(a, length, mask2, &idx);
  if (idx < length) {
    TRACE("possible bug in darray_max_ml");
  }

  if (max != 0.0) {
    TRACE("possible bug in darray_max_ml");
  }
  
  double min = darray_min(a, length, NULL);
  darray_min(a, length, &idx);
  if (a[idx] != min) {
    TRACE("possible bug in darray_min");
  }

  for (i = 0; i < length; i++) {
    if (a[i] < min) {
      TRACE("possible bug in darray_min");
    }
  }

  darray_min_l(a, length, &idx);
  if (a[idx] != min) {
    TRACE("possible bug in darray_min_l");
  }

  for (i = 0; i < length; i++) {
    if (a[i] < min) {
      TRACE("possible bug in darray_min_l");
    }
  }

  min = darray_min_m(a, length, mask, &idx);
  if (a[idx] != min) {
    TRACE("possible bug in darray_min");
  }

  for (i = 0; i < length; i++) {
    if ((a[i] < min) && (mask[i] == 1)) {
      TRACE("possible bug in darray_min");
    }
  }

  min = darray_min_ml(a, length, mask, &idx);
  if (a[idx] != min) {
    TRACE("possible bug in darray_min_ml");
  }

  for (i = 0; i < length; i++) {
    if ((a[i] < min) && (mask[i] == 1)) {
      TRACE("possible bug in darray_min_ml");
    }
  }

  min = darray_min_m(a, length, mask2, &idx);
  if (idx >= 0) {
    TRACE("possible bug in darray_min_m");
  }

  if (min != 0.0) {
    TRACE("possible bug in darray_min_m");
  }

  min = darray_min_ml(a, length, mask2, &idx);
  if (idx < length) {
    TRACE("possible bug in darray_min_ml");
  }

  if (min != 0.0) {
    TRACE("possible bug in darray_min_ml");
  }


  printf("Done.\n");

#endif

  return 0;
}
