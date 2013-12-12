/* tz_local_neuroseg.c
 *
 * 17-Jan-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include <utilities.h>

#include "tz_cdefs.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_math.h>
#  include <gsl/gsl_sort_uchar.h>
#  include <gsl/gsl_matrix.h>
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_blas.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif

#include "tz_local_neuroseg.h"
#include "tz_utilities.h"
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_optimize_utils.h"
#include "tz_gsl_linalg.h"
#include "tz_3dgeom.h"
#include "tz_geo3d_utils.h"
#include "tz_geo3d_point_array.h"
#include "tz_stack_sampling.h"
#include "tz_perceptor.h"
#include "tz_locnp_chain.h"
#include "tz_math.h"
#include "tz_stack_neighborhood.h"
#include "tz_xml_utils.h"
#include "tz_geoangle_utils.h"
#include "tz_workspace.h"
#include "tz_stack_utils.h"
#include "tz_geometry.h"

INIT_EXCEPTION

#include "private/tzp_local_neuroseg.c"

/* New_Local_Neuroseg(): New a local neuron segment.
 *
 * Return: a new local neuron segment. It should be freed by 
 *         Free_Local_Neuroseg().
 */
Local_Neuroseg* New_Local_Neuroseg()
{
  Local_Neuroseg *locseg = 
    (Local_Neuroseg *) Guarded_Malloc(sizeof(Local_Neuroseg), 
				     "New_Local_Neuroseg");
  Default_Local_Neuroseg(locseg);

  return locseg;
}

void Delete_Local_Neuroseg(Local_Neuroseg *locseg)
{
  free(locseg);
}

void Kill_Local_Neuroseg(Local_Neuroseg *locseg)
{
  Delete_Local_Neuroseg(locseg);
}

void Set_Local_Neuroseg(Local_Neuroseg *locseg, double r1, double c,
			double h, double theta, double psi, double curvature,
			double alpha, double scale,
			double x, double y, double z)
{
  ASSERT(locseg != NULL, "Null Local_Neuroseg.");

  Set_Neuroseg(&(locseg->seg), r1, c, h, theta, psi, curvature, alpha, scale);
  Set_Neuropos(locseg->pos, x, y, z);
}

void Default_Local_Neuroseg(Local_Neuroseg *locseg)
{
  if (locseg == NULL) {
    return;
  }
  
  Reset_Neuroseg(&(locseg->seg));
  Reset_Neuropos(locseg->pos);

  locseg->pf1 = locseg->pf2 = locseg->pf3 = NULL;
}

Local_Neuroseg* Copy_Local_Neuroseg(const Local_Neuroseg *locseg)
{
  Local_Neuroseg *new_locseg = New_Local_Neuroseg();
  Local_Neuroseg_Copy(new_locseg, locseg);
  
  return new_locseg;
}

void Local_Neuroseg_Copy(Local_Neuroseg *des, const Local_Neuroseg *src)
{
  Neuroseg_Copy(&(des->seg), &(src->seg));
  Neuropos_Copy(des->pos, src->pos);
}

void Fprint_Local_Neuroseg(FILE *fp, const Local_Neuroseg *locseg)
{
  if (locseg == NULL) {
    fprintf(fp, "Null locseg.\n");
  } else {
    Fprint_Neuroseg(fp, &(locseg->seg));
    fprintf(fp, "Position: ");
    Fprint_Neuropos(fp, locseg->pos);
  }
}

void Print_Local_Neuroseg(const Local_Neuroseg *locseg)
{
  Fprint_Local_Neuroseg(stdout, locseg);
}

Local_Neuroseg *Local_Neuroseg_Import_Xml(const char *file_path, 
					  Local_Neuroseg *locseg)
{
#ifdef HAVE_LIBXML2
  xmlDocPtr doc;
  xmlNodePtr cur;
    
  doc = xmlParseFile(file_path);
  if (doc == NULL) {
    fprintf(stderr, "XML parsing failed.\n");
    return NULL;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr, "empty document\n");
    xmlFreeDoc(doc);
    return NULL;
  }

  if (xmlStrcmp(cur->name, (const xmlChar*) "locseg")) {
    fprintf(stderr, "document of wrong type\n");
    xmlFreeDoc(doc);
    return NULL;
  }

  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "r1") == TRUE) {
      locseg->seg.r1 = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "c") == TRUE) {
      locseg->seg.c = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "height") == TRUE) {
      locseg->seg.h = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "theta") == TRUE) {
      locseg->seg.theta = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "psi") == TRUE){
      locseg->seg.psi = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "curvature") == TRUE) {
      locseg->seg.curvature = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "alpha") == TRUE) {
      locseg->seg.alpha = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "scale") == TRUE) {
      locseg->seg.scale = Xml_Node_Double_Value(doc, cur);
    } else if (Xml_Node_Is_Element(cur, "position") == TRUE) {
      Xml_Coordinate(doc, cur, locseg->pos);
    }

    cur = cur->next;
  }
    
  xmlFreeDoc(doc);
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return locseg;
}

void Local_Neuroseg_Fwrite(const Local_Neuroseg *locseg, FILE *fp)
{
  TZ_ASSERT(fp != NULL, "Null file pointer.");

  if (locseg != NULL) {
    Neuroseg_Fwrite(&(locseg->seg), fp);
    Neuropos_Fwrite(locseg->pos, fp);
  }
}

Local_Neuroseg* Local_Neuroseg_Fread(Local_Neuroseg *locseg, FILE *fp)
{
  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;
  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
    is_local_alloc = TRUE;
  }

  if (Neuroseg_Fread(&(locseg->seg), fp) == NULL) {
    if (is_local_alloc == TRUE) {
      Delete_Local_Neuroseg(locseg);
    }
    locseg = NULL;
  } else {
    if (Neuropos_Fread(locseg->pos, fp) == NULL) {
      if (is_local_alloc == TRUE) {
	Delete_Local_Neuroseg(locseg);
      }
      locseg = NULL;
    }
  }

  return locseg;
}


void Write_Local_Neuroseg_Array(const char *file_path, 
				const Local_Neuroseg *locseg, int length)
{
  ASSERT(locseg != NULL, "Null pointer.");
  ASSERT(file_path != NULL, "Null pointer.");
  ASSERT(length > 0, "Invalid length.");

  FILE *fp = fopen(file_path, "wb");
  
  if (fp == NULL) {
    TZ_ERROR(ERROR_IO_OPEN);
  }

  fwrite(&length, sizeof(int), 1, fp);
  fwrite(locseg, sizeof(Local_Neuroseg), length, fp);
  fclose(fp);
}

Local_Neuroseg* Read_Local_Neuroseg_Array(const char *file_path, int *length)
{
  ASSERT_NOT_NULL(file_path);
  ASSERT_NOT_NULL(length);
  
  FILE *fp = fopen(file_path, "rb");
  
  if (fp == NULL) {
    TZ_ERROR(ERROR_IO_OPEN);
  }
  
  fread(length, sizeof(int), 1, fp);
  
  Local_Neuroseg *locseg = (Local_Neuroseg *)
    Guarded_Malloc(sizeof(Local_Neuroseg) * 
		   *length, "Read_Local_Neuroseg_Array");
  fread(locseg, sizeof(Local_Neuroseg), *length, fp);

  fclose(fp);

  return locseg;
}


void Local_Neuroseg_Axis_Position(const Local_Neuroseg *locseg,
				  double *apos, double axis_offset)
{
  Neuroseg_Axis_Offset(&(locseg->seg), axis_offset, apos);
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  Neuropos_Absolute_Coordinate(bottom, apos, apos + 1, apos + 2);
}

void Local_Neuroseg_Axis_Coord_N(const Local_Neuroseg *locseg, double t,
				 double *coord)
{
  Local_Neuroseg_Axis_Position(locseg, coord, t * (locseg->seg.h - 1.0));
}

void Local_Neuroseg_Center(const Local_Neuroseg *locseg, double pos[])
{
  if (Neuropos_Reference == NEUROSEG_CENTER) {
    Neuropos_Coordinate(locseg->pos, pos, pos + 1, pos + 2);
  } else {
    Local_Neuroseg_Axis_Position(locseg, pos, (locseg->seg.h - 1.0) / 2.0);
  }
}

void Local_Neuroseg_Top(const Local_Neuroseg *locseg, double pos[])
{
  if (Neuropos_Reference == NEUROSEG_TOP) {
    Neuropos_Coordinate(locseg->pos, pos, pos + 1, pos + 2);
  } else {
    Local_Neuroseg_Axis_Position(locseg, pos, locseg->seg.h - 1.0);
  }
}

void Local_Neuroseg_Bottom(const Local_Neuroseg *locseg, double pos[])
{
  switch (Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    Neuropos_Coordinate(locseg->pos, pos, pos + 1, pos + 2);
  break;
  case NEUROSEG_TOP:
    Neuroseg_Axis_Offset(&(locseg->seg), -locseg->seg.h + 1.0, pos);
    Neuropos_Absolute_Coordinate(locseg->pos, pos, pos + 1, pos + 2);
    break;
  case NEUROSEG_CENTER:
    Neuroseg_Axis_Offset(&(locseg->seg), (-locseg->seg.h + 1.0) / 2.0, pos);
    Neuropos_Absolute_Coordinate(locseg->pos, pos, pos + 1, pos + 2);
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}

void  Set_Neuroseg_Position(Local_Neuroseg *locseg, const double pos[],
			    Neuropos_Reference_e ref)
{
  Set_Neuropos(locseg->pos, pos[0], pos[1], pos[2]);
  if (ref != Neuropos_Reference) {
    double axis_offset = 0.0;
    double apos[3];
    switch (Neuropos_Reference) {
    case NEUROSEG_BOTTOM:
      if (ref == NEUROSEG_TOP) {
	axis_offset = -locseg->seg.h + 1.0;
      }
      if (ref == NEUROSEG_CENTER) {
	axis_offset = -(locseg->seg.h - 1.0) / 2.0;
      }
      break;
    case NEUROSEG_TOP:
      if (ref == NEUROSEG_BOTTOM) {
	axis_offset = locseg->seg.h - 1.0;
      }
      if (ref == NEUROSEG_CENTER) {
	axis_offset = (locseg->seg.h - 1.0) / 2.0;
      }
      break;
    case NEUROSEG_CENTER:
      if (ref == NEUROSEG_BOTTOM) {
	axis_offset = (locseg->seg.h - 1.0) / 2.0;
      }
      if (ref == NEUROSEG_TOP) {
	axis_offset = -(locseg->seg.h - 1.0) / 2.0;
      }
      break;
    default:
      THROW(ERROR_DATA_VALUE);
      break;
    }
    
    Neuroseg_Axis_Offset(&(locseg->seg), axis_offset, apos);
    Neuropos_Translate(locseg->pos, apos[0], apos[1], apos[2]);
  }
}

/* Get_Neuroseg_Position(): Get the coordinate of a neuron segment position.
 *
 * Args: locseg - local neuron segment;
 *       pos - the resulted coordinate.
 *
 * Return: void.
 */
void  Get_Neuroseg_Position(const Local_Neuroseg *locseg, double pos[])
{
  Neuropos_Coordinate(locseg->pos, pos, pos + 1, pos +2);
}

void Local_Neuroseg_Normal(const Local_Neuroseg *locseg, double *ort)
{
  Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi,
			   ort, ort + 1, ort + 2);
}

/* Local_Neuroseg_Hit_Test(): Test if a point hits a neuron segment.
 *
 * Args: locseg - the local neuron segment for hit test;
 *       x, y, z - the coordinate of the point.
 *
 * Return: TRUE if it's hit. FALSE if it's missed.
 */
BOOL Local_Neuroseg_Hit_Test(const Local_Neuroseg *locseg, 
			     double x, double y, double z)
{
  double tmp_pos[3];

  //  Neuropos_Coordinate(locseg->pos, tmp_pos, tmp_pos + 1, tmp_pos + 2);
  Local_Neuroseg_Bottom(locseg, tmp_pos);

  tmp_pos[0] = x - tmp_pos[0];
  tmp_pos[1] = y - tmp_pos[1];
  tmp_pos[2] = z - tmp_pos[2];

  Rotate_XZ(tmp_pos, tmp_pos, 1, locseg->seg.theta, locseg->seg.psi, 1);
  /*Scale_X_Rotate_Z(tmp_pos, tmp_pos, 1, locseg->seg.scale, locseg->seg.alpha,
    1);*/

  Rotate_Z(tmp_pos, tmp_pos, 1, locseg->seg.alpha, 1);

  return Neuroseg_Hit_Test(&(locseg->seg), tmp_pos[0], tmp_pos[1], tmp_pos[2]);
}

BOOL Local_Neuroseg_Hit_Test2(const Local_Neuroseg *target,
			      const Local_Neuroseg *locseg)
{
  double center1[3];
  double center2[3];
  Local_Neuroseg_Center(target, center1);
  Local_Neuroseg_Center(locseg, center2);
  center1[0] -= center2[0];
  center1[1] -= center2[1];
  center1[2] -= center2[2];

  double max_dist = 
    dmax3(target->seg.h / 2.0, target->seg.r1, NEUROSEG_R2(&(target->seg))) +
    dmax3(locseg->seg.h / 2.0, locseg->seg.r1, NEUROSEG_R2(&(target->seg)));
  if ((center1[0] * center1[0] + center1[1] * center1[1] + 
       center1[2] * center1[2]) > max_dist * max_dist) {
    return FALSE;
  }

  double pos[3];
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);

  double z;
  for (z = 0.0; z < locseg->seg.h; z += 1.0) {
    Neuroseg_Axis_Offset(&(locseg->seg), z, pos);
    pos[0] += bottom[0];
    pos[1] += bottom[1];
    pos[2] += bottom[2];
    if (Local_Neuroseg_Hit_Test(target, pos[0], pos[1], pos[2])
	== TRUE) {
      return TRUE;
    }
  }
  
  return FALSE;
}

const static double Min_Axis_Dist = 1e-3;

BOOL Local_Neuroseg_Axis_Test(const Local_Neuroseg *locseg,
			      double x, double y, double z, double *t)
{
  double tmp_pos[3];

  //  Neuropos_Coordinate(locseg->pos, tmp_pos, tmp_pos + 1, tmp_pos + 2);
  Local_Neuroseg_Bottom(locseg, tmp_pos);

  tmp_pos[0] = x - tmp_pos[0];
  tmp_pos[1] = y - tmp_pos[1];
  tmp_pos[2] = z - tmp_pos[2];

  Rotate_XZ(tmp_pos, tmp_pos, 1, locseg->seg.theta, locseg->seg.psi, 1);
  Scale_X_Rotate_Z(tmp_pos, tmp_pos, 1, locseg->seg.scale, locseg->seg.alpha,
		   1);

  double dist = sqrt(tmp_pos[0] * tmp_pos[0] + tmp_pos[1] * tmp_pos[1]);
  if (t != NULL) {
    *t = tmp_pos[2];
  }

  if (dist <= Min_Axis_Dist) {
    return TRUE;
  }

  return FALSE;
}

/*
int Local_Neuroseg_Axis_Test2(const Local_Neuroseg *locseg,
			      const Local_Neuroseg *next_locseg)
{
  double pos[3]; 
  Local_Neuroset_Top(locseg, pos);
  
}
*/

BOOL Local_Neuroseg_Contain_Axis(const Local_Neuroseg *container,
				 const Local_Neuroseg *locseg)
{
  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  if (Local_Neuroseg_Hit_Test(container, pos[0], pos[1], pos[2]) == FALSE) {
    return FALSE;
  }

  Local_Neuroseg_Top(locseg, pos);
  if (Local_Neuroseg_Hit_Test(container, pos[0], pos[1], pos[2]) == FALSE) {
    return FALSE;
  }

  return TRUE;
}

void Local_Neuroseg_Stack_Position(const double position[3], int c[3], 
				   double offpos[3], double z_scale)
{
  local_neuroseg_stack_position(position, c, offpos, z_scale);
}

Stack* Local_Neuroseg_Substack(const Local_Neuroseg *seg, const Stack *stack,
			       Field_Range *range, double *offpos, 
			       double z_scale) 
{
  if ((stack == NULL) || (seg == NULL)){
    THROW(ERROR_POINTER_NULL);
  }

  double bottom_position[3];
  
  Local_Neuroseg_Bottom(seg, bottom_position);

  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);
   
  Neuroseg_Field_Range(&(seg->seg), range, z_scale);
  
  if((range->size[0] == 0) || (range->size[0] == 0) || (range->size[0] == 0)) {
    printf("Empty range\n");
    TZ_WARN(ERROR_DATA_VALUE);
    return 0;
  }

  Stack *substack = Crop_Stack(stack, range->first_corner[0] + c[0], 
			       range->first_corner[1] + c[1], 
			       range->first_corner[2] + c[2],
			       range->size[0], range->size[1], range->size[2],
			       NULL);

  return substack;  
}


double Local_Neuroseg_Score_P(const Local_Neuroseg *locseg, const Stack *stack, 
			      double z_scale, Stack_Fit_Score *fs)
{
  double score = 0.0;

  if ((locseg->seg.r1 > 0.0) && (locseg->seg.scale > 0.0)) {
    Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, NULL, NULL);
  //double norm_factor = sqrt(sqrt(locseg->seg.h)) / locseg->seg.h;
  //double norm_factor = 1.0;//sqrt(sqrt((locseg->seg.r1 + locseg->seg.r2) / 2.0 + 10.0));
    score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

    Kill_Geo3d_Scalar_Field(field);
  }
  
  return score;  
}

double Local_Neuroseg_Score_Pm(const Local_Neuroseg *locseg, 
			       const Stack *stack, double z_scale, 
			       const Stack *mask, Stack_Fit_Score *fs)
{
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg,
						     NULL, NULL);
  //double norm_factor = sqrt(sqrt(locseg->seg.h)) / locseg->seg.h;
  //double norm_factor = 1.0;//sqrt(sqrt((locseg->seg.r1 + locseg->seg.r2) / 2.0 + 10.0));
  double score = Geo3d_Scalar_Field_Stack_Score_M(field, stack, z_scale, mask,
						  fs);

  Kill_Geo3d_Scalar_Field(field);
  
  return score; // / sqrt(sqrt(NEUROSEG_CRC(&(locseg->seg))));  
}

double Local_Neuroseg_Score_W(const Local_Neuroseg *locseg, const Stack *stack, 
			      double z_scale, 
			      Locseg_Score_Workspace *ws)
{
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg,
						     ws->field_func, NULL);
  double score = 0.0;
  if (ws->mask == NULL) {
    score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &(ws->fs));
  } else {
    score = Geo3d_Scalar_Field_Stack_Score_M(field, stack, z_scale, ws->mask,
					     &(ws->fs));
  }

  Kill_Geo3d_Scalar_Field(field);
  
  return score;  
}

void Local_Neuroseg_Draw_Stack(Local_Neuroseg *seg, Stack *stack,
			       const Stack_Draw_Workspace *ws) 
{
  int color  = 1;
  if (ws->color_mode == 1) {
    if ((ws->color.r == 255) && (ws->color.g == 0) && (ws->color.b == 0)) {
      color = 0;
    } else if ((ws->color.r == 0) && (ws->color.g == 0) && (ws->color.b == 255)) {
      color = 2;
    }
  }

  Local_Neuroseg_Label(seg, stack, color, ws->z_scale);
}

void Local_Neuroseg_Label(const Local_Neuroseg *seg, Stack *stack, int color,
			  double z_scale)
{
  if (stack == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null stack.");
    THROW(ERROR_POINTER_NULL);
  }

  if (seg == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null local segment.");
    THROW(ERROR_POINTER_NULL);
  }
  
  if((stack->kind != COLOR) && (color >= 0)) {
    THROW(ERROR_DATA_TYPE);
  }

  if ((seg->seg.r1 == 0.0) || (seg->seg.scale == 0.0)){
    return;
  }

  double bottom_position[3];

  Local_Neuroseg_Bottom(seg, bottom_position);

  double offpos[3];
  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);

#if 0
  printf("c: %d, %d, %d\n", c[0], c[1], c[2]);
  printf("offpos: %g, %g, %g\n", offpos[0], offpos[1], offpos[2]);
#endif

  Field_Range range;
  Neuroseg_Field_Range(&(seg->seg), &range, z_scale);

#if 0
  print_field_range(&range);
#endif

  double *filter = Neurofilter(&(seg->seg), NULL, NULL, 
			       &range, offpos, z_scale);

  int i;
  int region_corner[3];
  
  for (i = 0; i < 3; i++) {
    region_corner[i] = range.first_corner[i] + c[i];
  }

#ifdef _DEBUG_2
  printf("corner: %d, %d, %d\n", 
	 region_corner[0], region_corner[1], region_corner[2]);
  printf("size: %d, %d, %d\n", range.size[0], range.size[1], range.size[2]);
#endif

  Stack *filter_stack = Scale_Double_Stack(filter, range.size[0], 
					   range.size[1], 
					   range.size[2], GREY);

#ifdef _DEBUG_2
  Write_Stack("../data/test2.tif", filter_stack);
#endif

  int point[3];
  int offset = 0;
  //uint8 pixel[3];
  
  int j, k;
  for (k = 0; k < filter_stack->depth; k++) {
    point[2] = region_corner[2] + k;
    for (j = 0; j < filter_stack->height; j++) {
      point[1] = region_corner[1] + j;
      for (i = 0; i < filter_stack->width; i++) {
	point[0] = region_corner[0] + i;
#ifdef _DEBUG_2
	printf("points: %d, %d, %d %g\n", point[0], point[1], point[2],
	       filter[offset]);
#endif
	if ((point[0] >= 0) && (point[0] < stack->width) &&
	    (point[1] >= 0) && (point[1] < stack->height) &&
	    (point[2] >= 0) && (point[2] < stack->depth) &&
	    (filter[offset] > 0)) {
#if 1
	  Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 0);

	  if (stack->kind == COLOR) {
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], 1, 0);
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], 2, 0);

	    if (color >= 0) {
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], color,
			      filter_stack->array[offset]);
	    }
	  }
#endif

#if 0
	  pixel[0] = Get_Stack_Pixel(stack, point[0], point[1], point[2], 0);
	  if (color >= 0) {
	    pixel[1] = Get_Stack_Pixel(stack, point[0], point[1], point[2], 1);
	    pixel[2] = Get_Stack_Pixel(stack, point[0], point[1], point[2], 2);
	  } else {
	    pixel[1] = pixel[0];
	    pixel[2] = pixel[0];
	  }

	  if ((pixel[0] == pixel[1]) && (pixel[1] == pixel[2])) {
	    /* tz- 19-Jan-2008 */
	    //Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 0);
	    /* tz-- */

	    if (color >= 0) {
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 0);
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], 1, 0);
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], 2, 0);
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], color,
			      filter_stack->array[offset]);
	    } else { /* erase */
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 0);
	    }
	  } else { /* label labeled voxels */
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 0);
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], 1, 0);
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], 2, 0);
	    Set_Stack_Pixel(stack, point[0], point[1], point[2], color,
			      filter_stack->array[offset]);
	  }
#endif
	}
	offset++;
      }
    }
  }     
  
  free(filter);
  Kill_Stack(filter_stack);  
} 

BOOL Local_Neuroseg_Has_Stack_Value(const Local_Neuroseg *seg, Stack *stack,
				    double z_scale, double value)
{
  if (stack == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null stack.");
    THROW(ERROR_POINTER_NULL);
  }

  if (seg == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null local segment.");
    THROW(ERROR_POINTER_NULL);
  }
  
  if(stack->kind == COLOR) {
    THROW(ERROR_DATA_TYPE);
  }

  if ((seg->seg.r1 == 0.0) || (seg->seg.scale == 0.0)){
    return FALSE;
  }

  double bottom_position[3];

  Local_Neuroseg_Bottom(seg, bottom_position);

  double offpos[3];
  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);

  Field_Range range;
  Neuroseg_Field_Range(&(seg->seg), &range, z_scale);
  double *filter = Neurofilter(&(seg->seg), NULL, NULL, 
			       &range, offpos, z_scale);

  int i;
  int region_corner[3];
  
  for (i = 0; i < 3; i++) {
    region_corner[i] = range.first_corner[i] + c[i];
  }

  int point[3];
  int offset = 0;
  int j, k;

  for (k = 0; k < range.size[2]; k++) {
    point[2] = region_corner[2] + k;
    for (j = 0; j < range.size[1]; j++) {
      point[1] = region_corner[1] + j;
      for (i = 0; i < range.size[0]; i++) {
	point[0] = region_corner[0] + i;
	if ((point[0] >= 0) && (point[0] < stack->width) &&
	    (point[1] >= 0) && (point[1] < stack->height) &&
	    (point[2] >= 0) && (point[2] < stack->depth)) {
	  if (filter[offset] > 0) {
	    if (Get_Stack_Pixel(stack, point[0], point[1], point[2], 0) == 
		value) {
	      free(filter);
	      return TRUE;
	    }
	  }
	}
	offset++;
      }
    }
  }     
  
  free(filter);  
  return FALSE;
}

/*
 * flag >= 0: only overwrite pixels with value flag
 * flag == -1: no constraint
 */
void Local_Neuroseg_Label_G(const Local_Neuroseg *seg, Stack *stack, 
			    int flag, int value, double z_scale)
{
  if (stack == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null stack.");
    THROW(ERROR_POINTER_NULL);
  }

  if (seg == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null local segment.");
    THROW(ERROR_POINTER_NULL);
  }
  
  if(stack->kind == COLOR) {
    THROW(ERROR_DATA_TYPE);
  }

  if ((seg->seg.r1 == 0.0) || (seg->seg.scale == 0.0)){
    return;
  }

  double bottom_position[3];

  Local_Neuroseg_Bottom(seg, bottom_position);

  double offpos[3];
  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);

  Field_Range range;
  Neuroseg_Field_Range(&(seg->seg), &range, z_scale);
  double *filter = Neurofilter(&(seg->seg), NULL, NULL, 
			       &range, offpos, z_scale);

  int i;
  int region_corner[3];
  
  for (i = 0; i < 3; i++) {
    region_corner[i] = range.first_corner[i] + c[i];
  }

  /*
  Stack *filter_stack = Scale_Double_Stack(filter, range.size[0], 
					   range.size[1], 
					   range.size[2], GREY);
  */
  int point[3];
  int offset = 0;
  //uint8 pixel[3];
  
  int j, k;
  BOOL label;

  for (k = 0; k < range.size[2]; k++) {
    point[2] = region_corner[2] + k;
    for (j = 0; j < range.size[1]; j++) {
      point[1] = region_corner[1] + j;
      for (i = 0; i < range.size[0]; i++) {
	point[0] = region_corner[0] + i;
	if ((point[0] >= 0) && (point[0] < stack->width) &&
	    (point[1] >= 0) && (point[1] < stack->height) &&
	    (point[2] >= 0) && (point[2] < stack->depth)) {
	  if (filter[offset] > 0) {
	    label = TRUE;
	    if (flag >= 0) {
	      if (iround(Get_Stack_Pixel
			 (stack, point[0], point[1], point[2], 0)) != flag) {
		label = FALSE;
	      }
	    }
	    
	    if (label == TRUE) {
	      Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, value);
	    }
	  }
	}
	offset++;
      }
    }
  }     
  
  free(filter);
  //Kill_Stack(filter_stack);  
} 

void Local_Neuroseg_Label_C(const Local_Neuroseg *seg, Stack *stack, 
			    double z_scale, const color_t color)
{
  if (stack == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null stack.");
    THROW(ERROR_POINTER_NULL);
  }

  if (seg == NULL) {
    PRINT_EXCEPTION("Null pointer", "Null local segment.");
    THROW(ERROR_POINTER_NULL);
  }
  
  /*
  if((stack->kind != COLOR)) {
    THROW(ERROR_DATA_TYPE);
  }
  */

  double bottom_position[3];

  Local_Neuroseg_Bottom(seg, bottom_position);

  double offpos[3];
  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);


  Field_Range range;
  Neuroseg_Field_Range(&(seg->seg), &range, z_scale);

  double *filter = Neurofilter(&(seg->seg), NULL, NULL, 
			       &range, offpos, z_scale);

  int i;
  int region_corner[3];
  
  for (i = 0; i < 3; i++) {
    region_corner[i] = range.first_corner[i] + c[i];
  }

  Stack *filter_stack = Scale_Double_Stack(filter, range.size[0], 
					   range.size[1], 
					   range.size[2], GREY);
  int point[3];
  int offset = 0;
  //uint8 pixel[3];
  
  int j, k;
  for (k = 0; k < filter_stack->depth; k++) {
    point[2] = region_corner[2] + k;
    for (j = 0; j < filter_stack->height; j++) {
      point[1] = region_corner[1] + j;
      for (i = 0; i < filter_stack->width; i++) {
	point[0] = region_corner[0] + i;
	if ((point[0] >= 0) && (point[0] < stack->width) &&
	    (point[1] >= 0) && (point[1] < stack->height) &&
	    (point[2] >= 0) && (point[2] < stack->depth) &&
	    (filter[offset] > 0)) {
	  Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, color[0]);
          if (Stack_Kind(stack) == COLOR) {
            Set_Stack_Pixel(stack, point[0], point[1], point[2], 1, color[1]);
            Set_Stack_Pixel(stack, point[0], point[1], point[2], 2, color[2]);
          }
	}
	offset++;
      }
    }
  }     
  
  free(filter);
  Kill_Stack(filter_stack);    
}

void Local_Neuroseg_Label_W(const Local_Neuroseg *seg, Stack *stack, 
			    double z_scale, Locseg_Label_Workspace *ws)
{
  double bottom_position[3];

  Local_Neuroseg_Bottom(seg, bottom_position);

  double offpos[3];
  int c[3];          /* position of the original point in filter range */

  local_neuroseg_stack_position(bottom_position, c, offpos, z_scale);

  Local_Neuroseg *label_locseg = Copy_Local_Neuroseg(seg);
  
  //  if (ws->option == 1) {
  if (ws->option == 10) {
    label_locseg->seg.r1 = 0.75;
    label_locseg->seg.scale = 1.0;
    label_locseg->seg.c = 0.0;
  } else {
    Neuroseg_Swell(&(label_locseg->seg), ws->sratio, ws->sdiff, ws->slimit);
  }
    //  } else {
    // Neuroseg_Swell(&(label_locseg->seg), ws->sratio, ws->sdiff, 4.0);
    // }

  Field_Range range;
  Neuroseg_Field_Range(&(label_locseg->seg), &range, z_scale);

  double *filter = NULL;

  if (ws->option > 10) {
    filter = Neurofilter(&(label_locseg->seg), NULL, NULL, 
			 &range, offpos, z_scale);
  } else {
    filter = Neuroseg_Dist_Filter(&(label_locseg->seg), NULL, NULL, 
				  &range, offpos, z_scale);    
  }

  int i;
  int region_corner[3];
  
  for (i = 0; i < 3; i++) {
    region_corner[i] = range.first_corner[i] + c[i];
  }

  int offset = 0;
  //uint8 pixel[3];

  double *filter2 = NULL;  
  double thre = 0;
  if ((ws->option >= 2) && (ws->option <= 4)){
    thre = Local_Neuroseg_Average_Weak_Signal(seg, ws->signal, z_scale);
    filter2 =  Neuroseg_Dist_Filter(&(seg->seg), NULL, NULL, 
				    &range, offpos, z_scale);
  }  

  for (i = 0; i < 3; i++) {
    if (ws->range[i] < 0) {
      ws->range[i] = region_corner[i];
    } else {
      if (ws->range[i] > region_corner[i]) {
	ws->range[i] = region_corner[i];
      }
    }
  }

  for (i = 0; i < 3; i++) {
    if (ws->range[i+3] < 0) {
      ws->range[i+3] = region_corner[i] + range.size[i] - 1;
    } else {
      if (ws->range[i+3] < region_corner[i] + range.size[i] - 1) {
	ws->range[i+3] = region_corner[i] + range.size[i] - 1;
      }
    }
  }

  for (i = 0; i < 6; i++) {
    if (ws->range[i] < 0) {
      ws->range[i] = 0;
    }
  }

  if (ws->range[0] >= stack->width) {
    ws->range[0] = stack->width;
  }
  if (ws->range[3] >= stack->width) {
    ws->range[3] = stack->width;
  }

  if (ws->range[1] >= stack->height) {
    ws->range[1] = stack->height;
  }
  if (ws->range[4] >= stack->height) {
    ws->range[4] = stack->height;
  }

  if (ws->range[2] >= stack->depth) {
    ws->range[2] = stack->depth;
  }
  if (ws->range[5] >= stack->depth) {
    ws->range[5] = stack->depth;
  }

  int j, k;
  BOOL label;

  int point[3];
  for (k = 0; k < range.size[2]; k++) {
    point[2] = region_corner[2] + k;
    for (j = 0; j < range.size[1]; j++) {
      point[1] = region_corner[1] + j;
      for (i = 0; i < range.size[0]; i++) {
	point[0] = region_corner[0] + i;
	if ((point[0] >= 0) && (point[0] < stack->width) &&
	    (point[1] >= 0) && (point[1] < stack->height) &&
	    (point[2] >= 0) && (point[2] < stack->depth)) {
	  label = TRUE;
	  if (ws->flag >= 0) {
	    if (iround(Get_Stack_Pixel
		       (stack, point[0], point[1], point[2], 0)) != ws->flag) {
	      label = FALSE;
	    }
	  }

	  if (label == TRUE) {
	    switch (ws->option) {
	    case 1:
	    case 10:
	      if (filter[offset] <= 1) {
		Set_Stack_Pixel(stack, point[0], point[1], point[2], 0,
				ws->value);
	      } 
	      break;
	    case 2:
	      if (filter2[offset] <= 1) {
		Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				ws->value);
	      } else if (filter[offset] <= 1) {		
		if (Get_Stack_Pixel(ws->signal, point[0], point[1], point[2], 0)
		    < thre) {
		  Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				  ws->value);
		}
	      }
	      break;
	    case 3:
	      if (filter2[offset] <= 1) {
		Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				ws->value);
	      } else if (filter[offset] <= 1) {		
		if (Stack_Neighbor_Mean(ws->signal, 18,
					point[0], point[1], point[2]) < thre) {
		  Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				  ws->value);
		}
	      }
	      break;
	    case 4:
	      if (filter2[offset] <= 1) {
		Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				ws->value);
	      } else if (filter[offset] <= 1) {		
		if (Stack_Neighbor_Min(ws->signal, 18,
				       point[0], point[1], point[2]) < thre) {
		  Set_Stack_Pixel(stack, point[0], point[1], point[2], 0, 
				  ws->value);
		}
	      }
	      break;
	    }
	  }
	}
	offset++;
      }
    }
  }     

  Delete_Local_Neuroseg(label_locseg);
  if (filter2 != NULL) {
    free(filter2);
  }
  free(filter);
}

/* Local_Neuroseg_Var_Mask_To_Index(): Initialize variable indices from a bit 
 *                                     mask.
 *
 * Args: neuroseg_mask - the variable bit mask of neuron segment;
 *       neuropos_mask - the position bit mask;
 *       index - an array to store the resulted variable indices. It must 
 *               be long enough.
 *
 * Return: the number of variables selected.
 */
int Local_Neuroseg_Var_Mask_To_Index(Bitmask_t neuroseg_mask, 
				     Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Neuroseg_Var_Mask_To_Index(neuroseg_mask, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
  
  int i;
  for (i = 0; i < nvar2; i++) {
    index[nvar1 + i] += NEUROSEG_NPARAM;
  }

  return nvar1 + nvar2;
}

int Local_Neuroseg_Var_Mask_To_Index_R(Bitmask_t neuroseg_mask, 
				       Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Neuroseg_Var_Mask_To_Index(neuroseg_mask | 
					 NEUROSEG_VAR_MASK_R1, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
 

  return nvar1 + nvar2;
}

/* Local_Neuroseg_Var(): Extract variables of a local neuron segment.
 *
 * Args: locseg - the input neuron segment;
 *       var - the extracted variables. Each element of var is the address of
 *             a variable of the neuron segment. So if the value of an element
 *             is changed, <locseg> will be changed as well.
 *       
 * Return: the number of variables.
 */
int Local_Neuroseg_Var(const Local_Neuroseg *locseg, double *var[])
{  
  int nvar = Neuroseg_Var(&(locseg->seg), var);
  Neuropos_Var(locseg->pos, var + nvar);

  return LOCAL_NEUROSEG_NPARAM;
}

void Local_Neuroseg_Set_Var(Local_Neuroseg *locseg, int var_index, double value)
{
  double *var[LOCAL_NEUROSEG_NPARAM];
  Local_Neuroseg_Var(locseg, var);
  *(var[var_index]) = value;
}

void Local_Neuroseg_Set_Var_W(Local_Neuroseg *seg, int var_index, double value,
			      double weight)
{
  double *var[LOCAL_NEUROSEG_NPARAM];
  Local_Neuroseg_Var(seg, var);
  *(var[var_index]) = value * weight;
}

void Local_Neuroseg_Validate(double *var, const double *var_min, 
			     const double *var_max, const void *param)
{
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    if (var[i] < var_min[i]) {
      var[i] = var_min[i];
    } else if (var[i] > var_max[i]) {
      var[i] = var_max[i];
    }
  }
  /* Note: this is coupled with Neuroseg_Var! */
  if (var[1] < 0.0) {
    var[1] = dmax2(var[1], (NEUROSEG_MIN_R - var[0]) / var[4]);
  }
}

double Fit_Local_Neuroseg_Pr_M(Local_Neuroseg *locseg, const Stack *stack, 
			       const int *var_index, int nvar, int *var_link,
			       double *var_min, double *var_max,
			       double z_scale, const Stack *mask,
			       Neuroseg_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  
  double *var = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  //Local_Neuroseg_Param_Array_W(locseg, z_scale, weight, var);
  Local_Neuroseg_Param_Array(locseg, z_scale, var);

  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.vs->link = var_link;
  perceptor.min_gradient = 1e-3;

  //double delta[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  double *delta = (double *) Delta;
  perceptor.delta = delta;

  int i;
  for (i = 0; i < nvar; i++) {
    weight[i] = delta[var_index[i]];
  }
  double wl = darray_norm(weight, nvar);
  
  for (i = 0; i < nvar; i++) {
    weight[i] /= wl;
  }
  perceptor.weight = weight;

  

  /*
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    var_min[i] /= weight[i];
    var_max[i] /= weight[i];
  }
  */

  perceptor.s = 
    Make_Continuous_Function(Local_Neuroseg_Score_Gvm, Local_Neuroseg_Validate,
			     var_min, var_max);

  Stack param[2];
  param[0] = *stack;
  param[1] = *mask;

  Fit_Perceptor(&perceptor, param);

  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    //Local_Neuroseg_Set_Var_W(locseg, i, perceptor.vs->var[i], weight[i]);
    Local_Neuroseg_Set_Var(locseg, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  return Local_Neuroseg_Score_Pm(locseg, stack, z_scale, mask, fs);
}


double Fit_Local_Neuroseg_Pr(Local_Neuroseg *locseg, const Stack *stack, 
			     const int *var_index, int nvar, int *var_link,
			     double *var_min, double *var_max,
			     double z_scale, Neuroseg_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  
  double *var = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  //Local_Neuroseg_Param_Array_W(locseg, z_scale, weight, var);
  Local_Neuroseg_Param_Array(locseg, z_scale, var);

  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.vs->link = var_link;
  perceptor.min_gradient = 1e-3;

  perceptor.arg = New_Locseg_Score_Workspace();

  //double delta[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  double *delta = (double *) Delta;
  perceptor.delta = delta;

  int i;
  for (i = 0; i < nvar; i++) {
    weight[i] = delta[var_index[i]];
  }
  double wl = darray_norm(weight, nvar);
  
  for (i = 0; i < nvar; i++) {
    weight[i] /= wl;
  }
  perceptor.weight = weight;

  

  /*
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    var_min[i] /= weight[i];
    var_max[i] /= weight[i];
  }
  */

  perceptor.s = 
    Make_Continuous_Function(Local_Neuroseg_Score_R, Local_Neuroseg_Validate,
			     var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    //Local_Neuroseg_Set_Var_W(locseg, i, perceptor.vs->var[i], weight[i]);
    Local_Neuroseg_Set_Var(locseg, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  locseg->seg.theta = Normalize_Radian(locseg->seg.theta);
  locseg->seg.psi = Normalize_Radian(locseg->seg.psi);
  
  Kill_Locseg_Score_Workspace((Receptor_Score_Workspace*)perceptor.arg);

  return Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
}

double Fit_Local_Neuroseg_P(Local_Neuroseg *locseg, const Stack *stack, 
			    const int *var_index, int nvar, int *var_link,
			    double z_scale, Neuroseg_Fit_Score *fs)
{
  DECLARE_LOCAL_NEUROSEG_VAR_MIN(var_min);
  DECLARE_LOCAL_NEUROSEG_VAR_MAX(var_max);
  return Fit_Local_Neuroseg_Pr(locseg, stack, var_index, nvar, var_link, 
			       var_min, var_max, z_scale, fs);
}

double Fit_Local_Neuroseg_Pm(Local_Neuroseg *locseg, const Stack *stack, 
			     const int *var_index, int nvar, int *var_link,
			     double z_scale, const Stack *mask,
			     Neuroseg_Fit_Score *fs)
{
  DECLARE_LOCAL_NEUROSEG_VAR_MIN(var_min);
  DECLARE_LOCAL_NEUROSEG_VAR_MAX(var_max);
  return Fit_Local_Neuroseg_Pr_M(locseg, stack, var_index, nvar, var_link, 
				 var_min, var_max, z_scale, mask, fs);
}

double Fit_Local_Neuroseg_W(Local_Neuroseg *locseg, const Stack *stack,
			    double z_scale, Locseg_Fit_Workspace *ws)
{

    if ((stack == NULL) || (locseg == NULL)) {
      THROW(ERROR_POINTER_NULL);
    }

    double *weight = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);
  
    double *var = darray_malloc(LOCAL_NEUROSEG_NPARAM + 1);

    Local_Neuroseg_Param_Array(locseg, z_scale, var);

    Perceptor perceptor;
    perceptor.vs = New_Variable_Set();
    perceptor.vs->nvar = ws->nvar;
    perceptor.vs->var_index = (int *) ws->var_index;
    perceptor.vs->var = var;
    perceptor.vs->link = ws->var_link;
    perceptor.min_gradient = 1e-3;
    
    perceptor.arg = ws->sws;

    double *delta = (double *) Delta;
    perceptor.delta = delta;

    int i;
    for (i = 0; i < perceptor.vs->nvar; i++) {
      weight[i] = delta[perceptor.vs->var_index[i]];
    }
    double wl = darray_norm(weight, perceptor.vs->nvar);
  
    for (i = 0; i < perceptor.vs->nvar; i++) {
      weight[i] /= wl;
    }
    perceptor.weight = weight;

    perceptor.s = 
      Make_Continuous_Function(Local_Neuroseg_Score_R, Local_Neuroseg_Validate,
			       ws->var_min, ws->var_max);

    Fit_Perceptor(&perceptor, stack);

    for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
      Local_Neuroseg_Set_Var(locseg, i, perceptor.vs->var[i]);
    }

    Delete_Variable_Set(perceptor.vs);
    free(var);
    free(weight);
    Free_Continuous_Function(perceptor.s);

    locseg->seg.theta = Normalize_Radian(locseg->seg.theta);
    locseg->seg.psi = Normalize_Radian(locseg->seg.psi);
  
    return Local_Neuroseg_Score_W(locseg, stack, z_scale, ws->sws);
}

double Local_Neuroseg_Orientation_Search(Local_Neuroseg *locseg, 
					 const Stack *stack, double z_scale,
					 Neuroseg_Fit_Score *fs)
{
  double best_theta, best_psi;
  double best_score;
  double theta, psi;
  double score;
  double step;
  Local_Neuroseg *tmp_locseg = New_Local_Neuroseg(); 

#ifdef _TEST_
  double test_scores[2000];
  double test_thetas[2000];
  double test_psis[2000];
  int test_length = 0;
  //locseg->seg.r1 = 1.0;
  //locseg->seg.r2 = 1.0;
#endif

#ifdef _TEST_2
  Stack *canvas = Translate_Stack((Stack *)stack, COLOR, 0);
  Local_Neuroseg_Label(locseg, canvas, 1, z_scale);
#endif
  
  best_score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs) * 2.0;
  best_theta = locseg->seg.theta;
  best_psi = locseg->seg.psi;

#ifdef _TEST_
  test_scores[test_length] =best_score;
  test_thetas[test_length] = best_theta;
  test_psis[test_length] = best_psi;
  test_length++;
#endif

  Local_Neuroseg_Copy(tmp_locseg, locseg);

#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION,
				     NEUROSEG_VAR_MASK_NONE, var_index);
#endif

  double theta_range = TZ_PI * 0.5;
  xz_orientation_t org_ort;
  xz_orientation_t new_ort;
  Set_Xz_Orientation(org_ort, 0.0, 0.0);

  for (theta = 0.1; theta <= theta_range; theta += 0.1) {
    if (theta < 0.005) {
      step = TZ_2PI * 2;
    } else {
      step = 1.0 / locseg->seg.h / sin(theta);
    }
     
    for (psi = 0.0; psi < TZ_2PI; psi += step) {
      tmp_locseg->seg.theta = theta;
      tmp_locseg->seg.psi = psi;
      Set_Xz_Orientation(new_ort, theta, psi);
      Geo3d_Rotate_Orientation(locseg->seg.theta, locseg->seg.psi,
			       &(tmp_locseg->seg.theta), 
			       &(tmp_locseg->seg.psi));   
      
      //score = Fit_Local_Neuroseg_P(tmp_locseg, stack, var_index, nvar,
      //			   z_scale, fs);

      score = Local_Neuroseg_Score_P(tmp_locseg, stack, z_scale, fs);

      double scale = Xz_Orientation_Dot(new_ort, org_ort);
      ASSERT(scale >= 0.0, "Bug found.");
      score *= (scale + 1.0);

#ifdef _TEST_
      test_scores[test_length] =score;
      test_thetas[test_length] = tmp_locseg->seg.theta;
      test_psis[test_length] = tmp_locseg->seg.psi;
      test_length++;
#endif

#ifdef _TEST_2
      Local_Neuroseg_Label(tmp_locseg, canvas, 0, z_scale);  
#endif

      if (score > best_score) {
	best_theta = tmp_locseg->seg.theta;
	best_psi = tmp_locseg->seg.psi;
	best_score = score;
	//printf("(%g, %g) (%g, %g)\n", best_theta, best_psi, theta, psi);
      }
    }

  }


  Print_Local_Neuroseg(locseg);
  Print_Local_Neuroseg(tmp_locseg);

  locseg->seg.theta = best_theta;
  locseg->seg.psi = best_psi;
  

#ifdef _TEST_2   
  Local_Neuroseg_Label(locseg, canvas, 0, z_scale);  
#endif

#ifdef _TEST_
  darray_write("../data/test_scores.bn", test_scores, test_length);
  darray_write("../data/test_thetas.bn", test_thetas, test_length);
  darray_write("../data/test_psis.bn", test_psis, test_length);
  printf("Number of angles searched: %d\n", test_length);
#endif

#ifdef _TEST_2
  Write_Stack("../data/test2.tif", canvas);
  Kill_Stack(canvas);
#endif

  Delete_Local_Neuroseg(tmp_locseg);

  return best_score;
}

double Local_Neuroseg_Orientation_Search_C(Local_Neuroseg *locseg, 
					   const Stack *stack, double z_scale,
					   Neuroseg_Fit_Score *fs)
{
  double best_theta, best_psi;
  double best_score;
  double theta, psi;
  double score;
  double step;
  Local_Neuroseg *tmp_locseg = New_Local_Neuroseg(); 

  double center[3];
  Local_Neuroseg_Center(locseg, center);

  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_Sp(locseg, NULL, NULL);

  best_score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);
  best_theta = locseg->seg.theta;
  best_psi = locseg->seg.psi;

  Local_Neuroseg_Copy(tmp_locseg, locseg);

  double theta_range = TZ_PI * 0.75;

  for (theta = 0.1; theta <= theta_range; theta += 0.2) {
    step = 2.0 / locseg->seg.h / sin(theta);

    for (psi = 0.0; psi < TZ_2PI; psi += step) {
      tmp_locseg->seg.theta = theta;
      tmp_locseg->seg.psi = psi;

      Geo3d_Rotate_Orientation(locseg->seg.theta, locseg->seg.psi,
			       &(tmp_locseg->seg.theta), 
			       &(tmp_locseg->seg.psi));   
      
      Set_Neuroseg_Position(tmp_locseg, center, NEUROSEG_CENTER);
      Local_Neuroseg_Field_Sp(tmp_locseg, NULL, field);

      score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

      if (score > best_score) {
	best_theta = tmp_locseg->seg.theta;
	best_psi = tmp_locseg->seg.psi;
	best_score = score;
      }
    }
  }

  locseg->seg.theta = best_theta;
  locseg->seg.psi = best_psi;
  Set_Neuroseg_Position(locseg, center, NEUROSEG_CENTER);

  Delete_Local_Neuroseg(tmp_locseg);
  Kill_Geo3d_Scalar_Field(field);

  return best_score;
}

double Local_Neuroseg_Orientation_Search_B(Local_Neuroseg *locseg, 
					   const Stack *stack, double z_scale,
					   Neuroseg_Fit_Score *fs)
{
  double best_theta, best_psi;
  double best_score;
  double theta, psi;
  double score;
  double step;
  Local_Neuroseg *tmp_locseg = New_Local_Neuroseg(); 

  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_Sp(locseg, NULL, NULL);

  best_score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);// * 3.0;
  //printf("best score: %g\n", best_score);

  best_theta = locseg->seg.theta;
  best_psi = locseg->seg.psi;

  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);

  Local_Neuroseg_Copy(tmp_locseg, locseg);
  /*
  tmp_locseg->seg.alpha = 0.0;
  tmp_locseg->seg.r1 = (tmp_locseg->seg.r1 * tmp_locseg->seg.scale 
			+ tmp_locseg->seg.r2) / 2.0;
  tmp_locseg->seg.r2 = tmp_locseg->seg.r1;
  tmp_locseg->seg.scale = 1.0;
  */
  double theta_range = TZ_PI_2;// * 0.75;

  for (theta = 0.1; theta <= theta_range; theta += 0.1) {
    step = 2.0 / locseg->seg.h / sin(theta);

    for (psi = 0.0; psi < TZ_2PI; psi += step) {
      tmp_locseg->seg.theta = theta;
      tmp_locseg->seg.psi = psi;

      Geo3d_Rotate_Orientation(locseg->seg.theta, locseg->seg.psi,
			       &(tmp_locseg->seg.theta), 
			       &(tmp_locseg->seg.psi));   
      Set_Neuroseg_Position(tmp_locseg, bottom, NEUROSEG_BOTTOM); 
      Local_Neuroseg_Field_Sp(tmp_locseg, NULL, field);

      score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);// * (2.0 + cos(theta));

      if (score > best_score) {
	best_theta = tmp_locseg->seg.theta;
	best_psi = tmp_locseg->seg.psi;
	best_score = score;
      }
    }
  }

  locseg->seg.theta = best_theta;
  locseg->seg.psi = best_psi;
  Set_Neuroseg_Position(locseg, bottom, NEUROSEG_BOTTOM); 

  Delete_Local_Neuroseg(tmp_locseg);
  Kill_Geo3d_Scalar_Field(field);

  //printf("best score: %g\n", best_score);

  return best_score;
}

void Local_Neuroseg_Orientation_Adjust(Local_Neuroseg *locseg, 
				       const Stack *stack, double z_scale)
{
/* alloc <Geo3d_Scalar_Field> */
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, NULL, NULL);
  
  Stack_Points_Sampling(stack, Coordinate_3d_Double_Array(field->points), 
			field->size, field->values);

  double vec[3], ext[3];
  Geo3d_Scalar_Field_Ort(field, vec, ext);

  double ort[3];
  Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi, 
			   ort, ort + 1, ort + 2);

  double angle = 
    Geo3d_Dot_Product(vec[0], vec[1], vec[2], ort[0], ort[1], ort[2]);

  Geo3d_Normal_Orientation(vec[0], vec[1], vec[2], &(locseg->seg.theta), 
			   &(locseg->seg.psi));

  if (angle < 0.0) {
    Flip_Local_Neuroseg(locseg);
  }

  /* free <Geo3d_Scalar_Field> */
  Kill_Geo3d_Scalar_Field(field);
}

double Local_Neuroseg_Position_Search(Local_Neuroseg *locseg, 
				      const Stack *stack, double z_scale,
				      double v[3], double range, double step,
				      Neuroseg_Fit_Score *fs)
{
  double best_pos[3];
  double best_score;
  double score;

  Get_Neuroseg_Position(locseg, best_pos);

  best_score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);

  double t = 0.0;
  double offset[3];
  offset[0] = v[0] * step;
  offset[1] = v[1] * step;
  offset[2] = v[2] * step;

  for (t = 0.0; t <= range; t += fabs(step)) {
    Neuropos_Translate(locseg->pos, offset[0], offset[1], offset[2]);
    score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
    if (score > best_score) {
      best_score = score;
      Get_Neuroseg_Position(locseg, best_pos);
    }
  }

  Set_Neuropos(locseg->pos, best_pos[0], best_pos[1], best_pos[2]);

  return best_score;
}

double Local_Neuroseg_Radius_Search(Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale,
				    double start, double end, double step,
				    Neuroseg_Fit_Score *fs)
{
  double best_score;
  double score;

  best_score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
  printf("%g\n", best_score);
  double best_r = locseg->seg.r1;
  
  double r;
  for (r = start; r <= end; r += step) {
    locseg->seg.r1 = r;
    //locseg->seg.r2 = r;
    score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
    printf("%g\n", score);
    if (score > best_score) {
      best_score = score;
      best_r = r;
    }
  }

  locseg->seg.r1 = best_r;
  //locseg->seg.r2 = best_r;

  return best_score;
}

double Local_Neuroseg_Scale_Search(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale,
				   double start, double end, double step,
				   Neuroseg_Fit_Score *fs)
{
  double best_score;
  double score;

  best_score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
  printf("%g\n", best_score);
  double best_s = locseg->seg.scale;
  
  double s;
  for (s = start; s <= end; s += step) {
    locseg->seg.scale = s;
    //locseg->seg.r2 = r;
    score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
    printf("%g\n", score);
    if (score > best_score) {
      best_score = score;
      best_s = s;
    }
  }

  locseg->seg.scale = best_s;

  return best_score;
}

double Local_Neuroseg_R_Scale_Search(Local_Neuroseg *locseg, 
				     const Stack *stack, double z_scale,
				     double r_start, double r_end, 
				     double r_step,
				     double s_start, double s_end, 
				     double s_step,
				     Neuroseg_Fit_Score *fs)
{
  double best_score;
  double score;

  best_score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
  //best_score /= (1.0 + 1.0 / (2.0 + exp(4.0 - NEUROSEG_CRC(&(locseg->seg)))));

  //printf("%g\n", best_score);
  double best_s = locseg->seg.scale;
  double best_r = locseg->seg.r1;

  double r, s;
  for (r = r_start; r <= r_end; r += r_step) {
    for (s = s_start; s <= s_end; s += s_step) {
      if ((r * s > NEUROSEG_MIN_R) && (r * s < 30.0)){
	locseg->seg.r1 = r;
	locseg->seg.scale = s;
	//locseg->seg.r2 = r;
	score = Local_Neuroseg_Score_P(locseg, stack, z_scale, fs);
	//score /= (1.0 + 1.0 / (2.0 + exp(4.0 - NEUROSEG_CRC(&(locseg->seg)))));
	//printf("%g\n", score);
	if (score > best_score) {
	  best_score = score;
	  best_s = s;
	  best_r = r;
	}
      }
    }
  }

  locseg->seg.scale = best_s;
  locseg->seg.r1 = best_r;

  return best_score;
}

void Local_Neuroseg_Position_Adjust(Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale)
{
  /* alloc <field> */
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, NULL, NULL);
    
  Geo3d_Scalar_Field_Stack_Sampling(field, stack, z_scale, 
				    field->values);
  
  coordinate_3d_t center;
  Geo3d_Scalar_Field_Centroid(field, center);
#ifdef _DEBUG_2
  printf("%g, %g, %g\n", center[0], center[1], center[2]);
#endif

  Set_Neuroseg_Position(locseg, center, NEUROSEG_CENTER);
  
  /* free <field> */
  Kill_Geo3d_Scalar_Field(field);
}

int Local_Neuroseg_Height_Search_P(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale)
{
  double old_height = locseg->seg.h;

  int length = iround(locseg->seg.h);

  if (length == 1) {
    return 0;
  }

  /* alloc <profile> */
  double *profile = Local_Neuroseg_Height_Profile(locseg, stack, z_scale,
						  length, STACK_FIT_CORRCOEF,
						  NULL, NULL);

  int index;
  for (index = length - 1; index >= 0; index--) {
    if (profile[index] > 0.5) {
      break;
    }
  }

  Local_Neuroseg_Change_Height(locseg, index + 1);
#if 1  
  length = index + 1;

  if (length > 1) {
    Local_Neuroseg_Height_Profile(locseg, stack, z_scale, length, 
				  STACK_FIT_MEAN_SIGNAL, NULL, profile);
    
    double min = profile[0];
    double max = darray_max(profile + 1, length - 1, NULL);

    if ((max == 0.0) || (min / max > 3.0)){
      index = 0;
    } else {
      for (index = 1; index < length - 1; index++) {
	if (min > profile[index]) {
	  min = profile[index];
	}
	if (max == profile[index]) {
	  max = darray_max(profile + index + 1, length - index - 1, NULL);
	}
	if ((max == 0.0) || (min / max > 3.0)){
	  break;
	}
      }
    }

    if (index < length - 1) {
      Local_Neuroseg_Change_Height(locseg, index + 1);
    }
  }
  /* free <profile> */
  free(profile);
#endif

#if 0
  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);

#ifdef _DEBUG_
  printf("bottom before height search: %g %g %g %g\n", pos[0], pos[1], pos[2],
	 locseg->seg.h);
#endif

  int length = round(locseg->seg.h);
  
  int i;

  Local_Neuroseg_Plane *locnp = New_Local_Neuroseg_Plane();

  Local_Neuroseg_To_Plane(locseg, 0.0, locnp);
  Geo3d_Scalar_Field *field = 
    Local_Neuroseg_Plane_Field(locnp, 0.2 * locnp->np.r, NULL);
 
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = 1;

  double score = 
    Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &fs);
  Kill_Geo3d_Scalar_Field(field);

  double prev_score = score;

  for (i = 1; i < length; i++) {
    //printf("height search: %g\n", score);
    if (score < 0.4) {
      //locseg->seg.r2 = (locseg->seg.r2 - locseg->seg.r1) * i / locseg->seg.h
      //	+ locseg->seg.r1;
      /*
      if (i == 1) {
	locseg->seg.h /= 2;
      } else {
      */
	locseg->seg.h = i;
	/*}*/
      break;
    } else {
      if (score < 0.6) {
	if (prev_score > score + 0.01) {
	  //locseg->seg.r2 = (locseg->seg.r2 - locseg->seg.r1) * i / locseg->seg.h
	  //+ locseg->seg.r1;
	  locseg->seg.h = i;
	  break;
	}
      }
    }
    Local_Neuroseg_To_Plane(locseg, i, locnp);
    field = Local_Neuroseg_Plane_Field(locnp, 0.2 * locnp->np.r, NULL);
    prev_score = score;
    score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &fs);
    Kill_Geo3d_Scalar_Field(field);
  }

  Delete_Local_Neuroseg_Plane(locnp);
  
  Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

#ifdef _DEBUG_2
  Local_Neuroseg_Bottom(locseg, pos);
  printf("bottom after height search: %g %g %g %g\n", pos[0], pos[1], pos[2],
	 locseg->seg.h);
#endif
#endif

  return (old_height != locseg->seg.h);
}

int Local_Neuroseg_Height_Search_W(Local_Neuroseg *locseg, 
				   const Stack *stack, double z_scale,
				   Locseg_Score_Workspace *sws)
{
  int length = iround(locseg->seg.h);

  /* alloc <profile> */
  double *profile = Local_Neuroseg_Height_Profile(locseg, stack, z_scale,
						  length, STACK_FIT_CORRCOEF,
						  sws->field_func,
						  NULL);

  int index;
  for (index = length - 1; index > 0; index--) {
    if (profile[index] > 0.5) {
      break;
    }
  }

  /* free <profile> */
  free(profile);

  Local_Neuroseg_Change_Height(locseg, index + 1);

  return (index + 1 != length);
}

int Local_Neuroseg_Height_Search_E(Local_Neuroseg *locseg, int base,
				   const Stack *stack, double z_scale)
{
  int length = iround(locseg->seg.h);

  /* alloc <profile> */
  double *profile = Local_Neuroseg_Height_Profile(locseg, stack, z_scale,
						  length, STACK_FIT_CORRCOEF,
						  NULL, NULL);

  size_t index;
  //int base = NEUROSEG_DEFAULT_H / 2.0;
  darray_max(profile + base, length - base, &index);

  /* free <profile> */
  free(profile);

  if (index == length - base -1) { /* height does not change */
    return 0;
  } else {
    Local_Neuroseg_Change_Height(locseg, index + base + 1);
  }

  return 1;
}

BOOL Local_Neuroseg_Good_Score(Local_Neuroseg *locseg, double score, 
			       double min_score)
{
  double cal_score = min_score * 
    (1.0 + 1.0 / (2.0 + exp(4.0 - NEUROSEG_CRC(&(locseg->seg)))));

  return score > cal_score;
}

Local_Neuroseg* Next_Local_Neuroseg(const Local_Neuroseg *locseg1, 
				    Local_Neuroseg *locseg2, double pos_step)
{
  if (locseg2 == NULL) {
    locseg2 = New_Local_Neuroseg();
  }

  Next_Neuroseg(&(locseg1->seg), &(locseg2->seg), pos_step);
  
  double bottom[3];
  Local_Neuroseg_Axis_Position(locseg1, bottom, 
			       pos_step * (locseg1->seg.h - 1.0));
  Set_Neuroseg_Position(locseg2, bottom, NEUROSEG_BOTTOM);

  return locseg2;
}

void Flip_Local_Neuroseg(Local_Neuroseg *locseg)
{
  double pos[3];
  switch(Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    Local_Neuroseg_Top(locseg, pos);
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);
    break;
  case NEUROSEG_TOP:
    Local_Neuroseg_Bottom(locseg, pos);
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_TOP);
    break;
  }

  locseg->seg.theta += TZ_PI;

  /*
  double tmpr = locseg->seg.r1;
  locseg->seg.r1 = locseg->seg.r2;
  locseg->seg.r2 = tmpr;
  */

  locseg->seg.r1 = NEUROSEG_R2(&(locseg->seg));
  locseg->seg.c = -locseg->seg.c;
}

void Local_Neuroseg_Fprint_Vrml(const Local_Neuroseg *locseg, FILE *stream)
{
  /* Test if the input is valid */
  if ((locseg == NULL) || (stream == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  /* Start a new line */
  fprintf(stream, "\n");

  /* Print transformation*/
  fprintf(stream, "Transform {\n");

  double cpos[3];
  Local_Neuroseg_Center(locseg, cpos);
  fprintf(stream, "translation %.4f %.4f %.4f\n", cpos[0], cpos[1], cpos[2]);

  fprintf(stream, "rotation 0 0 1 %.4f\n", locseg->seg.psi);
  fprintf(stream, "children [\n");

  fprintf(stream, "Transform {\n");
  fprintf(stream, "rotation 1 0 0 %.4f\n", locseg->seg.theta + TZ_PI_2);

  /*
  double rotate_axis[3];
  rotate_axis[0] = cos(locseg->seg.psi);
  rotate_axis[1] = sin(locseg->seg.psi);
  rotate_axis[2] = 0.0;
  fprintf(stream, "rotation %.4f %.4f %.4f %.4f\n", 
	  rotate_axis[0], rotate_axis[1], rotate_axis[2], locseg->seg.theta);
  */

  /* print shape */
  fprintf(stream, "children [\n");
  fprintf(stream, "Shape {\n");
  fprintf(stream, "appearance Appearance {\n");
  fprintf(stream, "material Material {\n");
  fprintf(stream, "diffuseColor %.4f %.4f %.4f\n", 1.0, 0.0, 0.0);
  fprintf(stream, "}\n");
  fprintf(stream, "}\n");
  fprintf(stream, "geometry Cylinder {\n");
  fprintf(stream, "radius %.4f\n", NEUROSEG_RC(&(locseg->seg)));
  fprintf(stream, "height %.4f\n", (locseg->seg.h));
  fprintf(stream, "}\n");
  fprintf(stream, "}\n");
  fprintf(stream, "]\n");
  fprintf(stream, "}\n");
  fprintf(stream, "]\n");
  fprintf(stream, "}\n");
}

static void local_neuroseg_field_shift(const Local_Neuroseg *locseg, 
				       double *offset)
{
  double pos[2][3];

  switch (Neuropos_Reference) {
  case NEUROSEG_BOTTOM:
    Neuroseg_Bottom(&(locseg->seg), pos[0]);
    Local_Neuroseg_Bottom(locseg, pos[1]);
    break;
  case NEUROSEG_CENTER:
    Neuroseg_Center(&(locseg->seg), pos[0]);
    Local_Neuroseg_Center(locseg, pos[1]);
    break;
  case NEUROSEG_TOP:
    Neuroseg_Top(&(locseg->seg), pos[0]);
    Local_Neuroseg_Top(locseg, pos[1]);
    break;
  }

  offset[0] = pos[1][0] - pos[0][0];
  offset[1] = pos[1][1] - pos[0][1];
  offset[2] = pos[1][2] - pos[0][2];

  //  printf("field shift: %g %g %g\n", offset[0], offset[1], offset[2]); 
}



Geo3d_Scalar_Field* Local_Neuroseg_Field_S(const Local_Neuroseg *locseg, 
					   Neuroseg_Field_f field_func,
					   Geo3d_Scalar_Field *field)
{
  field = Neuroseg_Field_S(&(locseg->seg), field_func, field);

  double offset[3];
  local_neuroseg_field_shift(locseg, offset);
  Geo3d_Point_Array_Translate(field->points, field->size, offset[0],
			      offset[1], offset[2]);

  return field;
}

Geo3d_Scalar_Field* Local_Neuroseg_Field_Sp(const Local_Neuroseg *locseg, 
					    Neuroseg_Field_f field_func,
					    Geo3d_Scalar_Field *field)
{
  field = Neuroseg_Field_Sp(&(locseg->seg), field_func, field);
  double pos[3];
  local_neuroseg_field_shift(locseg, pos);
  Geo3d_Point_Array_Translate(field->points, field->size,
			      pos[0], pos[1], pos[2]);

  return field;
}

Geo3d_Scalar_Field* Local_Neuroseg_Field_Z(const Local_Neuroseg *locseg,
					   double z, double step,
					   Neuroseg_Field_f field_func,
					   Geo3d_Scalar_Field *field)
{
  field = Neuroseg_Field_Z(&(locseg->seg), z, step, field);
  double pos[3];
  local_neuroseg_field_shift(locseg, pos);
  Geo3d_Point_Array_Translate(field->points, field->size,
			      pos[0], pos[1], pos[2]);

  return field;
}

Geo3d_Ball* Local_Neuroseg_Ball_Bound(const Local_Neuroseg *locseg,
				      Geo3d_Ball *ball)
{
  if (ball == NULL) {
#ifdef _MSC_VER
    ball = New_Geo3d_Ball();
#else
    ball = New_Geo3d_Ball(ball);
#endif
  }
  
  Local_Neuroseg_Center(locseg, ball->center);
  ball->r = Neuroseg_Ball_Range(&(locseg->seg)) / 2.0;
  
  return ball;
}

void Local_Neuroseg_From_Geo3d_Circle(Local_Neuroseg *locseg, 
				      const Geo3d_Circle *circle)
{
  locseg->seg.r1 = circle->radius;
  locseg->seg.c = 0.0;
  locseg->seg.h = 1.0;
  locseg->seg.theta = circle->orientation[0];
  locseg->seg.psi = circle->orientation[1];
  locseg->seg.curvature = 0.0;
  Set_Neuroseg_Position(locseg, circle->center, NEUROSEG_CENTER);
}

double* Local_Neuroseg_Height_Profile(const Local_Neuroseg *locseg, 
				      const Stack *stack, double z_scale,
				      int n, int option, 
				      Neuroseg_Field_f field_func,
				      double *profile)
{
  if (profile == NULL) {
    profile = darray_malloc(n);
  }
  
  double step = locseg->seg.h / n;
  int i;
  double z = 0.0;
  Geo3d_Scalar_Field *field = NULL;
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = option;
  for (i = 0; i < n; i++) {
    field = Local_Neuroseg_Field_Z(locseg, z, step, field_func, field);
    profile[i] = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &fs);
    z += step;
  }

  Kill_Geo3d_Scalar_Field(field);

  return profile;
}

int Local_Neuroseg_Param_Array(const Local_Neuroseg *locseg, double z_scale, 
			       double *param)
{
  int i;
  double *var[LOCAL_NEUROSEG_NPARAM];
  Local_Neuroseg_Var(locseg, var);
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    param[i] = *(var[i]);
  }

  param[LOCAL_NEUROSEG_NPARAM] = z_scale;
  
  return LOCAL_NEUROSEG_NPARAM + 1;
}

int Local_Neuroseg_Param_Array_W(const Local_Neuroseg *seg, double z_scale, 
				 const double *weight, double *param)
{
  int i;
  double *var[LOCAL_NEUROSEG_NPARAM];
  Local_Neuroseg_Var(seg, var);
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    param[i] = *(var[i]) / weight[i];
  }

  param[LOCAL_NEUROSEG_NPARAM] = z_scale;
  
  return LOCAL_NEUROSEG_NPARAM + 1;
}

double Local_Neuroseg_Score_G(const double *var, const Stack *stack)
{
  Local_Neuroseg seg;// = New_Local_Neuroseg();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    Local_Neuroseg_Set_Var(&seg, i, var[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_NPARAM];

  double score = Local_Neuroseg_Score_P(&seg, stack, z_scale, NULL);

  //Delete_Local_Neuroseg(seg);

  return score;
}

double Local_Neuroseg_Score_Gw(const double *var, const Stack *stack,
			       const double *weight)
{
  Local_Neuroseg seg;// = New_Local_Neuroseg();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    Local_Neuroseg_Set_Var_W(&seg, i, var[i], weight[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_NPARAM];

  double score = Local_Neuroseg_Score_P(&seg, stack, z_scale, NULL);

  //Delete_Local_Neuroseg(seg);

  return score;
}

double Local_Neuroseg_Score_Gvw(const double *var, const void *param)
{
  return Local_Neuroseg_Score_Gw(var, (const Stack *) param, Delta);
}

double Local_Neuroseg_Score_Gv(const double *var, const void *param)
{
  return Local_Neuroseg_Score_G(var, (const Stack *) param);
}

double Local_Neuroseg_Score_R(const double *var, const void *param)
{
  void **param_array = (void**) param;
  
  Stack *stack = (Stack *) param_array[0];
  Locseg_Score_Workspace *ws = 
    (Locseg_Score_Workspace *) param_array[1];
  
  Local_Neuroseg seg;// = New_Local_Neuroseg();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    Local_Neuroseg_Set_Var(&seg, i, var[i]);
  }
  
  double z_scale = var[LOCAL_NEUROSEG_NPARAM];
  
  double score = Local_Neuroseg_Score_W(&seg, stack, z_scale, ws);
  
  //Delete_Local_Neuroseg(seg);
  
  return score;
}

double Local_Neuroseg_Score_Gvm(const double *var, const void *param)
{
  Local_Neuroseg *seg = New_Local_Neuroseg();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    Local_Neuroseg_Set_Var(seg, i, var[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_NPARAM];

  Stack *stack = (Stack*) param;
  Stack *mask = stack + 1;

  double score = Local_Neuroseg_Score_Pm(seg, stack, z_scale, mask, NULL);

  Delete_Local_Neuroseg(seg);

  return score;
}

Local_Neuroseg_Plane* Local_Neuroseg_To_Plane(const Local_Neuroseg *locseg,
					      double z,
					      Local_Neuroseg_Plane *locnp)
{
  if (locnp == NULL) {
    locnp = New_Local_Neuroseg_Plane();
  }

  //double coef = (locseg->seg.r2 - locseg->seg.r1) / locseg->seg.h;
  double coef = NEUROSEG_COEF(&(locseg->seg));
  double r = locseg->seg.r1 + z * coef;
  if (r <= 0.0) {
    return NULL;
  } else {
    Set_Neuroseg_Plane(&(locnp->np), r, locseg->seg.theta, locseg->seg.psi,
		       0.0, 0.0);
    Local_Neuroseg_Axis_Position(locseg, locnp->pos, z);
  }

  return locnp;
}

Local_Neuroseg* Local_Neuroseg_From_Plane(Local_Neuroseg *locseg,
					  const Local_Neuroseg_Plane *locnp)
{
  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }
  double center[3];
  Local_Neuroseg_Plane_Center(locnp, center);
  Set_Local_Neuroseg(locseg, locnp->np.r, locnp->np.r, 1.0, 
		     locnp->np.theta, locnp->np.psi,
		     0.0, 0.0, 1.0, center[0], center[1], center[2]);

  return locseg;
}

Geo3d_Circle* Local_Neuroseg_To_Circle(const Local_Neuroseg *locseg,
				       Neuropos_Reference_e ref, int option,
				       Geo3d_Circle *sc) 
{
  if (sc == NULL) {
    sc = New_Geo3d_Circle();
  }

  switch (ref) {
  case NEUROSEG_BOTTOM:
    sc->radius = Neuroseg_Rc_Z(&(locseg->seg), 0.0, option);
    Local_Neuroseg_Bottom(locseg, sc->center);
    break;
  case NEUROSEG_TOP:
    sc->radius = Neuroseg_Rc_Z(&(locseg->seg), 0.5, option);
    Local_Neuroseg_Top(locseg, sc->center);
    break;
  case NEUROSEG_CENTER:
    sc->radius = Neuroseg_Rc_Z(&(locseg->seg), 1.0, option);
    Local_Neuroseg_Center(locseg, sc->center);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  Set_Xz_Orientation(sc->orientation, locseg->seg.theta, locseg->seg.psi); 

  return sc;
}

Geo3d_Circle* 
Local_Neuroseg_To_Circle_Z(const Local_Neuroseg *locseg, 
			   double z, int option, Geo3d_Circle *circle)
{
  if (circle == NULL) {
    circle = New_Geo3d_Circle();
  }

  circle->radius = Neuroseg_Rc_Z(&(locseg->seg), z, option);
  Local_Neuroseg_Axis_Position(locseg, circle->center, z);
  Set_Xz_Orientation(circle->orientation, locseg->seg.theta, locseg->seg.psi);

  /*
  //double coef = (locseg->seg.r2 - locseg->seg.r1) / locseg->seg.h;
  double coef= NEUROSEG_COEF(&(locseg->seg));
  //double r = locseg->seg.r1 + z * coef;
  double r = Neuroseg_Rc_Z(&(locseg->seg), z);

  if (r <= 0.0) {
    return NULL;
  } else {
    circle->radius = NEUROSEG_RADIUS(&(locseg->seg), z) 
      * sqrt(locseg->seg.scale);
    Local_Neuroseg_Axis_Position(locseg, circle->center, z);
  }
  */
  return circle;
}

Geo3d_Circle* 
Local_Neuroseg_To_Circle_T(const Local_Neuroseg *locseg, 
			   double t, int option, Geo3d_Circle *circle)
{
  double z = (locseg->seg.h - 1.0) * t;

  return Local_Neuroseg_To_Circle_Z(locseg, z, option, circle);
}

Local_Neuroseg_Ellipse* 
Local_Neuroseg_To_Ellipse_Z(const Local_Neuroseg *locseg, 
			    double z, Local_Neuroseg_Ellipse *locnp)
{
  if (locnp == NULL) {
    locnp = New_Local_Neuroseg_Ellipse();
  }

  //double coef = (locseg->seg.r2 - locseg->seg.r1) / locseg->seg.h;
  double coef= NEUROSEG_COEF(&(locseg->seg));
  double r = locseg->seg.r1 + z * coef;
  if (r <= 0.0) {
    return NULL;
  } else {
    Set_Neuroseg_Ellipse(&(locnp->np), r * locseg->seg.scale, r, 
			 locseg->seg.theta, locseg->seg.psi,
			 locseg->seg.alpha, 0.0, 0.0);
    Local_Neuroseg_Axis_Position(locseg, locnp->pos, z);
  }

  return locnp;
}

Local_Neuroseg_Ellipse* 
Local_Neuroseg_To_Ellipse_T(const Local_Neuroseg *locseg, 
			    double t, Local_Neuroseg_Ellipse *locnp)
{
  double z = (locseg->seg.h - 1.0) * t;
  
  return Local_Neuroseg_To_Ellipse_Z(locseg, z, locnp);
}

Local_Neuroseg_Ellipse* 
Local_Neuroseg_Central_Ellipse(const Local_Neuroseg *locseg, 
			       Local_Neuroseg_Ellipse *locnp)
{
  return Local_Neuroseg_To_Ellipse_Z(locseg, (locseg->seg.h - 1.0) / 2.0, 
				     locnp);
}

coordinate_3d_t* 
Local_Neuroseg_Halo_Points(const Local_Neuroseg *locseg,
			   int nsample, coordinate_3d_t *pts)
{
  if (pts == NULL) {
    pts = (coordinate_3d_t *) 
      Guarded_Malloc(sizeof(coordinate_3d_t) * nsample,
		     "Local_Neuroseg_Halo_Points");
  }

  Local_Neuroseg_Ellipse *locne = Local_Neuroseg_Central_Ellipse(locseg, NULL);
        
  Local_Neuroseg_Ellipse_Points(locne, nsample, 0, pts);

  Delete_Local_Neuroseg_Ellipse(locne);

  return pts;
}

void Local_Neuroseg_Scale_Z(Local_Neuroseg *locseg, double z_scale)
{
  locseg->pos[2] /= z_scale;
  coordinate_3d_t normal;
  Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi, 
			   normal, normal + 1, normal + 2);
  Neuroseg_Set_Model_Height(&(locseg->seg), 
      Neuroseg_Model_Height(&(locseg->seg)) * 
      sqrt(1.0 + (dsqr(1.0 / z_scale) - 1.0) * dsqr(normal[2])));
		      
  //locseg->seg.h *= sqrt(1.0 + (dsqr(1.0 / z_scale) - 1.0) * dsqr(normal[2]));
  /* cross section stretch for z scale */
  ASSERT(locseg->seg.alpha == 0.0, "Alpha not allowed yet.");
  double factor = sqrt(dsqr(cos(locseg->seg.theta)) + 
		       dsqr(sin(locseg->seg.theta) / z_scale));
  /* make sure that the tube is not destorted by PSF */
  //if (locseg->seg.scale > factor) {
  locseg->seg.r1 *= factor;
    //locseg->seg.r2 = locseg->seg.r2 * factor;
  locseg->seg.scale /= factor;
    //}

  normal[2] /= z_scale;
  Coordinate_3d_Unitize(normal);
  Geo3d_Normal_Orientation(normal[0], normal[1], normal[2], 
			   &(locseg->seg.theta), &(locseg->seg.psi));
}

void Local_Neuroseg_Scale_XY(Local_Neuroseg *locseg, double xy_scale)
{
  locseg->pos[0] *= xy_scale;
  locseg->pos[1] *= xy_scale;

  coordinate_3d_t normal;
  Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi, 
			   normal, normal + 1, normal + 2);

  double s = sqrt(dsqr(normal[0] * xy_scale) + 
		  dsqr(normal[1] * xy_scale) +
		  dsqr(normal[2]));

  Neuroseg_Set_Model_Height(&(locseg->seg), 
      s * Neuroseg_Model_Height(&(locseg->seg)));

  /*
  locseg->seg.h *= sqrt(dsqr(normal[0] * xy_scale) + 
			dsqr(normal[1] * xy_scale) +
			dsqr(normal[2]));
  */

  /* cross section stretch for z scale */
  ASSERT(locseg->seg.alpha == 0.0, "Alpha not allowed yet.");
  double factor = sqrt(dsqr(cos(locseg->seg.theta) * xy_scale) + 
		       dsqr(sin(locseg->seg.theta)));

  locseg->seg.r1 = locseg->seg.r1 * factor;
  locseg->seg.scale *= xy_scale / factor;

  normal[0] *= xy_scale;
  normal[1] *= xy_scale;
  Coordinate_3d_Unitize(normal);
  Geo3d_Normal_Orientation(normal[0], normal[1], normal[2], 
			   &(locseg->seg.theta), &(locseg->seg.psi));
}

void Local_Neuroseg_Scale(Local_Neuroseg *locseg, double xy_scale, 
			  double z_scale)
{
  if ((xy_scale != 1.0) || (z_scale != 1.0)) {
    locseg->pos[0] *= xy_scale;
    locseg->pos[1] *= xy_scale;
    locseg->pos[2] *= z_scale;

    coordinate_3d_t normal;
    Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi, 
			     normal, normal + 1, normal + 2);
    
    double s = sqrt(dsqr(normal[0] * xy_scale) + 
		    dsqr(normal[1] * xy_scale) +
		    dsqr(normal[2] * z_scale));
    
    Neuroseg_Set_Model_Height(&(locseg->seg), 
	s * Neuroseg_Model_Height(&(locseg->seg)));

    /*
    locseg->seg.h *= sqrt(dsqr(normal[0] * xy_scale) + 
			  dsqr(normal[1] * xy_scale) +
			  dsqr(normal[2] * z_scale));
    */
    ASSERT(locseg->seg.alpha == 0.0, "Alpha not allowed yet.");
    double factor = sqrt(dsqr(cos(locseg->seg.theta) * xy_scale) + 
			 dsqr(sin(locseg->seg.theta) * z_scale));

    locseg->seg.r1 *= factor;
    locseg->seg.scale *= xy_scale / factor;

    normal[0] *= xy_scale;
    normal[1] *= xy_scale;
    normal[2] *= z_scale;
    Coordinate_3d_Unitize(normal);
    Geo3d_Normal_Orientation(normal[0], normal[1], normal[2], 
			     &(locseg->seg.theta), &(locseg->seg.psi));  
  }
}

void Local_Neuroseg_Change_Top(Local_Neuroseg *locseg, const double *new_top)
{
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  double axis_vector[3];
  int i;
  for (i = 0; i < 3; i++) {
    axis_vector[i] = new_top[i] - bottom[i];
  }

  double h = Geo3d_Orgdist(axis_vector[0], axis_vector[1], axis_vector[2]);
  if (h < 0.1) {
    locseg->seg.h = 1.0;
    return;
  }

  for (i = 0; i < 3; i++) {
    axis_vector[i] /= h;
  }
  locseg->seg.h = h + 1.0;
  //ASSERT(locseg->seg.h >= 1.0, "invalid height");
  

  Geo3d_Normal_Orientation(axis_vector[0], axis_vector[1], axis_vector[2],
			   &(locseg->seg.theta), &(locseg->seg.psi));

  Set_Neuroseg_Position(locseg, new_top, NEUROSEG_TOP);  
}

void Local_Neuroseg_Change_Bottom(Local_Neuroseg *locseg, 
				  const double *new_bottom)
{
  double top[3];
  Local_Neuroseg_Top(locseg, top);
  double axis_vector[3];
  int i;
  for (i = 0; i < 3; i++) {
    axis_vector[i] = top[i] - new_bottom[i];
  }

  double h = Geo3d_Orgdist(axis_vector[0], axis_vector[1], axis_vector[2]);
  if (h < 0.1) {
    locseg->seg.h = 1.0;
    return;
  }

  for (i = 0; i < 3; i++) {
    axis_vector[i] /= h;
  }
  locseg->seg.h = h + 1.0;

  Geo3d_Normal_Orientation(axis_vector[0], axis_vector[1], axis_vector[2],
			   &(locseg->seg.theta), &(locseg->seg.psi));

  Set_Neuroseg_Position(locseg, new_bottom, NEUROSEG_BOTTOM);  
}

void Local_Neuroseg_Set_Bottom_Top(Local_Neuroseg *locseg, const double *bottom,
				   const double *top)
{
  double axis_vector[3];
  int i;
  for (i = 0; i < 3; i++) {
    axis_vector[i] = top[i] - bottom[i];
  }

  Set_Neuroseg_Position(locseg, bottom, NEUROSEG_BOTTOM);
  double h = Geo3d_Orgdist(axis_vector[0], axis_vector[1], axis_vector[2]);
  if (h < 0.1) {
    locseg->seg.h = 1.0;
    return;
  }

  for (i = 0; i < 3; i++) {
    axis_vector[i] /= h;
  }
  locseg->seg.h = h + 1.0;

  Geo3d_Normal_Orientation(axis_vector[0], axis_vector[1], axis_vector[2],
			   &(locseg->seg.theta), &(locseg->seg.psi));
}

void Local_Neuroseg_Change_Height(Local_Neuroseg *locseg, double new_height)
{
  locseg->seg.h = new_height;
}

void Local_Neuroseg_Break_Gap(Local_Neuroseg *locseg,
			      const Stack *stack, double z_scale)
{
  int n = 11;
  
  double *profile = Local_Neuroseg_Height_Profile(locseg, stack, z_scale, n,
						  STACK_FIT_MEAN_SIGNAL,
						  NULL, NULL);

  double s1, s2, s3;

  s1 = profile[0] + profile[1] + profile[2];
  s2 = profile[4] + profile[5] + profile[6];
  s3 = profile[8] + profile[9] + profile[10];

  if (((s1 > s2 * 1.5) && (s3 > s2 * 1.5)) ||
      (s3 > s2 * 5.0)){
    Local_Neuroseg_Chop(locseg, 0.5);
    profile = Local_Neuroseg_Height_Profile(locseg, stack, z_scale, n,
					    STACK_FIT_MEAN_SIGNAL,
					    NULL, NULL);
  }

  double max_value = darray_max(profile, n, NULL);
  int i;
  for (i = n - 1; i >= 0; i--) {
    if (max_value < profile[i] * 10.0) {
      break;
    }
  }

  Local_Neuroseg_Chop(locseg, ((double) (i + 1)) / n);

  free(profile);
}
#if 0
void Local_Neuroseg_Chop(Local_Neuroseg *locseg, double ratio)
{
  double pos[3];
  if (ratio > 0) { /* bottom half */
    Local_Neuroseg_Bottom(locseg, pos);
    locseg->seg.h *= ratio;

    if (Neuropos_Reference != NEUROSEG_BOTTOM) {
      Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);
    }
  } else { /* top half */
    Local_Neuroseg_Top(locseg, pos);
    locseg->seg.h *= -ratio;
    locseg->seg.r1 = NEUROSEG_RADIUS(&(locseg->seg), locseg->seg.h); 
    if (Neuropos_Reference != NEUROSEG_TOP) {
      Set_Neuroseg_Position(locseg, pos, NEUROSEG_TOP);
    }
  }
}
#endif

void Local_Neuroseg_Chop(Local_Neuroseg *locseg, double ratio)
{
  double pos[3];
  if (ratio > 0) { /* bottom half */
    Local_Neuroseg_Bottom(locseg, pos);
    locseg->seg.h = (locseg->seg.h - 1.0) * ratio + 1.0;

    if (Neuropos_Reference != NEUROSEG_BOTTOM) {
      Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);
    }
  } else { /* top half */
    Local_Neuroseg_Top(locseg, pos);
    locseg->seg.h = -(locseg->seg.h - 1.0) * ratio + 1.0;
    locseg->seg.r1 = NEUROSEG_RADIUS(&(locseg->seg), locseg->seg.h - 1.0); 
    if (Neuropos_Reference != NEUROSEG_TOP) {
      Set_Neuroseg_Position(locseg, pos, NEUROSEG_TOP);
    }
  }
}

double Local_Neuroseg_Average_Signal(const Local_Neuroseg *locseg, 
				     const Stack *stack, double z_scale)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_MEAN_SIGNAL;
  Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
  return fs.scores[0];
}

double Local_Neuroseg_Average_Weak_Signal(const Local_Neuroseg *locseg, 
					  const Stack *stack, double z_scale)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_LOW_MEAN_SIGNAL;
  Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
  return fs.scores[0];
}

/*
void Local_Neuroseg_To_Geo3d_Circle(const Local_Neuroseg *locseg,
				    Geo3d_Circle *sc,
				    Neuropos_Reference_e ref) 
{
  sc->radius = Neuroseg_Rx(&(locseg->seg), ref);

  switch (ref) {
  case NEUROSEG_BOTTOM:
    //sc->radius = NEUROSEG_CR1(&(locseg->seg));
    Local_Neuroseg_Bottom(locseg, sc->center);
    break;
  case NEUROSEG_TOP:
    //sc->radius = NEUROSEG_CR2(&(locseg->seg));//locseg->seg.r2;
    Local_Neuroseg_Top(locseg, sc->center);
    break;
  case NEUROSEG_CENTER:
    //sc->radius = NEUROSEG_CRC(&(locseg->seg)); //(locseg->seg.r1 + locseg->seg.r2) / 2.0;
    Local_Neuroseg_Center(locseg, sc->center);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  Set_Xz_Orientation(sc->orientation, locseg->seg.theta, locseg->seg.psi); 
}

void Local_Neuroseg_To_Geo3d_Circle_Z(const Local_Neuroseg *locseg,
				      Geo3d_Circle *sc,
				      double z) 
{
  sc->radius = NEUROSEG_RADIUS(&(locseg->seg), z) * locseg->seg.scale;
  Local_Neuroseg_Axis_Position(locseg, sc->center, z);
  Set_Xz_Orientation(sc->orientation, locseg->seg.theta, locseg->seg.psi); 
}
void Local_Neuroseg_To_Geo3d_Circle_S(const Local_Neuroseg *locseg,
				      Geo3d_Circle *sc,
				      Neuropos_Reference_e ref,
				      double xy_scale, double z_scale)
{
  Local_Neuroseg *locseg2 = Copy_Local_Neuroseg(locseg);

  Local_Neuroseg_Scale(locseg2, xy_scale, z_scale);

  Local_Neuroseg_To_Geo3d_Circle(locseg2, sc, ref);

  Kill_Local_Neuroseg(locseg2);
}
*/

void Local_Neuroseg_To_Geo3d_Ellipse_Z(const Local_Neuroseg *locseg,
				       Geo3d_Ellipse *sc,
				       double z)
{
  sc->radius =  NEUROSEG_RADIUS(&(locseg->seg), z);
  sc->scale = locseg->seg.scale;
  Local_Neuroseg_Axis_Position(locseg, sc->center, z);
  Set_Xz_Orientation(sc->orientation, locseg->seg.theta, locseg->seg.psi);
  sc->alpha = locseg->seg.alpha;
}


/* note: curvature is not considered yet */
void Local_Neuroseg_To_Ellipse(const Local_Neuroseg *locseg,
			       Local_Neuroseg_Ellipse *locne,
			       Neuropos_Reference_e ref)
{
  switch (ref) {
  case NEUROSEG_BOTTOM:
    Local_Neuroseg_To_Ellipse_Z(locseg, 0.0, locne);
    break;
  case NEUROSEG_TOP:
    Local_Neuroseg_To_Ellipse_Z(locseg, locseg->seg.h - 1.0, locne);
    break;
  case NEUROSEG_CENTER:
    Local_Neuroseg_To_Ellipse_Z(locseg, (locseg->seg.h - 1.0) / 2.0, locne);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
}

int Local_Neuroseg_Stack_Feature(Local_Neuroseg *locseg, Stack *stack,
				 double z_scale, double *feats)
{
  /* alloc <Geo3d_Scalar_Field> */
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_S(locseg, NULL, NULL);
  
  int nfeat = 0;

  Stack_Fit_Score fs;
  fs.n = 8;
  int i;
  for (i = 0; i < fs.n; i++) {
    fs.options[i] = i;
  }

  nfeat += fs.n;

  Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &fs);
  for (i = 0; i < fs.n; i++) {
    feats[i] = fs.scores[i];
  }

  double *signal = darray_malloc(field->size);
 
  if (z_scale == 1.0) {
    Stack_Points_Sampling(stack, Coordinate_3d_Double_Array(field->points), 
			  field->size, signal);
  } else {
    Stack_Points_Sampling_Z(stack, z_scale, 
			    Coordinate_3d_Double_Array(field->points),
			    field->size, signal);
  }

  Geo3d_Scalar_Field field2;
  field2.size = field->size;
  field2.points = field->points;
  field2.values = signal;

  double vec[3], ext[3];
  field2.values = signal;
  Geo3d_Scalar_Field_Ort(&field2, vec, ext);
  free(signal);

  double ort[3];
  Geo3d_Orientation_Normal(locseg->seg.theta, locseg->seg.psi, 
			   ort, ort + 1, ort + 2);

  feats[nfeat++] = 
    fabs(Geo3d_Dot_Product(vec[0], vec[1], vec[2], ort[0], ort[1], ort[2]));

  feats[nfeat++] = fabs(ext[0]);
  feats[nfeat++] = fabs(ext[1]);
  feats[nfeat++] = fabs(ext[2]);

  int nprofile = NEUROSEG_DEFAULT_H;

  Local_Neuroseg_Height_Profile(locseg, stack, z_scale, nprofile, 1, NULL,
				feats + nfeat);
  nfeat += nprofile;
  
  /* free <Geo3d_Scalar_Field> */
  Kill_Geo3d_Scalar_Field(field);

  return nfeat;
}

/*
 * option: 0 - position search
 *         1 - size search and position search
 *         2 - size search
 *         3 - no size search or position search
 */
double Local_Neuroseg_Optimize(Local_Neuroseg *locseg, const Stack *stack, 
			       double z_scale, int option)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  int i;
  for (i = 0; i < 1; i++) {
    Local_Neuroseg_Position_Adjust(locseg, stack, z_scale);
  }

  Local_Neuroseg_Orientation_Search_C(locseg, stack, z_scale, &fs); 

  if (option <= 1) {
    for (i = 0; i < 5; i++) {
      Local_Neuroseg_Position_Adjust(locseg, stack, z_scale);
    }
  }

  //Local_Neuroseg_Radius_Search(locseg, stack, z_scale, 1.0, 10.0, 1.0, NULL);
  if ((option == 1) || (option == 2)){
    Local_Neuroseg_R_Scale_Search(locseg, stack, z_scale, 1.0, 10.0, 1.0,
				  0.5, 5.0, 0.5, NULL);
  }

  int var_index[LOCAL_NEUROSEG_NPARAM];

  /*
#ifdef _CONE_SEG_
  int *var_link = NULL;
#else
  int var_link[LOCAL_NEUROSEG_NPARAM];

  for (i = 0; i < LOCAL_NEUROSEG_NPARAM; i++) {
    var_link[i] = 0;
  }
  Variable_Set_Add_Link(var_link, 0, 1);
#endif
  */

  
  int nvar;

  /*
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R | NEUROSEG_VAR_MASK_SCALE,
				     NEUROPOS_VAR_MASK_NONE,
				     var_index);
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, NULL);
  */
  /*
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_NONE,
				     NEUROPOS_VAR_MASK_ALL,
				     var_index);
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, NULL, z_scale, NULL);
  */
  /*
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R1 |
				     NEUROSEG_VAR_MASK_ORIENTATION,
				     NEUROPOS_VAR_MASK_NONE,
				     var_index);
  Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, var_link, z_scale, NULL);
  */
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
				     NEUROSEG_VAR_MASK_ORIENTATION |
				     //			     NEUROSEG_VAR_MASK_ALPHA |
				     NEUROSEG_VAR_MASK_SCALE,
				     NEUROPOS_VAR_MASK_NONE,
				     var_index);
  int *var_link = NULL;
  double score = Fit_Local_Neuroseg_P(locseg, stack, var_index, nvar, var_link, 
				      z_scale, NULL);
  /*
  nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R1,
				     NEUROPOS_VAR_MASK_NONE,
				     var_index);
  */

  return score;
}

double Local_Neuroseg_Optimize_W(Local_Neuroseg *locseg, const Stack *stack, 
				 double z_scale, int option,
				 Locseg_Fit_Workspace *ws)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  int i;
  for (i = 0; i < ws->pos_adjust; i++) {
    Local_Neuroseg_Position_Adjust(locseg, stack, z_scale);
  }

  Local_Neuroseg_Orientation_Search_C(locseg, stack, z_scale, &fs); 

  if (option <= 1) {
    for (i = 0; i < 3; i++) {
      Local_Neuroseg_Position_Adjust(locseg, stack, z_scale);
    }
  }

  if ((option == 1) || (option == 2)){
    Local_Neuroseg_R_Scale_Search(locseg, stack, z_scale, 1.0, 10.0, 1.0,
				  0.5, 5.0, 0.5, NULL);
  }

  double score = Fit_Local_Neuroseg_W(locseg, stack, z_scale, ws);

  return score;
}

double Local_Neuroseg_Array_Maxr(Local_Neuroseg *locseg, int length)
{
  double maxr = 0.0;
  int i;
  for (i = 0; i < length; i++) {
    double r = NEUROSEG_RC(&(locseg->seg));//(locseg->seg.r1 + locseg->seg.r2) / 2.0;
    if (maxr < r) {
      maxr = r;
    }
  }

  return maxr;
}

int Local_Neuroseg_Hit_Mask(const Local_Neuroseg *locseg, 
			    const Stack *stack, double z_scale)
{
  /*
  coordinate_3d_t pts[13];
  double astep = locseg->seg.h / 5.0;
  int i;
  double z = 0.0;
  for (i = 0; i < 5; i++) {
    Local_Neuroseg_Axis_Position(locseg, pts[i], z);
    z += astep;
  }

  Local_Neuroseg_Halo_Points(locseg, 8, pts + 5);
  
  for (i = 0; i < 13; i++) {
    int x = iround(pts[i][0]);
    int y = iround(pts[i][1]);
    int z = iround(pts[i][2] * z_scale);

    if (Stack_Pixel(stack, x, y, z, 0) > 0.0) {
      return i + 1;
    }
  }
  */

  coordinate_3d_t pts[11];
  int i;
  
  double astep = locseg->seg.h / 6.0;
  double z = locseg->seg.h / 3.0;
  for (i = 0; i < 3; i++) {
    Local_Neuroseg_Axis_Position(locseg, pts[i], z);
    z += astep;
  }
  
  Local_Neuroseg_Halo_Points(locseg, 8, pts + 1);
  
  for (i = 0; i < 11; i++) {
    int x = iround(pts[i][0]);
    int y = iround(pts[i][1]);
    int z = iround(pts[i][2] * z_scale);

    if (Stack_Pixel(stack, x, y, z, 0) > 0.0) {
      return i + 1;
    }
  }

  return 0;
}

double Local_Neuroseg_Center_Sample(const Local_Neuroseg *locseg, 
				    const Stack *stack, double z_scale)
{
  double center[3];
  Local_Neuroseg_Center(locseg, center);
  
  return Stack_Point_Sampling(stack, center[0], center[1], center[2] * z_scale);
}

/* under testing */
double Local_Neuroseg_Top_Sample(const Local_Neuroseg *locseg, 
				 const Stack *stack, double z_scale)
{
  double pos[3];
  Local_Neuroseg_Top(locseg, pos);
  
  double value = 
    Stack_Point_Sampling(stack, pos[0], pos[1], pos[2] * z_scale);

  double lambda;
  for (lambda = 0.6; lambda < 0.95; lambda += 0.1) {
    Local_Neuroseg_Axis_Position(locseg, pos, locseg->seg.h * lambda);

    double value2 = 
      Stack_Point_Sampling(stack, pos[0], pos[1], pos[2] * z_scale);
    
    if (value2 > value) {
      value = value2;
    }
  }

  return value;
}

double Local_Neuroseg_Planar_Dist(const Local_Neuroseg *locseg1,
				  const Local_Neuroseg *locseg2)
{
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];

  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);
  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
  
  double int1, int2;
  int cond;
  return Geo3d_Lineseg_Lineseg_Dist(bottom1, top1, bottom2, top2,
				    &int1, &int2, &cond);
}

double Local_Neuroseg_Planar_Dist_L(const Local_Neuroseg *locseg1,
				    const Local_Neuroseg *locseg2)
{
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];

  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);
  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
  
  return Geo3d_Line_Line_Dist(bottom1, top1, bottom2, top2);
}

void Local_Neuroseg_Stretch(Local_Neuroseg *locseg, double scale, 
			    double offset, int direction)
{
  double pos[3];
  double new_height = (locseg->seg.h-1.0) * scale + offset;

  switch(direction) {
  case -1: /* stretch backward */
    Local_Neuroseg_Axis_Position(locseg, pos, 
	locseg->seg.h - 1.0 - new_height);
    Local_Neuroseg_Change_Bottom(locseg, pos);
    break;
  case 1: /* stretch forward */
    Local_Neuroseg_Axis_Position(locseg, pos, new_height);
    Local_Neuroseg_Change_Top(locseg, pos);
    break;
  case 0: /* stretch to both directions */
    Local_Neuroseg_Axis_Position(locseg, pos, new_height / 2.0);
    Local_Neuroseg_Change_Top(locseg, pos);
    Local_Neuroseg_Axis_Position(locseg, pos, 
				 (locseg->seg.h - 1.0 - new_height) / 2.0);
    Local_Neuroseg_Change_Bottom(locseg, pos);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }
}

void Local_Neuroseg_Swc_Fprint(FILE *fp, const Local_Neuroseg *locseg,
			       int start_id, int parent_id)
{
  Local_Neuroseg_Swc_Fprint_T(fp, locseg, start_id, parent_id, 2);
}

void Local_Neuroseg_Swc_Fprint_T(FILE *fp, const Local_Neuroseg *locseg,
				 int start_id, int parent_id, int type)
{
  Geo3d_Circle *circle = New_Geo3d_Circle();

  Local_Neuroseg_To_Circle(locseg, NEUROSEG_BOTTOM, NEUROSEG_CIRCLE_RX, circle);
  Geo3d_Circle_Swc_Fprint_T(fp, circle, start_id, parent_id, type, 1.0);
  Local_Neuroseg_To_Circle(locseg, NEUROSEG_TOP, NEUROSEG_CIRCLE_RX, circle);
  Geo3d_Circle_Swc_Fprint_T(fp, circle, start_id + 1, start_id, type, 1.0);

  Kill_Geo3d_Circle(circle);
}

double Local_Neuroseg_Point_Dist(const Local_Neuroseg *locseg,
				 double x, double y, double z, double res)
{
  ASSERT(locseg->seg.c == 0.0, "cone not supported.");
  ASSERT(locseg->seg.alpha == 0.0, "alpha not supported.");

  double tmp_pos[3];

  //  Neuropos_Coordinate(locseg->pos, tmp_pos, tmp_pos + 1, tmp_pos + 2);
  Local_Neuroseg_Bottom(locseg, tmp_pos);

  tmp_pos[0] = x - tmp_pos[0];
  tmp_pos[1] = y - tmp_pos[1];
  tmp_pos[2] = z - tmp_pos[2];

  Rotate_XZ(tmp_pos, tmp_pos, 1, locseg->seg.theta, locseg->seg.psi, 1);

  double half_size = 0.5 * res;

  if ((tmp_pos[2] >= -half_size) && (tmp_pos[2] <= locseg->seg.h - half_size)) {
    tmp_pos[2] = 0.0;
  } else {
    if (tmp_pos[2] > locseg->seg.h - half_size) {
      tmp_pos[2] -= locseg->seg.h - half_size;
    } else {
      tmp_pos[2] += half_size;
    }
  }

  //printf("%g %g %g\n", tmp_pos[0], tmp_pos[1], tmp_pos[1]);

  double r = locseg->seg.r1;

  double d2 = (tmp_pos[0] * tmp_pos[0]) / 
    (locseg->seg.scale * locseg->seg.scale) + tmp_pos[1] * tmp_pos[1];
  if (d2 <= r * r) {
    return fabs(tmp_pos[2]);
  } else {
    double rs = locseg->seg.r1 * locseg->seg.scale;
    /*
    double step = 
      TZ_PI * res / (2.0 * locseg->seg.r1 * sqrt(locseg->seg.scale));
    double theta = 0.0;
    
    double mindist = INFINITY;
    for (theta = 0.0; theta < TZ_2PI; theta += step) {
      double ex = rs * cos(theta);
      double ey = r * sin(theta);
      ex -= tmp_pos[0];
      ey -= tmp_pos[1];
      double dist = sqrt(ex * ex + ey * ey);
      if (dist < mindist) {
	mindist = dist;
      }
    }
    */

    double mindist = Ellipse_Point_Distance(tmp_pos[0], tmp_pos[1], rs, r,
					    NULL, NULL);

    if (tmp_pos[2] != 0.0) {
      mindist = sqrt(mindist * mindist + tmp_pos[2] * tmp_pos[2]);
    }

    return mindist;
  }

  return 0.0;
}

double Local_Neuroseg_Lineseg_Dist(const Local_Neuroseg *locseg,
				   const coordinate_3d_t start, 
				   const coordinate_3d_t end, double res)
{
  double step[3];
  double d = Coordinate_3d_Distance(start, end);
  double mindist = 0.0;

  //Print_Local_Neuroseg(locseg);

  if (d < res) {
    mindist = Local_Neuroseg_Point_Dist(locseg, start[0], start[1], start[2],
					res);
  } else {
    step[0] = (end[0] - start[0]) / d * res;
    step[1] = (end[1] - start[1]) / d * res;
    step[2] = (end[2] - start[2]) / d * res;
    double x = start[0];
    double y = start[1];
    double z = start[2];
    mindist = Local_Neuroseg_Point_Dist(locseg, x, y, z, res);

    while ((d > 0.0) && (mindist > 0.0)) {
      d -= res;
      x += step[0];
      y += step[1];
      z += step[2];
      
      double dist = Local_Neuroseg_Point_Dist(locseg, x, y, z, res);
      if (dist < mindist) {
	mindist = dist;
      }
      //printf("%g %g %g\n", x, y, z);
    }
    //printf("%g %g %g\n", end[0], end[1], end[2]);
  }

  return mindist;
}

double Local_Neuroseg_Dist2(const Local_Neuroseg *locseg1, 
			    const Local_Neuroseg *locseg2, double *pos)
{
  double bottom[3];
  double top[3];
  Local_Neuroseg_Bottom(locseg1, bottom);
  Local_Neuroseg_Top(locseg1, top);

  return Local_Neuroseg_Lineseg_Dist_S(locseg2, bottom, top, pos);
}

double Local_Neuroseg_Lineseg_Dist_S(const Local_Neuroseg *locseg,
				     const coordinate_3d_t start, 
				     const coordinate_3d_t end,
				     double *pos)
{
  double step[3];
  double d = Coordinate_3d_Distance(start, end);
  double mindist = 0.0;

  double tmp_pos[3];
  
  //Print_Local_Neuroseg(locseg);
  double res = 1.0;

  if (d < res) {
    mindist = Local_Neuroseg_Point_Dist_S(locseg, 
					  start[0], start[1], start[2],
					  pos);
  } else {
    step[0] = (end[0] - start[0]) / d * res;
    step[1] = (end[1] - start[1]) / d * res;
    step[2] = (end[2] - start[2]) / d * res;
    double x = start[0];
    double y = start[1];
    double z = start[2];
    mindist = Local_Neuroseg_Point_Dist_S(locseg, x, y, z, pos);

    while ((d > 0.0) && (mindist > 0.0)) {
      d -= res;
      x += step[0];
      y += step[1];
      z += step[2];
      
      double dist = Local_Neuroseg_Point_Dist_S(locseg, x, y, z, tmp_pos);
      if (dist < mindist) {
	mindist = dist;
	if (pos != NULL) {
	  pos[0] = tmp_pos[0];
	  pos[1] = tmp_pos[1];
	  pos[2] = tmp_pos[2];
	}
      }
      //printf("%g %g %g\n", x, y, z);
    }
    //printf("%g %g %g\n", end[0], end[1], end[2]);
  }

  return mindist;  
}

double Local_Neuroseg_Point_Dist_S(const Local_Neuroseg *locseg, 
				   double x, double y, double z,
				   double *pt)
{
  double tmp_tx, tmp_ty, tmp_tz;

  double tmp_pos[3];

  //  Neuropos_Coordinate(locseg->pos, tmp_pos, tmp_pos + 1, tmp_pos + 2);
  Local_Neuroseg_Bottom(locseg, tmp_pos);

  tmp_pos[0] = x - tmp_pos[0];
  tmp_pos[1] = y - tmp_pos[1];
  tmp_pos[2] = z - tmp_pos[2];

  Rotate_XZ(tmp_pos, tmp_pos, 1, locseg->seg.theta, locseg->seg.psi, 1);
  Rotate_Z(tmp_pos, tmp_pos, 1, locseg->seg.alpha, 1);

  double mindist = 0.0;
  double coef = NEUROSEG_COEF(&(locseg->seg));

  if (Neuroseg_Hit_Test(&(locseg->seg), tmp_pos[0], tmp_pos[1], tmp_pos[2])
      == FALSE) {
    double rx, ry;
    ry = locseg->seg.r1;
    rx = ry * locseg->seg.scale;
    if (tmp_pos[2] <= 0.0) { /* below bottom */
      if (Point_In_Ellipse(tmp_pos[0], tmp_pos[1], rx, ry) == TRUE) {
	mindist = -tmp_pos[2];
	tmp_tx = 0;
	tmp_ty = 0;
	//tmp_tz = tmp_pos[2];
	tmp_tz = 0.0;
      } else { /* off range */
	double d = Ellipse_Point_Distance(tmp_pos[0], tmp_pos[1], rx, ry,
					  &tmp_tx, &tmp_ty);
	mindist = sqrt(tmp_pos[2] * tmp_pos[2] + d * d);
	//tmp_tz = tmp_pos[2];
	tmp_tz = 0.0;
      }
      if (pt != NULL) {
	pt[0] = tmp_tx;
	pt[1] = tmp_ty;
	pt[2]= tmp_tz;
      }
    } else if (tmp_pos[2] >= locseg->seg.h - 1.0) { /* above top */
      ry = locseg->seg.r1 + (locseg->seg.h - 1.0) * coef;
      rx = ry * locseg->seg.scale;
      tmp_tz = locseg->seg.h - 1.0;
      if (Point_In_Ellipse(tmp_pos[0], tmp_pos[1], rx, ry) == TRUE) {
	tmp_tx = 0;
	tmp_ty = 0;
	mindist = tmp_pos[2] - tmp_tz;
	if (pt != NULL) {
	  pt[0] = tmp_tx;
	  pt[1] = tmp_ty;
	  pt[2]= tmp_tz;
	}
      } else { /* off range */
	double d = Ellipse_Point_Distance(tmp_pos[0], tmp_pos[1], rx, ry,
					  &tmp_tx, &tmp_ty);
	mindist = sqrt(dsqr(tmp_pos[2] - tmp_tz) + d * d);

	if (pt != NULL) {
	  pt[0] = tmp_tx;
	  pt[1] = tmp_ty;
	  pt[2]= tmp_tz;
	}
      }
    } else {
      if (coef != 0.0) {
	ry += coef * tmp_pos[2];
	rx = ry * locseg->seg.scale;
      }
      mindist = Ellipse_Point_Distance(tmp_pos[0], tmp_pos[1], rx, ry,
				       &tmp_tx, &tmp_ty); 
      //mindist = d;
      //mindist = sqrt(tmp_pos[2] * tmp_pos[2] + d * d);
      tmp_tz = tmp_pos[2];

      if (pt != NULL) {
	pt[0] = tmp_tx;
	pt[1] = tmp_ty;
	pt[2]= tmp_tz;
      }

      if (coef != 0.0) { /* cone shape */
	double h;
	ry = locseg->seg.r1;
	for (h = 0.5; h < locseg->seg.h - 1.0; h += 0.5) {
	  ry += coef *0.5;
	  rx = ry * locseg->seg.scale;
	  double d = Ellipse_Point_Distance(tmp_pos[0], tmp_pos[1], rx, ry,
					    &tmp_tx, &tmp_ty);
	  tmp_tz = tmp_pos[2] - h;
	  d = sqrt(d * d + tmp_tz * tmp_tz);
	  if (mindist > d) {
	    mindist = d;
	    if (pt != NULL) {
	      pt[0] = tmp_tx;
	      pt[1] = tmp_ty;
	      pt[2]= h;
	    }
	  }
	}
      }
    }

    if (pt != NULL) {
      Rotate_Z(pt, pt, 1, locseg->seg.alpha, 0);
      Rotate_XZ(pt, pt, 1, locseg->seg.theta, locseg->seg.psi, 0);
      pt[0] += locseg->pos[0];
      pt[1] += locseg->pos[1];
      pt[2] += locseg->pos[2];
    }    
  } else {
    if (pt != NULL) {
      pt[0] = x;
      pt[1] = y;
      pt[2] = z;
    }
  }

  return mindist;
}


#define LOCAL_NEUROSEG_STACK(kind)					\
  for (k = 0; k < out->depth; k++) {					\
    for (j = 0; j < out->height; j++) {					\
      for (i = 0; i < out->width; i++) {				\
	x = i - out->width / 2;						\
	y = j - out->height / 2;					\
	z = k;								\
	Geo3d_Rotate_Coordinate(&x, &y, &z, locseg->seg.theta,		\
				locseg->seg.psi, FALSE);		\
	Geo3d_Translate_Coordinate(&x, &y, &z, locseg->pos[0],		\
				   locseg->pos[1], locseg->pos[2]);	\
	double value = Stack_Point_Sampling(stack, x, y, z);		\
	if (isnan(value) == 1) {					\
	  value = 0.0;							\
	} else {							\
	  TZ_CONCAT(VALIDATE_INTENSITY_GREY, kind)(value);		\
	}								\
	TZ_CONCAT(ima.array, kind)[offset++] = value;			\
      }									\
    }									\
  }

Stack* Local_Neuroseg_Stack(const Local_Neuroseg *locseg, const Stack *stack)
{
  int width = 
    iround(NEUROSEG_RB(&(locseg->seg)) * locseg->seg.scale * 2) * 2 + 1;
  int height = iround(NEUROSEG_RB(&(locseg->seg)) * 2) * 2 + 1;
  int depth = iround(locseg->seg.h);

  Stack *out = Make_Stack(stack->kind, width, height, depth);
  Image_Array ima;
  ima.array = out->array;

  double x, y, z;

  int offset = 0;
  int i, j, k;

  switch (out->kind) {
  case GREY:
    LOCAL_NEUROSEG_STACK(8);
    break;
  case GREY16:
    LOCAL_NEUROSEG_STACK(16);
    break;
  case FLOAT32:
    LOCAL_NEUROSEG_STACK(32);
    break;
  case FLOAT64:
    LOCAL_NEUROSEG_STACK(64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  /*
  for (k = 0; k < out->depth; k++) {
    for (j = 0; j < out->height; j++) {
      for (i = 0; i < out->width; i++) {
	x = i - out->width / 2;
	y = j - out->height / 2;
	z = k;
	Geo3d_Rotate_Coordinate(&x, &y, &z, locseg->seg.theta, locseg->seg.psi,
				FALSE);
	Geo3d_Translate_Coordinate(&x, &y, &z, locseg->pos[0], locseg->pos[1],
				   locseg->pos[2]);
	double value = Stack_Point_Sampling(stack, x, y, z);
	if (isnan(value) == 1) {
	  value = 0.0;
	} else {
	  VALIDATE_INTENSITY_GREY(value);
	}
	out->array[offset++] = value;
      }
    }
  }
  */
  return out;
}

void Local_Neuroseg_Rotate(Local_Neuroseg *locseg, double theta, double psi)
{
  Geo3d_Rotate_Orientation(theta, psi, &(locseg->seg.theta), 
			   &(locseg->seg.psi));
}

void Local_Neuroseg_Translate(Local_Neuroseg *locseg, const double *offset)
{
  locseg->pos[0] += offset[0];
  locseg->pos[1] += offset[1];
  locseg->pos[2] += offset[2];
}

double Local_Neuroseg_Intersect(const Local_Neuroseg *locseg1, 
				const Local_Neuroseg *locseg2,
				double *t1, double *t2)
{
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];
  
  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);

  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
  
  int cond;
  double dist = Geo3d_Lineseg_Lineseg_Dist(bottom1, top1, bottom2, top2,
					   t1, t2, &cond);
  
  return dist;
}

#define LOCAL_NEUROSEG_FIX_ON_EPS 1e-3

Neuropos_Reference_e Local_Neuroseg_Fixon(const Local_Neuroseg *locseg1, 
					   const Local_Neuroseg *locseg2, 
					   double *t)
{
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];
  
  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);

  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
    
  double dist = Geo3d_Point_Lineseg_Dist(bottom1, bottom2, top2, t);
  Neuropos_Reference_e fix = NEUROSEG_BOTTOM;
  double mindist = dist;
  dist = Geo3d_Point_Lineseg_Dist(top1, bottom2, top2, t);
  if (dist <  mindist) {
    mindist = dist;
    fix = NEUROSEG_TOP;
  }

  if (mindist > LOCAL_NEUROSEG_FIX_ON_EPS) {
    fix = NEUROSEG_REFERENCE_UNDEF;
  }

  return fix;
}

#define LOCAL_NEUROSEG_TANGENT_COMPARE_EPS 1e-3

int Local_Neuroseg_Tangent_Compare(const Local_Neuroseg *locseg1,
				  const Local_Neuroseg *locseg2)
{
  double pos1[3], pos2[3];
  double ort1[3], ort2[3];

  Local_Neuroseg_Bottom(locseg1, pos1);
  Local_Neuroseg_Bottom(locseg2, pos2);

  Local_Neuroseg_Normal(locseg1, ort1);
  Local_Neuroseg_Normal(locseg2, ort2);

  double ort_diff = Geo3d_Dist(ort1[0], ort1[1], ort1[2], 
			       ort2[0], ort2[1], ort2[2]);

  if (Geo3d_Dist(pos1[0], pos1[1], pos1[2], pos2[0], pos2[1], pos2[2])
      < LOCAL_NEUROSEG_TANGENT_COMPARE_EPS) { /* same position */
    if (ort_diff < LOCAL_NEUROSEG_TANGENT_COMPARE_EPS) { /* same orientation */
      return 0;
    } else {
      return 1;
    }
  } else { /* different positions */
    double pos_vec[3];
    pos_vec[0] = pos2[0] - pos1[0];
    pos_vec[1] = pos2[1] - pos1[1];
    pos_vec[2] = pos2[2] - pos1[2];

    double dot1 = Coordinate_3d_Dot(pos_vec, ort1);
    double dot2 = -Coordinate_3d_Dot(pos_vec, ort2);
    if (fabs(dot1) < LOCAL_NEUROSEG_TANGENT_COMPARE_EPS) { /* Q on P */
      if (fabs(dot2) < LOCAL_NEUROSEG_TANGENT_COMPARE_EPS) { /* P on Q */
	if (ort_diff < LOCAL_NEUROSEG_TANGENT_COMPARE_EPS) { /* same ort */
	  return 2;
	} else { /* different orientation */
	  return 3;
	}
      } else { /* P not on Q */
	return 4;
      }
    } else if (dot1 > 0.0) { /* Q in positive space of P */
      if (dot2 < 0.0) { /* P in negative space of Q */
	return -11;
      } else {
	return -12;
      }
    } else { /* Q in negative space of P */
      if (dot2 > 0.0) { /* P in positive space of Q */
	return 11;
      } else {
	return 12;
      }
    }
  }

  return -1;
}

double *Locseg_Conn_Feature(const Local_Neuroseg *locseg1, 
			    const Local_Neuroseg *locseg2,
			    const Stack *stack, const double *res, 
			    double *feat, int *n)
{
  TZ_ASSERT(locseg1 != NULL, "Null pointer.");
  TZ_ASSERT(locseg2 != NULL, "Null pointer.");

  *n = 9;
  if (feat == NULL) {
    feat = darray_malloc(*n);
  }
  
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];

  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);
  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
  
  Local_Neuroseg *zlocseg1 = Copy_Local_Neuroseg(locseg1);
  Local_Neuroseg *zlocseg2 = Copy_Local_Neuroseg(locseg2);

  double z_scale = res[0] / res[2];

  Local_Neuroseg_Scale_Z(zlocseg1, z_scale);
  Local_Neuroseg_Scale_Z(zlocseg2, z_scale);

  double zbottom1[3], ztop1[3];
  double zbottom2[3], ztop2[3];

  Local_Neuroseg_Bottom(zlocseg1, zbottom1);
  Local_Neuroseg_Top(zlocseg1, ztop1);
  Local_Neuroseg_Bottom(zlocseg2, zbottom2);
  Local_Neuroseg_Top(zlocseg2, ztop2);
  
  Delete_Local_Neuroseg(zlocseg1);
  Delete_Local_Neuroseg(zlocseg2);

  /* anisotropic planar distance */
  feat[0] = Geo3d_Line_Line_Dist(zbottom1, ztop1, zbottom2, ztop2);
  //feat[0] *= res[0];

  /* isotropic planar distance */
  feat[1] = Geo3d_Line_Line_Dist(bottom1, top1, bottom2, top2);
  
  double intersect1, intersect2;
  int cond;

  /* anisotropic euclidean distance */
  feat[2] = Geo3d_Lineseg_Lineseg_Dist(zbottom1, ztop1, zbottom2, ztop2,
				       &intersect1, &intersect2, &cond);
  //feat[2] *= res[0];

  /* isotropic euclidean distance */
  feat[3] = Geo3d_Lineseg_Lineseg_Dist(bottom1, top1, bottom2, top2,
				       &intersect1, &intersect2, &cond);

  /* thickness 1 */
  feat[4] = Neuroseg_Rx(&(locseg1->seg), NEUROSEG_CENTER);// * res[0];

  /* thickness 2 */
  feat[5] = Neuroseg_Rx(&(locseg2->seg), NEUROSEG_CENTER);// * res[0];

  /* angle */
  feat[6] = Neuroseg_Angle_Between(&(locseg1->seg), &(locseg2->seg));

  /* anisotropic surface distance */
  feat[7] = Local_Neuroseg_Lineseg_Dist_S(zlocseg2, zbottom1, ztop1, NULL);// * res[0];

/* isotropic surface distance */
  feat[8] = Local_Neuroseg_Lineseg_Dist_S(locseg2, bottom1, top1, NULL);

  return feat;
}

double Local_Neuroseg_Zscore(const Local_Neuroseg *locseg)
{
  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  double zbottom = pos[2];
  Local_Neuroseg_Top(locseg, pos);
  double ztop = pos[2];

  return fabs(zbottom - ztop) / locseg->seg.h;
}

Local_Neuroseg* Local_Neuroseg_From_Field(Geo3d_Scalar_Field *field,
    Local_Neuroseg *locseg)
{
  double vec[9];
  double value[3];
  Geo3d_Scalar_Field_Pca(field, value, vec);

  if (value[0] > 1.0) {
    if (locseg == NULL) {
      locseg = New_Local_Neuroseg();
    }
    Geo3d_Normal_Orientation(vec[0], vec[1], vec[2], &locseg->seg.theta,
	&locseg->seg.psi);
    double centroid[3];
    Geo3d_Scalar_Field_Centroid(field, centroid);
    locseg->seg.h = iround(sqrt(value[0]) * 3.2) + 1.0;
    locseg->seg.r1 = sqrt(value[1]) * 2.0;
    Set_Neuroseg_Position(locseg, centroid, NEUROSEG_CENTER);
    return locseg;
  }

  return NULL;
}
