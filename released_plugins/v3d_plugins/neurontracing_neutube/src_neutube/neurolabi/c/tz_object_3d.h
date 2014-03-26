/**@file tz_object_3d.h
 * @brief 3d object
 * @author Ting Zhao
 * @date 26-Nov-2007
 */

#ifndef _TZ_OBJECT_3D_H_
#define _TZ_OBJECT_3D_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"
#include "tz_tvoxel.h"
#include "tz_voxel_linked_list.h"
#include "tz_graph_defs.h"
#include "tz_coordinate_3d.h"
#include "tz_mc_stack.h"
#include "tz_color.h"
#include "tz_constant.h"
//#include "tz_swc_tree.h"

__BEGIN_DECLS

/*
 * Definition of a 3D object. <conn> is the connectivity of the stack 
 * neighborhood system (see tz_stack_neighborhood.h for more information). If 
 * <conn> is 0, there is no constraint for voxel connection.
 */
#ifdef _MSC_VER
typedef struct __Object_3d {
#else
typedef struct _Object_3d {
#endif
  size_t size; /* total number of voxels*/
  int conn; /* How the voxels are connected. */
  Voxel_t *voxels; /* voxel array */
} Object_3d;

typedef Object_3d* Object_3d_P;

#define OBJECT_3D_SIZE(obj) (obj->size)
#define OBJECT_3D_X(obj, index) (obj->voxels[index][0])
#define OBJECT_3D_Y(obj, index) (obj->voxels[index][1])
#define OBJECT_3D_Z(obj, index) (obj->voxels[index][2])

size_t Object_3d_Asize(size_t size);

Object_3d* New_Object_3d();
void Default_Object_3d();

Object_3d *Make_Object_3d(size_t size, int conn);
void Set_Object_3d_Asize(Object_3d *obj, size_t asize);

Object_3d *Copy_Object_3d(const Object_3d *obj);
void   Pack_Object_3d(Object_3d *obj);
void   Free_Object_3d(Object_3d *obj);
void   Kill_Object_3d(Object_3d *obj);
void   Reset_Object_3d();
int    Object_3d_Usage();

int Save_Object_3d(Object_3d *obj, char *file_path);
Object_3d* Load_Object_3d(char *file_path);

void Print_Object_3d(Object_3d *obj);
void Print_Object_3d_Info(Object_3d *obj);

void Object_3d_Set_Voxel(Object_3d *obj, size_t index, int x, int y, int z);  

/*
 * Object_3d_Range() retrieves the bounding box of <obj>. 
 * (<corners[0]>, <corners[1]>, <corners[2]>) is the left-top corner and
 * (<corners[3]>, <corners[4]>, <corners[5]>) is the right-bottom corner.
 */
void Object_3d_Range(const Object_3d *obj, int *corners);

/*
 * Object_3d_Indices() get the indices of the object voxels in a stack with
 * size <width> x <height> x <depth>. The result is stored in indices, which
 * must be initialized to be long enough to hold the data.
 */
void Object_3d_Indices(const Object_3d *obj, int width, int height, int depth,
		       size_t *indices);
/*
 * Object_3d_Translate() translates <obj> by (<x>, <y>, <z>).
 */
void Object_3d_Translate(Object_3d *obj, int x, int y, int z);

/*
 * Object_3d_Centroid() calculates the centroid of <obj> and stores the result
 * in <pos>.
 *
 * Object_3d_Centroid() calculates the covariance matrix of <obj> and stores
 * the result in <cov>.
 *
 * Object_3d_Cov_Zscale() calculates the covariance matrix by considering
 * z scale.
 */
void Object_3d_Centroid(const Object_3d *obj, double *pos);
void Object_3d_Cov(const Object_3d *obj, double *cov);
void Object_3d_Cov_Zscale(const Object_3d *obj, double *cov, double z_scale);

typedef enum {
  MAJOR_AXIS, PLANE_NORMAL
} Object_3d_Orientation_Pointer_e;

/*
 * Object_3d_Orientation() calculates the orientation of <obj>. The orientation
 * can be the orientation of the major axis (<pointer> is MAJOR_AXIS), or the
 * normal of the major plane (<pointer> is PLANE_NORMAL), which is useful for
 * flat objects.
 *
 * Object_3d_Orientation_Zscale() is the z-scale verion of 
 * Object_3d_Orientation().
 */
void Object_3d_Orientation(const Object_3d *obj, double *vec, 
			   Object_3d_Orientation_Pointer_e pointer);
void Object_3d_Orientation_Zscale(const Object_3d *obj, double *vec, 
				  Object_3d_Orientation_Pointer_e pointer,
				  double z_scale);

/*
 * Object_3d_Has_Overlap() returns TRUE if <objs1> and <obj2> has any common
 * voxel; otherwise it returns FALSE.
 */
BOOL Object_3d_Has_Overlap(const Object_3d *obj1, const Object_3d *obj2);

/*
 * Object_3d_Central_Points() extracts central points from an object.
 */
Voxel_List* Object_3d_Central_Points(const Object_3d *obj);

double** Object_3d_Distmat(const Object_3d *obj, double **dist);
double** Object_3d_Distmat_S(const Object_3d *obj, double **dist, Stack *stack);

void Object_3d_To_Mst(const Object_3d *obj, Graph *graph, Object_3d *centers);
void Object_3d_To_Mst_S(const Object_3d *obj, Graph *graph, Object_3d *centers,
			Stack *stack);

void Object_3d_Central_Voxel(const Object_3d *obj, Voxel_t center);

Object_3d* Object_3d_Load_Csv(const char *filepath);

void Object_3d_Bounding_Cuboid(const Object_3d *obj, coordinate_3d_t *axis_vec,
    double *min_span, double *max_span);

void Object_3d_Sort_By_Index(Object_3d *obj);

void Object_3d_Csv_Fprint(Object_3d *obj, FILE *fp);

//Swc_Tree* Object_3d_To_Tube(const Object_3d *obj);
    
/* Routines for handling mylib Region objects */

typedef int32_t mylib_Dimn_Type;
typedef int64_t mylib_Indx_Type;
typedef int64_t mylib_Size_Type;
typedef int64_t mylib_Offs_Type;
typedef int mylib_boolean;

typedef struct {
  mylib_Size_Type rastlen;
  mylib_Indx_Type *raster;
  mylib_Size_Type surflen;
  mylib_boolean iscon2n;
  int ndims;
  mylib_Dimn_Type *dims;
  uint8_t *ishole;
  mylib_Size_Type area;
} RasterCon;

typedef struct { 
  mylib_Size_Type  rastlen;  // Length of the raster pair vector (always even)
  mylib_Indx_Type *raster;   // Pixels are [raster[i],raster[i+1]] for i in [0,rastlen) and i even
} mylib_Region; 

typedef char mylib_string[MAX_PATH_LENGTH];

typedef struct {
  double cthresh;
  double ethresh;
  int sthresh;
  int nstacks;
  int numchans;
  mylib_string *ifiles;
  mylib_string vfile;
  int *echans;
  double *means;
  double *sdevs;
} mylib_Prolog;

mylib_Prolog* Make_Prolog(int nstacks, int numchans);
mylib_Prolog* Read_Prolog(FILE *input, int channelNumber, int mode,
    mylib_Prolog *prolog);
void Kill_Prolog(mylib_Prolog *prolog);
void Default_Prolog(mylib_Prolog *prolog);
void Print_Prolog(const mylib_Prolog *prolog);

int Get_Region_Dimensionality(const mylib_Region *region);
mylib_Dimn_Type *Get_Region_Dimensions(const mylib_Region *region);
BOOL Region_Same_Dimensions(const mylib_Region *r1, const mylib_Region *r2);
size_t Region_Size(const mylib_Region *region);
void Region_Stack_Size(const mylib_Region *region, int *width, int *height, int *depth);
void Kill_Region(mylib_Region *region);
void Print_Region(RasterCon *region);
mylib_Region *Read_Region(FILE *input);
mylib_Prolog* Read_Prolog(FILE *input, int channelNumber, int mode,
    mylib_Prolog *prolog);
void Write_Prolog(const mylib_Prolog *prolog, FILE *output);
void Write_Region(const mylib_Region *region, FILE *output);

/**@brief Merge two regions
 *
 */
void Region_Merge(mylib_Region *master, const mylib_Region *slave);
void Region_Merge_M(mylib_Region *master, mylib_Region **slave,
    int nslave);

Object_3d* Region_To_Object_3d(const mylib_Region *region);

/**@brief Convert a region to a binary stack
 * 
 * <out> must be GREY type if it is not NULL.
 */
Stack* Region_To_Stack(mylib_Region *region, Stack *out);

/**@brief Convert a region to a binary stack
 * 
 * <out> must be GREY type or GREY16 type.
 */
void Region_Label_Stack(mylib_Region *region, uint16_t label, Stack *out);
void Region_Draw_Stack(mylib_Region *region, Stack *canvas, 
    uint8_t r, uint8_t g, uint8_t b);

Rgb_Color Region_Average_Color(mylib_Region *region, const Mc_Stack *stack);

Mc_Stack* Region_Mip(const mylib_Region *region, const Mc_Stack *stack);

__END_DECLS

#endif
