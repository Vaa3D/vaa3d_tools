#include "nspixels-blur.h"


NS_PRIVATE NsProc _ns_pixel_proc_blur;


NS_PRIVATE NsClosureValue _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_NUM_PARAMS ] =
   {
   { "pixel_type",  NS_VALUE_PIXEL_TYPE },
   { "src_pixels",  NS_VALUE_CONSTPOINTER },
   { "width",       NS_VALUE_SIZE },
   { "height",      NS_VALUE_SIZE },
   { "length",      NS_VALUE_SIZE },
   { "row_align",   NS_VALUE_SIZE },
   { "dest_pixels", NS_VALUE_POINTER },
   { "region",      NS_VALUE_PIXEL_REGION },
   { "progress",    NS_VALUE_PROGRESS }
   };


typedef void ( *_NsPixelProcBlurFunc )
   (
   NsPixelType,
   nsconstpointer,
   nssize,
   nssize,
   nssize,
   nssize,
   nspointer,
   const NsPixelRegion*,
   NsProgress*
   );


typedef struct _NsPixelProcBlur
   {
   NsPixelRowGetFloatChannelFunc  row_get_float_channel_func;
   NsPixelRowSetFloatChannelFunc  row_set_float_channel_func;
   }
   NsPixelProcBlur;


#define _ns_pixel_proc_blur_to_kernel( row, x, f, ppb )\
   ( *(f) = ( ( ( NsPixelProcBlur* )(ppb) )->row_get_float_channel_func )( (row), (x) ) )

#define _ns_pixel_proc_blur_from_kernel( row, x, f, ppb )\
   ( ( ( ( NsPixelProcBlur* )(ppb) )->row_set_float_channel_func )( (row), (x), *(f) ) )


NS_CONV33F_IMPLEMENT(
   _ns_pixel_proc_blur_conv_33,
   _ns_pixel_proc_blur_to_kernel,
   _ns_pixel_proc_blur_from_kernel
   );


NS_PRIVATE void _ns_pixel_proc_blur_all_2d
   (
   NsPixelType           pixel_type,
   nsconstpointer        src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nspointer             dest_pixels,
   const NsPixelRegion  *pixel_region,
   NsProgress           *progress
   )
   {
   NsConv33f              C33;
   NsPixelInfo            info;
   const NsPixelChannel  *channel;
   NsPixelProcBlur        ppb;
   NsConvRegion2          conv_region2;
   nsfloat                percent_min;
   nsfloat                percent_max;
   nsfloat                percent_step;


   ns_verify( 1 == length );

   ns_conv33f_blur( &C33 );

   if( NULL != pixel_region )
      {
      ns_pixel_region_verify( pixel_region, width, height, length );

      ns_conv_region2(
         &conv_region2,
         pixel_region->x,
         pixel_region->y,
         pixel_region->width,
         pixel_region->height
         );
      }
   else
      ns_conv_region2( &conv_region2, 0, 0, width, height );

   ns_pixel_info( &info, pixel_type );

   percent_step = 100.0f / ( nsfloat )info.num_channels;
   percent_min  = 0.0f;
   percent_max  = percent_step;

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_BEGIN );

   /* NOTE: Dont blur the alpha channel, if there is one. */
   for( channel = info.channel; NULL != channel; channel = channel->next )
      {
      if( ! ns_pixel_channel_is( channel, NS_PIXEL_CHANNEL_ALPHA_NAME ) )
         {
         ppb.row_get_float_channel_func = ns_pixel_row_get_float_channel_func( pixel_type, channel->index );
         ppb.row_set_float_channel_func = ns_pixel_row_set_float_channel_func( pixel_type, channel->index );

         ns_assert( NULL != ppb.row_get_float_channel_func );
         ns_assert( NULL != ppb.row_set_float_channel_func );

         _ns_pixel_proc_blur_conv_33(
            &C33,
            NS_CONV_EDGE_OP_COPY_SRC,
            src_pixels,
            width,
            height,
            ns_pixel_buffer_row_size( pixel_type, width, row_align ),
            &conv_region2,
            dest_pixels,
            progress,
            percent_min,
            percent_max,
            &ppb
            );
         }

      percent_min += percent_step;
      percent_max += percent_step;
      }

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_END );
   }


/* NOTE: This procedure ignores edge pixels. */
#define _NS_PIXEL_PROC_BLUR_TEMPLATE( type, func )\
   nssize       x, y, z, w, h, l;\
   nslong       bytes_per_row;\
   nslong       bytes_per_slice;\
   const type  *src_slice;\
   const type  *src_row;\
   const type  *src_pixel;\
   type        *dest_slice;\
   type        *dest_row;\
   type        *dest_pixel;\
\
   NS_USE_VARIABLE( length );\
\
   w = region->width  - 1;\
   h = region->height - 1;\
   l = region->length - 1;\
\
   bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );\
   bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );\
\
   src_slice = src_pixels;\
   src_slice = NS_OFFSET_POINTER( const type, src_slice, region->z * bytes_per_slice );\
   src_slice = NS_OFFSET_POINTER( const type, src_slice, region->y * bytes_per_row );\
   src_slice = NS_OFFSET_POINTER( const type, src_slice, region->x * sizeof( type ) );\
\
   src_slice = NS_OFFSET_POINTER(\
                  const type,\
                  src_slice,\
                  bytes_per_slice + bytes_per_row + sizeof( type )\
                  );\
\
	dest_slice = dest_pixels;\
\
	if( dest_pixels == src_pixels )\
		{\
		dest_slice = NS_OFFSET_POINTER( type, dest_slice, region->z * bytes_per_slice );\
		dest_slice = NS_OFFSET_POINTER( type, dest_slice, region->y * bytes_per_row );\
		dest_slice = NS_OFFSET_POINTER( type, dest_slice, region->x * sizeof( type ) );\
		}\
\
   dest_slice = NS_OFFSET_POINTER(\
                  type,\
                  dest_slice,\
                  bytes_per_slice + bytes_per_row + sizeof( type )\
                  );\
\
   if( NULL != progress )\
      {\
      ns_progress_set_title( progress, _ns_pixel_proc_blur.title );\
      ns_progress_update( progress, NS_PROGRESS_BEGIN );\
      }\
\
   for( z = 1; z < l; ++z )\
      {\
      if( NULL != progress )\
         {\
         if( ns_progress_cancelled( progress ) )\
            return;\
\
         ns_progress_update( progress, 100.0f * ( z / ( nsfloat )region->length ) );\
         }\
\
      src_row  = src_slice;\
      dest_row = dest_slice;\
\
      for( y = 1; y < h; ++y )\
         {\
         src_pixel  = src_row;\
         dest_pixel = dest_row;\
\
         for( x = 1; x < w; ++x )\
            {\
            func;\
            ++src_pixel;\
            ++dest_pixel;\
            }\
\
         src_row  = NS_OFFSET_POINTER( const type, src_row, bytes_per_row );\
         dest_row = NS_OFFSET_POINTER( type, dest_row, bytes_per_row );\
         }\
\
      src_slice  = NS_OFFSET_POINTER( const type, src_slice, bytes_per_slice );\
      dest_slice = NS_OFFSET_POINTER( type, dest_slice, bytes_per_slice );\
      }\
\
   if( NULL != progress )\
      ns_progress_update( progress, NS_PROGRESS_END )


/* NOTE: For speed only doing the 6 voxel faces. */

#define _NS_PIXEL_PROC_BLUR_LUM_U8\
   nsfloat32  f32;\
   nsint32    i32;\
   nsfloat64  f64;\
   nsulong    sum = ( nsulong )( src_pixel->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU8, src_pixel,  bytes_per_slice ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU8, src_pixel, -bytes_per_slice ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU8, src_pixel,  bytes_per_row   ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU8, src_pixel, -bytes_per_row   ) )->luminance );\
   sum += ( nsulong )( ( src_pixel + 1                                                  )->luminance );\
   sum += ( nsulong )( ( src_pixel - 1                                                  )->luminance );\
   f32 = ( nsfloat )sum / 7.0f;\
   NS_FLOAT32_TO_INT32_WITH_PROPER_ROUNDING( f32, i32, f64 );\
   dest_pixel->luminance = ( nsuint8 )i32


NS_PRIVATE void _ns_pixel_proc_blur_lum_u8
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
   )
   {
   _NS_PIXEL_PROC_BLUR_TEMPLATE(
      NsPixelLumU8,
      _NS_PIXEL_PROC_BLUR_LUM_U8
      );
   }


/* NOTE: For speed only doing the 6 voxel faces. */
#define _NS_PIXEL_PROC_BLUR_LUM_U12_OR_LUM_U16\
   nsfloat32  f32;\
   nsint32    i32;\
   nsfloat64  f64;\
   nsulong    sum = ( nsulong )( src_pixel->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU16, src_pixel,  bytes_per_slice ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU16, src_pixel, -bytes_per_slice ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU16, src_pixel,  bytes_per_row   ) )->luminance );\
   sum += ( nsulong )( ( NS_OFFSET_POINTER( NsPixelLumU16, src_pixel, -bytes_per_row   ) )->luminance );\
   sum += ( nsulong )( ( src_pixel + 1                                                   )->luminance );\
   sum += ( nsulong )( ( src_pixel - 1                                                   )->luminance );\
   f32 = ( nsfloat )sum / 7.0f;\
   NS_FLOAT32_TO_INT32_WITH_PROPER_ROUNDING( f32, i32, f64 );\
   dest_pixel->luminance = ( nsuint16 )i32

NS_PRIVATE void _ns_pixel_proc_blur_lum_u12_or_u16
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
   )
   {
   _NS_PIXEL_PROC_BLUR_TEMPLATE(
      NsPixelLumU16,
      _NS_PIXEL_PROC_BLUR_LUM_U12_OR_LUM_U16
      );
   }


NS_PRIVATE _NsPixelProcBlurFunc _ns_pixel_proc_blur_func( NsPixelType pixel_type, nssize length )
   {
   _NsPixelProcBlurFunc func = NULL;

   if( 1 == length )
      switch( pixel_type )
         {
         case NS_PIXEL_LUM_U1:
         case NS_PIXEL_LUM_U8:
         case NS_PIXEL_LUM_U12:
         case NS_PIXEL_LUM_U16:
         case NS_PIXEL_LUM_F32:
         case NS_PIXEL_LUM_ALPHA_U8_U8:
         case NS_PIXEL_RGB_U8_U8_U8:
         case NS_PIXEL_RGBA_U8_U8_U8_U8:
         case NS_PIXEL_BGR_U8_U8_U8:
         case NS_PIXEL_BGRA_U8_U8_U8_U8:
            func = _ns_pixel_proc_blur_all_2d;
            break;
         }
   else
      switch( pixel_type )
         {
         case NS_PIXEL_LUM_U8:
            func = _ns_pixel_proc_blur_lum_u8;
            break;

         case NS_PIXEL_LUM_U12:
         case NS_PIXEL_LUM_U16:
            func = _ns_pixel_proc_blur_lum_u12_or_u16;
            break;
         }

   if( NULL == func )
      ns_warning(
         NS_WARNING_LEVEL_RECOVERABLE
         NS_MODULE
         " Pixel format "
         NS_FMT_STRING_QUOTED
         " not supported for procedure "
         NS_FMT_STRING_QUOTED
         ".",
         ns_pixel_type_to_string( pixel_type ),
         _ns_pixel_proc_blur.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_blur_call
   (
   NsPixelType           pixel_type,
   nsconstpointer        src_pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nspointer             dest_pixels,
   const NsPixelRegion  *sub_region,
   NsProgress           *progress
   )
   {
   _NsPixelProcBlurFunc   func;
   NsPixelRegion          full_region;
   const NsPixelRegion   *region;


   if( NULL == ( func = _ns_pixel_proc_blur_func( pixel_type, length ) ) )
      return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

   ns_assert( NULL != src_pixels );
   ns_assert( NULL != dest_pixels );
   ns_assert( 0 < width );
   ns_assert( 0 < height );
   ns_assert( 0 < length );

   region = ns_pixel_region_extract( sub_region, &full_region, width, height, length );
   ns_pixel_region_verify( region, width, height, length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", src_pixels=" NS_FMT_POINTER
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", row_align=" NS_FMT_ULONG
		", dest_pixels=" NS_FMT_POINTER
		", region={x:" NS_FMT_ULONG
			",y:" NS_FMT_ULONG
			",z:" NS_FMT_ULONG
			",width:" NS_FMT_ULONG
			",height:" NS_FMT_ULONG
			",length:" NS_FMT_ULONG
			"}"
		", progress=" NS_FMT_POINTER
		" )",
		ns_pixel_type_to_string( pixel_type ),
		src_pixels,
		width,
		height,
		length,
		row_align,
		dest_pixels,
		region->x, region->y, region->z, region->width, region->height, region->length,
		progress
		);

   ( func )(
      pixel_type,
      src_pixels,
      width,
      height,
      length,
      row_align,
      dest_pixels,
      region,
      progress
      );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_pixel_proc_blur_marshal
   (
   NsClosure  *closure,
   NsValue    *params,
   nssize      num_params,
   NsValue    *ret_value
   )
   {
   NS_USE_VARIABLE( closure );
   NS_USE_VARIABLE( num_params );
   NS_USE_VARIABLE( ret_value );

   #ifdef NS_DEBUG
	/*
   ns_println( NS_FMT_STRING, _ns_pixel_proc_blur.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_blur.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_blur.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_blur.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_blur.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_BLUR_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_SRC_PIXELS ].name,
      ns_value_get_constpointer( params + NS_PIXEL_PROC_BLUR_SRC_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_WIDTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BLUR_WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_HEIGHT ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BLUR_HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_LENGTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BLUR_LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_ROW_ALIGN ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BLUR_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_DEST_PIXELS ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_BLUR_DEST_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_REGION ].name,
      ns_value_get_pixel_region( params + NS_PIXEL_PROC_BLUR_REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_blur_params[ NS_PIXEL_PROC_BLUR_PROGRESS ].name,
      ns_value_get_progress( params + NS_PIXEL_PROC_BLUR_PROGRESS )
      );
	*/
   #endif/* NS_DEBUG */

   return _ns_pixel_proc_blur_call(
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_BLUR_PIXEL_TYPE ),
            ns_value_get_constpointer( params + NS_PIXEL_PROC_BLUR_SRC_PIXELS ),
            ns_value_get_size( params + NS_PIXEL_PROC_BLUR_WIDTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_BLUR_HEIGHT ),
            ns_value_get_size( params + NS_PIXEL_PROC_BLUR_LENGTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_BLUR_ROW_ALIGN ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_BLUR_DEST_PIXELS ),
            ns_value_get_pixel_region( params + NS_PIXEL_PROC_BLUR_REGION ),
            ns_value_get_progress( params + NS_PIXEL_PROC_BLUR_PROGRESS )
            );
   }


NsProc* ns_pixel_proc_blur( void )
   {
   _ns_pixel_proc_blur.name        = NS_PIXEL_PROC_BLUR;
   _ns_pixel_proc_blur.title       = "Blur";
   _ns_pixel_proc_blur.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_blur.params      = _ns_pixel_proc_blur_params;
   _ns_pixel_proc_blur.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_blur_params );
   _ns_pixel_proc_blur.marshal     = _ns_pixel_proc_blur_marshal;

   _ns_pixel_proc_blur.ret_value.name = NULL;
   _ns_pixel_proc_blur.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_blur;
   }



