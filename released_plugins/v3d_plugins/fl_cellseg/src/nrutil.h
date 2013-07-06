#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#include "../basic_c_fun/v3d_basicdatatype.h" //2010-05-20, PHC

static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)

static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))

static double dminarg1,dminarg2;
#define DMIN(a,b) (dminarg1=(a),dminarg2=(b),(dminarg1) < (dminarg2) ?\
        (dminarg1) : (dminarg2))

static float maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static float minarg1,minarg2;
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ?\
        (minarg1) : (minarg2))

static V3DLONG lmaxarg1,lmaxarg2;
#define LMAX(a,b) (lmaxarg1=(a),lmaxarg2=(b),(lmaxarg1) > (lmaxarg2) ?\
        (lmaxarg1) : (lmaxarg2))

static V3DLONG lminarg1,lminarg2;
#define LMIN(a,b) (lminarg1=(a),lminarg2=(b),(lminarg1) < (lminarg2) ?\
        (lminarg1) : (lminarg2))

static int imaxarg1,imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ?\
        (imaxarg1) : (imaxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#if defined(__STDC__) || defined(ANSI) || defined(NRANSI) /* ANSI */

void nrerror(char error_text[]);
float *vector(V3DLONG nl, V3DLONG nh);
int *ivector(V3DLONG nl, V3DLONG nh);
unsigned char *cvector(V3DLONG nl, V3DLONG nh);
unsigned V3DLONG *lvector(V3DLONG nl, V3DLONG nh);
double *dvector(V3DLONG nl, V3DLONG nh);
float **matrix(V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
double **dmatrix(V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
int **imatrix(V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
float **submatrix(float **a, V3DLONG oldrl, V3DLONG oldrh, V3DLONG oldcl, V3DLONG oldch,
	V3DLONG newrl, V3DLONG newcl);
float **convert_matrix(float *a, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
float ***f3tensor(V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch, V3DLONG ndl, V3DLONG ndh);
void free_vector(float *v, V3DLONG nl, V3DLONG nh);
void free_ivector(int *v, V3DLONG nl, V3DLONG nh);
void free_cvector(unsigned char *v, V3DLONG nl, V3DLONG nh);
void free_lvector(unsigned V3DLONG *v, V3DLONG nl, V3DLONG nh);
void free_dvector(double *v, V3DLONG nl, V3DLONG nh);
void free_matrix(float **m, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
void free_dmatrix(double **m, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
void free_imatrix(int **m, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
void free_submatrix(float **b, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
void free_convert_matrix(float **b, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch);
void free_f3tensor(float ***t, V3DLONG nrl, V3DLONG nrh, V3DLONG ncl, V3DLONG nch,
	V3DLONG ndl, V3DLONG ndh);

#else /* ANSI */
/* traditional - K&R */

void nrerror();
float *vector();
float **matrix();
float **submatrix();
float **convert_matrix();
float ***f3tensor();
double *dvector();
double **dmatrix();
int *ivector();
int **imatrix();
unsigned char *cvector();
unsigned V3DLONG *lvector();
void free_vector();
void free_dvector();
void free_ivector();
void free_cvector();
void free_lvector();
void free_matrix();
void free_submatrix();
void free_convert_matrix();
void free_dmatrix();
void free_imatrix();
void free_f3tensor();

#endif /* ANSI */

#endif /* _NR_UTILS_H_ */
