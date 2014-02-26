#ifndef __NS_STD_STACK_H__
#define __NS_STD_STACK_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nslist.h>

NS_DECLS_BEGIN

typedef struct _NsStack
	{
   NsList  list;
	}
	NsStack;


NS_IMPEXP void ns_stack_construct
	(
	NsStack         *stack,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_stack_destruct( NsStack *stack );

NS_IMPEXP NsError ns_stack_push( NsStack *stack, nspointer object );
NS_IMPEXP void ns_stack_pop( NsStack *stack );

#define ns_stack_clear( stack )\
	( ns_list_clear( &( stack )->list ) )

#define ns_stack_is_empty( stack )\
   ( ns_list_is_empty( &( stack )->list ) )
   
#define ns_stack_size( stack )\
   ( ns_list_size( &( stack )->list ) )

#define ns_stack_peek( stack )\
   ( ns_list_peek_back( &( stack )->list ) )

#define ns_stack_get_finalize_func( stack )\
	( ns_list_get_finalize_func( &( stack )->list ) )

#define ns_stack_set_finalize_func( stack, finalize_func )\
	( ns_list_set_finalize_func( &( stack )->list, ( finalize_func ) ) )

NS_DECLS_END

#endif/* __NS_STD_STACK_H__ */
