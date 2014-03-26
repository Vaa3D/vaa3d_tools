/**@file tz_neuroseg_plane.h
 * @brief neuroseg plane
 * @author Ting Zhao
 * @date 17-Apr-2008
 */

#ifndef _TZ_NEUROSEG_PLANE_H_
#define _TZ_NEUROSEG_PLANE_H_

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

/**@def NEUROSEG_PLANE_NPARAM tz_neuroseg_plane.h
 *
 * Number of parameters for neuroseg plane
 */
#define NEUROSEG_PLANE_NPARAM 5

/**@struct _Neuroseg_Plane tz_neuroseg_plane.h
 *
 * Neuroseg plane
 */
typedef struct _Neuroseg_Plane {
  double r;         /**< radius */
  double theta;     /**< theta (around x)  */
  double psi;       /**< psi (around z) */
  double offset_x;  /**< x offset */
  double offset_y;  /**< y offset */
} Neuroseg_Plane;

typedef Neuroseg_Plane* Neuroseg_Plane_P;

/* New and delete functions */
Neuroseg_Plane* New_Neuroseg_Plane();
void Delete_Neuroseg_Plane(Neuroseg_Plane *np);

void Kill_Neuroseg_Plane(Neuroseg_Plane *np);

/*
 * Set_Neuroseg_Plane() intialized <np> with radius <r>, orientation
 * (<theta>, <psi>) and offset (<offset_x>, <offset_y>). 
 *
 * Reset_Neursog_Plane() reset <np> to default values.
 */
void Set_Neuroseg_Plane(Neuroseg_Plane *np, double r, 
			double theta, double psi,
			double offset_x, double offset_y);
void Reset_Neuroseg_Plane(Neuroseg_Plane *np);

/*
 * Fprint_Neuroseg_Plane() prints <np> into the file <fp>.
 *
 * Print_Neuroseg_Plane() prints <np> to the standard output.
 */
void Fprint_Neuroseg_Plane(FILE *fp, const Neuroseg_Plane *np);
void Print_Neuroseg_Plane(const Neuroseg_Plane *np);

void Neuroseg_Plane_Fwrite(const Neuroseg_Plane *seg, FILE *fp);
Neuroseg_Plane* Neuroseg_Plane_Fread(Neuroseg_Plane *seg, FILE *fp);

/* variable operation routines */
#define NEUROSEG_PLANE_VAR_MASK_R             0x00000001
#define NEUROSEG_PLANE_VAR_MASK_THETA         0x00000002
#define NEUROSEG_PLANE_VAR_MASK_PSI           0x00000004
#define NEUROSEG_PLANE_VAR_MASK_ORIENTATION   0x00000006
#define NEUROSEG_PLANE_VAR_MASK_OFFSETX       0x00000008
#define NEUROSEG_PLANE_VAR_MASK_OFFSETY       0x00000010
#define NEUROSEG_PLANE_VAR_MASK_OFFSET        0x00000018

int Neuroseg_Plane_Var(const Neuroseg_Plane *np, double *var[]);
void Neuroseg_Plane_Set_Var(Neuroseg_Plane *np, int var_index, double value);
int Neuroseg_Plane_Var_Mask_To_Index(Bitmask_t mask, int *var_index);
/**********************************************************************/

/*
 * Neuroseg_Plane_Field() generates a field of <np>. <step> is the sampling
 * step. If <field> is not NULL, it is returned with the result. Otherwise
 * a new object is returned.
 */
Geo3d_Scalar_Field* Neuroseg_Plane_Field(const Neuroseg_Plane *np, 
					 double step,
					 Geo3d_Scalar_Field *field);

Geo3d_Scalar_Field* Neuroseg_Plane_Field_S(const Neuroseg_Plane *np, 
					   double step,
					   Geo3d_Scalar_Field *field);

__END_DECLS

#endif
