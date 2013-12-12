/**@file pfeat.c
 * @brief >> pixel feature calculation
 * @author Ting Zhao
 * @date 20-Mar-2009
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_dimage_lib.h"
#include "tz_iarray.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -m <string> -label <int>",
			 "[-a] [-scale <double>]",
			 "-o <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  FILE *out = NULL;
  
  if (Is_Arg_Matched("-a")) {
    out = fopen(Get_String_Arg("-o"), "a");
  } else {
    out = fopen(Get_String_Arg("-o"), "w");
  }

  if (out == NULL) {
    fprintf(stderr, "Unable to open %s\n", Get_String_Arg("-o"));
    exit(1);
  }

  Stack *stack = Read_Stack(Get_String_Arg("image"));
  
  double scale[] = {1.0, 1.0, 1.0};
  
  if (Is_Arg_Matched("-scale")) {
    scale[0] = scale[1] = scale[2] = Get_Double_Arg("-scale");
  }

  int m;
  int n;
  int *d = iarray_load_csv(Get_String_Arg("-m"), NULL, &m, &n);

  Object_3d *pts = Make_Object_3d(n, 0);
  int i;
  for (i = 0; i < n; i++) {
    pts->voxels[i][0] = d[i * m] - 1;
    pts->voxels[i][1] = stack->height - d[i * m + 1];
    pts->voxels[i][2] = d[i * m + 2] - 1;
  }

  DMatrix *result = Stack_Pixel_Feature_D(stack, scale, pts, NULL);

  int j;
  for (i = 0; i < n; i++) {
    fprintf(out, "%d ", Get_Int_Arg("-l"));
    for (j = 0; j < result->dim[0]; j++) {
      fprintf(out, "%g ", result->array[i * result->dim[0] + j]);
    }
    fprintf(out, "\n");
  }

  Kill_Stack(stack);

  fclose(out);

  return 0;
}
