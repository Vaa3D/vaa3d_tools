#include "nstypes.h"


typedef union _NsFloat32OrInt32
	{
	nsfloat32  f;
	nsint32    i;
	}
	NsFloat32OrInt32;


nspointer _ns_float32_to_pointer( nsfloat32 f )
	{
	NsFloat32OrInt32 value;

	value.f = f;
	return NS_INT32_TO_POINTER( value.i );
	}


nsfloat32 _ns_pointer_to_float32( nsconstpointer p )
	{
	NsFloat32OrInt32 value;

	value.i = NS_POINTER_TO_INT32( p );
	return value.f;
	}


#if 64 == NS_POINTER_BITS

typedef union _NsFloat64OrInt64
	{
	nsfloat64  f;
	nsint64    i;
	}
	NsFloat64OrInt64;


nspointer _ns_float64_to_pointer( nsfloat64 f )
	{
	NsFloat64OrInt64 value;

	value.f = f;
	return NS_INT64_TO_POINTER( value.i );
	}


nsfloat64 _ns_pointer_to_float64( nsconstpointer p )
	{
	NsFloat64OrInt64 value;

	value.i = NS_POINTER_TO_INT64( p );
	return value.f;
	}

#endif/* 64 == NS_POINTER_BITS */
