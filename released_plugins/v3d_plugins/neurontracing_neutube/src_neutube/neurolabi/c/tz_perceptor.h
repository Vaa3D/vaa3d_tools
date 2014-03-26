/**@file tz_perceptor.h
 * @brief perceptor or receptor
 * @author Ting Zhao
 * @date 04-Apt-2007
 */

#ifndef _TZ_PERCEPTOR_H_
#define _TZ_PERCEPTOR_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_cont_fun.h"
#include "tz_darray.h"
#include "tz_optimize_utils.h"

__BEGIN_DECLS

typedef struct _Perceptor {
  Variable_Set *vs;           /* variable set   */
  void *arg;                /* additional argument */
  Continuous_Function *s;     /* score function */
  double min_gradient;        /* minimal gradient for fitting */
  double *delta;              /* steps for estimating gradient */
  double *weight;             /* weight of each variable */
} Perceptor;

/*
 * Perceptor_Fit_Score() returns fitting score between <perceptor> and <stack>.
 * The higher the score, the better the fit. <f> is the specified function to
 * calculate the score. Perceptor_Fit_Score() does a similiar thing but it takes
 * the double array form of a perceptor as its argument.
 */
/*
double Perceptor_Fit_Score(void *perceptor, const Stack *stack,  
			   double (*f) (void *, const Stack *));
double Perceptor_Fit_Score_A(const double *param, const Stack *stack,
			     double (*f) (const double *, const Stack *));
*/

/*
 * Perceptor_Gradient_Partial() returns the partial gradient of the score
 * function of <param> and <stack>. The gradient is calculated with respect to
 * <param[param_index]>. <delta> is the step for numerical calculation and <f>
 * is the score function.
 */
double Perceptor_Gradient_Partial(Variable_Set *vs, const Stack *stack,
				  int index, double delta, void *arg,
				  double (*f) (const double *, const void *));

/*
 * Perceptor_Gradient() calculates the gradient of the score function of
 * <perceptor> and <stack>. The result is stored in <gradient>.
 */
void Perceptor_Gradient(const Perceptor *perceptor, const Stack *stack, 
			double gradient[]);

/*
 * Fit_Perceptor() fits a perceptor in <stack>. It returns the score of fitting.
 */
double Fit_Perceptor(Perceptor *perceptor, const Stack *stack);

__END_DECLS

#endif
