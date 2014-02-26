#ifndef __NS_STD_IO_H__
#define __NS_STD_IO_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsfile.h>
#include <std/nsprogress.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsIO
	{
	NsProgress  *progress;
	NsFile      *file;
	ns_jmp_buf   jb;
	}
	NsIO;


NS_IMPEXP nspointer ns_io_fopen( const nschar *name, const nschar *mode, NsIO *io );
NS_IMPEXP nsint ns_io_fclose( nspointer na, NsIO *io );

NS_IMPEXP nssize ns_io_fread( nspointer buffer, nssize size, nssize count, nspointer na, NsIO *io );
NS_IMPEXP nssize ns_io_fwrite( nsconstpointer buffer, nssize size, nssize count, nspointer na, NsIO *io );

NS_IMPEXP nsint ns_io_remove( const nschar *name, NsIO *io );

NS_IMPEXP nsint ns_io_fseek( nspointer na, nssize offset, nsboolean forwards, nsint which, NsIO *io );
NS_IMPEXP nsint ns_io_ftell( nspointer na, nssize *position, NsIO *io );

NS_IMPEXP nspointer ns_io_malloc( nssize bytes, NsIO *io );
NS_IMPEXP nspointer ns_io_calloc( nssize num, nssize size, NsIO *io );
NS_IMPEXP void ns_io_free( nspointer buffer, NsIO *io );

NS_IMPEXP void ns_io_print( NsIO *io, const nschar *format, ... );

NS_IMPEXP nsint ns_io_setjmp( NsIO *io );
NS_IMPEXP void ns_io_longjmp( nsint value, NsIO *io );

NS_DECLS_END

#endif/* __NS_STD_IO_H__ */
