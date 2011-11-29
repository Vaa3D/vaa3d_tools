#ifndef PCA_H
#define PCA_H

float **matrix(int n,int m);
void corcol(float **data,int n,int m,float ** symmat);
void covcol1(float **data,int n,int m,float ** symmat,float *avg);
void covcol(float **data,int n,int m,float ** symmat);
void scpcol(float **data,int n,int m,float ** symmat);
void erhand(char *err_msg);
float *vector1(int n);
	void stdDev(float **data,int n,int m,float *stddev);


void tred2(float **a,int  n, float *d, float *e);
void tqli(float d[],float e[],int n,float **z);
void free_vector(float *v,int n);


void free_matrix(float **mat,int n,int m);


#endif
