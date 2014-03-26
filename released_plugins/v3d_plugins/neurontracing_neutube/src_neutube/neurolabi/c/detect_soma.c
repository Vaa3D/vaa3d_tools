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
    "[-mask <string>] [-z <double>]",
    "<image:string> -D <string> -S<string> -o <string>",
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

  Local_Neuroseg_Ellipse *segs[20];



  Stack *stack = Read_Stack(Get_String_Arg("image"));
  Locne_Chain *somas[20];
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
    
    if ((traced == FALSE) && (nsoma < 20)){
      index = indices[i];

      tube_r = seed->values[index];

      printf("%g\n", tube_r);
      //return 1;

      segs[nsoma] = New_Local_Neuroseg_Ellipse();
      Set_Local_Neuroseg_Ellipse(segs[nsoma], r, r, TZ_PI_2, 0, 0, 0, 0,
				 seed->points[index][0],
				 seed->points[index][1],
				 seed->points[index][2]);
      Soma_Trace_Workspace *stw = New_Soma_Trace_Workspace();
      stw->tw.dyvar[0] = tube_r * 3.0 * 3.0;
      stw->tw.dyvar[1] = 20.0;
      stw->tw.test_func = Locne_Chain_Trace_Test;
      somas[nsoma] = Trace_Soma(stack, 1.0, segs[nsoma], stw);
      printf("soma score: %g\n", Soma_Score(somas[nsoma]));
      if ((Soma_Score(somas[nsoma]) < 1.5) ||
	  (Locne_Chain_Length(somas[nsoma]) < 10)) {
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
    Stack *traced = Make_Stack(GREY, stack->width, stack->height, stack->depth);
    One_Stack(traced);
    for (i = 0; i < nsoma; i++) {
      Soma_Stack_Mask(somas[i], traced, 1.0, 0.0);
    }
    sprintf(file_path, "%s/%s", dir, Get_String_Arg("-mask"));
    Write_Stack(file_path, traced);

    printf("%s created\n", file_path);
  }

  return 1;
}
