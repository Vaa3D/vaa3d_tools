#ifndef __NS_IMAGE_PIXELS_MP_FILTER_H__
#define __NS_IMAGE_PIXELS_MP_FILTER_H__

#include <image/nspixels.h>

NS_DECLS_BEGIN

#define NS_PIXEL_PROC_MP_FILTER_BLUR                1
#define NS_PIXEL_PROC_MP_FILTER_MEDIAN              2
#define NS_PIXEL_PROC_MP_FILTER_BLUR_MORE           3
#define NS_PIXEL_PROC_MP_FILTER_INVERT              4
#define NS_PIXEL_PROC_MP_FILTER_Z_BLUR              5
#define NS_PIXEL_PROC_MP_FILTER_LUM_U12_TO_LUM_U16  6

NS_IMPEXP NsProc* ns_pixel_proc_mp_filter( void );

NS_IMPEXP const nschar* ns_pixel_proc_mp_filter_type_to_string( nsint type );

NS_DECLS_END

#endif/* __NS_IMAGE_PIXELS_MP_FILTER_H__ */
