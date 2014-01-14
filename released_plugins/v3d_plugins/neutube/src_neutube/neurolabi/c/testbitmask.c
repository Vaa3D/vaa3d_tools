/* testbitmask.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_bitmask.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    /* example test */
    /* Define a mask with all bits 0. */
    bitmask_t mask = ZERO_BIT_MASK;

    /* Turn on the bit 3 in <mask>.*/
    /* Note: the mask is 0-based indexed. */
    Bitmask_Set_Bit(3, TRUE, &mask);

    /* We can retrieve any bit from the mask. */
    if (Bitmask_Get_Bit(mask, 3) == FALSE) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected bit value\n");
      return 1;
    }

    if (Bitmask_Get_Bit(mask, 1) == TRUE) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected bit value\n");
      return 1;
    }

    /* Turn on another bit. */
    Bitmask_Set_Bit(5, TRUE, &mask);

    /* The bit mask can be turned into an integer array. */
    int index[32];
    int count = Bitmask_To_Index(mask, 32, index);
    
    if (count != 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected number of on-bits\n");
      return 1;
    }

    if ((index[0] != 3) || (index[1] != 5)){
      PRINT_EXCEPTION(":( Bug?", "Unexpected bit index.\n");
      return 1;
    }

    /* We can turn an index array to a bit mask. */
    bitmask_t new_mask = Index_To_Bitmask(index, count);

    if (new_mask != mask) {      
      PRINT_EXCEPTION(":( Bug?", "Unexpected mask.\n");
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  int nvar;
  int index[MASK_BIT_NUMBER];
  nvar = Bitmask_To_Index(65535 * 2, MASK_BIT_NUMBER, index);

  iarray_print2(index, nvar, 1);

  Bitmask_t mask = 65535 * 2;
  Bitmask_Set_Bit(1, FALSE, &mask);

  Print_Bitmask(mask);

  Print_Bitmask(Index_To_Bitmask(index, nvar));

  printf("%d\n", Bitmask_Get_Bit(mask, 2 ));
#endif

  return 0;
}
