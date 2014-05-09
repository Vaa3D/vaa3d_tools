
#define _NS_SAMPLE_TINY_RADIUS  0.0f


NS_PRIVATE NsError _ns_do_sample_jitter
   (
	const NsVoxelInfo  *voxel_info,
	const NsVector3i   *jitter,
	NsRayburst         *radius_rayburst,
	NsVoxelBuffer      *voxel_buffer,
	NsByteArray        *buffer,
	nsdouble            average_intensity,
	nssize              jitter_points,
	nsfloat            *radius,
	NsVector3f         *F,
	NsVector3f         *R,
	NsVector3f         *C,
	const NsVector3i   *O
   )
   {
   nssize       i;
   nssize       bytes;
   NsPoint3i   *points, *point;
   NsVector3f   P;
   NsError      error;
   NsVector3i   J;


	NS_USE_VARIABLE( average_intensity );

   bytes = jitter_points * sizeof( NsPoint3i );

   if( ns_byte_array_size( buffer ) < bytes )
      if( NS_FAILURE( ns_byte_array_resize( buffer, bytes ), error ) )
         return error;

   points = ( NsPoint3i* )ns_byte_array_begin( buffer );

   i = 0;

   for( J.z = -jitter->z; J.z <= jitter->z; ++J.z )
      for( J.y = -jitter->y; J.y <= jitter->y; ++J.y )
         for( J.x = -jitter->x; J.x <= jitter->x; ++J.x )
            {
            ns_assert( i < jitter_points );
            ns_vector3i_add( points + i, O, &J );
            ++i;
            }

   *radius = 0.0f;

   for( i = 0; i < jitter_points; ++i )
      {
      point = points + i;

      if( 0 <= point->x && point->x < ns_voxel_buffer_width( voxel_buffer )  &&
          0 <= point->y && point->y < ns_voxel_buffer_height( voxel_buffer ) &&
          0 <= point->z && point->z < ns_voxel_buffer_length( voxel_buffer )   )
         {
         ns_to_voxel_space( point, &P, voxel_info );
         ns_rayburst( radius_rayburst, &P );

         if( *radius < ns_rayburst_radius( radius_rayburst ) )
            ns_rayburst_mlbd( radius_rayburst, C, F, R, radius );
         }
      }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_do_sample
   (
	const NsVoxelInfo  *voxel_info,
	nsdouble            average_intensity,
	const NsVector3i   *jitter,
	NsRayburst         *radius_rayburst,
	NsRayburst         *shape_rayburst,
	NsVoxelBuffer      *voxel_buffer,
	const NsVector3i   *max,
	NsByteArray        *buffer,
	nssize              jitter_points,
	nsfloat            *radius,
	nsfloat            *threshold,
	nsfloat            *contrast,
	const NsVector3f   *origin,
	nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar,
	nsint               min_window,
	nsboolean           use_fixed_threshold,
	nsfloat             threshold_value,
	NsVector3f         *F,
	NsVector3f         *R,
	NsVector3f         *C,
	NsVector3i         *C1,
	NsVector3i         *C2
   )
   {
   nsfloat           intensity;
   nssize            width, height, length;
   nssize            num_values;
   nssize            bytes;
   nsfloat          *values;
   ThresholdStruct   s;
   NsVector3i        O;
   NsError           error;


   ns_to_image_space( origin, &O, voxel_info );

   ns_assert( 0 <= O.x && O.x < ns_voxel_buffer_width( voxel_buffer ) );
   ns_assert( 0 <= O.y && O.y < ns_voxel_buffer_height( voxel_buffer ) );
   ns_assert( 0 <= O.z && O.z < ns_voxel_buffer_length( voxel_buffer ) );

   intensity = ( nsfloat )ns_voxel_get( voxel_buffer, O.x, O.y, O.z );

   if( ( nsdouble )intensity < average_intensity )
      {
      *radius = _NS_SAMPLE_TINY_RADIUS;
      return ns_no_error();
      }

   /* The initial threshold is the average of the seed value
      intensity and the average intensity of the whole image. */
   if( use_fixed_threshold )
		{
      *threshold = threshold_value;
		*contrast  = threshold_value;
		}
   else
      *threshold = ( nsfloat )( ( ( nsdouble )intensity + average_intensity ) / 2.0 );

   ns_rayburst_set_threshold( radius_rayburst, *threshold );
   ns_rayburst( radius_rayburst, origin );

   *radius = ns_rayburst_radius( radius_rayburst );

   ns_grafting_calc_aabbox( voxel_info, *radius, origin, max, use_2d_sampling, aabbox_scalar, min_window, C1, C2 );

   if( use_fixed_threshold )
		{
      *threshold = threshold_value;
		*contrast  = threshold_value;
		}
   else
      {
      width  = ( nssize )( C2->x - C1->x ) + 1;
      height = ( nssize )( C2->y - C1->y ) + 1;
      length = ( nssize )( C2->z - C1->z ) + 1;

      num_values = width * height * length;
      ns_assert( 0 < num_values );
      bytes = num_values * sizeof( nsfloat );

      if( ns_byte_array_size( buffer ) < bytes )
         if( NS_FAILURE( ns_byte_array_resize( buffer, bytes ), error ) )
            return error;

      values = ( nsfloat* )ns_byte_array_begin( buffer );

      ns_grafting_sample_values( voxel_buffer, values, num_values, C1, C2 );

      //ns_grafting_output_sample_values( values, num_values, "C:\\usr\\doug\\projects\\ns\\values.txt" );
      //_ns_sample_do_histogram( ns_voxel_buffer_pixel_type( voxel_buffer ), values, num_values );

      ComputeThreshold( values, ( nsint )num_values, &s, ns_voxel_buffer_dynamic_range( voxel_buffer ), NS_TRUE );

      *threshold = s.threshold;
		*contrast  = s.contrast;
      }

   ns_rayburst_set_threshold( radius_rayburst, *threshold );

   if( NULL != shape_rayburst )
      ns_rayburst_set_threshold( shape_rayburst, *threshold );

   return _ns_do_sample_jitter(
               voxel_info,
               jitter,
               radius_rayburst,
               voxel_buffer,
               buffer,
               average_intensity,
               jitter_points,
               radius,
               F,
               R,
               C,
               &O
               );
   }


NS_PRIVATE nsboolean _ns_sample_get_line
   (
	const NsPoint3f    *P1_in,
	const NsPoint3f    *P2_in,
	const NsVoxelInfo  *voxel_info,
	NsVoxelBuffer      *voxel_buffer,
	nsdouble            average_intensity,
	NsPoint3f          *P1_out,
	NsPoint3f          *P2_out
   )
   {
   nsdouble    intensity1, intensity2;
   NsVector3i  P1i, P2i;


   *P1_out = *P1_in;
   *P2_out = *P2_in;

   ns_to_image_space( P1_in, &P1i, voxel_info );
   ns_to_image_space( P2_in, &P2i, voxel_info );

   intensity1 = ( nsdouble )ns_voxel_get( voxel_buffer, P1i.x, P1i.y, P1i.z );
   intensity2 = ( nsdouble )ns_voxel_get( voxel_buffer, P2i.x, P2i.y, P2i.z );

   /*
   if( intensity1 < average_intensity )
      {
      if( intensity2 < average_intensity )
         return NS_FALSE;

      ns_println( "P1 invalid. Adjusting z." );
      P1_out->z = P2_out->z;
      }
   else if( intensity2 < average_intensity )
      {
      ns_println( "P2 invalid. Adjusting z." );
      P2_out->z = P1_out->z;
      }

   return NS_TRUE;
   */

   return intensity1 >= average_intensity && intensity2 >= average_intensity;
   }


NS_PRIVATE void _ns_sample_on_invalid
   (
   nsfloat    *ret_radius,
   nsfloat    *ret_length,
   nsboolean  *did_create_sample
   )
   {
   if( NULL != ret_radius )
      *ret_radius = 0.0f;

   if( NULL != ret_length )
      *ret_length = 0.0f;

   if( NULL != did_create_sample )
      *did_create_sample = NS_FALSE;

   ns_println( "Invalid sample." );
   }


NsError ns_sample_run
   (
   nspointer           sample,
   const NsImage      *image,
   const NsVoxelInfo  *voxel_info,
   nsdouble            average_intensity,
   const NsVector3i   *jitter,
   nsboolean           use_fixed_threshold,
   nsfloat             threshold_value,
   nsfloat            *ret_radius
   )
   {
   NsRayburst        radius_rayburst;
   NsVoxelBuffer     voxel_buffer;
   nssize            jitter_width, jitter_height, jitter_length;
   NsByteArray       buffer;
   nssize            jitter_points;
   nsfloat           radius, largest_radius;
   NsVector2f        V;
   NsVector3f        origin;
   NsVector3f        F, R, C;
   NsVector3f        P1, P2;
   nsfloat           intensity;
   nsfloat           angle;
   NsError           error;
   NsVector3i        O;


	NS_USE_VARIABLE( use_fixed_threshold );
	NS_USE_VARIABLE( threshold_value );

   *ret_radius = 0.0f;

   ns_voxel_buffer_init( &voxel_buffer, image );

   if( NS_FAILURE( ns_rayburst_construct(
                     &radius_rayburst,
                     NS_RAYBURST_KERNEL_SINGLE,
                     NS_RAYBURST_RADIUS_MLBD,
                     NS_RAYBURST_INTERP_BILINEAR,
                     image,
                     voxel_info,
                     NULL
                     ),
                     error ) )
      return error;

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer, 0 ), error ) );

   ns_assert( 0 <= jitter->x );
   ns_assert( 0 <= jitter->y );
   ns_assert( 0 <= jitter->z );

   jitter_width  = ( nssize )jitter->x * 2 + 1;
   jitter_height = ( nssize )jitter->y * 2 + 1;
   jitter_length = ( nssize )jitter->z * 2 + 1;

   jitter_points = jitter_width * jitter_height * jitter_length;
   ns_assert( 0 < jitter_points );

   error          = ns_no_error();
   largest_radius = _NS_SAMPLE_TINY_RADIUS;

   ns_sample_get_origin( sample, &origin );
   //ns_println( "sample origin at %.2f,%.2f,%.2f", origin.x, origin.y, origin.z );

   ns_to_image_space( &origin, &O, voxel_info );

   ns_assert( 0 <= O.x && O.x < ns_voxel_buffer_width( &voxel_buffer ) );
   ns_assert( 0 <= O.y && O.y < ns_voxel_buffer_height( &voxel_buffer ) );
   ns_assert( 0 <= O.z && O.z < ns_voxel_buffer_length( &voxel_buffer ) );

   intensity = ( nsfloat )ns_voxel_get( &voxel_buffer, O.x, O.y, O.z );

   if( ( nsdouble )intensity < average_intensity )
      {
      *ret_radius = _NS_SAMPLE_TINY_RADIUS;
      goto _NS_SAMPLE_EXIT;
      }

   angle = ns_sample_get_angle( sample );
   ns_sample_get_endpoints( sample, &P1, &P2 );

   get_z_rotated_vector_with_respect_to_line( P1.x, P1.y, P2.x, P2.y, angle, &V );
   ns_rayburst_init_single( &radius_rayburst, V.x, V.y );

   ns_rayburst_set_threshold( &radius_rayburst, ns_sample_get_threshold( sample ) );

   F = R = C = origin;
   radius = 0.0f;

   if( NS_FAILURE( _ns_do_sample_jitter(
                     voxel_info,
                     jitter,
                     &radius_rayburst,
                     &voxel_buffer,
                     &buffer,
                     average_intensity,
                     jitter_points,
                     &radius,
                     &F,
                     &R,
                     &C,
                     &O
                     ),
                     error ) )
      goto _NS_SAMPLE_EXIT;

   if( largest_radius < radius )
      {
      largest_radius = radius;

      ns_sample_set_radius( sample, radius );
      ns_sample_set_center( sample, &C );
      ns_sample_set_rays( sample, &F, &R );

      *ret_radius = radius;
      }

   if( NS_FLOAT_EQUAL( largest_radius, _NS_SAMPLE_TINY_RADIUS ) )
      _ns_sample_on_invalid( ret_radius, NULL, NULL );

   _NS_SAMPLE_EXIT:

   ns_byte_array_destruct( &buffer );
   ns_rayburst_destruct( &radius_rayburst );

   return error;
   }


NS_PRIVATE NsError _ns_sampler_run_with_length
   (
   NsSampler          *sampler,
   const NsImage      *image,
   const NsPoint3f    *V1,
   const NsPoint3f    *V2,
   nssize              num_steps,
   const NsVoxelInfo  *voxel_info,
   nsdouble            average_intensity,
   const NsVector3i   *jitter,
   nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar,
	nsint               min_window,
   nsboolean           use_fixed_threshold,
   nsfloat             threshold_value,
   nsfloat            *ret_radius,
   nsfloat            *ret_length,
   nsboolean          *did_create_sample
   )
   {
   NsRayburst        radius_rayburst;
   NsVoxelBuffer     voxel_buffer;
   nssize            jitter_width, jitter_height, jitter_length;
   NsByteArray       buffer;
   nssize            jitter_points;
   nspointer         sample;
   nsfloat           radius, largest_radius;
   nsfloat           threshold;
	nsfloat           contrast;
   nsfloat           length;
   NsVector2f        V;
   NsVector3f        origin;
   NsVector3f        P1, P2;
   NsVector3f        D, F, R, C;
   NsError           error;
   NsVector3i        C1, C2;
   NsVector3i        max;


   *ret_radius = 0.0f;
   *ret_length = 0.0f;

   ns_voxel_buffer_init( &voxel_buffer, image );

   if( ! _ns_sample_get_line(
            V1,
            V2,
            voxel_info,
            &voxel_buffer,
            average_intensity,
            &P1,
            &P2
            ) )
      {
      _ns_sample_on_invalid( ret_radius, ret_length, did_create_sample );
      return ns_no_error();
      }

   if( NS_FAILURE( ns_rayburst_construct(
                     &radius_rayburst,
                     NS_RAYBURST_KERNEL_SINGLE,
                     NS_RAYBURST_RADIUS_MLBD,
                     NS_RAYBURST_INTERP_BILINEAR,
                     image,
                     voxel_info,
                     NULL
                     ),
                     error ) )
      return error;

   ns_vector3i(
      &max,
      ns_voxel_buffer_width( &voxel_buffer ) - 1,
      ns_voxel_buffer_height( &voxel_buffer ) - 1,
      ns_voxel_buffer_length( &voxel_buffer ) - 1
      );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer, 0 ), error ) );

   ns_assert( 0 <= jitter->x );
   ns_assert( 0 <= jitter->y );
   ns_assert( 0 <= jitter->z );

   jitter_width  = ( nssize )jitter->x * 2 + 1;
   jitter_height = ( nssize )jitter->y * 2 + 1;
   jitter_length = ( nssize )jitter->z * 2 + 1;

   jitter_points = jitter_width * jitter_height * jitter_length;
   ns_assert( 0 < jitter_points );

   error          = ns_no_error();
   largest_radius = _NS_SAMPLE_TINY_RADIUS;
   sample         = NULL;

   if( NS_FAILURE( ns_sampler_add( sampler, &sample ), error ) )
      goto _NS_SAMPLE_EXIT;

   ns_sample_set_num_steps( sample, num_steps );
   ns_sample_set_curr_step( sample, num_steps / 2 );

   ns_sample_set_endpoints( sample, &P1, &P2 );
   ns_sample_get_unit_direction( sample, &D );

   *ret_length = length = ns_sample_get_length( sample );

   /* Do the sample at the "mid-point". */
   ns_vector3f_scale(
      &origin,
      &D,
      ( length / ( nsfloat )num_steps ) * ( nsfloat )ns_sample_get_curr_step( sample )
      );

   ns_vector3f_cmpd_add( &origin, &P1 );

   //ns_println( "sample origin at %.2f,%.2f,%.2f", origin.x, origin.y, origin.z );

   /* Initially sample is perpendicular. */
   ns_sample_set_angle( sample, 90.0f );

   /* TEMP?????? */
   if( NS_FLOAT_EQUAL( P1.x, P2.x ) && NS_FLOAT_EQUAL( P1.y, P2.y ) )
      {
      _ns_sample_on_invalid( ret_radius, ret_length, did_create_sample );
      ns_sampler_remove( sampler, sample );

      ns_println( "Zero vector in X,Y. Can not rotate vector around z. Aborting" );

      return ns_no_error();
      }

   get_z_rotated_vector_with_respect_to_line( P1.x, P1.y, P2.x, P2.y, 90.0f, &V );

   //ns_println( "P1=(%f,%f) P2=(%f,%f) V=(%f,%f)", P1.x, P1.y, P2.x, P2.y, V.x, V.y );
   ns_rayburst_init_single( &radius_rayburst, V.x, V.y );

   F = R = C = origin;
   radius = 0.0f;

   if( NS_FAILURE( _ns_do_sample(
                     voxel_info,
                     average_intensity,
                     jitter,
                     &radius_rayburst,
                     NULL,
                     &voxel_buffer,
                     &max,
                     &buffer,
                     jitter_points,
                     &radius,
                     &threshold,
							&contrast,
                     &origin,
                     use_2d_sampling,
							aabbox_scalar,
							min_window,
                     use_fixed_threshold,
                     threshold_value,
                     &F,
                     &R,
                     &C,
                     &C1,
                     &C2
                     ),
                     error ) )
      goto _NS_SAMPLE_EXIT;

   if( largest_radius < radius )
      {
      largest_radius = radius;

      ns_sample_set_radius( sample, radius );
      ns_sample_set_center( sample, &C );
      ns_sample_set_rays( sample, &F, &R );
      ns_sample_set_threshold( sample, threshold );
		ns_sample_set_contrast( sample, contrast );
      ns_sample_set_corners( sample, &C1, &C2 );
      ns_sample_set_origin( sample, &origin );

      *ret_radius = radius;
      }

   if( NS_FLOAT_EQUAL( largest_radius, _NS_SAMPLE_TINY_RADIUS ) )
      {
      _ns_sample_on_invalid( ret_radius, ret_length, did_create_sample );
      ns_sampler_remove( sampler, sample );
      }
   /*else
      error = ns_sample_make_images( sampler, sample );*/

   _NS_SAMPLE_EXIT:

   ns_byte_array_destruct( &buffer );
   ns_rayburst_destruct( &radius_rayburst );

   return error;
   }


NsError ns_sampler_run
   (
   NsSampler             *sampler,
   const NsImage         *image,
   const NsPoint3f       *V1,
   const NsPoint3f       *V2,
   nssize                 num_steps,
   nsboolean              do_3d_radii,
   NsRayburstKernelType   kernel_type,
   NsRayburstInterpType   interp_type,
   const NsVoxelInfo     *voxel_info,
   nsdouble               average_intensity,
   const NsVector3i      *jitter,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   nsboolean              use_fixed_threshold,
   nsfloat                threshold_value,
   nsfloat               *ret_radius,
   nsfloat               *ret_length,
   nsboolean             *did_create_sample
   )
   {
   NsRayburst      radius_rayburst, shape_rayburst, *p_shape_rayburst;
   NsVoxelBuffer   voxel_buffer;
   nssize          jitter_width, jitter_height, jitter_length;
   NsByteArray     buffer1, buffer2;
   nssize          jitter_points;
   nspointer       sample;
   nsfloat         radius, largest_radius;
   nsfloat         threshold;
	nsfloat         contrast;
   nsfloat         length;
   NsVector3f      origin;
   NsVector3f      P1, P2;
   NsVector3f      D, F, R, C;
   nssize          i;
   NsError         error;
   NsVector3i      C1, C2;
   NsVector3i      max;
   nsfloat        *distances;
   nssize          num_distances;


   ns_assert( NULL != image );
   ns_assert( NULL != V1 );
   ns_assert( NULL != V2 );
   ns_assert( NULL != voxel_info );
   ns_assert( NULL != jitter );
   ns_assert( NULL != ret_radius );
   ns_assert( NULL != ret_length );
   ns_assert( NULL != did_create_sample );

   *did_create_sample = NS_TRUE;

   /*TEMP???*/
   if( 0 < num_steps )
      return _ns_sampler_run_with_length(
               sampler,
               image,
               V1,
               V2,
               num_steps,
               voxel_info,
               average_intensity,
               jitter,
               use_2d_sampling,
					aabbox_scalar,
					min_window,
               use_fixed_threshold,
               threshold_value,
               ret_radius,
               ret_length,
               did_create_sample
               );

   *ret_radius = 0.0f;
   *ret_length = 0.0f;

   ns_voxel_buffer_init( &voxel_buffer, image );

   if( ! _ns_sample_get_line(
            V1,
            V2,
            voxel_info,
            &voxel_buffer,
            average_intensity,
            &P1,
            &P2
            ) )
      {
      _ns_sample_on_invalid( ret_radius, ret_length, did_create_sample );
      return ns_no_error();
      }

   /* TEMP Should we allow 3D sampling??? */
   if( NS_FAILURE( ns_rayburst_construct(
                     &radius_rayburst,
                     /*do_3d_radii ? NS_RAYBURST_KERNEL_STD :*/NS_RAYBURST_KERNEL_2D,
                     NS_RAYBURST_RADIUS_MLBD,
                     NS_RAYBURST_INTERP_BILINEAR,
                     image,
                     voxel_info,
                     NULL
                     ),
                     error ) )
      return error;

   /* Only allowing kernel type to be used in 3D. */
#ifdef NS_SAMPLER_RUN_ENABLE_3D
	if( NS_FAILURE( ns_rayburst_construct(
							&shape_rayburst,
							do_3d_radii ? kernel_type : NS_RAYBURST_KERNEL_2D_VIZ,
							NS_RAYBURST_RADIUS_NONE,
							interp_type,
							image,
							voxel_info,
							NULL
							),
							error ) )
		{
		ns_rayburst_destruct( &radius_rayburst );
		return error;
		}
#else
		NS_USE_VARIABLE( shape_rayburst );
		NS_USE_VARIABLE( interp_type );
		NS_USE_VARIABLE( kernel_type );
		NS_USE_VARIABLE( do_3d_radii );
		NS_USE_VARIABLE( distances );
		NS_USE_VARIABLE( num_distances );
		NS_USE_VARIABLE( buffer2 );
#endif/* NS_SAMPLER_RUN_ENABLE_3D */

   ns_vector3i(
      &max,
      ns_voxel_buffer_width( &voxel_buffer ) - 1,
      ns_voxel_buffer_height( &voxel_buffer ) - 1,
      ns_voxel_buffer_length( &voxel_buffer ) - 1
      );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer1, 0 ), error ) );

#ifdef NS_SAMPLER_RUN_ENABLE_3D
   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer2, 0 ), error ) );

   num_distances = ns_rayburst_num_samples( &shape_rayburst );

   if( NS_SUCCESS( ns_byte_array_resize(
                     &buffer2,
                     num_distances * sizeof( nsfloat )
                     ),
                     error ) )
      distances = ( nsfloat* )ns_byte_array_begin( &buffer2 );
   else
      distances = NULL;
#endif/* NS_SAMPLER_RUN_ENABLE_3D */

   ns_assert( 0 <= jitter->x );
   ns_assert( 0 <= jitter->y );
   ns_assert( 0 <= jitter->z );

   jitter_width  = ( nssize )jitter->x * 2 + 1;
   jitter_height = ( nssize )jitter->y * 2 + 1;
   jitter_length = ( nssize )jitter->z * 2 + 1;

   jitter_points = jitter_width * jitter_height * jitter_length;
   ns_assert( 0 < jitter_points );

   error          = ns_no_error();
   largest_radius = _NS_SAMPLE_TINY_RADIUS;
   sample         = NULL;

   if( NS_FAILURE( ns_sampler_add( sampler, &sample ), error ) )
      goto _NS_SAMPLE_EXIT;

   ns_sample_set_endpoints( sample, &P1, &P2 );

#ifdef NS_SAMPLER_RUN_ENABLE_3D
   ns_sample_set_rayburst_kernel_type( sample, ns_rayburst_kernel_type( &shape_rayburst ) );
	p_shape_rayburst = &shape_rayburst;
#else
	p_shape_rayburst = NULL;
#endif/* NS_SAMPLER_RUN_ENABLE_3D */

   *ret_length = length = ns_sample_get_length( sample );

   if( 0 < num_steps )
      {
      ns_vector3f_norm( &D );
      ns_vector3f_cmpd_scale( &D, length / ( nsfloat )num_steps );
      }

   origin = P1;

   /* NOTE: <= so that P2 is also run. If 'num_steps' is zero its OK
      since P1 should be equal to P2. */
   for( i = 0; i <= num_steps; ++i )
      {
      //ns_println( "sample origin at %.2f,%.2f,%.2f", origin.x, origin.y, origin.z );

      F = R = C = origin;
      radius = 0.0f;

		/* NOTE: Cant use #ifdef within NS_FAILURE macro, thats why a simple
			pointer 'p_shape_rayburst' was set above. */

      if( NS_FAILURE( _ns_do_sample(
                        voxel_info,
                        average_intensity,
                        jitter,
                        &radius_rayburst,
                        p_shape_rayburst,
                        &voxel_buffer,
                        &max,
                        &buffer1,
                        jitter_points,
                        &radius,
                        &threshold,
								&contrast,
                        &origin,
                        use_2d_sampling,
								aabbox_scalar,
								min_window,
                        use_fixed_threshold,
                        threshold_value,
                        &F,
                        &R,
                        &C,
                        &C1,
                        &C2
                        ),
                        error ) )
         goto _NS_SAMPLE_EXIT;

      if( largest_radius < radius )
         {
         largest_radius = radius;

         ns_sample_set_threshold( sample, threshold );
			ns_sample_set_contrast( sample, contrast );

         ns_sample_set_radius( sample, radius );
         ns_sample_set_center( sample, &C );
         ns_sample_set_rays( sample, &F, &R );
         ns_sample_set_corners( sample, &C1, &C2 );
         ns_sample_set_origin( sample, &origin );

#ifdef NS_SAMPLER_RUN_ENABLE_3D
			ns_rayburst_set_threshold( &shape_rayburst, threshold );

			//ns_println( "running shape rayburst..." );
         ns_rayburst( &shape_rayburst, &C );

         ns_rayburst_sample_distances( &shape_rayburst, NS_RAYBURST_SAMPLE_DISTANCE_FORWARD, distances );
			//ns_println( "setting distances %p %lu", distances, num_distances );
         ns_sample_set_distances( sample, distances, num_distances );

         ns_sample_set_volume( sample, ns_rayburst_volume( &shape_rayburst ) );
         ns_sample_set_surface_area( sample, ns_rayburst_surface_area( &shape_rayburst ) );
#endif/* NS_SAMPLER_RUN_ENABLE_3D */

         *ret_radius = radius;
         }

      ns_vector3f_cmpd_add( &origin, &D );
      }

   if( NS_FLOAT_EQUAL( largest_radius, _NS_SAMPLE_TINY_RADIUS ) )
      {
      _ns_sample_on_invalid( ret_radius, ret_length, did_create_sample );
      ns_sampler_remove( sampler, sample );
      }
   /*else
      error = ns_sample_make_images( sampler, sample );*/

   _NS_SAMPLE_EXIT:

   ns_byte_array_destruct( &buffer1 );

#ifdef NS_SAMPLER_RUN_ENABLE_3D
   ns_rayburst_destruct( &shape_rayburst );
   ns_byte_array_destruct( &buffer2 );
#endif/* NS_SAMPLER_RUN_ENABLE_3D */

   ns_rayburst_destruct( &radius_rayburst );
   return error;
   }


NsError ns_sampler_run_2d
	(
	NsSampler             *sampler,
	const NsImage         *image,
	const NsPoint2f       *P1,
	const NsPoint2f       *P2,
	nsfloat                z,
	nssize                 num_steps,
	NsRayburstInterpType   interp_type,
	const NsVoxelInfo     *voxel_info,
	nsdouble               average_intensity,
	const NsVector2i      *jitter2i,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	nsboolean              use_fixed_threshold,
	nsfloat                threshold_value,
	nsfloat               *radius,
	nsfloat               *length,
	nsboolean             *did_create_sample
	)
	{
	NsVector3f  V1, V2;
	NsVector3i  jitter3i;


	ns_assert( NULL != P1 );
	ns_assert( NULL != P2 );
	ns_assert( NULL != jitter2i );

	ns_vector2f_to_3f( P1, &V1 );
	ns_vector2f_to_3f( P2, &V2 );

	V1.z = V2.z = z;

	ns_vector2i_to_3i( jitter2i, &jitter3i );

	jitter3i.z = 0;

	return ns_sampler_run(
				sampler,
				image,
				&V1,
				&V2,
				num_steps,
				NS_FALSE,
				NS_RAYBURST_KERNEL_UNKNOWN, /* NOTE: Not used. */
				interp_type,
				voxel_info,
				average_intensity,
				&jitter3i,
				NS_TRUE,
				aabbox_scalar,
				min_window,
				use_fixed_threshold,
				threshold_value,
				radius,
				length,
				did_create_sample
				);
	}



NS_PRIVATE NsError _ns_sampler_run_direction
   (
   NsSampler             *sampler,
   nspointer              prev_sample,
   const NsVector3f      *prev_position,
   const NsVector3f      *direction,
   const NsImage         *image,
	const NsCubei         *roi,
   const NsVector3f      *max,
   nsboolean              do_3d_radii,
   NsRayburstKernelType   kernel_type,
   NsRayburstInterpType   interp_type,
   const NsVoxelInfo     *voxel_info,
   nsdouble               average_intensity,
   const NsVector3i      *jitter,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   nsboolean              use_fixed_threshold,
   nsfloat                threshold_value,
   nsboolean             *did_create_sample
   )
   {
   NsVector3f   curr_position;
   nsfloat      radius;
   nsfloat      d1;
   nsfloat      d2;
   nsfloat      delta;
   nsfloat      unused_radius;
   nsfloat      unused_length;
   NsVector3f   Pf;
   nsint       *indices;
   nsint        num_indices;
   nsint        i;
   nsint        closest;
   NsError      error;
   nsint        x;
   nsint        y;
   nsint        z;
   NsVector3i   Pi;


   *did_create_sample = NS_FALSE;

   radius = ns_sample_get_radius( prev_sample );

   /* Advance the position. Dont let the increment be too small or we'll
      end up on the same voxel as before. */
   d1    = radius * 0.5f;
   d2    = 1.1f * ns_voxel_info_max_size_xy( voxel_info );
   delta = NS_MAX( d1, d2 );

   curr_position.x = prev_position->x + direction->x * delta;
   curr_position.y = prev_position->y + direction->y * delta;

   /* Check for out of bounds.*/
   if( curr_position.x < 0.0f || max->x <= curr_position.x )return ns_no_error();
   if( curr_position.y < 0.0f || max->y <= curr_position.y )return ns_no_error();

   /* Convert back to image space and target this x,y location. */
   x = ns_to_image_space_component( curr_position.x, voxel_info, NS_COMPONENT_X );
   y = ns_to_image_space_component( curr_position.y, voxel_info, NS_COMPONENT_Y );

   ns_assert( 0 <= x );
   ns_assert( 0 <= y );
   ns_assert( ( nssize )x < ns_image_width( image ) );
   ns_assert( ( nssize )y < ns_image_height( image ) );

   indices     = NULL;
   num_indices = 0;

   if( NS_FAILURE(
         ns_seed_find_2d_ex(
            image,
				roi,
            x,
            y,
            NS_SEED_XY_FIND_Z,
            &indices,
            &num_indices,
				NULL
            ),
         error ) )
      return error;

   //ns_println( "# of indices = " NS_FMT_INT, num_indices );

   if( 0 == num_indices )
      z = ns_to_image_space_component( prev_position->z, voxel_info, NS_COMPONENT_Z );
   else
      {
      closest = 0;

      if( 1 < num_indices )
         {
         /* We have more than one possible z location. Find the one that
            is closest to the previous z. */
         z = ns_to_image_space_component( prev_position->z, voxel_info, NS_COMPONENT_Z );

         for( i = 1; i < num_indices; ++i )
            if( NS_ABS( indices[i] - z ) < NS_ABS( indices[ closest ] - z ) )
               closest = i;
         }

      z = ( nsint )indices[ closest ];
      }

   ns_free( indices );

   indices     = NULL;
   num_indices = 0;

   ns_assert( 0 <= z );
   ns_assert( ( nssize )z < ns_image_length( image ) );

   Pi.x = x;
   Pi.y = y;
   Pi.z = z;

   /* Convert back to voxel space. */
   ns_to_voxel_space( &Pi, &Pf, voxel_info );

   ns_assert( 0.0f <= Pf.x );
   ns_assert( 0.0f <= Pf.y );
   ns_assert( 0.0f <= Pf.z );
   ns_assert( Pf.x < max->x );
   ns_assert( Pf.y < max->y );
   ns_assert( Pf.z < max->z );

   /* Try to obtain a sample at this point. */
   if( NS_FAILURE(
         ns_sampler_run(
            sampler,
            image,
            &Pf,
            &Pf,
            0,
            do_3d_radii,
            kernel_type,
            interp_type,
            voxel_info,
            average_intensity,
            jitter,
            use_2d_sampling,
				aabbox_scalar,
				min_window,
            use_fixed_threshold,
            threshold_value,
            &unused_radius,
            &unused_length,
            did_create_sample
            ),
         error ) )
      return error;

   return ns_no_error();
   }


NS_PRIVATE nsfloat _ns_sample_perp_offset( nspointer sample, const NsVector3f *D )
   {
   NsVector3f  R1, R2, V;
   nsfloat     angle;
   nsfloat     perp_offset;


   ns_sample_get_rays( sample, &R1, &R2 );
   ns_vector3f_sub( &V, &R2, &R1 );
   V.z = 0;

   angle = ns_vector3f_angle( &V, D );
   angle = ( nsfloat )NS_RADIANS_TO_DEGREES( angle );

   perp_offset = 90.0f - angle;
   perp_offset = NS_ABS( perp_offset );

   return perp_offset;
   }


#define _ROT_ANGLE 45.0f

enum { _ROT_NEG, _ROT_NONE, _ROT_POS, _NUM_ROTS };

#define _ROT_FUNC( which )\
   if( NS_FAILURE(\
         _ns_sampler_run_direction(\
            sampler,\
            prev_sample,\
            prev_position,\
            D + which,\
            image,\
				roi,\
            max,\
            do_3d_radii,\
            kernel_type,\
            interp_type,\
            voxel_info,\
            average_intensity,\
            jitter,\
            use_2d_sampling,\
				aabbox_scalar,\
				min_window,\
            use_fixed_threshold,\
            threshold_value,\
            did_create_sample\
            ),\
         error ) )\
      return error;\
   \
   if( *did_create_sample )\
      {\
      sample[ which ] = ns_sampler_last( sampler );\
      ns_assert( sample[ which ] != prev_sample );\
      }

NS_PRIVATE NsError _ns_sampler_run_directions
   (
   NsSampler             *sampler,
   nspointer              prev_sample,
   const NsVector3f      *prev_position,
   const NsVector3f      *direction,
   const NsImage         *image,
	const NsCubei         *roi,
   const NsVector3f      *max,
   nsboolean              do_3d_radii,
   NsRayburstKernelType   kernel_type,
   NsRayburstInterpType   interp_type,
   const NsVoxelInfo     *voxel_info,
   nsdouble               average_intensity,
   const NsVector3i      *jitter,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   nsboolean              use_fixed_threshold,
   nsfloat                threshold_value,
   nsboolean             *did_create_sample
   )
   {
   NsMatrix44f  M;
   nssize       i;
   nspointer    sample[ _NUM_ROTS ];
   NsVector3f   D[ _NUM_ROTS ];
   nsfloat      perp_offset[ _NUM_ROTS ];
   nssize       most_perp;
   NsError      error;


   for( i = 0; i < _NUM_ROTS; ++i )
      sample[i] = NULL;

   /* Run direction vector. */
   D[ _ROT_NONE ] = *direction;
   _ROT_FUNC( _ROT_NONE );

   /* Rotate direction vector a positive angle and re-run. */
   ns_matrix44f_rotation_z( &M, ( nsfloat )NS_DEGREES_TO_RADIANS( _ROT_ANGLE ) );
   ns_matrix44f_mul_vector3f( D + _ROT_POS, &M, direction );
   ns_vector3f_norm( D + _ROT_POS );
   _ROT_FUNC( _ROT_POS );

   /* Rotate direction vector a negative angle and re-run. */
   ns_matrix44f_rotation_z( &M, ( nsfloat )NS_DEGREES_TO_RADIANS( -_ROT_ANGLE ) );
   ns_matrix44f_mul_vector3f( D + _ROT_NEG, &M, direction );
   ns_vector3f_norm( D + _ROT_NEG );
   _ROT_FUNC( _ROT_NEG );

   /* Compute the angle of the rayburst radius vectors versus the
      direction vector.  */
   for( i = 0; i < _NUM_ROTS; ++i )
      if( NULL != sample[i] )
         {
         //ns_print( "S" );
         perp_offset[i] = _ns_sample_perp_offset( sample[i], D + i );
         //ns_print( "::%.2f ", perp_offset[i] );
         }
      //else ns_print( "NULL " );

   //ns_println( "" );

   /* Find the one that is the most perpendicular. */
   most_perp = _NUM_ROTS;

   for( i = 0; i < _NUM_ROTS; ++i )
      if( NULL != sample[i] )
         if( _NUM_ROTS == most_perp || perp_offset[i] < perp_offset[ most_perp ] )
            most_perp = i;

   /* Remove all samples except the most perpendicular one. */
   if( _NUM_ROTS != most_perp )
      {
      //ns_println( "most_perp = %f\n", perp_offset[ most_perp ] );

      for( i = 0; i < _NUM_ROTS; ++i )
         if( ( NULL != sample[i] ) && ( i != most_perp ) )
            ns_sampler_remove( sampler, sample[i] );

      ns_assert( NULL != sample[ most_perp ] );
      ns_assert( ns_sampler_last( sampler ) == sample[ most_perp ] );

      *did_create_sample = NS_TRUE;
      }
   else
      *did_create_sample = NS_FALSE;

   return ns_no_error();
   }


NS_PRIVATE const NsSpheref* _ns_sample_to_sphere( const NsSample *sample, NsSpheref *S )
   {
   S->O = sample->C;
   S->r = sample->radius;

   return S;
   }


NS_PRIVATE nsfloat _ns_sample_iter_distance_to_next( nslistiter curr )
   {
   nslistiter next = ns_list_iter_next( curr );

   return ns_vector3f_distance(
            &( ( const NsSample* )ns_list_iter_get_object( curr ) )->C,
            &( ( const NsSample* )ns_list_iter_get_object( next ) )->C
            );
   }


#define _NS_SAMPLE_LOOKBACK  6

NS_PRIVATE nsboolean _ns_sample_intersects_samples( nspointer handle, NsSampler *sampler )
   {
   nslistiter       base, scan, end;
   const NsSample  *sample;
   NsPoint3f        P3;
   NsPoint2f        P2;
   NsCirclef        C;
   NsSpheref        S;
   nssize           i;


   ns_assert( ! ns_sampler_is_empty( sampler ) );
   ns_assert( handle == ns_sampler_last( sampler ) );

   if( ns_sampler_size( sampler ) < _NS_SAMPLE_LOOKBACK )
      return NS_FALSE;

   P3 = _ns_sample( handle )->C;

   base = ns_list_rev_begin( &sampler->samples );
   end  = ns_list_rev_end( &sampler->samples );

   /* Need a little "room" since samples can fall on each other and
      these are not really loops. */
   for( i = 0; i < _NS_SAMPLE_LOOKBACK - 1; ++i )
      base = ns_list_rev_iter_next( base );

   for( ; ns_list_rev_iter_not_equal( base, end );
          base = ns_list_rev_iter_next( base ) )
      {
      sample = ns_list_rev_iter_get_object( base );

      if( ns_point3f_inside_sphere( &P3, _ns_sample_to_sphere( sample, &S ) ) )
         {
         ns_println( "FOUND LOOP" );

         /* Check if all the loop points are within some scaled radius
            of the "base" of the loop. NOTE: Only do XY plane. */
         ns_circlef( &C, S.O.x, S.O.y, S.r * 4.0f );

         scan = ns_list_rev_begin( &sampler->samples );

         for( ; ns_list_rev_iter_not_equal( scan, base );
                scan = ns_list_rev_iter_next( scan ) )
            {
            sample = ns_list_rev_iter_get_object( scan );

            ns_point2f( &P2, sample->C.x, sample->C.y );

            /* Found a point in the loop that is outside the scaled
               radius so we wont cut this loop, i.e. just return. */
            if( ! ns_point2f_inside_circle( &P2, &C ) )
               {
               ns_println( "LOOP TOO BIG" );
               return NS_TRUE;
               }
            }

         ns_println( "ERASING LOOP" );

         /* All the points in the loop are within the scaled radius.
            So remove all points after the base. */
         ns_list_erase_all(
            &sampler->samples,
            ns_list_iter_next( base ),
            ns_list_end( &sampler->samples )
            );

         return NS_TRUE;
         }
      }

   return NS_FALSE;
   }


NS_PRIVATE nsboolean _ns_sample_intersects_model( nspointer handle, const NsModel *model )
   {
   NsPoint3f      P;
   NsSpheref      S;
   nsmodelvertex  curr;
   nsmodelvertex  end;


   P = _ns_sample( handle )->C;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      {
      ns_model_vertex_to_sphere( curr, &S );

      if( ns_point3f_inside_sphere( &P, &S ) )
         return NS_TRUE;
      }

   return NS_FALSE;
   }


NS_PRIVATE nsboolean _ns_sample_outside_roi( nspointer handle, const NsVoxelInfo *voxel_info, const NsCubei *roi )
	{
	NsPoint3i P;

	ns_to_image_space( &( _ns_sample( handle )->C ), &P, voxel_info );

	return P.x < roi->C1.x ||
			 P.y < roi->C1.y ||
			 P.z < roi->C1.z ||
			 P.x > roi->C2.x ||
			 P.y > roi->C2.y ||
			 P.z > roi->C2.z;
	}


nsboolean ns_sampler_is_running( const NsSampler *sampler )
   {
   ns_assert( NULL != sampler );
   return sampler->is_running;
   }


NsError ns_sampler_run_ex
   (
   NsSampler             *sampler,
   nspointer              prev_sample,
   const NsVector3f      *V1,
   const NsVector3f      *V2,
   const NsModel         *model,
   const NsImage         *image,
	const NsCubei         *roi,
   nsboolean              do_3d_radii,
   NsRayburstKernelType   kernel_type,
   NsRayburstInterpType   interp_type,
   const NsVoxelInfo     *voxel_info,
   nsdouble               average_intensity,
   const NsVector3i      *jitter,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   nsboolean              use_fixed_threshold,
   nsfloat                threshold_value,
   nssize                *num_created_samples,
   void                   ( *render )( nspointer user_data ),
   nspointer              user_data,
   NsProgress            *progress
   )
   {
   NsVector3f  direction;
   nspointer   curr_sample;
   NsVector3f  prev_position;
	nsfloat     prev_radius;
   NsVector3f  curr_position;
	nsfloat     curr_radius;
   nsfloat     magnitude;
	nsfloat     distance;
   nsboolean   did_create_sample;
   NsVector3f  max;
   NsVector3i  P;
   NsError     error;

   
   ns_assert( NULL != sampler );
   ns_assert( ! ns_sampler_is_empty( sampler ) );
   ns_assert( prev_sample == ns_sampler_last( sampler ) );
   ns_assert( NULL != V1 );
   ns_assert( NULL != V2 );
   ns_assert( NULL != image );
   ns_assert( NULL != voxel_info );
   ns_assert( NULL != jitter );
   ns_assert( NULL != num_created_samples );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( sampler=" NS_FMT_POINTER
		", prev_sample=" NS_FMT_POINTER
		", V1={x:" NS_FMT_DOUBLE ",y:" NS_FMT_DOUBLE ",z:" NS_FMT_DOUBLE "}"
		", V2={x:" NS_FMT_DOUBLE ",y:" NS_FMT_DOUBLE ",z:" NS_FMT_DOUBLE "}"
		", model=" NS_FMT_STRING_DOUBLE_QUOTED
		", image=" NS_FMT_POINTER
		", roi=" NS_FMT_POINTER
		", do_3d_radii=" NS_FMT_INT
		", kernel_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", interp_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", voxel_info=" NS_FMT_POINTER
		", average_intensity=" NS_FMT_DOUBLE
		", jitter={x:" NS_FMT_INT ",y:" NS_FMT_INT ",z:" NS_FMT_INT "}"
		", use_2d_sampling=" NS_FMT_INT
		", aabbox_scalar=" NS_FMT_DOUBLE
		", min_window=" NS_FMT_INT
		", use_fixed_threshold=" NS_FMT_INT
		", threshold_value=" NS_FMT_DOUBLE
		", num_created_samples=" NS_FMT_POINTER
		", render=" NS_FMT_POINTER
		", user_data=" NS_FMT_POINTER
		", progress=" NS_FMT_POINTER
		" )",
		sampler,
		prev_sample,
		V1->x, V1->y, V1->z,
		V2->x, V2->y, V2->z,
		ns_model_get_name( model ),
		image,
		roi,
		do_3d_radii,
		ns_rayburst_kernel_type_to_string( kernel_type ),
		ns_rayburst_interp_type_to_string( interp_type ),
		voxel_info,
		average_intensity,
		jitter->x, jitter->y, jitter->z,
		use_2d_sampling,
		aabbox_scalar,
		min_window,
		use_fixed_threshold,
		threshold_value,
		num_created_samples,
		render,
		user_data,
		progress
		);

   error = ns_no_error();

   /* Get direction vector. Disregard Z component. */
   ns_vector3f_sub( &direction, V2, V1 );
   direction.z = 0.0f;

   //ns_println( "initial direction = %f,%f,%f", direction.x, direction.y, direction.z );

   magnitude = ns_vector3f_mag( &direction );
   if( NS_FLOAT_EQUAL( magnitude, 0.0f ) )
      return ns_no_error();

   ns_vector3f_norm( &direction );

   ns_vector3i(
      &P,
      ( nsint )ns_image_width( image ),
      ( nsint )ns_image_height( image ),
      ( nsint )ns_image_length( image )
      );

   ns_to_voxel_space( &P, &max, voxel_info );

	/* First check if the passed sample already intersects 
		the current model. If so we just do nothing. */
	if( _ns_sample_intersects_model( prev_sample, model ) )
		return ns_no_error();

   sampler->is_running = NS_TRUE;

   NS_INFINITE_LOOP
      {
      if( NULL != progress && ns_progress_cancelled( progress ) )
         break;

      ns_sample_get_center( prev_sample, &prev_position );

      if( NS_FAILURE(
            _ns_sampler_run_directions(
               sampler,
               prev_sample,
               &prev_position,
               &direction,
               image,
					roi,
               &max,
               do_3d_radii,
               kernel_type,
               interp_type,
               voxel_info,
               average_intensity,
               jitter,
               use_2d_sampling,
					aabbox_scalar,
					min_window,
               use_fixed_threshold,
               threshold_value,
               &did_create_sample
               ),
            error ) )
         break;

      if( ! did_create_sample )
         break;

      curr_sample = ns_sampler_last( sampler );
      ns_assert( curr_sample != prev_sample );

      if( _ns_sample_intersects_model( curr_sample, model ) ||
          _ns_sample_intersects_samples( curr_sample, sampler ) )
         break;

		if( _ns_sample_outside_roi( curr_sample, voxel_info, roi ) )
			{
			ns_sampler_remove( sampler, curr_sample );
			break;
			}

      ns_sample_get_center( curr_sample, &curr_position );

      ns_vector3f_sub( &direction, &curr_position, &prev_position );
      direction.z = 0.0f;

      magnitude = ns_vector3f_mag( &direction );

      /* If we didnt move anywhere from the previous to the
         current sample... */
      if( NS_FLOAT_EQUAL( magnitude, 0.0f ) )
         break;

		distance    = ns_vector3f_distance( &curr_position, &prev_position );
		prev_radius = ns_sample_get_radius( prev_sample );
		curr_radius = ns_sample_get_radius( curr_sample );

		/* Did we hit a "jump"? If so then remove the current sample and end. */
		if( ( distance > /*TEMP???*/3.0f * prev_radius ) && ( distance > curr_radius ) )
			{
			ns_println( "Sampler run hit a jump. Aborting." );
			ns_sampler_remove( sampler, curr_sample );
			break;
			}

      ns_vector3f_norm( &direction );

      prev_sample = curr_sample;

      if( NULL != render )
         ( render )( user_data );
      }

   sampler->is_running = NS_FALSE;

   return error;
   }
