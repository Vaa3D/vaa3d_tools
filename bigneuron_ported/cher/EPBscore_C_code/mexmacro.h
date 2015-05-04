/*
 *			mexmacro.h
 *
 *	This header contains modifications neccessary for Mex files
 */

#ifndef _mexmacro_h
#define _mexmacro_h


#define PRINTF mexPrintf
#define PRINTFOUT mexPrintf
#define PRINTFERR mexPrintf
#define PRINTFSTDOUT mexPrintf
#define PRINTFSTDERR mexPrintf
#define PRINTFDEBUG mexPrintf
#define SPRINTF sprintf
#define FREE(_ptr,_size) mxFree(_ptr)
#define CALLOC mxCalloc
#define MALLOC mxMalloc
#define REALLOC mxRealloc
#define MEMCHECK(_ptr,_string,_size) 0
#define clean_up(_int) mexErrMsgTxt("")

#endif
