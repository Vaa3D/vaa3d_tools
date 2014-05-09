/*
NS_PRIVATE void _ns_spines_calc_event_bounding_box( NsSpines *spines, NsSpineEvent *event )
	{
   nslistiter           curr_layer, end_layers;
   const NsSpineLayer  *layer;
	nsboolean            found_one;


	NS_USE_VARIABLE( spines );

   ns_model_init_voxel_bounds( &event->min, &event->max );

	found_one  = NS_FALSE;
   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
      {
      layer = ns_list_iter_get_object( curr_layer );

      if( layer->calc_width )
			{
			found_one = NS_TRUE;

			if( layer->min.x < event->min.x )event->min.x = layer->min.x;
			if( layer->min.y < event->min.y )event->min.y = layer->min.y;
			if( layer->min.z < event->min.z )event->min.z = layer->min.z;

			if( layer->max.x > event->max.x )event->max.x = layer->max.x;
			if( layer->max.y > event->max.y )event->max.y = layer->max.y;
			if( layer->max.z > event->max.z )event->max.z = layer->max.z;
			}
      }

	event->can_threshold = found_one;
	}*/

/*
NS_PRIVATE NsError _ns_spines_calc_event_threshold_and_contrast( NsSpines *spines, NsSpineEvent *event )
	{
   ThresholdStruct     s;
	const NsVoxelInfo  *voxel_info;
   NsVector3f          Pf;
	NsVector3i          Pi;
	nsfloat             event_width, event_height, event_length;
   nssize              sample_width, sample_height, sample_length;
   nssize              num_values;
   nsfloat            *values;
   nssize              bytes;
   NsVector3i          C1, C2;
	NsAABBox3d          event_box;
   NsError             error;

	
	voxel_info = ns_settings_voxel_info( spines->settings );

	if( ns_settings_get_threshold_use_fixed( spines->settings ) )
		{
		event->threshold = ns_settings_get_threshold_fixed_value( spines->settings );
		event->contrast  = ns_settings_get_threshold_fixed_value( spines->settings );
		}
	else
		{
		event_width  = event->max.x - event->min.x;
		event_height = event->max.y - event->min.y;
		event_length = event->max.z - event->min.z;

		/* Compute the threshold at the center of the event's bounding box. *//*
		Pf.x = event->min.x + event_width  / 2.0f;
		Pf.y = event->min.y + event_height / 2.0f;
		Pf.z = event->min.z + event_length / 2.0f;

		ns_to_image_space( &Pf, &Pi, voxel_info );

		/* Check for out of bounds. *//*
		if( Pi.x < 0 || Pi.x >= spines->width  ||
			 Pi.y < 0 || Pi.y >= spines->height ||
			 Pi.z < 0 || Pi.z >= spines->length )
			{
			/* Set the threshold so "high" that no voxel intensity would never be greater,
				and set the contrast really low? *//*
			event->threshold = NS_FLOAT_MAX;
			event->contrast  = 0.0f;

			goto _NS_SPINES_CALC_EVENT_TC_EXIT;
			}

		ns_grafting_calc_aabbox_ex(
			voxel_info,
			event_width / 2.0f,
			event_height / 2.0f,
			event_length / 2.0f,
			&Pf,
			&spines->buffer_max,
			spines->use_2d_sampling,
			1.0f,
			spines->min_window,
			&C1,
			&C2
			);

		if( NULL != spines->roi )
			{
			ns_model_voxel_bounds_to_aabbox_ex( &C1, &C2, voxel_info, &event_box );

			if( ! ns_aabbox3d_intersects_aabbox( &event_box, &spines->roi_box ) )
				goto _NS_SPINES_CALC_EVENT_TC_EXIT;
			}

		sample_width  = ( nssize )( C2.x - C1.x ) + 1;
		sample_height = ( nssize )( C2.y - C1.y ) + 1;
		sample_length = ( nssize )( C2.z - C1.z ) + 1;

		num_values = sample_width * sample_height * sample_length;

		ns_assert( 0 < num_values );
		bytes = num_values * sizeof( nsfloat );

		if( ns_byte_array_size( &spines->samples ) < bytes )
			if( NS_FAILURE( ns_byte_array_resize( &spines->samples, bytes ), error ) )
				return error;

		values = ( nsfloat* )ns_byte_array_begin( &spines->samples );

		ns_grafting_sample_values( &spines->voxel_buffer, values, num_values, &C1, &C2 );

		ComputeThreshold(
			values,
			( nsint )num_values,
			&s,
			ns_voxel_buffer_dynamic_range( &spines->voxel_buffer ),
			NS_TRUE
			);

		event->threshold = s.threshold;
		event->contrast  = s.contrast;
		}

	_NS_SPINES_CALC_EVENT_TC_EXIT:
	return ns_no_error();
	}
*/

#define _NS_SPINES_MIN_RAYBURST_WIDTH\
   ( ns_voxel_info_size_x( ns_settings_voxel_info( spines->settings ) ) / 10.0f )

#define _NS_SPINES_MIN_SPREAD_VALUE  _NS_SPINES_MIN_RAYBURST_WIDTH


NS_PRIVATE NsError _ns_spines_calc_event_widths( NsSpines *spines, NsSpineEvent *event )
   {
   nslistiter     curr_layer, end_layers;
   NsSpineLayer  *layer;
	//NsError        error;


	event->can_threshold = NS_FALSE;
	/*_ns_spines_calc_event_bounding_box( spines, event );

	if( event->can_threshold )
		if( NS_FAILURE( _ns_spines_calc_event_threshold_and_contrast( spines, event ), error ) )
			return error;*/

   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
      {
      layer = ns_list_iter_get_object( curr_layer );

      if( layer->calc_width )
         {
         _ns_spine_layer_calc_weighted_center( layer, spines->voxel_info, __INTENSITY_POWER );

         _ns_spine_layer_calc_rayburst_width(
				layer,
				spines,
				event->can_threshold ? event : NULL
				);

         if( layer->width < _NS_SPINES_MIN_RAYBURST_WIDTH )
            layer->width = _NS_SPINES_MIN_RAYBURST_WIDTH;
         }
		else
			layer->width = ( nsfloat )layer->max_spread;

		/* This assertion should hold, BUT compiled with cl.exe version 14.00.50727.42
			on or after 2007-06-12 it fails due to floating point error. */
		/* ns_assert( _NS_SPINES_MIN_RAYBURST_WIDTH <= layer->width ); */
      }

	return ns_no_error();
   }


NS_PRIVATE void _ns_spines_event_calc_ratios( NsSpines *spines, NsSpineEvent *event )
	{
/* This commented out version computes count and spread ratio from layer to layer, but
	it has some problems...

   nslistiter     curr, end;
   NsSpineLayer  *curr_layer, *prev_layer;


	NS_USE_VARIABLE( spines );
	ns_assert( ! ns_list_is_empty( &event->layers ) );

   curr = ns_list_begin( &event->layers );
   end  = ns_list_end( &event->layers );

	prev_layer = ns_list_iter_get_object( curr );

	prev_layer->count_ratio  = 0.0;
	prev_layer->spread_ratio = 0.0;

	curr = ns_list_iter_next( curr );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      curr_layer = ns_list_iter_get_object( curr );

		curr_layer->count_ratio =
			( nsdouble )ns_list_size( &curr_layer->voxels ) / ( nsdouble )ns_list_size( &prev_layer->voxels );

		curr_layer->spread_ratio = ( nsdouble )curr_layer->max_spread / ( nsdouble )prev_layer->max_spread;

		prev_layer = curr_layer;
      }
*/
   nslistiter     curr_layer, end_layers;
   NsSpineLayer  *layer;
	nssize         curr_count;
	nssize         sum_count;
	nsdouble       sum_spread;
	nsdouble       layer_count;


	NS_USE_VARIABLE( spines );

	sum_count  = 0;
	sum_spread = 0.0;

   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

	layer = ns_list_iter_get_object( curr_layer );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
      {
      layer = ns_list_iter_get_object( curr_layer );

		curr_count = ns_list_size( &layer->voxels );

		sum_count  += curr_count;
		sum_spread += layer->max_spread;

		layer_count = ( nsdouble )layer->index + 1.0;

		layer->count_ratio  = ( ( nsdouble )curr_count / ( nsdouble )sum_count ) * layer_count;
		layer->spread_ratio = ( layer->max_spread / sum_spread ) * layer_count;
      }
	}


NS_PRIVATE nsint _ns_spines_convert_type( nsint type )
	{
   switch( type )
      {
      case SPINE_TYPE_INVALID:
         type = NS_SPINE_INVALID;
         break;

      case SPINE_TYPE_STUBBY:
         type = NS_SPINE_STUBBY;
         break;

      case SPINE_TYPE_MUSHROOM:
         type = NS_SPINE_MUSHROOM;
         break;

      case SPINE_TYPE_THIN:
         type = NS_SPINE_THIN;
         break;

      case SPINE_TYPE_OTHER:
         type = NS_SPINE_OTHER;
         break;

      default:
         ns_assert_not_reached();
      }

	return type;
	}

/*
NS_PRIVATE void _ns_spines_event_adjust_low_neighbor_voxels( NsSpineEvent *event, NsSpines *spines )
	{
   nslistiter     curr_layer, end_layers;
	nslistiter     curr_voxel, next_voxel, end_voxels;
	NsVoxel       *voxel;
   NsSpineLayer  *layer;
	NsVector3f     V;
	nsboolean      recompute;


   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
      {
		recompute = NS_FALSE;
      layer     = ns_list_iter_get_object( curr_layer );

		_ns_spine_layer_voxels_set_neighbors_count( layer, spines );
		_ns_spine_layer_calc_average_neighbor_count( layer );

      curr_voxel = ns_list_begin( &layer->voxels );
      end_voxels = ns_list_end( &layer->voxels );

      while( ns_list_iter_not_equal( curr_voxel, end_voxels ) )
         {
			next_voxel = ns_list_iter_next( curr_voxel );
         voxel      = ns_list_iter_get_object( curr_voxel );

			if( ( ( nsfloat )__DATA( voxel, num_neighbors ) ) < ( layer->avg_num_neighbors / 2.0f )
					&&
				 ! __GET( voxel, __MAXIMA ) )
				{
				__SET( voxel, __OUTLIER );
				recompute = NS_TRUE;
				}

			curr_voxel = next_voxel;
         }

//ns_print( "old_spread = %f", _ns_spine_layer_max_spread( layer, spines ) );

		if( recompute )
			{
			ns_model_init_voxel_bounds( &layer->min, &layer->max );

			curr_voxel = ns_list_begin( &layer->voxels );
			end_voxels = ns_list_end( &layer->voxels );

			for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
					curr_voxel = ns_list_iter_next( curr_voxel ) )
				{
				voxel = ns_list_iter_get_object( curr_voxel );

				if( ! __GET( voxel, __OUTLIER ) )
					{
					ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
					ns_model_update_voxel_bounds( &layer->min, &layer->max, &V, 0.0f );
					}
				}
			}

//ns_println( "  new_spread = %f", _ns_spine_layer_max_spread( layer, spines ) );
		}
	}
*/

NS_PRIVATE void _ns_spines_output_profile( NsSpines *spines, NsSpineEvent *event, nsint type )
	{
   nslistiter     curr_layer;
   nslistiter     end_layers;
   NsSpineLayer  *layer;


	ns_assert( type < ( nsint )NS_SPINE_NUM_TYPES );

   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
			 curr_layer = ns_list_iter_next( curr_layer ) )
      {
      layer = ns_list_iter_get_object( curr_layer );

		/* NOTE: Keep the width value within reason so that graphing the profiles is easier. */
		if( layer->width >= NS_FLOAT_MAX )
			ns_file_print( spines->files + type, NS_FMT_DOUBLE " ", ns_voxel_info_max_size( spines->voxel_info ) * 100.0f );
		else
			ns_file_print( spines->files + type, NS_FMT_DOUBLE " ", layer->width );
		}

	ns_file_print_newline( spines->files + type );
	}


NS_PRIVATE void _ns_spine_event_calc_head_and_neck_center
	(
	NsSpineEvent  *event,
	nssize         base_layer,
	nssize         num_layers,
	LAYERDESC     *layer_desc,
	NsSpines      *spines
	)
	{
	nssize         head_layer;
	nssize         neck_layer;
   NsSpineLayer  *layer;
	NsVector3f     attach;
	nsfloat        distance;
	NsVector3f     direction;


	head_layer = ( nssize )event->stats.head_layer;
	ns_assert( head_layer < num_layers );

	event->has_head_center = NS_FALSE;

	if( head_layer <= base_layer )
		{
		layer = ns_list_iter_get_object( ( nslistiter )layer_desc[ head_layer ].user_data );

		if( 0 < ns_list_size( &layer->voxels ) ) 
			{
			event->head_center     = layer->center;
			event->has_head_center = NS_TRUE;
			}
		}

	//if( event->has_head_center )
	//	ns_println( "HEAD-CENTER = %f,%f,%f", event->head_center.x, event->head_center.y, event->head_center.z );
	//else
	//	ns_println( "N/A" );

	neck_layer = ( nssize )event->stats.neck_layer;
	ns_assert( neck_layer < num_layers );

	event->has_neck_center = NS_FALSE;

	if( event->stats.has_neck )
		if( neck_layer <= base_layer )
			{
			layer = ns_list_iter_get_object( ( nslistiter )layer_desc[ neck_layer ].user_data );

			if( 0 < ns_list_size( &layer->voxels ) ) 
				{
				event->neck_center     = layer->center;
				event->has_neck_center = NS_TRUE;
				}
			else
				{
				/* Get the center of mass of the previous layer. Project this point
					halfway to the surface along a direction vector between itself
					and its attachment point. Set this as the neck center. */
				if( 0 < neck_layer )
					{
					--neck_layer;
					layer = ns_list_iter_get_object( ( nslistiter )layer_desc[ neck_layer ].user_data );

					if( 0 < ns_list_size( &layer->voxels ) )
						if( _ns_spines_calc_attach_and_distance_at( spines, &layer->center, &attach, &distance ) )
							{
							ns_vector3f_sub( &direction, &attach, &layer->center );
							ns_vector3f_norm( &direction );

							/* NOTE: distance is greater than or equal to the layers floor, so we just
								add the difference on to get a more accurate center. */
							ns_vector3f_cmpd_scale(
								&direction,
								( distance - layer->min_distance ) + ( layer->min_distance / 2.0f )
								);

							ns_vector3f_add( &event->neck_center, &layer->center, &direction );
							event->has_neck_center = NS_TRUE;
							}
					}
				}
			}

	//if( event->has_neck_center )
	//	ns_println( "NECK-CENTER = %f,%f,%f", event->neck_center.x, event->neck_center.y, event->neck_center.z );
	//else
	//	ns_println( "N/A" );
	}
 

NS_PRIVATE NsSpineType _ns_spines_analyze_event
   (
   NsSpines      *spines,
   NsSpineEvent  *event
   )
   {
   nssize         i;
   nssize         num_layers;
	nslistiter     first_layer;
   nslistiter     curr_layer;
   nslistiter     end_layers;
   NsSpineLayer  *layer;
   LAYERDESC     *layer_desc;
	NsSpineLayer  *prev;
   nssize         base;
   NsSpineType    spine_type;


	spine_type = NS_SPINE_INVALID;

   num_layers = ns_list_size( &event->layers );
   ns_assert( 1 <= num_layers );

   if( NULL != ( layer_desc = ns_new_array( LAYERDESC, num_layers ) ) )
      {
		_ns_spines_event_calc_ratios( spines, event );

      i          = 0;
      curr_layer = ns_list_begin( &event->layers );
      end_layers = ns_list_end( &event->layers );

		/* Check that the first layer has at least the origin voxel. */
		layer = ns_list_iter_get_object( curr_layer );
		ns_assert( 1 <= ns_list_size( &layer->voxels ) );

      for( ; ns_list_iter_not_equal( curr_layer, end_layers );
             ++i, curr_layer = ns_list_iter_next( curr_layer ) )
         {
         layer = ns_list_iter_get_object( curr_layer );

			layer_desc[i].spread       = layer->max_spread;
			layer_desc[i].depth        = layer->height;
			layer_desc[i].diameter     = layer->width;
			layer_desc[i].voxel_count  = ns_list_size( &layer->voxels );
			layer_desc[i].user_data    = curr_layer;
			layer_desc[i].count_ratio  = layer->count_ratio;
			layer_desc[i].spread_ratio = layer->spread_ratio;

			/* Adjust depth to compensate for the fact that we corrected
				the smear in the Z axis. */
			layer_desc[i].depth *= event->correction_factor;
         }

      ns_assert( i == num_layers );

		GetSpineStats( layer_desc, ( nsint )num_layers, &event->stats );
		ns_assert( 0 <= event->stats.base_id );

      /* NOTE: Dont bother eliminating layers if the whole
         thing is invalid anyway. */
      if( SPINE_TYPE_INVALID != event->stats.spine_type )
         {
			/* NOTE: If using a custom classifier then the type should
				always be set to "other". See also <ext/spinetype.inl>. */
			if( NULL == ns_spines_classifier_get() )
				SetSpineType( &event->stats );
			else{ ns_assert( SPINE_TYPE_OTHER == event->stats.spine_type ); }

			//if( event->stats.large_base )
			//	event->stats.spine_type = SPINE_TYPE_OTHER;

			/* NOTE: Output layers before removal of layers after the base. */
			if( spines->do_file )
				_ns_spines_output_profile( spines, event, _ns_spines_convert_type( event->stats.spine_type ) );

         base = ( nssize )event->stats.base_id;
         ns_assert( base < num_layers );

			_ns_spine_event_calc_head_and_neck_center( event, base, num_layers, layer_desc, spines );

         if( base != num_layers - 1 )
            {
            //ns_print( "[%lu,%lu]", base + 1, num_layers - 1 );

            /* Eliminate all layers AFTER the base layer. */
            if( spines->remove_base_layers )
               ns_list_erase_all(
                  &event->layers,
                  ns_list_iter_next( ( nslistiter )layer_desc[ base ].user_data ),
                  ns_list_end( &event->layers )
                  );
            }

         /* _ns_spines_event_set_voxels_layer( event ); */




			curr_layer = ns_list_rev_begin( &event->layers );

			/*curr_layer =*/ first_layer = ns_list_begin( &event->layers );
			//end_layers = ns_list_end( &event->layers );

			//for( ; ns_list_iter_not_equal( curr_layer, end_layers ); curr_layer = ns_list_iter_next( curr_layer ) )
				//{
				layer = ns_list_iter_get_object( curr_layer );

				//if( _ns_spine_voxels_max_angle( spines, &layer->center, &layer->voxels ) >= 70.0 )
				//	event->stats.spine_type = SPINE_TYPE_OTHER;

				if( ns_list_iter_not_equal( curr_layer, first_layer ) )
					prev = ns_list_iter_get_object( ns_list_iter_prev( curr_layer ) );
				else
					prev = layer;

				_ns_spine_voxels_set_high_angled( spines, &prev->center, &layer->voxels, 45.0 );
				_ns_spine_layer_remove_high_angled( layer );
				//}

         }

      ns_free( layer_desc );
		spine_type = _ns_spines_convert_type( event->stats.spine_type );
      }

   return spine_type;
   }


NS_PRIVATE void _ns_spine_event_shell_voxel_finalize( NsVoxel *voxel )
   {
   ns_assert( __GET( voxel, __SHELLED ) );
   __CLEAR( voxel, __SHELLED );
   }


NS_PRIVATE void _ns_spine_event_construct( NsSpineEvent *event )
   {
   event->origin = NULL;

   ns_list_construct( &event->layers, _ns_spine_layer_delete );

   event->curr_shell = &event->_curr_shell;
   event->next_shell = &event->_next_shell;

   ns_list_construct( event->curr_shell, _ns_spine_event_shell_voxel_finalize );
   ns_list_construct( event->next_shell, _ns_spine_event_shell_voxel_finalize );
   }


NS_PRIVATE void _ns_spine_event_clear( NsSpineEvent *event )
   {
   event->origin = NULL;

   ns_list_clear( &event->layers );
   ns_list_clear( event->curr_shell );
   ns_list_clear( event->next_shell );
   }


NS_PRIVATE void _ns_spine_event_destruct( NsSpineEvent *event )
   {
   ns_list_destruct( &event->layers );
   ns_list_destruct( event->curr_shell );
   ns_list_destruct( event->next_shell );
   }


NS_PRIVATE nssize _ns_spine_event_num_voxels( const NsSpineEvent *event )
   {
   nslistiter           curr, end;
   const NsSpineLayer  *layer;
   nssize               count;


   count = 0;
   curr  = ns_list_begin( &event->layers );
   end   = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      layer  = ns_list_iter_get_object( curr );
      count += ns_list_size( &layer->voxels ); 
      }

   return count;
   }


NS_PRIVATE NsError _ns_spine_event_add_voxel
   (
   NsSpineEvent  *event,
   NsVoxel       *voxel,
   NsList        *which_shell
   )
   {
   NsError error;

   ns_assert(
      which_shell == event->curr_shell ||
      which_shell == event->next_shell
      );

   ns_assert( ! __GET( voxel, __SHELLED ) );
   __SET( voxel, __SHELLED );

   if( NS_FAILURE( ns_list_push_back( which_shell, voxel ), error ) )
      return error;

   return ns_no_error();
   }


NS_PRIVATE void _ns_spine_event_establish_layer_limits
   (
	const NsSpines      *spines,
   const NsSpineEvent  *event,
   NsList              *which_shell,
	nsfloat             *ret_min_distance/*,
	nsfloat             *ret_max_advance*/
   )
   {
   nslistiter   curr_voxel;
   nslistiter   end_voxels;
   NsVoxel     *voxel;
   nsfloat      min_distance;
	//nsfloat      advance;
	//nsfloat      max_advance;
	//NsVector3f   V;
	//NsVector3f   B;


   ns_assert(
      /*which_shell == event->curr_shell ||*/
      which_shell == event->next_shell
      );

   ns_assert( ! ns_list_is_empty( which_shell ) );

   min_distance = NS_FLOAT_MAX;
	//max_advance  = -NS_FLOAT_MAX;

   curr_voxel = ns_list_begin( which_shell );
   end_voxels = ns_list_end( which_shell );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
         curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

      if( __DATA( voxel, distance ) < min_distance )
         min_distance = __DATA( voxel, distance );

		//ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
		//ns_vector3f_sub( &B, &V, &event->O ); /* B = V - O. */

		//advance = ns_vector3f_dot( &event->A, &B );

		//if( advance > max_advance )
		//	max_advance = advance;
      }

   *ret_min_distance = min_distance - ns_voxel_info_min_size( spines->voxel_info ) / 20.0f;
	//*ret_max_advance  = max_advance;
   }

/*
enum{ _NS_SPINES_GRADIENT_IN, _NS_SPINES_GRADIENT_OUT, _NS_SPINES_GRADIENT_PARALLEL };

NS_PRIVATE nsint _ns_spine_voxel_calc_gradient_direction
	(
	const NsVoxel   *voxel,
	const NsLine3f  *L,
	const NsSpines  *spines
	)
	{
	NsVector3f  V1, V2, X2mX1;
	nsfloat     d1, d2, dd, mag, dm, length;
	nsint       direction;


	ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );
	ns_vector3f_add( &V2, &V1, &( __DATA( voxel, gradient ) ) );

	/* If line endpoints are the same, calculate point to point distance
		instead of point to line distance. *//*

	ns_vector3f_sub( &X2mX1, &L->P2, &L->P1 );
	length = ns_vector3f_mag( &X2mX1 );

	if( length <= NS_FLOAT_ALMOST_ZERO )
		{
		/* Can use either end of line since they're the same. *//*
		d1 = ns_point3f_distance_to_point( &V1, &L->P1 );
		d2 = ns_point3f_distance_to_point( &V2, &L->P1 );
		}
	else
		{
		d1 = ns_point3f_distance_to_line( &V1, L );
		d2 = ns_point3f_distance_to_line( &V2, L );
		}

	dd  = d2 - d1;
	mag = __DATA( voxel, magnitude );
	dm  = 0.0f < mag ? dd / mag : 0.0f;

	if( dm < -spines->parallel_range )
		direction = _NS_SPINES_GRADIENT_IN;
	else if( dm > spines->parallel_range )
		direction = _NS_SPINES_GRADIENT_OUT;
	else
		direction = _NS_SPINES_GRADIENT_PARALLEL;

	return direction;
	}*/

/*
#define _NS_SPINE_LAYER_FIRST_SHELL_CENTER( layer )\
	( &(layer)->L.P1 )

#define _NS_SPINE_LAYER_FIRST_SHELL_ATTACH( layer )\
	( &(layer)->L.P2 )


NS_PRIVATE void _ns_spine_event_shell_calc_center
   (
   NsSpineEvent        *event,
   NsList              *which_shell,
	const NsSpines      *spines,
   nsfloat              power,
	const NsSpineLayer  *prev_layer,
	NsVector3f          *curr_center
   )
   {
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   const NsVoxel  *voxel;
   nsfloat         curr_intensity;
   nsfloat         sum_intensity;
   NsVector3f      V;


	/* Use the ones that pointed in to calculate a center of mass.
		The center is a WEIGHTED average on the x,y,z of the voxels. *//*
   sum_intensity = 0.0f;
   ns_vector3f_zero( curr_center );

   curr_voxel = ns_list_begin( which_shell );
   end_voxels = ns_list_end( which_shell );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

		//if( _NS_SPINES_DIR_IN == __DATA( voxel, direction ) )
			{
			curr_intensity = ns_powf( voxel->intensity, power );

			sum_intensity += curr_intensity;

			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

			ns_vector3f_cmpd_scale( &V, curr_intensity );
			ns_vector3f_cmpd_add( curr_center, &V );
			}

		__DATA( voxel, direction ) = _NS_SPINES_NO_DIR;
      }

	/* NOTE: If there arent any voxels pointing "in" then just set the
		center of mass to the previous one, that is if there is one. *//*
	if( 0.0f < sum_intensity )
		ns_vector3f_cmpd_scale( curr_center, 1.0f / sum_intensity );
	else
		*curr_center =
			NULL != prev_layer ?
				( *( _NS_SPINE_LAYER_FIRST_SHELL_CENTER( prev_layer ) ) ) : event->O;
   }
*/

/*
NS_PRIVATE nsfloat _ns_spine_voxel_distance_to_first_shell_center
	(
	const NsVoxel       *voxel,
	const NsSpineLayer  *layer,
	const NsSpines      *spines
	)
	{
	NsVector3f V;

	ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
	return ns_vector3f_distance( &V, _NS_SPINE_LAYER_FIRST_SHELL_CENTER( layer ) );
	}
*/


//NS_PRIVATE NsError _ns_spine_event_on_each_first_shell
//	(
//	NsSpineEvent        *event,
//	NsSpineLayer        *curr_layer,
//	NsSpines            *spines/*,
//	const NsSpineLayer  *prev_layer,
//	nsboolean           *enough_definites,	
//	nsfloat             *average_distance*/
//	)
//	{
	//nslistiter   curr, end;
	//nsfloat      sum;
	//nssize       count;
	//nssize       num_definites;
	//NsVoxel     *voxel;


//	ns_assert( NULL != curr_layer );

	//*enough_definites = NS_FALSE;

//	_ns_spine_event_establish_layer_limits(
//		spines,
//		event,
//		event->next_shell,
//		&curr_layer->min_distance,
//		&curr_layer->max_advance
//		);

	//_ns_spine_event_shell_calc_center(
	//	event,
	//	event->next_shell,
	//	spines,
	//	__INTENSITY_POWER,
	//	prev_layer,
	//	_NS_SPINE_LAYER_FIRST_SHELL_CENTER( curr_layer )
	//	);

	//if( ! _ns_spines_calc_attach_at(
	//		spines,
	//		_NS_SPINE_LAYER_FIRST_SHELL_CENTER( curr_layer ),
	//		_NS_SPINE_LAYER_FIRST_SHELL_ATTACH( curr_layer ),
	//		_ns_spine_voxel_octree_func_special
	//		) )
	//	{
	//	/*TEMP*/ns_println( "The center of mass of layer %d fell outside the octree!", ( nsint )curr_layer->index );
	//	return ns_error_dom( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	//	}

	/* On first shell find if anyone is definitely included. Of the ones that
		are, compute distance to the center of mass and in the end compute the
		average of these distances. */
	//sum   = 0.0f;
	//count = 0;

	//num_definites = 0;

   //curr = ns_list_begin( event->next_shell );
   //end  = ns_list_end( event->next_shell );

   //for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
     // {
      //voxel = ns_list_iter_get_object( curr );

		//if( _NS_SPINES_GRADIENT_IN == _ns_spine_voxel_calc_gradient_direction( voxel, &curr_layer->L, spines ) )
		//	++num_definites;

		/* NOTE: Use all voxel to compute the average distance. */
		//sum += _ns_spine_voxel_distance_to_first_shell_center( voxel, curr_layer, spines );
		//++count;
		//}

	//*average_distance = 0 < count ? sum / ( nsfloat )count : 0.0f;

	//if( ( ( nsfloat )num_definites / ( nsfloat )ns_list_size( event->next_shell ) ) >= 0.75f )
	//	*enough_definites = NS_TRUE;

//	return ns_no_error();
//	}


#define _ns_spine_layer_floor_is_established( layer )\
	( -NS_FLOAT_MAX < (layer)->min_distance )

#define _ns_spine_layer_floor_not_established( layer )\
	( ! _ns_spine_layer_floor_is_established( (layer) ) )
/* OR could use ( -NS_FLOAT_MAX == (layer)->min_distance )? */


/*
NS_PRIVATE NsError _ns_spines_create_dendrite_event
   (
   NsSpines      *spines,
   NsVoxel       *origin,
   NsSpineEvent  *event
   )
   {
   NsVoxel        N;
   nsint          iteration;
   NsSpineLayer  *prev_layer;
   NsSpineLayer  *curr_layer;
   nslistiter     curr;
	nslistiter     next;
   nslistiter     end;
   nssize         i;
   NsVoxel       *voxel;
   NsVoxel       *neighbor;
	nsfloat        max_layer_spread;
   NsError        error;


   error = ns_no_error();

	max_layer_spread = spines->max_declumping_spread;//ns_settings_get_spine_max_layer_spread( spines->settings );

   ns_assert( ns_list_is_empty( &event->layers ) );
   ns_assert( ns_list_is_empty( event->curr_shell ) );
   ns_assert( ns_list_is_empty( event->next_shell ) );

   event->origin = origin;
	ns_to_voxel_space( &origin->position, &event->O, spines->voxel_info );

   iteration = 0;

   prev_layer = NULL;

   NS_INFINITE_LOOP
      {
      /* Create the next layer and set its floor to the most negative integer
         so that all voxels of the first shell will go into the layer. *//*

      if( NS_FAILURE( _ns_spine_layer_new( &curr_layer, iteration ), error ) )
         goto _NS_SPINES_CREATE_EVENT_EXIT;

      if( NS_FAILURE( ns_list_push_back( &event->layers, curr_layer ), error ) )
         {
         _ns_spine_layer_delete( curr_layer );
         goto _NS_SPINES_CREATE_EVENT_EXIT;
         }

		/* Must set these first for every new layer! *//*
      curr_layer->min_distance = -NS_FLOAT_MAX;

      ++iteration;

      ns_assert( ns_list_is_empty( event->curr_shell ) );
      ns_assert( ns_list_is_empty( event->next_shell ) );

      /* If this is the first layer then add the origin to the layer AND
         the first shell. Otherwise add all voxels of the previous layer
         to the first shell. *//*

      if( NULL == prev_layer )
         {
         if( NS_FAILURE( _ns_spine_event_add_voxel(
                           event,
                           event->origin,
                           event->curr_shell
                           ),
                           error ) )
            goto _NS_SPINES_CREATE_EVENT_EXIT;

         if( NS_FAILURE( _ns_spine_layer_add_voxel(
                           curr_layer,
                           event->origin,
                           spines->voxel_info
                           ),
                           error ) )
            goto _NS_SPINES_CREATE_EVENT_EXIT;
         }
      else
         {
         curr = ns_list_begin( &prev_layer->voxels );
         end  = ns_list_end( &prev_layer->voxels );

         for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
            {
            voxel = ns_list_iter_get_object( curr );

            if( NS_FAILURE( _ns_spine_event_add_voxel( event, voxel, event->curr_shell ), error ) )
               goto _NS_SPINES_CREATE_EVENT_EXIT;
            }
         }

      NS_INFINITE_LOOP
         {
         //ns_assert( ! ns_list_is_empty( event->curr_shell ) );

         curr = ns_list_begin( event->curr_shell );
         end  = ns_list_end( event->curr_shell );

         for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
            {
            voxel = ns_list_iter_get_object( curr );

            for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
               {
               ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

               /* NOTE: Neighbor voxels are not added if they are less then the floor
                  established for the current layer. For the first shell the floor is
                  set to the maximum negative integer so all voxels should pass. *//*

               if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
                  if( ! __GET( neighbor, __LAYERED ) && ! __GET( neighbor, __SHELLED ) )
                     if( __DATA( neighbor, distance ) >= curr_layer->min_distance )
								if( __DATA( neighbor, clump_id ) < 2 ) /* 2 means already clumped, but not part of dendrite clump. *//*
									if( NS_FAILURE(
											_ns_spine_event_add_voxel(
												event,
												neighbor,
												event->next_shell
												),
											error ) )
										goto _NS_SPINES_CREATE_EVENT_EXIT;
               }
            }

         ns_list_clear( event->curr_shell );

         /* Did we run of voxels? If so we're currently done in this
            inner "shell building" loop. *//*
         if( ns_list_is_empty( event->next_shell ) )
            break;

			/* Establish the floor if this is the first shell. *//*
         if( _ns_spine_layer_floor_not_established( curr_layer ) )
				_ns_spine_event_establish_layer_limits(
					spines,
					event,
					event->next_shell,
					&curr_layer->min_distance
					);

         curr = ns_list_begin( event->next_shell );
         end  = ns_list_end( event->next_shell );

         while( ns_list_iter_not_equal( curr, end ) )
            {
				next  = ns_list_iter_next( curr );
            voxel = ns_list_iter_get_object( curr );

				if( NS_FAILURE(
						_ns_spine_layer_add_voxel(
							curr_layer,
							voxel,
							spines->voxel_info
							),
							error ) )
					goto _NS_SPINES_CREATE_EVENT_EXIT;

				curr = next;
            }

         /* Continue shell building. Note that swapping the current and next shell
            pointers is equivalent to clearing the current shell, adding the voxels
            of the next shell to the current shell, and clearing the next shell. *//*
         NS_SWAP( NsList*, event->curr_shell, event->next_shell );

         /* Check the spread of the layer. If we've gone beyong the maximum allowed
            width then we are done with this entire event. *//*
         if( max_layer_spread < _ns_spine_layer_max_spread( curr_layer, spines ) )
            {
            ns_list_clear( event->curr_shell );
            goto _NS_SPINES_CREATE_EVENT_DONE;
            }
         }

      /* If we ran out of voxels ( because the spine is not attached to the
         dendrite ), then we are done with this event. *//*
      if( ns_list_is_empty( &curr_layer->voxels ) )
         goto _NS_SPINES_CREATE_EVENT_DONE; /* NOTE: 'break' would work as well */

      /* Proceed to the next layer. *//*
      prev_layer = curr_layer;

		/* Set all the voxels in this layer as accepted (part of dendrite),
			i.e. make their clump ID not zero. *//*
      curr = ns_list_begin( &curr_layer->voxels );
      end  = ns_list_end( &curr_layer->voxels );

      for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
         {
         voxel = ns_list_iter_get_object( curr );
			__DATA( voxel, clump_id ) = 1;
         }
      }

   _NS_SPINES_CREATE_EVENT_DONE:

   ns_assert( ns_list_is_empty( event->curr_shell ) );
   ns_assert( ns_list_is_empty( event->next_shell ) );

   _NS_SPINES_CREATE_EVENT_EXIT:

   return error;
   }
*/

NS_PRIVATE NsError _ns_spines_create_spine_event
   (
   NsSpines      *spines,
   NsVoxel       *origin,
   NsSpineEvent  *event
   )
   {
   NsVoxel        N;
   nsint          iteration;
   NsSpineLayer  *prev_layer;
   NsSpineLayer  *curr_layer;
   nslistiter     curr;
	nslistiter     next;
   nslistiter     end;
   nssize         i;
   NsVoxel       *voxel;
   NsVoxel       *neighbor;
   nsboolean      out_of_bounds;
	nsboolean      hit_clump_zero;
	nsboolean      is_still_flat;
	nsfloat        max_layer_spread;
   nsint          width;
   nsint          height;
   NsError        error;


   error = ns_no_error();

	max_layer_spread = ns_settings_get_spine_max_layer_spread( spines->settings );

   width  = ns_voxel_buffer_width( &spines->voxel_buffer );
   height = ns_voxel_buffer_height( &spines->voxel_buffer );

   out_of_bounds  = NS_FALSE;
	hit_clump_zero = NS_FALSE;
	is_still_flat  = NS_TRUE;

   ns_assert( ns_list_is_empty( &event->layers ) );
   ns_assert( ns_list_is_empty( event->curr_shell ) );
   ns_assert( ns_list_is_empty( event->next_shell ) );

   event->origin = origin;
	ns_to_voxel_space( &origin->position, &event->O, spines->voxel_info );

   iteration = 0;

   prev_layer = NULL;

   NS_INFINITE_LOOP
      {
      /* Create the next layer and set its floor to the most negative integer
         so that all voxels of the first shell will go into the layer. */

      if( NS_FAILURE( _ns_spine_layer_new( &curr_layer, iteration ), error ) )
         goto _NS_SPINES_CREATE_EVENT_EXIT;

      if( NS_FAILURE( ns_list_push_back( &event->layers, curr_layer ), error ) )
         {
         _ns_spine_layer_delete( curr_layer );
         goto _NS_SPINES_CREATE_EVENT_EXIT;
         }

		/* Must set these first for every new layer! */
      curr_layer->min_distance = -NS_FLOAT_MAX;

      ++iteration;

      ns_assert( ns_list_is_empty( event->curr_shell ) );
      ns_assert( ns_list_is_empty( event->next_shell ) );

      /* If this is the first layer then add the origin to the layer AND
         the first shell. Otherwise add all voxels of the previous layer
         to the first shell. */

      if( NULL == prev_layer )
         {
         if( NS_FAILURE( _ns_spine_event_add_voxel(
                           event,
                           event->origin,
                           event->curr_shell
                           ),
                           error ) )
            goto _NS_SPINES_CREATE_EVENT_EXIT;

         if( NS_FAILURE( _ns_spine_layer_add_voxel(
                           curr_layer,
                           event->origin,
                           spines->voxel_info
                           ),
                           error ) )
            goto _NS_SPINES_CREATE_EVENT_EXIT;
         }
      else
         {
         curr = ns_list_begin( &prev_layer->voxels );
         end  = ns_list_end( &prev_layer->voxels );

         for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
            {
            voxel = ns_list_iter_get_object( curr );

            if( NS_FAILURE( _ns_spine_event_add_voxel( event, voxel, event->curr_shell ), error ) )
               goto _NS_SPINES_CREATE_EVENT_EXIT;
            }
         }

      NS_INFINITE_LOOP
         {
         //ns_assert( ! ns_list_is_empty( event->curr_shell ) );

         curr = ns_list_begin( event->curr_shell );
         end  = ns_list_end( event->curr_shell );

         for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
            {
            voxel = ns_list_iter_get_object( curr );

            for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
               {
               ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

               /* Check for out of bounds, that is in X and Y dimension only.
                  This will allow 2D spine detection to still work. */
               if( N.position.x < 0 || N.position.x >= width ||
                   N.position.y < 0 || N.position.y >= height   )
                  out_of_bounds = NS_TRUE;

               /* NOTE: Neighbor voxels are not added if they are less then the floor
                  established for the current layer. For the first shell the floor is
                  set to the maximum negative integer so all voxels should pass.

						Also the neighbor voxel must either be part of the dendrite (i.e. has
						a clump ID of zero), or is part of the same clump as the voxel.
					*/

               if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
						{
						if( 0 == __DATA( neighbor, clump_id ) )
							hit_clump_zero = NS_TRUE;

                  if( ! __GET( neighbor, __LAYERED ) && ! __GET( neighbor, __SHELLED ) )
                     if( __DATA( neighbor, distance ) >= curr_layer->min_distance )
								if( NULL == __DATA( neighbor, spine ) )
									if( ( __DATA( neighbor, clump_id ) == __DATA( voxel, clump_id ) ) ||
										 ( 0 == __DATA( neighbor, clump_id ) ) ||
										 ( hit_clump_zero && is_still_flat ) ||
										 _ns_spine_clump_has_connection( __DATA( neighbor, clump ), __DATA( voxel, clump ) )
										)
										if( NS_FAILURE(
												_ns_spine_event_add_voxel(
													event,
													neighbor,
													event->next_shell
													),
												error ) )
											goto _NS_SPINES_CREATE_EVENT_EXIT;
						}
               }
            }

         ns_list_clear( event->curr_shell );

         /* Did we run of voxels? If so we're currently done in this
            inner "shell building" loop. */
         if( ns_list_is_empty( event->next_shell ) )
            break;

			/* Establish the floor if this is the first shell. */
         if( _ns_spine_layer_floor_not_established( curr_layer ) )
				_ns_spine_event_establish_layer_limits(
					spines,
					event,
					event->next_shell,
					&curr_layer->min_distance
					);

         curr = ns_list_begin( event->next_shell );
         end  = ns_list_end( event->next_shell );

         while( ns_list_iter_not_equal( curr, end ) )
            {
				next  = ns_list_iter_next( curr );
            voxel = ns_list_iter_get_object( curr );

				if( NS_FAILURE(
						_ns_spine_layer_add_voxel(
							curr_layer,
							voxel,
							spines->voxel_info
							),
							error ) )
					goto _NS_SPINES_CREATE_EVENT_EXIT;

				curr = next;
            }

         /* Continue shell building. Note that swapping the current and next shell
            pointers is equivalent to clearing the current shell, adding the voxels
            of the next shell to the current shell, and clearing the next shell. */
         NS_SWAP( NsList*, event->curr_shell, event->next_shell );

         /* Check the spread of the layer. If we've gone beyong the maximum allowed
            width then we are done with this entire event. Also note that if we went
            out of bounds then we're done as well. */
         if( max_layer_spread < _ns_spine_layer_max_spread( curr_layer, spines ) || out_of_bounds )
            {
            ns_list_clear( event->curr_shell );
            goto _NS_SPINES_CREATE_EVENT_DONE;
            }
         }

      /* If we ran out of voxels ( because the spine is not attached to the
         dendrite ), then we are done with this event. */
      if( ns_list_is_empty( &curr_layer->voxels ) )
         goto _NS_SPINES_CREATE_EVENT_DONE; /* NOTE: 'break' would work as well */

		if( is_still_flat )
			{
			nsdouble layer_height, layer_width;
	
			layer_width  = _ns_spine_layer_max_spread( curr_layer, spines );
			layer_height = __DATA( event->origin, distance ) - curr_layer->min_distance;

			if( ( layer_height / layer_width ) > ( 2.0 * ____invalid_ratio ) )
				is_still_flat = NS_FALSE;
			}

      /* Proceed to the next layer. */
      prev_layer = curr_layer;
      }

   _NS_SPINES_CREATE_EVENT_DONE:
   ns_assert( ns_list_is_empty( event->curr_shell ) );
   ns_assert( ns_list_is_empty( event->next_shell ) );

   /* Go through all the layers and set their final
      width and depth values. */

   curr = ns_list_begin( &event->layers );
   end  = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      curr_layer = ns_list_iter_get_object( curr );

      if( ns_list_is_empty( &curr_layer->voxels ) )
         {
			ns_assert( ns_list_iter_equal( ns_list_rev_begin( &event->layers ), curr ) );

			curr_layer->max_spread = _NS_SPINES_MIN_SPREAD_VALUE;
			curr_layer->width      = _NS_SPINES_MIN_RAYBURST_WIDTH;
			curr_layer->height     = __DATA( event->origin, distance );
			curr_layer->calc_width = NS_FALSE;
         }
      else
         {
			curr_layer->min_distance = _ns_spine_layer_min_voxel_distance( curr_layer );
			curr_layer->height       = __DATA( event->origin, distance ) - curr_layer->min_distance;

			//ns_assert( 0.0f <= curr_layer->height );

			curr_layer->max_spread = _ns_spine_layer_max_spread( curr_layer, spines );
			curr_layer->calc_width = NS_TRUE;
         }
      }

   /* If the last layer has spread beyond the max spread, then set it to a large
      number since in reality we dont know how far it would have spread. Also if
      we went out of bounds, then the last layer's spread should be set huge since
		again we dont really know how far that layer should have spread. */
   ns_assert( ! ns_list_is_empty( &event->layers ) );

   curr       = ns_list_rev_begin( &event->layers );
   curr_layer = ns_list_iter_get_object( curr );

   if( max_layer_spread <= curr_layer->max_spread || out_of_bounds )
      {
		curr_layer->max_spread = NS_FLOAT_MAX;
      curr_layer->calc_width = NS_FALSE;
      }

   _NS_SPINES_CREATE_EVENT_EXIT:

   return error;
   }


NS_PRIVATE NsError _ns_spines_store_layers
	(
   const NsSpineEvent  *event,
	NsSpine             *spine,
	const NsSpines      *spines
	)
	{
	const NsSpineLayer  *orig_layer;
	NsSpineLayer        *dup_layer;
   nslistiter           curr_layer;
	nslistiter           end_layers;
	NsError              error;


   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
		{
		orig_layer = ns_list_iter_get_object( curr_layer );
		dup_layer  = NULL;

		if( NS_FAILURE( _ns_spine_layer_clone( orig_layer, spines, &dup_layer ), error ) )
			return error;

		if( NS_FAILURE( ns_list_push_back( &spine->layers, dup_layer ), error ) )
			{
			_ns_spine_layer_delete( dup_layer );
			return error;
			}
		}

	return ns_no_error();
	}


/* If the last layer has no voxels, the spine is detached. */
NS_PRIVATE nsboolean _ns_spine_event_is_attached( const NsSpineEvent *event )
	{
   const NsSpineLayer *layer;

	ns_assert( ! ns_list_is_empty( &event->layers ) );
	layer = ns_list_rev_iter_get_object( ns_list_rev_begin( &event->layers ) );

	return 0 < ns_list_size( &layer->voxels );
	}


NS_PRIVATE NsError _ns_spines_store_event
   (
   NsSpines            *spines,
   const NsSpineEvent  *event,
   NsSpineType          spine_type
   )
   {
   NsSpine       *spine;
   NsVoxel       *voxel;
   NsSpineLayer  *layer;
	nslistiter     prev_layer;
   nslistiter     curr_layer;
   nslistiter     end_layers;
   nslistiter     curr_voxel;
   nslistiter     end_voxels;
   NsError        error;


	ns_assert( NULL == __DATA( event->origin, spine ) );

   error = ns_no_error();

   ns_model_lock( spines->model );

   if( NS_FAILURE( _ns_spine_new( &spine, spines ), error ) )
      goto _NS_SPINES_STORE_EVENT_EXIT;

   if( NS_FAILURE( ns_list_push_back( &spines->list, spine ), error ) )
      {
      _ns_spine_delete( spine );
      goto _NS_SPINES_STORE_EVENT_EXIT;
      }

   spine->type               = spine_type;
   spine->origin             = event->origin;
	spine->correction_factor  = event->correction_factor;
   spine->auto_detected      = NS_TRUE;
	//spine->medial_axis_length = 0.0f;

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
   spine->id = (spines->id)++;

   spine->iter = ns_list_rev_begin( &spines->list );
   ns_assert( ns_list_iter_get_object( spine->iter ) == spine );

   /* Go through all the voxels of every layer. */
	prev_layer = NULL;
   curr_layer = ns_list_begin( &event->layers );
   end_layers = ns_list_end( &event->layers );

   for( ; ns_list_iter_not_equal( curr_layer, end_layers );
          curr_layer = ns_list_iter_next( curr_layer ) )
      {
      layer = ns_list_iter_get_object( curr_layer );

		//if( curr_layer == ns_list_begin( &event->layers ) )
		//	spine->first_layer_center = layer->center;

		//if( curr_layer == ns_list_rev_begin( &event->layers ) )
		//	spine->last_layer_center = layer->center;

		if( NULL != prev_layer )
			if( ! ns_list_is_empty( &layer->voxels ) )
				{}//- medial_axis_length += distance( prev->center, curr->center )

      curr_voxel = ns_list_begin( &layer->voxels );
      end_voxels = ns_list_end( &layer->voxels );

      for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
            curr_voxel = ns_list_iter_next( curr_voxel ) )
         {
         voxel = ns_list_iter_get_object( curr_voxel );

			/* New algorithm shouldnt allow adding any voxels that
				are already part of a spine. 3/1/07 */
			ns_assert( NULL == __DATA( voxel, spine ) );

			if( NULL == __DATA( voxel, spine ) )
				{
				if( NS_FAILURE( _ns_spine_add_voxel( spine, voxel, spines->voxel_info ), error ) )
					goto _NS_SPINES_STORE_EVENT_EXIT;

				//__DATA( voxel, layer_index ) = ( nsuchar )layer->index;

				/* TEMP!!!!!!!!!!! */
				//if( ( ( nsfloat )__DATA( voxel, num_neighbors ) ) <= ( layer->avg_num_neighbors / 3.0f ) )
				//	__DATA( voxel, layer_color ) = _NS_VOXEL_LAYER_DEVIANT_COLOR;
				//else
				//	__DATA( voxel, layer_color ) = ( nsuchar )( layer->index % _NS_SPINES_NUM_LAYER_COLORS );
				}
         }

		prev_layer = curr_layer;
      }

	/* If the last layer has no voxels, the spine is detached. */
	ns_assert( ! ns_list_is_empty( &event->layers ) );
	layer = ns_list_rev_iter_get_object( ns_list_rev_begin( &event->layers ) );

	spine->is_attached = ( 0 < ns_list_size( &layer->voxels ) );
	spine->min_floor   = layer->min_distance;

	/* For finding the spine medial axis we need to initialize the voxels in the last
		layer... that is the last layer that has voxels. Remember unattached spines have
		a last layer that is empty! */
	if( spine->is_attached )
		_ns_spine_layer_init_distance_to_bottom( layer );
	else
		{
		ns_assert( ns_list_size( &event->layers ) >= 2 );
		layer = ns_list_rev_iter_get_object( ns_list_rev_iter_next( ns_list_rev_begin( &event->layers ) ) );
		ns_assert( ! ns_list_is_empty( &layer->voxels ) );

		_ns_spine_layer_init_distance_to_bottom( layer );
		}

	spine->stats = event->stats;

	spine->has_head_center = event->has_head_center;
	spine->head_center     = event->head_center;
	spine->has_neck_center = event->has_neck_center;
	spine->neck_center     = event->neck_center;

	/* At least the origin should be included in this spine. */
	ns_assert( ! ns_list_is_empty( &spine->voxels ) );

   /* Set temporary values for some of the spines members
      so the spine can be renderered. */
   ns_to_voxel_space( &(spine->origin->position), &spine->center, spines->voxel_info );
   spine->total_volume = _ns_spine_voxel_volume( spine, spines );

	/* NOTE: Ignoring error on storing layers. */
	if( spines->save_layers )
		_ns_spines_store_layers( event, spine, spines );

   _NS_SPINES_STORE_EVENT_EXIT:

   ns_model_unlock( spines->model );

	if( spines->active_render && NULL != spines->render_func )
      ( spines->render_func )();

   return error;
   }


