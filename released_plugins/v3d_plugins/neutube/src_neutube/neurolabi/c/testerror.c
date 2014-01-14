/* testbitmask.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_error.h"

INIT_EXCEPTION_MAIN(e)

int main()
{
#if 0
  PRINT_EXCEPTION("error", "exception thrown");

  printf("%d\n", Get_Error_Code());
  TRACE(tz_errmsg(Get_Error_Code()));
#endif

#if 1
  printf("%s\n", tz_errmsg(ERROR_OTHER));
  printf("%s\n", tz_errmsg(EXCEPTION_THROWN));
  printf("%s\n", tz_errmsg(ERROR_DATA_TYPE));
#endif

  return 0;
}
