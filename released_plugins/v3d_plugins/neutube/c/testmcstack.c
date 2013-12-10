/* file testmcstack.c
 * @author Ting Zhao
 * @date 01-Jan-2010
 */

#include "tz_utilities.h"
#include "tz_mc_stack.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 1
  Mc_Stack *stack = Read_Mc_Stack("/Users/zhaot/Data/hanchuan/HP_091123_1_L1_Sum.lsm", -1);
  Mc_Stack *proj = Mc_Stack_Mip(stack);

  Write_Mc_Stack("../data/test.tif", proj, NULL);
    
#endif

  return 0;
}
