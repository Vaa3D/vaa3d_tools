/**@file tz_geo3d_point_array.h
 * @brief 3d point array
 * @author Ting Zhao
 * @date 02-Mar-2008
 */

#ifndef _TZ_GEO3D_POINT_ARRAY_H_
#define _TZ_GEO3D_POINT_ARRAY_H_ 

#include "tz_cdefs.h"
#include "tz_coordinate_3d.h"
#include "tz_zobject.h"

__BEGIN_DECLS

void Geo3d_Point_Array_Copy(coordinate_3d_t *dst, const coordinate_3d_t *src,
			    int n);

/*
 * Geo3d_Point_Array_Translate() translates points by (<dx>, <dy>, <dz>). <n>
 * is the number of points to translate. <points> is changed after the function
 * call.
 */
void Geo3d_Point_Array_Translate(coordinate_3d_t *points, int n, 
				 double dx, double dy, double dz);

/*
 * Geo3d_Point_Array_Rotate() rotates points by <theta> around X and <psi> 
 * around Z axis when <inverse> is 0. The function inverses the rotation 
 * if <inverse> is not 0.
 */
void Geo3d_Point_Array_Rotate(coordinate_3d_t *points, int n, 
			      double theta, double psi, int inverse);

void Geo3d_Point_Array_Scale(coordinate_3d_t *points, int n, 
			     double sx, double sy, double sz);

/*
 * Geo3d_Point_Array_Bend() bends z axis to do point transformations. 
 * <radius> indicates how much it bends. Z axis becomes a circle and
 * <radius> is the radius of the circle.
 */
void Geo3d_Point_Array_Bend(coordinate_3d_t *points, int n, double radius);

/*
 * Geo3d_Point_Array_Distmat() creates a distance matrix for a set of points.
 * The result is stored in <dist> and the returned pointer is also <dist> if 
 * <dist> is not NULL; otherwise the result is returned as a new pointer.
 */
double** Geo3d_Point_Array_Distmat(coordinate_3d_t *points, int n,
				   double **dist);

int Geo3d_Point_Array_Closest(coordinate_3d_t *points, int n,
			      const coordinate_3d_t source, double *md);

void Geo3d_Point_Array_Swc_Fprint(FILE *fp, coordinate_3d_t *points, int n,
				  int start_id, int parent_id, double radius,
				  int type);

typedef struct _Geo3d_Point_Array {
  int n;
  coordinate_3d_t *array;
} Geo3d_Point_Array;

DECLARE_ZOBJECT_INTERFACE(Geo3d_Point_Array)

Geo3d_Point_Array* Make_Geo3d_Point_Array(int n);

__END_DECLS

#endif
