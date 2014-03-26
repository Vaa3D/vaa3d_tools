/**@file tz_random.h
 * @brief random number generator
 * @author Ting Zhao
 * @date 14-Oct-2008
 */

#ifndef _TZ_RANDOM_H_
#define _TZ_RANDOM_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

void Random_Seed(unsigned long int seed);
double Unifrnd();
int Unifrnd_Int(int range);
unsigned int Poissonrnd(double mu);
double Normrnd_Std();
double Normrnd(double mu, double sigma);
int Bernrnd(double p);

double* Unifrnd_Double_Array(int length, double *array);
int* Unifrnd_Int_Array(int range, int length, int *array);

__END_DECLS

#endif
