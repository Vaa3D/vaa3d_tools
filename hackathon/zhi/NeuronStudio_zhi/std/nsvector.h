#ifndef __NS_STD_VECTOR_H__
#define __NS_STD_VECTOR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>

NS_DECLS_BEGIN

typedef struct _NsVector
	{
	nspointer       *first;
	nspointer       *last;
	nspointer       *term;
	NsFinalizeFunc   finalize_func;
	}
	NsVector;


typedef nspointer*    nsvectoriter;
typedef nsvectoriter  nsvectorreviter;


#define NS_VECTOR_FOREACH( list, iter )\
	NS_TYPE_FOREACH(\
		(list),\
		(iter),\
		ns_vector_begin,\
		ns_vector_end,\
		ns_vector_iter_next,\
		ns_vector_iter_not_equal\
		)


/* Pass NULL for 'finalize_func' if not needed. */
NS_IMPEXP NsError ns_vector_construct
	(
	NsVector        *vector,
	nssize           n,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_vector_destruct( NsVector *vector );

/* IMPORTANT: The vector array is free'd! */
NS_IMPEXP void ns_vector_clear( NsVector *vector );

/* Expands the vector size if 'n' is > ns_vector_capacity(). 
	NOTE: Iterators can become invalid after call! */
NS_IMPEXP NsError ns_vector_reserve( NsVector *vector, nssize n );

/* The size of the sequence stored in the vector, which will
	always be <= ns_vector_capacity(). */
NS_IMPEXP nssize ns_vector_size( const NsVector *vector );

/* The size of the vector. */
NS_IMPEXP nssize ns_vector_capacity( const NsVector *vector );

NS_IMPEXP nsboolean ns_vector_is_empty( const NsVector *vector );

/* NOTE: Iterators can become invalid after call! */
NS_IMPEXP NsError ns_vector_resize( NsVector *vector, nssize n );

NS_IMPEXP nsvectoriter ns_vector_begin( const NsVector *vector );
NS_IMPEXP nsvectoriter ns_vector_end( const NsVector *vector );

NS_IMPEXP nsvectorreviter ns_vector_rev_begin( const NsVector *vector );
NS_IMPEXP nsvectorreviter ns_vector_rev_end( const NsVector *vector );

NS_IMPEXP nsvectoriter ns_vector_at( const NsVector *vector, nssize index );

NS_IMPEXP nspointer ns_vector_peek_front( const NsVector *vector );
NS_IMPEXP nspointer ns_vector_peek_back( const NsVector *vector );

NS_IMPEXP NsError ns_vector_push_back( NsVector *vector, nspointer object );
NS_IMPEXP void ns_vector_pop_back( NsVector *vector );

/* Inserts the 'object' before the object pointer to by 'I'. */
NS_IMPEXP NsError ns_vector_insert( NsVector *vector, nsvectoriter I, nspointer object );

NS_IMPEXP void ns_vector_erase( NsVector *vector, nsvectoriter I );

NS_IMPEXP void ns_vector_erase_all( NsVector *vector, nsvectoriter first, nsvectoriter last );

NS_IMPEXP nspointer ns_vector_get_object( const NsVector *vector, nssize index );
NS_IMPEXP void ns_vector_set_object( NsVector *vector, nssize index, nspointer object );

#define ns_vector_iter_next( I )\
   ( (I) + 1 )

#define ns_vector_iter_prev( I )\
   ( (I) - 1 )

#define ns_vector_iter_offset( I, offset )\
   ( (I) + (offset) )

#define ns_vector_iter_get_object( I )\
   ( *(I) )

#define ns_vector_iter_set_object( I, object )\
   ( *(I) = (object) )

#define ns_vector_iter_equal( I1, I2 )\
	( (I1) == (I2) )

#define ns_vector_iter_less( I1, I2 )\
	( (I1) < (I2) )

#define ns_vector_iter_not_equal( I1, I2 )\
	( ! ns_vector_iter_equal( I1, I2 ) )

#define ns_vector_rev_iter_next( I )\
	ns_vector_iter_prev( ( I ) )

#define ns_vector_rev_iter_prev( I )\
	ns_vector_iter_next( ( I ) )

#define ns_vector_rev_iter_offset( I, offset )\
	ns_vector_iter_offset( (I), -(offset) )

#define ns_vector_rev_iter_get_object( I )\
	ns_vector_iter_get_object( ( I ) )

#define ns_vector_rev_iter_set_object( I, obj )\
	ns_vector_iter_set_object( ( I ), ( obj ) )

#define ns_vector_rev_iter_equal( I1, I2 )\
	ns_vector_iter_equal( ( I1 ), ( I2 ) )

#define ns_vector_rev_iter_not_equal( I1, I2 )\
	ns_vector_iter_not_equal( ( I1 ), ( I2 ) )

NS_DECLS_END

#endif/* __NS_STD_VECTOR_H__ */
