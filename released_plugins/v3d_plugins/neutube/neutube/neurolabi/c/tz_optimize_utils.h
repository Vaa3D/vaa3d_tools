/**@file tz_optimize_utils.h
 * @brief optimization utilities
 * @author Ting Zhao
 * @date 23-Jan-2008
 */

#ifndef _TZ_OPTIMIZE_UTILS_H_
#define _TZ_OPTIMIZE_UTILS_H_

#include "tz_cdefs.h"
#include "tz_cont_fun.h"

__BEGIN_DECLS

#define OPTIMIZE_CYCLE 20 /* number of epochs in a cycle */

/*
 * A structure for storing intermediate results of optimization. 
 */
typedef struct _Optimize_Workspace {
  double scores[OPTIMIZE_CYCLE];    /* scores from one cycle */
  int phase;                        /* number of iterations within a cycle */
  double max_score;                 /* maximum score found so far */
  BOOL is_improved;                 /* if the score is improved in this cycle */
  int nvar;                         /* number of variables */
  double *best_var;                 /* optimal variables found so far */
} Optimize_Workspace;

Optimize_Workspace *New_Optimize_Workspace(int nvar);
void Kill_Optimize_Workspace(Optimize_Workspace *ow);
void Update_Optimize_Workspace(Variable_Set *vs, double score,
			       Optimize_Workspace *ow);


typedef struct _Line_Search_Workspace {
  int nvar;
  double alpha;
  double ro;
  double c1;
  double c2;
  double min_direction;
  double *grad;             /* the only field not necessary to initialize */
  double *start_grad;
  double score;
} Line_Search_Workspace;

Line_Search_Workspace *New_Line_Search_Workspace(int nvar);
void Set_Line_Search_Workspace(Line_Search_Workspace *lsw,
			       double alpha, double ro, double c1, double c2,
			       double min_direction);
void Kill_Line_Search_Workspace(Line_Search_Workspace *lsw);

int Line_Search_Var(double *var, const int *var_index, int nvar,
		    const void *param, const Continuous_Function *cf,
		    const double *direction, double step);

BOOL Line_Search_Var_Backtrack(Variable_Set *vs,
			       const void *param, 
			       const Continuous_Function *cf,
			       const double *delta, const double *weight,
			       double *direction,
			       Line_Search_Workspace *lsw);

BOOL Line_Search_Var_Golden_Bracket(Variable_Set *vs,
				    const void *param, 
				    const Continuous_Function *cf,
				    const double *delta, const double *weight,
				    const double *direction,
				    Line_Search_Workspace *lsw);

void Conjugate_Update_Direction(int nvar, const double *grad, 
				const double *prev_grad, double *direction);
void Conjugate_Update_Direction_W(int nvar, const double *grad, 
    const double *prev_grad, const double *weight, double *direction);

BOOL Optimize_Update_Variable(Variable_Set *vs, const void *param, 
			      const Continuous_Function *cf,
			      const double *delta, double *update_direction, 
			      Line_Search_Workspace *lsw,
			      Optimize_Workspace *ow);

double Conjugate_Update_Direction_Pr(int nvar, const double *grad, 
				     const double *prev_grad, 
				     double *direction);

/* BFGS algorithm utilities
 * http://www.math.mtu.edu/~msgocken/ma5630spring2003/lectures/global2/global2/node8.html
 */

/*
 * BGFS_Update_Variable() calculates x(k+1) and s(k). BFGS_Update_Y() calculates
 * y(k). BFGS_Update_H() calculates H(k+1).
 */

void BFGS_Update_Variable(double nvar, const double *invh, const double *grad, 
			  double *var, double *s);
void BFGS_Update_Y(int nvar, double *grad2, double *grad1, double *y);
void BFGS_Update_H(int nvar, double *h, double *s, double *y);

/*
 * Polyfit() fits a polynomial funcion for data x and y so that 
 *     y = c[0] + c[1] * x + ... + c[order] * x^order
 *
 * The evaluation of the function on x will be stored in \a eval when \a eval
 * is not NULL. \a eval could be same pointer as \a x or \a y.
 */
void Polyfit(double *x, double *y, int length, int order, double *c, 
	     double *eval);

__END_DECLS

#endif 
