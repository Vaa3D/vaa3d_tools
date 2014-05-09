#include "nsmath.h"


#if 64 != NS_FLOAT_BITS

nsfloat ns_powf( nsfloat f, nsfloat p )
	{  return ( nsfloat )ns_pow( f, p );  }


nsfloat ns_ceilf( nsfloat f )
	{  return ( nsfloat )ns_ceil( f );  }


nsfloat ns_floorf( nsfloat f )
	{  return ( nsfloat )ns_floor( f );  }


nsfloat ns_sinf( nsfloat f )
	{  return ( nsfloat )ns_sin( f );  }


nsfloat ns_cosf( nsfloat f )
	{  return ( nsfloat )ns_cos( f );  }


nsfloat ns_tanf( nsfloat f )
	{  return ( nsfloat )ns_tan( f );  }


nsfloat ns_asinf( nsfloat f )
	{  return ( nsfloat )ns_asin( f );  }


nsfloat ns_acosf( nsfloat f )
	{  return ( nsfloat )ns_acos( f );  }


nsfloat ns_atan2f( nsfloat x, nsfloat y )
	{  return ( nsfloat )ns_atan2( x, y );  }


nsfloat ns_sqrtf( nsfloat f )
	{  return ( nsfloat )ns_sqrt( f );  }


nsfloat ns_fabsf( nsfloat f )
	{  return ( nsfloat )ns_fabs( f );  }


nsfloat ns_logf( nsfloat f )
	{  return ( nsfloat )ns_log( f );  }


nsfloat ns_log10f( nsfloat f )
	{  return ( nsfloat )ns_log10( f );  }

#endif/* 64 != NS_FLOAT_BITS */
