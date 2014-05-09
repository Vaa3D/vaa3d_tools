#include "nslist.h"


/* Inserts the nodes BEFORE the 'insert' node. */
NS_PRIVATE void _ns_list_splice_in( NsList *list, NsListNode *insert, NsListNode *first, nssize count )
   {
   NsListNode  *last;
   nssize        i;
   

   ns_assert( NULL != list );
   ns_assert( NULL != insert );
   ns_assert( NULL != first );
   ns_assert( 0 < count );

   last = first;

   /* NOTE: -1 since we DONT want a "past the end" node. */
   for( i = 0; i < count - 1; ++i )
      last = last->next;

   first->prev        = insert->prev;
   insert->prev->next = first;
   last->next         = insert;
   insert->prev       = last;

   list->size += count;
   }


NS_PRIVATE void _ns_list_splice_out( NsList *list, NsListNode *first, nssize count )
   {
   NsListNode  *last;
   nssize       i;


   ns_assert( NULL != list );
   ns_assert( NULL != first );
   ns_assert( 0 < count );

   last = first;

   for( i = 0; i < count; ++i )
      last = last->next;

   first->prev->next = last;
   last->prev        = first->prev;

   list->size -= count;
   }


NS_IMPEXP void ns_list_construct
	(
	NsList          *list,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != list );

   list->dummy.next   = &list->dummy;
   list->dummy.prev   = &list->dummy;
   list->dummy.object = NULL;

   list->head = &list->dummy;
	list->size = 0;

	list->finalize_func = finalize_func;
	}


void ns_list_destruct( NsList *list )
   {
   ns_assert( NULL != list );
   ns_list_clear( list );
   }


nssize ns_list_size( const NsList *list )
	{
   ns_assert( NULL != list );
	return list->size;
	}


nsboolean ns_list_is_empty( const NsList *list )
	{
   ns_assert( NULL != list );
	return 0 == ns_list_size( list );
	}


NsFinalizeFunc ns_list_get_finalize_func( const NsList *list )
	{
	ns_assert( NULL != list );
	return list->finalize_func;
	}


void ns_list_set_finalize_func( NsList *list, NsFinalizeFunc finalize_func )
	{
	ns_assert( NULL != list );
	list->finalize_func = finalize_func;
	}


void ns_list_clear( NsList *list )
	{
   ns_assert( NULL != list );
   ns_list_erase_all( list, ns_list_begin( list ), ns_list_end( list ) );
	}


NsError ns_list_insert( NsList *list, nslistiter I, nspointer object )
	{
	NsListNode *node;

   ns_assert( NULL != list );
   ns_assert( NULL != I );

   if( NULL == ( node = ns_new( NsListNode ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   node->object = object;
   _ns_list_splice_in( list, I, node, 1 );

   return ns_no_error();
	}


void ns_list_erase( NsList *list, nslistiter I )
	{
   ns_assert( NULL != list );
	ns_assert( NULL != I );

   _ns_list_splice_out( list, I, 1 );

   if( NULL != list->finalize_func )
      ( list->finalize_func )( I->object );

	ns_delete( I );
	}


void ns_list_erase_all( NsList *list, nslistiter first, nslistiter last )
	{
   NsListNode *next;

   ns_assert( NULL != list );
	ns_assert( NULL != first && NULL != last );

   while( first != last )
      {
      next = first->next;
      ns_list_erase( list, first );
      first = next;
      }
	}


NsError ns_list_push_front( NsList *list, nspointer object )
	{
	ns_assert( NULL != list );
   return ns_list_insert( list, ns_list_begin( list ), object );
	}


void ns_list_pop_front( NsList *list )
	{
	ns_assert( NULL != list );
   ns_assert( ! ns_list_is_empty( list ) );

   ns_list_erase( list, ns_list_begin( list ) );
	}


nspointer ns_list_peek_front( const NsList *list )
	{
   ns_assert( NULL != list );
   ns_assert( ! ns_list_is_empty( list ) );

   return ns_list_begin( list )->object;
	}


NsError ns_list_push_back( NsList *list, nspointer object )
	{
	ns_assert( NULL != list );
   return ns_list_insert( list, ns_list_end( list ), object );
	}


void ns_list_pop_back( NsList *list )
	{
	ns_assert( NULL != list );
   ns_assert( ! ns_list_is_empty( list ) );

   ns_list_erase( list, ns_list_rev_begin( list ) );
	}


nspointer ns_list_peek_back( const NsList *list )
	{
	ns_assert( NULL != list );
   ns_assert( ! ns_list_is_empty( list ) );

   return ns_list_rev_begin( list )->object;
	}


void ns_list_splice_in( NsList *list, nslistiter insert, nslistiter first, nssize count )
	{
	ns_assert( NULL != list );
	ns_assert( NULL != insert );
	ns_assert( NULL != first );
	ns_assert( 1 <= count );

	_ns_list_splice_in( list, insert, first, count );
	}


void ns_list_splice_out( NsList *list, nslistiter first, nssize count )
	{
	ns_assert( NULL != list );
	ns_assert( NULL != first );
	ns_assert( 1 <= count );

	_ns_list_splice_out( list, first, count );
	}


void ns_list_append( NsList *list1, NsList *list2 )
	{
	nssize      list2_size;
	nslistiter  list2_first;
	nslistiter  list1_end;


   ns_assert( NULL != list1 );
   ns_assert( NULL != list2 );
   ns_assert( list1 != list2 );

	if( ns_list_get_finalize_func( list1 ) != ns_list_get_finalize_func( list2 ) )
		ns_warning(
			NS_WARNING_LEVEL_CAUTION
			NS_MODULE
			" Lists have different finalizers."
			);

   if( ! ns_list_is_empty( list2 ) )
      {
      list2_size  = ns_list_size( list2 );
      list2_first = ns_list_begin( list2 );
      list1_end   = ns_list_end( list1 );

      _ns_list_splice_out( list2, list2_first, list2_size );
      _ns_list_splice_in( list1, list1_end, list2_first, list2_size );
      }
	}


NS_PRIVATE void ns_list_merge( NsList *dest_list, NsList *src_list, NsBinaryBooleanFunc order_func )
   {
	nslistiter dest_curr, dest_end, src_curr, src_end, src_splice;

   dest_curr = ns_list_begin( dest_list );
	dest_end  = ns_list_end( dest_list );

   src_curr = ns_list_begin( src_list );
	src_end  = ns_list_end( src_list );

   while( ns_list_iter_not_equal( dest_curr, dest_end ) &&
			 ns_list_iter_not_equal( src_curr,  src_end  )    )
      {
      if( ( order_func )(
				ns_list_iter_get_object( src_curr ),
				ns_list_iter_get_object( dest_curr )
				) )
			{
			src_splice = src_curr;
			src_curr   = ns_list_iter_next( src_curr );

			_ns_list_splice_out( src_list, src_splice, 1 );
         _ns_list_splice_in( dest_list, dest_curr, src_splice, 1 );
			}
      else
         dest_curr = ns_list_iter_next( dest_curr );
      }

   if( ! ns_list_is_empty( src_list ) )
		ns_list_append( dest_list, src_list );
   }


NS_PRIVATE void ns_list_insertion_sort( NsList *list, NsBinaryBooleanFunc order_func )
   { 
   nslistiter curr, end, scan, splice;

   if( 2 <= ns_list_size( list ) )
      { 
      curr = ns_list_iter_next( ns_list_begin( list ) );
		end  = ns_list_end( list );

      while( ns_list_iter_not_equal( curr, end ) )
         {
         scan = ns_list_iter_prev( curr );

         /* Search the sorted part of the list for a node with a
				lesser value than the current nodes value. */

         while( ns_list_iter_not_equal( scan, end ) )
            {
            if( ( order_func )(
						ns_list_iter_get_object( scan ),
						ns_list_iter_get_object( curr )
						) )
               break;

            scan = ns_list_iter_prev( scan );
            }

         /* If current node not sorted already, splice it into place! */ 

			scan = ns_list_iter_next( scan );

         if( ns_list_iter_not_equal( scan, curr ) ) 
				{
				splice = curr;
				curr   = ns_list_iter_next( curr );

				_ns_list_splice_out( list, splice, 1 );
            _ns_list_splice_in( list, scan, splice, 1 );
				}
         else
            curr = ns_list_iter_next( curr );
         }
      }
   }


#define _NS_LIST_INSERTION_SORT_MAX  12

void ns_list_sort( NsList *list, NsBinaryBooleanFunc order_func )
   {
   nssize size;

	ns_assert( NULL != list );
	size = ns_list_size( list );

   if( size <= _NS_LIST_INSERTION_SORT_MAX )
      ns_list_insertion_sort( list, order_func );
   else
      {
		NsList      temp;
		nslistiter  splice;

		/* NOTE: Only passing the finalize function to avoid
			warning in the ns_list_append() function. */
		ns_list_construct( &temp, ns_list_get_finalize_func( list ) );
		splice = ns_list_iter_offset( ns_list_begin( list ), size / 2 );

		_ns_list_splice_out( list, splice, size - size / 2 );
		_ns_list_splice_in( &temp, ns_list_end( &temp ), splice, size - size / 2 );

		/* NOTE: Recursive merge-sort. */
      ns_list_sort( list, order_func );
      ns_list_sort( &temp, order_func );

      ns_list_merge( list, &temp, order_func );

		ns_assert( ns_list_is_empty( &temp ) );
		ns_list_destruct( &temp );
      }
   }


void ns_list_reverse( NsList *list )
	{
	nslistiter  first, last;
	nspointer   object;


	ns_assert( NULL != list );

	if( ns_list_size( list ) <= 1 )
		return;

	first = ns_list_begin( list );
	last  = ns_list_end( list );

	for( ; ns_list_iter_not_equal( first, last ); first = ns_list_iter_next( first ) )
		{
		last = ns_list_iter_prev( last );

		if( ns_list_iter_equal( first, last ) )
			break;

		object = ns_list_iter_get_object( first );
		ns_list_iter_set_object( first, ns_list_iter_get_object( last ) );
		ns_list_iter_set_object( last, object );
		}
	}


void ns_list_splice_reverse( NsList *list )
	{
	nslistiter  first, last;
	nslistiter  next_of_first, next_of_last;


	ns_assert( NULL != list );

	if( ns_list_size( list ) <= 1 )
		return;

	first = ns_list_begin( list );
	last  = ns_list_end( list );

	for( ; ns_list_iter_not_equal( first, last ); first = ns_list_iter_next( first ) )
		{
		last = ns_list_iter_prev( last );

		if( ns_list_iter_equal( first, last ) )
			break;

		next_of_first = ns_list_iter_next( first );
		next_of_last  = ns_list_iter_next( last );

		ns_list_splice_out( list, first, 1 );
		ns_list_splice_out( list, last, 1 );

		ns_list_splice_in( list, next_of_first, last, 1 );
		ns_list_splice_in( list, next_of_last, first, 1 );

		NS_SWAP( nslistiter, first, last );
		}
	}


nslistiter ns_list_find( const NsList *list, const NsBooleanBind2nd *binder )
	{
	nslistiter curr, end;

	ns_assert( NULL != list );
	ns_assert( NULL != binder );
	ns_assert( NULL != binder->func );

	curr = ns_list_begin( list );
	end  = ns_list_end( list );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		if( ns_boolean_bind_2nd( binder, ns_list_iter_get_object( curr ) ) )
			return curr;

	return end;
	}


nslistiter ns_list_iter_offset( nslistiter I, nslong n )
	{
	if( n < 0 )
		{
		for( ; n < 0; ++n )
			I = ns_list_iter_prev( I );
		}
	else
		{
		for( ; 0 < n; --n )
			I = ns_list_iter_next( I );
		}
	
	return I;
	}
