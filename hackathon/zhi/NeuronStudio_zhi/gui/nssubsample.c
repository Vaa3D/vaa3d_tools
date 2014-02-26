#include "nssubsample.h"


enum{ _X_AXIS, _Y_AXIS, _Z_AXIS };


typedef struct _NsSubsampleDialog
   {
	HINSTANCE       instance;
	HWND            wnd;
   HWND            dlg;
   SUBSAMPLER      impl;
	nsuint          volume_width;
	nsuint          volume_height;
	nsuint          volume_length;
	nsuint          volume_bpp;
   nsdouble        scale_width;
   nsdouble        scale_height;
   nsdouble        scale_length;
   nsint           lock_changes;
	nsboolean       cancelled;
	const NsImage  *volume;
   }
   NsSubsampleDialog;


NS_PRIVATE NsSubsampleDialog ____ns_subsample_dialog;

NS_PRIVATE nsboolean ____ns_ssd_prev_fixed_width_height = NS_FALSE;
NS_PRIVATE nsboolean ____ns_ssd_prev_fixed_all          = NS_FALSE;

NS_PRIVATE nsboolean ____ns_ssd_curr_fixed_width_height;
NS_PRIVATE nsboolean ____ns_ssd_curr_fixed_all;


void _ns_subsample_dialog_calc_sizes( NsSubsampleDialog *ssd )
   {
	InitSubsampler(
		&ssd->impl,
		ssd->volume_width,
		ssd->volume_height,
		ssd->volume_length,
		ssd->scale_width  / 100.0,
		ssd->scale_height / 100.0,
		ssd->scale_length / 100.0
		);
	}


void _ns_subsample_dialog_show_fields( NsSubsampleDialog *ssd, nsint id )
   {
   nsint64  bytes;
	nsint    bytes_per_pixel;
   nschar   temp[ 128 ];


   ssd->lock_changes = NS_TRUE;
 
   ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ssd->volume_width );
   SetDlgItemText( ssd->dlg, IDS_SUB_SOURCE_WIDTH_PIXELS, temp );

   ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ssd->volume_height );
   SetDlgItemText( ssd->dlg, IDS_SUB_SOURCE_HEIGHT_PIXELS, temp );

   ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ssd->volume_length );
   SetDlgItemText( ssd->dlg, IDS_SUB_SOURCE_LENGTH_PIXELS, temp );

   if( IDE_SUB_DESTINATION_WIDTH_PIXELS != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ( nsuint )ssd->impl.newxdim );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_WIDTH_PIXELS, temp );
      }

   if( IDE_SUB_DESTINATION_WIDTH_PERCENT != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_MAKE_FMT( ".2", NS_FMT_TYPE_DOUBLE ), ssd->scale_width );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_WIDTH_PERCENT, temp );
      }

   if( IDE_SUB_DESTINATION_HEIGHT_PIXELS != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ( nsuint )ssd->impl.newydim );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_HEIGHT_PIXELS, temp );
      }

   if( IDE_SUB_DESTINATION_HEIGHT_PERCENT != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_MAKE_FMT( ".2", NS_FMT_TYPE_DOUBLE ), ssd->scale_height );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_HEIGHT_PERCENT, temp );
      }

   if( IDE_SUB_DESTINATION_LENGTH_PIXELS != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, ( nsuint )ssd->impl.newzdim );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_LENGTH_PIXELS, temp );
      }

   if( IDE_SUB_DESTINATION_LENGTH_PERCENT != id )
      {
      ns_snprint( temp, sizeof( temp ), NS_MAKE_FMT( ".2", NS_FMT_TYPE_DOUBLE ), ssd->scale_length );
      SetDlgItemText( ssd->dlg, IDE_SUB_DESTINATION_LENGTH_PERCENT, temp );
      }

	bytes_per_pixel = NS_BITS_TO_BYTES( ssd->volume_bpp );

   bytes  = ssd->volume_width;
   bytes *= ssd->volume_height;
   bytes *= ssd->volume_length;
	bytes *= bytes_per_pixel;

   ns_num_bytes_to_string( ( nssize )bytes, temp, sizeof( temp ) );
   SetDlgItemText( ssd->dlg, IDS_SUB_SOURCE_SIZE, temp );

   bytes  = ssd->impl.newxdim;
   bytes *= ssd->impl.newydim;
   bytes *= ssd->impl.newzdim;
	bytes *= bytes_per_pixel;

   ns_num_bytes_to_string( ( nssize )bytes, temp, sizeof( temp ) );
   SetDlgItemText( ssd->dlg, IDS_SUB_DESTINATION_SIZE, temp );

   ssd->lock_changes = NS_FALSE;
   }


void _ns_subsample_dialog_draw_icons( NsSubsampleDialog *ssd )
   {
   HWND  wnd;
   HDC   dc;
   RECT  rc;


   wnd = GetDlgItem( ssd->dlg, IDS_SUB_LOCK_XY );
   dc  = GetDC( wnd );

   GetClientRect( wnd, &rc );

   if( ____ns_ssd_curr_fixed_all || ____ns_ssd_curr_fixed_width_height )
      DrawIcon( dc, 0, 0, LoadIcon( ssd->instance, "LOCK_ICON" ) );
   else
      FillRect( dc, &rc, ( HBRUSH )( COLOR_3DFACE + 1 ) );

   ReleaseDC( wnd, dc );

   wnd = GetDlgItem( ssd->dlg, IDS_SUB_LOCK_YZ );
   dc  = GetDC( wnd );

   GetClientRect( wnd, &rc );

   if( ____ns_ssd_curr_fixed_all )
      DrawIcon( dc, 0, 0, LoadIcon( ssd->instance, "LOCK_ICON" ) );
   else
      FillRect( dc, &rc, ( HBRUSH )( COLOR_3DFACE + 1 ) );

   ReleaseDC( wnd, dc );
   }


void _ns_subsample_dialog_update
   ( 
   NsSubsampleDialog  *ssd,
   nsint               id,
   nsdouble            scale,
   nsenum              type
   )
   {
   if( ____ns_ssd_curr_fixed_all )
      {
      ssd->scale_width  = scale;
      ssd->scale_height = scale;
      ssd->scale_length = scale;
      }
   else if( ____ns_ssd_curr_fixed_width_height && ( _X_AXIS == type || _Y_AXIS == type ) )
      {
      ssd->scale_width  = scale;
      ssd->scale_height = scale;
      }
   else
      {
      switch( type )
         {
         case _X_AXIS: ssd->scale_width  = scale; break;
         case _Y_AXIS: ssd->scale_height = scale; break;
         case _Z_AXIS: ssd->scale_length = scale; break;
         }
      }

   _ns_subsample_dialog_calc_sizes( ssd );
	_ns_subsample_dialog_show_fields( ssd, id );
   }


void _ns_subsample_dialog_on_change_pixels( NsSubsampleDialog *ssd, nsint id, nsenum type )
   {
   nsuint    pixels;
   nsuint    dimension;
   nsdouble  scale;
   nschar    temp[ 128 ] = { '\0' };


   GetDlgItemText( ssd->dlg, id, temp, sizeof( temp ) - 1 );

   pixels = ( nsuint )ns_atoi( temp );

   switch( type )
      {
      case _X_AXIS: dimension = ssd->volume_width;  break;
      case _Y_AXIS: dimension = ssd->volume_height; break;
      case _Z_AXIS: dimension = ssd->volume_length; break;
      }

   if( dimension < pixels )
      {
      pixels = dimension;

      ssd->lock_changes = TRUE;

      ns_snprint( temp, sizeof( temp ), NS_FMT_UINT, pixels );
      SetDlgItemText( ssd->dlg, id, temp );

      ssd->lock_changes = FALSE;
      }

   scale = ( ( nsdouble )pixels / ( nsdouble )dimension ) * 100.0;

	_ns_subsample_dialog_update( ssd, id, scale, type );
   }


void _ns_subsample_dialog_on_change_percent( NsSubsampleDialog *ssd, nsint id, nsenum type )
   {
   nsdouble  scale;
   nschar    temp[ 128 ] = { '\0' };


   GetDlgItemText( ssd->dlg, id, temp, sizeof( temp ) - 1 );

   scale = ns_atod( temp );

   if( scale < 0.0 )
      {
      scale = 0.0;

      ssd->lock_changes = TRUE;

      ns_snprint( temp, sizeof( temp ), NS_MAKE_FMT( ".2", NS_FMT_TYPE_DOUBLE ), scale );
      SetDlgItemText( ssd->dlg, id, temp );

      ssd->lock_changes = FALSE;
      }
   else if( 100.0 < scale )
      {
      scale = 100.0;

      ssd->lock_changes = TRUE;

      ns_snprint( temp, sizeof( temp ), NS_MAKE_FMT( ".2", NS_FMT_TYPE_DOUBLE ), scale );
      SetDlgItemText( ssd->dlg, id, temp );

      ssd->lock_changes = FALSE;
      }

	_ns_subsample_dialog_update( ssd, id, scale, type );
   }


void _ns_subsample_dialog_verify_constraints( NsSubsampleDialog *ssd )
   {
   EnableWindow(
		GetDlgItem( ssd->dlg, IDC_SUB_DESTINATION_CONSTRAIN_WIDTH_HEIGHT ),
		! ____ns_ssd_curr_fixed_all
      );
   }


void _ns_subsample_dialog_on_constrain_width_height( NsSubsampleDialog *ssd )
   {
   ____ns_ssd_curr_fixed_width_height = ! ____ns_ssd_curr_fixed_width_height;

   _ns_subsample_dialog_verify_constraints( ssd );
   _ns_subsample_dialog_draw_icons( ssd );

   /* NOTE: Can pass either _X_AXIS or _Y_AXIS since they're tied. */

   if( ____ns_ssd_curr_fixed_width_height )
		_ns_subsample_dialog_update( ssd, 0, 100.0, _X_AXIS );
   }


void _ns_subsample_dialog_on_constrain_all( NsSubsampleDialog *ssd )
   {
   ____ns_ssd_curr_fixed_all = ! ____ns_ssd_curr_fixed_all;

   _ns_subsample_dialog_verify_constraints( ssd );
   _ns_subsample_dialog_draw_icons( ssd );

   /* NOTE: Can pass any of the axis types since theyre all tied. */

   if( ____ns_ssd_curr_fixed_all )
		_ns_subsample_dialog_update( ssd, 0, 100.0, _X_AXIS );
   }


void _ns_subsample_dialog_on_ok( NsSubsampleDialog *ssd )
   {
	/* Correct any possible floating point error. */
	ssd->scale_width  = NS_CLAMP( ssd->scale_width, 0.0, 100.0 );
	ssd->scale_height = NS_CLAMP( ssd->scale_height, 0.0, 100.0 );
	ssd->scale_length = NS_CLAMP( ssd->scale_length, 0.0, 100.0 );

   EndDialog( ssd->dlg, 0 );
   }


void _ns_subsample_dialog_on_cancel( NsSubsampleDialog *ssd )
   {
	ssd->cancelled = NS_TRUE;

   EndDialog( ssd->dlg, 0 );
   }


void _ns_subsample_dialog_on_command( NsSubsampleDialog *ssd, nsint id, nsint action )
   {
   if( ssd->lock_changes )
      return;

   switch( id )
      {
      case IDE_SUB_DESTINATION_WIDTH_PIXELS:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_pixels( ssd, IDE_SUB_DESTINATION_WIDTH_PIXELS, _X_AXIS );
         break;

      case IDE_SUB_DESTINATION_WIDTH_PERCENT:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_percent( ssd, IDE_SUB_DESTINATION_WIDTH_PERCENT, _X_AXIS );
         break;

      case IDE_SUB_DESTINATION_HEIGHT_PIXELS:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_pixels( ssd, IDE_SUB_DESTINATION_HEIGHT_PIXELS, _Y_AXIS );
         break;

      case IDE_SUB_DESTINATION_HEIGHT_PERCENT:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_percent( ssd, IDE_SUB_DESTINATION_HEIGHT_PERCENT, _Y_AXIS );
         break;

      case IDE_SUB_DESTINATION_LENGTH_PIXELS:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_pixels( ssd, IDE_SUB_DESTINATION_LENGTH_PIXELS, _Z_AXIS );
         break;

      case IDE_SUB_DESTINATION_LENGTH_PERCENT:
         if( EN_CHANGE == action )
				_ns_subsample_dialog_on_change_percent( ssd, IDE_SUB_DESTINATION_LENGTH_PERCENT, _Z_AXIS );
         break;

      case IDC_SUB_DESTINATION_CONSTRAIN_WIDTH_HEIGHT:
         if( BN_CLICKED == action )
				_ns_subsample_dialog_on_constrain_width_height( ssd );
         break;

      case IDC_SUB_DESTINATION_CONSTRAIN_ALL:
         if( BN_CLICKED == action )
				_ns_subsample_dialog_on_constrain_all( ssd );
         break;

      case IDOK:
         _ns_subsample_dialog_on_ok( ssd );
         break;

      case IDCANCEL:
         _ns_subsample_dialog_on_cancel( ssd );
         break;
      }
   }


extern void CenterWindow( HWND, HWND );

void _ns_subsample_dialog_on_init( NsSubsampleDialog *ssd, HWND dlg )
   {
	ssd->volume_width  = ( nsuint )ns_image_width( ssd->volume );
	ssd->volume_height = ( nsuint )ns_image_height( ssd->volume );
	ssd->volume_length = ( nsuint )ns_image_length( ssd->volume );
	ssd->volume_bpp    = ( nsuint )ns_pixel_used_bits( ns_image_pixel_type( ssd->volume ) );

	ssd->scale_width = ssd->scale_height = ssd->scale_length = 100.0;

	_ns_subsample_dialog_calc_sizes( ssd );

	ssd->dlg = dlg;

	ssd->lock_changes = NS_TRUE;

	SendMessage(
		GetDlgItem( dlg, IDC_SUB_DESTINATION_CONSTRAIN_WIDTH_HEIGHT ),
		BM_SETCHECK,
		____ns_ssd_curr_fixed_width_height ? BST_CHECKED : BST_UNCHECKED,
		0
		);

	SendMessage(
		GetDlgItem( dlg, IDC_SUB_DESTINATION_CONSTRAIN_ALL ),
		BM_SETCHECK,
		____ns_ssd_curr_fixed_all ? BST_CHECKED : BST_UNCHECKED,
		0
		);

	ssd->lock_changes = NS_FALSE;

	_ns_subsample_dialog_verify_constraints( ssd );

   CenterWindow( dlg, ssd->wnd );

   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_WIDTH_PERCENT ),  EM_SETLIMITTEXT, 8, 0 );
   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_HEIGHT_PERCENT ), EM_SETLIMITTEXT, 8, 0 );
   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_LENGTH_PERCENT ), EM_SETLIMITTEXT, 8, 0 );
   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_WIDTH_PIXELS ),   EM_SETLIMITTEXT, 8, 0 );
   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_HEIGHT_PIXELS ),  EM_SETLIMITTEXT, 8, 0 );
   SendMessage( GetDlgItem( dlg, IDE_SUB_DESTINATION_LENGTH_PIXELS ),  EM_SETLIMITTEXT, 8, 0 );

	_ns_subsample_dialog_show_fields( ssd, 0 );
   }


BOOL CALLBACK _ns_subsample_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
   switch( msg )
      {
      case WM_PAINT:
         _ns_subsample_dialog_draw_icons( &____ns_subsample_dialog );
         return FALSE;

      case WM_COMMAND:
         _ns_subsample_dialog_on_command( &____ns_subsample_dialog, LOWORD( wparam ), HIWORD( wparam ) );
			return TRUE;

      case WM_INITDIALOG:
			_ns_subsample_dialog_on_init( &____ns_subsample_dialog, dlg );
			return FALSE;

      case WM_CLOSE:
			_ns_subsample_dialog_on_cancel( &____ns_subsample_dialog );
			return TRUE;
      }

   return FALSE;
   }


nsboolean ns_subsample_dialog
	(
	HINSTANCE       instance,
	HWND            wnd,
	const NsImage  *volume,
	NsVector3d     *scale
	)
	{
	ns_assert( NULL != volume );
	ns_assert( NULL != scale );

	ns_vector3d( scale, -1.0, -1.0, -1.0 );

	ns_memzero( &____ns_subsample_dialog, sizeof( NsSubsampleDialog ) );

	____ns_subsample_dialog.instance = instance;
	____ns_subsample_dialog.wnd      = wnd;
	____ns_subsample_dialog.volume   = volume;

	____ns_ssd_curr_fixed_width_height = ____ns_ssd_prev_fixed_width_height;
	____ns_ssd_curr_fixed_all          = ____ns_ssd_prev_fixed_all;

	DialogBox( instance, "SUBSAMPLE", wnd, ( DLGPROC )_ns_subsample_dialog_proc );

	if( ! ____ns_subsample_dialog.cancelled )
		{
		____ns_ssd_prev_fixed_width_height = ____ns_ssd_curr_fixed_width_height;
		____ns_ssd_prev_fixed_all          = ____ns_ssd_curr_fixed_all;

		ns_vector3d(
			scale,
			____ns_subsample_dialog.scale_width,
			____ns_subsample_dialog.scale_height,
			____ns_subsample_dialog.scale_length
			);
		}

	return ! ____ns_subsample_dialog.cancelled;
	}
