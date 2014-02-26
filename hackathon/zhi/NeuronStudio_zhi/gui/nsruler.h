#ifndef __NS_GUI_RULER_H__
#define __NS_GUI_RULER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nslist.h>
#include <math/nsvector.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_RULER_MODE_FIRST_CONTROL,
	NS_RULER_MODE_DRAG_CONTROL,
	NS_RULER_MODE_DRAG_RULER,
	NS_RULER_MODE_IDLE
	}
	NsRulerModeType;


typedef enum
	{
	NS_RULER_CURSOR_STANDARD,
	NS_RULER_CURSOR_OVER_CONTROL,
	NS_RULER_CURSOR_OVER_LINE,

	/* Internal. DO NOT USE! */
	_NS_RULER_CURSOR_SENTINEL 
	}
	NsRulerCursorType;

#define NS_RULER_NUM_CURSORS  ( ( nsint )NS_RULER_CURSOR_SENTINEL )


typedef enum
	{
	NS_RULER_MOUSE_MOVE,
	NS_RULER_MOUSE_LBUTTONDOWN,
	NS_RULER_MOUSE_LBUTTONUP,
	NS_RULER_MOUSE_LBUTTONDBLCLK
	}
	NsRulerMouseType;


typedef struct _NsRuler
	{
	NsList             controls;
	nslistiter         active_control;
	NsRulerModeType    mode;
	NsRulerCursorType  cursor;
	}
	NsRuler;


NS_IMPEXP void ns_ruler_construct( NsRuler *ruler );
NS_IMPEXP void ns_ruler_destruct( NsRuler *ruler );

NS_IMPEXP void ns_ruler_on_mouse
	( 
	NsRuler           *ruler,
   nsint              x, 
   nsint              y,
   NsRulerMouseType   type
   );

NS_IMPEXP void ns_ruler_draw( const NsRuler *ruler );
NS_IMPEXP void ns_ruler_erase( NsRuler *ruler );

NS_IMPEXP nsfloat ns_ruler_length( NsRuler *ruler );

NS_DECLS_END

#endif/* __NS_GUI_RULER_H__ */
