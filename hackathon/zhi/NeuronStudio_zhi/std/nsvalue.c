#include "nsvalue.h"


NS_STATIC_MUTEX_IMPLEMENT( ns_value_static_mutex );


#define _NS_VALUE_NAME_SIZE  64


typedef struct _NsValueType
	{
	nssize  size;
	void    ( *reset )( NsValue* );
	void    ( *unset )( NsValue* );
	void    ( *arg )( NsValue*, ns_va_list* );
	nschar  name[ _NS_VALUE_NAME_SIZE ];
	}
	NsValueType;


NS_PRIVATE NsError ns_value_type_new
	(
	NsValueType   **type,
	const nschar   *name,
	nssize          size,
	void           ( *reset )( NsValue* ),
	void           ( *unset )( NsValue* ),
	void           ( *arg )( NsValue*, ns_va_list* )
	)
	{
	ns_assert( NULL != type );

	if( NULL == ( *type = ns_new( NsValueType ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*type)->size  = size;
	(*type)->reset = reset;
	(*type)->unset = unset;
	(*type)->arg   = arg;

	ns_assert( ns_ascii_strlen( name ) < _NS_VALUE_NAME_SIZE );
	ns_ascii_strcpy( (*type)->name, name );

	return ns_no_error();
	}


NS_PRIVATE void ns_value_type_delete( NsValueType *type )
	{
	ns_assert( NULL != type ); 
	ns_delete( type );
	}


NS_PRIVATE NsHashTable* ns_value_db( void )
	{
	NS_PRIVATE NsHashTable _ns_value_db;
	return &_ns_value_db;
	}


NS_PRIVATE nsboolean _ns_value_db_did_init = NS_FALSE;


NsError _ns_value_db_init( void )
	{
	NsError error;

	ns_verify( ! _ns_value_db_did_init );

	if( NS_SUCCESS( ns_hash_table_construct(
							ns_value_db(),
							ns_ascii_hash,
							ns_ascii_streq,
							NULL,
							ns_value_type_delete
							),
							error ) )
		_ns_value_db_did_init = NS_TRUE;

	return error;
	}


void _ns_value_db_finalize( void )
	{
	if( _ns_value_db_did_init )
		ns_hash_table_destruct( ns_value_db() );

	_ns_value_db_did_init = NS_FALSE;
	}


NsError ns_value_register
	(
	const nschar  *name,
	nssize         size,
	void          ( *reset )( NsValue* ),
	void          ( *unset )( NsValue* ),
	void          ( *arg )( NsValue*, ns_va_list* )
	)
	{
	NsValueType  *type;
	NsError       error;


	ns_assert( NULL != name );

	error = ns_no_error();

	ns_mutex_lock( ns_value_static_mutex() );

	if( ! ns_hash_table_exists( ns_value_db(), ( nschar* )name ) )
		{
		ns_verify( size <= NS_VALUE_MAX_SIZE );

		if( NS_SUCCESS( ns_value_type_new( &type, name, size, reset, unset, arg ), error ) )
			if( NS_FAILURE( ns_hash_table_add(
									ns_value_db(),
									( nschar* )type->name,
									type
									),
									error ) )
				ns_value_type_delete( type );
		}

	ns_mutex_unlock( ns_value_static_mutex() );

	return error;
	}


NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_char, nschar );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pchar, nschar* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uchar, nsuchar );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puchar, nsuchar* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_short, nsshort );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pshort, nsshort* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_ushort, nsushort );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pushort, nsushort* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_int, nsint );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pint, nsint* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uint, nsuint );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puint, nsuint* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_long, nslong );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_plong, nslong* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_ulong, nsulong );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pulong, nsulong* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_double, nsdouble );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pdouble, nsdouble* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pointer, nspointer );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_constpointer, nsconstpointer );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_boolean, nsboolean );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pboolean, nsboolean* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_size, nssize );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_psize, nssize* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_enum, nsenum );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_penum, nsenum* );

#ifdef NS_HAVE_LONGLONG
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_longlong, nslonglong );
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_plonglong, nslonglong* );

	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_ulonglong, nsulonglong );
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pulonglong, nsulonglong* );
#endif

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_int8, nsint8 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pint8, nsint8* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uint8, nsuint8 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puint8, nsuint8* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_int16, nsint16 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pint16, nsint16* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uint16, nsuint16 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puint16, nsuint16* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_int32, nsint32 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pint32, nsint32* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uint32, nsuint32 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puint32, nsuint32* );

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_float64, nsfloat64 );
NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pfloat64, nsfloat64* );

#ifdef NS_HAVE_INT64
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_int64, nsint64 );
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pint64, nsint64* );

	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_uint64, nsuint64 );
	NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_puint64, nsuint64* );
#endif

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_func, NsFunc );


NS_VALUE_GET_IMPLEMENT( ns_value_get_string, nschar* );


NsValue* ns_value_set_string( NsValue *value, nschar *src )
	{
	nschar *dest;

	ns_assert( NULL != value );
	
	dest = NULL != src ? ns_ascii_strdup( src ) : NULL;
	return ns_value_set( value, NS_VALUE_STRING, ( nspointer )&dest );
	}


NS_PRIVATE void _ns_value_free_string( NsValue *value )
	{
	ns_free( *( ( nschar** )ns_value_get( value ) ) );
	ns_value_memzero( value );
	}


NS_PRIVATE void _ns_value_reset_string( NsValue *value )
	{
	ns_assert( NULL != value );
	_ns_value_free_string( value );
	}


NS_PRIVATE void _ns_value_unset_string( NsValue *value )
	{
	ns_assert( NULL != value );
	_ns_value_free_string( value );
	}


NS_PRIVATE void _ns_value_arg_string( NsValue *value, ns_va_list *args )
	{
	nschar *src, *dest;

	ns_assert( NULL != value );
	ns_assert( NULL != args );

	src  = ns_va_arg( *args, nschar* );

	dest = NULL != src ? ns_ascii_strdup( src ) : NULL;
	ns_value_set_unsync( value, NS_VALUE_STRING, ( nspointer )&dest );
	}


/* IMPORTANT: Special case(s). float's (and float32's) are passed
	as doubles when using variable arguments!!! */
void _ns_value_arg_float( NsValue *value, ns_va_list *args )
	{
	nsfloat v;

	ns_assert( NULL != value );
	ns_assert( NULL != args );

	v = ( nsfloat )ns_va_arg( *args, nsdouble );
	ns_value_set_unsync( value, NS_VALUE_FLOAT, ( nspointer )&v );
	}

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pfloat, nsfloat* );


void _ns_value_arg_float32( NsValue *value, ns_va_list *args )
	{
	nsfloat32 v;

	ns_assert( NULL != value );
	ns_assert( NULL != args );

	v = ( nsfloat32 )ns_va_arg( *args, nsdouble );
	ns_value_set_unsync( value, NS_VALUE_FLOAT32, ( nspointer )&v );
	}

NS_VALUE_ARG_IMPLEMENT( _ns_value_arg_pfloat32, nsfloat32* );


typedef void ( *_NsValueArgFunc )( ns_va_list*, nspointer );


NsError ns_value_register_std( void )
	{
	NsError error;

	#define _NS_VALUE_REGISTER_TYPE( lcase, ucase )\
		if( NS_FAILURE( ns_value_register(\
								NS_VALUE_##ucase,\
								sizeof( ns##lcase ),\
								NULL,\
								NULL,\
								_ns_value_arg_##lcase\
								),\
								error ) )\
			return error

	#define _NS_VALUE_REGISTER_PTYPE( lcase, ucase )\
		if( NS_FAILURE( ns_value_register(\
								NS_VALUE_P##ucase,\
								sizeof( ns##lcase* ),\
								NULL,\
								NULL,\
								_ns_value_arg_p##lcase\
								),\
								error ) )\
			return error

	_NS_VALUE_REGISTER_TYPE( char, CHAR );
	_NS_VALUE_REGISTER_PTYPE( char, CHAR );

	_NS_VALUE_REGISTER_TYPE( uchar, UCHAR );
	_NS_VALUE_REGISTER_PTYPE( uchar, UCHAR );

	_NS_VALUE_REGISTER_TYPE( short, SHORT );
	_NS_VALUE_REGISTER_PTYPE( short, SHORT );

	_NS_VALUE_REGISTER_TYPE( ushort, USHORT );
	_NS_VALUE_REGISTER_PTYPE( ushort, USHORT );

	_NS_VALUE_REGISTER_TYPE( int, INT );
	_NS_VALUE_REGISTER_PTYPE( int, INT );

	_NS_VALUE_REGISTER_TYPE( uint, UINT );
	_NS_VALUE_REGISTER_PTYPE( uint, UINT );

	_NS_VALUE_REGISTER_TYPE( long, LONG );
	_NS_VALUE_REGISTER_PTYPE( long, LONG );

	_NS_VALUE_REGISTER_TYPE( ulong, ULONG );
	_NS_VALUE_REGISTER_PTYPE( ulong, ULONG );

	_NS_VALUE_REGISTER_TYPE( float, FLOAT );
	_NS_VALUE_REGISTER_PTYPE( float, FLOAT );

	_NS_VALUE_REGISTER_TYPE( double, DOUBLE );
	_NS_VALUE_REGISTER_PTYPE( double, DOUBLE );

	_NS_VALUE_REGISTER_TYPE( pointer, POINTER );

	_NS_VALUE_REGISTER_TYPE( constpointer, CONSTPOINTER );

	_NS_VALUE_REGISTER_TYPE( boolean, BOOLEAN );
	_NS_VALUE_REGISTER_PTYPE( boolean, BOOLEAN );

	_NS_VALUE_REGISTER_TYPE( size, SIZE );
	_NS_VALUE_REGISTER_PTYPE( size, SIZE );

	_NS_VALUE_REGISTER_TYPE( enum, ENUM );
	_NS_VALUE_REGISTER_PTYPE( enum, ENUM );

#ifdef NS_HAVE_LONGLONG
	_NS_VALUE_REGISTER_TYPE( longlong, LONGLONG );
	_NS_VALUE_REGISTER_PTYPE( longlong, LONGLONG );

	_NS_VALUE_REGISTER_TYPE( ulonglong, ULONGLONG );
	_NS_VALUE_REGISTER_PTYPE( ulonglong, ULONGLONG );
#endif

	_NS_VALUE_REGISTER_TYPE( int8, INT8 );
	_NS_VALUE_REGISTER_PTYPE( int8, INT8 );

	_NS_VALUE_REGISTER_TYPE( uint8, UINT8 );
	_NS_VALUE_REGISTER_PTYPE( uint8, UINT8 );

	_NS_VALUE_REGISTER_TYPE( int16, INT16 );
	_NS_VALUE_REGISTER_PTYPE( int16, INT16 );

	_NS_VALUE_REGISTER_TYPE( uint16, UINT16 );
	_NS_VALUE_REGISTER_PTYPE( uint16, UINT16 );

	_NS_VALUE_REGISTER_TYPE( int32, INT32 );
	_NS_VALUE_REGISTER_PTYPE( int32, INT32 );

	_NS_VALUE_REGISTER_TYPE( uint32, UINT32 );
	_NS_VALUE_REGISTER_PTYPE( uint32, UINT32 );

	_NS_VALUE_REGISTER_TYPE( float32, FLOAT32 );
	_NS_VALUE_REGISTER_PTYPE( float32, FLOAT32 );

	_NS_VALUE_REGISTER_TYPE( float64, FLOAT64 );
	_NS_VALUE_REGISTER_PTYPE( float64, FLOAT64 );

#ifdef NS_HAVE_INT64
	_NS_VALUE_REGISTER_TYPE( int64, INT64 );
	_NS_VALUE_REGISTER_PTYPE( int64, INT64 );

	_NS_VALUE_REGISTER_TYPE( uint64, UINT64 );
	_NS_VALUE_REGISTER_PTYPE( uint64, UINT64 );
#endif

	/* Special cases where the above macros wont work. */

	if( NS_FAILURE( ns_value_register(
							NS_VALUE_UNKNOWN,
							0,
							NULL,
							NULL,
							NULL
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_value_register(
							NS_VALUE_VOID,
							0,
							NULL,
							NULL,
							NULL
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_value_register(
							NS_VALUE_FUNC,
							sizeof( NsFunc ),
							NULL,
							NULL,
							_ns_value_arg_func
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_value_register(
							NS_VALUE_STRING,
							sizeof( nschar* ),
							_ns_value_reset_string,
							_ns_value_unset_string,
							_ns_value_arg_string
							),
							error ) )
		return error;

	return ns_no_error();
	}


nssize ns_value_registry_size( void )
	{
	nssize size;

	ns_mutex_lock( ns_value_static_mutex() );
	size = ns_hash_table_size( ns_value_db() );
	ns_mutex_unlock( ns_value_static_mutex() );

	return size;
	}


void ns_value_unregister( const nschar *name )
	{
	ns_assert( NULL != name );

	ns_mutex_lock( ns_value_static_mutex() );
	ns_hash_table_remove( ns_value_db(), ( nschar* )name );
	ns_mutex_unlock( ns_value_static_mutex() );
	}


nsboolean ns_value_is_registered( const nschar *name )
	{
	nsboolean ret_value;

	ns_assert( NULL != name );

	ns_mutex_lock( ns_value_static_mutex() );
	ret_value = ns_hash_table_exists( ns_value_db(), ( nschar* )name );
	ns_mutex_unlock( ns_value_static_mutex() );

	return ret_value;
	}


nsboolean ns_value_is_registered_unsync( const nschar *name )
	{
	ns_assert( NULL != name );
	return ns_hash_table_exists( ns_value_db(), ( nschar* )name );
	}


NsEnumeration* ns_value_registry_enum( NsEnumeration *enumeration )
	{  return ns_hash_table_key_enum( ns_value_db(), enumeration );  }




NS_PRIVATE NsValueType* _ns_value_type( const nschar *name )
	{
	nspointer type = NULL;

	ns_verify(
		ns_hash_table_lookup(
			ns_value_db(),
			( nschar* )name,
			&type
			)
		);

	ns_assert_with_details( NULL != type, name );
	return type;
	}


/* NOTE: Using simple routines since the size of the
	value->v buffer is small. */


NS_PRIVATE void _ns_value_memcpy( nsuint8 *dest, const nsuint8 *src, nssize bytes )
	{
	for( ; 0 < bytes; --bytes )
		*dest++ = *src++;
	}


NS_PRIVATE nsint _ns_value_memcmp( const nsuint8 *p1, const nsuint8 *p2, nssize bytes )
	{
	for( ; 0 < bytes; --bytes, ++p1, ++p2 )
		if( *p1 != *p2 )
			return ( nsint )( *p1 - *p2 );

	return 0;
	}


void ns_value_construct( NsValue *value )
	{
	ns_assert( NULL != value );
	ns_value_construct_type( value, NS_VALUE_UNKNOWN );
	}


void ns_value_construct_type( NsValue *value, const nschar *name )
	{
	ns_assert( NULL != value );

	#ifdef NS_DEBUG
	ns_cookie_set( value->v, NS_VALUE_MAX_SIZE );
	#endif

	value->name = NS_VALUE_UNKNOWN;
	ns_value_memzero( value );

	ns_value_set_type( value, name );
	}


void ns_value_destruct( NsValue *value )
	{
	#ifdef NS_DEBUG
	ns_cookie_valid( value->v, NS_VALUE_MAX_SIZE );
	#endif

	ns_assert( NULL != value );
	ns_value_unset( value );
	}


nsboolean ns_value_equal( const NsValue *v1, const NsValue *v2 )
	{
	nsint i;

	ns_assert( NULL != v1 );
	ns_assert( NULL != v2 );

	if( ! ns_value_is( v1, v2->name ) )
		return NS_FALSE;

	ns_mutex_lock( ns_value_static_mutex() );
	i = _ns_value_memcmp( v1->v, v2->v, _ns_value_type( v1->name )->size );
	ns_mutex_unlock( ns_value_static_mutex() );

	return 0 == i;
	}


void ns_value_assign( NsValue *dest, const NsValue *src )
	{
	ns_assert( NULL != dest );
	ns_assert( NULL != src );

	ns_value_unset( dest );

	dest->name = src->name;

	ns_mutex_lock( ns_value_static_mutex() );
	_ns_value_memcpy( dest->v, src->v, _ns_value_type( src->name )->size );
	ns_mutex_unlock( ns_value_static_mutex() );
	}


nspointer ns_value_get( const NsValue *value )
	{
	ns_assert( NULL != value );
	return ( nspointer )value->v;
	}


NsValue* ns_value_set_unsync( NsValue *value, const nschar *name, nspointer v )
	{
	const NsValueType *type;

	ns_assert( NULL != value );
	ns_assert( NULL != name );
	ns_verify_with_details( ns_value_is_registered_unsync( name ), name );

	ns_value_unset_unsync( value );

	type = _ns_value_type( name );

	value->name = type->name;
	_ns_value_memcpy( value->v, v, type->size );

	return value;
	}


NsValue* ns_value_set( NsValue *value, const nschar *name, nspointer v )
	{
	const NsValueType *type;

	ns_assert( NULL != value );
	ns_assert( NULL != name );
	ns_verify_with_details( ns_value_is_registered( name ), name );

	ns_value_unset( value );

	ns_mutex_lock( ns_value_static_mutex() );

	type = _ns_value_type( name );

	value->name = type->name;
	_ns_value_memcpy( value->v, v, type->size );

	ns_mutex_unlock( ns_value_static_mutex() );
	return value;
	}


NsValue* ns_value_vset( NsValue *value, const nschar *name, ns_va_list *args )
	{
	const NsValueType *type;

	ns_assert( NULL != value );
	ns_assert( NULL != name );
	ns_verify_with_details( ns_value_is_registered( name ), name );

	ns_value_unset( value );

	ns_mutex_lock( ns_value_static_mutex() );

	type = _ns_value_type( name );

	if( NULL != type->arg )
		( type->arg )( value, args );

	ns_mutex_unlock( ns_value_static_mutex() );
	return value;
	}


nsboolean ns_value_is( const NsValue *value, const nschar *name )
	{
	ns_assert( NULL != value );
	ns_assert( NULL != name );

	return ns_ascii_streq( value->name, name );
	}


const nschar* ns_value_get_type( const NsValue *value )
	{
	ns_assert( NULL != value );
	return value->name;
	}


NsValue* ns_value_set_type( NsValue *value, const nschar *name )
	{
	ns_assert( NULL != value );
	ns_assert( NULL != name );
	ns_verify_with_details( ns_value_is_registered( name ), name );

	ns_value_unset( value );

	value->name = name;
	ns_value_reset( value );

	return value;
	}


NsValue* ns_value_reset( NsValue *value )
	{
	const NsValueType *type;

	ns_assert( NULL != value );
	ns_mutex_lock( ns_value_static_mutex() );

	type = _ns_value_type( value->name );

	if( NULL != type->reset )
		( type->reset )( value );

	ns_mutex_unlock( ns_value_static_mutex() );
	return value;
	}


NsValue* ns_value_unset( NsValue *value )
	{
	const NsValueType *type;

	ns_assert( NULL != value );

	ns_mutex_lock( ns_value_static_mutex() );

	type = _ns_value_type( value->name );

	if( NULL != type->unset )
		( type->unset )( value );

	value->name = NS_VALUE_UNKNOWN;

	ns_mutex_unlock( ns_value_static_mutex() );
	return value;
	}


NsValue* ns_value_unset_unsync( NsValue *value )
	{
	const NsValueType *type;

	ns_assert( NULL != value );

	type = _ns_value_type( value->name );

	if( NULL != type->unset )
		( type->unset )( value );

	value->name = NS_VALUE_UNKNOWN;

	return value;
	}


NsValue* ns_value_memzero( NsValue *value )
	{
	ns_assert( NULL != value );

	ns_memzero( value->v, NS_VALUE_MAX_SIZE );
	return value;
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_char,  nschar  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_char,  nschar  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pchar, nschar* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pchar, nschar* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_uchar,  nsuchar  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_uchar,  nsuchar  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_puchar, nsuchar* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_puchar, nsuchar* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_short,  nsshort  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_short,  nsshort  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pshort, nsshort* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pshort, nsshort* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_ushort,  nsushort  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_ushort,  nsushort  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pushort, nsushort* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pushort, nsushort* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_int,  nsint  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_int,  nsint  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pint, nsint* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pint, nsint* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_uint,  nsuint  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_uint,  nsuint  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_puint, nsuint* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_puint, nsuint* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_long,  nslong  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_long,  nslong  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_plong, nslong* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_plong, nslong* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_ulong,  nsulong  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_ulong,  nsulong  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pulong, nsulong* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pulong, nsulong* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_float,  nsfloat  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_float,  nsfloat  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pfloat, nsfloat* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pfloat, nsfloat* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_double,  nsdouble  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_double,  nsdouble  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pdouble, nsdouble* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pdouble, nsdouble* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_pointer, nspointer );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pointer, nspointer );

NS_VALUE_GET_IMPLEMENT( ns_value_get_constpointer, nsconstpointer );
NS_VALUE_SET_IMPLEMENT( ns_value_set_constpointer, nsconstpointer );

NS_VALUE_GET_IMPLEMENT( ns_value_get_boolean,  nsboolean  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_boolean,  nsboolean  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pboolean, nsboolean* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pboolean, nsboolean* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_size,  nssize  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_size,  nssize  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_psize, nssize* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_psize, nssize* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_enum,  nsenum  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_enum,  nsenum  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_penum, nsenum* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_penum, nsenum* );

#ifdef NS_HAVE_LONGLONG
	NS_VALUE_GET_IMPLEMENT( ns_value_get_longlong,  nslonglong  );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_longlong,  nslonglong  );
	NS_VALUE_GET_IMPLEMENT( ns_value_get_plonglong, nslonglong* );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_plonglong, nslonglong* );

	NS_VALUE_GET_IMPLEMENT( ns_value_get_ulonglong,  nsulonglong  );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_ulonglong,  nsulonglong  );
	NS_VALUE_GET_IMPLEMENT( ns_value_get_pulonglong, nsulonglong* );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_pulonglong, nsulonglong* );
#endif

NS_VALUE_GET_IMPLEMENT( ns_value_get_int8,  nsint8  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_int8,  nsint8  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pint8, nsint8* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pint8, nsint8* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_uint8,  nsuint8  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_uint8,  nsuint8  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_puint8, nsuint8* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_puint8, nsuint8* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_int16,  nsint16  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_int16,  nsint16  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pint16, nsint16* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pint16, nsint16* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_uint16,  nsuint16  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_uint16,  nsuint16  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_puint16, nsuint16* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_puint16, nsuint16* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_int32,  nsint32  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_int32,  nsint32  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pint32, nsint32* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pint32, nsint32* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_uint32,  nsuint32  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_uint32,  nsuint32  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_puint32, nsuint32* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_puint32, nsuint32* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_float32,  nsfloat32  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_float32,  nsfloat32  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pfloat32, nsfloat32* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pfloat32, nsfloat32* );

NS_VALUE_GET_IMPLEMENT( ns_value_get_float64,  nsfloat64  );
NS_VALUE_SET_IMPLEMENT( ns_value_set_float64,  nsfloat64  );
NS_VALUE_GET_IMPLEMENT( ns_value_get_pfloat64, nsfloat64* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_pfloat64, nsfloat64* );

#ifdef NS_HAVE_INT64
	NS_VALUE_GET_IMPLEMENT( ns_value_get_int64,  nsint64  );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_int64,  nsint64  );
	NS_VALUE_GET_IMPLEMENT( ns_value_get_pint64, nsint64* );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_pint64, nsint64* );

	NS_VALUE_GET_IMPLEMENT( ns_value_get_uint64,  nsuint64  );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_uint64,  nsuint64  );
	NS_VALUE_GET_IMPLEMENT( ns_value_get_puint64, nsuint64* );
	NS_VALUE_SET_IMPLEMENT( ns_value_set_puint64, nsuint64* );
#endif

NS_VALUE_GET_IMPLEMENT( ns_value_get_func, NsFunc );
NS_VALUE_SET_IMPLEMENT( ns_value_set_func, NsFunc );
