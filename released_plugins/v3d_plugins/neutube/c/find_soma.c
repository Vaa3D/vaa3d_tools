/* trace_particle.c
 *
 * 20-May-2008 Initial write: Ting Zhao
 */
#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_stack_draw.h"
#include "tz_stack_lib.h"
#include "tz_neurotrace.h"
#include "tz_locne_chain.h"

/*
 * find_soma -p <int> <int> <int> -r <double> infile -o outfile
 */
int main(int argc, char *argv[])
{
  static char *Spec[] = { 
    "-p <int> <int> <int> -r <double>",
    "<image:string> -o <string>",
    NULL};

  Process_Arguments(argc, argv, Spec, 1);
  
  double pos[3];
  pos[0] = Get_Int_Arg("-p", 1);
  pos[1] = Get_Int_Arg("-p", 2);
  pos[2] = Get_Int_Arg("-p", 3);
  double r = Get_Double_Arg("-r");

  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, r, r, TZ_PI_2, 0, 0, 0,
			     pos[0], pos[1], pos[2]);

  char *imgfile = Get_String_Arg("image");
  Stack *stack = Read_Stack(imgfile);

  Locne_Chain *chain = Trace_Soma(stack, 1.0, locne, NULL);

#if 0
  Locne_Chain *chain = Locne_Chain_Trace_Init(stack, 1.0, locne);

  Trace_Workspace tw;
  tw.length = 50;
  tw.fit_first = FALSE;
  tw.tscore_option = 1;
  tw.min_score = 0.3;
  tw.trace_direction = DL_BOTHDIR;
  Trace_Locne(stack, 1.0, chain, &tw);

  int length = Locne_Chain_Length(chain);
  double *rx_array = darray_malloc(length);
  double *ry_array = darray_malloc(length);
  int i = 0;
  Locne_Chain_Iterator_Start(chain);
  while ((locne = Locne_Chain_Next(chain)) != NULL) {
    rx_array[i] = locne->np.rx;
    ry_array[i] = locne->np.ry;
    i++;
  }

  darray_write("../data/rx_array.bn", rx_array, length);
  
  
  darray_mul(rx_array, ry_array, length);
  //darraycpy(rx_array, ry_array, 0, length);
  //darray_max2(rx_array, ry_array, length);
  darray_avgsmooth(rx_array, length, 3, ry_array);
  darray_scale(ry_array, length, 1, length);
  darray_curvature(ry_array, length, rx_array);
  darray_div(rx_array, ry_array, length);

  darray_write("../data/rxy_array.bn", ry_array, length);
  darray_write("../data/test.bn", rx_array, length);

  int max_len = 0;
  int cur_len = 0;
  int status = 2;
  int best_ends[] = {-1, -1};
  int cur_ends[] = {-1, -1};

  double threshold = -0.01;
  for (i = 0; i < length; i++) {
    switch (status) {
    case 0: /* start */
      if (rx_array[i] >= threshold) {
	status = 1;
      } else {
	status = 2;
      }
      break;
    case 1: /* begin */
      if (rx_array[i] < threshold) {
	status = 2;
      }
      break;
    case 2: /* wait */
      if (rx_array[i] >= threshold) {
	status = 3;
	cur_ends[0] = i;
	cur_len = 1;
      }
      break;
    case 3: /* open */
      if (rx_array[i] >= threshold) {
	cur_len++;
      } else {
	cur_ends[1] = i;
	if (cur_len > max_len) {
	  best_ends[0] = cur_ends[0];
	  best_ends[1] = cur_ends[1];
	  max_len = cur_len;
	}
	status = 2;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
    }
    printf("%d ", status);
  }

  if (status == 3) {/* still open */
    cur_ends[1] = i;
    if (cur_len > max_len) {
      best_ends[0] = cur_ends[0];
      best_ends[1] = cur_ends[1];
      max_len = cur_len;
    }
    status = 2;
  }

  printf("\n");

  printf("%d: (%d, %d)\n", max_len, best_ends[0], best_ends[1]);
  //  return 1;


  free(rx_array);
  free(ry_array);
#endif

  
  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};

  Geo3d_Scalar_Field* field = NULL;

  Locne_Chain_Iterator_Start(chain);
  int offset = 0;
  while ((locne = Locne_Chain_Next(chain)) != NULL) {
    //if ((offset >= best_ends[0]) && (offset <= best_ends[1])) {
      field = Local_Neuroseg_Ellipse_Field(locne, 0.5, NULL);
      Geo3d_Scalar_Field_Draw_Stack(field, label, coef, range);
      Kill_Geo3d_Scalar_Field(field);
      //}
    offset++;
  }

  /* Turn the stack to GREY type */
  label = Scale_Float_Stack((float *) label->array, label->width, label->height,
			    label->depth, GREY);

  /* Make canvas */
  Translate_Stack(stack, COLOR, 1);
  
  /* Label the canvas */
  Stack_Label_Color(stack, label, 5.0, 1.0, label);

  /* Save the stack */
  Write_Stack(Get_String_Arg("-o"), stack);

  return 0;
}
