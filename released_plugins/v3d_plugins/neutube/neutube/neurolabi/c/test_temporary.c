#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "tz_string.h"
#include "tz_swc_tree.h"
#include "tz_workspace.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_graph.h"
#include "tz_math.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_image_io.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"
#include "tz_apo.h"


int main(int argc, char *argv[])
{
  Stack *stack = Read_Raw_Stack_C("/Users/feng/enhanced.raw", 0);
  size_t start = (size_t)stack->height * stack->width * stack->kind * 85;
  for (; start < (size_t)stack->height * stack->width * stack->kind * 95; start++) {
    if (*(uint8*)(stack->array+start)  != 0)
      printf("%d\n",*(uint8*)(stack->array+start) );
  }
  Write_Stack_U("/Users/feng/enhanced1.raw", stack, NULL);
  return 0;
  
}