/**@file tz_geo3d_circle.h
 * @brief 3d circle
 * @author Ting Zhao
 * @date 18-Mar-2008
 */

#ifndef _TZ_GEO3D_CIRCLE_H_
#define _TZ_GEO3D_CIRCLE_H_

#include "tz_cdefs.h"
#include "tz_xz_orientation.h"
#include "tz_coordinate_3d.h"
#include "tz_image_lib_defs.h"
#include "tz_swc_cell.h"

__BEGIN_DECLS

/**@struct _Geo3d_Circle tz_geo3d_circle.h
 *
 * The structure of a 3D circle.
 */
typedef struct _Geo3d_Circle {
  double radius; /**< radius */
  double center[3]; /**< center coordinates */
  xz_orientation_t orientation; /**< orientation */
} Geo3d_Circle;

typedef Geo3d_Circle* Geo3d_Circle_P;

#define NEUROCOMP_GEO3D_CIRCLE(nc) (((nc)->type == NEUROCOMP_TYPE_GEO3D_CIRCLE) ? ((Geo3d_Circle*) ((nc)->data)) : NULL)

Geo3d_Circle* New_Geo3d_Circle();
void Delete_Geo3d_Circle(Geo3d_Circle *circle);
void Kill_Geo3d_Circle(Geo3d_Circle *circle);

void Geo3d_Circle_Default(Geo3d_Circle *circle);

void Geo3d_Circle_Copy(Geo3d_Circle *dst, const Geo3d_Circle *src);
Geo3d_Circle* Copy_Geo3d_Circle(Geo3d_Circle *src);

void Print_Geo3d_Circle(const Geo3d_Circle *circle);

/**@brief Get points on a circle.
 *
 * Geo3d_Circle_Points() samples <npt> points on the circle <circle>. <start> is
 * the starting vector. The points are stored in <coord>.
 */
void Geo3d_Circle_Points(const Geo3d_Circle *circle, int npt, 
			 const coordinate_3d_t start,
			 coordinate_3d_t coord[]);

/**@brief Rotate a vector on a circle.
 *
 * Geo3d_Circle_Rotate_Vector() rotates the circle vector <coord> by <angle>
 * around the normal of <circle>. 
 */
void Geo3d_Circle_Rotate_Vector(const Geo3d_Circle *circle, double angle,
				coordinate_3d_t coord);

/**@brief The angle between two circle vectors.
 *
 * Geo3d_Circle_Vector_Angle() returns the angle between the two circle vectors
 * <pt1> and <pt2>.
 */
double Geo3d_Circle_Vector_Angle(const Geo3d_Circle *circle,
				 coordinate_3d_t pt1, coordinate_3d_t pt2);

/**@brief intersection between a circle and a plane
 *
 * Geo3d_Circle_Plane_Point() obtains an intersection of <circle> and
 * a plane with orientation <plane_ort>. The function assumes the center of 
 * the circle is located in the plane. Since there are two insections if the
 * circle is not on the plane, <direction> is used to pick up which one: 1 for
 * (circle, plane, .) and -1 for (plane, circle, .).
 * It returns 0 if the circle is on the plane, which means no intersection is 
 * obtained; otherwise it returns 1.
 */
int Geo3d_Circle_Plane_Point(const Geo3d_Circle *circle, 
			     const xz_orientation_t plane_ort,
			     int direction,
			     coordinate_3d_t coord);

/**@brief intersection between a unit circle and a plane
 *
 * Geo3d_Circle_Plane_Point() obtains an intersection of <circle> and
 * a plane with orientation <plane_ort>. The function assumes the center of 
 * the circle is located in the plane. Since there are two insections if the
 * circle is not on the plane, <direction> is used to pick up which one: 1 for
 * (circle, plane, .) and -1 for (plane, circle, .).
 * It returns 0 if the circle is on the plane, which means no intersection is 
 * obtained; otherwise it returns 1.
 */
int Geo3d_Unit_Circle_Plane_Point(const xz_orientation_t circle_ort,
				  const xz_orientation_t plane_ort,
				  int direction,
				  coordinate_3d_t coord);

coordinate_3d_t* 
Geo3d_Circle_Array_Points(const Geo3d_Circle *circle, int ncircle,
			 int npt_per_c, coordinate_3d_t *pts);

Geo3d_Circle* Make_Geo3d_Circle_Array(int n);

/**@brief Draw a circle in a stack.
 *
 * Under development.
 */
void Geo3d_Circle_Draw_Stack(const Geo3d_Circle *circle, Stack *stack, 
			     double z_scale);

void Geo3d_Circle_Swc_Fprint(FILE *fp, const Geo3d_Circle *circle, int id,
			     int parent_id);
void Geo3d_Circle_Swc_Fprint_Z(FILE *fp, const Geo3d_Circle *circle, int id,
			       int parent_id, double z_scale);
void Geo3d_Circle_Swc_Fprint_T(FILE *fp, const Geo3d_Circle *circle,
			       int id, int parent_id, int type, double z_scale);

Swc_Node* Geo3d_Circle_To_Swc_Node(const Geo3d_Circle *circle,
				   int id, int parent_id, double z_scale,
				   int type, Swc_Node *node);

__END_DECLS

#endif
