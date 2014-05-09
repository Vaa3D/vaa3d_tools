#include "nspixels-convert.h"


NS_PRIVATE NsProc _ns_pixel_proc_convert;


enum
   {
   __SRC_PIXEL_TYPE,
   __SRC_PIXELS,
   __WIDTH,
   __HEIGHT,
   __LENGTH,
   __SRC_ROW_ALIGN,
   __DEST_PIXEL_TYPE,
   __DEST_PIXELS,
   __DEST_ROW_ALIGN,
   __PROGRESS,

   __NUM_PARAMS
   };


NS_PRIVATE NsClosureValue _ns_pixel_proc_convert_params[ __NUM_PARAMS ] =
   {
   { "src_pixel_type",  NS_VALUE_PIXEL_TYPE },
   { "src_pixels",      NS_VALUE_CONSTPOINTER },
   { "width",           NS_VALUE_SIZE },
   { "height",          NS_VALUE_SIZE },
   { "length",          NS_VALUE_SIZE },
   { "src_row_align",   NS_VALUE_SIZE },
   { "dest_pixel_type", NS_VALUE_PIXEL_TYPE },
   { "dest_pixels",     NS_VALUE_POINTER },
   { "dest_row_align",  NS_VALUE_SIZE },
   { "progress",        NS_VALUE_PROGRESS }
   };


typedef void ( *_NsPixelProcConvertFunc )
   (
   NsPixelType,
   nsconstpointer,
   nssize,
   nssize,
   nssize,
   nssize,
   NsPixelType,
   nspointer,
   nssize,
   NsProgress*
   );


NS_PRIVATE void _ns_pixel_proc_convert_func_all
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
   )
   {
   nssize              x;
   nssize              y;
   nssize              z;
   nssize              src_bytes_per_row;
   nssize              src_bytes_per_slice;
   nsconstpointer      src_slice;
   nsconstpointer      src_row;
   nssize              dest_bytes_per_row;
   nssize              dest_bytes_per_slice;
   nspointer           dest_slice;
   nspointer           dest_row;
   NsPixel             src;
   NsPixel             dest;
   NsPixelRowGetFunc   row_get_func;
   NsPixelRowSetFunc   row_set_func;
   NsPixelConvertFunc  convert_func;


   src_bytes_per_row   = ns_pixel_buffer_row_size( src_pixel_type, width, src_row_align );
   src_bytes_per_slice = ns_pixel_buffer_slice_size( src_pixel_type, width, height, src_row_align );
   src_slice           = src_pixels;

   dest_bytes_per_row   = ns_pixel_buffer_row_size( dest_pixel_type, width, dest_row_align );
   dest_bytes_per_slice = ns_pixel_buffer_slice_size( dest_pixel_type, width, height, dest_row_align );
   dest_slice           = dest_pixels;

   ns_pixel_init( &src, src_pixel_type );
   ns_pixel_init( &dest, dest_pixel_type );

   row_get_func = ns_pixel_row_get_func( src_pixel_type );
   row_set_func = ns_pixel_row_set_func( dest_pixel_type );
   convert_func = ns_pixel_convert_func( src_pixel_type, dest_pixel_type );

   ns_assert( NULL != row_get_func );
   ns_assert( NULL != row_set_func );
   ns_assert( NULL != convert_func );

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_BEGIN );

   for( z = 0; z < length; ++z )
      {
      if( NULL != progress )
         {
         if( ns_progress_cancelled( progress ) )
            return;

         ns_progress_update(
            progress,
            100.0f * ( nsfloat )z / ( nsfloat )length
            );
         }

      src_row  = src_slice;
      dest_row = dest_slice;

      for( y = 0; y < height; ++y )
         {
         for( x = 0; x < width; ++x )
            {
            ( row_get_func )( &src, src_row, x );
            ( convert_func )( &src, &dest );
            ( row_set_func )( &dest, dest_row, x );
            }

         src_row  = NS_OFFSET_POINTER( const void, src_row, src_bytes_per_row );
         dest_row = NS_OFFSET_POINTER( void, dest_row, dest_bytes_per_row );
         }

      src_slice  = NS_OFFSET_POINTER( const void, src_slice, src_bytes_per_slice );
      dest_slice = NS_OFFSET_POINTER( void, dest_slice, dest_bytes_per_slice );
      }

   if( NULL != progress )
      ns_progress_update( progress, NS_PROGRESS_END );
   }


NS_PRIVATE _NsPixelProcConvertFunc _ns_pixel_proc_convert_func
   (
   NsPixelType  src_pixel_type,
   NsPixelType  dest_pixel_type
   )
   {
   _NsPixelProcConvertFunc func = NULL;

   if( NULL != ns_pixel_convert_func( src_pixel_type, dest_pixel_type ) )
      func = _ns_pixel_proc_convert_func_all;

   if( NULL == func )
      ns_warning(
         NS_WARNING_LEVEL_RECOVERABLE
         NS_MODULE
         " Pixel conversion from format "
         NS_FMT_STRING_QUOTED
         " to "
         NS_FMT_STRING_QUOTED
         " not supported for procedure "
         NS_FMT_STRING_QUOTED
         ".",
         ns_pixel_type_to_string( src_pixel_type ),
         ns_pixel_type_to_string( dest_pixel_type ),
         _ns_pixel_proc_convert.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_convert_call
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
   )
   {
   _NsPixelProcConvertFunc func;

   if( NULL == ( func = _ns_pixel_proc_convert_func( src_pixel_type, dest_pixel_type ) ) )
      return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

   ns_assert( NULL != src_pixels );
   ns_assert( NULL != dest_pixels );
   ns_assert( 0 < width );
   ns_assert( 0 < height );
   ns_assert( 0 < length );

   ( func )(
      src_pixel_type,
      src_pixels,
      width,
      height,
      length,
      src_row_align,
      dest_pixel_type,
      dest_pixels,
      dest_row_align,
      progress
      );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_pixel_proc_convert_marshal
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

   ns_println( NS_FMT_STRING, _ns_pixel_proc_convert.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_convert.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_convert.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_convert.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_convert.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_convert_params[ __SRC_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + __SRC_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_convert_params[ __SRC_PIXELS ].name,
      ns_value_get_constpointer( params + __SRC_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_convert_params[ __WIDTH ].name,
      ns_value_get_size( params + __WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_convert_params[ __HEIGHT ].name,
      ns_value_get_size( params + __HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_convert_params[ __LENGTH ].name,
      ns_value_get_size( params + __LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_convert_params[ __SRC_ROW_ALIGN ].name,
      ns_value_get_size( params + __SRC_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_convert_params[ __DEST_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + __DEST_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_convert_params[ __DEST_PIXELS ].name,
      ns_value_get_pointer( params + __DEST_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_convert_params[ __DEST_ROW_ALIGN ].name,
      ns_value_get_size( params + __DEST_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_convert_params[ __PROGRESS ].name,
      ns_value_get_progress( params + __PROGRESS )
      );

   #endif/* NS_DEBUG */

   return _ns_pixel_proc_convert_call(
            ns_value_get_pixel_type( params + __SRC_PIXEL_TYPE ),
            ns_value_get_constpointer( params + __SRC_PIXELS ),
            ns_value_get_size( params + __WIDTH ),
            ns_value_get_size( params + __HEIGHT ),
            ns_value_get_size( params + __LENGTH ),
            ns_value_get_size( params + __SRC_ROW_ALIGN ),
            ns_value_get_pixel_type( params + __DEST_PIXEL_TYPE ),
            ns_value_get_pointer( params + __DEST_PIXELS ),
            ns_value_get_size( params + __DEST_ROW_ALIGN ),
            ns_value_get_progress( params + __PROGRESS )
            );
   }


NsProc* ns_pixel_proc_convert( void )
   {
   _ns_pixel_proc_convert.name        = NS_PIXEL_PROC_CONVERT;
   _ns_pixel_proc_convert.title       = "Convert";
   _ns_pixel_proc_convert.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_convert.params      = _ns_pixel_proc_convert_params;
   _ns_pixel_proc_convert.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_convert_params );
   _ns_pixel_proc_convert.marshal     = _ns_pixel_proc_convert_marshal;

   _ns_pixel_proc_convert.ret_value.name = NULL;
   _ns_pixel_proc_convert.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_convert;
   }



