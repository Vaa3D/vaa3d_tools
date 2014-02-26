#include "statusbar.h"


//#define __STATUS_BAR_ZOOM_PART_WIDTH       74
//#define __STATUS_BAR_INFO_PART_WIDTH      296
//#define __STATUS_BAR_INFO_PART_WIDTH  74
//#define __STATUS_BAR_PROGRESS_PART_WIDTH   0


static HWND s_hStatusBarOwner = NULL;
HWND s_hStatusBar = NULL;


HWND GetStatusBar( void )
   {  return s_hStatusBar;  }


nsboolean CreateStatusBar( HWND hWnd, HINSTANCE hInstance )
   {
	s_hStatusBarOwner = hWnd;

   s_hStatusBar = 
       CreateWindowEx( 0,
                       STATUSCLASSNAME,
                       NULL,
                       WS_CHILD | WS_VISIBLE,
                       0,
                       0,
                       0,
                       0,
                       hWnd,
                       NULL,
                       hInstance,
                       NULL
                      );

   return ( ( NULL != s_hStatusBar ) ? NS_TRUE : NS_FALSE );
   }


void ResizeStatusBar( WPARAM wParam, LPARAM lParam )
   {
   SendMessage( s_hStatusBar, WM_SIZE, wParam, lParam );
   SetStatusBarParts();
   }


nsint ____status_lengths[ STATUS_BAR_NUM_PARTS ];
nsint ____status_coords[ STATUS_BAR_NUM_PARTS ];


/* NOTE: Using SendMessageCallback() instead of SendMessage() since I noticed that
	performance is seriously degraded when calling these functions from multiple
	threads, specifically from a thread that did not create the status bar. The callback
	avoids the delay of waiting for SendMessage() to return, but it makes synchronization
	more complex. Specifically when setting strings, we must allocate a string on the
	heap and free it with a callback function. See also _do_set_status_bar_text(). */


void SetStatusBarParts( void )
   {
   RECT   rc;
	nsint  i, length;


   GetClientRect( s_hStatusBarOwner, &rc );

	length = 0;
	for( i = 1; i < STATUS_BAR_NUM_PARTS; ++i )
		length += ____status_lengths[i];

   ____status_coords[ STATUS_BAR_MSG ] = rc.right - length - 46;

	____status_coords[ STATUS_BAR_FILE ] =
		____status_coords[ STATUS_BAR_FILE - 1 ] + ____status_lengths[ STATUS_BAR_FILE ];

	____status_coords[ STATUS_BAR_SLICE ] =
		____status_coords[ STATUS_BAR_SLICE - 1 ] + ____status_lengths[ STATUS_BAR_SLICE ];

	____status_coords[ STATUS_BAR_ZOOM ] =
		____status_coords[ STATUS_BAR_ZOOM - 1 ] + ____status_lengths[ STATUS_BAR_ZOOM ];

   ____status_coords[ STATUS_BAR_PROGRESS ] =
		____status_coords[ STATUS_BAR_PROGRESS - 1 ] + ____status_lengths[ STATUS_BAR_PROGRESS ];

   ____status_coords[ STATUS_BAR_LENGTH ] =
      ____status_coords[ STATUS_BAR_LENGTH - 1 ] + ____status_lengths[ STATUS_BAR_LENGTH ];

   ____status_coords[ STATUS_BAR_WIDTH ] =
      ____status_coords[ STATUS_BAR_WIDTH - 1 ] + ____status_lengths[ STATUS_BAR_WIDTH ];

   ____status_coords[ STATUS_BAR_VOLUME ] =
      ____status_coords[ STATUS_BAR_VOLUME - 1 ] + ____status_lengths[ STATUS_BAR_VOLUME ];

   ____status_coords[ STATUS_BAR_SURFACE_AREA ] =
      ____status_coords[ STATUS_BAR_SURFACE_AREA - 1 ] + ____status_lengths[ STATUS_BAR_SURFACE_AREA ];

	SendMessageCallback(
		s_hStatusBar, 
		SB_SETPARTS, 
		( WPARAM )STATUS_BAR_NUM_PARTS,
		( LPARAM )____status_coords,
		NULL,
		( ULONG_PTR )0
		);
   }


VOID CALLBACK _free_status_bar_text( HWND hwnd, UINT umsg, ULONG_PTR data, LRESULT result )
	{  ns_free( ( nspointer )data );  }


void _do_set_status_bar_text( const nschar *orig_text, nsint which, nsboolean fixed )
	{
	nschar *dup_text = NULL;

   ns_assert( NULL != orig_text );
	ns_assert( which < STATUS_BAR_NUM_PARTS );

	dup_text = ns_ascii_strdup( orig_text );
   SendMessageCallback( s_hStatusBar, SB_SETTEXT, which, ( LPARAM )dup_text, _free_status_bar_text, ( ULONG_PTR )dup_text );

	if( ! fixed )
		____status_lengths[ which ] = ( nsint )( ns_ascii_strlen( orig_text ) * STATUS_BAR_TEXT_MULTIPLIER );

	SetStatusBarParts();
	}


void SetStatusBarText( const nschar *text, nsint which )
   {  _do_set_status_bar_text( text, which, NS_FALSE );  }


void _do_status_bar_print( nsint which, const nschar *format, ns_va_list args, nsboolean fixed )
	{
	nschar text[ 256 ];

	ns_vsnprint( text, NS_ARRAY_LENGTH( text ), format, args );
	_do_set_status_bar_text( text, which, fixed );
	}


void status_bar_print( nsint which, const nschar *format, ... )
	{
	ns_va_list args;

	ns_va_start( args, format );
	_do_status_bar_print( which, format, args, NS_FALSE );
	ns_va_end( args );
	}


void status_bar_print_fixed( nsint which, const nschar *format, ... )
	{
	ns_va_list args;

	ns_va_start( args, format );
	_do_status_bar_print( which, format, args, NS_TRUE );
	ns_va_end( args );
	}
