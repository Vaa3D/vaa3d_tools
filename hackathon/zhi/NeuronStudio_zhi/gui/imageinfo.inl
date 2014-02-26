#include <std/nsmisc.h>
#include <std/nsstring.h>
#include <image/nspixels.h>
#include <image/nspixels-blur.h>
#include <image/nspixels-dynamicrange.h>
#include <image/nspixels-brightnesscontrast.h>
#include <image/nspixels-gammacorrect.h>
#include <image/nspixels-noise.h>

#include "tabcontrol.h"


NsTabControl  ____image_info_tc;

nschar ____image_info_buffer[ 512 ];
NsString ____image_info_filters_string;
NsString ____image_info_additional_string;


void _image_info_additional_do_put_string( HWND dlg, nschar *s, nschar *lf )
	{
	nschar     *cr;
	nsboolean   found;


	/* Terminate the string at the carriage return instead
		of the line feed character, that is if there is one. */

	found = NS_FALSE;
	cr    = ( NULL != lf ) && ( s < lf ) ? lf - 1 : NULL;

	if( ( NULL != cr ) && ( NS_ASCII_CARRIAGE_RETURN == *cr ) )
		{
		found = NS_TRUE;
		*cr   = NS_ASCII_NULL;
		}

	if( NS_ASCII_NULL != *s )
		{
		ns_string_append( &____image_info_additional_string, s );
		ns_string_append( &____image_info_additional_string, "\r\n" );
		
		/*SendMessage(
			GetDlgItem( dlg, IDL_IMAGE_INFO_ADDITIONAL_INFO ),
			LB_ADDSTRING,
			0,
			( LPARAM )s
			);*/
		}

	if( found )
		*cr = NS_ASCII_CARRIAGE_RETURN;
	}


void _image_info_additional_put_string( HWND dlg, const nschar *string )
	{
	nschar *lf, *s;

	s = ( nschar* )string;

	/* The string could have mulitple lines. */
	while( NULL != ( lf = ns_ascii_strchr( s, NS_UNICHAR_NEWLINE ) ) )
		{
		*lf = NS_ASCII_NULL;

		_image_info_additional_do_put_string( dlg, s, lf );

		*lf = ( nschar )NS_UNICHAR_NEWLINE;
		s   = lf + 1;
		}

	_image_info_additional_do_put_string( dlg, s, NULL );
	}


void _image_info_filters_put_string( HWND dlg, const nschar *string )
	{
	/*
	SendMessage(
		GetDlgItem( dlg, IDL_IMAGE_INFO_APPLIED_FILTERS ),
	   LB_ADDSTRING,
		0,
		( LPARAM )string
		);
	*/
	ns_string_append( &____image_info_filters_string, string );
	ns_string_append( &____image_info_filters_string, "\r\n" );
	}


void _image_info_filters_add_description( HWND dlg, const nschar *description, nssize index )
	{
	ns_snprint(
		____image_info_buffer,
		NS_ARRAY_LENGTH( ____image_info_buffer ),
		NS_FMT_ULONG ") " NS_FMT_STRING,
		( nsulong )index,
		description
		);

	_image_info_filters_put_string( dlg, ____image_info_buffer );
	}


void _image_info_filters_add_string( HWND dlg, const nschar *name, const nschar *value )
	{
	ns_snprint(
		____image_info_buffer,
		NS_ARRAY_LENGTH( ____image_info_buffer ),
		"   " NS_FMT_STRING " = " NS_FMT_STRING,
		name,
		value
		);

	_image_info_filters_put_string( dlg, ____image_info_buffer );
	}


void _image_info_filters_add_ulong( HWND dlg, const nschar *name, nsulong value )
	{
	ns_snprint(
		____image_info_buffer,
		NS_ARRAY_LENGTH( ____image_info_buffer ),
		"   " NS_FMT_STRING " = " NS_FMT_ULONG,
		name,
		value
		);

	_image_info_filters_put_string( dlg, ____image_info_buffer );
	}


void _image_info_filters_add_double( HWND dlg, const nschar *name, nsdouble value )
	{
	ns_snprint(
		____image_info_buffer,
		NS_ARRAY_LENGTH( ____image_info_buffer ),
		"   " NS_FMT_STRING " = %.2f",
		name,
		value
		);

	_image_info_filters_put_string( dlg, ____image_info_buffer );
	}


void _image_info_filters_blur( HWND dlg, const NsClosureRecord *record )
	{}


void _image_info_filters_blur_more( HWND dlg, const NsClosureRecord *record )
	{}


void _image_info_filters_brightness_contrast( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_BRIGHTNESS )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_BRIGHTNESS_CONTRAST_CONTRAST )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );
	}



void _image_info_filters_gamma_correct( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_GAMMA_CORRECT_GAMMA )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );
	}



void _image_info_filters_resize( HWND dlg, const NsClosureRecord *record )
	{
	const nschar  *name;
	const NsValue *param;

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_SRC_WIDTH )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_SRC_HEIGHT )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_SRC_LENGTH )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_DEST_WIDTH )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_DEST_HEIGHT )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_DEST_LENGTH )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );
	_image_info_filters_add_ulong( dlg, name, ( nsulong )ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_RESIZE_INTERP_TYPE )->name;
	param = ns_closure_record_actual_param_by_name( record, name );
	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_ENUM ) );
	_image_info_filters_add_string(
		dlg,
		name,
		ns_pixel_proc_resize_type_to_string( ns_value_get_enum( param ) )
		); 
	}



void _image_info_filters_dynamic_range( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;
	nsfloat         channel;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MIN )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_FLOAT ) );

	/* TEMP??? The returned pointer must be a global since it would be
		destroyed on the stack before we could access it!!! */
	channel = ns_value_get_float( param );
	_image_info_filters_add_double( dlg, name, ( nsdouble )channel );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_DYNAMIC_RANGE_CHANNEL_MAX )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_FLOAT ) );

	channel = ns_value_get_float( param );
	_image_info_filters_add_double( dlg, name, ( nsdouble )channel );
	}



void _image_info_filters_noise( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_NOISE_LEVEL )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_NOISE_ITERATIONS )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_SIZE ) );

	_image_info_filters_add_ulong( dlg, name, ns_value_get_size( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_NOISE_AVERAGE_INTENSITY )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_NOISE_SIGNAL_TO_NOISE_RATIO )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_PDOUBLE ) );

	_image_info_filters_add_double( dlg, name, *( ns_value_get_pdouble( param ) ) );
	}


void _image_info_filters_flip( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_FLIP_TYPE )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_INT ) );

	_image_info_filters_add_string(
		dlg,
		name,
		ns_pixel_proc_flip_type_to_string( ns_value_get_int( param ) )
		);
	}


void _image_info_filters_subsample( HWND dlg, const NsClosureRecord *record )
	{
	const nschar   *name;
	const NsValue  *param;


	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_SUBSAMPLE_SCALE_X )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_SUBSAMPLE_SCALE_Y )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );

	name  = ns_closure_record_formal_param( record, NS_PIXEL_PROC_SUBSAMPLE_SCALE_Z )->name;
	param = ns_closure_record_actual_param_by_name( record, name );

	ns_assert( NULL != param );
	ns_assert( ns_value_is( param, NS_VALUE_DOUBLE ) );

	_image_info_filters_add_double( dlg, name, ns_value_get_double( param ) );
	}




/*
#define __SEPARATOR\
	"________________________________________________________________"
*/


void _image_info_filters_add_separator( HWND dlg )
	{
	//_image_info_filters_put_string( dlg, __SEPARATOR );
	}


void _image_info_list_filter( HWND dlg, const NsClosureRecord *record, nssize index )
	{
	const nschar *description = ns_closure_record_description( record );

	_image_info_filters_put_string( dlg, " " );
	_image_info_filters_add_description( dlg, description, index );

	if( ns_ascii_streq( description, ns_pixel_proc_blur()->title ) )
		_image_info_filters_blur( dlg, record );
	else if( ns_ascii_streq( description, "Blur More" ) )/* TEMP??? Special case */
		_image_info_filters_blur_more( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_dynamic_range()->title ) )
		_image_info_filters_dynamic_range( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_brightness_contrast()->title ) )
		_image_info_filters_brightness_contrast( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_gamma_correct()->title ) )
		_image_info_filters_gamma_correct( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_resize()->title ) )
		_image_info_filters_resize( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_noise()->title ) )
		_image_info_filters_noise( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_flip()->title ) )
		_image_info_filters_flip( dlg, record );
	else if( ns_ascii_streq( description, ns_pixel_proc_subsample()->title ) )
		_image_info_filters_subsample( dlg, record );

	_image_info_filters_add_separator( dlg );
	//_image_info_put_string( dlg, " " );
	}


NS_PRIVATE void _image_info_draw_projection( HWND dlg, nsint id, nsint which )
	{
   HWND    hWnd;
   HDC     hDC;
   RECT    rc;
	//MemoryGraphics *graphics;
	BITMAPINFO bmi;
	const NsImage *thumbnail;


   hWnd = GetDlgItem( dlg, id );

   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );

	thumbnail = workspace_thumbnail( s_MainWindow.activeWorkspace, which );
	ns_assert( NS_PIXEL_RGB_U8_U8_U8 == ns_image_pixel_type( thumbnail ) );


/* TEMP!!! */
if( ( nssize )( rc.right - rc.left ) != ns_image_width( thumbnail ) )
	ns_println( "width: %lu != %lu", ( nssize )( rc.right - rc.left ), ns_image_width( thumbnail ) );

if( ( nssize )( rc.bottom - rc.top ) != ns_image_height( thumbnail ) )
	ns_println( "height: %lu != %lu", ( nssize )( rc.bottom - rc.top ), ns_image_height( thumbnail ) );


	ns_memzero( &bmi, sizeof( BITMAPINFOHEADER ) );

	bmi.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth       = ( LONG )ns_image_width( thumbnail );
	bmi.bmiHeader.biHeight      = -( LONG )ns_image_height( thumbnail );
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
/*
	StretchDIBits(
		hDC,
		0, 0,
		ns_image_width( thumbnail ), ns_image_height( thumbnail ),
		0, 0, 
		ns_image_width( thumbnail ), ns_image_height( thumbnail ),
		ns_image_pixels( thumbnail ),
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
		);*/

	SetDIBitsToDevice(
		hDC,
		0, 0,
		( DWORD )ns_image_width( thumbnail ),
		( DWORD )ns_image_height( thumbnail ),
		0, 0, 
		0, ( DWORD )ns_image_height( thumbnail ),
		ns_image_pixels( thumbnail ),
		&bmi,
		DIB_RGB_COLORS
		);


   //graphics = GetWorkspaceDisplayGraphics( s_MainWindow.activeWorkspace, which );
/*
	StretchBlt(
		hDC,
		rc.left,
		rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top,
		graphics->hDC,
		0,
		0,
		graphics->width,
		graphics->height,
		SRCCOPY
		);
*/

   ReleaseDC( hWnd, hDC );
	}


NS_PRIVATE void _image_info_draw_projections( HWND dlg )
	{
	//_image_info_draw_projection( dlg, IDS_IMAGE_INFO_FORWARD_PROJECTION, NS_XY );
	//_image_info_draw_projection( dlg, IDS_IMAGE_INFO_SIDE_PROJECTION, NS_ZY );
	//_image_info_draw_projection( dlg, IDS_IMAGE_INFO_TOP_PROJECTION, NS_XZ );
	}



NS_PRIVATE void _image_info_dimensions_init( HWND dlg )
	{
	NsVoxelBuffer voxel_buffer;
	const NsImage *volume = workspace_volume( s_MainWindow.activeWorkspace );
	
	ns_voxel_buffer_init( &voxel_buffer, volume );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, ns_image_width( volume ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_WIDTH, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, ns_image_height( volume ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_HEIGHT, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, ns_image_length( volume ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_LENGTH, ____image_info_buffer );
	}


NS_PRIVATE void _image_info_roi_init( HWND dlg )
	{
	NsRoiInfo info;

	workspace_roi_info( s_MainWindow.activeWorkspace, &info );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.x1 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_X1, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.y1 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_Y1, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.z1 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_Z1, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.x2 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_X2, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.y2 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_Y2, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.z2 );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_Z2, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.width );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_WIDTH, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.height );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_HEIGHT, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG, info.length );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_LENGTH, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), "%.2f", info.percent_x );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_PERCENT_X, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), "%.2f", info.percent_y );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_PERCENT_Y, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), "%.2f", info.percent_z );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_PERCENT_Z, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), "%.2f", info.total_percent );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_ROI_TOTAL_PERCENT, ____image_info_buffer );
	}


NS_PRIVATE void _image_info_pixels_init( HWND dlg )
	{
	const NsImage *volume = workspace_volume( s_MainWindow.activeWorkspace );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
		ns_image_width( volume ) * ns_image_height( volume ) * ns_image_length( volume ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_TOTAL_PIXELS, ____image_info_buffer );

	SetDlgItemText( dlg, IDS_IMAGE_INFO_PIXEL_FORMAT_TYPE,
		ns_pixel_format_type_to_string( ns_image_pixel_format_type( volume ) ) );

	SetDlgItemText( dlg, IDS_IMAGE_INFO_PIXEL_CHANNEL_TYPE,
		ns_pixel_channel_type_to_string( ns_image_pixel_channel_type( volume, NS_PIXEL_CHANNEL_LUM ) ) );

	SetDlgItemText( dlg, IDS_IMAGE_INFO_PIXEL_TYPE,
		ns_pixel_type_to_string( ns_image_pixel_type( volume ) ) );
	}


NS_PRIVATE void _image_info_sizes_init( HWND dlg )
	{
	NsVoxelBuffer voxel_buffer;
	const NsImage *volume = workspace_volume( s_MainWindow.activeWorkspace );

	ns_voxel_buffer_init( &voxel_buffer, volume );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
		ns_pixel_bits( ns_image_pixel_type( volume ) ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_BITS_PER_PIXEL, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
		ns_voxel_buffer_bytes_per_row( &voxel_buffer ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_BYTES_PER_ROW, ____image_info_buffer );


	//ns_num_bytes_to_string(
	//	ns_voxel_buffer_bytes_per_slice( &voxel_buffer ) ,
	//	____image_info_buffer,
	//	NS_ARRAY_LENGTH( ____image_info_buffer )
	//	);
	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
		ns_voxel_buffer_bytes_per_slice( &voxel_buffer ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_BYTES_PER_SLICE, ____image_info_buffer );


	ns_num_bytes_to_string(
		ns_voxel_buffer_bytes_per_slice( &voxel_buffer ) * ns_image_length( volume ),
		____image_info_buffer,
		NS_ARRAY_LENGTH( ____image_info_buffer )
		);
	//ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
	//	ns_voxel_buffer_bytes_per_slice( &voxel_buffer ) * ns_image_length( volume ) );
	SetDlgItemText( dlg, IDS_IMAGE_INFO_TOTAL_BYTES, ____image_info_buffer );
	}


NS_PRIVATE void _image_info_intensity_init( HWND dlg )
	{
	//ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), NS_FMT_ULONG,
	//	workspace_get_max_intensity( s_MainWindow.activeWorkspace ) );
	//SetDlgItemText( dlg, IDS_IMAGE_INFO_MAX_INTENSITY, ____image_info_buffer );

	ns_snprint( ____image_info_buffer, NS_ARRAY_LENGTH( ____image_info_buffer ), "%.2f",
		workspace_get_average_intensity( s_MainWindow.activeWorkspace ) );

	SetDlgItemText( dlg, IDS_IMAGE_INFO_AVERAGE_INTENSITY, ____image_info_buffer );
	}


NS_PRIVATE void _image_info_filters_init( HWND dlg )
	{
	const NsList  *records;
	nslistiter     curr;
	nssize         index;


	records = workspace_records( s_MainWindow.activeWorkspace );
	index   = 1;

	if( ns_list_is_empty( records ) )
		_image_info_filters_put_string( dlg, "None" );
	else
		NS_LIST_FOREACH( records, curr )
			_image_info_list_filter( dlg, ns_list_iter_get_object( curr ), index++ );
	}


NS_PRIVATE void _image_info_additional_init( HWND dlg )
	{
	const NsList  *extra_info;
	nslistiter     curr;


	extra_info = workspace_image_extra_info( s_MainWindow.activeWorkspace );

	if( ns_list_is_empty( extra_info ) )
		_image_info_additional_put_string( dlg, "None" );
	else
		NS_LIST_FOREACH( extra_info, curr )
			{
			//_image_info_additional_do_put_string( dlg, " ", NULL );
			_image_info_additional_put_string( dlg, ns_list_iter_get_object( curr ) );
			//_image_info_additional_do_put_string( dlg, __SEPARATOR, NULL );
			}
	}


INT_PTR CALLBACK _image_info_dimensions_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			_image_info_dimensions_init( dlg );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_roi_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			_image_info_roi_init( dlg );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_pixels_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			_image_info_pixels_init( dlg );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_sizes_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			_image_info_sizes_init( dlg );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_intensity_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			_image_info_intensity_init( dlg );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_filters_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			ns_string_set( &____image_info_filters_string, "" );
			_image_info_filters_init( dlg );
			SetDlgItemText( dlg, IDL_IMAGE_INFO_APPLIED_FILTERS, ns_string_get( &____image_info_filters_string ) );
			return FALSE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _image_info_additional_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			ns_string_set( &____image_info_additional_string, "" );
			_image_info_additional_init( dlg );
			SetDlgItemText( dlg, IDL_IMAGE_INFO_ADDITIONAL_INFO, ns_string_get( &____image_info_additional_string ) );
			return FALSE;
		}

	return FALSE;
	}


NS_PRIVATE void _image_info_create_tabs( void )
	{
	ns_tab_control_add(
		&____image_info_tc,
		"Dimensions",
		"IMAGE_INFO_DIMENSIONS",
		_image_info_dimensions_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"R.O.I.",
		"IMAGE_INFO_ROI",
		_image_info_roi_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"Pixels",
		"IMAGE_INFO_PIXELS",
		_image_info_pixels_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"Sizes",
		"IMAGE_INFO_SIZES",
		_image_info_sizes_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"Intensity",
		"IMAGE_INFO_INTENSITY",
		_image_info_intensity_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"Filters",
		"IMAGE_INFO_FILTERS",
		_image_info_filters_dialog_proc
		);

	ns_tab_control_add(
		&____image_info_tc,
		"Additional",
		"IMAGE_INFO_ADDITIONAL",
		_image_info_additional_dialog_proc
		);

	ns_tab_control_on_select( &____image_info_tc );
	}


INT_PTR CALLBACK _image_info_dialog_proc( HWND dlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
		case WM_PAINT:
			_image_info_draw_projections( dlg );
			return FALSE;

      case WM_INITDIALOG:
			CenterWindow( dlg, s_MainWindow.hWnd );

			ns_tab_control_init(
				&____image_info_tc,
				g_Instance,
				dlg,
				IDT_IMAGE_INFO,
				7
				);

			_image_info_create_tabs();

         return FALSE;

		case WM_NOTIFY:
			switch( ( ( LPNMHDR )lParam )->code )
				{
				case TCN_SELCHANGE:
					ns_tab_control_on_select( &____image_info_tc );
					break;
				} 
			break; 

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            //case IDL_IMAGE_INFO_APPLIED_FILTERS:
              // SetFocus( GetDlgItem( dlg, IDOK ) );
               //break;

            case IDOK:
               EndDialog( dlg, 0 );
               break;
            }
         return TRUE;

      case WM_CLOSE:
         EndDialog( dlg, 0 );
         return TRUE;
      }

   return FALSE;
	}


void _image_info_dialog( void )
   {
	ns_string_construct( &____image_info_filters_string );
	ns_string_construct( &____image_info_additional_string );

	DialogBox( g_Instance, "IMAGE_INFO", s_MainWindow.hWnd, _image_info_dialog_proc );

	ns_string_destruct( &____image_info_filters_string );
	ns_string_destruct( &____image_info_additional_string );
	}
