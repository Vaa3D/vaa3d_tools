#ifndef __NS_IMAGE_IO_WRITE_JPEG_H__
#define __NS_IMAGE_IO_WRITE_JPEG_H__

#include <std/nsfile.h>
#include <std/nsio.h>
#include <image/nspixels.h>
#include <image/jpeg.h>

NS_DECLS_BEGIN

enum
	{
	NS_IO_PROC_WRITE_JPEG_FILE_NAME,
	NS_IO_PROC_WRITE_JPEG_PIXEL_TYPE,
	NS_IO_PROC_WRITE_JPEG_PIXELS,
	NS_IO_PROC_WRITE_JPEG_WIDTH,
	NS_IO_PROC_WRITE_JPEG_HEIGHT,
	NS_IO_PROC_WRITE_JPEG_LENGTH,
	NS_IO_PROC_WRITE_JPEG_ROW_ALIGN,
	NS_IO_PROC_WRITE_JPEG_QUALITY,
	NS_IO_PROC_WRITE_JPEG_PROGRESS,

	NS_IO_PROC_WRITE_JPEG_NUM_PARAMS
	};


NS_IMPEXP NsProc* ns_io_proc_write_jpeg( void );

NS_DECLS_END

#endif/* __NS_IMAGE_IO_WRITE_JPEG_H__ */
