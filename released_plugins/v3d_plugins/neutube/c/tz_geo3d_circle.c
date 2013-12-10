/* tz_geo3d_circle.c
 *
 * 18-Mar-2008 Initial write: Ting Zhao
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_geo3d_utils.h"
#include "tz_geo3d_circle.h"
#include "tz_swc_cell.h"
#include "tz_math.h"

Geo3d_Circle* New_Geo3d_Circle()
{
  Geo3d_Circle* circle = (Geo3d_Circle *) Guarded_Malloc(sizeof(Geo3d_Circle),
							 "New_Geo3d_Circle");

  Geo3d_Circle_Default(circle);
  return circle;
}

void Delete_Geo3d_Circle(Geo3d_Circle *circle)
{
  free(circle);
}

void Kill_Geo3d_Circle(Geo3d_Circle *circle)
{
  Delete_Geo3d_Circle(circle);
}

void Geo3d_Circle_Default(Geo3d_Circle *circle)
{
  circle->radius = 1.0;
  circle->center[0] = 0.0;
  circle->center[1] = 0.0;
  circle->center[2] = 0.0;
  circle->orientation[0] = 0.0;
  circle->orientation[1] = 0.0;
}

void Geo3d_Circle_Copy(Geo3d_Circle *dst, const Geo3d_Circle *src)
{
  memcpy(dst, src, sizeof(Geo3d_Circle));
}

Geo3d_Circle* Copy_Geo3d_Circle(Geo3d_Circle *src)
{
  Geo3d_Circle *circle = New_Geo3d_Circle();
  Geo3d_Circle_Copy(circle, src);

  return circle;
}

void Print_Geo3d_Circle(const Geo3d_Circle *circle)
{
  printf("3D circle [ radius: %g; center: %g, %g, %g; orientation: %g, %g ]\n", 
	 circle->radius, circle->center[0], circle->center[1], 
	 circle->center[2], circle->orientation[0], circle->orientation[1]);
}

void Geo3d_Circle_Points(const Geo3d_Circle *circle, int npt, 
			 const coordinate_3d_t start,
			 coordinate_3d_t coord[])
{
  double step = TZ_2PI / npt;
  double normal[3];
  Geo3d_Orientation_Normal(circle->orientation[0], circle->orientation[1], 
			   normal, normal + 1, normal + 2);
  double tangent[3];
  Geo3d_Cross_Product(normal[0], normal[1], normal[2],
		      start[0], start[1], start[2],
		      tangent, tangent + 1, tangent + 2);
		
  int i, j;
  double t = 0.0;
  for (i = 0; i < npt; i++) {
    double rcos = cos(t);
    double rsin = sin(t);
    for (j = 0; j < 3; j++) {
      coord[i][j] = rcos * start[j] + rsin * tangent[j] + circle->center[j];
    }
    t += step;
  }
}

void Geo3d_Circle_Rotate_Vector(const Geo3d_Circle *circle, double angle,
			       coordinate_3d_t coord)
{
  double normal[3];
  Geo3d_Orientation_Normal(circle->orientation[0], circle->orientation[1], 
			   normal, normal + 1, normal + 2);
  double tangent[3];
  Geo3d_Cross_Product(normal[0], normal[1], normal[2],
		      coord[0], coord[1], coord[2],
		      tangent, tangent + 1, tangent + 2);
		
  double rcos = cos(angle);
  double rsin = sin(angle);
  int j;
  for (j = 0; j < 3; j++) {
    coord[j] = rcos * coord[j] + rsin * tangent[j];
  }
}

double Geo3d_Circle_Vector_Angle(const Geo3d_Circle *circle,
				 coordinate_3d_t v1, coordinate_3d_t v2)
{
  double normal[3];
  Geo3d_Orientation_Normal(circle->orientation[0], circle->orientation[1], 
			   normal, normal + 1, normal + 2);
  double tangent[3];
  Geo3d_Cross_Product(normal[0], normal[1], normal[2],
		      v1[0], v1[1], v1[2],
		      tangent, tangent + 1, tangent + 2);

  double angle = Geo3d_Angle2(v1[0], v1[1], v1[2], v2[0], v2[1], v2[2]);
  if (Geo3d_Dot_Product(v2[0], v2[1], v2[2], tangent[0], tangent[1], tangent[2])
      < 0.0) {
    angle = TZ_2PI - angle;
  }

  return angle;
}

int Geo3d_Circle_Plane_Point(const Geo3d_Circle *circle, 
			     const xz_orientation_t plane_ort,
			     int direction,
			     coordinate_3d_t coord)
{
  if (direction == 1) {
    Xz_Orientation_Cross(circle->orientation, plane_ort, coord);
  } else if (direction == -1) {
    Xz_Orientation_Cross(plane_ort, circle->orientation, coord);
  } else {
    TZ_WARN(ERROR_DATA_VALUE);
    return 0;
  }
  
  double length = Coordinate_3d_Norm(coord);
  
  /* the cross product is 0 if the two planes are parallel. Here we use
   * 0.001 as the threshold for rounding error. */
  if (length < 0.001) {
    return 0;
  } else {
    Coordinate_3d_Scale(coord, circle->radius / length);
  }

  //Geo3d_Translate_Coordinate(coord, coord + 1, coord + 2, circle->center[0],
  // 			     circle->center[1], circle->center[2]);

  return 1;
}

int Geo3d_Unit_Circle_Plane_Point(const xz_orientation_t circle_ort,
				  const xz_orientation_t plane_ort,
				  int direction,
				  coordinate_3d_t coord)
{
  if (direction == 1) {
    Xz_Orientation_Cross(circle_ort, plane_ort, coord);
  } else if (direction == -1) {
    Xz_Orientation_Cross(plane_ort, circle_ort, coord);
  } else {
    TZ_WARN(ERROR_DATA_VALUE);
    return 0;
  }
  
  double length = Coordinate_3d_Norm(coord);
  
  /* the cross product is 0 if the two planes are paralle. Here we use
   * 0.001 as the threshold for rounding error. */
  if (length < 0.001) {
    return 0;
  } else {
    Coordinate_3d_Scale(coord, 1.0 / length);
  }

  return 1;
}

coordinate_3d_t* 
Geo3d_Circle_Array_Points(const Geo3d_Circle *circle, int ncircle,
			 int npt_per_c, coordinate_3d_t *pts)
{
  int nsample = npt_per_c;

  if (pts == NULL) {
    pts = (coordinate_3d_t *) 
      Guarded_Malloc(sizeof(coordinate_3d_t) * ncircle * npt_per_c,
		     "Geo3d_Circle_Array_Points");
  }
  
  coordinate_3d_t *pts_head = pts;

  const Geo3d_Circle *bottom = &(circle[0]);
  const Geo3d_Circle *top = &(circle[1]);

  coordinate_3d_t plane_normal;
  Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
  Coordinate_3d_Unitize(plane_normal);

  xz_orientation_t plane_ort;
  /* 0.001 is picked to test if two vectors are parallel */
  if (Coordinate_3d_Norm(plane_normal) < 0.001) {
    /* pick a plane that is not parallel to the circles */
    plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
    plane_ort[1] = bottom->orientation[1];
  } else {
    Geo3d_Normal_Orientation(plane_normal[0], plane_normal[1], 
			     plane_normal[2], plane_ort, plane_ort + 1);
  }

  coordinate_3d_t start1;
  Geo3d_Circle_Plane_Point(bottom, plane_ort, 1, start1);
  Geo3d_Circle_Points(bottom, nsample, start1, pts);

  coordinate_3d_t start2;
  Geo3d_Circle_Plane_Point(top, plane_ort, 1, start2);
  pts += nsample;
  Geo3d_Circle_Points(top, nsample, start2, pts);

  
  int i;
  for (i = 1; i < ncircle - 1; i++) {
    bottom = &(circle[i]);
    top = &(circle[i + 1]);
    Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
    Coordinate_3d_Unitize(plane_normal);
    
    if (Coordinate_3d_Norm(plane_normal) < 0.001) {
      /* pick a plane that is not parallel to the circles */
      plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
      plane_ort[1] = bottom->orientation[1];
    } else {
      Geo3d_Normal_Orientation(plane_normal[0], plane_normal[1], 
			       plane_normal[2], plane_ort, plane_ort + 1);
    }
    
    Geo3d_Circle_Plane_Point(bottom, plane_ort, 1, start1);
    double angle = Geo3d_Circle_Vector_Angle(bottom, start1, start2);
    Geo3d_Circle_Plane_Point(top, plane_ort, 1, start2);
    Geo3d_Circle_Rotate_Vector(top, angle, start2);
    pts += nsample;
    Geo3d_Circle_Points(top, nsample, start2, pts);
  }

  return pts_head;
}

Geo3d_Circle* Make_Geo3d_Circle_Array(int n)
{
  Geo3d_Circle *circle = (Geo3d_Circle *) 
    Guarded_Malloc(sizeof(Geo3d_Circle) * n, "Make_Geo3d_Circle_Array");
  int i;
  for (i = 0; i < n; i++) {
    Geo3d_Circle_Default(circle + i);
  }

  return circle;
}

void Geo3d_Circle_Swc_Fprint(FILE *fp, const Geo3d_Circle *circle,
			     int id, int parent_id)
{
  Geo3d_Circle_Swc_Fprint_Z(fp, circle, id, parent_id, 1.0);
}

void Geo3d_Circle_Swc_Fprint_T(FILE *fp, const Geo3d_Circle *circle,
			       int id, int parent_id, int type, double z_scale)
{
  Swc_Node cell;
  cell.id = id;
  cell.type = type;
  cell.d = circle->radius;
  cell.x = circle->center[0];
  cell.y = circle->center[1];
  cell.z = circle->center[2];
  if (z_scale != 1.0) {
    cell.z /= z_scale;
  }
  cell.parent_id = parent_id;
  Swc_Node_Fprint(fp, &cell);
}

void Geo3d_Circle_Swc_Fprint_Z(FILE *fp, const Geo3d_Circle *circle, int id,
			       int parent_id, double z_scale)
{
  Geo3d_Circle_Swc_Fprint_T(fp, circle, id, parent_id, 2, z_scale);
}

Swc_Node* Geo3d_Circle_To_Swc_Node(const Geo3d_Circle *circle,
				   int id, int parent_id, double z_scale,
				   int type, Swc_Node *node)
{
  if (node == NULL) {
    node = New_Swc_Node();
  }

  if (circle->radius <= 0.0) { 
    /* virtual node for a circle with invalid size */
    node->id = -1;
  } else {
    node->id = id;
  }
  node->type = type;
  node->d = circle->radius;
  node->x = circle->center[0];
  node->y = circle->center[1];
  node->z = circle->center[2];
  if (z_scale != 1.0) {
    node->z /= z_scale;
  }
  node->parent_id = parent_id;

  return node;
}
