/*****************************************************************************************\
*                                                                                         *
*  Function minimization routines for 1 and multiple dimensions                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  May 2007                                                                      *
*                                                                                         *
\*****************************************************************************************/

#ifndef FUNCTION_MINIMIZATION

#define FUNCTION_MINIMIZATION

typedef struct
  { double lft;   //  Bracket for minimization
    double mid;   //      Assume:  lft < mid < rgt
    double rgt;   //         and:  f(mid) < f(lft), f(rgt)
  } Min_Bracket;

//  Given a function f, a point xin, and a step size size, attempt to find a bracket
//    about a minimum in the downhill direction from xin.  Return a pointer to a
//    statically allocated bracket which is reset with each call to Find_Min_Bracket.
//    If a bracket cannot reasonably be found, then NULL is returned.  It is
//    generally a good idea to keep the step size small as it determines the
//    jump sizes for the search, which can overshoot if it's too big.

Min_Bracket *Find_Min_Bracket(double xin, double step, double (*f)(double));

//  Given a minimization bracket, return a local minimum value of f within
//    the bracket and the coordinate at which it is acheived at the double
//    pointed at by xmin.  Accurate to first 8 digits (half double-precision).
//    If the derivative of f, df, is NULL, then parabolic estimation is used
//    to accelerate convergence, otherwise the secant method is employed.  In the
//    event df is used, one may take advantage of the fact that a call to df with a
//    value x, *always follows a prior call to f with the same value.  So one
//    may, via globally shared values, optimize the computation of f & df by
//    sharing the computation of any common sub-expressions.

double Minimize_Fct(Min_Bracket *brack, double (*f)(double), double (*df)(double), double *xmin);

//  Statically allocate an n-by-n matrix, set it to I * step, and return a pointer to
//    it.  This matrix can then be used as an initial set of direction vectors for Powell's
//    function minimization method.

double **Make_Orthogonal_Step(int n, double step);

//  Find a minimum of n-dimensional function f with Powell's method which does not require
//    the gradient.  Start the search at point xin with an initial set of n descent vectors
//    given in step.  The value of the minimum is returned and the value of xin upon return
//    is the point at which the minimum was acheived.  The routine uses the matrix step, so
//    its value upon return will not be what it was upon input.

double Powell_Minimizer(int n, double *xin, double **step, double (*f)(double *));

//  Find a minimum of n-dimensional function f with the conjugate gradient method of Polak
//    and Ribier.  Start the search at point xin and return the minimum value.  The value
//    of xin upon return is the point at which the mininum was acheived.

double Polak_Ribier_Minimizer(int n, double *xin, double (*f)(double *), double (*df)(double *));

#endif
