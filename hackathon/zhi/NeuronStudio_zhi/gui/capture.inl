

eboolean _app_init_screen_capture( void )
	{
	eboolean ok;

	if( NULL == ( ____app_screen_dc = GetDC( NULL ) ) )
		return E_FALSE;

	____app_buffer_dc     = NULL;
	____app_new_bitmap    = NULL;
	____app_old_bitmap    = NULL;
	____app_screen_width  = e_type_cast( eint, GetSystemMetrics( SM_CXSCREEN ) );
	____app_screen_height = e_type_cast( eint, GetSystemMetrics( SM_CYSCREEN ) );

	/* NOTE: Assume 24-bit RGB pixels. */
	____app_bytes_per_row = e_type_cast( esize, ____app_screen_width ) * E_SIZE_C(3);

	/* Remember Windows expect the rows of the buffer to be aligned to LONG boundaries. */
	____app_bytes_per_row = E_ALIGN( ____app_bytes_per_row, sizeof( LONG ) );
	____app_total_bytes   = ____app_bytes_per_row * ____app_screen_height;

	if( NULL == ( ____app_pixels = e_malloc( ____app_total_bytes + e_sizeof( ecookie ) ) ) )
		return E_FALSE;

	e_cookie_set( ____app_pixels, ____app_total_bytes );

	____app_buffer_dc  = CreateCompatibleDC( ____app_screen_dc );
	____app_new_bitmap = CreateCompatibleBitmap( ____app_screen_dc, ____app_screen_width, ____app_screen_height );

	if( NULL != ____app_buffer_dc && NULL != ____app_new_bitmap )
		{
		____app_old_bitmap = SelectObject( ____app_buffer_dc, ____app_new_bitmap );

		ok = E_TRUE;
		}
	else
		{
		if( NULL != ____app_new_bitmap )
			DeleteObject( ____app_new_bitmap );

		if( NULL != ____app_buffer_dc )
			DeleteDC( ____app_buffer_dc );

		____app_buffer_dc  = NULL;
		____app_new_bitmap = NULL;

		ok = E_FALSE;
		}

	if( ok )
		{
		____app_bitmap_info.bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
		____app_bitmap_info.bmiHeader.biWidth         = ____app_screen_width;
		____app_bitmap_info.bmiHeader.biHeight        = -____app_screen_height;
		____app_bitmap_info.bmiHeader.biPlanes        = 1;
		____app_bitmap_info.bmiHeader.biBitCount      = 24;
		____app_bitmap_info.bmiHeader.biCompression   = BI_RGB;
		____app_bitmap_info.bmiHeader.biSizeImage     = 0;
		____app_bitmap_info.bmiHeader.biXPelsPerMeter = 0;
		____app_bitmap_info.bmiHeader.biYPelsPerMeter = 0;
		____app_bitmap_info.bmiHeader.biClrUsed       = 0;
		____app_bitmap_info.bmiHeader.biClrImportant  = 0;
		}

	return ok;
	}


void _app_finalize_screen_capture( void )
	{
	if( NULL != ____app_buffer_dc )
		{
		SelectObject( ____app_buffer_dc, ____app_old_bitmap );

		if( NULL != ____app_new_bitmap )
			DeleteObject( ____app_new_bitmap );

		DeleteDC( ____app_buffer_dc );
		}

	____app_buffer_dc     = NULL;
	____app_new_bitmap    = NULL;
	____app_old_bitmap    = NULL;
	____app_screen_width  = E_INT_C(0);
	____app_screen_height = E_INT_C(0);

	if( NULL != ____app_pixels )
		{
		e_free( ____app_pixels );
		____app_pixels = NULL;
		}

	____app_bytes_per_row = E_SIZE_C(0);
	____app_total_bytes   = E_SIZE_C(0);

	____app_screen_dc = NULL;
	}


EError _app_run_screen_capture( void )
	{
	BitBlt(
		____app_buffer_dc,
		0, 0,
		____app_screen_width,
		____app_screen_height,
		____app_screen_dc,
		0, 0,
		SRCCOPY
		);

	GetDIBits(
		____app_buffer_dc,
		____app_new_bitmap,
		0,
		e_type_cast( UINT, ____app_screen_height ),
		____app_pixels,
		&____app_bitmap_info,
		DIB_RGB_COLORS
		);

	if( ! e_cookie_valid( ____app_pixels, ____app_total_bytes ) )
		e_abort();

	return e_no_error();
	}
