#include <math.h>
#include "tz_receptor_transform.h"
#include "tz_3dgeom.h"

int Receptor_Transform_Rect_2d_Var_Mask_To_Index(Bitmask_t mask, int *index)
{
  return Bitmask_To_Index(mask, RECEPTOR_TRANSFORM_RECT_2D_NPARAM, index);
}

void Receptor_Transform_Rect_2d_Apply(const Receptor_Transform_Rect_2d *tr,
    Geo3d_Scalar_Field *field)
{
  int i;
  double weight = 0;
  for (i = 0; i < field->size; i++) {
    if (field->values[i] < 0) {
      if (tr->r < 1.0) {
        /* map [r, 2r] to [r, r + 1.0] */
        field->points[i][0] = tr->r + (field->points[i][0]  - tr->r) / tr->r;
      } else if (tr->r > 2.0) {
        /* map [r, 2r] to [r, r + 2.0] */
        field->points[i][0] = 
          tr->r + 2.0 * (field->points[i][0]  - tr->r) / tr->r;
      }
    } else {
      field->values[i] *= sqrt(sqrt(tr->r));
    }
    weight += fabs(field->values[i]);
  }

  
  for (i = 0; i < field->size; i++) {
    field->values[i] /= weight;
    field->points[i][1] *= tr->h;
  }

  Scale_X_Rotate_Z(Coordinate_3d_Double_Array(field->points), 
      Coordinate_3d_Double_Array(field->points), field->size, tr->r, tr->theta,
      0);

  for (i = 0; i < field->size; i++) {
    field->points[i][0] += tr->x;
    field->points[i][1] += tr->y;
  }
}

int Receptor_Transform_Ellipse_2d_Var_Mask_To_Index(Bitmask_t mask, int *index)
{
  return Bitmask_To_Index(mask, RECEPTOR_TRANSFORM_ELLIPSE_2D_NPARAM, index);
}

static void scale_ellipse_field(coordinate_3d_t pt, double *value,
    double rx, double ry, double sqrt_r_scale)
{
  if (value[0] >= 0.0) {
    pt[0] *= rx;
    pt[1] *= ry;
    value[0] *= sqrt_r_scale;
  } else {    
    /* length in the base receptor */
    double norm = sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
    double alpha = norm - 1;

    /* positive boundary */
    pt[0] *= rx / norm;
    pt[1] *= ry / norm;

    double d = 2.0;

    /* length to positive boundary */
    double enorm = sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
    if (enorm < 1.0) {
      alpha /= enorm;
    } else {
      if (enorm > d) {
        alpha *= d / enorm;
      }
    }
    pt[0] *= (1.0 + alpha);
    pt[1] *= (1.0 + alpha);
  }
}

void Receptor_Transform_Ellipse_2d_Apply(
    const Receptor_Transform_Ellipse_2d *tr,
    Geo3d_Scalar_Field *field)
{
  double rx = (tr->loffset + tr->roffset) / 2.0 + 1.0;
  double ry = (tr->toffset + tr->boffset) / 2.0 + 1.0;
  double dx = (tr->roffset - tr->loffset) / 2.0;
  double dy = (tr->toffset - tr->boffset) / 2.0;
  
  int i;
  double weight = 0;
  double sqrt_r_scale = sqrt(sqrt(rx * ry));
  //double sqrt_r_scale = (rx * ry) ;
  //double sqrt_r_scale = 1.0;
  for (i = 0; i < field->size; i++) {
    scale_ellipse_field(field->points[i], field->values + i, rx, ry, 
      1.0);
    weight += fabs(field->values[i]);
  }
  
  Rotate_Z(Coordinate_3d_Double_Array(field->points), 
      Coordinate_3d_Double_Array(field->points), field->size, tr->alpha,
      0);
  
  for (i = 0; i < field->size; i++) {
    field->points[i][0] += dx;
    field->points[i][1] += dy;
    field->values[i] /= weight;
    field->values[i] *= sqrt_r_scale;
  }

}

void Receptor_Transform_Ellipse_2d_Apply_C(
    const Receptor_Transform_Ellipse_2d *tr,
    Geo3d_Scalar_Field *field)
{
  double rx = (tr->loffset + tr->roffset) / 2.0 + 1.0;
  double ry = (tr->toffset + tr->boffset) / 2.0 + 1.0;
  double dx = (tr->roffset - tr->loffset) / 2.0;
  double dy = (tr->toffset - tr->boffset) / 2.0;
  
  int i;
  double weight = 0;
  double sqrt_r_scale = sqrt(sqrt(rx * ry));
  for (i = 0; i < field->size; i++) {
    scale_ellipse_field(field->points[i], field->values + i, rx, ry, 
      sqrt_r_scale);
    weight += sqrt(field->values[i] * field->values[i]);
  }
  
  Rotate_Z(Coordinate_3d_Double_Array(field->points), 
      Coordinate_3d_Double_Array(field->points), field->size, tr->alpha,
      0);
  
  for (i = 0; i < field->size; i++) {
    field->points[i][0] += dx;
    field->points[i][1] += dy;
    field->values[i] /= weight;
    field->values[i] *= sqrt(sqrt_r_scale);
  }
}
