#include "nsclosure.h"


NsError ns_closure_record_construct
	(
	NsClosureRecord  *record,
	const nschar     *description,
	nspointer         user_data
	)
	{
	ns_assert( NULL != record );

	record->description = NULL;

	if( NULL != description )
		if( NULL == ( record->description = ns_ascii_strdup( description ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	record->formal_params = NULL;
	record->actual_params = NULL;

	record->formal_ret_value.name = NULL;
	record->formal_ret_value.type = NS_VALUE_UNKNOWN;

	ns_value_construct( &record->actual_ret_value );

	record->num_params = 0;

	record->user_data = user_data;

	return ns_no_error();
	}


void ns_closure_record_destruct( NsClosureRecord *record )
	{
	nssize i;

	ns_value_destruct( &record->actual_ret_value );

	for( i = 0; i < record->num_params; ++i )
		ns_value_destruct( record->actual_params + i );

	ns_delete( record->actual_params );
	ns_delete( record->formal_params );

	ns_free( record->description );
	}


const nschar* ns_closure_record_description( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return record->description;
	}


nssize ns_closure_record_num_params( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return record->num_params;
	}


const NsClosureValue* ns_closure_record_formal_params( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return record->formal_params;
	}


const NsClosureValue* ns_closure_record_formal_param( const NsClosureRecord *record, nssize i )
	{
	ns_assert( NULL != record );
	ns_assert( i < record->num_params );

	return record->formal_params + i;
	}


const NsClosureValue* ns_closure_record_formal_param_by_name( const NsClosureRecord *record, const nschar *name )
	{
	nssize i;

	for( i = 0; i < record->num_params; ++i )
		if( ns_ascii_streq( name, record->formal_params[i].name ) )
			return record->formal_params + i;

	return NULL;
	}


const NsValue* ns_closure_record_actual_params( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return record->actual_params;
	}


const NsValue* ns_closure_record_actual_param( const NsClosureRecord *record, nssize i )
	{
	ns_assert( NULL != record );
	ns_assert( i < record->num_params );

	return record->actual_params + i;
	}


const NsValue* ns_closure_record_actual_param_by_name( const NsClosureRecord *record, const nschar *name )
	{
	nssize i;

	for( i = 0; i < record->num_params; ++i )
		if( ns_ascii_streq( name, record->formal_params[i].name ) )
			return record->actual_params + i;

	return NULL;
	}


const NsClosureValue* ns_closure_record_formal_ret_value( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return &record->formal_ret_value;
	}


const NsValue* ns_closure_record_actual_ret_value( const NsClosureRecord *record )
	{
	ns_assert( NULL != record );
	return &record->actual_ret_value;
	}


NS_PRIVATE NsError _ns_closure_record_create
	(
	NsClosureRecord  *record,
	const NsClosure  *closure,
	const NsValue    *actual_params,
	nssize            num_params,
	const NsValue    *actual_ret_value
	)
	{
	nssize i;

	ns_assert( NULL != record );
	ns_assert( NULL != closure );

	record->formal_ret_value = closure->formal_ret_value;

	if( NULL != actual_ret_value )
		ns_value_assign( &record->actual_ret_value, actual_ret_value );
	else
		ns_value_set_type( &record->actual_ret_value, NS_VALUE_VOID );

	ns_assert( closure->num_params == num_params );
	record->num_params = num_params;

	if( 0 < num_params )
		{
		ns_assert( NULL != actual_params );

		record->formal_params = ns_new_array( NsClosureValue, num_params );
		record->actual_params = ns_new_array( NsValue, num_params );

		if( NULL == record->formal_params || NULL == record->actual_params )
			{
			ns_delete( record->formal_params );
			ns_delete( record->actual_params );

			record->num_params    = 0;
			record->formal_params = NULL;
			record->actual_params = NULL;

			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}

		for( i = 0; i < num_params; ++i )
			{
			record->formal_params[i] = closure->formal_params[i];

			ns_value_construct( record->actual_params + i );
			ns_value_assign( record->actual_params + i, actual_params + i );
			}
		}

	return ns_no_error();
	}




NsError ns_closure_construct
	(
	NsClosure             *closure,
	NsClosureMarshal       marshal,
	const NsClosureValue  *formal_params,
	nssize                 num_params,
	const NsClosureValue  *formal_ret_value
	)
	{
	nssize i;

	ns_assert( NULL != closure );
	ns_assert( NULL != marshal );

	closure->marshal       = marshal;
	closure->num_params    = num_params;
	closure->formal_params = NULL;
	closure->actual_params = NULL;

	if( NULL == formal_ret_value )
		{
		closure->formal_ret_value.name = NULL;
		closure->formal_ret_value.type = NS_VALUE_VOID;
		}
	else
		closure->formal_ret_value = *( formal_ret_value );

	if( 0 < num_params )
		{
		ns_assert( NULL != formal_params );

		closure->formal_params = ns_new_array( NsClosureValue, num_params );
		closure->actual_params = ns_new_array( NsValue, num_params );

		if( NULL == closure->formal_params || NULL == closure->actual_params )
			{
			ns_delete( closure->formal_params );
			ns_delete( closure->actual_params );

			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}

		for( i = 0; i < num_params; ++i )
			{
			closure->formal_params[i] = formal_params[i];
			ns_value_construct( closure->actual_params + i );
			}
		}

	return ns_no_error();
	}


void ns_closure_destruct( NsClosure *closure )
   {
	nssize i;

	ns_assert( NULL != closure );

	for( i = 0; i < closure->num_params; ++i )
		ns_value_destruct( closure->actual_params + i );

	ns_delete( closure->actual_params );
	ns_delete( closure->formal_params );
	}


NsError ns_closure_invoke
	(
	NsClosure        *closure,
	NsValue          *actual_params,
	nssize            num_params,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record
	)
	{
	nssize   i;
	NsError  error;


	ns_assert( NULL != closure );

	if( num_params != closure->num_params )
		ns_warning(
			NS_WARNING_LEVEL_CRITICAL
			NS_MODULE
			" Invalid number of parameters("
			NS_FMT_ULONG
			" instead of "
			NS_FMT_ULONG
			") passed to closure.",
			num_params,
			closure->num_params
			);


	if( 0 < num_params )
		{
		ns_assert( NULL != actual_params );

		for( i = 0; i < num_params; ++i )
			{
			if( ! ns_value_is( actual_params + i, closure->formal_params[i].type ) )
				{
				ns_warning(
					NS_WARNING_LEVEL_CRITICAL
					NS_MODULE
					" Invalid type passed to closure parameter "
					NS_FMT_STRING_QUOTED
					".",
					closure->formal_params[i].name
					);
				}
			}
		}

	if( ! ns_ascii_streq( NS_VALUE_VOID, closure->formal_ret_value.type ) )
		{
		ns_assert( NULL != actual_ret_value );

		if( ! ns_value_is( actual_ret_value, closure->formal_ret_value.type ) )
			ns_warning(
				NS_WARNING_LEVEL_CRITICAL
				NS_MODULE
				" Invalid type passed to closure return value "
				NS_FMT_STRING_QUOTED
				".",
				closure->formal_ret_value.name
				);
		}

	error = ( closure->marshal )( closure, actual_params, num_params, actual_ret_value );

	if( NULL != record )
		_ns_closure_record_create( record, closure, actual_params, num_params, actual_ret_value );

	return error;
	}


NsError ns_closure_run
	(
	NsClosure        *closure,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record,
	...
	)
	{
	ns_va_list  args;
	NsError     error;


	ns_assert( NULL != closure );

	error = ns_no_error();

	ns_va_start( args, record );
	error = ns_closure_vrun( closure, args, actual_ret_value, record );
	ns_va_end( args );

	return error;
	}


NsError ns_closure_vrun
	(
	NsClosure        *closure,
	ns_va_list        args,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record
	)
	{
	nssize i;

	ns_assert( NULL != closure );

	for( i = 0; i < closure->num_params; ++i )
		ns_value_vset( closure->actual_params + i, closure->formal_params[i].type, &args );

	/* TEMP??? */
	for( i = 0; i < closure->num_params; ++i )
		ns_assert( ns_value_is( closure->actual_params + i, closure->formal_params[i].type ) );

	return ns_closure_invoke(
				closure,
				closure->actual_params,
				closure->num_params,
				actual_ret_value,
				record
				);
	}


NsClosureMarshal ns_closure_get_marshal( const NsClosure *closure )
	{
	ns_assert( NULL != closure );
	return closure->marshal;
	}


void ns_closure_set_marshal( NsClosure *closure, NsClosureMarshal marshal )
	{
	ns_assert( NULL != closure );
	ns_assert( NULL != marshal );

	closure->marshal = marshal;
	}


nspointer ns_closure_get_user_data( const NsClosure *closure )
	{
	ns_assert( NULL != closure );
	return ( nspointer )closure->user_data;
	}


void ns_closure_set_user_data( NsClosure *closure, nspointer user_data )
	{
	ns_assert( NULL != closure );
	closure->user_data = user_data;
	}


NsFunc ns_closure_get_user_func( const NsClosure *closure )
	{
	ns_assert( NULL != closure );
	return closure->user_func;
	}


void ns_closure_set_user_func( NsClosure *closure, NsFunc user_func )
	{
	ns_assert( NULL != closure );
	closure->user_func = user_func;
	}
