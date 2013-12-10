/* trace_tube.c
 *
 * 04-Jun-2008
 */

/* trace_tube: trace a tube
 * 
 * trace_tube -p <int> <int> <int> infile -o outfile [-mask mask_file]
 *            [-r radius] [-z z_scale]
 *
 * -p: seed position
 * -z: z scale
 * -mask: mask image file
 * -r: initial radius
 * infile: image file to trace
 * -o outfile: output image file
 */

#include <utilities.h>
#include "tz_stack_math.h"
#include "tz_neurotrace.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "tz_stack_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = { "-p <int> <int> <int>",
			  " [-z <double>] [-mask <string>]",
			  " [-r <double>]",
			  " <image:string> -o <string>",
			  NULL};

  Process_Arguments(argc, argv, Spec, 1);
    
  double z_scale = 1.0;
  if (Is_Arg_Matched("-z")) {
    z_scale = Get_Double_Arg("-z");
  }

  double pos[3];
  pos[0] = Get_Int_Arg("-p", 1);
  pos[1] = Get_Int_Arg("-p", 2);
  pos[2] = Get_Int_Arg("-p", 3);

  pos[2] /= z_scale;

  //Neurochain *chain = New_Neurochain();
  Local_Neuroseg *locseg = New_Local_Neuroseg();

  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);
  double r0 = 5.0;
  if (Is_Arg_Matched("-r")) {
    r0 = Get_Double_Arg("-r");
  }
  Set_Neuroseg(&(locseg->seg), r0, 0.0, 11.0, 0.0, 0.0, 0.0,
	       NEUROSEG_DEFAULT_ALPHA, 1.0);

  Stack *stack =Read_Stack(Get_String_Arg("image"));
  Local_Neuroseg_Optimize(locseg, stack, z_scale);

  Print_Local_Neuroseg(locseg);

#ifdef _DEBUG_
  Print_Stack_Info(stack);
  Print_Local_Neuroseg(locseg);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);
  Geo3d_Scalar_Field_Stack_Sampling(field, stack, z_scale, field->values);
  //Print_Geo3d_Scalar_Field(field);
#endif

  Stack *mask = NULL;
  if (Is_Arg_Matched("-mask")) {
    mask = Read_Stack(Get_String_Arg("-mask"));
  }
  
  tic();

  Trace_Workspace *tw = Locseg_Chain_Default_Trace_Workspace(NULL, stack);
  /*
  Trace_Workspace *tw = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(tw);
  tw->trace_range[0] = 0;
  tw->trace_range[1] = 0;
  tw->trace_range[2] = 0;
  tw->trace_range[3] = stack->width - 1;
  tw->trace_range[4] = stack->height - 1;
  tw->trace_range[5] = stack->depth - 1;
  */
  /*
  tw->length = 200;
  tw->fit_first = FALSE;
  tw->tscore_option = STACK_FIT_CORRCOEF;
  tw->min_score = MIN_FIT_SCORE;
  //tw->trace_direction = DL_BOTHDIR;

  //tw->trace_status[1] = TRACE_NOT_ASSIGNED;
  tw->trace_mask = mask;N
  tw->dyvar[0] = 20.0;
  tw->dyvar[2] = 0.5;
  tw->test_func = Locseg_Chain_Trace_Test;
  */
  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;
  Locseg_Node *p = Make_Locseg_Node(locseg, tr);
  Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

  Trace_Locseg(stack, z_scale, locseg_chain, tw);
  
  printf("%d\n", tw->trace_status[0]);
  Print_Trace_Status(tw->trace_status[0]);
  printf("%d\n", tw->trace_status[1]);
  Print_Trace_Status(tw->trace_status[1]);
  printf("time passed: %lld\n", toc());

  Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
  Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

  //  Print_Locseg_Chain(locseg_chain);
  //Neurochain *chain_head = Neurochain_From_Locseg_Chain(chain);
  //chain_head = Neurochain_Remove_Overlap_Ends(chain_head);
  //chain_head = Neurochain_Remove_Turn_Ends(chain_head, 1.0);

  //printf("%d segments\n", Neurochain_Length(chain_head, BOTH));
  //printf("length: %g\n", Neurochain_Geolen(chain_head));

  Translate_Stack(stack, GREY, 1);
  Stretch_Stack_Value_Q(stack, 0.999);   

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Locseg_Chain_Label(locseg_chain, canvas, z_scale);
  Write_Stack(Get_String_Arg("-o"), canvas);
  Kill_Stack(canvas);
  Kill_Stack(stack);

  return 1;
}
