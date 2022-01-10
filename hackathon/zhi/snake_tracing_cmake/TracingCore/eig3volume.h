/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

/*=========================================================================

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#ifndef EIG3VOLUME_H
#define EIG3VOLUME_H

#include "math.h"
#define MAX(a, b) ((a)>(b)?(a):(b))
#define n 3

static double hypot2(double x, double y) { return sqrt(x*x+y*y); }

__inline double absd(double val){ if(val>0){ return val;} else { return -val;} };

/* Symmetric Householder reduction to tridiagonal form. */
static void tred2(double V[n][n], double d[n], double e[n]);

/* Symmetric tridiagonal QL algorithm. */
static void tql2(double V[n][n], double d[n], double e[n]);

void eigen_decomposition(double A[n][n], double V[n][n], double d[n]);

#endif