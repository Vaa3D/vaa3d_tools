/**@file tz_receptor_transform.h
 * @author Ting Zhao
 * @date 06-Nov-2010
 */

#ifndef _TZ_RECEPTOR_TRANSFORM_H_
#define _TZ_RECEPTOR_TRANSFORM_H_

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

/***** r2_rect *****/

typedef struct _Receptor_Transform_Rect_2d {
  double r;
  double theta;
  double h;
  double x;
  double y;
} Receptor_Transform_Rect_2d;

#define RECEPTOR_TRANSFORM_RECT_2D_NPARAM 5

#define RECEPTOR_TRANSFORM_VAR_MASK_R         0x00000001
#define RECEPTOR_TRANSFORM_VAR_MASK_THETA     0x00000002
#define RECEPTOR_TRANSFORM_VAR_MASK_H         0x00000004
#define RECEPTOR_TRANSFORM_VAR_MASK_X         0x00000008
#define RECEPTOR_TRANSFORM_VAR_MASK_Y         0x00000010
#define RECEPTOR_TRANSFORM_VAR_MASK_OFFSET    0x00000018

#define RECEPTOR_TRANSFORM_RECT_2D_VAR_NAME \
  const static char *Receptor_Transform_Rect_2d_Name[] = {		\
    "r", "theta", "h", "x", "y"				\
  };

int Receptor_Transform_Rect_2d_Var_Mask_To_Index(Bitmask_t mask, int *index);

void Receptor_Transform_Rect_2d_Apply(const Receptor_Transform_Rect_2d *tr,
    Geo3d_Scalar_Field *field);

/***************/


/***** r2_ellipse *****/

typedef struct _Receptor_Transform_Ellipse_2d {
  double loffset;
  double roffset;
  double toffset;
  double boffset;
  //double salpha; /* stretch direction */ 
  double alpha;
} Receptor_Transform_Ellipse_2d;

#define RECEPTOR_TRANSFORM_ELLIPSE_2D_NPARAM 5

#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_LOFFSET     0x00000001
#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_ROFFSET     0x00000002
#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_TOFFSET     0x00000004
#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_BOFFSET     0x00000008
#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_ALPHA       0x00000010
#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MASK_OFFSET      0x0000000F

#define RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_NAME \
  const static char *Receptor_Transform_Ellipse_2d_Name[] = {		\
    "left offset", "right offset", "top offset", "bottom offset", "alpha"				\
  };

int Receptor_Transform_Ellipse_2d_Var_Mask_To_Index(Bitmask_t mask, int *index);

void Receptor_Transform_Ellipse_2d_Apply(
    const Receptor_Transform_Ellipse_2d *tr,
    Geo3d_Scalar_Field *field);

void Receptor_Transform_Ellipse_2d_Apply_C(
    const Receptor_Transform_Ellipse_2d *tr,
    Geo3d_Scalar_Field *field);

/***************/

__END_DECLS

#endif
