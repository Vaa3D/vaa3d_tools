#ifndef __NS_STD_UTIL_H__
#define __NS_STD_UTIL_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nserror.h>

#ifdef NS_HAVE_STDARG_H
	#include <stdarg.h>
#endif

#ifdef NS_HAVE_SETJMP_H
	#include <setjmp.h>
#endif

/* TEMP */
#ifdef NS_HAVE_STDIO_H
	#include <stdio.h>
#endif


NS_DECLS_BEGIN


NS_IMPEXP void ns_print_record_init( void );
NS_IMPEXP void ns_print_record_finalize( void );
NS_IMPEXP nsboolean ns_print_record_is_enabled( void );
NS_IMPEXP void ns_print_record_enable( nsboolean b );
NS_IMPEXP const nschar* ns_print_record( void );
NS_IMPEXP void ns_print_record_dump( const nschar *startup_directory );


NS_IMPEXP void ns_abort( void );
NS_IMPEXP void ns_exit( nsint status );


typedef struct _NsStat
	{
	nsulong  st_dev;
	nsulong  st_ino;
	nsulong  st_mode;
	nslong   st_nlink;
	nslong   st_uid;
	nslong   st_gid;
	nsulong  st_rdev;
	nslong   st_size;
	nslong   st_atime;
	nslong   st_mtime;
	nslong   st_ctime;
	}
	NsStat;

NS_IMPEXP NsError ns_stat( const nschar *name, NsStat *stat );


typedef struct _NsStat64
	{
	nsuint64  st_dev;
	nsuint64  st_ino;
	nsuint64  st_mode;
	nsint64   st_nlink;
	nsint64   st_uid;
	nsint64   st_gid;
	nsuint64  st_rdev;
	nsint64   st_size;
	nsint64   st_atime;
	nsint64   st_mtime;
	nsint64   st_ctime;
	}
	NsStat64;

NS_IMPEXP NsError ns_stat64( const nschar *name, NsStat64 *stat );


NS_IMPEXP nsboolean ns_stat_is_dir( const NsStat *stat );
NS_IMPEXP nsboolean ns_stat_is_file( const NsStat *stat );

#define ns_stat_dev( s )\
	( (s)->st_dev )

#define ns_stat_ino( s )\
	( (s)->st_ino )

#define ns_stat_mode( s )\
	( (s)->st_mode )

#define ns_stat_nlink( s )\
	( (s)->st_nlink )

#define ns_stat_uid( s )\
	( (s)->st_uid )

#define ns_stat_gid( s )\
	( (s)->st_gid )

#define ns_stat_rdev( s )\
	( (s)->st_rdev )

#define ns_stat_size( s )\
	( (s)->st_size )

#define ns_stat_atime( s )\
	( (s)->st_atime )

#define ns_stat_mtime( s )\
	( (s)->st_mtime )

#define ns_stat_ctime( s )\
	( (s)->st_ctime )


NS_IMPEXP NsError ns_chdir( const nschar *dir_name );
NS_IMPEXP NsError ns_mkdir( const nschar *dir_name );
NS_IMPEXP NsError ns_rmdir( const nschar *dir_name );
NS_IMPEXP NsError ns_isdir( const nschar *name, nsboolean *is_dir );


NS_IMPEXP NsError ns_remove( const nschar *file_name );


/* NOTE: For this implementation, 'buffer' CANNOT be NULL. */
NS_IMPEXP NsError ns_getcwd( nschar *buffer, nsint bytes );


/* NOTE: For this implementation, 'command' CANNOT be NULL. */
NS_IMPEXP NsError ns_system( const nschar *command );


NS_IMPEXP nsint ns_atoi( const nschar *s );
NS_IMPEXP nsdouble ns_atod( const nschar *s );


#ifdef NS_HAVE_STDARG_H
	typedef va_list ns_va_list;
	#define ns_va_arg    va_arg
	#define ns_va_end    va_end
	#define ns_va_start  va_start
#endif


#ifdef NS_HAVE_SETJMP_H
	typedef jmp_buf ns_jmp_buf;
	#define _ns_setjmp   setjmp
	#define _ns_longjmp  longjmp
#endif

#define NS_EOF  -1


/* Internal. DO NOT USE! */
#define _NS_STDIN   0
#define _NS_STDOUT  1
#define _NS_STDERR  2


NS_IMPEXP NS_INTERNAL NsError _ns_vsnprint( nschar*, nssize, const nschar*, ns_va_list );
NS_IMPEXP NS_INTERNAL NsError _ns_vfprint( nspointer, const nschar*, ns_va_list );

NS_IMPEXP NS_INTERNAL nsint ____ns_vsnprint( nschar*, nssize, const nschar*, ns_va_list );


/* TEMP Windows doesnt have vsscanf()! */
#ifdef NS_HAVE_STDIO_H
	#define _ns_sscan sscanf
#endif


NS_IMPEXP NS_INTERNAL nschar* _ns_strtok( nschar*, const nschar* );


NS_IMPEXP NS_INTERNAL void _ns_qsort( nspointer, nssize, nssize, nsint (*)( nsconstpointer, nsconstpointer ) );


NS_IMPEXP NS_INTERNAL NsError _ns_fopen( nspointer*, const nschar*, const nschar* );
NS_IMPEXP NS_INTERNAL NsError _ns_fclose( nspointer );


NS_IMPEXP NS_INTERNAL NsError _ns_fflush( nspointer );


NS_IMPEXP NS_INTERNAL NsError _ns_fileno( nspointer, nsint* );
NS_IMPEXP NS_INTERNAL NsError _ns_filesize( nspointer, nssize* );


enum{ _NS_SEEK_BEGINNING, _NS_SEEK_CURRENT, _NS_SEEK_END };

//NS_IMPEXP NS_INTERNAL NsError _ns_fseek( nspointer, nssize, nsboolean, nsint );
//NS_IMPEXP NS_INTERNAL NsError _ns_ftell( nspointer, nssize* );


//NS_IMPEXP NS_INTERNAL NsError _ns_fread( nspointer, nspointer, nssize, nssize, nssize* );
//NS_IMPEXP NS_INTERNAL NsError _ns_fwrite( nspointer, nsconstpointer, nssize, nssize, nssize* );


//NS_IMPEXP NS_INTERNAL NsError _ns_ungetc( nspointer, nsunichar );


//NS_IMPEXP NS_INTERNAL NsError _ns_getc( nspointer, nsunichar* );
//NS_IMPEXP NS_INTERNAL NsError _ns_gets( nspointer, nschar*, nssize, nsboolean* );


//NS_IMPEXP NS_INTERNAL NsError _ns_putc( nspointer, nsunichar );
//NS_IMPEXP NS_INTERNAL NsError _ns_puts( nspointer, const nschar* );


NS_IMPEXP NS_INTERNAL nspointer _ns_malloc( nssize );
NS_IMPEXP NS_INTERNAL nspointer _ns_realloc( nspointer, nssize );
NS_IMPEXP NS_INTERNAL void _ns_free( nspointer );


NS_IMPEXP NS_INTERNAL nspointer _ns_memset( nspointer, nsint8, nssize );
NS_IMPEXP NS_INTERNAL nspointer _ns_memcpy( nspointer, nsconstpointer, nssize );
NS_IMPEXP NS_INTERNAL nspointer _ns_memmove( nspointer, nsconstpointer, nssize );


NS_IMPEXP NS_INTERNAL nsdouble _ns_pow( nsdouble, nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_ceil( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_floor( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_sin( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_cos( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_tan( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_asin( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_acos( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_atan2( nsdouble, nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_sqrt( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_fabs( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_log( nsdouble );
NS_IMPEXP NS_INTERNAL nsdouble _ns_log10( nsdouble );


NS_IMPEXP NS_INTERNAL nsulong _ns_time( void );
NS_IMPEXP NS_INTERNAL const nschar* _ns_ctime( nsulong );
NS_IMPEXP NS_INTERNAL const nschar* _ns_ctime_ex( void );


#ifdef NS_OS_WINDOWS

NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_console_gets( nschar*, nssize, nsboolean* );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_console_puts( const nschar* );

NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_find_first( nslong*, nspointer );
NS_IMPEXP NS_INTERNAL nsboolean _ns_os_windows_find_next( nslong, nspointer );
NS_IMPEXP NS_INTERNAL void _ns_os_windows_find_close( nslong, nspointer );
NS_IMPEXP NS_INTERNAL const nschar* _ns_os_windows_find_name( nsconstpointer );

NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_mutex_create( nspointer* );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_mutex_destroy( nspointer );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_mutex_lock( nspointer );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_mutex_try_lock( nspointer, nsboolean* );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_mutex_unlock( nspointer );

NS_IMPEXP NS_INTERNAL nsulong _ns_os_windows_thread_id( void );
NS_IMPEXP NS_INTERNAL nsulong _ns_os_windows_process_id( void );

NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_thread_begin( nsulong*, void (*)( nspointer ), nspointer );
NS_IMPEXP NS_INTERNAL void _ns_os_windows_thread_end( void );
NS_IMPEXP NS_INTERNAL nsulong _ns_os_windows_thread_self( void );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_thread_get_priority( nsulong, nspointer );
NS_IMPEXP NS_INTERNAL NsError _ns_os_windows_thread_set_priority( nsulong, nsenum );
NS_IMPEXP NS_INTERNAL void _ns_os_windows_thread_sleep( nssize );

NS_IMPEXP NS_INTERNAL nsintmax _ns_os_windows_time( nsintmax* );
NS_IMPEXP NS_INTERNAL nsintmax _ns_os_windows_timer( void );
NS_IMPEXP NS_INTERNAL nsdouble _ns_os_windows_difftimer( nsintmax, nsintmax );

#endif/* NS_OS_WINDOWS */

NS_DECLS_END

#endif/* __NS_STD_UTIL_H__ */
