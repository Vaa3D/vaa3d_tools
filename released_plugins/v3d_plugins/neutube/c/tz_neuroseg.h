/**@file tz_neuroseg.h
 * @brief neuron segment
 * @author Ting Zhao
 * @date 23-Nov-2007
 */

#ifndef _TZ_NEUROSEG_H_
#define _TZ_NEUROSEG_H_

#include <math.h>
#include <stdio.h>
#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_image_lib.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_constant.h"
#include "tz_neurofield.h"

__BEGIN_DECLS

/*
#if NEUROFIELD == 3
#define MIN_SCORE -5000.0
#else
#define MIN_SIGNIFICANCE 0.0
#define MIN_CORRCOEF 0.2
#define MIN_SCORE MIN_CORRCOEF
#endif
*/

#define NEUROSEG_NPARAM 8

#define NEUROSEG_MIN_CURVATURE 0.2
#define NEUROSEG_MAX_CURVATURE TZ_PI

/**@struct _Neuroseg tz_neuroseg.h
 *
 * The structure of neuron segment. Any instance of the structure is called a 
 * neuroseg.
 * r1, c scale, and h are geometry properties. rx = r1 * scale, ry = r1. h = 1
 * means the segment has overlapping bottom and top.
 * The distance between the top and bottom slice of a segment is h-1, which is
 * called the model height.
 * theta, psi, curvature, alpha, scale are transformation properties.
 */
typedef struct _Neuroseg {
  double r1;    /**< radius of the bottom             */
  double c;     /**< expand coeffient                 */
  double h;     /**< height                           */
  double theta; /**< rotation around X  (unit radian) */
  double psi;   /**< rotation around Z  (unit radian) */
  double curvature; /**< the ratio between height and radius */
  double alpha; /**< rotation around z (happens before theta and psi ) */
  double scale; /**< scale (rx / ry) or eccentricity */
} Neuroseg;

/**@struct _Neuorseg_Slice tz_neuroseg.h
 *
 * The structure of a slice of a neuron segment.
 */
typedef struct _Neuroseg_Slice {
  double r1;    /**< radius of the bottom             */
  double c;     /**< expand coeffient                 */
  double theta; /**< rotation around X  (unit radian) */
  double psi;   /**< rotation around Z  (unit radian) */
  double alpha; /**< rotation around z (happens before theta and psi ) */
  double scale; /**< scale (rx / ry) or eccentricity*/  
} Neuroseg_Slice;

typedef enum { /* reference of the segment position */
  NEUROSEG_REFERENCE_UNDEF, /* undefined reference position */
  NEUROSEG_BOTTOM = 0, /* bottom as reference position */
  NEUROSEG_TOP,        /* top as reference position */
  NEUROSEG_CENTER      /* center as reference position */
} Neuropos_Reference_e;

#define NEUROSEG_MIN_R 0.5

#define NEUROSEG_DEFAULT_R1 1.0
#define NEUROSEG_DEFAULT_C 0.0
#define NEUROSEG_DEFAULT_H 11.0
#define NEUROSEG_DEFAULT_THETA 0.0
#define NEUROSEG_DEFAULT_PSI 0.0
#define NEUROSEG_DEFAULT_CURVATURE 0.0
#define NEUROSEG_DEFAULT_ALPHA 0.0
#define NEUROSEG_DEFAULT_SCALE 1.0


enum { NEUROSEG_CIRCLE_RX, NEUROSEG_CIRCLE_RY, NEUROSEG_CIRCLE_RXY };

typedef Neuroseg* Neuroseg_P;

/**@struct _Field_Range tz_neuroseg.h
 */
typedef struct _Field_Range {
  int first_corner[3];  /* position of the 1st corner (see external doc) */
  int size[3];          /* size of the cuboid                            */
} Field_Range;

typedef double (*Neuroseg_Field_f) (double, double);

inline static Neuroseg_Field_f Neuroseg_Slice_Field_Func(int option)
{
  Neuroseg_Field_f f = NULL;
 
  switch (option) {
  case 1:
    f = Neurofield_S1;
    break;
  case 2:
    f = Neurofield_S2;
    break;
  case 3:
    f = Neurofield_S3;
    break;
  case 4:
    f = Neurofield_Rpi;
  default:
    printf("The default field is used.");
    break;
  }
  
  return f;
}

/*
 * Constructor and destructor for neuron segment. New_Neuroseg() returns a new
 * neuron segment and Delete_Neuroseg() release a neuron segment.
 */
Neuroseg* New_Neuroseg();
void Delete_Neuroseg(Neuroseg *seg);

/**@brief Initialize a neuroseg.
 *
 * Reset_Neuroseg() initializes <seg> with default parameters.
 */
void Reset_Neuroseg(Neuroseg *seg);

/**@brief Set attributes of a neuroseg.
 *
 * Set_Neuroseg() sets the attributes of a neuron segment.
 */
void Set_Neuroseg(Neuroseg *seg, double r1, double c, 
		  double h, double theta, double psi, double curvature,
		  double alpha, double scale);

/**@brief Copy a neuroseg.
 *
 * Neuroseg_Copy() copies the attributes of \a src to \a dst.
 */
void Neuroseg_Copy(Neuroseg *dst, const Neuroseg *src);

/**@brief print a neuroseg to the file pointer
 */
void Fprint_Neuroseg(FILE *fp, const Neuroseg *seg);

/**@brief print a neuroseg to the standard output
 *
 * Print_Neuroseg print a neuron segment to the starndard output.
 */
void Print_Neuroseg(const Neuroseg *seg);

/**@brief Write a neuroseg to a file.
 *
 * Neuroseg_Fwrite() writes <seg> to the file stream <fp>. 
 */
void Neuroseg_Fwrite(const Neuroseg *seg, FILE *fp);

/**@brief read a neuroseg from a file.
 *
 * Neuroseg_Fread() reads a neuron segment to <seg> from <fp>. It supposes the 
 * segment is wrtten by Neuroseg_Fwrite(). If it is read successfully, it
 * returns the pointer where the result is stored. The pointer will be the same
 * <seg> when <seg> is not NULL. If it is failed, it returns NULL.
 */
Neuroseg* Neuroseg_Fread(Neuroseg *seg, FILE *fp);

/**@brief Set orientation of a segment.
 *
 * Neuroseg_Set_Orientation() sets the orientation of <seg> to <theta> 
 * (around x) and <psi> (around z).
 */
void Neuroseg_Set_Orientation(Neuroseg *seg, double theta, double psi);

/**@brief Set orientation of a segment using a vector.
 *
 * Neuroseg_Set_Orientation_V() change the orientation of <seg> so that it has
 * the same orientation as the vector (<x>, <y>, <z>). If the vector has length
 * 0, the orientation is set to (0, 0).
 */
void Neuroseg_Set_Orientation_V(Neuroseg *seg, double x, double y, double z);

/*
 * Neuroseg attributes
 */
/* scaling coefficient from bottom to top */
#define NEUROSEG_COEF(seg)					\
  (((seg)->h == 1.0) ? (seg)->c : dmax2((seg)->c, (NEUROSEG_MIN_R - (seg)->r1) / ((seg)->h - 1.0)))
/* radius at z */
#define NEUROSEG_RADIUS(seg, z)	((seg)->r1 + (z) * NEUROSEG_COEF((seg)))  

/* bottom radius */
#define NEUROSEG_R1(seg) ((seg)->r1)
/* top radius */
#define NEUROSEG_R2(seg) ((seg)->r1 + ((seg)->h - 1.0) * NEUROSEG_COEF((seg)))
/* radius of the cetral plane */
#define NEUROSEG_RC(seg) ((seg)->r1 + ((seg)->h - 1.0) * NEUROSEG_COEF(seg) / 2.0)
/* minimum radius */
#define NEUROSEG_RA(seg) (((seg)->c >= 0) ? (seg)->r1 : NEUROSEG_R2(seg))
/* maximum radius */
#define NEUROSEG_RB(seg) (((seg)->c <= 0) ? (seg)->r1 : NEUROSEG_R2(seg))

/* calibrated bottom radius */
#define NEUROSEG_CR1(seg) (NEUROSEG_R1(seg) * sqrt((seg)->scale))
/* calibrated top radius */
#define NEUROSEG_CR2(seg) (NEUROSEG_R2(seg) * sqrt((seg)->scale))
/* calibrated central radius */
#define NEUROSEG_CRC(seg) (NEUROSEG_RC(seg) * sqrt((seg)->scale))
/* calibrated mimimal radius */
#define NEUROSEG_CRA(seg) (NEUROSEG_RA(seg) * sqrt((seg)->scale))
/* calibrated maximal radius */
#define NEUROSEG_CRB(seg) (NEUROSEG_RB(seg) * sqrt((seg)->scale))

/* calibrated bottom radius with more long axis weight */
#define NEUROSEG_MR1(seg) (((seg)->scale > 1.0) ? NEUROSEG_R1(seg) * (0.2 + (seg)->scale * 0.8) : NEUROSEG_R1(seg) * (0.8 + (seg)->scale * 0.2))
/* calibrated top radius with more long axis weight */
#define NEUROSEG_MR2(seg) (((seg)->scale > 1.0) ? NEUROSEG_R2(seg) * (0.2 + (seg)->scale * 0.8) : NEUROSEG_R2(seg) * (0.8 + (seg)->scale * 0.2))
/* calibrated central radius with more long axis weight */
#define NEUROSEG_MRC(seg) (((seg)->scale > 1.0) ? NEUROSEG_RC(seg) * (0.2 + (seg)->scale * 0.8) : NEUROSEG_RC(seg) * (0.8 + (seg)->scale * 0.2))

/**@brief radius along x axis
 */
double Neuroseg_Rx(const Neuroseg *seg, Neuropos_Reference_e ref);

/**@brief physical radius along x axis
 */
double Neuroseg_Rx_P(const Neuroseg *seg, const double *res, 
		     Neuropos_Reference_e ref);

double Neuroseg_Rx_Z(const Neuroseg *seg, double z);
double Neuroseg_Rx_T(const Neuroseg *seg, double t);

/**@brief radius along y axis
 */
double Neuroseg_Ry(const Neuroseg *seg, Neuropos_Reference_e ref);

/**@brief physical radius along y axis
 */
double Neuroseg_Ry_P(const Neuroseg *seg, const double *res, 
		     Neuropos_Reference_e ref);

double Neuroseg_Ry_Z(const Neuroseg *seg, double z);
double Neuroseg_Ry_T(const Neuroseg *seg, double t);

double Neuroseg_Rxy_Z(const Neuroseg *seg, double z);
double Neuroseg_Rxy_T(const Neuroseg *seg, double t);

/**@brief Radius of a neuroseg.
 *
 * <option> can be NEUROSEG_CIRCLE_RX, NEUROSEG_CIRCLE_RY or 
 * NEUROSEG_CIRCLE_RXY.
 */
double Neuroseg_Rc_Z(const Neuroseg *seg, double z, int option);
double Neuroseg_Rc_T(const Neuroseg *seg, double t, int option);

/**@brief Model_Height of a neuroseg.
 *
 * Neuroseg_Model_Height() returns the model height of <seg>, which is defined
 * as the distance between the bottom and top of <seg>.
 */
double Neuroseg_Model_Height(const Neuroseg *seg);

/**@brief Seg the model height of a neuroseg.
 *
 * Neuroseg_Set_Model_Height() sets the model height of <seg> to <len>.
 */
void Neuroseg_Set_Model_Height(Neuroseg *seg, double len);

/**@brief next neuron segment
 *
 * Next_Neuroseg() calculates the segment next to <seg1>. The result is stored
 * in <seg2> if it is not NULL. It returns <seg2> too if <seg2> is not NULL.
 * Otherwise it returns a new object, where the result is stored. The caller
 * is responsible to free the new object.
 */
Neuroseg* Next_Neuroseg(const Neuroseg *seg1, Neuroseg *seg2, double pos_step);

/**@brief range of a neuroseg
 *
 * Neuroseg_Ball_Range() returns the radius of the ball that contain <seg> with 
 * all orientations.
 */
double Neuroseg_Ball_Range(const Neuroseg *seg);

/**@brief range along z axis
 *
 * Neuroseg_Z_Ranage() returns the z range that covers <seg>.
 */
double Neuroseg_Z_Range(const Neuroseg *seg);

/**@brief bounding box of a neuroseg
 *
 * Neuroseg_Field_Range() calculates the sampling range of a neuron segment. 
 * The result is stored in <range>.
 */
void Neuroseg_Field_Range(const Neuroseg *seg, Field_Range *range, 
			  double z_scale);

/**@brief neuroseg field generator
 *
 * Neurofilter() generates a filtering array of a neuron segment in the range
 * determined by <range> and with offset <offpos>. <offpos> is specfically
 * passed for compensating for sampling from an integer grid. The result is
 * stored in <filter> and returns it if <filter> is not NULL. Otherwise the
 * function returns a new pointer to the result array. The caller is
 * responsible to free the new pointer. The size of <filter> is stored in
 * <length> if it is not NULL.
 */
double* Neurofilter(const Neuroseg *seg, double *filter, int *length,
		    const Field_Range *range, const double *offpos,
		    double z_scale);

/**@brief neuroseg distance field generator
 *
 * Neuroseg_Dist_Filter() generates a distance field of <seg>. 
 */
double* Neuroseg_Dist_Filter(const Neuroseg *seg, double *filter, int *length,
			     const Field_Range *range, const double *offpos,
			     double z_scale);

/*
 * Variable mask definition.
 */
#define NEUROSEG_VAR_MASK_R1             0x00000001
#define NEUROSEG_VAR_MASK_C              0x00000002
#ifdef _CONE_SEG_
#  define NEUROSEG_VAR_MASK_R            (NEUROSEG_VAR_MASK_R1 | NEUROSEG_VAR_MASK_C)
#else
#  define NEUROSEG_VAR_MASK_R            NEUROSEG_VAR_MASK_R1
#endif
#define NEUROSEG_VAR_MASK_THETA          0x00000004
#define NEUROSEG_VAR_MASK_PSI            0x00000008
#define NEUROSEG_VAR_MASK_ORIENTATION    0x0000000C
#define NEUROSEG_VAR_MASK_HEIGHT         0x00000010
#define NEUROSEG_VAR_MASK_CURVATURE      0x00000020
#define NEUROSEG_VAR_MASK_ALPHA          0x00000040
#define NEUROSEG_VAR_MASK_SCALE          0x00000080
#define NEUROSEG_VAR_MASK_ALL            0x000000FF
#define NEUROSEG_VAR_MASK_NONE           0x00000000

/**@brief variable mask to indices
 *
 * Neuroseg_Var_Mask_To_Index() turns a mask to variable indices. The masks can
 * be combined by logical operation. It returns the number of variables.
 *
 * Neuroseg_Var_Option_To_Mask() returns a variable mask from mask option. 
 */
int Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index);

/**@brief variable retrieval
 *
 * Neuroseg_Var() retrieves the variables (addresses of the attributes) in a 
 * neuron segment and stores them in <var>. var[0] is the first variable, var[1]
 * is the second variable and so on. It returns the number of variables. <mask>
 * is an array to determine which parameters will be added the variable list.
 * Use Init_Neuroseg_Var_Mask() to build a mask. If <mask> is NULL, all 
 * parameters will be added to the variable list.
 */
int Neuroseg_Var(const Neuroseg *seg, double *var[]);

/**@brief thickness change
 *
 * Neuroseg_Change_Thickness() changes the thickness of a neuron segment by
 * the functin ratio * r + diff.
 */
void Neuroseg_Change_Thickness(Neuroseg *seg, double ratio, double diff);

/**@brief restricted thickness change
 *
 * Neuroseg_Swell() changes the thickness of \a seg with \a ratio and \a diff.
 * But the maximum radius change cannot exceed \a max_diff. When it is exceeded,
 * the thickness of \a seg will be set smaller to meet the restriction.
 */
void Neuroseg_Swell(Neuroseg *seg, double ratio, double diff,
		    double max_diff);

/**@brief coordinate offset along neuroseg axis
 *
 * Neuroseg_Axis_Offset() calulate the coordinate offset of a point on the 
 * central axis of \a seg to the reference point of \a seg. \a axis_offset is 
 * the distance from the axis point to the reference point.
 */
void Neuroseg_Axis_Offset(const Neuroseg *seg, double axis_offset,
			  double pos_offset[3]);

void Neuroseg_Center(const Neuroseg *seg, double *pos);
void Neuroseg_Bottom(const Neuroseg *seg, double *pos);
void Neuroseg_Top(const Neuroseg *seg, double *pos);

/*@brief The theta angle difference between two neurosegs.
 *
 * Neuroseg_Theta_Offset() returns the difference between theta of <seg1> and
 * theta of <seg2>. The range of the difference is [0, PI].
 */
double Neuroseg_Theta_Offset(const Neuroseg *seg1, const Neuroseg *seg2);

/**@brief Test if a points hits a neuroseg.
 *
 * Neuroseg_Hit_Test returns TRUE is the point (<x>, <y>, <z>) hits <seg>,
 * otherwise it returns FALSE.
 */
BOOL Neuroseg_Hit_Test(const Neuroseg *seg, double x, double y, double z);

/**@brief Test if any of a set of points hits a neuroseg.
 *
 * Neuroseg_Hit_Test_A returns TRUE is any element in the point array <pts>
 * hits <seg>, otherwise it returns FALSE.
 */
int Neuroseg_Hit_Test_A(const Neuroseg *seg, coordinate_3d_t *pts, int n);

/**@brief The angle between two neurosegs.
 *
 * Neuroseg_Angle_Between() returns the angle between <seg1> and <seg2>. The
 * range of the returned value is [0, PI].
 */
double Neuroseg_Angle_Between(const Neuroseg *seg1, const Neuroseg *seg2);

/**@brief The angle between two neurosegs with z calibration.
 *
 * Neuroseg_Angle_Between_Z() returns the angle between <seg1> and <seg2> with
 * z-direction scaling by <z_scale>.
 */
double Neuroseg_Angle_Between_Z(const Neuroseg *seg1, const Neuroseg *seg2,
				double z_scale);

/**@addtogroup neuroseg_field_ neuroseg field
 *
 * @{
 */
#define NEUROSEG_SLICE_FIELD_LENGTH 277

/**@brief Neuroseg cross section field
 *
 * Neuroseg_Slice_Field() generates an intensity field for a cross section of
 * the neuroseg filter. The results are stored in \a pcoord (coordinates), 
 * \a value (intensity values), \a length (number of sampling points).
 */
void Neuroseg_Slice_Field(coordinate_3d_t *pcoord, double *value, int *length,
			  Neuroseg_Field_f field_func);

/**@brief Neuroseg cross section positive field
 *
 * Neuroseg_Slice_Field_P() generates an intensity field for a cross section of
 * the neuroseg filter. It only covers positive signal.
 */
void Neuroseg_Slice_Field_P(coordinate_3d_t *pcoord, double *value, 
			    int *length, Neuroseg_Field_f field_func);

/**@brief Intensity field of a neuroseg
 *
 * Neuroseg_Field_S() returns the intensity field of \a seg. \a step is
 * not useful yet. Just set it to 1.0.
 * The returned pointer is the same as \a field if \a field is not NULL.
 */
Geo3d_Scalar_Field* Neuroseg_Field_S(const Neuroseg *seg,
				     Neuroseg_Field_f field_func,
				     Geo3d_Scalar_Field *field);

/**@brief Positive intensity field of a neuroseg
 *
 * Neuroseg_Field_Sp() generates the positive intensity field of \a seg. 
 * \a step is not useful yet. Just set it to 1.0.
 */
Geo3d_Scalar_Field* Neuroseg_Field_Sp(const Neuroseg *seg,
				      Neuroseg_Field_f field_func,
				      Geo3d_Scalar_Field *field);

/**@brief Intensity field of a certain cross section of a neuroseg
 *
 * Neuroseg_Field_Z()
 */
Geo3d_Scalar_Field* Neuroseg_Field_Z(const Neuroseg *seg, double z, double step,
				     Geo3d_Scalar_Field *field);

/**@brief Normalize the orientation of a neuroseg.
 * 
 * Neuroseg_Normalize_Orientation() normalizes the orientation of <seg>. After
 * normalization, all neuroseg with the same orientation will have the same
 * theta and psi values.
 */
void Neuroseg_Normalize_Orientation(Neuroseg *seg);

/**@} */

void Neuroseg_Set_Var(Neuroseg *seg, int var_index, double value);



#ifdef _DEBUG_ /* private functions. for debugging only */

PRIVATE void field_range(const Neuroseg *seg, Field_Range *range, 
			 double z_scale);
PRIVATE double neuroseg_score(const Stack *stack, const Neuroseg *seg, 
			      const double position[3], int option);
PRIVATE void neuroseg_label(Stack *stack, const Neuroseg *seg, 
			    const double position[3], int color);
PRIVATE int update_var_single(const Stack *stack, Neuroseg *seg, 
			      double position[3], 
			      double *var, double delta, double var_min);
PRIVATE void next_seg_pos(const Neuroseg *seg, const double cur_pos[3], 
			  double next_pos[3], double pos_step);
PRIVATE void print_field_range(const Field_Range *range);

#endif /* _DEBUG_ */

__END_DECLS

#endif
