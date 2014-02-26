#include "nsseed3d.h"


NS_PRIVATE void _ns_do_seed_find_get_ray_and_line
	(
	nsint            window_x,
	nsint            window_y,
	const GLdouble   modelview_matrix[16],
	const GLdouble   projection_matrix[16],
	NsRay3d         *R,
	NsLine3d        *L
	)
	{
   GLdouble  xi, yi, xn, yn, zn, xf, yf, zf;
   GLint     viewport[4];


   glGetIntegerv( GL_VIEWPORT, viewport );

   xi = window_x;
   yi = ( viewport[3] - window_y ) - 1;

   gluUnProject( xi, yi, 0.0, modelview_matrix, projection_matrix, viewport, &xn, &yn, &zn );
   gluUnProject( xi, yi, 1.0, modelview_matrix, projection_matrix, viewport, &xf, &yf, &zf );

   R->O.x = xn;
   R->O.y = yn;
   R->O.z = zn;

   R->D.x = xf - xn;
   R->D.y = yf - yn;
   R->D.z = zf - zn;

   ns_vector3d_norm( &R->D );

	if( NULL != L )
		{
		L->P1 = R->O;
		
		L->P2.x = xf;
		L->P2.y = yf;
		L->P2.z = zf;
		}
	}


void ns_seed_find_get_ray
	(
	nsint            window_x,
	nsint            window_y,
	const GLdouble   modelview_matrix[16],
	const GLdouble   projection_matrix[16],
	NsRay3d         *R
	)
	{
	_ns_do_seed_find_get_ray_and_line(
		window_x,
		window_y,
		modelview_matrix,
		projection_matrix,
		R,
		NULL
		);
	}


void ns_seed_find_get_ray_and_line
	(
	nsint            window_x,
	nsint            window_y,
	const GLdouble   modelview_matrix[16],
	const GLdouble   projection_matrix[16],
	NsRay3d         *R,
	NsLine3d        *L
	)
	{
	_ns_do_seed_find_get_ray_and_line(
		window_x,
		window_y,
		modelview_matrix,
		projection_matrix,
		R,
		L
		);
	}


NsError ns_seed_find_3d
	(
	const NsVoxelInfo  *voxel_info,
	const NsImage      *image,
	const NsCubei      *roi,
	const NsSplats     *splats,
	NsRay3d            *R,
	NsVector3i         *seed,
	nsboolean          *found,
	NsProgress         *progress
	)
	{
	NsVector3d     S;
	NsVoxelBuffer  voxels;
	nsulong        intensity;
	NsRayburst     rayburst;
	NsVector3f     P, N, E, M;
	NsVector3i     I;
	NsError        error;


	ns_assert( NULL != voxel_info );
	ns_assert( NULL != image );
	ns_assert( NULL != roi );
	ns_assert( NULL != splats );
	ns_assert( NULL != R );
	ns_assert( NULL != seed );
	ns_assert( NULL != found );

	*found = NS_FALSE;

	if( ns_splats_find_closest_by_ray( splats, voxel_info, R, &S, progress ) )
		{
		ns_vector3d_to_3f( &S, &P );
		ns_to_image_space( &P, &I, voxel_info );

		if( ns_point3i_inside_cube( &I, roi ) )
			{
			if( NS_FAILURE(
					ns_rayburst_construct(
						&rayburst,
						NS_RAYBURST_KERNEL_SINGLE_3D,
						NS_RAYBURST_RADIUS_MLBD,
						NS_RAYBURST_INTERP_BILINEAR,
						image,
						voxel_info,
						NULL
						),
					error ) )
				return error;

			ns_vector3d_to_3f( &R->D, &N );
			ns_vector3f_norm( &N );
			ns_rayburst_init_single_ex( &rayburst, N.x, N.y, N.z );

			ns_voxel_buffer_init( &voxels, image );
			intensity = ns_voxel_get( &voxels, I.x, I.y, I.z );

			/* Only pass 99.9% of the splats threshold value to avoid the rayburst from
				terminating at the origin due to roundoff error in the interpolation of
				the threshold at the origin. See also nsrayburst.c. */
			ns_rayburst_set_threshold( &rayburst, ( nsfloat )intensity * 0.999f );
			ns_rayburst( &rayburst, &P );

			if( 0.0f < ns_rayburst_radius( &rayburst ) )
				{
				ns_assert( 1 == ns_rayburst_num_samples( &rayburst ) );
				ns_rayburst_sample_points( &rayburst, NS_TRUE, &E );

				ns_vector3f_add( &M, &P, &E );
				ns_vector3f_cmpd_scale( &M, 0.5f );
				
				ns_to_image_space( &M, seed, voxel_info );
				*found = NS_TRUE;
				}

			ns_rayburst_destruct( &rayburst );
			}
		}

	return ns_no_error();
	}
