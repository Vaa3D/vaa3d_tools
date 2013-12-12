/**file teststacktile.c
 * @author Ting Zhao
 * @date 17-Nov-2009
 */

#include <utilities.h>
#include "tz_stack_tile.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 1
  Stack_Tile *tile = Read_Stack_Tile("../data/diadem_d1_003.xml");
  Print_Stack_Tile(tile);
#endif
  
  return 0;
}
