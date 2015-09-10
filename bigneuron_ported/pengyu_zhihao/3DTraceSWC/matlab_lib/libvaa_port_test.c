/*
 * MATLAB Compiler: 6.0 (R2015a)
 * Date: Thu Sep 10 12:38:51 2015
 * Arguments: "-B" "macro_default" "-B" "csharedlib:libvaa_port_test" "-W"
 * "lib:libvaa_port_test" "-T" "link:lib" "vaa3d_trace3D" 
 */

#include <stdio.h>
#define EXPORTING_libvaa_port_test 1
#include "libvaa_port_test.h"

static HMCRINSTANCE _mcr_inst = NULL;


#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultPrintHandler(const char *s)
{
  return mclWrite(1 /* stdout */, s, sizeof(char)*strlen(s));
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultErrorHandler(const char *s)
{
  int written = 0;
  size_t len = 0;
  len = strlen(s);
  written = mclWrite(2 /* stderr */, s, sizeof(char)*len);
  if (len > 0 && s[ len-1 ] != '\n')
    written += mclWrite(2 /* stderr */, "\n", sizeof(char));
  return written;
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libvaa_port_test_C_API
#define LIB_libvaa_port_test_C_API /* No special import/export declaration */
#endif

LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV libvaa_port_testInitializeWithHandlers(
    mclOutputHandlerFcn error_handler,
    mclOutputHandlerFcn print_handler)
{
    int bResult = 0;
  if (_mcr_inst != NULL)
    return true;
  if (!mclmcrInitialize())
    return false;
    {
        mclCtfStream ctfStream = 
            mclGetEmbeddedCtfStream((void *)(libvaa_port_testInitializeWithHandlers));
        if (ctfStream) {
            bResult = mclInitializeComponentInstanceEmbedded(   &_mcr_inst,
                                                                error_handler, 
                                                                print_handler,
                                                                ctfStream);
            mclDestroyStream(ctfStream);
        } else {
            bResult = 0;
        }
    }  
    if (!bResult)
    return false;
  return true;
}

LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV libvaa_port_testInitialize(void)
{
  return libvaa_port_testInitializeWithHandlers(mclDefaultErrorHandler, 
                                                mclDefaultPrintHandler);
}

LIB_libvaa_port_test_C_API 
void MW_CALL_CONV libvaa_port_testTerminate(void)
{
  if (_mcr_inst != NULL)
    mclTerminateInstance(&_mcr_inst);
}

LIB_libvaa_port_test_C_API 
void MW_CALL_CONV libvaa_port_testPrintStackTrace(void) 
{
  char** stackTrace;
  int stackDepth = mclGetStackTrace(&stackTrace);
  int i;
  for(i=0; i<stackDepth; i++)
  {
    mclWrite(2 /* stderr */, stackTrace[i], sizeof(char)*strlen(stackTrace[i]));
    mclWrite(2 /* stderr */, "\n", sizeof(char)*strlen("\n"));
  }
  mclFreeStackTrace(&stackTrace, stackDepth);
}


LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV mlxVaa3d_trace3D(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
  return mclFeval(_mcr_inst, "vaa3d_trace3D", nlhs, plhs, nrhs, prhs);
}

LIB_libvaa_port_test_C_API 
bool MW_CALL_CONV mlfVaa3d_trace3D(int nargout, mxArray** result, mxArray* raw, mxArray* 
                                   unit_bytes, mxArray* x, mxArray* y, mxArray* z, 
                                   mxArray* t, mxArray* para)
{
  return mclMlfFeval(_mcr_inst, "vaa3d_trace3D", nargout, 1, 7, result, raw, unit_bytes, x, y, z, t, para);
}

