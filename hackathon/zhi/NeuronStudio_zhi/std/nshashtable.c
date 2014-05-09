#include "nshashtable.h"


NS_PRIVATE void _ns_hash_table_enum_advance( NsHashTable *hash_table )
	{
	while( NULL == hash_table->node )
		{
		hash_table->bucket += 1;

		if( hash_table->num_buckets <= hash_table->bucket )
			return;

		hash_table->node = hash_table->buckets[ hash_table->bucket ];
		}
	}


NS_PRIVATE nsboolean _ns_hash_table_enum_has_more_elements( const NsEnumeration *e )
	{
	NsHashTable *hash_table = e->user_data;
	return hash_table->bucket < hash_table->num_buckets;
	}


NS_PRIVATE nspointer _ns_hash_table_enum_next_element( NsEnumeration *e )
	{
	NsHashTable  *hash_table;
	nspointer     ret_value;

	hash_table = e->user_data;
	ret_value  = ( hash_table->do_keys ) ? hash_table->node->key : hash_table->node->value;

	hash_table->node = hash_table->node->next;
	_ns_hash_table_enum_advance( hash_table );

	return ret_value;
	}


NS_PRIVATE nsboolean _ns_hash_table_lookup
	(
	const NsHashTable   *hash_table,
	nspointer            key,
	nssize              *ret_bucket,
	NsHashTableNode    **ret_node
	)
	{
	nssize                  bucket;
	const NsHashTableNode  *node;


	bucket = ( hash_table->key_hash_func )( key ) % hash_table->num_buckets;

	*ret_bucket = bucket;

	node = hash_table->buckets[ bucket ];

	for( ; NULL != node; node = node->next )
		if( ( hash_table->key_equal_func )( key, node->key ) )
			{
			*ret_node = ( NsHashTableNode* )node;
			return NS_TRUE;
			}

	return NS_FALSE;
	}


#define _ns_hash_table_rehash( hash_table )\
if( ( hash_table->num_buckets >= 3 * hash_table->num_nodes ) ||\
	 ( 3 * hash_table->num_buckets <= hash_table->num_nodes ) )\
	_ns_hash_table_do_rehash( hash_table )


NS_PRIVATE void _ns_hash_table_do_rehash( NsHashTable *hash_table )
	{
	NsHashTableNode  **new_buckets;
	nssize             new_num_buckets;
	nssize             new_bucket;
	NsHashTableNode   *node;
	NsHashTableNode   *next;
	nssize             bucket;


	new_num_buckets = ns_prime( ns_prime_nearest( ( nsuint )hash_table->num_nodes ) );

	if( new_num_buckets == hash_table->num_buckets )
		return;

	if( NULL == ( new_buckets = ns_new_array0( NsHashTableNode*, new_num_buckets ) ) )
		return;

	/* Splice out all nodes from the old buckets to the new ones. */
	for( bucket = 0; bucket < hash_table->num_buckets; ++bucket )
		{
		node = hash_table->buckets[ bucket ];

		while( NULL != node )
			{
			next = node->next;

			new_bucket = ( hash_table->key_hash_func )( node->key ) % new_num_buckets;

			node->next = new_buckets[ new_bucket ];
			new_buckets[ new_bucket ] = node;

			node = next;
			}
		}

	ns_delete( hash_table->buckets );

	hash_table->buckets     = new_buckets;
	hash_table->num_buckets = new_num_buckets;
	}


NsError ns_hash_table_construct
	(
   NsHashTable     *hash_table,
   NsHashFunc       key_hash_func,
   NsEqualFunc      key_equal_func,
   NsFinalizeFunc   key_finalize_func,
   NsFinalizeFunc   value_finalize_func
	)
	{
	ns_assert( NULL != hash_table );
	ns_assert( NULL != key_hash_func );
	ns_assert( NULL != key_equal_func );

	hash_table->num_nodes           = 0;
	hash_table->key_hash_func       = key_hash_func;
	hash_table->key_equal_func      = key_equal_func;
	hash_table->key_finalize_func   = key_finalize_func;
	hash_table->value_finalize_func = value_finalize_func;
	hash_table->can_grow            = NS_TRUE;
	hash_table->can_shrink          = NS_TRUE;

	hash_table->num_buckets = ns_prime( NS_PRIME0 );

	hash_table->free_list = NULL;
	hash_table->recycle   = NS_FALSE;

	hash_table->buckets = ns_new_array0( NsHashTableNode*, hash_table->num_buckets );

	return ( NULL != hash_table->buckets ) ?
			 ns_no_error() : ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


#define _NS_HASH_TABLE_NODE_NEXT( node )\
	( *( ( nspointer* )(node) ) )


void ns_hash_table_destruct( NsHashTable *hash_table )
	{
	NsHashTableNode *curr, *next;

	ns_assert( NULL != hash_table );

	/* NOTE: Assure no-reallocation during clear(). */
	hash_table->can_grow   = NS_FALSE;
	hash_table->can_shrink = NS_FALSE;

	ns_hash_table_clear( hash_table );
	ns_delete( hash_table->buckets );

	/* Upon destruction actually delete the recycled nodes. */

	curr = hash_table->free_list;

	while( NULL != curr )
		{
		next = _NS_HASH_TABLE_NODE_NEXT( curr );
		ns_delete( curr );
		curr = next;
		}
	}


NS_PRIVATE NsHashTableNode* _ns_hash_table_pop_node( NsHashTable *hash_table )
	{
	nspointer node = hash_table->free_list;

	if( NULL == node )
		return ns_new( NsHashTableNode );

	hash_table->free_list = _NS_HASH_TABLE_NODE_NEXT( node );

	return node;
	}


NS_PRIVATE void _ns_hash_table_push_node( NsHashTable *hash_table, NsHashTableNode *node )
	{
	_NS_HASH_TABLE_NODE_NEXT( node ) = hash_table->free_list;
	hash_table->free_list = node;
	}


void ns_hash_table_clear( NsHashTable *hash_table )
	{
	nssize            bucket;
	NsHashTableNode  *node;
	NsHashTableNode  *next;


	ns_assert( NULL != hash_table );

	for( bucket = 0; bucket < hash_table->num_buckets; ++bucket )
		{
		node = hash_table->buckets[ bucket ];

		while( NULL != node )
			{
			next = node->next;

			if( NULL != hash_table->key_finalize_func )
				( hash_table->key_finalize_func )( node->key );

			if( NULL != hash_table->value_finalize_func )
				( hash_table->value_finalize_func )( node->value );

			if( hash_table->recycle )
				_ns_hash_table_push_node( hash_table, node );
			else
				ns_delete( node );

			node = next;
			}

		hash_table->buckets[ bucket ] = NULL;
		}

	hash_table->num_nodes = 0;
	_ns_hash_table_rehash( hash_table );
	}


void ns_hash_table_recycle( NsHashTable *hash_table, nsboolean recycle )
	{
	ns_assert( NULL != hash_table );
	hash_table->recycle = recycle;
	}


nssize ns_hash_table_size( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->num_nodes;
	}


nsboolean ns_hash_table_is_empty( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return 0 == ns_hash_table_size( hash_table );
	}


nsboolean ns_hash_table_get_can_grow( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->can_grow;
	}


void ns_hash_table_set_can_grow( NsHashTable *hash_table, nsboolean can_grow )
	{
	ns_assert( NULL != hash_table );
	hash_table->can_grow = can_grow;
	}


nsboolean ns_hash_table_get_can_shrink( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->can_shrink;
	}


void ns_hash_table_set_can_shrink( NsHashTable *hash_table, nsboolean can_shrink )
	{
	ns_assert( NULL != hash_table );
	hash_table->can_shrink = can_shrink;
	}


NsHashFunc ns_hash_table_get_key_hash_func( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->key_hash_func;
	}


void ns_hash_table_set_key_hash_func( NsHashTable *hash_table, NsHashFunc key_hash_func )
	{
	ns_assert( NULL != hash_table );
	ns_assert( NULL != key_hash_func );

	hash_table->key_hash_func = key_hash_func;
	}


NsEqualFunc ns_hash_table_get_key_equal_func( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->key_equal_func;
	}


void ns_hash_table_set_key_equal_func( NsHashTable *hash_table, NsEqualFunc key_equal_func )
	{
	ns_assert( NULL != hash_table );
	ns_assert( NULL != key_equal_func );

	hash_table->key_equal_func = key_equal_func;
	}


NsFinalizeFunc ns_hash_table_get_key_finalize_func( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->key_finalize_func;
	}


void ns_hash_table_set_key_finalize_func( NsHashTable *hash_table, NsFinalizeFunc key_finalize_func )
	{
	ns_assert( NULL != hash_table );
	hash_table->key_finalize_func = key_finalize_func;
	}


NsFinalizeFunc ns_hash_table_get_value_finalize_func( const NsHashTable *hash_table )
	{
	ns_assert( NULL != hash_table );
	return hash_table->value_finalize_func;
	}


void ns_hash_table_set_value_finalize_func( NsHashTable *hash_table, NsFinalizeFunc value_finalize_func )
	{
	ns_assert( NULL != hash_table );
	hash_table->value_finalize_func = value_finalize_func;
	}


NsError ns_hash_table_add( NsHashTable *hash_table, nspointer key, nspointer value )
	{
	nssize            bucket;
	NsHashTableNode  *node;

	if( _ns_hash_table_lookup( hash_table, key, &bucket, &node )  )
		{
		/* Key already exists. Destroy old value and replace with new one. */
		if( NULL != hash_table->value_finalize_func )
			( hash_table->value_finalize_func )( node->value );

		node->value = value;
		}
	else
		{
		/* Allocate a new node and push to front of bucket. */

		node = hash_table->recycle ?
				 _ns_hash_table_pop_node( hash_table ) : ns_new( NsHashTableNode );
			
		if( NULL == node )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

		node->key   = key;
		node->value = value;

		node->next = hash_table->buckets[ bucket ];
		hash_table->buckets[ bucket ] = node;

		hash_table->num_nodes += 1;

		if( hash_table->can_grow )
			_ns_hash_table_rehash( hash_table );
		}

	return ns_no_error();
	}


nsboolean ns_hash_table_remove( NsHashTable *hash_table, nspointer key )
	{
	nssize            bucket;
	NsHashTableNode  *node;
	NsHashTableNode  *prev;


	ns_assert( NULL != hash_table );

	if( ! _ns_hash_table_lookup( hash_table, key, &bucket, &node ) )
		return NS_FALSE;

	if( NULL != hash_table->key_finalize_func )
		( hash_table->key_finalize_func )( node->key );

	if( NULL != hash_table->value_finalize_func )
		( hash_table->value_finalize_func )( node->value );

	/* If first node in this bucket... */
	if( hash_table->buckets[ bucket ] == node )
		hash_table->buckets[ bucket ] = node->next;
	else
		{
		prev = hash_table->buckets[ bucket ];

		while( prev->next != node )
			prev = prev->next;

		prev->next = node->next;
		}

	if( hash_table->recycle )
		_ns_hash_table_push_node( hash_table, node );
	else
		ns_delete( node );

	hash_table->num_nodes -= 1;

	if( hash_table->can_shrink )
		_ns_hash_table_rehash( hash_table );	

	return NS_TRUE;
	}


nsboolean ns_hash_table_lookup( const NsHashTable *hash_table, nspointer key, nspointer *value )
	{
	nssize            bucket;
	NsHashTableNode  *node;

	ns_assert( NULL != hash_table );
	ns_assert( NULL != value );
    if( ! _ns_hash_table_lookup( hash_table, key, &bucket, &node ) )
        return NS_FALSE;

    *value = node->value;
	return NS_TRUE;
	}


nsboolean ns_hash_table_exists( const NsHashTable *hash_table, nspointer key )
	{
	nspointer value;

	ns_assert( NULL != hash_table );
	return ns_hash_table_lookup( hash_table, key, &value );
	}


NS_PRIVATE NsEnumeration* _ns_hash_table_enum
	(
	NsHashTable    *hash_table,
	NsEnumeration  *enumeration,
	nsboolean       do_keys
	)
	{
	enumeration->has_more_elements = _ns_hash_table_enum_has_more_elements;
	enumeration->next_element      = _ns_hash_table_enum_next_element;
	enumeration->user_data         = hash_table;

	hash_table->bucket  = 0;
	hash_table->node    = hash_table->buckets[ 0 ];
	hash_table->do_keys = do_keys;

	_ns_hash_table_enum_advance( hash_table );

	return enumeration;
	}


NsEnumeration* ns_hash_table_key_enum( const NsHashTable *hash_table, NsEnumeration *enumeration )
	{
	ns_assert( NULL != hash_table );
	ns_assert( NULL != enumeration );

	return _ns_hash_table_enum( ( NsHashTable* )hash_table, enumeration, NS_TRUE );
	}


NsEnumeration* ns_hash_table_value_enum( const NsHashTable *hash_table, NsEnumeration *enumeration )
	{
	ns_assert( NULL != hash_table );
	ns_assert( NULL != enumeration );

	return _ns_hash_table_enum( ( NsHashTable* )hash_table, enumeration, NS_FALSE );
	}
