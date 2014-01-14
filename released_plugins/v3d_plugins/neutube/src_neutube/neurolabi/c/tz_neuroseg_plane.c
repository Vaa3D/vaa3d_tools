/* tz_neuroseg_plane.c
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
#include "tz_3dgeom.h"
#include "tz_neuroseg_plane.h"

Neuroseg_Plane* New_Neuroseg_Plane()
{
  Neuroseg_Plane *np = (Neuroseg_Plane *) 
    Guarded_Malloc(sizeof(Neuroseg_Plane), "New_Neuroseg_Plane");

  Reset_Neuroseg_Plane(np);

  return np;
}

void Delete_Neuroseg_Plane(Neuroseg_Plane *np)
{
  free(np);
}

void Kill_Neuroseg_Plane(Neuroseg_Plane *np)
{
  Delete_Neuroseg_Plane(np);
}

void Set_Neuroseg_Plane(Neuroseg_Plane *np, double r, double theta, double psi,
			double offset_x, double offset_y)
{
  np->r = r;
  np->theta = theta;
  np->psi = psi;
  np->offset_x = offset_x;
  np->offset_y = offset_y;
}

void Reset_Neuroseg_Plane(Neuroseg_Plane *np)
{
  Set_Neuroseg_Plane(np, 1.0, 0.0, 0.0, 0.0, 0.0);
}

void Fprint_Neuroseg_Plane(FILE *fp, const Neuroseg_Plane *np)
{
  fprintf(fp, "Neuroseg plane: r: %g, theta: %g, psi: %g, offset: (%g, %g)\n",
	  np->r, np->theta, np->psi, np->offset_x, np->offset_y);
}

void Print_Neuroseg_Plane(const Neuroseg_Plane *np)
{
  Fprint_Neuroseg_Plane(stdout, np);
}

int Neuroseg_Plane_Var(const Neuroseg_Plane *np, double *var[])
{  
  var[0] = (double *) &(np->r);
  var[1] = (double *) &(np->theta);
  var[2] = (double *) &(np->psi);
  var[3] = (double *) &(np->offset_x);
  var[4] = (double *) &(np->offset_y);

  return NEUROSEG_PLANE_NPARAM;
}

void Neuroseg_Plane_Set_Var(Neuroseg_Plane *np, int var_index, double value)
{
  double *var[NEUROSEG_PLANE_NPARAM];
  Neuroseg_Plane_Var(np, var);
  *(var[var_index]) = value;
}

int Neuroseg_Plane_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, NEUROSEG_PLANE_NPARAM, var_index);
}

Geo3d_Scalar_Field* Neuroseg_Plane_Field(const Neuroseg_Plane *np, double step,
					 Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    int width = lrint((np->r * 4.0 + 1.0) / step);
    field = Make_Geo3d_Scalar_Field(width * width);
  }

  int offset = 0;
  int i;
  double x, y;
  double z_min = -0.5;
  double z_max = 0.5;
  double range = np->r + np->r;
  double range_squre = range * range;
  double weight = 0.0;
  double field_value = 0.0;
  int array_length = 0;
  double d2;
  double *coord = Coordinate_3d_Double_Array(field->points);
  double *value = field->values;

  range = np->r  * 2.0;
  range_squre = range * range;

  for (x = 0.0; x <= range; x += step) {
    for (y = 0.0; y <= range; y += step) {
      d2 = x * x + y * y;
      if ((d2 <= range_squre) && (d2 > 0.0)) { 
	field_value = Neurofield(0.0, np->r, x, y, 0.0, z_min, z_max);
	coord[0] = x + np->offset_x;
	coord[1] = y + np->offset_y;
	coord[2] = 0.0;
	coord += 3;
	value[offset++] = field_value;
	weight += fabs(field_value);

	if (x != 0.0) {
	  coord[0] = -x + np->offset_x;
	  coord[1] = y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  weight += fabs(field_value);
	}
	
	if (y != 0.0) {
	  coord[0] = x + np->offset_x;
	  coord[1] = -y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  weight += fabs(field_value);
	}

	if ((x != 0.0) && (y != 0.0)) {
	  coord[0] = -x + np->offset_x;
	  coord[1] = -y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  weight += fabs(field_value);
	}
      }
    }
  }
   
  coord[0] = np->offset_x;
  coord[1] = np->offset_y;
  coord[2] = 0.0;
  coord += 3;
  field_value = Neurofield(0.0, np->r, 0.0, 0.0, 0.0, z_min, z_max);
  value[offset++] = field_value;
  weight += fabs(field_value);

  for (i = 0; i < offset; i++) {
    value[i] /= weight;
  }
  
  array_length = offset;

  if (array_length > field->size) {
    fprintf(stderr, "Array out of bound: %d, %d\n", 
	    array_length, field->size);
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  field->size = array_length;

  if ((np->theta != 0.0) || (np->psi != 0.0)) {
    Geo3d_Point_Array_Rotate(field->points, field->size, 
			     np->theta, np->psi, 0);
  }

  return field;
}

Geo3d_Scalar_Field* Neuroseg_Plane_Field_S(const Neuroseg_Plane *np, 
					   double step,
					   Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, NEUROSEG_SLICE_FIELD_LENGTH); 
  }
  
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  int length;
  Neuroseg_Slice_Field(points, values, &length, NULL);
  field->size = length;

  int i;
  double weight = 0.0;
  for (i = 0; i < length; i++) {
    points[i][0] *= np->r;
    points[i][0] += np->offset_x;
    points[i][1] *= np->r;
    points[i][1] += np->offset_y;
    points[i][2] = 0.0;
    if (field->values[i] > 0.0) {
      field->values[i] *= sqrt(np->r);
    }
    weight += fabs(values[i]);
  }
  for (i = 0; i < length; i++) {
    values[i] /= weight;
  }

  if ((np->theta != 0.0) || (np->psi != 0.0)) {
    Rotate_XZ((double *)field->points, (double *)field->points, field->size, 
	      np->theta, np->psi, 0);
  }

  return field;
}

void Neuroseg_Plane_Fwrite(const Neuroseg_Plane *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  fwrite(seg, sizeof(*seg), 1, fp);
}

Neuroseg_Plane* Neuroseg_Plane_Fread(Neuroseg_Plane *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;

  if (seg == NULL) {
    seg = New_Neuroseg_Plane();
    is_local_alloc = TRUE;
  }
  
  if (fread(seg, sizeof(Neuroseg_Plane), 1, fp) != 1) {
    if (is_local_alloc == TRUE) {
      Delete_Neuroseg_Plane(seg);
    }
    seg = NULL;
  }

  return seg;
}
