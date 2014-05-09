#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsprint.h>
#include <std/nsascii.h>
#include <windows.h>
#include <commctrl.h>


extern HWND s_hStatusBar;

HWND GetStatusBar( void );

nsboolean CreateStatusBar( HWND hWnd, HINSTANCE hInstance );

void ResizeStatusBar( WPARAM wParam, LPARAM lParam );
void SetStatusBarParts( void );


enum{
	STATUS_BAR_MSG,
	STATUS_BAR_FILE,
	STATUS_BAR_SLICE,
	STATUS_BAR_ZOOM,
	STATUS_BAR_PROGRESS,
	STATUS_BAR_LENGTH,
	STATUS_BAR_WIDTH,
	STATUS_BAR_VOLUME,
	STATUS_BAR_SURFACE_AREA,
	
	STATUS_BAR_NUM_PARTS
	};


#define STATUS_BAR_SEED  STATUS_BAR_LENGTH
#define STATUS_BAR_ROI   STATUS_BAR_LENGTH

void SetStatusBarText( const nschar* message, nsint which );

#define STATUS_BAR_TEXT_MULTIPLIER  8 

void status_bar_print( nsint which, const nschar *format, ... );
void status_bar_print_fixed( nsint which, const nschar *format, ... );

#endif /* __STATUSBAR_H__ */
