/* testarray.c
 *
 * 01-Oct-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_farray.h"
#include "tz_random.h"
#define FORCE_PROGRESS
#include "tz_interface.h"

static int test_iarray_max(const int *array, size_t length, int tm, size_t ti)
{
  int maxv;
  size_t idx;
  maxv = iarray_max(array, length, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_max(const double *array, size_t length, double tm, 
    size_t ti)
{
  double maxv;
  size_t idx;
  maxv = darray_max(array, length, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_max_l(const int *array, size_t length, int tm, size_t ti)
{
  size_t idx;
  int maxv = iarray_max_l(array, length, &idx);
  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static double test_darray_max_l(const double *array, size_t length, 
    double tm, size_t ti)
{
  size_t idx;
  double maxv = darray_max_l(array, length, &idx);
  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_min(const int *array, size_t length, int tm, size_t ti)
{
  int minv;
  size_t idx;
  minv = iarray_min(array, length, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_min(const double *array, size_t length, double tm, 
    size_t ti)
{
  double minv;
  size_t idx;
  minv = darray_min(array, length, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_min_l(const int *array, size_t length, int tm, size_t ti)
{
  size_t idx;
  int minv = iarray_min_l(array, length, &idx);
  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_min_l(const double *array, size_t length,
    double tm, size_t ti) 
{
  size_t idx;
  double dminv = darray_min_l(array, length, &idx);
  if (dminv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    fprintf(stderr, "bug found");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_max_m(const int *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  int maxv;
  size_t idx;
  maxv = iarray_max_m(array, length, mask, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_max_m(const double *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  double maxv;
  size_t idx;
  maxv = darray_max_m(array, length, mask, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_max_ml(const int *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  int maxv;
  size_t idx;
  maxv = iarray_max_ml(array, length, mask, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_max_ml(const double *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  double maxv;
  size_t idx;
  maxv = darray_max_ml(array, length, mask, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_min_m(const int *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  int minv;
  size_t idx;
  minv = iarray_min_m(array, length, mask, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_min_m(const double *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  double minv;
  size_t idx;
  minv = darray_min_m(array, length, mask, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_iarray_min_ml(const int *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  int minv;
  size_t idx;
  minv = iarray_min_ml(array, length, mask, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_min_ml(const double *array, size_t length, 
    const int *mask, int tm, size_t ti)
{
  double minv;
  size_t idx;
  minv = darray_min_ml(array, length, mask, &idx);

  if (minv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected min value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

static int test_darray_max_n(const double *array, size_t length, 
    int tm, size_t ti)
{
  double maxv;
  size_t idx;
  maxv = darray_max_n(array, length, &idx);

  if (maxv != tm) {
    PRINT_EXCEPTION("Bug?", "Unexpected max value.");
    return 1;
  }
  if (idx != ti) {
    PRINT_EXCEPTION("Bug?", "Unexpected index.");
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[])
{
#if 1
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    size_t idx;

    int iarray[] = {3, 2, 2, 2, 4, 5, 10, 19, 43, -1};
    double darray[] = {3.0, 2.0, 2.0, 2.0, 4.0, 5.0, 10.0, 19.0, 43.0, -1.0};
    int mask1[] = {0, 0, 1, 0, 1, 0, 1, 1, 0, 0};
    int mask2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t length = sizeof(iarray) / sizeof(int);

    if (test_iarray_max(iarray, length, 43, 8) > 0) {
      return 1;
    }

    if (test_darray_max(darray, length, 43.0, 8) > 0) {
      return 1;
    }
   
    if (test_iarray_max_l(iarray, length, 43, 8) > 0) {
      return 1;
    }

    if (test_darray_max_l(darray, length, 43.0, 8) > 0) {
      return 1;
    }

    if (test_iarray_min(iarray, length, -1, 9) > 0) {
      return 1;
    }

    if (test_darray_min(darray, length, -1.0, 9) > 0) {
      return 1;
    }
   
    if (test_iarray_min_l(iarray, length, -1, 9) > 0) {
      return 1;
    }

    if (test_darray_min_l(darray, length, -1.0, 9) > 0) {
      return 1;
    }

    if (test_iarray_max_m(iarray, length, mask1, 19, 7) > 0) {
      return 1;
    }

    if (test_darray_max_m(darray, length, mask1, 19.0, 7) > 0) {
      return 1;
    }

    if (test_iarray_max_ml(iarray, length, mask1, 19, 7) > 0) {
      return 1;
    }

    if (test_darray_max_ml(darray, length, mask1, 19.0, 7) > 0) {
      return 1;
    }

    if (test_iarray_min_m(iarray, length, mask1, 2, 2) > 0) {
      return 1;
    }

    if (test_darray_min_m(darray, length, mask1, 2.0, 2) > 0) {
      return 1;
    }

    if (test_iarray_min_ml(iarray, length, mask1, 2, 2) > 0) {
      return 1;
    }

    if (test_darray_min_ml(darray, length, mask1, 2.0, 2) > 0) {
      return 1;
    }

    if (test_iarray_min_m(iarray, length, mask2, 0, INVALID_ARRAY_INDEX) > 0) {
      return 1;
    }

    if (test_darray_min_m(darray, length, mask2, 0.0, INVALID_ARRAY_INDEX) > 0) {
      return 1;
    }

    if (test_iarray_max_m(iarray, length, mask2, 0, INVALID_ARRAY_INDEX) > 0) {
      return 1;
    }

    if (test_darray_max_m(darray, length, mask2, 0.0, INVALID_ARRAY_INDEX) > 0) {
      return 1;
    }

    if (test_iarray_max_ml(iarray, length, mask2, 0, length) > 0) {
      return 1;
    }

    if (test_darray_max_ml(darray, length, mask2, 0.0, length) > 0) {
      return 1;
    }

    if (test_iarray_min_ml(iarray, length, mask2, 0, length) > 0) {
      return 1;
    }

    if (test_darray_min_ml(darray, length, mask2, 0.0, length) > 0) {
      return 1;
    }

    double darray2[] = {3.0, 2.0, NaN, NaN, 4.0, 5.0, 10.0, 19.0, 43.0, -1.0};
    if (test_darray_max_n(darray2, length, 43.0, 8) > 0) {
      return 1;
    }

    int iarray2[] = {3, 2, 2, 2, 4, 5, 43, 43, 43, -1};
    if (test_iarray_max(iarray2, length, 43, 6) > 0) {
      return 1;
    }

    if (test_iarray_max_l(iarray2, length, 43, 8) > 0) {
      return 1;
    }

    if (test_iarray_max_ml(iarray2, length, mask1, 43, 7) > 0) {
      return 1;
    }

    if (test_iarray_max_m(iarray2, length, mask1, 43, 6) > 0) {
      return 1;
    }

    Random_Seed(time(NULL) - getpid());

    HOURGLASS_BEGIN("testing array", hg);
    int ncycle = 10000;
    while (ncycle > 0) {
      HOURGLASS_UPDATE(hg);
      size_t length = 0;
      while ((length = Unifrnd_Int(1000)) <= 0);

      double *d = Unifrnd_Double_Array(length, NULL);

      int *indices = iarray_malloc(length);
      double dmax = darray_max(d, length, &idx);
      if (d[idx] != dmax) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      double dmin = darray_min(d, length, &idx);
      if (d[idx] != dmin) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      if (dmax < dmin) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      double *d3 = darray_malloc(length);
      darraycpy(d3, d, 0, length);

      darray_qsort(d, indices, length);
      if (d[0] != dmin) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      if (d[length - 1] != dmax) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      int i;
      for (i = 0; i < length - 1; i++) {
        if (d[i] > d[i + 1]) {
          darray_write("../data/test.bn", d, length);
          fprintf(stderr, "bug found");
          return 1;
        }
      }

      for (i = 0; i < length; i++) {
        if (d3[indices[i]] != d[i]) {
          darray_write("../data/test.bn", d, length);
          fprintf(stderr, "bug found");
          return 1;
        }
      }

      double sum = darray_sum(d, length);
      double mean = darray_mean(d, length);
      if (sum - mean * length > 0.01) {
        darray_write("../data/test.bn", d, length);
        fprintf(stderr, "bug found");
        return 1;
      }

      double *d2 = darray_malloc(length + 4);
      darray_linsum1(d, d2, length, 1, 5, 1);
      if (length > 5) {
        if (d2[4] != darray_sum(d, 5)) {
          darray_write("../data/test.bn", d, length);
          fprintf(stderr, "bug found");
        }
      }
      free(d);
      free(d2);
      free(indices);
      ncycle--;
    }
    HOURGLASS_END("done");
    printf(":) Testing passed.\n");

    return 0;
  }
#endif

#if 0
  int length;
  double *d = darray_read("../data/test.bn", &length);
  printf("%d\n", length);
  darray_print2(d, length, 1);
#endif

#if 0
  int length = 100000;
  double *d1 = Unifrnd_Double_Array(length, NULL);
  double *d2 = Unifrnd_Double_Array(length, NULL);
  tic();
  int i;
  for (i = 0; i < 1000; i++) {
    darray_dot(d1, d2, length);
    //cblas_ddot (length, d1, 1, d2, 1);
    // Unifrnd_Double_Array(length, d1);
    //Unifrnd_Double_Array(length, d2);
  }
  printf("time: %llu\n", toc());
#endif

#if 0
  float d[100];
  printf("%d\n", farray_fscanf(stdin, d, 5));
  farray_print2(d, 5, 1);
  printf("%g\n", farray_var(d, 5));
#endif

#if 0
  int x1[] = {1, 2, 3, 4, 5};
  int x2[] = {4, 7, 2, 5, 0};
  printf("%g\n", iarray_var(x1, 5));
  printf("%g\n", iarray_var(x2, 5));
  printf("%g\n", iarray_cov(x1, x2, 5));
#endif

#if 0
  int m, n;
  int *d = iarray_load_csv("../data/fly_neuron_n1_branch.csv", NULL, 
			      &m, &n);
  iarray_print2(d, m, n);
#endif

#if 0
  int length;
  double res[3];
  darray_read2("../data/mouse_neuron3_org.res", res, &length);
  darray_print2(res, 3, 1);
#endif

#if 0
  int i;
  int x = 100;
  int y;
  tic();
  for (i = 0; i < 100000000; i++) {
    y = -x * 65;
    //y = x << 6;
  }
  printf("%lld\n", toc());  
#endif

#if 0
  double *a = darray_contarray(-1, 10, NULL);
  darray_print2(a, 12, 1);
#endif

#if 0
  int a[3] = {1, 2, 3};
  int b[3] = {0, 0, 0};
  /* Cannot be assigned. */
  a = b;
  
  printf("%d, %d, %d\n", a[0], a[1], a[2]);
#endif
  
#if 0
  struct A {
    int array[3];
  };

  struct A a;
  struct A b;
  a.array[0] = 1;
  a.array[1] = 2;
  a.array[2] = 3;

  b = a;

  printf("%d, %d, %d\n", b.array[0],b.array[1],b.array[2]);
  b.array[0] = 3;

  printf("%d, %d, %d\n", a.array[0],a.array[1],a.array[2]);
  printf("%d, %d, %d\n", b.array[0],b.array[1],b.array[2]);
#endif

#if 0
  struct A {
    int array[3];
  };
  
  struct B {
    int width;
    int height;
    int depth;
  };

  struct A a;
  struct B b;

  int i;
  tic();
  for (i = 0; i < 100000000; i++) {
    a.array[1] = 1;
  }
  ptoc();

  tic();
  for (i = 0; i < 100000000; i++) {
    b.height = 1;
  }
  ptoc();
#endif

#if 0
  struct A {
    int array[3];
  };
  
  struct B {
    union {
      int width;
      int height;
      int depth;
      int size[3];
    };
  };

  struct A a;
  struct B b;

  /* invalid syntax */
  b.width = 1;
#endif

#if 1
  int array[] = {1, 2, 1, 1, 2, 2, 2, 2, 1, 2, 3, 4, 5};
  size_t length = sizeof(array) / sizeof(int);
  int *out = iarray_medfilter(array, length, 3, NULL);
  iarray_print(out, length);
#endif


  return 0;
} 
