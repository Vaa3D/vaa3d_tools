#ifndef __NS_CONFIGURE_H__
#define __NS_CONFIGURE_H__

/* These macros configure the build. *****************************************/ 

#define NS_HAVE_STDLIB_H
#define NS_HAVE_STDIO_H
#define NS_HAVE_STRING_H
#define NS_HAVE_MATH_H
#define NS_HAVE_STDARG_H
#define NS_HAVE_SETJMP_H
#define NS_HAVE_ERRNO_H
#define NS_HAVE_SYS_STAT_H
#define NS_HAVE_TIME_H
/* #define NS_HAVE_UNISTD_H */

// #define NS_OS_LINUX
 #define NS_OS_MAC
/* #define NS_OS_UNIX */

#define NS_CPU_LITTLE_ENDIAN
/* #define NS_CPU_BIG_ENDIAN */

#define NS_DEBUG
#define NS_DEBUG_EXTREME

#define NS_STATIC_LIBRARY
/* #define NS_IMPORT_DYNAMIC_LIBRARY */
/* #define NS_EXPORT_DYNAMIC_LIBRARY */

#define NS_POINTER_BITS  64

#define NS_FLOAT_BITS  32
/* #define NS_FLOAT_BITS  64 */

/* #define NS_HAVE_LONGDOUBLE */

/* #define NS_ENABLE_PROFILING */

#define NS_VERSION_MAJOR  0
#define NS_VERSION_MINOR  9
#define NS_VERSION_MICRO  92


#ifdef NS_OS_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef _MT
		#define _MT
	#endif
#endif

#ifdef NS_IMPORT_DYNAMIC_LIBRARY
	#ifdef NS_OS_WINDOWS
		#define NS_IMPEXP __declspec( dllimport )
	#else
		#define NS_IMPEXP
	#endif
#else
	#define NS_IMPEXP
#endif

#ifdef NS_EXPORT_DYNAMIC_LIBRARY
	#ifdef NS_OS_WINDOWS
		#define NS_IMPEXP __declspec( dllexport )
	#else
		#define NS_IMPEXP
	#endif
#else
	#define NS_IMPEXP
#endif


#if defined( NS_STATIC_LIBRARY ) && ( defined( NS_IMPORT_DYNAMIC_LIBARY ) || defined( NS_EXPORT_DYNAMIC_LIBRARY ) )
	#error nsconfigure.h compilation error: Conflicting libary build flags
#endif

#if defined( NS_IMPORT_DYNAMIC_LIBARY ) && defined( NS_EXPORT_DYNAMIC_LIBRARY )
	#error nsconfigure.h compilation error: Invalid dynamic library build flags
#endif

#if ! defined( NS_CPU_LITTLE_ENDIAN ) && ! defined( NS_CPU_BIG_ENDIAN )
	#error nsconfigure.h compilation error: Must specify CPU endian
#endif

#if defined( NS_CPU_LITTLE_ENDIAN ) && defined( NS_CPU_BIG_ENDIAN )
	#error nsconfigure.h compilation error: Conflicting CPU endian build flags
#endif

#ifndef NS_POINTER_BITS
	#error nsconfigure.h compilation error: Must specify pointer bits
#endif

#if 32 != NS_POINTER_BITS && 64 != NS_POINTER_BITS
	#error nsconfigure.h compilation error: Invalid pointer bits setting
#endif

#if 32 != NS_FLOAT_BITS && 64 != NS_FLOAT_BITS
	#error nsconfigure.h compilation error: Invalid float bits setting
#endif

#endif/* __NS_CONFIGURE_H__ */
