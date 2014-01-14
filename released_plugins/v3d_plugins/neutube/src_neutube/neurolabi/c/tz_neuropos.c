/* tz_neuropos.c
 *
 * 23-Nov-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_geo3d_utils.h"
#include "tz_neuropos.h"

void Set_Neuropos(neuropos_t pos, double x, double y, double z)
{
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;
}

void Reset_Neuropos(neuropos_t pos)
{
  Set_Neuropos(pos, 0.0, 0.0, 0.0);
}

void Neuropos_Copy(neuropos_t pos1, const neuropos_t pos2)
{
  memcpy(pos1, pos2, sizeof(neuropos_t));
}

void Fprint_Neuropos(FILE *fp, const neuropos_t pos)
{
  fprintf(fp, "%g, %g, %g\n", pos[0], pos[1], pos[2]);
}

/* Print_Position(): Print position.
 *
 * Args: pos - the position object to print.
 *
 * Return: void.
 */
void Print_Neuropos(const neuropos_t pos)
{
  Fprint_Neuropos(stdout, pos);
}

void Neuropos_Fwrite(const neuropos_t pos, FILE *fp)
{
  fwrite(pos, sizeof(pos[0]), 3, fp);
}

neuropos_tp Neuropos_Fread(neuropos_t pos, FILE *fp)
{
  ASSERT(pos != NULL, "NULL neuropos_t is not supported.");

  if (feof(fp)) {
    return NULL;
  }

  if (fread(pos, sizeof(neuropos_t), 1, fp) != 1) {
    return NULL;
  }

  return (neuropos_tp) pos;
}

void Neuropos_Coordinate(const neuropos_t pos, double *x, double *y, double *z)
{
  *x = pos[0];
  *y = pos[1];
  *z = pos[2];
}

void Neuropos_Translate(neuropos_t pos, double x, double y, double z)
{
  Geo3d_Translate_Coordinate(pos, pos + 1, pos + 2, x, y, z);
}

void Neuropos_Relative_Coordinate(const neuropos_t pos, 
				  double *x, double *y, double *z)
{
  Geo3d_Coordinate_Offset(pos[0], pos[1], pos[2], *x, *y, *z, x, y, z);
}

void Neuropos_Absolute_Coordinate(const neuropos_t pos,
				  double *rx, double *ry, double *rz)
{
  Geo3d_Translate_Coordinate(rx, ry, rz, pos[0], pos[1], pos[2]);
}


int Neuropos_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, NEUROPOS_NPARAM, var_index);
}

int Neuropos_Var(const neuropos_t pos, double *var[])
{
  var[0] = (double *) pos;
  var[1] = (double *) pos + 1;
  var[2] = (double *) pos + 2;

  return 3;
}

void Neuropos_Set_Var(neuropos_t pos, int var_index, double value)
{
  pos[var_index] = value;
}
