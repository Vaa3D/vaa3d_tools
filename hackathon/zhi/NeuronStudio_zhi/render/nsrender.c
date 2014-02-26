#include "nsrender.h"


void ns_render_state_init( NsRenderState *state )
   {
   ns_assert( NULL != state );
   ns_memzero( state, sizeof( NsRenderState ) );

   state->constants.iface.pre_borders_func     = NULL;
   state->constants.iface.border_func          = NULL;
   state->constants.iface.post_borders_func    = NULL;

   state->constants.iface.pre_points_func      = NULL;
   state->constants.iface.point_func           = NULL;
   state->constants.iface.post_points_func     = NULL;

   state->constants.iface.pre_lines_func       = NULL;
   state->constants.iface.line_func            = NULL;
   state->constants.iface.post_lines_func      = NULL;

   state->constants.iface.pre_images_func      = NULL;
   state->constants.iface.image_func           = NULL;
   state->constants.iface.post_images_func     = NULL;

   state->constants.iface.pre_texts_func       = NULL;
   state->constants.iface.text_func            = NULL;
   state->constants.iface.post_texts_func      = NULL;

   state->constants.iface.pre_triangles_func   = NULL;
   state->constants.iface.triangle_func        = NULL;
   state->constants.iface.post_triangles_func  = NULL;

   state->constants.iface.pre_spheres_func     = NULL;
   state->constants.iface.sphere_func          = NULL;
   state->constants.iface.post_spheres_func    = NULL;

   state->constants.iface.pre_planes_func      = NULL;
   state->constants.iface.plane_func           = NULL;
   state->constants.iface.post_planes_func     = NULL;

   state->constants.iface.pre_ellipses_func    = NULL;
   state->constants.iface.ellipse_func         = NULL;
   state->constants.iface.post_ellipses_func   = NULL;

   state->constants.iface.pre_aabboxes_func    = NULL;
   state->constants.iface.aabbox_func          = NULL;
   state->constants.iface.post_aabboxes_func   = NULL;

   state->constants.iface.pre_frustums_func    = NULL;
   state->constants.iface.frustum_func         = NULL;
   state->constants.iface.post_frustums_func   = NULL;

   state->constants.iface.pre_rectangles_func  = NULL;
   state->constants.iface.rectangle_func       = NULL;
   state->constants.iface.post_rectangles_func = NULL;

	state->constants.dataset    = NULL;
	state->constants.proj_xy    = NULL;
	state->constants.proj_zy    = NULL;
	state->constants.proj_xz    = NULL;
   state->constants.model      = NULL;
   state->constants.sampler    = NULL;
   state->constants.voxel_info = NULL;

   state->constants.vertex_type_colors = NULL;

   state->constants.component_colors = NULL;

   state->constants.edge_order_colors   = NULL;
   state->constants.edge_section_colors = NULL;

   state->constants.spine_type_colors   = NULL;
   state->constants.spine_random_colors = NULL;

   state->constants.brush_db  = NULL;
   state->constants.get_brush = NULL;
   state->constants.set_brush = NULL;
   state->constants.pen_db    = NULL;
   state->constants.get_pen   = NULL;
   state->constants.set_pen   = NULL;

	state->constants.model_mask_factor = 1.0f;
   }


/* NOTE: Avoid circular inclusion! */
#include <image/nssampler.h>
#include <model/nsmodel.h>
#include <model/nsmodel-graft.h>
#include <model/nsmodel-spines.h>


#define NS_RENDER_CLASSIFIER_NUM_COLORS  15
NS_PRIVATE NsColor4ub ____ns_render_classifier_colors[ NS_RENDER_CLASSIFIER_NUM_COLORS ] =
	{
	/*aqua*/    { 0x00, 0xFF, 0xFF },
	/*green*/   { 0x00, 0x80, 0x00 },
	/*navy*/    { 0x00, 0x00, 0x80 },
	/*olive*/   { 0x80, 0x80, 0x00 },
	/*teal*/    { 0x00, 0x80, 0x80 },
	/*blue*/    { 0x00, 0x00, 0xFF },
	/*lime*/    { 0x00, 0xFF, 0x00 },
	/*purple*/  { 0x80, 0x00, 0x80 },
	/*white*/   { 0xFF, 0xFF, 0xFF },
	/*fuchsia*/ { 0xFF, 0x00, 0xFF },
	/*maroon*/  { 0x80, 0x00, 0x00 },
	/*red*/     { 0xFF, 0x00, 0x00 },
	/*yellow*/  { 0xFF, 0xFF, 0x00 },
	/*silver*/  { 0xC0, 0xC0, 0xC0 },
	/*gray*/    { 0x80, 0x80, 0x80 }
	};


NsColor4ub ns_render_state_spine_color( NsRenderState *state, nsspine spine )
   {
   NsColor4ub  C;
	nssize      c;
	nsenum      type;


	C = NS_COLOR4UB_BLACK;

   if( state->constants.spine_color_scheme == state->constants.SPINE_SINGLE_COLOR )
      C = state->constants.spine_single_color;
   else if( state->constants.spine_color_scheme == state->constants.SPINE_COLOR_BY_TYPE )
      {
		c = ( nssize )ns_spine_get_type( spine );

		if( NULL == state->constants.classifier )
			{
			if( c < state->constants.num_spine_type_colors )
				C = state->constants.spine_type_colors[ c ];
			}
		else
			{
			if( 0 < c )
				C = ____ns_render_classifier_colors[ ( c - 1 ) % NS_RENDER_CLASSIFIER_NUM_COLORS ];
			}
      }
   else if( state->constants.spine_color_scheme == state->constants.SPINE_COLOR_RANDOM )
      {
		c = ( nssize )ns_spine_get_id( spine );

      if( c < state->constants.num_spine_random_colors )
         C = state->constants.spine_random_colors[ c ];
      else
         C = state->constants.spine_random_colors[ c % state->constants.num_spine_random_colors ];
      }
   else if( state->constants.spine_color_scheme == state->constants.SPINE_COLOR_IS_MANUAL )
		{
      if( ! ns_spine_get_auto_detected( spine ) )
         C = state->constants.manually_traced_color; /* TEMP? */
		}
   else if( state->constants.spine_color_scheme == state->constants.SPINE_COLOR_BY_ANGLE )
		{
		type = ns_spine_xyplane_angle_type( ns_spine_get_xyplane_angle( spine ) );

		if( NS_SPINE_XYPLANE_ANGLE_PARALLEL == type )
			C = NS_COLOR4UB_GREEN;
		else if( NS_SPINE_XYPLANE_ANGLE_OBLIQUE == type )
			C = NS_COLOR4UB_RED;
		}

   return C;
   }


NsColor4ub ns_render_state_vertex_color( NsRenderState *state, nsmodelvertex V )
   {
   NsColor4ub  C;
	nssize      c;


	C = NS_COLOR4UB_BLACK;

   if( state->constants.vertex_color_scheme == state->constants.VERTEX_SINGLE_COLOR )
      C = state->constants.vertex_single_color;
   else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_TYPE )
      {
		c = ( nssize )ns_model_vertex_get_type( V );

      if( c < state->constants.num_vertex_type_colors )
         C = state->constants.vertex_type_colors[ c ];
      }
	else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_FUNCTION )
		{
		c = ( nssize )ns_model_vertex_get_function_type( V );
		
		if( c < state->constants.num_function_type_colors )
			C = state->constants.function_type_colors[ c ];

		if( ns_model_vertex_is_origin( V ) )
			C = state->variables.altered_color;
		}
   else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_COMPONENT )
      {
		c = ( nssize )ns_model_vertex_get_conn_comp( V ); /* 0-based index */
		C = state->constants.component_colors[ c % state->constants.num_component_colors ];
      }
   else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_IS_MANUALLY_TRACED )
      {
      if( ns_model_vertex_is_manually_traced( V ) )
         C = state->constants.manually_traced_color;
      }
   else if( state->constants.vertex_color_scheme == state->constants.VERTEX_COLOR_BY_FILE_NUMBER )
      {
		c = ( nssize )ns_model_vertex_get_file_num( V ); /* 1-based index */

		/* TEMP: Should pass color array for file number rendering as well. */

		if( 0 < c )
			C = state->constants.component_colors[ ( c - 1 ) % state->constants.num_component_colors ];
      }

   return C;
   }


NsColor4ub ns_render_state_edge_color( NsRenderState *state, nsmodeledge E )
   {
   NsColor4ub  C;
	nssize      c;

	
	C = NS_COLOR4UB_BLACK;

   if( state->constants.edge_color_scheme == state->constants.EDGE_SINGLE_COLOR )
      C = state->constants.edge_single_color;
	else if( state->constants.edge_color_scheme == state->constants.EDGE_COLOR_BY_FUNCTION )
		{
		c = ( nssize )ns_model_edge_get_function_type( E );
		
		if( c < state->constants.num_function_type_colors )
			C = state->constants.function_type_colors[ c ];
		}
   else if( state->constants.edge_color_scheme == state->constants.EDGE_COLOR_BY_COMPONENT )
      {
      /* NOTE: The source and destination vertices of the edge should be in the same component.
         We could assert this but since this is only display code, lets forget it. */
      nsmodelvertex V = ns_model_edge_src_vertex( E );

		c = ( nssize )ns_model_vertex_get_conn_comp( V ); /* 0-based index */
		C = state->constants.component_colors[ c % state->constants.num_component_colors ];
      }
   else if( state->constants.edge_color_scheme == state->constants.EDGE_COLOR_BY_ORDER )
      {
		c = ( nssize )ns_model_edge_get_order( E ); /* 1-based index */

      if( 0 < c )
         C = state->constants.edge_order_colors[ ( c - 1 ) % state->constants.num_edge_order_colors ];
      }
   else if( state->constants.edge_color_scheme == state->constants.EDGE_COLOR_BY_SECTION )
      {
		c = ( nssize )ns_model_edge_get_section( E ); /* 1-based index */

      if( 0 < c )
         C = state->constants.edge_section_colors[ ( c - 1 ) % state->constants.num_edge_section_colors ];
      }

   return C;
   }


NsColor4ub ns_render_state_edge_color_ex( NsRenderState *state, nsint edge_color_scheme, nspointer edge )
	{
	NsColor4ub  C;
	nsint       old;


	/* Save the old color scheme first! */
	old = state->constants.edge_color_scheme;

	state->constants.edge_color_scheme = edge_color_scheme;
	C = ns_render_state_edge_color( state, edge );

	state->constants.edge_color_scheme = old;

	return C;
	}


NS_PRIVATE nsboolean _ns_render_state_lock_model( NsRenderState *state )
   {
   nsboolean locked = NS_TRUE;

   if( state->constants.flicker_free_rendering )
      ns_model_lock( state->constants.model );
   else
      locked = ns_model_try_lock( state->constants.model );

   return locked;
   }


NS_PRIVATE nsboolean _ns_render_state_lock_sampler( NsRenderState *state )
   {
   nsboolean locked = NS_TRUE;

   if( state->constants.flicker_free_rendering )
      ns_sampler_lock( state->constants.sampler );
   else
      locked = ns_sampler_try_lock( state->constants.sampler );

   return locked;
   }


NS_PRIVATE void _ns_render_samples( NsRenderState *state )
	{
   if( ! ns_model_spines_is_running( state->constants.model ) &&
       ! ns_model_grafter_is_running( state->constants.model ) )
      if( _ns_render_state_lock_sampler( state ) )
         {
         state->variables.filled = NS_TRUE;

         if( state->constants.view_sampler_thresholding )
            {
            //ns_sampler_render_images( state->constants.sampler, state );
            ns_sampler_render_borders( state->constants.sampler, state );
            }

         ns_sampler_render_lines( state->constants.sampler, state );

         /*ns_sampler_render_markers( state->constants.sampler, state );
         ns_sampler_render_invalid_markers( state->constants.sampler, state );*/

         if( state->constants.view_sampler_surface_mesh )
            ns_sampler_render_triangles( state->constants.sampler, state );

         state->variables.filled = ns_sampler_is_running( state->constants.sampler );
         ns_sampler_render_last( state->constants.sampler, state );
         state->variables.filled = NS_TRUE;

         ns_sampler_render_selected( state->constants.sampler, state );

         if( state->constants.view_sampler_text )
            ns_sampler_render_text( state->constants.sampler, state );

         ns_sampler_unlock( state->constants.sampler );
         }
	}


NS_PRIVATE void _ns_render_grafting( NsRenderState *state )
	{
	NS_USE_VARIABLE( state );
   if( state->constants.view_graft_thresholding )
      if( _ns_render_state_lock_model( state ) )
         {
         ns_model_render_grafter( state->constants.model, state );
         ns_model_unlock( state->constants.model );
         }
	}


NS_PRIVATE void _ns_render_vertices( NsRenderState *state )
	{
   if( ( NULL == state->constants.sampler || ! ns_sampler_is_running( state->constants.sampler ) )
			&&
       ( state->constants.is_model_mask || ! ns_model_spines_is_running( state->constants.model ) ) )
      if( _ns_render_state_lock_model( state ) )
         {
			state->variables.filled = NS_TRUE;

			if( state->constants.vertex_shape_mode == state->constants.VERTEX_SHAPE_HOLLOW_ELLIPSES ||
				 state->constants.vertex_shape_mode == state->constants.VERTEX_SHAPE_SOLID_ELLIPSES    )
				{
				if( state->constants.vertex_shape_mode == state->constants.VERTEX_SHAPE_HOLLOW_ELLIPSES )
					state->variables.filled = NS_FALSE;

				if( ns_model_grafter_is_running( state->constants.model ) ||
					 ns_model_get_is_filtering( state->constants.model )     )
					{
					//if( state->constants.view_graft_all_vertices )
						ns_model_render_vertices_fixed_spheres( state->constants.model, state );
					//else
					//	ns_model_render_vertex_list_spheres( state->constants.model, state );
					}
				else
					ns_model_render_vertices_spheres( state->constants.model, state );
				}
			else if( state->constants.vertex_shape_mode == state->constants.VERTEX_SHAPE_POINTS )
				{
				if( ns_model_grafter_is_running( state->constants.model ) )
					{
					if( state->constants.view_graft_all_vertices )
						ns_model_render_vertices_points( state->constants.model, state );
					else
						ns_model_render_vertex_list_points( state->constants.model, state );
					}
				else
					ns_model_render_vertices_points( state->constants.model, state );
				}

			if( ! ns_model_grafter_is_running( state->constants.model ) )
				ns_model_render_vertices_selected( state->constants.model, state );

         ns_model_unlock( state->constants.model );
         }
	}


NS_PRIVATE void _ns_render_edges( NsRenderState *state )
	{
   if( ( NULL == state->constants.sampler || ! ns_sampler_is_running( state->constants.sampler ) )
			&&
       ( state->constants.is_model_mask || ! ns_model_spines_is_running( state->constants.model ) )
         &&
		 ! ns_model_grafter_is_running( state->constants.model ) )
      if( _ns_render_state_lock_model( state ) )
         {
			state->variables.filled = NS_TRUE;

			if( state->constants.edge_shape_mode == state->constants.EDGE_SHAPE_HOLLOW_FRUSTUMS ||
				 state->constants.edge_shape_mode == state->constants.EDGE_SHAPE_SOLID_FRUSTUMS    )
				{
				if( state->constants.edge_shape_mode == state->constants.EDGE_SHAPE_HOLLOW_FRUSTUMS )
					state->variables.filled = NS_FALSE;

				ns_model_render_edges_frustums( state->constants.model, state );
				}
			else if( state->constants.edge_shape_mode == state->constants.EDGE_SHAPE_LINES )
				ns_model_render_edges_lines( state->constants.model, state );

         ns_model_unlock( state->constants.model );
			}
	}


//extern nsboolean ____use_antialiasing;
//extern nsboolean ____view_sholl_analysis;


NS_PRIVATE void _ns_render_spines( NsRenderState *state )
	{
   if( ( NULL == state->constants.sampler || ! ns_sampler_is_running( state->constants.sampler ) ) &&
       ! ns_model_grafter_is_running( state->constants.model ) )
      if( _ns_render_state_lock_model( state ) )
         {
         state->variables.filled = NS_TRUE;

			//if( state->constants.view_maximum_spine_height )
				//ns_model_render_msh_zone( state->constants.model, state );

         if( ns_model_spines_is_running( state->constants.model ) &&
             state->constants.spine_shape_mode != state->constants.SPINE_NO_SHAPE )
            ns_model_render_spines_fixed_spheres( state->constants.model, state );
         else
            {
            //if( state->constants.spines_color_by_layer )
              // ns_model_render_spines_samples( state->constants.model, state );

            if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_POINTS ||
                state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_RECTANGLES ||
                state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_RECTANGLES )
               {
               if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_RECTANGLES )
                  state->variables.filled = NS_FALSE;

               if( state->constants.spines_color_by_layer )
                  {
                  if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_POINTS )
							ns_model_render_spines_layers_as_points( state->constants.model, state );
                  else
                     ns_model_render_spines_layers_as_rectangles( state->constants.model, state );
                  }
               else
                  {
                  if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_POINTS )
                     ns_model_render_spines_voxels( state->constants.model, state );
                  else
                     ns_model_render_spines_rectangles( state->constants.model, state );
                  }

               if( state->constants.view_spine_candidate_voxels ||
                   state->constants.view_spine_maxima_voxels )
                  {
                  if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_POINTS )
                     ns_model_render_spines_candidates_as_points( state->constants.model, state );
                  else
                     ns_model_render_spines_candidates_as_rectangles( state->constants.model, state );
                  }
               }
            else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_ELLIPSES ||
							state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES )
					{
					//ns_model_render_spines_candidates_gradients( state->constants.model, state );

					if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES )
						state->variables.filled = NS_FALSE;

               ns_model_render_spines_fixed_spheres( state->constants.model, state );
					}
            else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_ELLIPSES_HD ||
							state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES_HD )
					{
					if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES_HD )
						state->variables.filled = NS_FALSE;

               ns_model_render_spines_ellipses_hd( state->constants.model, state );
					}
            else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_BALL_AND_STICK ||
							state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_BALL_AND_STICK )
					{
					if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_BALL_AND_STICK )
						state->variables.filled = NS_FALSE;

               ns_model_render_spines_ball_and_stick( state->constants.model, state );
					}
            else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_VOLUMETRIC ||
							state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_VOLUMETRIC )
					{
					if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_VOLUMETRIC )
						state->variables.filled = NS_FALSE;

               ns_model_render_spines_volumetric( state->constants.model, state );
					}
            else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_SURFACE ||
							state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_SURFACE  )
					{
					if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_HOLLOW_SURFACE )
						state->variables.filled = NS_FALSE;

					/*TEMP?*/
					if( 3 != state->constants.dimensions )
						ns_model_render_spines_fixed_spheres( state->constants.model, state );
					else
						ns_model_render_spines_surface( state->constants.model, state );
					}
				else if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_CLUMPS )
					ns_model_render_spines_clumps_voxels( state->constants.model, state );

///*TEMP*/ns_model_render_spines_layers_as_points( state->constants.model, state );

				//if( state->constants.spine_shape_mode == state->constants.SPINE_SHAPE_SOLID_ELLIPSES )
					ns_model_render_spines_fixed_selected( state->constants.model, state );
				//else
				//	ns_model_render_spines_selected( state->constants.model, state );

            if( state->constants.view_spine_vectors )
				  ns_model_render_spines_axis( state->constants.model, state );

            if( state->constants.view_spine_attachments )
              ns_model_render_spines_vectors( state->constants.model, state );


/*TEMP!!!!!!!!!*/
//ns_model_render_spines_borderers( state->constants.model, state );
//ns_model_render_spines_centroids( state->constants.model, state );
//ns_model_render_spines_interface_voxels( state->constants.model, state );


				//ns_model_render_spines_merging( state->constants.model, state );

/*TEMP*///ns_model_render_spine_head_and_neck_centers( state->constants.model, state );
            }

         ns_model_unlock( state->constants.model );
         }
	}


typedef void ( *NsRenderStepFunc )( NsRenderState* );

NS_PRIVATE void _ns_do_render( NsRenderState *state, NsRenderStep step )
	{
	NS_PRIVATE nsboolean ____ns_render_step_map_init = NS_FALSE;
	NS_PRIVATE NsRenderStepFunc ____ns_render_step_map[ NS_RENDER_NUM_STEPS ];

	if( ! ____ns_render_step_map_init )
		{
		____ns_render_step_map[ NS_RENDER_STEP_SAMPLES  ] = _ns_render_samples;
		____ns_render_step_map[ NS_RENDER_STEP_GRAFTING ] = _ns_render_grafting;
		____ns_render_step_map[ NS_RENDER_STEP_VERTICES ] = _ns_render_vertices;
		____ns_render_step_map[ NS_RENDER_STEP_EDGES    ] = _ns_render_edges;
		____ns_render_step_map[ NS_RENDER_STEP_SPINES   ] = _ns_render_spines;

		____ns_render_step_map_init = NS_TRUE;
		}

	ns_assert( 0 <= step && step < NS_RENDER_NUM_STEPS );
	( ____ns_render_step_map[ step ] )( state );
	}


void ns_render_state_set_step( NsRenderState *state, nssize index, NsRenderStep step )
	{
	ns_assert( NULL != state );
	ns_assert( index < ( nssize )NS_RENDER_NUM_STEPS );
	ns_assert( 0 <= step && step < NS_RENDER_NUM_STEPS );

	state->constants.steps[ index ] = step;
	}


void ns_render_state_set_default_steps_order( NsRenderState *state )
	{
	nsenum i;

	for( i = 0; i < NS_RENDER_NUM_STEPS; ++i )
		state->constants.steps[i] = i;
	}


void ns_render( NsRenderState *state )
   {
	nsenum i;

	/* TEMP: Currently the dataset is drawn elsewhere, so just return. */
	if( state->constants.draw_dataset_only )
		return;

	if( NULL != state->constants.function_type_colors )
		ns_color4ub_lighten_or_darken(
			state->constants.function_type_colors + NS_MODEL_FUNCTION_SOMA,
			&state->variables.altered_color,
			25.0f
			);

   ns_model_set_voxel_info( state->constants.model, state->constants.voxel_info );

	for( i = 0; i < NS_RENDER_NUM_STEPS; ++i )
		_ns_do_render( state, state->constants.steps[i] );
   }
