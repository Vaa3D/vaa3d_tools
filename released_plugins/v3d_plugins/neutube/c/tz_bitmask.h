/**@file tz_bitmask.h
 * @brief bit mask
 * @author Ting Zhao
 * @date 22-Jan-2008
 */

#ifndef _TZ_BITMASK_UTILS_H_
#define _TZ_BITMASK_UTILS_H_ 

#include "tz_cdefs.h"
#include "tz_utilities.h"

__BEGIN_DECLS

/*
 * It's a 0-indexed system and the lowest bit has index 0.
 */

#define MASK_BIT_NUMBER 32         /* Number of bits of a mask. 
				    * This also determines how many independent
				    * binary switchs are available in one 
				    * mask variable. */
#define FIRST_BIT_MASK  0x00000001  /* Only the lowest bit is on */
#define LAST_BIT_MASK   0x80000000  /* Only the highest bit is on */
#define ZERO_BIT_MASK   0x00000000  /* All bits are off */
#define ALL_ON_BIT_MASK 0xFFFFFFFF  /* All bits are on  */

typedef tz_uint32 Bitmask_t;
#define bitmask_t Bitmask_t

/**@addtogroup bitmask_ Bit mask (tz_bitmask.h)
 * @{
 */

/**@brief prints a bit mask.
 *
 * Print_Bitmask prints a bit mask to the standard output.
 */
void Print_Bitmask(Bitmask_t mask);

/**@brief sets a bit.
 *
 * Bitmask_Set_Bit() sets a bit in mask to <value>. 
 */
void Bitmask_Set_Bit(int bit, BOOL value, Bitmask_t *mask);

/**@brief gets a bit.
 *
 * Bitmask_Get_Bit() returns the value of a bit in <mask>. The value is TRUE 
 * or FALSE.
 */
BOOL Bitmask_Get_Bit(Bitmask_t mask, int bit);

/**@brief converts a bit mask to an index array.
 *
 * Bitmask_To_Index() retrieves the bit indices of 1s in a mask and put them
 * into an integer array. <length> is the maximum number of elements in the 
 * array. It returns the number of 1s detected.
 */
int Bitmask_To_Index(Bitmask_t mask, int length, int index[]);

/**@brief converts an index array to a bit mask.
 *
 * Index_To_Bitmask() is the inverse function of Bitmask_To_Index().
 */
Bitmask_t Index_To_Bitmask(const int index[], int length);

/**@}*/

__END_DECLS

#endif
