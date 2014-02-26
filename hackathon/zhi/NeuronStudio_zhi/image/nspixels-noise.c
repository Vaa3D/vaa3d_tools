#include "nspixels-noise.h"


NS_PRIVATE NsProc _ns_pixel_proc_noise;


NS_PRIVATE NsClosureValue _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_NUM_PARAMS ] =
   {
   { "pixel_type",            NS_VALUE_PIXEL_TYPE },
   { "pixels",                NS_VALUE_POINTER },
   { "width",                 NS_VALUE_SIZE },
   { "height",                NS_VALUE_SIZE },
   { "length",                NS_VALUE_SIZE },
   { "row_align",             NS_VALUE_SIZE },
   { "level",                 NS_VALUE_DOUBLE },
   { "iterations",            NS_VALUE_SIZE },
   { "average_intensity",     NS_VALUE_DOUBLE },
   { "signal_to_noise_ratio", NS_VALUE_PDOUBLE },
   { "region",                NS_VALUE_PIXEL_REGION },
   { "progress",              NS_VALUE_PROGRESS }
   };


typedef NsError ( *_NsPixelProcNoiseFunc )
   (
   NsPixelType,
   nspointer,
   nssize,
   nssize,
   nssize,
   nssize,
   nsdouble,
   nssize,
   nsdouble,
   nsdouble*,
   const NsPixelRegion*,
   NsProgress*
   );


#include <ext/noise.inl>


NS_PRIVATE NsError _ns_pixel_proc_noise_lum_all_rescaled
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
   )
   {
   NsPixelInfo                   info;
   nssize                        lum_bits;
   nssize                        x;
   nssize                        y;
   nssize                        z;
   nssize                        bytes_per_row;
   nssize                        bytes_per_slice;
   nspointer                     slice;
   nspointer                     row;
   nsdouble                      value;
   nsdouble                      noise;
   NsPixelRowGetUintChannelFunc  row_get_uint_channel_func;
   NsPixelRowSetUintChannelFunc  row_set_uint_channel_func;
   nsdouble                      signal_range_min;
   nsdouble                      signal_range_max;
   nsdouble                      noise_range_min;
   nsdouble                      noise_range_max;
   nsdouble                      noise_to_signal_slope;
   nsdouble                      noise_to_signal_intercept;
   nsdouble                      sum_signal;
   nsdouble                      sum_noise;


   NS_USE_VARIABLE( length );

   ns_pixel_info( &info, pixel_type );
   lum_bits = info.channels[ NS_PIXEL_CHANNEL_LUM ].bits;

   signal_range_min = 0.0;
   signal_range_max = ( nsdouble )( ( ( nssize )1 << lum_bits ) - 1 );

   noise_range_min = signal_range_min - level;
   noise_range_max = signal_range_max + level;

   noise_to_signal_slope =
      ( signal_range_max - signal_range_min ) / ( noise_range_max - noise_range_min );

   noise_to_signal_intercept =
      signal_range_min - noise_to_signal_slope * noise_range_min;

   row_get_uint_channel_func =
      ns_pixel_row_get_uint_channel_func( pixel_type, NS_PIXEL_CHANNEL_LUM );

   row_set_uint_channel_func =
      ns_pixel_row_set_uint_channel_func( pixel_type, NS_PIXEL_CHANNEL_LUM );

   ns_assert( NULL != row_get_uint_channel_func );
   ns_assert( NULL != row_set_uint_channel_func );

   bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
   bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

   slice = pixels;
   slice = NS_OFFSET_POINTER( void, slice, region->z * bytes_per_slice );
   slice = NS_OFFSET_POINTER( void, slice, region->y * bytes_per_row );

   if( NULL != progress )
      {
      ns_progress_set_title( progress, _ns_pixel_proc_noise.title );
      ns_progress_update( progress, NS_PROGRESS_BEGIN );
      }

   sum_signal = 0.0;
   sum_noise  = 0.0;

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
            {
            value = ( nsdouble )( row_get_uint_channel_func )( row, region->x + x );
            noise = GenerateNoise( ( nsint )iterations, level );

            if( noise < -level )
               noise = -level;
            else if( level < noise )
               noise = level;

            sum_signal += NS_POW2( value - average_intensity );
            sum_noise  += NS_POW2( noise );

            value += noise;
            value = noise_to_signal_slope * value + noise_to_signal_intercept;

            ( row_set_uint_channel_func )( row, region->x + x, ( nsuint )( value + 0.5 ) );
            }

         row = NS_OFFSET_POINTER( void, row, bytes_per_row );
         }

      slice = NS_OFFSET_POINTER( void, slice, bytes_per_slice );
      }

   *signal_to_noise_ratio = 10.0 * ns_log10( sum_signal / sum_noise );

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_END );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_pixel_proc_noise_lum_all_clamped
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
   )
   {
   NsPixelInfo                   info;
   nssize                        lum_bits;
   nssize                        x;
   nssize                        y;
   nssize                        z;
   nssize                        bytes_per_row;
   nssize                        bytes_per_slice;
   nspointer                     slice;
   nspointer                     row;
   nsdouble                      value;
   nsdouble                      noise;
   NsPixelRowGetUintChannelFunc  row_get_uint_channel_func;
   NsPixelRowSetUintChannelFunc  row_set_uint_channel_func;
   nsdouble                      signal_range_min;
   nsdouble                      signal_range_max;
   nsdouble                      sum_signal;
   nsdouble                      sum_noise;


   NS_USE_VARIABLE( length );

   ns_pixel_info( &info, pixel_type );
   lum_bits = info.channels[ NS_PIXEL_CHANNEL_LUM ].bits;

   signal_range_min = 0.0;
   signal_range_max = ( nsdouble )( ( ( nssize )1 << lum_bits ) - 1 );

   row_get_uint_channel_func =
      ns_pixel_row_get_uint_channel_func( pixel_type, NS_PIXEL_CHANNEL_LUM );

   row_set_uint_channel_func =
      ns_pixel_row_set_uint_channel_func( pixel_type, NS_PIXEL_CHANNEL_LUM );

   ns_assert( NULL != row_get_uint_channel_func );
   ns_assert( NULL != row_set_uint_channel_func );

   bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );
   bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );

   slice = pixels;
   slice = NS_OFFSET_POINTER( void, slice, region->z * bytes_per_slice );
   slice = NS_OFFSET_POINTER( void, slice, region->y * bytes_per_row );

   if( NULL != progress )
      {
      ns_progress_set_title( progress, _ns_pixel_proc_noise.title );
      ns_progress_update( progress, NS_PROGRESS_BEGIN );
      }

   sum_signal = 0.0;
   sum_noise  = 0.0;

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
            {
            value = ( nsdouble )( row_get_uint_channel_func )( row, region->x + x );
            noise = GenerateNoise( ( nsint )iterations, level );

            if( value + noise < signal_range_min )
               {
               ns_assert( noise <= 0.0 );
               noise += ( signal_range_min - ( value + noise ) );
               }
            else if( signal_range_max < value + noise )
               {
               ns_assert( 0.0 <= noise );
               noise -= ( ( value + noise ) - signal_range_max );
               }

            sum_signal += NS_POW2( value - average_intensity );
            sum_noise  += NS_POW2( noise );

            value += noise;

            ( row_set_uint_channel_func )( row, region->x + x, ( nsuint )( value + 0.5 ) );
            }

         row = NS_OFFSET_POINTER( void, row, bytes_per_row );
         }

      slice = NS_OFFSET_POINTER( void, slice, bytes_per_slice );
      }

   *signal_to_noise_ratio = 10.0 * ns_log10( sum_signal / sum_noise );

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_END );

   return ns_no_error();
   }


NS_PRIVATE _NsPixelProcNoiseFunc _ns_pixel_proc_noise_func( NsPixelType pixel_type )
   {
   _NsPixelProcNoiseFunc func = NULL;

   switch( pixel_type )
      {
      case NS_PIXEL_LUM_U1:
      case NS_PIXEL_LUM_U8:
      case NS_PIXEL_LUM_U12:
      case NS_PIXEL_LUM_U16:
      case NS_PIXEL_LUM_ALPHA_U8_U8:
         func = _ns_pixel_proc_noise_lum_all_clamped;
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
         _ns_pixel_proc_noise.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_noise_call
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
   const NsPixelRegion  *sub_region,
   NsProgress           *progress
   )
   {
   _NsPixelProcNoiseFunc   func;
   NsPixelRegion           full_region;
   const NsPixelRegion    *region;


   if( NULL == ( func = _ns_pixel_proc_noise_func( pixel_type ) ) )
      return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

   ns_assert( NULL != pixels );
   ns_assert( 0 < width );
   ns_assert( 0 < height );
   ns_assert( 0 < length );

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
		", level=" NS_FMT_DOUBLE
		", iterations=" NS_FMT_ULONG
		", average_intensity=" NS_FMT_DOUBLE
		", signal_to_noise_ratio=" NS_FMT_POINTER
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
		level,
		iterations,
		average_intensity,
		signal_to_noise_ratio,
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
            level,
            iterations,
            average_intensity,
            signal_to_noise_ratio,
            region,
            progress
            );
   }


NS_PRIVATE NsError _ns_pixel_proc_noise_marshal
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

   ns_println( NS_FMT_STRING, _ns_pixel_proc_noise.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_noise.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_noise.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_noise.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_noise.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_NOISE_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_PIXELS ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_NOISE_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_WIDTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_NOISE_WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_HEIGHT ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_NOISE_HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_LENGTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_NOISE_LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_ROW_ALIGN ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_NOISE_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_LEVEL ].name,
      ns_value_get_double( params + NS_PIXEL_PROC_NOISE_LEVEL )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_ITERATIONS ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_NOISE_ITERATIONS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_AVERAGE_INTENSITY ].name,
      ns_value_get_double( params + NS_PIXEL_PROC_NOISE_AVERAGE_INTENSITY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_SIGNAL_TO_NOISE_RATIO ].name,
      ns_value_get_pdouble( params + NS_PIXEL_PROC_NOISE_SIGNAL_TO_NOISE_RATIO )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_REGION ].name,
      ns_value_get_pixel_region( params + NS_PIXEL_PROC_NOISE_REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_noise_params[ NS_PIXEL_PROC_NOISE_PROGRESS ].name,
      ns_value_get_progress( params + NS_PIXEL_PROC_NOISE_PROGRESS )
      );

   #endif/* NS_DEBUG */

   return _ns_pixel_proc_noise_call(
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_NOISE_PIXEL_TYPE ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_NOISE_PIXELS ),
            ns_value_get_size( params + NS_PIXEL_PROC_NOISE_WIDTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_NOISE_HEIGHT ),
            ns_value_get_size( params + NS_PIXEL_PROC_NOISE_LENGTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_NOISE_ROW_ALIGN ),
            ns_value_get_double( params + NS_PIXEL_PROC_NOISE_LEVEL ),
            ns_value_get_size( params + NS_PIXEL_PROC_NOISE_ITERATIONS ),
            ns_value_get_double( params + NS_PIXEL_PROC_NOISE_AVERAGE_INTENSITY ),
            ns_value_get_pdouble( params + NS_PIXEL_PROC_NOISE_SIGNAL_TO_NOISE_RATIO ),
            ns_value_get_pixel_region( params + NS_PIXEL_PROC_NOISE_REGION ),
            ns_value_get_progress( params + NS_PIXEL_PROC_NOISE_PROGRESS )
            );
   }


NsProc* ns_pixel_proc_noise( void )
   {
   _ns_pixel_proc_noise.name        = NS_PIXEL_PROC_NOISE;
   _ns_pixel_proc_noise.title       = "Noise";
   _ns_pixel_proc_noise.version     = "1.0";
   _ns_pixel_proc_noise.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_noise.params      = _ns_pixel_proc_noise_params;
   _ns_pixel_proc_noise.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_noise_params );
   _ns_pixel_proc_noise.marshal     = _ns_pixel_proc_noise_marshal;

   _ns_pixel_proc_noise.ret_value.name = NULL;
   _ns_pixel_proc_noise.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_noise;
   }



