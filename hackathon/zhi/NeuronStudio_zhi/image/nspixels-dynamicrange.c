#include "nspixels-dynamicrange.h"


NS_PRIVATE NsProc _ns_pixel_proc_dynamic_range;


NS_PRIVATE NsClosureValue _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_NUM_PARAMS ] =
   {
   { "pixel_type",  NS_VALUE_PIXEL_TYPE },
   { "pixels",      NS_VALUE_POINTER },
   { "width",       NS_VALUE_SIZE },
   { "height",      NS_VALUE_SIZE },
   { "length",      NS_VALUE_SIZE },
   { "row_align",   NS_VALUE_SIZE },
   { "channel_min", NS_VALUE_FLOAT },
   { "channel_max", NS_VALUE_FLOAT },
   { "region",      NS_VALUE_PIXEL_REGION },
   { "progress",    NS_VALUE_PROGRESS }
   };


typedef void ( *_NsPixelProcDynamicRangeFunc )
   (
   NsPixelType,
   nspointer,
   nssize,
   nssize,
   nssize,
   nssize,
   nsfloat,
   nsfloat,
   const NsPixelRegion*,
   NsProgress*
   );


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_TEMPLATE(\
   type,\
   init_min_max_func,\
   do_min_max_func,\
   init_lerp_func,\
   do_lerp_func\
   )\
   nssize    x;\
   nssize    y;\
   nssize    z;\
   nssize    bytes_per_row;\
   nssize    bytes_per_slice;\
   type     *slice;\
   type     *row;\
   type     *pixel;\
   type      min;\
   type      max;\
   nsfloat    slope[ NS_PIXEL_NUM_CHANNELS ];\
   nsfloat   intercept[ NS_PIXEL_NUM_CHANNELS ];\
\
   NS_USE_VARIABLE( length );\
\
   bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, width, row_align );\
   bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, width, height, row_align );\
\
   if( NULL != progress )\
      {\
      ns_progress_set_title( progress, _ns_pixel_proc_dynamic_range.title );\
      ns_progress_update( progress, NS_PROGRESS_BEGIN );\
      }\
\
   init_min_max_func;\
\
   slice = pixels;\
   slice = NS_OFFSET_POINTER( type, slice, region->z * bytes_per_slice );\
   slice = NS_OFFSET_POINTER( type, slice, region->y * bytes_per_row );\
   slice = NS_OFFSET_POINTER( type, slice, region->x * sizeof( type ) );\
\
   for( z = 0; z < region->length; ++z )\
      {\
      if( NULL != progress )\
         {\
         if( ns_progress_cancelled( progress ) )\
            return;\
\
         ns_progress_update(\
            progress,\
            100.0f * ( ( nsfloat )z / ( nsfloat )region->length )\
            );\
         }\
\
      row = slice;\
\
      for( y = 0; y < region->height; ++y )\
         {\
         pixel = row;\
\
         for( x = 0; x < region->width; ++x )\
            {\
            do_min_max_func;\
\
            ++pixel;\
            }\
\
         row = NS_OFFSET_POINTER( type, row, bytes_per_row );\
         }\
\
      slice = NS_OFFSET_POINTER( type, slice, bytes_per_slice );\
      }\
\
   init_lerp_func;\
\
   slice = pixels;\
   slice = NS_OFFSET_POINTER( type, slice, region->z * bytes_per_slice );\
   slice = NS_OFFSET_POINTER( type, slice, region->y * bytes_per_row );\
   slice = NS_OFFSET_POINTER( type, slice, region->x * sizeof( type ) );\
\
   for( z = 0; z < region->length; ++z )\
      {\
      if( NULL != progress )\
         {\
         if( ns_progress_cancelled( progress ) )\
            return;\
\
         ns_progress_update(\
            progress,\
            100.0f * ( ( nsfloat )z / ( nsfloat )region->length )\
            );\
         }\
\
      row = slice;\
\
      for( y = 0; y < region->height; ++y )\
         {\
         pixel = row;\
\
         for( x = 0; x < region->width; ++x )\
            {\
            do_lerp_func;\
\
            ++pixel;\
            }\
\
         row = NS_OFFSET_POINTER( type, row, bytes_per_row );\
         }\
\
      slice = NS_OFFSET_POINTER( type, slice, bytes_per_slice );\
      }\
\
   if( NULL != progress )\
      ns_progress_update( progress, NS_PROGRESS_END )


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_MIN_MAX_FUNC_LUM\
   if( pixel->luminance < min.luminance )\
      min.luminance = pixel->luminance;\
   if( pixel->luminance > max.luminance )\
      max.luminance = pixel->luminance


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_LERP_FUNC_LUM\
   slope[ NS_PIXEL_CHANNEL_LUM ] =\
      ( channel_max - channel_min )\
      /\
      ( nsfloat )( max.luminance - min.luminance );\
   \
   intercept[ NS_PIXEL_CHANNEL_LUM ] =\
      channel_min - slope[ NS_PIXEL_CHANNEL_LUM ] * ( nsfloat )min.luminance


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_U8\
   min.luminance = NS_UINT8_MAX;\
   max.luminance = 0


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_U8\
   pixel->luminance =\
      ( nsuint8 )\
      (\
      slope[ NS_PIXEL_CHANNEL_LUM ] *\
      ( nsfloat )pixel->luminance +\
      intercept[ NS_PIXEL_CHANNEL_LUM ] +\
      .5f\
      )

NS_PRIVATE void _ns_pixel_proc_dynamic_range_lum_u8
   (
   NsPixelType           pixel_type,
   NsPixelLumU8         *pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsfloat               channel_min,
   nsfloat               channel_max,
   const NsPixelRegion  *region,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_DYNAMIC_RANGE_TEMPLATE(
      NsPixelLumU8,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_U8,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_MIN_MAX_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_LERP_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_U8
      );
   }


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_U16\
   min.luminance = NS_UINT16_MAX;\
   max.luminance = 0


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_U16\
   pixel->luminance =\
      ( nsuint16 )\
      (\
      slope[ NS_PIXEL_CHANNEL_LUM ] *\
      ( nsfloat )pixel->luminance +\
      intercept[ NS_PIXEL_CHANNEL_LUM ] +\
      .5f\
      )

NS_COMPILE_TIME_SIZE_EQUAL( NsPixelLumU12, NsPixelLumU16 );
NS_PRIVATE void _ns_pixel_proc_dynamic_range_lum_u12_or_lum_u16
   (
   NsPixelType           pixel_type,
   NsPixelLumU16        *pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsfloat               channel_min,
   nsfloat               channel_max,
   const NsPixelRegion  *region,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_DYNAMIC_RANGE_TEMPLATE(
      NsPixelLumU16,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_U16,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_MIN_MAX_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_LERP_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_U16
      );
   }


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_F32\
   min.luminance = NS_FLOAT32_MAX;\
   max.luminance = 0.0f


#define _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_F32\
   pixel->luminance =\
      ( nsfloat32 )\
      (\
      slope[ NS_PIXEL_CHANNEL_LUM ] *\
      ( nsfloat )pixel->luminance +\
      intercept[ NS_PIXEL_CHANNEL_LUM ] +\
      .5f\
      )

NS_PRIVATE void _ns_pixel_proc_dynamic_range_lum_f32
   (
   NsPixelType           pixel_type,
   NsPixelLumF32        *pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsfloat               channel_min,
   nsfloat               channel_max,
   const NsPixelRegion  *region,
   NsProgress           *progress
   )
   {
   _NS_PIXEL_PROC_DYNAMIC_RANGE_TEMPLATE(
      NsPixelLumF32,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_MIN_MAX_FUNC_LUM_F32,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_MIN_MAX_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_INIT_LERP_FUNC_LUM,
      _NS_PIXEL_PROC_DYNAMIC_RANGE_DO_LERP_FUNC_LUM_F32
      );
   }


NS_PRIVATE _NsPixelProcDynamicRangeFunc _ns_pixel_proc_dynamic_range_func( NsPixelType pixel_type )
   {
   _NsPixelProcDynamicRangeFunc func = NULL;

   switch( pixel_type )
      {
      case NS_PIXEL_LUM_U8:
         func = _ns_pixel_proc_dynamic_range_lum_u8;
         break;

      case NS_PIXEL_LUM_U12:
      case NS_PIXEL_LUM_U16:
         func = _ns_pixel_proc_dynamic_range_lum_u12_or_lum_u16;
         break;

      case NS_PIXEL_LUM_F32:
         func = _ns_pixel_proc_dynamic_range_lum_f32;
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
         _ns_pixel_proc_dynamic_range.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_dynamic_range_call
   (
   NsPixelType           pixel_type,
   nspointer             pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsfloat               channel_min,
   nsfloat               channel_max,
   const NsPixelRegion  *sub_region,
   NsProgress           *progress
   )
   {
   _NsPixelProcDynamicRangeFunc   func;
   NsPixelRegion                  full_region;
   const NsPixelRegion           *region;


   if( NULL == ( func = _ns_pixel_proc_dynamic_range_func( pixel_type ) ) )
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
		", channel_min=" NS_FMT_DOUBLE
		", channel_max=" NS_FMT_DOUBLE
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
		channel_min,
		channel_max,
		region->x, region->y, region->z, region->width, region->height, region->length,
		progress
		);

   ( func )(
      pixel_type,
      pixels,
      width,
      height,
      length,
      row_align,
      channel_min,
      channel_max,
      region,
      progress
      );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_pixel_proc_dynamic_range_marshal
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

   ns_println( NS_FMT_STRING, _ns_pixel_proc_dynamic_range.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_dynamic_range.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_dynamic_range.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_dynamic_range.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_dynamic_range.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_PIXELS ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_WIDTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_HEIGHT ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_LENGTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_ROW_ALIGN ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MIN ].name,
      ns_value_get_float( params + NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MIN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_DOUBLE,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MAX ].name,
      ns_value_get_float( params + NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MAX )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_REGION ].name,
      ns_value_get_pixel_region( params + NS_PIXEL_PROC_DYNAMIC_RANGE_REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_dynamic_range_params[ NS_PIXEL_PROC_DYNAMIC_RANGE_PROGRESS ].name,
      ns_value_get_progress( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PROGRESS )
      );

   #endif/* NS_DEBUG */

   return _ns_pixel_proc_dynamic_range_call(
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PIXEL_TYPE ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PIXELS ),
            ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_WIDTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_HEIGHT ),
            ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_LENGTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_DYNAMIC_RANGE_ROW_ALIGN ),
            ns_value_get_float( params + NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MIN ),
            ns_value_get_float( params + NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MAX ),
            ns_value_get_pixel_region( params + NS_PIXEL_PROC_DYNAMIC_RANGE_REGION ),
            ns_value_get_progress( params + NS_PIXEL_PROC_DYNAMIC_RANGE_PROGRESS )
            );
   }


NsProc* ns_pixel_proc_dynamic_range( void )
   {
   _ns_pixel_proc_dynamic_range.name        = NS_PIXEL_PROC_DYNAMIC_RANGE;
   _ns_pixel_proc_dynamic_range.title       = "Dynamic Range";
   _ns_pixel_proc_dynamic_range.version     = "1.0";
   _ns_pixel_proc_dynamic_range.help        = "Increase or decrease the dynamic range of an image.";
   _ns_pixel_proc_dynamic_range.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_dynamic_range.params      = _ns_pixel_proc_dynamic_range_params;
   _ns_pixel_proc_dynamic_range.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_dynamic_range_params );
   _ns_pixel_proc_dynamic_range.marshal     = _ns_pixel_proc_dynamic_range_marshal;

   _ns_pixel_proc_dynamic_range.ret_value.name = NULL;
   _ns_pixel_proc_dynamic_range.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_dynamic_range;
   }



