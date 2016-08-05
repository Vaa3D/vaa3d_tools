/*
 * MATLAB Compiler: 6.0 (R2015a)
 * Date: Tue Nov 17 15:17:21 2015
 * Arguments: "-B" "macro_default" "-B" "csharedlib:libvaa_port_test" "-W"
 * "lib:libvaa_port_test" "-T" "link:lib" "vaa3d_trace3D" 
 */

#ifndef __libvaa_port_test_h
#define __libvaa_port_test_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_libvaa_port_test
#define PUBLIC_libvaa_port_test_C_API __global
#else
#define PUBLIC_libvaa_port_test_C_API /* No import statement needed. */
#endif

#define LIB_libvaa_port_test_C_API PUBLIC_libvaa_port_test_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_libvaa_port_test
#define PUBLIC_libvaa_port_test_C_API __declspec(dllexport)
#else
#define PUBLIC_libvaa_port_test_C_API __declspec(dllimport)
#endif

#define LIB_libvaa_port_test_C_API PUBLIC_libvaa_port_test_C_API


#else

#define LIB_libvaa_port_test_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libvaa_port_test_C_API 
#define LIB_libvaa_port_test_C_API /* No special import/export declaration */
#endif

extern LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV libvaa_port_testInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV libvaa_port_testInitialize(void);

extern LIB_libvaa_port_test_C_API 
void MW_CALL_CONV libvaa_port_testTerminate(void);



extern LIB_libvaa_port_test_C_API 
void MW_CALL_CONV libvaa_port_testPrintStackTrace(void);

extern LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV mlxVaa3d_trace3D(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);



extern LIB_libvaa_port_test_C_API bool MW_CALL_CONV mlfVaa3d_trace3D(int nargout, mxArray** result, mxArray* raw, mxArray* unit_bytes, mxArray* x, mxArray* y, mxArray* z, mxArray* t, mxArray* para, mxArray* fileDir);

#ifdef __cplusplus
}
#endif
#endif
