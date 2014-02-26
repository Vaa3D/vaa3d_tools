
nsboolean ns_spines_conical_frustum_octree_intersect_func
   (
   nsconstpointer     object,
   const NsAABBox3d  *box
   )
   {
   NsModel            *model;
   nsmodeledge         edge;
   NsConicalFrustumd   cf;
   NsVector3d          N;
   NsVector3d          C;
   nsdouble            radius;
   nsdouble            length;
   nsdouble            d;
   nsdouble            u;
   nsdouble            max_voxel_distance;
   NsSphered           sphere;
   nsboolean           done;


   edge  = ( nsmodeledge )object;
   model = ns_model_vertex_model( ns_model_edge_src_vertex( edge ) );

   max_voxel_distance = ( nsdouble )ns_value_get_float( ns_model_value( model ) );

   ns_model_edge_to_conical_frustum( edge, &cf );

   /* Increase the frustum size to allow for spine detection.
      NOTE: If the 'max_voxel_distance' is zero then the cylinder is   
      unchanged. */
   cf.r1 += max_voxel_distance;
   cf.r2 += max_voxel_distance;

   /* Get direction vector between the two endpoints. */
   ns_vector3d_sub( &N, &cf.C2, &cf.C1 );
   length = ns_vector3d_mag( &N );

   if( length <= NS_FLOAT_ALMOST_ZERO )
      {
      /* If the frustum has no length, just take the larger of
         the spheres and use as the intersection test. */
      if( cf.r1 > cf.r2 )
         ns_sphered( &sphere, cf.C1.x, cf.C1.y, cf.C1.z, cf.r1 );
      else
         ns_sphered( &sphere, cf.C2.x, cf.C2.y, cf.C2.z, cf.r2 );

      if( ns_sphered_intersects_aabbox( &sphere, box ) )
         return NS_TRUE;
      }
   else
      {
      ns_vector3d_norm( &N );

      /* Check intersection between spheres, which approximate the
         conical frustum, and the bounding box since there is no easy
         intersection test for a conical frustum and axis-aligned
         bounding box. */

      done = NS_FALSE;
      d    = 0.0;

      while( ! done )
         {
         /* Create a sphere by interpolating the current coordinate and radius. */
         ns_vector3d_scale( &C, &N, d );
         ns_vector3d_cmpd_add( &C, &cf.C1 );

         u = d / length;

         if( 1.0 <= u )
            {
            u    = 1.0;
            done = NS_TRUE;
            }

         /* If u = 0, then radius == radius1
            If u = 1, then radius == radius2 */
         radius = ( 1.0 - u ) * cf.r1 + u * cf.r2;

         /* Increase radius by 5% to account for "space" between spheres. */
         ns_sphered( &sphere, C.x, C.y, C.z, radius + radius * .05 );

         if( ns_sphered_intersects_aabbox( &sphere, box ) )
            return NS_TRUE;

         /* Move half the radius of the previous sphere. */
         d += radius / 2.0;
         }
      }

   return NS_FALSE;
   }


NS_PRIVATE NsError _ns_spines_do_build_model_octree( NsSpines *spines, nsboolean inflated )
	{
   NsError error;

   ns_progress_set_title( spines->progress, "Building Octree..." );

   ns_value_set_float(
      ns_model_value( spines->model ),
      inflated ? ns_settings_get_spine_max_voxel_distance( spines->settings ) : 0.0f
      );

   /* NOTE: Need to scale up the size of the models
      edges by the max spine distance so we wont miss any
      intersections in the octree. Also extend the bounding
      box of the octree. */
   if( NS_FAILURE( ns_model_build_edge_octree_ex(
                     spines->model,
                     /* TEMP: Hardcode these parameters? */
                     0,///*inflated ? */16 /*: 32*/,
                     inflated ? spines->max_octree_recursion : spines->max_octree_recursion + 1,
                     32,
                     ns_spines_conical_frustum_octree_intersect_func,
                     ns_settings_get_spine_max_voxel_distance( spines->settings ) * 2.0,
							ns_voxel_info_max_size( spines->voxel_info ) * 4.0,
                     spines->progress
                     ),
                     error ) )
      return error;


/* TEMP!!!!!!!!!!!!!!!!! */
{
const NsAABBox3d *B; NsPoint3d min, max;
B = ns_model_octree_aabbox( spines->model, NS_MODEL_EDGE_OCTREE );
ns_aabbox3d_min( B, &min );
ns_aabbox3d_max( B, &max );
ns_println( "BBOX = x[ %f, %f ] y[ %f, %f ] z[ %f, %f ]",
   min.x, max.x, min.y, max.y, min.z, max.z );
}



/* TEMP!!!!! */
   ns_progress_set_title( spines->progress, "Getting Octree Stats..." );   
   ns_model_octree_stats( spines->model, NS_MODEL_EDGE_OCTREE, &spines->octree_stats );

ns_println( "octree->num_objects          = " NS_FMT_ULONG, spines->octree_stats.num_objects );
ns_println( "octree->num_internal_nodes   = " NS_FMT_ULONG, spines->octree_stats.num_internal_nodes );
ns_println( "octree->num_leaf_nodes       = " NS_FMT_ULONG, spines->octree_stats.num_leaf_nodes );
ns_println( "octree->max_recursion        = " NS_FMT_ULONG, spines->octree_stats.max_recursion );
ns_println( "octree->max_node_objects     = " NS_FMT_ULONG, spines->octree_stats.max_node_objects );
ns_println( "octree->num_empty_leaf_nodes = " NS_FMT_ULONG, spines->octree_stats.num_empty_leaf_nodes );

ns_println( "octree->min_aabbox_size      = " NS_FMT_DOUBLE,
	ns_octree_get_min_aabbox_size( spines->model->octrees + NS_MODEL_EDGE_OCTREE ) );

ns_println( "octree->min_leaf_aabbox_size = " NS_FMT_DOUBLE, spines->octree_stats.min_leaf_aabbox_size );
ns_println( "octree->max_leaf_aabbox_size = " NS_FMT_DOUBLE, spines->octree_stats.max_leaf_aabbox_size );


   ns_progress_end( spines->progress );
   return ns_no_error();
	}


NS_PRIVATE NsError _ns_spines_build_inflated_model_octree( NsSpines *spines )
   {  return _ns_spines_do_build_model_octree( spines, NS_TRUE );  }


NS_PRIVATE NsError _ns_spines_build_normal_model_octree( NsSpines *spines )
   {  return _ns_spines_do_build_model_octree( spines, NS_FALSE );  }


NS_PRIVATE nsboolean _ns_falls_within_any_edge_zones( nsmodelvertex V, const NsPoint3f *P, nsmodeledge this_edge )
	{
	nsmodeledge     curr_edge, end_edges;
   nsmodelvertex   src_vertex, dest_vertex;
	NsVector3f      V1, V2, A, B;
	nsfloat         dot;


	curr_edge = ns_model_vertex_begin_edges( V );
	end_edges = ns_model_vertex_end_edges( V );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		if( ns_model_edge_not_equal( curr_edge, this_edge ) )
			{
			src_vertex  = ns_model_edge_src_vertex( curr_edge );
			dest_vertex = ns_model_edge_dest_vertex( curr_edge );

			ns_model_vertex_get_position( src_vertex,  &V1 );
			ns_model_vertex_get_position( dest_vertex, &V2 );

			ns_vector3f_sub( &A, &V2, &V1 );
			ns_vector3f_sub( &B, P, &V1 );

			if( ns_vector3f_dot( &A, &A ) > NS_FLOAT_ALMOST_ZERO )
				{	
				dot = ns_vector3f_dot( &A, &B );

				/* If A.B < 0 or A.B > A.A, P doesn't project onto the line. */

				if( 0.0f <= dot && dot <= ns_vector3f_dot( &A, &A ) )
					return NS_TRUE;
				}
			}

	return NS_FALSE;
	}


NS_PRIVATE nsfloat _ns_spines_distance_to_conical_frustum
   (
	NsSpines           *spines,
   const NsPoint3f    *P,
   const nsmodeledge   edge,
   NsVector3f         *attach,
   nsfloat            *threshold,
	nsfloat            *contrast,
   nsmodelvertex      *vertex,
	nsfloat            *radius
   )
   {
   nsmodelvertex  src, dest, endpt;
   NsVector3f     V1, V2, A, B, R, T/*, Z*/;
   nsfloat        dts, dotAB, dotAA, dotBB, d, u, v, r1, r2, r, t1, t2, c1, c2, te, ce, dt, ut/*, angle, sine*/;


	endpt = NS_MODEL_VERTEX_NIL;

	t1 = t2 = 0.0f;
	c1 = c2 = 0.0f;

   src  = ns_model_edge_src_vertex( edge );
   dest = ns_model_edge_dest_vertex( edge );

   ns_model_vertex_get_position( src,  &V1 );
   ns_model_vertex_get_position( dest, &V2 );

   r1 = ns_model_vertex_get_radius( src );
   r2 = ns_model_vertex_get_radius( dest );

   if( NULL != threshold )
      {
      t1 = ns_model_vertex_get_threshold( src, NS_MODEL_VERTEX_NEURITE_THRESHOLD );
      t2 = ns_model_vertex_get_threshold( dest, NS_MODEL_VERTEX_NEURITE_THRESHOLD );
      }

   if( NULL != contrast )
      {
      c1 = ns_model_vertex_get_contrast( src, NS_MODEL_VERTEX_NEURITE_CONTRAST );
      c2 = ns_model_vertex_get_contrast( dest, NS_MODEL_VERTEX_NEURITE_CONTRAST );
      }

   ns_vector3f_sub( &A, &V2, &V1 );
   ns_vector3f_sub( &B, P, &V1 );

   /* Avoid divide by zero if magA = 0. Just take the
      larger of the 2 vertices and use that one. */
   if( ns_vector3f_dot( &A, &A )/*ns_vector3f_mag( &A )*/ <= NS_FLOAT_ALMOST_ZERO )
      {
      if( r1 > r2 )
         {
			ns_vector3f_sub( &R, P, &V1 );
			r = r1;

         if( NULL != attach )
            *attach = V1;

         if( NULL != threshold )
            *threshold = t1;

			if( NULL != contrast )
				*contrast = c1;

			endpt = src;
         if( NULL != vertex )
            *vertex = src;
         }
      else
         {
			ns_vector3f_sub( &R, P, &V2 );
			r = r2;

         if( NULL != attach )
            *attach = V2;

         if( NULL != threshold )
            *threshold = t2;

			if( NULL != contrast )
				*contrast = c2;

			endpt = dest;
         if( NULL != vertex )
            *vertex = dest;
         }
      }
	else
		{
		dotAB = ns_vector3f_dot( &A, &B );
		dotAA = ns_vector3f_dot( &A, &A );

		/* If A.B < 0 or A.B > A.A, P doesn't project onto the line,
			so the use the distance to the closest vertex instead. */

		if( dotAB < 0.0f )
			{
			ns_vector3f_sub( &R, P, &V1 );
			r = r1;

			if( NULL != attach )
				*attach = V1;

			if( NULL != threshold )
				*threshold = t1;

			if( NULL != contrast )
				*contrast = c1;

			endpt = src;
			if( NULL != vertex )
				*vertex = src;
			}
		else if( dotAB > dotAA )
			{
			ns_vector3f_sub( &R, P, &V2 );
			r = r2;

			if( NULL != attach )
				*attach = V2;

			if( NULL != threshold )
				*threshold = t2;

			if( NULL != contrast )
				*contrast = c2;

			endpt = dest;
			if( NULL != vertex )
				*vertex = dest;
			}
		else
			{
			dotBB = ns_vector3f_dot( &B, &B );

			dt = dotBB - ( dotAB * dotAB ) / dotAA;
			if( dt < 0.0f )dt = 0.0f;

			/* 'u'[0.0-1.0] holds how far "along" the line 'P' falls and
				'distance' holds how far the point is from the line. */

			d = ns_sqrtf( dt );

			ut = ( dotBB - d * d ) / dotAA;
			if( ut < 0.0f )ut = 0.0f;

			u = ns_sqrtf( ut );

			/* Correct for any slight floating point error. */
			if( u < 0.0f )
				u = 0.0f;
			else if( 1.0f < u )
				u = 1.0f;

			/* However we want the distance to the conical frustum formed
				by the edge not to the medial axis. Therefore we can just
				interpolate the radius and subtract it from the above
				distance value. */

			/* if u = 0.0 then r == r1
				if u = 1.0 then r == r2 */
			r = ( 1.0f - u ) * r1 + u * r2;

			ns_vector3f_cmpd_scale( &A, u );
			ns_vector3f_add( &T, &V1, &A );

			if( NULL != attach )
				*attach = T;

			ns_vector3f_sub( &R, P, &T );

			if( NULL != threshold )
				{
				/* If doing edge thresholds, then use the edge threshold (the one
					at the center of the edge) to interpolate. */
				if( spines->do_edge_thresholds )
					{
					te = ns_model_edge_get_threshold( edge, NS_MODEL_EDGE_NEURITE_THRESHOLD );

					if( u < 0.5f )
						{
						v = u * 2.0f; /* Maps a value in the range [0,0.5] to [0,1]. */
						*threshold = ( 1.0f - v ) * t1 + v * te;
						}
					else
						{
						v = u * 2.0f - 1.0f; /* Maps a value in the range [0.5,1] to [0,1]. */
						*threshold = ( 1.0f - v ) * te + v * t2;
						}
					}
				else
					*threshold = ( 1.0f - u ) * t1 + u * t2;
				}

			if( NULL != contrast )
				{
				if( spines->do_edge_thresholds )
					{
					ce = ns_model_edge_get_contrast( edge, NS_MODEL_EDGE_NEURITE_CONTRAST );

					if( u < 0.5f )
						{
						v = u * 2.0f; /* Maps a value in the range [0,0.5] to [0,1]. */
						*contrast = ( 1.0f - v ) * c1 + v * ce;
						}
					else
						{
						v = u * 2.0f - 1.0f; /* Maps a value in the range [0.5,1] to [0,1]. */
						*contrast = ( 1.0f - v ) * ce + v * c2;
						}
					}
				else
					*contrast = ( 1.0f - u ) * c1 + u * c2;
				}

			/* This point doesnt attach to either end, so set
				this parameter to NULL to indicate that. */
			if( NULL != vertex )
				*vertex = NS_MODEL_VERTEX_NIL;
			}
		}

	/* We now have the vector R which is from P to the attachment point, and
		the radius. Therefore adjust for the Z smear before returing the distance.
		Adjust how much we compensate by the angle of R with the Z-axis. If R is
		parallel with the Z-axis we DONT want to compensate since this will make
		spines parallel with the z-axis too short. */

//	ns_vector3f( &Z, 0.0f, 0.0f, 1.0f ); /* Z-axis vector */

//	R.z = R.z / spines->stretch_factor;
//	angle = ns_vector3f_angle( &R, &Z );
//	R.z = R.z * spines->stretch_factor;

//	sine = ns_sinf( angle );

	/* Correct for any slight floating point error. */
//	if( sine < 0.0f )
//		sine = 0.0f;
//	else if( 1.0f < sine )
//		sine = 1.0f;

	/* NOTE: When angle is zero (parallel with Z-axis) then sin() is 0, so the
		denominator becomes 1, meaning no compensation. When sin() is 1, the z
		component of the R vector if fully compensated. */
//	R.z = R.z / ( 1.0f + ( sine * ( spines->stretch_factor - 1.0f ) ) );

//	if( ns_sqrtf( R.x * R.x + R.y * R.y ) > r )
		R.z = R.z / spines->stretch_factor;

	/* Remember we want the distance to surface, so subtract off the radius. */
	dts = ( ns_vector3f_mag( &R ) - r );

	if( NULL != radius )
		*radius = r;

	/* If the point P attaches to and end point of this edge, then see if it
		will fall within any edges attached to that end point. If so then we
		dont want to use the distance to the sphere at the end point. Return
		"infinity" instead. */
	if( ns_model_vertex_not_equal( endpt, NS_MODEL_VERTEX_NIL ) )
		if( _ns_falls_within_any_edge_zones( endpt, P, edge ) )
			dts = NS_FLOAT_MAX;

	return dts;
   }


NS_PRIVATE NsError _ns_spines_candidate_voxels
   (
   NsSpines            *spines,
   const NsOctreeNode  *octree_node
   )
   {
   const NsAABBox3d   *box;
   NsPoint3d           minVd;
   NsPoint3d           maxVd;
   NsPoint3f           minV;
   NsPoint3f           maxV;
   NsPoint3f           V;
   NsPoint3f           Bf;
   nsulong             intensity;
   nsvectoriter        curr;
   nsvectoriter        end;
   nsmodeledge         edge;
   nsfloat             min_distance;
   nsfloat             distance;
   nsfloat             f_global_threshold;
   nsulong             i_global_threshold;
   nsfloat             f_local_threshold;
   nsulong             i_local_threshold;
	nsfloat             f_local_contrast;
   nsulong             local_threshold;
   nsboolean           do_add;
   NsVoxel            *voxel;
   NsSpineData        *data;
   nsmodelvertex       vertex;
   nsmodelvertex       min_vertex;
   nspointer           attach;
   nsfloat             multiplier;
	nsfloat             radius, radius_at_min_dts;
	const NsVoxelInfo  *voxel_info;
	//NsVector3f          gradient;
	nsfloat             voxel_size_x;
	nsfloat             voxel_size_y;
	nsfloat             voxel_size_z;
	//nsfloat             magnitude;
   NsError             error;
   NsPoint3i           Bi;
   NsPoint3i           I;
   NsPoint3i           minI;
   NsPoint3i           maxI;


	if( ns_progress_cancelled( spines->progress ) )
		return ns_no_error();

   ns_assert( ns_octree_node_is_leaf( octree_node ) );

   if( 0 == ns_octree_node_num_objects( octree_node ) )
      return ns_no_error();

	local_threshold = 0;
	attach          = NULL;

   box = ns_octree_node_aabbox( octree_node );

   multiplier = ns_settings_get_threshold_multiplier( spines->settings );
	voxel_info = ns_settings_voxel_info( spines->settings );

	voxel_size_x = ns_voxel_info_size_x( voxel_info );
	voxel_size_y = ns_voxel_info_size_y( voxel_info );
	voxel_size_z = ns_voxel_info_size_z( voxel_info );

   f_global_threshold = ns_value_get_float( ns_octree_node_value( octree_node, NS_MODEL_OCTREE_THRESHOLD ) );
   i_global_threshold = ( nsulong )f_global_threshold;

   /* NOTE: See nsmodel.h for a description of how the bounding
      box is oriented with respect to the model coordinate system. */
   ns_vector3d_to_3f( ns_aabbox3d_max( box, &maxVd ), &maxV );
   ns_vector3d_to_3f( ns_aabbox3d_min( box, &minVd ), &minV );

   /* NOTE: Clip this octree node's box to the boundaries of the image.
      i.e. > 0 and < x,y,or z dim since the octree bounding box was made
      into a cube and can extend beyond the image boundaries. */
   ns_vector3i(
      &Bi,
      ns_voxel_buffer_width( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_height( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_length( &spines->voxel_buffer ) - 1
      );

   ns_to_voxel_space( &Bi, &Bf, spines->voxel_info );

	/* Clip the bounding box of this node. If both sides of a particular
		dimension fall out of range of the image, then disregard this node. */

	if( ( minV.x < 0.0f && maxV.x < 0.0f ) || ( minV.x > Bf.x && maxV.x > Bf.x ) )
		return ns_no_error();

	if( ( minV.y < 0.0f && maxV.y < 0.0f ) || ( minV.y > Bf.y && maxV.y > Bf.y ) )
		return ns_no_error();

	if( ( minV.z < 0.0f && maxV.z < 0.0f ) || ( minV.z > Bf.z && maxV.z > Bf.z ) )
		return ns_no_error();

	if( minV.x < 0.0f )minV.x = 0.0f;
	if( minV.y < 0.0f )minV.y = 0.0f;
	if( minV.z < 0.0f )minV.z = 0.0f;
	if( maxV.x > Bf.x )maxV.x = Bf.x;
	if( maxV.y > Bf.y )maxV.y = Bf.y;
	if( maxV.z > Bf.z )maxV.z = Bf.z;

   ns_to_image_space( &minV, &minI, spines->voxel_info );
   ns_to_image_space( &maxV, &maxI, spines->voxel_info );

   /* Loop through all the voxels and check distance to all the
      edges that intersect this octree node. */
   for( I.z = minI.z; I.z <= maxI.z; ++I.z )
      for( I.y = minI.y; I.y <= maxI.y; ++I.y )
         for( I.x = minI.x; I.x <= maxI.x; ++I.x )
            {
				/* Make sure this voxel falls within the region of interest. */
				if( ! ns_point3i_inside_cube( &I, spines->roi ) )
					continue;

            intensity = ns_voxel_get( &spines->voxel_buffer, I.x, I.y, I.z );

            /* Only test voxels that meet the threshold. Global means
               global to the current octree node. */
            if( intensity < i_global_threshold )
               continue;

            ns_to_voxel_space( &I, &V, spines->voxel_info );

            do_add            = NS_FALSE;
            min_distance      = NS_FLOAT_MAX;
            min_vertex        = NS_MODEL_VERTEX_NIL;
				radius_at_min_dts = NS_FLOAT_MAX;

            curr = ns_octree_node_begin_objects( octree_node );
            end  = ns_octree_node_end_objects( octree_node );

            for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
               {
               /* Remember the models octree nodes store nsmodeledge iterators. */
               edge = ( nsmodeledge )ns_vector_iter_get_object( curr );

               distance = _ns_spines_distance_to_conical_frustum(
										spines,
                              &V,
                              edge,
                              NULL,
                              &f_local_threshold,
										&f_local_contrast,
                              &vertex,
										&radius
                              );

               i_local_threshold = ( nsulong )
                  ns_model_scale_threshold(
                     f_local_threshold,
                     f_local_contrast,
                     multiplier
                     );

               /* A potential cluster voxel since it is within the proper distance, from
                  this frustum at least, and has a high enough intensity. */
               if( distance <= ns_settings_get_spine_max_voxel_distance( spines->settings ) )
                  {
                  do_add = NS_TRUE;

                  if( distance < min_distance )
                     {
                     min_distance      = distance;
                     local_threshold   = i_local_threshold;
                     min_vertex        = vertex;
                     attach            = edge;
							radius_at_min_dts = radius;
                     }
                  }
               }

            /* NOTE: Avoid duplicates! */
            if( do_add && intensity >= local_threshold )
               if( ! ns_voxel_table_exists( &spines->voxel_table, &I ) )
                  {
                  if( NS_FAILURE( _ns_spine_data_new( &data ), error ) )
                     return error;

                  if( NS_FAILURE( ns_voxel_table_add(
                                    &spines->voxel_table,
                                    &I,
                                    ( nsfloat )intensity,
                                    data,
                                    &voxel
                                    ),
                                    error ) )
                     {
                     ns_delete( data );
                     return error;
                     }

                  __DATA( voxel, spine )     = NULL;
                  __DATA( voxel, distance )  = min_distance;
                  __DATA( voxel, intensity ) = intensity;
						__DATA( voxel, db )        = -1.0f;
						__DATA( voxel, ds )        = -1.0f;
						__DATA( voxel, a )         = 0.0f;
						__DATA( voxel, next )      = NULL;
						//__DATA( voxel, dta )       = min_distance + radius_at_min_dts; /* = distance to axis. */
                  //__DATA( voxel, attach )    = attach;
						//__DATA( voxel, rejected )  = 0;
						//__DATA( voxel, cluster )   = __NO_CLUSTER;
						//__DATA( voxel, is_sink )   = 0;

						//gradient.x = 0.0f;
						//gradient.y = 0.0f;
						//gradient.z = 0.0f;

						//if( 0 < I.x && I.x < Bi.x )
						//	gradient.x =
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x + 1, I.y, I.z ) -
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x - 1, I.y, I.z );

						//if( 0 < I.y && I.y < Bi.y )
						//	gradient.y =
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x, I.y + 1, I.z ) -
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x, I.y - 1, I.z );

						//if( 0 < I.z && I.z < Bi.z )
						//	gradient.z =
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x, I.y, I.z + 1 ) -
						//		( nsfloat )ns_voxel_get( &spines->voxel_buffer, I.x, I.y, I.z - 1 );

						//magnitude = ns_sqrtf( NS_POW2( gradient.x ) + NS_POW2( gradient.y ) + NS_POW2( gradient.z ) );

						//if( 0.0f < magnitude )
						//	{
						//	ns_vector3f_norm( &gradient );

						//	gradient.x *= voxel_size_x;
						//	gradient.y *= voxel_size_y;
						//	gradient.z *= voxel_size_z;
						//	}

						//__DATA( voxel, gradient )  = gradient;
						//__DATA( voxel, magnitude ) = ns_vector3f_mag( &gradient );
						//__DATA( voxel, direction ) = _NS_SPINES_NO_DIR;

                  if( min_distance < 0.0f )
                     __SET( voxel, __MINIMA );

                  if( ns_model_vertex_not_equal( min_vertex, NS_MODEL_VERTEX_NIL ) )
                     if( ns_model_vertex_is_external( min_vertex ) )
                        __SET( voxel, __TIP_ATTACH );
                  }
            }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spines_do_get_candidates
   (
   NsSpines            *spines,
   const NsOctreeNode  *octree_node
   )
   {
   NsError  error;
   nsint    octant;


   if( NULL == octree_node )
      return ns_no_error();

   if( ns_octree_node_is_internal( octree_node ) )
      {
      for( octant = 0; octant < 8; ++octant )
         if( NS_FAILURE( _ns_spines_do_get_candidates(
                           spines,
                           ns_octree_node_child( octree_node, octant )
                           ),
                           error ) )
            return error;

      ns_progress_next_iter( spines->progress );
      }
   else
      {
      if( NS_FAILURE( _ns_spines_candidate_voxels( spines, octree_node ), error ) )
         return error;
      }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spines_get_which_candidates( NsSpines *spines )
   {
   NsError error;

   ns_progress_set_title( spines->progress, "Finding spine voxels..." );
   ns_progress_num_iters( spines->progress, spines->octree_stats.num_internal_nodes );

   ns_progress_begin( spines->progress );

   if( NS_FAILURE( _ns_spines_do_get_candidates(
                     spines,
                     ns_model_octree_root( spines->model, NS_MODEL_EDGE_OCTREE )
                     ),
                     error ) )
      return error;

   ns_progress_end( spines->progress );

   /*TEMP*/ns_println(
      "\n# of voxels in spines->voxel_table = " NS_FMT_ULONG,
      ns_voxel_table_size( &spines->voxel_table )
      );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spines_get_candidates( NsSpines *spines )
   {
   NsError error;

   ns_model_octree_calc_thresholds_and_contrasts(
      spines->model,
		spines->settings,
      NS_MODEL_EDGE_OCTREE,
		spines->do_edge_thresholds
      );

   if( NS_FAILURE( _ns_spines_get_which_candidates( spines ), error ) )
      return error;

   return ns_no_error();
   }


NS_PRIVATE void _ns_spines_classify_candidates( NsSpines *spines )
   {
   NsVoxel   N;
   NsVoxel  *voxel;
   NsVoxel  *neighbor;
   nssize    i;
	nsint     length;


	length = ( nsint )ns_image_length( spines->image );

   ns_println( "" );

   ns_progress_set_title( spines->progress, "Finding maxima..." );
   ns_progress_begin( spines->progress );

	/* NOTE: Dont allow any maxima that are on the edges (in Z).
		Have to allow for 2D spine detection though. */
   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      if( ! __GET( voxel, __MINIMA ) )
			if( 1 == length || ( 0 < voxel->position.z && voxel->position.z < length - 1 ) )
				{
				/* Dont check any voxels that are on the outer edges of the image. */
				__SET( voxel, __MAXIMA );

				/* Find which voxels are maxima, i.e. have no neighbors closer
					then themselves to the model. */
				for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
					{
					ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						if( __DATA( voxel, distance ) < __DATA( neighbor, distance ) )
							{
							__CLEAR( voxel, __MAXIMA );
							break;
							}
					}
				}

   spines->num_maxima = 0;

   /* Clear maxima that attach to an external vertex,
      i.e. a tip of a branch since these have too many
      artifcats and are probably not spines anyway. */
   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      {
      if( __GET( voxel, __MAXIMA ) )
         if( __GET( voxel, __TIP_ATTACH ) )
            __CLEAR( voxel, __MAXIMA );

      if( __GET( voxel, __MAXIMA ) )
         ++(spines->num_maxima);
      }

	/*TEMP*/ns_println( "# of maxima = " NS_FMT_ULONG, spines->num_maxima );
   /*TEMP*/ns_println( "" );

   ns_progress_end( spines->progress );
   }
