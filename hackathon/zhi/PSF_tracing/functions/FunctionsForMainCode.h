// Prototype for FunctionsForMainCode.cpp

// Global constants and macros
#include <v3d_interface.h>

#define beta 1
#define ScaleInt 0.5
#define muT 0.0025
#define DEFAULT_LOOKUP_WIDTH		15
#define DEFAULT_ANISOTROPIC			TRUE
#define DEFAULT_SIGMA				4
#define DEFAULT_WHILE_LOOP_THR		9999
#define DEFAULT_SCORE				INFINITY
//#define DEFAULT_SCORE				0
#define DEFAULT_FLAG				1
#define W_THRESHOLD					0.05
#define CV							FALSE
#define INTRINSIC_DIM 1


#define NUM_OF_PREPROCESS_THREADS_TO_CREATE		1
#define NUM_OF_THREADS_TO_CREATE			1
#define THREAD_EXIT_SUCCESS 0

#define ROWCOL(row,col,tot_rows) (row + col*tot_rows)
#define pi 3.1415926536
//#define eps (pow((double)10, -16))//double pow (double, int)
#define NORM_CONST (1/(pow((2*pi),0.5)))

#define TRUE 1
#define FALSE 0

int Compute_3D_Min(int *, int, int, int);
int Compute_3D_Max(int *, int, int, int);
int Compute_3D_Len(int *, int, int, int);

void Get_ImagePixel_Neighbors (int *, int, int, double *, int *, int *);
void Generate_Weights_for_KDE (double, int *, int, double *, double *);
void Generate_Eig_Vecs_For_KDE(double *, int *, int, int , int, double *);
void Compute_H_perp (double *, double *, int, double *);
double Compute_Numerator(double *, double *, int);
double Compute_Denominator(double *, double *, int);
void Double_Compute_Transpose(double *, double *, int, int);
double Compute_Norm(double *, int) ;
void Compute_Updated_CurrP(double *, double *, double *, int);
void Compute_Normc(double *, int, int, double *);
double Compute_Gamma (double dInput);
void Get_CurrP (int *, int, int, double *);
void SortTangentialSpace(double *, double *, double*,int );
