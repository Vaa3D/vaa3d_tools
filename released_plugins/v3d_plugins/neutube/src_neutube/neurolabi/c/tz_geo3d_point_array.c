/* tz_geo3d_point_array.c
 *
 * 02-Mar-2008 Initial write: Ting Zhao
 */

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "tz_utilities.h"
#include "tz_3dgeom.h"
#include "tz_geo3d_point_array.h"
#include "tz_geo3d_utils.h"
#include "tz_swc_cell.h"

void Geo3d_Point_Array_Copy(coordinate_3d_t *dst, const coordinate_3d_t *src,
			    int n)
{
  memcpy(dst, src, sizeof(coordinate_3d_t) * n);
}

void Geo3d_Point_Array_Translate(coordinate_3d_t *points, int n, 
				 double dx, double dy, double dz)
{
  int i;
  for (i = 0; i < n; i++) {
    points[i][0] += dx;
    points[i][1] += dy;
    points[i][2] += dz;
  }
}

void Geo3d_Point_Array_Rotate(coordinate_3d_t *points, int n, 
			      double theta, double psi, int inverse)
{
  Rotate_XZ((double *) points, (double *) points, n, theta, psi, inverse);
}

void Geo3d_Point_Array_Scale(coordinate_3d_t *points, int n, 
			     double sx, double sy, double sz)
{
  int i;
  for (i = 0; i < n; i++) {
    points[i][0] *= sx;
    points[i][1] *= sy;
    points[i][2] *= sz;
  }
}

void Geo3d_Point_Array_Bend(coordinate_3d_t *points, int n, double c)
{
  int i;
  double d;
  for (i = 0; i < n; i++) {
    d = c - points[i][1];
    if (d == 0.0) {
      points[i][1] = 0.0;
      points[i][2] = 0.0;
    } else { 
      points[i][2] /= d;
      points[i][1] += d - d * cos(points[i][2]);
      points[i][2] = sin(points[i][2]);
    }
    //points[i][1] += d - d * sqrt(1.0 - points[i][2] * points[i][2]);

    points[i][2] *= d;
  }
}

double** Geo3d_Point_Array_Distmat(coordinate_3d_t *points, int n,
				   double **dist)
{
  int i, j;

  if (dist == NULL) {
    /*** for triangle form (maybe used in the future) ****/
    /*
    dist = (double **) malloc(sizeof(double *) * n);

    dist[0] = NULL;

    for (i = 1; i < n; i++) {
      dist[i] = (double *) Guarded_Malloc(sizeof(double) * i);
    }
    */
    MALLOC_2D_ARRAY(dist, n, n, double, i);
  }

  for (i = 0; i < n; i++) {
    dist[i][i] = 0.0;
  }

  for (i = 1; i < n; i++) {
    for (j = 0; j < i; j++) {
      dist[i][j] = Coordinate_3d_Distance(points[i], points[j]);
      dist[j][i] = dist[i][j];
    }
  }

  return dist;
}

int Geo3d_Point_Array_Closest(coordinate_3d_t *points, int n,
			      const coordinate_3d_t source, double *md)
{
  int index = 0;
  double min_dist = Geo3d_Dist_Sqr(points[0][0], points[0][1], points[0][2],
				   source[0], source[1], source[2]);

  int i;
  for (i = 1; i < n; i++) {
    double dist = Geo3d_Dist_Sqr(points[i][0], points[i][1], points[i][2],
				 source[0], source[1], source[2]);
    if (dist < min_dist) {
      min_dist = dist;
      index = i;
    }
  }
  
  if (md != NULL) {
    *md = min_dist;
  }

  return index;
}

void Geo3d_Point_Array_Swc_Fprint(FILE *fp, coordinate_3d_t *points, int n,
				  int start_id, int parent_id, double radius,
				  int type)
{
  Swc_Node node;
  int i;
  Default_Swc_Node(&node);
  node.type = type;
  node.parent_id = parent_id;
  for (i = 0; i < n; i++) {
    node.id = start_id + i;
    node.d = radius;
    node.x = points[i][0];
    node.y = points[i][1];
    node.z = points[i][2];
    Swc_Node_Fprint(fp, &node);
    node.parent_id = node.id;
  }
}

DEFINE_ZOBJECT_INTERFACE(Geo3d_Point_Array)

void Default_Geo3d_Point_Array(Geo3d_Point_Array *pa)
{
  pa->n = 0;
  pa->array = NULL;
}

void Clean_Geo3d_Point_Array(Geo3d_Point_Array *pa)
{
  free(pa->array);
  Default_Geo3d_Point_Array(pa);
}

void Print_Geo3d_Point_Array(Geo3d_Point_Array *pa)
{
  printf("%d 3D points:\n", pa->n);

  int i;
  for (i = 0; i < pa->n; i++) {
    Print_Coordinate_3d(pa->array[i]);
  }
}

Geo3d_Point_Array* Make_Geo3d_Point_Array(int n)
{
  Geo3d_Point_Array *pa = New_Geo3d_Point_Array();
  pa->n = n;
  GUARDED_MALLOC_ARRAY(pa->array, n, coordinate_3d_t);
  return pa;
}
