/*****************************************************************************************\
*                                                                                         *
*  Root finding of 1- and multi-dimensional functions                                     *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _FUNCTION_ROOT_FINDING

#define _FUNCTION_ROOT_FINDING

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

typedef struct       //  Bracket for root finding
  { double lft;
    double rgt;
  } Root_Bundle;

Root_Bundle *Find_Root_Bracket(Root_Bundle *R(M(brack)), double (*f)(double));

double Find_Function_Root(Root_Bundle *brack, double (*f)(double), double (*df)(double));

#ifdef __cplusplus
}
#endif

#endif
