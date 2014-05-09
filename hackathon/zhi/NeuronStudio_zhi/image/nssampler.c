#include "nssampler.h"


typedef struct _NsSample
   {
   NsSampler   *owner;
   nslong       id;

   nsint        user_type;

	nsboolean    is_selected;

   nsulong      threshold;
   nsfloat      exact_threshold;

	nsfloat      contrast;

   NsVector3f   O;
   NsVector3f   C;
   NsVector3f   D;

   NsVector3f   R1;
   NsVector3f   R2;

   NsVector3f   P1;
   NsVector3f   P2;

   nsfloat      radius;
   nsfloat      length;

   nsfloat      angle;

   nsdouble     volume;
   nsdouble     surface_area;

   nssize       num_steps;
   nssize       curr_step;

   nssize       image_width;
   nssize       image_height;
   nssize       image_length;

   NsImage      image_xy;
   NsImage      image_zy;
   NsImage      image_xz;

   NsByteArray  distances;
   NsByteArray  normals;
   nssize       num_vectors;

   NsRayburstKernelType  kernel_type;

   NsVector3i   C1;
   NsVector3i   C2;
   }
   NsSample;


NS_PRIVATE NsError _ns_sample_new( NsSample **ret_sample, NsSampler *owner )
   {
   NsSample  *sample;
   NsError    error;


   if( NULL == ( sample = ns_new0( NsSample ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   ns_image_construct( &sample->image_xy );
   ns_image_construct( &sample->image_zy );
   ns_image_construct( &sample->image_xz );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &sample->distances, 0 ), error ) );
   ns_verify( NS_SUCCESS( ns_byte_array_construct( &sample->normals, 0 ), error ) );

   sample->num_vectors = 0;

   sample->owner = owner;

   *ret_sample = sample;
   return ns_no_error();
   }


NS_PRIVATE void _ns_sample_delete( NsSample *sample )
   {
   ns_byte_array_destruct( &sample->distances );
   ns_byte_array_destruct( &sample->normals );

   ns_image_destruct( &sample->image_xy );
   ns_image_destruct( &sample->image_zy );
   ns_image_destruct( &sample->image_xz );

   ns_delete( sample );
   }


void ns_sampler_construct( NsSampler *sampler )
   {
   ns_assert( NULL != sampler );
   ns_memzero( sampler, sizeof( NsSampler ) );

   sampler->lut_lum_u8 = ns_voxel_buffer_lut_lum_u8();

   ns_list_construct( &sampler->samples, _ns_sample_delete );

   sampler->is_running = NS_FALSE;

   ns_mutex_construct( &sampler->mutex );
	ns_mutex_set_name( &sampler->mutex, "sampler->mutex" );
   ns_mutex_create( &sampler->mutex ); /* NOTE: Ignore failure. */
   }


void ns_sampler_destruct( NsSampler *sampler )
   {
   ns_assert( NULL != sampler );

   ns_list_destruct( &sampler->samples );
   ns_mutex_destruct( &sampler->mutex );
   }


#define _ns_sampler_mutex( sampler )\
   ( ( NsMutex* )( &(sampler)->mutex ) )


void ns_sampler_lock( const NsSampler *sampler )
   {
   NsError error;

   ns_assert( NULL != sampler );

   if( ! ns_mutex_is_created( _ns_sampler_mutex( sampler ) ) ||
       NS_FAILURE( ns_mutex_lock( _ns_sampler_mutex( sampler ) ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );
   }


nsboolean ns_sampler_try_lock( const NsSampler *sampler )
   {
   NsError    error;
   nsboolean  did_lock = NS_FALSE;


   ns_assert( NULL != sampler );

   if( ! ns_mutex_is_created( _ns_sampler_mutex( sampler ) ) ||
       NS_FAILURE( ns_mutex_try_lock( _ns_sampler_mutex( sampler ), &did_lock ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );

   return did_lock;
   }


void ns_sampler_unlock( const NsSampler *sampler )
   {
   NsError error;

   ns_assert( NULL != sampler );

   if( ! ns_mutex_is_created( _ns_sampler_mutex( sampler ) ) ||
       NS_FAILURE( ns_mutex_unlock( _ns_sampler_mutex( sampler ) ), error ) )
      ns_warning(
         NS_WARNING_LEVEL_CRITICAL
         NS_MODULE
         " " __FUNCTION__ " failed."
         );
   }


void ns_sampler_init
   (
   NsSampler          *sampler,
   const NsVoxelInfo  *voxel_info,
   const NsImage      *image,
   const NsImage      *proj_xy,
   const NsImage      *proj_zy,
   const NsImage      *proj_xz
   )
   {
   ns_assert( NULL != sampler );
   ns_assert( NULL != voxel_info );
   ns_assert( NULL != proj_xy );
   ns_assert( NULL != proj_zy );
   ns_assert( NULL != proj_xz );

   sampler->voxel_info = voxel_info;

   ns_voxel_buffer_init( &sampler->src, image );

   //ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_xy ) );
   //ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_zy ) );
   //ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_xz ) );

	if( NS_PIXEL_UNSUPPORTED != ns_image_pixel_type( proj_xy ) )
		ns_voxel_buffer_init( &sampler->src_xy, proj_xy );

	if( NS_PIXEL_UNSUPPORTED != ns_image_pixel_type( proj_zy ) )
		ns_voxel_buffer_init( &sampler->src_zy, proj_zy );

	if( NS_PIXEL_UNSUPPORTED != ns_image_pixel_type( proj_xz ) )
		ns_voxel_buffer_init( &sampler->src_xz, proj_xz );
   }


nssize ns_sampler_size( const NsSampler *sampler )
   {
   ns_assert( NULL != sampler );
   return ns_list_size( &sampler->samples );
   }


void ns_sampler_clear( NsSampler *sampler )
   {
   ns_assert( NULL != sampler );

   ns_sampler_lock( sampler );
   ns_list_clear( &sampler->samples );
   ns_sampler_unlock( sampler );
   }


#ifdef NS_DEBUG

NS_PRIVATE nsulong _ns_sampler_lut( NsSampler *sampler, nsulong intensity )
   {
   ns_assert( intensity <= ( nsulong )NS_UINT8_MAX );
   return ( nsulong )( sampler->lut_lum_u8[ intensity ] );
   }

#else

#define _ns_sampler_lut( sampler, intensity )\
   ( ( nsulong )( (sampler)->lut_lum_u8[ (intensity) ] ) )

#endif/* NS_DEBUG */


#define _ns_sample( handle )\
   ( ( NsSample* )ns_list_iter_get_object( ( nslistiter )(handle) ) )


NS_PRIVATE void _ns_sampler_reset_ids( NsSampler *sampler )
   {
   nslistiter   curr_sample;
   NsSample    *sample;
   nslong       id;


   id = 1;

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );
      sample->id = id++;
      }
   }


NsError ns_sampler_add( NsSampler *sampler, nspointer *handle )
   {
   NsSample  *sample;
   NsError    error;


   ns_assert( NULL != sampler );
   ns_assert( NULL != handle );

   *handle = NULL;

   error = ns_no_error();
   ns_sampler_lock( sampler );

   if( NS_FAILURE( _ns_sample_new( &sample, sampler ), error ) )
      goto _NS_SAMPLER_ADD_EXIT;

   if( NS_FAILURE( ns_list_push_back( &sampler->samples, sample ), error ) )
      {
      _ns_sample_delete( sample );
      goto _NS_SAMPLER_ADD_EXIT;
      }

   _ns_sampler_reset_ids( sampler );

   *handle = ns_list_rev_begin( &sampler->samples );

   _NS_SAMPLER_ADD_EXIT:

   ns_sampler_unlock( sampler );
   return error;
   }


void ns_sampler_remove( NsSampler *sampler, nspointer handle )
   {
   ns_assert( NULL != sampler );

   if( NULL != handle )
      {
      ns_sampler_lock( sampler );

      ns_list_erase( &sampler->samples, handle );
      _ns_sampler_reset_ids( sampler );

      ns_sampler_unlock( sampler );
      }
   }


void ns_sampler_remove_selected( NsSampler *sampler )
   {
	nslistiter curr, next, end;

	ns_assert( NULL != sampler );

	ns_sampler_lock( sampler );

	curr = ns_list_begin( &sampler->samples );
	end  = ns_list_end( &sampler->samples );

	while( ns_list_iter_not_equal( curr, end ) )
		{
		next = ns_list_iter_next( curr );

		if( _ns_sample( curr )->is_selected )
			ns_list_erase( &sampler->samples, curr );

		curr = next;
		}

	_ns_sampler_reset_ids( sampler );

	ns_sampler_unlock( sampler );
   }


void ns_sampler_select( NsSampler *sampler, nsboolean selected )
	{
	nslistiter curr, end;

	ns_assert( NULL != sampler );

	curr = ns_list_begin( &sampler->samples );
	end  = ns_list_end( &sampler->samples );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		_ns_sample( curr )->is_selected = selected;
	}


void ns_sampler_foreach
	(
	NsSampler *sampler,
	void ( *callback )( NsSampler *sampler, nspointer sample, nspointer user_data ),
	nspointer user_data
	)
	{
	nslistiter curr, end;

	ns_assert( NULL != sampler );
	ns_assert( NULL != callback );

	curr = ns_list_begin( &sampler->samples );
	end  = ns_list_end( &sampler->samples );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		( callback )( sampler, curr, user_data );
	}


nspointer ns_sampler_begin( const NsSampler *sampler )
	{
	ns_assert( NULL != sampler );
	return ns_list_begin( &sampler->samples );
	}


nspointer ns_sampler_end( const NsSampler *sampler )
	{
	ns_assert( NULL != sampler );
	return ns_list_end( &sampler->samples );
	}


NS_PRIVATE NsVector3f* _ns_sample_radii( const NsSample *sample, NsVector3f *R )
	{
	ns_vector3f( R, sample->radius, sample->radius, sample->radius );
	return R;
	}


NsSpheref* ns_sample_to_sphere( const NsSampler *sampler, nspointer handle, NsSpheref *S )
	{
	NsSample *sample;

	ns_assert( NULL != sampler );
	ns_assert( NULL != handle );
	ns_assert( NULL != S );

	sample = _ns_sample( handle );

	S->O = sample->C;
	S->r = sample->radius;

	return S;
	}


NsSphered* ns_sample_to_sphere_ex( const NsSampler *sampler, nspointer handle, NsSphered *Sd )
	{
	NsSpheref Sf;

	ns_assert( NULL != sampler );
	ns_assert( NULL != handle );
	ns_assert( NULL != Sd );

	ns_sample_to_sphere( sampler, handle, &Sf );

	ns_vector3f_to_3d( &Sf.O, &Sd->O );
	Sd->r = ( nsdouble )Sf.r;

	return Sd;
	}


NsError ns_sampler_select_by_aabbox( NsSampler *sampler, const NsAABBox3d *B )
   {
   nslistiter   curr, end;
   NsList       intersected;
   nslistiter   iter;
   nsboolean    any_selected;
	NsSample    *sample;
   NsAABBox3d   S;
	NsVector3f   R;
   NsError      error;


	ns_assert( NULL != sampler );
	ns_assert( NULL != B );

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the samples and store the ones that intersect
      this bounding box. */

   curr = ns_list_begin( &sampler->samples );
   end  = ns_list_end( &sampler->samples );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		sample = _ns_sample( curr );

		ns_model_position_and_radii_to_aabbox(
			&sample->C,
			_ns_sample_radii( sample, &R ),
			&S
			);

      if( ns_aabbox3d_intersects_aabbox( &S, B ) )
         {
         if( NS_FAILURE( ns_list_push_back( &intersected, sample ), error ) )
            {
            ns_list_destruct( &intersected );
            return error;
            }

         if( sample->is_selected )
            any_selected = NS_TRUE;
         }
		}

   /* If any of the intersecting samples are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		( ( NsSample* )ns_list_iter_get_object( iter ) )->is_selected = ! any_selected;

   ns_list_destruct( &intersected );
   return ns_no_error();
   }


NsError ns_sampler_select_by_projection_onto_drag_rect
	(
	NsSampler          *sampler,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj
	)
	{
   nslistiter   curr, end;
   NsList       intersected;
   nslistiter   iter;
   nsboolean    any_selected;
	NsSample    *sample;
   NsSphered    S;
   NsError      error;


	ns_assert( NULL != sampler );
	ns_assert( NULL != drag_rect );
	ns_assert( NULL != proj );

   ns_list_construct( &intersected, NULL );

   any_selected = NS_FALSE;

   /* Go through the samples and store the ones that intersect
      this measurement (as a sphere). */

   curr = ns_list_begin( &sampler->samples );
   end  = ns_list_end( &sampler->samples );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		sample = _ns_sample( curr );

		ns_vector3f_to_3d( &sample->C, &S.O );
		S.r = ( nsdouble )sample->radius;

      if( ns_sphered_projects_onto_drag_rect( &S, drag_rect, proj ) )
         {
         if( NS_FAILURE( ns_list_push_back( &intersected, sample ), error ) )
            {
            ns_list_destruct( &intersected );
            return error;
            }

         if( sample->is_selected )
            any_selected = NS_TRUE;
         }
		}

   /* If any of the intersecting samples are selected then clear
      them all. Otherwise set them all as selected. */
   NS_LIST_FOREACH( &intersected, iter )
		( ( NsSample* )ns_list_iter_get_object( iter ) )->is_selected = ! any_selected;

   ns_list_destruct( &intersected );
   return ns_no_error();
	}


nspointer ns_sampler_first( const NsSampler *sampler )
   {
   ns_assert( NULL != sampler );
   ns_assert( ! ns_sampler_is_empty( sampler ) );

   return ns_list_begin( &sampler->samples );
   }


nspointer ns_sampler_last( const NsSampler *sampler )
   {
   ns_assert( NULL != sampler );
   ns_assert( ! ns_sampler_is_empty( sampler ) );

   return ns_list_rev_begin( &sampler->samples );
   }


nsfloat ns_sample_get_threshold( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->exact_threshold : 0.0f;  }


void ns_sample_set_threshold( nspointer handle, nsfloat threshold )
   {
   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      sample->exact_threshold = threshold;

      sample->threshold = ns_voxel_buffer_convert_to_lum_u8(
                           &(sample->owner->src),
                           ( nsulong )threshold
                           );
      }
   }


nsfloat ns_sample_get_contrast( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->contrast : 0.0f;  }


void ns_sample_set_contrast( nspointer handle, nsfloat contrast )
	{
   if( NULL != handle )
      _ns_sample( handle )->contrast = contrast;
	}


void ns_sample_get_origin( nsconstpointer handle, NsVector3f *O )
   {
   ns_assert( NULL != O );

   if( NULL != handle )
      *O = _ns_sample( handle )->O;
   }


void ns_sample_set_origin( nspointer handle, const NsVector3f *O )
   {
   ns_assert( NULL != O );

   if( NULL != handle )
      _ns_sample( handle )->O = *O;
   }


void ns_sample_get_center( nsconstpointer handle, NsVector3f *C )
   {
   ns_assert( NULL != C );

   if( NULL != handle )
      *C = _ns_sample( handle )->C;
   }


void ns_sample_set_center( nspointer handle, const NsVector3f *C )
   {
   ns_assert( NULL != C );

   if( NULL != handle )
      _ns_sample( handle )->C = *C;
   }


nsfloat ns_sample_get_angle( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->angle : 0.0f;  }


void ns_sample_set_angle( nspointer handle, nsfloat angle )
   {
   if( NULL != handle )
      _ns_sample( handle )->angle = angle;
   }


nsdouble ns_sample_get_volume( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->volume : 0.0;  }


void ns_sample_set_volume( nspointer handle, nsdouble volume )
   {
   if( NULL != handle )
      _ns_sample( handle )->volume = volume;
   }


nsdouble ns_sample_get_surface_area( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->surface_area : 0.0;  }


void ns_sample_set_surface_area( nspointer handle, nsdouble surface_area )
   {
   if( NULL != handle )
      _ns_sample( handle )->surface_area = surface_area;
   }


nssize ns_sample_get_num_steps( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->num_steps : 0;  }


void ns_sample_set_num_steps( nspointer handle, nssize num_steps )
   {
   if( NULL != handle )
      _ns_sample( handle )->num_steps = num_steps;
   }


nssize ns_sample_get_curr_step( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->curr_step : 0;  }


void ns_sample_set_curr_step( nspointer handle, nssize curr_step )
   {
   if( NULL != handle )
      _ns_sample( handle )->curr_step = curr_step;
   }


nsfloat ns_sample_get_radius( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->radius : 0.0f;  }


void ns_sample_set_radius( nspointer handle, nsfloat radius )
   {
   if( NULL != handle )
      _ns_sample( handle )->radius = radius;
   }


void ns_sample_change_radius( nspointer handle, nsfloat percent, nsfloat min_radius )
	{
	if( NULL != handle )
		{
		NsSample    *sample;
		nsfloat      radius;
		NsVector3f   D1, D2;


		sample = _ns_sample( handle );

		/* Increase or decrease radius by a percentage. */
		radius = sample->radius * ( percent / 100.0f );

		if( radius < min_radius )
			radius = min_radius;

		sample->radius = radius;

		/* Have to re-position the sample's rays. */

		/* Make unit direction vectors. */
		ns_vector3f_sub( &D1, &sample->R1, &sample->C );
		ns_vector3f_sub( &D2, &sample->R2, &sample->C );
		ns_vector3f_norm( &D1 );
		ns_vector3f_norm( &D2 );

		/* Scale the direction vectors and add to center. */
		ns_vector3f_cmpd_scale( &D1, radius );
		ns_vector3f_cmpd_scale( &D2, radius );
		ns_vector3f_add( &sample->R1, &sample->C, &D1 );
		ns_vector3f_add( &sample->R2, &sample->C, &D2 );
		}
	}


nsfloat ns_sample_get_length( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->length : 0.0f;  }


void ns_sample_get_unit_direction( nsconstpointer handle, NsVector3f *D )
   {
   if( NULL != handle )
      *D = _ns_sample( handle )->D;
   }


void ns_sample_get_endpoints( nsconstpointer handle, NsVector3f *P1, NsVector3f *P2 )
   {
   ns_assert( NULL != P1 );
   ns_assert( NULL != P2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      *P1 = sample->P1;
      *P2 = sample->P2;
      }
   }


void ns_sample_set_endpoints( nspointer handle, const NsVector3f *P1, const NsVector3f *P2 )
   {
   ns_assert( NULL != P1 );
   ns_assert( NULL != P2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      sample->P1 = *P1;
      sample->P2 = *P2;

      ns_vector3f_sub( &sample->D, P2, P1 );

      sample->length = ns_vector3f_mag( &sample->D );

      ns_vector3f_norm( &sample->D );
      }
   }


void ns_sample_get_rays( nsconstpointer handle, NsVector3f *R1, NsVector3f *R2 )
   {
   ns_assert( NULL != R1 );
   ns_assert( NULL != R2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      *R1 = sample->R1;
      *R2 = sample->R2;
      }
   }


void ns_sample_set_rays( nspointer handle, const NsVector3f *R1, const NsVector3f *R2 )
   {
   ns_assert( NULL != R1 );
   ns_assert( NULL != R2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      sample->R1 = *R1;
      sample->R2 = *R2;
      }
   }


void ns_sample_get_corners( nsconstpointer handle, NsVector3i *C1, NsVector3i *C2 )
   {
   ns_assert( NULL != C1 );
   ns_assert( NULL != C2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      *C1 = sample->C1;
      *C2 = sample->C2;
      }
   }


void ns_sample_set_corners( nspointer handle, const NsVector3i *C1, const NsVector3i *C2 )
   {
   ns_assert( NULL != C1 );
   ns_assert( NULL != C2 );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      sample->C1 = *C1;
      sample->C2 = *C2;

      ns_assert( C1->x <= C2->x );
      ns_assert( C1->y <= C2->y );
      ns_assert( C1->z <= C2->z );

      sample->image_width  = ( nssize )( C2->x - C1->x ) + 1;
      sample->image_height = ( nssize )( C2->y - C1->y ) + 1;
      sample->image_length = ( nssize )( C2->z - C1->z ) + 1;
      }
   }


void ns_sample_get_distances( nsconstpointer handle, const nsfloat **distances, nssize *num_distances )
   {
   ns_assert( NULL != distances );
   ns_assert( NULL != num_distances );

   if( NULL != handle )
      {
      NsSample *sample = _ns_sample( handle );

      *distances = ( 0 < sample->num_vectors && 0 < ns_byte_array_size( &sample->distances ) ) ?
                   ( const nsfloat* )ns_byte_array_begin( &sample->distances ) : NULL;

      *num_distances = sample->num_vectors;
      }
   }


void ns_sample_set_distances( nspointer handle, const nsfloat *distances, nssize num_distances )
   {
   ns_assert( NULL != distances );
   ns_assert( 0 < num_distances );

   if( NULL != handle )
      {
      NsSample                 *sample;
      nssize                    bytes;
      nssize                    num_triangles;
      const NsVector3f         *vectors;
      const NsIndexTriangleus  *triangles;
      NsVector3f               *normals;
      NsTriangle3f              T;
      nssize                    i;
      NsVector3f                BA, CA, N;
      NsError                   error;


      sample = _ns_sample( handle );

      bytes = num_distances * sizeof( nsfloat );
      if( NS_FAILURE( ns_byte_array_resize( &sample->distances, bytes ), error ) )
         return;

      ns_byte_array_memcpy( &sample->distances, distances, bytes );

      sample->num_vectors = num_distances;

      if( NULL != distances && 0 < num_distances )
         {
         vectors = ns_rayburst_kernel_type_vectors( sample->kernel_type );
         ns_assert( NULL != vectors );

         triangles     = ns_rayburst_kernel_type_triangles( sample->kernel_type );
         num_triangles = ns_rayburst_kernel_type_num_triangles( sample->kernel_type );

         if( NULL != triangles )
            {
            bytes = num_distances * sizeof( NsVector3f );
            if( NS_FAILURE( ns_byte_array_resize( &sample->normals, bytes ), error ) )
               return;

            normals = ( NsVector3f* )ns_byte_array_begin( &sample->normals );

            for( i = 0; i < num_distances; ++i )
               ns_vector3f_zero( normals + i );

            for( i = 0; i < num_triangles; ++i )
               {
               T.A = vectors[ triangles[i].a ];
               T.B = vectors[ triangles[i].b ];
               T.C = vectors[ triangles[i].c ];

               ns_vector3f_cmpd_scale( &T.A, distances[ triangles[i].a ] );
               ns_vector3f_cmpd_scale( &T.B, distances[ triangles[i].b ] );
               ns_vector3f_cmpd_scale( &T.C, distances[ triangles[i].c ] );

               ns_vector3f_cmpd_add( &T.A, &sample->C );
               ns_vector3f_cmpd_add( &T.B, &sample->C );
               ns_vector3f_cmpd_add( &T.C, &sample->C );

               ns_vector3f_sub( &BA, &T.B, &T.A ); /* BA = B - A */
               ns_vector3f_sub( &CA, &T.C, &T.A ); /* CA = C - A */

               ns_vector3f_cross( &N, &BA, &CA ); /* N = BA x CA */
               ns_vector3f_norm( &N );

               ns_vector3f_cmpd_add( normals + triangles[i].a, &N );
               ns_vector3f_cmpd_add( normals + triangles[i].b, &N );
               ns_vector3f_cmpd_add( normals + triangles[i].c, &N );
               }

            for( i = 0; i < num_distances; ++i )
               ns_vector3f_norm( normals + i );
            }
         }
      }
   }


NsRayburstKernelType ns_sample_get_rayburst_kernel_type( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->kernel_type : NS_RAYBURST_KERNEL_UNKNOWN;  }


void ns_sample_set_rayburst_kernel_type( nspointer handle, NsRayburstKernelType kernel_type )
   {
   if( NULL != handle )
      _ns_sample( handle )->kernel_type = kernel_type;
   }


nsint ns_sample_get_user_type( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->user_type : 0;  }


void ns_sample_set_user_type( nspointer handle, nsint user_type )
   {
   if( NULL != handle )
      _ns_sample( handle )->user_type = user_type;
   }


nsboolean ns_sample_get_is_selected( nsconstpointer handle )
   {  return NULL != handle ? _ns_sample( handle )->is_selected : NS_FALSE;  }


void ns_sample_set_is_selected( nspointer handle, nsboolean is_selected )
   {
   if( NULL != handle )
      _ns_sample( handle )->is_selected = is_selected;
   }


NS_PRIVATE NsError _ns_sampler_make_images_lum_u8( NsSampler *sampler, NsSample *sample )
   {
   NsVoxelBuffer  dest_xy, dest_zy, dest_xz;
   nsulong        threshold, intensity;
   NsError        error;
   nsint          dx, dy, dz, sx, sy, sz, c1x, c1y, c1z, c2x, c2y, c2z;


   /* IMPORTANT: Windows bitmap scan-lines must be aligned
      to LONG boundaries, which should be 4 bytes. */
   #ifdef NS_OS_WINDOWS
      #define __ROW_ALIGN  4  //sizeof( LONG )
   #else
      #define __ROW_ALIGN  1
   #endif

   if( NS_FAILURE( ns_image_create(
                     &sample->image_xy,
                     NS_PIXEL_LUM_U8,
                     sample->image_width,
                     sample->image_height,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_image_create(
                     &sample->image_zy,
                     NS_PIXEL_LUM_U8,
                     sample->image_length,
                     sample->image_height,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_image_create(
                     &sample->image_xz,
                     NS_PIXEL_LUM_U8,
                     sample->image_width,
                     sample->image_length,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   ns_image_zero( &sample->image_xy );
   ns_image_zero( &sample->image_zy );
   ns_image_zero( &sample->image_xz );

   ns_voxel_buffer_init( &dest_xy, &sample->image_xy );
   ns_voxel_buffer_init( &dest_zy, &sample->image_zy );
   ns_voxel_buffer_init( &dest_xz, &sample->image_xz );

   threshold = sample->threshold;

   c1x = sample->C1.x;
   c1y = sample->C1.y;
   c1z = sample->C1.z;
   c2x = sample->C2.x;
   c2y = sample->C2.y;
   c2z = sample->C2.z;

   for( dy = 0, sy = c1y; sy <= c2y; ++dy, ++sy )
      for( dx = 0, sx = c1x; sx <= c2x; ++dx, ++sx )
         {
         intensity = ns_voxel_get( &sampler->src_xy, sx, sy, 0 );

         if( threshold <= intensity )
            ns_voxel_set( &dest_xy, dx, dy, 0, _ns_sampler_lut( sampler, intensity ) );
         }

   for( dy = 0, sy = c1y; sy <= c2y; ++dy, ++sy )
      for( dz = 0, sz = c1z; sz <= c2z; ++dz, ++sz )
         {
         intensity = ns_voxel_get( &sampler->src_zy, sz, sy, 0 );

         if( threshold <= intensity )
            ns_voxel_set( &dest_zy, dz, dy, 0, _ns_sampler_lut( sampler, intensity ) );
         }

   for( dz = 0, sz = c1z; sz <= c2z; ++dz, ++sz )
      for( dx = 0, sx = c1x; sx <= c2x; ++dx, ++sx )
         {
         intensity = ns_voxel_get( &sampler->src_xz, sx, sz, 0 );

         if( threshold <= intensity )
            ns_voxel_set( &dest_xz, dx, dz, 0, _ns_sampler_lut( sampler, intensity ) );
         }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_sampler_make_images_rgb_u8_u8_u8( NsSampler *sampler, NsSample *sample )
   {
   NsImage        lum_u8;
   NsVoxelBuffer  dest;
   nsulong        threshold, intensity;
   NsError        error;
   nsint          dx, dy, dz, sx, sy, sz, c1x, c1y, c1z, c2x, c2y, c2z/*, last_z*/;
   NsColor3ub     C;
   NsPixel        pixel;
   nsuint8        channel;


   /* IMPORTANT: Windows bitmap scan-lines must be aligned
      to LONG boundaries, which should be 4 bytes. */
   #ifdef NS_OS_WINDOWS
      #define __ROW_ALIGN  4  //sizeof( LONG )
   #else
      #define __ROW_ALIGN  1
   #endif

   if( NS_FAILURE( ns_image_create(
                     &sample->image_xy,
                     NS_PIXEL_RGB_U8_U8_U8,
                     sample->image_width,
                     sample->image_height,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_image_create(
                     &sample->image_zy,
                     NS_PIXEL_RGB_U8_U8_U8,
                     sample->image_length,
                     sample->image_height,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_image_create(
                     &sample->image_xz,
                     NS_PIXEL_RGB_U8_U8_U8,
                     sample->image_width,
                     sample->image_length,
                     1,
                     __ROW_ALIGN
                     ),
                     error ) )
      return error;

   ns_image_construct( &lum_u8 );
   ns_verify( NS_SUCCESS( ns_image_create( &lum_u8, NS_PIXEL_LUM_U8, 1, 1, 1, 1 ), error ) );

   ns_voxel_buffer_init( &dest, &lum_u8 );

   threshold = sample->threshold;

   c1x = sample->C1.x;
   c1y = sample->C1.y;
   c1z = sample->C1.z;
   c2x = sample->C2.x;
   c2y = sample->C2.y;
   c2z = sample->C2.z;

   for( dy = 0, sy = c1y; sy <= c2y; ++dy, ++sy )
      for( dx = 0, sx = c1x; sx <= c2x; ++dx, ++sx )
         {
         intensity = ns_voxel_get( &sampler->src_xy, sx, sy, 0 );
         ns_voxel_set( &dest, 0, 0, 0, intensity );
         channel = ( nsuint8 )ns_voxel_get( &dest, 0, 0, 0 );

         C.x = C.y = C.z = 0;

         if( threshold <= intensity )
            NS_COLOR_SET_GREEN_U8( C, channel );
         else
            NS_COLOR_SET_RED_U8( C, channel );

         ns_pixel_init_rgb_u8_u8_u8( &pixel, C.x, C.y, C.z );
         ns_image_set_pixel_ex( &sample->image_xy, dx, dy, 0, &pixel );
         }

   for( dy = 0, sy = c1y; sy <= c2y; ++dy, ++sy )
      for( dz = 0, sz = c1z; sz <= c2z; ++dz, ++sz )
         {
         intensity = ns_voxel_get( &sampler->src_zy, sz, sy, 0 );
         ns_voxel_set( &dest, 0, 0, 0, intensity );
         channel = ( nsuint8 )ns_voxel_get( &dest, 0, 0, 0 );

         C.x = C.y = C.z = 0;

         if( threshold <= intensity )
            NS_COLOR_SET_GREEN_U8( C, channel );
         else
            NS_COLOR_SET_RED_U8( C, channel );

         ns_pixel_init_rgb_u8_u8_u8( &pixel, C.x, C.y, C.z );
         ns_image_set_pixel_ex( &sample->image_zy, dz, dy, 0, &pixel );
         }

   /* IMPORTANT: Assuming the XZ projection has been flipped in
      the z direction, i.e. vertically. */
   /* ns_assert( 1 <= ns_voxel_buffer_height( &sampler->src_xz ) );
		last_z = ns_voxel_buffer_height( &sampler->src_xz ) - 1; */

   for( dz = 0, sz = c1z; sz <= c2z; ++dz, ++sz )
      for( dx = 0, sx = c1x; sx <= c2x; ++dx, ++sx )
         {
         intensity = ns_voxel_get( &sampler->src_xz, sx, /*last_z - */sz, 0 );
         ns_voxel_set( &dest, 0, 0, 0, intensity );
         channel = ( nsuint8 )ns_voxel_get( &dest, 0, 0, 0 );

         C.x = C.y = C.z = 0;

         if( threshold <= intensity )
            NS_COLOR_SET_GREEN_U8( C, channel );
         else
            NS_COLOR_SET_RED_U8( C, channel );

         ns_pixel_init_rgb_u8_u8_u8( &pixel, C.x, C.y, C.z );
         ns_image_set_pixel_ex( &sample->image_xz, dx, dz, 0, &pixel );
         }

   ns_image_destruct( &lum_u8 );
   return ns_no_error();
   }


NS_PRIVATE NsError _ns_sampler_make_images( NsSampler *sampler, NsSample *sample )
   {  return _ns_sampler_make_images_rgb_u8_u8_u8( sampler, sample );  }


NsError ns_sample_make_images( NsSampler *sampler, nspointer handle )
   {
   NsError error;

   ns_assert( NULL != sampler );

   error = ns_no_error();

   if( NULL != handle )
      {
      ns_sampler_lock( sampler );
      error = _ns_sampler_make_images( sampler, _ns_sample( handle ) );
      ns_sampler_unlock( sampler );
      }

   return error;
   }


enum{ __UP, __DOWN, __NONE };


NS_PRIVATE void _ns_sample_do_translate( NsSample *sample, nsint how )
   {
   if( 0 < sample->num_steps )
      {
      switch( how )
         {
         case __UP:
            if( sample->curr_step < sample->num_steps )
               ++(sample->curr_step);
            break;

         case __DOWN:
            if( 0 < sample->curr_step )
               --(sample->curr_step);
            break;

         case __NONE:
            break;

         default:
            ns_assert_not_reached();
         }

      ns_vector3f_scale(
         &sample->O,
         &sample->D,
         ( sample->length / ( nsfloat )sample->num_steps ) * ( nsfloat )sample->curr_step
         );

      ns_vector3f_cmpd_add( &sample->O, &sample->P1 );
      }
   }



#define _NS_SAMPLE_ANGLE_DELTA  5.0f


NS_PRIVATE void _ns_sample_do_rotate( NsSample *sample, nsboolean right )
   {
   if( right )
      sample->angle -= _NS_SAMPLE_ANGLE_DELTA;
   else
      sample->angle += _NS_SAMPLE_ANGLE_DELTA;      

   if( 360.0f <= sample->angle )
      sample->angle -= 360.0f;
   else if( sample->angle < 0.0f )
      sample->angle += 360.0f;

   _ns_sample_do_translate( sample, __NONE );
   }


void ns_sample_rotate_right( nspointer handle )
   {
   if( NULL != handle )
      _ns_sample_do_rotate( _ns_sample( handle ), NS_TRUE );
   }


void ns_sample_rotate_left( nspointer handle )
   {
   if( NULL != handle )
      _ns_sample_do_rotate( _ns_sample( handle ), NS_FALSE );
   }


void ns_sample_translate_up( nspointer handle )
   {
   if( NULL != handle )
      _ns_sample_do_translate( _ns_sample( handle ), __UP );
   }


void ns_sample_translate_down( nspointer handle )
   {
   if( NULL != handle )
      _ns_sample_do_translate( _ns_sample( handle ), __DOWN );
   }


void ns_sampler_render_borders
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, border_func ) );

   state->variables.color = NS_COLOR4UB_WHITE;
   state->variables.shape = NS_RENDER_SHAPE_BORDER;

   NS_RENDER_PRE_OR_POST( state, pre_borders_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      NS_RENDER( state, border_func )(
         &sample->C1,
         &sample->C2,
         state
         );
      }

   NS_RENDER_PRE_OR_POST( state, post_borders_func );
   }


void ns_sampler_render_lines
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, line_func ) );

   state->variables.shape = NS_RENDER_SHAPE_LINE;
   NS_RENDER_PRE_OR_POST( state, pre_lines_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      state->variables.color = NS_COLOR4UB( 0x00, 0x60, 0xFF, 255 );
      NS_RENDER( state, line_func )(
         &sample->R1,
         &sample->R2,
         state
         );

      state->variables.color = NS_COLOR4UB( 0xFF, 0x7f, 0x00, 255 );
      NS_RENDER( state, line_func )(
         &sample->P1,
         &sample->P2,
         state
         );
      }

   NS_RENDER_PRE_OR_POST( state, post_lines_func );
   }


nsfloat ____ns_sampler_render_triangles_offset = 0.0f;

void ns_sampler_render_triangles
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter                curr_sample;
   const NsSample           *sample;
   const nsfloat            *distances;
   nssize                    num_distances;
   nssize                    num_triangles;
   const NsVector3f         *vectors;
   const NsIndexTriangleus  *triangles;
   const NsVector3f         *normals;
   NsTriangle3f              T;
   nssize                    i;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, triangle_func ) );

   state->variables.color = NS_COLOR4UB( 255, 174, 0, 255 );
   state->variables.shape = NS_RENDER_SHAPE_TRIANGLE;

   NS_RENDER_PRE_OR_POST( state, pre_triangles_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      ns_sample_get_distances( curr_sample, &distances, &num_distances );

      normals = 0 < ns_byte_array_size( &sample->normals ) ?
                ( const NsVector3f* )ns_byte_array_begin( &sample->normals ) : NULL;

      if( NULL != distances && 0 < num_distances )
         {
         vectors = ns_rayburst_kernel_type_vectors( sample->kernel_type );
         ns_assert( NULL != vectors );

         triangles     = ns_rayburst_kernel_type_triangles( sample->kernel_type );
         num_triangles = ns_rayburst_kernel_type_num_triangles( sample->kernel_type );

         if( NULL != triangles )
            for( i = 0; i < num_triangles; ++i )
               {
               T.A = vectors[ triangles[i].a ];
               T.B = vectors[ triangles[i].b ];
               T.C = vectors[ triangles[i].c ];

               ns_vector3f_cmpd_scale( &T.A, distances[ triangles[i].a ] + ____ns_sampler_render_triangles_offset );
               ns_vector3f_cmpd_scale( &T.B, distances[ triangles[i].b ] + ____ns_sampler_render_triangles_offset );
               ns_vector3f_cmpd_scale( &T.C, distances[ triangles[i].c ] + ____ns_sampler_render_triangles_offset );

               ns_vector3f_cmpd_add( &T.A, &sample->C );
               ns_vector3f_cmpd_add( &T.B, &sample->C );
               ns_vector3f_cmpd_add( &T.C, &sample->C );

               NS_RENDER( state, triangle_func )(
                  NULL != normals ? normals + triangles[i].a : vectors + triangles[i].a,
                  &T.A,
                  NULL != normals ? normals + triangles[i].b : vectors + triangles[i].b,
                  &T.B,
                  NULL != normals ? normals + triangles[i].c : vectors + triangles[i].c,
                  &T.C,
                  state
                  );
               }
         }
      }

   NS_RENDER_PRE_OR_POST( state, post_triangles_func );
   }


void ns_sampler_render_spheres
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, sphere_func ) );

   state->variables.color = NS_COLOR4UB_BLUE;
   state->variables.shape = NS_RENDER_SHAPE_SPHERE;

   NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      NS_RENDER( state, sphere_func )(
         &sample->C,
         sample->radius,
         state
         );
      }

   NS_RENDER_PRE_OR_POST( state, post_spheres_func );
   }


void ns_sampler_render_planes
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter         curr_sample;
   const NsSample    *sample;
   const nsfloat     *distances;
   nssize             num_distances;
   const NsVector3f  *vectors;
   //NsVector3f         V;
   NsVector3f         V1;
   NsVector3f         V2;
   NsVector3f         max;
   NsVector3f         min;
   nssize             i;
   nssize             j;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, plane_func ) );

   state->variables.shape = NS_RENDER_SHAPE_PLANE;
   NS_RENDER_PRE_OR_POST( state, pre_planes_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      if( 2 == ns_rayburst_kernel_type_dimension( sample->kernel_type ) )
         {
         min.x = min.y = min.z = NS_FLOAT_MAX;
         max.x = max.y = max.z = -NS_FLOAT_MAX;

         ns_sample_get_distances( curr_sample, &distances, &num_distances );

         if( NULL != distances && 0 < num_distances )
            {
            vectors = ns_rayburst_kernel_type_vectors( sample->kernel_type );
            ns_assert( NULL != vectors );

            for( i = 0; i < num_distances; ++i )
               {
               ns_vector3f_scale( &V1, vectors + i, distances[i] );
               ns_vector3f_cmpd_add( &V1, &sample->C );

               j = ( i + 1 ) % num_distances;

               ns_vector3f_scale( &V2, vectors + j, distances[j] );
               ns_vector3f_cmpd_add( &V2, &sample->C );
               
               /*NS_RENDER( state, triangle_func )(
                  &sample->C, &V1, &V2,
                  state->plane_color,
                  state
                  );*/

               /*if( V.x < min.x )min.x = V.x;
               if( V.y < min.y )min.y = V.y;
               if( V.z < min.z )min.z = V.z;
               if( max.x < V.x )max.x = V.x;
               if( max.y < V.y )max.y = V.y;
               if( max.z < V.z )max.z = V.z;*/
               }
            }

         /*
         NS_RENDER( state, plane_func )(
            &min,
            &max,
            state->plane_color,
            state
            );
         */
         }
      }

   NS_RENDER_PRE_OR_POST( state, post_planes_func );
   }


void ns_sampler_render_images
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, image_func ) );

   state->variables.shape = NS_RENDER_SHAPE_IMAGE;
   NS_RENDER_PRE_OR_POST( state, pre_images_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      NS_RENDER( state, image_func )(
         &sample->C1,
         &sample->image_xy,
         &sample->image_zy,
         &sample->image_xz,
         state
         );
      }

   NS_RENDER_PRE_OR_POST( state, post_images_func );
   }


void ns_sampler_render_ellipses
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;
   NsVector3f       R;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, ellipse_func ) );

   state->variables.color = NS_COLOR4UB_BLUE;
   state->variables.shape = NS_RENDER_SHAPE_ELLIPSE;

   NS_RENDER_PRE_OR_POST( state, pre_ellipses_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      NS_RENDER( state, ellipse_func )(
         &sample->C,
         ns_vector3f( &R, sample->radius, sample->radius, sample->radius ),
         state
         );
      }

   NS_RENDER_PRE_OR_POST( state, post_ellipses_func );
   }


void ns_sampler_render_text
   (
   const NsSampler  *sampler,
   NsRenderState    *state
   )
   {
   nslistiter       curr_sample;
   const NsSample  *sample;
   NsVector3i       Pi;
   NsVector3f       Pf;
   nschar           text[ NS_FLOATING_TYPE_MAX_DIGITS + 32 ];


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, text_func ) );

   state->variables.color = NS_COLOR4UB_WHITE;
   state->variables.shape = NS_RENDER_SHAPE_TEXT;

   NS_RENDER_PRE_OR_POST( state, pre_texts_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

      Pi = sample->C1;

      //++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), /*"id = "*/ NS_FMT_ULONG, sample->id );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );

      NS_RENDER( state, text_func )( &Pf, text, state );

/*
      ++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), "radius = %.2f", sample->radius );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
      NS_RENDER( state, text_func )( &Pf, text, state );

      if( 0 < sample->num_steps )
         {
         ++Pi.y;
         ns_snprint( text, NS_ARRAY_LENGTH( text ), "length = %.2f", sample->length );
         ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
         NS_RENDER( state, text_func )( &Pf, text, state );

         ++Pi.y;
         ns_snprint( text, NS_ARRAY_LENGTH( text ), "angle = " NS_FMT_ULONG, ( nsulong )sample->angle );
         ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
         NS_RENDER( state, text_func )( &Pf, text, state );

         ++Pi.y;
         ns_snprint( text, NS_ARRAY_LENGTH( text ), "num_steps = " NS_FMT_ULONG, sample->num_steps );
         ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
         NS_RENDER( state, text_func )( &Pf, text, state );

         ++Pi.y;
         ns_snprint( text, NS_ARRAY_LENGTH( text ), "curr_step = " NS_FMT_ULONG, sample->curr_step );
         ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
         NS_RENDER( state, text_func )( &Pf, text, state );
         }

      ++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), "threshold = " NS_FMT_ULONG, sample->threshold );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
      NS_RENDER( state, text_func )( &Pf, text, state );

      ++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), "sample_width = " NS_FMT_ULONG, sample->image_width );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
      NS_RENDER( state, text_func )( &Pf, text, state );

      ++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), "sample_height = " NS_FMT_ULONG, sample->image_height );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
      NS_RENDER( state, text_func )( &Pf, text, state );

      ++Pi.y;
      ns_snprint( text, NS_ARRAY_LENGTH( text ), "sample_length = " NS_FMT_ULONG, sample->image_length );
      ns_to_voxel_space( &Pi, &Pf, sampler->voxel_info );
      NS_RENDER( state, text_func )( &Pf, text, state );

      NS_RENDER( state, text_func )( &sample->O,  "O", state );
      NS_RENDER( state, text_func )( &sample->C,  "C", state );
      NS_RENDER( state, text_func )( &sample->R1, "R1", state );
      NS_RENDER( state, text_func )( &sample->R2, "R2", state );
      NS_RENDER( state, text_func )( &sample->P1, "P1", state );
      NS_RENDER( state, text_func )( &sample->P2, "P2", state );
*/
      }

   NS_RENDER_PRE_OR_POST( state, post_texts_func );
   }


void ns_sampler_render_selected
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	)
	{
	nslistiter       curr_sample;
   const NsSample  *sample;
   NsAABBox3d       B;
	NsVector3f       R;


   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_AABBOX;

	NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

   NS_LIST_FOREACH( &sampler->samples, curr_sample )
      {
      sample = ns_list_iter_get_object( curr_sample );

		if( sample->is_selected )
			NS_RENDER( state, aabbox_func )(
				ns_model_position_and_radii_to_aabbox(
					&sample->C,
					_ns_sample_radii( sample, &R ),
					&B
					),
				state
				);
		}

	NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
	}


void ns_sampler_render_last
	(
	const NsSampler  *sampler,
	NsRenderState    *state
	)
	{
   const NsSample *sample;

   ns_assert( NULL != sampler );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, sphere_func ) );

	/* NOTE: The last sample is drawn in different color. */
	if( ! ns_list_is_empty( &sampler->samples ) )
		{
		state->variables.color = NS_COLOR4UB_BLUE;
		state->variables.shape = NS_RENDER_SHAPE_SPHERE;

		NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

		sample = ns_list_iter_get_object( ns_list_rev_begin( &sampler->samples ) );
      NS_RENDER( state, sphere_func )( &sample->C, sample->radius, state );

		NS_RENDER_PRE_OR_POST( state, post_spheres_func );
		}
	}




#define _NS_SAMPLER_INT_WIDTH         ( ( nsint )6 )
#define _NS_SAMPLER_FLOAT_WIDTH       ( ( nsint )10 )
#define _NS_SAMPLER_FLOAT_PRECISION   ( ( nsint )6 )

NsError ns_sampler_output( const NsSampler *sampler, const nschar *name )
   {
   NsFile           file;
   nslistiter       curr_sample;
   const NsSample  *sample;
   NsError          error;


   ns_assert( NULL != sampler );
   ns_assert( NULL != name );

   error = ns_no_error();

   ns_file_construct( &file );

   if( NS_FAILURE( ns_file_open( &file, name, NS_FILE_MODE_WRITE ), error ) )
      return error;

   error = ns_file_print(
            &file,
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            /*NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
            NS_STRING_TAB
            NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )*/
            NS_STRING_NEWLINE,
            _NS_SAMPLER_INT_WIDTH,
            "ID",
            _NS_SAMPLER_FLOAT_WIDTH,
            "WIDTH",
            _NS_SAMPLER_FLOAT_WIDTH,
            "X",
            _NS_SAMPLER_FLOAT_WIDTH,
            "Y",
            _NS_SAMPLER_FLOAT_WIDTH,
            "Z",
            _NS_SAMPLER_FLOAT_WIDTH,
            "LENGTH"/*,
            _NS_SAMPLER_FLOAT_WIDTH,
            "Volume",
            _NS_SAMPLER_FLOAT_WIDTH,
            "S.A."*/
            );

   if( ! ns_is_error( error ) )
      NS_LIST_FOREACH( &sampler->samples, curr_sample )
         {
         sample = ns_list_iter_get_object( curr_sample );

			if( NS_FAILURE(
					ns_file_print(
						&file,
						NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_LONG )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						/*NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
						NS_STRING_TAB
						NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )*/
						NS_STRING_NEWLINE,
						_NS_SAMPLER_INT_WIDTH,
						sample->id,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->radius * 2.0f,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->C.x,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->C.y,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->C.z,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->length/*,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->volume,
						_NS_SAMPLER_FLOAT_WIDTH, _NS_SAMPLER_FLOAT_PRECISION,
						sample->surface_area*/
						),
					error ) )
				break;
         }

   ns_file_close( &file );
   ns_file_destruct( &file );

   return error;
   }


NsError ns_sampler_to_model( const NsSampler *sampler, NsModel *model )
   {
   nslistiter       iter;
   const NsSample  *sample;
   nsmodelvertex    prev_vertex, curr_vertex;
   NsError          error;


   /* Convert all the samples to vertices. Note that vertices are
      just created in a linear fashion, i.e. edges are created only
      between the current and previous vertices. */

   ns_assert( NULL != sampler );
   ns_assert( NULL != model );

   ns_model_clear( model );

	error = ns_no_error();

	ns_sampler_lock( sampler );
	ns_model_lock( model );

   prev_vertex = NS_MODEL_VERTEX_NIL;

   NS_LIST_FOREACH( &sampler->samples, iter )
      {
      sample = ns_list_iter_get_object( iter );

		if( NS_FAILURE( ns_model_add_vertex( model, &curr_vertex ), error ) )
			goto _NS_SAMPLER_TO_MODEL_EXIT;

		/* NOTE: Center the vertex. */
		ns_model_vertex_set_position( curr_vertex, &sample->C );
		ns_model_vertex_set_radius( curr_vertex, sample->radius );
		ns_model_vertex_mark_manually_traced( curr_vertex, NS_TRUE );

		ns_model_vertex_set_threshold(
			curr_vertex,
			NS_MODEL_VERTEX_NEURITE_THRESHOLD,
			sample->exact_threshold
			);

		ns_model_vertex_set_contrast(
			curr_vertex,
			NS_MODEL_VERTEX_NEURITE_CONTRAST,
			sample->contrast
			);

		if( ns_model_vertex_not_equal( prev_vertex, NS_MODEL_VERTEX_NIL ) )
			if( NS_FAILURE( ns_model_vertex_add_edge( curr_vertex, prev_vertex, NULL, NULL ), error ) )
				goto _NS_SAMPLER_TO_MODEL_EXIT;

		prev_vertex = curr_vertex;
      }

	_NS_SAMPLER_TO_MODEL_EXIT:

	ns_sampler_unlock( sampler );
	ns_model_unlock( model );

	return ( ! ns_is_error( error ) ) ? ns_model_create_origins( model, NULL ) : error;
   }


/* NOTE: Avoid circular inclusion! */
#include <model/nsmodel-graft.h>
#include "nssampler-run.inl"
