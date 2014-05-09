#include "oglwin.h"
#include "app_data.h"


void ns_roi_controls_init
	(
	NsRoiControls      *rc,
	nsushort            which_to_init,
	const NsCubei      *roi,
	const NsImage      *volume,
	const NsVoxelInfo  *voxel_info,
	nsushort            selected
	)
	{
	/* NOTE: +1 to include C2 edge voxels. */
	rc->C1.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.x,         voxel_info, NS_COMPONENT_X );
	rc->C1.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.y,         voxel_info, NS_COMPONENT_Y );
	rc->C1.z = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.z,         voxel_info, NS_COMPONENT_Z );
	rc->C2.x = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.x + 1 ), voxel_info, NS_COMPONENT_X );
	rc->C2.y = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.y + 1 ), voxel_info, NS_COMPONENT_Y );
	rc->C2.z = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.z + 1 ), voxel_info, NS_COMPONENT_Z );

	rc->dim[0] = ( nsint )ns_image_width( volume );
	rc->dim[1] = ( nsint )ns_image_height( volume );
	rc->dim[2] = ( nsint )ns_image_length( volume );

	rc->max = NS_MAX3_INDEX( rc->dim[0], rc->dim[1], rc->dim[2] );

	rc->sz = ns_to_voxel_space_component_ex( ( nsfloat )rc->dim[ rc->max ], voxel_info, ( nsint )rc->max ) / 50.0f;

	rc->w = rc->sz * NS_ROI_CONTROL_TIP_RADIUS_SCALAR * 2.0f;
	rc->h = rc->sz * NS_ROI_CONTROL_TIP_RADIUS_SCALAR * 2.0f;
	rc->l = rc->sz * NS_ROI_CONTROL_BASE_HEIGHT_SCALAR + rc->sz * NS_ROI_CONTROL_TIP_HEIGHT_SCALAR;

	if( 1 < rc->dim[2] )
		{
		rc->d2a = ( rc->C1.x + rc->C2.x ) * 0.5f;
		rc->d2b = ( rc->C1.y + rc->C2.y ) * 0.5f;

		if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_NEAR_BIT ) )
			{
			ns_vector3d( &( rc->L[ NS_ROI_CONTROL_NEAR_IDX ].P1 ), rc->d2a, rc->d2b, rc->C1.z );
			ns_vector3d( &( rc->L[ NS_ROI_CONTROL_NEAR_IDX ].P2 ), rc->d2a, rc->d2b, rc->C1.z - rc->l );
			}

		if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_FAR_BIT ) )
			{
			ns_vector3d( &( rc->L[ NS_ROI_CONTROL_FAR_IDX ].P1 ), rc->d2a, rc->d2b, rc->C2.z );
			ns_vector3d( &( rc->L[ NS_ROI_CONTROL_FAR_IDX ].P2 ), rc->d2a, rc->d2b, rc->C2.z + rc->l );
			}
		}

	rc->d1a = ( rc->C1.x + rc->C2.x ) * 0.5f;
	rc->d1b = ( rc->C1.z + rc->C2.z ) * 0.5f;

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_TOP_BIT ) )
		{
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_TOP_IDX ].P1 ), rc->d1a, rc->C1.y, rc->d1b );
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_TOP_IDX ].P2 ), rc->d1a, rc->C1.y - rc->l, rc->d1b );
		}

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_BOTTOM_BIT ) )
		{
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_BOTTOM_IDX ].P1 ), rc->d1a, rc->C2.y, rc->d1b );
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_BOTTOM_IDX ].P2 ), rc->d1a, rc->C2.y + rc->l, rc->d1b );
		}

	rc->d0a = ( rc->C1.y + rc->C2.y ) * 0.5f;
	rc->d0b = ( rc->C1.z + rc->C2.z ) * 0.5f;

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_LEFT_BIT ) )
		{
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_LEFT_IDX ].P1 ), rc->C1.x, rc->d0a, rc->d0b );
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_LEFT_IDX ].P2 ), rc->C1.x - rc->l, rc->d0a, rc->d0b );
		}

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_RIGHT_BIT ) )
		{
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_RIGHT_IDX ].P1 ), rc->C2.x, rc->d0a, rc->d0b );
		ns_vector3d( &( rc->L[ NS_ROI_CONTROL_RIGHT_IDX ].P2 ), rc->C2.x + rc->l, rc->d0a, rc->d0b );
		}

	if( 1 < rc->dim[2] )
		{
		if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_NEAR_BIT ) )
			ns_aabbox3d( rc->B + NS_ROI_CONTROL_NEAR_IDX, rc->d2a - rc->w / 2.0f, rc->d2b + rc->h / 2.0f, rc->C1.z, rc->w, rc->h, rc->l ); /* Near */

		if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_FAR_BIT ) )
			ns_aabbox3d( rc->B + NS_ROI_CONTROL_FAR_IDX, rc->d2a - rc->w / 2.0f, rc->d2b + rc->h / 2.0f, rc->C2.z + rc->l, rc->w, rc->h, rc->l ); /* Far */
		}

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_TOP_BIT ) )
		ns_aabbox3d( rc->B + NS_ROI_CONTROL_TOP_IDX, rc->d1a - rc->w / 2.0f, rc->C1.y, rc->d1b + rc->h / 2.0f, rc->w, rc->l, rc->h ); /* Top */

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_BOTTOM_BIT ) )
		ns_aabbox3d( rc->B + NS_ROI_CONTROL_BOTTOM_IDX, rc->d1a - rc->w / 2.0f, rc->C2.y + rc->l, rc->d1b + rc->h / 2.0f, rc->w, rc->l, rc->h ); /* Bottom */

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_LEFT_BIT ) )
		ns_aabbox3d( rc->B + NS_ROI_CONTROL_LEFT_IDX, rc->C1.x - rc->l, rc->d0a + rc->w / 2.0f, rc->d0b + rc->h / 2.0f, rc->l, rc->w, rc->h ); /* Left */

	if( ( nsboolean )( which_to_init & NS_ROI_CONTROL_RIGHT_BIT ) )
		ns_aabbox3d( rc->B + NS_ROI_CONTROL_RIGHT_IDX, rc->C2.x, rc->d0a + rc->w / 2.0f, rc->d0b + rc->h / 2.0f, rc->l, rc->w, rc->h ); /* Right */

	rc->selected = selected;
	}


UINT_PTR ____ogl_wheel_timer_id      = 0;
nsint ____ogl_curr_mouse_mode        = -1;
nsint ____ogl_lbutton_double_clicked = NS_FALSE;
nsboolean ____ogl_drag_rect_active   = NS_FALSE;

extern nsboolean ____optimize_splats_movement;
extern nsuint *____active_workspace;
extern OpenGLWindow *____ogl;
extern void _SetDisplayWindowCursor( HWND hWnd, const nschar* name, const nsint doSet );
extern nsboolean __progress_cancelled( NsProgress *progress );
extern void __progress_update( NsProgress *progress, nsfloat percent );
extern void __progress_set_title( NsProgress *progress, const nschar *title );
extern NsSplats ____splats;
extern GLdouble ____modelview_matrix[16];
extern GLdouble ____projection_matrix[16];
extern void _show_seed_on_status_bar( void );
extern void ____redraw_2d( void );
extern void ____redraw_3d( void );
extern void ____redraw_all( void );
extern nsboolean ____alt_key_is_down;
extern nsboolean ____alt_key_released;
extern nsint _main_window_on_keydown( nsint key );
extern nsint _main_window_on_keyup( nsint key );
extern void _mouse_mode_spine_do_add_or_delete( nsuint, const NsVector3f*, const NsVector3f*, const nsuint );
extern void _mouse_mode_spine_do_type_popup( HWND hWnd, nsuint workspace, nsint x, nsint y );
extern nsint ____jitter_x;
extern nsint ____jitter_y;
extern nsint ____jitter_z;
extern nschar ____config_file[];
extern NsRayburstKernelType ____measuring_rays_kernel_type;
extern NsRayburstInterpType ____measurement_interp_type;
extern nsboolean ____measurement_do_3d_rayburst;
extern void _on_convert_measurements( void );
extern void _mouse_mode_neurite_do_labeling_popup( HWND hWnd, nsuint workspace, nsint x, nsint y );
extern void _mouse_mode_roi_do_popup( HWND hWnd, nsuint workspace, nsint x, nsint y );
extern nsint ____spines_render_mode;
extern void _status_measurement( void );
extern nsint *____2d_display_type;
extern void _adjust_slice_viewer_due_to_roi_update( const NsCubei *roi );
extern void _do_adjust_seed_due_to_roi_update( nsuint workspace, const NsCubei *roi );
extern void _do_show_roi_on_status_bar( nsuint workspace );
extern nsboolean ____alert_user_about_splats;
extern void _open_gl_on_regenerate_splats( nsboolean );

static nsint ____ogl_zooming_in = 1;
#define ____ogl_allow_drag_rect ____ogl_zooming_in


void _force_opengl_is_dragging( nsboolean *old_value )
	{
	*old_value                 = ____ogl->is_mouse_dragging;
	____ogl->is_mouse_dragging = NS_TRUE;
	}


void _reset_opengl_is_dragging( nsboolean old_value )
	{  ____ogl->is_mouse_dragging = old_value;  }


void _ogl_window_mouse_mode_on_unset( HWND hWnd )
   {  SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )LoadCursor( NULL, IDC_ARROW ) );  }


void _ogl_window_mouse_mode_zoom_on_set( HWND hWnd )
   {  _SetDisplayWindowCursor( hWnd, "ZOOM_IN_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_seed_on_set( HWND hWnd )
   {  _SetDisplayWindowCursor( hWnd, "SEED_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_rays_on_set( HWND hWnd )
	{  _SetDisplayWindowCursor( hWnd, "RAY_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_spine_on_set( HWND hWnd )
	{  _SetDisplayWindowCursor( hWnd, "SPINE_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_neurite_on_set( HWND hWnd )
	{  _SetDisplayWindowCursor( hWnd, "NEURITE_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_roi_on_set( HWND hWnd )
	{
	//____ogl->roi_cursors[ _ROI_CURSOR_NW    ] = LoadCursor( NULL, IDC_SIZENWSE );
	//____ogl->roi_cursors[ _ROI_CURSOR_SE    ] = LoadCursor( NULL, IDC_SIZENWSE );
	//____ogl->roi_cursors[ _ROI_CURSOR_NE    ] = LoadCursor( NULL, IDC_SIZENESW );
	//____ogl->roi_cursors[ _ROI_CURSOR_SW    ] = LoadCursor( NULL, IDC_SIZENESW );
	//____ogl->roi_cursors[ _ROI_CURSOR_W     ] = LoadCursor( NULL, IDC_SIZEWE );
	//____ogl->roi_cursors[ _ROI_CURSOR_E     ] = LoadCursor( NULL, IDC_SIZEWE );
	//____ogl->roi_cursors[ _ROI_CURSOR_N     ] = LoadCursor( NULL, IDC_SIZENS );
	//____ogl->roi_cursors[ _ROI_CURSOR_S     ] = LoadCursor( NULL, IDC_SIZENS );
	____ogl->roi_cursors[ _ROI_CURSOR_ALL   ] = LoadCursor( NULL, IDC_SIZEALL );
	____ogl->roi_cursors[ _ROI_CURSOR_ARROW ] = LoadCursor( NULL, IDC_ARROW );
	____ogl->roi_cursors[ _ROI_CURSOR_CROSS ] = LoadCursor( NULL, IDC_CROSS );

   SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )____ogl->roi_cursors[ _ROI_CURSOR_ARROW ] );
	//_SetDisplayWindowCursor( hWnd, IDC_ARROW, 0 );

	____ogl->roi_curr_cursor = _ROI_CURSOR_ARROW;
	}


void _ogl_window_mouse_mode_magnet_on_set( HWND hWnd )
	{  _SetDisplayWindowCursor( hWnd, "MAGNET_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_drag_on_set( HWND hWnd )
   {   _SetDisplayWindowCursor( hWnd, "DRAG_TOOL_CURSOR", 0 );  }


void _ogl_window_mouse_mode_drag_on_mouse_move( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	if( ____ogl->is_lbutton_down )
		____ogl->is_mouse_dragging = NS_TRUE;

	if( UpdateInteractor( &____ogl->interactor, x, -y ) )
		RepaintOpenGLWindow( ____ogl );
	}


void _ogl_window_mouse_mode_drag_on_lbutton_down( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	____ogl->is_lbutton_down   = NS_TRUE;
	____ogl->is_mouse_dragging = NS_FALSE;

   StartInteractor( &____ogl->interactor, ____ogl->hWnd, x, -y );

   if( flags & MK_CONTROL )
		SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_MOVE );
	else if( flags & MK_SHIFT )
		SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_ZOOM );
	}


void _ogl_window_mouse_mode_drag_on_lbutton_up( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	____ogl->is_lbutton_down   = NS_FALSE;
	____ogl->is_mouse_dragging = NS_FALSE;

	EndInteractor( &____ogl->interactor );

	RepaintOpenGLWindow( ____ogl );
	}


void _ogl_window_mouse_mode_drag_on_rbutton_down( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
#define ____ROTATE_ID     76790
#define ____ZOOM_ID       76791
#define ____TRANSLATE_ID  76792
#define ____ROLL_ID       76793

	nsint         mode;
   MENUITEMINFO  miim;
   HMENU         hMenu;
	POINT         pt;
	nsint         id;


	mode = GetInteractorMode( &____ogl->interactor );

   hMenu = CreatePopupMenu();

   miim.cbSize = sizeof( MENUITEMINFO );
	miim.fMask  = MIIM_TYPE | MIIM_ID | MIIM_STATE;
	miim.fType  = MFT_STRING;

	miim.fState     = INTERACTOR_MODE_ROTATE == mode ?
							MFS_CHECKED | MFS_ENABLED : MFS_ENABLED;
   miim.wID        = ____ROTATE_ID;
   miim.dwTypeData = "Rotate";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Rotate" );
   InsertMenuItem( hMenu, 0, TRUE, &miim );

	miim.fState     = INTERACTOR_MODE_ZOOM == mode ?
							MFS_CHECKED | MFS_ENABLED : MFS_ENABLED;
   miim.wID        = ____ZOOM_ID;
   miim.dwTypeData = "Zoom";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Zoom" );
   InsertMenuItem( hMenu, 1, TRUE, &miim );

	miim.fState     = INTERACTOR_MODE_MOVE == mode ?
							MFS_CHECKED | MFS_ENABLED : MFS_ENABLED;
   miim.wID        = ____TRANSLATE_ID;
   miim.dwTypeData = "Translate";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Translate" );
   InsertMenuItem( hMenu, 2, TRUE, &miim );

	miim.fState     = INTERACTOR_MODE_ROLL == mode ?
							MFS_CHECKED | MFS_ENABLED : MFS_ENABLED;
   miim.wID        = ____ROLL_ID;
   miim.dwTypeData = "Roll";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Roll" );
   InsertMenuItem( hMenu, 3, TRUE, &miim );

   pt.x = x;
   pt.y = y;

   ClientToScreen( ____ogl->hWnd, &pt );

   id = TrackPopupMenu( hMenu,                  
                        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                        pt.x,
                        pt.y,
                        0,
                        ____ogl->hWnd,
                        NULL
                      );

	switch( id )
		{
		case ____ROTATE_ID:
			SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_ROTATE );
			break;

		case ____ZOOM_ID:
			SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_ZOOM );
			break;

		case ____TRANSLATE_ID:
			SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_MOVE );
			break;

		case ____ROLL_ID:
			SetInteractorMode( &____ogl->interactor, INTERACTOR_MODE_ROLL );
			break;
		}

   DestroyMenu( hMenu );
	}


void _ogl_window_mouse_mode_seed_on_mouse_move( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	/* TODO? */
	}


void _ogl_window_mouse_mode_seed_on_lbutton_down( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	/* TODO? */
	}


void _ogl_window_mouse_mode_seed_do_find( void *dialog )
	{
   Progress    progress;
   NsProgress  nsprogress;
	NsError     error;


   g_Progress = dialog;

   ns_progress(
		&nsprogress,
		 __progress_cancelled,
		 __progress_update,
		 __progress_set_title,
		 NULL,
		 &progress
		);

	if( NS_FAILURE(
			ns_seed_find_3d(
				workspace_get_voxel_info( *____active_workspace ),
				workspace_volume( *____active_workspace ),
				workspace_visual_roi( *____active_workspace ),
				&____splats,
				&____ogl->ray,
				&____ogl->seed,
				&____ogl->found,
				&nsprogress
				),
			error ) )
		{
		EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
		return;
		}

   EndProgressDialog( dialog, eNO_ERROR );
	}


eERROR_TYPE _ogl_window_mouse_mode_seed_find( HWND wnd )
	{
	eERROR_TYPE  error;
	nsint        was_cancelled;


	/* IMPORTANT: This function makes OpenGL calls, so it must be called in the
		proper thread, i.e. not the thread launched by the progress bar. */

	ns_seed_find_get_ray(
		____ogl->mouseX,
		____ogl->mouseY,
		____modelview_matrix,
		____projection_matrix,
		&____ogl->ray
		);

	error =
		ProgressDialog(
			wnd,
			NULL,
			_ogl_window_mouse_mode_seed_do_find,
			NULL,
			NULL,
			&was_cancelled
			);

	if( ! was_cancelled && eNO_ERROR != error )
		return error;

	return eNO_ERROR;
	}


eERROR_TYPE _ogl_window_mouse_mode_seed_find_ex( HWND wnd, nsint x, nsint y, NsVector3i *V, nsboolean *found )
	{
	eERROR_TYPE  error;
	nsint        was_cancelled;


	/* IMPORTANT: This function makes OpenGL calls, so it must be called in the
		proper thread, i.e. not the thread launched by the progress bar. */

	ns_seed_find_get_ray(
		x,
		y,
		____modelview_matrix,
		____projection_matrix,
		&____ogl->ray
		);

	error =
		ProgressDialog(
			wnd,
			NULL,
			_ogl_window_mouse_mode_seed_do_find,
			NULL,
			NULL,
			&was_cancelled
			);

	if( ! was_cancelled && eNO_ERROR != error )
		return error;

	*V     = ____ogl->seed;
	*found = ____ogl->found;

	return eNO_ERROR;
	}


void _ogl_window_mouse_mode_seed_on_lbutton_up( HWND wnd, nsint x, nsint y, const nsuint flags )
	{
	Vector3i seed;

	_ogl_window_mouse_mode_seed_find( wnd );

	if( ____ogl->found &&
		 ns_point3i_inside_cube( &____ogl->seed, workspace_update_roi( *____active_workspace ) ) )
		{
		seed.x = ____ogl->seed.x;
		seed.y = ____ogl->seed.y;
		seed.z = ____ogl->seed.z;

		SetWorkspaceSeed( *____active_workspace, &seed );

		_show_seed_on_status_bar();

		____redraw_all();

		ns_model_spines_print_clump_mergers( GetWorkspaceNeuronTree( *____active_workspace ), seed.x, seed.y, seed.z );
		}
	}


void _ogl_window_on_begin_drag_rect( HWND hWnd, nsint x, nsint y )
	{
   ns_drag_rect_bounds(
      &____ogl->drag_rect,
      NS_INT_MIN,
      NS_INT_MIN,
      NS_INT_MAX,
      NS_INT_MAX
      );

   SetCapture( hWnd );
   ____ogl->hasCapture = 1;

   if( ____ogl_allow_drag_rect )
      {
      ns_drag_rect_start( &____ogl->drag_rect, x, y );

      ns_drag_rect_set_tolerance(
         &____ogl->drag_rect,
         eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == ____ogl_curr_mouse_mode ? 6 : 0
         );

      ____ogl_drag_rect_active = NS_TRUE;
      }

   ____ogl->isDragging = 1;
	}


void _ogl_window_on_move_drag_rect( HWND hWnd, nsint x, nsint y )
   {
   if( ____ogl_allow_drag_rect && ____ogl->isDragging )
      {
      DragRectRenderer state;
   
      drag_rect_renderer_init( &state, hWnd );

      ns_drag_rect_erase( &____ogl->drag_rect, drag_rect_renderer_line, &state );
      ns_drag_rect_end( &____ogl->drag_rect, x, y );
      ns_drag_rect_draw( &____ogl->drag_rect, drag_rect_renderer_line, &state );

      drag_rect_renderer_finalize( &state );
      }
   }


nsboolean _ogl_window_on_end_drag_rect( HWND hWnd, nsint x, nsint y )
   {
   DragRectRenderer state;

   if( ____ogl_allow_drag_rect )
      {
      ____ogl_drag_rect_active = NS_FALSE;

      drag_rect_renderer_init( &state, hWnd );

      ns_drag_rect_erase( &____ogl->drag_rect, drag_rect_renderer_line, &state );
      ns_drag_rect_end( &____ogl->drag_rect, x, y );

      drag_rect_renderer_finalize( &state );
      }

   if( ____ogl->hasCapture )
      {
      ____ogl->hasCapture = 0;
      ReleaseCapture();
      }

   ____ogl->isDragging = 0;

   if( ____ogl_allow_drag_rect && ns_drag_rect_valid( &____ogl->drag_rect ) )
      {
      ns_drag_rect_clip( &____ogl->drag_rect );
      ns_drag_rect_normalize( &____ogl->drag_rect );

      if( ns_drag_rect_valid( &____ogl->drag_rect ) )
         return NS_TRUE;
      }

	return NS_FALSE;
   }


void _ogl_window_cancel_alt_drag( void )
   {
   if( ____ogl_drag_rect_active )
      {
      ____ogl_drag_rect_active = NS_FALSE;

      if( ____ogl->hasCapture )
         {
         ____ogl->hasCapture = 0;
         ReleaseCapture();
         }

      ____ogl->isDragging = 0;

      ____alt_key_released = NS_TRUE;

      ____redraw_3d();
      }
   }


void _ogl_window_mouse_mode_rays_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags );


void _ogl_window_mouse_mode_rays_get_point( HWND hWnd, nsint mx, nsint my, NsVector3d *P )
	{
   GLint viewport[4];

   glGetIntegerv( GL_VIEWPORT, viewport );

	gluUnProject(
		( GLdouble )( mx ),
		( GLdouble )( ____ogl->height - my - 1 ), /* Window coordinate axis reversed in OpenGL. */
		0.5,
		____modelview_matrix,
		____projection_matrix,
		viewport,
		&P->x,
		&P->y,
		&P->z
		);
	}


void _ogl_window_mouse_mode_spine_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	if( ! ____ogl_wheel_timer_id )
		{
		if( ____ogl_drag_rect_active )
			_ogl_window_on_move_drag_rect( hWnd, x, y );
		else
			_ogl_window_mouse_mode_rays_on_mouse_move( hWnd, x, y, flags );
		}
	}


void _ogl_window_mouse_mode_spine_on_lbutton_down_get_center( HWND hWnd, nsint mx, nsint my, const nsuint flags )
   {
   _ogl_window_mouse_mode_rays_get_point( hWnd, mx, my, &____ogl->old_voxel );

	____ogl->old_client.x = mx;
	____ogl->old_client.y = my;

   SetCapture( hWnd );

   ____ogl->hasCapture = 1;
   ____ogl->isDragging = 1;
	____ogl->is_mouse_dragging = NS_TRUE;
   }


void _ogl_window_mouse_mode_spine_on_lbutton_up_get_attachment( HWND hWnd, nsint mx, nsint my, const nsuint flags )
   {
	NsVector3i          V;
	NsVector3f          P, A;
	nsboolean           found;
	const NsVoxelInfo  *voxel_info;
	const NsCubei      *roi;


	if( ____ogl->isDragging )
		{
		voxel_info = workspace_get_voxel_info( *____active_workspace );
		roi        = workspace_update_roi( *____active_workspace );

		//_ogl_window_mouse_mode_rays_get_point( hWnd, mx, my, &____ogl->new_voxel );

		____ogl->hasCapture = 0;
		____ogl->isDragging = 0;
		____ogl->is_mouse_dragging = NS_FALSE;

		ReleaseCapture();

		____ogl->new_client.x = mx;
		____ogl->new_client.y = my;

		_ogl_window_mouse_mode_seed_find_ex( hWnd, ____ogl->old_client.x, ____ogl->old_client.y, &V, &found );

		/* The center must be found and be in the ROI. */
		if( ! found || ! ns_point3i_inside_cube( &V, roi ) )
			{
			RepaintOpenGLWindow( ____ogl );
			return;
			}

		ns_to_voxel_space( &V, &P, voxel_info );

		_ogl_window_mouse_mode_seed_find_ex( hWnd, ____ogl->new_client.x, ____ogl->new_client.y, &V, &found );

		/* The "attachment finder point" just has to be in the ROI. */
		if( found && ! ns_point3i_inside_cube( &V, roi ) )
			{
			RepaintOpenGLWindow( ____ogl );
			return;
			}

		if( found )
			ns_to_voxel_space( &V, &A, voxel_info );

		ns_print_newline();//TEMP
		_mouse_mode_spine_do_add_or_delete( *____active_workspace, &P, found ? &A : NULL, flags );
		}
   }


void _ogl_window_mouse_mode_spine_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	if( ! ____ogl_wheel_timer_id )
		{
		if( ____alt_key_is_down )
			{
			____ogl_drag_rect_active = NS_TRUE;
			_ogl_window_on_begin_drag_rect( hWnd, x, y );
			}
		else if( ! ____alt_key_released )
			_ogl_window_mouse_mode_spine_on_lbutton_down_get_center( hWnd, x, y, flags );
		}
	}


NsSpineRenderMode _ogl_gui_render_mode_to_spine_render_mode( void )
	{
	NsSpineRenderMode mode = NS_SPINE_RENDER_MODE_OFF;

	switch( ____spines_render_mode )
		{
		case IDM_VIEW_SPINE_MODE_SOLID_ELLIPSES:
			mode = NS_SPINE_RENDER_MODE_SOLID_FIXED_ELLIPSE;
			break;

		case IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSES:
			mode = NS_SPINE_RENDER_MODE_HOLLOW_FIXED_ELLIPSE;
			break;

		case IDM_VIEW_SPINE_MODE_SOLID_ELLIPSE_HEAD_DIAM:
			mode = NS_SPINE_RENDER_MODE_SOLID_HEAD_DIAM_ELLIPSE;
			break;

		case IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSE_HEAD_DIAM:
			mode = NS_SPINE_RENDER_MODE_HOLLOW_HEAD_DIAM_ELLIPSE;
			break;

		case IDM_VIEW_SPINE_MODE_SOLID_SURFACE:
			mode = NS_SPINE_RENDER_MODE_SOLID_SURFACE;
			break;

		case IDM_VIEW_SPINE_MODE_HOLLOW_SURFACE:
			mode = NS_SPINE_RENDER_MODE_HOLLOW_SURFACE;
			break;
		}

	return mode;
	}


void _ogl_window_mouse_mode_spine_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( _ogl_window_on_end_drag_rect( hWnd, x, y ) )
      {
		NsProjector proj;

		ns_projector_init(
			&proj,
			( nsdouble )____ogl->height,
			____ogl->zNear,
			____ogl->zFar,
			____ogl->fieldOfView,
			____modelview_matrix,
			____projection_matrix
			);

      ns_model_select_spines_by_projection_onto_drag_rect(
			workspace_filtered_model( *____active_workspace ),
			&____ogl->drag_rect,
			&proj,
			_ogl_gui_render_mode_to_spine_render_mode()
			);

		____redraw_all();
      }
   }


void _ogl_window_mouse_mode_spine_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	//if( ! ____lbutton_double_clicked )
	if( ! ____ogl_wheel_timer_id )
		{
		if( ____alt_key_is_down && ____ogl_drag_rect_active )
			_ogl_window_mouse_mode_spine_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			_ogl_window_mouse_mode_spine_on_lbutton_up_get_attachment( hWnd, x, y, flags );

		____alt_key_released = NS_FALSE;
		}
	}


void _ogl_window_mouse_mode_spine_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{  _mouse_mode_spine_do_type_popup( hWnd, *____active_workspace, x, y );  }


void _ogl_window_mouse_mode_neurite_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   if( ____ogl_drag_rect_active )
      _ogl_window_on_move_drag_rect( hWnd, x, y );
	}


void _ogl_window_mouse_mode_neurite_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   if( ____alt_key_is_down && ! ____ogl_wheel_timer_id )
      {
      ____ogl_drag_rect_active = NS_TRUE;
      _ogl_window_on_begin_drag_rect( hWnd, x, y );
      }
	}


void _ogl_window_mouse_mode_neurite_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( _ogl_window_on_end_drag_rect( hWnd, x, y ) )
      {
		NsProjector proj;

/*ns_println(
	"DRAG RECT = %d,%d to %d,%d",
	____ogl->drag_rect.start_x, ____ogl->drag_rect.start_y,
	____ogl->drag_rect.end_x, ____ogl->drag_rect.end_y
	);*/

		ns_projector_init(
			&proj,
			( nsdouble )____ogl->height,
			____ogl->zNear,
			____ogl->zFar,
			____ogl->fieldOfView,
			____modelview_matrix,
			____projection_matrix
			);

      ns_model_select_vertices_by_projection_onto_drag_rect(
			workspace_filtered_model( *____active_workspace ),
			&____ogl->drag_rect,
			&proj
			);

		____redraw_all();
      }
   }


/* NOTE: Currently this routine does not operate as it does in 2D. This 3D version just adds a single vertex. */
void _ogl_window_mouse_mode_neurite_on_lbutton_up_do_neurite_tracer( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	NsVector3f         P;
   NsVector3i         jitter;
   nsboolean          use_2d_sampling;
   nsfloat            aabbox_scalar;
   nsint              min_window;
   NsSampler         *sampler;
   const NsSettings  *settings;
   nsfloat            radius, length;
   nsboolean          did_create_sample;
	NsError            error;


   ____ogl->isDragging = 0;
	____ogl->hasCapture = 0;
   ReleaseCapture();

	_ogl_window_mouse_mode_seed_find( hWnd );

	if( ____ogl->found &&
		 ns_point3i_inside_cube( &____ogl->seed, workspace_update_roi( *____active_workspace ) ) )
		{
		ns_to_voxel_space( &____ogl->seed, &P, workspace_get_voxel_info( *____active_workspace ) );

		ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );

		sampler    = workspace_sampler( *____active_workspace );
		settings   = workspace_settings( *____active_workspace );

		if( 0 < ns_sampler_size( sampler ) )
			if( IDNO == MessageBox( hWnd,
				"Existing measurements may not be connected properly. Proceed?", "NeuronStudio",
				MB_YESNO | MB_ICONQUESTION ) )
				return;

		ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

		if( NS_FAILURE(
				ns_sampler_run(
					sampler,
					workspace_volume( *____active_workspace ),
					&P,
					&P,
					0,
					____measurement_do_3d_rayburst,
					____measuring_rays_kernel_type,
					____measurement_interp_type,
					workspace_get_voxel_info( *____active_workspace ),
					workspace_get_average_intensity( *____active_workspace ),
					&jitter,
					use_2d_sampling,
					aabbox_scalar,
					min_window,
					ns_settings_get_threshold_use_fixed( settings ),
					ns_settings_get_threshold_fixed_value( settings ),
					&radius,
					&length,
					&did_create_sample
					),
				error ) )
			{
			ns_println( "not enough memory to complete sample" );
			return;
			}

		_on_convert_measurements();
		____redraw_all();
		}
	}


void _ogl_window_mouse_mode_neurite_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	//if( ! ____lbutton_double_clicked )
		{
		if( ____alt_key_is_down && ____ogl_drag_rect_active )
			_ogl_window_mouse_mode_neurite_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			_ogl_window_mouse_mode_neurite_on_lbutton_up_do_neurite_tracer( hWnd, x, y, flags );

		____alt_key_released = NS_FALSE;

		____ogl->isDragging = 0;
		}
	}


void _ogl_window_mouse_mode_neurite_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{ _mouse_mode_neurite_do_labeling_popup( hWnd, *____active_workspace, x, y );  }


void _ogl_window_mouse_mode_rays_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   if( ____ogl_drag_rect_active )
      _ogl_window_on_move_drag_rect( hWnd, x, y );
	else if( ____ogl->isDragging )
      {
      _ogl_window_mouse_mode_rays_get_point( hWnd, x, y, &____ogl->new_voxel );
      RepaintOpenGLWindow( ____ogl );
      }
	}


void _ogl_window_mouse_mode_rays_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   if( _ogl_window_on_end_drag_rect( hWnd, x, y ) )
      {
		NsProjector proj;

		ns_projector_init(
			&proj,
			( nsdouble )____ogl->height,
			____ogl->zNear,
			____ogl->zFar,
			____ogl->fieldOfView,
			____modelview_matrix,
			____projection_matrix
			);

      ns_sampler_select_by_projection_onto_drag_rect(
			workspace_sampler( *____active_workspace ),
			&____ogl->drag_rect,
			&proj
			);

		____redraw_all();
      }
	}


void _ogl_window_mouse_mode_rays_on_lbutton_up_do_run_samples( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   nsfloat            radius, length;
   NsSampler         *sampler;
   nspointer          prev_sample, curr_sample;
   const NsSettings  *settings;
   nsboolean          did_create_sample;
   nsboolean          use_2d_sampling;
   nsfloat            aabbox_scalar;
   nsint              min_window;
   NsVector3i         jitter;
   NsVector3f         P;
   NsError            error;


   ____ogl->isDragging = 0;
	____ogl->hasCapture = 0;
   ReleaseCapture();

	_ogl_window_mouse_mode_seed_find( hWnd );

	if( ____ogl->found &&
		 ns_point3i_inside_cube( &____ogl->seed, workspace_update_roi( *____active_workspace ) ) )
		{
		ns_to_voxel_space( &____ogl->seed, &P, workspace_get_voxel_info( *____active_workspace ) );

		ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );

		sampler     = workspace_sampler( *____active_workspace );
		prev_sample = ns_sampler_is_empty( sampler ) ? NULL : ns_sampler_last( sampler );
		settings    = workspace_settings( *____active_workspace );

		ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

		if( NS_FAILURE(
				ns_sampler_run(
					sampler,
					workspace_volume( *____active_workspace ),
					&P,
					&P,
					0,
					____measurement_do_3d_rayburst,
					____measuring_rays_kernel_type,
					____measurement_interp_type,
					workspace_get_voxel_info( *____active_workspace ),
					workspace_get_average_intensity( *____active_workspace ),
					&jitter,
					use_2d_sampling,
					aabbox_scalar,
					min_window,
					ns_settings_get_threshold_use_fixed( settings ),
					ns_settings_get_threshold_fixed_value( settings ),
					&radius,
					&length,
					&did_create_sample
					),
				error ) )
			{
			ns_println( "not enough memory to complete sample" );
			return;
			}

      if( did_create_sample )
         {
         if( ( flags & MK_CONTROL ) && NULL != prev_sample )
            ns_sampler_remove( sampler, prev_sample );

         ns_assert( 0 < ns_sampler_size( sampler ) );
         curr_sample = ns_sampler_last( sampler );

         /* TEMP */
         ns_println(
            "volume       = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "surface_area = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "threshold    = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "radius       = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "length       = " NS_FMT_DOUBLE NS_STRING_NEWLINE,
            ns_sample_get_volume( curr_sample ),
            ns_sample_get_surface_area( curr_sample ),
            ns_sample_get_threshold( curr_sample ),
            radius,
            length
            );

         ____redraw_all();
			_status_measurement();
         }
		}
	}


void _ogl_window_mouse_mode_rays_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   if( ____alt_key_is_down && ! ____ogl_wheel_timer_id )
      {
      ____ogl_drag_rect_active = NS_TRUE;
      _ogl_window_on_begin_drag_rect( hWnd, x, y );
      }
	}


void _ogl_window_mouse_mode_rays_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	//if( ! ____lbutton_double_clicked )
		{
		if( ____alt_key_is_down && ____ogl_drag_rect_active )
			_ogl_window_mouse_mode_rays_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			_ogl_window_mouse_mode_rays_on_lbutton_up_do_run_samples( hWnd, x, y, flags );

		____alt_key_released = NS_FALSE;
		}
	}


nsushort _ogl_window_mouse_mode_roi_find_selections( nsuint workspace, NsVector3d *P )
	{
	NsRoiControls  rc;
	NsPoint3d      I;
	nsdouble       distance, min_distance;
	nsushort       closest_bit;
	nsint          closest_idx;
	NsLine3d       L;


	ns_roi_controls_init(
		&rc,
		NS_ROI_CONTROL_ALL_BITS,
		workspace_update_roi( workspace ),
		workspace_volume( workspace ),
		workspace_get_voxel_info( workspace ),
		workspace_get_roi_control_selections( workspace )
		);

	closest_bit  = 0;
	closest_idx  = -1;
	min_distance = NS_DOUBLE_MAX;

	#define _OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( name )\
		if( ns_ray3d_intersects_aabbox( &____ogl->ray, rc.B + NS_ROI_CONTROL_##name##_IDX, &I ) )\
			{\
			distance = ns_vector3d_distance( &____ogl->ray.O, &I );\
			\
			if( distance < min_distance )\
				{\
				min_distance = distance;\
				closest_bit  = NS_ROI_CONTROL_##name##_BIT;\
				closest_idx  = NS_ROI_CONTROL_##name##_IDX;\
				}\
			}

	if( 1 < rc.dim[2] )
		{
		_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( NEAR );
		_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( FAR );
		}

	_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( TOP );
	_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( BOTTOM );
	_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( LEFT );
	_OGL_WINDOW_MOUSE_MODE_ROI_FIND_SELECTION( RIGHT );

	if( closest_bit && NULL != P )
		{
		ns_assert( -1 != closest_idx );

		ns_line3d_find_shortest_between( rc.L + closest_idx, &____ogl->line, &L );
		*P = L.P1;
		}

	return closest_bit;
	}


void _ogl_window_mouse_mode_roi_set_cursor( HWND hWnd, nsint cursor )
	{
	if( cursor != ____ogl->roi_curr_cursor )
		{
		____ogl->roi_curr_cursor = cursor;

		SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )____ogl->roi_cursors[ ____ogl->roi_curr_cursor ] );
		SetCursor( ____ogl->roi_cursors[ ____ogl->roi_curr_cursor ] );
		}
	}


nsint _ogl_window_mouse_mode_roi_get_cursor( nsint x, nsint y )
	{
	nsint     cursor;
	nsushort  closest;


	cursor = -1;

	ns_seed_find_get_ray_and_line(
		x, y,
		____modelview_matrix,
		____projection_matrix,
		&____ogl->ray,
		&____ogl->line
		);

	closest = _ogl_window_mouse_mode_roi_find_selections( *____active_workspace, NULL );

	if( closest )
		cursor = _ROI_CURSOR_ALL;//_ROI_CURSOR_CROSS;

	if( -1 == cursor )
		cursor = _ROI_CURSOR_ARROW;

	ns_assert( 0 <= cursor );
	return cursor;
	}


void _ogl_window_mouse_mode_roi_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	nsboolean           redraw;
	nsushort            closest_bit;
	nsuint              workspace;
	nsdouble            delta;
	const nsdouble     *prev;
	const nsdouble     *curr;
	const NsVoxelInfo  *voxel_info;
	NsCubei            *roi;


   if( ! ____ogl->isDragging )
		_ogl_window_mouse_mode_roi_set_cursor( hWnd, _ogl_window_mouse_mode_roi_get_cursor( x, y ) );
	else if( _ROI_CURSOR_ARROW != ____ogl->roi_curr_cursor )
		{
		NsRoiControls  rc;
		NsLine3d       L;


		if( ____ogl->is_lbutton_down )
			____ogl->is_mouse_dragging = NS_TRUE;

		____ogl->new_client.x = x;
		____ogl->new_client.y = y;

		workspace  = *____active_workspace;

		voxel_info = workspace_get_voxel_info( workspace );
		roi        = workspace_update_roi( workspace );

		redraw = NS_TRUE;

		closest_bit = workspace_get_roi_control_selections( *____active_workspace );

		ns_seed_find_get_ray_and_line(
			x, y,
			____modelview_matrix,
			____projection_matrix,
			&____ogl->ray,
			&____ogl->line
			);

		ns_roi_controls_init(
			&rc,
			closest_bit,
			workspace_update_roi( workspace ),
			workspace_volume( workspace ),
			workspace_get_voxel_info( workspace ),
			closest_bit
			);

		ns_line3d_find_shortest_between( rc.L + ____ogl->closest_idx, &____ogl->line, &L );
		____ogl->new_voxel = L.P1;

		prev = ns_vector3d_const_array( &____ogl->old_voxel );
		curr = ns_vector3d_const_array( &____ogl->new_voxel );

		delta = curr[ ____ogl->coord ] - prev[ ____ogl->coord ];

		if( NS_DOUBLE_EQUAL( delta, 0.0 ) )
			redraw = NS_FALSE;
		else
			{
			____ogl->track += delta;

			if( ____ogl->track < ____ogl->voxel_face_min )
				*____ogl->face = ____ogl->image_face_min;
			else if( ____ogl->track > ____ogl->voxel_face_max )
				*____ogl->face = ____ogl->image_face_max;
			else
				*____ogl->face = ns_to_image_space_component( ( nsfloat )____ogl->track, voxel_info, ____ogl->coord );
			}

		/* TEMP? Can redraw in 2D by erasing ROI by XOR? */
		if( redraw )
			{
			____ogl->roi_did_change = NS_TRUE;

			_adjust_slice_viewer_due_to_roi_update( roi );
			_do_adjust_seed_due_to_roi_update( workspace, roi );
			_do_show_roi_on_status_bar( workspace );

			____redraw_3d();

			/* Small optimization. May not need to redraw 2D display, depending
				on the current 2D view and which control is being dragged. */
			switch( *____2d_display_type )
				{
				case NS_XY:
					if( ! ( NS_ROI_CONTROL_NEAR_BIT == closest_bit || NS_ROI_CONTROL_FAR_BIT == closest_bit ) )
						____redraw_2d();
					break;

				case NS_ZY:
					if( ! ( NS_ROI_CONTROL_LEFT_BIT == closest_bit || NS_ROI_CONTROL_RIGHT_BIT == closest_bit ) )
						____redraw_2d();
					break;

				case NS_XZ:
					if( ! ( NS_ROI_CONTROL_TOP_BIT == closest_bit || NS_ROI_CONTROL_BOTTOM_BIT == closest_bit ) )
						____redraw_2d();
					break;

				default:
					ns_assert_not_reached();
				}
			}

		____ogl->old_client = ____ogl->new_client;
		____ogl->old_voxel  = ____ogl->new_voxel;
		}
	}


void _ogl_window_mouse_mode_roi_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	nsushort            closest_bit;
	const NsVoxelInfo  *voxel_info;
	const NsImage      *volume;
	NsCubei            *roi;


	____ogl->is_lbutton_down   = NS_TRUE;
	____ogl->is_mouse_dragging = NS_FALSE;

	SetCapture( hWnd );
	____ogl->hasCapture = 1;

	____ogl->roi_did_change = NS_FALSE;

	if( _ROI_CURSOR_ARROW != ____ogl->roi_curr_cursor )
		{
		ns_seed_find_get_ray_and_line(
			x, y,
			____modelview_matrix,
			____projection_matrix,
			&____ogl->ray,
			&____ogl->line
			);

		closest_bit = _ogl_window_mouse_mode_roi_find_selections( *____active_workspace, &____ogl->old_voxel );

		if( closest_bit )
			{
			____ogl->new_voxel = ____ogl->old_voxel;

			workspace_set_roi_control_selections( *____active_workspace, closest_bit );

			voxel_info = workspace_get_voxel_info( *____active_workspace );
			roi        = workspace_update_roi( *____active_workspace );
			volume     = workspace_volume( *____active_workspace );


			switch( closest_bit )
				{
				case NS_ROI_CONTROL_NEAR_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_NEAR_IDX;
					____ogl->coord          = 2;
					____ogl->face           = &roi->C1.z;
					____ogl->image_face_min = 0;
					____ogl->image_face_max = roi->C2.z;
					break;

				case NS_ROI_CONTROL_FAR_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_FAR_IDX;
					____ogl->coord          = 2;
					____ogl->face           = &roi->C2.z;
					____ogl->image_face_min = roi->C1.z;
					____ogl->image_face_max = ( nsint )ns_image_length( volume ) - 1;
					break;

				case NS_ROI_CONTROL_TOP_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_TOP_IDX;
					____ogl->coord          = 1;
					____ogl->face           = &roi->C1.y;
					____ogl->image_face_min = 0;
					____ogl->image_face_max = roi->C2.y;
					break;

				case NS_ROI_CONTROL_BOTTOM_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_BOTTOM_IDX;
					____ogl->coord          = 1;
					____ogl->face           = &roi->C2.y;
					____ogl->image_face_min = roi->C1.y;
					____ogl->image_face_max = ( nsint )ns_image_height( volume ) - 1;
					break;

				case NS_ROI_CONTROL_LEFT_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_LEFT_IDX;
					____ogl->coord          = 0;
					____ogl->face           = &roi->C1.x;
					____ogl->image_face_min = 0;
					____ogl->image_face_max = roi->C2.x;
					break;

				case NS_ROI_CONTROL_RIGHT_BIT:
					____ogl->closest_idx    = NS_ROI_CONTROL_RIGHT_IDX;
					____ogl->coord          = 0;
					____ogl->face           = &roi->C2.x;
					____ogl->image_face_min = roi->C1.x;
					____ogl->image_face_max = ( nsint )ns_image_width( volume ) - 1;
					break;

				default:
					ns_assert_not_reached();
				}

			____ogl->track = ( nsdouble )ns_to_voxel_space_component( *____ogl->face, voxel_info, ____ogl->coord );

			____ogl->voxel_face_min = ( nsdouble )ns_to_voxel_space_component( ____ogl->image_face_min, voxel_info, ____ogl->coord );
			____ogl->voxel_face_max = ( nsdouble )ns_to_voxel_space_component( ____ogl->image_face_max, voxel_info, ____ogl->coord ); 

			____ogl->new_client.x = ____ogl->old_client.x = x;
			____ogl->new_client.y = ____ogl->old_client.y = y;

			____ogl->isDragging = 1;
			____redraw_3d();
			}
		}
	}


void _ogl_window_mouse_mode_roi_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	____ogl->is_lbutton_down   = NS_FALSE;
	____ogl->is_mouse_dragging = NS_FALSE;

   if( ____ogl->hasCapture )
      {
      ____ogl->hasCapture = 0;
      ReleaseCapture();
      }

   ____ogl->isDragging = 0;

	workspace_set_roi_control_selections( *____active_workspace, 0 );

	if( _ROI_CURSOR_ARROW != ____ogl->roi_curr_cursor )
		____redraw_3d();

	if( ____ogl->roi_did_change )
		____redraw_2d();

	____ogl->roi_did_change = NS_FALSE;
	}


void _ogl_window_mouse_mode_roi_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{  _mouse_mode_roi_do_popup( hWnd, *____active_workspace, x, y );  }


static MouseMode ____ogl_mouse_modes[mDISPLAY_WINDOW_NUM_MOUSE_MODES] =
   {
	{
   NULL,
   NULL,//_zoom_on_mouse_move,
   NULL,//_zoom_on_lbutton_down,
   NULL,//_zoom_on_lbutton_up,
   NULL,
   NULL,//_zoom_on_rbutton_down,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_zoom_on_set
   },

	{
   NULL,
	_ogl_window_mouse_mode_drag_on_mouse_move,
	_ogl_window_mouse_mode_drag_on_lbutton_down,
	_ogl_window_mouse_mode_drag_on_lbutton_up,
   NULL,//_display_window_mouse_mode_drag_on_lbutton_dbl_clk,
   _ogl_window_mouse_mode_drag_on_rbutton_down,
   NULL,
	_ogl_window_mouse_mode_on_unset,
   _ogl_window_mouse_mode_drag_on_set
   },

   {
   NULL,
	_ogl_window_mouse_mode_seed_on_mouse_move,
	_ogl_window_mouse_mode_seed_on_lbutton_down,
	_ogl_window_mouse_mode_seed_on_lbutton_up,
   NULL,
   NULL,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_seed_on_set
   },

   {
   NULL,
   _ogl_window_mouse_mode_rays_on_mouse_move,
   _ogl_window_mouse_mode_rays_on_lbutton_down,
   _ogl_window_mouse_mode_rays_on_lbutton_up,
   NULL,
   NULL,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_rays_on_set
   },

   {
   NULL,
   _ogl_window_mouse_mode_spine_on_mouse_move,
	_ogl_window_mouse_mode_spine_on_lbutton_down,
	_ogl_window_mouse_mode_spine_on_lbutton_up,
   NULL,
	_ogl_window_mouse_mode_spine_on_rbutton_down,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_spine_on_set
   },

   {
   NULL,
	_ogl_window_mouse_mode_neurite_on_mouse_move,
   _ogl_window_mouse_mode_neurite_on_lbutton_down,
   _ogl_window_mouse_mode_neurite_on_lbutton_up,
   NULL,
   _ogl_window_mouse_mode_neurite_on_rbutton_down,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_neurite_on_set
   },

	{
   NULL,
   _ogl_window_mouse_mode_roi_on_mouse_move,
	_ogl_window_mouse_mode_roi_on_lbutton_down,
	_ogl_window_mouse_mode_roi_on_lbutton_up,
   NULL,
	_ogl_window_mouse_mode_roi_on_rbutton_down,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_roi_on_set
   },

	{
   NULL,
   NULL,//_mouse_mode_magnet_on_mouse_move,
   NULL,//_mouse_mode_magnet_on_lbutton_down,
   NULL,//_mouse_mode_magnet_on_lbutton_up,
   NULL,
   NULL,
   NULL,
	_ogl_window_mouse_mode_on_unset,
	_ogl_window_mouse_mode_magnet_on_set
   }
   };


#define _OGLWIN_DESTROY  ( WM_USER + 1 )


void ApplyOpenGLLight( const OpenGLLight *const light )
   {
   glLightfv( light->index, GL_POSITION, light->position );
   glLightfv( light->index, GL_DIFFUSE, light->diffuse );
   glLightfv( light->index, GL_AMBIENT, light->ambient );
   glLightfv( light->index, GL_SPECULAR, light->specular );
   }



void ApplyOpenGLMaterial( const OpenGLMaterial *const material )
   {
   glMaterialfv( GL_FRONT, GL_DIFFUSE, material->diffuse );
   glMaterialfv( GL_FRONT, GL_AMBIENT, material->ambient );
   glMaterialfv( GL_FRONT, GL_SPECULAR, material->specular );
   glMaterialfv( GL_FRONT, GL_SHININESS, material->shininess );
   }



void ConstructOpenGLTexture( OpenGLTexture *const tex )
   {  memset( tex, 0, sizeof( OpenGLTexture ) );  }


nsint CreateOpenGLTexture
   (
   OpenGLTexture *const  tex,
   const GLint           level,
   const GLsizei         width,
   const GLsizei         height,
   const GLint           channels,
   const GLenum          format,
   const GLenum          type,
   void                 *pixels
   )
   {
   if( ! tex->ID )
      {
      glGenTextures( 1, &tex->ID );
      
      if( tex->ID )
         {
         tex->level    = level;
         tex->width    = width;
         tex->height   = height;
         tex->format   = format;
         tex->type     = type;
         tex->channels = channels;
         tex->pixels   = pixels;

         glBindTexture( GL_TEXTURE_2D, tex->ID );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
         glTexImage2D( GL_TEXTURE_2D, level, channels, width, height, 0, format, type, pixels );

         return 1;
         }
      }

   return 0;
   }


void ApplyOpenGLTexture( const OpenGLTexture *const tex )
   {  glBindTexture( GL_TEXTURE_2D, tex->ID );  }


void DestructOpenGLTexture( OpenGLTexture *const tex )
   {  glDeleteTextures( 1, &tex->ID );  }



static const char *s_OpenGLWindowClassName = "OpenGLWindowClass";


void _CenterOpenGLWindow( OpenGLWindow *const ogl )
   {
   RECT  parent;
   RECT  child;
   long  width;
   long  x;
   long  height;
   long  y;


   GetWindowRect( ogl->hWnd, &child );
   GetWindowRect( ogl->hOwner, &parent );

   width = ( child.right - child.left );

   x = parent.left +
       ( parent.right - parent.left ) / 2 - 
       ( child.right - child.left ) / 2;

   child.left  = x;
   child.right = x + width;

   height = ( child.bottom - child.top );

   y = parent.top +
       ( parent.bottom - parent.top ) / 2 - 
       ( child.bottom - child.top ) / 2;

   child.top    = y;
   child.bottom = y + height;

   SetWindowPos( ogl->hWnd,
                 NULL, 
                 child.left,
                 child.top,
                 0, 0, 
                 SWP_NOZORDER | SWP_NOSIZE
               );
   }

#define _OGL_ABS( v ) ( ( (v) < 0 ) ? -(v) : (v) )

/* TEMP */
extern unsigned long ____threshold;
extern char ____fps_string[];
extern unsigned int ____frame_count;
extern nsuint *____active_workspace;

extern nschar _startup_directory[];
extern nschar ____config_file[];


nsboolean ____opengl_window_do_screen_capture = NS_FALSE;

#include "app_data.h"

void _opengl_window_screen_capture( OpenGLWindow *ogl )
	{
	NsImage     scan_aligned, no_alignment;
	BITMAPINFO  info;
	HDC        	buffer_dc;
	HBITMAP     old_bitmap, new_bitmap;
	nschar      file_name[ NS_PATH_SIZE ];
	NsError     error;


	if( NULL == ogl->hDC || 0 == *____active_workspace )
		return;

	ns_println( "Doing 3D screen capture..." );

	buffer_dc  = NULL;
	old_bitmap = NULL;
	new_bitmap = NULL;

	ns_image_construct( &scan_aligned );
	ns_image_construct( &no_alignment );

	if( NULL == ( buffer_dc = CreateCompatibleDC( ogl->hDC ) ) )
		goto _SCREEN_CAPTURE_EXIT;

	if( NULL == ( new_bitmap = CreateCompatibleBitmap( ogl->hDC, ogl->width, ogl->height ) ) )
		goto _SCREEN_CAPTURE_EXIT;

	if( NULL != buffer_dc && NULL != new_bitmap )
		old_bitmap = SelectObject( buffer_dc, new_bitmap );

	if( NS_FAILURE( ns_image_create( &scan_aligned, NS_PIXEL_RGB_U8_U8_U8, ( nssize )ogl->width, ( nssize )ogl->height, 1, sizeof( DWORD ) ), error ) )
		goto _SCREEN_CAPTURE_EXIT;

	BitBlt(
		buffer_dc,
		0, 0,
		ogl->width,
		ogl->height,
		ogl->hDC,
		0, 0,
		SRCCOPY
		);

	info.bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
	info.bmiHeader.biWidth         = ( nsint )ogl->width;
	info.bmiHeader.biHeight        = -( ( nsint )ogl->height );
	info.bmiHeader.biPlanes        = 1;
	info.bmiHeader.biBitCount      = 24;
	info.bmiHeader.biCompression   = BI_RGB;
	info.bmiHeader.biSizeImage     = 0;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biClrUsed       = 0;
	info.bmiHeader.biClrImportant  = 0;

	GetDIBits(
		buffer_dc,
		new_bitmap,
		0,
		( UINT )ogl->height,
		ns_image_pixels( &scan_aligned ),
		&info,
		DIB_RGB_COLORS
		);

	ns_image_set_pixel_proc_db( &scan_aligned, workspace_pixel_proc_db( *____active_workspace ) );

	if( NS_FAILURE( ns_image_convert( &scan_aligned, NS_PIXEL_BGR_U8_U8_U8, 1, &no_alignment, NULL ), error ) )
		goto _SCREEN_CAPTURE_EXIT;

	ns_sprint( file_name, "%s" NS_STRING_PATH "test.jpg", _startup_directory );

	ns_image_set_io_proc_db( &no_alignment, workspace_io_proc_db( *____active_workspace ) );
	ns_image_write_jpeg( &no_alignment, file_name, 85, NULL );

	_SCREEN_CAPTURE_EXIT:

	if( NULL != buffer_dc )
		{
		SelectObject( buffer_dc, old_bitmap );

		if( NULL != new_bitmap )
			DeleteObject( new_bitmap );

		DeleteDC( buffer_dc );
		}

	ns_image_destruct( &scan_aligned );
	ns_image_destruct( &no_alignment );
	}


/*
void _ogl_draw_projected_vertices( OpenGLWindow *ogl )
	{
	nsmodelvertex  curr, end;
	NsVector3f     position;
	nsdouble       r, k, rp;//, F, M;
	GLdouble       ox, oy, oz, wx, wy, wz;
	nsint          x, y, rx, ry;
   GLint          viewport[4];
	HPEN           pen;


   glGetIntegerv( GL_VIEWPORT, viewport );

	curr = ns_model_begin_vertices( ogl->model );
	end  = ns_model_end_vertices( ogl->model );

	// The following code uses simple triognometry and interpolation to estimate the radius (in pixels) of a sphere
	//	projected onto the screen by the OpenGL rendering pipeline.

	//	These equations allow for a simple circle and box intersection test for selecting objects on the 3D window.
	

	// Given a sphere of radius 'r', the equations below determine the size of the circle that encloses the OpenGL
	//	rendered sphere.

	// Equations by Alfredo Rodriguez on June 2, 2009.
	//F = ( ( nsdouble )ogl->height ) / ( ns_tan( 0.5 * NS_DEGREES_TO_RADIANS( ogl->fieldOfView ) ) * 2.0 * ogl->zFar );
	//M = ( ogl->zFar / ogl->zNear ) - 1.0;

	// Simpler equation by Alfredo Rodriguez on June 3, 2009.
	k = ( ( nsdouble )ogl->height ) / ( ogl->zNear * 2.0 * ns_tan( 0.5 * NS_DEGREES_TO_RADIANS( ogl->fieldOfView ) ) );

	pen = SelectObject( ogl->hDC, GetStockObject( WHITE_PEN ) );

	for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
		{
		ns_model_vertex_get_position( curr, &position );

		ox = position.x;
		oy = position.y;
		oz = position.z;

		gluProject( ox, oy, oz, ____modelview_matrix, ____projection_matrix, viewport, &wx, &wy, &wz );

		if( 0.0 <= wz && wz <= 1.0 )
			{
			x = ( nsint )( wx + 0.5 );
			y = ( nsint )( wy + 0.5 );

			y = ( viewport[3] - y ) - 1;

			r = ns_model_vertex_get_radius( curr );

			// Equation by Alfredo Rodriguez on June 2, 2009.
			//rp = F * ( r * M * ( 1.0 - wz ) + r );
			rp = k * r * ( 1.0 - wz );

			rx = ry = ( nsint )( rp + 0.5 );

			Arc(
				ogl->hDC,
				x - rx,
				y - ry,
				x + rx + 1,
				y + ry + 1,
				x - rx,
				y,
				x - rx,
				y
				);
			}
		}

	SelectObject( ogl->hDC, pen );
	}
*/


void _OpenGLWindowOnPaint( OpenGLWindow *const ogl )
   {
   if( ogl->visible )
      {
		//char buf[32];

      glPushMatrix();

         //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      if( NULL != ogl->viewTransform )
         ( ogl->viewTransform )( ogl, ogl->userData );


         //switch( ogl->mouseMode )
           // {
            //case OGLWIN_MOUSE_MODE_INTERACTOR:
      InteractorViewTransformation( &ogl->interactor );
              // break;
            //}


      if( NULL != ogl->renderFrame )
         ( ogl->renderFrame )( ogl, ogl->userData );

      glPopMatrix();

		if( ____opengl_window_do_screen_capture )
			{
			_opengl_window_screen_capture( ogl );
			____opengl_window_do_screen_capture = NS_FALSE;
			}		

      SwapBuffers( ogl->hDC );

/* TEMP!!!!!!!!!!!!!!!!!!!! */
//sprintf( buf, "%u", ____threshold );
//TextOut( ogl->hDC, 20, 20, buf, strlen( buf ) );
//sprintf( buf, "%u", ____num_selected );
//TextOut( ogl->hDC, 20, 80, buf, strlen( buf ) );

/*
	if( 100 <= ____frame_count )
		TextOut( ogl->hDC, 20, 20, ____fps_string, ( unsigned nsint )strlen( ____fps_string ) );
	else
		TextOut( ogl->hDC, 20, 20, "N/A", 3 );
*/

		if( NULL != ogl->text )
			{
			int       old_bk_mode;
			COLORREF  old_text_color;


			old_bk_mode    = SetBkMode( ogl->hDC, TRANSPARENT );
			old_text_color = SetTextColor( ogl->hDC, RGB( 255, 255, 255 ) );

			TextOut( ogl->hDC, 10, ogl->height - 25, ogl->text, ( unsigned int )strlen( ogl->text ) );

			SetBkMode( ogl->hDC, old_bk_mode );
			SetTextColor( ogl->hDC, old_text_color );
			}

		//_ogl_draw_projected_vertices( ogl );
      }
   }


void _OpenGLWindowOnSize( OpenGLWindow *const ogl, unsigned width, unsigned height )
   {
   /* Avoid divide by zero. */

   if( 0 == height )
      height = 1;

   glViewport( 0, 0, width, height );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();

   if( ! ogl->isOrtho )
      gluPerspective( ogl->fieldOfView, ( ( GLdouble )width )/height, ogl->zNear, ogl->zFar );
   else
      glOrtho( ogl->xOrthoLeft,
               ogl->xOrthoRight,
               ogl->yOrthoBottom,
               ogl->yOrthoTop,
               ogl->zOrthoNear,
               ogl->zOrthoFar
             );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();

   ogl->width  = width;
   ogl->height = height;

	SetInteractorWindowSize( &ogl->interactor, ( nsint )width, ( nsint )height );
   }


void _OpenGLWindowOnMove( OpenGLWindow *const ogl, nsint x, nsint y )
   {
   ogl->x = x;
   ogl->y = y;
   }


void _OpenGLWindowOnCommand( OpenGLWindow *const ogl, nsint notifyCode, nsint ID )
   {
   if( 0 == notifyCode )
      {
      ogl->menuID = ID;

      if( NULL != ogl->menuSelection )
         ( ogl->menuSelection )( ogl, ogl->userData );
      }
   }


void _OpenGLWindowOnMouseEvent( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   ogl->oldMouseX = ogl->mouseX;
   ogl->oldMouseY = ogl->mouseY;

   ogl->mouseX        = x;
   ogl->mouseY        = y;
   ogl->mouseKeyFlags = flags;
   }


void _OpenGLWindowOnMouseMove( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   if( 0 != *____active_workspace )
      {
		_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onMouseMove )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onMouseMove )( ogl->hWnd, x, y, flags );

		//if( NULL != ogl->mouseMove )
		//	( ogl->mouseMove )( ogl, ogl->userData );
		}
   }


extern nsint _workspace_get_volume_length( void );

void _opengl_window_on_mouse_wheel( OpenGLWindow *ogl, nsint delta, nsint x, nsint y, nsuint flags )
	{
	nsint    sign;
	nsfloat  zpos;
	//nsint    zdim;
	//nsfloat  length;
	//nsfloat  middle;


	StartInteractor( &ogl->interactor, ogl->hWnd, x, -y );
	SetInteractorMode( &ogl->interactor, INTERACTOR_MODE_ZOOM );

	sign  = delta < 0 ? -1 : 1;
	delta = 0;

	zpos = ogl->interactor.izpos;

	zpos /= ns_voxel_info_size_z( workspace_get_voxel_info( *____active_workspace ) );
	delta = ( nsint )( zpos * 0.15f );

	//ns_println( "DELTA = %d", delta );


	//zdim = _workspace_get_volume_length();

	//length = 0.05f/*ogl->interactor.idtouch*/ * zdim;
	//middle = length / 2.0f;

	//zpos -= middle;
	//zpos = NS_ABS( zpos );

	//delta = ( nsint )( zpos * 4.0f );

	if( delta < ogl->height / 8 )
		delta = ogl->height / 8;

	delta *= sign;

	if( UpdateInteractor( &ogl->interactor, ogl->interactor.iprevx, ogl->interactor.iprevy - delta ) )
		RepaintOpenGLWindow( ogl );

	EndInteractor( &ogl->interactor );
	}


void _OpenGLWindowOnMouseLButtonDown( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   if( 0 != *____active_workspace )
      {
		_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onLButtonDown )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onLButtonDown )( ogl->hWnd, x, y, flags );

		//if( NULL != ogl->mouseLButtonDown )
		//	( ogl->mouseLButtonDown )( ogl, ogl->userData );
		}
   }


void _OpenGLWindowOnMouseLButtonUp( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   if( 0 != *____active_workspace )
      {
		_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onLButtonUp )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onLButtonUp )( ogl->hWnd, x, y, flags );

		//if( NULL != ogl->mouseLButtonUp )
		//	( ogl->mouseLButtonUp )( ogl, ogl->userData );
		}
   }


//void _OpenGLWindowOnMouseLButtonDblClk( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
  // {
   //_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

   //if( NULL != ogl->mouseLButtonDblClk )
     // ( ogl->mouseLButtonDblClk )( ogl, ogl->userData );
   //}


void _OpenGLWindowOnMouseRButtonDown( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   if( 0 != *____active_workspace )
      {
		_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onRButtonDown )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onRButtonDown )( ogl->hWnd, x, y, flags );

		//if( NULL != ogl->mouseRButtonDown )
		//	( ogl->mouseRButtonDown )( ogl, ogl->userData );
		}
   }


void _OpenGLWindowOnMouseRButtonUp( OpenGLWindow *const ogl, nsint x, nsint y, unsigned flags )
   {
   if( 0 != *____active_workspace )
      {
		_OpenGLWindowOnMouseEvent( ogl, x, y, flags );

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onRButtonUp )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onRButtonUp )( ogl->hWnd, x, y, flags );

		//if( NULL != ogl->mouseRButtonUp )
		//	( ogl->mouseRButtonUp )( ogl, ogl->userData );
		}
   }


//extern Image *____forward;
//extern Image *____side;
//extern Image *____top;

nsint _OpenGLWindowOnCreate( OpenGLWindow *const ogl )
   {
   PIXELFORMATDESCRIPTOR  pfd;
   nsint                  format;


   memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
   pfd.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
   pfd.nVersion   = 1;
   pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = ogl->colorBits;
   pfd.cDepthBits = ogl->depthBits;
   pfd.iLayerType = PFD_MAIN_PLANE;

   /* NOTE: Private device contexts do not need to be released. */

   ogl->hDC = GetDC( ogl->hWnd );

   if( 0 == ( format = ChoosePixelFormat( ogl->hDC, &pfd ) ) ||
       ! SetPixelFormat( ogl->hDC, format, &pfd )            ||
       NULL == ( ogl->hGLRC = wglCreateContext( ogl->hDC ) ) ||
       ! wglMakeCurrent( ogl->hDC, ogl->hGLRC )                 )
      return 0;

   glEnable( GL_DEPTH_TEST );
   glEnable( GL_CULL_FACE );
   glClearColor( ogl->clearRed, ogl->clearGreen, ogl->clearBlue, ogl->clearAlpha );
   glClearDepth( ogl->clearDepth );
   glShadeModel( GL_SMOOTH );
   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   glFrontFace( GL_CCW );

	glGenTextures( 3, ogl->textures );

	ogl->text_base = glGenLists( 128 );
	wglUseFontBitmaps( ogl->hDC, 0, 128, ogl->text_base );

	if( NULL != ogl->postCreate )
		( ogl->postCreate )( ogl, ogl->userData );

   return 1;
   }


void _OpenGLWindowOnNcDestroy( OpenGLWindow *const ogl )
   {
   if( NULL != ogl->preNcDestroy )
      ( ogl->preNcDestroy )( ogl, ogl->userData );

   if( NULL != ogl->hDC && NULL != ogl->hGLRC )
      {
      if( 0 != ogl->dlCylinder )
         glDeleteLists( ogl->dlCylinder, 1 );

      if( 0 != ogl->dlSphere )
         glDeleteLists( ogl->dlSphere, 1 );

		if( 0 != ogl->dlHalfSphere )
			glDeleteLists( ogl->dlHalfSphere, 1 );

		glDeleteTextures( 3, ogl->textures );

		glDeleteLists( ogl->text_base, 128 );

      wglMakeCurrent( ogl->hDC, ogl->hGLRC );
      wglDeleteContext( ogl->hGLRC );
      }

   ogl->hDC          = NULL;
   ogl->hGLRC        = NULL;
   ogl->dlCylinder   = 0;
   ogl->dlSphere     = 0;
	ogl->dlHalfSphere = 0;
   ogl->isCreated    = 0;

   if( NULL != ogl->postNcDestroy )
      ( ogl->postNcDestroy )( ogl, ogl->userData );
   }


void _OpenGLWindowOnKeyDown( OpenGLWindow *const ogl, nsint key )
   {
/*
	if( OGLWIN_MOUSE_MODE_INTERACTOR == ogl->mouseMode )
		{
		switch( key )
			{
			case VK_CONTROL:
				SetInteractorMode( &ogl->interactor, INTERACTOR_MODE_MOVE );
				break;

			case VK_SHIFT:
				SetInteractorMode( &ogl->interactor, INTERACTOR_MODE_ZOOM );
				break;

			case VK_MENU:
				SetInteractorMode( &ogl->interactor, INTERACTOR_MODE_ROLL );
				break;
			}
		}*/
	
	if( 0 <= key && key < 256 && NULL != ogl->keyMap[ key ] )
      ( ogl->keyMap[ key ] )( ogl, ogl->userData );
   }


void _OpenGLWindowOnKeyUp( OpenGLWindow *const ogl, nsint key )
	{}


//void _OpenGLWindowOnKeyUp( OpenGLWindow *const ogl, nsint key )
//	{  SetInteractorMode( &ogl->interactor, INTERACTOR_MODE_ROTATE );  }


void _open_gl_on_timer( OpenGLWindow *ogl )
	{
	if( NULL != ogl->on_timer )
		( ogl->on_timer )( ogl, ogl->userData );
	}


nsboolean ____opengl_timer_is_active = NS_FALSE;
extern nsint ____rotation_speed;


void open_gl_begin_timer( OpenGLWindow *ogl, nssize milliseconds )
	{
	if( ! ____opengl_timer_is_active )
		{
		____opengl_timer_is_active = NS_TRUE;
		SetTimer( ogl->hWnd, 101, ( UINT )milliseconds, NULL );
		}
	else
		{
		____rotation_speed *= 2;

		if( ____rotation_speed > 1024 )
			____rotation_speed = 1024;
		}
	}


void open_gl_end_timer( OpenGLWindow *ogl )
	{
	if( ____opengl_timer_is_active )
		{
		KillTimer( ogl->hWnd, 101 );

		____opengl_timer_is_active = NS_FALSE;
		____rotation_speed         = 1;
		}
	}


#define OGL_MESSAGE_BEGIN_TIMER  ( WM_USER + 11 )
#define OGL_MESSAGE_END_TIMER  ( WM_USER + 12 )

void open_gl_begin_timer_by_message( OpenGLWindow *ogl )
	{  SendMessage( ogl->hWnd, OGL_MESSAGE_BEGIN_TIMER, 0, 0 );  }

void open_gl_end_timer_by_message( OpenGLWindow *ogl )
	{  SendMessage( ogl->hWnd, OGL_MESSAGE_END_TIMER, 0, 0 );  }



VOID CALLBACK _ogl_on_wheel_timer( HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
	{
	OpenGLWindow *ogl = ( OpenGLWindow* )GetWindowLongPtr( hWnd, GWLP_USERDATA );

	ogl->is_wheeling = NS_FALSE;

	KillTimer( hWnd, ____ogl_wheel_timer_id );
	____ogl_wheel_timer_id = 0;

	RepaintOpenGLWindow( ogl );
	}


void _ogl_window_on_set_mouse_mode( HWND hWnd, nsint newMode )
	{
	ns_assert( -1 <= newMode && newMode < mDISPLAY_WINDOW_NUM_MOUSE_MODES );

	if( newMode != ____ogl_curr_mouse_mode )
		{
		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onUnset )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onUnset )( hWnd );

		____ogl_curr_mouse_mode = newMode;

		if( -1 != ____ogl_curr_mouse_mode && NULL != ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onSet )
			( ____ogl_mouse_modes[ ____ogl_curr_mouse_mode ].onSet )( hWnd );
		}
	}


void _ogl_window_on_set_focus( OpenGLWindow *ogl )
	{
	if( ____alert_user_about_splats )
		{
		____alert_user_about_splats = NS_FALSE;

		if( IDYES == MessageBox(
				ogl->hWnd,
					"A filter has been applied to the image data.\n"
					"Do you want to regenerate the volume rendering?",
					"NeuronStudio",
					MB_YESNO | MB_ICONQUESTION
				) )
			_open_gl_on_regenerate_splats( NS_FALSE );
		}
	}


LRESULT CALLBACK _OpenGLWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   OpenGLWindow *ogl = ( OpenGLWindow* )GetWindowLongPtr( hWnd, GWLP_USERDATA );

   switch( uMsg )
      {
		case WM_TIMER:
			_open_gl_on_timer( ogl );
			break;

      case WM_PAINT:
         _OpenGLWindowOnPaint( ogl );
         ValidateRect( ogl->hWnd, NULL );
         break;

      case WM_MOUSEMOVE:
         _OpenGLWindowOnMouseMove( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         break;

		case _DEFINE_WM_MOUSE_WHEEL:
			if( 0 != *____active_workspace && ! ogl->isDragging && ! ogl->is_mouse_dragging )
				{
				if( ____ogl_wheel_timer_id )
					{
					KillTimer( hWnd, ____ogl_wheel_timer_id );
					____ogl_wheel_timer_id = 0;
					}

				ogl->is_wheeling = NS_TRUE;

				if( ____optimize_splats_movement )
					____ogl_wheel_timer_id = SetTimer( hWnd, 1337, 1000, _ogl_on_wheel_timer );

				_opengl_window_on_mouse_wheel(
					ogl,
					( nsint )( nsshort )HIWORD( wParam ),
					( nsint )( nsshort )LOWORD( lParam ),
					( nsint )( nsshort )HIWORD( lParam ),
					( nsuint )LOWORD( wParam )
					);
				}
			break;

      case WM_LBUTTONDOWN:
         _OpenGLWindowOnMouseLButtonDown( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         break;

      case WM_LBUTTONUP:
         _OpenGLWindowOnMouseLButtonUp( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         break;

      //case WM_LBUTTONDBLCLK:
        // _OpenGLWindowOnMouseLButtonDblClk( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         //break;

      case WM_RBUTTONDOWN:
         _OpenGLWindowOnMouseRButtonDown( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         break;

      case WM_RBUTTONUP:
         _OpenGLWindowOnMouseRButtonUp( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam), ( unsigned )wParam );
         break;



		case WM_SYSKEYDOWN:
         if( _main_window_on_keydown( ( nsint )wParam ) )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );

         _OpenGLWindowOnKeyDown( ogl, ( nsint )wParam );
         break;

      case WM_KEYDOWN:
         _OpenGLWindowOnKeyDown( ogl, ( nsint )wParam );
			return ( LRESULT )_main_window_on_keydown( ( nsint )wParam );

      case WM_SYSKEYUP:
         if( _main_window_on_keyup( ( nsint )wParam ) )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );

			_OpenGLWindowOnKeyUp( ogl, ( nsint )wParam );
			break;

      case WM_KEYUP:
			_OpenGLWindowOnKeyUp( ogl, ( nsint )wParam );
         return ( LRESULT )_main_window_on_keyup( ( nsint )wParam );



      case WM_SIZE:
         _OpenGLWindowOnSize( ogl, LOWORD( lParam ), HIWORD( lParam ) );
         break;

      case WM_MOVE:
         _OpenGLWindowOnMove( ogl, (nsint)(short)LOWORD(lParam), (nsint)(short)HIWORD(lParam) );
         break;


		case WM_ENTERSIZEMOVE:
			ogl->is_sizing_or_moving = NS_TRUE;
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
	
		case WM_EXITSIZEMOVE:
			ogl->is_sizing_or_moving = NS_FALSE;
			RepaintOpenGLWindow( ogl );
			return DefWindowProc( hWnd, uMsg, wParam, lParam );



      case WM_COMMAND:
         _OpenGLWindowOnCommand( ogl, HIWORD( wParam ), LOWORD( wParam ) );
         break;


		case WM_SETFOCUS:
			_ogl_window_on_set_focus( ogl );
			break;


      case WM_CREATE:
         if( ! _OpenGLWindowOnCreate( ogl ) )
            return -1;
         break;

      case WM_NCCREATE:
         ogl = ( OpenGLWindow* )( ( ( LPCREATESTRUCT )lParam )->lpCreateParams );
         SetWindowLongPtr( hWnd, GWLP_USERDATA, ( LPARAM )ogl );
         ogl->hWnd = hWnd;
         return TRUE;

      case WM_NCDESTROY:
			open_gl_end_timer( ogl );
         _OpenGLWindowOnNcDestroy( ogl );
         break;

      case _OGLWIN_DESTROY:
         DestroyWindow( hWnd );
         break;

		case OGL_MESSAGE_BEGIN_TIMER:
			open_gl_begin_timer( ogl, 1 );
			break;

		case OGL_MESSAGE_END_TIMER:
			open_gl_end_timer( ogl );
			break;

      case mMSG_USER_SetMouseMode:
			_ogl_window_on_set_mouse_mode( hWnd, ( nsint )lParam );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;
   }


extern void _set_window_title( HWND wnd, const nschar *path );


void _DoBeginOpenGLWindow( void *args )
   {
   OpenGLWindow  *ogl;
   MSG            msg;
   HWND           hWnd;
   nsint          doCenter;
	nsint          mouse_mode;


   ogl = ( OpenGLWindow* )args;

   if( ! FindAtom( s_OpenGLWindowClassName ) )
      {
      WNDCLASSEX wcx;

      wcx.cbSize        = sizeof( WNDCLASSEX );
      wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC /*| CS_DBLCLKS*/; 
      wcx.lpfnWndProc   = _OpenGLWindowProc;
      wcx.cbClsExtra    = 0;
      wcx.cbWndExtra    = 0;
      wcx.hInstance     = ogl->hInstance;
      wcx.hIcon         = ( NULL != ogl->iconName ) ?
                          LoadIcon( ogl->hInstance, ogl->iconName ) : LoadIcon( NULL, IDI_APPLICATION );
      wcx.hCursor       = ( NULL != ogl->cursorName ) ?
                          LoadCursor( ogl->hInstance, ogl->cursorName ) : LoadCursor( NULL, IDC_ARROW );
      wcx.hbrBackground = NULL;
      wcx.lpszMenuName  = ogl->menuName;
      wcx.lpszClassName = s_OpenGLWindowClassName;
      wcx.hIconSm       = NULL;

      if( 0 == RegisterClassEx( &wcx ) )
         {
         ogl->isCreating = 0;
         _endthread();
         return;
         }

      AddAtom( s_OpenGLWindowClassName );
      }

   doCenter = ( -1 == ogl->x || -1 == ogl->y ) ? 1 : 0;

   hWnd = CreateWindowEx( 0,
                          s_OpenGLWindowClassName,
                          NULL,
                          WS_POPUP | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU |
                          WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          ( 0 <= ogl->x ) ? ogl->x : 0,
                          ( 0 <= ogl->y ) ? ogl->y : 0,
                          ogl->width,
                          ogl->height,
                          ogl->hOwner,
                          NULL,
                          ogl->hInstance,
                          ogl
                        );

   if( NULL == hWnd )
      {
      ogl->isCreating = 0;
      _endthread();
      return;
      }

   /* The handle should have been set. */

   ns_assert( ogl->hWnd == hWnd );

   ogl->isCreating = 0;
   ogl->isCreated  = 1;

   if( doCenter )
      _CenterOpenGLWindow( ogl );

	/* Set the global to "no mode" first. */
	mouse_mode = ____ogl_curr_mouse_mode;
	____ogl_curr_mouse_mode = -1;
	_ogl_window_on_set_mouse_mode( ogl->hWnd, mouse_mode );

   UpdateWindow( ogl->hWnd );
   EnableOpenGLWindow( ogl, ogl->enabled );
   ShowOpenGLWindow( ogl, ogl->visible );

   _set_window_title( ogl->hWnd, ogl->title );

   while( 0 < GetMessage( &msg, NULL, 0, 0 ) )
      {
      TranslateMessage( &msg );
      DispatchMessage( &msg ); 
      }

   _endthread();
   }


void InitOpenGLWindow( OpenGLWindow *const ogl, HINSTANCE hInstance, HWND hOwner )
   {
   ns_assert( NULL != ogl );

   ogl->hInstance          = hInstance;
   ogl->hOwner             = hOwner;
   ogl->hWnd               = NULL;
   ogl->hDC                = NULL;
   ogl->hGLRC              = NULL;
   ogl->enabled            = 1;
   ogl->visible            = 1;
   ogl->x                  = -1;
   ogl->y                  = -1;
   ogl->width              = 600;
   ogl->height             = 600;
   ogl->title              = NULL;
   ogl->menuName           = NULL;
   ogl->iconName           = NULL;
   ogl->cursorName         = NULL;
   ogl->menuID             = 0;
   ogl->colorBits          = 32;
   ogl->depthBits          = 16;
   ogl->clearRed           = 0.0f;
   ogl->clearGreen         = 0.0f;
   ogl->clearBlue          = 0.0f;
   ogl->clearAlpha         = 1.0f;
   ogl->clearDepth         = 1.0f;
   ogl->fieldOfView        = 45.0f;
   ogl->zNear              = 1.0f;
   ogl->zFar               = 1024.0f;
   ogl->isOrtho            = 0;
   ogl->xOrthoLeft         = -1.0f;
   ogl->xOrthoRight        = 1.0f;
   ogl->yOrthoTop          = 1.0f;
   ogl->yOrthoBottom       = -1.0f;
   ogl->zOrthoNear         = -1.0f;
   ogl->zOrthoFar          = 1.0f;
   ogl->xEye               = 0.0f;
   ogl->yEye               = 0.0f;
   ogl->zEye               = 0.0f;
   ogl->userData           = NULL;
   ogl->oldMouseX          = -1;
   ogl->oldMouseY          = -1;
   ogl->mouseX             = -1;
   ogl->mouseY             = -1;
   ogl->mouseKeyFlags      = 0;
   //ogl->mouseMode          = OGLWIN_MOUSE_MODE_NONE;
   ogl->menuSelection      = NULL;
   ogl->viewTransform      = NULL;
   ogl->renderFrame        = NULL;
   //ogl->mouseMove          = NULL;
   //ogl->mouseLButtonDown   = NULL;
   //ogl->mouseLButtonUp     = NULL;
   //ogl->mouseLButtonDblClk = NULL;
   //ogl->mouseRButtonDown   = NULL;
   //ogl->mouseRButtonUp     = NULL;
	ogl->preNcDestroy       = NULL;
   ogl->postNcDestroy      = NULL;
	ogl->postCreate         = NULL;
   ogl->dlCylinder         = 0;
   ogl->dlSphere           = 0;
	ogl->dlHalfSphere       = 0;
   ogl->isCreating         = 0;
   ogl->isCreated          = 0;
	ogl->text               = NULL;

	ogl->is_lbutton_down     = NS_FALSE;
	ogl->is_mouse_dragging   = NS_FALSE;
	ogl->is_sizing_or_moving = NS_FALSE;
	ogl->is_wheeling         = NS_FALSE;

   InitInteractor( &ogl->interactor );

	//SetInteractorRotationalTouch( &ogl->interactor, .5f );
	//SetInteractorTranslationalTouch( &ogl->interactor, .5f );
   
   memset( ogl->keyMap, 0, sizeof( ogl->keyMap ) );
   }


nsint CreateOpenGLWindow( OpenGLWindow *const ogl )
   {
   ns_assert( NULL != ogl );

   ogl->isCreating = 1;

   if( -1 == _beginthread( _DoBeginOpenGLWindow, 0, ogl ) )
      {
      ogl->isCreating = 0;
      return 0;
      }

   while( ogl->isCreating )
      {  Sleep( 0 );  }

   return ogl->isCreated;
   }


void DestroyOpenGLWindow( OpenGLWindow *const ogl )
   {
   ns_assert( NULL != ogl );
  
   /* NOTE: Cant use WM_DESTROY to destroy a window created by a
      different thread. */

   SendMessage( ogl->hWnd, _OGLWIN_DESTROY, 0, 0 );
   }


void EnableOpenGLWindow( OpenGLWindow *const ogl, nsint enabled )
   {
   ns_assert( NULL != ogl );

   ogl->enabled = enabled;
   EnableWindow( ogl->hWnd, enabled );
   }


void ShowOpenGLWindow( OpenGLWindow *const ogl, nsint visible )
   {
   ns_assert( NULL != ogl );

   ogl->visible = visible;
   ShowWindow( ogl->hWnd, ( visible ) ? SW_SHOW : SW_HIDE );
   }


void MaximizeOpenGLWindow( OpenGLWindow *const ogl )
	{
   ns_assert( NULL != ogl );
	ShowWindow( ogl->hWnd, SW_MAXIMIZE );
	}


void MinimizeOpenGLWindow( OpenGLWindow *const ogl )
	{
   ns_assert( NULL != ogl );
	ShowWindow( ogl->hWnd, SW_MINIMIZE );
	}


void RepaintOpenGLWindow( OpenGLWindow *const ogl )
   {
   ns_assert( NULL != ogl );
   InvalidateRect( ogl->hWnd, NULL, FALSE );
   }


void OpenGLWindowDrawText( OpenGLWindow *ogl, const char *s, float x, float y, float z )
	{
	ns_assert( NULL != ogl );
	
	glPushAttrib( GL_LIST_BIT );
	glRasterPos3f( x, y, z );
	glListBase( ogl->text_base );
	glCallLists( ( GLsizei )ns_ascii_strlen( s ), GL_UNSIGNED_BYTE, s );
	glPopAttrib();
	}


#define _OGLWIN_PI  3.141592654f

/*
void OpenGLWindowDrawCylinder( OpenGLWindow *const ogl, float radius1, float radius2, float height, nsint complexity )
   {
   float  theta, x, y, z, delta;
   nsint    i;


   delta = _OGLWIN_PI / 2 / complexity;
   y     = height;
   theta = 0.0f;

   glBegin( GL_TRIANGLE_STRIP );

   for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
      {
      x = ( float )cos( theta );
      z = ( float )-sin( theta );

      glNormal3f( x, 0.0f, z );
      glVertex3f( radius1 * x, y, radius1 * z );
      glVertex3f( radius2 * x, y - height, radius2 * z );
      }

   glEnd();
   }
*/

void _do_OpenGLWindowDrawCylinder
	( OpenGLWindow *const ogl, float radius1, float radius2, float height, int complexity, int top, int bottom )
   {
   float  theta, x, y, z, delta, step;
   nsint    i, j;


   delta = _OGLWIN_PI / 2 / complexity;
   y     = height;
   theta = 0.0f;
   step  = height / complexity;

   glBegin( GL_TRIANGLE_STRIP );

   for( j = 0; j < complexity; ++j )
      {
      for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

         glNormal3f( x, 0.0f, z );
         glVertex3f( radius1 * x, y, radius1 * z );
         glVertex3f( radius2 * x, y - step, radius2 * z );
         }

      y -= step;
      }

   glEnd();

	if( top )
		{
		/* TODO? */
		}

	if( bottom )
		{
		glBegin( GL_TRIANGLE_FAN );

		glNormal3f( 0.0f, -1.0f, 0.0f );
		glVertex3f( 0.0f, 0.0f, 0.0f );

		theta = 2.0f * _OGLWIN_PI;

      for( i = 0; i < 4 * complexity + 1; i++, theta -= delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

         glNormal3f( 0.0f, -1.0f, 0.0f );
         glVertex3f( radius1 * x, 0.0f, radius1 * z );
         }

		glEnd();
		}
   }


void OpenGLWindowDrawCylinder( OpenGLWindow *const ogl, float radius1, float radius2, float height, nsint complexity )
	{  _do_OpenGLWindowDrawCylinder( ogl, radius1, radius2, height, complexity, 0, 0 );  }

void OpenGLWindowDrawCylinderWithCaps( OpenGLWindow *ogl, float r1, float r2, float h, int compl, int top, int bottom )
	{  _do_OpenGLWindowDrawCylinder( ogl, r1, r2, h, compl, top, bottom );  }


void _do_OpenGLWindowDrawCone( OpenGLWindow *const ogl, float radius, float height, int complexity, int base )
	{
   //float  theta, x, y, z, delta, step, radius1, radius2, rstep;
	nsfloat x, z, theta, delta;
   nsint  i/*, j*/;


	//radius1 = radius2 = 0.0f;
	//rstep = radius / complexity;

   delta = _OGLWIN_PI / 2 / complexity;
   /*y     = height;
   theta = 0.0f;
   step  = height / complexity;

   glBegin( GL_TRIANGLE_STRIP );

   for( j = 0; j < complexity; ++j )
      {
      for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

         glNormal3f( x, 0.0f, z );
         glVertex3f( radius1 * x, y, radius1 * z );
         glVertex3f( ( radius2 + rstep ) * x, y - step, ( radius2 + rstep ) * z );
         }

      y -= step;

		radius1 += rstep;
		radius2 += rstep;
      }

   glEnd();*/

	glBegin( GL_TRIANGLE_FAN );

	glNormal3f( 0.0f, 1.0f, 0.0f );
	glVertex3f( 0.0f, height, 0.0f );

	theta = 0.0f;

   for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
      {
      x = ( float )cos( theta );
      z = ( float )-sin( theta );

      glNormal3f( x, 0.0f, z );
      glVertex3f( radius * x, 0.0f, radius * z );
      }

	glEnd();

	if( base )
		{
		glBegin( GL_TRIANGLE_FAN );

		glNormal3f( 0.0f, -1.0f, 0.0f );
		glVertex3f( 0.0f, 0.0f, 0.0f );

		theta = 2.0f * _OGLWIN_PI;

      for( i = 0; i < 4 * complexity + 1; i++, theta -= delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

         glNormal3f( 0.0f, -1.0f, 0.0f );
         glVertex3f( radius * x, 0.0f, radius * z );
         }

		glEnd();
		}
	}


void OpenGLWindowDrawCone( OpenGLWindow *const ogl, float radius, float height, int complexity )
	{  _do_OpenGLWindowDrawCone( ogl, radius, height, complexity, 0 ); }

void OpenGLWindowDrawConeWithBase( OpenGLWindow *const ogl, float radius, float height, int complexity )
	{  _do_OpenGLWindowDrawCone( ogl, radius, height, complexity, 1 ); }


void OpenGLWindowDrawAxis( OpenGLWindow *const ogl, nsint n )
   {
   float y = ( float )( n - 2 );

	glBegin( GL_LINES );
	glVertex3f( 0.0f, y, 0.0f );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();
   }


void OpenGLWindowDrawRevolution( OpenGLWindow *const ogl, float *radii, nsint n, nsint complexity )
   {
   float  theta, x, y, z, delta, step;
   nsint    i, r;
	nsfloat  red, green, blue;
	GLint polygon_mode[2];
	GLboolean cull_face;
	GLfloat line_width;


	red   = 0.0f;
	green = .6f;
	blue  = 0.5f;

   delta = _OGLWIN_PI / 2 / complexity;
   y     = ( float )( n - 2 );
   step  = 1.0f;

   for( r = n - 1; r > 0; --r )
      {
		//red = 1.0f;
		blue -= .05f;

		theta = 0.0f;

		glBegin( GL_TRIANGLE_STRIP );

      for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

			red = ns_cosf( theta / 2.0f );
			red = NS_ABS( red );

			glColor3f( red, green, blue );

         glNormal3f( x, 0.0f, z );
         glVertex3f( radii[r]   * x, y + step, radii[r]   * z );
         glVertex3f( radii[r-1] * x, y,        radii[r-1] * z );
         }

		glEnd();

      y -= step;
      }




	complexity /= 2;

	glColor3f( 0.0f, 0.0f, 0.0f );

	glGetFloatv( GL_LINE_WIDTH, &line_width );
	glGetIntegerv( GL_POLYGON_MODE, polygon_mode );
	glGetBooleanv( GL_CULL_FACE, &cull_face );

	glLineWidth( 1.5f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glDisable( GL_CULL_FACE );

   delta = _OGLWIN_PI / 2 / complexity;
   y     = ( float )( n - 2 );
   step  = 1.0f;

   for( r = n - 1; r > 0; --r )
      {
		theta = 0.0f;

		glBegin( GL_QUAD_STRIP );

      for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
         {
         x = ( float )cos( theta );
         z = ( float )-sin( theta );

         glNormal3f( x, 0.0f, z );
         glVertex3f( radii[r]   * 1.005f * x, y + step, radii[r]  * 1.005f * z );
         glVertex3f( radii[r-1] * 1.005f * x, y,        radii[r-1]* 1.005f * z );
         }

		glEnd();

      y -= step;
      }

	if( cull_face )
		glEnable( GL_CULL_FACE );
	else
		glDisable( GL_CULL_FACE );

	glPolygonMode( GL_FRONT, polygon_mode[0] );
	glPolygonMode( GL_BACK, polygon_mode[1] );

	glLineWidth( line_width );
   }




void OpenGLWindowDrawSphere( OpenGLWindow *const ogl, float radius, nsint complexity )
   {
   float  phi, theta, delta, x, y, z, p;
   nsint    i, j;


   delta = _OGLWIN_PI / 2 / complexity;

   for( i = 0, theta = 0; i < 4 * complexity; i++, theta += delta )
      {
      glBegin( GL_TRIANGLE_STRIP );
       
      glNormal3f( 0.0f, 1.0f, 0.0f );
      glVertex3f( 0.0f, radius, 0.0f );

      for( j = 1, phi = delta; j < 2 * complexity; j++, phi += delta )
         {
         y = ( float )cos( phi );
         p = ( float )sin( phi );
         x = p * ( float )cos( theta );
         z = p * ( float )-sin( theta );

         glNormal3f( x, y, z );
         glVertex3f( radius * x, radius * y, radius * z );

         x = p * ( float )cos( theta + delta );
         z = p * ( float )-sin( theta + delta );

         glNormal3f( x, y, z );
         glVertex3f( radius * x, radius * y, radius * z );
         }

      glNormal3f( 0.0f, -1.0f, 0.0f );
      glVertex3f( 0.0f, -radius, 0.0f );

      glEnd();
      }
   }


void OpenGLWindowDrawHalfSphere( OpenGLWindow *ogl, float radius,nsint complexity)
	{
   float phi, theta, delta;
   nsint   i, j;
   float x, y, z, p;


   delta = _OGLWIN_PI / 2 / complexity;

   for( i = 0, theta = 0; i < 4 * complexity; i++, theta += delta )
		{
      glBegin( GL_TRIANGLE_STRIP );

      glNormal3f( 0.0f, 1.0f, 0.0f );
      glVertex3f( 0.0f, radius, 0.0f );

      for( j = 1, phi = delta; j <= complexity; j++, phi += delta )
			{
         y = ( float )cos( phi );
         p = ( float )sin( phi );
         x = p * ( float )cos( theta );
         z = p * ( float )-sin( theta );
            
			glNormal3f( x, y, z );
         glVertex3f( radius * x, radius * y, radius * z );

         x = p * ( float )cos( theta + delta );
         z = p * ( float )-sin( theta + delta );
            
			glNormal3f( x, y, z );
         glVertex3f( radius * x, radius * y, radius * z );
			}

      glEnd();
		}
	}


void OpenGLWindowDrawCircle( OpenGLWindow *const ogl, float radius, nsint complexity )
   {
   float  theta, x, z, delta;
   nsint    i;


   delta = _OGLWIN_PI / 2 / complexity;
   theta = 0.0f;

   glBegin( GL_TRIANGLE_FAN );
   glNormal3f( 0.0f, 1.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );

   for( i = 1 + complexity * 4; i > 0; i-- )
      {
      x = ( float )cos( theta );
      z = ( float )-sin( theta );

      glNormal3f( 0.0f, 1.0f, 0.0f );
      glVertex3f( radius * x, 0.0f, radius * z );

      theta += delta;
      }

   glEnd();
   }


void OpenGLWindowCylinder( OpenGLWindow *const ogl, nsint complexity )
   {
   if( 0 == ogl->dlCylinder )
      {
      ogl->dlCylinder = glGenLists( 1 );

      glNewList( ogl->dlCylinder, GL_COMPILE );
      OpenGLWindowDrawCylinder( ogl, 1.0f, 1.0f, 1, complexity );
      glEndList();
      }

   glCallList( ogl->dlCylinder );
   }


void DestroyOpenGLWindowCylinder( OpenGLWindow *const ogl )
   {
   if( 0 != ogl->dlCylinder )
      glDeleteLists( ogl->dlCylinder, 1 );

   ogl->dlCylinder = 0;
   }


void OpenGLWindowSphere( OpenGLWindow *const ogl, nsint complexity )
   {
   if( 0 == ogl->dlSphere )
      {
      ogl->dlSphere = glGenLists( 1 );

      glNewList( ogl->dlSphere, GL_COMPILE );
      OpenGLWindowDrawSphere( ogl, 1.0f, complexity );
      glEndList();
      }

   glCallList( ogl->dlSphere );
   }


void DestroyOpenGLWindowSphere( OpenGLWindow *const ogl )
   {
   if( 0 != ogl->dlSphere )
      glDeleteLists( ogl->dlSphere, 1 );

   ogl->dlSphere = 0;
   }


void OpenGLWindowHalfSphere( OpenGLWindow *const ogl, nsint complexity )
   {
   if( 0 == ogl->dlHalfSphere )
      {
      ogl->dlHalfSphere = glGenLists( 1 );

      glNewList( ogl->dlHalfSphere, GL_COMPILE );
      OpenGLWindowDrawHalfSphere( ogl, 1.0f, complexity );
      glEndList();
      }

   glCallList( ogl->dlHalfSphere );
   }


void DestroyOpenGLWindowHalfSphere( OpenGLWindow *const ogl )
   {
   if( 0 != ogl->dlHalfSphere )
      glDeleteLists( ogl->dlHalfSphere, 1 );

   ogl->dlHalfSphere = 0;
   }



void OpenGLWindowPerspectiveProjection
   (
   OpenGLWindow *const  ogl,
   float                fieldOfView,
   float                aspectRatio,
   float                zNear,
   float                zFar
   )
   {
   ogl->fieldOfView = fieldOfView;
   ogl->zNear       = zNear;
   ogl->zFar        = zFar;

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   gluPerspective( fieldOfView, aspectRatio, zNear, zFar );

   glMatrixMode( GL_MODELVIEW );
   }


void SetOpenGLWindowMenuText( OpenGLWindow *const ogl, nsint menuID, const char *text )
   {
   MENUITEMINFO  mii;

   mii.cbSize     = sizeof( MENUITEMINFO );
   mii.fMask      = MIIM_TYPE;
   mii.fType      = MFT_STRING;
   mii.dwTypeData = ( LPTSTR )text;
   mii.cch        = ( UINT )ns_ascii_strlen( text );

   SetMenuItemInfo( GetMenu( ogl->hWnd ), menuID, FALSE, &mii );
   }


void SetOpenGLWindowMenuChecked( OpenGLWindow *const ogl, nsint menuID, nsint checked )
   {
   HMENU         hMenu;
   MENUITEMINFO  mii;


   hMenu = GetMenu( ogl->hWnd );

   mii.cbSize = sizeof( MENUITEMINFO );
   mii.fMask  = MIIM_STATE;

   GetMenuItemInfo( hMenu, menuID, FALSE, &mii );

   if( checked )
      {
      mii.fState &= ~MFS_UNCHECKED;
      mii.fState |= MFS_CHECKED;
      }
   else
      {
      mii.fState &= ~MFS_CHECKED;
      mii.fState |= MFS_UNCHECKED;
      }

   SetMenuItemInfo( hMenu, menuID, FALSE, &mii );
   }

/*
void SetOpenGLWindowClearColor
   ( 
   OpenGLWindow *const  ogl,
   float                red,
   float                green,
   float                blue,
   float                alpha
   )
   {
   ogl->clearRed   = red;
   ogl->clearGreen = green;
   ogl->clearBlue  = blue;
   ogl->clearAlpha = alpha;

   glClearColor( red, green, blue, alpha );
   }
*/


