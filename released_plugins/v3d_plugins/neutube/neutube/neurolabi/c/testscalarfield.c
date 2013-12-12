/* testscalarfield.c
 *
 * 27-Apr-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <image_lib.h>
#include "tz_constant.h"
#include "tz_stack_draw.h"
#include "tz_local_neuroseg.h"
#include "tz_bifold_neuroseg.h"
#include "tz_stack_stat.h"
#include "tz_farray.h"
#include "tz_geo3d_point_array.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_stack_bwmorph.h"
#include "tz_darray.h"

int main()
{
#if 0
  /* Make a label stack */
  Stack *stack = Make_Stack(FLOAT32, 100, 100, 50);
  Zero_Stack(stack);
#endif

#if 0
  /* Make test neuron segment */
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 1, 2, 24, 0, 0, 
		     NEUROSEG_MAX_CURVATURE, 50, 50, 25);

  Print_Local_Neuroseg(locseg);

  /* Generate field */
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, 1.0, NULL);

  //Print_Geo3d_Scalar_Field(field);

  Delete_Local_Neuroseg(locseg);
#endif
  
#if 0
  /* Make bifold neuron segment */
  Bifold_Neuroseg *bn = New_Bifold_Neuroseg();
  Set_Bifold_Neuroseg(bn, 2, 2, 2, 2, 24, 0.5, 1, 1, 1, 0);

  /* Generate field */
  Geo3d_Scalar_Field *field = Bifold_Neuroseg_Field(bn, 1.0, NULL);

  Geo3d_Scalar_Field_Translate(field, 50, 50, 25);

  Delete_Bifold_Neuroseg(bn);
#endif

#if 0
  //Print_Geo3d_Scalar_Field(field);

  /* Draw it in a stack */
  double coef[] = {0.1, 1000.0};
  double range[] = {0.0, 10000.0};
  Geo3d_Scalar_Field_Draw_Stack(field, stack, coef, range);


  int idx;
  printf("%g\n", Stack_Max(stack, &idx, NULL));

  /* Turn the stack to GREY type */
  Translate_Stack(stack, GREY, 1);
  printf("%g\n", Stack_Max(stack, &idx, NULL));

  /* Make canvas */
  Stack *canvas = Make_Stack(COLOR, stack->width, stack->height, stack->depth);
  Zero_Stack(canvas);
  
  /* Label the canvas */
  Stack_Label_Color(canvas, stack, 5.0, 1.0, stack);

  /* Save the stack */
  Write_Stack("../data/test.tif", canvas);

  /* clean up */
  Kill_Geo3d_Scalar_Field(field);
  Kill_Stack(stack);
  Kill_Stack(canvas);
#endif

#if 0
  Geo3d_Scalar_Field *field = Read_Geo3d_Scalar_Field("../data/diadem_e3/seeds");
  Print_Geo3d_Scalar_Field(field);
  Geo3d_Scalar_Field_Export_V3d_Marker(field, "../data/test.marker");
#endif
  
#if 0
  Stack_Fit_Score fs;
  fs.n = 2;
  fs.scores[0] = 0.5;
  fs.options[0] = 1;
  fs.scores[1] = 110.5;
  fs.options[1] = 0;
  
  Print_Stack_Fit_Score(&fs);

  /*
  FILE *fp = fopen("../data/test.bn", "w");
  Stack_Fit_Score_Fwrite(&fs, fp);
  fclose(fp);
  */
  Stack_Fit_Score fs2;
  FILE *fp2 = fopen("../data/test.bn", "r");
  Stack_Fit_Score_Fread(&fs2, fp2);
  fclose(fp2);
  
  Print_Stack_Fit_Score(&fs2);
#endif

#if 0
  Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Import_Apo("/Users/zhaot/Data/jinny/edswc_A copy/edswc_A0002.swc.apo");
  Print_Geo3d_Scalar_Field(field);
#endif

#if 0
  Geo3d_Ball *ball = New_Geo3d_Ball();
  ball->r = 3.0;
  Stack *stack = Make_Stack(GREY, 100, 100, 100);
  One_Stack(stack);
  int i, j, k;
  int offset = 0;
  for (k = 0; k < stack->depth; k++) {
    for (j = 0; j < stack->height; j++) {
      for (i = 0; i < stack->width; i++) {
	if ((i == 0) || (j == 0) || (k == 0) ||
	    (i == stack->width-1) || (j == stack->height-1) || 
	    (k == stack->depth - 1)) {
	  stack->array[offset] = 0;
	}
	offset++;
      }
    }
  }
  Stack *distmap = Stack_Bwdist_L_U16(stack, NULL, 0);
  tic();
  Geo3d_Ball_Mean_Shift(ball, distmap, 1.0, 0.5);
  printf("%llu\n", toc());
  Print_Geo3d_Ball(ball);
#endif

#if 0
  Geo3d_Scalar_Field *field1 = Make_Geo3d_Scalar_Field(3);
  Set_Coordinate_3d(field1->points[0], 1, 1, 1);
  Set_Coordinate_3d(field1->points[1], 2, 2, 2);
  Set_Coordinate_3d(field1->points[2], 3, 3, 3);
  field1->values[0] = 1;
  field1->values[1] = 2;
  field1->values[2] = 3;

  Geo3d_Scalar_Field *field2 = Make_Geo3d_Scalar_Field(3);
  Set_Coordinate_3d(field2->points[0], 4, 4, 4);
  Set_Coordinate_3d(field2->points[1], 5, 5, 5);
  Set_Coordinate_3d(field2->points[2], 6, 6, 6);
  field2->values[0] = 4;
  field2->values[1] = 5;
  field2->values[2] = 6;

  Print_Geo3d_Scalar_Field(field1);
  Print_Geo3d_Scalar_Field(field2);

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(2);
  Geo3d_Scalar_Field_Merge(field1, field2, field);
  Print_Geo3d_Scalar_Field(field);
#endif

#if 1
  Geo3d_Scalar_Field *field1 = Make_Geo3d_Scalar_Field(6);
  Set_Coordinate_3d(field1->points[0], 1, 1, 1);
  Set_Coordinate_3d(field1->points[1], 2, 4, 2);
  Set_Coordinate_3d(field1->points[2], 3, 3, 8);
  Set_Coordinate_3d(field1->points[3], 8, 4, 4);
  Set_Coordinate_3d(field1->points[4], 3, 5, 5);
  Set_Coordinate_3d(field1->points[5], 6, 7, 9);
  field1->values[0] = 1;
  field1->values[1] = 1;
  field1->values[2] = 1;
  field1->values[3] = 1;
  field1->values[4] = 1;
  field1->values[5] = 1;
  double vec[9];
  double value[3]; 
  Geo3d_Scalar_Field_Pca(field1, value, vec);
  darray_print2(vec, 3, 3);
  darray_print2(value, 3, 1);
#endif

  return 0;
}
