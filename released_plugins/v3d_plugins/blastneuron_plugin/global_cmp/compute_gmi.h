#ifndef __COMPUTE_GMI_H__
#define __COMPUTE_GMI_H__
#include "basic_surf_objs.h"
#include <QtGlobal>
#include <math.h>
#include <iostream>
#include <complex>
#include <vector>
using namespace std;

#define VOID 1000000000
#define PI 3.141592653589793



void compute_neuron_GMI(double **b, int siz,  double* centerpos, double radius_thres, double * gmi);
double compute_moments_neuron(double ** a, int siz, double p, double q, double r, double radius_thres);
void computeGMI(const NeuronTree & nt, double * gmi);

#endif
