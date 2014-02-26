#include "nscolordb.h"


typedef struct _NsColorEntry
	{
	nspointer       object;
	NsFinalizeFunc  finalize_func;
	}
	NsColorEntry;


NS_PRIVATE NsError _ns_color_entry_new
	(
	NsColorEntry    **entry,
	nspointer         object,
	NsFinalizeFunc    finalize_func
	)
	{
	if( NULL == ( *entry = ns_new( NsColorEntry ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*entry)->object        = object;
	(*entry)->finalize_func = finalize_func;

	return ns_no_error();
	}


NS_PRIVATE void _ns_color_entry_delete( NsColorEntry *entry )
	{
	if( NULL != entry->finalize_func )
		( entry->finalize_func )( entry->object );

	ns_delete( entry );
	}


NS_PRIVATE nsuint _ns_color_db_key_hash_func( nsconstpointer color )
	{  return NS_POINTER_TO_UINT( color );  }


NS_PRIVATE nsboolean _ns_color_db_key_equal_func( nsconstpointer color1, nsconstpointer color2 )
	{  return color1 == color2;  }


NsError ns_color_db_construct( NsColorDb *db )
	{
	ns_assert( NULL != db );

	return ns_hash_table_construct(
				&db->entries,
				_ns_color_db_key_hash_func,
				_ns_color_db_key_equal_func,
				NULL,
				_ns_color_entry_delete
				);
	}


void ns_color_db_destruct( NsColorDb *db )
	{
	ns_assert( NULL != db );
	ns_hash_table_destruct( &db->entries );
	}


void ns_color_db_clear( NsColorDb *db )
	{
	ns_assert( NULL != db );
	ns_hash_table_clear( &db->entries );
	}


nssize ns_color_db_size( const NsColorDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_table_size( &db->entries );
	}


nsboolean ns_color_db_is_empty( const NsColorDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_table_is_empty( &db->entries );
	}


NsError ns_color_db_add
	(
	NsColorDb       *db,
	nsuint           color,
	nspointer        object,
	NsFinalizeFunc   finalize_func
	)
	{
	NsColorEntry  *entry;
	NsError        error;


	ns_assert( NULL != db );

	if( NS_FAILURE( _ns_color_entry_new( &entry, object, finalize_func ), error ) )
		return error;

	if( NS_FAILURE( ns_hash_table_add( &db->entries, NS_ULONG_TO_POINTER( ( nsulong )color ), entry ), error ) )
		{
		/* NOTE: On failure dont call the 'finalize_func'. */
		ns_delete( entry );
		return error;
		}

	return ns_no_error();
	}


nsboolean ns_color_db_remove( NsColorDb *db, nsuint color )
	{
	ns_assert( NULL != db );
	return ns_hash_table_remove( &db->entries, NS_ULONG_TO_POINTER( ( nsulong )color ) );
	}


nsboolean ns_color_db_exists( const NsColorDb *db, nsuint color )
	{
	ns_assert( NULL != db );
	return ns_hash_table_exists( &db->entries, NS_ULONG_TO_POINTER( ( nsulong )color ) );
	}


nsboolean ns_color_db_lookup( const NsColorDb *db, nsuint color, nspointer *object )
	{
	NsColorEntry *entry = NULL;

	ns_assert( NULL != db );
	ns_assert( NULL != object );

	if( ns_hash_table_lookup( &db->entries, NS_ULONG_TO_POINTER( ( nsulong )color ), &entry ) )
		*object = entry->object;

	return NULL != entry;
	}


nspointer ns_color_db_get( NsColorDb *db, nsuint color )
	{
	nspointer object = NULL;

	ns_assert( NULL != db );

	ns_color_db_lookup( db, color, &object );
	return object;
	}


void ns_color_db_set
	(
	NsColorDb       *db,
	nsuint           color,
	nspointer        object,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != db );
	ns_color_db_add( db, color, object, finalize_func );
	}
