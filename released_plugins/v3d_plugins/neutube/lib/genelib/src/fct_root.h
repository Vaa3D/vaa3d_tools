/*****************************************************************************************\
*                                                                                         *
*  Root finding of 1- and multi-dimensional functions                                     *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
\*****************************************************************************************/

#ifndef FUNCTION_ROOT_FINDING

#define FUNCTION_ROOT_FINDING

typedef struct       //  Bracket for root finding
  { double lft;
    double rgt;
  } Root_Bracket;

//  Given a function f and interval brack, attempt to find a bracket about a root
//    expanding the interval toward the point closer to 0.  Reset the values of the
//    bracket to the first root bracket found and return a pointer to brack, or
//    return NULL if a root isn't found.

Root_Bracket *Find_Root_Bracket(Root_Bracket *brack, double (*f)(double));

//  Given a root bracket, return a root of of f within the bracket.  Generally
//    accurate to 15 digits (double-precision).  If the derivative of f, df, is NULL,
//    then Brent's algorithm which uses quadratic estimation is employed,
//    otherwise the Raphson-Newton which uses the derivative is employed.  In the
//    event df is used, one may take advantage of the fact that a call to df with a
//    value x, *always* follows a prior call to f with the same value.  So one
//    may, via globally shared values, optimize the computation of f & df by
//    sharing the computation of any common sub-expressions.

double Find_Function_Root(Root_Bracket *brack, double (*f)(double), double (*df)(double));

#endif
