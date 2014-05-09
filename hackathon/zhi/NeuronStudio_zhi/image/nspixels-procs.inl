
/* Just for reference, don't include or call! */

void ____ns_pixel_proc_get_pixel
	(
	NsPixelType      pixel_type,
	nsconstpointer   pixels,
	nssize           width,
	nssize           height,
	nssize           length,
	nssize           row_align,
	nssize           x,
	nssize           y,
	nssize           z,
	NsPixel         *pixel
	);


void ____ns_pixel_proc_set_pixel
	(
	NsPixelType     pixel_type,
	nspointer       pixels,
	nssize          width,
	nssize          height,
	nssize          length,
	nssize          row_align,
	nssize          x,
	nssize          y,
	nssize          z,
	const NsPixel  *pixel
	);


void ____ns_pixel_proc_convert
	(
	NsPixelType     src_pixel_type,
	nsconstpointer  src_pixels,
	nssize          width,
	nssize          height,
	nssize          length,
	nssize          src_row_align,
	NsPixelType     dest_pixel_type,
	nspointer       dest_pixels,
	nssize          dest_row_align,
	NsProgress     *progress
	);


void ____ns_pixel_proc_copy
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_blur
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nspointer             dest_pixels,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_brightness_contrast
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsdouble              brightness,
	nsdouble              contrast,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_ortho_project
	(
	NsPixelType           src_pixel_type,
	nsconstpointer        src_pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                src_row_align,
	nsenum                project_type,
	NsPixelType           dest_pixel_type,
	nspointer             dest_pixels_xy,
	nssize                dest_row_align_xy,
	nspointer             dest_pixels_zy,
	nssize                dest_row_align_zy,
	nspointer             dest_pixels_xz,
	nssize                dest_row_align_xz,
	const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
	nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
	NsProgress           *progress
	);


void ____ns_pixel_proc_dynamic_range
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsfloat               channel_min,
	nsfloat               channel_max,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_mp_filter
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsint                 type,
	nssize                num_cpu,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_resize
	(
	NsPixelType      pixel_type,
	nsconstpointer   src_pixels,
	nssize           src_width,
	nssize           src_height,
	nssize           src_length,
	nssize           src_row_align,
	nsenum           interp_type,
	nspointer        dest_pixels,
	nssize           dest_width,
	nssize           dest_height,
	nssize           dest_length,
	nssize           dest_row_align,
	NsProgress      *progress
	);


void ____ns_pixel_proc_noise
	(
	NsPixelType           pixel_type,
	nspointer             pixels,
	nssize                width,
	nssize                height,
	nssize                length,
	nssize                row_align,
	nsdouble              level,
	nssize                iterations,
	nsdouble              average_intensity,
	nsdouble             *signal_to_noise_ratio,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);


void ____ns_pixel_proc_flip
	(
	NsPixelType   pixel_type,
	nspointer     pixels,
	nssize        width,
	nssize        height,
	nssize        length,
	nssize        row_align,
	nsint         type,
	NsProgress   *progress
	);


void ____ns_pixel_proc_subsample
	(
	NsPixelType           pixel_type,
	nsconstpointer        src_pixels,
	nssize                src_width,
	nssize                src_height,
	nssize                src_length,
	nssize                src_row_align,
	nspointer             dest_pixels,
	nssize                dest_width,
	nssize                dest_height,
	nssize                dest_length,
	nssize                dest_row_align,
	nsdouble              scale_x,
	nsdouble              scale_y,
	nsdouble              scale_z,
	const NsPixelRegion  *region,
	NsProgress           *progress
	);

