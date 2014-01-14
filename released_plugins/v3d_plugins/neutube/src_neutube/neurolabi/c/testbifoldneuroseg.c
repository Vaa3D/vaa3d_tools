/* testbifoldneuroseg.c
 *
 * 27-Apr-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <image_lib.h>
#include "tz_constant.h"
#include "tz_stack_draw.h"
#include "tz_local_neuroseg.h"
#include "tz_local_bifold_neuroseg.h"
#include "tz_stack_stat.h"
#include "tz_farray.h"
#include "tz_geo3d_scalar_field.h"

int main()
{
  /* Read stack */
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  double z_scale = 1.0;

  /* New a bifold segment */
  Local_Bifold_Neuroseg *locbn = New_Local_Bifold_Neuroseg();
  /*
  Set_Local_Bifold_Neuroseg(locbn, 2, 2, 2, 2, 30, 0.5,
			    TZ_PI_2, TZ_PI_2, -TZ_PI_2, 0, 
			    461, 296, 144); //fly_neuron.tif
  */
  
  Set_Local_Bifold_Neuroseg(locbn, 2, 2, 2, 2, 40, 0.5, 
			    TZ_PI_2, TZ_PI_2, TZ_PI_2, TZ_PI_2, 
			    290, 304, 112); //fly_neuron.tif
  
  
  /*
  Set_Local_Bifold_Neuroseg(locbn, 2, 2, 2, 2, 30, 0.5, 
			    TZ_PI_2, TZ_PI_2, -TZ_PI_2, 0, 
			    320, 164, 148); //fly_neuron.tif
  */  

  /*
  Set_Local_Bifold_Neuroseg(locbn, 3, 3, 3, 3, 30, 0.5, 
			    TZ_PI_2, TZ_PI_2, -TZ_PI_2, 0,
			    262, 136, 141); //fly_neuron2.tif
  */

  /*
  Set_Local_Bifold_Neuroseg(locbn, 3, 3, 3, 3, 30, 0.5, 
			    TZ_PI_2, TZ_PI_2, -TZ_PI_2, 0,
			    236, 396, 143); //fly_neuron2.tif
  */

  /* fit */
  int var_index[LOCAL_BIFOLD_NEUROSEG_NPARAM];
  int nvar = Local_Bifold_Neuroseg_Var_Mask_To_Index
    (BIFOLD_NEUROSEG_VAR_MASK_R |
     BIFOLD_NEUROSEG_VAR_MASK_KNOT |
     BIFOLD_NEUROSEG_VAR_MASK_ORIENTATION2 |
     BIFOLD_NEUROSEG_VAR_MASK_ORIENTATION, NEUROPOS_VAR_MASK_NONE, var_index);

  Fit_Local_Bifold_Neuroseg(locbn, stack, var_index, nvar, z_scale, NULL);

  Print_Local_Bifold_Neuroseg(locbn);

  /* Generate field */
  Geo3d_Scalar_Field *field = Local_Bifold_Neuroseg_Field(locbn, 1.0, NULL);
  Delete_Local_Bifold_Neuroseg(locbn);

  /* Draw it in a stack */
  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};
  Geo3d_Scalar_Field_Draw_Stack(field, label, coef, range);

  /* Turn the stack to GREY type */
  Translate_Stack(label, GREY, 1);

  /* Make canvas */
  Translate_Stack(stack, COLOR, 1);
  
  /* Label the canvas */
  Stack_Label_Color(stack, label, 5.0, 1.0, label);

  /* Save the stack */
  Write_Stack("../data/test.tif", stack);

  /* clean up */
  Kill_Geo3d_Scalar_Field(field);
  Kill_Stack(stack);
  Kill_Stack(label);
  
  return 0;
}
