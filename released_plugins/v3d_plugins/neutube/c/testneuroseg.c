#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_math.h"
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_vrml_io.h"
#include "tz_geometry.h"
#include "tz_geo3d_utils.h"
#include "tz_local_neuroseg.h"
#include "tz_local_neuroseg_plane.h"
#include "tz_local_neuroseg_ellipse.h"
#include "tz_coordinate_3d.h"
#include "tz_stack_draw.h"
#include "tz_cont_fun.h"
#include "tz_locnp_chain.h"
#include "tz_locne_chain.h"
#include "tz_workspace.h"
#include "tz_rpi_neuroseg.h"
#include "tz_local_rpi_neuroseg.h"
#include "tz_image_io.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{
  static char *Spec[] = {"[-t]", NULL};
 
  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    /* example test */

    Neuroseg *seg = New_Neuroseg();
    Set_Neuroseg(seg, 2.0, 1.0, 10.0, 0.0, 1.0, 0.0, 0.0, 2.0);
    
    double eps = 0.00001;

    /* There are several routines to get the shape parameters of a segment */
    if (seg->r1 != Neuroseg_Ry_Z(seg, 0.0)) {
      PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
      return 1;
    }

    /* t is a normalized parameter for z ([0, 1])*/
    if (seg->r1 != Neuroseg_Ry_T(seg, 0.0)) {
      PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
      return 1;
    }

    if (Compare_Float(Neuroseg_Ry_T(seg, 0.5), 
		      Neuroseg_Ry(seg, NEUROSEG_CENTER), eps) != 0.0) {
      PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
      return 1;
    }
		      
    if (Compare_Float(Neuroseg_Ry_T(seg, 1.0), 
		      Neuroseg_Ry(seg, NEUROSEG_TOP), eps) != 0.0) {
      PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
      return 1;
    }
    
    /* The cone coefficient */
    if (Compare_Float((Neuroseg_Ry_T(seg, 1.0) - Neuroseg_Ry_T(seg, 0.0)) / (seg->h - 1.0), NEUROSEG_COEF(seg), eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
	return 1;
    }
    
    if (Compare_Float((Neuroseg_Rx_T(seg, 1.0) - Neuroseg_Rx_T(seg, 0.0)) / (seg->h - 1.0), NEUROSEG_COEF(seg) * seg->scale, eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent ry.");
	return 1;
    }

    double z;
    
    for (z = 0.0; z < seg->h; z += 1.0) {
      /* The scale (rx / ry) is preserved */
      if (Compare_Float(Neuroseg_Rx_Z(seg, z) / Neuroseg_Ry_Z(seg, z), 
			seg->scale, eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      if (Compare_Float(Neuroseg_Rx_Z(seg, z), 
			Neuroseg_Rx_T(seg, z / (seg->h - 1.0)), eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      if (Compare_Float(Neuroseg_Rx_Z(seg, z) * Neuroseg_Ry_Z(seg, z), 
		        Neuroseg_Rxy_Z(seg, z) * Neuroseg_Rxy_Z(seg, z), 
			eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }      

      if (Neuroseg_Rx_Z(seg, z) != Neuroseg_Rc_Z(seg, z, NEUROSEG_CIRCLE_RX)) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      if (Neuroseg_Ry_Z(seg, z) != Neuroseg_Rc_Z(seg, z, NEUROSEG_CIRCLE_RY)) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      /* Comparing the returned values directly got some problem with linux */
      double tmp1 = Neuroseg_Rxy_Z(seg, z);
      double tmp2 = Neuroseg_Rc_Z(seg, z, NEUROSEG_CIRCLE_RXY);

      if (tmp1 != tmp2) {
	printf("%g\n", Neuroseg_Rxy_Z(seg, z) -
	       Neuroseg_Rc_Z(seg, z, NEUROSEG_CIRCLE_RXY));
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }
    }

    double t;

    for (t = 0.0; t < 1.0; t += 0.1) {
      /* The scale (rx / ry) is preserved */
      if (Compare_Float(Neuroseg_Rx_T(seg, t) / Neuroseg_Ry_T(seg, t), 
			seg->scale, eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      if (Compare_Float(Neuroseg_Rx_T(seg, t) * Neuroseg_Ry_T(seg, t), 
		        Neuroseg_Rxy_T(seg, t) * Neuroseg_Rxy_T(seg, t), 
			eps) != 0) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }      

      double tmp1 = Neuroseg_Rx_T(seg, t);
      double tmp2 = Neuroseg_Rc_T(seg, t, NEUROSEG_CIRCLE_RX);

      if (tmp1 != tmp2) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      tmp1 = Neuroseg_Ry_T(seg, t);
      tmp2 = Neuroseg_Rc_T(seg, t, NEUROSEG_CIRCLE_RY);
      if (tmp1 != tmp2) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }

      tmp1 = Neuroseg_Rxy_T(seg, t);
      tmp2 = Neuroseg_Rc_T(seg, t, NEUROSEG_CIRCLE_RXY);
      if (tmp1 != tmp2) {
	PRINT_EXCEPTION("Bug?", "Inconsistent r.");
	return 1;
      }
    }

    /* Local_Neuroseg is a localized Neuroseg */
    Local_Neuroseg *locseg = New_Local_Neuroseg();
    
    locseg->seg = *seg;
    double pos[]= {10.0, 20.0, 100.0};
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);
    
    /* We can fix the bottom and change the top */
    double new_top[] = {15.0, 21.0, 101.0};
    Local_Neuroseg_Change_Top(locseg, new_top);
    
    double bottom[3], top[3];
    Local_Neuroseg_Bottom(locseg, bottom);
    if (Coordinate_3d_Distance(bottom, pos) > eps) {
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }

    Local_Neuroseg_Top(locseg, top);
    if (Coordinate_3d_Distance(top, new_top) > eps) {
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }

    /* We can fix the top and change the bottom */
    double new_bottom[] = {11.0, 24.0, 99.0};
    Local_Neuroseg_Change_Bottom(locseg, new_bottom);

    Local_Neuroseg_Bottom(locseg, bottom);
    if (Coordinate_3d_Distance(bottom, new_bottom) > eps) {
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }

    Local_Neuroseg_Top(locseg, top);
    if (Coordinate_3d_Distance(top, new_top) > eps) {
      printf("%g, %g, %g -> %g, %g, %g\n", top[0], top[1], top[2],
	     new_top[0], new_top[1], new_top[2]);
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }    
    
    Local_Neuroseg_Set_Bottom_Top(locseg, new_bottom, new_top);
    
    Local_Neuroseg_Bottom(locseg, bottom);
    if (Coordinate_3d_Distance(bottom, new_bottom) > eps) {
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }

    Local_Neuroseg_Top(locseg, top);
    if (Coordinate_3d_Distance(top, new_top) > eps) {
      printf("%g, %g, %g -> %g, %g, %g\n", top[0], top[1], top[2],
	     new_top[0], new_top[1], new_top[2]);
      PRINT_EXCEPTION("Bug?", "Inconsistent position.");
      return 1;
    }   

    /* Set the orientation by a vector*/
    Neuroseg_Set_Orientation_V(&(locseg->seg), 0.0, 0.0, 1.0);

    if (fabs(locseg->seg.theta) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected angle.");
      return 1;
    }

    /* A segment can be scaled in different ways */
    
    Local_Neuroseg *oldseg = Copy_Local_Neuroseg(locseg);

    /* inverse z-scaling */
    Local_Neuroseg_Scale_Z(locseg, 0.5);
    
    if (fabs(locseg->seg.theta) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected angle.");
      return 1;
    }

    if (fabs(Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM) - 
	     Neuroseg_Rx(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs(Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM) - 
	     Neuroseg_Ry(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs((locseg->seg.h - 1.0) / (oldseg->seg.h - 1.0) - 2.0) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected height.");
      return 1;
    }

    Local_Neuroseg_Scale_Z(locseg, 2.0);
    if (fabs(Neuroseg_Model_Height(&(locseg->seg)) - 
	     Neuroseg_Model_Height(&(oldseg->seg))) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected height.");
      return 1;
    }
    

    /* xy-scaling */
    Local_Neuroseg_Scale_XY(locseg, 0.5);
    
    if (fabs(Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM) * 2.0- 
	     Neuroseg_Rx(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs(Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM) * 2.0- 
	     Neuroseg_Ry(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs(locseg->seg.h - oldseg->seg.h) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected height.");
      return 1;
    }

    /* xy-z-scaling */
    oldseg = Copy_Local_Neuroseg(locseg);

    Local_Neuroseg_Scale(locseg, 0.5, 1.0);
    
    if (fabs(Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM) * 2.0- 
	     Neuroseg_Rx(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs(Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM) * 2.0- 
	     Neuroseg_Ry(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;      
    }

    if (fabs(locseg->seg.h - oldseg->seg.h) > eps) {
      PRINT_EXCEPTION("Bug?", "Unexpected height.");
      return 1;
    }
    
    {
      /* unit testing */
      Local_Neuroseg *locseg = New_Local_Neuroseg();
      Local_Neuroseg *oldseg = Copy_Local_Neuroseg(locseg);
      
      Neuroseg_Set_Orientation_V(&(locseg->seg), 1.0, 0.0, 0.0);

      Local_Neuroseg_Scale(locseg, 0.5, 2.0);

      if (fabs(Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM) * 2.0- 
	       Neuroseg_Rx(&(oldseg->seg), NEUROSEG_BOTTOM)) > eps) {
	PRINT_EXCEPTION("Bug?", "Unexpected size.");
	return 1;      
      }

      if (fabs(Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM)- 
	       Neuroseg_Ry(&(oldseg->seg), NEUROSEG_BOTTOM) * 2.0) > eps) {
	PRINT_EXCEPTION("Bug?", "Unexpected size.");
	return 1;      
      }
      
      if (fabs(Neuroseg_Model_Height(&(locseg->seg)) - 
	       Neuroseg_Model_Height(&(oldseg->seg)) * 0.5) > eps) {
	printf("%g -> %g\n", oldseg->seg.h, locseg->seg.h);
	PRINT_EXCEPTION("Bug?", "Unexpected height.");
	return 1;
      }      
    }

    printf(":) Testing passed.\n");
    return 0;
  }

#if 0
  Neuroseg *seg = New_Neuroseg();
  Set_Neuroseg(seg, 2.0, 2.0, 5.0, 0, 0);
  Print_Neuroseg(seg);
  
  Neuroseg *seg2 = New_Neuroseg();
  Copy_Neuroseg(seg2, seg);
  Print_Neuroseg(seg2);

  Stack *stack = Make_Stack(COLOR, 100, 100, 30);
  Zero_Stack(stack);
  const double position[3] = {50.0, 50.0, 15.0};
  
  Neuroseg_Label(stack, seg, &(position), 0);

  Write_Stack("../data/test.tif", stack);

  Field_Range range;
  field_range(seg, &range);
  print_field_range(&range);
#endif  

#if 0 /* Test the sum of filters */
  int length;
  int i;
  double r = 1.0;
  for (i = 0; i < 10; i++) {
    Set_Neuroseg(seg, 1.0, r, 12.0, 0.0, 0);
    field_range(seg, &range);
    double *filter = Neurofilter(seg, NULL, &length, &range, NULL);
  //darray_print3(filter, range.size[0], range.size[1], range.size[2]);
    Print_Neuroseg(seg);
    printf("%g\n", darray_sum(filter, length));
    free(filter);

    r += 1.0;
  }
#endif

#if 0 /* test the sum of postive values of the filters*/
  int length;
  int i, j;
  double sum = 0.0;
  double r = 1.0;
  for (i = 0; i < 10; i++) {
    Set_Neuroseg(seg, 1.0, r, 12.0, 0.0, 0);
    field_range(seg, &range);
    double *filter = Neurofilter(seg, NULL, &length, &range, NULL);
  //darray_print3(filter, range.size[0], range.size[1], range.size[2]);
    Print_Neuroseg(seg);
    for (j = 0; j < length; j++) {
      if (filter[j] > 0) {
	sum += filter[j];
      }
    }
    printf("%g\n", sum);
    free(filter);

    r += 1.0;
  }

  Free_Neuroseg(seg);
  Free_Neuroseg(seg2);
  Kill_Stack(stack);
#endif

#if 0 /* crop a stack by a neuron segment */
  double z_scale = 0.488/0.585;

  Stack *stack = Read_Stack("../data/testneurotrace.tif");
  
  Local_Neuroseg locseg;
  double position[3] = {140, 75, 107*z_scale};
  //double position[3] = {112, 136, 190*z_scale};
  //  Set_Neuroseg(&(seg), 5.0, 5.0, 12.0, -TZ_PI_2, 0.9);
  Set_Neuroseg(&(locseg.seg), 5.0, 3.0, 12.0, -TZ_PI_2, 0.9);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  Translate_Stack(stack, COLOR, 1);  
  Local_Neuroseg_Label(&locseg, stack, 0, z_scale);

  Local_Neuroseg locseg2;
  Next_Local_Neuroseg(&locseg, &locseg2);
  Local_Neuroseg_Label(&locseg2, stack, 1, z_scale);

  Write_Stack("../data/test.tif", stack);
  Kill_Stack(stack);

  //  Stack *substack = Neuroseg_Substack(&seg, position, stack);

  // Write_Stack("../data/test.tif",substack);
  //Kill_Stack(substack);
#endif

#if 0
  double z_scale = 0.488/0.585;
  Stack *stack = Read_Stack("../data/testneurotrace.tif");
  double position[3] = {140, 75, 107*z_scale};
  Local_Neuroseg locseg;
  Set_Neuroseg(&(locseg.seg), 3.0, 3.0, 10.0, -TZ_PI_2, TZ_PI_2 / 2.0);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  double scores[360];
  double psi = 0.0;
  int i;
  for (i = 0; i < 360; i++) {
    psi = (double) i / 180.0  * TZ_PI;
    Set_Neuroseg(&(locseg.seg), 3.0, 3.0, 10.0, -TZ_PI_2, psi);
    scores[i] = Local_Neuroseg_Score(&locseg, stack, z_scale, NULL);
  }
  darray_write("../data/scores.ar", scores, 360);
 
#endif

#if 0 /* test fit neuron segment */
  double z_scale = 0.488/0.585;

  Stack *stack = Read_Stack("../data/testneurotrace.tif");
  
  Local_Neuroseg locseg;
  double position[3] = {140, 75, 107*z_scale};
  //double position[3] = {90, 100, 160*z_scale};
  //double position[3] = {96, 96, 163*z_scale};

  //double position[3] = {91.7328, 98.1664, 131.632};

  Set_Neuroseg(&(locseg.seg), 3.0, 3.0, 10.0, -TZ_PI_2, TZ_PI_2 / 2.0);
  //Set_Neuroseg(&(locseg.seg), 1.0, 2.28, 5, -1.28758, 0.689579);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  Print_Local_Neuroseg(&locseg);
  printf("%g\n", Local_Neuroseg_Score(&locseg, stack, z_scale, NULL));

#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL & 
					      ~NEUROSEG_VAR_MASK_HEIGHT,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  iarray_printf(var_index, nvar, "%d");
  
  /*
  int nvar = Init_Local_Neuroseg_Var_Index(NEUROSEG_PARAM_NO_H, FALSE, 
					   var_index);
  */

  printf("%g\n", 
	 Fit_Local_Neuroseg(&locseg, stack, var_index, nvar, z_scale, NULL));
#endif

#if 0
  printf("%g\n", 
	 Local_Neuroseg_Orientation_Search(&locseg, stack, z_scale, NULL));
#endif

#if 0
  double theta = 1.0;
  double psi = 0.1;
  Geo3d_Rotate_Orientation(locseg.seg.theta, locseg.seg.psi, &theta, &psi);
  locseg.seg.theta = theta;
  locseg.seg.psi = psi;
#endif

  Print_Local_Neuroseg(&locseg);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);  
  Local_Neuroseg_Label(&locseg, canvas, 0, z_scale);
  
#if 0
  Local_Neuroseg locseg2;
  Next_Local_Neuroseg(&(locseg), &(locseg2), 1.0);
  printf("%g\n", 
	 Local_Neuroseg_Orientation_Search(&locseg2, stack, z_scale, NULL));
  Print_Local_Neuroseg(&locseg2);
  Local_Neuroseg_Label(&locseg2, canvas, 0, z_scale);
#endif

#if 0
  Set_Neuroseg(&(locseg2.seg), 2.28, 3.56, 12.0, 1.38, 3.00);
  printf("%g\n", Local_Neuroseg_Score(&locseg2, stack, z_scale, NULL));
  Local_Neuroseg_Label(&locseg2, canvas, 0, z_scale);
#endif

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0 /* test parameter search */
  double z_scale = 0.488/0.585;
  Stack *stack = Read_Stack("../data/testneurotrace.tif");
  Local_Neuroseg locseg;
  //double position[3] = {140, 75, 107*z_scale};
  double position[3] = {306.728, 49.7703, 135.295};
  //Set_Neuroseg(&(locseg.seg), 1.0, 1.0, 10.0, -TZ_PI_2, TZ_PI_2 / 2.0);
  Set_Neuroseg(&(locseg.seg), 4.38015, 5.26016, 12, 1.67419, 5.31563);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Local_Neuroseg_Label(&locseg, canvas, 0, z_scale);

  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL &
					      ~NEUROSEG_VAR_MASK_HEIGHT,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  Fit_Local_Neuroseg(&locseg, stack, var_index, nvar, z_scale, NULL);
						   
  //  Local_Neuroseg_Orientation_Search(&locseg, stack, z_scale, NULL);
  Print_Local_Neuroseg(&locseg);

  Local_Neuroseg_Label(&locseg, canvas, 0, z_scale);

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0 /* test vrml */
  Local_Neuroseg locseg;
  double position[3] = {306.666, 49.7646, 135.158};
  Set_Neuroseg(&(locseg.seg), 2.50014, 3.38015, 3.04933, 3.56951, 1.66136);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  Local_Neuroseg_Fprint_Vrml(&locseg, stdout);

  FILE *fp = fopen("../data/neuroseg.wrl", "w+");
  fprintf(fp, "#VRML V2.0 utf8\n");
  Local_Neuroseg_Fprint_Vrml(&locseg, fp);
  fclose(fp);
#endif

#if 0 /* test fit neuron segment on fly neuron*/
  double z_scale = 1.0;

  Stack *stack = Read_Stack("../data/fly_neuron_crop.tif");
  
  Local_Neuroseg locseg;
  //double position[3] = {140, 75, 107*z_scale};
  //double position[3] = {90, 100, 160*z_scale};
  //double position[3] = {96, 96, 163*z_scale};

  //double position[3] = {91.7328, 98.1664, 131.632};
  //double position[3] = {306.345, 50.0462, 138.817};
  //double position[3] = {306.794, 50.0325, 134.385};
  //double position[3] = {306.399, 49.8572, 138.748};
  //double position[3] = {306.728, 49.7703, 135.295};
  //double position[3] = {306.666, 49.7646, 135.158};
  //double position[3] = {329, 153, 149};
  double position[3] = {97, 231, 149}; //fly_neuron_crop

  Set_Neuroseg(&(locseg.seg), 1.0, 1.0, 12.0, -TZ_PI_2, 0.0, 0.0);
  //Set_Neuroseg(&(locseg.seg), 2.65681, 2.43645, 4.45459, 3.04069, 1.54034);
  //Set_Neuroseg(&(locseg.seg), 2.43645, 2.21609, 12, 0.256263, 2.28227);
  //Set_Neuroseg(&(locseg.seg), 2.43645, 2.21609, 12, 1.05116, 2.00452);
  //Set_Neuroseg(&(locseg.seg), 2.21442, 2.09664, 12, 1.55113, 1.99907);
  //Set_Neuroseg(&(locseg.seg), 2.43645, 2.21609, 12, 0.769681, 1.96536);
  //Set_Neuroseg(&(locseg.seg), 2.43645, 2.21609, 12, 0.580189, 1.90139);
  //Set_Neuroseg(&(locseg.seg), 2.43645, 2.21609, 12, 0.256263, 2.28227);
  //Set_Neuroseg(&(locseg.seg), 2.47344, 2.4878, 12, 1.66084, 5.37769);
  //Set_Neuroseg(&(locseg.seg), 4.38015, 5.26016, 12, 1.67419, 5.31563);
  //Set_Neuroseg(&(locseg.seg), 4.38015, 5.26016, 12, 1.67, 2.2);
  //Set_Neuroseg(&(locseg.seg), 2.50014, 3.38015, 3.04933, 3.56951, 1.66136);

  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

#if 0
  Local_Neuroseg sensor_seg;
  double pos_step = 1.0 - locseg.seg.r1 / locseg.seg.h;
  if (pos_step < 0.5) {
    pos_step = 0.5;
  }

  Next_Local_Neuroseg(&locseg, &sensor_seg, pos_step);

  Print_Local_Neuroseg(&sensor_seg);
  printf("%g\n", Local_Neuroseg_Score(&sensor_seg, stack, z_scale, NULL));

  Neuroseg_Change_Thickness(&(sensor_seg.seg), 1.0, 1.0);
  Neuroseg_Fit_Score fs;
  fs.options[0] = 3;
  fs.n = 1;
  Local_Neuroseg_Orientation_Search(&sensor_seg, stack, z_scale, &fs);
  Print_Local_Neuroseg(&sensor_seg);
  printf("%g\n", Local_Neuroseg_Score(&sensor_seg, stack, z_scale, &fs));

  sensor_seg.seg.theta = 1.6;
  sensor_seg.seg.psi = 2.0;
  printf("%g\n", Local_Neuroseg_Score(&sensor_seg, stack, z_scale, &fs));
#endif

#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL &
					      ~NEUROSEG_VAR_MASK_HEIGHT,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  //Fit_Local_Neuroseg(&locseg, stack, var_index, nvar, z_scale, NULL);
#endif


  
#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					      NEUROSEG_VAR_MASK_ORIENTATION,
					      NEUROPOS_VAR_MASK_ALL,
					      var_index);
  double score = Fit_Local_Neuroseg_P(&locseg, stack, var_index, nvar, 
				      z_scale, NULL);
  printf("%g\n", score);
#endif

  Stack *canvas = Translate_Stack(stack, COLOR, 0);  
  Local_Neuroseg_Label(&locseg, canvas, 0, z_scale);

#if 0
  //Local_Neuroseg locseg2;
  //Next_Local_Neuroseg(&(locseg), &(locseg2), 1.0);
  //printf("%g\n", 
  //	 Local_Neuroseg_Orientation_Search(&locseg, stack, z_scale, NULL));
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar;
  double score;

  double v[3];
  Geo3d_Orientation_Normal(3.04069, 1.54034, v, v + 1, v + 2);
  Local_Neuroseg_Position_Search(&locseg, stack, z_scale, v, 5, -0.2, NULL);

  nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL &
					      ~NEUROSEG_VAR_MASK_HEIGHT,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  Fit_Local_Neuroseg(&locseg, stack, var_index, nvar, 
				    z_scale, NULL);
  printf("%g\n", score);
  Print_Local_Neuroseg(&locseg);
  Local_Neuroseg_Label(&locseg, canvas, 1, z_scale);
#endif

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  printf("Waiting for memory leak testing ...\n");
  while(1) {

  }
#endif

#if 0 /* test Local_Neuroseg_Hit */
  Local_Neuroseg locseg;
  double position[3] = {306.794, 50.0325, 134.385};
  Set_Neuroseg(&(locseg.seg), 2.21442, 2.09664, 12, 1.55113, 1.99907);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);
  Local_Neuroseg_Center(&locseg, position);

  printf("%d\n", Local_Neuroseg_Hit_Test(&locseg, position[0], 
					 position[1], position[2]));
#endif


#if 0
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, 5.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			     50.0, 50.0, 5.0);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Ellipse_Field(locne, 1.0, NULL);

  Stack *stack = Make_Stack(FLOAT32, 100, 100, 10);
  Zero_Stack(stack);
  Geo3d_Scalar_Field_Draw_Stack(field, stack, NULL, NULL);
  Stack *stack2 = Scale_Float_Stack((float *) stack->array, stack->width, 
				    stack->height, stack->depth, GREY);

  /*
  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));

  Set_Local_Neuroseg_Ellipse(locne, 8.0, 8.0, 1.0, 1.0, 1.0, 0.0, 0.0,
			     50.0, 50.0, 50.0);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;
  printf("%g\n", Local_Neuroseg_Ellipse_Score(locne, stack, 1.0, NULL));

  
  Bitmask_t varmask = NEUROSEG_ELLIPSE_VAR_MASK_R 
    | NEUROSEG_ELLIPSE_VAR_MASK_ALPHA 
    | NEUROSEG_ELLIPSE_VAR_MASK_ORIENTATION;

  int var_index[NEUROSEG_ELLIPSE_NPARAM];
  int nvar = Local_Neuroseg_Ellipse_Var_Mask_To_Index(varmask,
						      NEUROPOS_VAR_MASK_NONE,
						      var_index);

  printf("%d\n", nvar);

  Print_Local_Neuroseg_Ellipse(locne);
  printf("%g\n", Fit_Local_Neuroseg_Ellipse(locne, stack, var_index, nvar, 1.0, NULL));
  
  Print_Local_Neuroseg_Ellipse(locne);
  */
  Write_Stack("../data/test.tif", stack2);
  
#endif

#if 0 /* test Neuroseg_Field */
  Stack *stack = Make_Stack(GREY, 100, 100, 100);
  Zero_Stack(stack);
  
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  double r = 3.0;
  Set_Neuroseg(&(locseg->seg), r, 0.0, 12, 0.0, 0.0, 0, 0, 2.5);
  Set_Neuropos(locseg->pos, 50, 50, 50);


  //  int length = 10000;

  /*
  coordinate_3d_t *coord = (coordinate_3d_t *) 
    malloc(sizeof(coordinate_3d_t) * length * 3);
  double *value = (double *) malloc(sizeof(double) * length);
  */

  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);

  int length = field->size;
  printf("%d\n", length);

  printf("%g\n", darray_sum(field->values, length));


  //darray_scale(field->values, length, 0, 255);
  //darray_print2(value, length, 1);
  
  int i;
  for (i = 0; i < length; i++) {
    if (field->values[i] < 0) {
      field->values[i] = 32.0;
    } else {
      field->values[i] = 255.0;
    }
  }
  Stack_Draw_Points(stack, field->points, field->values, length, 1);

  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
#endif

#if 0 /* test Local_Neuroseg_Score2 */
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  double z_scale = 1.0;
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuroseg(&(locseg->seg), 3, 3, 12, TZ_PI_2, 0);
  Set_Neuropos(locseg->pos, 157, 151, 149);
  
#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL & 
					      ~NEUROSEG_VAR_MASK_HEIGHT,
					      NEUROPOS_VAR_MASK_ALL,
					      var_index);

  Neuroseg_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 4;

  printf("%g\n", Fit_Local_Neuroseg(locseg, stack, var_index, nvar, z_scale, &fs));
#endif


  Print_Local_Neuroseg(locseg);
#if 0
  printf("%g\n", Local_Neuroseg_Orientation_Search(locseg,stack, z_scale, NULL));
#endif

#if 0
  nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL & 
					  ~NEUROSEG_VAR_MASK_HEIGHT,
					  NEUROPOS_VAR_MASK_NONE,
					  var_index);
  printf("%g\n", Fit_Local_Neuroseg(locseg, stack, var_index, nvar, z_scale, NULL));
  Print_Local_Neuroseg(locseg);
#endif

  Stack *canvas = Translate_Stack(stack, COLOR, 0);  
  Local_Neuroseg_Label(locseg, canvas, 0, z_scale);

  Write_Stack("../data/test.tif", canvas);

  Kill_Stack(stack);
  Kill_Stack(canvas);
#endif

#if 0 /* test variable settng */
  printf("Auto testing ...\n");
  Local_Neuroseg *locseg = New_Local_Neuroseg();

  if (LOCAL_NEUROSEG_NPARAM != NEUROSEG_NPARAM + NEUROPOS_NPARAM) {
    TRACE("Possible bug in LOCAL_NEUROSEG_NPARAM.");
  }

  double *var[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var(locseg, var);
  if (nvar != LOCAL_NEUROSEG_NPARAM)  {
    TRACE("Possible bug in Local_Neuroseg_Var.");
  }

  int i;
  for (i = 0; i < nvar; i++) {
    if (*var[i] < 0.0) {
      printf("%d, %g\n", i, *var[i]);
      TRACE("Possible bug in Local_Neuroseg_Var.");
    }
  }
  printf("Done.\n");

  Print_Local_Neuroseg(locseg);

  Free_Local_Neuroseg(locseg);

#endif

#if 0 /* test neuroseg plane */
  double z_scale = 1.0;

  Stack *stack = Read_Stack("../data/fly_neuron2.tif");
  
  Stack *stack2 = Read_Stack("../data/fly_neuron2.tif");

  Local_Neuroseg locseg;  

  //double position[3] = {328, 152, 149*z_scale};

  //double position[3] = {319, 168, 149 * z_scale};

  //double position[3] = {292, 287, 117 * z_scale};
  //double position[3] = {363, 116, 159};

  //double position[3] = {143, 381, 51}; //fly_neuron2
  double position[3] = {240, 149, 134}; //fly_neuron2
  //double position[3] = {261, 136, 141}; //fly_neuron2

  //double position[3] = {270, 237, 62}; //mouse_neuron

  //double position[3] = {177, 279, 77}; //mouse_neuron
  //double position[3] = {314, 185, 89}; //mouse_neuron
  //double position[3] = {207, 221, 113}; //mouse_neuron
  //double position[3] = {244, 405, 84}; //mouse_neuron

  Set_Neuroseg(&(locseg.seg), 5.0, 5.0, 20, TZ_PI_2, TZ_PI_2, 0.0);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);

  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					      NEUROSEG_VAR_MASK_ORIENTATION,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  //double score = 
  Fit_Local_Neuroseg_P(&locseg, stack, var_index, nvar, z_scale, NULL);
  
  Print_Local_Neuroseg(&locseg);
  //printf("%g\n", score);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

#  if 0
  Local_Neuroseg_Label(&locseg, canvas, 0, z_scale);
  Write_Stack("../data/test.tif", canvas);
  return 0;
#  endif

  Stack *brush = Make_Stack(GREY, stack->width, stack->height, 
			    stack->depth);

#if 0
  Local_Neuroseg_Ellipse *locnp = New_Local_Neuroseg_Ellipse();
  int z;
  double coef[] = {0.0, 20000.0};
  double range[] = {0.0, 255.0};

  Bitmask_t varmask = NEUROSEG_ELLIPSE_VAR_MASK_R | 
    NEUROSEG_ELLIPSE_VAR_MASK_OFFSET;
  Print_Bitmask(varmask);

  nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(varmask,
					   NEUROPOS_VAR_MASK_NONE,
					   var_index);

  
  //return 1;


  int idx;
  Geo3d_Circle *bottom = NULL;
  Geo3d_Circle *top = NULL;

  FILE *vrml_fp = fopen("../data/fly_neuron/soma.wrl", "w");
  Vrml_Head_Fprint(vrml_fp, "2.0", "utf8");
  int n = 0;
  double xx[100];
  double yy[100];
  double zz[100];

  for (z = 0; z <= 30; z += 1) {
    Local_Neuroseg_To_Ellipse(&locseg, z, locnp);
    Fit_Local_Neuroseg_Ellipse(locnp, stack2, var_index, nvar, z_scale, NULL);
    xx[n] = locnp->np.offset_x;
    yy[n] = locnp->np.offset_y;
    zz[n] = z;
    n++;
    Geo3d_Scalar_Field* field = 
      Local_Neuroseg_Ellipse_Field(locnp, 0.8, NULL);
    Geo3d_Scalar_Field_Draw_Stack(field, brush, coef, range);
    printf("total: %g\n", darray_max(field->values, field->size, &idx));
    Kill_Geo3d_Scalar_Field(field);
    Print_Local_Neuroseg_Ellipse(locnp);

    if (bottom != NULL) {
      Copy_Geo3d_Circle(bottom, top);
      //top = Local_Neuroseg_Plane_To_Geo3d_Circle(locnp, top);
    } else {
      //bottom = Local_Neuroseg_E_To_Geo3d_Circle(locnp, NULL);
    }

    if ((bottom != NULL) && (top != NULL)) {
      int indent = 2;
      Vrml_Circle_Platform_Fprint(vrml_fp, bottom, top, indent);
    }
  }
 
  darray_write("../data/locne_x.bn", xx, n);
  darray_write("../data/locne_y.bn", yy, n);
  darray_write("../data/locne_z.bn", zz, n);
  

  fclose(vrml_fp);

  Stack_Label_Color(canvas, brush, 2.0, 1.0, brush);

  Delete_Local_Neuroseg_Ellipse(locnp);
#endif
  
  Write_Stack("../data/test.tif", canvas);
  
  Kill_Stack(brush);
  Kill_Stack(stack);
  Kill_Stack(canvas);

#endif

#if 0 /* fit test */
  /* Read stack */
  Stack *stack = Read_Stack("../data/fly_neuron_crop.tif");
  double z_scale = 1.0;

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  
  Set_Local_Neuroseg(locseg, 2, 2, 12, -TZ_PI_2, 0.0, 0.0, 
		     92, 227, 149);

  /* fit */
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index
    (NEUROSEG_VAR_MASK_R |
     NEUROSEG_VAR_MASK_ORIENTATION,
     NEUROPOS_VAR_MASK_ALL, var_index);

  tic();
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, NULL);
  printf("time passed: %llu\n", toc());

  Print_Local_Neuroseg(locseg);

#if 0
  /* Generate field */
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);
  Delete_Local_Neuroseg(locseg);

  /* Draw it in a stack */
  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};
  Geo3d_Scalar_Field_Draw_Stack(field, label, coef, range);

  /* Turn the stack to GREY type */
  Translate_Stack(label, GREY, 1);

  
  
  /* Label the canvas */
  Stack_Label_Color(stack, label, 5.0, 1.0, label);
#endif
  
  /* Make canvas */
  Translate_Stack(stack, COLOR, 1);
  Local_Neuroseg_Label(locseg, stack, 0, z_scale);

  /* Save the stack */
  Write_Stack("../data/test.tif", stack);

  /* clean up */
  //Kill_Geo3d_Scalar_Field(field);
  Kill_Stack(stack);
  //Kill_Stack(label);
  
  return 0;
#endif

#if 0
  double z_scale = 1.0;

  Stack *stack = Read_Stack("../data/mouse_neuron.tif");
  
  Local_Neuroseg locseg;  

  //double position[3] = {363, 116, 159};
  //double position[3] = {406, 316, 68};
  //double position[3] = {387, 220, 194};
  //double position[3] = {236, 396, 143}; //fly_neuron2
  //double position[3] = {234, 149, 134}; //fly_neuron2
  //double position[3] = {261, 136, 141}; //fly_neuron2

  //double position[3] = {269, 220, 71}; //mouse_neuron
  double position[3] = {236, 150, 136}; //mouse_neuron

  Set_Neuroseg(&(locseg.seg), 5.0, 5.0, 20, -TZ_PI_2, 0, 0.0);
  Set_Neuropos(locseg.pos, position[0], position[1], position[2]);



  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION,
					      NEUROPOS_VAR_MASK_NONE,
					      var_index);
  //Fit_Local_Neuroseg_P(&locseg, stack, var_index, nvar, z_scale, NULL);

  //  Local_Neuroseg_Plane *locnp = New_Local_Neuroseg_Plane();
  // Local_Neuroseg_To_Plane(&locseg, 0.0, locnp);

  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Local_Neuroseg_To_Ellipse(&locseg, 0.0, locne);
  
  Locne_Chain *chain = Locne_Chain_Trace_Init(stack, z_scale, locne);
  
  Trace_Locne(stack, z_scale, 100, chain);

  Print_Locne_Chain(chain);

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
  //Translate_Stack(label, GREY, 1);
  label = Scale_Float_Stack((float *) label->array, label->width, label->height,
			    label->depth, GREY);

  //Kill_Stack(stack);
  //stack = Read_Stack("../data/fly_neuron2.tif");
  /* Make canvas */
  Translate_Stack(stack, COLOR, 1);
  
  /* Label the canvas */
  Stack_Label_Color(stack, label, 5.0, 1.0, label);

  /* Save the stack */
  Write_Stack("../data/test.tif", stack);  
#  endif

#endif


#if 0 /* test orientation search */
  double z_scale = 1.0;

  Stack *stack = Read_Stack("../data/fly_neuron_crop.tif");

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  
  Set_Local_Neuroseg(locseg, 5.0, 1.0, 12.0, TZ_PI_2, 0.0, 0.0, 0, 0, 0);

  double pos[3] = {97, 234, 149};
  //double pos[3] = {115, 114, 144};
  //double pos[3] = {231, 159, 160};
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;
  Local_Neuroseg_Orientation_Search_C(locseg, stack, 1.0, &fs);
  
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION |
					       NEUROSEG_VAR_MASK_R,
					       NEUROPOS_VAR_MASK_ALL,
					       var_index);

   int var_link[LOCAL_NEUROSEG_NPARAM];
   int i;
   for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
     var_link[i] = 0;
   }
   //Variable_Set_Add_Link(var_link, 0, 1);

   fs.options[0] = 7;
   double score = Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, var_link,
        z_scale, &fs);
  /*
  nvar = Local_Neuroseg_Var_Mask_To_Index(
					  NEUROSEG_VAR_MASK_R,
					  NEUROPOS_VAR_MASK_NONE,
					  var_index);
  Variable_Set_Add_Link(var_link, 0, 1);
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, var_link,
		       z_scale, &fs);
  */  
  Print_Local_Neuroseg(locseg);
  
  double center[3];
  Local_Neuroseg_Center(locseg, center);

  darray_print2(center, 3, 1);
  /*
  double r = 0.5;
  for (r = 0.5; r < 5; r+=0.25) {
    locseg->seg.r1 = r;
    locseg->seg.r2 = r;
    printf("%g, %g\n", r, Local_Neuroseg_Score_P(locseg, stack, z_scale, NULL));
  }
  */

  double theta = 0.0;
  for (; theta < 3.0; theta+=0.05) {
    locseg->seg.theta = theta;
    printf("%g, %g\n", theta, Local_Neuroseg_Score_P(locseg, stack, z_scale, NULL));
  }

  Translate_Stack(stack, COLOR, 1);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);
  darray_scale(field->values, field->size, 1.0, 254.0);
  //Geo3d_Scalar_Field_Label_Stack(field, stack);
  Local_Neuroseg_Label(locseg, stack, 1, 1.0);
  Write_Stack("../data/test.tif", stack);
  
#endif

#if 0 /* test orientation search 2*/
  double z_scale = 1.0;

  Stack *stack = Read_Stack("../data/mouse_neuron3_org.tif");

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  
  Set_Local_Neuroseg(locseg, 3.0, 3.0, 12.0, TZ_PI_2, 0.0, 0.0, 0, 0, 0);

  double pos[3] = {199, 324, 95};
  //double pos[3] = {115, 114, 144};
  //double pos[3] = {231, 159, 160};
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;
  Local_Neuroseg_Orientation_Search_B(locseg, stack, 1.0, &fs);

  Flip_Local_Neuroseg(locseg);
  Local_Neuroseg_Axis_Position(locseg, pos, locseg->seg.h * 1.4);
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

  Local_Neuroseg_Orientation_Search_B(locseg, stack, 1.0, &fs);

  Stretch_Stack_Value_Q(stack, 0.999);
  Translate_Stack(stack, COLOR, 1);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);
  darray_scale(field->values, field->size, 1.0, 254.0);
  //Geo3d_Scalar_Field_Label_Stack(field, stack);
  Local_Neuroseg_Label(locseg, stack, 1, 1.0);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(277);
  Neuroseg_Slice_Field(field->points, field->values, &(field->size));
  Print_Geo3d_Scalar_Field(field);
  darray_write("../data/point.ar",
	       Coordinate_3d_Double_Array(field->points),
	       field->size * 3);
  darray_write("../data/value.ar", field->values, field->size);
  
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  locseg->seg.theta = TZ_PI / 4.0;
  Print_Local_Neuroseg(locseg);

  Field_Range range;
  field_range(&(locseg->seg), &range, 1.0);

  print_field_range(&range);

  double cpos[3];
  Local_Neuroseg_Center(locseg, cpos);
  darray_print2(cpos, 3, 1);

  double bpos[3];
  Local_Neuroseg_Bottom(locseg, bpos);
  darray_print2(bpos, 3, 1);

  double tpos[3];
  Local_Neuroseg_Top(locseg, tpos);
  darray_print2(tpos, 3, 1);
    
  printf("%g\n", sqrt(Geo3d_Dist_Sqr(tpos[0], tpos[1], tpos[2], bpos[0],
				     bpos[1], bpos[2])));
#endif

#if 0
  Stack *canvas = Read_Stack("../data/mouse_neuron3_org.tif");
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  //Set_Local_Neuroseg(locseg, 14.301, 14.301, 12, 17.2585, 3.45688, 0, 0.0, 1.0, 128.911, 282.286, 29.3167);
  //Set_Local_Neuroseg(locseg, 2.00001, 1.31565, 11, 4.54934, 5.57307, 0,
  //		     129.186, 263.38, 29.7895);

  //Set_Local_Neuroseg(locseg, 1.27572, 1.27572, 11, 23.4876, 7.20091, 0, 0.0, 1.0, 131.369, 267.901, 28.6368);
    //Flip_Local_Neuroseg(locseg);

  Set_Local_Neuroseg(locseg, 1.27572, 0.0, 11, 0.0, 0.0, 0, 0.0, 1.0, 115, 316, 30);
  //Local_Neuroseg_Optimize(locseg, canvas, 1.0);
  /*
  int var_index[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index((NEUROSEG_VAR_MASK_R1 |
				      NEUROSEG_VAR_MASK_ORIENTATION) &
				     ~NEUROSEG_VAR_MASK_HEIGHT &
				     ~NEUROSEG_VAR_MASK_CURVATURE,
				     NEUROSEG_VAR_MASK_NONE, var_index);

  int var_link[LOCAL_NEUROSEG_NPARAM];
  int k;
  for (k = 0; k < LOCAL_NEUROSEG_NPARAM; k++) {
    var_link[k] = 0;
  }
  Variable_Set_Add_Link(var_link, 0, 1);  

  Fit_Local_Neuroseg_P(locseg, canvas, var_index, nvar, var_link, 1.0, NULL);

  
  Local_Neuroseg_Radius_Search(locseg, canvas, 1.0, 1.0, 10.0, 1.0,
			       NULL);
  */
  /*  
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R1,
				     NEUROSEG_VAR_MASK_NONE, var_index);

  Fit_Local_Neuroseg_P(locseg, canvas, var_index, nvar, var_link, 1.0, NULL);
  */
  Print_Local_Neuroseg(locseg);

  Neuroseg_Fit_Score ort_fs;
  ort_fs.n = 1;		
  ort_fs.options[0] = STACK_FIT_DOT;
  /*
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  
  Geo3d_Rotate_Orientation(0.0, 0.4, &(locseg->seg.theta), &(locseg->seg.psi));
  Set_Neuroseg_Position(locseg, bottom, NEUROSEG_BOTTOM); 

  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_Sp(locseg, 1.0, NULL);
  double score = Geo3d_Scalar_Field_Stack_Score(field, canvas, 1.0, &ort_fs);

  printf("%g\n", score);

  Local_Neuroseg_Orientation_Search_B(locseg, canvas, 1.0, &ort_fs);
  Print_Local_Neuroseg(locseg);
  */  
  Translate_Stack(canvas, GREY, 1);
  Stretch_Stack_Value_Q(canvas, 0.999);  
  Translate_Stack(canvas, COLOR, 1);

  Local_Neuroseg_Label(locseg, canvas, 0, 1.0);
  /*
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();
  Next_Local_Neuroseg(locseg, locseg2, 0.7);
  Local_Neuroseg_Label(locseg2, canvas, 1, 1.0);
  */
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 1.27572, 1.27572, 11, 23.4876, 7.20091, 0, 0.0, 1.0, 131.369, 267.901, 28.6368);
  double pos[3] = {131.369, 267.901, 28.6368};
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

  double center[3];
  Local_Neuroseg_Bottom(locseg, center);
  darray_print2(center, 3, 1);

  Geo3d_Circle sc;
  Local_Neuroseg_To_Geo3d_Circle(locseg, &sc, NEUROSEG_CENTER);
  Print_Geo3d_Circle(&sc);
#endif

#if 0
  Print_Bitmask(NEUROSEG_VAR_MASK_R1);
  Print_Bitmask(NEUROSEG_VAR_MASK_R2);
  Print_Bitmask(NEUROSEG_VAR_MASK_THETA);
  Print_Bitmask(NEUROSEG_VAR_MASK_PSI);
  Print_Bitmask(NEUROSEG_VAR_MASK_HEIGHT);
  Print_Bitmask(NEUROSEG_VAR_MASK_CURVATURE);
  Print_Bitmask(NEUROSEG_VAR_MASK_ALPHA);
  Print_Bitmask(NEUROSEG_VAR_MASK_SCALE);

  Print_Bitmask(NEUROSEG_VAR_MASK_R);
  Print_Bitmask(NEUROSEG_VAR_MASK_R1 | NEUROSEG_VAR_MASK_R2);
  Print_Bitmask(NEUROSEG_VAR_MASK_ORIENTATION);
  Print_Bitmask(NEUROSEG_VAR_MASK_THETA | NEUROSEG_VAR_MASK_PSI);
  Print_Bitmask(NEUROSEG_VAR_MASK_ALL);
  Print_Bitmask(NEUROSEG_VAR_MASK_NONE);
#endif

#if 0
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, 5.0, 10.0, 1.0, 3.0, 1.0, 2.0, 10.0, 10.0,
			     1.0, 2.0);
  coordinate_3d_t coord[20];
  Local_Neuroseg_Ellipse_Points(locne, 20, 1.5, coord);
  printf("%lu\n", sizeof(coord) / sizeof(double));
  darray_write("../data/test.bn", Coordinate_3d_Double_Array(coord), 
	       sizeof(coord) / sizeof(double));

  int i;
  coordinate_3d_t center;
  Local_Neuroseg_Ellipse_Center(locne, center);

  Print_Coordinate_3d(center);

  for (i = 0; i < 20; i++) {
    coord[i][0] -= center[0];
    coord[i][1] -= center[1];
    coord[i][2] -= center[2];
    coord[i][0] *= 2.0;
    coord[i][1] *= 2.0;
    coord[i][2] *= 2.0;
    printf("%g\n", Neuroseg_Ellipse_Vector_Angle(&(locne->np), coord[i]));
  }
#endif

#if 0
  Local_Neuroseg_Ellipse locne[3];
  Set_Local_Neuroseg_Ellipse(locne, 5.0, 5.0, 1.0, 3.0, 1.0, 2.0, 1.0, 10.0,
			     1.0, 2.0);
  Set_Local_Neuroseg_Ellipse(locne + 1, 5.0, 10.0, 1.0, 3.0, 1.0, 2.0, 10.0, 
			     10.0, 1.0, 20.0);
  Set_Local_Neuroseg_Ellipse(locne + 2, 5.0, 10.0, 1.5, 3.0, 1.0, 2.0, 10.0, 
			     10.0, 1.0, 30.0);
  
  coordinate_3d_t coord[60];
  Local_Neuroseg_Ellipse_Array_Points(locne, 3, 20, coord);
  printf("%lu\n", sizeof(coord) / sizeof(double));
  darray_write("../data/test.bn", Coordinate_3d_Double_Array(coord), 
	       sizeof(coord) / sizeof(double));
#endif

#if 0
  Neuroseg *seg = New_Neuroseg();
  Print_Neuroseg(seg);
  Set_Neuroseg(seg, 2.0, -0.2, 11.0, 0.6, 0.1, 0.0, 0.0, 2.0);
  Print_Neuroseg(seg);
  
  printf("coef: %g\n", NEUROSEG_COEF(seg));
  printf("rz: %g\n", NEUROSEG_RADIUS(seg, 1.0));
  printf("r1: %g\n", NEUROSEG_R1(seg));
  printf("r2: %g\n", NEUROSEG_R2(seg));
  printf("rc: %g\n", NEUROSEG_RC(seg));
  printf("ra: %g\n", NEUROSEG_RA(seg));
  printf("rb: %g\n", NEUROSEG_RB(seg));
  printf("ball range: %g\n", Neuroseg_Ball_Range(seg));
  printf("Z range: %g\n", Neuroseg_Z_Range(seg));

  double t;
  int i = 0;
  double d[100];
  for (t = 0; t < 3.14; t += 0.1) {
    Set_Neuroseg(seg, 2.0, -0.2, 11.0, t, 0.1, 0.0, 0.0, 2.0);
    d[i++] = Neuroseg_Z_Range(seg);
  }
  darray_write("../data/test.bn", d, i);

  Reset_Neuroseg(seg);
  Print_Neuroseg(seg);

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Print_Local_Neuroseg(locseg);
  Set_Local_Neuroseg(locseg,
		     1.0, -0.2, 11.0, 0.0, 0.1, 0.0, 0.0, 2.0, 10.0, 11.0, 12.0);
  Print_Local_Neuroseg(locseg);
  printf("%d\n", LOCAL_NEUROSEG_NPARAM);

  double var[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Param_Array(locseg, 1.5, var);
  printf("%d\n", nvar);

  for (i = 0; i < nvar; i++) {
    printf("%g\n", var[i]);
  }

  Default_Local_Neuroseg(locseg);
  Print_Local_Neuroseg(locseg);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg,
		     1.0, -0.2, 11.0, 0.0, 0.1, 0.0, 0.0, 2.0, 10.0, 11.0, 12.0);

  Print_Local_Neuroseg(locseg);

  Local_Neuroseg_Chop(locseg, -0.5);
  Print_Local_Neuroseg(locseg);

  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Local_Neuroseg_To_Ellipse(locseg, locne, NEUROSEG_CENTER);
  Print_Local_Neuroseg_Ellipse(locne);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg,
		     3.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0);
  
  Print_Local_Neuroseg(locseg);

  printf("%g\n", Local_Neuroseg_Point_Dist(locseg, 3.0, 3.0, 0.0, 1.0));

  return 1;

  Neuroseg_Swell(&(locseg->seg), 1.5, 0.0, 3.0);
  Print_Local_Neuroseg(locseg);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 8, 0, 11, 0.5, 0.5, 0, 0.0, 2.0, 20, 20, 20);
  Stack *stack = Make_Stack(1, 50, 50, 50);
  Zero_Stack(stack);
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = Make_Stack(1, 50, 50, 50);
  ws->sratio = 1.5;
  ws->sdiff = 0.0;
  int i;
  for (i = 0; i < 50  * 50 * 50; i++) {
    ws->signal->array[i] = i % 255;
  }
  ws->option = 10;
  ws->value = 255;
  Local_Neuroseg_Label_W(locseg, stack, 1.0, ws);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Local_Neuroseg *locseg = Local_Neuroseg_Import_Xml("../data/locseg.xml", 
						     NULL);
  Print_Local_Neuroseg(locseg);
  printf("%d\n", Local_Neuroseg_Hit_Test(locseg, 102, 100, 12));
#endif

#if 0 /* test Fit_Local_Neuroseg_W */
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  double z_scale = 1.0;
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuroseg(&(locseg->seg), 3, 0, 11, TZ_PI_2, 0, 0, 0, 1);
  Set_Neuropos(locseg->pos, 297, 229, 38);
  
  Locseg_Fit_Workspace *ws = New_Locseg_Fit_Workspace();

  
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_DOT;

  ws->sws->field_func = NULL;
  
  ws->nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
				     NEUROSEG_VAR_MASK_C |
				     NEUROSEG_VAR_MASK_ORIENTATION |
				     NEUROSEG_VAR_MASK_SCALE,
				     NEUROPOS_VAR_MASK_NONE,
				     ws->var_index);
  
  Locseg_Fit_Workspace_Disable_Cone(ws);
  Locseg_Fit_Workspace_Enable_Cone(ws);

  Local_Neuroseg_Optimize_W(locseg, stack, z_scale, 0, ws);

  //printf("%g\n", Fit_Local_Neuroseg_W(locseg, stack, z_scale, ws));

  /*
  printf("%g\n", Fit_Local_Neuroseg_P(locseg, stack, ws->var_index,
				      ws->nvar, ws->var_link, 1.0, &fs));
  */
  Print_Local_Neuroseg(locseg);

  //return 1;

  Translate_Stack(stack, COLOR, 1);
  Stack_Draw_Workspace *sdw = New_Stack_Draw_Workspace();
  Local_Neuroseg_Draw_Stack(locseg, stack, sdw);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Local_Neuroseg *locseg = Local_Neuroseg_Import_Xml("../data/locseg.xml", 
						     NULL);
  Print_Local_Neuroseg(locseg);

  printf("coef: %g\n", NEUROSEG_COEF(&(locseg->seg)));
  printf("rz: %g\n", NEUROSEG_RADIUS(&(locseg->seg), 1.0));
  printf("r1: %g\n", NEUROSEG_R1(&(locseg->seg)));
  printf("r2: %g\n", NEUROSEG_R2(&(locseg->seg)));
  printf("rc: %g\n", NEUROSEG_RC(&(locseg->seg)));
  printf("ra: %g\n", NEUROSEG_RA(&(locseg->seg)));
  printf("rb: %g\n", NEUROSEG_RB(&(locseg->seg)));
  printf("ball range: %g\n", Neuroseg_Ball_Range(&(locseg->seg)));
  printf("Z range: %g\n", Neuroseg_Z_Range(&(locseg->seg)));

#endif

#if 0
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  Print_Locseg_Label_Workspace(ws);

  Locseg_Score_Workspace *sws = New_Locseg_Score_Workspace();
  Print_Locseg_Score_Workspace(sws);

  Locseg_Fit_Workspace *fws = New_Locseg_Fit_Workspace();
  Locseg_Fit_Workspace_Enable_Cone(fws);
  Print_Locseg_Fit_Workspace(fws);
#endif

#if 0 /* test neuroseg spatial relation */
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];
  
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
						      NULL);

  Print_Local_Neuroseg(locseg1);

  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);

  printf("%g\n", Geo3d_Dist(bottom1[0], bottom1[1], bottom1[2],
			    top1[0], top1[1], top1[2]));
  
  Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg1, NULL, 0.5);
  locseg2->pos[0] -= 1.0;
  locseg2->pos[1] -= 5.0;
  Local_Neuroseg_Rotate(locseg2, 0.5, 0.5);
  
  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);

  printf("%g\n", Geo3d_Dist(bottom1[0], bottom1[1], bottom1[2],
			    bottom2[0], bottom2[1], bottom2[2]));

  FILE *fp = fopen("../data/test.swc", "w");

  int id = 1;
  Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);
  id += 2;

  Neuroseg_Normalize_Orientation(&(locseg2->seg));
  Print_Local_Neuroseg(locseg2);

  Local_Neuroseg_Swc_Fprint_T(fp, locseg2, id, -1, 3);
  id += 2;

  double t1, t2;
  int cond;
  double dist = Geo3d_Lineseg_Lineseg_Dist(bottom1, top1, bottom2, top2,
					  &t1, &t2, &cond);
  printf("%g, %g, %g, %d\n", dist, t1, t2, cond);

  
  Geo3d_Circle *circle = Local_Neuroseg_To_Circle_T(locseg1, t1, NULL);

  Print_Geo3d_Circle(circle);

  Geo3d_Circle_Swc_Fprint_T(fp, circle, id, - 1, 4, 1.0);
  id += 1;

  Local_Neuroseg_To_Circle_T(locseg2, t2, circle);
  Geo3d_Circle_Swc_Fprint_T(fp, circle, id, id - 1, 4, 1.0);

  fclose(fp);
#endif

#if 0
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
						      NULL);
  locseg1->seg.theta = TZ_PI_2;
  locseg1->seg.psi = 0.0;

  Print_Local_Neuroseg(locseg1);

  int id = 1;

  FILE *fp = fopen("../data/test.swc", "w");

  Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);
  
  
  int i;
  for (i = 0; i < 10; i++) {
    Local_Neuroseg_Rotate(locseg1, 0.0, 0.1);
    id += 2;
    Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 3);
  }
  

  /*
  double theta, psi, step;
  double theta_range = TZ_PI * 0.75;
  Local_Neuroseg *tmp_locseg = Copy_Local_Neuroseg(locseg1);

  for (theta = 0.1; theta <= theta_range; theta += 0.2) {
    if (theta < 0.005) {
      step = TZ_2PI * 2;
    } else {
      step = 2.0 / locseg1->seg.h / sin(theta);
    }
     
    for (psi = 0.0; psi < TZ_2PI; psi += step) {
      tmp_locseg->seg.theta = theta;
      tmp_locseg->seg.psi = psi;
      //Set_Xz_Orientation(new_ort, theta, psi);
      Geo3d_Rotate_Orientation(locseg1->seg.theta, locseg1->seg.psi,
			       &(tmp_locseg->seg.theta), 
			       &(tmp_locseg->seg.psi));
      Local_Neuroseg_Swc_Fprint_T(fp, tmp_locseg, id, -1, 3);
    }
  }
  */

  fclose(fp);
#endif

#if 0
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
						      NULL);
  int id = 1;

  FILE *fp = fopen("../data/test.swc", "w");

  Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);
  locseg1->pos[0] += 5;
  locseg1->pos[1] += 5;
  locseg1->pos[2] += 5;

  id += 2;
  Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);

  fclose(fp);
#endif

#if 0
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, 3, 4, 0.5, 1.0, 0.0, 0, 0, 0, 0, 
			     0);

  coordinate_3d_t coord[20];

  //Local_Neuroseg_Ellipse_Scale_Z(locne, 0.5);

  Local_Neuroseg_Ellipse_Points(locne, 20, 0, coord);

  FILE *fp = fopen("../data/test.swc", "w");

  int i;
  for (i = 0; i < 20; i++) {
    Swc_Node node;
    node.id = i + 1;
    node.type = 2;
    node.d = 1;
    node.x = coord[i][0];
    node.y = coord[i][1];
    node.z = coord[i][2];
    node.parent_id = -1;
    Swc_Node_Fprint(fp, &node);
  }

  fclose(fp);

#endif

#if 0
  
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
						      NULL);
						      
  //locseg1->seg.c = 0.0;
  
  //Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  locseg1->seg.r1 = 3.0;

  Print_Local_Neuroseg(locseg1);

  int id = 1;

  FILE *fp = fopen("../data/test.swc", "w");

  Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);

  id += 2;

  double pt[3];
  
  pt[0] = locseg1->pos[0]-5.0;
  pt[1] = locseg1->pos[1] -5.0;
  pt[2] = locseg1->pos[2] - 5.0;
  
  printf("%g\n", Local_Neuroseg_Point_Dist_S(locseg1, pt[0], pt[1], pt[2],
					     NULL));
  
  Swc_Node node;
  node.id = id;
  node.type = 3;
  
  double cpt[3];
  node.d = Local_Neuroseg_Point_Dist_S(locseg1, pt[0], pt[1], pt[2],
				       cpt);

  node.x = pt[0];
  node.y = pt[1];
  node.z = pt[2];
  node.parent_id = -1;

  Swc_Node_Fprint(fp, &node);
  
  node.x = cpt[0];
  node.y = cpt[1];
  node.z = cpt[2];
  node.d = 0.5;
  node.parent_id = id;
  node.id = id + 1;
  Swc_Node_Fprint(fp, &node);
  
  fclose(fp);
#endif

#if 0
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
						      NULL);
  Local_Neuroseg *locseg2 = Copy_Local_Neuroseg(locseg1);
  locseg2->pos[0] += 5.0;
  locseg2->pos[1] += 5.0;
  locseg2->pos[2] += 5.0;

  printf("%g\n", Local_Neuroseg_Dist2(locseg1, locseg2));
#endif

#if 0
  Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  Print_Local_Neuroseg(locseg1);

  
  double pt[3] = {1.0, 1.0, 5.0};
  double tmp_pt[3];
  printf("%g\n", Local_Neuroseg_Point_Dist_S(locseg1, pt[0], pt[1], pt[2],
					     tmp_pt));
  darray_print2(tmp_pt, 3, 1);
#endif

#if 0
  Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg1, NULL, 0.5);
  Flip_Local_Neuroseg(locseg2);
  printf("%d\n", Local_Neuroseg_Tangent_Compare(locseg1, locseg2));
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Stack *stack = Make_Stack(GREY, 10, 10, 10);
  Zero_Stack(stack);
  printf("%d\n", Local_Neuroseg_Has_Stack_Value(locseg, stack, 1.0, 1.0));
  Local_Neuroseg_Label_G(locseg, stack, 0, 2, 1.0);
  printf("%d\n", Local_Neuroseg_Has_Stack_Value(locseg, stack, 1.0, 2.0));
#endif

#if 0
  Rpi_Neuroseg *seg = New_Rpi_Neuroseg();
  Print_Rpi_Neuroseg(seg);
  Geo3d_Scalar_Field *field = Rpi_Neuroseg_Field(seg, NULL);
  Print_Geo3d_Scalar_Field(field);
  Geo3d_Scalar_Field_Translate(field, 5, 5, 1);

  Stack *stack = Make_Stack(GREY, 11, 11, 10);
  double coef[2] = {3, 10};
  Geo3d_Scalar_Field_Draw_Stack(field, stack, coef, NULL);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Local_Rpi_Neuroseg *rpiseg = New_Local_Rpi_Neuroseg();
  Print_Local_Rpi_Neuroseg(rpiseg);

  Local_Neuroseg *locseg = Local_Rpi_Neuroseg_To_Local_Neuroseg(rpiseg, NULL);
  Print_Local_Neuroseg(locseg);
#endif

#if 0
  Stack *stack = Read_Stack("../data/rn003/cross_60_4.tif");
  Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg1, 3.1956, 0.0, 11.0, 14.1439, 0.58702, 0.0, 0.0,
      0.53506, 46.7298, 54.0021, 56.2958);
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg2, 3.02447, 0.0, 11.0, 14.136, 1.56699, 0.0, 0.0,
      0.60647, 44.5455, 48.4025, 50.1361);
  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Fit_Workspace(ws);
  ws->sws->mask = Copy_Stack(stack);

  //ws->sws->fs.n = 1;
  //ws->sws->fs.options[0] = STACK_FIT_CORRCOEF;
  Locseg_Label_Workspace *llw = New_Locseg_Label_Workspace();
  
  //llw->sdiff = 2.0;
  llw->flag = 0;
  llw->value = 1;


  double center[3];
  int k;
  for (k = 0; k < 20; k++) {
    Fit_Local_Neuroseg_W(locseg1, stack, 1.0, ws);
    /*
    Local_Neuroseg_Stack_Centroid(locseg1, stack, ws->sws->mask, center);
    Local_Neuroseg_Reset_Center(locseg1, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg1, ws->sws->mask, 1.0, llw);
    }
    Fit_Local_Neuroseg_W(locseg2, stack, 1.0, ws);
    /*
    Local_Neuroseg_Ellipse_Stack_Centroid(locseg2, stack, ws->sws->mask, center);
    Local_Neuroseg_Ellipse_Reset_Center(locseg2, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg2, ws->sws->mask, 1.0, llw);
    }
  }

  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));
  
  Translate_Stack(stack, COLOR, 1);
  Stack_Draw_Workspace *dw = New_Stack_Draw_Workspace();
  dw->color.r = 255;
  dw->color.g = 0;
  dw->color.b = 0;

  Local_Neuroseg_Draw_Stack(locseg1, stack,  dw);

  dw->color.r = 0;
  dw->color.g = 255;
  dw->color.b = 0;
  Local_Neuroseg_Draw_Stack(locseg2, stack,  dw);
  
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();
  /*
  Set_Local_Neuroseg(locseg1, 3.14581, 0, 11, 2.12363, 4.55054, 0.0, 0.0,
      1.12757, 151.483, 235.144, 50.5362);
  Set_Local_Neuroseg(locseg2, 1.93437, 0, 11, 1.67588, 2.29605, 0.0, 0.0,
      0.86144, 138.532, 233.474, 43.7171);
      */
  
  Set_Local_Neuroseg(locseg1, 2.95271, 0.0, 15.0, 14.374, 0.175, 0.0, 0.0, 0.71,
      344, 187, 23);
  Set_Local_Neuroseg(locseg2, 2.49, 0.0, 15.0, 14.38, 4.7, 0.0, 0.0, 0.76, 358,
      184, 25);;
  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Fit_Workspace(ws);
  //ws->sws->mask = Copy_Stack(stack);

  //ws->sws->fs.n = 1;
  //ws->sws->fs.options[0] = STACK_FIT_CORRCOEF;
  Locseg_Label_Workspace *llw = New_Locseg_Label_Workspace();
  
  //llw->sdiff = 2.0;
  llw->flag = 0;
  llw->value = 1;


  double center[3];
  int k;
  for (k = 0; k < 20; k++) {
    Fit_Local_Neuroseg_W(locseg1, stack, 1.0, ws);
    /*
    Local_Neuroseg_Stack_Centroid(locseg1, stack, ws->sws->mask, center);
    Local_Neuroseg_Reset_Center(locseg1, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg1, ws->sws->mask, 1.0, llw);
    }
    Fit_Local_Neuroseg_W(locseg2, stack, 1.0, ws);
    /*
    Local_Neuroseg_Ellipse_Stack_Centroid(locseg2, stack, ws->sws->mask, center);
    Local_Neuroseg_Ellipse_Reset_Center(locseg2, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg2, ws->sws->mask, 1.0, llw);
    }
  }

  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));
  
  Translate_Stack(stack, COLOR, 1);
  Stack_Draw_Workspace *dw = New_Stack_Draw_Workspace();
  dw->color.r = 255;
  dw->color.g = 0;
  dw->color.b = 0;

  Local_Neuroseg_Draw_Stack(locseg1, stack,  dw);

  dw->color.r = 0;
  dw->color.g = 255;
  dw->color.b = 0;
  Local_Neuroseg_Draw_Stack(locseg2, stack,  dw);
  
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/diadem_c1_01.xml");
  Local_Neuroseg *locseg1 = New_Local_Neuroseg();
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg1, 1.85767, 0, 11, 1.66543, 2.83355, 0.0, 0.0,
      1.07149, 300.237, 71.2499, 46.6658);
  Set_Local_Neuroseg(locseg2, 1.45179, 0, 11, 1.59399, 4.42977, 0.0, 0.0,
      0.85898, 305.443, 72.3464, 47.0313);
  /*
  Set_Local_Neuroseg(locseg1, 1.82315, 0, 11, 14.1418, 4.62167, 0.0, 0.0,
      0.83473, 194.615, 199.018, 39.795);
  Set_Local_Neuroseg(locseg2, 1.817, 0, 11, 1.46173, 2.68922, 0.0, 0.0,
      0.699693, 188.12, 194.161, 39.2096);
      */

  /*
  Set_Local_Neuroseg(locseg1, 2.95271, 0.0, 15.0, 14.374, 0.175, 0.0, 0.0, 0.71,
      344, 187, 23);
  Set_Local_Neuroseg(locseg2, 2.49, 0.0, 15.0, 14.38, 4.7, 0.0, 0.0, 0.76, 358,
      184, 25);;
      */
  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));

  Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
  Default_Locseg_Fit_Workspace(ws);
  ws->sws->mask = Copy_Stack(stack);

  //ws->sws->fs.n = 1;
  //ws->sws->fs.options[0] = STACK_FIT_CORRCOEF;
  Locseg_Label_Workspace *llw = New_Locseg_Label_Workspace();
  
  //llw->sdiff = 2.0;
  llw->flag = 0;
  llw->value = 1;


  double center[3];
  int k;
  for (k = 0; k < 20; k++) {
    Fit_Local_Neuroseg_W(locseg1, stack, 1.0, ws);
    /*
    Local_Neuroseg_Stack_Centroid(locseg1, stack, ws->sws->mask, center);
    Local_Neuroseg_Reset_Center(locseg1, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg1, ws->sws->mask, 1.0, llw);
    }
    Fit_Local_Neuroseg_W(locseg2, stack, 1.0, ws);
    /*
    Local_Neuroseg_Ellipse_Stack_Centroid(locseg2, stack, ws->sws->mask, center);
    Local_Neuroseg_Ellipse_Reset_Center(locseg2, center);
    */
    if (ws->sws->mask != NULL) {
      Zero_Stack(ws->sws->mask);
      Local_Neuroseg_Label_W(locseg2, ws->sws->mask, 1.0, llw);
    }
  }

  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));
  
  Translate_Stack(stack, COLOR, 1);
  Stack_Draw_Workspace *dw = New_Stack_Draw_Workspace();
  dw->color.r = 255;
  dw->color.g = 0;
  dw->color.b = 0;

  Local_Neuroseg_Draw_Stack(locseg1, stack,  dw);

  dw->color.r = 0;
  dw->color.g = 255;
  dw->color.b = 0;
  Local_Neuroseg_Draw_Stack(locseg2, stack,  dw);
  
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 2.47595, 0, 197.38, 1.35452, 6.9815, 0.0, 0.0,
      2.13552, 36.6836, 435.91, 0.858181);
  double pos[3];
  Local_Neuroseg_Top(locseg, pos);
  Print_Coordinate_3d(pos);
  Field_Range range;
  Neuroseg_Field_Range(&(locseg->seg), &range, 1.0);
  printf("(%d, %d, %d); %d x %d x %d\n", range.first_corner[0],
      range.first_corner[1], range.first_corner[2], range.size[0],
      range.size[1], range.size[2]);

  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, NULL, NULL);

  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  Translate_Stack(stack, COLOR, 1);
  Stack_Draw_Workspace *ws = New_Stack_Draw_Workspace();
  Local_Neuroseg_Draw_Stack(locseg, stack, ws); 
  //Geo3d_Scalar_Field_Draw_Stack(field, stack, NULL, NULL);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 5.0, 0, 20, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 0.0, 0.0);
  double pos[3];
  Local_Neuroseg_Top(locseg, pos);
  Print_Coordinate_3d(pos);
  Field_Range range;
  Neuroseg_Field_Range(&(locseg->seg), &range, 1.0);
  printf("(%d, %d, %d); %d x %d x %d\n", range.first_corner[0],
      range.first_corner[1], range.first_corner[2], range.size[0],
      range.size[1], range.size[2]);
#endif

#if 1
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  locseg->seg.r1 = 2.0;
  Print_Local_Neuroseg(locseg);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_Sp(locseg, NULL, NULL);
  Print_Geo3d_Scalar_Field(field);
  double minv = darray_min(field->values, field->size, NULL);
  int i;
  for (i = 0; i < field->size; i++) {
    field->values[i] -= minv;
  }

  Local_Neuroseg *locseg2 = Local_Neuroseg_From_Field(field, NULL);
  Print_Local_Neuroseg(locseg2);
#endif

  return 0;
}
