/*****************************************************************************************\
*                                                                                         *
*  Distribution generator data abstraction                                                *
*     One can create a distribution generator for a number of parameterized distribution  *
*     types and then generate events with that distribution                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2007                                                                  *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _SR_CDF
#define _SR_CDF

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

double drand();
double erand(uint64 *M(state));

void   dseed(uint32 seedval);
uint64 eseed(uint32 seedval);

typedef void CDF;

CDF *G(Normal_CDF)(double mean, double stdev);
CDF *G(Exponential_CDF)(double a);
CDF *G(Poisson_CDF)(double a);
CDF *G(Geometric_CDF)(double p);
CDF *G(Uniform_CDF)(double low, double hgh);
CDF *G(Binomial_CDF)(int n, double p);
CDF *G(FairCoin_CDF)(int n);
CDF *G(Bernouilli_CDF)(int n, double *weight);

CDF *G(Copy_CDF)(CDF *cdf);   /* As per convention */
CDF *Pack_CDF(CDF *R(M(cdf)));
CDF *Inc_CDF(CDF *R(I(cdf)));
void Free_CDF(CDF *F(cdf));
void Kill_CDF(CDF *K(cdf));
void Reset_CDF();
int  CDF_Usage();
void CDF_List(void (*handler)(CDF *));
int  CDF_Refcount(CDF *cdf);
CDF *G(Read_CDF)(FILE *input);
void Write_CDF(CDF *cdf, FILE *output);

double Sample_CDF(CDF *cdf);

void   Seed_CDF(CDF *cdf, uint32 seedval);

void   Link_CDF(CDF *ref, CDF *cdf);
void   Unlink_CDF(CDF *cdf);

#ifdef __cplusplus
}
#endif

#endif
