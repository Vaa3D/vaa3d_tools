#include "nsio.h"


nspointer ns_io_fopen( const nschar *name, const nschar *mode, NsIO *io )
	{
	NsError error;

	ns_assert( NULL != name );
	ns_assert( NULL != mode );
	ns_assert( NULL != io );

	if( NS_FAILURE( ns_file_open( io->file, name, ns_file_mode_from_string( mode ) ), error ) )
		return NULL;

	return io->file->stream;
	}


nsint ns_io_fclose( nspointer na, NsIO *io )
	{
	NsError error;

	ns_assert( NULL != io );
	NS_USE_VARIABLE( na );

	if( NS_FAILURE( ns_file_close( io->file ), error ) )
		return NS_EOF;

	return 0;
	}


nssize ns_io_fread( nspointer buffer, nssize size, nssize count, nspointer na, NsIO *io )
	{
	NsError  error;
	nssize   read;


	ns_assert( NULL != buffer );
	ns_assert( NULL != io );
	NS_USE_VARIABLE( na );

	if( NS_FAILURE( ns_file_read( io->file, buffer, size, count, &read ), error ) )
		return 0;

	return read;
	}


nssize ns_io_fwrite( nsconstpointer buffer, nssize size, nssize count, nspointer na, NsIO *io )
	{
	NsError  error;
	nssize   write;


	ns_assert( NULL != buffer );
	ns_assert( NULL != io );
	NS_USE_VARIABLE( na );

	if( NS_FAILURE( ns_file_write( io->file, buffer, size, count, &write ), error ) )
		return 0;

	return write;
	}


nsint ns_io_remove( const nschar *name, NsIO *io )
	{
	NsError error;

	ns_assert( NULL != name );
	NS_USE_VARIABLE( io );

	if( NS_FAILURE( ns_remove( name ), error ) )
		return -1;

	return 0;
	}


nsint ns_io_fseek( nspointer na, nssize offset, nsboolean forwards, nsint which, NsIO *io )
	{
	NsError error;

	ns_assert( NULL != io );
	NS_USE_VARIABLE( na );

	if( NS_FAILURE( ns_file_seek( io->file, offset, forwards, which ), error ) )
		return -1;

	return 0;
	}


nsint ns_io_ftell( nspointer na, nssize *position, NsIO *io )
	{
	NsError error;

	ns_assert( NULL != position );
	ns_assert( NULL != io );
	NS_USE_VARIABLE( na );

	if( NS_FAILURE( ns_file_tell( io->file, position ), error ) )
		return -1;

	return 0;
	}


nspointer ns_io_malloc( nssize bytes, NsIO *io )
	{
	NS_USE_VARIABLE( io );
	return ns_malloc( bytes );
	}


nspointer ns_io_calloc( nssize num, nssize size, NsIO *io )
	{
	NS_USE_VARIABLE( io );
	return ns_calloc( num, size );
	}


void ns_io_free( nspointer buffer, NsIO *io )
	{
	NS_USE_VARIABLE( io );
	ns_free( buffer );
	}


void ns_io_print( NsIO *io, const nschar *format, ... )
	{
	ns_va_list args;

	NS_USE_VARIABLE( io );

	ns_va_start( args, format );
	ns_vprint( format, args );
	ns_va_end( args );
	}


nsint ns_io_setjmp( NsIO *io )
	{
	ns_assert( NULL != io );
	return _ns_setjmp( io->jb );
	}


void ns_io_longjmp( nsint value, NsIO *io )
	{
	ns_assert( NULL != io );
	_ns_longjmp( io->jb, value );
	}
