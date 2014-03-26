#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tz_utilities.h"
#include "tz_rpi_neuroseg.h"
#include "tz_coordinate_3d.h"
#include "tz_math.h"
#include "tz_3dgeom.h"

void Default_Rpi_Neuroseg(Rpi_Neuroseg *seg)
{
  seg->theta = 0.0;
  seg->psi = 0.0;
  seg->left_offset = -3.0;
  seg->right_offset = 3.0;
  seg->bottom_offset = -3.0;
  seg->top_offset = 3.0;
  seg->length = 11.0;
}

void Clean_Rpi_Neuroseg(Rpi_Neuroseg *seg)
{
  Default_Rpi_Neuroseg(seg);
}

void Print_Rpi_Neuroseg(Rpi_Neuroseg *seg)
{
  printf("RPI neuroseg: orientation(%g, %g), offset(L%g, R%g, B%g, T%g), "
      "length(%g)\n", seg->theta, seg->psi, seg->left_offset, seg->right_offset,
      seg->bottom_offset, seg->top_offset, seg->length);
}


DEFINE_ZOBJECT_INTERFACE(Rpi_Neuroseg)
  
int Rpi_Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, RPI_NEUROSEG_NPARAM, var_index);
}

#define RPI_NEUROSEG_SLICE_FIELD_LENGTH 20  

int Rpi_Neuroseg_Var(const Rpi_Neuroseg *seg, double *var[])
{
  var[0] = (double *) &(seg->theta);
  var[1] = (double *) &(seg->psi);
  var[2] = (double *) &(seg->left_offset);
  var[3] = (double *) &(seg->right_offset);
  var[4] = (double *) &(seg->bottom_offset);
  var[5] = (double *) &(seg->top_offset);
  var[6] = (double *) &(seg->length);

  return RPI_NEUROSEG_NPARAM;
}

void Rpi_Neuroseg_Slice_Field(const Rpi_Neuroseg *seg, 
    coordinate_3d_t *pcoord, double *value, int *length)
{
  *length = 20;

  pcoord[0][0] = -2.0 + seg->left_offset;
  pcoord[1][0] = -1.0 + seg->left_offset;
  pcoord[2][0] = 0.0 + seg->left_offset;
  pcoord[3][0] = 1.0 + seg->left_offset;
  pcoord[4][0] = 2.0 + seg->left_offset;
  pcoord[5][0] = -2.0 + seg->right_offset;
  pcoord[6][0] = -1.0 + seg->right_offset;
  pcoord[7][0] = 0.0 + seg->right_offset;
  pcoord[8][0] = 1.0 + seg->right_offset;
  pcoord[9][0] = 2.0 + seg->right_offset;

  value[0] = -2.0;
  value[1] = -1.0;
  value[2] = 0.0;
  value[3] = 1.0;
  value[4] = 2.0;
  value[5] = 2.0;
  value[6] = 1.0;
  value[7] = 0.0;
  value[8] = -1.0;
  value[9] = -2.0;

  int i;
  for (i = 0; i < 10; i++) {
    pcoord[i][1] = 0.0;
    pcoord[i][2] = 0.0;
  }

  pcoord[10][1] = -2.0 + seg->bottom_offset;
  pcoord[11][1] = -1.0 + seg->bottom_offset;
  pcoord[12][1] = 0.0 + seg->bottom_offset;
  pcoord[13][1] = 1.0 + seg->bottom_offset;
  pcoord[14][1] = 2.0 + seg->bottom_offset;
  pcoord[15][1] = -2.0 + seg->top_offset;
  pcoord[16][1] = -1.0 + seg->top_offset;
  pcoord[17][1] = 0.0 + seg->top_offset;
  pcoord[18][1] = 1.0 + seg->top_offset;
  pcoord[19][1] = 2.0 + seg->top_offset;

  value[10] = -2.0;
  value[11] = -1.0;
  value[12] = 0.0;
  value[13] = 1.0;
  value[14] = 2.0;
  value[15] = 2.0;
  value[16] = 1.0;
  value[17] = 0.0;
  value[18] = -1.0;
  value[19] = -2.0;

  for (i = 10; i < 20; i++) {
    pcoord[i][0] = 0.0;
    pcoord[i][2] = 0.0;
  }
}

Geo3d_Scalar_Field* Rpi_Neuroseg_Field(const Rpi_Neuroseg *seg,
    Geo3d_Scalar_Field *field)
{
  int nslice = iround(seg->length);
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, 
	RPI_NEUROSEG_SLICE_FIELD_LENGTH * nslice);
  }

  double z_start = 0.0;
  double z_step = 1.0;
  int length;
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  double z = z_start;

  Rpi_Neuroseg_Slice_Field(seg, points, values, &length);
  field->size = length;

  int i, j;
  for (j = 1; j < nslice; j++) {
    z += z_step;
    points += length;
    values += length;
    field->size += length;    

    memcpy(points, field->points, sizeof(coordinate_3d_t) * length);
    memcpy(values, field->values, sizeof(double) * length);
    for (i = 0; i < length; i++) {
      points[i][2] = z;
    }
  }

  if ((seg->theta != 0.0) || (seg->psi != 0.0)) {
    Rotate_XZ(Coordinate_3d_Double_Array(field->points), 
	      Coordinate_3d_Double_Array(field->points),
	      field->size, seg->theta, seg->psi, 0);
  }

  return field;
}

