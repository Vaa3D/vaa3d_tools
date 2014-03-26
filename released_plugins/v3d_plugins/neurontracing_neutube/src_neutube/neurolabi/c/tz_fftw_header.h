/**@file tz_fftw_header.h
 * @brief Wrapping header for fftw
 * @author Ting Zhao
 * @date 09-Oct-2008
 */

#ifndef _TZ_FFTW_HEADER_H_
#define _TZ_FFTW_HEADER_H_

#include "tz_cdefs.h"

#if defined(HAVE_LIBFFTW3) || defined(HAVE_LIBFFW3F)

#  include <fftw3.h>

#else

typedef double fftw_complex[2];
typedef float fftwf_complex[2];
#  define FFTW_CONCAT(prefix, name) prefix ## name
#  define FFTW_MANGLE_DOUBLE(name) FFTW_CONCAT(fftw_, name)
#  define FFTW_MANGLE_FLOAT(name) FFTW_CONCAT(fftwf_, name)
#  define FFTW_MANGLE_LONG_DOUBLE(name) FFTW_CONCAT(fftwl_, name)
#  include <stdlib.h>
#  define fftw_malloc malloc
#  define fftwf_malloc malloc

#endif


#endif
