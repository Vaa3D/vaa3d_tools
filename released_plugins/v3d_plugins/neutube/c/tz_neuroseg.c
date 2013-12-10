/* tz_neuroseg.c
 *
 * 22-Nov-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include "tz_neuroseg.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_math.h>
#  include <gsl/gsl_cdf.h>
#  include <gsl/gsl_sort_uchar.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include "tz_math.h"
#include "tz_utilities.h"
#include "utilities.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_constant.h"
#include "tz_3dgeom.h"
#include "tz_neurofield.h"
#include "tz_geo3d_vector.h"
#include "tz_geo3d_point_array.h"
#include "tz_geo3d_utils.h"

INIT_EXCEPTION

#include "private/tz_neuroseg.c"


Neuroseg* New_Neuroseg() 
{
  Neuroseg *seg = (Neuroseg *) Guarded_Malloc(sizeof(Neuroseg), "New_Neuroseg");

  Reset_Neuroseg(seg);

  return seg;
}

void Delete_Neuroseg(Neuroseg *seg)
{
  free(seg);
}


/* Set_Neuroseg(): initialize a neuron segment.
 * 
 * Args: seg - pointer to the segment.
 */	
void Set_Neuroseg(Neuroseg *seg, double r1, double c, 
		  double h, double theta, double psi, double curvature,
		  double alpha, double scale)
{
  if (seg == NULL) {
    THROW(ERROR_POINTER_NULL);
  }

  if (!is_valid_radius(r1) || !is_valid_height(h)) {
    THROW(ERROR_DATA_VALUE);
  }

  seg->r1 = r1;
  seg->c = c;
  seg->h = h;
  seg->theta = theta;
  seg->psi = psi;
  seg->curvature = curvature;
  seg->alpha = alpha;
  seg->scale = scale;
}

void Reset_Neuroseg(Neuroseg *seg)
{
  Set_Neuroseg(seg, NEUROSEG_DEFAULT_R1, NEUROSEG_DEFAULT_C,
	       NEUROSEG_DEFAULT_H, NEUROSEG_DEFAULT_THETA,
	       NEUROSEG_DEFAULT_PSI, NEUROSEG_DEFAULT_CURVATURE,
	       NEUROSEG_DEFAULT_ALPHA, NEUROSEG_DEFAULT_SCALE);
}

		
/* Neuroseg_Copy(): Copy neuron segment.
 *
 * Args: <seg1> - destination;
 *       <seg2> - source.
 *
 * Return: void.
 */
void Neuroseg_Copy(Neuroseg *seg1, const Neuroseg *seg2)
{
  *seg1 = *seg2;
}

void Fprint_Neuroseg(FILE *fp, const Neuroseg *seg)
{
  if (seg == NULL) {
    fprintf(fp, "Null segment\n");
  } else {
    fprintf(fp, "Neuron segment: \n");
    fprintf(fp, "r1: %g, r2: %g, h: %g; theta: %g, psi: %g, curvature: %g, "
	    "alpha: %g, scale: %g\n",
	    seg->r1, NEUROSEG_R2(seg), seg->h, seg->theta, seg->psi, 
	    seg->curvature, seg->alpha, seg->scale);
  }
}

/* Print_Neuroseg(): Print neuron segment.
 *
 * Args: seg - pointer of the neuron segment to print.
 *
 * Return: no return.
 */
void Print_Neuroseg(const Neuroseg *seg)
{
  Fprint_Neuroseg(stdout, seg);
}

void Neuroseg_Fwrite(const Neuroseg *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  fwrite(seg, sizeof(*seg), 1, fp);
}

Neuroseg* Neuroseg_Fread(Neuroseg *seg, FILE *fp)
{
  if ((seg == NULL) || (fp == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;

  if (seg == NULL) {
    seg = New_Neuroseg();
    is_local_alloc = TRUE;
  }
  
  if (fread(seg, sizeof(Neuroseg), 1, fp) != 1) {
    if (is_local_alloc == TRUE) {
      Delete_Neuroseg(seg);
    }
    seg = NULL;
  }

  return seg;
}

void Neuroseg_Set_Orientation(Neuroseg *seg, double theta, double psi)
{
  seg->theta = theta;
  seg->psi = psi;
}

void Neuroseg_Set_Orientation_V(Neuroseg *seg, double x, double y, double z)
{
  Geo3d_Vector v;
  Set_Geo3d_Vector(&v, x, y, z);
  Geo3d_Vector_Orientation(&v, &(seg->theta), &(seg->psi));
}

double Neuroseg_Ball_Range(const Neuroseg *seg)
{
  double r = NEUROSEG_RB(seg);
  
  if (seg->scale > 1.0) {
    r *= seg->scale;
  }

  return sqrt(seg->h * seg->h + r * r);
}

double Neuroseg_Z_Range(const Neuroseg *seg)
{
  double r = 0.0;
  if (seg->c < 0.0) {
    r = Neuroseg_Ry(seg, NEUROSEG_BOTTOM) * 2.0;
  } else {
    r = Neuroseg_Ry(seg, NEUROSEG_TOP) * 2.0;
  }

  return fabs(r * sin(seg->theta)) + fabs(seg->h * cos(seg->theta));
}

void Neuroseg_Field_Range(const Neuroseg *seg, Field_Range *range, 
			  double z_scale)
{
  /* alloc <field> */
  Geo3d_Scalar_Field *field = Neuroseg_Field_S(seg, NULL, NULL);
  if (field == NULL)
    return;
  coordinate_3d_t bound[2];
  Geo3d_Scalar_Field_Boundbox(field, bound);
  
  range->first_corner[0] = (int) (bound[0][0] - 1.5);
  range->first_corner[1] = (int) (bound[0][1] - 1.5);
  range->first_corner[2] = (int) (bound[0][2] * z_scale - 1.5);

  range->size[0] = (int) (bound[1][0] - range->first_corner[0] + 1.5);
  range->size[1] = (int) (bound[1][1] - range->first_corner[1] + 1.5);
  range->size[2] = (int) (bound[1][2]* z_scale - range->first_corner[2] 
			  + 1.5);

#ifdef _DEBUG_2
  print_field_range(range);
#endif

  /* free <field> */
  Kill_Geo3d_Scalar_Field(field);
}

/* This functin is only used for drawing 
 *
 * Neurofilter(): generate neuron segment filter.
 *
 * Note: A new pointer will be created if the input [filter] is NULL. The
 *       caller is reposible for clearing up the returned pointer. 
 *
 * Args: seg - neuron segment;
 *       filter - an array to store result;
 *       range - field range;
 *       offpos - position offset.
 *
 * Return: the filter array.
 */
double* Neurofilter(const Neuroseg *seg, double *filter, int *length,
		    const Field_Range *range, const double *offpos,
		    double z_scale)
{
  double coord[3];

  double weight = 0.0;
#if NEUROFIELD == 2
  double weight2 = 0.0;
#endif
  int coffset[3];
  int i, j, k;
  int offset = 0;
  
  if (filter == NULL) {
    int length2 = range->size[0] * range->size[1] * range->size[2];
    if (length != NULL) {
      *length = length2;
    }
    filter = (double *) malloc(sizeof(double) * length2);
  }

  coffset[0] = range->first_corner[0];
  coffset[1] = range->first_corner[1];
  coffset[2] = range->first_corner[2];
		
  /* double coef = (seg->r2 - seg->r1) / seg->h; */
  double coef = seg->c;

  for (k = 0; k < range->size[2]; k++) {
    for (j = 0; j < range->size[1]; j++) {
      for (i = 0; i < range->size[0]; i++) {
	coord[0] = (double) (i + coffset[0]);
	coord[1] = (double) (j + coffset[1]);
	coord[2] = (double) (k + coffset[2]);
	if (test_zscale(z_scale) != 0) {
	  /* from image space to physical space */
	  coord[2] /= z_scale;
	}
	if (offpos != NULL) {
	  coord[0] -= offpos[0];
	  coord[1] -= offpos[1];
	  coord[2] -= offpos[2];
	}
	
	Rotate_XZ(coord, coord, 1, seg->theta, seg->psi, 1);
	Scale_X_Rotate_Z(coord, coord, 1, seg->scale, seg->alpha, 1);

	filter[offset] = Neurofield7(coef, seg->r1, 
				    coord[0], coord[1], coord[2],
				    -0.5, seg->h - 0.5);

#if NEUROFIELD == 1
	weight += fabs(filter[offset]);
#endif

#if NEUROFIELD == 2
	if (filter[offset] > 0) {
	  weight += 1.0;
	} else if (filter[offset] < 0) {
	  weight2 += 1.0;
	}
#endif

#if NEUROFIELD == 3
	if (filter[offset] > 0) {
	  weight += filter[offset];
	}
#endif

#if NEUROFIELD == 4
	weight += fabs(filter[offset]);
#endif

#if NEUROFIELD == 5
	weight += fabs(filter[offset]);
#endif
	offset++;
      }
    }
  }

#if NEUROFIELD == 1
  for (i = 0; i < offset; i++)
    filter[i] /= weight;
#endif

#if NEUROFIELD == 2
  for (i = 0; i < offset; i++) {
    if (filter[i] > 0) {
      filter[i] /= weight;
    } else if (filter[i] < 0) {
      filter[i] /= weight2;
    }
  }
#endif

#if NEUROFIELD == 3
  for (i = 0; i < offset; i++) {
    if (filter[i] > 0) {
      filter[i] /= weight;
    } 
  }
#endif

#if NEUROFIELD == 5
  for (i = 0; i < offset; i++)
    filter[i] /= weight;
#endif

#if 0
  darray_print3(filter, range->size[0], range->size[1], range->size[2]);
#endif

  return filter;
}

double* Neuroseg_Dist_Filter(const Neuroseg *seg, double *filter, int *length,
			     const Field_Range *range, const double *offpos,
			     double z_scale)
{
  double coord[3];

  int coffset[3];
  int i, j, k;
  int offset = 0;
  
  if (filter == NULL) {
    int length2 = range->size[0] * range->size[1] * range->size[2];
    if (length != NULL) {
      *length = length2;
    }
    filter = (double *) malloc(sizeof(double) * length2);
  }

  coffset[0] = range->first_corner[0];
  coffset[1] = range->first_corner[1];
  coffset[2] = range->first_corner[2];
		
  /* double coef = (seg->r2 - seg->r1) / seg->h; */
  double coef = seg->c;

  for (k = 0; k < range->size[2]; k++) {
    for (j = 0; j < range->size[1]; j++) {
      for (i = 0; i < range->size[0]; i++) {
	coord[0] = (double) (i + coffset[0]);
	coord[1] = (double) (j + coffset[1]);
	coord[2] = (double) (k + coffset[2]);
	if (test_zscale(z_scale) != 0) {
	  /* from image space to physical space */
	  coord[2] /= z_scale;
	}
	if (offpos != NULL) {
	  coord[0] -= offpos[0];
	  coord[1] -= offpos[1];
	  coord[2] -= offpos[2];
	}
	
	Rotate_XZ(coord, coord, 1, seg->theta, seg->psi, 1);
	Scale_X_Rotate_Z(coord, coord, 1, seg->scale, seg->alpha, 1);

	filter[offset] = Neuroseg_Dist_Field(coef, seg->r1, 
					     coord[0], coord[1], coord[2],
					     -0.5, seg->h - 0.5);
	
	offset++;
      }
    }
  }

  return filter;
}

/* Neuroseg_Axis_Offset(): get the position offset along the axis of a neuron
 *                         segment.
 *
 * Args: seg - neuron segment; 
 *       axis_offset - linear offset along the axis (relative to the bottom);
 *       pos_offset - resulted spatial offset.
 *         
 * Return: void.
 */
void Neuroseg_Axis_Offset(const Neuroseg *seg, double axis_offset,
			  double pos_offset[3])
{
  pos_offset[0] = 0.0;
  pos_offset[1] = 0.0;
  pos_offset[2] = axis_offset;

  Rotate_XZ(pos_offset, pos_offset, 1, seg->theta, seg->psi, 0);
}

void Neuroseg_Center(const Neuroseg *seg, double *pos)
{
  Neuroseg_Axis_Offset(seg, (seg->h - 1.0) / 2.0, pos);
}

void Neuroseg_Bottom(const Neuroseg *seg, double *pos)
{
  pos[0] = 0.0;
  pos[1] = 0.0;
  pos[2] = 0.0;
}

void Neuroseg_Top(const Neuroseg *seg, double *pos)
{
  Neuroseg_Axis_Offset(seg, seg->h - 1.0, pos);
}

/* Neuroseg_Var_Mask_To_Index(): Turn mask into variable indices.
 */
int Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index)
{
  return Bitmask_To_Index(mask, NEUROSEG_NPARAM, var_index);
}

/* Neuroseg_Var() : build a variable list for a neuron segment;
 * 
 * Args: seg - neuron segment;
 *       var - variable list, which must be long enough to hold the variables.
 *
 * Return: number of variables.
 */
int Neuroseg_Var(const Neuroseg *seg, double *var[])
{  
  var[0] = (double *) &(seg->r1);
  var[1] = (double *) &(seg->c);
  var[2] = (double *) &(seg->theta);
  var[3] = (double *) &(seg->psi);
  var[4] = (double *) &(seg->h);
  var[5] = (double *) &(seg->curvature);
  var[6] = (double *) &(seg->alpha);
  var[7] = (double *) &(seg->scale);

  return NEUROSEG_NPARAM;
}

/*
 * Neuroseg_Change_Thickness(): change the thickness of a neuron segment.
 *
 * Args: seg - the neurosegment to change;
 *       ratio - ratio of change;
 *       diff - offset of change.
 *
 * Return: void.
 */
void Neuroseg_Change_Thickness(Neuroseg *seg, double ratio, double diff)
{
  seg->r1 *= ratio;
  seg->r1 += diff;
}

void Neuroseg_Swell(Neuroseg *seg, double ratio, double diff,
		    double max_diff)
{
  double rby = NEUROSEG_RB(seg);
  double rbx = rby * seg->scale;
  double nrbx = rbx * ratio + diff;
  double nrby = rby * ratio + diff;
  if (max_diff > 0.0) {
    nrbx = dmin2(nrbx, rbx + max_diff);
    nrby = dmin2(nrby, rby + max_diff);
  }

  seg->scale = nrbx / nrby;
  seg->r1 = nrby;
  if (seg->c > 0.0) {
    seg->r1 -= (seg->h - 1.0) * seg->c;
  }
}

/* Next_Neuroseg(): Get next neuron segment.
 *
 * Args: seg1 - Current neuron segment;
 *       seg2 - Next neuron segment, which could be NULL.
 *       pos_step - position step (relative to the bottom. can be negative).
 *
 * Return: The next neuron segment, which is seg2 if it is not NULL.
 */
Neuroseg* Next_Neuroseg(const Neuroseg *seg1, Neuroseg *seg2, double pos_step)
{
  if (seg2 == NULL) {
    seg2 = New_Neuroseg();
  }

  Neuroseg_Copy(seg2, seg1);  
  seg2->r1 = seg1->r1 + pos_step * NEUROSEG_COEF(seg1) * (seg1->h - 1.0);

  if (seg2->r1 < NEUROSEG_MIN_R) {
    seg2->r1 = NEUROSEG_MIN_R;
  }

  if (NEUROSEG_DEFAULT_H > 1.0) {
    seg2->h = NEUROSEG_DEFAULT_H;
  }

  seg2->c = NEUROSEG_COEF(seg2);

  //seg2->alpha = NEUROSEG_DEFAULT_ALPHA;
  //seg2->scale = NEUROSEG_DEFAULT_SCALE;

  return seg2;
}


double Neuroseg_Theta_Offset(const Neuroseg *seg1, const Neuroseg *seg2)
{
  double diff = fabs(normalize_angle(seg1->theta) - 
		     normalize_angle(seg2->theta));

  if (diff > TZ_PI) {
    diff = TZ_2PI - diff;
  }

  return diff;
}

BOOL Neuroseg_Hit_Test(const Neuroseg *seg, double x, double y, double z)
{
  if ((z >= -0.5) && (z <= seg->h - 0.5)) {
    double d2 = (x * x) / (seg->scale * seg->scale) + y * y;
    /* double r = seg->r1 + (seg->r2 - seg->r1) * z / seg->h; */
    double r = NEUROSEG_RADIUS(seg, z);
    if (d2 <= r * r) {
      return TRUE;
    }
  }

  return FALSE;
}

int Neuroseg_Hit_Test_A(const Neuroseg *seg, coordinate_3d_t *pts, int n)
{
  int i;
  double d2;
  double r;
  /* double coef = (seg->r2 - seg->r1) / seg->h; */
  for (i = 0; i < n; i++) {
    d2 = pts[i][0] * pts[i][0] + pts[i][1] * pts[i][1];
    /* r = seg->r1 + coef * pts[i][2]; */
    r = NEUROSEG_RADIUS(seg, pts[i][2]);
    if (d2 <= r * r) {
      return i;
    }
  }

  return -1;
}

double Neuroseg_Angle_Between(const Neuroseg *seg1, const Neuroseg *seg2)
{
  Geo3d_Vector v1, v2;
  Geo3d_Orientation_Normal(seg1->theta, seg1->psi, &(v1.x), &(v1.y), &(v1.z));
  Geo3d_Orientation_Normal(seg2->theta, seg2->psi, &(v2.x), &(v2.y), &(v2.z));
  return Geo3d_Vector_Angle2(&v1, &v2);
}

double Neuroseg_Angle_Between_Z(const Neuroseg *seg1, const Neuroseg *seg2,
				double z_scale)
{
  Geo3d_Vector v1, v2;

  Geo3d_Orientation_Normal(seg1->theta, seg1->psi, &(v1.x), &(v1.y), &(v1.z));
  Geo3d_Orientation_Normal(seg2->theta, seg2->psi, &(v2.x), &(v2.y), &(v2.z));
  v1.z /= z_scale;
  v2.z /= z_scale;

  return Geo3d_Vector_Angle2(&v1, &v2);
}

double Neuroseg_Rx(const Neuroseg *seg, Neuropos_Reference_e ref)
{
  switch (ref) {
  case NEUROSEG_BOTTOM:
    return NEUROSEG_R1(seg) * seg->scale;
  case NEUROSEG_TOP:
    return NEUROSEG_R2(seg) * seg->scale;
  case NEUROSEG_CENTER:
    return NEUROSEG_RC(seg) * seg->scale;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return 0.0;
}

double Neuroseg_Rx_P(const Neuroseg *seg, const double *res, 
		   Neuropos_Reference_e ref)
{
  return Neuroseg_Rx(seg, ref) * res[0];
}

double Neuroseg_Rx_Z(const Neuroseg *seg, double z)
{
  return NEUROSEG_RADIUS(seg, z) * seg->scale;
}

double Neuroseg_Rx_T(const Neuroseg *seg, double t)
{
  return NEUROSEG_RADIUS(seg, t * (seg->h - 1.0)) * seg->scale;
}

double Neuroseg_Ry(const Neuroseg *seg, Neuropos_Reference_e ref)
{
  switch (ref) {
  case NEUROSEG_BOTTOM:
    return NEUROSEG_R1(seg);
  case NEUROSEG_TOP:
    return NEUROSEG_R2(seg);
  case NEUROSEG_CENTER:
    return NEUROSEG_RC(seg);
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return 0.0;
}

double Neuroseg_Ry_P(const Neuroseg *seg, const double *res, 
		     Neuropos_Reference_e ref)
{ 
  return Neuroseg_Ry(seg, ref) * sqrt(dsqr(res[0]) * cos(seg->theta) + 
				      dsqr(res[1]) * sin(seg->theta));
}

double Neuroseg_Ry_Z(const Neuroseg *seg, double z)
{
  return NEUROSEG_RADIUS(seg, z);
}

double Neuroseg_Ry_T(const Neuroseg *seg, double t)
{
  return NEUROSEG_RADIUS(seg, t * (seg->h - 1.0));
}

double Neuroseg_Rxy_Z(const Neuroseg *seg, double z)
{
  return NEUROSEG_RADIUS(seg, z) * sqrt(seg->scale);
}

double Neuroseg_Rxy_T(const Neuroseg *seg, double t)
{
  return NEUROSEG_RADIUS(seg, t * (seg->h - 1.0)) * sqrt(seg->scale);
}

double Neuroseg_Rc_Z(const Neuroseg *seg, double z, int option)
{
  switch (option) {
  case NEUROSEG_CIRCLE_RX:
    return Neuroseg_Rx_Z(seg, z);
  case NEUROSEG_CIRCLE_RY:
    return Neuroseg_Ry_Z(seg, z);
  case NEUROSEG_CIRCLE_RXY:
    return Neuroseg_Rxy_Z(seg, z);
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    return 0.0;
  }
}

double Neuroseg_Rc_T(const Neuroseg *seg, double t, int option)
{
  switch (option) {
  case NEUROSEG_CIRCLE_RX:
    return Neuroseg_Rx_T(seg, t);
  case NEUROSEG_CIRCLE_RY:
    return Neuroseg_Ry_T(seg, t);
  case NEUROSEG_CIRCLE_RXY:
    return Neuroseg_Rxy_T(seg, t);
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    return 0.0;
  }
}

double Neuroseg_Model_Height(const Neuroseg *seg)
{
  return seg->h - 1.0;
}

void Neuroseg_Set_Model_Height(Neuroseg *seg, double len)
{
  seg->h = len + 1.0;
}

double neurofield(double x, double y)
{
  double t = x * x + y * y;

  double value = (1 - t) * exp(-t);

  /*
  if (value < 0) {
    value *= 2.0;
  }
  */
  //double value = exp(-t) - exp(-1.0);

  /*
  if (value > 0.0) {
    value *= r;
  }
  */
  return value;
}

#define NEUROSEG_PLANE_FIELD(x, y)			\
  pcoord[*length][0] = x;				\
  pcoord[*length][1] = y;				\
  value[*length] = field_func(x, y);			\
  (*length)++;						\
  if (x != 0.0) {					\
    pcoord[*length][0] = -x;				\
    pcoord[*length][1] = y;				\
    value[*length] = value[*length - 1];		\
    (*length)++;					\
  }							\
  if (y != 0.0) {					\
    pcoord[*length][0] = x;				\
    pcoord[*length][1] = -y;				\
    value[*length] = value[*length - 1];		\
    (*length)++;					\
    if (x != 0.0) {					\
      pcoord[*length][0] = -x;				\
      pcoord[*length][1] = -y;				\
      value[*length] = value[*length - 1];		\
      (*length)++;					\
    }							\
  }

void Neuroseg_Slice_Field(coordinate_3d_t *pcoord, double *value, int *length,
			  Neuroseg_Field_f field_func)
{ 
  if (field_func == NULL) {
    field_func = neurofield;
  }

  *length = 0;

  double start = 0.2;
  double end = 1.65;
  double y = 0.0;
  double x = 0.0;
  double step = 0.2;

  double range = (end - 0.05) * (end - 0.05) + 0.1;

  /* x = 0.0, y = 0.0; 1 */
  value[*length] = field_func(x, y);
  pcoord[*length][0] = 0.0;
  pcoord[*length][1] = 0.0;
  (*length)++;


  /* y = 0.0, x = -1.8 : 1.8 \ 0.0; 18 */
  for (x = start; x < end; x += step) {
    pcoord[*length][0] = x;		
    pcoord[*length][1] = y;		
    value[*length] = field_func(x, y);
    (*length)++;				
    pcoord[*length][0] = -x;		
    pcoord[*length][1] = y;		
    value[*length] = value[*length - 1];
    (*length)++;				
  }

  /* x = 0.0, y = -1.8 : 1.8 \ 0.0; 18 */
  x = 0.0;
  for (y = start; y < end; y += step) {
    pcoord[*length][0] = x;		
    pcoord[*length][1] = y;		
    value[*length] = field_func(x, y);
    (*length)++;				
    pcoord[*length][0] = x;		
    pcoord[*length][1] = -y;		
    value[*length] = value[*length - 1];
    (*length)++;				
  }

  /* y = 0.2 : 0.8, x = -1.8 : 1.8 \ 0.0; 18 */
  for (y = start; y < 0.85; y += step) {
    for (x = start; x < end; x+= step) {
      if (x * x + y * y < range) {
        NEUROSEG_PLANE_FIELD(x, y);
      }
    }
  }


  /* y = 1.0, x = -1.6 : 1.6 \ 0.0; 16 */
  y = 1.0;
  for (x = start; x < 1.65; x+= step) {
    if (x * x + y * y < range) {
      NEUROSEG_PLANE_FIELD(x, y);
    }
  }

  /* y = 1.2 : 1.4, x = -1.4 : 1.4 \ 0.0; 14 */
  for (y = 1.2; y < 1.45; y += step) {
    for (x = 0.2; x < 1.45; x+= step) {
      if (x * x + y * y < range) {
        NEUROSEG_PLANE_FIELD(x, y);
      }
    }
  }

  /* y = 1.6, x = -1.0 : 1.0 \ 0.0; 8 */
  if (end >= 1.6) {
    y = 1.6;
    for (x = start; x < 1.05; x+= step) {
      if (x * x + y * y < range) {
        NEUROSEG_PLANE_FIELD(x, y);
      }
    }
  }

  /* y = 1.8, x = -0.8 : 0.8; 6 */
  if (end >= 1.8) {
    y = 1.8;
    for (x = start; x < 0.85; x+= step) {
      if (x * x + y * y < range) {
        NEUROSEG_PLANE_FIELD(x, y);
      }
    }
  }
}

void Neuroseg_Slice_Field_P(coordinate_3d_t *pcoord, double *value, int *length,
			    Neuroseg_Field_f field_func)
{ 
  if (field_func == NULL) {
    field_func = neurofield;
  }
  
  *length = 0;
  
  double y = 0.0;
  double x = 0.0;
  double step = 0.2;
  double start = 0.2;
  double end = 0.85;

  /* x = 0.0, y = 0.0; 1 */
  value[*length] = field_func(x, y);
  pcoord[*length][0] = 0.0;
  pcoord[*length][1] = 0.0;
  (*length)++;

  for (x = start; x < end; x += step) {
    pcoord[*length][0] = x;		
    pcoord[*length][1] = y;		
    value[*length] = field_func(x, y);
    (*length)++;				
    pcoord[*length][0] = -x;		
    pcoord[*length][1] = y;		
    value[*length] = value[*length - 1];
    (*length)++;				
  }

  x = 0.0;
  for (y = start; y < end; y += step) {
    pcoord[*length][0] = x;		
    pcoord[*length][1] = y;		
    value[*length] = field_func(x, y);
    (*length)++;				
    pcoord[*length][0] = x;		
    pcoord[*length][1] = -y;		
    value[*length] = value[*length - 1];
    (*length)++;				
  }
  
  for (y = start; y < 0.45; y += step) {
    for (x = start; x < end; x+= step) {
      NEUROSEG_PLANE_FIELD(x, y);
    }
  }

  if (y < 0.65) {
    y = 0.6;
    for (x = start; x < 0.65; x+= step) {
      NEUROSEG_PLANE_FIELD(x, y);
    }
  }

  y = 0.8;
  for (x = start; x < 0.45; x+= step) {
    NEUROSEG_PLANE_FIELD(x, y);
  }
}

Geo3d_Scalar_Field* Neuroseg_Field_Sp(const Neuroseg *seg,
				      Neuroseg_Field_f field_func,
				      Geo3d_Scalar_Field *field)
{
  if ((seg->r1 == 0) || (seg->scale == 0)) {
    return NULL;
  }

  int nslice = NEUROSEG_DEFAULT_H;

  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, NEUROSEG_SLICE_FIELD_LENGTH * nslice);
  }

  double z_start = 0.0;
  //double z_step = seg->h / nslice;
  double z_step = (seg->h - 1.0) / (nslice - 1);
  int length;
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  double coef = NEUROSEG_COEF(seg) * z_step;
  double r = seg->r1;
  double z = z_start;

  field->size = 0;

  int j;
  for (j = 0; j < nslice; j++) {
    Neuroseg_Slice_Field_P(points, values, &length, field_func);
    int i;
    double weight = 0.0;
    for (i = 0; i < length; i++) {
      points[i][0] *= r * seg->scale;
      points[i][1] *= r;
      points[i][2] = z;
      weight += fabs(values[i]);
    }
    for (i = 0; i < length; i++) {
      values[i] /= weight;
    }
    z += z_step;
    r += coef;
    points += length;
    values += length;
    field->size += length;
  }


  if (seg->alpha != 0.0) {
    Rotate_Z(Coordinate_3d_Double_Array(field->points),
	     Coordinate_3d_Double_Array(field->points),
	     field->size, seg->alpha, 0);
  }

  if (seg->curvature >= NEUROSEG_MIN_CURVATURE) {
    double curvature = seg->curvature;
    if (curvature > NEUROSEG_MAX_CURVATURE) {
      curvature = NEUROSEG_MAX_CURVATURE;
    }

    Geo3d_Point_Array_Bend(field->points, field->size, seg->h / curvature);
  }

  if ((seg->theta != 0.0) || (seg->psi != 0.0)) {
    Rotate_XZ((double *)field->points, (double *)field->points, field->size, 
	      seg->theta, seg->psi, 0);
  }

  return field;
}

static void neuroseg_scale_field(coordinate_3d_t pt, double *value,
				 double r, double scale, double r_scale,
				 double sqrt_r, double sqrt_r_scale)
{
  if (value[0] >= 0.0) {
    pt[0] *= r_scale;
    pt[1] *= r;
    value[0] *= sqrt_r_scale;
    /*
    if (scale <= 1.0) {
      value[0] *= sqrt_r;
    } else {
      value[0] *= sqrt_r_scale;
    }
    */
      //value[0] *= sqrt(r * sqrt(MIN2(scale, 1.0)));
  } else {    
#if 1
    /* length in the base receptor */
    double norm = sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
    double alpha = norm - 1;

    /* positive boundary */
    pt[0] *= r_scale / norm;
    pt[1] *= r / norm;

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
#else
    double norm = sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
    double t = 2.0 / 0.6;
    double t2 = t * (norm - 1) + r * sqrt(scale * scale + 1.0);
    pt[0] *= scale;
    double enorm = sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
    t2 /= enorm;
    pt[0] *= t2;
    pt[1] *= t2;
#endif
    //value[0] = -exp(- (1.0 + alpha));

    //pt[0] *= r;
    //pt[1] *= r;

    /*
       double t = pt[1] * pt[1];
       double d1 = sqrt(pt[0] * pt[0] + t);
       pt[0] *= scale;
       double d = sqrt(pt[0] * pt[0] + t);

       t = d / d1;
       if (t > 2.0) {
       t = 2.0;
       }
       double s = (r - t) / d1 + t;
       pt[0] *= s;
       pt[1] *= s;
       */
    //pt[0] *= (r - t) / d1 + t;
    //pt[1] *= (r - t) / d1 + t;
  }
}

Geo3d_Scalar_Field* Neuroseg_Field_Z(const Neuroseg *seg, double z, double step,
				     Geo3d_Scalar_Field *field)
{
  if ((seg->r1 == 0) || (seg->scale == 0)) {
    return NULL;
  }

  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, NEUROSEG_SLICE_FIELD_LENGTH);
  }

  int length;
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  /* double coef = (seg->r2 - seg->r1) / seg->h; */
  double coef = NEUROSEG_COEF(seg);
  double r = seg->r1 + coef * z;

  Neuroseg_Slice_Field(points, values, &length, NULL);
  
  field->size = length;

  int i;
  double weight = 0.0;
  double r_scale = r * seg->scale;
  double sqrt_r = sqrt(r);
  double sqrt_r_scale = sqrt_r * sqrt(sqrt(seg->scale));
  for (i = 0; i < length; i++) {
    neuroseg_scale_field(points[i], values + i, r, seg->scale, r_scale,
			 sqrt_r, sqrt_r_scale);
    points[i][2] = z;
    weight += fabs(values[i]);
  }

  for (i = 0; i < length; i++) {
    values[i] /= weight;
  }

  if (seg->alpha != 0.0) {
    Rotate_Z(Coordinate_3d_Double_Array(field->points),
    	     Coordinate_3d_Double_Array(field->points),
    	     field->size, seg->alpha, 0);
  }

  if (seg->curvature >= NEUROSEG_MIN_CURVATURE) {
    double curvature = seg->curvature;
    if (curvature > NEUROSEG_MAX_CURVATURE) {
      curvature = NEUROSEG_MAX_CURVATURE;
    }

    Geo3d_Point_Array_Bend(field->points, field->size, seg->h / curvature);
  }

  if ((seg->theta != 0.0) || (seg->psi != 0.0)) {
    Rotate_XZ((double *)field->points, (double *)field->points, field->size, 
	      seg->theta, seg->psi, 0);
  }

  return field;
}

Geo3d_Scalar_Field* Neuroseg_Field_S(const Neuroseg *seg,
				     Neuroseg_Field_f field_func,
				     Geo3d_Scalar_Field *field)
{
  if ((seg->r1 == 0) || (seg->scale == 0)) {
    return NULL;
  }

  //int nslice = (int) round(seg->h);
  //int nslice = NEUROSEG_DEFAULT_H;
  int nslice = NEUROSEG_DEFAULT_H;
  /*
  if (nslice == 0) {
    nslice = 1;
  }
  */
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    //Neuroseg_Field_Point_Number(seg, step);
    Construct_Geo3d_Scalar_Field(field, NEUROSEG_SLICE_FIELD_LENGTH * nslice);
				 //Neuroseg_Field_Point_Number(seg, step));
  }

  /*
  Neuroseg_Field(seg, step, Coordinate_3d_Double_Array(field->points), 
		 field->values, &(field->size));
  */

  double z_start = 0.0;
  double z_step = (seg->h - 1.0) / (nslice - 1);
  //double z_step = (seg->h - 1.0) / nslice;
  //double z_step = seg->h / nslice;
  
  //double z_end = nslice;
  int length;
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  double coef = NEUROSEG_COEF(seg) * z_step;
  double r = seg->r1;
  double z = z_start;

  Neuroseg_Slice_Field(points, values, &length, field_func);
  field->size = length;

  int i, j;
  double weight = 0.0;
  double r_scale = r * seg->scale;
  double sqrt_r = sqrt(r);
  double sqrt_sqrt_scale = sqrt(sqrt(seg->scale));
  double sqrt_r_scale = sqrt_r * sqrt_sqrt_scale;
  for (j = 1; j < nslice; j++) {
    z += z_step;
    if (coef != 0.0) {
      r += coef;
      r_scale = r * seg->scale;
      sqrt_r = sqrt(r);
      sqrt_r_scale = sqrt_r * sqrt_sqrt_scale;
    }
    points += length;
    values += length;
    field->size += length;    

    memcpy(points, field->points, sizeof(coordinate_3d_t) * length);
    memcpy(values, field->values, sizeof(double) * length);
    weight = 0.0;
    for (i = 0; i < length; i++) {
      points[i][2] = z;
      neuroseg_scale_field(points[i], values + i, r, seg->scale, r_scale,
			   sqrt_r, sqrt_r_scale);
      weight += fabs(values[i]);
      //weight += values[i] * values[i];
    }
    for (i = 0; i < length; i++) {
      //values[i] /= sqrt(weight);
      values[i] /= weight;
    }
  }

  points = field->points;
  values = field->values;
  r = seg->r1;
  r_scale = r * seg->scale;
  sqrt_r = sqrt(r);
  sqrt_r_scale = sqrt_r * sqrt_sqrt_scale;
  weight = 0.0;
  for (i = 0; i < length; i++) {
    points[i][2] = z_start;
    neuroseg_scale_field(points[i], values + i, r, seg->scale, r_scale,
			 sqrt_r, sqrt_r_scale);
    weight += fabs(values[i]);
    //weight += values[i] * values[i];
  }
  for (i = 0; i < length; i++) {
    values[i] /= weight;
    //values[i] /= sqrt(weight);
  }
  
  
  if (seg->alpha != 0.0) {
    Rotate_Z(Coordinate_3d_Double_Array(field->points),
	     Coordinate_3d_Double_Array(field->points),
	     field->size, seg->alpha, 0);
  }

  if (seg->curvature >= NEUROSEG_MIN_CURVATURE) {
    double curvature = seg->curvature;
    if (curvature > NEUROSEG_MAX_CURVATURE) {
      curvature = NEUROSEG_MAX_CURVATURE;
    }

    Geo3d_Point_Array_Bend(field->points, field->size, seg->h / curvature);
  }

  if ((seg->theta != 0.0) || (seg->psi != 0.0)) {
    Rotate_XZ(Coordinate_3d_Double_Array(field->points), 
	      Coordinate_3d_Double_Array(field->points),
	      field->size, seg->theta, seg->psi, 0);
  }

  return field;
}


/*
Geo3d_Scalar_Field* Neuroseg_Field_S(const Neuroseg *seg, double step,
				     Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    //Neuroseg_Field_Point_Number(seg, step);
    Construct_Geo3d_Scalar_Field(field, 
				 Neuroseg_Field_Point_Number(seg, step));
  }

  Neuroseg_Field(seg, step, Coordinate_3d_Double_Array(field->points), 
		 field->values, &(field->size));


  //Print_Geo3d_Scalar_Field(field);

  return field;
}
*/
 /*
Geo3d_Scalar_Field* Neuroseg_Field_Sp(const Neuroseg *seg, double step,
				     Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    //Neuroseg_Field_Point_Number(seg, step);
    Construct_Geo3d_Scalar_Field(field, 
				 Neuroseg_Field_Point_Number(seg, step));
  }

  Neuroseg_Field_P(seg, step, Coordinate_3d_Double_Array(field->points), 
		 field->values, &(field->size));


  //Print_Geo3d_Scalar_Field(field);

  return field;
}
 */

void Neuroseg_Set_Var(Neuroseg *seg, int var_index, double value)
{
  double *var[NEUROSEG_NPARAM];
  Neuroseg_Var(seg, var);
  *(var[var_index]) = value;
}

void Neuroseg_Normalize_Orientation(Neuroseg *seg)
{  
  double ort[3];
  Geo3d_Orientation_Normal(seg->theta, seg->psi, ort, ort + 1, ort + 2);
  Geo3d_Normal_Orientation(ort[0], ort[1], ort[2], &(seg->theta), &(seg->psi));
}
