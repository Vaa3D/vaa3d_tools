
NS_PRIVATE HWND       ____ns_dr_dialog_wnd;
NS_PRIVATE nsfloat    ____ns_dr_dialog_channel_min;
NS_PRIVATE nsfloat    ____ns_dr_dialog_channel_max;
NS_PRIVATE nsboolean  ____ns_dr_dialog_cancelled;
NS_PRIVATE nsboolean  ____ns_dr_dialog_lock_changes;


void _ns_dr_dialog_on_init( HWND dlg )
	{
	HWND ctrl;

	____ns_dr_dialog_lock_changes = NS_TRUE;

	ctrl = GetDlgItem( dlg, IDS_DYN_RANGE_MIN_VALUE );
	SendMessage( ctrl, TBM_SETRANGE, FALSE, MAKELONG( 0, 255 ) );
	SendMessage( ctrl, TBM_SETPOS, TRUE, 0 );

	ctrl = GetDlgItem( dlg, IDS_DYN_RANGE_MAX_VALUE );
	SendMessage( ctrl, TBM_SETRANGE, FALSE, MAKELONG( 0, 255 ) );
	SendMessage( ctrl, TBM_SETPOS, TRUE, 255 );

	SetDlgItemText( dlg, IDE_DYN_RANGE_MIN_VALUE, "0" );
	SetDlgItemText( dlg, IDE_DYN_RANGE_MAX_VALUE, "255" );

	____ns_dr_dialog_lock_changes = NS_FALSE;

	CenterWindow( dlg, ____ns_dr_dialog_wnd );
	}


void _ns_dr_dialog_on_ok( HWND dlg )
   {
	/* Correct any possible floating point error. */
	____ns_dr_dialog_channel_min = NS_CLAMP( ____ns_dr_dialog_channel_min, 0.0f, 255.0f );
	____ns_dr_dialog_channel_max = NS_CLAMP( ____ns_dr_dialog_channel_max, 0.0f, 255.0f );

	/* Swap them if maximum is less than the minimum. */
	if( ____ns_dr_dialog_channel_max < ____ns_dr_dialog_channel_min )
		NS_SWAP( nsfloat, ____ns_dr_dialog_channel_min, ____ns_dr_dialog_channel_max );

   EndDialog( dlg, 1 );
   }


void _ns_dr_dialog_on_cancel( HWND dlg )
	{
	____ns_dr_dialog_cancelled = NS_TRUE;
	EndDialog( dlg, 0 );
	}


void _ns_dr_dialog_on_change_by_text( HWND dlg, nsfloat *value, nsint ids, nsint ide )
   {
   nsint   val;
   nschar  temp[ 128 ] = { '\0' };


   GetDlgItemText( dlg, ide, temp, sizeof( temp ) - 1 );

   val = ( nsuint )ns_atoi( temp );

	____ns_dr_dialog_lock_changes = NS_TRUE;

   if( val < 0 || 255 < val )
      {
		if( val < 0 )
			val = 0;
		else if( 255 < val )
			val = 255;

      ns_snprint( temp, sizeof( temp ), NS_FMT_INT, val );
      SetDlgItemText( dlg, ide, temp );
      }

	SendMessage( GetDlgItem( dlg, ids ), TBM_SETPOS, TRUE, val );

	____ns_dr_dialog_lock_changes = NS_FALSE;

	*value = ( nsfloat )val;
   }


void _ns_dr_dialog_on_change_channel_min_by_text( HWND dlg )
	{
	_ns_dr_dialog_on_change_by_text(
		dlg,
		&____ns_dr_dialog_channel_min,
		IDS_DYN_RANGE_MIN_VALUE,
		IDE_DYN_RANGE_MIN_VALUE
		);
	}


void _ns_dr_dialog_on_change_channel_max_by_text( HWND dlg )
	{
	_ns_dr_dialog_on_change_by_text(
		dlg,
		&____ns_dr_dialog_channel_max,
		IDS_DYN_RANGE_MAX_VALUE,
		IDE_DYN_RANGE_MAX_VALUE
		);
	}


void _ns_dr_dialog_on_command( HWND dlg, nsint id, nsint action )
	{
   if( ____ns_dr_dialog_lock_changes )
      return;

   switch( id )
      {
      case IDE_DYN_RANGE_MIN_VALUE:
         if( EN_CHANGE == action )
				_ns_dr_dialog_on_change_channel_min_by_text( dlg );
         break;

      case IDE_DYN_RANGE_MAX_VALUE:
         if( EN_CHANGE == action )
				_ns_dr_dialog_on_change_channel_max_by_text( dlg );
         break;

      case IDOK:
         _ns_dr_dialog_on_ok( dlg );
         break;

      case IDCANCEL:
         _ns_dr_dialog_on_cancel( dlg );
         break;
      }
	}


void _ns_dr_dialog_on_change_by_slider( HWND dlg, nsint val, nsfloat *value, nsint ide )
   {
   nschar temp[ 128 ] = { '\0' };

	____ns_dr_dialog_lock_changes = NS_TRUE;

	ns_snprint( temp, sizeof( temp ), NS_FMT_INT, val );
	SetDlgItemText( dlg, ide, temp );

	____ns_dr_dialog_lock_changes = NS_FALSE;

	*value = ( nsfloat )val;
   }


void _ns_dr_dialog_on_change_channel_min_by_slider( HWND dlg, nsint val )
	{
	_ns_dr_dialog_on_change_by_slider(
		dlg,
		val,
		&____ns_dr_dialog_channel_min,
		IDE_DYN_RANGE_MIN_VALUE
		);
	}


void _ns_dr_dialog_on_change_channel_max_by_slider( HWND dlg, nsint val )
	{
	_ns_dr_dialog_on_change_by_slider(
		dlg,
		val,
		&____ns_dr_dialog_channel_max,
		IDE_DYN_RANGE_MAX_VALUE
		);
	}


void _ns_dr_dialog_on_hscroll( HWND dlg, nsint code, nsint val, HWND ctrl )
	{
	if( SB_THUMBPOSITION != code && SB_THUMBTRACK != code )
		val = ( nsint )( SendMessage( ctrl, TBM_GETPOS, 0, 0 ) );

	if( ctrl == GetDlgItem( dlg, IDS_DYN_RANGE_MIN_VALUE ) )
		_ns_dr_dialog_on_change_channel_min_by_slider( dlg, val );
	else if( ctrl == GetDlgItem( dlg, IDS_DYN_RANGE_MAX_VALUE ) )
		_ns_dr_dialog_on_change_channel_max_by_slider( dlg, val );
	}


INT_PTR CALLBACK _ns_dr_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
	switch( msg )
		{
     case WM_HSCROLL:
			_ns_dr_dialog_on_hscroll( dlg, LOWORD( wparam ), HIWORD( wparam ), ( HWND )lparam );
			return TRUE;

      case WM_COMMAND:
         _ns_dr_dialog_on_command( dlg, LOWORD( wparam ), HIWORD( wparam ) );
			return TRUE;

		case WM_INITDIALOG:
			_ns_dr_dialog_on_init( dlg );
			return FALSE;

      case WM_CLOSE:
         _ns_dr_dialog_on_cancel( dlg );
         return TRUE;
		}

	return FALSE;
	}


nsboolean ns_dynamic_range_dialog
	(
	HINSTANCE   instance,
	HWND        wnd,
	nsfloat    *channel_min,
	nsfloat    *channel_max
	)
	{
	ns_assert( NULL != channel_min );
	ns_assert( NULL != channel_max );

	*channel_min = 0.0f;
	*channel_max = 255.0f;

	____ns_dr_dialog_wnd          = wnd;
	____ns_dr_dialog_channel_min  = 0.0f;
	____ns_dr_dialog_channel_max  = 255.0f;
	____ns_dr_dialog_cancelled    = NS_FALSE;
	____ns_dr_dialog_lock_changes = NS_FALSE;

	DialogBox( instance, "DYNAMIC_RANGE", wnd, _ns_dr_dialog_proc );

	if( ! ____ns_dr_dialog_cancelled )
		{
		*channel_min = ____ns_dr_dialog_channel_min;
		*channel_max = ____ns_dr_dialog_channel_max;
		}

	return ! ____ns_dr_dialog_cancelled;
	}
