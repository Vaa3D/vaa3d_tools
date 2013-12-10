/* detect_soma.c
 *
 * 11-Jun-2008 Initial write: Ting Zhao
 */
#include <stdio.h>
#include <utilities.h>
#include "tz_constant.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_neurotrace.h"
#include "tz_locne_chain.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {
    "[-mask <string>]",
    "<image:string> -D<string> -S<string> -o <string>",
    NULL};

  Process_Arguments(argc, argv, Spec, 1);

  char *dir = Get_String_Arg("-D");
  
  char file_path[100];
  sprintf(file_path, "%s/%s", dir, Get_String_Arg("-S"));

  Geo3d_Scalar_Field *seed = Read_Geo3d_Scalar_Field(file_path);

  int *indices = iarray_malloc(seed->size);
  double *values = darray_malloc(seed->size);

  int i;
  for (i = 0; i < seed->size; i++) {
    indices[i] = i;
  }

  darraycpy(values, seed->values, 0, seed->size);

  darray_qsort(values, indices, seed->size);
  
  int index = indices[seed->size-1];
  
  double r = seed->values[index];

  Local_Neuroseg_Ellipse *segs[10];



  Stack *stack = Read_Stack(Get_String_Arg("image"));
  Locne_Chain *somas[10];
  int nsoma = 0;

  /*
  segs[0] = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(segs[0], r, r, TZ_PI_2, 0, 0, 0,
			     seed->points[index][0],
			     seed->points[index][1],
			     seed->points[index][2]);
  
  somas[nsoma++] = Trace_Soma(stack, 1.0, segs[0], NULL);
  */

  int j;
  double soma_r, tube_r;
  double max_r;
  soma_r = -1.0;

  for (i = seed->size - 1; i >= 0; i--) {
    BOOL traced = FALSE;
    for (j = 0; j < nsoma; j++) {
      if (Locne_Chain_Hittest(somas[j], seed->points[indices[i]]) == TRUE) {
	traced = TRUE;
	Print_Coordinate_3d(seed->points[indices[i]]);
	break;
      }
    }
    
    if (traced == FALSE) {
      index = indices[i];

      tube_r = seed->values[index];
      segs[nsoma] = New_Local_Neuroseg_Ellipse();
      Set_Local_Neuroseg_Ellipse(segs[nsoma], r, r, TZ_PI_2, 0, 0, 0,
				 seed->points[index][0],
				 seed->points[index][1],
				 seed->points[index][2]);
      somas[nsoma] = Trace_Soma(stack, 1.0, segs[nsoma], NULL);

      if ((Soma_Score(somas[nsoma]) < 1.5) || (nsoma >= 9)
	  || (Locne_Chain_Length(somas[nsoma]) < 5)) {
	sprintf(file_path, "%s/%s.bn", dir, "max_r");
	if (soma_r < 0.0) {
	  max_r = tube_r * 1.5;
	} else {
	  max_r = (tube_r + soma_r) / 2.0;
	}
	darray_write(file_path, &max_r, 1);
	break;
      } else {
	soma_r = tube_r;
      }

      nsoma++;
    }
  }
 
  printf("%d soma found.\n", nsoma);

  for (i = 0; i < nsoma; i++) {
    Print_Local_Neuroseg_Ellipse(segs[i]);
    printf("soma score: %g\n", Soma_Score(somas[i]));
    sprintf(file_path, "%s/%s%d.bn", dir, Get_String_Arg("-o"), i); 
    Write_Locne_Chain(file_path, somas[i]);
  }

  if (Is_Arg_Matched("-mask")) {
    Stack *trace_mask = Make_Stack(GREY16, 
				   stack->width, stack->height, stack->depth);
    Zero_Stack(trace_mask);
    for (i = 0; i < nsoma; i++) {
      Soma_Stack_Mask(somas[i], trace_mask, 1.0, i + 1);
    }
    sprintf(file_path, "%s/%s", dir, Get_String_Arg("-mask"));
    Write_Stack(file_path, trace_mask);

    printf("%s created", file_path);
  }

  return 0;
}
