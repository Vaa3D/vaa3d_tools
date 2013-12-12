/**@file tz_array.h
 * @brief array operations
 * @author Ting Zhao
 */

#ifndef _TZ_ARRAY_H_
#define _TZ_ARRAY_H_

#include "tz_cdefs.h"
#include "tz_utilities.h"

__BEGIN_DECLS

/**@addtogroup array_opr_ Array operations
 * @{
 */

/**@addtogroup general_array_opr_ General array operations
 * @{
 */

/**@brief Copy an array.
 *
 * arraycpy() copies <length> bytes in array <a2> to array <a1>. <offset> is
 * the started position of the copied bytes in <a2>. If <overlap> is 0, it
 * means the two arrays do not overlap, otherwise the function will think the 
 * arrays may overlap.
 */
void arraycpy(void *a1,void *a2,int offset,int length,int overlap);

/**@brief Print an array.
 *
 * array_print() prints an array in a certain format to the standard output.
 */
void array_print(void* a1,int length,Data_Type_t type);

/**@brief Write an array.
 */
int array_write(const void *a1,int length,const char *filename);

/**@brief Read an array.
 *
 * array_read() read an array to the file with the path <filename>. <length>
 * is the number of bytes to write. array_read() is used to read a file
 * created by array_write() and the length of the array will be stored in
 * <length>. 
 */
int array_read(void *a1,int *length,const char* filename);

/**@brief Write an array with a type.
 *
 * array_writet() writes an array with its type and <length> for this
 * function is the number of elements of the specified type. The written file
 * can be read by array_readt() and all information will be restored in <a1>,
 * <type> and <length>.
 */
int array_writet(const void *a1,Data_Type_t type,int length,
		 const char *filename);

/**@brief Read an array with a type.
 */
int array_readt(void *a1,Data_Type_t *type,int *length,
		 const char *filename);

/**@}*/

/**@}*/

__END_DECLS

#endif
