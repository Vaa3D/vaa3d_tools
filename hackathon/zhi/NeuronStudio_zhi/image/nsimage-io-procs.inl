
/* Just for reference, don't include or call! */

void ____ns_io_proc_write_raw
	(
	const nschar         *file_name,
	NsPixelType           pixel_type,
	nsconstpointer        pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	const NsPixelRegion  *region,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	);


void ____ns_io_proc_write_tiff
	(
	const nschar         *file_name,
	NsPixelType           pixel_type,
	nsconstpointer        pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	const NsPixelRegion  *region,
	NsPixelType           conv_pixel_type,
	NsProgress           *progress
	);


void ____ns_io_proc_write_jpeg
	(
	const nschar    *file_name,
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nsint            quality,
	NsProgress      *progress
	);
