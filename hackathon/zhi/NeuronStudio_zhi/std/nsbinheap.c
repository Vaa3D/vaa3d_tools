#include "nsbinheap.h"


/* NOTE: The heap's indices are zero-based. */

#define _NS_BIN_HEAP_PARENT( i )\
	( ( (i) - 1 ) >> 1 )

#define _NS_BIN_HEAP_LEFT_CHILD( i )\
	( ( (i) << 1 ) + 1 )

#define _NS_BIN_HEAP_RIGHT_CHILD( i )\
	( ( (i) << 1 ) + 2 )


NS_PRIVATE NsError _ns_bin_heap_resize( NsBinHeap *heap, nssize capacity )
	{
	NsError error;

	if( heap->capacity < capacity )
		{
		if( NS_FAILURE( ns_byte_array_resize(
								&heap->buffer,
								capacity * sizeof( NsBinHeapEntry )
								),
								error ) )
			return error;

		heap->array = 0 < ns_byte_array_size( &heap->buffer ) ?
						  ( NsBinHeapEntry* )ns_byte_array_begin( &heap->buffer ) : NULL;

		heap->capacity = capacity;
		}

	return ns_no_error();
	}


NsError ns_bin_heap_construct
	( 
	NsBinHeap            *heap,
	nssize                capacity,
	NsBinaryBooleanFunc   order_func,
	NsFinalizeFunc        finalize_func
	)
	{
	NsError error;

	ns_assert( NULL != heap );
	ns_assert( NULL != order_func );

	/* NOTE: Assure successful construction of the internal array by
		initially passing 0 as the size. */
	ns_verify( NS_SUCCESS( ns_byte_array_construct( &heap->buffer, 0 ), error ) );
	
	heap->array         = NULL;
	heap->size          = 0;
	heap->capacity      = 0;
	heap->order_func    = order_func;
	heap->finalize_func = finalize_func;

	if( NS_FAILURE( _ns_bin_heap_resize( heap, capacity ), error ) )
		{
		ns_byte_array_destruct( &heap->buffer );
		return error;
		}

	return ns_no_error();
	}


void ns_bin_heap_destruct( NsBinHeap *heap )
   {
	ns_assert( NULL != heap );

	ns_bin_heap_clear( heap );
	ns_byte_array_destruct( &heap->buffer );
	}


nssize ns_bin_heap_size( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	return heap->size;
	}


nssize ns_bin_heap_capacity( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	return ns_byte_array_size( &heap->buffer ) / sizeof( NsBinHeapEntry );
	}


nsboolean ns_bin_heap_is_empty( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	return 0 == ns_bin_heap_size( heap );
	}


NsBinaryBooleanFunc ns_bin_heap_get_order_func( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	return heap->order_func;
	}
	

void ns_bin_heap_set_order_func( NsBinHeap *heap, NsBinaryBooleanFunc order_func )
	{
	ns_assert( NULL != heap );
	ns_assert( NULL != order_func );

	heap->order_func = order_func;
	}


NsFinalizeFunc ns_bin_heap_get_finalize_func( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	return heap->finalize_func;
	}


void ns_bin_heap_set_finalize_func( NsBinHeap *heap, NsFinalizeFunc finalize_func )
	{
	ns_assert( NULL != heap );
	heap->finalize_func = finalize_func;
	}


void ns_bin_heap_clear( NsBinHeap *heap )
	{
	nssize  size;
	nssize  i;


	ns_assert( NULL != heap );

	if( NULL != heap->finalize_func )
		{
		size = ns_bin_heap_size( heap );

		for( i = 0; i < size; ++i )
			( heap->finalize_func )( heap->array[ i ].object );
		}

	heap->size = 0;
	}


NS_PRIVATE void _ns_bin_heap_set( NsBinHeap *heap, nssize index, NsBinHeapEntry entry )
	{
	if( NULL != entry.handle )
		*entry.handle = NS_SIZE_TO_POINTER( index );

	heap->array[ index ] = entry;
	}


NS_PRIVATE void _ns_bin_heap_bubble_up( NsBinHeap *heap, nssize index )
	{
	nssize          curr_index;
	nssize          next_index;
	NsBinHeapEntry  entry;
	NsBinHeapEntry  next_entry;


	ns_assert( NULL != heap );

	entry      = heap->array[ index ];
	curr_index = index;
	
	while( 0 < curr_index )
		{
		next_index = _NS_BIN_HEAP_PARENT( curr_index );
		next_entry = heap->array[ next_index ];

		if( !( ( heap->order_func )( entry.object, next_entry.object ) ) )
			break;

		_ns_bin_heap_set( heap, curr_index, next_entry );

		curr_index = next_index;
		}

	_ns_bin_heap_set( heap, curr_index, entry );
	}


NS_PRIVATE void _ns_bin_heap_bubble_down( NsBinHeap *heap, nssize index )
	{
	nssize          size;
	nssize          curr_index;
	nssize          next_index;
	NsBinHeapEntry  entry;
	NsBinHeapEntry  next_entry;


	ns_assert( NULL != heap );

	size       = ns_bin_heap_size( heap );
	entry      = heap->array[ index ];
	curr_index = index;

	for( ; ( next_index = _NS_BIN_HEAP_LEFT_CHILD( curr_index ) ) < size;
		  curr_index = next_index )
		{
		/* Find larger of the children. Note the +1 advances
			from the left to the right child. */
		if( next_index < size - 1 &&
			 ( heap->order_func )(
					heap->array[ next_index + 1 ].object,
					heap->array[ next_index ].object
					) )
			++next_index;

		next_entry = heap->array[ next_index ];

		if( !( ( heap->order_func )( entry.object, next_entry.object ) ) )
			_ns_bin_heap_set( heap, curr_index, next_entry );
		else
			break;
		}

	_ns_bin_heap_set( heap, curr_index, entry );
	}


NsError ns_bin_heap_push( NsBinHeap *heap, nspointer object, nspointer *handle )
	{
	nssize          size;
	nssize          capacity;
	NsBinHeapEntry  entry;
	NsError         error;


	ns_assert( NULL != heap );

	size     = ns_bin_heap_size( heap );
	capacity = ns_bin_heap_capacity( heap );

	/* NOTE: +1 in case the current capacity is zero. */
	if( size == capacity )
		if( NS_FAILURE( _ns_bin_heap_resize( heap, capacity * 2 + 1 ), error ) )
			return error;

	entry.object = object;
	entry.handle = handle;

	_ns_bin_heap_set( heap, size, entry );

	_ns_bin_heap_bubble_up( heap, size );
	++(heap->size);

	return ns_no_error();
	}


nspointer ns_bin_heap_peek( const NsBinHeap *heap )
	{
	ns_assert( NULL != heap );
	ns_assert( ! ns_bin_heap_is_empty( heap ) );

	return heap->array[ 0 ].object;
	}


void ns_bin_heap_pop( NsBinHeap *heap )
	{
	nssize size, capacity;

	ns_assert( NULL != heap );
	ns_assert( ! ns_bin_heap_is_empty( heap ) );

	size     = ns_bin_heap_size( heap );
	capacity = ns_bin_heap_capacity( heap );

	/* Erase the object at index 0. */
	if( NULL != heap->finalize_func )
		( heap->finalize_func )( heap->array[ 0 ].object );

	/* Move the last object to the top of the heap. */
	_ns_bin_heap_set( heap, 0, heap->array[ size - 1 ] );

	--(heap->size);

	if( 0 < heap->size )
		_ns_bin_heap_bubble_down( heap, 0 );
	}


void ns_bin_heap_repush( NsBinHeap *heap, nspointer object, nspointer *handle )
	{
	nssize index;

	ns_assert( NULL != heap );
	ns_assert( NULL != handle );

	index = NS_POINTER_TO_SIZE( *handle );

	ns_assert( index < heap->size );
	ns_assert( heap->array[ index ].handle == handle );

	heap->array[ index ].object = object;

	if( 0 < index &&
		 ( heap->order_func )(
			heap->array[ index ].object,
			heap->array[ _NS_BIN_HEAP_PARENT( index ) ].object ) )
		_ns_bin_heap_bubble_up( heap, index );
	else
		_ns_bin_heap_bubble_down( heap, index );
	}
