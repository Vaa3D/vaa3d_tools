#include "nshashlist.h"


#define _NS_HASH_LIST_ENUM_KEY    0
#define _NS_HASH_LIST_ENUM_VALUE  1
#define _NS_HASH_LIST_ENUM        2


NS_PRIVATE nsboolean _ns_hash_list_enum_has_more_elements( const NsEnumeration *e )
	{
	NsHashList *hl = e->user_data;
	return ns_hash_list_iter_not_equal( hl->iter, ns_hash_list_end( hl ) );
	}


NS_PRIVATE nspointer _ns_hash_list_enum_next_element( NsEnumeration *e )
	{
	NsHashList      *hl;
	NsHashListNode  *node;
	nspointer        ret_value;


	hl        = e->user_data;
	node      = ns_list_iter_get_object( hl->iter );
	ret_value = NULL;

	switch( hl->enum_type )
		{
		case _NS_HASH_LIST_ENUM_KEY:
			ret_value = node->key;
			break;

		case _NS_HASH_LIST_ENUM_VALUE:
			ret_value = node->value;
			break;

		default:
			ns_assert_not_reached();
		}

	hl->iter = ns_hash_list_iter_next( hl->iter );
	return ret_value;
	}


NsError ns_hash_list_construct
	(
   NsHashList      *hl,
   NsHashFunc       key_hash_func,
   NsEqualFunc      key_equal_func,
   NsFinalizeFunc   key_finalize_func,
   NsFinalizeFunc   value_finalize_func
	)
	{
	NsError error;

	hl->key_finalize_func   = key_finalize_func;
	hl->value_finalize_func = value_finalize_func;

	if( NS_FAILURE( ns_hash_table_construct(
							&hl->hash,
							key_hash_func,
							key_equal_func,
							NULL,
							NULL
							),
							error ) )
		return error;

	ns_list_construct( &hl->list, NULL );

	return ns_no_error();
	}


void ns_hash_list_destruct( NsHashList *hl )
	{
	ns_hash_list_clear( hl );

	ns_list_destruct( &hl->list );
	ns_hash_table_destruct( &hl->hash );
	}


void ns_hash_list_clear( NsHashList *hl )
	{
	ns_assert( NULL != hl );

	ns_hash_list_erase_all(
		hl,
		ns_hash_list_begin( hl ),
		ns_hash_list_end( hl )
		);
	}


nssize ns_hash_list_size( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	ns_assert( ns_list_size( &hl->list ) == ns_hash_table_size( &hl->hash ) );

	return ns_list_size( &hl->list );
	}


nsboolean ns_hash_list_is_empty( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	return 0 == ns_hash_list_size( hl );
	}


NsHashFunc ns_hash_list_get_key_hash_func( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	return ns_hash_table_get_key_hash_func( &hl->hash );
	}


void ns_hash_list_set_key_hash_func( NsHashList *hl, NsHashFunc key_hash_func )
	{
	ns_assert( NULL != hl );
	ns_hash_table_set_key_hash_func( &hl->hash, key_hash_func );
	}


NsEqualFunc ns_hash_list_get_key_equal_func( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	return ns_hash_table_get_key_equal_func( &hl->hash );
	}


void ns_hash_list_set_key_equal_func( NsHashList *hl, NsEqualFunc key_equal_func )
	{
	ns_assert( NULL != hl );
	ns_hash_table_set_key_equal_func( &hl->hash, key_equal_func );
	}


NsFinalizeFunc ns_hash_list_get_key_finalize_func( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	return hl->key_finalize_func;
	}


void ns_hash_list_set_key_finalize_func( NsHashList *hl, NsFinalizeFunc key_finalize_func )
	{
	ns_assert( NULL != hl );
	hl->key_finalize_func = key_finalize_func;
	}


NsFinalizeFunc ns_hash_list_get_value_finalize_func( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	return hl->value_finalize_func;
	}


void ns_hash_list_set_value_finalize_func( NsHashList *hl, NsFinalizeFunc value_finalize_func )
	{
	ns_assert( NULL != hl );
	hl->value_finalize_func = value_finalize_func;
	}


NsError ns_hash_list_add( NsHashList *hl, nspointer key, nspointer value )
	{
	ns_assert( NULL != hl );
	return ns_hash_list_push_back( hl, key, value );
	}


/* NOTE: The hash table maps keys to list iterators
	so 'elistiter' must be a primitive pointer! */
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nslistiter, nspointer );


nsboolean ns_hash_list_remove( NsHashList *hl, nspointer key )
	{
	nshashlistiter I;

	ns_assert( NULL != hl );

	if( ! ns_hash_list_lookup( hl, key, &I ) )
		return NS_FALSE;

	ns_hash_list_erase( hl, I );
	return NS_TRUE;
	}


nsboolean ns_hash_list_lookup( const NsHashList *hl, nspointer key, nshashlistiter *I )
	{
	ns_assert( NULL != hl );
	ns_assert( NULL != I );

	return ns_hash_table_lookup( &hl->hash, key, I );
	}


nsboolean ns_hash_list_exists( const NsHashList *hl, nspointer key )
	{
	nspointer not_used;

	ns_assert( NULL != hl );
	return ns_hash_table_lookup( &hl->hash, key, &not_used );
	}


nshashlistiter ns_hash_list_begin( const NsHashList *hl )
	{  return ns_list_begin( &hl->list );  }


nshashlistiter ns_hash_list_end( const NsHashList *hl )
	{  return ns_list_end( &hl->list );  }


nshashlistreviter ns_hash_list_rev_begin( const NsHashList *hl )
	{  return ns_list_rev_begin( &hl->list );   }


nshashlistreviter ns_hash_list_rev_end( const NsHashList *hl )
	{  return ns_list_rev_end( &hl->list );  }


NsError ns_hash_list_insert( NsHashList *hl, nshashlistiter I, nspointer key, nspointer value )
	{
	NsHashListNode  *node;
	nslistiter       prev;
	NsError          error;


	ns_assert( NULL != hl );
	ns_assert( ! ns_hash_list_exists( hl, key ) );

	if( NULL == ( node = ns_new( NsHashListNode ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	node->key   = key;
	node->value = value;

	if( NS_FAILURE( ns_list_insert( &hl->list, I, node ), error ) )
		{
		ns_delete( node );
		return error;
		}

	prev = ns_list_iter_prev( I );

	if( NS_FAILURE( ns_hash_table_add( &hl->hash, key, prev ), error ) )
		{
		ns_delete( node );
		ns_list_erase( &hl->list, prev );

		return error;
		}

	return ns_no_error();
	}


void ns_hash_list_erase( NsHashList *hl, nshashlistiter I )
	{
	NsHashListNode *node;

	ns_assert( NULL != hl );

	node = ns_list_iter_get_object( I );

	ns_verify( ns_hash_table_remove( &hl->hash, node->key ) );

	if( NULL != hl->key_finalize_func )
		( hl->key_finalize_func )( node->key );

	if( NULL != hl->value_finalize_func )
		( hl->value_finalize_func )( node->value );

	ns_delete( node );

	ns_list_erase( &hl->list, I );
	}


void ns_hash_list_erase_all( NsHashList *hl, nshashlistiter first, nshashlistiter last )
	{
	nshashlistiter next;

	ns_assert( NULL != hl );

	while( ns_hash_list_iter_not_equal( first, last ) )
		{
 		next = ns_hash_list_iter_next( first );
		ns_hash_list_erase( hl, first );
		first = next;
		}
	}


NsError ns_hash_list_push_front( NsHashList *hl, nspointer key, nspointer value )
	{
	ns_assert( NULL != hl );
	return ns_hash_list_insert( hl, ns_hash_list_begin( hl ), key, value );
	}


void ns_hash_list_pop_front( NsHashList *hl )
	{
	ns_assert( NULL != hl );
	ns_assert( ! ns_hash_list_is_empty( hl ) );

	ns_hash_list_erase( hl, ns_hash_list_begin( hl ) );
	}


nspointer ns_hash_list_peek_front( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	ns_assert( ! ns_hash_list_is_empty( hl ) );

	return ns_hash_list_iter_get_object(
				ns_hash_list_begin( hl )
				);
	}


NsError ns_hash_list_push_back( NsHashList *hl, nspointer key, nspointer value )
	{
	ns_assert( NULL != hl );
	return ns_hash_list_insert( hl, ns_hash_list_end( hl ), key, value );
	}


void ns_hash_list_pop_back( NsHashList *hl )
	{
	ns_assert( NULL != hl );
	ns_assert( ! ns_hash_list_is_empty( hl ) );

	ns_hash_list_erase( hl, ns_hash_list_rev_begin( hl ) );
	}


nspointer ns_hash_list_peek_back( const NsHashList *hl )
	{
	ns_assert( NULL != hl );
	ns_assert( ! ns_hash_list_is_empty( hl ) );

	return ns_hash_list_rev_iter_get_object(
				ns_hash_list_rev_begin( hl )
				);
	}


nspointer ns_hash_list_iter_get_object( const nshashlistiter I )
	{
	const NsHashListNode *node = ns_list_iter_get_object( I );
	return node->value;
	}


void ns_hash_list_iter_set_object( nshashlistiter I, nspointer object )
	{
	NsHashListNode *node = ns_list_iter_get_object( I );
	node->value = object;
	}


NS_PRIVATE NsEnumeration* _ns_hash_list_enum
	(
	NsHashList     *hl,
	NsEnumeration  *enumeration,
	nsint           enum_type
	)
	{
	enumeration->has_more_elements = _ns_hash_list_enum_has_more_elements;
	enumeration->next_element      = _ns_hash_list_enum_next_element;
	enumeration->user_data         = hl;

	hl->iter      = ns_hash_list_begin( hl );
	hl->enum_type = enum_type;

	return enumeration;
	}


NsEnumeration* ns_hash_list_key_enum( const NsHashList *hl, NsEnumeration *enumeration )
	{
	ns_assert( NULL != hl );
	return _ns_hash_list_enum( ( NsHashList* )hl, enumeration, _NS_HASH_LIST_ENUM_KEY );
	}


NsEnumeration* ns_hash_list_value_enum( const NsHashList *hl, NsEnumeration *enumeration )
	{
	ns_assert( NULL != hl );
	return _ns_hash_list_enum( ( NsHashList* )hl, enumeration, _NS_HASH_LIST_ENUM_VALUE );
	}
