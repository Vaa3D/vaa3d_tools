
NS_PRIVATE void _ns_spines_do_analyze( NsSpines *spines, NsSpine *spine )
   {
   nslistiter   curr_voxel;
   nslistiter   end_voxels;
   NsVoxel     *voxel;
   NsVoxel     *min;
   nsfloat      distance_range;
   nsfloat      distance_cutoff;
   nsfloat      curr_intensity;
   nsfloat      sum_intensity;
   NsVector3f   V;
   //NsVector3f   BC;
	nsdouble     min_dts;
	nsdouble     max_dts;
	nsdouble     half_delta;


   spine->min_distance = NS_FLOAT_MAX;
   spine->max_distance = -NS_FLOAT_MAX;

	spine->maxima = NULL;
   min = NULL;

   ns_assert( ! ns_list_is_empty( &spine->voxels ) );

   curr_voxel = ns_list_begin( &spine->voxels );
   end_voxels = ns_list_end( &spine->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

      if( __DATA( voxel, distance ) < spine->min_distance )
         {
         spine->min_distance = __DATA( voxel, distance );
         min                 = voxel;
         }

		if( NULL == spine->maxima || __DATA( voxel, distance ) > __DATA( spine->maxima, distance ) )
			spine->maxima = voxel;
      }

   ns_assert( NULL != spine->origin );
   spine->max_distance = __DATA( spine->origin, distance );
   ns_to_voxel_space( &(spine->origin->position), &spine->max, spines->voxel_info );

   ns_assert( NULL != min );

	spine->minima = min;
   ns_to_voxel_space( &min->position, &spine->min, spines->voxel_info );

   /* Partition the spine into the farthest 80%( for center )
      and closest 20%( for base ). */
   distance_range = spine->max_distance - spine->min_distance;
   ns_assert( 0.0f <= distance_range );

   /* TEMP??? */
   #define __LOWER_PERCENT  .2f

   distance_cutoff = spine->min_distance + distance_range * __LOWER_PERCENT;

   /* Do a weighted average on the x,y,z of the farthest 80% to
      get the center point of the spine. */
   sum_intensity = 0.0f;
   ns_vector3f_zero( &spine->center );

   curr_voxel = ns_list_begin( &spine->voxels );
   end_voxels = ns_list_end( &spine->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

      /* Using greater or equal so that voxels on the boundary (or when
         there is only one voxel) are counted. This prevents dividing by
         zero if all the voxels have the same DTS. See alos base calculation
         where less than or equal is used as well. */
      if( distance_cutoff /*greater-equal*/<= __DATA( voxel, distance ) )
         {
         curr_intensity = ns_powf( voxel->intensity, __INTENSITY_POWER );

         sum_intensity += curr_intensity;

         ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

         ns_vector3f_cmpd_scale( &V, curr_intensity );
         ns_vector3f_cmpd_add( &spine->center, &V );
         }
      }

   //ns_println( "#of voxels = %lu sum_intensity=%f", ns_list_size( &spine->voxels ), sum_intensity );

   ns_vector3f_cmpd_scale( &spine->center, 1.0f / sum_intensity );

   /* Do a weighted average on the x,y,z of the closest 20% to
      get the base point of the spine. */
   sum_intensity = 0.0f;
   ns_vector3f_zero( &spine->base );

   curr_voxel = ns_list_begin( &spine->voxels );
   end_voxels = ns_list_end( &spine->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

      if( __DATA( voxel, distance ) /*less-equal*/<= distance_cutoff )
         {
         curr_intensity = ns_powf( voxel->intensity, __INTENSITY_POWER );

         sum_intensity += curr_intensity;

         ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

         ns_vector3f_cmpd_scale( &V, curr_intensity );
         ns_vector3f_cmpd_add( &spine->base, &V );
         }
      }

   ns_vector3f_cmpd_scale( &spine->base, 1.0f / sum_intensity );

   //ns_vector3f_sub( &BC, &spine->base, &spine->center );

	spine->rays_origin = spine->center;

   /* Include the length of the "cap" onto the medial axis. */

   /* Include the length of the "unseen neck" onto the medial axis,
      that is if there is one. */

	/* Spine Height Calculation:
			|
			|
			|
			|--------*--------------*--------------*
			|        ^min_dts       ^half_delta    ^max_dts
			|                                                   
			|
			^ medial axis of dendrite

	Somewhere in between 'half_delta' and 'max_dts' lies the ideal maximum.
	*/

	min_dts = spine->min_distance * spine->correction_factor;
	max_dts = spine->max_distance * spine->correction_factor;

	half_delta = ( max_dts - min_dts ) / 2.0;

	spine->height = ( nsfloat )( min_dts + half_delta + half_delta / spines->stretch_factor );
	}


NS_PRIVATE void _ns_spines_analyze_shapes( NsSpines *spines )
   {
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   ns_progress_set_title( spines->progress, "Analyzing spine shapes..." );
   ns_progress_begin( spines->progress );

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      if( ns_progress_cancelled( spines->progress ) )
         return;

      spine = ns_list_iter_get_object( curr );
 
		if( ! spine->is_analyzed )
			_ns_spines_do_analyze( spines, spine );
      }

   ns_progress_end( spines->progress );
   }


NS_PRIVATE NsError _ns_spines_do_calc_attachment
   (
   NsSpines  *spines,
   NsSpine   *spine,
   NsVector  *nodes
   )
	{
	const NsSpineAxisPoint  *point;
	NsVector3f               P;
   NsVector3f               V;
   nsvectoriter             curr_node;
   nsvectoriter             end_nodes;
   const NsOctreeNode      *node;
   nsvectoriter             curr_object;
   nsvectoriter             end_objects;
   nsmodeledge              edge;
   NsVector3f               attach;
   nsfloat                  distance;
   nsfloat                  min_distance;
	NsError                  error;


	if( ! ns_list_is_empty( &(spine->axis.points) ) )
		{
		point = ns_list_iter_get_object( ns_list_rev_begin( &spine->axis.points ) );
		P     = point->P;
		}
	else
		P = spine->center;

   if( NS_FAILURE(
         ns_model_octree_intersections(
            spines->model,
            NS_MODEL_EDGE_OCTREE,
            &P,//&spine->center,//&spine->base,
            _ns_spine_voxel_octree_func_special,
            nodes
            ),
         error ) )
		return error;

   min_distance = NS_FLOAT_MAX;

   curr_node = ns_vector_begin( nodes );
   end_nodes = ns_vector_end( nodes );

   for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
          curr_node = ns_vector_iter_next( curr_node ) )
      {
      node = ns_vector_iter_get_object( curr_node );
      //ns_assert( 0 < ns_octree_node_num_objects( node ) );

      curr_object = ns_octree_node_begin_objects( node );
      end_objects = ns_octree_node_end_objects( node );

      for( ; ns_vector_iter_not_equal( curr_object, end_objects );
             curr_object = ns_vector_iter_next( curr_object ) )
         {
         edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

         distance = _ns_spines_distance_to_conical_frustum(
							spines,
                     &P,//&spine->center,//&spine->base,
                     edge,
                     &attach,
                     NULL,
							NULL,
                     NULL,
							NULL
                     );

         if( distance < min_distance )
            {
            min_distance = distance;

            spine->attach        = attach;
            //spine->base_distance = distance;

            spine->section = ns_model_edge_get_section( edge );
            spine->order   = ns_model_edge_get_order( edge );

            spine->section_length = ns_model_section_length( spines->model, spine->section );
            }
         }
      }

   ns_verify( NS_SUCCESS( ns_vector_resize( nodes, 0 ), error ) );

   /* The spine->attach point is on the medial axis. Now find where it
      hits the surface. Use the maximum to attach vector. */
   ns_vector3f_sub( &spine->approx_axis, &spine->attach, &spine->max );
   ns_vector3f_norm( &spine->approx_axis );
   ns_vector3f_scale( &V, &spine->approx_axis, spine->max_distance );
   ns_vector3f_add( &spine->surface, &spine->max, &V );

	/* Reset axis for rendering. Use the maximum to surface vector. */
	ns_vector3f_sub( &spine->approx_axis, &spine->surface, &spine->max );
   ns_vector3f_norm( &spine->approx_axis );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spines_calc_attachments( NsSpines *spines )
   {
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;
   NsVector     nodes;
   NsError      error;


   error = ns_no_error();

   ns_progress_set_title( spines->progress, "Calculating spine attachments..." );
   ns_progress_begin( spines->progress );

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      if( ns_progress_cancelled( spines->progress ) )
         return ns_no_error();

      spine = ns_list_iter_get_object( curr );
 
		if( ! spine->is_analyzed )
			if( NS_FAILURE( _ns_spines_do_calc_attachment( spines, spine, &nodes ), error ) )
				goto _NS_SPINES_ANALYZE_SHAPES_EXIT;
      }

   ns_progress_end( spines->progress );

   _NS_SPINES_ANALYZE_SHAPES_EXIT:

   ns_vector_destruct( &nodes );
   return error;
   }


/* TEMP!!!!!!!!!! **************************/
nsfloat ____attach_sa_scalar;

void ____load_attach_sa_scalar( void )
   {
   ____attach_sa_scalar = 2.0f;
   ns_println( "\n\nattachment surface area scalar = " NS_FMT_DOUBLE "\n", ____attach_sa_scalar );
   }


NS_PRIVATE NsTriangle3f* _ns_spines_make_triangle
   (
   const NsSpine            *spine,
   const NsVector3f         *vectors,
   const NsIndexTriangleus  *triangles,
   const nsfloat            *distances,
   nssize                    index,
   NsTriangle3f             *T
   )
   {
   T->A = vectors[ triangles[ index ].a ];
   T->B = vectors[ triangles[ index ].b ];
   T->C = vectors[ triangles[ index ].c ];

	T->A.z /= spine->stretch_factor;
	T->B.z /= spine->stretch_factor;
	T->C.z /= spine->stretch_factor;

   ns_vector3f_cmpd_scale( &T->A, distances[ triangles[ index ].a ] );
   ns_vector3f_cmpd_scale( &T->B, distances[ triangles[ index ].b ] );
   ns_vector3f_cmpd_scale( &T->C, distances[ triangles[ index ].c ] );

   ns_vector3f_cmpd_add( &T->A, &spine->rays_origin );
   ns_vector3f_cmpd_add( &T->B, &spine->rays_origin );
   ns_vector3f_cmpd_add( &T->C, &spine->rays_origin );

   return T;
   }


NS_PRIVATE NsError _ns_spines_calc_normals
   (
   NsSpines          *spines,
   NsSpine           *spine,
   const NsRayburst  *rayburst
   )
   {
/*TEMP: Avoid lengthy normal calculation process. Its only for visual display anyways. */
NS_USE_VARIABLE( spines );
NS_USE_VARIABLE( spine );
NS_USE_VARIABLE( rayburst );

/*
   nssize                    num_samples;
   const NsVector3f         *vectors;
   const NsIndexTriangleus  *triangles;
   nssize                    num_triangles;
   NsTriangle3f              T;
   nssize                    i;
   NsVector3f                BA, CA, N;
   NsRayburstKernelType      kernel_type;


   NS_USE_VARIABLE( spines );

   num_samples = ns_rayburst_num_samples( rayburst );

   if( NULL == ( spine->normals = ns_new_array( NsVector3f, num_samples ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   kernel_type = ns_rayburst_kernel_type( rayburst );

   vectors = ns_rayburst_kernel_type_vectors( kernel_type );
   ns_assert( NULL != vectors );

   triangles = ns_rayburst_kernel_type_triangles( kernel_type );
   ns_assert( NULL != triangles );

   num_triangles = ns_rayburst_kernel_type_num_triangles( kernel_type );

   for( i = 0; i < num_samples; ++i )
      ns_vector3f_zero( spine->normals + i );

   for( i = 0; i < num_triangles; ++i )
      {
      _ns_spines_make_triangle(
         spine,
         vectors,
         triangles,
         spine->distances,
         i,
         &T         
         );

      ns_vector3f_sub( &BA, &T.B, &T.A ); /* BA = B - A *//*
      ns_vector3f_sub( &CA, &T.C, &T.A ); /* CA = C - A *//*

      ns_vector3f_cross( &N, &BA, &CA ); /* N = BA x CA *//*
      ns_vector3f_norm( &N );

      ns_vector3f_cmpd_add( spine->normals + triangles[i].a, &N );
      ns_vector3f_cmpd_add( spine->normals + triangles[i].b, &N );
      ns_vector3f_cmpd_add( spine->normals + triangles[i].c, &N );
      }

   for( i = 0; i < num_samples; ++i )
      ns_vector3f_norm( spine->normals + i );
*/
	return ns_no_error();
   }


/* Applying the or '||' operator allows triangles on the edges to
   sort of "fold out" from the actual neurite surface area.

   Applying the and '&&' operator allows triangles on the edges to
   sort of "fold in" from the actual neurite surface area. */

#define __IS_INTERIOR_TRIANGLE( interior, triangles, i, op )\
   ( ns_bit_buf_is_set( (interior), (triangles)[(i)].a ) op\
     ns_bit_buf_is_set( (interior), (triangles)[(i)].b ) op\
     ns_bit_buf_is_set( (interior), (triangles)[(i)].c )    )


NS_PRIVATE NsError _ns_spines_calc_shapes( NsSpines *spines )
   {
   nslistiter                curr;
   nslistiter                end;
   NsSpine                  *spine;
   nssize                    num_samples;
   const NsVector3f         *vectors;
   nssize                    num_triangles;
   const NsIndexTriangleus  *triangles;
   nssize                    i, j;
   nsulong                   intensity;
   NsVector3f               *points;
   nsfloat                   threshold;
	nsfloat                   contrast;
   nsfloat                   multiplier;
   const NsVoxel            *voxel;
   nsfloat                   distance;
   NsTriangle3f              T;
   nsdouble                  attach_sa_fold_out;
   nsdouble                  attach_sa_fold_in;
   nsboolean                 exterior;
   NsVector3i                P;
   NsVector3i                N;
   nsint                     width, height, length;
	NsRayburstKernelType      kernel_type;
	NsError                   error;



   /*TEMP*/
   ____load_attach_sa_scalar();


   ns_progress_set_title( spines->progress, "Creating spine surfaces..." );
   ns_progress_num_iters( spines->progress, _ns_spines_size( spines ) );
   ns_progress_begin( spines->progress );

   num_samples   = ns_rayburst_num_samples( &spines->rb_surface );
   vectors       = ns_rayburst_vectors( &spines->rb_surface );
   triangles     = ns_rayburst_triangles( &spines->rb_surface );
   num_triangles = ns_rayburst_num_triangles( &spines->rb_surface );

   points = NULL;

   if( NULL == ( points = ns_new_array( NsVector3f, num_samples ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   width  = ns_voxel_buffer_width( &spines->voxel_buffer );
   height = ns_voxel_buffer_height( &spines->voxel_buffer );
   length = ns_voxel_buffer_length( &spines->voxel_buffer );

   multiplier = ns_settings_get_threshold_multiplier( spines->settings );

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      if( ns_progress_cancelled( spines->progress ) )
			{
			ns_free( points );
         return ns_no_error();
			}

      spine = ns_list_iter_get_object( curr );

		if( spine->is_analyzed )
			{
			ns_progress_next_iter( spines->progress );
			continue;
			}

      spine->total_volume   = 0.0;
      spine->total_sa       = 0.0;
		spine->stretch_factor = spines->stretch_factor;

      spine->owner = spines;
      ns_rayburst_set_user_data( &spines->rb_surface, spine );

      _ns_spine_calc_threshold_and_contrast( spine, &threshold, &contrast, spines );
      threshold = ns_model_scale_threshold( threshold, contrast, multiplier );

      ns_rayburst_set_threshold( &spines->rb_surface, threshold );

		spine->threshold = ( nsulong )threshold;

      /* Run a high-N rayburst at the center of mass to get the
         surface area and volume of this cluster. */
      ns_rayburst( &spines->rb_surface, &spine->rays_origin );

		kernel_type = ns_rayburst_kernel_type( &spines->rb_surface );


///*TEMP*/ns_print( "BEFORE V/S.A.=%f/%f   ",
//	ns_rayburst_volume( &spines->rb_surface ), ns_rayburst_surface_area( &spines->rb_surface ) );
		
		ns_rayburst_compute_volume_and_surface_area( &spines->rb_surface, spine->stretch_factor );

///*TEMP*/ns_println( "AFTER V/S.A.=%f/%f",
//	ns_rayburst_volume( &spines->rb_surface ), ns_rayburst_surface_area( &spines->rb_surface ) );


		spine->vectors        = ns_rayburst_kernel_type_vectors( kernel_type );
		spine->num_vectors    = ns_rayburst_kernel_type_num_vectors( kernel_type );
		spine->triangles      = ns_rayburst_kernel_type_triangles( kernel_type );
		spine->num_triangles  = ns_rayburst_kernel_type_num_triangles( kernel_type );

      if( NULL == ( spine->distances = ns_new_array( nsfloat, num_samples ) ) )
         {
         ns_free( points );
         return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
         }

      ns_rayburst_sample_distances(
         &spines->rb_surface,
         NS_RAYBURST_SAMPLE_DISTANCE_FORWARD,
         spine->distances
         );

      if( NS_FAILURE( _ns_spines_calc_normals( spines, spine, &spines->rb_surface ), error ) )
			{
         ns_free( points );
			return error;
			}

      spine->total_volume = ns_rayburst_volume( &spines->rb_surface );
      spine->total_sa     = ns_rayburst_surface_area( &spines->rb_surface );

      /* Find the surface area of the spine that is shared with a neurite, if there is any. */
      attach_sa_fold_in  = 0.0;
      attach_sa_fold_out = 0.0;
      spine->attach_sa   = 0.0;

      ns_rayburst_sample_points( &spines->rb_surface, NS_TRUE, points );

      if( NULL == ( spine->interior = ns_new_array( nsuint8, NS_BITS_TO_BYTES( num_samples ) ) ) )
         {
         ns_free( points );
         return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
         }			

      for( i = 0; i < num_samples; ++i )
         {
         ns_bit_buf_clear( spine->interior, i );

         /* Check the face neighbors of the current sample point. If all
            of them are above the threshold, mark this sample as interior,
            else mark it as exterior. */
         exterior = NS_FALSE;

         /* TEMP: Do we need to check against width, height, and length also? */
         if( 0.0f < points[i].x && 0.0f < points[i].y && 0.0f < points[i].z )
            {
            ns_to_image_space( points + i, &P, spines->voxel_info );

            for( j = 0; j < NS_VOXEL_NUM_FACE_OFFSETS; ++j )
               {
               ns_vector3i_add( &N, &P, spines->voxel_face_offsets + j );

               /* Make sure the position falls within the image. */
               if( 0 <= N.x && N.x < width  &&
                   0 <= N.y && N.y < height &&
                   0 <= N.z && N.z < length   )
                  {
                  intensity = ns_voxel_get( &spines->voxel_buffer, N.x, N.y, N.z );

                  if( intensity < spine->threshold )
                     exterior = NS_TRUE;
                  }
               else /* Falls outside image, default to being exterior. */
                  exterior = NS_TRUE;

               if( exterior )
                  break;
               }
            }

         if( ! exterior )
            {
            /* Check whether this sample is close enough to the
               "bottom" of the spine. This tries to prevent marking
               samples that traverse into other spines. */
            if( NULL != ( voxel = ns_voxel_table_find( &spines->voxel_table, &P ) ) )
               {
               distance = spine->min_distance - __DATA( voxel, distance );
               distance = NS_ABS( distance );

               if( distance <= ____attach_sa_scalar * ns_voxel_info_max_size( spines->voxel_info ) )
                  ns_bit_buf_set( spine->interior, i );
               }
            }
         }

		/* TEMP? is this correct, use de-stretched triangle for below calculcations. */

      for( i = 0; i < num_triangles; ++i )
         if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, || ) )
            {
            _ns_spines_make_triangle( spine, vectors, triangles, spine->distances, i, &T );
            attach_sa_fold_out += ns_triangle3f_surface_area( &T );
            }

      for( i = 0; i < num_triangles; ++i )
         if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, && ) )
            {
            _ns_spines_make_triangle( spine, vectors, triangles, spine->distances, i, &T );
            attach_sa_fold_in += ns_triangle3f_surface_area( &T );
            }

      /* The actual neurite surface area is the average of the two. */
      spine->attach_sa     = ( attach_sa_fold_out + attach_sa_fold_in ) / 2.0;
      spine->non_attach_sa = spine->total_sa - spine->attach_sa;

      spine->voxel_volume = ( nsdouble )ns_voxel_info_volume( spines->voxel_info ) *
                            ( nsdouble )ns_list_size( &spine->voxels );

      ns_progress_next_iter( spines->progress );
      }

   ns_progress_end( spines->progress );

   ns_free( points );
   return ns_no_error();
   }



