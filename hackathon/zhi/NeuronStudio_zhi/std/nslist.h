#ifndef __NS_STD_LIST_H__
#define __NS_STD_LIST_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>

NS_DECLS_BEGIN

typedef struct _NsListNode
	{
   struct _NsListNode  *prev;
   struct _NsListNode  *next;
   nspointer            object;
	}
	NsListNode;

typedef struct _NsList
	{
   NsListNode      dummy;
   NsListNode     *head;
	nssize          size;
	NsFinalizeFunc  finalize_func;
	}
	NsList;

typedef NsListNode*  nslistiter;
typedef nslistiter   nslistreviter;


#define NS_LIST_FOREACH( list, iter )\
	NS_TYPE_FOREACH(\
		(list),\
		(iter),\
		ns_list_begin,\
		ns_list_end,\
		ns_list_iter_next,\
		ns_list_iter_not_equal\
		)


/* NOTE: Pass NULL for 'finalize_func' if not needed. */
NS_IMPEXP void ns_list_construct
	(
	NsList          *list,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_list_destruct( NsList *list );

NS_IMPEXP NsFinalizeFunc ns_list_get_finalize_func( const NsList *list );
NS_IMPEXP void ns_list_set_finalize_func( NsList *list, NsFinalizeFunc finalize_func );

NS_IMPEXP nssize ns_list_size( const NsList *list );
NS_IMPEXP nsboolean ns_list_is_empty( const NsList *list );

NS_IMPEXP void ns_list_clear( NsList *list );

/* Inserts the object BEFORE the object pointed to by the passed iterator. */
NS_IMPEXP NsError ns_list_insert( NsList *list, nslistiter I, nspointer object );

NS_IMPEXP void ns_list_erase( NsList *list, nslistiter I );
/* Erase all nodes from first up to but NOT including last.  */
NS_IMPEXP void ns_list_erase_all( NsList *list, nslistiter first, nslistiter last );

NS_IMPEXP NsError ns_list_push_front( NsList *list, nspointer object );
NS_IMPEXP void ns_list_pop_front( NsList *list );
NS_IMPEXP nspointer ns_list_peek_front( const NsList *list );

NS_IMPEXP NsError ns_list_push_back( NsList *list, nspointer object );
NS_IMPEXP void ns_list_pop_back( NsList *list );
NS_IMPEXP nspointer ns_list_peek_back( const NsList *list );

/* Inserts the nodes BEFORE the 'insert' node. */
NS_IMPEXP void ns_list_splice_in( NsList *list, nslistiter insert, nslistiter first, nssize count );
NS_IMPEXP void ns_list_splice_out( NsList *list, nslistiter first, nssize count );


#define ns_list_splice_in_front( list, first, count )\
	ns_list_splice_in( (list), ns_list_begin( (list) ), (first), (count) )

#define ns_list_splice_in_back( list, first, count )\
	ns_list_splice_in( (list), ns_list_end( (list) ), (first), (count) )


NS_IMPEXP void ns_list_append( NsList *dest, NsList *src );

NS_IMPEXP void ns_list_sort( NsList *list, NsBinaryBooleanFunc order_func );


/* The order of the nodes does not change and the objects move. */
NS_IMPEXP void ns_list_reverse( NsList *list );

/* Just the order of the nodes are reversed. */
NS_IMPEXP void ns_list_splice_reverse( NsList *list );


#define ns_list_begin( list )\
	( (list)->head->next )

#define ns_list_end( list )\
   ( (list)->head )

#define ns_list_rev_begin( list )\
   ( (list)->head->prev )

#define ns_list_rev_end( list )\
   ( (list)->head )

#define ns_list_iter_next( I )\
   ( ( I )->next )

#define ns_list_iter_prev( I )\
   ( ( I )->prev )

#define ns_list_iter_get_object( I )\
   ( ( I )->object ) 

#define ns_list_iter_set_object( I, obj )\
   ( ( I )->object = ( obj ) )

#define ns_list_iter_equal( I1, I2 )\
	( ( I1 ) == ( I2 ) )

NS_IMPEXP nslistiter ns_list_iter_offset( nslistiter I, nslong n );

#define ns_list_iter_not_equal( I1, I2 )\
	( ! ns_list_iter_equal( ( I1 ), ( I2 ) ) )

#define ns_list_rev_iter_next( I )\
	ns_list_iter_prev( ( I ) )

#define ns_list_rev_iter_prev( I )\
	ns_list_iter_next( ( I ) )

#define ns_list_rev_iter_get_object( I )\
	ns_list_iter_get_object( ( I ) )

#define ns_list_rev_iter_set_object( I, obj )\
	ns_list_iter_set_object( ( I ), ( obj ) )

#define ns_list_rev_iter_equal( I1, I2 )\
	ns_list_iter_equal( ( I1 ), ( I2 ) )

#define ns_list_rev_iter_not_equal( I1, I2 )\
	ns_list_iter_not_equal( ( I1 ), ( I2 ) )

NS_DECLS_END

#endif/* __NS_STD_LIST_H__ */
