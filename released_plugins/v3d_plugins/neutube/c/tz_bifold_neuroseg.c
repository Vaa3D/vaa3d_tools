/* tz_bifold_neuroseg.c
 *
 * 17-Apr-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_geo3d_point_array.h"
#include "tz_neurofield.h"
#include "tz_neuroseg.h"
#include "tz_bifold_neuroseg.h"

Bifold_Neuroseg* New_Bifold_Neuroseg()
{
  Bifold_Neuroseg *bn = (Bifold_Neuroseg *) 
    Guarded_Malloc(sizeof(Bifold_Neuroseg), "New_Bifold_Neuroseg");

  Reset_Bifold_Neuroseg(bn);

  return bn;
}

void Delete_Bifold_Neuroseg(Bifold_Neuroseg *bn)
{
  free(bn);
}

void Set_Bifold_Neuroseg(Bifold_Neuroseg *bn, double r1, double r2, double r3,
			 double r4, double h, double knot,
			 double theta2, double psi2,
			 double theta, double psi)
{
  bn->r1 = r1;
  bn->r2 = r2;
  bn->r3 = r3;
  bn->r4 = r4;
  bn->h = h;
  bn->knot = knot;
  bn->theta2 = theta2;
  bn->psi2 = psi2;
  bn->theta = theta;
  bn->psi = psi;
}

void Reset_Bifold_Neuroseg(Bifold_Neuroseg *bn)
{
  Set_Bifold_Neuroseg(bn, 1.0, 1.0, 1.0, 1.0, 12.0, 0.5, 0.0, 0.0, 0.0, 0.0);
}

void Fprint_Bifold_Neuroseg(FILE *fp, const Bifold_Neuroseg *bn)
{
  fprintf(fp, "Bifold neuroseg: r: (%g, %g, %g, %g); h: (%g, %g), orientation 2: (%g, %g), overall orientation: (%g, %g)\n",
	  bn->r1, bn->r2, bn->r3, bn->r4, bn->h, bn->knot, 
	  bn->theta2, bn->psi2, bn->theta, bn->psi);
}

void Print_Bifold_Neuroseg(const Bifold_Neuroseg *bn)
{
  Fprint_Bifold_Neuroseg(stdout, bn);
}

int Bifold_Neuroseg_Var(const Bifold_Neuroseg *bn, double *var[])
{  
  var[0] = (double *) &(bn->r1);
  var[1] = (double *) &(bn->r2);
  var[2] = (double *) &(bn->r3);
  var[3] = (double *) &(bn->r4);
  var[4] = (double *) &(bn->h);
  var[5] = (double *) &(bn->knot);
  var[6] = (double *) &(bn->theta2);
  var[7] = (double *) &(bn->psi2);
  var[8] = (double *) &(bn->theta);
  var[9] = (double *) &(bn->psi);

  return BIFOLD_NEUROSEG_NPARAM;
}

void Bifold_Neuroseg_Set_Var(Bifold_Neuroseg *bn, int var_index, double value)
{
  double *var[BIFOLD_NEUROSEG_NPARAM];
  Bifold_Neuroseg_Var(bn, var);
  *(var[var_index]) = value;
}

int Bifold_Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, BIFOLD_NEUROSEG_NPARAM, var_index);
}

Geo3d_Scalar_Field* Bifold_Neuroseg_Field(const Bifold_Neuroseg *bn,
					  Geo3d_Scalar_Field *field)
{
  Neuroseg *seg = New_Neuroseg();

  double c1 = (bn->r2 - bn->r1) / bn->h;
  double c2 = (bn->r4 - bn->r3) / bn->h;
  if (bn->knot < BIFOLD_NEUROSEG_MIN_KNOT) {
    Set_Neuroseg(seg, bn->r1, c1, bn->h, bn->theta2, bn->psi2, 0.0, 
		 0.0, 1.0);
    field = Neuroseg_Field_S(seg, NULL, NULL);
  } else if (bn->knot > BIFOLD_NEUROSEG_MAX_KNOT) {
    Set_Neuroseg(seg, bn->r1, c1, bn->h, bn->theta, bn->psi, 0.0, 0.0, 1.0);
    field = Neuroseg_Field_S(seg, NULL, NULL);
  } else {
    //Set_Neuroseg(seg, bn->r1, bn->r2, bn->h * bn->knot, 0.0, 0.0);
    Set_Neuroseg(seg, bn->r1, c1, bn->h, 0.0, 0.0, 0.0, 0.0, 1.0);
    Geo3d_Scalar_Field *field1 = Neuroseg_Field_S(seg, NULL, NULL);
    
    Set_Neuroseg(seg, bn->r3, c2, bn->h * (1 - bn->knot), 
		 bn->theta2, bn->psi2, 0.0, 0.0, 1.0);
    Geo3d_Scalar_Field *field2 = Neuroseg_Field_S(seg, NULL, NULL);

    Geo3d_Point_Array_Translate(field2->points, field2->size, 0.0, 0.0, 
				bn->h * bn->knot);

    field = Geo3d_Scalar_Field_Merge(field1, field2, field);

    Kill_Geo3d_Scalar_Field(field1);
    Kill_Geo3d_Scalar_Field(field2);
    
    ASSERT(field != NULL, "NULL field");
    
    Geo3d_Point_Array_Rotate(field->points, field->size, bn->theta, bn->psi, 0);
  }

  Delete_Neuroseg(seg);

  return field;
}
