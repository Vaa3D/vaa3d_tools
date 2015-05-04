// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__DBABA2F7_C298_4A5E_8104_5607EF370F01__INCLUDED_)
#define AFX_STDAFX_H__DBABA2F7_C298_4A5E_8104_5607EF370F01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

/*
 *			mexmacro.h
 *
 *	This header contains modifications neccessary for Mex files
 */

#include "windows.h"

#ifndef _mexmacro_h
#define _mexmacro_h

#include <mex.h>


#define PRINTF mexPrintf
#define PRINTFOUT mexPrintf
#define PRINTFERR mexPrintf
#define PRINTFSTDOUT mexPrintf
#define PRINTFSTDERR mexPrintf
#define PRINTFDEBUG mexPrintf
#define FREE(_ptr,_size) mxFree(_ptr)
#define CALLOC mxCalloc
#define MALLOC mxMalloc
#define REALLOC mxRealloc
#define MEMCHECK(_ptr,_string,_size) 0
#define clean_up(_int) mexErrMsgTxt("")

#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__DBABA2F7_C298_4A5E_8104_5607EF370F01__INCLUDED_)
