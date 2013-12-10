#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
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
#include "tz_stack_bwdist.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  Stack *seedimg = Read_Stack("../data/fly_neuron_objseed.tif");
  Object_3d_List *objs = Stack_Find_Object(seedimg, 1, 0);

  Stack *stack2 = NULL;
  Object_3d *obj = NULL;
  stack2 = NULL;

  Object_3d_List *objs2 = NULL; 

  while (objs != NULL) {
    stack2 = Read_Stack("../data/fly_neuron_bundle.tif");
    obj = Stack_Grow_Object(stack2, 1, objs->data->voxels[0]);
    if (obj->size > 1000) {
      if (objs2 == NULL) {
	objs2 = Object_3d_List_New();
	objs2->data = obj;
      } else {
	Object_3d_List_Add(&objs2, obj);
	Print_Object_3d_Info(obj);
      }
    } else {
      Kill_Object_3d(obj);
    }
    objs = objs->next;
  }

  Print_Object_3d_List_Compact(objs2);

  Stack *sig = Read_Stack("../data/fly_neuron.tif");
  //Stack *sig = Read_Stack("../data/fly_neuron.tif");
  Stack *stack = Translate_Stack(sig, COLOR, 0);
  //Stack *stack = Read_Stack("../data/blobimg2.tif");

  Stack *traced = Make_Stack(GREY, sig->width, sig->height, sig->depth);
  One_Stack(traced);

  Stack *soma = Read_Stack("../data/fly_neuron_soma.tif");
  Stack_Threshold(soma, 1);
  Stack_Binarize(soma);
  Stack_Not(soma, soma);
  Stack_And(traced, soma, traced);
  Kill_Stack(soma);

  Neurochain *chain = New_Neurochain();
  Neurochain *chain2 = NULL;
  while (objs2 != NULL) {  
    if (objs2->data->size > 100) {
      Set_Neuroseg(&(chain->seg), 5, 5, 12, -TZ_PI_2, 0.0);
      Set_Position(chain->position, objs2->data->voxels[0][0], 
		   objs2->data->voxels[0][1], objs2->data->voxels[0][2]);

#if 1
      Try {
	Fit_Neuroseg(sig, &(chain->seg), chain->position, TRUE);
      } Catch(e) {
	TZ_ERROR(e);
      }
#endif
      
#if 1
      Try {
	chain2 = Trace_Neuron(sig, chain, BOTH, traced);
      } Catch(e) {
	TZ_ERROR(e);
      }
#endif
      Neurochain_Erase(traced,  chain2);
      chain = Extend_Neurochain(chain2);
    }
    objs2 = objs2->next;
  }

  Neurochain_Label(stack, Neurochain_Head(chain));
  Write_Stack("../data/fly_neuron_trace.tif", stack);

  return 0;
}
