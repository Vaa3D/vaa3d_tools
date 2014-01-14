/* private/tz_local_neuroseg.c
 *
 * 18-Jan-2008 Initial write: Ting Zhao
 */



#define DELTA_R1 0.1
#define DELTA_C 0.1
#define DELTA_H 1.0
#define DELTA_THETA 0.015
#define DELTA_PSI  0.015
#define DELTA_CURVATURE 0.05
#define DELTA_ALPHA 0.015
#define DELTA_SCALE 0.05
#define DELTA_POS 0.5
#define DELTA_ZSCALE 0.1
#define LAMDA 0.05
#define MAX_ITER 500

/* <delta> is an array of the steps to calculate the gradient.
 */
static const double Delta[] = {
  DELTA_R1, DELTA_C, DELTA_THETA, DELTA_PSI, DELTA_H, DELTA_CURVATURE,
  DELTA_ALPHA, DELTA_SCALE,
  DELTA_POS, DELTA_POS, DELTA_POS, DELTA_ZSCALE
};

#ifndef HAVE_LIBGSL
#  define gsl_fcmp Compare_Float
#endif

/* test_zscale(): Test how large a z scale is.
 * 
 * Return: 1 if the z scale is greater than 1;
 *         0 if the z scale is 1;
 *         -1 if the z scale is less than 1.
 */
#define TEST_Z_SCALE_ACCURACY 1e-5
static int test_zscale(double z_scale)
{
  return Compare_Float(z_scale, 1.0, TEST_Z_SCALE_ACCURACY);
}
#undef TEST_Z_SCALE_ACCURACY

#if 0
static void local_neuroseg_axis_position(const Local_Neuroseg *locseg,
					 double apos[3], double axis_offset)
{
  Neuroseg_Axis_Offset(&(locseg->seg), axis_offset, apos);
  Neuropos_Absolute_Coordinate(locseg->pos, apos, apos + 1, apos + 2);
}

static void local_neuroseg_bottom_position(const Local_Neuroseg *locseg, 
					   double bpos[3])
{
  int i;
  double position[3];
  
  Neuropos_Coordinate(locseg->pos, position, position + 1, position + 2);

  switch (Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    for (i = 0; i < 3; i++) {
      bpos[i] = position[i];
    }
    break;
  case NEUROSEG_TOP:
    local_neuroseg_axis_position(locseg, bpos, -locseg->seg.h + 1.0);
    break;
  case NEUROSEG_CENTER:
    local_neuroseg_axis_position(locseg, bpos, (-locseg->seg.h + 1.0) / 2.0);
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}

static void local_neuroseg_top_position(const Local_Neuroseg *locseg, 
					double tpos[3])
{
  int i;
  double position[3];
  
  Neuropos_Coordinate(locseg->pos, position, position + 1, position + 2);

  switch (Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    local_neuroseg_axis_position(locseg, tpos, locseg->seg.h - 1.0);
    break;
  case NEUROSEG_TOP:
    for (i = 0; i < 3; i++) {
      tpos[i] = position[i];
    }
    break;
  case NEUROSEG_CENTER:
    local_neuroseg_axis_position(locseg, tpos, (locseg->seg.h - 1.0) / 2.0);
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}

static void local_neuroseg_center_position(const Local_Neuroseg *locseg, 
					   double cpos[3])
{
  int i;
  double position[3];
  
  Neuropos_Coordinate(locseg->pos, position, position + 1, position + 2);

  switch (Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    local_neuroseg_axis_position(locseg, cpos, (locseg->seg.h - 1.0) / 2.0);
    break;
  case NEUROSEG_TOP:
    local_neuroseg_axis_position(locseg, cpos, (-locseg->seg.h + 1.0) / 2.0);
    break;
  case NEUROSEG_CENTER:
    for (i = 0; i < 3; i++) {
      cpos[i] = position[i];
    }
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}
#endif

/* local_neuroseg_stack_position(): Turn a physical position to a stack 
 *        position.
 *
e * Args: position - physical position;
 *       c - returned stack position;
 *       offpos - returned offset between physical and stack position.
 *
 * Return: void.
 */
static void local_neuroseg_stack_position(const double position[3], int c[3], 
					  double offpos[3], double z_scale)
{
  c[0] = lrint(position[0]);
  c[1] = lrint(position[1]);
  offpos[0] = position[0] - c[0];
  offpos[1] = position[1] - c[1];

  if (test_zscale(z_scale) != 0) {
    c[2] = lrint(position[2] * z_scale);
    offpos[2] = position[2] * z_scale - c[2];
  } else {
    c[2] = lrint(position[2]);
    offpos[2] = position[2] - c[2];
  }
}



PRIVATE double neuroseg_correlation(double *filter, uint8 *signal, int length)
{
  double score = 0.0;
  int i;

  for (i = 0; i < length; i++) {
    score += filter[i] * ((double) signal[i]);
  }

  return score;
}

PRIVATE double neuroseg_overlap(double *filter, uint8 *signal, int length)
{
  double score = 0.0;
  int i;

  for (i = 0; i < length; i++) {
    if (filter[i] > 0.0) {
      score += ((double) signal[i]);
    }
  }

  return score;
}

PRIVATE double neuroseg_significance(double *filter, uint8 *signal, 
				     int length)
{
  double sig = 0.0;
  uint8* outer_signal = (uint8 *) malloc(sizeof(uint8) * length);
  uint8* inner_signal = (uint8 *) malloc(sizeof(uint8) * length);

  int i;
  int outer_counter = 0;
  int inner_counter = 0;
  for (i = 0; i < length; i++) {
    if (filter[i] >= 0.0) {
      inner_signal[inner_counter++] = signal[i];
    } else {
      outer_signal[outer_counter++] = signal[i];
    }
  }
  
  if ((outer_counter > 0) && (inner_counter > 0)) {
    //    gsl_sort_uchar(outer_signal, 1, outer_counter);
    u8array_qsort(outer_signal, NULL, outer_counter);
    uint8 outer_median = outer_signal[outer_counter / 2];
    
    int rank = 0;
    for (i = 0; i < inner_counter; i++) {
      if (inner_signal[i] > outer_median) {
	rank++;
      }
    }

    sig = ((double) rank) / inner_counter;
  }
  
  free(outer_signal);
  free(inner_signal);

  return sig;
}

PRIVATE double neuroseg_fit_statistic(double *filter, double *signal, 
				      int length)
{
  double stat = 0.0;
  double mu = darray_mean(signal, length);
  double var = 0.0;

  int i;
  for (i = 0; i < length; i++) {
    var += (signal[i] - mu) * (signal[i] - mu);
  }

  if (var == 0.0) {
    stat = 0.0;
  } else {
    double score = darray_dot(filter, signal, length);
    double filter_sum = darray_sum(filter, length);
    double filter_square_sum = darray_dot(filter, filter, length);
    var /= (double) (length - 1);
    
    stat = (score - filter_sum * mu) / sqrt(filter_square_sum * var);
  }

  return stat;
}
