#include <string>
#include <math.h>
#include <gsl/gsl_poly.h>
#include <algorithm>
#include <stdlib.h>
#include <iostream>
//#include "convolver.h"

static double approxZero(double n);
static double opposite(double theta);
static int getTemplateN(int M);
double* getWeights(int M, double sigma); 
void computeBaseTemplates(double* input, int nx, int ny, int M, int borderCondition, double sigma, double** templates); 
double pointRespM1(int i, double angle, double* alpha, double** templates);
double pointRespM2(int i, double angle, double* alpha, double** templates);
double pointRespM3(int i, double angle, double* alpha, double** templates); 
double pointRespM4(int i, double angle, double* alpha, double** templates); 
double pointRespM5(int i, double angle, double* alpha, double** templates); 

int getRealRoots(double* z, int nz, double* roots); 
void filterM1(double** templates, int nx, int ny, double* alpha, double* response, double* orientation); 
void filterM2(double** templates, int nx, int ny, double* alpha, double* response, double* orientation); 
void filterM3(double** templates, int nx, int ny, double* alpha, double* response, double* orientation); 
void filterM4(double** templates, int nx, int ny, double* alpha, double* response, double* orientation); 
void filterM5(double** templates, int nx, int ny, double* alpha, double* response, double* orientation); 
int mirror(int x, int nx);
double interp(double* image, int nx, int ny, double x, double y);
void computeNMS(double* response, double* orientation, double* nms, int nx, int ny); 
void steerablefilter2Dcore(unsigned char * input, long* in_sz, int M, double sigma,double* response, double* orientation, double* nms);