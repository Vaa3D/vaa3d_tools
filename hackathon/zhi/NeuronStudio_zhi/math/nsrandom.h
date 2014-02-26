#ifndef __NS_MATH_RANDOM_H__
#define __NS_MATH_RANDOM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>

NS_DECLS_BEGIN

#define NS_RANDI_MAX  0x7fff

typedef struct _NsRandi
	{
   nsuint32 seed;
	}
	NsRandi;


NS_IMPEXP void ns_randi_init( NsRandi *r, nsuint32 seed );

NS_IMPEXP void ns_randi_seed( NsRandi *r, nsuint32 seed );

NS_IMPEXP nsuint16 ns_randi( NsRandi *r );

NS_IMPEXP nsuint16 ns_randi_ranged( NsRandi *r, nsuint16 min, nsuint16 max );


NS_IMPEXP void ns_srand( nsuint32 seed );
NS_IMPEXP nsuint16 ns_rand( void );


#define _NS_RANDF_N  99

typedef struct _NsRandf
	{
   nsulong table[ _NS_RANDF_N ];
	}
	NsRandf;


/* 'shuffle' indicates how many time ns_randf() is called
	before this function returns. */
NS_IMPEXP void ns_randf_init( NsRandf *r, nssize shuffle );

/* Returns value in the range [0,1] */
NS_IMPEXP nsfloat ns_randf( NsRandf *r );


NS_IMPEXP void ns_global_randi_init( nsuint32 seed );
NS_IMPEXP nsuint16 ns_global_randi( void );
NS_IMPEXP nsuint16 ns_global_randi_ranged( nsuint16 min, nsuint16 max );

NS_IMPEXP void ns_global_randf_init( nssize shuffle );
NS_IMPEXP nsfloat ns_global_randf( void );

NS_DECLS_END

#endif/* __NS_MATH_RANDOM_H__ */
