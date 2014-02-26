#include "tabcontrol.h"


void ns_tab_control_init
	(
	NsTabControl  *tc,
	HINSTANCE      instance,
	HWND           owner,
	nsint          id,
	nssize         size
	)
	{
	ns_assert( NULL != tc );
	ns_assert( size <= NS_TAB_CONTROL_MAX_SIZE );

	ns_memzero( tc, sizeof( NsTabControl ) );

	tc->instance = instance;
	tc->owner    = owner;
	tc->id       = id;
	tc->size     = size;
	tc->current  = NULL;
	tc->index    = 0;
	}


NsError ns_tab_control_add
	(
	NsTabControl  *tc,
	const nschar  *text,
	const nschar  *tmplate,
	DLGPROC        proc
	)
	{
	HWND     tab;
	TCITEM   t;
	HRSRC    hrsrc;
	HGLOBAL  hglb;


	ns_assert( NULL != tc );
	ns_assert( NULL != text );
	ns_assert( NULL != tmplate );
	ns_assert( tc->index < tc->size );

	if( NULL == ( tab = GetDlgItem( tc->owner, tc->id ) ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	t.mask    = TCIF_TEXT;
	t.pszText = ( nschar* )text;

	if( -1 == TabCtrl_InsertItem( tab, tc->index, &t ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NULL == ( hrsrc = FindResource( NULL, tmplate, RT_DIALOG ) ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

   if( NULL == ( hglb = LoadResource( tc->instance, hrsrc ) ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
 
	tc->templates[ tc->index ] = ( DLGTEMPLATE* )LockResource( hglb );
	tc->functions[ tc->index ] = proc;

	++(tc->index);

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_tab_control_center( HWND owner, HWND tab, HWND current )
   {
	RECT   screen, tab_client, curr_client;
	POINT  lt, rb;
	nsint  x, y;


   if( ! GetWindowRect( tab, &screen ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	lt.x = screen.left;
	lt.y = screen.top;
	rb.x = screen.right;
	rb.y = screen.bottom;

	if( ! ScreenToClient( owner, &lt ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
	
	if( ! ScreenToClient( owner, &rb ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	tab_client.left   = lt.x;
	tab_client.top    = lt.y;
	tab_client.right  = rb.x;
	tab_client.bottom = rb.y;

	if( ! GetClientRect( current, &curr_client ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

   x = tab_client.left +
		 ( tab_client.right - tab_client.left ) / 2 - 
       ( curr_client.right - curr_client.left ) / 2;

	/* NOTE: The +18 is to account for the height of the tab. i.e.
		the display where the text for the tab is seen. */
   y = tab_client.top +
		 ( tab_client.bottom - tab_client.top + 18 ) / 2 - 
       ( curr_client.bottom - curr_client.top ) / 2;

	if( ! SetWindowPos( current, HWND_TOP, x, y, 0, 0, SWP_NOSIZE ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	return ns_no_error();
   }


NsError ns_tab_control_on_select( NsTabControl *tc )
	{
	HWND     tab;
	nsint    which;
	NsError  error;


	if( NULL == ( tab = GetDlgItem( tc->owner, tc->id ) ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );		

	if( -1 == ( which = TabCtrl_GetCurSel( tab ) ) )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
 
	if( NULL != tc->current )
		DestroyWindow( tc->current );
 
	tc->current =
		CreateDialogIndirect(
			tc->instance,
			tc->templates[ which ],
			tc->owner,
			tc->functions[ which ]
			);

	if( NULL == tc->current )
		return ns_error_os( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NS_FAILURE( _ns_tab_control_center( tc->owner, tab, tc->current ), error ) )
		return error;

	ShowWindow( tc->current, SW_SHOW );

	return ns_no_error();
	}
