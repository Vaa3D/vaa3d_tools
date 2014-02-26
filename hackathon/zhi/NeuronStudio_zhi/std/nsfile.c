#include "nsfile.h"

/* NOTE: Avoid circular inclusion! */
#include <std/nsvalue.h>


typedef struct _NsFileMode
	{
	const nschar  *name;
	nsulong        value;
	}
	NsFileMode;


NS_PRIVATE NsFileMode _ns_file_modes[] =
	{
	{ "r", NS_FILE_MODE_READ },
	{ "w", NS_FILE_MODE_WRITE },
	{ "w", NS_FILE_MODE_WRITE | NS_FILE_MODE_TRUNCATE },
	{ "a", NS_FILE_MODE_WRITE | NS_FILE_MODE_APPEND },
	{ "rb", NS_FILE_MODE_READ | NS_FILE_MODE_BINARY },
	{ "wb", NS_FILE_MODE_WRITE | NS_FILE_MODE_BINARY },
	{ "wb", NS_FILE_MODE_WRITE | NS_FILE_MODE_TRUNCATE | NS_FILE_MODE_BINARY },
	{ "ab", NS_FILE_MODE_WRITE | NS_FILE_MODE_APPEND | NS_FILE_MODE_BINARY },
	{ "r+", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE },
	{ "w+", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE | NS_FILE_MODE_TRUNCATE },
	{ "a+", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE | NS_FILE_MODE_APPEND },
	{ "r+b", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE | NS_FILE_MODE_BINARY },
	{ "w+b", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE | NS_FILE_MODE_TRUNCATE | NS_FILE_MODE_BINARY },
	{ "a+b", NS_FILE_MODE_READ | NS_FILE_MODE_WRITE | NS_FILE_MODE_APPEND | NS_FILE_MODE_BINARY },
	};


const nschar* ns_file_mode_to_string( nsulong mode )
	{
	nssize i;

	for( i = 0; i < NS_ARRAY_LENGTH( _ns_file_modes ); ++i )
		if( mode == _ns_file_modes[i].value )
			return _ns_file_modes[i].name;

	return NULL;
	}


nsulong ns_file_mode_from_string( const nschar *mode )
	{
	nssize i;

	ns_assert( NULL != mode );

	for( i = 0; i < NS_ARRAY_LENGTH( _ns_file_modes ); ++i )
		if( ns_ascii_streq( mode, _ns_file_modes[i].name ) )
			return _ns_file_modes[i].value;

	return 0;
	}


void ns_file_construct( NsFile *file )
	{
	ns_assert( NULL != file );

	file->stream = NULL;
	file->mode   = 0;
	}


void ns_file_destruct( NsFile *file )
	{
	ns_assert( NULL != file );

	/* NOTE: Ignore error on close. */
	if( ns_file_is_open( file ) )
		ns_file_close( file );
	}


NsError ns_file_open( NsFile *file, const nschar *path, nsulong imode )
	{
	const nschar  *smode;
	NsError        error;


	ns_assert( NULL != file );
	ns_assert( NULL != path );
	ns_assert( ! ns_file_is_open( file ) );

	if( NULL == ( smode = ns_file_mode_to_string( imode ) ) )
		return ns_error_inval( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( _ns_fopen( &file->stream, path, smode ), error ) )
		return error;

	file->mode = imode;

	return ns_no_error();
	}


NsError ns_file_close( NsFile *file )
	{
	NsError error;

	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );

	if( NS_FAILURE( _ns_fclose( file->stream ), error ) )
		return error;

	file->stream = NULL;
	file->mode   = 0;

	return ns_no_error();
	}


NsError ns_file_copy( const nschar *dest_path, const nschar *src_path, nsulong binary )
	{
	NsFile     dest_file, src_file;
	nsunichar  c;
	NsError    error;


	ns_assert( NULL != dest_path );
	ns_assert( NULL != src_path );
	ns_assert( 0 == binary || NS_FILE_MODE_BINARY == binary );

	error = ns_no_error();

	ns_file_construct( &dest_file );
	ns_file_construct( &src_file );

	if( NS_FAILURE( ns_file_open( &dest_file, dest_path, binary | NS_FILE_MODE_WRITE ), error ) ||
		 NS_FAILURE( ns_file_open( &src_file, src_path, binary | NS_FILE_MODE_READ ), error ) )
		goto _NS_FILE_COPY_EXIT;

	NS_INFINITE_LOOP
		{
		if( NS_FAILURE( _ns_getc( src_file.stream, &c ), error ) )
			goto _NS_FILE_COPY_EXIT;

		if( NS_EOF == c )
			break;

		if( NS_FAILURE( _ns_putc( dest_file.stream, c ), error ) )
			goto _NS_FILE_COPY_EXIT;
		}

	_NS_FILE_COPY_EXIT:

	ns_file_destruct( &dest_file );
	ns_file_destruct( &src_file );

	return error;
	}


NsError ns_file_flush( NsFile *file )
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );

	return _ns_fflush( file->stream );
	}


NsError ns_file_size( const NsFile *file, nssize *size )
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( NULL != size );

	return _ns_filesize( file->stream, size );
	}


nsboolean ns_file_is_open( const NsFile *file )
	{
	ns_assert( NULL != file );
	return NULL != file->stream;
	}


nsboolean ns_file_can_read( const NsFile *file )
	{
	ns_assert( NULL != file );
	return ( file->mode & NS_FILE_MODE_READ ) ? NS_TRUE : NS_FALSE;
	}


nsboolean ns_file_can_write( const NsFile *file )
	{
	ns_assert( NULL != file );
	return ( file->mode & NS_FILE_MODE_WRITE ) ? NS_TRUE : NS_FALSE;
	}


nsboolean ns_file_is_text( const NsFile *file )
	{
	ns_assert( NULL != file );
	return ! ns_file_is_binary( file );
	}


nsboolean ns_file_is_binary( const NsFile *file )
	{
	ns_assert( NULL != file );
	return ( file->mode & NS_FILE_MODE_BINARY ) ? NS_TRUE : NS_FALSE;
	}


NsError ns_file_print( NsFile *file, const nschar *format, ... )
	{
	ns_va_list  args;
	NsError     error;


	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_write( file ) );
	ns_assert( NULL != format );

	ns_va_start( args, format );
	error = _ns_vfprint( file->stream, format, args );
	ns_va_end( args );

	return error;
	}


NsError ns_file_vprint( NsFile *file, const nschar *format, ns_va_list args )
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_write( file ) );
	ns_assert( NULL != format );

	return _ns_vfprint( file->stream, format, args );
	}


NsError ns_file_read
	(
	NsFile     *file,
	nspointer   buffer,
	nssize      size,
	nssize      count,
	nssize     *read
	)
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_read( file ) );
	ns_assert( NULL != buffer );

	return _ns_fread( file->stream, buffer, size, count, read );
	}


NsError ns_file_write
	(
	NsFile          *file,
	nsconstpointer   buffer,
	nssize           size,
	nssize           count,
	nssize          *write
	)
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_write( file ) );
	ns_assert( NULL != buffer );

	return _ns_fwrite( file->stream, buffer, size, count, write );
	}


NsError ns_file_seek
	(
	NsFile          *file,
	nssize           offset,
	nsboolean        forwards,
	NsFileSeekType   which
	)
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );

	return _ns_fseek( file->stream, offset, forwards, which );
	}


NsError ns_file_tell( NsFile *file, nssize *position )
	{
	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( NULL != position );

	return _ns_ftell( file->stream, position );
	}


NsError ns_file_readline
	(
	NsFile     *file,
	nschar     *buffer,
	nssize      max_chars,
	nsboolean  *eof
	)
	{
	nsunichar  ch;
	nssize     count;
	NsError    error;


	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_read( file ) );
	ns_assert( NULL != buffer );
	ns_assert( NULL != eof );

	*eof = NS_FALSE;

	ch    = 0;
	count = 0;

	while( count < max_chars )
		{
		if( NS_FAILURE( _ns_getc( file->stream, &ch ), error ) )
			return error;

		if( NS_EOF == ch )
			{
			if( count < max_chars )
				*buffer = NS_ASCII_NULL;

			*eof = NS_TRUE;
			return ns_no_error();
			}

		if( ( nsunichar )NS_ASCII_LINE_FEED == ch || ( nsunichar )NS_ASCII_CARRIAGE_RETURN == ch )
			break;

		*buffer++ = ( nschar )ch;
		++count;
		}

	if( count < max_chars )
		*buffer = NS_ASCII_NULL;

	if( NS_ASCII_CARRIAGE_RETURN == ch )
		{
		if( NS_FAILURE( _ns_getc( file->stream, &ch ), error ) )
			return error;

		if( NS_EOF == ch )
			{
			*eof = NS_TRUE;
			return ns_no_error();
			}

		/* If not Windows convention, then set file pointer back 1 character. */
		if( NS_ASCII_LINE_FEED != ch )
			if( NS_FAILURE( _ns_ungetc( file->stream, ch ), error ) )
				return error;
		}

	return ns_no_error();
	}


NsError ns_file_read_string( NsFile *file, NsString *string )
	{
	nssize   count;
	NsError  error;
	nschar   buffer[ 1024 ];


	ns_assert( NULL != file );
	ns_assert( ns_file_is_open( file ) );
	ns_assert( ns_file_can_read( file ) );
	ns_assert( NULL != string );

	ns_string_resize( string, 0 );

	/* Read the file in chunks until either an error occurs or the
		end of the file is reached. */
	NS_INFINITE_LOOP
		{
		if( NS_FAILURE(
				ns_file_read(
					file,
					buffer,
					1,
					sizeof( buffer ) - 1,
					&count
					),
				error ) )
			return error;

		if( 0 == count )
			break;

		ns_assert( count < sizeof( buffer ) );
		buffer[ count ] = NS_ASCII_NULL;

		if( NS_FAILURE( ns_string_append( string, buffer ), error ) )
			return error;
		}

	return ns_no_error();
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_file, NsFile* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_file, NsFile* );
NS_VALUE_ARG_IMPLEMENT( ns_value_arg_file, NsFile* );

NsError ns_value_register_file( void )
	{
	return ns_value_register(
				NS_VALUE_FILE,
				sizeof( NsFile* ),
				NULL,
				NULL,
				ns_value_arg_file
				);
	}
