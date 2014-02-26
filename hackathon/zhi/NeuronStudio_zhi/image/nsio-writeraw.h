#ifndef __NS_IMAGE_IO_WRITE_RAW_H__
#define __NS_IMAGE_IO_WRITE_RAW_H__

#include <std/nsfile.h>
#include <std/nsio.h>
#include <image/nspixels.h>
#include <image/raw.h>

NS_DECLS_BEGIN

enum
	{
	NS_IO_PROC_WRITE_RAW_FILE_NAME,
	NS_IO_PROC_WRITE_RAW_PIXEL_TYPE,
	NS_IO_PROC_WRITE_RAW_PIXELS,
	NS_IO_PROC_WRITE_RAW_WIDTH,
	NS_IO_PROC_WRITE_RAW_HEIGHT,
	NS_IO_PROC_WRITE_RAW_LENGTH,
	NS_IO_PROC_WRITE_RAW_ROW_ALIGN,
	NS_IO_PROC_WRITE_RAW_REGION,
	NS_IO_PROC_WRITE_RAW_CONV_PIXEL_TYPE,
	NS_IO_PROC_WRITE_RAW_PROGRESS,

	NS_IO_PROC_WRITE_RAW_NUM_PARAMS
	};


NS_IMPEXP NsProc* ns_io_proc_write_raw( void );

NS_IMPEXP RawPixelType ns_pixel_type_to_raw_pixel_type( NsPixelType pixel_type );

NS_DECLS_END

#endif/* __NS_IMAGE_IO_WRITE_RAW_H__ */
