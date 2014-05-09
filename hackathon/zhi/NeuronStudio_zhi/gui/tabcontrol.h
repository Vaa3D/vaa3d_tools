#ifndef __TAB_CONTROL_H__
#define __TAB_CONTROL_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmemory.h>
#include <std/nsprint.h>
#include <windows.h>
#include <commctrl.h>


#define NS_TAB_CONTROL_MAX_SIZE  16

typedef struct _NsTabControl
	{
	HINSTANCE     instance;
	HWND          owner;
	nsint         id;
	nssize        size;
	HWND          current;
	nssize        index;
	DLGTEMPLATE*  templates[ NS_TAB_CONTROL_MAX_SIZE ];
	DLGPROC       functions[ NS_TAB_CONTROL_MAX_SIZE ];
	}
	NsTabControl;


NS_IMPEXP void ns_tab_control_init
	(
	NsTabControl  *tc,
	HINSTANCE      instance,
	HWND           owner,
	nsint          id,
	nssize         size
	);

NS_IMPEXP NsError ns_tab_control_add
	(
	NsTabControl  *tc,
	const nschar  *text,
	const nschar  *tmplate,
	DLGPROC        proc
	);

NS_IMPEXP NsError ns_tab_control_on_select( NsTabControl *tc );

#endif /* __TAB_CONTROL_H__ */
