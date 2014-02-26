
#define _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE( src_type, dest_type, func )\
   nssize           x;\
   nssize           y;\
   nssize           z;\
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
   /*PAGELOADER      *pl;*/\
\
   NS_USE_VARIABLE( length );\
	NS_USE_VARIABLE( z_indices );\
	NS_USE_VARIABLE( num_z_indices );\
\
	/*TEMP*/NS_USE_VARIABLE( use_src_size );\
\
   src_bytes_per_row   = ns_pixel_buffer_row_size( src_pixel_type, width, src_row_align );\
   src_bytes_per_slice = ns_pixel_buffer_slice_size( src_pixel_type, width, height, src_row_align );\
   src_slice           = src_pixels;\
\
   src_slice = NS_OFFSET_POINTER( src_type, src_slice, region->z * src_bytes_per_slice );\
   src_slice = NS_OFFSET_POINTER( src_type, src_slice, region->y * src_bytes_per_row );\
   src_slice = NS_OFFSET_POINTER( src_type, src_slice, region->x * sizeof( src_type ) );\
\
   dest_bytes_per_row_xy = ns_pixel_buffer_row_size( dest_pixel_type, region->width,  dest_row_align_xy );\
   dest_bytes_per_row_zy = ns_pixel_buffer_row_size( dest_pixel_type, region->length, dest_row_align_zy );\
   dest_bytes_per_row_xz = ns_pixel_buffer_row_size( dest_pixel_type, region->width,  dest_row_align_xz );\
\
   dest_row_xy = dest_pixels_xy;\
   dest_col_zy = dest_pixels_zy;\
   dest_row_xz = dest_pixels_xz;\
\
   sum = 0;\
\
   if( NULL != progress )\
      {\
      ns_progress_set_title( progress, _ns_pixel_proc_ortho_project.title );\
      ns_progress_update( progress, NS_PROGRESS_BEGIN );\
      }\
\
   /*pl = CreatePageLoader( ( const nsuchar* )src_pixels, src_bytes_per_slice * length, 10485760 );*/\
\
   for( z = 0; z < region->length; ++z )\
      {\
      /*AnnounceAccess( pl, ( const nsuchar* )src_slice );*/\
\
      if( NULL != progress )\
         {\
         if( ns_progress_cancelled( progress ) )\
            return ns_no_error();\
\
         ns_progress_update(\
            progress,\
            100.0f * ( ( nsfloat )z / ( nsfloat )region->length )\
            );\
         }\
\
      src_row     = src_slice;\
      dest_row_xy = dest_pixels_xy;\
      dest_zy     = dest_col_zy;\
      dest_xz     = dest_row_xz;\
\
      for( y = 0; y < region->height; ++y )\
         {\
         src     = src_row;\
         dest_xy = dest_row_xy;\
\
         for( x = 0; x < region->width; ++x )\
            {\
            func;\
\
            ++src;\
            ++dest_xy;\
            ++dest_xz;\
\
            sum += ( nsulonglong )src->luminance;\
            }\
\
         src_row     = NS_OFFSET_POINTER( src_type, src_row, src_bytes_per_row );\
         dest_row_xy = NS_OFFSET_POINTER( dest_type, dest_row_xy, dest_bytes_per_row_xy );\
         dest_xz     = dest_row_xz;\
         dest_zy     = NS_OFFSET_POINTER( dest_type, dest_zy, dest_bytes_per_row_zy );\
         }\
\
      src_slice   = NS_OFFSET_POINTER( src_type, src_slice, src_bytes_per_slice );\
      dest_row_xz = NS_OFFSET_POINTER( dest_type, dest_row_xz, dest_bytes_per_row_xz );\
      ++dest_col_zy;\
      }\
\
	*num_pixels        = region->width * region->height * region->length;\
	*average_intensity = ( ( nsdouble )sum ) / ( ( nsdouble )(*num_pixels) );\
\
   /*DestroyPageLoader( pl );*/\
\
   if( NULL != progress )\
      ns_progress_update( progress, NS_PROGRESS_END );\
\
	return ns_no_error()


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_max_lum_u8_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumU8,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_min_lum_u8_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumU8,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_sum_lum_u8_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU8,
      NsPixelLumF32,
      _NS_PIXEL_PROC_ORTHO_PROJECT_SUM_LUM
      );
   }


NS_COMPILE_TIME_SIZE_EQUAL( NsPixelLumU12, NsPixelLumU16 );

NS_PRIVATE NsError _ns_pixel_proc_ortho_project_max_lum_u12_or_u16_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumU16,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_min_lum_u12_or_u16_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumU16,
      _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM
      );
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_sum_lum_u12_or_u16_no_indices
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
   _NS_PIXEL_PROC_ORTHO_PROJECT_NO_INDICES_TEMPLATE(
      NsPixelLumU16,
      NsPixelLumF32,
      _NS_PIXEL_PROC_ORTHO_PROJECT_SUM_LUM
      );
   }
