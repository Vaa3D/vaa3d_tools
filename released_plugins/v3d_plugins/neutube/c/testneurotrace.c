#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tz_error.h"
#if HAVE_LIBGSL == 1
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics_double.h>
#endif
#include "tz_constant.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_voxel_graphics.h"
#include "tz_neurotrace.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"
#include "tz_objdetect.h"
#include "tz_voxeltrans.h"
#include "tz_stack_stat.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_vector.h"
#include "tz_stack_bwmorph.h"
#include "tz_vrml_io.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "tz_fmatrix.h"
#include "tz_fimage_lib.h"
#include "tz_arraytrans.h"
#include "tz_neuron_structure.h"
#include "tz_utilities.h"
#include "tz_workspace.h"
#include "tz_stack_graph.h"
#include "tz_stack_attribute.h"
#include "tz_math.h"
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
#if 0
  Stack *stack = Read_Stack("/tmp/binimg.tif");
 
  Stack *dist = Stack_Bwdist(stack);

  Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_ALTER1);
  
  Write_Stack("/tmp/seedimg.tif", seeds);
#endif

#if 0
  Stack *stack = Read_Stack("/tmp/seedimg.tif");

  Voxel_List *list = Stack_To_Voxel_List(stack);

  Print_Voxel_List(list);

  Kill_Stack(stack);
  
  
  stack = Read_Stack("/tmp/binimg.tif");
  Stack *dist = Stack_Bwdist(stack);

  Pixel_Array *pa = Voxel_List_Sampling(dist, list);
  
  Print_Pixel_Array(pa);

  Pixel_Array_Write("/tmp/seeds.pa", pa);
#endif

#if 0
  Stack *stack = Read_Stack("/tmp/seedimg.tif");

  Voxel_List *list = Stack_To_Voxel_List(stack);

  //Print_Voxel_List(list);

  Pixel_Array *pa = Pixel_Array_Read("/tmp/seeds.pa");
  //Print_Pixel_Array(pa);

  Voxel *seed;

  int i;
  double *pa_array = (double *) pa->array;

  gsl_vector_view gv = gsl_vector_view_array(pa_array, pa->size);
  printf("mean: %g, std: %g\n", gsl_stats_mean(pa_array, 1, pa->size), 
	 sqrt(gsl_stats_variance(pa_array, 1, pa->size)));
  
  double threshold = gsl_stats_mean(pa_array, 1, pa->size) + 
    3.0 * sqrt(gsl_stats_variance(pa_array, 1, pa->size));
  

  
  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  
  IMatrix *chord = Make_IMatrix(dim, 3);
  
  Stack *code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);

  Kill_Stack(stack);
  stack = Read_Stack("/tmp/binimg.tif");

  Stack_Not(stack, stack);

  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  Stack *signal = Read_Stack("../data/testneurotrace.tif");

  Stack *canvas = Translate_Stack(signal, COLOR, 0);


  /************** soma detection *************/
#if 0
  Struct_Element *se = Make_Ball_Se(((int) threshold));
  Stack *stack1 = Stack_Erode_Fast(stack, NULL, se);
  Stack *soma = Stack_Dilate(stack1, NULL, se);
  Kill_Stack(stack1);  

  Stack_And(stack, soma, soma);

  Stack_Label_Bwc(canvas, soma, color);

  Kill_Stack(soma);

  Write_Stack("../data/test.tif", canvas);
  return 0;
#endif
  /*******************************************/
  

  Object_3d *obj = NULL;
  int seed_offset = -1;

  Neurochain *chain = NULL;
  double z_scale = 0.8;
 
  Stack *traced = 
    Make_Stack(GREY, signal->width, signal->height, signal->depth);
  One_Stack(traced);

  for (i = 0; i < pa->size; i++) {
    seed = Voxel_Queue_De(&list);
    printf("%d, color: %u, %u, %u\n", 
	   i,
	   *STACK_PIXEL_8(canvas, seed->x, seed->y, seed->z, 0), 
	   *STACK_PIXEL_8(canvas, seed->x, seed->y, seed->z, 1),
	   *STACK_PIXEL_8(canvas, seed->x, seed->y, seed->z, 2));
    if (*STACK_PIXEL_8(canvas, seed->x, seed->y, seed->z, 2) == 0) {
      continue;
    }

    double width = pa_array[i];

    if (width > threshold) {
      continue;
    }

    chain = New_Neurochain();

    Print_Voxel(seed);
    printf("%g\n", width);

    
    int max_level = (int) (width + 0.5);
    if (max_level < 6) {
      max_level = 6;
    }

    seed_offset = Stack_Util_Offset(seed->x, seed->y, seed->z, 
				    stack->width, stack->height, stack->depth);
    Stack_Level_Code_Constraint(stack, code, chord->array, &seed_offset, 1, 
				max_level + 1);

    Voxel_t v;
    Voxel_To_Tvoxel(seed, v);

    Print_Tvoxel(v);

    Stack *tmp_stack = Copy_Stack(stack);
    obj = Stack_Grow_Object_Constraint(tmp_stack, 1, v, chord, code, 
				       max_level);
    Free_Stack(tmp_stack);

    Print_Object_3d_Info(obj);

    double vec[3];
    Object_3d_Orientation_Zscale(obj, vec, MAJOR_AXIS, z_scale);

    double theta, psi;
    Geo3d_Vector obj_vec;
    Set_Geo3d_Vector(&obj_vec, vec[0], vec[1], vec[2]);

    darray_print2(vec, 3, 1);

    Geo3d_Vector_Orientation(&obj_vec, &theta, &psi);
    
    Set_Neuroseg(&(chain->locseg.seg), width, width, 10.0, 
		 theta, psi);

    double cpos[3];
    cpos[0] = seed->x;
    cpos[1] = seed->y;
    cpos[2] = seed->z;
    cpos[2] *= 0.8;

    Set_Neuroseg_Position(&(chain->locseg), cpos, NEUROSEG_CENTER);

    if (Initialize_Tracing(signal, chain, NULL, z_scale) >= MIN_SCORE) {
      chain = Trace_Neuron(signal, chain, BOTH, traced, z_scale);

      Print_Neurochain(chain);

      //Stack_Draw_Object_Bwc(canvas, obj, color);

      Neurochain_Erase(traced, Neurochain_Head(chain), z_scale);
      Neurochain_Label(canvas, Neurochain_Head(chain), z_scale);
    }

    Free_Neurochain(chain);

    free(seed);

    Kill_Object_3d(obj);
  }

  Write_Stack("../data/test2.tif", traced);
  Write_Stack("../data/test.tif", canvas);
  
#endif

#if 0
  Stack *stack = Read_Stack("/tmp/seedimg.tif");
  Voxel_List *list = Stack_To_Voxel_List(stack);
  
  Kill_Stack(stack);
  stack = Read_Stack("../data/testneurotrace.tif");
  Pixel_Array *pa = Voxel_List_Sampling(stack, list);
  
  Print_Pixel_Array(pa);

  Pixel_Array_Write("/tmp/test.pa", pa);
  Pixel_Array *pa2 = Pixel_Array_Read("/tmp/test.pa");
  Print_Pixel_Array(pa2);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  double z_scale = 1.0;
  Neurochain *chain = New_Neurochain();

#if 0
  Set_Neuroseg(&(chain->locseg.seg), 2.78302, 3.60932, 12.0, 3.58051, 4.6416);
  Set_Neuropos(chain->locseg.pos, 308.154, 49.7471, 134.73);
#endif

#if 0
  Set_Neuroseg(&(chain->locseg.seg), 2.7830153868018668, 3.6093167912750355, 12.0, 3.5805148810681988, 4.641602279067599);
  Set_Neuropos(chain->locseg.pos, 308.15367011522028, 49.747085927056794, 134.73010125059292);
#endif
  
#if 0
  Set_Neuroseg(&(chain->locseg.seg), 2.7165, 3.49483, 12.0, 3.39146, 4.70638);
  Set_Neuropos(chain->locseg.pos, 306.119, 49.8914, 139.075);
#endif

#if 0
  Set_Neuroseg(&(chain->locseg.seg), 3.17991, 4.56142, 12.0, 2.10521, 2.34306,
	       0);
  Set_Neuropos(chain->locseg.pos, 344.281, 258.937, 138.332);
#endif
  
  Set_Neuroseg(&(chain->locseg.seg), 2.12805, 3.37411, 12.0, 2.94503, 2.25645,
	       0);
  Set_Neuropos(chain->locseg.pos, 343.556, 258.344, 143.04);

  //refit_local_neuroseg(&(chain->locseg), stack, z_scale, NULL);
  
  Neuroseg_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;
  printf("%g\n", Local_Neuroseg_Score(&(chain->locseg), stack, z_scale, &fs));
  //Trace_Neuron2(stack, chain, BACKWARD, NULL, z_scale, 10);
  
  //Print_Neurochain(chain);

  stack = Translate_Stack(stack, COLOR, 1);
  Neurochain_Label(stack, Neurochain_Head(chain), z_scale);
  Write_Stack("../data/test.tif", stack);

#endif

#if 0
  Stack *stack = Read_Stack("../data/testneurotrace.tif");

  double z_scale = 0.488/0.585;

  Neurochain *chain = New_Neurochain();
  Set_Neuroseg(&(chain->locseg.seg), 2.0, 2.0, 12.0, -TZ_PI_2, 0.0);
  //Set_Neuropos(chain->locseg.pos, 95, 96, 162*0.8);
  //Set_Neuropos(chain->locseg.pos, 39, 192, 283 * z_scale);
  Set_Neuropos(chain->locseg.pos, 129, 170, 239 * z_scale);

  //Trace_Neuron(stack, chain, BOTH, NULL, z_scale, 100);

  //Set_Neuropos(chain->locseg.pos, 143, 99, 107*0.8);
  //Set_Position(chain->position, 127, 68, 34*0.8);
  //Set_Position(chain->position, 60, 136, 91*0.8);
  //Set_Position(chain->position, 176, 221, 184*0.8);

  //Fit_Local_Neuroseg(stack, &(chain->seg), chain->position, TRUE); 

  /*  
  Set_Neuroseg(&(chain->locseg.seg), 1.8295717513170491, 1.2647574210394514, 
	       12, -1.2864278377709504, 0.51799979287881037);
  Set_Neuropos(chain->locseg.pos, 91.370009398825573, 98.614687377839488, 
	       131.9121806489851);
  */

  /*
  Set_Neuroseg(&(chain->locseg.seg), 5, 5, 
	       11.1336, -TZ_PI_2, 0.0);
  Set_Neuropos(chain->locseg.pos, 90.1456, 99.1741, 132.564);
  */

  int i;
  for (i = 0; i < 1; i++) {
    Initialize_Tracing(stack, chain, NULL, z_scale);
  }
  
  /*
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION, 
					      NEUROPOS_VAR_MASK_NONE, 
					      var_index);

  Fit_Local_Neuroseg(&(chain->locseg), stack, var_index, nvar, z_scale,
		      NULL);

  nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R, 
					  NEUROPOS_VAR_MASK_NONE, 
					  var_index);

  Fit_Local_Neuroseg(&(chain->locseg), stack, var_index, nvar, z_scale,
		      NULL);

  printf("score: %g\n", 
	 Local_Neuroseg_Score(&(chain->locseg), stack, z_scale, NULL));
  */

  Trace_Neuron2(stack, chain, BOTH, NULL, z_scale, 10);

  

  //Print_Neurochain(chain);

  stack = Translate_Stack(stack, COLOR, 1);
  Neurochain_Label(stack, Neurochain_Head(chain), z_scale);
  //Flip_Local_Neuroseg(&(chain->locseg));
  //Local_Neuroseg_Label(&(chain->locseg), stack, 1, z_scale);

  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
#endif
  
#if 0 /* fly neuron tracing */
  //Stack *stack = Read_Stack("../data/fly_neuron.tif");
  //Stack *stack = Read_Stack("../data/resimg.tif");
  Stack *stack = Read_Stack("../data/fly_neuron2.tif");

  double z_scale = 1.0;
  
  Neurochain *chain = New_Neurochain();

  //Set_Neuroseg(&(chain->locseg.seg), 2.19832, 2.18053, 12, -3.54093, 0.779524);
  Set_Neuropos(chain->locseg.pos, 166, 325, 43); /* fly_neuron2 */
  

  Set_Neuroseg(&(chain->locseg.seg), 2.0, 2.0, 12.0, -TZ_PI_2, 0.0);
  //Set_Neuroseg(&(chain->locseg.seg), 2.0, 2.0, 12.0, TZ_PI_2, TZ_PI_2);
  //Set_Neuroseg(&(chain->locseg.seg), 1.70074, 1.09443, 12, 2.12734, 1.77422);
  //Set_Neuroseg(&(chain->locseg.seg), 4.51936, 4.19414, 12, 3.56059, 3.13054);
  
  //Set_Neuropos(chain->locseg.pos, 294, 178, 172);

  //Set_Neuropos(chain->locseg.pos, 280.328, 60.8357, 198.226);
  //Set_Neuropos(chain->locseg.pos, 281, 64, 202);
  //Set_Neuropos(chain->locseg.pos, 207, 227, 168);
  //Set_Neuropos(chain->locseg.pos, 166, 238, 141);
  //Set_Neuropos(chain->locseg.pos, 294, 178, 175);
  //Set_Neuropos(chain->locseg.pos, 296, 217, 160);
  //Set_Neuropos(chain->locseg.pos, 296, 217, 160);
  //Set_Neuropos(chain->locseg.pos, 282, 64, 204);
  //Set_Neuropos(chain->locseg.pos, 294, 255, 158);
  //Set_Neuropos(chain->locseg.pos, 220, 342, 68);
  //Set_Neuropos(chain->locseg.pos, 227, 283, 139);
  //Set_Neuropos(chain->locseg.pos, 231, 327, 67);
  //Set_Neuropos(chain->locseg.pos, 179, 266, 142);
  //Set_Neuropos(chain->locseg.pos, 246.546, 296.202, 106.407);
  //Set_Neuropos(chain->locseg.pos, 307.867, 62.5301, 274.218);
  
  //Trace_Neuron(stack, chain, BOTH, NULL, z_scale, 100);

  //Set_Neuropos(chain->locseg.pos, 143, 99, 107*0.8);
  //Set_Position(chain->position, 127, 68, 34*0.8);
  //Set_Position(chain->position, 60, 136, 91*0.8);
  //Set_Position(chain->position, 176, 221, 184*0.8);

  //Fit_Local_Neuroseg(stack, &(chain->seg), chain->position, TRUE); 

  /*
  Set_Neuroseg(&(chain->locseg.seg), 1.8295717513170491, 1.2647574210394514, 
	       12, -1.2864278377709504, 0.51799979287881037);
  Set_Neuropos(chain->locseg.pos, 91.370009398825573, 98.614687377839488, 
	       131.9121806489851);
  */  

  /*
  Set_Neuroseg(&(chain->locseg.seg), 5, 5, 
	       11.1336, -TZ_PI_2, 0.0);
  Set_Neuropos(chain->locseg.pos, 90.1456, 99.1741, 132.564);
  */  


  int i;
  for (i = 0; i < 1; i++) {
    Initialize_Tracing(stack, chain, NULL, z_scale);
  }

#if 0  
  /*
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION, 
					      NEUROPOS_VAR_MASK_NONE, 
					      var_index);

  Fit_Local_Neuroseg(&(chain->locseg), stack, var_index, nvar, z_scale,
		      NULL);

  nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R, 
					  NEUROPOS_VAR_MASK_NONE, 
					  var_index);

  Fit_Local_Neuroseg(&(chain->locseg), stack, var_index, nvar, z_scale,
		      NULL);

  printf("score: %g\n", 
	 Local_Neuroseg_Score(&(chain->locseg), stack, z_scale, NULL));
  */

  Trace_Neuron2(stack, chain, FORWARD, NULL, z_scale, 30);  
#endif

  Print_Neurochain(chain);

  printf("%g\n", Local_Neuroseg_Score(&(chain->locseg), stack, z_scale, NULL));
  
  stack = Translate_Stack(stack, COLOR, 1);
  Neurochain_Label(stack, Neurochain_Head(chain), z_scale);
  //Flip_Local_Neuroseg(&(chain->locseg));
  //Local_Neuroseg_Label(&(chain->locseg), stack, 1, z_scale);

  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
#endif

#if 0 /* check segments */
  Neurochain *chain = New_Neurochain();
  FILE *fp = fopen("../data/fly_neuron/chain103.bn", "r");
  Neurochain_Fread(chain, fp);
  fclose(fp);
  int length = Neurochain_Length(chain, FORWARD);
  printf("%d\n", length);
  //Print_Neurochain(chain);

  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  double z_scale = 1.0;
  Neuroseg_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;
  double score = Neurochain_Score(chain, stack, z_scale, &fs);
  printf("%g\n", score / length);
#endif

#if 0 /* check chain features */
  char cmd[100];
  sprintf(cmd, "ls ../data/%s/*.wrl > ../data/%s/wrllist.txt")
  system(cmd);
#endif

#if 0
  /* read stack */
  Stack *stack = Read_Stack("../data/run_w1_t1.tif");
  double z_scale = 1.0;

  /* Initialize neurochain */
  Neurochain *chain = New_Neurochain();
  Set_Neuroseg(&(chain->locseg.seg), 3.0, 3.0, 12.0, 0.0, 0.0, 0.0);
  Set_Neuropos(chain->locseg.pos, 72, 35, 0);

  //Initialize_Tracing(stack, chain, NULL, z_scale);
  Trace_Neuron2(stack, chain, FORWARD, NULL, z_scale, 30);

  Translate_Stack(stack, COLOR, 1);
  Neurochain_Label(stack, Neurochain_Head(chain), z_scale);
  
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  double pos[] = {268, 265, 56};
  double r = 3.0;
  Local_Neuroseg_Ellipse *locne = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(locne, r, r, TZ_PI_2, 0, 0, 0,
			     pos[0], pos[1], pos[2]);
  char *imgfile = "../data/mouse_neuron.tif";
  Stack *stack = Read_Stack(imgfile);

  Locne_Chain *chain = Trace_Soma(stack, 1.0, locne, NULL);
  
  double point[] = {277.5, 255.5, 56.5};
  BOOL hit = Locne_Chain_Hittest(chain, point);

  if (hit == TRUE) {
    printf("The point hits the object\n");
  } else {
    printf("The point misses the object\n");
  }
#endif

#if 0
  Geo3d_Scalar_Field *seed = 
    Read_Geo3d_Scalar_Field("../data/mouse_neuron_org/seeds");

  int *indices = iarray_malloc(seed->size);
  double *values = darray_malloc(seed->size);

  int i;
  for (i = 0; i < seed->size; i++) {
    indices[i] = i;
  }

  darraycpy(values, seed->values, 0, seed->size);

  darray_qsort(values, indices, seed->size);
  
  int index = indices[seed->size-1];

  printf("%d\n", seed->size);

  printf("%g\n", seed->values[index]);
  printf("%g, %g, %g\n", seed->points[index][0], 
	 seed->points[index][1], seed->points[index][2]);
  
  double r = seed->values[index];


  Local_Neuroseg_Ellipse *segs[10];

  segs[0] = New_Local_Neuroseg_Ellipse();
  Set_Local_Neuroseg_Ellipse(segs[0], r, r, TZ_PI_2, 0, 0, 0, 0,
			     seed->points[index][0],
			     seed->points[index][1],
			     seed->points[index][2]);
  char *imgfile = "../data/mouse_neuron_org.tif";
  Stack *stack = Read_Stack(imgfile);

  Locne_Chain *somas[10];
  int nsoma = 0;

  Soma_Trace_Workspace *stw = New_Soma_Trace_Workspace();
  stw->tw.dyvar[0] = r * 3.0;
  stw->tw.dyvar[1] = 5.0;
  stw->tw.test_func = Locne_Chain_Trace_Test;

  somas[nsoma++] = Trace_Soma(stack, 1.0, segs[0], stw);


  int j;

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

      r = seed->values[index];
      segs[nsoma] = New_Local_Neuroseg_Ellipse();
      Set_Local_Neuroseg_Ellipse(segs[nsoma], r, r, TZ_PI_2, 0, 0, 0, 0,
				 seed->points[index][0],
				 seed->points[index][1],
				 seed->points[index][2]);
      somas[nsoma] = Trace_Soma(stack, 1.0, segs[nsoma], stw);
      nsoma++;
    }
    
    if (nsoma >= 10) {
      break;
    }
  }

  for (i = 0; i < nsoma; i++) {
    Print_Local_Neuroseg_Ellipse(segs[i]);
    printf("soma score: %g\n", Soma_Score(somas[i]));
    if (Soma_Score(somas[i]) > 1.0) {
      char file_path[100];
      sprintf(file_path, "../data/mouse_neuron/soma%d.bn", i); 
      Write_Locne_Chain(file_path, somas[i]);
      //Soma_Stack_Mask(somas[i], stack, 1.0, 0.0);
    }
  }

  darray_write("../data/test.bn", values, seed->size);

  //Write_Stack("../data/test.tif", stack);

#  if 1
  Stack *label = Make_Stack(FLOAT32, stack->width, stack->height, stack->depth);
  Zero_Stack(label);
  double coef[] = {0.1, 255.0};
  double range[] = {0.0, 10000.0};

  FILE *fp = fopen("../data/test.wrl", "w");
  
  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  Geo3d_Scalar_Field* field = NULL;
  Local_Neuroseg_Ellipse *locne;
  for (i = 0; i < nsoma; i++) {
    if ((Soma_Score(somas[i]) > 1.0) && Locne_Chain_Length(somas[i]) >= 10) {
      Locne_Chain *chain = somas[i];
      Locne_Chain_Vrml_Fprint(fp, chain, 0);
      Locne_Chain_Iterator_Start(chain);
      int offset = 0;
      while ((locne = Locne_Chain_Next(chain)) != NULL) {
	field = Local_Neuroseg_Ellipse_Field(locne, 0.5, NULL);
	Geo3d_Scalar_Field_Draw_Stack(field, label, coef, range);
	Kill_Geo3d_Scalar_Field(field);
	offset++;
      }

      //printf("%d\n", Locne_Chain_Length(chain));
    }
  }

  fclose(fp);

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
  Trace_Workspace *tw = New_Trace_Workspace();
  tw->trace_direction = DL_FORWARD;

  Stack *stack = Read_Stack("../data/fly_neuron2.tif");

  double z_scale = 1.0;
  
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuropos(locseg->pos, 166, 325, 43); /* fly_neuron2 */
  Set_Neuroseg(&(locseg->seg), 2.0, 2.0, 12.0, -TZ_PI_2, 0.0, 0.0);

  tw->test_func = Locseg_Chain_Trace_Test;
  tw->dyvar[0] = 10.0;
  
  Stack_Fit_Score fs;
  fs.n = 2;
  fs.options[0] = 0;
  fs.options[1] = tw->tscore_option;
  Locseg_Chain* chain = Locseg_Chain_Trace_Init(stack, z_scale, locseg, &fs);
  
  Trace_Locseg(stack, z_scale, chain, tw);

  Print_Locseg_Chain(chain);

  Translate_Stack(stack, COLOR, 1);
  Locseg_Chain_Label(chain, stack, z_scale);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Trace_Workspace *tw = New_Trace_Workspace();
  tw->trace_direction = DL_BOTHDIR;

  Stack *stack = Read_Stack("../data/fly_neuron2.tif");

  double z_scale = 1.0;
  
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuropos(locseg->pos, 164, 322, 43);
  Set_Neuroseg(&(locseg->seg), 2.0, 0.0, 12.0, -TZ_PI_2, 0.0, 0.0, 0.0, 1.0);
  //Local_Neuroseg_Height_Search_P(locseg, stack, 1.0);
  Print_Local_Neuroseg(locseg);


  tw->test_func = Locseg_Chain_Trace_Test;
  tw->dyvar[0] = 10.0;
  
  Locseg_Fit_Workspace *fw = New_Locseg_Fit_Workspace();
  
  Locseg_Chain* chain = Locseg_Chain_Trace_Init(stack, z_scale, locseg, fw);


  // Local_Neuroseg_Height_Search_P(locseg, stack, 1.0);
  Print_Locseg_Chain(chain);

  Trace_Locseg(stack, z_scale, chain, tw);
  //Locseg_Chain_Remove_Overlap_Ends(chain);
  //Locseg_Chain_Remove_Turn_Ends(chain, 1.0);

  /*
  Local_Neuroseg_Plane *locnp = Local_Neuroseg_To_Plane(locseg, 12.0, NULL);
  Locnp_Chain *locnp_chain = Locnp_Chain_Trace_Init(stack, z_scale, locnp);
  
  tw->length = locseg->seg.h * 10;
  tw->fit_first = FALSE;
  tw->tscore_option = 1;
  tw->min_score = 0.5;
  tw->trace_direction = DL_FORWARD;
  tw->trace_mask = NULL;
  tw->dyvar[0] = 100.0;
  tw->test_func = Locnp_Chain_Trace_Test;
  Trace_Locnp(stack, 1.0, locnp_chain, tw);
  
  Locseg_Chain_Append_Locnp(chain, locnp_chain);

  Kill_Locnp_Chain(locnp_chain);
  */
  Translate_Stack(stack, COLOR, 1);
  Locseg_Chain_Label(chain, stack, z_scale);

  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
  Kill_Locseg_Chain(chain);
#endif

#if 0
  Neurochain *chain = Read_Neurochain("../data/mouse_neuron3_org/chain9810.bn");

  Stack *stack = Read_Stack("../data/mouse_neuron3_org.tif");
  double z_scale = 1.0;

  Locseg_Chain *locseg_chain = Neurochain_To_Locseg_Chain(chain);

  Locseg_Chain_Remove_End(locseg_chain, DL_HEAD);
  Locseg_Chain_Remove_End(locseg_chain, DL_TAIL);

  Print_Locseg_Chain(locseg_chain);


  Trace_Workspace tw;
  tw.length = 100;
  tw.fit_first = FALSE;
  tw.tscore_option = 1;
  tw.min_score = 0.3;
  tw.trace_direction = DL_FORWARD;
  tw.trace_mask = NULL;//Read_Stack("../data/mouse_neuron3_org/trace_mask.tif");
  tw.dyvar[0] = 10.0;
  tw.test_func = Locnp_Chain_Trace_Test;
  Locseg_Chain_Trace_Np(stack, z_scale, locseg_chain, &tw);

  Stretch_Stack_Value_Q(stack, 0.999);
  Stack *canvas = Translate_Stack(stack, COLOR, 1);
  Locseg_Chain_Label(locseg_chain, canvas, 1.0);
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *stack1 = Read_Stack("../data/fly_neuron/trace_mask.tif");
  Stack *stack2 = Read_Stack("../data/fly_neuron/mask.tif");
  Stack_And(stack1, stack2, stack2);
  Write_Stack("../data/test.tif", stack2);
#endif

#if 0
  static char *Spec[] = { "-p <int> <int> <int>",
			  NULL};

  Process_Arguments(argc, argv, Spec, 1);
  
  double pos[3];
  pos[0] = Get_Int_Arg("-p", 1);
  pos[1] = Get_Int_Arg("-p", 2);
  pos[2] = Get_Int_Arg("-p", 3);
  
  Geo3d_Scalar_Field *seed = 
    Read_Geo3d_Scalar_Field("../data/mouse_neuron3_org/seeds");

  int i;
  for (i = 0; i < seed->size; i++) {
    if ((fabs(pos[0] - seed->points[i][0]) < 1.0) &&
	(fabs(pos[1] - seed->points[i][1]) < 1.0) &&
	(fabs(pos[2] - seed->points[i][2]) < 1.0)) {
      printf("%d, %g\n", i, seed->values[i]);
      break;
    }
  }
  
  char chain_file_path[100];
  sprintf(chain_file_path, "../data/mouse_neuron3_org/chain%d.bn", i);
  Neurochain *chain = Read_Neurochain(chain_file_path);
  Print_Neurochain(chain);
#endif

#if 0
  Trace_Record *tr = NULL;
  fp = fopen("../data/test", "w");
  Trace_Record_Fwrite(tr, fp);
  fclose(fp);

 
#endif


#if 0
  Geo3d_Scalar_Field *seed = 
    Read_Geo3d_Scalar_Field("../data/adult_fly_neuron/seeds");

  Stack *stack = Read_Stack("../data/adult_fly_neuron.tif");

  double z_scale = 1.0;
  int index = 1213;
  double r0 = seed->values[index];
  if (r0 < 3.0) {
    r0 = 3.0;
  }

  printf("(%g %g %g) %g\n", seed->points[index][0], seed->points[index][1],
	 seed->points[index][2], seed->values[index]);

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuroseg_Position(locseg, seed->points[index], NEUROSEG_CENTER);
  Set_Neuroseg(&(locseg->seg), r0, r0, 12.0, 0.0, 0.0, 0.0);
  Stack_Fit_Score ortfs;
  ortfs.n = 1;
  ortfs.options[0] = STACK_FIT_CORRCOEF;
  Local_Neuroseg_Orientation_Search_C(locseg, stack, z_scale, &ortfs); 
  

  Trace_Workspace *tw = New_Trace_Workspace();
  tw->length = 200;
  tw->fit_first = FALSE;
  tw->tscore_option = STACK_FIT_CORRCOEF;
  tw->min_score = 0.3;
  tw->trace_direction = DL_BOTHDIR;
  tw->trace_status[0] = TRACE_NORMAL;
  tw->trace_status[1] = TRACE_NORMAL;
  tw->trace_range[0] = 0;
  tw->trace_range[1] = 0;
  tw->trace_range[2] = 0;
  tw->trace_range[3] = stack->width - 1;
  tw->trace_range[4] = stack->height - 1;
  tw->trace_range[5] = stack->depth - 1;
  tw->trace_mask = NULL;
  tw->dyvar[0] = 10.0;
  tw->test_func = Locseg_Chain_Trace_Test;

  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = tw->tscore_option;
  Locseg_Chain *chain = Locseg_Chain_Trace_Init(stack, z_scale, locseg, &fs);

  tic();
  if (fs.scores[0] >= tw->min_score) {
    Trace_Locseg(stack, z_scale, chain, tw);
  }
  printf("time passed: %lld\n", toc());
  
  Print_Locseg_Chain(chain);

  Translate_Stack(stack, GREY, 1);
  Stretch_Stack_Value_Q(stack, 0.999);   
  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Neurochain *chain_head = Neurochain_From_Locseg_Chain(chain);
  Neurochain_Label(canvas, chain_head, z_scale);
  Write_Stack("../data/test.tif", canvas);
  Kill_Stack(canvas);
  Kill_Stack(stack);
  
#endif

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/fly_neuron/chain26.bn");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/fly_neuron/chain90.bn");
  
  Stack *stack = Read_Stack("../data/fly_neuron.tif");

  neurocomp_conn_t conn;
  conn[0] = NEUROCOMP_CONN_HL;
  Locseg_Chain_Connection_Test(chain1, chain2, stack, 1.0, conn, NULL);

  Print_Neurocomp_Conn(conn);

  /*
  Int_Arraylist *path = Locseg_Chain_Path2(chain1, chain2, stack, 1.0);
  //Print_Int_Arraylist(path, "%d");
  
  Pixel_Array *pa = Stack_Index_Sampling(stack, path->array, path->length);
  Print_Pixel_Array(pa);
  Pixel_Array_Export("../data/test.bn", pa);
  */
  
  /*
  Image_Array ima;
  Translate_Stack(stack, COLOR, 1);
  ima.array = stack->array;

  int i;
  for (i = 0; i < path->length; i++) {
    ima.arrayc[path->array[i]][0] = 255;
    ima.arrayc[path->array[i]][1] = 0;
    ima.arrayc[path->array[i]][1] = 0;
  }

  Locseg_Chain_Label(chain1, stack, 1.0);
  //Locseg_Chain_Label(chain2, stack, 1.0);
  Write_Stack("../data/test.tif",stack);
  */
#endif

#if 0
  
  Stack *stack = Read_Stack("../data/mouse_single_org.tif");
  Stack *mask = Read_Stack("../data/mouse_single_org/mask2.tif");
  
  /*
  Stack *stack = Read_Stack("../data/rn003/cross_45_0.tif");
  Stack *mask = Read_Stack("../data/rn003/cross_45_0/mask2.tif");
  */

  Stack_Seed_Workspace *ssw = New_Stack_Seed_Workspace();
  Geo3d_Scalar_Field *seed = Stack_Seed(mask, ssw);

  Trace_Evaluate_Seed_Workspace *ws = New_Trace_Evaluate_Seed_Workspace();
  Trace_Evaluate_Seed(seed, stack, 1.0, ws);
  
  Trace_Workspace *tw = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(tw, stack);
  
  
  double sigma[3] = {1.0, 1.0, 0.5};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *result = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  
  Array_Link *a = FMatrix_To_Stack(result); 

  tw->sup_stack = Get_Stack_At(a);


  int nchain1;
  
  Locseg_Chain **chain = 
    Trace_Locseg_S(stack, 1.0, ws->locseg, ws->score, ws->nseed, tw, &nchain1);

  Stack_Binarize(tw->trace_mask);
  Stack *trace_mask = Translate_Stack(tw->trace_mask, GREY, 0);

  Stack_Or(trace_mask, ws->base_mask, trace_mask);
  Struct_Element *se = Make_Zline_Se(5);
  Stack *submask = Stack_Dilate(trace_mask, NULL, se);

  Stack_Bsub(mask, submask, mask);
  // Stack_Brighten_Bw(mask);

  Stack *mask2 = Stack_Remove_Small_Object(mask, NULL, 27, 26);
  /*
  Stack_Brighten_Bw(mask2);

  Write_Stack("../data/test.tif", mask2);

  return 1;
  */

  Geo3d_Scalar_Field *seed2 = Stack_Seed(mask2, ssw);
  Trace_Evaluate_Seed(seed2, stack, 1.0, ws);

  tw->min_chain_length = NEUROSEG_DEFAULT_H * 1.5;

  int nchain2;
  Locseg_Chain **chain2 = 
    Trace_Locseg_S(stack, 1.0, ws->locseg, ws->score, ws->nseed, tw, &nchain2); 
  
  Print_Geo3d_Scalar_Field(seed2);

  Write_Stack("../data/test.tif", tw->canvas);


  Kill_FMatrix(result);

  int nchain = nchain1 + nchain2;

  Locseg_Chain **allchain = (Locseg_Chain**) 
    Guarded_Malloc(sizeof(Locseg_Chain*) * nchain, "main");
  memcpy(allchain, chain, sizeof(Locseg_Chain*) * nchain1);
  memcpy(allchain + nchain1, chain2, sizeof(Locseg_Chain*) * nchain2);

#  if 1
  /* Array to store corrected chains */
  Neuron_Component *chain_array = Make_Neuron_Component_Array(nchain);
  int chain_number = 0;

  int i;

  /* build chain map */
  for (i = 0; i < nchain; i++) {
    if (allchain[i] != NULL) {
      if (Locseg_Chain_Geolen(allchain[i]) >= tw->min_chain_length) {
	Set_Neuron_Component(chain_array + chain_number, 
			     NEUROCOMP_TYPE_LOCSEG_CHAIN, allchain[i]);
	chain_number++;
      }
    }
  }

  Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(chain_array, 
						       chain_number,
						       stack, 1.0, NULL);
  Process_Neuron_Structure(ns);
  Neuron_Structure* ns2=
    Neuron_Structure_Locseg_Chain_To_Circle(ns);
  
  Neuron_Structure_Main_Graph(ns2);
  Neuron_Structure_To_Tree(ns2);
  
  Neuron_Structure_To_Swc_File_Circle(ns2, 
				      fullpath("../data", "test.swc", NULL));
#  endif

#endif

#if 0
  Stack *signal = Read_Stack("../data/diadem_e1.tif");
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  Local_Neuroseg locseg[2];
  Local_Neuroseg_Copy(locseg, Locseg_Chain_Head_Seg(chain1));
  Local_Neuroseg_Copy(locseg + 1, Locseg_Chain_Tail_Seg(chain1));
  while (1) {
    Trace_Workspace *tw = New_Trace_Workspace();
    Locseg_Chain_Default_Trace_Workspace(tw, signal);
    tw->break_refit = TRUE;
    strcpy(tw->save_path, "../data/test/chain");

    int nchain;
    Locseg_Chain **chain = Trace_Locseg_S(signal, 1.0, locseg, NULL, 2, tw,
					  &nchain);

    printf("%d\n", nchain);

    Kill_Trace_Workspace(tw);
    int i;
    for (i = 0; i < nchain; i++) {
      Kill_Locseg_Chain(chain[i]);
    }
    free(chain);
  }
#endif

#if 0
  Stack *signal = Read_Stack("../data/fly_neuron_n1.tif");
  Stack *mask = Read_Stack("../data/fly_neuron_n1/trace_mask.tif");
  
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n1/chain0.tb");
  Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain);
  Local_Neuroseg_Push(locseg, signal, 1.0, mask, STACK_FIT_VALID_SIGNAL_RATIO,
		      NULL, NULL);
#endif

#if 0
  Trace_Workspace *tw = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(tw, NULL);
  Print_Trace_Workspace(tw);
  Print_Locseg_Fit_Workspace((Locseg_Fit_Workspace*) tw->fit_workspace);
#endif

#if 0
  Trace_Record *tr = New_Trace_Record();
  tr->mask = ALL_ON_BIT_MASK;
  
  tr->fs.n = 1;
  tr->fs.options[0] = 1;
  tr->fs.scores[0] = 0.3;
  tr->hit_region = 1;
  tr->index = 2;
  tr->refit = 1;

  Print_Trace_Record(tr);

  /*
  FILE *fp = fopen("../data/test.bn", "w");
  Trace_Record_Fwrite(tr, fp);
  fclose(fp);
  */

  Trace_Record *tr2 = New_Trace_Record();

  FILE *fp2 = fopen("../data/test.bn", "r");
  Trace_Record_Fread(tr2, fp2);
  fclose(fp2);
  
  Print_Trace_Record(tr2);

  char *ct1 = (char*) tr;
  char *ct2 = (char*) tr2;
  
  int i;
  for (i = 0; i < sizeof(Trace_Record); i++) {
    if (ct1[i] != ct2[i]) {
      printf("Possible bug.\n");
      break;
    }
  }

#endif

#if 0
  Trace_Record *tr = New_Trace_Record();
  tr->mask = ALL_ON_BIT_MASK;
  
  tr->fs.n = 1;
  tr->fs.options[0] = 1;
  tr->fs.scores[0] = 0.3;
  tr->hit_region = 1;
  tr->index = 2;
  tr->refit = 1;

  Local_Neuroseg *locseg = 
    Local_Neuroseg_Import_Xml("../data/locseg.xml", NULL);

  Locseg_Node *node = Make_Locseg_Node(locseg, tr);
  Print_Locseg_Node(node);

  
  FILE *fp = fopen("../data/test.bn", "w");
  Locseg_Node_Fwrite(node, fp);
  fclose(fp);
  

  Locseg_Node *node2 = New_Locseg_Node();

  FILE *fp2 = fopen("../data/test.bn", "r");
  Locseg_Node_Fread(node2, fp2);
  fclose(fp2);

  Print_Locseg_Node(node2);
#endif

#if 0 /* Test what's going on during end tuning */
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain7.tb");
  Stack *signal = Read_Stack("../data/diadem_e3.tif");
  double z_scale = 1.0;

  Trace_Workspace *tw = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(tw, signal);
  tw->refit = FALSE;
  tw->tune_end = TRUE;

  tw->fit_workspace = New_Locseg_Fit_Workspace();  

  tw->trace_step = 0.1;
  //fw->sws->field_func = Neurofield_S1;

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = signal;
  ws->sratio = 1.0;
  ws->sdiff = 0.0;
  ws->option = 6;
  //ws->flag = 0;

  tw->trace_mask = Make_Stack(GREY, signal->width, signal->height, 
			      signal->depth);
  Zero_Stack(tw->trace_mask);

  tw->chain_id = 28;

  int i;
  char file_path[100];
  for (i = 0; i < tw->chain_id; i++) {
    sprintf(file_path, "../data/diadem_e3/chain%d.tb", i);
    if (fexist(file_path)) {
      printf("id: %d\n", i);
      Locseg_Chain *chain = Read_Locseg_Chain(file_path);
      //ws->value = i + 1;
      Locseg_Chain_Label_W(chain, tw->trace_mask, 1.0, 
			   0, Locseg_Chain_Length(chain) - 1, 
			   ws); 
    }
  }
  
  ws->option = 7;
  Locseg_Chain_Label_W(chain, tw->trace_mask, 1.0, 
		       0, Locseg_Chain_Length(chain) - 1, 
		       ws); 
      
#  if 1
  tw->trace_status[0] = TRACE_NORMAL;
  tw->trace_status[1] = TRACE_NORMAL;
  Trace_Locseg(signal, 1.0, chain, tw);
#  endif

  Locseg_Chain_Down_Sample(chain);
      
  Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_HEAD);
  Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_TAIL);
  
  if (Locseg_Chain_Length(chain) > 0) {
    Write_Locseg_Chain("../data/test.tb", chain);
  }
#endif

#if 0
  Stack *stack = 
    Read_Stack("../data/benchmark/stack_graph/neuroseg/cross_60_8.tif");

  Locseg_Chain *chain = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/neuroseg/seg0.tb");
  Print_Locseg_Chain(chain);

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;
  sgw->group_mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(sgw->group_mask);
  
  Locseg_Chain_Label_G(chain, sgw->group_mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);

  //sgw->group_mask = NULL;

  int start[3] = {66, 21, 58};
  int end[3] = {49, 48, 49};
  //int end[3] = {13, 49, 50};

  //int start[3] = {66, 21, 58};
  //int end[3] = {63, 26, 58};

  Stack_Graph_Workspace_Set_Range(sgw, start[0], end[0], 
				  start[1], end[1], 
				  start[2], end[2]);

  sgw->sp_option = 1;
  Int_Arraylist *offset_path = 
    Stack_Route(stack, start, end, sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
    
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  //Write_Stack("../data/presentation9/cross_path.tif", canvas); 
  Write_Stack("../data/test.tif", canvas); 
#endif

#if 1
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain1.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain0.tb");

  double pos[3];
  Locseg_Chain_End_Pos(source, DL_TAIL, pos);

  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index, NULL);
  
  printf("dist: %g\n", dist);

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_R;
  sgw->group_mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(sgw->group_mask);

  int start, end;
  start = seg_index - 1;
  end = seg_index + 2;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  printf("seg index: %d--%d\n", start, end);
  
  ws->flag = 0;
  ws->value = 1;
  Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]); 
  
  int start_pos[3];
  int end_pos[3];
  
  start_pos[0] = (int) pos[0];
  start_pos[1] = (int) pos[1];
  start_pos[2] = (int) pos[2];

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  sgw->sp_option = 1;
  Int_Arraylist *offset_path = 
    Stack_Route(stack, start_pos, end_pos, sgw);
  
  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
    
  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  //Write_Stack("../data/presentation9/fork_path.tif", canvas);
  Write_Stack("../data/test.tif", canvas);

#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e1/chain7.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  double pos[3];
  Locseg_Chain_End_Pos(source, DL_TAIL, pos);

  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index);
  
  printf("dist: %g\n", dist);

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->group_mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(sgw->group_mask);

  int start, end;
  start = seg_index - 2;
  end = seg_index + 2;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  printf("seg index: %d--%d\n", start, end);
  
  ws->flag = 0;
  ws->value = 1;
  Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]); 
  
  int start_pos[3];
  int end_pos[3];
  
  start_pos[0] = (int) pos[0];
  start_pos[1] = (int) pos[1];
  start_pos[2] = (int) pos[2];

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start_pos, end_pos, sgw);
  
  printf("dist: %g\n", sgw->value);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  Int_Arraylist *path_value = Make_Int_Arraylist(0, 1);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      printf("%d-(%d)->", index, stack->array[index]);
      Int_Arraylist_Add(path_value, stack->array[index]);
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
  printf("\n");
    
  iarray_write("../data/presentation9/diadem_e1/path_value_ws.bn", 
	       path_value->array, path_value->length - 1);
	       

  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  Write_Stack("../data/presentation9/diadem_e1_path_ws.tif", canvas);

#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e1/chain22.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  //double pos[3] = {378, 183, 38};
  //Locseg_Chain_End_Pos(source, DL_TAIL, pos);

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  double pos[3];
  Locseg_Chain_Bright_End(source, DL_HEAD, stack, 1.0, pos);

  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index);
  
  printf("dist: %g\n", dist);

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->group_mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(sgw->group_mask);

  int start, end;
  start = seg_index - 1;
  end = seg_index + 2;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  printf("seg index: %d--%d\n", start, end);
  
  ws->flag = 0;
  ws->value = 1;
  Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]); 
  
  int start_pos[3];
  int end_pos[3];
  
  start_pos[0] = (int) pos[0];
  start_pos[1] = (int) pos[1];
  start_pos[2] = (int) pos[2];

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start_pos, end_pos, sgw);
  
  printf("dist: %g\n", sgw->value);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  Int_Arraylist *path_value = Make_Int_Arraylist(0, 1);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      printf("%d-(%d)->", index, stack->array[index]);
      Int_Arraylist_Add(path_value, stack->array[index]);
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
  printf("\n");
    
  iarray_write("../data/presentation9/diadem_e1/path_value_gap_ws.bn", 
	       path_value->array, path_value->length - 1);
	       

  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  Write_Stack("../data/presentation9/diadem_e1_path_gap_ws.tif", canvas);

#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e1/chain22.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  double pos[3];
  Locseg_Chain_Bright_End(source, DL_HEAD, stack, 1.0, pos);

  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index);
  
  printf("dist: %g\n", dist);

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();  
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->group_mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(sgw->group_mask);

  int start, end;
  start = seg_index - 2;
  end = seg_index + 2;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  printf("seg index: %d--%d\n", start, end);
  
  ws->flag = 0;
  ws->value = 1;
  Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]); 
  
  int start_pos[3];
  int end_pos[3];
  
  start_pos[0] = (int) pos[0];
  start_pos[1] = (int) pos[1];
  start_pos[2] = (int) pos[2];

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  Int_Arraylist *offset_path = 
    Stack_Route(stack, start_pos, end_pos, sgw);
  
  printf("dist: %g\n", sgw->value);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Image_Array ima;
  ima.array = canvas->array;

  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  Int_Arraylist *path_value = Make_Int_Arraylist(0, 1);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      printf("%d-(%d)->", index, stack->array[index]);
      Int_Arraylist_Add(path_value, stack->array[index]);
      ima.arrayc[index][0] = 255;
      ima.arrayc[index][1] = 0;
      ima.arrayc[index][2] = 0;
    }
  }
  printf("\n");
  
  /*  
  iarray_write("../data/presentation9/diadem_e1/path_value_ws.bn", 
	       path_value->array, path_value->length - 1);
  */	       

  Kill_Stack_Graph_Workspace(sgw);
  Kill_Int_Arraylist(offset_path);

  Write_Stack("../data/test.tif", canvas);

#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e3/chain14.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e3/chain0.tb");

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->resolution[2] = 3.03;
  sgw->signal_mask = 
    Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(sgw->signal_mask);

  Locseg_Chain_Erase(source, sgw->signal_mask, 1.0);
  Locseg_Chain_Erase(target, sgw->signal_mask, 1.0);
  
  Local_Neuroseg *source_seg = Locseg_Chain_Tail_Seg(source);
  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, stack,
					   1.0, sgw);

  /*  
  Stack *substack = Crop_Stack(new_stack, sgw->range[0], sgw->range[2], 
			       sgw->range[4], 
			       sgw->range[1] - sgw->range[0] + 1, 
			       sgw->range[3] - sgw->range[2] + 1,
			       sgw->range[5] - sgw->range[4] + 1, NULL);
  Kill_Stack(new_stack);

  double c1, c2;
  int thre = Stack_Threshold_RC_R(substack, 0, 255, &c1, &c2);

  sgw->argv[3] = thre;
  sgw->argv[4] = c2 - c1;
  if (sgw->argv[4] < 1.0) {
    sgw->argv[4] = 1.0;
  }
  sgw->argv[4] /= 9.2;
  */
  Int_Arraylist *path = Locseg_Chain_Shortest_Path(source, target, stack, 1.0,
						   sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Kill_Stack_Graph_Workspace(sgw);
  
  Image_Array ima;
  ima.array = canvas->array;

  int i;
  for (i = 0; i < path->length; i++) {
    int index = path->array[i];
    ima.arrayc[index][0] = stack->array[i];
    ima.arrayc[index][1] = 128;
    ima.arrayc[index][2] = 128;
  }

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  Kill_Stack_Graph_Workspace(sgw);
#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e3/chain13.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e3/chain8.tb");

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->resolution[0] = 0.33;
  sgw->resolution[1] = 0.33;
  sgw->resolution[2] = 1.00;

  /*
  Int_Arraylist *path = Locseg_Chain_Shortest_Path(source, target, stack, 1.0,
						   sgw);
  */

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->dist_thre = NEUROSEG_DEFAULT_H * 1.5;
  ctw->mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(ctw->mask);
  Locseg_Chain_Erase(source, ctw->mask, 1.0);
  Locseg_Chain_Erase(target, ctw->mask, 1.0);

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  Locseg_Chain_Connection_Test(source, target, stack, 1.0, &conn, ctw);
  Print_Neurocomp_Conn(&conn);

  Kill_Locseg_Chain(source);
  Kill_Locseg_Chain(target);
  Kill_Stack(stack);
  Kill_Connection_Test_Workspace(ctw);
  
  Kill_Stack_Graph_Workspace(sgw);
  
#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e1/chain7.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Local_Neuroseg *source_seg = Locseg_Chain_Tail_Seg(source);
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, stack,
					   1.0, sgw);
  Int_Arraylist *path = Locseg_Chain_Shortest_Path(source, target, stack, 1.0,
						   sgw);
  Locseg_Chain *chain2 = Locseg_Chain_Bridge_Sp(source_seg, target,
                                                stack, 1.0, sgw, NULL);

  Print_Locseg_Chain(chain2);
#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e3/chain1.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e3/chain0.tb");

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->resolution[2] = 3.03;
  sgw->signal_mask = 
    Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(sgw->signal_mask);

  Locseg_Chain_Erase(source, sgw->signal_mask, 1.0);
  Locseg_Chain_Erase(target, sgw->signal_mask, 1.0);
  
  Local_Neuroseg *source_seg = Locseg_Chain_Tail_Seg(source);
  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, stack,
					   1.0, sgw);

  printf("%g, %g\n", sgw->argv[3], sgw->argv[4]);

  Int_Arraylist *path = Locseg_Chain_Shortest_Path(source, target, stack, 
							1.0, sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 4.0);
  }

  Kill_Stack_Graph_Workspace(sgw);
  
  Image_Array ima;
  ima.array = canvas->array;

  int i;
  for (i = 0; i < path->length; i++) {
    int index = path->array[i];
    ima.arrayc[index][0] = stack->array[index];
    ima.arrayc[index][1] = 128;
    ima.arrayc[index][2] = 128;
  }

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
    Locseg_Chain *source = 
    Read_Locseg_Chain("../data/diadem_e3/chain3.tb");

  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/diadem_e3/chain1.tb");

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->resolution[2] = 3.03;
  sgw->signal_mask = 
    Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(sgw->signal_mask);

  Locseg_Chain_Erase(source, sgw->signal_mask, 1.0);
  Locseg_Chain_Erase(target, sgw->signal_mask, 1.0);
  
  Local_Neuroseg *source_seg = Locseg_Chain_Tail_Seg(source);
  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, stack,
					   1.0, sgw);

  printf("%g, %g\n", sgw->argv[3], sgw->argv[4]);

  Int_Arraylist *path = Locseg_Chain_Shortest_Path(source, target, stack, 
							1.0, sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  if (sgw->group_mask != NULL) {
    Stack_Blend_Mc(canvas, sgw->group_mask, 3.0);
  }

  Kill_Stack_Graph_Workspace(sgw);
  
  Image_Array ima;
  ima.array = canvas->array;

  int i;
  for (i = 0; i < path->length; i++) {
    int index = path->array[i];
    ima.arrayc[index][0] = stack->array[index];
    ima.arrayc[index][1] = 255;
    ima.arrayc[index][2] = 128;
  }

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/fork2/fork2.tif");
  int z = 34;
  stack_adjust_zpos(stack, 50, 44, &z);
  printf("%d\n", z);
#endif

  return 0;
}
