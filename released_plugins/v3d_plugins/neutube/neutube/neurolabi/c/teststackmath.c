#include "tz_stack_math.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"

int main()
{
#if 1
  Stack *stack1 = Make_Stack(GREY, 5, 5, 5);
  Stack *stack2 = Make_Stack(GREY, 5, 5, 5);
  
  int nvoxel = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < nvoxel; i++) {
    stack1->array[i] = i;
    stack2->array[i] = i + 1;
  }

  int range[6] = {1, 0, 0, 3, 2, 3};

  Stack *out = Stack_Sub_R(stack2, stack1, range, stack1);

  Print_Stack_Value(out);

#endif

  return 0;
}
