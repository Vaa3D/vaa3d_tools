#include "nsfunc.h"


nsuint ns_pointer_hash_func( nsconstpointer o )
	{  return NS_POINTER_TO_UINT( o );  }


nsboolean ns_pointer_equal_func( nsconstpointer o1, nsconstpointer o2 )
	{  return o1 == o2;  }
