/**@file tz_cont_fun.h
 * @brief continuous function
 * @author Ting Zhao
 * @date 06-Apr-2008 
 */

#ifndef _TZ_CONT_FUN_H_
#define _TZ_CONT_FUN_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

typedef struct _Variable_Set {
  double *var;    /* array of all variables */
  int *var_index; /* searchable variable indices. */
  int *link;      /* Use (i + 1) * 100 + j for multiple variables. */
  int nvar;       /* length of var_index */
} Variable_Set;

typedef struct _Variable_Set_View {
  Variable_Set vs;
} Variable_Set_View;

Variable_Set *New_Variable_Set();
Variable_Set *Construct_Variable_Set(Variable_Set *vs, 
				     const double *var, int nvar);
Variable_Set *Make_Variable_Set(const double *var, int nvar);
void Delete_Variable_Set(Variable_Set *vs);
void Clean_Variable_Set(Variable_Set *vs);
void Kill_Variable_Set(Variable_Set *vs);
void Variable_Set_Update_Link(Variable_Set *vs);
void Variable_Set_Add_Link(int *var_index, int master, int slave);
Variable_Set_View View_Variable_Set(double *var, int *var_index, int nvar);

typedef void (*Validator_f) 
(double *, const double *, const double *, const void *);

typedef struct _Continuous_Function {
  double (*f) (const double *, const void *); /* f(var | arg) */
  Validator_f v;
  double *var_min; /* min of the variables */
  double *var_max; /* max of the variables */
} Continuous_Function;

Continuous_Function* 
Make_Continuous_Function(double (*f) (const double *, const void *),
			 Validator_f v,
			 double *var_min, double *var_max);
void Free_Continuous_Function(Continuous_Function *cf);

__END_DECLS

#endif
