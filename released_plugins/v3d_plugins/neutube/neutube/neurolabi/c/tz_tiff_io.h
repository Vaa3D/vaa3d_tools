/**@file tz_tiff_io.h
 * @brief Wrapping header for tiff_io.h
 * @author Ting Zhao
 * @date 25-Dec-2008
 */

/* _TIFF_IO instead of _TZ_TIFF_IO_H */
#ifndef _TIFF_IO
/* let _TIFF_IO be defined in tiff_io.h */

#include "tz_cdefs.h"

__BEGIN_DECLS

#ifdef TIFF_BYTE /* avoid redefinition from libtiff */
#  undef TIFF_BYTE
#  undef TIFF_ASCII
#  undef TIFF_SHORT
#  undef TIFF_LONG
#  undef TIFF_RATIONAL
#  undef TIFF_SBTYE
#  undef TIFF_UNDEFINED
#  undef TIFF_SSHORT
#  undef TIFF_SLONG
#  undef TIFF_SRATIONAL
#  undef TIFF_FLOAT
#  undef TIFF_DOUBLE
#endif

#include <tiff_io.h>

__END_DECLS

#endif
