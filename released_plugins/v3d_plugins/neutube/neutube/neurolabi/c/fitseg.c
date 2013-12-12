#include <stdio.h>
#include <utilities.h>
#include "tz_stack_math.h"
#include "tz_neurotrace.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_constant.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = { "-p <int> <int> <int>",
			  " [-z <double>] [-r <double>]",
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

  Local_Neuroseg *locseg = New_Local_Neuroseg();

  double r0 = 2.0;
  if (Is_Arg_Matched("-r")) {
    r0 = Get_Double_Arg("-r");
  }
  Set_Neuroseg(&(locseg->seg), r0, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

  printf("%g, %g, %g\n", pos[0], pos[1], pos[2]);
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

  Stack *stack =Read_Stack(Get_String_Arg("image"));

  
  Print_Local_Neuroseg(locseg);

  double score = Local_Neuroseg_Optimize(locseg, stack, z_scale);

  Print_Local_Neuroseg(locseg);
  printf("%g\n", score);

  /*  
  locseg->seg.r1 *= 1.5;
  locseg->seg.r2 *= 1.5;
  locseg->seg.scale = 0.5;
  printf("%g\n", Local_Neuroseg_Score(locseg, stack, z_scale, NULL));
  */
  /*
  int var_index[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALPHA |
				     NEUROSEG_VAR_MASK_SCALE,
				     NEUROSEG_VAR_MASK_NONE, var_index);

  
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, NULL);

  Print_Local_Neuroseg(locseg);
  */
  /*
  Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.75);
  Neuroseg_Fit_Score ort_fs;
  ort_fs.n = 1;		
  ort_fs.options[0] = STACK_FIT_CORRCOEF;
  Local_Neuroseg_Orientation_Search_B(locseg2, stack, z_scale, &ort_fs);
  //Local_Neuroseg_Position_Adjust(locseg2, stack, z_scale);
  */
  double feats[100];
  int nfeat;

  nfeat = Local_Neuroseg_Stack_Feature(locseg, stack, z_scale, feats);
  printf("%d features\n", nfeat);
  darray_print2(feats, nfeat, 1);

  Translate_Stack(stack, GREY, 1);
  Stretch_Stack_Value_Q(stack, 0.999);   

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Local_Neuroseg_Label(locseg, canvas, 0, z_scale);


  //Local_Neuroseg_Label(locseg2, canvas, 1, z_scale);

  //  Locseg_Chain_Label(chain, canvas, z_scale);
  Write_Stack(Get_String_Arg("-o"), canvas);
  Kill_Stack(canvas);
  Kill_Stack(stack);

  return 0;
}
