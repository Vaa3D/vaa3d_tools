#include "nspixels-mpfilter.h"
#include <ext/filtermgr.h>


NS_PRIVATE NsProc _ns_pixel_proc_mp_filter;


enum
   {
   __PIXEL_TYPE,
   __PIXELS,
   __WIDTH,
   __HEIGHT,
   __LENGTH,
   __ROW_ALIGN,
   __TYPE,
   __NUM_CPU,
   __REGION,
   __PROGRESS,

   __NUM_PARAMS
   };


NS_PRIVATE NsClosureValue _ns_pixel_proc_mp_filter_params[ __NUM_PARAMS ] =
   {
   { "pixel_type", NS_VALUE_PIXEL_TYPE },
   { "pixels",     NS_VALUE_POINTER },
   { "width",      NS_VALUE_SIZE },
   { "height",     NS_VALUE_SIZE },
   { "length",     NS_VALUE_SIZE },
   { "row_align",  NS_VALUE_SIZE },
   { "type",       NS_VALUE_INT },
   { "num_cpu",    NS_VALUE_SIZE },
   { "region",     NS_VALUE_PIXEL_REGION },
   { "progress",   NS_VALUE_PROGRESS }
   };


typedef NsError ( *_NsPixelProcExtFunc )
   (
   NsPixelType,
   nspointer,
   nssize,
   nssize,
   nssize,
   nssize,
   nsint,
   nssize,
   const NsPixelRegion*,
   NsProgress*
   );


const nschar* ns_pixel_proc_mp_filter_type_to_string( nsint type )
   {
   switch( type )
      {
      case NS_PIXEL_PROC_MP_FILTER_BLUR:
         return "Blur - MP";

      case NS_PIXEL_PROC_MP_FILTER_MEDIAN:
         return "Median - MP";

      case NS_PIXEL_PROC_MP_FILTER_BLUR_MORE:
         return "Blur More - MP";

      case NS_PIXEL_PROC_MP_FILTER_INVERT:
         return "Invert - MP";

      case NS_PIXEL_PROC_MP_FILTER_Z_BLUR:
         return "Z-Blur - MP";

      case NS_PIXEL_PROC_MP_FILTER_LUM_U12_TO_LUM_U16:
         return "12 to 16 - MP";

      default:
         ns_assert_not_reached();
      }

   return NULL;
   }


NS_PRIVATE nsint _ns_pixel_proc_mp_filter_xfrm_type( nsint type )
   {
   switch( type )
      {
      case NS_PIXEL_PROC_MP_FILTER_BLUR:
         type = FILTER_BLUR;
         break;

      case NS_PIXEL_PROC_MP_FILTER_MEDIAN:
         type = FILTER_MEDIAN;
         break;

      case NS_PIXEL_PROC_MP_FILTER_BLUR_MORE:
         type = FILTER_BLUR26;
         break;

      case NS_PIXEL_PROC_MP_FILTER_INVERT:
         type = FILTER_INVERT;
         break;

      case NS_PIXEL_PROC_MP_FILTER_Z_BLUR:
         type = FILTER_ZBLUR;
         break;

      case NS_PIXEL_PROC_MP_FILTER_LUM_U12_TO_LUM_U16:
         type = FILTER_12to16;
         break;

      default:
         ns_assert_not_reached();
      }

   return type;
   }


NS_PRIVATE NsError _ns_pixel_proc_mp_filter_lum_u8
   (
   NsPixelType           pixel_type,
   NsPixelLumU8         *pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsint                 type,
   nssize                num_cpu,
   const NsPixelRegion  *region,
   NsProgress           *progress
   )
   {
   FILTERDEFINITION fd;

	NS_USE_VARIABLE( pixel_type );
	NS_USE_VARIABLE( row_align );

   fd.image = ( nsuchar* )pixels;

   ns_assert( width  <= ( nssize )NS_INT_MAX );
   ns_assert( height <= ( nssize )NS_INT_MAX );
   ns_assert( length <= ( nssize )NS_INT_MAX );
   fd.imagexdim  = ( nslong )width;
   fd.imageydim  = ( nslong )height;
   fd.imagezdim  = ( nslong )length;

   fd.threads    = ( nsint )NS_MAX( 1, num_cpu );
   fd.filtertype = _ns_pixel_proc_mp_filter_xfrm_type( type );
   fd.progress   = progress;

	fd.region_x      = ( nslong )region->x;
	fd.region_y      = ( nslong )region->y;
	fd.region_z      = ( nslong )region->z;
	fd.region_width  = ( nslong )region->width;
	fd.region_height = ( nslong )region->height;
	fd.region_length = ( nslong )region->length;

   if( NULL != progress )
      ns_progress_set_title( progress, ns_pixel_proc_mp_filter_type_to_string( type ) );

   return RunFilter( &fd ) ?
          ns_no_error() : ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
   }


NS_PRIVATE NsError _ns_pixel_proc_mp_filter_lum_u12_or_u16
   (
   NsPixelType           pixel_type,
   NsPixelLumU16        *pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsint                 type,
   nssize                num_cpu,
   const NsPixelRegion  *region,
   NsProgress           *progress
   )
   {
   FILTERDEFINITION16 fd;

	NS_USE_VARIABLE( pixel_type );
	NS_USE_VARIABLE( row_align );

   fd.image = ( nsushort* )pixels;

   ns_assert( width  <= ( nssize )NS_INT_MAX );
   ns_assert( height <= ( nssize )NS_INT_MAX );
   ns_assert( length <= ( nssize )NS_INT_MAX );
   fd.imagexdim  = ( nslong )width;
   fd.imageydim  = ( nslong )height;
   fd.imagezdim  = ( nslong )length;

   fd.threads    = ( nsint )NS_MAX( 1, num_cpu );
   fd.filtertype = _ns_pixel_proc_mp_filter_xfrm_type( type );
   fd.progress   = progress;

   fd.is12bit = ( NS_PIXEL_LUM_U12 == pixel_type );

	fd.region_x      = ( nslong )region->x;
	fd.region_y      = ( nslong )region->y;
	fd.region_z      = ( nslong )region->z;
	fd.region_width  = ( nslong )region->width;
	fd.region_height = ( nslong )region->height;
	fd.region_length = ( nslong )region->length;

   if( NULL != progress )
      ns_progress_set_title( progress, ns_pixel_proc_mp_filter_type_to_string( type ) );

   return RunFilter_16bit( &fd ) ?
          ns_no_error() : ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
   }


NS_PRIVATE _NsPixelProcExtFunc _ns_pixel_proc_mp_filter_func( NsPixelType pixel_type, nsint type )
   {
   _NsPixelProcExtFunc func = NULL;

	NS_USE_VARIABLE( type );

   switch( pixel_type )
      {
      case NS_PIXEL_LUM_U8:
         func = _ns_pixel_proc_mp_filter_lum_u8;
         break;

      case NS_PIXEL_LUM_U12:
      case NS_PIXEL_LUM_U16:
         func = _ns_pixel_proc_mp_filter_lum_u12_or_u16;
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
         _ns_pixel_proc_mp_filter.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_mp_filter_call
   (
   NsPixelType           pixel_type,
   nspointer             pixels,
   nssize                width,
   nssize                height,
   nssize                length,
   nssize                row_align,
   nsint                 type,
   nssize                num_cpu,
   const NsPixelRegion  *sub_region,
   NsProgress           *progress
   )
   {
   _NsPixelProcExtFunc   func;
   NsPixelRegion         full_region;
   const NsPixelRegion  *region;


   if( NULL == ( func = _ns_pixel_proc_mp_filter_func( pixel_type, type ) ) )
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
		", type=" NS_FMT_STRING_DOUBLE_QUOTED
		", num_cpu=" NS_FMT_ULONG
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
		ns_pixel_proc_mp_filter_type_to_string( type ),
		num_cpu,
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
            type,
            num_cpu,
            region,
            progress
            );
   }


NS_PRIVATE NsError _ns_pixel_proc_mp_filter_marshal
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
   ns_println( NS_FMT_STRING, _ns_pixel_proc_mp_filter.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_mp_filter.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_mp_filter.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_mp_filter.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_mp_filter.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_mp_filter_params[ __PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + __PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_mp_filter_params[ __PIXELS ].name,
      ns_value_get_pointer( params + __PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_mp_filter_params[ __WIDTH ].name,
      ns_value_get_size( params + __WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_mp_filter_params[ __HEIGHT ].name,
      ns_value_get_size( params + __HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_mp_filter_params[ __LENGTH ].name,
      ns_value_get_size( params + __LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_mp_filter_params[ __ROW_ALIGN ].name,
      ns_value_get_size( params + __ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_STRING,
      _ns_pixel_proc_mp_filter_params[ __TYPE ].name,
      ns_pixel_proc_mp_filter_type_to_string( ns_value_get_int( params + __TYPE ) )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_mp_filter_params[ __NUM_CPU ].name,
      ns_value_get_size( params + __NUM_CPU )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_mp_filter_params[ __REGION ].name,
      ns_value_get_pixel_region( params + __REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_mp_filter_params[ __PROGRESS ].name,
      ns_value_get_progress( params + __PROGRESS )
      );
	*/
   #endif/* NS_DEBUG */

   return _ns_pixel_proc_mp_filter_call(
            ns_value_get_pixel_type( params + __PIXEL_TYPE ),
            ns_value_get_pointer( params + __PIXELS ),
            ns_value_get_size( params + __WIDTH ),
            ns_value_get_size( params + __HEIGHT ),
            ns_value_get_size( params + __LENGTH ),
            ns_value_get_size( params + __ROW_ALIGN ),
            ns_value_get_int( params + __TYPE ),
            ns_value_get_size( params + __NUM_CPU ),
            ns_value_get_pixel_region( params + __REGION ),
            ns_value_get_progress( params + __PROGRESS )
            );
   }


NsProc* ns_pixel_proc_mp_filter( void )
   {
   _ns_pixel_proc_mp_filter.name        = NS_PIXEL_PROC_MP_FILTER;
   _ns_pixel_proc_mp_filter.version     = "1.0";
   _ns_pixel_proc_mp_filter.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_mp_filter.params      = _ns_pixel_proc_mp_filter_params;
   _ns_pixel_proc_mp_filter.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_mp_filter_params );
   _ns_pixel_proc_mp_filter.marshal     = _ns_pixel_proc_mp_filter_marshal;

   _ns_pixel_proc_mp_filter.ret_value.name = NULL;
   _ns_pixel_proc_mp_filter.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_mp_filter;
   }



