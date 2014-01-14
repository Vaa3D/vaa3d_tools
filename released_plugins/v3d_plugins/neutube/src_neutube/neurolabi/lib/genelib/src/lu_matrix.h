/*****************************************************************************************\
*                                                                                         *
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*                                                                                         *
\*****************************************************************************************/

#ifndef LU_MATRIX

#define LU_MATRIX

//  a is a square matrix of dimension n, and should be indexible
//    as a[i][j] for i,j in [0,n-1].  LU_Decompose takes a  and
//    * in place * produces an LU decomposition where u is the
//    upper half (including the diagonal) and l is the lower half
//    (excluding the diagonal, as it is 1's).  p[0..n-1] should be
//    the identity vector on input, and will be the permutation of
//    rows due to pivoting.  The return value is (-1)^k where k is
//    the number of pivots that occured and is passed to LU_Determinant
//    as sign below.

int  LU_Decompose(int n, double **a, int *p);

//  For debug purposes: print the matrix a and permutation p to file

void Show_LU_Product(FILE *file, int n, double **a, int *p);

//  Given rhs vector b and LU-decomposed matrix a of dimension n, and
//    pivot permutation p that acheived the decomposition, solve the system
//    of equations and return the result in b.

void LU_Solve(int n, double **a, double *b, int *p);

//  Given LU-decomposed matrix a of dimension n, and the sign returned
//    by LU_Decompose, return the value of the determinant of the original
//    matrix.

double LU_Determinant(int n, double **a, int sign);

//  To invert a given the LU-decomposition, simply call LU_Solve with
//    b = [ 0^k-1 1 0^n-k] to get the k'th column of the inverse matrix.

//  Solve the tridiagonal system [ a, b, c] x = v returning the answer in v.
//    [ a, b, c] is unmodified and can be reused.  A non-zero value is returned
//    if the matrix is singular or an underflow of the pivot occured.  Handles
//    the circular case where a[0] != 0 or c[n-1] != 0, but uses a more optimized
//    version if the problem is not circular.

int TDG_Solve(int n, double *a, double *b, double *c, double *v);

//  Given a set of n control points v[0], ... v[n-1], solve, in place, for the
//    derivative at each control point so that the splines interpolate, and are
//    C0,1,&2 continuous.  Set closed to non-zero if you want a closed curve
//    and 0 otherwise.

void Cubic_Spline_Slopes(int n, double *v, int closed);

#endif
