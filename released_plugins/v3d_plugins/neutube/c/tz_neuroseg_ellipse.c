/* tz_neuroseg_ellipse.c
 *
 * 17-Apr-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_geo3d_point_array.h"
#include "tz_neurofield.h"
#include "tz_neuroseg.h"
#include "tz_neuroseg_ellipse.h"

Neuroseg_Ellipse* New_Neuroseg_Ellipse()
{
  Neuroseg_Ellipse *np = (Neuroseg_Ellipse *) 
    Guarded_Malloc(sizeof(Neuroseg_Ellipse), "New_Neuroseg_Ellipse");

  Reset_Neuroseg_Ellipse(np);

  return np;
}

void Delete_Neuroseg_Ellipse(Neuroseg_Ellipse *np)
{
  free(np);
}

void Kill_Neuroseg_Ellipse(Neuroseg_Ellipse *np)
{
  Delete_Neuroseg_Ellipse(np);
}

void Set_Neuroseg_Ellipse(Neuroseg_Ellipse *np, double rx, double ry,
			  double theta, double psi, double alpha,
			  double offset_x, double offset_y)
{
  np->rx = rx;
  np->ry = ry;
  np->theta = theta;
  np->psi = psi;
  np->alpha = alpha;
  np->offset_x = offset_x;
  np->offset_y = offset_y;
}

void Reset_Neuroseg_Ellipse(Neuroseg_Ellipse *np)
{
  Set_Neuroseg_Ellipse(np, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

void Fprint_Neuroseg_Ellipse(FILE *fp, const Neuroseg_Ellipse *np)
{
  fprintf(fp, "Neuroseg plane: r: (%g, %g), theta: %g, psi: %g, offset: (%g, %g)\n",
	  np->rx, np->ry, np->theta, np->psi, np->offset_x, np->offset_y);
}

void Print_Neuroseg_Ellipse(const Neuroseg_Ellipse *np)
{
  Fprint_Neuroseg_Ellipse(stdout, np);
}

void Neuroseg_Ellipse_Fwrite(const Neuroseg_Ellipse *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  fwrite(seg, sizeof(*seg), 1, fp);
}

Neuroseg_Ellipse* Neuroseg_Ellipse_Fread(Neuroseg_Ellipse *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;

  if (seg == NULL) {
    seg = New_Neuroseg_Ellipse();
    is_local_alloc = TRUE;
  }
  
  if (fread(seg, sizeof(Neuroseg_Ellipse), 1, fp) != 1) {
    if (is_local_alloc == TRUE) {
      Delete_Neuroseg_Ellipse(seg);
    }
    seg = NULL;
  }

  return seg;
}



int Neuroseg_Ellipse_Var(const Neuroseg_Ellipse *np, double *var[])
{  
  var[0] = (double *) &(np->rx);
  var[1] = (double *) &(np->ry);
  var[2] = (double *) &(np->theta);
  var[3] = (double *) &(np->psi);
  var[4] = (double *) &(np->offset_x);
  var[5] = (double *) &(np->offset_y);
  var[6] = (double *) &(np->alpha);

  return NEUROSEG_ELLIPSE_NPARAM;
}

void Neuroseg_Ellipse_Set_Var(Neuroseg_Ellipse *np, int var_index, double value)
{
  double *var[NEUROSEG_ELLIPSE_NPARAM];
  Neuroseg_Ellipse_Var(np, var);
  *(var[var_index]) = value;
}

int Neuroseg_Ellipse_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, NEUROSEG_ELLIPSE_NPARAM, var_index);
}

Geo3d_Scalar_Field* Neuroseg_Ellipse_Field(const Neuroseg_Ellipse *np, double step,
					 Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, NEUROSEG_SLICE_FIELD_LENGTH);
  }

  Neuroseg_Slice_Field(field->points, field->values, &(field->size), NULL);

  int i;
  double weight = 0.0;
  for (i = 0; i < field->size; i++) {
    field->points[i][0] *= np->rx;
    field->points[i][1] *= np->ry;
    field->points[i][2] = 0.0;

    if (field->values[i] > 0.0) {
      field->values[i] *= sqrt(np->rx * np->ry);
    }
    weight += fabs(field->values[i]);
  }

  for (i = 0; i < field->size; i++) {
    field->values[i] /= weight;
  }


  if (np->alpha != 0.0) {
    Geo3d_Point_Array_Rotate(field->points, field->size, 
			     0.0, np->alpha, 0);
  }

  Geo3d_Point_Array_Translate(field->points, field->size, 
			      np->offset_x, np->offset_y, 0.0);


  Geo3d_Point_Array_Rotate(field->points, field->size, 
			   np->theta, np->psi, 0);

  return field;
}

#if 0
Geo3d_Scalar_Field* Neuroseg_Ellipse_Field(const Neuroseg_Ellipse *np, double step,
					 Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    int width = (int) round((np->rx * 2.0 + np->ry * 2.0 + 1.0) / step);
    field = Make_Geo3d_Scalar_Field(width * width);
  }

  int offset = 0;
  int i;
  double x, y;
  double range_x = np->rx + np->rx;
  double range_y = np->ry + np->ry;
  double range = 2.0;
  double range_squre = range * range;
  double weight_pos = 0.0;
  double weight_neg = 0.0;
  double field_value = 0.0;
  int array_length = 0;
  double d2;
  double *coord = Coordinate_3d_Double_Array(field->points);
  double *value = field->values;

  double sigma_x2 = np->rx * np->rx;
  double sigma_y2 = np->ry * np->ry;
  double np_size = np->rx * np->ry;
  double norm_x, norm_y;
  double norm_x2, norm_y2;
  
  for (x = 0.0; x <= range_x; x += step) {
    for (y = 0.0; y <= range_y; y += step) {
      norm_x = x / np->rx;
      norm_y = y / np->ry;

      d2 = norm_x * norm_x + norm_y * norm_y;
      if ((d2 <= range_squre) && (d2 > 0.0)) { 
	norm_x2 = norm_x * norm_x;
	norm_y2 = norm_y * norm_y;	
	field_value = 
	  (1.0 - norm_x2 - 
	   norm_y2) * 
	  exp(- (norm_x2 + norm_y2));
	//field_value = exp(- (norm_x2 + norm_y2) / 2.0);

	coord[0] = x + np->offset_x;
	coord[1] = y + np->offset_y;
	coord[2] = 0.0;
	coord += 3;
	value[offset++] = field_value;
	if (field_value > 0.0) {
	  weight_pos += field_value;
	} else if (field_value < 0.0) {
	  weight_neg += field_value;
	}

	if (x != 0.0) {
	  coord[0] = -x + np->offset_x;
	  coord[1] = y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  if (field_value > 0.0) {
	    weight_pos += field_value;
	  } else if (field_value < 0.0) {
	    weight_neg += field_value;
	  }
	}
	
	if (y != 0.0) {
	  coord[0] = x + np->offset_x;
	  coord[1] = -y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  if (field_value > 0.0) {
	    weight_pos += field_value;
	  } else if (field_value < 0.0) {
	    weight_neg += field_value;
	  }
	}

	if ((x != 0.0) && (y != 0.0)) {
	  coord[0] = -x + np->offset_x;
	  coord[1] = -y + np->offset_y;
	  coord[2] = 0.0;
	  coord += 3;
	  value[offset++] = field_value;
	  if (field_value > 0.0) {
	    weight_pos += field_value;
	  } else if (field_value < 0.0) {
	    weight_neg += field_value;
	  }
	}
      }
    }
  }
   
  coord[0] = np->offset_x;
  coord[1] = np->offset_y;
  coord[2] = 0.0;
  coord += 3;
  field_value = 1.0 / sigma_x2 + 1.0 / sigma_y2;
  value[offset++] = field_value;
  weight_pos += field_value;

  
  //double coeff_pos = np_size / (np_size + 1.0) / weight_pos;
  double coeff_pos = 1.0 / weight_pos;
  double coeff_neg = -2.0 / sqrt(np_size) / weight_neg;
  
  //double coeff_neg = 1.0 / (weight_pos - weight_neg);
  //double coeff_pos = coeff_neg * sqrt(np_size) / 2.0;
  

  //  printf("%g, %g\n", weight_pos, weight_neg);
  //printf("%g, %g\n", coeff_pos, coeff_neg);

  double weight = 0.0;
  for (i = 0; i < offset; i++) {
    if (value[i] > 0.0) {
      weight += value[i];
      value[i] *= coeff_pos;
    } else if (value[i] < 0.0) {
      value[i] *= coeff_neg;
    }

  }
  
  //printf("%g\n", weight);
  
  array_length = offset;

  if (array_length > field->size) {
    fprintf(stderr, "Array out of bound: %d, %d\n", 
	    array_length, field->size);
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  field->size = array_length;

  if (np->alpha != 0.0) {
    Geo3d_Point_Array_Rotate(field->points, field->size, 
			     0.0, np->alpha, 0);
  }

  if ((np->theta != 0.0) || (np->psi != 0.0)) {
    Geo3d_Point_Array_Rotate(field->points, field->size, 
			     np->theta, np->psi, 0);
  }

  return field;
}
#endif

void Neuroseg_Ellipse_Points(const Neuroseg_Ellipse *np, int npt, 
			     double start, coordinate_3d_t coord[])
{
  double step = TZ_2PI / npt;
  int i;
  double t = start;
  for (i = 0; i < npt; i++) {
    coord[i][0] = np->rx * cos(t) * cos(np->alpha) - 
      np->ry * sin(t) * sin(np->alpha) + np->offset_x;
    coord[i][1] = np->rx * cos(t) * sin(np->alpha) + 
      np->ry * sin(t) * cos(np->alpha) + np->offset_y;
    coord[i][2] = 0;
    t += step;
  }

  Geo3d_Point_Array_Rotate(coord, npt, np->theta, np->psi, 0);
}

void Neuroseg_Ellipse_Ortvec(const Neuroseg_Ellipse *ne,
			       coordinate_3d_t coord)
{
  coord[0] = cos(ne->psi) * cos(ne->alpha) - 
    cos(ne->theta) * sin(ne->psi) * sin(ne->alpha);
  coord[1] = sin(ne->psi) * cos(ne->alpha) + 
    cos(ne->theta) * cos(ne->psi) * sin(ne->alpha);
  coord[2] = sin(ne->theta) * sin(ne->alpha);
}

void Neuroseg_Ellipse_Secortvec(const Neuroseg_Ellipse *ne,
			       coordinate_3d_t coord)
{
  coord[0] = -cos(ne->psi) * sin(ne->alpha) - 
    cos(ne->theta) * sin(ne->psi) * cos(ne->alpha);
  coord[1] = -sin(ne->psi) * sin(ne->alpha) + 
    cos(ne->theta) * cos(ne->psi) * cos(ne->alpha);
  coord[2] = sin(ne->theta) * cos(ne->alpha);
}

void Neuroseg_Ellipse_Normvec(const Neuroseg_Ellipse *ne,
			      coordinate_3d_t coord)
{
  coord[0] = sin(ne->theta) * sin(ne->psi);
  coord[1] = -sin(ne->theta) * cos(ne->psi);
  coord[2] = cos(ne->theta);
}

double Neuroseg_Ellipse_Vector_Angle(const Neuroseg_Ellipse *ne,
				     const coordinate_3d_t coord)
{
  if (Coordinate_3d_Length_Square(coord) == 0.0) {
    return 0.0;
  }

  coordinate_3d_t ortvec;
  coordinate_3d_t secortvec;

  Neuroseg_Ellipse_Ortvec(ne, ortvec);
  Neuroseg_Ellipse_Secortvec(ne, secortvec);

  coordinate_3d_t normcoord;
  normcoord[0] = Coordinate_3d_Dot(ortvec, coord) / ne->rx;
  normcoord[1] = Coordinate_3d_Dot(secortvec, coord) / ne->ry;
  
  double a = normcoord[0] / 
    sqrt(normcoord[0] * normcoord[0] + normcoord[1] * normcoord[1]);

  if (fabs(a) > 1.0) {
    a = round(a);
  }

  a = acos(a);

  if (a != 0.0) {
    coordinate_3d_t cross;
    coordinate_3d_t normvec;
    Neuroseg_Ellipse_Normvec(ne, normvec);
    Coordinate_3d_Cross(ortvec, coord, cross);
    if (Coordinate_3d_Dot(cross, normvec) < 0.0) {
      a = TZ_2PI - a;
    }
  }
  
  return a;
}
