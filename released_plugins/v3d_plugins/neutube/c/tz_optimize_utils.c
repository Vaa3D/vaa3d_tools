/* tz_optimize_utils.c
 *
 * 23-Jan-2008 Initial write: Ting Zhao
 */
#include <math.h>
#include "tz_optimize_utils.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_math.h>
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_blas.h>
#  include <gsl/gsl_multifit.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include <utilities.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_gsl_linalg.h"
#include "tz_constant.h"

/* New_Optimize_Workspace(): new a workspace for optimization
 *
 * Args: nvar - number of variables to optimize. This is used to determine
 *              how much space should be allocated for the workspace.
 * 
 * Return: the workspace. The fields are initialized to default values except
 *         best_var, which is only assigned by an allocated array without
 *         initialization of the elements.
 * 
 * Dependence: Guarded_Malloc()
 */
Optimize_Workspace *New_Optimize_Workspace(int nvar)
{
  Optimize_Workspace *ow =
    (Optimize_Workspace *) Guarded_Malloc(sizeof(Optimize_Workspace),
					  "New_Optimize_Workspace");

  ow->nvar = nvar;
  ow->phase = 0;
  ow->max_score = -Infinity;
  ow->is_improved = FALSE;
  ow->best_var = darray_malloc(nvar);

  return ow;
}

void Kill_Optimize_Workspace(Optimize_Workspace *ow)
{
  free(ow->best_var);
  free(ow);
}


void Update_Optimize_Workspace(Variable_Set *vs, double score,
			       Optimize_Workspace *ow)
{
  ow->scores[ow->phase] = score;
  if (ow->scores[ow->phase] > ow->max_score) {
    ow->max_score = ow->scores[ow->phase];
    int i;
    for (i = 0; i < vs->nvar; i++) {
      ow->best_var[i] = vs->var[vs->var_index[i]];
    }
    ow->is_improved = TRUE;
  }

  ow->phase++;
}

Line_Search_Workspace *New_Line_Search_Workspace(int nvar)
{
  Line_Search_Workspace *lsw = 
    (Line_Search_Workspace *) Guarded_Malloc(sizeof(Line_Search_Workspace),
					     "New_Line_Search_Workspace");
  lsw->nvar = nvar;
  lsw->alpha = 1.0;
  lsw->ro = 0.5;
  lsw->c1 = 0.01;
  lsw->c2 = 0.3;
  lsw->grad = darray_malloc(nvar);
  lsw->start_grad = darray_malloc(nvar);
  lsw->score = -Infinity;

  return lsw;
}

void Set_Line_Search_Workspace(Line_Search_Workspace *lsw,
			       double alpha, double ro, double c1, double c2,
			       double min_direction)
{
  lsw->alpha = alpha;
  lsw->ro = ro;
  lsw->c1 = c1;
  lsw->c2 = c2;
  lsw->min_direction = min_direction;
}

void Kill_Line_Search_Workspace(Line_Search_Workspace *lsw)
{
  free(lsw->grad);
  free(lsw->start_grad);
  free(lsw);
}


inline static void validate_var(double *var, double min, double max)
{
  if (*var < min) {
    *var = min;
  } else if (*var > max) {
    *var = max;
  }
}


/* Backtracking line search */
#define LINE_SEARCH_ESTIMATE_GRADIENT(x, grad, i)			\
  for (i = 0; i < nvar; i++) {						\
    tmp_var = x[var_index[i]];						\
    x[var_index[i]] += delta[var_index[i]];				\
    grad[i] = (cf->f(x, param) - lsw->score) / delta[var_index[i]];	\
    x[var_index[i]] = tmp_var;						\
    if (grad[i] < 0.0) {						\
      tmp_var = x[var_index[i]];					\
      x[var_index[i]] -= delta[var_index[i]];				\
      if (cf->f(x, param) < lsw->score)	{				\
	grad[i] = 0.0;							\
      }									\
      x[var_index[i]] = tmp_var;					\
    }									\
  }
  
#define STOP_GRADIENT 1e-1

BOOL Line_Search_Var_Backtrack(Variable_Set *vs,
			       const void *param, 
			       const Continuous_Function *cf,
			       const double *delta, const double *weight,
			       double *direction,
			       Line_Search_Workspace *lsw)
{
  if (weight != NULL) {
    int i;
    for (i = 0; i < vs->nvar; i++) {
      direction[i] *= weight[i];
    }
  }

  double direction_length = sqrt(darray_sqsum(direction, vs->nvar));

  BOOL improved = TRUE;

  if (direction_length > lsw->min_direction) {

    double alpha = 0.0;
    double *org_var = darray_malloc(vs->nvar);
    int i;
    for (i = 0; i < vs->nvar; i++) {
      org_var[i] = vs->var[vs->var_index[i]];
    }
    double start_score = lsw->score;
    alpha = lsw->alpha / direction_length;

    double gd_dot = darray_dot(lsw->start_grad, direction, vs->nvar);
    double gd_dot_c1 = gd_dot * lsw->c1;

    //double gd_dot_c2 = gd_dot * lsw->c2; /* for wolfe2*/

    double wolfe1 = 0.0;
    //printf("start line search\n");
    do {
      for (i = 0; i < vs->nvar; i++) {
        vs->var[vs->var_index[i]] = alpha * direction[i];
        vs->var[vs->var_index[i]] += org_var[i];

        //validate_var(vs->var + vs->var_index[i], cf->var_min[vs->var_index[i]],
        //	     cf->var_max[vs->var_index[i]]);
      }
      cf->v(vs->var, cf->var_min, cf->var_max, NULL);
      Variable_Set_Update_Link(vs);
      lsw->score = cf->f(vs->var, param);

      alpha *= lsw->ro;
      //printf("alpha %g\n", alpha * direction_length);
      if (alpha * direction_length < STOP_GRADIENT) {
        for (i = 0; i < vs->nvar; i++) {
          vs->var[vs->var_index[i]] = org_var[i];
        }
        Variable_Set_Update_Link(vs);
        lsw->score = start_score;
        improved = FALSE;
        break;
      }

      wolfe1 = dmax2(alpha / lsw->ro * gd_dot_c1, 0.0);
    } while //(lsw->score <= start_score);
    ((lsw->score < start_score + wolfe1)/* || 
                                           (darray_dot(lsw->grad, direction, vs->nvar) > gd_dot_c2)*/);
      //|| (fabs(darray_dot(lsw->grad, direction, vs->nvar)) >= fabs(gd_dot_c2)));
      //printf("wolf: %g\n", alpha / lsw->ro * gd_dot_c1);
      //((lsw->score < start_score + alpha / lsw->ro * gd_dot_c1) ||
      // (fabs(darray_dot(lsw->grad, direction, nvar)) > gd_dot_c2));
#if 0
      alpha = lsw->alpha;
    improved= TRUE;
    if (improved == FALSE) {
      do {
        for (i = 0; i < nvar; i++) {
          var[var_index[i]] = org_var[i] - alpha * direction[i];
          validate_var(var + var_index[i], cf->var_min[var_index[i]],
              cf->var_max[var_index[i]]);
        }
        lsw->score = cf->f(var, param);

        alpha *= lsw->ro;

        if (alpha * weight < 1e-3) {
          for (i = 0; i < nvar; i++) {
            var[var_index[i]] = org_var[i];
          }
          lsw->score = start_score;
          improved = FALSE;
          break;
        }
      } while (lsw->score < start_score);
    }
#endif

#if 0
    if (improved == TRUE) {
      for (i = 0; i < nvar; i++) {					
        tmp_var = vs->var[vs->var_index[i]];		   
        vs->var[var_index[i]] += delta[var_index[i]];			       
        lsw->grad[i] = (cf->f(var, param) - lsw->score) / delta[var_index[i]];
        var[var_index[i]] = tmp_var;					  
        if (lsw->grad[i] < 0.0) {						
          tmp_var = var[var_index[i]];					
          var[var_index[i]] -= delta[var_index[i]];			   
          if (cf->f(lsw->grad, param) < lsw->score)	{		   
            lsw->grad[i] = 0.0;						     
          }							    	
          var[var_index[i]] = tmp_var;					
        }								     
      }
    } 
#endif
    free(org_var);
  } else {
    improved = FALSE;
  }
  //printf("%g\n", alpha);
  return improved;
}

static inline 
void line_search_update_variable(Variable_Set *vs, 
				 const double *start, const double *direction, 
				 double alpha, const Continuous_Function *cf,
				 const double *weight)
{
  int i;
  for (i = 0; i < vs->nvar; i++) {
    vs->var[vs->var_index[i]] = alpha * direction[i];
    if (weight != NULL) {
      vs->var[vs->var_index[i]] *= weight[vs->var_index[i]];
    }
    vs->var[vs->var_index[i]] += start[i];
    //validate_var(vs->var + vs->var_index[i], cf->var_min[vs->var_index[i]],
    //cf->var_max[vs->var_index[i]]);
  }
  cf->v(vs->var, cf->var_min, cf->var_max, NULL);
  Variable_Set_Update_Link(vs);
}

BOOL Line_Search_Var_Golden_Bracket(Variable_Set *vs,
				    const void *param, 
				    const Continuous_Function *cf,
				    const double *delta, const double *weight,
				    const double *direction,
				    Line_Search_Workspace *lsw)
{
  double r =  TZ_GOLDEN_SEC;
  double c = 1.0 - r;
  
  double direction_length = sqrt(darray_sqsum(direction, vs->nvar));

  BOOL improved = TRUE;

  if (direction_length > lsw->min_direction) {
    double alpha = 0.0;
    double *org_var = darray_malloc(vs->nvar);
    int i;
    for (i = 0; i < vs->nvar; i++) {
      org_var[i] = vs->var[vs->var_index[i]];
    }

    alpha = lsw->alpha / direction_length * 5.0;

    double x1, x2;
    double ax = 0.0;
    double cx = alpha;
    double x0 = ax;
    double x3 = cx;
    double bx = alpha / 2.0;
    
    x1 = bx;
    x2 = bx + c * (cx - bx);

    line_search_update_variable(vs, org_var, direction, x1, cf, weight);
    double f1 = -cf->f(vs->var, param);
    line_search_update_variable(vs, org_var, direction, x2, cf, weight);
    double f2 = -cf->f(vs->var, param);
    double tol = 1.0e-2;

    while (fabs(x3 - x0) > tol / direction_length) {
      if (f2 < f1) {
	SHIFT3(x0, x1, x2, r * x2 + c * x3);
	line_search_update_variable(vs, org_var, direction, x2, cf, weight);
	SHIFT2(f1, f2, -cf->f(vs->var, param));
      } else {
	SHIFT3(x3, x2, x1, r * x1 + c * x0);
	line_search_update_variable(vs, org_var, direction, x1, cf, weight);
	SHIFT2(f2, f1, -cf->f(vs->var, param));
      }
    }

    double xmax, fmax;
    if (f1 < f2) {
      xmax = x1;
      fmax = -f1;
    } else {
      xmax = x2;
      fmax = -f2;
    }

    if (fmax <= lsw->score) {
      improved = FALSE;
      line_search_update_variable(vs, org_var, direction, 0.0, cf, weight);
    } else {
      improved = TRUE;
      line_search_update_variable(vs, org_var, direction, xmax, cf, weight);
      lsw->score = fmax;
    }
    free(org_var);
  } else {
    improved = FALSE;
  }

  return improved;
}

int Line_Search_Var(double *var, const int *var_index, int nvar,
		    const void *param, const Continuous_Function *cf,
		    const double *direction, double step)
{
  double original_score = cf->f(var, param);
  double search_score = original_score;
  double max_search_score = search_score;
  int i, j;
  int niter = 10;
  int nstep = 0;
  
  double weight = darray_sqsum(direction, nvar);

  if (weight > 1e-5) {
    step /= sqrt(darray_sqsum(direction, nvar));

    for (j = 0; j < niter; j++) {
      for (i = 0; i < nvar; i++) {
	nstep++;
	var[var_index[i]] += direction[i] * step;	
	//	validate_var(var + var_index[i], cf->var_min[var_index[i]], 
		     //		     cf->var_max[var_index[i]]);
      }
      cf->v(var, cf->var_min, cf->var_max, NULL);

      search_score = cf->f(var, param);
      if (search_score >= max_search_score) {
	max_search_score = search_score;
    } else {
	if (j > 0) { /* restore the variables if there is no improvement */
	  for (i = 0; i < nvar; i++) {
	    var[var_index[i]] -= direction[i] * step;
	    nstep--;
	  }	  
	}
	break;
      }
    }  
  }

  return nstep;
}


/*
 * Polok-Ribiere+ formula
 */
void Conjugate_Update_Direction(int nvar, const double *grad, 
				const double *prev_grad, double *direction)
{
  double *dg = darray_malloc(nvar);

  int i;
  for (i = 0; i < nvar; i++) {
    dg[i] = grad[i] - prev_grad[i];
  }

  double beta = darray_dot(grad, dg, nvar) / 
    darray_dot(prev_grad, prev_grad, nvar);

  if (beta < 0) {
    beta = 0;
  }

  for (i = 0; i < nvar; i++) {
    direction[i] *= beta;
    direction[i] += grad[i];
  } 

  free(dg);
}

void Conjugate_Update_Direction_W(int nvar, const double *grad, 
    const double *prev_grad, const double *weight, double *direction)
{
  double *dg = darray_malloc(nvar);

  int i;
  for (i = 0; i < nvar; i++) {
    dg[i] = grad[i] - prev_grad[i];
  }

  double beta;
  if (weight == NULL) {
    beta = darray_dot(grad, dg, nvar) / darray_dot(prev_grad, prev_grad, nvar);
  } else {
    double d1 = 0.0;
    double d2 = 0.0;
    for (i = 0; i < nvar; i++) {
      d1 += grad[i] * dg[i] * weight[i] * weight[i];
      d2 += prev_grad[i] * prev_grad[i] *weight[i] * weight[i];
      //d2 += dg[i] * direction[i] * weight[i];
      beta = d1 / d2;
    }
  }

  if (beta < 0) {
    beta = 0;
  }

  for (i = 0; i < nvar; i++) {
    direction[i] *= beta;
    if (weight != NULL) {
      direction[i] += grad[i];
    } else {
      direction[i] += grad[i] * weight[i];
    }
  } 

  free(dg);
}

double Conjugate_Update_Direction_Pr(int nvar, const double *grad, 
				     const double *prev_grad, 
				     double *direction)
{
  double prev_grad_norm = darray_dot(prev_grad, prev_grad, nvar);
  if (prev_grad_norm < 1e-5) {
    darraycpy(direction, grad, 0, nvar);
  }

  double *dg = darray_malloc(nvar);

  int i;
  for (i = 0; i < nvar; i++) {
    dg[i] = grad[i] - prev_grad[i];
  }

  double beta_pr = darray_dot(grad, dg, nvar) / prev_grad_norm;
  double beta_fr = darray_dot(grad, grad, nvar) / prev_grad_norm;

  double beta = beta_pr;
  if (beta_pr < -beta_fr) {
    beta = -beta_fr;
  } else if (beta_pr > beta_fr) {
    beta = beta_fr;
  }

  for (i = 0; i < nvar; i++) {
    direction[i] *= beta;
    direction[i] += grad[i];
  } 

  //printf("dot: %g\n", darray_dot(direction, grad, nvar));
  //darray_print2(direction, nvar, 1);
  //darray_print2(grad, nvar, 1);

  free(dg);
  
  return beta;

  //darraycpy(direction, grad, 0, nvar);
}

static double score_slope(const double *scores, double n)
{ 
  int i;
  double sxi = 0.0;
  double sx2 = 0.0;
  double sx = 0.0;
  for (i = 0; i < n; i++) {
    sx += scores[i];
    sxi += scores[i] * (double) i;
    sx2 += scores[i] * scores[i];
  }

  if (sx2 * n - sx * sx <= 0.0) {
    return 0.0;
  } else {
    return (sxi - sx * (n-1) / 2.0) / 
      (sqrt((sx2 * n - sx * sx) * (n - 1) * (n + 1) / 12.0));
  }
}

BOOL Optimize_Update_Variable(Variable_Set *vs, const void *param, 
			      const Continuous_Function *cf,
			      const double *delta, double *update_direction, 
			      Line_Search_Workspace *lsw,
			      Optimize_Workspace *ow)
{ 
  BOOL changed = Line_Search_Var_Backtrack(vs, param, cf, delta, NULL,
					   update_direction, lsw);

  Update_Optimize_Workspace(vs, lsw->score, ow);
  
  if (ow->phase == OPTIMIZE_CYCLE) {
    if (ow->scores[ow->phase - 1] <= ow->scores[0]) {
      double corrcoef = score_slope(ow->scores, ow->phase);
      if (corrcoef < 0.05) {
	if (ow->is_improved == FALSE) {
	  changed = FALSE;
	}
      }
    }

    ow->phase = 0;
    ow->is_improved = FALSE;
  }

  return changed;
}

#ifdef HAVE_LIBGSL

const static CBLAS_UPLO_t Sy_Uplo_Option = CblasUpper;

/* BFGS_Update_Variable(): x(k+1) = x(k) - inv(H(k))grad(k)
 * 
 * Args: nvar - number of variables;
 *       invh - inverse of the estimated Hessian matrix;
 *       grad - gradient;
 *       var - variable to update. Its values are modified by the function;
 *       s - x(k+1) - x(k).
 *
 * Return: void;
 */
void BFGS_Update_Variable(double nvar, const double *invh, const double *grad, 
			  double *var, double *s)
{
  gsl_vector_view grad_view = gsl_vector_view_array((double *) grad, nvar);
  gsl_matrix_view invh_view = gsl_matrix_view_array((double *) invh, 
						    nvar, nvar);
  gsl_vector_view var_view = gsl_vector_view_array(var, nvar);
  gsl_vector_view s_view = gsl_vector_view_array(s, nvar);

  /* s <- inv(H(k))grad(f,x(k))*/
  /*
  gsl_blas_dsymv(Sy_Uplo_Option, 1.0, &(invh_view.matrix), &(grad_view.vector), 
		 0.0, &(s_view.vector));
  */
  gsl_blas_dgemv(CblasNoTrans, 1.0, &(invh_view.matrix), &(grad_view.vector), 
		 0.0, &(s_view.vector));

  /* var <- var - s */
  gsl_vector_add(&(var_view.vector), &(s_view.vector));
  
  /* s <- -s */
  //darray_neg(s_view.vector.data, s_view.vector.size);
}

/* BFGS_Update_Y(): y(k) = grad(k+1) - grad(k)
 *
 * Args: nvar - number of variables;
 *       grad2 - grad(k+1);
 *       grad1 - grad(k);
 *       y - y(k).
 */
void BFGS_Update_Y(int nvar, double *grad2, double *grad1, double *y)
{
  darraycpy(y, grad1, 0, nvar);
  darray_sub(y, grad2, nvar);
}

void BFGS_Update_H(int nvar, double *h, double *s, double *y) 
{
  gsl_vector_view s_view = gsl_vector_view_array(s, nvar);
  gsl_vector_view y_view = gsl_vector_view_array(y, nvar);
  
  double ys;
  gsl_blas_ddot(&(y_view.vector), &(s_view.vector), &ys);
  
  printf("ys: %g\n", ys);

  gsl_matrix_view h_view = gsl_matrix_view_array(h, nvar, nvar);
  gsl_vector *hs = gsl_vector_alloc(nvar);
  /*
  gsl_blas_dsymv(Sy_Uplo_Option, 1.0, &(h_view.matrix), &(s_view.vector), 0, 
		 hs);
  */
  gsl_blas_dgemv(CblasNoTrans, 1.0, &(h_view.matrix), &(s_view.vector), 0, 
		 hs);

  gsl_matrix *y2 = gsl_matrix_calloc(nvar, nvar);  
  //gsl_blas_dsyr(Sy_Uplo_Option, 1.0, &(y_view.vector), y2);
  gsl_blas_dger(1.0, &(y_view.vector), &(y_view.vector), y2);
  gsl_matrix_scale(y2, 1.0 / ys);

  gsl_matrix_add(&(h_view.matrix), y2);

  gsl_matrix *hs2 = y2;
  //gsl_blas_dsyr(Sy_Uplo_Option, 1.0, &(y_view.vector), hs2);
  gsl_blas_dger(1.0, hs, hs, hs2);

  double hss;
  gsl_blas_ddot(hs, &(s_view.vector), &hss);

  gsl_matrix_scale(hs2, 1.0 / hss);
  gsl_matrix_sub(&(h_view.matrix), hs2);

  gsl_matrix_free(y2);
}

#endif

void Polyfit(double *x, double *y, int length, int order, double *c,
	     double *eval)
{
#ifdef HAVE_LIBGSL
  /* alloc <in> */
  gsl_matrix *in = gsl_matrix_alloc(length, order + 1);
  int i, j;
  int offset = 0;
  for (i = 0; i < length; i++) {
    in->data[offset++] = 1.0;
    for (j = 0; j < order; j++) {
      in->data[offset] = in->data[offset - 1];
      in->data[offset++] *= x[i];
    }
  }
  
  gsl_vector_view yview = gsl_vector_view_array(y, length);
  gsl_vector_view cview = gsl_vector_view_array(c, order + 1);

  double chisq;
  /* alloc <cov> */
  gsl_matrix *cov = gsl_matrix_alloc(order + 1, order + 1);

  /* alloc <work> */
  gsl_multifit_linear_workspace *work = 
    gsl_multifit_linear_alloc(length, order + 1);

  gsl_multifit_linear(in, &(yview.vector), &(cview.vector), cov, &chisq, work);
  
  /* free <work>*/
  gsl_multifit_linear_free(work);

  /* free <cov> */
  gsl_matrix_free(cov);

  if (eval != NULL) {
    gsl_vector_view evalview = gsl_vector_view_array(eval, length);
    gsl_blas_dgemv(CblasNoTrans, 1.0, in, &(cview.vector), 0.0, 
		   &(evalview.vector));
  }

  /* free <in> */
  gsl_matrix_free(in);
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
}
