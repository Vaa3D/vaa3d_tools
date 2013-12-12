/* testperceptor.c
 *
 * 06-Apr-2007 Initial write: Ting Zhao
 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_perceptor.h"
#include "tz_local_neuroseg.h"
#include "tz_local_neuroseg_ellipse.h"
#include "tz_locne_chain.h"
#include "tz_stack_draw.h"
#include "tz_testdata.h"
#include "tz_math.h"
#include "tz_voxel_graphics.h"
#include "tz_r2_ellipse.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
#if 0
  double z_scale = 1.0;
  Stack *stack = Read_Stack("../data/resimg.tif");

  //double position[3] = {306.666, 49.7646, 135.158};
  //double position[3] = {91.7328, 98.1664, 131.632};
  double position[3] = {157, 149, 148};
  Local_Neuroseg locseg;
  Set_Neuroseg(&(locseg.seg), 2.0, 2.0, 12, 0.0, 0.0, 0.0);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  Local_Neuroseg_Fitpack *perceptor = 
    Make_Local_Neuroseg_Fitpack(&locseg, z_scale, NULL);

  printf("%g\n", Perceptor_Fit_Score((void *) perceptor, stack,
				     Local_Neuroseg_Score_V));

  printf("%g\n", Local_Neuroseg_Score(&locseg, stack, z_scale, NULL));

  double *param = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  Local_Neuroseg_Param_Array(&locseg, z_scale, param);
  
  printf("%g\n", Perceptor_Fit_Score_A(param, stack, Local_Neuroseg_Score_G));

  int param_index[4];
  int nparam = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
				     NEUROSEG_VAR_MASK_ORIENTATION,
				     NEUROPOS_VAR_MASK_NONE,
				     param_index);

  double delta[] = {0.5, 0.5, 0.015, 0.015, 1.0, 0.01, 0.5, 0.5, 0.5, 0.1};
  double var_min[] = {1.0, 1.0, -INFINITY, -INFINITY, 3.0, 0.5, -INFINITY, 
		      -INFINITY, -INFINITY, 0.5};
  double var_max[] = {10.0, 10.0, INFINITY, INFINITY, 12.0, 1.0, INFINITY, 
		      INFINITY, INFINITY, 6.0};
		      
  Continuous_Function *cf = 
    Make_Continuous_Function(Local_Neuroseg_Score_Gv, var_min, var_max);

  Print_Local_Neuroseg(&locseg);

#if 0
  Fit_Perceptor(param, stack, param_index, nparam, delta, cf);

  
  nparam = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R,
				     NEUROPOS_VAR_MASK_NONE,
				     param_index);
  Fit_Perceptor(param, stack, param_index, nparam, delta, cf);
  
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    Local_Neuroseg_Set_Var(&(locseg), i, param[i]);
  }
#else
  Fit_Local_Neuroseg_P(&locseg, stack, param_index, nparam, z_scale, NULL);
#endif
  
  printf("best score: %g\n", 
	 Local_Neuroseg_Score(&locseg, stack, z_scale, NULL));
  Print_Local_Neuroseg(&locseg);

  free(param);

  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Label(&locseg, stack, 0, z_scale);
  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
  Free_Local_Neuroseg_Fitpack(perceptor);
  Free_Continuous_Function(cf);
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron.tif");
  double z_scale = 1.0;
  //double position[3] = {269, 222, 73}; //mouse_neuron
  //double position[3] = {274, 261, 57}; //mouse_neuron
  //double position[3] = {269, 257, 73}; //mouse_neuron

  //double position[3] = {269, 232, 63};
  //double position[3] = {177, 280, 78};
  double position[3] = {250, 377, 85};
  //double position[3] = {249, 381, 86};
  //double position[3] = {266, 188, 78};
  // double position[3] = {137, 122, 87};

  //double position[3] = {266, 188, 78};
  
  //double position[3] = {242, 151, 134}; // fly_neuron2
  //double position[3] = {236, 150, 136}; //fly_neuron2
  // double position[3] = {259, 150, 140}; //fly_neuron2
  //double position[3] = {142, 380, 51}; //fly_neuron2

  //double position[3] = {249, 107, 138}; //mouse_single
  //double position[3] = {263, 174, 137};

  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, 1.0, 1.0, TZ_PI_2, 0, 0, 0, 0,
			     position[0], position[1], position[2]);
  

  double score = Local_Neuroseg_Ellipse_Score(locne, stack, z_scale, NULL);
  printf("%g\n", score);

#  if 0
  int var_index[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  int nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(NEUROSEG_ELLIPSE_VAR_MASK_R | 
					   NEUROSEG_ELLIPSE_VAR_MASK_OFFSET,
					   NEUROPOS_VAR_MASK_NONE,
					   var_index);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 0;
  Fit_Local_Neuroseg_Ellipse(locne, stack, var_index, nvar, z_scale, NULL);
  
  Print_Local_Neuroseg_Ellipse(locne);
#  endif

#  if 0
  Locne_Chain *chain = Locne_Chain_Trace_Init(stack, z_scale, locne);
  Trace_Workspace tw;
  tw.length = 100;
  tw.fit_first = FALSE;
  tw.tscore_option = 1;
  tw.min_score = 0.3;
  tw.trace_direction = DL_BACKWARD;
  Trace_Locne(stack, z_scale, chain, &tw);

  Print_Locne_Chain(chain);  
#  endif

#  if 0
  /* Draw it in a stack */
  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};

  Geo3d_Scalar_Field* field = NULL;

  Locne_Chain_Iterator_Start(chain);
  while ((locne = Locne_Chain_Next(chain)) != NULL) {
    field = Local_Neuroseg_Ellipse_Field(locne, 0.5, NULL);
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
  Write_Stack("../data/test.tif", stack);  
#  endif
  
#endif

#if 0
  //Stack *stack = Circle_Image(10.0);
  Stack *mask = Ellipse_Mask(5.0, 10.0);

  Stack *stack = Noisy_Ellipse_Image(5.0, 10.0, 200.0);
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  locne->np.rx = 2.0;
  locne->np.ry = 2.0;
  locne->pos[0] = 15.0;
  locne->pos[1] = 25.0;
  locne->pos[2] = 0.0;

  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));
  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Ellipse_Fit_Workspace(ws);
  //Fit_Local_Neuroseg_Ellipse_W(locne, stack, 1.0, ws);
  Local_Neuroseg_Ellipse_Optimize_W(locne, stack, 1.0, ws);
  Print_Local_Neuroseg_Ellipse(locne);

  //Geo3d_Ellipse* ellipse = Local_Neuroseg_Ellipse_To_Geo3d_Ellipse(locne, NULL);

  Write_Stack("../data/test.tif", stack);

  Translate_Stack(stack, COLOR, 1);
  //coordinate_3d_t *pts = Geo3d_Ellipse_Sampling(ellipse, 20, 0, NULL);
  coordinate_3d_t pts[20];
  Local_Neuroseg_Ellipse_Points(locne, 20, 0, pts);
  int start[3], end[3];
  int i;
  int j;
  for (i = 0; i < 19; i++) {
    for (j = 0; j < 3; j++) {
      start[j] = iround(pts[i][j]);
      end[j] = iround(pts[i+1][j]);
    }
    Object_3d *line = Line_To_Object_3d(start, end);
    Stack_Draw_Object_C(stack, line, 255, 0, 0);
    Kill_Object_3d(line);
  }
    
  for (j = 0; j < 3; j++) {
    start[j] = iround(pts[19][j]);
    end[j] = iround(pts[0][j]);
  }
  Object_3d *line = Line_To_Object_3d(start, end);
  Stack_Draw_Object_C(stack, line, 255, 0, 0);
  Kill_Object_3d(line);

  Stack_Blend_Mc(stack, mask, 2.0);

  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/md/sperm.tif");
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  locne->np.rx = 2.0;
  locne->np.ry = 2.0;
  locne->pos[0] = 365.0;
  locne->pos[1] = 165.0;
  locne->pos[2] = 0.0;

  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));
  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Ellipse_Fit_Workspace(ws);
  Local_Neuroseg_Ellipse_Optimize_W(locne, stack, 1.0, ws);
  //Fit_Local_Neuroseg_Ellipse_W(locne, stack, 1.0, ws);
  Print_Local_Neuroseg_Ellipse(locne);
  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Ellipse_Draw_Stack(locne, stack, 255, 0, 0);
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Noisy_Two_Circle_Image(10.0, 25.0, 25.0, 10.0, 25.0, 45.0, 50.0);
  Write_Stack("../data/test.tif", stack);
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  locne->np.rx = 2.0;
  locne->np.ry = 2.0;
  locne->pos[0] = 20.0;
  locne->pos[1] = 20.0;
  locne->pos[2] = 0.0;

  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));
  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Ellipse_Fit_Workspace(ws);
  Local_Neuroseg_Ellipse_Optimize_W(locne, stack, 1.0, ws);
  //Fit_Local_Neuroseg_Ellipse_W(locne, stack, 1.0, ws);
  Print_Local_Neuroseg_Ellipse(locne);
  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Ellipse_Draw_Stack(locne, stack, 255, 0, 0);
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Two_Circle_Image(10.0, 25.0, 25.0, 10.0, 35.0, 35.0);
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  locne->np.rx = 5.0;
  locne->np.ry = 10.0;
  locne->pos[0] = 25.0;
  locne->pos[1] = 25.0;
  locne->pos[2] = 0.0;


  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));
  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Ellipse_Fit_Workspace(ws);
  Local_Neuroseg_Ellipse_Optimize_W(locne, stack, 1.0, ws);
  //Fit_Local_Neuroseg_Ellipse_W(locne, stack, 1.0, ws);
  Print_Local_Neuroseg_Ellipse(locne);
  Local_Neuroseg_Ellipse_Label(locne, stack, 1.0, -1, 0);
  Write_Stack("../data/test.tif", stack);
  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Ellipse_Draw_Stack(locne, stack, 255, 0, 0);
  Write_Stack("../data/test2.tif", stack);
  
#endif

#if 0
  int start[3] = {50, 10, 10};
  int end[3] = {50, 50, 10};
  double sigma[3] = {1.5, 1.5, 3.0};
  Stack *stack = Tube_Stack(start, end, 40, sigma);
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 5.0, 0.0, 11.0, -TZ_PI_2+0.5, 0.5, 0.0, 0.0, 2.0,
      50.0, 20.0, 10.0);

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Fit_Workspace(ws);
  tic();
  printf("%g\n", Fit_Local_Neuroseg_W(locseg, stack, 1.0, ws));
  ptoc();
  
#endif

#if 0
  Stack *stack = Noisy_Two_Circle_Image(10.0, 25.0, 25.0, 15.0, 25.0, 35.0, 20.0);
  Write_Stack("../data/test.tif", stack);
  Local_Neuroseg_Ellipse *locne1 = New_Local_Neuroseg_Ellipse();
  locne1->np.rx = 2.0;
  locne1->np.ry = 2.0;
  locne1->pos[0] = 20.0;
  locne1->pos[1] = 20.0;
  locne1->pos[2] = 0.0;

  Local_Neuroseg_Ellipse *locne2 = New_Local_Neuroseg_Ellipse();
  locne2->np.rx = 2.0;
  locne2->np.ry = 2.0;
  locne2->pos[0] = 20.0;
  locne2->pos[1] = 40.0;
  locne2->pos[2] = 0.0;

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Ellipse_Fit_Workspace(ws);
  ws->sws->mask = Copy_Stack(stack);
  /*
  ws->nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(NEUROSEG_ELLIPSE_VAR_MASK_R |
					     NEUROSEG_ELLIPSE_VAR_MASK_OFFSET,
					     NEUROPOS_VAR_MASK_NONE,
					     ws->var_index);
					     */
  //ws->sws->mask = NULL;
  ws->sws->fs.n = 1;
  ws->sws->fs.options[0] = STACK_FIT_CORRCOEF;

  double center[3];
  int k;
  for (k = 0; k < 200; k++) {
    Fit_Local_Neuroseg_Ellipse_W(locne1, stack, 1.0, ws);
    Local_Neuroseg_Ellipse_Stack_Centroid(locne1, stack, ws->sws->mask, center);
    Local_Neuroseg_Ellipse_Reset_Center(locne1, center);
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Ellipse_Label(locne1, ws->sws->mask, 1.0, 0, 1);
    }
    Fit_Local_Neuroseg_Ellipse_W(locne2, stack, 1.0, ws);
    Local_Neuroseg_Ellipse_Stack_Centroid(locne2, stack, ws->sws->mask, center);
    Local_Neuroseg_Ellipse_Reset_Center(locne2, center);
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Ellipse_Label(locne2, ws->sws->mask, 1.0, 0, 1);
    }
  }

  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Ellipse_Draw_Stack(locne1, stack, 255, 0, 0);
  Local_Neuroseg_Ellipse_Draw_Stack(locne2, stack, 0, 255, 0);
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Local_R2_Ellipse *ellipse = New_Local_R2_Ellipse();
  ellipse->transform.alpha = 1.0;
  Print_Local_R2_Ellipse(ellipse);
  double center[3];
  center[0] = 111;
  center[1] = 230;
  center[2] = 0;
  Local_R2_Ellipse_Set_Center(ellipse, center);
  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field(ellipse, NULL);
  Print_Geo3d_Scalar_Field(field);
  Geo3d_Scalar_Field_Centroid(field, center);
  printf("%g, %g, %g\n", center[0], center[1], center[2]);

  Local_R2_Ellipse_Position_Adjust(ellipse, NULL, 1.0);
  Local_R2_Ellipse_Center(ellipse, center);
  printf("%g, %g, %g\n", center[0], center[1], center[2]);

  Local_Neuroseg *locseg = Local_R2_Ellipse_To_Local_Neuroseg(ellipse,NULL);
  Print_Local_Neuroseg(locseg);

#endif

#if 1
  Local_R2_Ellipse *ellipse = New_Local_R2_Ellipse();
  double center[3];
  center[0] = 132;
  center[1] = 115;
  center[2] = 0;
  Local_R2_Ellipse_Set_Center(ellipse, center);
  Print_Local_R2_Ellipse(ellipse);

  Stack *stack = Read_Stack("/home/zhaot/Work/neurolabi/data/benchmark/rice_label.tif");
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_R2_Ellipse_Fit_Workspace(ws);
  Local_R2_Ellipse_Optimize_W(ellipse, stack, 1.0, 1, ws);
  Local_R2_Ellipse_Center(ellipse, center);
  printf("center after fitting: %g, %g, %g\n", center[0], center[1], center[2]);

  Print_Local_R2_Ellipse(ellipse);

  Local_Neuroseg *locseg = Local_R2_Ellipse_To_Local_Neuroseg(ellipse,NULL);
  Print_Local_Neuroseg(locseg);
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.loffset += 0.5;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.loffset -= 1.0;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.loffset += 5;

  ellipse->transform.roffset += 0.5;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.roffset -= 1.0;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));
  ellipse->transform.roffset += 0.5;

  ellipse->transform.toffset += 0.5;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.toffset -= 1.0;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));
  ellipse->transform.toffset += 0.5;

  ellipse->transform.boffset += 0.5;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));

  ellipse->transform.boffset -= 1.0;
  printf("Score: %g\n", Local_R2_Ellipse_Score_P(ellipse, stack, 1.0, NULL));
  ellipse->transform.boffset += 0.5;
#endif

  return 0;
}
