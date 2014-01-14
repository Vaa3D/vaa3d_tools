/**@file tz_geo3d_ellipse.h
 * @brief 3D ellipse
 * @author Ting Zhao
 * @date 14-Sep-2009
 */

#ifndef _TZ_GEO3D_ELLIPSE_H_
#define _TZ_GEO3D_ELLIPSE_H_

#include "tz_cdefs.h"
#include "tz_xz_orientation.h"
#include "tz_coordinate_3d.h"
#include "tz_image_lib_defs.h"
#include "tz_neuron_component.h"
#include "tz_swc_cell.h"

__BEGIN_DECLS

/**@struct _Geo3d_Ellipse tz_geo3d_ellipse.h
 *
 * The structure of a 3D ellipse. The defintion is consistent with the ellipse
 * representation in the M document.
 */
typedef struct _Geo3d_Ellipse {
  double radius; /**< radius (ry) */
  double scale; /**< scale (rx / ry) */  
  double alpha; /**< alpha */
  coordinate_3d_t center; /**< center coordinates */
  xz_orientation_t orientation; /**< orientation */
} Geo3d_Ellipse;

typedef Geo3d_Ellipse* Geo3d_Ellipse_P;

#define NEUROCOMP_GEO3D_ELLIPSE(nc) (((nc)->type == NEUROCOMP_TYPE_GEO3D_ELLIPSE) ? ((Geo3d_Ellipse*) ((nc)->data)) : NULL)

Geo3d_Ellipse* New_Geo3d_Ellipse();
void Delete_Geo3d_Ellipse(Geo3d_Ellipse *ellipse);
void Kill_Geo3d_Ellipse(Geo3d_Ellipse *ellipse);

void Default_Geo3d_Ellipse(Geo3d_Ellipse *ellipse);

void Geo3d_Ellipse_Copy(Geo3d_Ellipse *dst, const Geo3d_Ellipse *src);
Geo3d_Ellipse* Copy_Geo3d_Ellipse(Geo3d_Ellipse *src);

void Print_Geo3d_Ellipse(const Geo3d_Ellipse *ellipse);

double Geo3d_Ellipse_Point_Distance(const Geo3d_Ellipse *ellipse,
				    const double *pt);

Geo3d_Ellipse* Geo3d_Ellipse_Interpolate(const Geo3d_Ellipse *start,
					 const Geo3d_Ellipse *end,
					 double lambda,
					 Geo3d_Ellipse *p);

Swc_Node* Geo3d_Ellipse_To_Swc_Node(const Geo3d_Ellipse *ellipse,
				   int id, int parent_id, double z_scale,
				   int type, Swc_Node *node);

void Geo3d_Ellipse_Center(const Geo3d_Ellipse *ellipse, double *center);

/**@brief Sample points on an ellipse.
 *
 * Geo3d_Ellipse_Sampling() returns a point array on <ellipse>. The number of
 * sampling points is <npt> and the first point is at <start>. If <pts> is not
 * NULL, it is the same as the returned pointer.
 */
coordinate_3d_t* Geo3d_Ellipse_Sampling(const Geo3d_Ellipse *ellipse,
					int npt, double start, 
					coordinate_3d_t *pts);

/**@brief First vector (rx) of an ellipse.
 *
 * Geo3d_Ellipse_First_Vector() stores the first vector of <ellipse> in <coord>.
 * The first vector is \vec{u} in the M document.
 */
void Geo3d_Ellipse_First_Vector(const Geo3d_Ellipse *ellipse, double *vec);

/**@brief Second vector (ry) of an ellipse.
 *
 * Geo3d_Ellipse_Second_Vector() stores the second vector of <ellipse> in 
 * <coord>. The second vector is a vector on the ellipse plane and orthogonal to
 * the first vector.
 */
void Geo3d_Ellipse_Second_Vector(const Geo3d_Ellipse *ellipse, double *vec);

/**@brief Normal vector (rz) of an ellipse.
 *
 * Geo3d_Ellipse_Normal_Vector() stores the normal vector of <ellipse> in 
 * <coord>.
 */
void Geo3d_Ellipse_Normal_Vector(const Geo3d_Ellipse *ellipse, double *vec);

/**@brief Sampling an ellipse array.
 *
 * Geo3d_Ellipse_Array_Sampling() returns sampling points from the ellipse array
 * <ellipse>, which has <nobj> elements. Each ellipse has <nsample> points.
 * If <pts> is not NULL, it is the same as the returned pointer.
 */
coordinate_3d_t* 
Geo3d_Ellipse_Array_Sampling(const Geo3d_Ellipse *ellipse, 
			     int nobj, int nsample, coordinate_3d_t *pts);

/**@brief Parameter of a vector in an ellipse.
 *
 * Geo3d_Ellipse_Vector_Angle() returns t (see the M document) for <vec> in 
 * <ellipse>.
 */
double Geo3d_Ellipse_Vector_Angle(const Geo3d_Ellipse *ellipse,
				  const double *vec);

__END_DECLS

#endif
