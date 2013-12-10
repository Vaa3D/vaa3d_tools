/*****************************************************************************************\
*                                                                                         *
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*                                                                                         *
\*****************************************************************************************/

#ifndef _MATRIX_LIB

#define _MATRIX_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"

typedef struct
  { Double_Matrix *lu_mat;  //  LU decomposion: L is below the diagonal and U is on and above it
    int           *perm;    //  Permutation of the original rows of m due to pivoting
    int            sign;    //  Sign to apply to the determinant due to pivoting (+1 or -1)
  } LU_Factor;

LU_Factor *G(Copy_LU_Factor)(LU_Factor *f);   //  As per convention for an packable object
LU_Factor *Pack_LU_Factor(LU_Factor *R(M(f)));
LU_Factor *Inc_LU_Factor(LU_Factor *R(I(f)));
void       Free_LU_Factor(LU_Factor *F(f));
void       Kill_LU_Factor(LU_Factor *F(f));
void       Reset_LU_Factor();
int        LU_Factor_Usage();
void       LU_Factor_List(void (*handler)(LU_Factor *));
int        LU_Factor_Refcount(LU_Factor *f);
LU_Factor *G(Read_LU_Factor)(FILE *input);
void       Write_LU_Factor(LU_Factor *f, FILE *output);

typedef void Band_Factor;

Band_Factor *G(Copy_Band_Factor)(Band_Factor *f);   //  As per convention for an packable object
Band_Factor *Pack_Band_Factor(Band_Factor *R(M(f)));
Band_Factor *Inc_Band_Factor(Band_Factor *R(I(f)));
void         Free_Band_Factor(Band_Factor *F(f));
void         Kill_Band_Factor(Band_Factor *F(f));
void         Reset_Band_Factor();
int          Band_Factor_Usage();
void         Band_Factor_List(void (*handler)(Band_Factor *));
int          Band_Factor_Refcount(Band_Factor *f);
Band_Factor *G(Read_Band_Factor)(FILE *input);
void         Write_Band_Factor(Band_Factor *f, FILE *output);

Double_Matrix *Set_Matrix2Identity(Double_Matrix *R(M(m)));
Double_Matrix *G(Identity_Matrix)(Dimn_Type n);

Double_Matrix *Transpose_Matrix(Double_Matrix *R(M(m)));

Double_Matrix *Set_Matrix2Product(Double_Matrix *R(M(c)),
                                  Double_Matrix *a, boolean ta, Double_Matrix *b, boolean tb);
Double_Matrix *G(Matrix_Multiply)(Double_Matrix *a, boolean ta, Double_Matrix *b, boolean tb);

LU_Factor *G(LU_Decompose)(Double_Matrix *S(m), boolean *O(stable));
void Show_LU_Factor(FILE *file, LU_Factor *f);
Double_Vector *LU_Solve(Double_Vector *R(M(b)), LU_Factor *f);
Double_Matrix *G(LU_Invert)(LU_Factor *f, boolean transpose);
double LU_Determinant(LU_Factor *f);

Double_Matrix *Orthogonalize_Matrix(Double_Matrix *R(M(basis)));

Band_Factor *G(Triband_Decompose)(Double_Matrix *t);
Double_Vector *Triband_Solve(Double_Vector *R(M(v)), Band_Factor *f);

Band_Factor *G(Spline_Decompose)(Dimn_Type n, boolean closed);
Double_Vector *Spline_Slopes(Double_Vector *R(M(v)), Band_Factor *f);

Band_Factor *G(Pentaband_Decompose)(Double_Matrix *t);
Double_Vector *Pentaband_Solve(Double_Vector *R(M(v)), Band_Factor *f);

#ifdef __cplusplus
}
#endif

#endif
