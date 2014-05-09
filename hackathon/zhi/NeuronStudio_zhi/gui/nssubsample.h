#ifndef __NS_GUI_SUBSAMPLE_H__
#define __NS_GUI_SUBSAMPLE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsmisc.h>
#include <math/nsvector.h>
#include <image/nsimage.h>
#include <ext/subsample.h>

#include <windows.h>
#include "resource.h"

NS_DECLS_BEGIN

NS_IMPEXP nsboolean ns_subsample_dialog
	(
	HINSTANCE       instance,
	HWND            wnd,
	const NsImage  *volume,
	NsVector3d     *scale
	);

NS_DECLS_END

#endif/* __NS_GUI_SUBSAMPLE_H__ */
