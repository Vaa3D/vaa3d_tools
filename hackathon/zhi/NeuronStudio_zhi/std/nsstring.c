#include "nsstring.h"


NS_PRIVATE nssize _ns_string_closest_power_of_2( nssize current, nssize value )
	{
	if( NS_SIZE_MAX / 2 < value )
		return NS_SIZE_MAX;

	if( 0 == current )
		current = 1;

	while( current < value )
		current <<= 1; /* i.e. multiply by 2 */

	return current;
	}


NS_PRIVATE NsError _ns_string_do_construct( NsString *string, nssize size, const nschar *s )
   {
	NsError error;

	string->size = 0;

	/* NOTE: Assure successful construction of the internal byte array by
		initially passing 0 as the size of array. */
	ns_verify( NS_SUCCESS( ns_byte_array_construct( &string->buffer, 0 ), error ) );

	if( NULL != s )
		error = ns_string_set( string, s );
	else if( 0 < size )
		error = ns_string_resize( string, size );
	else
		error = ns_no_error();

	if( ns_is_error( error ) )
		ns_byte_array_destruct( &string->buffer );

	return error;
   }


void ns_string_destruct( NsString *string )
   {
	ns_assert( NULL != string );

	ns_string_clear( string );
	ns_byte_array_destruct( &string->buffer );
	}


void ns_string_construct( NsString *string )
	{
	NsError error;

	ns_assert( NULL != string );
	ns_verify( NS_SUCCESS( _ns_string_do_construct( string, 0, NULL ), error ) );
	}


NsError ns_string_construct_init( NsString *string, const nschar *s )
	{
	ns_assert( NULL != string );
	return _ns_string_do_construct( string, 0, s );
	}


NsError ns_string_construct_sized( NsString *string, nssize size )
	{
	ns_assert( NULL != string );
	return _ns_string_do_construct( string, size, NULL );
	}


void ns_string_clear( NsString *string )
	{
	ns_assert( NULL != string );

	ns_byte_array_clear( &string->buffer );
	string->size = 0;
	}


nsboolean ns_string_is_null( const NsString *string )
	{
	ns_assert( NULL != string );
	return 0 == ns_string_capacity( string );
	}


const nschar* ns_string_get( const NsString *string )
	{
	ns_assert( NULL != string );
	return ns_string_is_null( string ) ? NULL : ns_string_begin( string );
	}


NsError ns_string_set( NsString *string, const nschar *s )
	{
	nssize   size;
	NsError  error;


	ns_assert( NULL != string );

	size = ns_ascii_strsize( s );

	if( NS_FAILURE( ns_string_resize( string, size ), error ) )
		return error;

	ns_byte_array_memcpy( &string->buffer, s, size );
	string->size = size;

	return ns_no_error();
	}


NsError ns_string_empty( NsString *string )
	{
	ns_assert( NULL != string );
	return ns_string_set( string, "" );
	}


NsError ns_string_resize( NsString *string, nssize size )
	{
	ns_assert( NULL != string );

	/* NOTE: Not allowing the string to shrink. */
	if( size <= ns_string_capacity( string ) )
		return ns_no_error();

	return ns_byte_array_resize(
				&string->buffer,
				_ns_string_closest_power_of_2( ns_string_capacity( string ), size )
				);
	}


nssize ns_string_size( const NsString *string )
	{
	ns_assert( NULL != string );
	return string->size;
	}


nssize ns_string_capacity( const NsString *string )
	{
	ns_assert( NULL != string );
	return ns_byte_array_size( &string->buffer );
	}


/* NOTE: -1 to discard the NULL terminator. */
#define _ns_string_length( string )\
	( (string)->size - 1 )


nssize ns_string_length( const NsString *string )
	{
	ns_assert( NULL != string );
	return 0 < string->size ? _ns_string_length( string ) : 0;

	/* return ns_string_is_null( string ) ?
			 0 : ns_ascii_strlen( ns_string_begin( string ) ); */
	}


NsError ns_string_append( NsString *string, const nschar *s )
	{
	ns_assert( NULL != string );
	ns_assert( NULL != s );

	return ns_string_insert( string, ns_string_length( string ), s );
	}


NsError ns_string_append_c( NsString *string, nsunichar c )
	{
	ns_assert( NULL != string );
	return ns_string_insert_c( string, ns_string_length( string ), c );
	}


NsError ns_string_prepend( NsString *string, const nschar *s )
	{
	ns_assert( NULL != string );
	ns_assert( NULL != s );

	return ns_string_insert( string, 0, s );
	}


NsError ns_string_prepend_c( NsString *string, nsunichar c )
	{
	ns_assert( NULL != string );
	return ns_string_insert_c( string, 0, c );
	}


NS_PRIVATE NsError _ns_string_insert( NsString *string, nssize at, nssize bytes )
	{
	nschar   *s;
	NsError   error;


	if( NS_FAILURE( ns_string_resize( string, string->size + bytes ), error ) )
		return error;

	s = ns_string_at( string, at );
	ns_memmove( s + bytes, s, ns_ascii_strsize( s ) );

	string->size += bytes;

	return ns_no_error();
	}


NsError ns_string_insert( NsString *string, nssize at, const nschar *s )
	{
	nssize   bytes;
	NsError  error;


	ns_assert( NULL != string );

	/* Set the string to the empty string if NULL. This allows appending
		or prepending to a string which is initially NULL. */
	if( ns_string_is_null( string ) )
		if( NS_FAILURE( ns_string_set( string, "" ), error ) )
			return error;

	ns_assert( at <= ns_string_length( string ) );
	ns_assert( NULL != s );

	/* NOTE: -1 to discard the null terminator. */
	bytes = ns_ascii_strsize( s ) - 1;

	if( NS_FAILURE( _ns_string_insert( string, at, bytes ), error ) )
		return error;

	ns_memcpy( ns_string_at( string, at ), s, bytes );

	return ns_no_error();
	}


NsError ns_string_insert_c( NsString *string, nssize at, nsunichar c )
	{
	nschar s[ 16 ]; /* Way more room than needed! */

	ns_assert( NULL != string );

	ns_sprint( s, NS_FMT_UNICHAR, c );
	return ns_string_insert( string, at, s );
	}


NsError ns_string_splice_in_c( NsString *string, nschar *I, nsunichar c )
	{
	ns_assert( NULL != string );
	return ns_string_insert_c( string, ns_string_distance( string, I ), c );
	}


void ns_string_delete( NsString *string, nssize at, nssize count )
	{
	nschar *s;

	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	/* NOTE: Dont allow null terminator to be deleted,
		i.e. assert using '<' instead of '<='. */
	ns_assert( at < ns_string_length( string ) );

	if( 0 == count )
		return;

	s = ns_string_at( string, at );
	ns_assert( count <= ns_ascii_strlen( s ) );

	/* Shift the rest of the string over. */
	ns_memmove( s, s + count, ns_ascii_strsize( s + count ) );

	string->size -= count;
	}


void ns_string_delete_c( NsString *string, nssize at )
	{
	ns_assert( NULL != string );
	ns_string_delete( string, at, 1 );
	}


void ns_string_splice_out_c( NsString *string, nschar *I )
	{
	ns_assert( NULL != string );
	ns_string_delete_c( string, ns_string_distance( string, I ) );
	}


NsError ns_string_splice_out( NsString *src, nssize at, nssize count, NsString *dest )
	{
	nschar   *I;
	nssize    n;
	NsError   error;


	ns_assert( NULL != src );
	ns_assert( NULL != dest );
	ns_assert( at < ns_string_length( src ) );

	if( 0 == count )
		return ns_no_error();

	/* Have to save count to use in final delete operation. */
	n = count;

	ns_string_resize( dest, 0 );

	I = ns_string_at( src, at );
	ns_assert( count <= ns_ascii_strlen( I ) );

	while( 0 < count )
		{
		ns_assert( NS_ASCII_NULL != *I );
		if( NS_FAILURE( ns_string_append_c( dest, *I ), error ) )
			return error;

		++I;
		--count;
		}

	ns_string_delete( src, at, n );
	return ns_no_error();
	}


void ns_string_upper( NsString *string )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	ns_ascii_strupr( ns_string_begin( string ) );
	}


void ns_string_lower( NsString *string )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	ns_ascii_strlwr( ns_string_begin( string ) );
	}


nsuint ns_string_hash( const NsString *string )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	return ns_ascii_hash( ns_string_begin( string ) );
	}


NsError ns_string_getcwd( NsString *string )
	{
	NsError  error;
	nschar   cwd[ NS_PATH_SIZE ];


	ns_assert( NULL != string );

	if( NS_FAILURE( ns_getcwd( cwd, NS_PATH_SIZE ), error ) )
		return error;

	return ns_string_set( string, cwd );
	}


NsError ns_string_vformat( NsString *string, const nschar *format, ns_va_list lock_args )
	{
    return ns_no_error();
	}


NsError ns_string_format( NsString *string, const nschar *format, ... )
	{
    return ns_no_error();
	}


NsError ns_string_vformat_and_append( NsString *string, const nschar *format, ns_va_list args )
	{
	NsString  temp;
	NsError   error;


	ns_assert( NULL != string );
	ns_assert( NULL != format );
	ns_assert( NULL != args );

	ns_string_construct( &temp );

	if( NS_FAILURE( ns_string_vformat( &temp, format, args ), error ) )
		{
		ns_string_destruct( &temp );
		return error;
		}

	if( NS_FAILURE( ns_string_append( string, ns_string_get( &temp ) ), error ) )
		{
		ns_string_destruct( &temp );
		return error;
		}

	ns_string_destruct( &temp );
	return ns_no_error();
	}


NsError ns_string_format_and_append( NsString *string, const nschar *format, ... )
	{
	ns_va_list  args;
	NsError     error;


	ns_assert( NULL != string );
	ns_assert( NULL != format );

	ns_va_start( args, format );
	error = ns_string_vformat_and_append( string, format, args );
	ns_va_end( args );

	return error;
	}


nssize ns_string_distance( const NsString *string, const nschar *I )
	{
	const nschar  *F, *L;
	nssize         d;


	ns_assert( NULL != string );

	d = 0;
	F = ns_string_begin( string );
	L = ns_string_end( string );

	for( ; F != I && F != L; ++F )
		++d;

	return d;
	}


nschar* ns_string_begin( const NsString *string )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	return ( nschar* )ns_byte_array_begin( &string->buffer );
	}


nschar* ns_string_end( const NsString *string )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );

	return ns_string_begin( string ) + _ns_string_length( string );
	}


nschar* ns_string_at( const NsString *string, nssize index )
	{
	ns_assert( NULL != string );
	ns_assert( ! ns_string_is_null( string ) );
	ns_assert( index <= ns_string_length( string ) );

	return ns_string_begin( string ) + index;
	}


nsunichar ns_string_getc( const NsString *string, nssize at )
	{
	ns_assert( NULL != string );
	return *( ns_string_at( string, at ) );
	}


void ns_string_setc( NsString *string, nssize at, nsunichar c )
	{
	ns_assert( NULL != string );
	*( ns_string_at( string, at ) ) = ( nschar )c;
	}


nschar* ns_string_find( const NsString *string, const nschar *s )
	{
	ns_assert( NULL != string );
	ns_assert( NULL != s );

	return ns_ascii_strstr( ns_string_begin( string ), s );
	}


nschar* ns_string_find_c( const NsString *string, nsunichar c )
	{
	ns_assert( NULL != string );
	return ns_ascii_strchr( ns_string_begin( string ), c );
	}


const nschar* ns_string_find_from( const NsString *string, const nschar *at, const nschar *s )
	{
	ns_assert( NULL != string );
	ns_assert( NULL != at );
	ns_assert( NULL != s );

	return ns_ascii_strstr( at, s );
	}


const nschar* ns_string_find_c_from( const NsString *string, const nschar *at, nsunichar c )
	{
	ns_assert( NULL != string );
	ns_assert( NULL != at );

	return ns_ascii_strchr( at, c );
	}
