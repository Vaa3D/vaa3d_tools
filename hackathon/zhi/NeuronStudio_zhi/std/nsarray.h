#ifndef __NS_STD_ARRAY_H__
#define __NS_STD_ARRAY_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsArray
	{
	nssize      size;
	nspointer  *objects;
	}
	NsArray;


typedef nspointer*   nsarrayiter;
typedef nsarrayiter  nsarrayreviter;


#define NS_ARRAY_FOREACH( array, iter )\
	NS_TYPE_FOREACH(\
		(array),\
		(iter),\
		ns_array_begin,\
		ns_array_end,\
		ns_array_iter_next,\
		ns_array_iter_not_equal\
		)


NS_IMPEXP NsError ns_array_construct( NsArray *array, nssize size );
NS_IMPEXP void ns_array_destruct( NsArray *array );

NS_IMPEXP nssize ns_array_size( const NsArray *array );

NS_IMPEXP void ns_array_clear( NsArray *array );

/* If size is zero then the array is made NULL and no error occurs. */
NS_IMPEXP NsError ns_array_resize( NsArray *array, nssize size );

NS_IMPEXP void ns_array_memzero( NsArray *array );

NS_IMPEXP nsarrayreviter ns_array_rev_begin( const NsArray *array );
NS_IMPEXP nsarrayreviter ns_array_rev_end( const NsArray *array );

NS_IMPEXP nsarrayiter ns_array_at( const NsArray *array, nssize index );

#define ns_array_begin_ex( array )\
	( (array)->objects )

#define ns_array_end_ex( array )\
	( (array)->objects + (array)->size )

#define ns_array_get_object( array, index )\
   ( (array)->objects[ (index) ] )

#define ns_array_set_object( array, index, object )\
   ( (array)->objects[ (index) ] = (object) )

#define ns_array_iter_next( I )\
   ( (I) + 1 )

#define ns_array_iter_prev( I )\
   ( (I) - 1 )

#define ns_array_iter_offset( I, offset )\
   ( (I) + (offset) )

#define ns_array_iter_get_object( I )\
   ( *(I) )

#define ns_array_iter_set_object( I, object )\
   ( *(I) = (object) )

#define ns_array_iter_equal( I1, I2 )\
	( (I1) == (I2) )

#define ns_array_iter_less( I1, I2 )\
	( (I1) < (I2) )

#define ns_array_rev_iter_get_object( I )\
	ns_array_iter_get_object( (I).iter )

#define ns_array_rev_iter_set_object( I, obj )\
	ns_array_iter_set_object( (I).iter, (obj) )

#define ns_array_rev_iter_equal( I1, I2 )\
	ns_array_iter_equal( (I1).iter, (I2).iter )

#define ns_array_iter_not_equal( I1, I2 )\
	( ! ns_array_iter_equal( (I1), (I2) ) )

#define ns_array_iter_less_equal( I1, I2 )\
   ( ns_array_iter_less( (I1), (I2) ) || ns_array_iter_equal( (I1), (I2) ) )

#define ns_array_iter_greater( I1, I2 )\
   ( ! ns_array_iter_less_equal( (I1), (I2) ) )

#define ns_array_iter_greater_equal( I1, I2 )\
	( ! ns_array_iter_less( (I1), (I2) ) )

#define ns_array_rev_iter_not_equal( I1, I2 )\
	( ! ns_array_rev_iter_equal( (I1), (I2) ) )

#define ns_array_rev_iter_next( I )\
	ns_array_iter_prev( (I) )

#define ns_array_rev_iter_prev( I )\
	ns_array_iter_next( (I) )

#define ns_array_rev_iter_offset( I, offset )\
	ns_array_iter_offset( (I), -(offset) )

NS_DECLS_END

#endif/* __NS_STD_ARRAY_H__ */
