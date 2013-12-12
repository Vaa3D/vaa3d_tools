/*****************************************************************************************\
*                                                                                         *
*  Function minimization routines for 1 and multiple dimensions                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  May 2007                                                                      *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _FUNCTION_MINIMIZATION

#define _FUNCTION_MINIMIZATION

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"

typedef struct
  { double lft;   //  Bracket for minimization
    double mid;   //      Assume:  lft < mid < rgt
    double rgt;   //         and:  f(mid) < f(lft), f(rgt)
  } Minimum_Bundle;

Minimum_Bundle *Find_Min_Bracket(Minimum_Bundle *R(M(brack)),
                                 double xin, double step, double (*f)(double));

double Minimize_Fct(Minimum_Bundle *brack, double (*f)(double), double (*df)(double));

Double_Vector *Powell_Minimizer(Double_Vector *R(M(xinit)), double step, double (*f)(double *));

Double_Vector *Polak_Ribier_Minimizer(Double_Vector *R(M(xinit)),
                                      double (*f) (double *x),
                                      double (*df)(double *x, double *R(O(g))));

#ifdef __cplusplus
}
#endif

#endif
