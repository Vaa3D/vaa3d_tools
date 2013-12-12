/* testsystem.c
 * 
 * 29-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include "tz_utilities.h"
#include "tz_error.h"

INIT_EXCEPTION_MAIN(e)

typedef double test_t[3];

int main(int argc, char *argv[])
{
#if 0
  printf("Checking ...\n");
  printf("int size: %lu; short size: %lu; long size: %lu; pointer size: %lu\n",
	 sizeof(int),sizeof(short),sizeof(long),sizeof(int*));
  if(sizeof(int)==4 && sizeof(long)==4 && sizeof(void*)==4 && 
     sizeof(long long)==8) {
    printf("Possible ILP32LL model.\n");
  }

  if(sizeof(int)<4) {
    printf("This program assumes the int type to have at least 4 types.");
  }

  double d = 0.0;
  int i = 0;
  for (d = 0.5; d < 2147400.0; d += 1.0, i++) {
    int t1 = (int) floor(d);
    int t2 = (int) (floor(d) + 0.5);
    //printf("%d ",  t1);
    if (t1 != i) {
      printf("Type conversion problem: %d, %d\n", t1, t2);
      printf("Affected function: Stack_Point_Sampling\n");
    }
  }
#endif

#if 0
  int i;
  for (i = 0; i < 10; i++) {
    if (i > 5) {
      int j;
      int k = 0;
      printf("%d\n", j++);
    }
    int k = 0;
  }
#endif

#if 0
  free(NULL);
#endif

#if 0
  DIR *dir = opendir("/Users/zhaot/Work/neurolabi/c");
  struct dirent *ent = readdir(dir);
  while (ent != NULL) {
    printf("%s\n", ent->d_name);
    ent = readdir(dir);
  }
  closedir(dir);
#endif

#if 0
  printf("%d\n", dir_fnum("/Users/zhaot/Work/neurolabi/data/fly_neuron", "bn"));
#endif

#if 0
  test_t *x = (test_t *) malloc(100 * sizeof(test_t));
  printf("%p\n", x);

  double *y = (double *) x;
  
  printf("%p\n", y);
  y++;
  printf("%p\n", y);
  printf("%p\n", x);

  free(x);
#endif

#if 1
  char str[65];
  printf("%g\n", round(1.6));
  double_binstr((1.6) + ldexp(1.0, 52), str);
  printf("%lu: %s\n", strlen(str), str);
  int i;
  printf("%c ", str[0]);
  for (i = 1; i < 12; i++) {
    printf("%c", str[i]);
  }
  printf(" ");
  for (i = 12; i < 64; i++) {
    printf("%c", str[i]);
  }
  printf("\n");

  double_binstr(INFINITY, str);
  printf("%lu: %s\n", strlen(str), str);
#endif

  return 0;
}
