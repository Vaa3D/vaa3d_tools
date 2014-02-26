
extern nsboolean ____xy_slice_enabled;
extern nssize ____xy_slice_index;


NS_PRIVATE void _ns_model_spines_render_as_spheres_begin( NsRenderState *state )
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		CirclesBegin( state->constants.polygon_complexity, state->constants.line_size );
	}


NS_PRIVATE void _ns_model_spines_render_as_spheres_end( NsRenderState *state )
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		CirclesEnd();
	}


NS_PRIVATE void _ns_model_spine_render_as_sphere
	(
	NsRenderState  *state,
	const NsSpine  *spine,
	nsfloat         radius
	)
	{
	if( 3 == state->constants.dimensions && ! state->variables.filled )
		{
		NsVector3d Pd;

		ns_vector3f_to_3d( &spine->center, &Pd );
		CircleRender( &Pd, radius, ( const nsuchar* )( &( state->variables.color ) ) );
		}
	else
		{
		NS_RENDER( state, sphere_func )( &spine->center, radius, state );
		}
	}


void ns_model_render_spines_surface
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines           *spines;
	nslistiter                curr;
	nslistiter                end;
	const NsSpine            *spine;
	nssize                    i;
	NsTriangle3f              T;
	const NsIndexTriangleus  *triangles;
	nssize                    num_triangles;
	const NsVector3f         *vectors;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, triangle_func ) );

	if( 3 != state->constants.dimensions )
		return;

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_TRIANGLE;
	NS_RENDER_PRE_OR_POST( state, pre_triangles_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ( ! spine->hidden || state->constants.draw_hidden_objects )
				&&
			 ( NULL != spine->distances && NULL != spine->interior ) )
			{
			triangles     = spine->triangles;
			num_triangles = spine->num_triangles;
			vectors       = spine->vectors;

			for( i = 0; i < num_triangles; ++i )
				{
				_ns_spines_make_triangle( spine, vectors, triangles, spine->distances, i, &T );

				/* NOTE: Apply stricter '&&' operator first! */
				if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, && ) )
					state->variables.color = NS_COLOR4UB_BLACK;
				else if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, || ) )
					state->variables.color = NS_COLOR4UB_DARK_GREY;
				else
					state->variables.color = ns_render_state_spine_color( state, curr );

				NS_RENDER( state, triangle_func )(
					NULL != spine->normals ? spine->normals + triangles[i].a : vectors + triangles[i].a,
					&T.A,
					NULL != spine->normals ? spine->normals + triangles[i].b : vectors + triangles[i].b,
					&T.B,
					NULL != spine->normals ? spine->normals + triangles[i].c : vectors + triangles[i].c,
					&T.C,
					state
					);
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_triangles_func );
	}



void ns_model_render_spines_points
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	NsSpine         *spine;
	NsVector3b       N;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_POINT;
	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_spine_color( state, curr );
			NS_RENDER( state, point_func )( &N, &spine->center, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}


void ns_model_render_spines_volumetric
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	NsSpine         *spine;
	nsdouble         radius;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, sphere_func ) );

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_SPHERE;
	NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_spines_render_as_spheres_begin( state );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_spine_color( state, curr );

			/* V = 4/3 * PI * r^3
				So r = ( 3/4 * V / PI )^1/3 */
			radius = ns_pow( ( ( 3.0 / 4.0 ) * spine->total_volume ) / NS_PI_D, 1.0 / 3.0 );

			_ns_model_spine_render_as_sphere( state, spine, ( nsfloat )radius );
			}
		}

	_ns_model_spines_render_as_spheres_end( state );

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );

	/* Draw points in 2D so that the spine is visible no matter
		what the zoom factor. */
	if( 2 == state->constants.dimensions )
		ns_model_render_spines_points( model, state );
	}


void ns_model_render_spines_origins
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;
	NsVector3f       V;
	NsVector3b       N;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_RED;
	state->variables.shape = NS_RENDER_SHAPE_POINT;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		ns_to_voxel_space( &spine->origin->position, &V, spines->voxel_info );
		NS_RENDER( state, point_func )( &N/*&( __DATA( spine->origin, N ) )*/, &V, state );
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}


/*
void ns_model_render_spines_maximas
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	const NsVoxel   *voxel;
	NsVector3f       V;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_RED;
	state->variables.shape = NS_RENDER_SHAPE_POINT;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		if( __GET( voxel, __MAXIMA ) )
			{
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
			NS_RENDER( state, point_func )( &V, state );
			}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}
*/


#include <ext/squares.inl>
void ns_model_render_spines_voxels
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;
	nslistiter       curr_voxel;
	nslistiter       end_voxels;	
	NsVoxel         *voxel;
	//NsVector3f       V;
	NsVector3f       V1;
	//NsVector3f       V2;
	nsfloat         *v1;
	NsColor4ub       C, L;
	//nsboolean        polygon_borders;
	//NsVector3f       S, H;
	NsVector3b       N;
   nsfloat          color[4];


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );

	ns_assert( NULL != NS_RENDER( state, point_func ) );
	//ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	v1 = ns_vector3f_array( &V1 );

	//S = *( ns_voxel_info_size( state->constants.voxel_info ) );

	//H.x = ( ns_voxel_info_size_x( state->constants.voxel_info ) / 2.0f );
	//H.y = ( ns_voxel_info_size_y( state->constants.voxel_info ) / 2.0f );
	//H.z = ( ns_voxel_info_size_z( state->constants.voxel_info ) / 2.0f );

	if( 2 == state->constants.dimensions )
		{
		state->variables.shape = NS_RENDER_SHAPE_POINT;
		NS_RENDER_PRE_OR_POST( state, pre_points_func );

		//polygon_borders                  = state->constants.polygon_borders;
		//state->constants.polygon_borders = NS_FALSE;

		//state->variables.shape = NS_RENDER_SHAPE_RECTANGLE;
		//NS_RENDER_PRE_OR_POST( state, pre_rectangles_func );

		curr = ns_list_begin( &spines->list );
		end  = ns_list_end( &spines->list );

		for( ; ns_list_iter_not_equal( curr, end );
				 curr = ns_list_iter_next( curr ) )
			{
			spine = ns_list_iter_get_object( curr );

			if( ! spine->hidden || state->constants.draw_hidden_objects )
				{
				state->variables.color = C = ns_render_state_spine_color( state, curr );

				curr_voxel = ns_list_begin( &spine->voxels );
				end_voxels = ns_list_end( &spine->voxels );

				for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
						 curr_voxel = ns_list_iter_next( curr_voxel ) )
					{
					voxel = ns_list_iter_get_object( curr_voxel );

					//if( state->constants.view_spine_maxima_voxels )
					//	{
					//	if( __GET( voxel, __MAXIMA ) )
					//		state->variables.color = NS_COLOR4UB_RED;
					//	else
					//		state->variables.color = C;
					//	}

					ns_to_voxel_space( &voxel->position, &V1, state->constants.voxel_info );

					//ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );
					NS_RENDER( state, point_func )( &N/*&( __DATA( voxel, N ) )*/, &V1, state );

					/* The voxel position falls in the center of the "square" in image and client
						space, but in this case we want it to fall in the corner. */
					//V1.x -= H.x;
					//V1.y -= H.y;
					//V1.z -= H.z;

					//ns_vector3f_add( &V2, &V1, &S );

					//NS_RENDER( state, rectangle_func )( &V1, &V2, state );
					}
				}
			}

		NS_RENDER_PRE_OR_POST( state, post_points_func );
		//NS_RENDER_PRE_OR_POST( state, post_rectangles_func );

		//state->constants.polygon_borders = polygon_borders;
		}
	else
		{
		SquareSetSize( ns_voxel_info_max_size( state->constants.voxel_info ) );
		SquareBegin();

		curr = ns_list_begin( &spines->list );
		end  = ns_list_end( &spines->list );

		for( ; ns_list_iter_not_equal( curr, end );
				 curr = ns_list_iter_next( curr ) )
			{
			spine = ns_list_iter_get_object( curr );

			if( ! spine->hidden || state->constants.draw_hidden_objects )
				{
				L = ns_render_state_spine_color( state, curr );

				curr_voxel = ns_list_begin( &spine->voxels );
				end_voxels = ns_list_end( &spine->voxels );

				for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
						 curr_voxel = ns_list_iter_next( curr_voxel ) )
					{
					voxel = ns_list_iter_get_object( curr_voxel );

					//if( state->constants.view_spine_maxima_voxels )
					//	{
					//	if( __GET( voxel, __MAXIMA ) )
					//		state->variables.color = NS_COLOR4UB_RED;
					//	else
					//		state->variables.color = C;
					//	}

					C = L;

					//if( __GET( voxel, __HIGH_ANGLE ) )
					//	C = NS_COLOR4UB_RED;

					ns_to_voxel_space( &voxel->position, &V1, state->constants.voxel_info );

					color[0] = C.x / 255.0f;
					color[1] = C.y / 255.0f;
					color[2] = C.z / 255.0f;
					color[3] = 1.0f;

					glNormal3b( __DATA( voxel, N ).x, __DATA( voxel, N ).y, __DATA( voxel, N ).z );

					SquareRender( v1, color );
					}
				}
			}

		SquareEnd();
		}
	}


void ns_model_render_spines_clumps_voxels
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	NsVoxel         *voxel;
	NsVector3f       V;
	nsfloat         *v;
	NsColor4ub       C;
	NsVector3b       N;
	nssize           num_colors;
   nsfloat          color[4];


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );

	ns_assert( NULL != NS_RENDER( state, point_func ) );

	N.x = N.y = N.z = 0;

	spines     = model->spines;
	num_colors = ns_spines_num_random_colors();

	v = ns_vector3f_array( &V );

	if( 2 == state->constants.dimensions )
		{
		state->variables.shape = NS_RENDER_SHAPE_POINT;
		NS_RENDER_PRE_OR_POST( state, pre_points_func );

		NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
			{
			if( ! ____xy_slice_enabled || voxel->position.z == ( nsint )____xy_slice_index )
				{
				state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );

				ns_to_voxel_space( &voxel->position, &V, state->constants.voxel_info );
				NS_RENDER( state, point_func )( &N, &V, state );
				}
			}

		NS_RENDER_PRE_OR_POST( state, post_points_func );
		}
	else
		{
		SquareSetSize( ns_voxel_info_max_size( state->constants.voxel_info ) );
		SquareBegin();

		NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
			{
			if( ! ____xy_slice_enabled || voxel->position.z == ( nsint )____xy_slice_index )
				{
				C = state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );

				/*TEMP*/
				//if( 0 == __DATA( voxel, clump_id ) )
				//	C = state->variables.color = NS_COLOR4UB_BLACK;
				//else if( __GET( voxel, __TOUCH_CLUMP0 ) )
				//	C = state->variables.color = NS_COLOR4UB_RED;
				//else
				//	C = state->variables.color = NS_COLOR4UB_GREEN;

				//if( __GET( voxel, __MAXIMA ) )
				//	C = state->variables.color = NS_COLOR4UB_RED;

				ns_to_voxel_space( &voxel->position, &V, state->constants.voxel_info );

				color[0] = C.x / 255.0f;
				color[1] = C.y / 255.0f;
				color[2] = C.z / 255.0f;
				color[3] = 1.0f;

				glNormal3b( __DATA( voxel, N ).x, __DATA( voxel, N ).y, __DATA( voxel, N ).z );

				SquareRender( v, color );
				}
			}

		SquareEnd();
		}
	}


void ns_model_render_spines_interface_voxels
   (
   const NsModel  *model,
   NsRenderState  *state
   )
	{
	const NsSpines  *spines;
	NsVoxel         *voxel;
	nslistiter       iter;
	NsVector3f       V;
	nsfloat         *v;
	NsColor4ub       C;
	NsVector3b       N;
	nssize           num_colors;
   nsfloat          color[4];


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );

	ns_assert( NULL != NS_RENDER( state, point_func ) );

	N.x = N.y = N.z = 0;

	spines     = model->spines;
	num_colors = ns_spines_num_random_colors();

	v = ns_vector3f_array( &V );

	if( 2 == state->constants.dimensions )
		{
		state->variables.shape = NS_RENDER_SHAPE_POINT;
		NS_RENDER_PRE_OR_POST( state, pre_points_func );

		NS_LIST_FOREACH( &spines->iface_voxels, iter )
			{
			voxel = ns_list_iter_get_object( iter );

			state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );

			ns_to_voxel_space( &voxel->position, &V, state->constants.voxel_info );
			NS_RENDER( state, point_func )( &N, &V, state );
			}

		NS_RENDER_PRE_OR_POST( state, post_points_func );
		}
	else
		{
		SquareSetSize( ns_voxel_info_max_size( state->constants.voxel_info ) );
		SquareBegin();

		NS_LIST_FOREACH( &spines->iface_voxels, iter )
			{
			voxel = ns_list_iter_get_object( iter );

			C = state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );

			ns_to_voxel_space( &voxel->position, &V, state->constants.voxel_info );

			color[0] = C.x / 255.0f;
			color[1] = C.y / 255.0f;
			color[2] = C.z / 255.0f;
			color[3] = 1.0f;

			glNormal3b( __DATA( voxel, N ).x, __DATA( voxel, N ).y, __DATA( voxel, N ).z );

			SquareRender( v, color );
			}

		SquareEnd();
		}
	}


/* TEMP!!!!!!!!!!!!!!!!!!! */
NS_PRIVATE void _ns_model_render_spines_voxels_as_rectangles
	(
	const NsList    *voxels,
	const NsSpines  *spines,
	NsRenderState   *state,
	nsboolean        layers,
	NsColor4ub       color
	)
	{
	nslistiter         curr_voxel;
	nslistiter         end_voxels;
	NsVoxel           *voxel;
	NsVector3f         V1, V2;
	const NsColor4ub  *colors;
	NsVector3f         O;


	static nsboolean ____rectangles_borders = NS_FALSE;


	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	colors = _ns_spines_get_layer_colors();

	O.x = ns_voxel_info_size_x( state->constants.voxel_info ) / 100.0f;
	O.y = ns_voxel_info_size_y( state->constants.voxel_info ) / 100.0f;
	O.z = ns_voxel_info_size_z( state->constants.voxel_info ) / 100.0f;

	if( ____rectangles_borders )
		state->variables.color = NS_COLOR4UB_BLACK;

	state->variables.shape = NS_RENDER_SHAPE_RECTANGLE;
	NS_RENDER_PRE_OR_POST( state, pre_rectangles_func );

	curr_voxel = ns_list_begin( voxels );
	end_voxels = ns_list_end( voxels );

	for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
			 curr_voxel = ns_list_iter_next( curr_voxel ) )
		{
		voxel = ns_list_iter_get_object( curr_voxel );

		if( spines->cutoff_layer < 0 /*|| __DATA( voxel, layer_index ) < spines->cutoff_layer*/ )
			{
			ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );
			ns_vector3f_add( &V2, &V1, ns_voxel_info_size( state->constants.voxel_info ) );

			/* Offset lines by a little. */
			if( ____rectangles_borders )
				{
				ns_vector3f_cmpd_sub( &V1, &O );
				ns_vector3f_cmpd_add( &V2, &O );
				}
			else
				{
				//if( layers )
				//	state->variables.color = colors[ __DATA( voxel, layer_color ) ];
				//else
				//	{
					//if( state->constants.view_spine_maxima_voxels )
					//	{
					//	if( __GET( voxel, __MAXIMA ) )
					//		state->variables.color = NS_COLOR4UB_RED;
					//	else
							state->variables.color = color;
					//	}
				//	}
				}

			NS_RENDER( state, rectangle_func )( &V1, &V2, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_rectangles_func );

	if( ! ____rectangles_borders )
		{
		____rectangles_borders  = NS_TRUE;
		state->variables.filled = NS_FALSE;

		_ns_model_render_spines_voxels_as_rectangles( voxels, spines, state, layers, color );
		}
	else
		{
		____rectangles_borders  = NS_FALSE;
		state->variables.filled = NS_TRUE;
		}
	}


void ns_model_render_spines_rectangles
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	spines = model->spines;

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			_ns_model_render_spines_voxels_as_rectangles(
				&spine->voxels,
				spines,
				state,
				NS_FALSE,
				ns_render_state_spine_color( state, curr )
				);
		}
/*
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;
	nslistiter       curr_voxel;
	nslistiter       end_voxels;	
	NsVoxel         *voxel;
	NsVector3f       V1, V2;
	NsColor4ub       C;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_RECTANGLE;
	NS_RENDER_PRE_OR_POST( state, pre_rectangles_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		state->variables.color = C = ns_render_state_spine_color( state, curr );

		curr_voxel = ns_list_begin( &spine->voxels );
		end_voxels = ns_list_end( &spine->voxels );

		for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
				 curr_voxel = ns_list_iter_next( curr_voxel ) )
			{
			voxel = ns_list_iter_get_object( curr_voxel );

			if( state->constants.view_spine_maxima_voxels )
				{
				if( __GET( voxel, __MAXIMA ) )
					state->variables.color = NS_COLOR4UB_RED;
				else
					state->variables.color = C;
				}

			ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );
			ns_vector3f_add( &V2, &V1, ns_voxel_info_size( state->constants.voxel_info ) );

			NS_RENDER( state, rectangle_func )( &V1, &V2, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_rectangles_func );*/
	}


void ns_model_render_spines_layers_as_rectangles
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines      *spines;
	const NsSpineLayer  *layer;
	nslistiter           curr;
	nslistiter           end;
	const NsSpine       *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	spines = model->spines;

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			_ns_model_render_spines_voxels_as_rectangles(
				&spine->voxels,
				spines,
				state,
				NS_TRUE,
				NS_COLOR4UB_BLACK
				);
		}

	NS_RENDER_PRE_OR_POST( state, post_rectangles_func );

	if( 0 < spines->cutoff_layer )
		{
		ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

		state->variables.color = NS_COLOR4UB_WHITE;
		state->variables.shape = NS_RENDER_SHAPE_AABBOX;

		NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

		curr = ns_list_begin( &spines->list );
		end  = ns_list_end( &spines->list );

		for( ; ns_list_iter_not_equal( curr, end );
				 curr = ns_list_iter_next( curr ) )
			{
			spine = ns_list_iter_get_object( curr );

			if( ! spine->hidden || state->constants.draw_hidden_objects )
				if( NULL != ( layer = _ns_spine_layer_at( spine, spines->cutoff_layer - 1 ) ) )
					NS_RENDER( state, aabbox_func )( &layer->B, state );
			}

		NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
		}
	}



void ns_model_render_spines_layers_as_points
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines    *spines;
	const NsVoxel     *voxel;
	nssize             num_colors;
	NsVector3f         V/*, O, D*/;
	NsVector3b         N;
	//const NsVector3i  *offsets;
	//NsVector3i         offset;


	N.x = N.y = N.z = 0;

	spines     = model->spines;
	num_colors = ns_spines_num_random_colors();
//	offsets    = ns_voxel_offsets();


	state->variables.shape = NS_RENDER_SHAPE_POINT;
	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );
			//0 < __DATA( voxel, clump_id ) ? NS_COLOR4UB_WHITE : NS_COLOR4UB_BLACK;

		//if( NULL == __DATA( voxel, spine ) && 0 != __DATA( voxel, clump_id ) )
		//	state->variables.color = NS_COLOR4UB_WHITE;

		//if( __GET( voxel, __MAXIMA ) )
		//	state->variables.color = NS_COLOR4UB_RED;

		if( ____xy_slice_enabled )
			{
			if( voxel->position.z == ( nsint )____xy_slice_index )
				{
				ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
				NS_RENDER( state, point_func )( &N/*&( __DATA( voxel, N ) )*/, &V, state );
				}
			}
		else
			{
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

			//if( 0 < __DATA( voxel, clump_id ) )
				NS_RENDER( state, point_func )( &N/*&( __DATA( voxel, N ) )*/, &V, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );



	//state->variables.shape = NS_RENDER_SHAPE_LINE;
	//state->variables.color = NS_COLOR4UB_WHITE;

	//NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	//NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
	//	{
	//	state->variables.color = ns_spines_random_color( __DATA( voxel, clump_id ) % num_colors );

		//if( ____xy_slice_enabled )
		//	{
		//	if( voxel->position.z == ( nsint )____xy_slice_index )
		//		{
		//		ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
		//		}
		//	}
		//else
		//	{
			//if( -1 != __DATA( voxel, parent_dir ) )
			//	{
			//	ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

				//ns_vector3i_rev( &offset, offsets + __DATA( voxel, parent_dir ) );
			//	offset = offsets[ __DATA( voxel, parent_dir ) ];

			//	ns_to_voxel_space( &offset, &O, spines->voxel_info );

			//	ns_vector3f_add( &D, &V, &O );

			//	NS_RENDER( state, line_func )( &V, &D, state );
			//	}
			//}
		//}

	//NS_RENDER_PRE_OR_POST( state, post_lines_func );


/*
	const NsSpines    *spines;
	nslistiter         curr;
	nslistiter         end;
	const NsSpine     *spine;
	nslistiter         curr_voxel;
	nslistiter         end_voxels;
	NsVoxel           *voxel;
	NsVector3f         V;
	const NsColor4ub  *colors;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	colors = _ns_spines_get_layer_colors();
	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_POINT;
	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			curr_voxel = ns_list_begin( &spine->voxels );
			end_voxels = ns_list_end( &spine->voxels );

			for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
					 curr_voxel = ns_list_iter_next( curr_voxel ) )
				{
				voxel = ns_list_iter_get_object( curr_voxel );

				ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

				state->variables.color = colors[ __DATA( voxel, layer_color ) ];

				NS_RENDER( state, point_func )( &( __DATA( voxel, N ) ), &V, state );
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
*/
	}


void ns_model_render_spines_candidates_as_points
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	NsVoxel         *voxel;
	//NsVector3f       V1;
	//NsVector3f       V2;
	NsVector3f       V;
	nsboolean        draw;
	nslistiter           curr_spine, end_spines;
	nslistiter           curr_layer, end_layers;
	const NsSpine       *spine;
	const NsSpineLayer  *layer;
	const NsColor4ub  *layer_colors;
	//nsuint direction;
	NsColor4ub direction_colors[ __NS_SPINES_NUM_DIRS ];
	NsVector3b N;



	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, line_func ) );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	layer_colors = _ns_spines_get_layer_colors();

	direction_colors[ _NS_SPINES_DIR_IN ] = NS_COLOR4UB_BLACK;//NS_COLOR4UB_WHITE;
	direction_colors[ _NS_SPINES_DIR_OUT_OR_PAR ] = NS_COLOR4UB_BLACK;//NS_COLOR4UB_WHITE;


	state->variables.shape = NS_RENDER_SHAPE_LINE;
	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

/*
	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );
		ns_vector3f_add( &V2, &V1, &( __DATA( voxel, gradient ) ) );

		direction = __DATA( voxel, direction );

		if( /*_NS_SPINES_NO_DIR != direction &&*//* direction < __NS_SPINES_NUM_DIRS
				&&
			 NULL != __DATA( voxel, spine )
			)
			{
			state->variables.color = direction_colors[ direction ];
			NS_RENDER( state, line_func )( &V1, &V2, state );
			}
		}*/



	state->variables.color = NS_COLOR4UB_BLUE;

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			curr_layer = ns_list_begin( &spine->layers );
			end_layers = ns_list_end( &spine->layers );

			for( ; ns_list_iter_not_equal( curr_layer, end_layers );
					 curr_layer = ns_list_iter_next( curr_layer ) )
				{
				layer = ns_list_iter_get_object( curr_layer );

				if( -1 == spines->render_layer || spines->render_layer == layer->index )
					{
					state->variables.color = layer_colors[ layer->index % _NS_SPINES_NUM_LAYER_COLORS ];
					NS_RENDER( state, line_func )( &layer->L.P1, &layer->L.P2, state );
					}
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );


/*
	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	ns_assert(
		state->constants.view_spine_candidate_voxels ||
		state->constants.view_spine_maxima_voxels
		);
*/
	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_POINT;
	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		if( NULL == __DATA( voxel, spine ) )
			{
			draw = NS_TRUE;
	
			state->variables.color = NS_COLOR4UB_BLACK;

			if( state->constants.view_spine_maxima_voxels )
				{
				if( __GET( voxel, __MAXIMA ) )
					state->variables.color = NS_COLOR4UB_WHITE;//NS_COLOR4UB_RED;
				//else if( 0 < __DATA( voxel, rejected ) )
				//	state->variables.color = NS_COLOR4UB_WHITE;
				else if( ! state->constants.view_spine_candidate_voxels )
					draw = NS_FALSE;
				}

			if( draw )
				{
				ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
				NS_RENDER( state, point_func )( &N/*&( __DATA( voxel, N ) )*/, &V, state );
				}
			}

	NS_RENDER_PRE_OR_POST( state, post_points_func );


/*
	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, point_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_RED;
	state->variables.shape = NS_RENDER_SHAPE_POINT;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	if( state->constants.view_spine_maxima_voxels )
		{
		curr_spine = ns_list_begin( &spines->list );
		end_spines = ns_list_end( &spines->list );

		for( ; ns_list_iter_not_equal( curr_spine, end_spines );
				 curr_spine = ns_list_iter_next( curr_spine ) )
			{
			spine = ns_list_iter_get_object( curr_spine );
			NS_RENDER( state, point_func )( &spine->max, state );
			}
		}
	else
		{
		NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
			if( __GET( voxel, __MAXIMA ) )
				{
				ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
				NS_RENDER( state, point_func )( &V, state );
				}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );*/
	}



/*
void ns_model_render_spines_candidates_gradients
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines    *spines;
	const NsVoxel     *voxel;
	const NsColor4ub  *colors;
	nssize             num_colors;
	NsVector3f         V;


	colors     = ns_spines_random_colors();
	num_colors = ns_spines_num_random_colors();
	spines     = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_POINT;
	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		if( -1 == __DATA( voxel, cluster ) )
			state->variables.color = NS_COLOR4UB_BLACK;
		else if( __DATA( voxel, is_sink ) )
			state->variables.color = NS_COLOR4UB_WHITE;
		else
			state->variables.color = colors[ ( nssize )__DATA( voxel, cluster ) % num_colors ];

		if( ! __GET( voxel, __MINIMA ) )
			{
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
			NS_RENDER( state, point_func )( &( __DATA( voxel, N ) ), &V, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}
*/










void ns_model_render_spines_candidates_as_rectangles
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	NsVoxel         *voxel;
	NsVector3f       V1;
	NsVector3f       V2;
	nsboolean        draw;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	ns_assert(
		state->constants.view_spine_candidate_voxels ||
		state->constants.view_spine_maxima_voxels
		);

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_RECTANGLE;
	NS_RENDER_PRE_OR_POST( state, pre_rectangles_func );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		if( NULL == __DATA( voxel, spine ) )
			{
			draw = NS_TRUE;
	
			state->variables.color = NS_COLOR4UB_BLACK;

			if( state->constants.view_spine_maxima_voxels )
				{
				if( __GET( voxel, __MAXIMA ) )
					state->variables.color = NS_COLOR4UB_RED;
				else if( ! state->constants.view_spine_candidate_voxels )
					draw = NS_FALSE;
				}

			if( draw )
				{
				ns_to_voxel_space( &voxel->position, &V1, spines->voxel_info );

				ns_vector3f_add( &V2, &V1, ns_voxel_info_size( state->constants.voxel_info ) );
				//ns_vector3f_cmpd_sub( &V1, ns_voxel_info_size( state->constants.voxel_info ) );
				//ns_vector3f_cmpd_add( &V2, ns_voxel_info_size( state->constants.voxel_info ) );

				NS_RENDER( state, rectangle_func )( &V1, &V2, state );
				}
			}

	NS_RENDER_PRE_OR_POST( state, post_rectangles_func );
	}


void ns_model_render_spines_dts
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;
	nslistiter       curr_voxel;
	nslistiter       end_voxels;	
	const NsVoxel   *voxel;
	NsVector3f       V;
	nschar           text[ 256 ];


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, text_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_TEXT;

	NS_RENDER_PRE_OR_POST( state, pre_texts_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		curr_voxel = ns_list_begin( &spine->voxels );
		end_voxels = ns_list_end( &spine->voxels );

		for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
				 curr_voxel = ns_list_iter_next( curr_voxel ) )
			{
			voxel = ns_list_iter_get_object( curr_voxel );
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

			ns_snprint( text, NS_ARRAY_LENGTH( text ), "%.3f", __DATA( voxel, distance ) );
			NS_RENDER( state, text_func )( &V, text, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_texts_func );
	}


void ns_model_render_spines_vectors
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines          *spines;
	nslistiter               curr;
	nslistiter               end;
	const NsSpine           *spine;
	//const NsSpineAxisPoint  *first, *last;
	NsVector3f               P;
	const NsSpineAxisPoint  *point;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, line_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_GREEN;
	state->variables.shape = NS_RENDER_SHAPE_LINE;

	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			//state->variables.color = NS_COLOR4UB_RED;
			//NS_RENDER( state, line_func )( &spine->max, &spine->center, state );
			//state->variables.color = NS_COLOR4UB_GREEN;
			//NS_RENDER( state, line_func )( &spine->center, &spine->base, state );
			//state->variables.color = NS_COLOR4UB_BLUE;
			//NS_RENDER( state, line_func )( &spine->base, &spine->attach, state );
			//state->variables.color = NS_COLOR4UB_WHITE;
			//NS_RENDER( state, line_func )( &spine->base, &spine->surface, state );

			/*switch( ns_spine_xyplane_angle_type( spine->xyplane_angle ) )
				{
				case NS_SPINE_XYPLANE_ANGLE_PARALLEL:
					state->variables.color = NS_COLOR4UB_GREEN;
					break;

				case NS_SPINE_XYPLANE_ANGLE_OBLIQUE:
					state->variables.color = NS_COLOR4UB_RED;
					break;

				case NS_SPINE_XYPLANE_ANGLE_UNKNOWN:
					state->variables.color = NS_COLOR4UB_BLACK;
					break;

				default:
					ns_assert_not_reached();
				}*/

			//NS_RENDER( state, line_func )( &spine->max, &spine->surface, state );

			if( ! ns_list_is_empty( &(spine->axis.points) ) )
				{
				point = ns_list_iter_get_object( ns_list_rev_begin( &spine->axis.points ) );
				P     = point->P;
				}
			else
				P = spine->center;

			NS_RENDER( state, line_func )( &P, &spine->attach, state );
		
			//if( ns_list_size( &(spine->axis.points) ) < 2 )
			//	NS_RENDER( state, line_func )( &spine->center, &spine->attach, state );
			//else
			//	{
				//first = ns_list_iter_get_object( ns_list_begin( &(spine->axis.points) ) );
				//last  = ns_list_iter_get_object( ns_list_rev_begin( &(spine->axis.points) ) );

				//NS_RENDER( state, line_func )( &first->P, &last->P, state );
				//NS_RENDER( state, line_func )( &last->P, &spine->attach, state );
			//	}

			//NS_RENDER( state, line_func )( &spine->center, &spine->base, state );
			//NS_RENDER( state, line_func )( &spine->base, &spine->attach, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );
	}


void ns_model_render_spines_axis
   (
   const NsModel  *model,
   NsRenderState  *state
   )
	{
	const NsSpines          *spines;
	nslistiter               curr_spine, end_spines;
	nslistiter               curr_point, next_point, end_points;
	const NsSpineAxisPoint  *point1, *point2;
	const NsSpine           *spine;
	//NsVector3f               V;
	//NsVector3b               N;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, line_func ) );

	//N.x = N.y = N.z = 0;

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_LINE;
	state->variables.color = NS_COLOR4UB_WHITE;

	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			curr_point = ns_list_begin( &spine->axis.points );
			end_points = ns_list_end( &spine->axis.points );

			for( ; ns_list_iter_not_equal( curr_point, end_points );
					 curr_point = ns_list_iter_next( curr_point ) )
				{
				next_point = ns_list_iter_next( curr_point );

				if( ns_list_iter_not_equal( next_point, end_points ) )
					{
					point1 = ns_list_iter_get_object( curr_point );
					point2 = ns_list_iter_get_object( next_point );

					NS_RENDER( state, line_func )( &point1->P, &point2->P, state );
					}
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );


/*
	state->variables.shape = NS_RENDER_SHAPE_POINT;
	state->variables.color = NS_COLOR4UB_RED;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			NS_RENDER( state, point_func )( &N/*&( __DATA( voxel, N ) )*//*, &spine->max, state );
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	*/
	}


void ns_model_render_spines_selected
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_AABBOX;

	NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ( ! spine->hidden || state->constants.draw_hidden_objects )
				&&
			 spine->is_selected )
			NS_RENDER( state, aabbox_func )( &spine->bounding_box, state );
		}

	NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
	}


/*
void ns_model_render_spines_samples
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines      *spines;
	nslistiter           curr_spine, end_spines;
	nslistiter           curr_layer, end_layers;
	const NsSpine       *spine;
	const NsSpineLayer  *layer;
	nssize               i;
	nssize               num_samples;
	const NsVector3f    *vectors;
	const NsColor4ub    *colors;
	NsVector3f           V;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, line_func ) );

	spines = model->spines;

	if( ! spines->save_layers )
		return;

	num_samples = ns_rayburst_num_samples( &spines->rb_layer_perimeter );
	vectors     = ns_rayburst_vectors( &spines->rb_layer_perimeter );
	colors      = _ns_spines_get_layer_colors();


	state->variables.shape = NS_RENDER_SHAPE_LINE;

	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			curr_layer = ns_list_begin( &spine->layers );
			end_layers = ns_list_end( &spine->layers );

			for( ; ns_list_iter_not_equal( curr_layer, end_layers );
					 curr_layer = ns_list_iter_next( curr_layer ) )
				{
				layer = ns_list_iter_get_object( curr_layer );

				if( layer->num_samples == num_samples )
					if( -1 == spines->render_layer || spines->render_layer == layer->index )
						{
						ns_assert( NULL != layer->distances );

						// state->variables.color = colors[ layer->index % _NS_SPINES_NUM_LAYER_COLORS ];
						state->variables.color = NS_COLOR4UB_RED;

						for( i = 0; i < num_samples; ++i )
							{
							V = vectors[i];

							ns_vector3f_cmpd_scale( &V, layer->distances[i] );
							ns_vector3f_cmpd_add( &V, &layer->center );

							NS_RENDER( state, line_func )( &layer->center, &V, state );
							}
						}
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );


	state->variables.shape = NS_RENDER_SHAPE_LINE;
	state->constants.line_size += 4;

	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		curr_layer = ns_list_begin( &spine->layers );
		end_layers = ns_list_end( &spine->layers );

		for( ; ns_list_iter_not_equal( curr_layer, end_layers );
				 curr_layer = ns_list_iter_next( curr_layer ) )
			{
			layer = ns_list_iter_get_object( curr_layer );

			if( -1 == spines->render_layer || spines->render_layer == layer->index )
				{
				state->variables.color = NS_COLOR4UB_BLUE;
				NS_RENDER( state, line_func )( &layer->F, &layer->R, state );
				}
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );
*/

	/*
	state->variables.color = NS_COLOR4UB_GREEN;
	state->constants.point_size += 4;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr_spine = ns_list_begin( &spines->list );
	end_spines = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr_spine, end_spines );
			 curr_spine = ns_list_iter_next( curr_spine ) )
		{
		spine = ns_list_iter_get_object( curr_spine );

		curr_layer = ns_list_begin( &spine->layers );
		end_layers = ns_list_end( &spine->layers );

		for( ; ns_list_iter_not_equal( curr_layer, end_layers );
				 curr_layer = ns_list_iter_next( curr_layer ) )
			{
			layer = ns_list_iter_get_object( curr_layer );
			NS_RENDER( state, point_func )( &layer->center, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	*/
//	}


nsfloat _ns_model_spine_fixed_render_radius( NsSpineType type )
	{
	nsfloat radius;

	switch( type )
		{
		case NS_SPINE_THIN:
			radius = NS_MODEL_SPINES_FIXED_RADIUS * 0.9f;
			break;

		case NS_SPINE_MUSHROOM:
			radius = NS_MODEL_SPINES_FIXED_RADIUS * 1.1f;
			break;

		default:
			radius = NS_MODEL_SPINES_FIXED_RADIUS;
		}

	return radius;
	}


nsfloat ns_model_spine_render_radius( nsspine S, NsSpineRenderMode mode )
	{  return ( nsfloat )ns_model_spine_render_radius_ex( S, mode );  }


nsdouble ns_model_spine_render_radius_ex( nsspine S, NsSpineRenderMode mode )
	{
	const NsSpine  *spine;
	nsdouble        radius;


	spine  = ns_list_iter_get_object( S );
	radius = 0.0;

	switch( mode )
		{
		case NS_SPINE_RENDER_MODE_SOLID_FIXED_ELLIPSE:
		case NS_SPINE_RENDER_MODE_HOLLOW_FIXED_ELLIPSE:
			radius = ( nsdouble )_ns_model_spine_fixed_render_radius( spine->type );
			break;

		case NS_SPINE_RENDER_MODE_SOLID_HEAD_DIAM_ELLIPSE:
		case NS_SPINE_RENDER_MODE_HOLLOW_HEAD_DIAM_ELLIPSE:
		case NS_SPINE_RENDER_MODE_SOLID_SURFACE:
		case NS_SPINE_RENDER_MODE_HOLLOW_SURFACE:
			radius = spine->stats.head_diameter / 2.0;
			break;
		}

	return radius;
	}


void ns_model_render_spines_fixed_spheres
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	nsfloat          radius;
	NsSpine         *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, sphere_func ) );

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_SPHERE;
	NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_spines_render_as_spheres_begin( state );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_spine_color( state, curr );

			radius = _ns_model_spine_fixed_render_radius( spine->type );

			_ns_model_spine_render_as_sphere( state, spine, radius );
			}
		}

	_ns_model_spines_render_as_spheres_end( state );

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );

	/* Draw points in 2D so that the spine is visible no matter
		what the zoom factor. */
	if( 2 == state->constants.dimensions )
		ns_model_render_spines_points( model, state );
	}


void ns_model_render_spines_centroids
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines      *spines;
	nslistiter           curr;
	nslistiter           end;
	const NsSpinePoint  *point;
	NsVector3b           N;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_POINT;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr = ns_list_begin( &spines->centroids );
	end  = ns_list_end( &spines->centroids );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		point = ns_list_iter_get_object( curr );
		NS_RENDER( state, point_func )( &N, &point->P, state );
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}


void ns_model_render_spines_borderers
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines      *spines;
	nslistiter           curr;
	nslistiter           end;
	const NsSpinePoint  *point;
	NsVector3b           N;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );

	N.x = N.y = N.z = 0;

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_GREEN;
	state->variables.shape = NS_RENDER_SHAPE_POINT;

	NS_RENDER_PRE_OR_POST( state, pre_points_func );

	curr = ns_list_begin( &spines->borderers );
	end  = ns_list_end( &spines->borderers );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		point = ns_list_iter_get_object( curr );
		NS_RENDER( state, point_func )( &N, &point->P, state );
		}

	NS_RENDER_PRE_OR_POST( state, post_points_func );
	}


void ns_model_render_spine_head_and_neck_centers( const NsModel *model, NsRenderState *state )
	{
	const NsSpines  *spines;
	nslistiter       curr, end;
	NsSpine         *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, sphere_func ) );

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_SPHERE;
	NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( spine->has_head_center )
			{
			state->variables.color = NS_COLOR4UB_WHITE;
			NS_RENDER( state, sphere_func )( &spine->head_center, NS_MODEL_SPINES_FIXED_RADIUS / 2.0f, state );
			}

		if( spine->has_neck_center )
			{
			state->variables.color = NS_COLOR4UB_CYAN;
			NS_RENDER( state, sphere_func )( &spine->neck_center, NS_MODEL_SPINES_FIXED_RADIUS / 2.0f, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );
	}


void ns_model_render_spines_ellipses_hd
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	NsSpine         *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, sphere_func ) );

	spines = model->spines;

	state->variables.shape = NS_RENDER_SHAPE_SPHERE;
	NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

	_ns_model_spines_render_as_spheres_begin( state );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_spine_color( state, curr );
			_ns_model_spine_render_as_sphere( state, spine, ( nsfloat )( spine->stats.head_diameter / 2.0 ) );
			}
		}

	_ns_model_spines_render_as_spheres_end( state );

	NS_RENDER_PRE_OR_POST( state, post_spheres_func );

	/* Draw points in 2D so that the spine is visible no matter
		what the zoom factor. */
	if( 2 == state->constants.dimensions )
		ns_model_render_spines_points( model, state );
	}


void ns_model_render_spines_ball_and_stick
	(
   const NsModel  *model,
   NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	nsfloat          head_radius;
	nsfloat          neck_radius;
	nsboolean        draw_head;
	nsboolean        draw_neck;
	NsVector3f       V;
	NsVector3f       P;
	nsboolean        polygon_borders;
	const NsSpine   *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, sphere_func ) );
	ns_assert( NULL != NS_RENDER( state, frustum_func ) );

	spines = model->spines;

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->hidden || state->constants.draw_hidden_objects )
			{
			state->variables.color = ns_render_state_spine_color( state, curr );

			polygon_borders = NS_TRUE;

			draw_neck   = draw_head   = NS_TRUE;
			neck_radius = head_radius = NS_MODEL_SPINES_FIXED_RADIUS;

			switch( spine->type )
				{
				case NS_SPINE_THIN:
					neck_radius = NS_MODEL_SPINES_FIXED_RADIUS * 0.25f;
					head_radius = NS_MODEL_SPINES_FIXED_RADIUS * 0.85f;
					break;

				case NS_SPINE_MUSHROOM:
					neck_radius = NS_MODEL_SPINES_FIXED_RADIUS * 0.5f;
					head_radius = NS_MODEL_SPINES_FIXED_RADIUS * 1.65f;
					break;

				case NS_SPINE_STUBBY:
					neck_radius = NS_MODEL_SPINES_FIXED_RADIUS * 1.25f;
					head_radius = NS_MODEL_SPINES_FIXED_RADIUS * 1.25f;
					break;

				default:
					draw_neck = NS_FALSE;
				}

			if( draw_neck )
				{
				if( ! state->variables.filled || NS_SPINE_STUBBY == spine->type )
					{
					ns_vector3f_scale(
						&V,
						&spine->approx_axis,
						NS_SPINE_STUBBY == spine->type ? head_radius : head_radius * 2.0f
						);

					ns_vector3f_add( &P, &spine->max, &V );
					}
				else
					P = spine->max;

				state->variables.shape = NS_RENDER_SHAPE_FRUSTUM;
				NS_RENDER_PRE_OR_POST( state, pre_frustums_func );

				NS_RENDER( state, frustum_func )(
					&P,
					neck_radius,
					&spine->surface,
					neck_radius,
					state
					);

				NS_RENDER_PRE_OR_POST( state, post_frustums_func );
				}

			if( draw_head )
				{
				ns_vector3f_scale( &V, &spine->approx_axis, head_radius * 0.9f );
				ns_vector3f_add( &P, &spine->max, &V );

				if( NS_SPINE_STUBBY == spine->type )
					{
					polygon_borders = state->constants.polygon_borders;
					state->constants.polygon_borders = NS_FALSE;
					}

				state->variables.shape = NS_RENDER_SHAPE_SPHERE;
				NS_RENDER_PRE_OR_POST( state, pre_spheres_func );

				NS_RENDER( state, sphere_func )( &P, head_radius, state );

				NS_RENDER_PRE_OR_POST( state, post_spheres_func );

				if( NS_SPINE_STUBBY == spine->type )
					state->constants.polygon_borders = polygon_borders;
				}
			}
		}
	}


void ns_model_render_spines_fixed_selected
	(
	const NsModel  *model,
	NsRenderState  *state
	)
	{
	const NsSpines  *spines;
	nslistiter       curr;
	nslistiter       end;
	const NsSpine   *spine;
	NsVector3d       C;
	NsAABBox3d       bbox;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );
	ns_assert( NULL != state );
	ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

	spines = model->spines;

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_AABBOX;

	NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ( ! spine->hidden || state->constants.draw_hidden_objects )
				&&
			 spine->is_selected )
			{
			ns_model_position_and_radius_to_aabbox_ex(
				ns_vector3f_to_3d( &spine->center, &C ),
				NS_MODEL_SPINES_FIXED_RADIUS * 2.0,
				&bbox
				);

			NS_RENDER( state, aabbox_func )( &bbox, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
	}


#include <ns-gl.h>
void ns_model_render_spines_merging
	(
	const NsModel  *model,
	NsRenderState  *state
	)
   {
	const NsSpines  *spines;
   nslistiter       curr, end;
   const NsSpine   *spine;
	NsVector3f       L;
	nsfloat          length;
	nsfloat          u1, u2, step;
	NsVector3f       P1, P2;
	nsfloat          r1, r2;


   ns_assert( NULL != model );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, frustum_func ) );

	state->variables.color = NS_COLOR4UB_RED;
   state->variables.shape = NS_RENDER_SHAPE_FRUSTUM;



//glDisable( GL_CULL_FACE );
//glEnable( GL_BLEND );
//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//state->variables.color.w = 128;



   NS_RENDER_PRE_OR_POST( state, pre_frustums_func );

	spines = model->spines;

	step = 1.0f / 50.0f;

	curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ( ! spine->hidden || state->constants.draw_hidden_objects )
				&&
			 ! spine->is_attached )
			{
			ns_vector3f_sub( &L, &spine->surface, &spine->base );
			length = ns_vector3f_mag( &L );
			
			ns_vector3f_norm( &L );

			u1 = 0.0f;
			u2 = u1 + step;

			while( u2 <= 1.0f )
				{
				ns_vector3f_scale( &P1, &L, u1 * length );
				ns_vector3f_scale( &P2, &L, u2 * length );

				ns_vector3f_cmpd_add( &P1, &spine->base );
				ns_vector3f_cmpd_add( &P2, &spine->base );

				r1 = ns_powf( u1, spines->merge_power ) * spines->merge_value;
				r2 = ns_powf( u2, spines->merge_power ) * spines->merge_value;

				if( 0.0f == u1 )
					r1 = 0.0001f;

		      NS_RENDER( state, frustum_func )( &P1, r1, &P2, r2, state );

				u1 = u2;
				u2 = u1 + step;
				}
			}
		}

   NS_RENDER_PRE_OR_POST( state, post_frustums_func );



//glDisable( GL_BLEND );
//glEnable( GL_CULL_FACE );
   }

