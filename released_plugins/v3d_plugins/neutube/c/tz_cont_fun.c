/* tz_cont_fun.c
 *
 * 06-Apr-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_cont_fun.h"


Variable_Set *New_Variable_Set()
{
  Variable_Set *vs = 
    (Variable_Set *) Guarded_Malloc(sizeof(Variable_Set), "New_Variable_Set");
  vs->nvar = 0;
  vs->var = NULL;
  vs->var_index = NULL;
  vs->link = NULL;

  return vs;
}

Variable_Set *Construct_Variable_Set(Variable_Set *vs, 
				     const double *var, int nvar)
{
  ASSERT(vs != NULL, "Null pointer.");

  vs->nvar = nvar;
  vs->var_index = (int *) Guarded_Malloc(sizeof(int) * nvar,
					 "Construct_Variable_Set");
  vs->var = (double *) var;

  return vs;
}

Variable_Set *Make_Variable_Set(const double *var, int nvar)
{
  Variable_Set *vs = New_Variable_Set();
  Construct_Variable_Set(vs, var, nvar);
  return vs;
}

void Delete_Variable_Set(Variable_Set *vs)
{
  free(vs);
}

void Clean_Variable_Set(Variable_Set *vs)
{
  ASSERT(vs != NULL, "Null pointer.");

  vs->link = NULL;
  free(vs->var_index);
  vs->var_index = NULL;
  vs->nvar = 0;
  vs->var = NULL;
}

void Kill_Variable_Set(Variable_Set *vs)
{
  ASSERT(vs != NULL, "Null pointer.");
  Clean_Variable_Set(vs);
  free(vs);
}

void Variable_Set_Update_Link(Variable_Set *vs)
{
  if (vs->link != NULL) {
    int i;
    for (i = 0; i < vs->nvar; i++) {
      int remain = vs->link[i];
      while (remain > 0) {
	vs->var[remain % 100 - 1] = vs->var[vs->var_index[i]];
	remain /= 100;
      }
    }
  }
}

void Variable_Set_Add_Link(int *var_index, int master, int slave)
{
  var_index[master] = var_index[master] * 100 + slave + 1;
}

Continuous_Function* 
Make_Continuous_Function(double (*f) (const double *, const void *),
			 Validator_f v,
			 double *var_min, double *var_max)
{
  Continuous_Function *cf = 
    (Continuous_Function *) Guarded_Malloc(sizeof(Continuous_Function),
					   "Make_Continuous_Function");
  cf->f = f;
  cf->v = v;
  cf->var_min = var_min;
  cf->var_max = var_max;

  return cf;
}

void Free_Continuous_Function(Continuous_Function *cf)
{
  free(cf);
}
