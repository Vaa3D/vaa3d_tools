#ifndef __NS_STD_HASH_LIST_H__
#define __NS_STD_HASH_LIST_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nshashtable.h>
#include <std/nsenumeration.h>

NS_DECLS_BEGIN

typedef struct _NsHashListNode
	{
	nspointer  key;
	nspointer  value;
	}
	NsHashListNode;


typedef nslistiter    nshashlistiter;
typedef nslistreviter nshashlistreviter;


typedef struct _NsHashList
	{
	NsHashTable     hash;
	NsList          list;
	NsFinalizeFunc  key_finalize_func;
	NsFinalizeFunc  value_finalize_func;
	nshashlistiter  iter;
	nsint           enum_type;
	}
	NsHashList;


NS_IMPEXP NsError ns_hash_list_construct
	(
   NsHashList      *hl,
   NsHashFunc       key_hash_func,
   NsEqualFunc      key_equal_func,
   NsFinalizeFunc   key_finalize_func,
   NsFinalizeFunc   value_finalize_func
	);

NS_IMPEXP void ns_hash_list_destruct( NsHashList *hl );

NS_IMPEXP void ns_hash_list_clear( NsHashList *hl );

NS_IMPEXP nssize ns_hash_list_size( const NsHashList *hl );

NS_IMPEXP nsboolean ns_hash_list_is_empty( const NsHashList *hl );

NS_IMPEXP NsHashFunc ns_hash_list_get_key_hash_func( const NsHashList *hl );
NS_IMPEXP void ns_hash_list_set_key_hash_func( NsHashList *hl, NsHashFunc key_hash_func );

NS_IMPEXP NsEqualFunc ns_hash_list_get_key_equal_func( const NsHashList *hl );
NS_IMPEXP void ns_hash_list_set_key_equal_func( NsHashList *hl, NsEqualFunc key_equal_func );

NS_IMPEXP NsFinalizeFunc ns_hash_list_get_key_finalize_func( const NsHashList *hl );
NS_IMPEXP void ns_hash_list_set_key_finalize_func( NsHashList *hl, NsFinalizeFunc key_finalize_func );

NS_IMPEXP NsFinalizeFunc ns_hash_list_get_value_finalize_func( const NsHashList *hl );
NS_IMPEXP void ns_hash_list_set_value_finalize_func( NsHashList *hl, NsFinalizeFunc value_finalize_func );

/* Always added to the end of the list. For this and any functions that
	add to the list, adding a 'key' that exists already is not allowed. */
NS_IMPEXP NsError ns_hash_list_add( NsHashList *hl, nspointer key, nspointer value );
NS_IMPEXP nsboolean ns_hash_list_remove( NsHashList *hl, nspointer key );

NS_IMPEXP nsboolean ns_hash_list_lookup( const NsHashList *hl, nspointer key, nshashlistiter *I );

NS_IMPEXP nsboolean ns_hash_list_exists( const NsHashList *hl, nspointer key );

NS_IMPEXP nshashlistiter ns_hash_list_begin( const NsHashList *hl );
NS_IMPEXP nshashlistiter ns_hash_list_end( const NsHashList *hl );

NS_IMPEXP nshashlistreviter ns_hash_list_rev_begin( const NsHashList *hl );
NS_IMPEXP nshashlistreviter ns_hash_list_rev_end( const NsHashList *hl );

/* Inserts BEFORE the passed iterator. */
NS_IMPEXP NsError ns_hash_list_insert( NsHashList *hl, nshashlistiter I, nspointer key, nspointer value );

NS_IMPEXP void ns_hash_list_erase( NsHashList *hl, nshashlistiter I );

/* Erase up to but NOT including last.  */
NS_IMPEXP void ns_hash_list_erase_all( NsHashList *hl, nshashlistiter first, nshashlistiter last );

NS_IMPEXP NsError ns_hash_list_push_front( NsHashList *hl, nspointer key, nspointer value );
NS_IMPEXP void ns_hash_list_pop_front( NsHashList *hl );
NS_IMPEXP nspointer ns_hash_list_peek_front( const NsHashList *hl );

NS_IMPEXP NsError ns_hash_list_push_back( NsHashList *hl, nspointer key, nspointer value );
NS_IMPEXP void ns_hash_list_pop_back( NsHashList *hl );
NS_IMPEXP nspointer ns_hash_list_peek_back( const NsHashList *hl );

#define ns_hash_list_iter_next( I )\
   ns_list_iter_next( (I) )

#define ns_hash_list_iter_prev( I )\
   ns_list_iter_prev( (I) )

#define ns_hash_list_iter_equal( I1, I2 )\
	ns_list_iter_equal( (I1), (I2) )

#define ns_hash_list_iter_offset( I, n )\
	ns_list_iter_offset( (I), (n) )

NS_IMPEXP nspointer ns_hash_list_iter_get_object( const nshashlistiter I );
NS_IMPEXP void ns_hash_list_iter_set_object( nshashlistiter I, nspointer object );

#define ns_hash_list_iter_not_equal( I1, I2 )\
	( ! ns_hash_list_iter_equal( I1, I2 ) )

#define ns_hash_list_rev_iter_next( I )\
	ns_hash_list_iter_prev( ( I ) )

#define ns_hash_list_rev_iter_prev( I )\
	ns_hash_list_iter_next( ( I ) )

#define ns_hash_list_rev_iter_get_object( I )\
	ns_hash_list_iter_get_object( ( I ) )

#define ns_hash_list_rev_iter_set_object( I, obj )\
	ns_hash_list_iter_set_object( ( I ), ( obj ) )

#define ns_hash_list_rev_iter_equal( I1, I2 )\
	ns_hash_list_iter_equal( ( I1 ), ( I2 ) )

#define ns_hash_list_rev_iter_not_equal( I1, I2 )\
	ns_hash_list_iter_not_equal( ( I1 ), ( I2 ) )

NS_IMPEXP NsEnumeration* ns_hash_list_key_enum( const NsHashList *hl, NsEnumeration *enumeration );
NS_IMPEXP NsEnumeration* ns_hash_list_value_enum( const NsHashList *hl, NsEnumeration *enumeration );

NS_DECLS_END

#endif /* __NS_STD_HASH_LIST_H__ */
