#include "nslog.h"


typedef struct _NsLog
   {
   NsFile     file;
   nsboolean  enabled;
	nsboolean  echo;
	nsboolean  flush_after_entry;
	nssize     reset_size;
   }
   NsLog;


NS_PRIVATE NsLog ____ns_log;


void ns_log_init( nsboolean enabled, nsboolean flush_after_entry, nssize reset_size )
   {
   ____ns_log.enabled           = enabled;
	____ns_log.flush_after_entry = flush_after_entry;
	____ns_log.reset_size        = reset_size;

   ns_file_construct( &____ns_log.file );
   }


NsError ns_log_open( const nschar *path )
   {
	nssize   curr_size;
   NsError  error;


   if( ____ns_log.enabled )
		{
		/* Open the file for appending first. If the current size of the file is
			greater than the 'reset_size' specified in the initialize function, then
			close the file and re-open for writing. */

      if( NS_FAILURE( ns_file_open( &____ns_log.file, path, NS_FILE_MODE_WRITE | NS_FILE_MODE_APPEND ), error ) )
         return error;

		/* NOTE: Just default to leaving the file open for appending if the ns_file_size()
			function were to fail for some reason. */
		if( NS_SUCCESS( ns_file_size( &____ns_log.file, &curr_size ), error ) )
			if( curr_size > ____ns_log.reset_size )
				{
				ns_file_close( &____ns_log.file );

				if( NS_FAILURE( ns_file_open( &____ns_log.file, path, NS_FILE_MODE_WRITE ), error ) )
					return error;
				}
		}

   ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( path=" NS_FMT_STRING_DOUBLE_QUOTED " )",
		path
		);

   return ns_no_error();
   }


nsboolean ns_log_is_open( void )
   {  return ns_file_is_open( &____ns_log.file );  }


void ns_log_close( void )
   {
   ns_log_entry( NS_LOG_ENTRY_FUNCTION, NS_FUNCTION "()" );

   if( ns_log_is_open() )
      ns_file_close( &____ns_log.file );
   }


NS_PRIVATE void _ns_log_time( const nschar *type )
   {
   if( ns_log_is_open() )
      {
      nssize  length;
      nschar  buffer[ 64 ];


      ns_file_print( &____ns_log.file, NS_FMT_STRING ": ", type );

      ns_ascii_strcpy( buffer, _ns_ctime( _ns_time() ) );

      length = ns_ascii_strlen( buffer );
      ns_assert( length < NS_ARRAY_LENGTH( buffer ) - 1 );

      if( NS_UNICHAR_NEWLINE == ( nsunichar )buffer[ length - 1 ] )
         buffer[ length - 1 ] = NS_ASCII_NULL;

      ns_file_print( &____ns_log.file, NS_FMT_STRING ": ", buffer );
      }
   }


NS_PRIVATE const nschar* _ns_log_entry_names[ _NS_LOG_ENTRY_NUM_TYPES ] = 
   {
   "Unknown Entry",
   "Function Call",
   "Begin Event",
   "Event Step",
   "End Event",
   "Note",
	"!ERROR!"
   };


void ns_log_enable( nsboolean enabled )
	{  ____ns_log.enabled = enabled;  }


nsboolean ns_log_is_enabled( void )
	{  return ____ns_log.enabled;  }


void ns_log_echo( nsboolean echo )
	{  ____ns_log.echo = echo;  }


nsboolean ns_log_is_echo( void )
	{  return ____ns_log.echo;  }


void ns_log_entry( NsLogEntryType type, const nschar *format, ... )
   {
   if( ns_log_is_open() && ____ns_log.enabled )
      {
		ns_va_list args;

      ns_va_start( args, format );

		ns_assert( type < _NS_LOG_ENTRY_NUM_TYPES );
		_ns_log_time( _ns_log_entry_names[ type ] );

		ns_file_vprint( &____ns_log.file, format, args );
      ns_file_print( &____ns_log.file, NS_STRING_NEWLINE );

      ns_va_end( args );

		if( ____ns_log.echo )
			{
			ns_va_start( args, format );
			ns_vprint( format, args );
			ns_va_end( args );

			ns_print_newline();
			}

		if( ____ns_log.flush_after_entry )
			ns_file_flush( &____ns_log.file );
      }
   }
