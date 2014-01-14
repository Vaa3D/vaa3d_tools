/* tz_bitmask_utils.h
 *
 * 22-Jan-2008 Initial write Ting Zhao
 */

#include <stdio.h>
#include "tz_error.h"
#include "tz_bitmask.h"

INIT_EXCEPTION

/* Print_Bitmask(): Print a bit mask.
 *
 * Args: mask - mask to print.
 *
 * Return: void. 
 */
void Print_Bitmask(Bitmask_t mask)
{
  int i;
  Bitmask_t test_mask = LAST_BIT_MASK;
  for (i = 0; i < MASK_BIT_NUMBER; i++) {
    printf("%d", (mask & test_mask) > 0);
    test_mask >>= 1;
  }
  printf("\n");
}

void Bitmask_Set_Bit(int bit, BOOL value, Bitmask_t *mask)
{
  Bitmask_t switch_mask = FIRST_BIT_MASK;
  switch_mask <<= bit;

  if (value == TRUE) {    
    *mask |= switch_mask;
  } else {
    *mask &= ~switch_mask;
  }
}

BOOL Bitmask_Get_Bit(Bitmask_t mask, int bit)
{
  mask >>= bit;
  if ((mask & FIRST_BIT_MASK) > 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

int Bitmask_To_Index(Bitmask_t mask, int length, int index[])
{
  if (length > MASK_BIT_NUMBER) {
    length = MASK_BIT_NUMBER;
  }

  int i;
  int n = 0;
  for (i = 0; i < length; i++) {
    if (FIRST_BIT_MASK & mask) {
      index[n++] = i;
    }
    mask >>= 1;
  }

  return n;
}

Bitmask_t Index_To_Bitmask(const int index[], int length)
{
  int i;
  Bitmask_t mask = ZERO_BIT_MASK;

  for (i = 0; i < length; i++) {
    Bitmask_Set_Bit(index[i], TRUE, &mask);
  }
  
  return mask;
}
