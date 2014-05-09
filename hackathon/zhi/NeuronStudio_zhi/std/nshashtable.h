#ifndef __NS_STD_HASH_TABLE_H__
#define __NS_STD_HASH_TABLE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nsprimes.h>
#include <std/nsenumeration.h>

NS_DECLS_BEGIN

typedef struct _NsHashTableNode
	{
	nspointer                 key;
	nspointer                 value;
	struct _NsHashTableNode  *next;
	}
	NsHashTableNode;


typedef struct _NsHashTable
	{
	NsHashTableNode  **buckets;
	nssize             num_buckets;
	nssize             num_nodes;
	NsHashFunc         key_hash_func;
	NsEqualFunc        key_equal_func;
	NsFinalizeFunc     key_finalize_func;
	NsFinalizeFunc     value_finalize_func;
	nsboolean          can_grow;
	nsboolean          can_shrink;
	nssize             bucket;
	NsHashTableNode   *node;
	nsboolean          do_keys;
	nspointer          free_list;
	nsboolean          recycle;
	}
	NsHashTable;


NS_IMPEXP NsError ns_hash_table_construct
	(
	NsHashTable     *hash_table,
	NsHashFunc       key_hash_func,
	NsEqualFunc      key_equal_func,
	NsFinalizeFunc   key_finalize_func,
	NsFinalizeFunc   value_finalize_func
	);

NS_IMPEXP void ns_hash_table_destruct( NsHashTable *hash_table );

NS_IMPEXP void ns_hash_table_clear( NsHashTable *hash_table );

NS_IMPEXP void ns_hash_table_recycle( NsHashTable *hash_table, nsboolean recycle );

/* Returns how many key/value pairs are currently stored. */
NS_IMPEXP nssize ns_hash_table_size( const NsHashTable *hash_table );

NS_IMPEXP nsboolean ns_hash_table_is_empty( const NsHashTable *hash_table );

NS_IMPEXP nsboolean ns_hash_table_get_can_grow( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_can_grow( NsHashTable *hash_table, nsboolean can_grow );

NS_IMPEXP nsboolean ns_hash_table_get_can_shrink( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_can_shrink( NsHashTable *hash_table, nsboolean can_shrink );

NS_IMPEXP NsHashFunc ns_hash_table_get_key_hash_func( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_key_hash_func( NsHashTable *hash_table, NsHashFunc key_hash_func );

NS_IMPEXP NsEqualFunc ns_hash_table_get_key_equal_func( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_key_equal_func( NsHashTable *hash_table, NsEqualFunc key_equal_func );

NS_IMPEXP NsFinalizeFunc ns_hash_table_get_key_finalize_func( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_key_finalize_func( NsHashTable *hash_table, NsFinalizeFunc key_finalize_func );

NS_IMPEXP NsFinalizeFunc ns_hash_table_get_value_finalize_func( const NsHashTable *hash_table );
NS_IMPEXP void ns_hash_table_set_value_finalize_func( NsHashTable *hash_table, NsFinalizeFunc value_finalize_func );

/* NOTE: If 'key' already exists in the table then the corresponding
	value is released and the new 'value' is added. */
NS_IMPEXP NsError ns_hash_table_add( NsHashTable *hash_table, nspointer key, nspointer value );
NS_IMPEXP nsboolean ns_hash_table_remove( NsHashTable *hash_table, nspointer key );

NS_IMPEXP nsboolean ns_hash_table_lookup( const NsHashTable *hash_table, nspointer key, nspointer *value );

NS_IMPEXP nsboolean ns_hash_table_exists( const NsHashTable *hash_table, nspointer key );

/* NOTE: Obviously the enumeration is only valid if the hash table is
	not altered during the enumeration. */
NS_IMPEXP NsEnumeration* ns_hash_table_key_enum( const NsHashTable *hash_table, NsEnumeration *enumeration );
NS_IMPEXP NsEnumeration* ns_hash_table_value_enum( const NsHashTable *hash_table, NsEnumeration *enumeration );

NS_DECLS_END

#endif/* __NS_STD_HASH_TABLE_H__ */
