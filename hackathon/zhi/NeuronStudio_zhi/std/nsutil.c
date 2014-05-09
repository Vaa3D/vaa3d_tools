#include "nsutil.h"

/* NOTE: Avoid circular inclusion! */
#include <std/nsdebug.h>
#include <std/nsmemory.h>
#include <std/nsthread.h>
#include <std/nsprint.h>
#include <std/nsascii.h>


#ifdef NS_HAVE_STDLIB_H
	#include <stdlib.h>
#endif

#ifdef NS_HAVE_STDIO_H
	#include <stdio.h>
#endif

#ifdef NS_HAVE_STRING_H
	#include <string.h>
#endif

#ifdef NS_HAVE_MATH_H
	#include <math.h>
#endif

#ifdef NS_HAVE_ERRNO_H
	#include <errno.h>
#endif

#ifdef NS_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif

#ifdef NS_HAVE_TIME_H
	#include <time.h>
#endif

#include "nsutil-errno.inl"


NS_COMPILE_TIME_BOOLEAN( NS_EOF, ==, EOF );

NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsint, int );
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsuint, unsigned int );
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nssize, size_t );
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsdouble, double );
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsunichar, int );
//NS_COMPILE_TIME_SIZE_EQUAL( nsint, long );


#include <std/nsstring.h>
#include <std/nsfile.h>

NS_PRIVATE nsboolean ____ns_print_record_enabled = NS_FALSE;
NS_PRIVATE NsString ____ns_print_record;


void ns_print_record_init( void )
	{  ns_string_construct( &____ns_print_record );  }


void ns_print_record_finalize( void )
	{  ns_string_destruct( &____ns_print_record );  }


void ns_print_record_enable( nsboolean b )
	{  ____ns_print_record_enabled = b;  }


nsboolean ns_print_record_is_enabled( void )
	{  return ____ns_print_record_enabled;  }


const nschar* ns_print_record( void )
	{  return ns_string_get( &____ns_print_record );  }


void ns_print_record_dump( const nschar *startup_directory )
	{
	NsFile    file;
	nschar    name[ NS_PATH_SIZE ];
	NsError   error;


   ns_ascii_strcpy( name, startup_directory );
   ns_ascii_strcat( name, "\\.print" );

	ns_file_construct( &file );

	if( NS_SUCCESS( ns_file_open( &file, name, NS_FILE_MODE_WRITE ), error ) )
		ns_file_print( &file, NS_FMT_STRING, ns_print_record() );

	ns_file_destruct( &file );
	}


void ns_abort( void )
	{
#ifdef NS_HAVE_STDLIB_H
	abort();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " abort() not implemented." );
#endif
	}


void ns_exit( nsint status )
	{
#ifdef NS_HAVE_STDLIB_H
	exit( status );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " exit() not implemented." );
#endif	
	}


NsError ns_stat( const nschar *name, NsStat *s_out )
	{
#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_SYS_STAT_H )
	struct stat s_in;

	ns_assert( NULL != name );
	ns_assert( NULL != s_out );

	if( -1 == ( nsint )stat( ( const char* )name, &s_in ) )
		return _ns_errno_to_error();

	s_out->st_dev   = ( nsulong )s_in.st_dev;
	s_out->st_ino   = ( nsulong )s_in.st_ino;
	s_out->st_mode  = ( nsulong )s_in.st_mode;
	s_out->st_nlink = ( nslong )s_in.st_nlink;
	s_out->st_uid   = ( nslong )s_in.st_uid;
	s_out->st_gid   = ( nslong )s_in.st_gid;
	s_out->st_rdev  = ( nsulong )s_in.st_rdev;
	s_out->st_size  = ( nslong )s_in.st_size;


	return ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " stat() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError ns_stat64( const nschar *name, NsStat64 *s_out )
	{
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " _stat64() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
    }


nsboolean e_stat_is_dir( const NsStat *stat )
	{
	ns_assert( NULL != stat );

#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_SYS_STAT_H )
	return ( stat->st_mode & ( ( nsulong )S_IFDIR ) ) ? NS_TRUE : NS_FALSE;
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " stat() not implemented." );
	return NS_FALSE;
#endif
	}


nsboolean ns_stat_is_file( const NsStat *stat )
	{
	ns_assert( NULL != stat );

#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_SYS_STAT_H )
	return ( stat->st_mode & ( ( nsulong )S_IFREG ) ) ? NS_TRUE : NS_FALSE;
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " stat() not implemented." );
	return NS_FALSE;
#endif
	}


NsError ns_chdir( const nschar *dir_name )
	{
	ns_assert( NULL != dir_name );
#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_UNISTD_H )
	return -1 == chdir( dir_name ) ? _ns_errno_to_error() : ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " chdir() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError ns_mkdir( const nschar *dir_name )
	{
	ns_assert( NULL != dir_name );
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " mkdir() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
    }


NsError ns_rmdir( const nschar *dir_name )
	{
	ns_assert( NULL != dir_name );
#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_UNISTD_H )
    return -1 == rmdir( dir_name ) ? _ns_errno_to_error() : ns_no_error();
#else
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " rmdir() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
    }


NsError ns_isdir( const nschar *name, nsboolean *is_dir )
	{
#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_SYS_STAT_H )
	struct stat s;

	ns_assert( NULL != name );
	ns_assert( NULL != is_dir );

	if( -1 == stat( name, &s ) )
		return _ns_errno_to_error();

	*is_dir = ( s.st_mode & S_IFDIR ) ? NS_TRUE : NS_FALSE;
	return ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " stat() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError ns_remove( const nschar *file_name )
	{
	ns_assert( NULL != file_name );
#ifdef NS_HAVE_STDIO_H
	return -1 == remove( file_name ) ? _ns_errno_to_error() : ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " remove() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError ns_getcwd( nschar *buffer, nsint bytes )
	{
	ns_assert( NULL != buffer );
#if defined( NS_OS_WINDOWS ) || defined( NS_HAVE_UNISTD_H )
	return NULL == getcwd( buffer, bytes ) ? _ns_errno_to_error() : ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " getcwd() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError ns_system( const nschar *command )
	{
	ns_assert( NULL != command );
#ifdef NS_HAVE_STDLIB_H
	return -1 == system( command ) ? _ns_errno_to_error() : ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " system() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


nsint ns_atoi( const nschar *s )
	{
	ns_assert( NULL != s );
#ifdef NS_HAVE_STDLIB_H
	return atoi( s );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " atoi() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


nsdouble ns_atod( const nschar *s )
	{
	ns_assert( NULL != s );
#ifdef NS_HAVE_STDLIB_H
	return atof( s );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " atof() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


nsint ____ns_vsnprint( nschar *buffer, nssize count, const nschar *format, ns_va_list args )
	{
#ifdef NS_OS_WINDOWS
	#ifdef NS_HAVE_STDIO_H
		return _vsnprintf( buffer, count, format, args );
	#endif
#else
	#ifdef NS_HAVE_STDIO_H
		return vsnprintf( buffer, count, format, args );
	#endif
#endif
	}


NsError _ns_vsnprint( nschar *buffer, nssize count, const nschar *format, ns_va_list args )
	{
	#ifdef NS_HAVE_STDIO_H
	return -1 == ____ns_vsnprint( buffer, count, format, args )
			 ? _ns_errno_to_error() : ns_no_error();
	#else
		ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " vsnprintf() not implemented." );
		return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	#endif
	}


#define _NS_STREAM_READ   0
#define _NS_STREAM_WRITE  1
#define _NS_STREAM_BOTH   2


NS_PRIVATE FILE* _ns_stream( FILE *stream, nsint mode )
	{

    return stream;
    }


NS_PRIVATE nschar _ns_vprint_buffer[ 4096 ];

NsError _ns_vfprint( nspointer stream, const nschar *format, ns_va_list args )
	{
#ifdef NS_OS_WINDOWS
	if( _NS_STDOUT == NS_POINTER_TO_INT( stream ) && ns_print_get_is_gui() )
		{
		NsError error;

		if( NS_FAILURE( _ns_vsnprint(
								_ns_vprint_buffer,
								sizeof( _ns_vprint_buffer ),
								format,
								args
								),
								error ) )
			return error;

		if( ns_print_record_is_enabled() )
			/*error*/ns_string_append( &____ns_print_record, _ns_vprint_buffer );

		return ns_print_get_enabled() ?
					_ns_os_windows_console_puts( _ns_vprint_buffer ) : ns_no_error();
		}
#endif/* NS_OS_WINDOWS */

#ifdef NS_HAVE_STDIO_H
	return -1 == vfprintf( _ns_stream( stream, _NS_STREAM_WRITE ), format, args )
			 ? _ns_errno_to_error() : ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " vfprintf() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


nschar* _ns_strtok( nschar *s, const nschar *delims )
	{
#ifdef NS_HAVE_STRING_H
	return strtok( s, delims );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " strtok() not implemented." );
	return NULL;
#endif
	}


void _ns_qsort
	(
	nspointer  buffer,
	nssize     count,
	nssize     width,
	nsint      ( *compare_func )( nsconstpointer, nsconstpointer )
	)
	{
#ifdef NS_HAVE_STDLIB_H
	qsort( buffer, count, width, compare_func );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " qsort() not implemented." );
	return NULL;
#endif
	}


NsError _ns_fopen( nspointer *file, const nschar *name, const nschar *mode )
	{
#ifdef NS_HAVE_STDIO_H
	return NULL != ( *( ( FILE** )file ) = fopen( name, mode ) )
			 ? ns_no_error() : _ns_errno_to_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fopen() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_fclose( nspointer file )
	{
#ifdef NS_HAVE_STDIO_H
	return 0 == fclose( file )
			 ? ns_no_error() : _ns_errno_to_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fclose() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_fflush( nspointer file )
	{
#ifdef NS_HAVE_STDIO_H
	return 0 == fflush( file )
			 ? ns_no_error() : _ns_errno_to_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fflush() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_fileno( nspointer stream, nsint *handle )
	{
#ifdef NS_HAVE_STDIO_H
	ns_assert( NULL != handle );

	return -1 != ( *handle = fileno( _ns_stream( stream, _NS_STREAM_BOTH ) ) )
				? ns_no_error() : _ns_errno_to_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fileno() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_filesize( nspointer stream, nssize *size )
	{
#ifdef NS_OS_WINDOWS
	nsint    fd;
	nslong   ret;
	NsError  error;


	ns_assert( NULL != size );

	if( NS_FAILURE( _ns_fileno( stream, &fd ), error ) )
		return error;

	if( -1 == ( ret = _filelength( fd ) ) )
		return _ns_errno_to_error();

	*size = ( nssize )ret;
	return ns_no_error();
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " _filelength() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


/* NOTE: The range of a signed integer on the negative side
	is always one more than the positive range.

	Example: 'char' ranges from -128 to 127.

	Therefore we know we can safely negate the most positive
	representable value for any signed integer type. */
#define __POS  NS_INT_MAX
#define __NEG  ( -__POS )

NsError _ns_fseek( FILE *stream, nssize offset, nsboolean forwards, nsint which )
    {
    NS_PRIVATE nsint _ns_std_seek[ 3 ] = { SEEK_SET, SEEK_CUR, SEEK_END };

#ifdef NS_HAVE_STDIO_H
    nssize  temp;
    nsint   amount, i, iterations, remainder;


    ns_assert( which < 3 );

    stream = _ns_stream( stream, _NS_STREAM_BOTH );

    temp = offset / ( nssize )__POS;
    ns_assert( temp <= __POS );
    iterations = ( nsint )temp;

    temp = offset % ( nssize )__POS;
    ns_assert( temp <= __POS );
    remainder = ( nsint )temp;

    /* NOTE: 'which' is allowed to have its initial value only
        for the first fseek() so that the file pointer can be
        properly set. After that all seeking should be done from
        the current position. */

    amount = forwards ? __POS : __NEG;

    for( i = 0; i < iterations; ++i )
        if( 0 != fseek( stream, amount, _ns_std_seek[ which ] ) )
            return _ns_errno_to_error();
        else
            which = _NS_SEEK_CURRENT;

    amount = forwards ? remainder : -remainder;

    if( 0 != fseek( stream, remainder, _ns_std_seek[ which ] ) )
        return _ns_errno_to_error();
    else
        which = _NS_SEEK_CURRENT;

    return ns_no_error();
#else
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fseek() not implemented." );
    return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
    }


NsError _ns_ftell( FILE *stream, nssize *position )
    {
#ifdef NS_OS_WINDOWS
    nsint64 p;

    stream = _ns_stream( stream, _NS_STREAM_BOTH );
    p      = _telli64( _fileno( stream ) );

    if( -1 == p )
        return _ns_errno_to_error();

    *position = ( nssize )p;
    return ns_no_error();
#else
    #ifdef NS_HAVE_STDIO_H
    nsint p;

    stream = _ns_stream( stream, _NS_STREAM_BOTH );
    p      = ftell( stream );

    if( -1 == p )
        return _ns_errno_to_error();

    *position = p;
    return ns_no_error();
    #else
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " ftell() not implemented." );
    return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
    #endif
#endif
    }


NsError _ns_fread( FILE *stream, nspointer buffer, nssize size, nssize count, nssize *ret_in )
    {
#ifdef NS_HAVE_STDIO_H
    nssize   in;
    NsError  error;


    ns_assert( NULL != buffer );

    stream = _ns_stream( stream, _NS_STREAM_READ );
    error  = ns_no_error();
    in     = fread( buffer, size, count, stream );

    if( in < count && ferror( stream ) )
        error = _ns_errno_to_error();

    if( NULL != ret_in )
        *ret_in = in;

    return error;
#else
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fread(), ferror() not implemented." );
    return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
    }


NsError _ns_fwrite( FILE *stream, nsconstpointer buffer, nssize size, nssize count, nssize *ret_out )
    {
#ifdef NS_HAVE_STDIO_H
    nssize   out;
    NsError  error;


    ns_assert( NULL != buffer );

    stream = _ns_stream( stream, _NS_STREAM_WRITE );
    error  = ns_no_error();
    out    = fwrite( buffer, size, count, stream );

    if( out < count && ferror( stream ) )
        error = _ns_errno_to_error();

    if( NULL != ret_out )
        *ret_out = out;

    return error;
#else
    ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fwrite(), ferror() not implemented." );
    return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
    }


NS_PRIVATE NsError _ns_do_ungetc( FILE *stream, nsunichar c )
	{
#ifdef NS_HAVE_STDIO_H
	NsError error = ns_no_error();

	if( EOF != c )
		{
		c = ungetc( c, stream );

		if( EOF == c && ferror( stream ) )
			error = _ns_errno_to_error();
		}

	return error;
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " ungetc(), ferror() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_ungetc( FILE *stream, nsunichar c )
	{  return _ns_do_ungetc( _ns_stream( stream, _NS_STREAM_READ ), c );  }


NS_PRIVATE NsError _ns_do_getc( FILE *stream, nsunichar *c )
	{
#ifdef NS_HAVE_STDIO_H
	NsError error = ns_no_error();

	*c = getc( stream );

	if( EOF == *c && ferror( stream ) )
		error = _ns_errno_to_error();

	return error;
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " getc(), ferror() not implemented." );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_getc( FILE *stream, nsunichar *c )
	{  return _ns_do_getc( _ns_stream( stream, _NS_STREAM_READ ), c );  }


NS_PRIVATE NsError _ns_do_gets
	(
	FILE       *stream,
	nschar     *s,
	nssize      length,
	nsboolean  *eof,
	NsError     ( *getc_func )( nspointer, nsunichar* ),
	NsError     ( *ungetc_func )( nspointer, nsunichar )
	)
	{
	nsunichar  c;
	nssize     index;
	NsError    error;


	NS_USE_VARIABLE( ungetc_func );

	if( 0 == length )
		return ns_no_error();

	/* NOTE: Ensure room for the NULL terminator. */
	--length;

	*eof = NS_FALSE;

	c     = 0;
	index = 0;

	while( index < length )
		{
		if( NS_FAILURE( ( getc_func )( stream, &c ), error ) )
			return error;

		if( ( nsunichar )EOF == c )
			{
			*s   = NS_ASCII_NULL;
			*eof = NS_TRUE;
			return ns_no_error();
			}

		if( NS_ASCII_LINE_FEED == c || NS_ASCII_CARRIAGE_RETURN == c )
			break;

		*s = ( nschar )c;
		++s;

		++index;
		}

	*s = NS_ASCII_NULL;
/*
	if( NS_ASCII_CARRIAGE_RETURN == c )
		{
		if( NS_FAILURE( ( getc_func )( stream, &c ), error ) )
			return error;

		if( ( nsunichar )EOF == c )
			{
			*eof = NS_TRUE;
			return ns_no_error();
			}

		/* If not Windows convention, then set file
			pointer back 1 character. *//*
		if( NS_ASCII_LINE_FEED != c )
			if( NS_FAILURE( ( ungetc_func )( stream, c ), error ) )
				return error;
		}*/

	return ns_no_error();
	}


NsError _ns_gets( FILE *handle, nschar *s, nssize length, nsboolean *eof )
	{
	FILE     *stream;
	NsError   error;


	ns_assert( NULL != s );
	ns_assert( NULL != eof );

	stream = _ns_stream( handle, _NS_STREAM_READ );

#ifdef NS_OS_WINDOWS
	if( NS_INT_TO_POINTER( _NS_STDIN ) == handle && ns_print_get_is_gui() )
		error = _ns_os_windows_console_gets( s, length, eof );
	else
		error = _ns_do_gets( stream, s, length, eof, _ns_do_getc, _ns_do_ungetc );
#else
	error = _ns_do_gets( stream, s, length, eof, _ns_do_getc, _ns_do_ungetc );
#endif

	return error;
	}


NS_PRIVATE NsError _ns_do_putc( FILE *stream, nsunichar c )
	{
#ifdef NS_HAVE_STDIO_H
	NsError error = ns_no_error();

	c = putc( c, stream );

	if( EOF == c && ferror( stream ) )
		error = _ns_errno_to_error();

	return error;
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " putc(), ferror() not implemented." ) );
	return ns_error_noimp( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
#endif
	}


NsError _ns_putc( FILE *stream, nsunichar c )
	{  return _ns_do_putc( _ns_stream( stream, _NS_STREAM_WRITE ), c );  }


NS_PRIVATE NsError _ns_do_puts( FILE *stream, const nschar *s )
	{
	nsunichar  c;
	NsError    error;


	c = *s;

	while( NS_ASCII_NULL != c )
		{
		if( NS_FAILURE( _ns_do_putc( stream, c ), error ) )
			return error;

		++s;
		c = *s;
		}

	return ns_no_error();
	}


NsError _ns_puts( FILE *handle, const nschar *s )
	{
	FILE     *stream;
	NsError   error;


	ns_assert( NULL != s );

	stream = _ns_stream( handle, _NS_STREAM_WRITE );

#ifdef NS_OS_WINDOWS
	if( NS_INT_TO_POINTER( _NS_STDOUT ) == handle && ns_print_get_is_gui() )
		error = _ns_os_windows_console_puts( s );
	else
		error = _ns_do_puts( stream, s );
#else
	error = _ns_do_puts( stream, s );
#endif

	return error;
	}


nspointer _ns_malloc( nssize bytes )
	{
#ifdef NS_HAVE_STDLIB_H
	return malloc( bytes );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " malloc() not implemented." );
	return NULL;
#endif
	}


nspointer _ns_realloc( nspointer mem_block, nssize bytes )
	{
#ifdef NS_HAVE_STDLIB_H
	return realloc( mem_block, bytes );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " realloc() not implemented." );
	return NULL;
#endif
	}


void _ns_free( nspointer mem_block )
	{
#ifdef NS_HAVE_STDLIB_H
	free( mem_block );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " free() not implemented." );
#endif
	}


nspointer _ns_memset( nspointer mem_block, nsint8 c, nssize count )
	{
#ifdef NS_HAVE_STRING_H
	return memset( mem_block, c, count );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " memset() not implemented." );
	return NULL;
#endif
	}


nspointer _ns_memcpy( nspointer dest, nsconstpointer src, nssize count )
	{
#ifdef NS_HAVE_STRING_H
	return memcpy( dest, src, count );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " memcpy() not implemented." );
	return NULL;
#endif
	}


nspointer _ns_memmove( nspointer dest, nsconstpointer src, nssize count )
	{
#ifdef NS_HAVE_STRING_H
	return memmove( dest, src, count );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " memmove() not implemented." );
	return NULL;
#endif
	}


nsdouble _ns_pow( nsdouble f, nsdouble p )
	{
#ifdef NS_HAVE_MATH_H
	return pow( f, p );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " pow() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_ceil( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return ceil( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " ceil() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_floor( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return floor( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " floor() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_sin( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return sin( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " sin() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_cos( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return cos( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " cos() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_tan( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return tan( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " tan() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_asin( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return asin( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " asin() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_acos( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return acos( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " acos() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_atan2( nsdouble x, nsdouble y )
	{
#ifdef NS_HAVE_MATH_H
	return atan2( x, y );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " atan2() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_sqrt( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return sqrt( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " sqrt() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_fabs( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return fabs( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " fabs() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_log( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return log( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " log() not implemented." );
	return 0.0;
#endif
	}


nsdouble _ns_log10( nsdouble f )
	{
#ifdef NS_HAVE_MATH_H
	return log10( f );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " log10() not implemented." );
	return 0.0;
#endif
	}


nsulong _ns_time( void )
	{
#ifdef NS_HAVE_TIME_H
	return ( nsulong )time( NULL );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " time() not implemented." );
	return 0;
#endif
	}


const nschar* _ns_ctime( nsulong ti )
	{
#ifdef NS_HAVE_TIME_H
	time_t to = ( time_t )ti;
	return ctime( &to );
#else
	ns_warning( NS_WARNING_LEVEL_CRITICAL NS_MODULE " ctime() not implemented." );
	return 0;
#endif
	}


const nschar* _ns_ctime_ex( void )
	{  return _ns_ctime( _ns_time() );  }

