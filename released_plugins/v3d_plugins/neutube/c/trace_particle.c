/* trace_particle.c
 *
 * 20-May-2008 Initial write: Ting Zhao
 */

#include <utilities.h>
#include "tz_constant.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_stack_draw.h"
#include "tz_trace_utils.h"
#include "tz_stack_lib.h"
#include "tz_locne_chain.h"

/*
 * trace_particle -p <int> <int> <int> -r <double> infile -o outfile
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

  Local_Neuroseg_Ellipse *locnp = New_Local_Neuroseg_Ellipse();
  //Set_Local_Neuroseg_Plane(locnp, r, 0, 0, 0, 0, pos[0], pos[1], pos[2]);
  Set_Local_Neuroseg_Ellipse(locnp, r, r, TZ_PI_2, 0, 0, 0, 0, pos[0], pos[1], pos[2]);

  char *imgfile = Get_String_Arg("image");
  Stack *stack = Read_Stack(imgfile);

  Locne_Chain *chain = Locne_Chain_Trace_Init(stack, 1.0, locnp, NULL);

  /*
  Trace_Workspace *tw = New_Trace_Workspace();
  tw->length = 100;
  tw->fit_first = FALSE;
  tw->tscore_option = 1;
  tw->min_score = 0.3;
  tw->trace_mask = NULL;
  tw->dyvar[0] = 100.0;
  tw->test_func = Locnp_Chain_Trace_Test;
  tw->trace_status[0] = TRACE_NORMAL;
  tw->trace_status[1] = TRACE_NORMAL;
  */
  Trace_Workspace *tw = Make_Locne_Trace_Workspace(DL_BOTHDIR, 100, 0.3,
						   100.0, 20.0);
  Trace_Locne(stack, 1.0, chain, tw);

  Print_Trace_Status(tw->trace_status[0]);
  Print_Trace_Status(tw->trace_status[1]);

  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};

  Geo3d_Scalar_Field* field = NULL;

  Locne_Chain_Iterator_Start(chain, DL_HEAD);
  while ((locnp = Locne_Chain_Next_Seg(chain)) != NULL) {
    field = Local_Neuroseg_Ellipse_Field(locnp, 0.5, NULL);
    Geo3d_Scalar_Field_Draw_Stack(field, label, coef, range);
    Kill_Geo3d_Scalar_Field(field);
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
