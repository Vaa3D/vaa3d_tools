#ifndef __NS_IMAGE_IO_WRITE_TIFF_H__
#define __NS_IMAGE_IO_WRITE_TIFF_H__

#include <std/nsfile.h>
#include <std/nsio.h>
#include <image/nspixels.h>
#include <image/tiff.h>

NS_DECLS_BEGIN

enum
	{
	NS_IO_PROC_WRITE_TIFF_FILE_NAME,
	NS_IO_PROC_WRITE_TIFF_PIXEL_TYPE,
	NS_IO_PROC_WRITE_TIFF_PIXELS,
	NS_IO_PROC_WRITE_TIFF_WIDTH,
	NS_IO_PROC_WRITE_TIFF_HEIGHT,
	NS_IO_PROC_WRITE_TIFF_LENGTH,
	NS_IO_PROC_WRITE_TIFF_ROW_ALIGN,
	NS_IO_PROC_WRITE_TIFF_REGION,
	NS_IO_PROC_WRITE_TIFF_CONV_PIXEL_TYPE,
	NS_IO_PROC_WRITE_TIFF_PROGRESS,

	NS_IO_PROC_WRITE_TIFF_NUM_PARAMS
	};


NS_IMPEXP NsProc* ns_io_proc_write_tiff( void );

NS_IMPEXP TiffPixelInfo* ns_pixel_type_to_tiff_pixel_info( NsPixelType pixel_type, TiffPixelInfo *info );

NS_DECLS_END

#endif/* __NS_IMAGE_IO_WRITE_TIFF_H__ */
