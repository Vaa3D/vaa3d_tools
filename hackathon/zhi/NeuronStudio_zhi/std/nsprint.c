#include "nsprint.h"


NsError ns_vsnprint( nschar *buffer, nssize bytes, const nschar *format, ns_va_list args )
	{  return _ns_vsnprint( buffer, bytes, format, args );  }


#define _NS_PRINT_BUFFER_SIZE  4096


NsError ns_vsprint( nschar *buffer, const nschar *format, ns_va_list args )
	{  return ns_vsnprint( buffer, _NS_PRINT_BUFFER_SIZE, format, args );  }


NsError ns_snprint( nschar *buffer, nssize bytes, const nschar *format, ... )
	{
	ns_va_list  args;
	NsError     error;


	ns_va_start( args, format );
	error = ns_vsnprint( buffer, bytes, format, args ); 
	ns_va_end( args );

	return error;
	}


NsError ns_sprint( nschar *buffer, const nschar *format, ... )
	{
	ns_va_list  args;
	NsError     error;


	ns_va_start( args, format );
	error = ns_vsnprint( buffer, _NS_PRINT_BUFFER_SIZE, format, args );
	ns_va_end( args );

	return error;
	}


NsError ns_vprint( const nschar *format, ns_va_list args )
	{
	return ns_print_get_enabled() ?
				_ns_vfprint( ns_print_get_io(), format, args ) : ns_no_error();
	}


NsError ns_print( const nschar *format, ... )
	{
	NsError error;

	if( ns_print_get_enabled() || ns_print_record_is_enabled() )
		{
		ns_va_list args;

		ns_va_start( args, format );
		error = _ns_vfprint( ns_print_get_io(), format, args );
		ns_va_end( args );
		}
	else
		error = ns_no_error();

	return error;
	}


NsError ns_println( const nschar *format, ... )
	{
	NsError error;

	
	if( ns_print_get_enabled() || ns_print_record_is_enabled() )
		{
		ns_va_list  args;
		nspointer   io;


		ns_va_start( args, format );
		io = ns_print_get_io();

		error = _ns_vfprint( io, format, args );

		if( ! ns_is_error( error ) )
			error = _ns_vfprint( io, NS_STRING_NEWLINE, args );

		ns_va_end( args );
		}
	else
		error = ns_no_error();

	return error;
	}


NS_PRIVATE nspointer _ns_print_io = NULL;


nspointer ns_print_get_io( void )
	{  return NULL != _ns_print_io ? _ns_print_io : NS_INT_TO_POINTER( _NS_STDOUT );  }


void ns_print_set_io( nspointer io )
	{  _ns_print_io = io;  }


NS_PRIVATE nsboolean _ns_print_is_gui = NS_FALSE;


nsboolean ns_print_get_is_gui( void )
	{  return _ns_print_is_gui;  }


void ns_print_set_is_gui( nsboolean is_gui )
	{  _ns_print_is_gui = is_gui;  }


NS_PRIVATE nsboolean _ns_print_enabled = NS_FALSE;


nsboolean ns_print_get_enabled( void )
	{  return _ns_print_enabled;  }


void ns_print_set_enabled( nsboolean enabled )
	{  _ns_print_enabled = enabled;  }

