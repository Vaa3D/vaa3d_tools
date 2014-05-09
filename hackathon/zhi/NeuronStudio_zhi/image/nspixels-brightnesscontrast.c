#include "nspixels-brightnesscontrast.h"


NS_PRIVATE NsProc _ns_pixel_proc_brightness_contrast;


NS_PRIVATE NsClosureValue _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_NUM_PARAMS ] =
   {
   { "pixel_type", NS_VALUE_PIXEL_TYPE },
   { "pixels",     NS_VALUE_POINTER },
   { "width",      NS_VALUE_SIZE },
   { "height",     NS_VALUE_SIZE },
   { "length",     NS_VALUE_SIZE },
   { "row_align",  NS_VALUE_SIZE },
   { "brightness", NS_VALUE_DOUBLE },
   { "contrast",   NS_VALUE_DOUBLE },
   { "region",     NS_VALUE_PIXEL_REGION },
   { "progress",   NS_VALUE_PROGRESS }
   };


typedef NsError ( *_NsPixelProcBrightnessContrastFunc )
   (
   NsPixelType,
   nspointer,
   nssize,
   nssize,
   nssize,
   nssize,
   nsdouble,
   nsdouble,
   const NsPixelRegion*,
   NsProgress*
   );


NS_PRIVATE void _ns_pixel_proc_brightness_contrast_lut
   (
   NsPixelLut  *lut,
   nsdouble     brightness,
   nsdouble     contrast
   )
   {
   nsdouble  value;
   nsdouble  nvalue;
   nsdouble  power;
   nsuint    i;
   nsuint    umax;
   nsdouble  fmax;


   umax = ( nsuint )( lut->size - 1 );
   fmax = ( nsdouble )umax;

   for( i = 0; i <= umax; ++i )
      {
      value = ( nsdouble )i / fmax;

      if( brightness < 0.0 )
         value = value * ( 1.0 + brightness );
      else
         value = value + ( ( 1.0 - value ) * brightness );

      if( contrast < 0.0 )
         {
         if( value > 0.5 )
            nvalue = 1.0 - value;
         else
            nvalue = value;

         if( nvalue < 0.0 )
            nvalue = 0.0;

         nvalue = 0.5 * ns_pow( nvalue * 2.0, 1.0 + contrast );

         if( value > 0.5 )
            value = 1.0 - nvalue;
         else
            value = nvalue;
         }
      else
         {
         if( value > 0.5 )
            nvalue = 1.0 - value;
         else
            nvalue = value;

         if( nvalue < 0.0 )
            nvalue = 0.0;

         power = ( contrast == 1.0 ) ?
                 127.0 : 1.0 / ( 1.0 - contrast );

         nvalue = 0.5 * ns_pow( 2.0 * nvalue, power );

         if( value > 0.5 )
            value = 1.0 - nvalue;
         else
            value = nvalue;
         }

      value = ( fmax * value + 0.5 );

      if( value < 0.0 )
         value = 0.0;
      else if( value > fmax )
         value = fmax;

      ns_pixel_lut_set_value( lut, i, ( nsuint )value );
      }
   }


NS_PRIVATE NsError _ns_pixel_proc_brightness_contrast_all
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
   )
   {
   NsPixelInfo                    info;
   const NsPixelChannel          *channel;
   nssize                         x;
   nssize                         y;
   nssize                         z;
   nssize                         bytes_per_row;
   nssize                         bytes_per_slice;
   nspointer                      slice;
   nspointer                      row;
   nsuint                         value;
   NsPixelRowGetUintChannelFunc   row_get_uint_channel_funcs[ NS_PIXEL_NUM_CHANNELS ];
   NsPixelRowSetUintChannelFunc   row_set_uint_channel_funcs[ NS_PIXEL_NUM_CHANNELS ];
   NsPixelLut                     luts[ NS_PIXEL_NUM_CHANNELS ];
   NsError                        error;


   NS_USE_VARIABLE( length );

   error = ns_no_error();

   ns_pixel_info( &info, pixel_type );

   for( channel = info.channel; NULL != channel; channel = channel->next )
      {
      row_get_uint_channel_funcs[ channel->index ] =
         ns_pixel_row_get_uint_channel_func( pixel_type, channel->index );

      row_set_uint_channel_funcs[ channel->index ] =
         ns_pixel_row_set_uint_channel_func( pixel_type, channel->index );

      ns_assert( NULL != row_get_uint_channel_funcs[ channel->index ] );
      ns_assert( NULL != row_set_uint_channel_funcs[ channel->index ] );

      luts[ channel->index ].values = NULL;
      }

   /* NOTE: Dont affect the alpha channel, if there is one. */
   for( channel = info.channel; NULL != channel; channel = channel->next )
      if( ! ns_pixel_channel_is( channel, NS_PIXEL_CHANNEL_ALPHA_NAME ) )
         {
         if( NS_FAILURE( ns_pixel_lut_init( luts + channel->index, channel ), error ) )
            goto _NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_ALL_EXIT;

         _ns_pixel_proc_brightness_contrast_lut( luts + channel->index, brightness, contrast );
         }

   bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
   bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

   slice = pixels;
   slice = NS_OFFSET_POINTER( void, slice, region->z * bytes_per_slice );
   slice = NS_OFFSET_POINTER( void, slice, region->y * bytes_per_row );

   if( NULL != progress )
      {
      ns_progress_set_title( progress, _ns_pixel_proc_brightness_contrast.title );
      ns_progress_update( progress, NS_PROGRESS_BEGIN );
      }

   for( z = 0; z < region->length; ++z )
      {
      if( NULL != progress )
         {
         if( ns_progress_cancelled( progress ) )
            break;

         ns_progress_update(
            progress,
            100.0f * ( ( nsfloat )z / ( nsfloat )region->length )
            );
         }

      row = slice;

      for( y = 0; y < region->height; ++y )
         {
         for( x = 0; x < region->width; ++x )
            for( channel = info.channel; NULL != channel; channel = channel->next )
               if( NULL != luts[ channel->index ].values )
                  {
                  value = ( row_get_uint_channel_funcs[ channel->index ] )( row, region->x + x );
                  value = ns_pixel_lut_get_value( luts + channel->index, value );
                  ( row_set_uint_channel_funcs[ channel->index ] )( row, region->x + x, value );
                  }

         row = NS_OFFSET_POINTER( void, row, bytes_per_row );
         }

      slice = NS_OFFSET_POINTER( void, slice, bytes_per_slice );
      }

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_END );

   _NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_ALL_EXIT:

   for( channel = info.channel; NULL != channel; channel = channel->next )
      ns_pixel_lut_finalize( luts + channel->index );

   return error;
   }


NS_PRIVATE _NsPixelProcBrightnessContrastFunc _ns_pixel_proc_brightness_contrast_func( NsPixelType pixel_type )
   {
   _NsPixelProcBrightnessContrastFunc func = NULL;

   switch( pixel_type )
      {
      case NS_PIXEL_LUM_U1:
      case NS_PIXEL_LUM_U8:
      case NS_PIXEL_LUM_U12:
      case NS_PIXEL_LUM_U16:
      case NS_PIXEL_LUM_ALPHA_U8_U8:
      case NS_PIXEL_RGB_U8_U8_U8:
      case NS_PIXEL_RGBA_U8_U8_U8_U8:
      case NS_PIXEL_BGR_U8_U8_U8:
      case NS_PIXEL_BGRA_U8_U8_U8_U8:
         func = _ns_pixel_proc_brightness_contrast_all;
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
         _ns_pixel_proc_brightness_contrast.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_brightness_contrast_call
   (
   NsPixelType           pixel_type,
   nspointer             pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsdouble              brightness,
   nsdouble              contrast,
   const NsPixelRegion  *sub_region,
   NsProgress           *progress
   )
   {
   _NsPixelProcBrightnessContrastFunc   func;
   NsPixelRegion                        full_region;
   const NsPixelRegion                 *region;


   if( NULL == ( func = _ns_pixel_proc_brightness_contrast_func( pixel_type ) ) )
      return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

   ns_assert( NULL != pixels );
   ns_assert( 0 < width );
   ns_assert( 0 < height );
   ns_assert( 0 < length );
   ns_assert( NS_BRIGHTNESS_MIN <= brightness && brightness <= NS_BRIGHTNESS_MAX );
   ns_assert( NS_CONTRAST_MIN <= contrast && contrast <= NS_CONTRAST_MAX );

   region = ns_pixel_region_extract( sub_region, &full_region, width, height, length );
   ns_pixel_region_verify( region, width, height, length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", pixels=" NS_FMT_POINTER
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", row_align=" NS_FMT_ULONG
		", brightness=" NS_FMT_DOUBLE
		", contrast=" NS_FMT_DOUBLE
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
		pixels,
		width,
		height,
		length,
		row_align,
		brightness,
		contrast,
		region->x, region->y, region->z, region->width, region->height, region->length,
		progress
		);

   return ( func )(
            pixel_type,
            pixels,
            width,
            height,
            length,
            row_align,
            brightness,
            contrast,
            region,
            progress
            );
   }


NS_PRIVATE NsError _ns_pixel_proc_brightness_contrast_marshal
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

	/*
   #ifdef NS_DEBUG
	
   ns_println( NS_FMT_STRING, _ns_pixel_proc_brightness_contrast.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_brightness_contrast.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_brightness_contrast.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_brightness_contrast.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_brightness_contrast.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXELS ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_WIDTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_HEIGHT ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_LENGTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_ROW_ALIGN ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_BRIGHTNESS ].name,
      ns_value_get_double( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_BRIGHTNESS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_CONTRAST ].name,
      ns_value_get_double( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_CONTRAST )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_REGION ].name,
      ns_value_get_pixel_region( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_brightness_contrast_params[ NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PROGRESS ].name,
      ns_value_get_progress( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PROGRESS )
      );

   #endif/* NS_DEBUG */

   return _ns_pixel_proc_brightness_contrast_call(
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXEL_TYPE ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PIXELS ),
            ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_WIDTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_HEIGHT ),
            ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_LENGTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_ROW_ALIGN ),
            ns_value_get_double( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_BRIGHTNESS ),
            ns_value_get_double( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_CONTRAST ),
            ns_value_get_pixel_region( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_REGION ),
            ns_value_get_progress( params + NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_PROGRESS )
            );
   }


NsProc* ns_pixel_proc_brightness_contrast( void )
   {
   _ns_pixel_proc_brightness_contrast.name        = NS_PIXEL_PROC_BRIGHTNESS_CONTRAST;
   _ns_pixel_proc_brightness_contrast.title       = "Brightness/Contrast";
   _ns_pixel_proc_brightness_contrast.version     = "1.0";
   _ns_pixel_proc_brightness_contrast.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_brightness_contrast.params      = _ns_pixel_proc_brightness_contrast_params;
   _ns_pixel_proc_brightness_contrast.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_brightness_contrast_params );
   _ns_pixel_proc_brightness_contrast.marshal     = _ns_pixel_proc_brightness_contrast_marshal;

   _ns_pixel_proc_brightness_contrast.ret_value.name = NULL;
   _ns_pixel_proc_brightness_contrast.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_brightness_contrast;
   }



