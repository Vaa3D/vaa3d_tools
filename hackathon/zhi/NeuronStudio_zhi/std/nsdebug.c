#include "nsdebug.h"


#ifdef NS_DEBUG

NS_PRIVATE void _ns_assert_std
	(
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details_or_file,
	nsint          line
	)
   {
	ns_print_newline();

	ns_println(
		"ns_assert( " NS_FMT_STRING_QUOTED " ) failed at " NS_FMT_STRING,
		expression,
		module
		);

	if( NULL != details_or_file )
		ns_println( "details/file: " NS_FMT_STRING, details_or_file );

	if( 0 < line )
		ns_println( "line: " NS_FMT_INT, line );

	if( NS_ASSERT_POLICY_ABORT == ns_assert_policy_get() )
		ns_abort();
   }


NS_PRIVATE NsAssertFunc _ns_assert_func = NULL;


NsAssertFunc ns_assert_get( void )
	{  return NULL != _ns_assert_func ? _ns_assert_func : _ns_assert_std;  }
   

NsError ns_assert_set( NsAssertFunc func )
	{
	_ns_assert_func = func;
	return ns_no_error();
	}


NS_PRIVATE NsAssertPolicy _ns_assert_policy = NS_ASSERT_POLICY_ABORT;


NsAssertPolicy ns_assert_policy_get( void )
	{  return _ns_assert_policy;  }


NsError ns_assert_policy_set( NsAssertPolicy policy )
	{
	_ns_assert_policy = policy;
	return ns_no_error();
	}

#endif/* NS_DEBUG */


nsboolean _ns_assert_not_reached( void )
	{  return NS_FALSE;  }


NS_PRIVATE void _ns_verify_std
	(
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details
	)
   {
	ns_print_newline();

	ns_println(
		"ns_verify( " NS_FMT_STRING_QUOTED " ) failed at " NS_FMT_STRING,
		expression,
		module
		);

	if( NULL != details )
		ns_println( "details: " NS_FMT_STRING, details );
   }


NS_PRIVATE NsVerifyFunc _ns_verify_func = NULL;


NsVerifyFunc ns_verify_get( void )
	{  return NULL != _ns_verify_func ? _ns_verify_func : _ns_verify_std;  }
   

NsError ns_verify_set( NsVerifyFunc func )
	{
	_ns_verify_func = func;
	return ns_no_error();
	}


NS_PRIVATE nsint _ns_warning_parse_level( const nschar *warning )
	{
	nsint level = 0;

	if( '<' == warning[ 0 ] )
		if( '1' == warning[ 1 ] ||
			 '2' == warning[ 1 ] ||
			 '3' == warning[ 1 ] ||
			 '4' == warning[ 1 ]   )
			if( '>' == warning[ 2 ] )
				level = ( nsint )( warning[ 1 ] - '0' );

	return level;
	}


NS_PRIVATE void _ns_warning_std( const nschar *warning, ns_va_list args )
	{
	nsint level;

	ns_assert( NULL != warning );
	ns_print( "**warning" );

	level = _ns_warning_parse_level( warning );

	if( 0 != level )
		{
		warning += 3;

		switch( level )
			{
			case 1:
				ns_print( "(level::caution)" );
				break;

			case 2:
				ns_print( "(level::recoverable)" );
				break;

			case 3:
				ns_print( "(level::critical)" );
				break;

			case 4:
				ns_print( "(level::fatal)" );
				break;
			}
		}

	ns_print( "** " );
	ns_vprint( warning, args );
	ns_print_newline();
	}


void ns_warning( const nschar *warning, ... )
	{
	ns_va_list args;

	ns_va_start( args, warning );
	( ns_warning_get() )( warning, args );
	ns_va_end( args );
	}


NS_PRIVATE NsWarningFunc _ns_warning_func = NULL;


NsWarningFunc ns_warning_get( void )
	{  return NULL != _ns_warning_func ? _ns_warning_func : _ns_warning_std;  }


NsError ns_warning_set( NsWarningFunc func )
	{
	_ns_warning_func = func;
	return ns_no_error();
	}


nschar* ns_assert_format
	(
	nschar        *string,
	nssize         in_bytes,
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details
	)
	{
	nsint out_bytes;

	out_bytes = ns_snprint(
						string,
						in_bytes,
						"ns_assert( "
						NS_FMT_STRING_QUOTED
						" ) failed at "
						NS_FMT_STRING,
						expression,
						module
						);

	if( 1 <= out_bytes && NULL != details )
		{
		/* NOTE: -1 to overwrite the null terminator. */
		--out_bytes;

		ns_snprint(
			string + out_bytes,
			in_bytes - ( nssize )out_bytes,
			NS_STRING_NEWLINE
			"details: "
			NS_FMT_STRING,
			details
			);
		}

	return string;
	}


nschar* ns_verify_format
	(
	nschar        *string,
	nssize         in_bytes,
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details
	)
	{
	nsint out_bytes;

	out_bytes = ns_snprint(
						string,
						in_bytes,
						"ns_verify( "
						NS_FMT_STRING_QUOTED
						" ) failed at "
						NS_FMT_STRING,
						expression,
						module
						);

	if( 1 <= out_bytes && NULL != details )
		{
		/* NOTE: -1 to overwrite the null terminator. */
		--out_bytes;

		ns_snprint(
			string + out_bytes,
			in_bytes - ( nssize )out_bytes,
			NS_STRING_NEWLINE
			"details: "
			NS_FMT_STRING,
			details
			);
		}

	return string;
	}


nschar* ns_warning_format
	(
	nschar        *string,
	nssize         in_bytes,
	const nschar  *warning,
	ns_va_list     args
	)
	{
	nsint          ilevel;
	const nschar  *slevel;
	nsint          out_bytes;


	ilevel = _ns_warning_parse_level( warning );
	slevel = "";

	if( 0 != ilevel )
		{
		warning += 3;

		switch( ilevel )
			{
			case 1:
				slevel = "(level::caution)";
				break;

			case 2:
				slevel = "(level::recoverable)";
				break;

			case 3:
				slevel = "(level::critical)";
				break;

			case 4:
				slevel = "(level::fatal)";
				break;
			}
		}

	out_bytes = ns_snprint(
						string,
						in_bytes,
						"**warning"
						NS_FMT_STRING
						"** ",
						slevel
						);

	if( 1 <= out_bytes )
		{
		/* NOTE: -1 to overwrite the null terminator. */
		--out_bytes;

		ns_vsnprint(
			string + out_bytes,
			in_bytes - ( nssize )out_bytes,
			warning,
			args
			);
		}

	return string;
	}
