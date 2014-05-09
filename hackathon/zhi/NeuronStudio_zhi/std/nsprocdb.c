#include "nsprocdb.h"


typedef struct _NsProcDbEntry
	{
	NsProc     *proc;
	NsClosure   closure;
	}
	NsProcDbEntry;


NS_PRIVATE NsError ns_proc_db_entry_new( NsProcDbEntry **entry, NsProc *proc )
	{
	NsClosureMarshal  marshal;
	NsError           error;


	ns_assert( NULL != entry );

	if( NULL == ( *entry = ns_new( NsProcDbEntry ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*entry)->proc = proc;

	marshal = NULL;

	switch( proc->invoke_type )
		{
		case NS_PROC_INVOKE_MARSHAL:
			marshal = proc->marshal;
			break;

		default:
			ns_assert_not_reached();
		}

	ns_assert( NULL != marshal );

	if( NS_FAILURE( ns_closure_construct(
							&( (*entry)->closure ),
							marshal,
							proc->params,
							proc->num_params,
							&proc->ret_value
							),
							error ) )
		{
		ns_delete( *entry );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE void ns_proc_db_entry_delete( NsProcDbEntry *entry )
	{
	ns_assert( NULL != entry );

	ns_closure_destruct( &entry->closure ); 
	ns_delete( entry );
	}


NS_PRIVATE NsProcDbEntry* _ns_proc_db_entry( const NsProcDb *db, const nschar *name )
	{
	nspointer entry = NULL;

	return ns_hash_table_lookup( &db->entries, ( nschar* )name, &entry )
			 ? entry : NULL;
	}


NsError ns_proc_db_construct( NsProcDb *db )
	{
	ns_assert( NULL != db );

	return ns_hash_table_construct(
				&db->entries,
				ns_ascii_hash,
				ns_ascii_streq,
				NULL,
				ns_proc_db_entry_delete
				);
	}


void ns_proc_db_destruct( NsProcDb *db )
   {
	ns_assert( NULL != db );
	ns_hash_table_destruct( &db->entries );
	}


nssize ns_proc_db_size( const NsProcDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_table_size( &db->entries );
	}


NsError ns_proc_db_register( NsProcDb *db, NsProc *proc )
	{
	NsProcDbEntry  *entry;
	NsError         error;
 

	ns_assert( NULL != db );
	ns_assert( NULL != proc );
	ns_assert( ! ns_proc_db_is_registered( db, proc->name ) );

	if( NS_FAILURE( ns_proc_db_entry_new( &entry, proc ), error ) )
		return error;

	if( NS_FAILURE( ns_hash_table_add(
							&db->entries,
							( nschar* )proc->name,
							entry
							),
							error ) )
		{
		ns_proc_db_entry_delete( entry );
		return error;
		}

	return ns_no_error();
	}


void ns_proc_db_unregister( NsProcDb *db, const nschar *name )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ns_proc_db_is_registered( db, name ) );

	ns_verify( ns_hash_table_remove( &db->entries, ( nschar* )name ) );
	}


nsboolean ns_proc_db_is_registered( const NsProcDb *db, const nschar *name )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != name );

	return NULL != _ns_proc_db_entry( db, name );
	}


NsProc* ns_proc_db_lookup( const NsProcDb *db, const nschar *name )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ns_proc_db_is_registered( db, name ) );

	return _ns_proc_db_entry( db, name )->proc;
	}


NsError ns_proc_db_invoke
	( 
	NsProcDb         *db,
	const nschar     *name,
	NsValue          *params,
	nssize            num_params,
	NsValue          *ret_value,
	NsClosureRecord  *record
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ns_proc_db_is_registered( db, name ) );

	return ns_closure_invoke(
				&( _ns_proc_db_entry( db, name )->closure ),
				params,
				num_params,
				ret_value,
				record
				);
	}


NsError ns_proc_db_run
	( 
	NsProcDb         *db,
	const nschar     *name,
	NsValue          *ret_value,
	NsClosureRecord  *record,
	...
	)
	{
	ns_va_list  args;
	NsError     error;

	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ns_proc_db_is_registered( db, name ) );

	ns_va_start( args, record );
	error = ns_proc_db_vrun( db, name, args, ret_value, record );
	ns_va_end( args );

	return error;
	}


NsError ns_proc_db_vrun
	( 
	NsProcDb         *db,
	const nschar     *name,
	ns_va_list        args,
	NsValue          *ret_value,
	NsClosureRecord  *record
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ns_proc_db_is_registered( db, name ) );

	return ns_closure_vrun(
				&( _ns_proc_db_entry( db, name )->closure ),
				args,
				ret_value,
				record
				);
	}
