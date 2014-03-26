/* private/tz_neuroseg.c
 * Initial write: Ting Zhao
 */

#ifndef HAVE_LIBGSL
#  define gsl_fcmp Compare_Float
#endif

/* test_zscale(): Test how large a z scale is.
 * 
 * Args: z_scale - Scale for Z axis. When it is not 1, the z coordiate will be 
 *                 multiplied by <Z_scale> while x and y coordinates are unchanged.
 *
 * Return: 1 if the z scale is greater than 1;
 *         0 if the z scale is 1;
 *         -1 if the z scale is less than 1.
 */
#define TEST_Z_SCALE_ACCURACY 1e-5
inline static int test_zscale(double z_scale)
{
  return Compare_Float(z_scale, 1.0, TEST_Z_SCALE_ACCURACY);
}
#undef TEST_Z_SCALE_ACCURACY

/* compare_angle(): Compare angles.
 *
 * Note: the uint of the angles is radian.
 *
 * Args: a1 - angle 1;
 *       a2 - angle 2.
 *
 * Return: 0 means the two angles are the same.
 *         1 means a1 > a2.
 *         -1 means a1 < a2.
 */
#define COMPARE_ANGLE_ACCURACY 1e-5
static inline int compare_angle(double a1, double a2)
{
  return Compare_Float(a1, a2, COMPARE_ANGLE_ACCURACY);
}
#undef COMPARE_ANGLE_ACCURACY

/* compare_radius(): Compare radii.
 *
 * Args: r1 - radius 1;
 *       r2 - radius 2.
 *
 * Return: 0 means the two radii are the same.
 *         1 means r1 > r2.
 *         -1 means r1 < r2.
 */
#define COMPARE_RADIUS_ACCURACY 1e-2
static inline int compare_radius(double r1, double r2)
{
  return Compare_Float(r1, r2, COMPARE_RADIUS_ACCURACY);
}
#undef COMPARE_RADIUS_ACCURACY

/* compare_height(): Compare height.
 *
 * Args: h1 - height 1;
 *       h2 - height 2.
 *
 * Return: 0 means the two heights are the same.
 *         1 means h1 > h2.
 *         -1 means h1 < h2.
 */
#define COMPARE_HEIGHT_ACCURACY 1e-2
static inline int compare_height(double h1, double h2)
{
  return Compare_Float(h1, h2, COMPARE_HEIGHT_ACCURACY);
}
#undef COMPARE_HEIGHT_ACCURACY

/* normalize_angle(): Normalize an angle to [0, 2*pi).
 */
static inline double normalize_angle(double a)
{
  double norma = a;
  
  if ((compare_angle(a, 0.0) <= 0) || (compare_angle(a, TZ_2PI) > 0)) {
    norma = a - floor(a / TZ_2PI) * 2.0 * TZ_PI;
  }

  return norma;
}

/* is_valid_radius(): Test if a radius is valid.
 *
 * Args: r - radius.
 *
 * Return: 1 if it is valid or 0 if it is invalid.
 */
static inline int is_valid_radius(double r)
{
  return r > 0.0;
} 

/* is_valid_height(): Test if a height is valid.
 *
 * Args: h - height.
 *
 * Return: 1 if it is valid or 0 if it is invalid.
 */
static inline int is_valid_height(double h)
{
  return h > 0.0;
} 

/* field_range(): Range of the neuron field.
 *
 * Args: seg - neuron segment
 *       range - returned field range
 *
 * Return: no return.
 */
#if 0
PRIVATE inline void field_range(const Neuroseg *seg, Field_Range *range, 
				double z_scale)
{
  if ((seg == NULL) || (range == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }
  
  double d1 = seg->r1 * 2.0;
  double d2 = NEUROSEG_R2(seg) * 2.0;
  double corner_z[4];
  double corner_y[8];
  double corner_x[8];
  double sin_theta = sin(seg->theta);
  double cos_theta = cos(seg->theta);
  double sin_psi = sin(seg->psi);
  double cos_psi = cos(seg->psi);
  int last_corner[3];
  int i;
      
  corner_z[0] = d1 * sin_theta;
  corner_z[1] = d2 * sin_theta + seg->h * cos_theta;
  corner_z[2] = - d2 * sin_theta +  seg->h * cos_theta;
  corner_z[3] = - corner_z[0];

  corner_y[0] = corner_y[1] = d1 * cos_theta;
  corner_y[2] = corner_y[3] = - corner_y[0];
  corner_y[4] = corner_y[5] = d2 * cos_theta - seg->h * sin_theta;
  corner_y[6] = corner_y[7] = - d2 * cos_theta - seg->h * sin_theta;

  corner_x[0] = d1 * cos_psi - corner_y[0] * sin_psi;
  corner_x[1] = - d1 * cos_psi - corner_y[1] * sin_psi;
  corner_x[2] = - corner_x[0];
  corner_x[3] = - corner_x[1];
  corner_x[4] = d2 * cos_psi - corner_y[4] * sin_psi;
  corner_x[5] = - d2 * cos_psi - corner_y[5] * sin_psi;
  corner_x[6] = - d2 * cos_psi - corner_y[6] * sin_psi;
  corner_x[7] = d2 * cos_psi - corner_y[7] * sin_psi;

  corner_y[0] = d1 * sin_psi + corner_y[0] * cos_psi;
  corner_y[1] = -d1 * sin_psi + corner_y[1] * cos_psi;
  corner_y[2] = -corner_y[0];
  corner_y[3] = -corner_y[1];
  corner_y[4] = d2 * sin_psi + corner_y[4] * cos_psi;
  corner_y[5] = - d2 * sin_psi + corner_y[5] * cos_psi;
  corner_y[6] = - d2 * sin_psi + corner_y[6] * cos_psi;
  corner_y[7] = d2 * sin_psi + corner_y[7] * cos_psi;

  if (test_zscale(z_scale) != 0) {
    for (i = 0; i < 4; i++) {
      corner_z[i] /= z_scale;
    }
  }
    
  range->first_corner[0] = lrint(corner_x[0]);
  last_corner[0] = lrint(corner_x[0]);
  range->first_corner[1] =lrint(corner_y[0]);
  last_corner[1] = lrint(corner_y[0]);  
  range->first_corner[2] = lrint(corner_z[0]);
  last_corner[2] = lrint(corner_z[0]);

  for (i = 1; i < 4; i++) {
    if (last_corner[2] < lrint(corner_z[i])) {
      last_corner[2] = lrint(corner_z[i]);
    } else if (range->first_corner[2] > lrint(corner_z[i])) {
      range->first_corner[2] = lrint(corner_z[i]);
    }
  }

  for (i = 1; i < 8; i++) {
    if (last_corner[0] < lrint(corner_x[i])) {
      last_corner[0] = lrint(corner_x[i]);
    } else if (range->first_corner[0] > lrint(corner_x[i])) {
      range->first_corner[0] = lrint(corner_x[i]);
    }

    if (last_corner[1] < lrint(corner_y[i])) {
      last_corner[1] = lrint(corner_y[i]);
    } else if (range->first_corner[1] > lrint(corner_y[i])) {
      range->first_corner[1] = lrint(corner_y[i]);
    }
  }

  for (i = 0; i < 3; i++) {
    range->size[i] = last_corner[i] - range->first_corner[i] + 1;
  }
}
#endif


/* Print_Field_Range(): Print field range.
 * 
 * Args: range - pointer of the field range to print.
 *
 * Return: no return.
 */
PRIVATE void print_field_range(const Field_Range *range)
{
  if (range == NULL) {
    printf("Null range\n");
  } else {
    printf("Field range: \n");
    printf("Starting corner: (%d, %d, %d)\n", range->first_corner[0], 
	   range->first_corner[1], range->first_corner[2]);
    printf("Size: %d x %d x %d\n", 
	   range->size[0], range->size[1], range->size[2]);
  }
}
