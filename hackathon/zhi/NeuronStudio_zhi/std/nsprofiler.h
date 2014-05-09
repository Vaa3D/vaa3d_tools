#ifndef __NS_STD_PROFILER_H__
#define __NS_STD_PROFILER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nsascii.h>
#include <std/nsmutex.h>
#include <std/nshier.h>
#include <std/nstime.h>
#include <std/nsprint.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsProfiler
	{
	NsHier       profiles;
	nshieriter   curr_profile;
	nsdouble     elapsed_seconds;
	nshieriter   shell_profile;
	nsboolean    exit_shell;
	nstimer      timer;
	nssize       begin_count;
	NsMutex      mutex;
	}
	NsProfiler;


NS_IMPEXP void ns_profiler_construct( NsProfiler *profiler );
NS_IMPEXP void ns_profiler_destruct( NsProfiler *profiler );


NS_IMPEXP void ns_profiler_clear( NsProfiler *profiler );


/* NOTE: If either of E_DEBUG or E_ENABLE_PROFILING is not defined,
	the following functions will do nothing. */

/* Profiles a block of code with the given 'name' between
	the calls to begin/end. */
NS_IMPEXP NS_SYNCHRONIZED void ns_profiler_begin( NsProfiler *profiler, const nschar* name );
NS_IMPEXP NS_SYNCHRONIZED void ns_profiler_end( NsProfiler *profiler );


/* These functions just call ns_profiler_end() and then return the
	passed 'value'. */
NS_IMPEXP NsError ns_profiler_end_error( NsProfiler *profiler, NsError value );
NS_IMPEXP nspointer ns_profiler_end_pointer( NsProfiler *profiler, nspointer value );
NS_IMPEXP nslong ns_profiler_end_long( NsProfiler *profiler, nslong value );
NS_IMPEXP nsulong ns_profiler_end_ulong( NsProfiler *profiler, nsulong value );
#ifdef NS_HAVE_LONGLONG
NS_IMPEXP nslonglong ns_profiler_end_longlong( NsProfiler *profiler, nslonglong value );
NS_IMPEXP nsulonglong ns_profiler_end_ulonglong( NsProfiler *profiler, nsulonglong value );
#endif
NS_IMPEXP nsfloat ns_profiler_end_float( NsProfiler *profiler, nsfloat value );
NS_IMPEXP nsdouble ns_profiler_end_double( NsProfiler *profiler, nsdouble value );


NS_IMPEXP void ns_profiler_shell( NsProfiler *profiler );


/* Global profiler. Only active if both NS_DEBUG
	and NS_ENABLE_PROFILING are defined. */
NS_IMPEXP NsProfiler* ns_profiler( void );


/* Internal. DO NOT USE! */
NS_IMPEXP NS_INTERNAL void _ns_profiler_init( void );
NS_IMPEXP NS_INTERNAL void _ns_profiler_finalize( void );


#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )

#define NS_PROFILER_CLEAR()\
	ns_profiler_clear( ns_profiler() )

#define NS_PROFILER_BEGIN( name )\
	ns_profiler_begin( ns_profiler(), (name) )

#define NS_PROFILER_END()\
	ns_profiler_end( ns_profiler() )

#define NS_PROFILER_END_ERROR( value )\
	ns_profiler_end_error( ns_profiler(), (value) )

#define NS_PROFILER_END_POINTER( value )\
	ns_profiler_end_pointer( ns_profiler(), (value) )

#define NS_PROFILER_END_LONG( value )\
	ns_profiler_end_long( ns_profiler(), (value) )

#define NS_PROFILER_END_ULONG( value )\
	ns_profiler_end_ulong( ns_profiler(), (value) )

#define NS_PROFILER_END_FLOAT( value )\
	ns_profiler_end_float( ns_profiler(), (value) )

#define NS_PROFILER_END_DOUBLE( value )\
	ns_profiler_end_double( ns_profiler(), (value) )

#define NS_PROFILER_SHELL()\
	ns_profiler_shell( ns_profiler() )

#else

#define NS_PROFILER_CLEAR()

#define NS_PROFILER_BEGIN( name )

#define NS_PROFILER_END()

#define NS_PROFILER_END_ERROR( value ) (value)

#define NS_PROFILER_END_POINTER( value ) (value)

#define NS_PROFILER_END_LONG( value ) (value)

#define NS_PROFILER_END_ULONG( value ) (value)

#define NS_PROFILER_END_FLOAT( value ) (value)

#define NS_PROFILER_END_DOUBLE( value ) (value)

#define NS_PROFILER_SHELL()

#endif

NS_DECLS_END

#endif/* __NS_STD_PROFILER_H__ */
