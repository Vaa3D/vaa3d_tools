#include <math.h>
#include "tz_geo3d_ellipse.h"
#include "tz_geometry.h"
#include "tz_geo3d_utils.h"
#include "tz_geoangle_utils.h"
#include "tz_constant.h"
#include "tz_geo3d_transform.h"
#include "tz_geo3d_point_array.h"

Geo3d_Ellipse* New_Geo3d_Ellipse()
{
  Geo3d_Ellipse *ellipse = 
    (Geo3d_Ellipse*) Guarded_Malloc(sizeof(Geo3d_Ellipse),
				    "New_Geo3d_Ellipse");

  Default_Geo3d_Ellipse(ellipse);

  return ellipse;
}

void Delete_Geo3d_Ellipse(Geo3d_Ellipse *ellipse)
{
  free(ellipse);
}

void Kill_Geo3d_Ellipse(Geo3d_Ellipse *ellipse)
{
  Delete_Geo3d_Ellipse(ellipse);
}

void Default_Geo3d_Ellipse(Geo3d_Ellipse *ellipse)
{
  ellipse->radius = 1.0;
  ellipse->scale = 1.0;
  ellipse->alpha = 0.0;
  ellipse->center[0] = 0.0;
  ellipse->center[1] = 0.0;
  ellipse->center[2] = 0.0;
  ellipse->orientation[0] = 0.0;
  ellipse->orientation[1] = 0.0;
}

void Geo3d_Ellipse_Copy(Geo3d_Ellipse *dst, const Geo3d_Ellipse *src)
{
  *dst = *src;
}

Geo3d_Ellipse* Copy_Geo3d_Ellipse(Geo3d_Ellipse *src)
{
  Geo3d_Ellipse *ellipse = New_Geo3d_Ellipse();
  Geo3d_Ellipse_Copy(ellipse, src);
  return ellipse;
}

void Print_Geo3d_Ellipse(const Geo3d_Ellipse *ellipse)
{
  printf("Ellipse: rx: %g, ry: %g, alpha: %g, center: (%g, %g, %g), "
	 "orientation: (%g, %g)\n", ellipse->radius * ellipse->scale,
	 ellipse->radius, ellipse->alpha, 
	 ellipse->center[0], ellipse->center[1], ellipse->center[2],
	 ellipse->orientation[0], ellipse->orientation[1]);
}

void Geo3d_Ellipse_Center(const Geo3d_Ellipse *ellipse, double *center)
{
  center[0] = ellipse->center[0];
  center[1] = ellipse->center[1];
  center[2] = ellipse->center[2];
}

double Geo3d_Ellipse_Point_Distance(const Geo3d_Ellipse *ellipse,
				    const double *pt)
{
  coordinate_3d_t tmp_pt;
  tmp_pt[0] = pt[0] - ellipse->center[0];
  tmp_pt[1] = pt[1] - ellipse->center[1];
  tmp_pt[2] = pt[2] - ellipse->center[2];
  
  if (ellipse->alpha != 0.0) {
    Rotate_Z((double*) tmp_pt, tmp_pt, 1, ellipse->alpha, 1);
  }

  if ((ellipse->orientation[0] != 0.0) || (ellipse->orientation[1] != 0.0)) {
    Rotate_XZ(tmp_pt, tmp_pt, 1, 
	      ellipse->orientation[0], ellipse->orientation[1], 1);
  }

  double d = 0.0;
  if (Point_In_Ellipse(tmp_pt[0], tmp_pt[1],
		       ellipse->radius * ellipse->scale, ellipse->radius)
      == FALSE) {
    d = Ellipse_Point_Distance(tmp_pt[0], tmp_pt[1],
			       ellipse->radius * ellipse->scale, 
			       ellipse->radius, NULL, NULL);
  }

  return sqrt(d * d + tmp_pt[2] * tmp_pt[2]);
}

Geo3d_Ellipse* Geo3d_Ellipse_Interpolate(const Geo3d_Ellipse *start,
					 const Geo3d_Ellipse *end,
					 double lambda,
					 Geo3d_Ellipse *p)
{
  if (p == NULL) {
    p = New_Geo3d_Ellipse();
  }

  double alpha = 1.0 - lambda;
  p->center[0] = alpha * start->center[0] + lambda * end->center[0];
  p->center[1] = alpha * start->center[1] + lambda * end->center[1];
  p->center[2] = alpha * start->center[2] + lambda * end->center[2];

  if ((start->orientation[0] == end->orientation[0]) && 
      (start->orientation[1] == end->orientation[1])) {
    p->orientation[0] = start->orientation[0];
    p->orientation[1] = start->orientation[1];
  } else {
    double start_normal[3], end_normal[3], normal[3];
    Geo3d_Orientation_Normal(start->orientation[0], start->orientation[1],
			     start_normal, start_normal + 1, start_normal + 2);
    Geo3d_Orientation_Normal(end->orientation[0], end->orientation[1],
			     end_normal, end_normal + 1, end_normal + 2);
    normal[0] = alpha * start_normal[0] + lambda * end_normal[0];
    normal[1] = alpha * start_normal[1] + lambda * end_normal[1];
    normal[2] = alpha * start_normal[2] + lambda * end_normal[2];
    Geo3d_Normal_Orientation(normal[0], normal[1], normal[2], 
			     p->orientation, p->orientation + 1);
  }

  
  if ((start->alpha != 0.0) || (end->alpha != 0.0)) {
    double start_alpha = Normalize_Radian(start->alpha);
    double end_alpha = Normalize_Radian(end->alpha);
    if (fabs(end_alpha - start_alpha) > TZ_PI) {
      p->alpha = alpha * start_alpha + lambda * (end_alpha - TZ_2PI);
    } else {
      p->alpha = alpha * start_alpha + lambda * end_alpha;
    }
  } else {
    p->alpha = 0.0;
  }

  p->radius = alpha * start->radius + lambda * end->radius;
  p->scale = alpha * start->scale + lambda * end->scale;

  return p;
}

Swc_Node* Geo3d_Ellipse_To_Swc_Node(const Geo3d_Ellipse *ellipse,
				    int id, int parent_id, double z_scale,
				    int type, Swc_Node *node)
{
  if (node == NULL) {
    node = New_Swc_Node();
  }

  node->id = id;
  node->type = type;
  node->d = sqrt(ellipse->scale) * ellipse->radius;
  node->x = ellipse->center[0];
  node->y = ellipse->center[1];
  node->z = ellipse->center[2];
  if (z_scale != 1.0) {
    node->z /= z_scale;
  }
  node->parent_id = parent_id;

  return node;  
}

coordinate_3d_t* Geo3d_Ellipse_Sampling(const Geo3d_Ellipse *ellipse,
					int npt, double start, 
					coordinate_3d_t *pts)
{
  if (pts == NULL) {
    GUARDED_MALLOC_ARRAY(pts, npt, coordinate_3d_t);
  }

  double step = TZ_2PI / npt;
  int i;
  double t = start;
  double ry = ellipse->radius;
  double rx = ry * ellipse->scale;

  for (i = 0; i < npt; i++) {
    if (ellipse->alpha != 0.0) {
      pts[i][0] = rx * cos(t) * cos(ellipse->alpha) - 
	ry * sin(t) * sin(ellipse->alpha);
      pts[i][1] = rx * cos(t) * sin(ellipse->alpha) + 
	ry * sin(t) * cos(ellipse->alpha);
    } else {
      pts[i][0] = rx * cos(t);
      pts[i][1] = ry * sin(t);      
    }
    pts[i][2] = 0;
    t += step;
  }

  Geo3d_Point_Array_Rotate(pts, npt, ellipse->orientation[0], 
			   ellipse->orientation[1], 0);
  Geo3d_Point_Array_Translate(pts, npt, ellipse->center[0], 
			      ellipse->center[1], ellipse->center[2]);

  return pts;
}

void Geo3d_Ellipse_First_Vector(const Geo3d_Ellipse *ellipse, double *coord)
{
  double theta = ellipse->orientation[0];
  double psi = ellipse->orientation[1];
  coord[0] = cos(psi) * cos(ellipse->alpha) - 
    cos(theta) * sin(psi) * sin(ellipse->alpha);
  coord[1] = sin(psi) * cos(ellipse->alpha) + 
    cos(theta) * cos(psi) * sin(ellipse->alpha);
  coord[2] = sin(theta) * sin(ellipse->alpha);  
}

void Geo3d_Ellipse_Second_Vector(const Geo3d_Ellipse *ellipse, double *coord)
{
  double theta = ellipse->orientation[0];
  double psi = ellipse->orientation[1];
  coord[0] = -cos(psi) * sin(ellipse->alpha) - 
    cos(theta) * sin(psi) * cos(ellipse->alpha);
  coord[1] = -sin(psi) * sin(ellipse->alpha) + 
    cos(theta) * cos(psi) * cos(ellipse->alpha);
  coord[2] = sin(theta) * cos(ellipse->alpha);
}

void Geo3d_Ellipse_Normal_Vector(const Geo3d_Ellipse *ellipse, double *coord)
{
  double theta = ellipse->orientation[0];
  double psi = ellipse->orientation[1];
  coord[0] = sin(theta) * sin(psi);
  coord[1] = -sin(theta) * cos(psi);
  coord[2] = cos(theta);
}

coordinate_3d_t* 
Geo3d_Ellipse_Array_Sampling(const Geo3d_Ellipse *ellipse, 
			     int nobj, int nsample, coordinate_3d_t *pts)
{
  if (pts == NULL) {
    pts = (coordinate_3d_t *) 
      Guarded_Malloc(sizeof(coordinate_3d_t) * nobj * nsample,
		     "Local_Neuroseg_Ellipse_Array_Points");
  }
  
  coordinate_3d_t *pts_head = pts;

  const Geo3d_Ellipse *bottom = &(ellipse[0]);
  const Geo3d_Ellipse *top = &(ellipse[1]);

  coordinate_3d_t plane_normal;
  Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
  //  Coordinate_3d_Unitize(plane_normal);

  xz_orientation_t plane_ort;
  /* 0.001 is picked to test if two vectors are parallel */
  if (Coordinate_3d_Norm(plane_normal) < 0.001) {
    /* pick a plane that is not parallel to the circles */
    plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
    plane_ort[1] = bottom->orientation[1];
    Geo3d_Orientation_Normal(plane_ort[0], plane_ort[1], plane_normal,
			     plane_normal + 1, plane_normal + 2);
  } else {
    Coordinate_3d_Unitize(plane_normal);
  }

  double start1 = 
    Geo3d_Ellipse_Vector_Angle(bottom, plane_normal);
  Geo3d_Ellipse_Sampling(bottom, nsample, start1, pts);

  double start2 = 
    Geo3d_Ellipse_Vector_Angle(top, plane_normal);
  pts += nsample;
  Geo3d_Ellipse_Sampling(top, nsample, start2, pts);

  
  int i;
  double prev_start = start2;
  for (i = 1; i < nobj - 1; i++) {
    bottom = ellipse + i;
    top = ellipse + i + 1;
    Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
    
    if (Coordinate_3d_Norm(plane_normal) < 0.001) {
      /* pick a plane that is not parallel to the circles */
      plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
      plane_ort[1] = bottom->orientation[1];
      Geo3d_Orientation_Normal(plane_ort[0], plane_ort[1], plane_normal,
			       plane_normal + 1, plane_normal + 2);
    } else {
      Coordinate_3d_Unitize(plane_normal);
    }
    
    double start1 = Geo3d_Ellipse_Vector_Angle(bottom, plane_normal);
    double start2 = Geo3d_Ellipse_Vector_Angle(top, plane_normal);
    
    start2 += prev_start - start1;

    pts += nsample;
    Geo3d_Ellipse_Sampling(top, nsample, start2, pts);

    prev_start = start2;
  }

  return pts_head;  
}

double Geo3d_Ellipse_Vector_Angle(const Geo3d_Ellipse *ellipse,
				  const double *coord)
{
  if (Coordinate_3d_Length_Square(coord) == 0.0) {
    return 0.0;
  }

  coordinate_3d_t ortvec;
  coordinate_3d_t secortvec;

  Geo3d_Ellipse_First_Vector(ellipse, ortvec);
  Geo3d_Ellipse_Second_Vector(ellipse, secortvec);

  double ry = ellipse->radius;
  double rx = ry * ellipse->scale;
  coordinate_3d_t normcoord;
  normcoord[0] = Coordinate_3d_Dot(ortvec, coord) / rx;
  normcoord[1] = Coordinate_3d_Dot(secortvec, coord) / ry;
  
  double a = normcoord[0] / 
    sqrt(normcoord[0] * normcoord[0] + normcoord[1] * normcoord[1]);

  if (fabs(a) > 1.0) {
    a = round(a);
  }

  a = acos(a);

  if (a != 0.0) {
    coordinate_3d_t cross;
    coordinate_3d_t normvec;
    Geo3d_Ellipse_Normal_Vector(ellipse, normvec);
    Coordinate_3d_Cross(ortvec, coord, cross);
    if (Coordinate_3d_Dot(cross, normvec) < 0.0) {
      a = TZ_2PI - a;
    }
  }
  
  return a;  
}
