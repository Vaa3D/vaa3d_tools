
NS_PRIVATE HWND       ____ns_bc_dialog_wnd;
NS_PRIVATE nsdouble   ____ns_bc_dialog_brightness;
NS_PRIVATE nsdouble   ____ns_bc_dialog_contrast;
NS_PRIVATE nsboolean  ____ns_bc_dialog_cancelled;
NS_PRIVATE nsboolean  ____ns_bc_dialog_lock_changes;


#define _NS_BC_DIALOG_INT_VAL_MIN  -100
#define _NS_BC_DIALOG_INT_VAL_MAX   100

#define _NS_BC_DIALOG_TRACK_MIN  0
#define _NS_BC_DIALOG_TRACK_MAX  200
#define _NS_BC_DIALOG_TRACK_MID  100


void _ns_bc_dialog_on_init( HWND dlg )
	{
	HWND ctrl;

	____ns_bc_dialog_lock_changes = NS_TRUE;

	ctrl = GetDlgItem( dlg, IDS_BRIGHTNESS_VALUE );
	SendMessage( ctrl, TBM_SETRANGE, FALSE, MAKELONG( _NS_BC_DIALOG_TRACK_MIN, _NS_BC_DIALOG_TRACK_MAX ) );
	SendMessage( ctrl, TBM_SETPOS, TRUE, _NS_BC_DIALOG_TRACK_MID );
	//SendMessage( ctrl, TBM_SETTIC, 0, _NS_BC_DIALOG_TRACK_MID );

	ctrl = GetDlgItem( dlg, IDS_CONTRAST_VALUE );
	SendMessage( ctrl, TBM_SETRANGE, FALSE, MAKELONG( _NS_BC_DIALOG_TRACK_MIN, _NS_BC_DIALOG_TRACK_MAX ) );
	SendMessage( ctrl, TBM_SETPOS, TRUE, _NS_BC_DIALOG_TRACK_MID );
	//SendMessage( ctrl, TBM_SETTIC, 0, _NS_BC_DIALOG_TRACK_MID );

	SetDlgItemText( dlg, IDE_BRIGHTNESS_VALUE, "0" );
	SetDlgItemText( dlg, IDE_CONTRAST_VALUE, "0" );

	____ns_bc_dialog_lock_changes = NS_FALSE;

	CenterWindow( dlg, ____ns_bc_dialog_wnd );
	}


void _ns_bc_dialog_on_ok( HWND dlg )
   {
	/* Correct any possible floating point error. */
	____ns_bc_dialog_brightness = NS_CLAMP( ____ns_bc_dialog_brightness, NS_BRIGHTNESS_MIN, NS_BRIGHTNESS_MAX );
	____ns_bc_dialog_contrast   = NS_CLAMP( ____ns_bc_dialog_contrast, NS_CONTRAST_MIN, NS_CONTRAST_MAX );

   EndDialog( dlg, 1 );
   }


void _ns_bc_dialog_on_cancel( HWND dlg )
	{
	____ns_bc_dialog_cancelled = NS_TRUE;
	EndDialog( dlg, 0 );
	}


#define _NS_BC_DIALOG_CONV_TO_TRACK( val )\
	( (val) + _NS_BC_DIALOG_TRACK_MID )


#define _NS_BC_DIALOG_CONV_FROM_TRACK( val )\
	( (val) - _NS_BC_DIALOG_TRACK_MID )


void _ns_bc_dialog_on_change_by_text( HWND dlg, nsdouble *value, nsdouble min, nsdouble max, nsint ids, nsint ide )
   {
	NsLerpd  lerp;
   nsint    val;
   nschar   temp[ 128 ] = { '\0' };


   GetDlgItemText( dlg, ide, temp, sizeof( temp ) - 1 );

   val = ( nsuint )ns_atoi( temp );

	____ns_bc_dialog_lock_changes = NS_TRUE;

   if( val < _NS_BC_DIALOG_INT_VAL_MIN || _NS_BC_DIALOG_INT_VAL_MAX < val )
      {
		if( val < _NS_BC_DIALOG_INT_VAL_MIN )
			val = _NS_BC_DIALOG_INT_VAL_MIN;
		else if( _NS_BC_DIALOG_INT_VAL_MAX < val )
			val = _NS_BC_DIALOG_INT_VAL_MAX;

      ns_snprint( temp, sizeof( temp ), NS_FMT_INT, val );
      SetDlgItemText( dlg, ide, temp );
      }

	SendMessage( GetDlgItem( dlg, ids ), TBM_SETPOS, TRUE, _NS_BC_DIALOG_CONV_TO_TRACK( val ) );

	____ns_bc_dialog_lock_changes = NS_FALSE;

	ns_lerpd_init( &lerp, ( nsdouble )_NS_BC_DIALOG_INT_VAL_MIN, min, ( nsdouble )_NS_BC_DIALOG_INT_VAL_MAX, max );
	*value = ns_lerpd( &lerp, ( nsdouble )val );
   }


void _ns_bc_dialog_on_change_brightness_by_text( HWND dlg )
	{
	_ns_bc_dialog_on_change_by_text(
		dlg,
		&____ns_bc_dialog_brightness,
		NS_BRIGHTNESS_MIN,
		NS_BRIGHTNESS_MAX,
		IDS_BRIGHTNESS_VALUE,
		IDE_BRIGHTNESS_VALUE
		);
	}


void _ns_bc_dialog_on_change_contrast_by_text( HWND dlg )
	{
	_ns_bc_dialog_on_change_by_text(
		dlg,
		&____ns_bc_dialog_contrast,
		NS_CONTRAST_MIN,
		NS_CONTRAST_MAX,
		IDS_CONTRAST_VALUE,
		IDE_CONTRAST_VALUE
		);
	}


void _ns_bc_dialog_on_command( HWND dlg, nsint id, nsint action )
	{
   if( ____ns_bc_dialog_lock_changes )
      return;

   switch( id )
      {
      case IDE_BRIGHTNESS_VALUE:
         if( EN_CHANGE == action )
				_ns_bc_dialog_on_change_brightness_by_text( dlg );
         break;

      case IDE_CONTRAST_VALUE:
         if( EN_CHANGE == action )
				_ns_bc_dialog_on_change_contrast_by_text( dlg );
         break;

      case IDOK:
         _ns_bc_dialog_on_ok( dlg );
         break;

      case IDCANCEL:
         _ns_bc_dialog_on_cancel( dlg );
         break;
      }
	}


void _ns_bc_dialog_on_change_by_slider( HWND dlg, nsint val, nsdouble *value, nsdouble min, nsdouble max, nsint ide )
   {
	NsLerpd  lerp;
   nschar   temp[ 128 ] = { '\0' };


	____ns_bc_dialog_lock_changes = NS_TRUE;

	ns_snprint( temp, sizeof( temp ), NS_FMT_INT, _NS_BC_DIALOG_CONV_FROM_TRACK( val ) );
	SetDlgItemText( dlg, ide, temp );

	____ns_bc_dialog_lock_changes = NS_FALSE;

	ns_lerpd_init( &lerp, ( nsdouble )_NS_BC_DIALOG_TRACK_MIN, min, ( nsdouble )_NS_BC_DIALOG_TRACK_MAX, max );
	*value = ns_lerpd( &lerp, ( nsdouble )val );
   }


void _ns_bc_dialog_on_change_brightness_by_slider( HWND dlg, nsint val )
	{
	_ns_bc_dialog_on_change_by_slider(
		dlg,
		val,
		&____ns_bc_dialog_brightness,
		NS_BRIGHTNESS_MIN,
		NS_BRIGHTNESS_MAX,
		IDE_BRIGHTNESS_VALUE
		);
	}


void _ns_bc_dialog_on_change_contrast_by_slider( HWND dlg, nsint val )
	{
	_ns_bc_dialog_on_change_by_slider(
		dlg,
		val,
		&____ns_bc_dialog_contrast,
		NS_CONTRAST_MIN,
		NS_CONTRAST_MAX,
		IDE_CONTRAST_VALUE
		);
	}


void _ns_bc_dialog_on_hscroll( HWND dlg, nsint code, nsint val, HWND ctrl )
	{
	if( SB_THUMBPOSITION != code && SB_THUMBTRACK != code )
		val = ( nsint )( SendMessage( ctrl, TBM_GETPOS, 0, 0 ) );

	if( ctrl == GetDlgItem( dlg, IDS_BRIGHTNESS_VALUE ) )
		_ns_bc_dialog_on_change_brightness_by_slider( dlg, val );
	else if( ctrl == GetDlgItem( dlg, IDS_CONTRAST_VALUE ) )
		_ns_bc_dialog_on_change_contrast_by_slider( dlg, val );
	}


INT_PTR CALLBACK _ns_bc_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
	switch( msg )
		{
     case WM_HSCROLL:
			_ns_bc_dialog_on_hscroll( dlg, LOWORD( wparam ), HIWORD( wparam ), ( HWND )lparam );
			return TRUE;

      case WM_COMMAND:
         _ns_bc_dialog_on_command( dlg, LOWORD( wparam ), HIWORD( wparam ) );
			return TRUE;

		case WM_INITDIALOG:
			_ns_bc_dialog_on_init( dlg );
			return FALSE;

      case WM_CLOSE:
         _ns_bc_dialog_on_cancel( dlg );
         return TRUE;
		}

	return FALSE;
	}


nsboolean ns_brightness_contrast_dialog
	(
	HINSTANCE   instance,
	HWND        wnd,
	nsdouble   *brightness,
	nsdouble   *contrast
	)
	{
	ns_assert( NULL != brightness );
	ns_assert( NULL != contrast );

	*brightness = *contrast = 0.0;

	____ns_bc_dialog_wnd          = wnd;
	____ns_bc_dialog_brightness   = 0.0;
	____ns_bc_dialog_contrast     = 0.0;
	____ns_bc_dialog_cancelled    = NS_FALSE;
	____ns_bc_dialog_lock_changes = NS_FALSE;

	DialogBox( instance, "BRIGHTNESS_CONTRAST", wnd, _ns_bc_dialog_proc );

	if( ! ____ns_bc_dialog_cancelled )
		{
		*brightness = ____ns_bc_dialog_brightness;
		*contrast   = ____ns_bc_dialog_contrast;
		}

	return ! ____ns_bc_dialog_cancelled;
	}
