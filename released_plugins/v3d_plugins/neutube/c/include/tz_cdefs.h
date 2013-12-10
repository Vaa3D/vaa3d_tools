/* tz_cdefs.h
 * 
 * Initial write: Ting Zhao
 */

#ifndef _TZ_CDEFS_H_
#define _TZ_CDEFS_H_

#ifdef HAVE_CONFIG_H
#  include "neurolabi_config.h"
#endif

#if defined __cplusplus
#  include <limits>
#endif

#ifdef _WIN32
#  ifdef __cplusplus
#    define __BEGIN_DECLS extern "C" {
#    define __END_DECLS	  }
#  else
#    define __BEGIN_DECLS
#    define __END_DECLS
#  endif
#  ifdef WIN32_DLL
#    ifdef BUILD_DLL 
#      define PUBLIC_API __declspec(dllexport)
#      define PUBLIC_VAR extern __declspec(dllexport)
#    else
#      define PUBLIC_API __declspec(dllimport)
#      define PUBLIC_VAR extern __declspec(dllimport)
#    endif
#  endif
#else
#  include <sys/cdefs.h>
#  define PUBLIC_API
#  define PUBLIC_VAR extern
#endif

#ifdef _DEBUG_
#  define PRIVATE 
#else
#  define PRIVATE static
#endif

#if defined(INVALID_ARRAY_INDEX)
#  error macro conflict
#endif

#ifndef ARCH_BIT
#  define ARCH_BIT 64
#endif

#if ARCH_BIT == 32
#  define INVALID_ARRAY_INDEX 2147483647
#endif

#if ARCH_BIT == 64
#  define INVALID_ARRAY_INDEX 9223372036854775807L
#endif

/* The BOOL type is the same as the type of a logical statement, e.g. 2 > 1 */
#ifndef HAVE_BOOL
#  ifdef C99
#    define BOOL bool
#    define TRUE true
#    define FALSE false
#  else
#    define BOOL  int
#    ifndef TRUE
#      define TRUE  1
#    endif
#    ifndef FALSE
#      define FALSE 0
#    endif
#  endif
#endif

/* typedef enum {FALSE = 0, TRUE} BOOL; */ /* might not work for C++*/

#ifndef NO_INLINE
#  ifndef HAVE_INLINE
#    define HAVE_INLINE
#  endif
#endif

#if defined __cplusplus
#  define INLINE inline
#elif (defined HAVE_INLINE)
#  define INLINE static inline
/* for gsl compatibility */
#  define INLINE_FUN static inline
#else
#  define INLINE static
#endif

#define TZ_CONCAT(t1, t2) t1 ## t2
#define TZ_CONCAT3(t1, t2, t3) t1 ## t2 ## t3
#define TZ_CONCATU(t1, t2) t1 ## _ ## t2
#define TZ_CONCATUC(t1, t2) _ ## t1 ## _ ## t2 ## _
#define TZ_CONCATU3(t1, t2, t3) t1 ## _ ## t2 ## _ ## t3

#define EXTLIB_EXIST(lib) TZ_CONCAT(HAVE_LIB, lib) == 1
//#define EXTLIB_EXIST(lib) defined(TZ_CONCATUC(HAVE, lib)) /* doesn't work because the macro is replaced by a literal constant first */

#ifndef NULL
#  define NULL 0x0
#endif

#ifndef REG_BASIC
#  define REG_BASIC 0
#endif

#ifndef _NAN
/* (big endian) 0x7FF8000000000000 */
#  ifndef __cplusplus
#    define _NAN (0.0 / 0.0) 
#  else
#    define _NAN (std::numeric_limits<double>::quiet_NaN())
#  endif
#ifdef _MSC_VER
#define NaN _NAN
#else
static const double NaN = _NAN;
#endif
#endif

#ifndef _INFINITY
/* (big endian) 0x7FF0000000000000 */
#  ifndef __cplusplus
#    define _INFINITY (1.0 / 0.0)
#  else
#    define _INFINITY (std::numeric_limits<double>::infinity())
#  endif
#ifdef _MSC_VER
#define Infinity _INFINITY
#else
static const double Infinity = _INFINITY;
#endif
#endif

#define UNUSED_PARAMETER(var) if (var) {}

#endif

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#include <cmath>
#define isnan _isnan
#define lrint(x) (floor((x)+((x)>0) ? 0.5 : -0.5))
#define isinf(x) ((x)==_INFINITY)
#define INFINITY _INFINITY
#define round(x) floor((x)+.5)
#endif
