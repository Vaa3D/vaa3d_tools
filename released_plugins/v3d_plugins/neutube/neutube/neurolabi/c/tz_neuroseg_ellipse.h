/**@file tz_neuroseg_ellipse.h
 * @brief neuroseg ellipse
 * @author Ting Zhao
 * @date 17-Apr-2008
 */

#ifndef _TZ_NEUROSEG_ELLIPSE_H_
#define _TZ_NEUROSEG_ELLIPSE_H_

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

#define NEUROSEG_ELLIPSE_NPARAM 7

/**@struct _Neuroseg_Ellipse tz_neuroseg_ellipse.h
 *
 * An ellipse data structure.
 */
typedef struct _Neuroseg_Ellipse {
  double rx;        /**< axis x length */
  double ry;        /**< axis y length */
  double theta;     /**< theta (around x)  */
  double psi;       /**< psi (around z) */
  double alpha;     /**< around z before any other rotation */
  double offset_x;  /**< x offset */
  double offset_y;  /**< y offset */
} Neuroseg_Ellipse;

typedef Neuroseg_Ellipse* Neuroseg_Ellipse_P;

/* New and delete functions */
Neuroseg_Ellipse* New_Neuroseg_Ellipse();
void Delete_Neuroseg_Ellipse(Neuroseg_Ellipse *np);

void Kill_Neuroseg_Ellipse(Neuroseg_Ellipse *np);

/*
 * Set_Neuroseg_Ellipse() intialized <np> with radius <r>, orientation
 * (<theta>, <psi>) and offset (<offset_x>, <offset_y>). 
 *
 * Reset_Neursog_Plane() reset <np> to default values.
 */
void Set_Neuroseg_Ellipse(Neuroseg_Ellipse *np, double rx, double ry,
			  double theta, double psi, double alpha,
			  double offset_x, double offset_y);
void Reset_Neuroseg_Ellipse(Neuroseg_Ellipse *np);

/*
 * Fprint_Neuroseg_Ellipse() prints <np> into the file <fp>.
 *
 * Print_Neuroseg_Ellipse() prints <np> to the standard output.
 */
void Fprint_Neuroseg_Ellipse(FILE *fp, const Neuroseg_Ellipse *np);
void Print_Neuroseg_Ellipse(const Neuroseg_Ellipse *np);

void Neuroseg_Ellipse_Fwrite(const Neuroseg_Ellipse *seg, FILE *fp);
Neuroseg_Ellipse* Neuroseg_Ellipse_Fread(Neuroseg_Ellipse *seg, FILE *fp);

/* variable operation routines */
#define NEUROSEG_ELLIPSE_VAR_MASK_RX            0x00000001
#define NEUROSEG_ELLIPSE_VAR_MASK_RY            0x00000002
#define NEUROSEG_ELLIPSE_VAR_MASK_R             0x00000003
#define NEUROSEG_ELLIPSE_VAR_MASK_THETA         0x00000004
#define NEUROSEG_ELLIPSE_VAR_MASK_PSI           0x00000008
#define NEUROSEG_ELLIPSE_VAR_MASK_ORIENTATION   0x0000000C
#define NEUROSEG_ELLIPSE_VAR_MASK_OFFSETX       0x00000010
#define NEUROSEG_ELLIPSE_VAR_MASK_OFFSETY       0x00000020
#define NEUROSEG_ELLIPSE_VAR_MASK_OFFSET        0x00000030
#define NEUROSEG_ELLIPSE_VAR_MASK_ALPHA         0x00000040

int Neuroseg_Ellipse_Var(const Neuroseg_Ellipse *np, double *var[]);
void Neuroseg_Ellipse_Set_Var(Neuroseg_Ellipse *np, int var_index, double value);
int Neuroseg_Ellipse_Var_Mask_To_Index(Bitmask_t mask, int *var_index);
/**********************************************************************/

/*
 * Neuroseg_Ellipse_Field() generates a field of <np>. <step> is the sampling
 * step. If <field> is not NULL, it is returned with the result. Otherwise
 * a new object is returned.
 */
Geo3d_Scalar_Field* Neuroseg_Ellipse_Field(const Neuroseg_Ellipse *np, 
					 double step,
					 Geo3d_Scalar_Field *field);

void Neuroseg_Ellipse_Points(const Neuroseg_Ellipse *np, int npt, 
			     double start, coordinate_3d_t coord[]);
/* Vector of the major axis.
 */
void Neuroseg_Ellipse_Ortvec(const Neuroseg_Ellipse *ne,
			       coordinate_3d_t coord);
void Neuroseg_Ellipse_Secortvec(const Neuroseg_Ellipse *ne,
			       coordinate_3d_t coord);
void Neuroseg_Ellipse_Normvec(const Neuroseg_Ellipse *ne,
			      coordinate_3d_t coord);

double Neuroseg_Ellipse_Vector_Angle(const Neuroseg_Ellipse *ne,
				     const coordinate_3d_t coord);

__END_DECLS

#endif
