/* tz_perceptor.c
 *
 * 06-Apt-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <math.h>
#include "tz_cont_fun.h"
#include "tz_optimize_utils.h"
#include "tz_perceptor.h"

#define PERCEPTOR_LAMDA 0.1
#define PERCEPTOR_MIN_GRADIENT 0.1

/*
double Perceptor_Fit_Score(void *perceptor, const Stack *stack,  
			   double (*f) (void *, const Stack *))
{
  return f(perceptor, stack);
}

double Perceptor_Fit_Score_A(const double *param, const Stack *stack,
			     double (*f) (const double *, const Stack *))
{
  return f(param, stack);
}
*/

inline static void update_variable(Variable_Set *vs, int index, 
				   double delta)
{
  vs->var[vs->var_index[index]] += delta;
  Variable_Set_Update_Link(vs);
}

#define DEFINE_PARAM_ARRAY(param_array, stack, arg)	\
  const void *param_array[2];					\
  param_array[0] = stack;				\
  param_array[1] = arg;

static double 
perceptor_gradient_partial(Variable_Set *vs, int index, const Stack *stack,
			   double delta, double score, void *arg,
			   double (*f) (const double *, const void *))
{
  update_variable(vs, index, delta);

  DEFINE_PARAM_ARRAY(param_array, stack, arg);

  double right_score = f(vs->var, param_array);

  /* restore param */
  update_variable(vs, index, -delta); 

  double grad = (right_score - score) / delta;
  if (grad < 0.0) {
    update_variable(vs, index, -delta);
    double left_score = f(vs->var, param_array);
    if (left_score < score) {
      grad = 0.0;
    } else {
      //grad = dmin2(grad, (score - left_score) / delta);
      grad = (score - left_score) / delta;
    }
    /* restore param */
    update_variable(vs, index, delta);
  } else if (grad > 0.0) {
    update_variable(vs, index, -delta);
    double left_score = f(vs->var, param_array);
    if (left_score > score) {
      //grad = 0.0;
      grad = (score - left_score) / delta;
    } /*else {
      grad = dmax2(grad, (score - left_score) / delta);
    }*/
    /* restore param */
    update_variable(vs, index, delta);
  } else {
    update_variable(vs, index, -delta);
    double left_score = f(vs->var, param_array);
    grad = (score - left_score) / delta;
    /* restore param */
    update_variable(vs, index, delta);
  }
  
  return grad;
}

double Perceptor_Gradient_Partial(Variable_Set *vs, const Stack *stack,
				  int index, double delta, void *arg,
				  double (*f) (const double *, const void *))
{
  DEFINE_PARAM_ARRAY(param_array, stack, arg);

  double score1 = f(vs->var, param_array);
  
  return perceptor_gradient_partial(vs, index, stack, delta, score1, arg, f);
}

void Perceptor_Gradient(const Perceptor *perceptor, const Stack *stack, 
			double gradient[])
{
  DEFINE_PARAM_ARRAY(param_array, stack, perceptor->arg);

  double score = perceptor->s->f(perceptor->vs->var, 
				 param_array);

  int i;
  for (i = 0; i < perceptor->vs->nvar; i++) {
    int var_index = perceptor->vs->var_index[i];
    gradient[i] =
      perceptor_gradient_partial(perceptor->vs, i, stack,
				 perceptor->delta[var_index],
				 score, perceptor->arg, perceptor->s->f);
    /* testing 
    if (perceptor->weight != NULL) {
      //gradient[i] *= perceptor->weight[i];
    }
    *************/
  }
}


#define FIT_PERCEPTOR_MAXITER 500
//#define LINE_SEARCH Line_Search_Var_Golden_Bracket
#define LINE_SEARCH Line_Search_Var_Backtrack
#define GRADESC_METHOD 0

/* The optimization uses a slightly modified form of conjugate gradient method. 
 * It uses Polak-Reibiere method to update searching directions. However, when
 * the line search is performed in each iteration, the real searching direction
 * is scaled by a weight vector. So the direction is a little off from the
 * theoretical direction. Following the PR method without revision seems a 
 * little worse than this modified form, although they are both significantly 
 * bettern than the steepest gradient descent method. I have not figured out 
 * the reason. It could be just a coicendence in the testing cases or due to 
 * the approximation of the sub-steps such as gradient calculation.
 */
double Fit_Perceptor(Perceptor *perceptor, const Stack *stack)
{
  Line_Search_Workspace *lsw = New_Line_Search_Workspace(perceptor->vs->nvar);
  Set_Line_Search_Workspace(lsw, 0.2, 0.8, 0.01, 0.1, perceptor->min_gradient);
  //lsw->alpha = 0.3;

  double *update_direction = darray_malloc(perceptor->vs->nvar);
  Perceptor_Gradient(perceptor, stack, lsw->start_grad);
  DEFINE_PARAM_ARRAY(param_array, stack, perceptor->arg);
  lsw->score = perceptor->s->f(perceptor->vs->var, param_array);

  darraycpy(update_direction, lsw->start_grad, 0, perceptor->vs->nvar);
    
  BOOL stop = FALSE;
  int iter = 0;
  BOOL succ = TRUE;


  while (stop == FALSE) {  
#ifdef _DEBUG_
    darray_print2(lsw->start_grad, perceptor->vs->nvar, 1);
    darray_print2(perceptor->vs->var, perceptor->vs->nvar, 1);
#endif

    double direction_length = darray_norm(update_direction, 
        perceptor->vs->nvar);
    if (direction_length < lsw->min_direction) {
      succ = FALSE;
    } else {
#if GRADESC_METHOD == 1
      //darray_divc(update_direction, direction_length, perceptor->vs->nvar);
#endif
      succ = LINE_SEARCH(perceptor->vs, param_array,
          perceptor->s, perceptor->delta,
          perceptor->weight,
          update_direction, lsw);
    }

    if (succ == FALSE) {
      direction_length = darray_norm(lsw->start_grad, perceptor->vs->nvar);
      if (direction_length > perceptor->min_gradient) {
#if GRADESC_METHOD == 1
        //darray_divc(update_direction, direction_length, perceptor->vs->nvar);
#endif
        darraycpy(update_direction, lsw->start_grad, 0, perceptor->vs->nvar);
        succ = LINE_SEARCH(perceptor->vs, param_array,
            perceptor->s, perceptor->delta,
            perceptor->weight, update_direction, lsw);
      }
    }

    if (succ == TRUE) {
      iter++;
#ifdef _DEBUG_
      //darray_print2(lsw->grad, perceptor->vs->nvar, 1);
      printf("iter: %d, score: %g\n", iter, lsw->score);
      //darray_print2(update_direction, perceptor->vs->nvar, 1);
#endif
      if (iter >= FIT_PERCEPTOR_MAXITER) {
        stop = TRUE;
      } else {
        Perceptor_Gradient(perceptor, stack, lsw->grad);

#if GRADESC_METHOD == 0 
        Conjugate_Update_Direction(perceptor->vs->nvar, lsw->grad,
            lsw->start_grad, update_direction);
#endif

#if GRADESC_METHOD == 1
        Conjugate_Update_Direction_W(perceptor->vs->nvar, lsw->grad,
            lsw->start_grad, perceptor->weight, update_direction);
#endif

#if GRADESC_METHOD == 2
        darraycpy(update_direction, lsw->grad, 0, perceptor->vs->nvar);
#endif

        darraycpy(lsw->start_grad, lsw->grad, 0, perceptor->vs->nvar);
      }
    } else {
      stop = TRUE;
    }
  }
 
  free(update_direction);

  double score = lsw->score;
  Kill_Line_Search_Workspace(lsw);

  return score;

}
