/* tz_object_3d.c
 *
 * 26-Nov-2007 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_error.h"
#include "tz_object_3d.h"
#ifdef HAVE_LIBGSL
#  if defined(HAVE_INLINE)
#    undef HAVE_INLINE
#    define INLINE_SUPPRESSED
#  endif
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_matrix.h>
#  include <gsl/gsl_eigen.h>
#  include <gsl/gsl_statistics_int.h>
#  if defined(INLINE_SUPPRESSED)
#    define HAVE_INLINE
#  endif
#endif
#include <utilities.h>
#include "tz_utilities.h"
#include "tz_stack_lib.h"
//#include "tz_stack_bwdist.h"
#include "tz_stack_math.h"
#include "tz_voxel_graphics.h"
#include "tz_voxel_linked_list_ext.h"
#include "tz_graph.h"
#include "tz_graph_utils.h"
#include "tz_error.h"
#include "tz_objdef.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_utils.h"
#include "tz_iarray.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_utils.h"
#include "tz_image_trans.h"

INIT_EXCEPTION

size_t Object_3d_Asize(size_t size)
{
  return size * sizeof(Voxel_t);
}

void Default_Object_3d(Object_3d *object)
{
  if (object->voxels != NULL) {
    return;
  }
  object->size = 0;
  object->conn = 0;
}

static inline size_t obj_asize(const Object_3d *object)
{ return object->size * sizeof(Voxel_t); }

DEFINE_OBJECT_MANAGEMENT(Object_3d, voxels, asize, obj)

Object_3d *Make_Object_3d(size_t size, int conn)
{
  Object_3d *obj;
  
  obj = new_obj(sizeof(Voxel_t) * size, "Make_Object_3d");
  obj->size = size;
  obj->conn = conn;

  return obj;
}

void Print_Object_3d(Object_3d *obj)
{
  if (obj == NULL) {
    printf("Null 3D object\n");
    return;
  }

  int i;
  printf("3D Object: %zu voxels with connectivity %d\n", obj->size, obj->conn);
  for (i = 0; i < obj->size; i++) {
    printf("%d, %d, %d\n", obj->voxels[i][0], obj->voxels[i][1], 
	   obj->voxels[i][2]);  
  }
}

void Print_Object_3d_Info(Object_3d *obj)
{
  if (obj == NULL) {
    printf("Null 3D object\n");
    return;
  }

  printf("3D Object: %zu voxels with connectivity %d\n", obj->size, obj->conn);
}

/* Object_3d_Range(): The range of a 3D object.
 * 
 * Args: obj - input object;
 *       corners - the array of the coordinates of the first and last corners.
 *                 It must be able to hold 6 integers.
 *
 * Return: void.
 */
void Object_3d_Range(const Object_3d *obj, int *corners)
{
#if 0
#  ifdef INT_VOXEL_TYPE
  gsl_vector_int *vec = (gsl_vector_int *) 
    Guarded_Malloc(sizeof(gsl_vector_int), "Object_3d_Range");
  int i;
  for (i = 0; i < 3; i++) {
    vec->size = obj->size;
    vec->stride = 3;
    vec->data = &(obj->voxels[0][i]);
    vec->block = NULL;
    vec->owner = 0;
    gsl_vector_int_minmax(vec, &(corners[i]), &(corners[i + 3]));
  }

  gsl_vector_int_free(vec);
#  else
  THROW(ERROR_DATA_TYPE);
#  endif
#else
  int i;
  for (i = 0; i < 6; i++) {
    corners[i] = 0.0;
  }

  if (obj == NULL) {
    return;
  }

  if (obj->size == 0) {
    return;
  }

  corners[0] = obj->voxels[0][0];
  corners[1] = obj->voxels[0][1];
  corners[2] = obj->voxels[0][2];
  corners[3] = obj->voxels[0][0];
  corners[4] = obj->voxels[0][1];
  corners[5] = obj->voxels[0][2];

  for (i = 1; i < obj->size; i++) {
    int j;
    for (j = 0; j < 3; j++) {
      if (corners[j] > obj->voxels[i][j]) {
	corners[j] = obj->voxels[i][j];
      } else if (corners[j+3] < obj->voxels[i][j]) {
	corners[j+3] = obj->voxels[i][j];
      }
    }
  }
#endif
  
}

void Object_3d_Indices(const Object_3d *obj, int width, int height, int depth,
		       size_t *indices)
{
  int area = width * height;
  int i;
  for (i = 0; i <  obj->size; i++) {
    if ((obj->voxels[i][0] >= 0) && (obj->voxels[i][0] < width) &&
	(obj->voxels[i][1] >= 0) && (obj->voxels[i][1] < height) &&
	(obj->voxels[i][2] >= 0) && (obj->voxels[i][2] < depth)) {
      indices[i] = (size_t) obj->voxels[i][0] + obj->voxels[i][1] * width + 
	obj->voxels[i][2] * area;
    } else {
      indices[i] = INVALID_ARRAY_INDEX;
    }
  }  
}

/* Object_3d_Translate(): translate an object.
 *
 * Args: obj - object to translate;
 *       x - x offset;
 *       y - y offset;
 *       z - z offset.
 *
 * Return: void.
 *
 * Note: The coordinate of each voxel of the object has the order (X, Y, Z)
 */
void Object_3d_Translate(Object_3d *obj, int x, int y, int z)
{
  if (obj == NULL) {
    THROW(ERROR_POINTER_NULL);
  }

  int i;
  for (i = 0; i < obj->size; i++) {
    obj->voxels[i][0] += x;
    obj->voxels[i][1] += y;
    obj->voxels[i][2] += z;
  }
}

/* Object_3d_Centroid(): Calculate the centroid of an object.
 */
void Object_3d_Centroid(const Object_3d *obj, double *pos)
{
  if (obj == NULL) {
    THROW(ERROR_POINTER_NULL);
  }

  int i, j;
  pos[0] = pos[1] = pos[2] = 0.0;

  for (i = 0; i < obj->size; i++) {
    for (j = 0; j < 3; j++) {
      pos[j] += (double) obj->voxels[i][j];
    }
  }

  for (j = 0; j < 3; j++) {
    pos[j] /= (double) obj->size;
  }
}

/* Object_3d_Cov: Calculate the covariance matrix of an object.
 *
 * Args: obj - 3D object;
 *       cov - an array to store the covariance matrix.
 *         cov[0] cov[1] cov[2]
 *         cov[3] cov[4] cov[5] 
 *         cov[6] cov[7] cov[8] 
 */
void Object_3d_Cov(const Object_3d *obj, double *cov)
{   
  /*
#ifdef HAVE_LIBGSL
  size_t stride = 3;
  cov[0] = gsl_stats_int_variance(&(obj->voxels[0][0]), stride, obj->size);
  cov[1] = gsl_stats_int_covariance(&(obj->voxels[0][0]), stride, 
				    &(obj->voxels[0][1]), stride, obj->size);
  cov[2] = gsl_stats_int_covariance(&(obj->voxels[0][0]), stride, 
				    &(obj->voxels[0][2]), stride, obj->size);
  cov[4] = gsl_stats_int_variance(&(obj->voxels[0][1]), stride, obj->size);
  cov[5] = gsl_stats_int_covariance(&(obj->voxels[0][1]), stride, 
				    &(obj->voxels[0][2]), stride, obj->size);
  cov[8] = gsl_stats_int_variance(&(obj->voxels[0][2]), stride, obj->size);
#else
*/
  int *x = iarray_malloc(obj->size);
  int *y = iarray_malloc(obj->size);
  int *z = iarray_malloc(obj->size);
  size_t i;
  for (i = 0; i < obj->size; i++) {
    x[i] = obj->voxels[i][0];
    y[i] = obj->voxels[i][1];
    z[i] = obj->voxels[i][2];
  }

  cov[0] = iarray_var(x, obj->size);
  cov[1] = iarray_cov(x, y, obj->size);
  cov[2] = iarray_cov(x, z, obj->size);
  cov[4] = iarray_var(y, obj->size);
  cov[5] = iarray_cov(y, z, obj->size);
  cov[8] = iarray_var(z, obj->size);

  free(x);
  free(y);
  free(z);
  /*
#endif
*/
  cov[3] = cov[1];
  cov[6] = cov[2];
  cov[7] = cov[5];
}

void Object_3d_Cov_Zscale(const Object_3d *obj, double *cov, double z_scale)
{
  Object_3d_Cov(obj, cov);
  cov[8] *= z_scale * z_scale;
  cov[2] *= z_scale;
  cov[5] *= z_scale;
  cov[6] = cov[2];
  cov[7] = cov[5];
}

/*
#ifdef HAVE_LIBGSL
static void cov_3d_orientation(gsl_matrix *gm, double *vec,
			       Object_3d_Orientation_Pointer_e pointer)
{
  gsl_vector *eval = gsl_vector_alloc(3);
  gsl_matrix *evec = gsl_matrix_alloc(3,3);
  gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(3);

  gsl_eigen_symmv(gm, eval, evec, w);
  
  switch (pointer) {
  case MAJOR_AXIS:
    gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_DESC);
    break;
  case PLANE_NORMAL:
    gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_ASC);
    break;
  default:
    gsl_vector_free(eval);
    gsl_matrix_free(evec);
    gsl_eigen_symmv_free(w);
    gsl_matrix_free(gm);
    THROW(ERROR_DATA_VALUE);
    break;
  }
  
  gsl_vector_view v = gsl_vector_view_array(vec, 3);
  gsl_matrix_get_col(&(v.vector), evec, 0);

  gsl_vector_free(eval);
  gsl_matrix_free(evec);
  gsl_eigen_symmv_free(w); 
}
#else
*/
static void cov_3d_orientation(double *cov, double *vec,
			       Object_3d_Orientation_Pointer_e pointer)
{
  double eigv[3];
  
  Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], eigv);

  switch (pointer) {
  case MAJOR_AXIS:
    Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	eigv[0], vec);
    break;
  case PLANE_NORMAL:
    Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
	eigv[2], vec);
    break;
  default:
    THROW(ERROR_DATA_VALUE);
    break;
  }
}
/*
#endif
*/

/* Object_3d_Direction: Calculate the orientation of an object.
 */
void Object_3d_Orientation(const Object_3d *obj, double *vec,
			   Object_3d_Orientation_Pointer_e pointer)
{
  /*
#ifdef HAVE_LIBGSL
  gsl_matrix *gm = gsl_matrix_alloc(3, 3);
  Object_3d_Cov(obj, gm->data);

  cov_3d_orientation(gm, vec, pointer);

  gsl_matrix_free(gm);
#else
*/
  double cov[9];
  Object_3d_Cov(obj, cov);
  cov_3d_orientation(cov, vec, pointer);
  /*
#endif
*/
}

void Object_3d_Orientation_Zscale(const Object_3d *obj, double *vec, 
				  Object_3d_Orientation_Pointer_e pointer,
				  double z_scale)
{
#ifdef HAVE_LIBGSL
  gsl_matrix *gm = gsl_matrix_alloc(3, 3);
  Object_3d_Cov_Zscale(obj, gm->data, z_scale);

  cov_3d_orientation(gm, vec, pointer);

  gsl_matrix_free(gm);
#else
  double cov[9];
  Object_3d_Cov_Zscale(obj, cov, z_scale);
  cov_3d_orientation(cov, vec, pointer);
#endif
}

/* Obejct_3d_Has_Overlap(): To test if 2 objects have overlap.
 *
 * Args: obj1 - the first object;
 *       obj2 - the second object;
 *
 * Return: TRUE if they have overlap or FALSE.
 */
BOOL Object_3d_Has_Overlap(const Object_3d *obj1, const Object_3d *obj2)
{
  if ((obj1 == NULL) || (obj2 == NULL)) {
    return FALSE;
  }

  if (obj1 == obj2) {
    return TRUE;
  }

  int corner1[6], corner2[6];
  Object_3d_Range(obj1, corner1);
  Object_3d_Range(obj2, corner2);

  /* check the bounding boxes first */
  if ((corner1[3] < corner2[0]) || (corner1[4] < corner2[1]) || 
      (corner1[5] < corner2[2]) ||
      (corner2[3] < corner1[0]) || (corner2[4] < corner1[1]) || 
      (corner2[5] < corner1[2])) {
    return FALSE;
  }

  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj1, 1, offset);
  Object_3d *tmp_obj = Copy_Object_3d(obj2);

  Object_3d_Translate(tmp_obj, -offset[0], -offset[1], -offset[2]);
  
  //int indices[obj2->size];
  size_t* indices = (size_t*) malloc(sizeof(size_t) * obj2->size);
  Object_3d_Indices(tmp_obj, 
		    stack->width, stack->height, stack->depth, indices);
  Kill_Object_3d(tmp_obj);

  int i;
  for (i = 0; i < obj2->size; i++) {
    if (Is_Valid_Array_Index(indices[i])) {
      if (stack->array[indices[i]] > 0) {
        Kill_Stack(stack);
        return TRUE;
      }
    }
  }
  free(indices);
  
  Kill_Stack(stack);

  return FALSE;
}


Voxel_List* Object_3d_Central_Points(const Object_3d *obj)
{
  int corners[6];
  Object_3d_Range(obj, corners);
  
  Stack *stack = Make_Stack(GREY, corners[3] - corners[0] + 1, 
			    corners[4] - corners[1] +1, 
			    corners[5] - corners[2] + 1);

  Zero_Stack(stack);

  int i;
  int area = stack->width * stack->height;

  for (i = 0; i < obj->size; i++) {
    stack->array[obj->voxels[i][0] - corners[0] + 
		 (obj->voxels[i][1] - corners[1]) * stack->width + 
		 (obj->voxels[i][2] - corners[2]) * area] = 1;
  }
  
  //Stack_Not(stack, stack);
  Stack *dist = Stack_Bwdist_L_U16(stack, NULL, 0);
  Kill_Stack(stack);
  Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_ALTER1);

#ifdef _DEBUG_2
  Stack *dist_stack = Scale_Double_Stack((double *) dist->array, dist->width, 
					 dist->height, dist->depth, GREY);
  Write_Stack("../data/test2.tif", dist_stack);
  Kill_Stack(dist_stack);
#endif

  Kill_Stack(dist);

  Voxel_List *list = Stack_To_Voxel_List(seeds);
  Kill_Stack(seeds);
  
  Voxel_List *tmp = list;
  while (tmp != NULL) {
    tmp->data->x += corners[0];
    tmp->data->y += corners[1];
    tmp->data->z += corners[2];
    tmp = tmp->next;
  }

  return list;
}

double** Object_3d_Distmat(const Object_3d *obj, double **dist)
{
  int i, j;
  int n = obj->size;

  if (dist == NULL) {
    MALLOC_2D_ARRAY(dist, n, n, double, i);
  }
  
  for (i = 0; i < n; i++) {
    dist[i][i] = 0.0;
  }
  
  int dv[3];
  
  for (i = 1; i < n; i++) {
    for (j = 0; j < i; j++) {
      dv[0] = obj->voxels[i][0] - obj->voxels[j][0];
      dv[1] = obj->voxels[i][1] - obj->voxels[j][1];
      dv[2] = obj->voxels[i][2] - obj->voxels[j][2];
      dist[i][j] = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
      dist[j][i] = dist[i][j];
    }
  }
  
  return dist;
}

double** Object_3d_Distmat_S(const Object_3d *obj, double **dist, Stack *stack)
{
  int i, j;
  int n = obj->size;

  if (dist == NULL) {
    MALLOC_2D_ARRAY(dist, n, n, double, i);
  }
  
  for (i = 0; i < n; i++) {
    dist[i][i] = 0.0;
  }
  
  int dv[3];
  Voxel_t center;
  int area = stack->width * stack->height;
  int offset = 0;

  for (i = 1; i < n; i++) {
    for (j = 0; j < i; j++) {
      center[0] = (obj->voxels[i][0] + obj->voxels[j][0]) / 2;
      center[1] = (obj->voxels[i][1] + obj->voxels[j][1]) / 2;
      center[2] = (obj->voxels[i][2] + obj->voxels[j][2]) / 2;

      offset = center[0] + center[1] * stack->width + center[2] * area;
      

      dv[0] = obj->voxels[i][0] - obj->voxels[j][0];
      dv[1] = obj->voxels[i][1] - obj->voxels[j][1];
      dv[2] = obj->voxels[i][2] - obj->voxels[j][2];
      dist[i][j] = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
      if (stack->array[offset] == 0) {
	dist[i][j] *= 2.0;
      }
      dist[j][i] = dist[i][j];
    }
  }
  
  return dist;
}

void Object_3d_To_Mst(const Object_3d *obj, Graph *graph, Object_3d *centers)
{
  Voxel_List *clist = Object_3d_Central_Points(obj); /* clist created */
  Voxel_List_To_Object_3d(clist, centers); /* centers constructed */
  Voxel_List_Removeall(&clist); /* clist destroyed */

  double **dist = Object_3d_Distmat(centers, NULL); /* dist created */

  /* graph constructed */
  Construct_Graph(graph, centers->size, centers->size - 1, TRUE); 

  Graph_Mst_From_Adjmat(dist, graph->edges, graph->nvertex, &(graph->nedge),
			NULL);

  int i;
  for (i = 0; i < graph->nedge; i++) {
    graph->weights[i] = dist[graph->edges[i][0]][graph->edges[i][1]];
  }

  FREE_2D_ARRAY(dist, centers->size); /* dist  destroyed */
}

void Object_3d_To_Mst_S(const Object_3d *obj, Graph *graph, Object_3d *centers,
			Stack *stack)
{
  Voxel_List *clist = Object_3d_Central_Points(obj); /* clist created */
  Voxel_List_To_Object_3d(clist, centers); /* centers constructed */
  Voxel_List_Removeall(&clist); /* clist destroyed */

  double **dist = Object_3d_Distmat_S(centers, NULL, stack); /* dist created */

  /* graph constructed */
  Construct_Graph(graph, centers->size, centers->size - 1, TRUE); 

  Graph_Mst_From_Adjmat(dist, graph->edges, graph->nvertex, &(graph->nedge),
			NULL);

  int i;
  for (i = 0; i < graph->nedge; i++) {
    graph->weights[i] = dist[graph->edges[i][0]][graph->edges[i][1]];
  }

  FREE_2D_ARRAY(dist, centers->size); /* dist  destroyed */
}

void Object_3d_Central_Voxel(const Object_3d *obj, Voxel_t center)
{
  double centroid[3];
  Object_3d_Centroid(obj, centroid);
  int i;
  int minidx = 0;
  double mind = Geo3d_Dist_Sqr(obj->voxels[0][0], obj->voxels[0][1],
			       obj->voxels[0][2], centroid[0], centroid[1],
			       centroid[2]);
  for (i = 0; i < obj->size; i++) {
    double dist = Geo3d_Dist_Sqr(obj->voxels[i][0], obj->voxels[i][1],
				 obj->voxels[i][2], centroid[0], centroid[1],
				 centroid[2]);
    if (mind > dist) {
      minidx = i;
      mind = dist;
    }
  }

  center[0] = obj->voxels[minidx][0];
  center[1] = obj->voxels[minidx][1];
  center[2] = obj->voxels[minidx][2];

    
  if (mind > 5) {
    printf("%g\n", mind);
    printf("%d %d %d %g %g %g\n", center[0], center[1], center[2],
	   centroid[0], centroid[1], centroid[2]);
  }

}

Object_3d* Object_3d_Load_Csv(const char *filepath)
{
  TZ_ERROR(ERROR_PART_FUNC);
  return NULL;
}


void Object_3d_Bounding_Cuboid(const Object_3d *obj, coordinate_3d_t *axis_vec,
    double *min_span, double *max_span)
{
  TZ_WARN(ERROR_UNTESTED_FUNC);

  Object_3d_Orientation(obj, axis_vec[0], MAJOR_AXIS);
  Object_3d_Orientation(obj, axis_vec[2], PLANE_NORMAL); 
  Coordinate_3d_Cross(axis_vec[0], axis_vec[1], axis_vec[2]);

  double center[3];
  Object_3d_Centroid(obj, center);

  int i;
  for (i = 0; i < 3; i++) {
    min_span[i] = Infinity;
    max_span[i] = -Infinity;
  }

  int j;
  for (j = 0; j < 3; j++) {
    for (i = 0; i < obj->size; i++) {
      double proj = Geo3d_Dot_Product(axis_vec[j][0], axis_vec[j][1],
          axis_vec[j][2], 
          (double) obj->voxels[i][0] - center[0], 
          (double) obj->voxels[i][1] - center[1], 
          (double) obj->voxels[i][2] - center[2]);
      min_span[j] = dmin2(proj, min_span[j]);
      max_span[j] = dmax2(proj, max_span[j]);
    }
  }
}

static int voxel_compar(const void *p1, const void *p2)
{
  svoxel_t *v1 = (svoxel_t*) v1;
  svoxel_t *v2 = (svoxel_t*) p2;

  if (v1[2] < v2[2]) {
    return -1;
  } else if (v1[2] > v2[2]) {
    return 1;
  } else {
    if (v1[1] < v2[1]) {
      return -1;
    } else if (v1[1] > v2[1]) {
      return 1;
    } else {
      if (v1[0] < v2[0]) {
        return -1;
      } else if (v1[0] > v2[0]) {
        return 1;
      }
    }
  }

  return 0;
}

void Object_3d_Sort_By_Index(Object_3d *obj)
{
  TZ_WARN(ERROR_UNTESTED_FUNC);

  qsort(obj->voxels, obj->size, sizeof(Voxel_t), voxel_compar); 
}

void Object_3d_Csv_Fprint(Object_3d *obj, FILE *fp)
{
  TZ_ASSERT(fp != NULL, "Null file pointer");

  if (obj != NULL) {
    size_t i;
    for (i = 0; i < obj->size; i++) {
      fprintf(fp, "%d, %d, %d\n", obj->voxels[i][0], obj->voxels[i][1],
          obj->voxels[i][2]);
    }
  }
}

void Object_3d_Set_Voxel(Object_3d *obj, size_t index, int x, int y, int z)
{
  TZ_ASSERT(index < obj->size, "Invalid index");

  obj->voxels[index][0] = x;
  obj->voxels[index][1] = y;
  obj->voxels[index][2] = z;
}

static inline size_t rastercon_rsize(const RasterCon *region)
{ 
  return (sizeof(mylib_Indx_Type) * region->surflen); 
}
static inline size_t rastercon_hsize(const RasterCon *region)
{ return (sizeof(uint8) * (region->rastlen >> 1)); }

static inline size_t rastercon_dsize(const RasterCon *region)
{ return (sizeof(int) * region->ndims); }

static inline void allocate_rastercon_raster(
    RasterCon *rastercon, size_t rsize, char *routine)
{ 
  rastercon->raster = Guarded_Realloc(rastercon->raster, rsize,routine);
}

static inline void allocate_rastercon_ishole(
    RasterCon *rastercon, size_t hsize, char *routine)
{
  rastercon->ishole = Guarded_Malloc(hsize, routine);
}

static inline void allocate_rastercon_dims(
    RasterCon *rastercon, int dsize, char *routine)
{ 
  rastercon->dims = Guarded_Malloc(dsize, routine);
}

static inline void kill_rastercon(RasterCon *rastercon)
{ 
  if (rastercon->dims != NULL)
    free(rastercon->dims);
  if (rastercon->ishole != NULL)
    free(rastercon->ishole);
  if (rastercon->raster != NULL)
    free(rastercon->raster);

  free(rastercon);
}

static inline RasterCon *new_rastercon(size_t rsize, size_t hsize, int dsize, 
    char *routine)
{ 
  RasterCon *rastercon = 
    (RasterCon*) Guarded_Malloc(sizeof(RasterCon), "new_rastercon");
  rastercon->raster = NULL;
  rastercon->ishole = NULL;
  rastercon->dims = NULL;
  allocate_rastercon_raster(rastercon,rsize,"new_rastercon");
  allocate_rastercon_ishole(rastercon,hsize,routine);
  allocate_rastercon_dims(rastercon,dsize,routine);

  return (rastercon);
}

#define Region mylib_Region

static inline RasterCon *read_rastercon(FILE *input)
{ 
  char name[6];
  RasterCon *obj;
  RasterCon read;
  fread(name,6,1,input);
  if (strncmp(name,"Region",6) != 0)
    return (NULL);
  obj = new_rastercon(0,0,0,"Read_Region");
  if (obj == NULL) return (NULL);
  read = *obj;
  if (fread(obj,sizeof(RasterCon),1,input) == 0) goto error;
  obj->raster = read.raster;
  if (rastercon_rsize(obj) != 0) { 
    allocate_rastercon_raster(obj,rastercon_rsize(obj), "Read_Region");
    if (fread(obj->raster,(size_t) rastercon_rsize(obj),1,input) == 0) {
      goto error;
    }
  }
  obj->ishole = read.ishole;
  if (rastercon_hsize(obj) != 0) { 
    allocate_rastercon_ishole(obj,rastercon_hsize(obj),"Read_Region");
    if (fread(obj->ishole,(size_t) rastercon_hsize(obj),1,input) == 0) {
      goto error;
    }
  }
  obj->dims = read.dims;
  if (rastercon_dsize(obj) != 0) { 
    allocate_rastercon_dims(obj,rastercon_dsize(obj),"Read_Region");
    if (fread(obj->dims,(size_t) rastercon_dsize(obj),1,input) == 0) {
      goto error;
    }
  }

  return (obj);

error:
  kill_rastercon(obj);

  return (NULL);
}

Region *Read_Region(FILE *input)
{ 
  return ((Region *) read_rastercon(input)); 
}

void Print_Region(RasterCon *region)
{ 
  int i;

  printf("\nRegion [0..%llu..%llu] is2n = %d ndims = %d area = %llu\n",
      region->rastlen,region->surflen,region->iscon2n,region->ndims,region->area);
  for (i = 0; i < region->rastlen; i += 2)
    printf("  r %llu %llu (%d)\n",region->raster[i],region->raster[i+1],
        region->ishole[i>>1]);
  for (i = region->rastlen; i < region->surflen; i += 1)
    printf("  s %llu\n",region->raster[i]);
}

void Kill_Region(Region *region)
{ 
  kill_rastercon(((RasterCon *) region)); 
}

int Get_Region_Dimensionality(const Region *region)
{ 
  const RasterCon *cont = (RasterCon *) region;

  return (cont->ndims);
}

mylib_Dimn_Type *Get_Region_Dimensions(const Region *region)
{ 
  RasterCon *cont = (RasterCon *) region;
  return (cont->dims);
}

void Region_Stack_Size(const Region *region, int *width, int *height, int *depth)
{
  *width = 1;
  *height = 1;
  *depth = 1;

  int ndim = Get_Region_Dimensionality(region);
  mylib_Dimn_Type *dims = Get_Region_Dimensions(region);

  *width = dims[0];
  if (ndim > 1) {
    *height = dims[1];
  }

  if (ndim > 2) {
    *depth = dims[2];
  }
}

size_t Region_Size(const Region *region)
{
  size_t size = 0;
  mylib_Indx_Type *raster = region->raster;
  mylib_Indx_Type len = region->rastlen;
  mylib_Indx_Type i, v, w;
  for (i = 0; i < len; i += 2) {
#ifdef _DEBUG_2
    printf("%d - %d\n", (int)v, (int)w);
#endif
    v = raster[i];
    w = raster[i + 1];
    size += w - v + 1;
  }

  return size;
}

mylib_Prolog* Make_Prolog(int nstacks, int numchans)
{
  mylib_Prolog *prolog = (mylib_Prolog*) Guarded_Malloc(sizeof(mylib_Prolog),
      "Make_Prolog");
  
  Default_Prolog(prolog);

  prolog->nstacks = nstacks;
  prolog->numchans = numchans;
  prolog->ifiles = (mylib_string*) 
    Guarded_Malloc(sizeof(mylib_string) * nstacks, "Make_Prolog");
  prolog->echans = (int*) Guarded_Malloc(sizeof(int) * nstacks, "Make_Prolog");
  prolog->means =  (double*) Guarded_Malloc(sizeof(double) * numchans, 
      "Make_Prolog");
  prolog->sdevs =  (double*) Guarded_Malloc(sizeof(double) * numchans, 
      "Make_Prolog");


  return prolog;
}

void Default_Prolog(mylib_Prolog *prolog)
{
  prolog->cthresh = 0.0;
  prolog->ethresh = 0.0;
  prolog->sthresh = 0;
  prolog->nstacks = 0;
  prolog->numchans = 0;
  prolog->ifiles = NULL;
  prolog->vfile[0] = '\0';
  prolog->echans = NULL;
  prolog->means = NULL;
  prolog->sdevs = NULL;
} 

void Kill_Prolog(mylib_Prolog *prolog)
{
  if (prolog->ifiles != NULL) {
    safe_free((void**) &(prolog->ifiles), free);
  }

  if (prolog->echans != NULL) {
    safe_free((void**) &(prolog->echans), free);
  }

  if (prolog->means != NULL) {
    safe_free((void**) &(prolog->means), free);
  }

  if (prolog->sdevs != NULL) {
    safe_free((void**) &(prolog->sdevs), free);
  }

  free(prolog);
}

mylib_Prolog* Read_Prolog(FILE *input, int channelNumber, int mode,
    mylib_Prolog *prolog)
{
  int NStacks = 1;
  if (mode == 1) {
    fseek(input, sizeof(double), SEEK_CUR);
    fseek(input, sizeof(double), SEEK_CUR);
    fseek(input, sizeof(int), SEEK_CUR);

    fread(&NStacks,sizeof(int),1,input);
    rewind(input);
  }

  if (prolog == NULL) {
    prolog = Make_Prolog(NStacks, channelNumber);
  }

  TZ_ASSERT(prolog->nstacks == NStacks, "Unmatched stack number");
  /*
  static char *Type_Name[] = { "uint8", "uint16", "uint32", "uint64",
    "int8", "int16", "int32", "int64", "float32", "float64" };
    */

  int i, m;
  /*
  double CThresh, EThresh;
  int SThresh;
  */

  if (mode == 1) {
    TZ_ASSERT(prolog->numchans == channelNumber, "Unmatched channel number");

    fread(&prolog->cthresh,sizeof(double),1,input);
    fread(&prolog->ethresh,sizeof(double),1,input);
    fread(&prolog->sthresh,sizeof(int),1,input);

#ifdef _DEBUG_
    printf("\nParameters: c=%g e=%g s=%d\n",prolog->cthresh,prolog->ethresh,
        prolog->sthresh);  fflush(stdout);
#endif

    fread(&NStacks,sizeof(int),1,input);

    //IFiles = (char **) Guarded_Malloc(sizeof(char *)*NStacks,Program_Name());
    //EChans = (int *) Guarded_Malloc(sizeof(int)*NStacks,Program_Name());

    for (i = 0; i < NStacks; i++) { 
      fread(&m,sizeof(int),1,input);
      TZ_ASSERT(m < MAX_PATH_LENGTH, "Too long file name"); 
      prolog->ifiles[i][m] = '\0';
      fread(prolog->ifiles[i], sizeof(char), m, input);
      fread(prolog->echans, sizeof(int), 1, input);

      //IFiles[i] = (char *) Guarded_Malloc(sizeof(char)*(m+1),Program_Name());
      //fread(IFiles[i],sizeof(char),m,input);
      //IFiles[i][m] = '\0';
      //fread(EChans+i,sizeof(int),1,input);
      //
      //fseek(input, sizeof(char) * m, SEEK_CUR);
      //fseek(input, sizeof(int), SEEK_CUR);
    }
    fread(&m,sizeof(int),1,input);
    TZ_ASSERT(m < MAX_PATH_LENGTH, "Too long file name"); 
    prolog->vfile[m] = '\0';
    fread(prolog->vfile, sizeof(char), m, input);

    //VFile = (char *) Guarded_Malloc(sizeof(char)*(m+1),Program_Name());
    //fread(VFile,sizeof(char),m,input);
    //VFile[m] = '\0';
    //fseek(input, sizeof(char) * m, SEEK_CUR);


    //Means  = (double *) Guarded_Malloc(sizeof(double)*NumChans,Program_Name());
    //Sdevs  = (double *) Guarded_Malloc(sizeof(double)*NumChans,Program_Name());

    for (i = 0; i < prolog->numchans; i++) { 
      fread(prolog->means + i, sizeof(double), 1, input);
      fread(prolog->sdevs + i, sizeof(double), 1, input);
      //fseek(input, sizeof(double), SEEK_CUR);
      //fseek(input, sizeof(double), SEEK_CUR);
      //fread(Means+i,sizeof(double),1,input);
      //fread(Sdevs+i,sizeof(double),1,input);
    }
  } else {
    fseek(input, sizeof(double), SEEK_CUR);
    fseek(input, sizeof(double), SEEK_CUR);
    fseek(input, sizeof(int), SEEK_CUR);

    fseek(input, sizeof(int), SEEK_CUR);

    for (i = 0; i < NStacks; i++) {
      fread(&m,sizeof(int),1,input);
      fseek(input, sizeof(char) * m, SEEK_CUR);
      fseek(input, sizeof(int), SEEK_CUR);
    }
    fread(&m,sizeof(int),1,input);
    fseek(input, sizeof(char) * m, SEEK_CUR);

    int oldChannelNumber = prolog->numchans;
    prolog->numchans += channelNumber;

    prolog->means  = (double *) Guarded_Realloc(prolog->means, 
        sizeof(double)*prolog->numchans, "Make_Prolog");
    prolog->sdevs  = (double *) Guarded_Realloc(prolog->sdevs, 
        sizeof(double)*prolog->numchans,"Make_Prolog");

    for (i = oldChannelNumber; i < prolog->numchans; i++) { 
      fread(prolog->means + i, sizeof(double), 1, input);
      fread(prolog->sdevs + i, sizeof(double), 1, input);
      /*
      fseek(input, sizeof(double), SEEK_CUR);
      fseek(input, sizeof(double), SEEK_CUR);
      */
      /*
      fread(Means+i,sizeof(double),1,input);
      fread(Sdevs+i,sizeof(double),1,input);
      */
    }
  }

  return prolog;
}

static inline void write_rastercon(const RasterCon *rastercon, FILE *output)
{ fwrite("Region",6,1,output);
  fwrite(rastercon,sizeof(RasterCon),1,output);
  if (rastercon_rsize(rastercon) != 0)
    fwrite(rastercon->raster,(size_t) rastercon_rsize(rastercon),1,output);
  if (rastercon_hsize(rastercon) != 0)
    fwrite(rastercon->ishole,(size_t) rastercon_hsize(rastercon),1,output);
  if (rastercon_dsize(rastercon) != 0)
    fwrite(rastercon->dims,(size_t) rastercon_dsize(rastercon),1,output);
}

void Write_Region(const mylib_Region *region, FILE *output)
{ write_rastercon(((const RasterCon *) region),output); }

void Write_Prolog(const mylib_Prolog *prolog, FILE *output)
{
  int i, m;
  fwrite(&(prolog->cthresh), sizeof(double), 1, output);
  fwrite(&(prolog->ethresh), sizeof(double), 1, output);
  fwrite(&(prolog->sthresh), sizeof(int), 1, output);
  fwrite(&(prolog->nstacks), sizeof(int), 1, output);
  for (i = 0; i < prolog->nstacks; ++i) {
    m = strlen(prolog->ifiles[i]);
    fwrite(&m, sizeof(int), 1, output);
    fwrite(prolog->ifiles[i], sizeof(char), m, output);
    fwrite(prolog->echans + i, sizeof(int), 1, output); 
  }
  m = strlen(prolog->vfile);
  fwrite(&m, sizeof(int), 1, output);
  fwrite(prolog->vfile, sizeof(char), m, output);
  for (i = 0; i < prolog->numchans; ++i) {
    fwrite(prolog->means + i, sizeof(double), 1, output);
    fwrite(prolog->sdevs + i, sizeof(double), 1, output);
  }
}

void Print_Prolog(const mylib_Prolog *prolog)
{
  printf("\nParameters: c=%g e=%g s=%d\n",prolog->cthresh,prolog->ethresh,
      prolog->sthresh);

  printf("Number of stacks: %d\n", prolog->nstacks);
  int i;
  for (i = 0; i < prolog->nstacks; ++i) {
    printf("  %s: eliminated channel %d\n", prolog->ifiles[i], 
        prolog->echans[i]);
  }
  printf("8-bit scaled image: %s\n", prolog->vfile);
  printf("Number of channels: %d\n", prolog->numchans);

  for (i = 0; i < prolog->numchans; ++i) {
    printf("  mean: %g; sdev: %g\n", prolog->means[i], prolog->sdevs[i]);
  }
}

Object_3d* Region_To_Object_3d(const Region *region)
{
  size_t region_size = Region_Size(region);
  Object_3d *obj = Make_Object_3d(region_size, 0);

  int width, height, depth;
  Region_Stack_Size(region, &width, &height, &depth);

  size_t i;
  int x, y, z;
  size_t obj_index = 0;
  for (i = 0; i < region->rastlen; i += 2) {
    mylib_Indx_Type v = region->raster[i];
    mylib_Indx_Type w = region->raster[i + 1];
    mylib_Indx_Type s;
    for (s = v; s <= w; ++s) {
      Stack_Util_Coord(s, width, height, &x, &y, &z);
      Object_3d_Set_Voxel(obj, obj_index++, x, y, z);
    }
  }

  return obj;
}

Stack* Region_To_Stack(mylib_Region *region, Stack *out)
{
  if (region == NULL) {
    return NULL;
  }

  int width, height, depth;
  Region_Stack_Size(region, &width, &height, &depth);
  if (out == NULL) {
    out = Make_Stack(GREY, width, height, depth); 
  }

  Zero_Stack(out);

  size_t i;
  for (i = 0; i < region->rastlen; i += 2) {
    mylib_Indx_Type v = region->raster[i];
    mylib_Indx_Type w = region->raster[i + 1];
    mylib_Indx_Type s;
    for (s = v; s <= w; ++s) {
      out->array[s] = 1;
    }
  }

  return out;
}

void Region_Label_Stack(mylib_Region *region, uint16_t label, Stack *out)
{
  Image_Array ima;
  ima.array = out->array;

  size_t i;
  for (i = 0; i < region->rastlen; i += 2) {
    mylib_Indx_Type v = region->raster[i];
    mylib_Indx_Type w = region->raster[i + 1];
    mylib_Indx_Type s;
    for (s = v; s <= w; ++s) {
      if (out->kind == GREY) {
        ima.array8[s] = label % 256;
      } else {
        ima.array16[s] = label;
      }
    }
  }
}

Rgb_Color Region_Average_Color(mylib_Region *region, const Mc_Stack *stack)
{
  double chv[] = {0, 0, 0};

  TZ_ASSERT(stack->kind == GREY || stack->kind == GREY16, "Unsupported kind");
  
  Image_Array ima;
  ima.array = stack->array;

  size_t nvoxel = Mc_Stack_Voxel_Number(stack);

  int ch;
  size_t i;
  for (ch = 0; ch < stack->nchannel; ++ch) {
    uint8_t *array = ima.array8 + nvoxel * ch;
    uint16_t *array16 = ima.array16 + nvoxel * 2 * ch;

    int count = 0;
    for (i = 0; i < region->rastlen; i += 2) {
      mylib_Indx_Type v = region->raster[i];
      mylib_Indx_Type w = region->raster[i + 1];
      mylib_Indx_Type s;
      for (s = v; s <= w; ++s) {
        if (stack->kind == GREY) {
          chv[ch] += array[s];
        } else {
          chv[ch] += array16[s];
        }
        ++count;
      }
    }
    chv[ch] /= count;
  }
  
  double maxv = chv[0];
  for (ch = 1; ch < stack->nchannel; ++ch) {
    if (maxv < chv[ch]) {
      maxv = chv[ch];
    }
  }

  if (maxv > 255.0) {
    double scale = 255.0 / maxv;
    for (ch = 0; ch < stack->nchannel; ++ch) {
      chv[ch] *= scale;
    }
  }

  Rgb_Color color;
  color.r = iround(chv[0]);
  color.g = 0;
  color.b = 0;
  if (stack->nchannel > 1) {
    color.g = iround(chv[1]);
  }
  if (stack->nchannel > 2) {
    color.b = iround(chv[2]);
  }

  return color;
}

void Region_Draw_Stack(mylib_Region *region, Stack *canvas, 
    uint8_t r, uint8_t g, uint8_t b)
{
  TZ_ASSERT(canvas->kind == COLOR, "Must be color stack");

  color_t *array = (color_t*) canvas->array;

  size_t i;
  for (i = 0; i < region->rastlen; i += 2) {
    mylib_Indx_Type v = region->raster[i];
    mylib_Indx_Type w = region->raster[i + 1];
    mylib_Indx_Type s;
    for (s = v; s <= w; ++s) {
      array[s][0] = r;
      array[s][1] = g;
      array[s][2] = b;
    }
  }
}

BOOL Region_Same_Dimensions(const mylib_Region *r1, const mylib_Region *r2)
{
  if (Get_Region_Dimensionality(r1) != Get_Region_Dimensionality(r2)) {
    return FALSE;
  }

  int ndim = Get_Region_Dimensionality(r1);

  mylib_Dimn_Type *dims1 = Get_Region_Dimensions(r1);
  mylib_Dimn_Type *dims2 = Get_Region_Dimensions(r2);

  int i;
  for (i = 0; i < ndim; ++i) {
    if (dims1[i] != dims2[i]) {
      return FALSE;
    }
  }

  return TRUE;
}

void merge_rastercon(RasterCon *master, const RasterCon *slave)
{
  if (slave == NULL) {
    return;
  }

  TZ_ASSERT(master != NULL, "Null pointer.");
  TZ_ASSERT(Region_Same_Dimensions((const mylib_Region*)master, 
        (const mylib_Region*) slave) == TRUE, "Unmatched dimensions");

  if (slave->surflen > 0) {
    int new_rastlen = master->rastlen + slave->rastlen;
    int newlen = master->surflen + slave->surflen;
    master->raster = Guarded_Realloc(master->raster, 
        sizeof(mylib_Indx_Type) * newlen, Program_Name());
    int master_non_raster_len = master->surflen - master->rastlen;
    int slave_non_raster_len = slave->surflen - slave->rastlen;
    if (master_non_raster_len > 0) {
      /* Move non-raster pixels */
      memmove(master->raster + new_rastlen, 
          master->raster + master->rastlen, 
          sizeof(mylib_Indx_Type) * master_non_raster_len);
    }
    /* Copy raster */
    memcpy(master->raster + master->rastlen, slave->raster, 
        sizeof(mylib_Indx_Type) * slave->rastlen);
    if (slave_non_raster_len > 0) {
    /* Copy non-raster pixels */
    memcpy(master->raster + master->surflen + slave->rastlen,
        slave->raster + slave->rastlen,
        sizeof(mylib_Indx_Type) * slave_non_raster_len);
    }

    master->surflen = newlen;
    master->rastlen = new_rastlen;

    /* copy holes*/
    if (rastercon_hsize(slave) > 0) {
      size_t hsize = rastercon_hsize(slave) + rastercon_hsize(master);
      master->ishole = Guarded_Realloc(master->ishole, hsize, Program_Name());
      memcpy(master->ishole + rastercon_hsize(master),
          slave->ishole, rastercon_hsize(slave));
    }
  }
}

void Region_Merge(mylib_Region *master, const mylib_Region *slave)
{
  merge_rastercon((RasterCon*)master, (const RasterCon*) slave);
  /*
  if (slave == NULL) {
    return;
  }

  TZ_ASSERT(master != NULL, "Null pointer.");
  TZ_ASSERT(Region_Same_Dimensions(master, slave) == TRUE, "Unmatched dimensions");

  if (slave->rastlen > 0) {
    int newlen = master->rastlen + slave->rastlen;
    master->raster = Guarded_Realloc(master->raster, 
        sizeof(mylib_Indx_Type) * newlen, "Region_Merge");
    memcpy(master->raster + master->rastlen, slave->raster, 
        sizeof(mylib_Indx_Type) * slave->rastlen);
    master->rastlen = newlen;
  }
  */
}

void Region_Merge_M(mylib_Region *master, mylib_Region **slave,
    int nslave)
{
  int i;
  for (i = 0; i < nslave; ++i) {
    Region_Merge(master, slave[i]);
#ifdef _DEBUG_2
    printf("%d %d %d\n", i, (int) Region_Size(slave[i]), (int) Region_Size(master));
#endif
  }
}

Mc_Stack* Region_Mip(const mylib_Region *region, const Mc_Stack *stack)
{
  Object_3d *obj = Region_To_Object_3d(region);

  Mc_Stack *out = Make_Mc_Stack(stack->kind, stack->width, stack->height, 1, 
      stack->nchannel);
  Mc_Stack_Set_Zero(out);

  size_t i;
  int ch;
  for (ch = 0; ch < stack->nchannel; ++ch) {
    Stack stack_view = Stack_View_Mc_Stack(stack, ch);
    Stack out_view = Stack_View_Mc_Stack(out, ch);

    for (i = 0; i < obj->size; ++i) {
      int x = obj->voxels[i][0];
      int y = obj->voxels[i][1];
      int z = obj->voxels[i][2];
      double v = Get_Stack_Pixel(&stack_view, x, y, z, 0);
      double maxv = Get_Stack_Pixel(&out_view, x, y, 0, 0);
      if (v > maxv) {
        Set_Stack_Pixel(&out_view, x, y, 0, 0, v);
      }
    }
  }

  return out;
}
