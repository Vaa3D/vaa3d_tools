#ifndef __NS_STD_FILE_H__
#define __NS_STD_FILE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsascii.h>
#include <std/nsstring.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

#define NS_FILE_MODE_READ      0x0001
#define NS_FILE_MODE_WRITE     0x0002
#define NS_FILE_MODE_APPEND    0x0004
#define NS_FILE_MODE_TRUNCATE  0x0008
#define NS_FILE_MODE_BINARY    0x0010


/* Use to convert a combination of the flags values
	to a string compatible with ANSI C fopen(). */
NS_IMPEXP const nschar* ns_file_mode_to_string( nsulong mode );
NS_IMPEXP nsulong ns_file_mode_from_string( const nschar *mode );


typedef struct _NsFile
	{
	nspointer  stream;
	nsulong    mode;
	}
	NsFile;


typedef enum
	{
	NS_FILE_SEEK_BEGINNING = _NS_SEEK_BEGINNING,
	NS_FILE_SEEK_CURRENT   = _NS_SEEK_CURRENT,
	NS_FILE_SEEK_END       = _NS_SEEK_END
	}
	NsFileSeekType;


NS_IMPEXP void ns_file_construct( NsFile *file );
NS_IMPEXP void ns_file_destruct( NsFile *file );

NS_IMPEXP NsError ns_file_open( NsFile *file, const nschar *path, nsulong mode );
NS_IMPEXP NsError ns_file_close( NsFile *file );


/* Pass NS_FILE_MODE_BINARY for binary if binary files. */
NS_IMPEXP NsError ns_file_copy( const nschar *dest_path, const nschar *src_path, nsulong binary );


NS_IMPEXP NsError ns_file_flush( NsFile *file );

NS_IMPEXP NsError ns_file_size( const NsFile *file, nssize *size );


NS_IMPEXP nsboolean ns_file_is_open( const NsFile *file );
NS_IMPEXP nsboolean ns_file_can_read( const NsFile *file );
NS_IMPEXP nsboolean ns_file_can_write( const NsFile *file );
NS_IMPEXP nsboolean ns_file_is_text( const NsFile *file );
NS_IMPEXP nsboolean ns_file_is_binary( const NsFile *file );


NS_IMPEXP NsError ns_file_print( NsFile *file, const nschar *format, ... );
NS_IMPEXP NsError ns_file_vprint( NsFile *file, const nschar *format, ns_va_list args );

#define ns_file_print_newline( file )\
	ns_file_print( (file), NS_STRING_NEWLINE )


NS_IMPEXP NsError ns_file_read
	(
	NsFile     *file,
	nspointer   buffer,
	nssize      size,
	nssize      count,
	nssize     *read
	);

NS_IMPEXP NsError ns_file_read_string( NsFile *file, NsString *string );

NS_IMPEXP NsError ns_file_write
	(
	NsFile          *file,
	nsconstpointer   buffer,
	nssize           size,
	nssize           count,
	nssize          *write
	);

NS_IMPEXP NsError ns_file_seek
	(
	NsFile          *file,
	nssize           offset,
	nsboolean        forwards,
	NsFileSeekType   which
	);

NS_IMPEXP NsError ns_file_tell( NsFile *file, nssize *position );

NS_IMPEXP NsError ns_file_readline
	(
	NsFile     *file,
	nschar     *buffer,
	nssize      max_chars,
	nsboolean  *eof
	);

NS_DECLS_END

#endif/* __NS_STD_FILE_H__ */
