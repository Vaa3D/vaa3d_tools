/* file teststitch.c
 * @brief >>stitching test
 * @author Ting Zhao
 * @date 11-Mar-2009
 */

#include <utilities.h>
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "tz_random.h"
#include "tz_iarray.h"

int main(int argc, char *argv[])
{
#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");
  
  Stack *stack1 = Crop_Stack(stack, 0, 0, 0, 200, 300, 100, NULL);
  Stack *stack2 = Crop_Stack(stack, 100, 200, 10, 300, 200, 100, NULL);
  
  int i;
  int nvoxel = Stack_Voxel_Number(stack1);
  double mu = 20.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += stack2->array[i];
    if (noise > 255) {
      noise = 255;
    }
    stack2->array[i] = noise;

    noise = Poissonrnd(10);

    noise += stack1->array[i];
    if (noise > 255) {
      noise = 255;
    }
    stack1->array[i] = noise;
  }

  Write_Stack("../data/benchmark/stitch/fly_neuron_n1/fl_neuron_n1_part1.tif", 
	      stack1);
  Write_Stack("../data/benchmark/stitch/fly_neuron_n1/fl_neuron_n1_part2.tif", 
	      stack2);
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/neurolineage/lsm/63_61LHregion.lsm", -1);
  Stack *stack1 = Crop_Stack(stack, 0, 0, 0, 200, 300, 100, NULL);
  Stack *stack2 = Crop_Stack(stack, 100, 200, 10, 300, 200, 100, NULL);
  
  Write_Stack("../data/benchmark/stitch/63_61LHregion/63_61LHregion_part1.tif", 
	      stack1);
  Write_Stack("../data/benchmark/stitch/63_61LHregion/63_61LHregion_part2.tif", 
	      stack2);
#endif
  
#if 1
  int nstack = 2;
  Mc_Stack **stacks = (Mc_Stack **) malloc(sizeof(Mc_Stack*) * nstack);
  const char* filepath[2] = {"../data/diadem_c1_01.xml", 
			     "../data/diadem_c1_02.xml"};
  
  int i;
  for (i = 0; i < nstack; i++) {
    stacks[i] = Read_Mc_Stack(filepath[i], -1);
  }

  int **final_offset;

  final_offset = malloc(sizeof(int*) * nstack);
  for(i=0;i<nstack;i++) {
    final_offset[i] = iarray_calloc(3);
  }
  
  final_offset[1][0] = 453;
  final_offset[1][1] = -23;
  final_offset[1][2] = 16;

  Mc_Stack *new_stack = Mc_Stack_Merge(stacks, nstack, final_offset, 3);
  
  Write_Mc_Stack("../data/test.tif", new_stack, NULL);
#endif

  return 0;
}
