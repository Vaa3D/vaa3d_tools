#ifndef _TZ_STDINT_H_
#define _TZ_STDINT_H_

#include "neurolabi_config.h"

#if !defined(HAVE_STDINT_H)
#  if SIZEOF_CHAR == 1
#    define int8_t char
#    define uint8_t unsigned char
#    define int16_t short int
#    define uint16_t unsigned short int
#    define int32_t int
#    define uint32_t unsigned int
#    define int64_t long long
#    define uint64_t unsigned long long
#  else
#    error "Unspported system: the char size is not 1"
#  endif
#else
#  include <stdint.h>
#endif

#    ifdef HAVE_SYS_TYPES_H
#      include <sys/types.h>
#    endif

#if !defined(_SSIZE_T) && !defined(__ssize_t_defined) && !defined(_WIN64) && !defined(_WIN32) && !defined(_SSIZE_T_)
#define _SSIZE_T
#define __ssize_t_defined
#  if !defined(ARCH_64)
typedef int32_t ssize_t;
#  else
typedef int64_t ssize_t;
#  endif
#endif

#if defined(_MSC_VER)
#ifdef _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif

typedef int8_t byte_t;
typedef int16_t word_t;

#if !defined(HAVE_BZERO)
#define bzero(dest,count) memset(dest,0,count)
#endif

#endif
