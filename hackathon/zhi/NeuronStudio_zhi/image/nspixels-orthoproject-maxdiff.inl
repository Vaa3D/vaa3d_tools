
#define _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE( src_type, dest_type, func )\
   nssize           x;\
   nssize           y;\
   nssize           src_z;\
	nssize           dest_z;\
	nssize           i;\
   nssize           src_bytes_per_row;\
   nssize           src_bytes_per_slice;\
   const src_type  *src_slice;\
   const src_type  *src_row;\
   const src_type  *src;\
   nssize           dest_bytes_per_row_xy;\
   dest_type       *dest_row_xy;\
   dest_type       *dest_xy;\
   nssize           dest_bytes_per_row_zy;\
   dest_type       *dest_col_zy;\
   dest_type       *dest_zy;\
   nssize           dest_bytes_per_row_xz;\
   dest_type       *dest_row_xz;\
   dest_type       *dest_xz;\
   nsulonglong      sum;\
   nsdouble         curr_average_intensity;\
	nssize           curr_num_pixels;\
	nssize           dest_temp_bytes_per_slice_xy;\
	nssize           dest_temp_bytes_per_row_xy;\
	nsint           *dest_temp_pixels_xy;\
	nsint           *dest_temp_row_xy;\
	nsint           *dest_temp_xy;\
	nsint            lum1, lum2, diff;\
\
	NS_USE_VARIABLE( lum1 ); NS_USE_VARIABLE( lum2 ); NS_USE_VARIABLE( diff );\
\
   src_bytes_per_row   = ns_pixel_buffer_row_size( src_pixel_type, width, src_row_align );\
   src_bytes_per_slice = ns_pixel_buffer_slice_size( src_pixel_type, width, height, src_row_align );\
\
   dest_bytes_per_row_xy = ns_pixel_buffer_row_size( dest_pixel_type, use_src_size ? width  : region->width,  dest_row_align_xy );\
   dest_bytes_per_row_zy = ns_pixel_buffer_row_size( dest_pixel_type, use_src_size ? length : region->length, dest_row_align_zy );\
   dest_bytes_per_row_xz = ns_pixel_buffer_row_size( dest_pixel_type, use_src_size ? width  : region->width,  dest_row_align_xz );\
\
   sum = 0;\
\
   if( NULL != progress )\
      {\
      ns_progress_set_title( progress, _ns_pixel_proc_ortho_project.title );\
      ns_progress_update( progress, NS_PROGRESS_BEGIN );\
      }\
\
	dest_temp_bytes_per_row_xy   = sizeof( nsint ) * ( use_src_size ? width : region->width );\
	dest_temp_bytes_per_slice_xy = dest_temp_bytes_per_row_xy * ( use_src_size ? height : region->height );\
\
	if( NULL == ( dest_temp_pixels_xy = ns_calloc( dest_temp_bytes_per_slice_xy, 1 ) ) )\
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );\
\
   for( i = 0; i < num_z_indices; ++i )\
      {\
      if( NULL != progress )\
         {\
         if( ns_progress_cancelled( progress ) )\
				{\
				ns_free( dest_temp_pixels_xy );\
            return ns_no_error();\
				}\
\
         ns_progress_update(\
            progress,\
            100.0f * ( ( nsfloat )i / ( nsfloat )num_z_indices )\
            );\
         }\
\
		src_z = z_indices[i];\
		ns_assert( region->z <= src_z && src_z < region->z + region->length );\
\
		if( use_src_size )\
			dest_z = src_z;\
		else\
			{\
			dest_z = ( src_z - region->z );\
			ns_assert( dest_z < region->length );\
			}\
\
		src_slice = src_pixels;\
		src_slice = NS_OFFSET_POINTER( src_type, src_slice, src_z * src_bytes_per_slice );\
		src_slice = NS_OFFSET_POINTER( src_type, src_slice, region->y * src_bytes_per_row );\
		src_slice = NS_OFFSET_POINTER( src_type, src_slice, region->x * sizeof( src_type ) );\
\
		dest_row_xy      = dest_pixels_xy;\
		dest_temp_row_xy = dest_temp_pixels_xy;\
		dest_col_zy      = dest_pixels_zy + dest_z;\
		dest_row_xz      = NS_OFFSET_POINTER( dest_type, dest_pixels_xz, dest_bytes_per_row_xz * dest_z );\
\
		if( use_src_size )\
			{\
			dest_row_xy = NS_OFFSET_POINTER( dest_type, dest_row_xy, region->y * dest_bytes_per_row_xy );\
			dest_row_xy = NS_OFFSET_POINTER( dest_type, dest_row_xy, region->x * sizeof( dest_type ) );\
\
			dest_temp_row_xy = NS_OFFSET_POINTER( nsint, dest_temp_row_xy, region->y * dest_temp_bytes_per_row_xy );\
			dest_temp_row_xy = NS_OFFSET_POINTER( nsint, dest_temp_row_xy, region->x * sizeof( nsint ) );\
\
			dest_col_zy = NS_OFFSET_POINTER( dest_type, dest_col_zy, region->y * dest_bytes_per_row_zy );\
\
			dest_row_xz = NS_OFFSET_POINTER( dest_type, dest_row_xz, region->x * sizeof( dest_type ) );\
			}\
\
      src_row = src_slice;\
		dest_zy = dest_col_zy;\
		dest_xz = dest_row_xz;\
\
      for( y = 0; y < region->height; ++y )\
         {\
         src          = src_row;\
         dest_xy      = dest_row_xy;\
         dest_temp_xy = dest_temp_row_xy;\
\
         for( x = 0; x < region->width; ++x )\
            {\
            func;\
\
            ++src;\
            ++dest_xy;\
				++dest_temp_xy;\
            ++dest_xz;\
\
            sum += ( nsulonglong )src->luminance;\
            }\
\
         src_row          = NS_OFFSET_POINTER( src_type, src_row, src_bytes_per_row );\
         dest_row_xy      = NS_OFFSET_POINTER( dest_type, dest_row_xy, dest_bytes_per_row_xy );\
         dest_temp_row_xy = NS_OFFSET_POINTER( nsint, dest_temp_row_xy, dest_temp_bytes_per_row_xy );\
         dest_xz          = dest_row_xz;\
         dest_zy          = NS_OFFSET_POINTER( dest_type, dest_zy, dest_bytes_per_row_zy );\
         }\
      }\
\
	curr_num_pixels        = region->width * region->height * num_z_indices;\
   curr_average_intensity = ( ( nsdouble )sum ) / ( ( nsdouble )curr_num_pixels );\
\
   *average_intensity =\
		( (*average_intensity)    * ( nsdouble )(*num_pixels) +\
			curr_average_intensity * ( nsdouble )curr_num_pixels )\
			/\
		( ( nsdouble )( *num_pixels + curr_num_pixels ) );\
\
	*num_pixels += curr_num_pixels;\
\
   if( NULL != progress )\
      ns_progress_update( progress, NS_PROGRESS_END );\
\
	ns_free( dest_temp_pixels_xy );\
	return ns_no_error()


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_max_lum_u8_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU8   *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumU8         *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumU8         *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumU8         *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumU8,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM_MAX_DIFF( NsPixelLumU8 )
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_min_lum_u8_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU8   *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumU8         *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumU8         *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumU8         *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumU8,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM_MAX_DIFF( NsPixelLumU8 )
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_sum_lum_u8_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU8   *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumF32        *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumF32        *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumF32        *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumF32,
      _NS_PIXEL_PROC_ORTHO_PROJECT_SUM_LUM
      );
   }


NS_COMPILE_TIME_SIZE_EQUAL( NsPixelLumU12, NsPixelLumU16 );

NS_PRIVATE NsError _ns_pixel_proc_ortho_project_max_lum_u12_or_u16_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU16  *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumU16        *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumU16        *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumU16        *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumU16,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM_MAX_DIFF( NsPixelLumU16 )
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_min_lum_u12_or_u16_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU16  *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumU16        *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumU16        *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumU16        *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumU16,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM_MAX_DIFF( NsPixelLumU16 )
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_sum_lum_u12_or_u16_max_diff
   (
   NsPixelType           src_pixel_type,
   const NsPixelLumU16  *src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                src_row_align,
   NsPixelType           dest_pixel_type,
   NsPixelLumF32        *dest_pixels_xy,
   nssize                dest_row_align_xy,
   NsPixelLumF32        *dest_pixels_zy,
   nssize                dest_row_align_zy,
   NsPixelLumF32        *dest_pixels_xz,
   nssize                dest_row_align_xz,
   const NsPixelRegion  *region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_DIFF_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumF32,
      _NS_PIXEL_PROC_ORTHO_PROJECT_SUM_LUM
      );
   }
