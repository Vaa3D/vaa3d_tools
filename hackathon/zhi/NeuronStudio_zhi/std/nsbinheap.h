#ifndef __NS_STD_BIN_HEAP_H__
#define __NS_STD_BIN_HEAP_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nsbytearray.h>

NS_DECLS_BEGIN

/* Internal. DO NOT USE! */
typedef struct _NsBinHeapEntry
	{
	nspointer   object;
	nspointer  *handle;
	}
	NsBinHeapEntry;


typedef struct _NsBinHeap
	{
	NsByteArray           buffer;
	NsBinHeapEntry       *array;
	nssize                size;
	nssize                capacity;
	NsBinaryBooleanFunc   order_func;
	NsFinalizeFunc        finalize_func;
	}
	NsBinHeap;


NS_IMPEXP NsError ns_bin_heap_construct
	( 
	NsBinHeap            *heap,
	nssize                capacity,
	NsBinaryBooleanFunc   order_func,
	NsFinalizeFunc        finalize_func
	);

NS_IMPEXP void ns_bin_heap_destruct( NsBinHeap *heap );

NS_IMPEXP nssize ns_bin_heap_size( const NsBinHeap *heap );  /* Current # of elements */
NS_IMPEXP nssize ns_bin_heap_capacity( const NsBinHeap *heap ); /* Max # of elements before resizing */

NS_IMPEXP nsboolean ns_bin_heap_is_empty( const NsBinHeap *heap );

NS_IMPEXP NsBinaryBooleanFunc ns_bin_heap_get_order_func( const NsBinHeap *heap );
NS_IMPEXP void ns_bin_heap_set_order_func( NsBinHeap *heap, NsBinaryBooleanFunc order_func );

NS_IMPEXP NsFinalizeFunc ns_bin_heap_get_finalize_func( const NsBinHeap *heap );
NS_IMPEXP void ns_bin_heap_set_finalize_func( NsBinHeap *heap, NsFinalizeFunc finalize_func );

NS_IMPEXP void ns_bin_heap_clear( NsBinHeap *heap );

/* NOTE: push() sets a "handle" for use in reset() only! */
NS_IMPEXP NsError ns_bin_heap_push( NsBinHeap *heap, nspointer object, nspointer *handle );
NS_IMPEXP void ns_bin_heap_pop( NsBinHeap *heap );
NS_IMPEXP nspointer ns_bin_heap_peek( const NsBinHeap *heap );

/* Re-heap an object whose "key" value may have changed. */
NS_IMPEXP void ns_bin_heap_repush( NsBinHeap *heap, nspointer object, nspointer *handle );

NS_DECLS_END

#endif/* __NS_STD_BIN_HEAP_H__ */
