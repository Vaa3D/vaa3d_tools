/**@file tz_png_io.h
 * @author Ting Zhao
 * @date 03-Jul-2012
 */

#ifndef _TZ_PNG_IO_H_
#define _TZ_PNG_IO_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

#if defined(HAVE_LIBPNG)
#include <png.h>
#else
typedef int png_info;
typedef void* png_infop;
#endif

Stack* Read_Png(const char *file_path);
Mc_Stack* Read_Png_M(const char *file_path);
png_infop Read_Png_Info(const char *file_path);
BOOL Is_Png(const char *file_path);

void Png_Attribute(const char *file_path, int *kind, int *width, int *height);

__END_DECLS

#endif
