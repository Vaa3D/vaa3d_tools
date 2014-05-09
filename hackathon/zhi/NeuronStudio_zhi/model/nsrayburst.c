#include "nsrayburst.h"


const nschar* ns_rayburst_kernel_type_to_string( NsRayburstKernelType type )
	{
	NS_PRIVATE const nschar* _ns_rayburst_kernel_type_strings[ _NS_RAYBURST_KERNEL_NUM_TYPES ] =
		{
		NULL,
		"3D-radius-" NS_STRINGIZE( NS_RAYBURST_STD_NUM_SAMPLES ),
		"2D-radius-" NS_STRINGIZE( NS_RAYBURST_2D_NUM_SAMPLES ),
		"2D-surface-" NS_STRINGIZE( NS_RAYBURST_2D_VIZ_NUM_SAMPLES ),
		"3D-surface-" NS_STRINGIZE( NS_RAYBURST_SUB_VIZ_NUM_SAMPLES ),
		"3D-surface-" NS_STRINGIZE( NS_RAYBURST_VIZ_NUM_SAMPLES ),
		"3D-surface-" NS_STRINGIZE( NS_RAYBURST_SUPER_VIZ_NUM_SAMPLES ),
		"3D-surface-" NS_STRINGIZE( NS_RAYBURST_HYPER_VIZ_NUM_SAMPLES ),
		"2D-single",
		"3D-target"
		};

	ns_assert( type < _NS_RAYBURST_KERNEL_NUM_TYPES );
	return _ns_rayburst_kernel_type_strings[ type ];
	}


const nschar* ns_rayburst_radius_type_to_string( NsRayburstRadiusType type )
	{
	NS_PRIVATE const nschar* _ns_rayburst_radius_type_strings[ _NS_RAYBURST_RADIUS_NUM_TYPES ] =
		{
		"none",
		"MLBD",
		"MPED"
		};

	ns_assert( type < _NS_RAYBURST_RADIUS_NUM_TYPES );
	return _ns_rayburst_radius_type_strings[ type ];
	}


const nschar* ns_rayburst_interp_type_to_string( NsRayburstInterpType type )
	{
	NS_PRIVATE const nschar* _ns_rayburst_interp_type_strings[ _NS_RAYBURST_INTERP_NUM_TYPES ] =
		{
		"bilinear",
		"trilinear"
		};

	ns_assert( type < _NS_RAYBURST_INTERP_NUM_TYPES );
	return _ns_rayburst_interp_type_strings[ type ];
	}


nspointer _ns_rayburst_2d_vectors          = NULL;
nspointer _ns_rayburst_2d_planes           = NULL;
nspointer _ns_rayburst_super_viz_vectors   = NULL;
nspointer _ns_rayburst_super_viz_planes    = NULL;
nspointer _ns_rayburst_super_viz_triangles = NULL;


/* Deprecated */
nspointer _ns_rayburst_std_vectors         = NULL;
nspointer _ns_rayburst_std_planes          = NULL;
nspointer _ns_rayburst_2d_viz_vectors      = NULL;
nspointer _ns_rayburst_2d_viz_planes       = NULL;
nspointer _ns_rayburst_2d_viz_triangles    = NULL;
nspointer _ns_rayburst_sub_viz_vectors     = NULL;
nspointer _ns_rayburst_sub_viz_planes      = NULL;
nspointer _ns_rayburst_sub_viz_triangles   = NULL;
nspointer _ns_rayburst_viz_vectors         = NULL;
nspointer _ns_rayburst_viz_planes          = NULL;
nspointer _ns_rayburst_viz_triangles       = NULL;
nspointer _ns_rayburst_hyper_viz_vectors   = NULL;
nspointer _ns_rayburst_hyper_viz_planes    = NULL;
nspointer _ns_rayburst_hyper_viz_triangles = NULL;


NS_PRIVATE NsError _ns_rayburst_init_on_error( NsError error )
	{
	ns_rayburst_finalize();
	return error;
	}


NsError ns_rayburst_init( const nschar *startup_directory )
	{
	nssize num_samples, num_triangles;

	NS_USE_VARIABLE( startup_directory );

	if( NULL == ( _ns_rayburst_2d_vectors = ns_new_array( NsVector3f, NS_RAYBURST_2D_NUM_SAMPLES ) ) )
		return _ns_rayburst_init_on_error( ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE ) );

	if( NULL == ( _ns_rayburst_2d_planes = ns_new_array( NsRayburstPlane, NS_RAYBURST_2D_NUM_SAMPLES ) ) )
		return _ns_rayburst_init_on_error( ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE ) );

	ns_rayburst_generate_2d(
		_ns_rayburst_2d_vectors,
		_ns_rayburst_2d_planes,
		NS_RAYBURST_2D_NUM_SAMPLES,
		NS_FALSE
		);

	num_samples =
		ns_rayburst_generate_3d_num_samples(
			NS_RAYBURST_SUPER_VIZ_RECURSIVE_DEPTH,
			NS_RAYBURST_SUPER_VIZ_INIT_TYPE
			);

	ns_assert( NS_RAYBURST_SUPER_VIZ_NUM_SAMPLES == num_samples );

	num_triangles =
		ns_rayburst_generate_3d_num_triangles(
			NS_RAYBURST_SUPER_VIZ_RECURSIVE_DEPTH,
			NS_RAYBURST_SUPER_VIZ_INIT_TYPE
			);

	ns_assert( NS_RAYBURST_SUPER_VIZ_NUM_TRIANGLES == num_triangles );

	if( NULL == ( _ns_rayburst_super_viz_vectors = ns_new_array( NsVector3f, num_samples ) ) )
		return _ns_rayburst_init_on_error( ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE ) );	

	if( NULL == ( _ns_rayburst_super_viz_planes = ns_new_array( NsRayburstPlane, num_samples ) ) )
		return _ns_rayburst_init_on_error( ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE ) );

	if( NULL == ( _ns_rayburst_super_viz_triangles = ns_new_array( NsIndexTriangleus, num_triangles ) ) )
		return _ns_rayburst_init_on_error( ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE ) );

	ns_rayburst_generate_3d(
		_ns_rayburst_super_viz_vectors,
		_ns_rayburst_super_viz_planes,
		_ns_rayburst_super_viz_triangles,
		num_samples,
		num_triangles,
		NS_RAYBURST_SUPER_VIZ_RECURSIVE_DEPTH,
		NS_RAYBURST_SUPER_VIZ_INIT_TYPE,
		NS_RAYBURST_SUPER_VIZ_ROTATE
		);

	return ns_no_error();
	}


void ns_rayburst_finalize( void )
	{
	ns_free( _ns_rayburst_2d_vectors );
	ns_free( _ns_rayburst_2d_planes );

	ns_free( _ns_rayburst_super_viz_vectors );
	ns_free( _ns_rayburst_super_viz_planes );
	ns_free( _ns_rayburst_super_viz_triangles );
	}


#define __TRILINEAR_INCREMENT  0.3f


enum{ __X, __Y, __Z };


typedef void ( *NsRayburstKernelFunc )( NsRayburst*, const NsPoint3f* );
typedef void ( *NsRayburstRadiusFunc )( NsRayburst* );
typedef nsfloat ( *NsRayburstBilinearFunc )( NsRayburst*, const NsVector3f*, nsulong* );
typedef void ( *NsRayburstAdvanceFunc )( NsRayburst*, NsRayburstRay* );


NS_PRIVATE NsRayburstBilinearFunc _ns_rayburst_bilinear_funcs[3];


#define _NS_RAYBURST_VOXEL( shape, index, x, y, z )\
	if( ( r->voxel_func )( r->user_data, (x), (y), (z), &value ) )\
		{\
		(shape)[(index)] = ( nsfloat )value;\
		flags |= ( 1 << (index) );\
		}\
	else\
		(shape)[(index)] = ( nsfloat )ns_voxel_get( &r->voxel_buffer, (x), (y), (z) )


/* Returns the interpolated intensity value given the intensities
	'i1' and 'i2' and an 'offset' in the range[0-1]. */
#define _NS_RAYBURST_LERP( i1, i2, offset )\
	( ( ( (i2) - (i1) ) * (offset) ) + (i1) )


/*
	COORD SYSTEM:

      +z
        \
         \
          \
           \_________ +x
           /
          /
         /
        /
       +y
*/


/*
	CUBE

        4__________5
        /\          /\
       /  \        /  \
      /    \      /    \
     /      \    /      \
   6/________\0_/_______1\
    \        / 7\        /
     \      /    \      /
      \    /      \    /
       \  /        \  /
        \/2_________3/
*/


NS_PRIVATE nsulong _ns_rayburst_make_cube
	(
	NsRayburst  *r,
	nsfloat     *cube,
	nsint        x,
	nsint        y,
	nsint        z
	)
	{
	nsulong value, flags = 0;

	_NS_RAYBURST_VOXEL( cube, 0, x,   y,   z   );
	_NS_RAYBURST_VOXEL( cube, 1, x+1, y,   z   );
	_NS_RAYBURST_VOXEL( cube, 2, x,   y+1, z   );
	_NS_RAYBURST_VOXEL( cube, 3, x+1, y+1, z   );
	_NS_RAYBURST_VOXEL( cube, 4, x,   y,   z+1 );
	_NS_RAYBURST_VOXEL( cube, 5, x+1, y,   z+1 );
	_NS_RAYBURST_VOXEL( cube, 6, x,   y+1, z+1 );
	_NS_RAYBURST_VOXEL( cube, 7, x+1, y+1, z+1 );

	return flags;
	}


/*
	X (PLANE) RECT:

         2
        /\
       /  \
      /    \
     /      \
   3/        \0 ________ +x
    \        /
     \      /
      \    /
       \  /
        \/
        1


	Y (PLANE) RECT:

    2____________3
	  \           \
		\           \  
       \           \ 
        \           \
         \___________\
         0           1
        /
       /
      /
     +y


	Z (PLANE) RECT:

    +z
     \
      \
       \
        0____________1
        /           /
       /           /
      /           /
     /           /
    /___________/
   2            3
*/


NS_PRIVATE nsulong _ns_rayburst_make_x_rect
	(
	NsRayburst  *r,
	nsfloat     *rect,
	nsint        x,
	nsint        y,
	nsint        z
	)
	{
	nsulong value, flags = 0;

	_NS_RAYBURST_VOXEL( rect, 0, x, y,   z   );
	_NS_RAYBURST_VOXEL( rect, 1, x, y+1, z   );
	_NS_RAYBURST_VOXEL( rect, 2, x, y,   z+1 );
	_NS_RAYBURST_VOXEL( rect, 3, x, y+1, z+1 );

	return flags;
	}


NS_PRIVATE nsulong _ns_rayburst_make_y_rect
	(
	NsRayburst  *r,
	nsfloat     *rect,
	nsint        x,
	nsint        y,
	nsint        z
	)
	{
	nsulong value, flags = 0;

	_NS_RAYBURST_VOXEL( rect, 0, x,   y, z   );
	_NS_RAYBURST_VOXEL( rect, 1, x+1, y, z   );
	_NS_RAYBURST_VOXEL( rect, 2, x,   y, z+1 );
	_NS_RAYBURST_VOXEL( rect, 3, x+1, y, z+1 );

	return flags;
	}


NS_PRIVATE nsulong _ns_rayburst_make_z_rect
	(
	NsRayburst  *r,
	nsfloat     *rect,
	nsint        x,
	nsint        y,
	nsint        z
	)
	{
	nsulong value, flags = 0;

	_NS_RAYBURST_VOXEL( rect, 0, x,   y,   z );
	_NS_RAYBURST_VOXEL( rect, 1, x+1, y,   z );
	_NS_RAYBURST_VOXEL( rect, 2, x,   y+1, z );
	_NS_RAYBURST_VOXEL( rect, 3, x+1, y+1, z );

	return flags;
	}


NS_PRIVATE nsfloat _ns_rayburst_trilinear
	(
	NsRayburst        *rayburst,
	const NsVector3f  *point,
	nsulong           *flags
	)
	{
	nsint32    i32;
	nsfloat64  f64;
	nsfloat    offset_x;
	nsfloat    offset_y;
	nsfloat    offset_z;
	nsfloat    cube[ 8 ];
	nsfloat    rect[ 4 ];
	nsfloat    line[ 2 ];
	nsfloat    value;
	nsint      x;
	nsint      y;
	nsint      z;


	//if( point->x < 0.0f || point->y < 0.0f || point->z < 0.0f )
	//	return -1.0f;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->x, i32, f64 );
	x = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->y, i32, f64 );
	y = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->z, i32, f64 );
	z = ( nsint )i32;

	if( x < 0 || y < 0 || z < 0 )
		return -1.0f;

	if( x >= rayburst->width  - 1 ||
		 y >= rayburst->height - 1 ||
		 z >= rayburst->length - 1   )
		return -1.0f;

	*flags = _ns_rayburst_make_cube( rayburst, cube, x, y, z );

	offset_x = point->x - ( nsfloat )x;
	offset_y = point->y - ( nsfloat )y;
	offset_z = point->z - ( nsfloat )z;

	rect[0] = _NS_RAYBURST_LERP( cube[0], cube[1], offset_x );
	rect[1] = _NS_RAYBURST_LERP( cube[2], cube[3], offset_x );
	rect[2] = _NS_RAYBURST_LERP( cube[4], cube[5], offset_x );
	rect[3] = _NS_RAYBURST_LERP( cube[6], cube[7], offset_x );

	line[0] = _NS_RAYBURST_LERP( rect[0], rect[1], offset_y );
	line[1] = _NS_RAYBURST_LERP( rect[2], rect[3], offset_y );

	value = _NS_RAYBURST_LERP( line[0], line[1], offset_z );

	if( ! (*flags) && value >= rayburst->threshold )
		value = 0.0f;

	return value;
	}


NS_PRIVATE nsfloat _ns_rayburst_bilinear_x
	(
	NsRayburst        *rayburst,
	const NsVector3f  *point,
	nsulong           *flags
	)
	{
	nsint32    i32;
	nsfloat64  f64;
	nsfloat    offset_y;
	nsfloat    offset_z;
	nsfloat    rect[ 4 ];
	nsfloat    line[ 2 ];
	nsfloat    value;
	nsint      x;
	nsint      y;
	nsint      z;


	//if( point->x < 0.0f || point->y < 0.0f || point->z < 0.0f )
	//	return -1.0f;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->x, i32, f64 );
	x = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->y, i32, f64 );
	y = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->z, i32, f64 );
	z = ( nsint )i32;

	if( x < 0 || y < 0 || z < 0 )
		return -1.0f;

	if( x >= rayburst->width      ||
		 y >= rayburst->height - 1 ||
		 z >= rayburst->length - 1   )
		return -1.0f;

	*flags = _ns_rayburst_make_x_rect( rayburst, rect, x, y, z );

	offset_y = point->y - ( nsfloat )y;
	offset_z = point->z - ( nsfloat )z;

	line[0] = _NS_RAYBURST_LERP( rect[0], rect[1], offset_y );
	line[1] = _NS_RAYBURST_LERP( rect[2], rect[3], offset_y );

	value = _NS_RAYBURST_LERP( line[0], line[1], offset_z );

	if( ! (*flags) && value >= rayburst->threshold )
		value = 0.0f;

	return value;
	}


NS_PRIVATE nsfloat _ns_rayburst_bilinear_y
	(
	NsRayburst        *rayburst,
	const NsVector3f  *point,
	nsulong           *flags
	)
	{
	nsint32    i32;
	nsfloat64  f64;
	nsfloat    offset_x;
	nsfloat    offset_z;
	nsfloat    rect[ 4 ];
	nsfloat    line[ 2 ];
	nsfloat    value;
	nsint      x;
	nsint      y;
	nsint      z;


	//if( point->x < 0.0f || point->y < 0.0f || point->z < 0.0f )
	//	return -1.0f;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->x, i32, f64 );
	x = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->y, i32, f64 );
	y = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->z, i32, f64 );
	z = ( nsint )i32;

	if( x < 0 || y < 0 || z < 0 )
		return -1.0f;

	if( x >= rayburst->width  - 1 ||
		 y >= rayburst->height     ||
		 z >= rayburst->length - 1   )
		return -1.0f;

	*flags = _ns_rayburst_make_y_rect( rayburst, rect, x, y, z );

	offset_x = point->x - ( nsfloat )x;
	offset_z = point->z - ( nsfloat )z;

	line[0] = _NS_RAYBURST_LERP( rect[0], rect[1], offset_x );
	line[1] = _NS_RAYBURST_LERP( rect[2], rect[3], offset_x );

	value = _NS_RAYBURST_LERP( line[0], line[1], offset_z );

	if( ! (*flags) && value >= rayburst->threshold )
		value = 0.0f;

	return value;
	}


NS_PRIVATE nsfloat _ns_rayburst_bilinear_z
	(
	NsRayburst        *rayburst,
	const NsVector3f  *point,
	nsulong           *flags
	)
	{
	nsint32    i32;
	nsfloat64  f64;
	nsfloat    offset_x;
	nsfloat    offset_y;
	nsfloat    rect[ 4 ];
	nsfloat    line[ 2 ];
	nsfloat    value;
	nsint      x;
	nsint      y;
	nsint      z;


	//if( point->x < 0.0f || point->y < 0.0f || point->z < 0.0f )
	//	return -1.0f;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->x, i32, f64 );
	x = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->y, i32, f64 );
	y = ( nsint )i32;

	NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( point->z, i32, f64 );
	z = ( nsint )i32;

	if( x < 0 || y < 0 || z < 0 )
		return -1.0f;

	if( x >= rayburst->width  - 1 ||
		 y >= rayburst->height - 1 ||
		 z >= rayburst->length       )
		return -1.0f;

	*flags = _ns_rayburst_make_z_rect( rayburst, rect, x, y, z );

	offset_x = point->x - ( nsfloat )x;
	offset_y = point->y - ( nsfloat )y;

	line[0] = _NS_RAYBURST_LERP( rect[0], rect[1], offset_x );
	line[1] = _NS_RAYBURST_LERP( rect[2], rect[3], offset_x );

	value = _NS_RAYBURST_LERP( line[0], line[1], offset_y );

	if( ! (*flags) && value >= rayburst->threshold )
		value = 0.0f;

	return value;
	}


/*
#define _NS_RAYBURST_ON_INVALID_ORIGIN( origin_value, threshold )\
	ns_warning(\
		NS_WARNING_LEVEL_CAUTION\
		NS_MODULE\
		" Rayburst origin value < threshold ( "\
		NS_FMT_DOUBLE\
		" < "\
		NS_FMT_DOUBLE\
		" )",\
		(origin_value),\
		(threshold)\
		)
*/
#define _NS_RAYBURST_ON_INVALID_ORIGIN( origin_value, threshold )


NS_PRIVATE nsboolean _ns_rayburst_valid_origin_2d( NsRayburst *rayburst, nsfloat *origin_value )
	{
	nsulong flags;

	*origin_value = _ns_rayburst_bilinear_z( rayburst, &rayburst->origin, &flags );

	if( *origin_value < rayburst->threshold )
		{
		if( rayburst->allow_dark_start )
			{
			rayburst->dark_start = NS_TRUE;
			*origin_value        = 0.0f;
			}
		else
			{
			_NS_RAYBURST_ON_INVALID_ORIGIN( *origin_value, rayburst->threshold );
			return NS_FALSE;
			}
		}

	return NS_TRUE;
	}


NS_PRIVATE nsboolean _ns_rayburst_valid_origin_3d( NsRayburst *rayburst, nsfloat *origin_value )
	{
	nsulong flags;

	*origin_value = _ns_rayburst_trilinear( rayburst, &rayburst->origin, &flags );

/*TEMP*///ns_println( "Rayburst origin value is " NS_FMT_DOUBLE, *origin_value );

	if( *origin_value < rayburst->threshold )
		{
		_NS_RAYBURST_ON_INVALID_ORIGIN( *origin_value, rayburst->threshold );
		return NS_FALSE;
		}

	return NS_TRUE;
	}


NS_PRIVATE nsboolean _ns_rayburst_voxel_func
	(
	const NsVoxelBuffer  *voxel_buffer,
	nsint                 x,
	nsint                 y,
	nsint                 z,
	nsulong              *value
	)
	{
	*value = ns_voxel_get( voxel_buffer, x, y, z );
	return NS_TRUE;
	}


NS_PRIVATE nsboolean _ns_rayburst_init
	(
	NsRayburst        *rayburst,
	const NsVector3f  *origin,
	nsboolean          ( *valid_origin )( NsRayburst*, nsfloat* )
	)
	{
	NsRayburstSample       *samples;
	NsRayburstSample       *sample;
	const NsVector3f       *vectors;
	const NsVector3f       *vector;
	const NsRayburstPlane  *planes;
	const NsRayburstPlane  *plane;
	nssize                  i;
	nsfloat                 x;
	nsfloat                 y;
	nsfloat                 z;


	/* NOTE: Vectors traverse image space, but as floating point values! */
	ns_to_image_space_ex( origin, &rayburst->origin, rayburst->voxel_info );

	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* NOTE: Tri-linear interpolation performed
		on the initial origin. */
	if( ! ( valid_origin )( rayburst, &rayburst->origin_value ) )
		{
		rayburst->radius = 0.0f;

		ns_to_voxel_space_ex( &rayburst->origin, &rayburst->mlbd_fwd_intercept, rayburst->voxel_info );
		ns_to_voxel_space_ex( &rayburst->origin, &rayburst->mlbd_rev_intercept, rayburst->voxel_info );

		return NS_FALSE;
		}

	vectors = rayburst->vectors;
	planes  = rayburst->planes;

	x = ns_floorf( rayburst->origin.x );
	y = ns_floorf( rayburst->origin.y );
	z = ns_floorf( rayburst->origin.z );

	/* Initialize all the rays. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		sample = samples + i;
		plane  = planes  + i;
		vector = vectors + i;

		#ifdef NS_DEBUG_EXTREME

		#define _NS_RAYBURST_CHECK_PLANE( which )\
			if( vector->which < 0.0f )\
				{\
				ns_assert( 1 == ( nslong )plane->d_delta.which );\
				ns_assert( 0 == ( nslong )plane->d.which );\
				}\
			else\
				{\
				ns_assert( -1 == ( nslong )plane->d_delta.which );\
				ns_assert(  1 == ( nslong )plane->d.which );\
				}

		_NS_RAYBURST_CHECK_PLANE( x );
		_NS_RAYBURST_CHECK_PLANE( y );
		_NS_RAYBURST_CHECK_PLANE( z );

		#endif/* NS_DEBUG_EXTREME */

		sample->done = NS_FALSE;

		sample->fwd.done = NS_FALSE;

		sample->fwd.curr = sample->fwd.prev = rayburst->origin;
		sample->fwd.prev_value = rayburst->origin_value;

		sample->fwd.R.O = rayburst->origin;
		sample->fwd.R.D = *vector;

		sample->fwd.distance = 0.0f;

		sample->fwd.plane_d_delta[__X] = plane->d_delta.x;
		sample->fwd.plane_d_delta[__Y] = plane->d_delta.y;
		sample->fwd.plane_d_delta[__Z] = plane->d_delta.z;

		/* NOTE: The origin is now in image space, so we can convert it to
			an integer, i.e. floor(), and then set the initial 'd' for the
			potential intersection planes.

			Example(for X plane):

			- ray.O.x = 7.35, floor'd to an integer = 7.0

			- if ray.D.x > 0, i.e. traveling along the positive
			  x-axis, then _ns_rayburst_plane_d[i] is 1.0 so
			  7.0 + 1.0 = 8.0

			- if ray.D.x < 0, i.e. traveling along the negative
			  x-axis, then _ns_rayburst_plane_d[i] is 0.0 so
			  7.0 + 0.0 = 7.0
		*/
		sample->fwd.plane_d[__X] = -( plane->d.x + x );
		sample->fwd.plane_d[__Y] = -( plane->d.y + y );
		sample->fwd.plane_d[__Z] = -( plane->d.z + z );

		/* Now reverse everything if necessary... except the origin. */
		if( rayburst->reverse_rays )
			{
			sample->rev.done = NS_FALSE;

			sample->rev.prev       = rayburst->origin;
			sample->rev.prev_value = rayburst->origin_value;

			sample->rev.R.O = rayburst->origin;
			ns_vector3f_rev( &(sample->rev.R.D), &(sample->fwd.R.D) );

			sample->rev.distance = 0.0f;

			sample->rev.plane_d_delta[__X] = -(sample->fwd.plane_d_delta[__X]);
			sample->rev.plane_d_delta[__Y] = -(sample->fwd.plane_d_delta[__Y]);
			sample->rev.plane_d_delta[__Z] = -(sample->fwd.plane_d_delta[__Z]);

			/* NS_ABS( _ns_rayburst_plane_d[i].x - 1.0f ),
				i.e. 0.0f becomes 1.0f and 1.0f becomes 0.0f */
			sample->rev.plane_d[__X] = -( NS_ABS( plane->d.x - 1.0f ) + x );
			sample->rev.plane_d[__Y] = -( NS_ABS( plane->d.y - 1.0f ) + y );
			sample->rev.plane_d[__Z] = -( NS_ABS( plane->d.z - 1.0f ) + z );
			}

		sample->distance = 0.0f;
		}

	rayburst->done_count = 0;

	return NS_TRUE;
	}


NS_PRIVATE nsfloat _ns_rayburst_sample_working_distance
	(
	NsRayburst        *rayburst,
	NsRayburstSample  *sample,
	nsfloat           *ret_distance_sq_fwd,
	nsfloat           *ret_distance_sq_rev
	)
	{
	nsfloat distance_sq_fwd;
	nsfloat distance_sq_rev;

	/* NOTE: Returns distance squared to avoid square root function. */

	distance_sq_fwd = NS_DISTANCE_SQUARED(
								sample->fwd.curr.x, sample->fwd.curr.y, sample->fwd.curr.z,
								rayburst->origin.x, rayburst->origin.y, rayburst->origin.z
								);

	distance_sq_rev = NS_DISTANCE_SQUARED(
								sample->rev.curr.x, sample->rev.curr.y, sample->rev.curr.z,
								rayburst->origin.x, rayburst->origin.y, rayburst->origin.z
								);

	if( ret_distance_sq_fwd )
		*ret_distance_sq_fwd = distance_sq_fwd;

	if( ret_distance_sq_rev )
		*ret_distance_sq_rev = distance_sq_rev;

	return distance_sq_fwd + distance_sq_rev;
	}


NS_PRIVATE void _ns_rayburst_advance_3d_trilinear( NsRayburst *rayburst, NsRayburstRay *ray )
	{
	nsulong flags;

	/* Compute the intersection point. */
	ray->curr.x = ray->R.O.x + ray->R.D.x * __TRILINEAR_INCREMENT;
	ray->curr.y = ray->R.O.y + ray->R.D.y * __TRILINEAR_INCREMENT;
	ray->curr.z = ray->R.O.z + ray->R.D.z * __TRILINEAR_INCREMENT;

	ray->curr_value = _ns_rayburst_trilinear( rayburst, &ray->curr, &flags );

	if( ray->curr_value < rayburst->threshold || ! flags )
		ray->done = NS_TRUE;
	else
		{
		ray->R.O = ray->curr;

		ray->prev_value = ray->curr_value;
		ray->prev       = ray->curr;
		}
	}


NS_PRIVATE void _ns_rayburst_advance_3d_bilinear( NsRayburst *rayburst, NsRayburstRay *ray )
	{
	nsint    closest;
	nsulong  flags;
	nsfloat  t[3];


	/* Find the closest plane by computing the minimum
		't' value to the point of intersection.
		IMPORTANT: Assuming division by zero here produces an
		infinity value which will compare greater than any
		possible float. */
	#ifdef NS_DEBUG

	t[__X] = NS_FLOAT_EQUAL( ray->R.D.x, 0.0f ) ? NS_FLOAT_MAX :
				( -( ray->R.O.x + ray->plane_d[__X] ) ) / ray->R.D.x;

	t[__Y] = NS_FLOAT_EQUAL( ray->R.D.y, 0.0f ) ? NS_FLOAT_MAX :
				( -( ray->R.O.y + ray->plane_d[__Y] ) ) / ray->R.D.y;

	t[__Z] = NS_FLOAT_EQUAL( ray->R.D.z, 0.0f ) ? NS_FLOAT_MAX :
				( -( ray->R.O.z + ray->plane_d[__Z] ) ) / ray->R.D.z;

	#else

	t[__X] = ( -( ray->R.O.x + ray->plane_d[__X] ) ) / ray->R.D.x;
	t[__Y] = ( -( ray->R.O.y + ray->plane_d[__Y] ) ) / ray->R.D.y;
	t[__Z] = ( -( ray->R.O.z + ray->plane_d[__Z] ) ) / ray->R.D.z;

	#endif/* NS_DEBUG */

	closest = ( ( t[__X] < t[__Y] ) ?
					( ( t[__X] < t[__Z] ) ?
						__X
							:
						__Z )
						:
					( ( t[__Y] < t[__Z] ) ?
						__Y
							:
						__Z ) );

	/* Compute the intersection point. */
	ray->curr.x = ray->R.O.x + ray->R.D.x * t[ closest ];
	ray->curr.y = ray->R.O.y + ray->R.D.y * t[ closest ];
	ray->curr.z = ray->R.O.z + ray->R.D.z * t[ closest ];

	ray->curr_value = ( _ns_rayburst_bilinear_funcs[ closest ] )( rayburst, &ray->curr, &flags );

	if( ray->curr_value < rayburst->threshold || ! flags )
		ray->done = NS_TRUE;
	else
		{
		/* Move the plane we just hit by one voxel. */
		ray->plane_d[ closest ] += ray->plane_d_delta[ closest ];

		ray->R.O = ray->curr;

		ray->prev_value = ray->curr_value;
		ray->prev       = ray->curr;
		}
	}


NS_PRIVATE void _ns_rayburst_intercept_3d
	(
	NsRayburst     *rayburst,
	NsRayburstRay  *ray,
	NsVector3f     *intercept
	)
	{
	NsLerpf     lerp;
	NsVector3f  origin;
	NsVector3f  mid_pos;
	nsfloat     mid_value;
	nsulong     flags;


	/* Interpolate the final intercept using the threshold. */
	if( NS_FLOAT_EQUAL( ray->prev_value, ray->curr_value ) )
		{
		intercept->x = ray->curr.x;
		intercept->y = ray->curr.y;
		intercept->z = ray->curr.z;
		}
	else
		{
		#define __LINEAR\
		ns_lerpf_init( &lerp, ray->prev_value, ray->prev.x, ray->curr_value, ray->curr.x );\
		intercept->x = ns_lerpf( &lerp, rayburst->threshold );\
		ns_lerpf_init( &lerp, ray->prev_value, ray->prev.y, ray->curr_value, ray->curr.y );\
		intercept->y = ns_lerpf( &lerp, rayburst->threshold );\
		ns_lerpf_init( &lerp, ray->prev_value, ray->prev.z, ray->curr_value, ray->curr.z );\
		intercept->z = ns_lerpf( &lerp, rayburst->threshold );

		#define __TRILINEAR\
		mid_pos   = *intercept;\
		mid_value = _ns_rayburst_trilinear( rayburst, &mid_pos, &flags );\
		if( mid_value < rayburst->threshold )\
			{\
			ray->curr       = mid_pos;\
			ray->curr_value = mid_value;\
			}\
		else\
			{\
			ray->prev       = mid_pos;\
			ray->prev_value = mid_value;\
			}

		__LINEAR
		__TRILINEAR
		__LINEAR
		//__TRILINEAR
		//__LINEAR
		//__TRILINEAR
		//__LINEAR
		}

	/* Convert back to voxel space for the actual distance calculation! */
	ns_to_voxel_space_ex( &rayburst->origin, &origin, rayburst->voxel_info );
	ns_to_voxel_space_ex( intercept, intercept, rayburst->voxel_info );

	ray->distance = ns_vector3f_distance( &origin, intercept );
	}


/* Simple insertion sort. Is it OK since the array is small?
	IMPORTANT: This routine alters the ordering of the samples
	in the array! */
void _ns_rayburst_sort_samples( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples, sample;
	nslong             curr;
	nslong             scan;
	nslong             num_samples;


	samples     = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );
	num_samples = ( nslong )rayburst->num_samples; 

	for( curr = 1; curr < num_samples; ++curr )
		{
		sample = samples[ curr ];
		scan   = curr;

		while( 0 < scan && sample.distance < samples[ scan - 1 ].distance )
			{
			samples[ scan ] = samples[ scan - 1 ];
			--scan;
			}

		samples[ scan ] = sample;
		}
	}


/* Not optimized. Casting all rays to completion. */
NS_PRIVATE void _ns_rayburst_mlbd_3d( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples;
	NsRayburstSample  *sample;
	nssize             i;


	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* Get interception distance for every direction. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		sample = samples + i;

		while( ! sample->fwd.done )
			( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->fwd );

		while( ! sample->rev.done )
			( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->rev );

		_ns_rayburst_intercept_3d( rayburst, &sample->fwd, &rayburst->mlbd_fwd_intercept );
		_ns_rayburst_intercept_3d( rayburst, &sample->rev, &rayburst->mlbd_rev_intercept );

		sample->distance = ( sample->fwd.distance + sample->rev.distance ) / 2.0f;
		}

	/* Sort the samples and use the MLBD. */
	_ns_rayburst_sort_samples( rayburst );

	_ns_rayburst_intercept_3d(
		rayburst,
		&( samples[ rayburst->mlbd_sample ].fwd ),
		&rayburst->mlbd_fwd_intercept
		);

	_ns_rayburst_intercept_3d(
		rayburst,
		&( samples[ rayburst->mlbd_sample ].rev ),
		&rayburst->mlbd_rev_intercept
		);

	rayburst->radius = ns_rayburst_sample_distance(
								rayburst,
								rayburst->mlbd_sample,
								NS_RAYBURST_SAMPLE_DISTANCE_AVERAGE
								);

	if( rayburst->radius < ns_voxel_info_min_radius( rayburst->voxel_info ) )
		rayburst->radius = ns_voxel_info_min_radius( rayburst->voxel_info );
	}


NS_PRIVATE void _ns_rayburst_all_3d( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples;
	NsRayburstSample  *sample;
	nssize             i;
	NsVector3f         curr, origin;
	nsdouble           ray_distance_sq;


	ns_to_voxel_space_ex( &rayburst->origin, &origin, rayburst->voxel_info );

	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* Get interception distance for every direction. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		sample = samples + i;

		while( ! sample->fwd.done )
			{
			( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->fwd );

			if( rayburst->limit_rays )
				{
				ns_to_voxel_space_ex( &(sample->fwd.curr), &curr, rayburst->voxel_info );
				ray_distance_sq = NS_DISTANCE_SQUARED( curr.x, curr.y, curr.z, origin.x, origin.y, origin.z );

				if( rayburst->max_ray_distance_sq <= ray_distance_sq )
					sample->fwd.done = NS_TRUE;
				}

			if( NULL != rayburst->render_func )
				( rayburst->render_func )( rayburst, ( nspointer )rayburst->user_data );
			}

		_ns_rayburst_intercept_3d( rayburst, &sample->fwd, &rayburst->mlbd_fwd_intercept );

		sample->distance = sample->fwd.distance;
		}
	}


NS_PRIVATE void _ns_rayburst_target_3d( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples;
	NsRayburstSample  *sample;
	nssize             i;


	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* Get interception distance for every direction. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		sample = samples + i;

		while( ! sample->fwd.done )
			( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->fwd );

		_ns_rayburst_intercept_3d( rayburst, &sample->fwd, &rayburst->mlbd_fwd_intercept );

		sample->distance = sample->fwd.distance;

		/* Should be only one ray casted. If it exceeds the target distance then
			we know the target was hit since the ray was casted in the direction of
			the target. */
		if( sample->distance >= rayburst->target_distance )
			rayburst->target_was_hit = NS_TRUE;
		}
	}


NS_PRIVATE void _ns_rayburst_advance_2d_trilinear( NsRayburst *rayburst, NsRayburstRay *ray )
	{
	nsulong flags;

	/* Compute the intersection point. */
	ray->curr.x = ray->R.O.x + ray->R.D.x * __TRILINEAR_INCREMENT;
	ray->curr.y = ray->R.O.y + ray->R.D.y * __TRILINEAR_INCREMENT;
	ray->curr.z = ray->R.O.z;

	ray->curr_value = _ns_rayburst_trilinear( rayburst, &ray->curr, &flags );

	if( ray->curr_value < rayburst->threshold || ! flags )
		ray->done = NS_TRUE;
	else
		{
		ray->R.O = ray->curr;

		ray->prev_value = ray->curr_value;
		ray->prev       = ray->curr;
		}
	}


NS_PRIVATE void _ns_rayburst_advance_2d_bilinear( NsRayburst *rayburst, NsRayburstRay *ray )
	{
	nsint    closest;
	nsulong  flags;
	nsfloat  t[2];


	/* Find the closest plane by computing the minimum
		't' value to the point of intersection.
		IMPORTANT: Assuming division by zero here produces an
		infinity value which will compare greater than any
		possible float. */
	#ifdef NS_DEBUG

	t[__X] = NS_FLOAT_EQUAL( ray->R.D.x, 0.0f ) ? NS_FLOAT_MAX :
				( -( ray->R.O.x + ray->plane_d[__X] ) ) / ray->R.D.x;

	t[__Y] = NS_FLOAT_EQUAL( ray->R.D.y, 0.0f ) ? NS_FLOAT_MAX :
				( -( ray->R.O.y + ray->plane_d[__Y] ) ) / ray->R.D.y;
	#else

	t[__X] = ( -( ray->R.O.x + ray->plane_d[__X] ) ) / ray->R.D.x;
	t[__Y] = ( -( ray->R.O.y + ray->plane_d[__Y] ) ) / ray->R.D.y;

	#endif/* NS_DEBUG */

	closest = t[__X] < t[__Y] ? __X : __Y;

	/* Compute the intersection point. */
	ray->curr.x = ray->R.O.x + ray->R.D.x * t[ closest ];
	ray->curr.y = ray->R.O.y + ray->R.D.y * t[ closest ];
	ray->curr.z = ray->R.O.z;

	ray->curr_value = _ns_rayburst_bilinear_z( rayburst, &ray->curr, &flags );

	if( ( ray->curr_value < 0.0f )
       ||
		 ( ray->curr_value < rayburst->threshold && ray->prev_value >= rayburst->threshold )
		 ||
		 ( ! flags )
		)
		ray->done = NS_TRUE;
	else
		{
		/* Move the plane we just hit by one voxel. */
		ray->plane_d[ closest ] += ray->plane_d_delta[ closest ];

		ray->R.O = ray->curr;

		ray->prev_value = ray->curr_value;
		ray->prev       = ray->curr;
		}
	}


NS_PRIVATE void _ns_rayburst_intercept_2d
	(
	NsRayburst     *rayburst,
	NsRayburstRay  *ray,
	NsVector3f     *intercept
	)
	{
	NsLerpf     lerp;
	NsVector3f  origin;


	intercept->z = ray->curr.z;

	/* Interpolate the final intercept using the threshold. */
	if( NS_FLOAT_EQUAL( ray->prev_value, ray->curr_value ) )
		{
		intercept->x = ray->curr.x;
		intercept->y = ray->curr.y;
		}
	else
		{
		ns_lerpf_init( &lerp, ray->prev_value, ray->prev.x, ray->curr_value, ray->curr.x );
		intercept->x = ns_lerpf( &lerp, rayburst->threshold );

		ns_lerpf_init( &lerp, ray->prev_value, ray->prev.y, ray->curr_value, ray->curr.y );
		intercept->y = ns_lerpf( &lerp, rayburst->threshold );
		}

	/* Convert back to voxel space for the actual distance calculation! */
	ns_to_voxel_space_ex( &rayburst->origin, &origin, rayburst->voxel_info );
	ns_to_voxel_space_ex( intercept, intercept, rayburst->voxel_info );

	ray->distance = ns_vector3f_distance( &origin, intercept );
	}


/* Optimized. Only casting rays until the one with the minimum distance finishes. */
NS_PRIVATE void _ns_rayburst_mlbd_2d( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples;
	NsRayburstSample  *sample, *min_sample;
	nsfloat            distance, min_distance;
	nsfloat            dist_sq_fwd, dist_sq_rev;
	nssize             i;


	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	NS_INFINITE_LOOP
		{
		min_sample   = NULL;
		min_distance = NS_FLOAT_MAX;

		for( i = 0; i < rayburst->num_samples; ++i )
			{
			sample = samples + i;

			if( ! sample->fwd.done )
				( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->fwd );

			if( ! sample->rev.done )
				( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->rev );

			if( ! sample->done && sample->fwd.done && sample->rev.done )
				{
				sample->done = NS_TRUE;
				++(rayburst->done_count);
				}

			/* NOTE: 'dark_start' will never be true if 'allow_dark_start' wasnt true.
				See also _ns_rayburst_valid_origin_2d(). */
			if( rayburst->dark_start )
				if( ! sample->done && ( sample->fwd.done || sample->rev.done ) )
					{
					_ns_rayburst_sample_working_distance( rayburst, sample, &dist_sq_fwd, &dist_sq_rev );

					if( sample->fwd.done )
						{
						//ns_assert( ! sample->rev.done );
						if( dist_sq_rev / dist_sq_fwd > 16.0 )
							{
							//ns_println( "REV TOO FAR!" );
							sample->done = sample->rev.done = NS_TRUE;
							++(rayburst->done_count);

							// This allows the interpolation in _ns_rayburst_intercept_2d()
							//	to work properly.
							//sample->rev.prev_value = rayburst->threshold;

							//ns_println( "REV too big!" );
							return;
							}
						}
					else
						{
						//ns_assert( sample->rev.done );
						if( dist_sq_fwd / dist_sq_rev > 16.0 )
							{
							//ns_println( "FWD TOO FAR!" );
							sample->done = sample->fwd.done = NS_TRUE;
							++(rayburst->done_count);

							// This allows the interpolation in _ns_rayburst_intercept_2d()
							//	to work properly.
							//sample->fwd.prev_value = rayburst->threshold;

							//ns_println( "FWD too big!" );
							return;
							}
						}
					}

			distance = _ns_rayburst_sample_working_distance( rayburst, sample, NULL, NULL );

			if( distance < min_distance )
				{
				min_sample   = sample;
				min_distance = distance;
				}
			}

		ns_assert( NULL != min_sample );

		if( min_sample->done )
			break;
		}

	ns_assert( NULL != min_sample );

	_ns_rayburst_intercept_2d( rayburst, &min_sample->fwd, &rayburst->mlbd_fwd_intercept );
	_ns_rayburst_intercept_2d( rayburst, &min_sample->rev, &rayburst->mlbd_rev_intercept );

	min_sample->distance = ( min_sample->fwd.distance + min_sample->rev.distance ) / 2.0f;

	rayburst->radius = min_sample->distance;

	if( rayburst->radius < ns_voxel_info_min_radius( rayburst->voxel_info ) )
		rayburst->radius = ns_voxel_info_min_radius( rayburst->voxel_info );
	}


NS_PRIVATE void _ns_rayburst_all_2d( NsRayburst *rayburst )
	{
	NsRayburstSample  *samples;
	NsRayburstSample  *sample;
	nssize             i;


	samples = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* Get interception distance for every direction. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		sample = samples + i;

		while( ! sample->fwd.done )
			( ( NsRayburstAdvanceFunc )( rayburst->advance_func ) )( rayburst, &sample->fwd );

		_ns_rayburst_intercept_2d( rayburst, &sample->fwd, &rayburst->mlbd_fwd_intercept );

		sample->distance = sample->fwd.distance;
		}
	}


NS_PRIVATE void _ns_rayburst_kernel_std( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_3d ) )
		( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );
	}


NS_PRIVATE void _ns_rayburst_kernel_2d( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_2d ) )
		( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );
	}


NS_PRIVATE void _ns_rayburst_kernel_single( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_2d ) )
		( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );
	}


NS_PRIVATE void _ns_rayburst_kernel_2d_viz( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_2d ) )
		( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );
	}


void ns_rayburst_compute_volume_and_surface_area
	(
	NsRayburst  *rayburst,
	nsfloat      stretch_factor
	)
	{
	const NsRayburstSample   *samples;
	const NsVector3f         *vectors;
	const NsIndexTriangleus  *triangles;
	nssize                    i;
	NsVector3f                A, B, C, AB, AC, X, N, S;
	nsdouble                  height, volume, surface_area;


	rayburst->volume       = 0.0;
	rayburst->surface_area = 0.0;

	samples   = ( NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );
	vectors   = rayburst->vectors;
	triangles = rayburst->triangles;

	/* Compute the combined volume of all the pyramids making
		up the sampling core. */
	for( i = 0; i < rayburst->num_triangles; ++i )
		if( 0.0f < samples[ triangles[i].a ].distance &&
			 0.0f < samples[ triangles[i].b ].distance &&
			 0.0f < samples[ triangles[i].c ].distance   )
			{
			/* Get and scale the vectors of this triangle. */
			A = vectors[ triangles[i].a ];
			B = vectors[ triangles[i].b ];
			C = vectors[ triangles[i].c ];

			A.z /= stretch_factor;
			B.z /= stretch_factor;
			C.z /= stretch_factor;

			ns_vector3f_cmpd_scale( &A, samples[ triangles[i].a ].distance );
			ns_vector3f_cmpd_scale( &B, samples[ triangles[i].b ].distance );
			ns_vector3f_cmpd_scale( &C, samples[ triangles[i].c ].distance );

			/* Compute the cross product. */
			ns_vector3f_sub( &AB, &B, &A ); // AB = B - A
			ns_vector3f_sub( &AC, &C, &A ); // AC = C - A

			ns_vector3f_cross( &X, &AB, &AC ); // X = AB x AC

			/* The area of a triangle is 1/2 the magnitude of
				the cross product. */
			surface_area = .5 * ns_vector3f_mag( &X );

			/* Get the normal. */
			N = X;
			ns_vector3f_norm( &N );

			/* Create a vector to one corner of the triangle. */
			S = A;

			/* Compute the magnitude of the projection of S onto N.

				N*S = ||N||||S||cosTHETA
				N*S = ||S||cosTHETA  since ||N|| = 1
				N*S = magnitude of projection of S onto N
			*/
			height = ns_vector3f_dot( &N, &S );
			height = NS_ABS( height );

			/* Volume of a pyramid is 1/3 of the area times the height. */
			volume = ( 1.0 / 3.0 ) * surface_area * height;

			rayburst->volume       += volume;
			rayburst->surface_area += surface_area;
			}
	}


NS_PRIVATE void _ns_rayburst_kernel_viz( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( ! _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_3d ) )
		return;

	( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );

	ns_rayburst_compute_volume_and_surface_area( rayburst, 1.0f );
	}


NS_PRIVATE nsboolean _ns_rayburst_acquire_target( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	NsVector3f D;

	ns_assert( NS_RAYBURST_KERNEL_TARGET == rayburst->kernel_type );

	rayburst->target_was_hit = NS_FALSE;

	ns_vector3f_sub( &D, &rayburst->target, origin ); /* D = target - origin */

	/* Check if the passed 'origin' is right on the target.
		If so just set the 'target_was_hit' variable to true but
		return false so that no rays are cast. */

	rayburst->target_distance = ns_vector3f_mag( &D );

	if( NS_FLOAT_EQUAL( rayburst->target_distance, 0.0f ) )
		{
		rayburst->target_was_hit = NS_TRUE;
		return NS_FALSE;
		}

	ns_vector3f_norm( &D );

	rayburst->single_vector.x = D.x;
	rayburst->single_vector.y = D.y;
	rayburst->single_vector.z = D.z;

	if( D.x < 0.0f )
		{
		rayburst->single_plane.d_delta.x = 1.0f;
		rayburst->single_plane.d.x       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.x = -1.0f;
		rayburst->single_plane.d.x       =  1.0f;
		}

	if( D.y < 0.0f )
		{
		rayburst->single_plane.d_delta.y = 1.0f;
		rayburst->single_plane.d.y       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.y = -1.0f;
		rayburst->single_plane.d.y       =  1.0f;
		}

	if( D.z < 0.0f )
		{
		rayburst->single_plane.d_delta.z = 1.0f;
		rayburst->single_plane.d.z       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.z = -1.0f;
		rayburst->single_plane.d.z       =  1.0f;
		}

	return NS_TRUE;
	}


NS_PRIVATE void _ns_rayburst_kernel_target( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	if( _ns_rayburst_acquire_target( rayburst, origin ) )
		if( _ns_rayburst_init( rayburst, origin, _ns_rayburst_valid_origin_3d ) )
			( ( NsRayburstRadiusFunc )rayburst->radius_func )( rayburst );
	}


NsError ns_rayburst_construct
	(
	NsRayburst            *rayburst,
	NsRayburstKernelType   kernel_type,
	NsRayburstRadiusType   radius_type,
	NsRayburstInterpType   interp_type,
	const NsImage         *image,
	const NsVoxelInfo     *voxel_info,
	NsRayburstVoxelFunc    voxel_func
	)
	{
	ns_assert( NULL != rayburst );
	ns_assert( NULL != image );
	ns_assert( NULL != voxel_info );

	_ns_rayburst_bilinear_funcs[__X] = _ns_rayburst_bilinear_x;
	_ns_rayburst_bilinear_funcs[__Y] = _ns_rayburst_bilinear_y;
	_ns_rayburst_bilinear_funcs[__Z] = _ns_rayburst_bilinear_z;

	ns_memzero( rayburst, sizeof( NsRayburst ) );

	rayburst->kernel_type = kernel_type;
	rayburst->radius_type = radius_type;
	rayburst->interp_type = interp_type;
	rayburst->voxel_info  = voxel_info;
	rayburst->threshold   = 0.0f;
	rayburst->render_func = NULL;
	rayburst->limit_rays  = NS_FALSE;

	ns_vector3f_zero( &rayburst->origin );

	ns_voxel_buffer_init( &rayburst->voxel_buffer, image );

	rayburst->width  = ns_voxel_buffer_width( &rayburst->voxel_buffer );
	rayburst->height = ns_voxel_buffer_height( &rayburst->voxel_buffer );
	rayburst->length = ns_voxel_buffer_length( &rayburst->voxel_buffer );

	if( NULL == voxel_func )
		{
		rayburst->voxel_func = _ns_rayburst_voxel_func;
		rayburst->user_data  = &rayburst->voxel_buffer;
		}
	else
		{
		rayburst->voxel_func = voxel_func;
		rayburst->user_data  = NULL;
		}

	rayburst->radius       = 0.0f;
	rayburst->volume       = 0.0;
	rayburst->surface_area = 0.0;

	switch( kernel_type )
		{
		case NS_RAYBURST_KERNEL_STD:
			rayburst->kernel_func  = ( NsFunc )_ns_rayburst_kernel_std;
			rayburst->num_samples  = NS_RAYBURST_STD_NUM_SAMPLES;
			rayburst->planes       = _ns_rayburst_std_planes;
			rayburst->vectors      = _ns_rayburst_std_vectors;
			rayburst->reverse_rays = NS_TRUE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_MLBD:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_mlbd_3d;
					rayburst->mlbd_sample = NS_RAYBURST_STD_MLBD_SAMPLE;
					rayburst->mlbd_count  = NS_RAYBURST_STD_MLBD_COUNT;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_2D:
			rayburst->kernel_func  = ( NsFunc )_ns_rayburst_kernel_2d;
			rayburst->num_samples  = NS_RAYBURST_2D_NUM_SAMPLES;
			rayburst->planes       = _ns_rayburst_2d_planes;
			rayburst->vectors      = _ns_rayburst_2d_vectors;
			rayburst->reverse_rays = NS_TRUE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_MLBD:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_mlbd_2d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_2D_VIZ:
			rayburst->kernel_func   = ( NsFunc )_ns_rayburst_kernel_2d_viz;
			rayburst->num_samples   = NS_RAYBURST_2D_VIZ_NUM_SAMPLES;
			rayburst->planes        = _ns_rayburst_2d_viz_planes;
			rayburst->vectors       = _ns_rayburst_2d_viz_vectors;
			rayburst->reverse_rays  = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_NONE:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_all_2d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_SUB_VIZ:
			rayburst->kernel_func   = ( NsFunc )_ns_rayburst_kernel_viz;
			rayburst->num_samples   = NS_RAYBURST_SUB_VIZ_NUM_SAMPLES;
			rayburst->planes        = _ns_rayburst_sub_viz_planes;
			rayburst->vectors       = _ns_rayburst_sub_viz_vectors;
			rayburst->triangles     = _ns_rayburst_sub_viz_triangles;
			rayburst->num_triangles = NS_RAYBURST_SUB_VIZ_NUM_TRIANGLES;
			rayburst->reverse_rays  = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_NONE:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_all_3d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_VIZ:
			rayburst->kernel_func   = ( NsFunc )_ns_rayburst_kernel_viz;
			rayburst->num_samples   = NS_RAYBURST_VIZ_NUM_SAMPLES;
			rayburst->planes        = _ns_rayburst_viz_planes;
			rayburst->vectors       = _ns_rayburst_viz_vectors;
			rayburst->triangles     = _ns_rayburst_viz_triangles;
			rayburst->num_triangles = NS_RAYBURST_VIZ_NUM_TRIANGLES;
			rayburst->reverse_rays  = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_NONE:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_all_3d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_SUPER_VIZ:
			rayburst->kernel_func   = ( NsFunc )_ns_rayburst_kernel_viz;
			rayburst->num_samples   = NS_RAYBURST_SUPER_VIZ_NUM_SAMPLES;
			rayburst->planes        = _ns_rayburst_super_viz_planes;
			rayburst->vectors       = _ns_rayburst_super_viz_vectors;
			rayburst->triangles     = _ns_rayburst_super_viz_triangles;
			rayburst->num_triangles = NS_RAYBURST_SUPER_VIZ_NUM_TRIANGLES;
			rayburst->reverse_rays  = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_NONE:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_all_3d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_HYPER_VIZ:
			rayburst->kernel_func   = ( NsFunc )_ns_rayburst_kernel_viz;
			rayburst->num_samples   = NS_RAYBURST_HYPER_VIZ_NUM_SAMPLES;
			rayburst->planes        = _ns_rayburst_hyper_viz_planes;
			rayburst->vectors       = _ns_rayburst_hyper_viz_vectors;
			rayburst->triangles     = _ns_rayburst_hyper_viz_triangles;
			rayburst->num_triangles = NS_RAYBURST_HYPER_VIZ_NUM_TRIANGLES;
			rayburst->reverse_rays  = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_NONE:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_all_3d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_SINGLE:
			rayburst->kernel_func  = ( NsFunc )_ns_rayburst_kernel_single;
			rayburst->num_samples  = NS_RAYBURST_SINGLE_NUM_SAMPLES;
			rayburst->planes       = &rayburst->single_plane;
			rayburst->vectors      = &rayburst->single_vector;
			rayburst->reverse_rays = NS_TRUE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_MLBD:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_mlbd_2d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_2d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_SINGLE_3D:
			rayburst->kernel_func  = ( NsFunc )_ns_rayburst_kernel_std;
			rayburst->num_samples  = NS_RAYBURST_SINGLE_NUM_SAMPLES;
			rayburst->planes       = &rayburst->single_plane;
			rayburst->vectors      = &rayburst->single_vector;
			rayburst->reverse_rays = NS_TRUE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_MLBD:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_mlbd_3d;
					rayburst->mlbd_sample = 0;
					rayburst->mlbd_count  = 1;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		case NS_RAYBURST_KERNEL_TARGET:
			rayburst->kernel_func  = ( NsFunc )_ns_rayburst_kernel_target;
			rayburst->num_samples  = NS_RAYBURST_TARGET_NUM_SAMPLES;
			rayburst->planes       = &rayburst->single_plane;
			rayburst->vectors      = &rayburst->single_vector;
			rayburst->reverse_rays = NS_FALSE;

			switch( radius_type )
				{
				case NS_RAYBURST_RADIUS_MLBD:
					rayburst->radius_func = ( NsFunc )_ns_rayburst_target_3d;
					break;

				default:
					ns_assert_not_reached();
				}

			switch( interp_type )
				{
				case NS_RAYBURST_INTERP_BILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_bilinear;
					break;

				case NS_RAYBURST_INTERP_TRILINEAR:
					rayburst->advance_func = ( NsFunc )_ns_rayburst_advance_3d_trilinear;
					break;
				}
			break;

		default:
			ns_assert_not_reached();
		}

	return ns_byte_array_construct(
				&rayburst->samples,
				sizeof( NsRayburstSample ) * rayburst->num_samples
				);
	}


NS_PRIVATE nsconstpointer _ns_rayburst_kernel_type_objects( NsRayburstKernelType type, nsboolean vectors )
	{
	nsconstpointer objects = NULL;

	switch( type )
		{
		case NS_RAYBURST_KERNEL_STD:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_std_vectors :
						 NULL;
			break;

		case NS_RAYBURST_KERNEL_2D:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_2d_vectors :
						 NULL;
			break;

		case NS_RAYBURST_KERNEL_2D_VIZ:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_2d_viz_vectors :
						 NULL;
			break;

		case NS_RAYBURST_KERNEL_SUB_VIZ:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_sub_viz_vectors :
						 ( nsconstpointer )_ns_rayburst_sub_viz_triangles;
			break;

		case NS_RAYBURST_KERNEL_VIZ:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_viz_vectors :
						 ( nsconstpointer )_ns_rayburst_viz_triangles;
			break;

		case NS_RAYBURST_KERNEL_SUPER_VIZ:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_super_viz_vectors :
						 ( nsconstpointer )_ns_rayburst_super_viz_triangles;
			break;

		case NS_RAYBURST_KERNEL_HYPER_VIZ:
			objects = vectors ?
						 ( nsconstpointer )_ns_rayburst_hyper_viz_vectors :
						 ( nsconstpointer )_ns_rayburst_hyper_viz_triangles;
			break;

		case NS_RAYBURST_KERNEL_SINGLE:
			objects = NULL;
			break;

		case NS_RAYBURST_KERNEL_TARGET:
			objects = NULL;
			break;
		}

	return objects;
	}


const NsVector3f* ns_rayburst_kernel_type_vectors( NsRayburstKernelType type )
	{  return _ns_rayburst_kernel_type_objects( type, NS_TRUE );  }


const NsIndexTriangleus* ns_rayburst_kernel_type_triangles( NsRayburstKernelType type )
	{  return _ns_rayburst_kernel_type_objects( type, NS_FALSE );  }


NS_PRIVATE nssize _ns_rayburst_kernel_type_size( NsRayburstKernelType type, nsboolean vectors )
	{
	nssize size = 0;

	switch( type )
		{
		case NS_RAYBURST_KERNEL_STD:
			size = vectors ?
						 NS_RAYBURST_STD_NUM_SAMPLES :
						 0;
			break;

		case NS_RAYBURST_KERNEL_2D:
			size = vectors ?
						 NS_RAYBURST_2D_NUM_SAMPLES :
						 0;
			break;

		case NS_RAYBURST_KERNEL_2D_VIZ:
			size = vectors ?
						 NS_RAYBURST_2D_VIZ_NUM_SAMPLES :
						 0;
			break;

		case NS_RAYBURST_KERNEL_SUB_VIZ:
			size = vectors ?
						 NS_RAYBURST_SUB_VIZ_NUM_SAMPLES :
						 NS_RAYBURST_SUB_VIZ_NUM_TRIANGLES;
			break;

		case NS_RAYBURST_KERNEL_VIZ:
			size = vectors ?
						 NS_RAYBURST_VIZ_NUM_SAMPLES :
						 NS_RAYBURST_VIZ_NUM_TRIANGLES;
			break;

		case NS_RAYBURST_KERNEL_SUPER_VIZ:
			size = vectors ?
						 NS_RAYBURST_SUPER_VIZ_NUM_SAMPLES :
						 NS_RAYBURST_SUPER_VIZ_NUM_TRIANGLES;
			break;

		case NS_RAYBURST_KERNEL_HYPER_VIZ:
			size = vectors ?
						 NS_RAYBURST_HYPER_VIZ_NUM_SAMPLES :
						 NS_RAYBURST_HYPER_VIZ_NUM_TRIANGLES;
			break;

		case NS_RAYBURST_KERNEL_SINGLE:
			size = 0;
			break;

		case NS_RAYBURST_KERNEL_TARGET:
			size = 0;
			break;
		}

	return size;
	}


nssize ns_rayburst_kernel_type_num_vectors( NsRayburstKernelType type )
	{  return _ns_rayburst_kernel_type_size( type, NS_TRUE );  }


nssize ns_rayburst_kernel_type_num_triangles( NsRayburstKernelType type )
	{  return _ns_rayburst_kernel_type_size( type, NS_FALSE );  }


nssize ns_rayburst_kernel_type_dimension( NsRayburstKernelType type )
	{
	nssize dim = 0;

	switch( type )
		{
		case NS_RAYBURST_KERNEL_STD:
		case NS_RAYBURST_KERNEL_SUB_VIZ:
		case NS_RAYBURST_KERNEL_VIZ:
		case NS_RAYBURST_KERNEL_SUPER_VIZ:
		case NS_RAYBURST_KERNEL_HYPER_VIZ:
		case NS_RAYBURST_KERNEL_TARGET:
			dim = 3;
			break;

		case NS_RAYBURST_KERNEL_2D:
		case NS_RAYBURST_KERNEL_2D_VIZ:
		case NS_RAYBURST_KERNEL_SINGLE:
			dim = 2;
			break;
		}

	return dim;
	}


void ns_rayburst_destruct( NsRayburst *rayburst )
	{
	ns_assert( NULL != rayburst );
	ns_byte_array_destruct( &rayburst->samples );
	}


void ns_rayburst_allow_dark_start( NsRayburst *rayburst, nsboolean b )
	{
	ns_assert( NULL != rayburst );
	rayburst->allow_dark_start = b;
	}


nsconstpointer ns_rayburst_get_user_data( const NsRayburst *rayburst )
	{
	ns_assert( NULL != rayburst );
	return rayburst->user_data;
	}


void ns_rayburst_set_user_data( NsRayburst *rayburst, nsconstpointer user_data )
	{
	ns_assert( NULL != rayburst );
	ns_assert( rayburst->voxel_func != _ns_rayburst_voxel_func );

	rayburst->user_data = user_data;
	}


nsfloat ns_rayburst_get_threshold( const NsRayburst *rayburst )
	{
	ns_assert( NULL != rayburst );
	return rayburst->threshold;
	}


void ns_rayburst_set_threshold( NsRayburst *rayburst, nsfloat threshold )
	{
	ns_assert( NULL != rayburst );
	rayburst->threshold = threshold;
	}


void ns_rayburst_limit_rays( NsRayburst *rayburst, nsboolean yes_or_no, nsdouble max_ray_distance_sq )
	{
	ns_assert( NULL != rayburst );

	rayburst->limit_rays = yes_or_no;

	if( rayburst->limit_rays )
		rayburst->max_ray_distance_sq = max_ray_distance_sq;
	}


void ns_rayburst_init_single( NsRayburst *rayburst, nsfloat dx, nsfloat dy )
	{
	ns_assert( NULL != rayburst );

	rayburst->single_vector.x = dx;
	rayburst->single_vector.y = dy;
	rayburst->single_vector.z = 0.0f;

	if( dx < 0.0f )
		{
		rayburst->single_plane.d_delta.x = 1.0f;
		rayburst->single_plane.d.x       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.x = -1.0f;
		rayburst->single_plane.d.x       =  1.0f;
		}

	if( dy < 0.0f )
		{
		rayburst->single_plane.d_delta.y = 1.0f;
		rayburst->single_plane.d.y       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.y = -1.0f;
		rayburst->single_plane.d.y       =  1.0f;
		}

	rayburst->single_plane.d_delta.z = -1.0f;
	rayburst->single_plane.d.z       =  1.0f;
	}


void ns_rayburst_init_single_ex( NsRayburst *rayburst, nsfloat dx, nsfloat dy, nsfloat dz )
	{
	ns_assert( NULL != rayburst );

	ns_rayburst_init_single( rayburst, dx, dy );

	rayburst->single_vector.z = dz;

	if( dz < 0.0f )
		{
		rayburst->single_plane.d_delta.z = 1.0f;
		rayburst->single_plane.d.z       = 0.0f;
		}
	else
		{
		rayburst->single_plane.d_delta.z = -1.0f;
		rayburst->single_plane.d.z       =  1.0f;
		}
	}


void ns_rayburst_init_target( NsRayburst *rayburst, const NsVector3f *target )
	{
	ns_assert( NULL != rayburst );
	ns_assert( NULL != target );

	rayburst->target = *target;
	}


void ns_rayburst( NsRayburst *rayburst, const NsPoint3f *origin )
	{
	ns_assert( NULL != rayburst );
	ns_assert( NULL != origin );
	ns_assert( NULL != rayburst->kernel_func );

	( ( NsRayburstKernelFunc )rayburst->kernel_func )( rayburst, origin );
	}


void ns_rayburst_mlbd
	(
	const NsRayburst  *rayburst,
	NsVector3f        *center,
	NsVector3f        *fwd,
	NsVector3f        *rev,
	nsfloat           *radius
	)
	{
	ns_assert( NULL != rayburst );

	if( NULL != fwd )
		*fwd = rayburst->mlbd_fwd_intercept;

	if( NULL != rev )
		*rev = rayburst->mlbd_rev_intercept;

	if( NULL != radius )
		*radius = rayburst->radius;

	if( NULL != center )
		{
		ns_assert( NULL != fwd && NULL != rev );

		ns_vector3f_add( center, fwd, rev );
		ns_vector3f_cmpd_scale( center, 0.5f );
		}
	}


void ns_rayburst_sample_distances
	(
	const NsRayburst  *rayburst,
	nsenum             which,
	nsfloat           *distances
	)
	{
	const NsRayburstSample  *samples;
	nssize                   i;


	ns_assert( NULL != rayburst );
	ns_assert( NULL != distances );

	samples = ( const NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	switch( which )
		{
		case NS_RAYBURST_SAMPLE_DISTANCE_FORWARD:
			for( i = 0; i < rayburst->num_samples; ++i )
				distances[i] = samples[i].fwd.distance;
			break;

		case NS_RAYBURST_SAMPLE_DISTANCE_REVERSE:
			for( i = 0; i < rayburst->num_samples; ++i )
				distances[i] = samples[i].rev.distance;
			break;

		case NS_RAYBURST_SAMPLE_DISTANCE_AVERAGE:
			for( i = 0; i < rayburst->num_samples; ++i )
				distances[i] = samples[i].distance;
			break;

		default:
			ns_assert_not_reached();
		}
	}


nsfloat ns_rayburst_sample_distance
	(
	const NsRayburst  *rayburst,
	nssize             index,
	nsenum             which
	)
	{
	const NsRayburstSample  *samples;
	nsfloat                  distance;


	ns_assert( NULL != rayburst );
	ns_assert( index < rayburst->num_samples );

	samples  = ( const NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );
	distance = 0.0f;

	switch( which )
		{
		case NS_RAYBURST_SAMPLE_DISTANCE_FORWARD:
			distance = samples[ index ].fwd.distance;
			break;

		case NS_RAYBURST_SAMPLE_DISTANCE_REVERSE:
			distance = samples[ index ].rev.distance;
			break;

		case NS_RAYBURST_SAMPLE_DISTANCE_AVERAGE:
			distance = samples[ index ].distance;
			break;

		default:
			ns_assert_not_reached();
		}

	return distance;
	}


const NsRayburstSample* _ns_rayburst_samples( const NsRayburst *rayburst )
	{
	return 0 < ns_byte_array_size( &rayburst->samples ) ?
			 ( const NsRayburstSample* )ns_byte_array_begin( &rayburst->samples ) :
			 NULL;
	}


void ns_rayburst_sample_points
	(
	const NsRayburst  *rayburst,
	nsboolean          forward,
	NsPoint3f         *points
	)
	{
	const NsRayburstSample  *samples;
	nssize                   i;


	ns_assert( NULL != rayburst );
	ns_assert( NULL != points );

	samples = ( const NsRayburstSample* )ns_byte_array_begin( &rayburst->samples );

	/* Get interception distance for every direction. */
	for( i = 0; i < rayburst->num_samples; ++i )
		{
		points[i] = forward ? samples[i].fwd.curr : samples[i].rev.curr;

		/* Convert back to voxel space! */
		ns_to_voxel_space_ex( points + i, points + i, rayburst->voxel_info );
		}
	}


nsboolean ns_rayburst_targeter
	(
	const NsVector3f   *origin,
	const NsVector3f   *target,
	nsfloat             threshold,
	const NsImage      *image,
	const NsVoxelInfo  *voxel_info
	)
	{
	NsRayburst  rayburst;
	nsboolean   was_hit;
	NsError     error;


	if( NS_FAILURE(
			ns_rayburst_construct(
				&rayburst,
				NS_RAYBURST_KERNEL_TARGET,
				NS_RAYBURST_RADIUS_MLBD,
				NS_RAYBURST_INTERP_BILINEAR,
				image,
				voxel_info,
				NULL
				),
			error ) )
		return NS_FALSE;

	ns_rayburst_init_target( &rayburst, target );
	ns_rayburst_set_threshold( &rayburst, threshold );

	ns_rayburst( &rayburst, origin );
	was_hit = ns_rayburst_target_was_hit( &rayburst );

	ns_rayburst_destruct( &rayburst );

	return was_hit;
	}


#include "nsrayburst-gen-2d.inl"
#include "nsrayburst-gen-3d.inl"
