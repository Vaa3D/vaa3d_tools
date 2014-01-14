/* private/tz_neurotrace.c
 *
 * 28-Jan--08 Initial write: Ting Zhao
 */

static double Neuroseg_Max_Radius = 10.0;
#define NEUROTRACE_MIN_SCORE 10.0
#undef NEUROTRACE_MIN_SCORE
#define NEUROTRACE_MIN_SCORE 0.3

PRIVATE double refit_local_neuroseg(Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale,
				    Neuroseg_Fit_Score *fs)
{
  locseg->seg.r1 *= 2.0;

  //  if (locseg->seg.r1 > locseg->seg.r2) {
  if (locseg->seg.c < 0.0) {
    //locseg->seg.r2 = locseg->seg.r1;
    locseg->seg.c = 0.0;
  } else if (locseg->seg.c > 0.0) {
    locseg->seg.r1 = NEUROSEG_R2(&(locseg->seg));
    locseg->seg.c = 0.0;
      //locseg->seg.r1 = locseg->seg.r2;
  }
      
  locseg->seg.h *= 2.0;

  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION, 
					      NEUROPOS_VAR_MASK_NONE, 
					      var_index);

  locseg->seg.h /= 2.0;

  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, NULL);

  nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R, 
					  NEUROPOS_VAR_MASK_NONE, 
					  var_index);

  return Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, fs);
}

static BOOL is_trace_valid(const Local_Neuroseg *locseg, const Stack *mask,
			   Neurochain *chain,
			   double score, double *theta, double *psi,
			   BOOL *hit_traced, int index, double z_scale,
			   int *invalid_code)
{
  BOOL is_valid = TRUE;
  *invalid_code = 0;

  if (score < NEUROTRACE_MIN_SCORE) { /* score too low */
    printf("low score\n");
    is_valid = FALSE;
    *invalid_code = 1;
  }

  if (is_valid == TRUE) {
    if ((locseg->seg.r1 >= Neuroseg_Max_Radius) || 
	(NEUROSEG_R2(&(locseg->seg)) >= Neuroseg_Max_Radius)) { /* too thick*/
      printf("too thick\n");
      *invalid_code = 3;
      is_valid = FALSE;
    }
  }
    
  if (is_valid == TRUE) {
    double max_change = Neuroseg_Max_Radius / 3;
    if (fabs(locseg->seg.r1 - NEUROSEG_R2(&(locseg->seg))) >= max_change) { /* irregular */
      printf("irregular tube\n");
      is_valid = FALSE;
    } else if (dmax2(locseg->seg.r1, NEUROSEG_R2(&(locseg->seg))) > 2.5) {
      if (dmax2(locseg->seg.r1, NEUROSEG_R2(&(locseg->seg))) / 
	  dmin2(locseg->seg.r1, NEUROSEG_R2(&(locseg->seg))) > 3.0) {
	printf("irregular tube\n");
	is_valid = FALSE;
      }
    }
    *invalid_code = 4;
  }

  if (is_valid == TRUE) {
    theta[index] = locseg->seg.theta;
    psi[index] = locseg->seg.psi;

    if (index > 0) { /* bounced back */
      Geo3d_Vector v1, v2;
      Geo3d_Orientation_Normal(theta[index - 1], psi[index - 1],
			       &(v1.x), &(v1.y), &(v1.z));      
      Geo3d_Orientation_Normal(theta[index], psi[index],
			       &(v2.x), &(v2.y), &(v2.z));

#if 0
      if (Geo3d_Vector_Angle2(&v1, &v2) > 2.5) { 
	printf("bounced back: %g, %g\n", theta[index], theta[index - 1]);
	is_valid = FALSE;
      }
#endif
    }
    *invalid_code = 5;
  }

  if (is_valid == TRUE) {
    if (chain != NULL) {
      double top_pos[3];
      Local_Neuroseg_Top(locseg, top_pos);
      int hit_index = Neurochain_Hit_Test(chain, BACKWARD, 0, 
					  top_pos[0], top_pos[1], top_pos[2]);
      if (hit_index > 0) {
	printf("Loop formed: %d\n", hit_index);
	is_valid = FALSE;
      }
    }
    *invalid_code = 6;
  }

  if (is_valid == TRUE) {
    if (mask != NULL) { /* meet traced region */
      hit_traced[index] = Neuroseg_Hit_Traced(locseg, mask, z_scale);

      /*
      if (hit_traced[index] == TRUE) {
	printf("hit traced region\n");
	is_valid = FALSE;
	*invalid_code = 2;
      }
      */           
      
      if ((hit_traced[index] == TRUE) && (index > 0)) {
	if (hit_traced[index - 1] == TRUE) {
	  printf("hit traced region\n");
	  is_valid = FALSE;
	  *invalid_code = 2;
	} else {
	  *invalid_code = 2;
	}
      }      
    }
  }

  return is_valid;
}

void update_cache_seg(Local_Neuroseg *cache[], int ncache, 
		      const Local_Neuroseg *seg)
{
  int i;
  for (i = 0; i < ncache - 1; i++) {
    Local_Neuroseg_Copy(cache[i],  cache[i + 1]);
  }

  Local_Neuroseg_Copy(cache[ncache - 1], seg);
}

void find_soma_range(double *area_array, int length, double threshold,
		     int *range)
{
  int max_len = 0;
  int cur_len = 0;
  int status = 2;
  int cur_ends[] = {-1, -1};
  int i;

  range[0] = range[1] = -1;

  for (i = 0; i < length; i++) {
    switch (status) {
    case 0: /* start */
      if (area_array[i] >= threshold) {
	status = 1;
      } else {
	status = 2;
      }
      break;
    case 1: /* begin */
      if (area_array[i] < threshold) {
	status = 2;
      }
      break;
    case 2: /* wait */
      if (area_array[i] >= threshold) {
	status = 3;
	cur_ends[0] = i;
	cur_len = 1;
      }
      break;
    case 3: /* open */
      if (area_array[i] >= threshold) {
	cur_len++;
      } else {
	cur_ends[1] = i;
	if (cur_len > max_len) {
	  range[0] = cur_ends[0];
	  range[1] = cur_ends[1];
	  max_len = cur_len;
	}
	status = 2;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
    }
  }

  if (status == 3) {/* still open */
    cur_ends[1] = i;
    if (cur_len > max_len) {
      range[0] = cur_ends[0];
      range[1] = cur_ends[1];
      max_len = cur_len;
    }
    status = 2;
  }
}

double soma_score_from_area(double *area, int length)
{
  int center;
  double max_area = darray_max(area, length, &center);

  if ((double) (length * 2) < sqrt(max_area)) {
    return 0.0;
  }


  int start = imax2(0, center - length / 5);
  int end = imin2(length - 1, center + length / 5);

  //double total_area = darray_sum(area, length);
  double large_area = 0.0;
  double small_area = 0.0;
  int i;
  for (i = start; i <= end; i++) {
    large_area += area[i];
  }

  large_area /= end - start + 1;

  start /= 2;
  for (i = 0; i < start; i++) {
    small_area += area[i];
  }

  end = (length + end) / 2;
  for (i = end; i < length; i++) {
    small_area += area[i];
  }

  small_area /= length - end + start;

  return large_area / small_area;
}
