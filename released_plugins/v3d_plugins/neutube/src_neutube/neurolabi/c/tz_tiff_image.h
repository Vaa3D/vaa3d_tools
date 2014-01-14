/**@file tz_tiff_image.h
 * @brief hack of tiff_image.h
 * @author Ting Zhao
 * @date 20-Nov-2008
 */

#ifndef _TZ_TIFF_IMAGE_H_
#define _TZ_TIFF_IMAGE_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#include <tiff_image.h>

/* rename public types and functions */
#define Channel_Meaning Tz_Channel_Meaning
#define CHAN_WHITE TZ_CHAN_WHITE
#define CHAN_BLACK TZ_CHAN_BLACK
#define CHAN_MAPPED TZ_CHAN_MAPPED
#define CHAN_RED TZ_CHAN_RED
#define CHAN_GREEN TZ_CHAN_GREEN
#define CHAN_BLUE TZ_CHAN_BLUE
#define CHAN_ALPHA TZ_CHAN_ALPHA
#define CHAN_MATTE TZ_CHAN_MATTE
#define CHAN_MASK TZ_CHAN_MASK
#define CHAN_OTHER TZ_CHAN_OTHER
#define CHAN_UNSIGNED TZ_CHAN_UNSIGNED
#define CHAN_SIGNED TZ_CHAN_SIGNED
#define CHAN_FLOAT TZ_CHAN_FLOAT
#define Channel_Type Tz_Channel_Type
#define Tiff_Histogram Tz_Tiff_Histogram
#define Tiff_Channel Tz_Tiff_Channel
#define Tiff_Image Tz_Tiff_Image
#define Tiff_Image_Error Tz_Tiff_Image_Error
#define Extract_Image_From_IFD Tz_Extract_Image_From_IFD
#define Create_Tiff_Image Tz_Create_Tiff_Image
#define Add_Tiff_Image_Channel Tz_Add_Tiff_Image_Channel
#define Make_IFD_For_Image Tz_Make_IFD_For_Image
#define Scale_Tiff_Channel Tz_Scale_Tiff_Channel
#define Range_Tiff_Channel Tz_Range_Tiff_Channel
#define Shift_Tiff_Channel Tz_Shift_Tiff_Channel
#define Histogram_Tiff_Channel Tz_Histogram_Tiff_Channel
#define Histogram_Tiff_Image_Channels Tz_Histogram_Tiff_Image_Channels
#define Tiff_Histogram_Merge Tz_Tiff_Histogram_Merge
#define Scale_Tiff_Image Tz_Scale_Tiff_Image
#define Shift_Tiff_Image Tz_Shift_Tiff_Image
#define Copy_Tiff_Channel Tz_Copy_Tiff_Channel
#define Pack_Tiff_Channel Tz_Pack_Tiff_Channel
#define Free_Tiff_Channel Tz_Free_Tiff_Channel
#define Kill_Tiff_Channel Tz_Kill_Tiff_Channel
#define Reset_Tiff_Channel Tz_Reset_Tiff_Channel
#define Tiff_Channel_Usage Tz_Tiff_Channel_Usage
#define Copy_Tiff_Image Tz_Copy_Tiff_Image
#define Pack_Tiff_Image Tz_Pack_Tiff_Image
#define Free_Tiff_Image Tz_Free_Tiff_Image
#define Kill_Tiff_Image Tz_Kill_Tiff_Image
#define Reset_Tiff_Image Tz_Reset_Tiff_Image
#define Tiff_Image_Usage Tz_Tiff_Image_Usage
#define Copy_Tiff_Histogram Tz_Copy_Tiff_Histogram
#define Pack_Tiff_Histogram Tz_Pack_Tiff_Histogram
#define Free_Tiff_Histogram Tz_Free_Tiff_Histogram
#define Kill_Tiff_Histogram Tz_Kill_Tiff_Histogram
#define Reset_Tiff_Histogram Tz_Reset_Tiff_Histogram
#define Tiff_Histogram_Usage Tz_Tiff_Histogram_Usage
#define halfp_2_fullp tz_halfp_2_fullp
#define PackBits_Decoder Tz_PackBits_Decoder
#define PackBits_Encoder Tz_PackBits_Encoder
#define LZW_Decoder Tz_LZW_Decoder
#define LZW_Encoder Tz_LZW_Encoder
/*****************************/

#undef _TIFF_IMAGE
#include <tiff_image.h>
#undef _TIFF_IMAGE

Tiff_IFD *Make_IFD_For_Image_I(Tiff_Image *image, int compression, 
			       Tiff_IFD *ifd);

Tiff_IFD *Make_IFD_For_Lsm_Image(Tiff_Image *image, int compression, 
				 Tiff_IFD *template_ifd, int depth);

__END_DECLS

#endif
