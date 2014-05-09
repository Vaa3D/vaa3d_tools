#include "nspixels-orthoproject.h"


NS_PRIVATE NsProc _ns_pixel_proc_ortho_project;


NS_PRIVATE NsClosureValue _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PARAMS ] =
   {
   { "src_pixel_type",    NS_VALUE_PIXEL_TYPE },
   { "src_pixels",        NS_VALUE_CONSTPOINTER },
   { "width",             NS_VALUE_SIZE },
   { "height",            NS_VALUE_SIZE },
   { "length",            NS_VALUE_SIZE },
   { "src_row_align",     NS_VALUE_SIZE },
   { "project_type",      NS_VALUE_ENUM },
   { "dest_pixel_type",   NS_VALUE_PIXEL_TYPE },
   { "dest_pixels_xy",    NS_VALUE_POINTER },
   { "dest_row_align_xy", NS_VALUE_SIZE },
   { "dest_pixels_zy",    NS_VALUE_POINTER },
   { "dest_row_align_zy", NS_VALUE_SIZE },
   { "dest_pixels_xz",    NS_VALUE_POINTER },
   { "dest_row_align_xz", NS_VALUE_SIZE },
   { "region",            NS_VALUE_PIXEL_REGION },
	{ "z_indices",         NS_VALUE_PSIZE },
	{ "num_z_indices",     NS_VALUE_SIZE },
   { "average_intensity", NS_VALUE_PDOUBLE },
	{ "num_pixels",        NS_VALUE_PSIZE },
	{ "use_src_size",      NS_VALUE_BOOLEAN },
   { "progress",          NS_VALUE_POINTER }
   };


typedef NsError ( *_NsPixelProcOrthoProjectFunc )
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
   nspointer,
   nssize,
   nspointer,
   nssize,
   const NsPixelRegion*,
	const nssize*,
	nssize,
   nsdouble*,
	nssize*,
	nsboolean,
   NsProgress*
   );


#define _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM\
   if( src->luminance > dest_xy->luminance )\
      dest_xy->luminance = src->luminance;\
   if( src->luminance > dest_zy->luminance )\
      dest_zy->luminance = src->luminance;\
   if( src->luminance > dest_xz->luminance )\
      dest_xz->luminance = src->luminance


#define _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM\
   if( src->luminance < dest_xy->luminance )\
      dest_xy->luminance = src->luminance;\
   if( src->luminance < dest_zy->luminance )\
      dest_zy->luminance = src->luminance;\
   if( src->luminance < dest_xz->luminance )\
      dest_xz->luminance = src->luminance


#define _NS_PIXEL_PROC_ORTHO_PROJECT_SUM_LUM\
   dest_xy->luminance += ( nsfloat32 )src->luminance;\
   dest_zy->luminance += ( nsfloat32 )src->luminance;\
   dest_xz->luminance += ( nsfloat32 )src->luminance


#include "nspixels-orthoproject-no-indices.inl"
#include "nspixels-orthoproject-with-indices.inl"


#define _NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF\
	diff = NS_ABS( lum1 - lum2 );\
	if( diff > *dest_temp_xy )\
		{\
		*dest_temp_xy      = diff;\
		dest_xy->luminance = src->luminance;\
		}

#define _NS_PIXEL_PROC_ORTHO_PROJECT_MAX_LUM_MAX_DIFF( src_type )\
	lum1 = ( nsint )( src->luminance );\
	if( x > 0 )\
		{\
		lum2 = ( nsint )( ( src - 1 )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( x < region->width - 1 )\
		{\
		lum2 = ( nsint )( ( src + 1 )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( y > 0 )\
		{\
		lum2 = ( nsint )( NS_OFFSET_POINTER( src_type, src, -( ( nslong )( src_bytes_per_row ) ) )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( y < region->height - 1 )\
		{\
		lum2 = ( nsint )( NS_OFFSET_POINTER( src_type, src, src_bytes_per_row )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
   if( src->luminance > dest_zy->luminance )\
      dest_zy->luminance = src->luminance;\
   if( src->luminance > dest_xz->luminance )\
      dest_xz->luminance = src->luminance


#define _NS_PIXEL_PROC_ORTHO_PROJECT_MIN_LUM_MAX_DIFF( src_type )\
	lum1 = ( nsint )( src->luminance );\
	if( x > 0 )\
		{\
		lum2 = ( nsint )( ( src - 1 )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( x < region->width - 1 )\
		{\
		lum2 = ( nsint )( ( src + 1 )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( y > 0 )\
		{\
		lum2 = ( nsint )( NS_OFFSET_POINTER( src_type, src, -( ( nslong )( src_bytes_per_row ) ) )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
	if( y < region->height - 1 )\
		{\
		lum2 = ( nsint )( NS_OFFSET_POINTER( src_type, src, src_bytes_per_row )->luminance );\
		_NS_PIXEL_PROC_ORTHO_PROJECT_LUM_MAX_DIFF;\
		}\
   if( src->luminance < dest_zy->luminance )\
      dest_zy->luminance = src->luminance;\
   if( src->luminance < dest_xz->luminance )\
      dest_xz->luminance = src->luminance


#include "nspixels-orthoproject-maxdiff.inl"


NS_PRIVATE _NsPixelProcOrthoProjectFunc _ns_pixel_proc_ortho_project_func
   (
   NsPixelType    src_pixel_type,
   NsPixelType    dest_pixel_type,
   nsenum         project_type,
	const nssize  *z_indices
   )
   {
   _NsPixelProcOrthoProjectFunc func = NULL;

   switch( src_pixel_type )
      {
      case NS_PIXEL_LUM_U8:
         switch( project_type )
            {
            case NS_PIXEL_PROC_ORTHO_PROJECT_MAX:
               if( NS_PIXEL_LUM_U8 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_max_lum_u8_no_indices :
								 _ns_pixel_proc_ortho_project_max_lum_u8_with_indices;
               break;
            case NS_PIXEL_PROC_ORTHO_PROJECT_MIN:
               if( NS_PIXEL_LUM_U8 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_min_lum_u8_no_indices :
								 _ns_pixel_proc_ortho_project_min_lum_u8_with_indices;
               break;
            case NS_PIXEL_PROC_ORTHO_PROJECT_SUM:
               if( NS_PIXEL_LUM_F32 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_sum_lum_u8_no_indices :
								 _ns_pixel_proc_ortho_project_sum_lum_u8_with_indices;
               break;
            }
         break;

      case NS_PIXEL_LUM_U12:
      case NS_PIXEL_LUM_U16:
         switch( project_type )
            {
            case NS_PIXEL_PROC_ORTHO_PROJECT_MAX:
               if( NS_PIXEL_LUM_U12 == dest_pixel_type || NS_PIXEL_LUM_U16 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_max_lum_u12_or_u16_no_indices :
								 _ns_pixel_proc_ortho_project_max_lum_u12_or_u16_with_indices;
               break;
            case NS_PIXEL_PROC_ORTHO_PROJECT_MIN:
               if( NS_PIXEL_LUM_U12 == dest_pixel_type || NS_PIXEL_LUM_U16 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_min_lum_u12_or_u16_no_indices :
								 _ns_pixel_proc_ortho_project_min_lum_u12_or_u16_with_indices;
               break;
            case NS_PIXEL_PROC_ORTHO_PROJECT_SUM:
               if( NS_PIXEL_LUM_F32 == dest_pixel_type )
						func = NULL == z_indices ?
								 _ns_pixel_proc_ortho_project_sum_lum_u12_or_u16_no_indices :
								 _ns_pixel_proc_ortho_project_sum_lum_u12_or_u16_with_indices;
               break;
            }
         break;
      }

   if( NULL == func )
      ns_warning(
         NS_WARNING_LEVEL_RECOVERABLE
         NS_MODULE
         " Pixel format "
         NS_FMT_STRING_QUOTED
         " ortho-project of type "
         NS_FMT_STRING_QUOTED
         " to pixel format "
         NS_FMT_STRING_QUOTED
         " not supported for procedure "
         NS_FMT_STRING_QUOTED
         ".",
         ns_pixel_type_to_string( src_pixel_type ),
         ns_pixel_proc_ortho_project_type_to_string( project_type ),
         ns_pixel_type_to_string( dest_pixel_type ),
         _ns_pixel_proc_ortho_project.name
         );

   return func;
   }


NS_PRIVATE NsError _ns_pixel_proc_ortho_project_call
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
   const NsPixelRegion  *sub_region,
	const nssize         *z_indices,
	nssize                num_z_indices,
   nsdouble             *average_intensity,
	nssize               *num_pixels,
	nsboolean             use_src_size,
   NsProgress           *progress
   )
   {
   _NsPixelProcOrthoProjectFunc   func;
   NsPixelRegion                  full_region;
   const NsPixelRegion           *region;


	func = _ns_pixel_proc_ortho_project_func(
				src_pixel_type,
				dest_pixel_type,
				project_type,
				z_indices
				);

   if( NULL == func )
      return ns_error_noimp( NS_ERROR_LEVEL_CAUTION, NS_MODULE );

   ns_assert( NULL != src_pixels );
   ns_assert( 0 < width );
   ns_assert( 0 < height );
   ns_assert( 0 < length );
   ns_assert( NULL != dest_pixels_xy );
   ns_assert( NULL != dest_pixels_zy );
   ns_assert( NULL != dest_pixels_xz );
   ns_assert( NULL != average_intensity );

   region = ns_pixel_region_extract( sub_region, &full_region, width, height, length );
   ns_pixel_region_verify( region, width, height, length );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( src_pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", src_pixels=" NS_FMT_POINTER
		", width=" NS_FMT_ULONG
		", height=" NS_FMT_ULONG
		", length=" NS_FMT_ULONG
		", src_row_align=" NS_FMT_ULONG
		", project_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", dest_pixel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", dest_pixels_xy=" NS_FMT_POINTER
		", dest_row_align_xy=" NS_FMT_ULONG
		", dest_pixels_zy=" NS_FMT_POINTER
		", dest_row_align_zy=" NS_FMT_ULONG
		", dest_pixels_xz=" NS_FMT_POINTER
		", dest_row_align_xz=" NS_FMT_ULONG
		", region={x:" NS_FMT_ULONG
			",y:" NS_FMT_ULONG
			",z:" NS_FMT_ULONG
			",width:" NS_FMT_ULONG
			",height:" NS_FMT_ULONG
			",length:" NS_FMT_ULONG
			"}"
		", z_indices=" NS_FMT_POINTER
		", num_z_indices=" NS_FMT_ULONG
		", average_intensity=" NS_FMT_POINTER
		", num_pixels=" NS_FMT_POINTER
		", use_src_size=" NS_FMT_INT
		", progress=" NS_FMT_POINTER
		" )",
		ns_pixel_type_to_string( src_pixel_type ),
		src_pixels,
		width,
		height,
		length,
		src_row_align,
		ns_pixel_proc_ortho_project_type_to_string( project_type ),
		ns_pixel_type_to_string( dest_pixel_type ),
		dest_pixels_xy,
		dest_row_align_xy,
		dest_pixels_zy,
		dest_row_align_zy,
		dest_pixels_xz,
		dest_row_align_xz,
		region->x, region->y, region->z, region->width, region->height, region->length,
		z_indices,
		num_z_indices,
		average_intensity,
		num_pixels,
		use_src_size,
		progress
		);

   return ( func )(
				src_pixel_type,
				src_pixels,
				width,
				height,
				length,
				src_row_align,      
				dest_pixel_type,
				dest_pixels_xy,
				dest_row_align_xy,
				dest_pixels_zy,
				dest_row_align_zy,
				dest_pixels_xz,
				dest_row_align_xz,
				region,
				z_indices,
				num_z_indices,
				average_intensity,
				num_pixels,
				use_src_size,
				progress
				);
   }


NsError _ns_pixel_proc_ortho_project_marshal
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

   ns_println( NS_FMT_STRING, _ns_pixel_proc_ortho_project.title );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_ortho_project.author );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_ortho_project.version );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_ortho_project.copyright );
   ns_println( NS_FMT_STRING, _ns_pixel_proc_ortho_project.help );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_CONSTPOINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXELS ].name,
      ns_value_get_constpointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_WIDTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_WIDTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_HEIGHT ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_HEIGHT )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_LENGTH ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_LENGTH )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_SRC_ROW_ALIGN ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_ROW_ALIGN )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_PROJECT_TYPE ].name,
      ns_value_get_enum( params + NS_PIXEL_PROC_ORTHO_PROJECT_PROJECT_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXEL_TYPE ].name,
      ns_value_get_pixel_type( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXEL_TYPE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XY ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XY ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_ZY ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_ZY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_ZY ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_ZY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XZ ].name,
      ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XZ )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XZ ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XZ )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_REGION ].name,
      ns_value_get_pixel_region( params + NS_PIXEL_PROC_ORTHO_PROJECT_REGION )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_Z_INDICES ].name,
      ns_value_get_psize( params + NS_PIXEL_PROC_ORTHO_PROJECT_Z_INDICES )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_ULONG,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_NUM_Z_INDICES ].name,
      ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_NUM_Z_INDICES )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_AVERAGE_INTENSITY ].name,
      ns_value_get_pdouble( params + NS_PIXEL_PROC_ORTHO_PROJECT_AVERAGE_INTENSITY )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PIXELS ].name,
      ns_value_get_psize( params + NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PIXELS )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_INT,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_USE_SRC_SIZE ].name,
      ns_value_get_pulong( params + NS_PIXEL_PROC_ORTHO_PROJECT_USE_SRC_SIZE )
      );

   ns_println(
      NS_FMT_STRING " = " NS_FMT_POINTER,
      _ns_pixel_proc_ortho_project_params[ NS_PIXEL_PROC_ORTHO_PROJECT_PROGRESS ].name,
      ns_value_get_progress( params + NS_PIXEL_PROC_ORTHO_PROJECT_PROGRESS )
      );

   #endif/* NS_DEBUG */

   return _ns_pixel_proc_ortho_project_call(
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXEL_TYPE ),
            ns_value_get_constpointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXELS ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_WIDTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_HEIGHT ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_LENGTH ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_SRC_ROW_ALIGN ),
            ns_value_get_enum( params + NS_PIXEL_PROC_ORTHO_PROJECT_PROJECT_TYPE ),
            ns_value_get_pixel_type( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXEL_TYPE ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XY ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XY ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_ZY ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_ZY ),
            ns_value_get_pointer( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XZ ),
            ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XZ ),
            ns_value_get_pixel_region( params + NS_PIXEL_PROC_ORTHO_PROJECT_REGION ),
				ns_value_get_psize( params + NS_PIXEL_PROC_ORTHO_PROJECT_Z_INDICES ),
				ns_value_get_size( params + NS_PIXEL_PROC_ORTHO_PROJECT_NUM_Z_INDICES ),
            ns_value_get_pdouble( params + NS_PIXEL_PROC_ORTHO_PROJECT_AVERAGE_INTENSITY ),
            ns_value_get_psize( params + NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PIXELS ),
				ns_value_get_boolean( params + NS_PIXEL_PROC_ORTHO_PROJECT_USE_SRC_SIZE ),
            ns_value_get_progress( params + NS_PIXEL_PROC_ORTHO_PROJECT_PROGRESS )
            );
   }


const nschar* ns_pixel_proc_ortho_project_type_to_string( nsenum project_type )
   {
   NS_PRIVATE const nschar*
      _ns_pixel_proc_ortho_project_type_strings[ _NS_PIXEL_PROC_ORTHO_PROJECT_NUM_TYPES ] =
      {
      "max",
      "min",
      "sum"
      };

   ns_assert( project_type < _NS_PIXEL_PROC_ORTHO_PROJECT_NUM_TYPES );
   return _ns_pixel_proc_ortho_project_type_strings[ project_type ];
   }


NsProc* ns_pixel_proc_ortho_project( void )
   {
   _ns_pixel_proc_ortho_project.name        = NS_PIXEL_PROC_ORTHO_PROJECT;
   _ns_pixel_proc_ortho_project.title       = "Orthogonal Projections";
   _ns_pixel_proc_ortho_project.version     = "2.0";
   _ns_pixel_proc_ortho_project.help        = "Projections of 3D image along the three planes defined by the coordinate axis'. Forward(XY),Side(ZY),Top(XZ)";
   _ns_pixel_proc_ortho_project.invoke_type = NS_PROC_INVOKE_MARSHAL;
   _ns_pixel_proc_ortho_project.params      = _ns_pixel_proc_ortho_project_params;
   _ns_pixel_proc_ortho_project.num_params  = NS_ARRAY_LENGTH( _ns_pixel_proc_ortho_project_params );
   _ns_pixel_proc_ortho_project.marshal     = _ns_pixel_proc_ortho_project_marshal;

   _ns_pixel_proc_ortho_project.ret_value.name = NULL;
   _ns_pixel_proc_ortho_project.ret_value.type = NS_VALUE_VOID;

   return &_ns_pixel_proc_ortho_project;
   }



