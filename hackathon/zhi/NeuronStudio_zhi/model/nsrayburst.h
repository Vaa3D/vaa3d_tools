#ifndef __NS_MODEL_RAYBURST_H__
#define __NS_MODEL_RAYBURST_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsbytearray.h>
#include <math/nsvector.h>
#include <math/nsmatrix.h>
#include <math/nstriangle.h>
#include <math/nsinterpolate.h>
#include <math/nsray.h>
#include <math/nsrandom.h>
#include <image/nspixels.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>

NS_DECLS_BEGIN


/* IMPORTANT: The initialization function must be called
	before any other functions can be used in this module.
	Call the finalize function to cleanup memory. */
NS_IMPEXP NsError ns_rayburst_init( const nschar *startup_directory );
NS_IMPEXP void ns_rayburst_finalize( void );


typedef enum
	{
	NS_RAYBURST_KERNEL_UNKNOWN,
	NS_RAYBURST_KERNEL_STD,
	NS_RAYBURST_KERNEL_2D,
	NS_RAYBURST_KERNEL_2D_VIZ,
	NS_RAYBURST_KERNEL_SUB_VIZ,
	NS_RAYBURST_KERNEL_VIZ,
	NS_RAYBURST_KERNEL_SUPER_VIZ,
	NS_RAYBURST_KERNEL_HYPER_VIZ,
	NS_RAYBURST_KERNEL_SINGLE,
	NS_RAYBURST_KERNEL_SINGLE_3D,
	NS_RAYBURST_KERNEL_TARGET,

	_NS_RAYBURST_KERNEL_NUM_TYPES
	}
	NsRayburstKernelType;

NS_IMPEXP const nschar* ns_rayburst_kernel_type_to_string( NsRayburstKernelType type );


typedef enum
	{
	NS_RAYBURST_RADIUS_NONE,
	NS_RAYBURST_RADIUS_MLBD,
	NS_RAYBURST_RADIUS_MPED,

	_NS_RAYBURST_RADIUS_NUM_TYPES
	}
	NsRayburstRadiusType;

NS_IMPEXP const nschar* ns_rayburst_radius_type_to_string( NsRayburstRadiusType type );


typedef enum
	{
	NS_RAYBURST_INTERP_BILINEAR,
	NS_RAYBURST_INTERP_TRILINEAR,

	_NS_RAYBURST_INTERP_NUM_TYPES
	}
	NsRayburstInterpType;

NS_IMPEXP const nschar* ns_rayburst_interp_type_to_string( NsRayburstInterpType type );


typedef nsboolean ( *NsRayburstVoxelFunc )
	(
	nsconstpointer,
	nsint,
	nsint,
	nsint,
	nsulong*
	);


/* Internal. DO NOT USE! */
typedef struct _NsRayburstPlane
	{
	NsVector3f  d_delta;
	NsVector3f  d;
	}
	NsRayburstPlane;


struct _NsRayburst;
typedef struct _NsRayburst NsRayburst;


typedef void ( *NsRayburstRenderFunc )( const NsRayburst*, nspointer );


struct _NsRayburst
	{
	NsRayburstKernelType   kernel_type;
	NsRayburstRadiusType   radius_type;
	NsRayburstInterpType   interp_type;
	NsByteArray            samples;
	NsVoxelBuffer          voxel_buffer;
	NsVector3f             origin;
	nsfloat                origin_value;
	NsVector3f             mlbd_fwd_intercept;
	NsVector3f             mlbd_rev_intercept;
	NsVector3f             single_vector;
	nsconstpointer         planes;
	nsconstpointer         vectors;
	nsconstpointer         triangles;
	nsconstpointer         user_data;
	nsfloat                radius;
	nsfloat                threshold;
	const NsVoxelInfo     *voxel_info;
	nssize                 done_count;
	nssize                 num_triangles;
	nssize                 num_samples;
	nssize                 mlbd_sample;
	nssize                 mlbd_count;
	NsRayburstVoxelFunc    voxel_func;
	NsFunc                 kernel_func;
	NsFunc                 radius_func;
	NsFunc                 advance_func;
	NsRayburstRenderFunc   render_func;
	nsboolean              limit_rays;
	nsdouble               max_ray_distance_sq;
	nsdouble               volume;
	nsdouble               surface_area;
	NsRayburstPlane        single_plane;
	nsboolean              reverse_rays;
	NsVector3f             target;
	nsboolean              target_was_hit;
	nsfloat                target_distance;
	nsint                  width;
	nsint                  height;
	nsint                  length;
	nsboolean              dark_start;
	nsboolean              allow_dark_start;
	};



/* Encapsulates construction/destruction of a
	rayburst (targeting) object. */
NS_IMPEXP nsboolean ns_rayburst_targeter
	(
	const NsVector3f   *origin,
	const NsVector3f   *target,
	nsfloat             threshold,
	const NsImage      *image,
	const NsVoxelInfo  *voxel_info
	);



/* NOTE: Pass NULL for 'voxel_func' for
	direct lookup into the 'image'. */
NS_IMPEXP NsError ns_rayburst_construct
	(
	NsRayburst            *rayburst,
	NsRayburstKernelType   kernel_type,
	NsRayburstRadiusType   radius_type,
	NsRayburstInterpType   interp_type,
	const NsImage         *image,
	const NsVoxelInfo     *voxel_info,
	NsRayburstVoxelFunc    voxel_func
	);

NS_IMPEXP void ns_rayburst_destruct( NsRayburst *rayburst );


/* This sets whether the rayburst is allowed to work from within an area
	that is darker than the threshold. */
NS_IMPEXP void ns_rayburst_allow_dark_start( NsRayburst *rayburst, nsboolean b );


NS_IMPEXP nsconstpointer ns_rayburst_get_user_data( const NsRayburst *rayburst );
NS_IMPEXP void ns_rayburst_set_user_data( NsRayburst *rayburst, nsconstpointer user_data );

NS_IMPEXP nsfloat ns_rayburst_get_threshold( const NsRayburst *rayburst );
NS_IMPEXP void ns_rayburst_set_threshold( NsRayburst *rayburst, nsfloat threshold );


/* If 'yes_or_no' is true, then 'may_ray_distance_sq' should be set to an appropriate value. Note that
	this value is a distance squared!!! */
NS_IMPEXP void ns_rayburst_limit_rays( NsRayburst *rayburst, nsboolean yes_or_no, nsdouble max_ray_distance_sq );


NS_IMPEXP void ns_rayburst_init_single( NsRayburst *rayburst, nsfloat dx, nsfloat dy );
NS_IMPEXP void ns_rayburst_init_single_ex( NsRayburst *rayburst, nsfloat dx, nsfloat dy, nsfloat dz );


/* The 'target' is where the single ray will "aim" at. (in 3D). After calling ns_rayburst()
	access the ns_rayburst_target_was_hit() function to see if the target was hit. */
NS_IMPEXP void ns_rayburst_init_target( NsRayburst *rayburst, const NsVector3f *target );


/* NOTE: Call ns_rayburst_set_threshold() prior to this function.  */
NS_IMPEXP void ns_rayburst( NsRayburst *rayburst, const NsPoint3f *origin );


#define ns_rayburst_target_was_hit( rayburst )\
	( (rayburst)->target_was_hit )


#define ns_rayburst_num_triangles( rayburst )\
	( (rayburst)->num_triangles )

#define ns_rayburst_triangles( rayburst )\
	( ( const NsIndexTriangleus* )(rayburst)->triangles )


#define ns_rayburst_num_vectors( rayburst )\
	ns_rayburst_num_samples( (rayburst) )

#define ns_rayburst_vectors( rayburst )\
	( ( const NsVector3f* )(rayburst)->vectors )


#define ns_rayburst_kernel_type( rayburst )\
	( (rayburst)->kernel_type )

NS_IMPEXP const NsVector3f* ns_rayburst_kernel_type_vectors( NsRayburstKernelType type );
NS_IMPEXP const NsIndexTriangleus* ns_rayburst_kernel_type_triangles( NsRayburstKernelType type );

NS_IMPEXP nssize ns_rayburst_kernel_type_num_vectors( NsRayburstKernelType type );
NS_IMPEXP nssize ns_rayburst_kernel_type_num_triangles( NsRayburstKernelType type );


/* Returns 2 for 2D, 3 for 3D, etc... */
NS_IMPEXP nssize ns_rayburst_kernel_type_dimension( NsRayburstKernelType type );


#define ns_rayburst_num_samples( rayburst )\
	( (rayburst)->num_samples )


NS_IMPEXP void ns_rayburst_compute_volume_and_surface_area
	(
	NsRayburst  *rayburst,
	nsfloat      stretch_factor
	);


/* The following functions are only meaningful after
	a call to ns_rayburst(). */

#define ns_rayburst_radius( rayburst )\
	( (rayburst)->radius )

#define ns_rayburst_volume( rayburst )\
	( (rayburst)->volume )

#define ns_rayburst_surface_area( rayburst )\
	( (rayburst)->surface_area )


/* Any of 'center', 'fwd', 'rev', or 'radius' can be
	NULL if not needed. */
NS_IMPEXP void ns_rayburst_mlbd
	(
	const NsRayburst  *rayburst,
	NsVector3f        *center,
	NsVector3f        *fwd,
	NsVector3f        *rev,
	nsfloat           *radius
	);


/* IMPORTANT: The passed arrays must be of length
	ns_rayburst_num_samples(). */


/* Values for ns_rayburst_sample_distance(s)()
	'which' parameter. */
enum
	{
	NS_RAYBURST_SAMPLE_DISTANCE_FORWARD,
	NS_RAYBURST_SAMPLE_DISTANCE_REVERSE,
	NS_RAYBURST_SAMPLE_DISTANCE_AVERAGE
	};

NS_IMPEXP void ns_rayburst_sample_distances
	(
	const NsRayburst  *rayburst,
	nsenum             which,
	nsfloat           *distances
	);

NS_IMPEXP nsfloat ns_rayburst_sample_distance
	(
	const NsRayburst  *rayburst,
	nssize             index,
	nsenum             which
	);

/* Pass true for 'forward' for end points of the
	samples in the forward direction, else false
	for reverse direction. */
NS_IMPEXP void ns_rayburst_sample_points
	(
	const NsRayburst  *rayburst,
	nsboolean          forward,
	NsPoint3f         *points
	);


#define NS_RAYBURST_STD_NUM_SAMPLES  64
#define NS_RAYBURST_STD_MLBD_SAMPLE  15
#define NS_RAYBURST_STD_MLBD_COUNT   ( NS_RAYBURST_STD_MLBD_SAMPLE + 1 )


#define NS_RAYBURST_2D_NUM_SAMPLES  64


#define NS_RAYBURST_2D_VIZ_NUM_SAMPLES  64


#define NS_RAYBURST_SUB_VIZ_NUM_SAMPLES    42
#define NS_RAYBURST_SUB_VIZ_NUM_TRIANGLES  80


#define NS_RAYBURST_VIZ_NUM_SAMPLES    162
#define NS_RAYBURST_VIZ_NUM_TRIANGLES  320


#define NS_RAYBURST_SUPER_VIZ_NUM_SAMPLES      2562
#define NS_RAYBURST_SUPER_VIZ_NUM_TRIANGLES    5120
#define NS_RAYBURST_SUPER_VIZ_RECURSIVE_DEPTH  4
#define NS_RAYBURST_SUPER_VIZ_INIT_TYPE        NS_RAYBURST_GENERATE_3D_ICOSAHEDRON
#define NS_RAYBURST_SUPER_VIZ_ROTATE           NS_TRUE


#define NS_RAYBURST_HYPER_VIZ_NUM_SAMPLES    10242
#define NS_RAYBURST_HYPER_VIZ_NUM_TRIANGLES  20480


#define NS_RAYBURST_SINGLE_NUM_SAMPLES  1

#define NS_RAYBURST_TARGET_NUM_SAMPLES  1


/* Internal. DO NOT USE! */

typedef struct _NsRayburstRay
	{
	NsRay3f     R;
	nsfloat     plane_d[3];
	nsfloat     plane_d_delta[3];
	nsboolean   done;
	NsVector3f  prev;
	NsVector3f  curr;
	nsfloat     prev_value;
	nsfloat     curr_value;
	nsfloat     distance;
	}
	NsRayburstRay;


typedef struct _NsRayburstSample
	{
	NsRayburstRay  fwd;
	NsRayburstRay  rev;
	nsfloat        distance;
	nsboolean      done;
	}
	NsRayburstSample;


NS_IMPEXP const NsRayburstSample* _ns_rayburst_samples( const NsRayburst *rayburst );


NS_IMPEXP void ns_rayburst_generate_2d
	(
	NsVector3f       *vectors,
	NsRayburstPlane  *planes,
	nssize            num_samples,
	nsboolean         do_360
	);


#define NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH  5

enum
	{
	NS_RAYBURST_GENERATE_3D_ICOSAHEDRON,
	NS_RAYBURST_GENERATE_3D_OCTAHEDRON
	};

/* 'init_type' is one of the above enumerated values. */
NS_IMPEXP nssize ns_rayburst_generate_3d_num_samples( nssize recursive_depth, nsint init_type );
NS_IMPEXP nssize ns_rayburst_generate_3d_num_triangles( nssize recursive_depth, nsint init_type );

NS_IMPEXP void ns_rayburst_generate_3d
	(
	NsVector3f         *vectors,
	NsRayburstPlane    *planes,
	NsIndexTriangleus  *triangles,
	nssize              num_samples,
	nssize              num_triangles,
	nssize              recursive_depth,
	nsint               init_type,
	nsboolean           rotate
	);

#ifdef NS_DEBUG

NS_IMPEXP void ns_rayburst_generate_2d_output
	(
	const NsVector3f       *vectors,
	const NsRayburstPlane  *planes,
	nssize                  num_samples
	);

NS_IMPEXP void ns_rayburst_generate_3d_output
	(
	const NsVector3f         *vectors,
	const NsRayburstPlane    *planes,
	const NsIndexTriangleus  *triangles,
	nssize                    num_samples,
	nssize                    num_triangles
	);

#endif/* NS_DEBUG */

NS_DECLS_END

#endif/* __NS_MODEL_RAYBURST_H__ */
