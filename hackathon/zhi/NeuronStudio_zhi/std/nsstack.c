#include "nsstack.h"


void ns_stack_construct
	(
	NsStack         *stack,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != stack );
	ns_list_construct( &stack->list, finalize_func );
	}


void ns_stack_destruct( NsStack *stack )
	{
	ns_assert( NULL != stack );
	ns_list_destruct( &stack->list );
	}


NsError ns_stack_push( NsStack *stack, nspointer object )
	{
	ns_assert( NULL != stack );
	return ns_list_push_back( &stack->list, object );
	}


void ns_stack_pop( NsStack *stack )
	{
	ns_assert( NULL != stack );
	ns_assert( ! ns_stack_is_empty( stack ) );

   ns_list_pop_back( &stack->list );
	}
