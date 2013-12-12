#include "tz_json.h"


void json_info()
{
#if defined(HAVE_LIBJANSSON)
  printf("Using libjansson.\n");
#else
  printf("No json function is available.\n");
#endif
}


