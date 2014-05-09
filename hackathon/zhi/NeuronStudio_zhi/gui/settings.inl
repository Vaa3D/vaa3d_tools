
/*
NS_PRIVATE void _settings_window_on_file_save( HWND hDlg )
	{
   OPENFILENAME  ofn;
	NsFile        file;
   nschar        path[ NS_PATH_SIZE ] = { '\0' };


	ns_memzero( &ofn, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = hDlg;
   ofn.lpstrFile   = path;
	ofn.lpstrTitle  = "Save Settings";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "NeuronStudio Settings (*.nss)\0*.nss\0\0\0";
   ofn.lpstrDefExt = ".nss";
   ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( GetSaveFileName( &ofn ) )
		{
		/* TEMP!!! Ignore error on file creation. *//*
		ns_file_construct( &file );
		ns_file_open( &file, path, NS_FILE_MODE_WRITE );
		ns_file_close( &file );
		ns_file_destruct( &file );

		/* Have to get the settings as they currently appear to the user. *//*
		_settings_window_obtain_them( hDlg );

		/* The write function needs a file to be there, even if its empty.
			Thats why we had to create one using the NsFile object. *//*
      ns_settings_write( path, "settings", &____settings_current );
		}
   }*/

/*
NS_PRIVATE void _settings_window_on_file_import( HWND hDlg )
	{
   OPENFILENAME  ofn;
	nschar        path[ NS_PATH_SIZE ] = { '\0' };


   ns_memzero( &ofn, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = hDlg;
   ofn.lpstrFile   = path;
	ofn.lpstrTitle  = "Import Settings";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "NeuronStudio Settings (*.nss)\0*.nss\0\0\0";
   ofn.lpstrDefExt = ".nss";
   ofn.Flags       = OFN_READONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;

   if( GetOpenFileName( &ofn ) )
		{
      ns_settings_read( path, "settings", &____settings_current );

		/* Have to set the settings so they currently appear to the user. *//*
		_settings_window_show_them( hDlg );
		}
	}*/

/*
NS_PRIVATE void _settings_window_on_examples( HWND hDlg, NsSettingsDefaultsType type )
	{
	ns_settings_choose_defaults( &____settings_current, type );
	_settings_window_show_them( hDlg );
	}
*/



extern nschar ____config_file[];

#include "tabcontrol.h"

NsSettings     ____settings_current;
NsSettings    *____settings_last_applied;
static HWND    ____settings_owner;
NsTabControl   ____settings_tc;
nsboolean     *____setttings_did_apply;


NS_PRIVATE void _settings_volume_show_them( HWND hDlg )
	{
	nschar        buffer[64];
	NsVoxelInfo  *voxel_info;
	HWND          list;
	nssize        type;


	voxel_info = ns_settings_voxel_info( &____settings_current );

	ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "%.3f", ns_voxel_info_size_x( voxel_info ) );
	SetDlgItemText( hDlg, IDE_TREE_VOX_X, buffer );

	ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "%.3f", ns_voxel_info_size_y( voxel_info ) );
	SetDlgItemText( hDlg, IDE_TREE_VOX_Y, buffer );

	if( 1 == ns_image_length( workspace_volume( s_MainWindow.activeWorkspace ) ) )
		ns_voxel_info(
			voxel_info,
			ns_voxel_info_size_x( voxel_info ),
			ns_voxel_info_size_y( voxel_info ),
			1.0f
			);

	ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "%.3f", ns_voxel_info_size_z( voxel_info ) );
	SetDlgItemText( hDlg, IDE_TREE_VOX_Z, buffer );

	list = GetDlgItem( hDlg, IDC_VOLUME_LABELING );

	for( type = 0; type < NS_LABELING_NUM_TYPES; ++type )
		SendMessage( list, CB_ADDSTRING, ( WPARAM )type, ( LPARAM )ns_labeling_type_to_string( type ) );

	SendMessage(
		list,
		CB_SELECTSTRING,
		-1,
		( LPARAM )ns_labeling_type_to_string( ns_settings_get_volume_labeling_type( &____settings_current ) )
		);

	list = GetDlgItem( hDlg, IDC_RESIDUAL_SMEAR );

	for( type = 0; type < NS_RESIDUAL_SMEAR_NUM_TYPES; ++type )
		{
		ns_snprint(
			buffer,
			sizeof( buffer ),
			"%s (%.1fx)",
			ns_residual_smear_type_to_string( type ),
			ns_residual_smear_type_to_value( type )
			);

		SendMessage( list, CB_ADDSTRING, ( WPARAM )type, ( LPARAM )buffer );
		}

	/* TEMP: Get initial selected from the settings structure. */
	ns_snprint(
		buffer,
		sizeof( buffer ),
		"%s (%.1fx)",
		ns_residual_smear_type_to_string( ns_settings_get_volume_residual_smear_type( &____settings_current ) ),
		ns_residual_smear_type_to_value( ns_settings_get_volume_residual_smear_type( &____settings_current ) )
		);

	SendMessage( list, CB_SELECTSTRING, -1, ( LPARAM )buffer );
	}


NS_PRIVATE void _settings_volume_obtain_them( HWND hDlg )
	{
	nsfloat  x, y, z;
	nschar   number[64];
	nsenum   type;


	GetDlgItemText( hDlg, IDE_TREE_VOX_X, number, sizeof( number ) - 1 );
	x = ( nsfloat )ns_atod( number );

	GetDlgItemText( hDlg, IDE_TREE_VOX_Y, number, sizeof( number ) - 1 );
	y = ( nsfloat )ns_atod( number );

	GetDlgItemText( hDlg, IDE_TREE_VOX_Z, number, sizeof( number ) - 1 );
	z = ( nsfloat )ns_atod( number );

	if( NS_FLOAT_EQUAL( x, 0.0f ) )x = NS_VOXEL_SIZE_X_DEFAULT;
	if( NS_FLOAT_EQUAL( y, 0.0f ) )y = NS_VOXEL_SIZE_Y_DEFAULT;
	if( NS_FLOAT_EQUAL( z, 0.0f ) )z = NS_VOXEL_SIZE_Z_DEFAULT;

	if( 1 == ns_image_length( workspace_volume( s_MainWindow.activeWorkspace ) ) )
		z = x;//z = 1.0f;

	ns_voxel_info( ns_settings_voxel_info( &____settings_current ), x, y, z );

	type = ( nsenum )SendMessage( GetDlgItem( hDlg, IDC_VOLUME_LABELING ), CB_GETCURSEL, 0, 0 );
	type = NS_CLAMP( type, 0, ( nsenum )( NS_LABELING_NUM_TYPES - 1 ) );

	ns_settings_set_volume_labeling_type( &____settings_current, ( NsLabelingType )type );

	type = ( nsenum )SendMessage( GetDlgItem( hDlg, IDC_RESIDUAL_SMEAR ), CB_GETCURSEL, 0, 0 );
	type = NS_CLAMP( type, 0, ( nsenum )( NS_RESIDUAL_SMEAR_NUM_TYPES - 1 ) );

	ns_settings_set_volume_residual_smear_type( &____settings_current, ( NsResidualSmearType )type );
	}


INT_PTR CALLBACK _settings_volume_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			SetFocus( GetDlgItem( hDlg, IDE_TREE_VOX_X ) );
			_settings_volume_show_them( hDlg );
			return FALSE;

		case WM_DESTROY:
			_settings_volume_obtain_them( hDlg );
			return TRUE;
		}

	return FALSE;
	}


NS_PRIVATE void _settings_neurites_show_them( HWND hDlg )
	{
   nschar number[64];
	//HWND hComboBox;
	//nsint i;


   ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_neurite_attachment_ratio( &____settings_current ) );
   SetDlgItemText( hDlg, IDE_TREE_MBR, number );

   ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_neurite_min_length( &____settings_current ) );
   SetDlgItemText( hDlg, IDE_TREE_MIN_LENGTH, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.1f", ns_settings_get_neurite_vertex_radius_scalar( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_DISCRETIZATION_RATIO, number );

	SendMessage(
		GetDlgItem( hDlg, IDC_REALIGN_JUNCTIONS ),
		BM_SETCHECK,
		ns_settings_get_neurite_realign_junctions( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED,
		0
		);

	//SendMessage( GetDlgItem( hDlg, IDC_USE_3D_DIAMETER_ESTIMATION ), BM_SETCHECK,
	//				 ns_settings_get_neurite_do_3d_radii( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED, 0 );

	/*
	hComboBox = GetDlgItem( hDlg, IDC_TREE_LABELING );

	for( i = 0; i < NS_MODEL_EDGE_LABELING_NUM_TYPES; ++i )
		SendMessage( hComboBox, CB_ADDSTRING, i,
						 ( LPARAM )ns_model_edge_labeling_string( i )
					  );
   SendMessage( hComboBox, CB_SELECTSTRING, -1,
					 ( LPARAM )ns_model_edge_labeling_string( ns_settings_get_neurite_labeling_type( &____settings_current ) )
				  );
	*/
	}


NS_PRIVATE void _settings_neurites_obtain_them( HWND hDlg )
	{
	nschar  number[64];
	//nslong  index;


	GetDlgItemText( hDlg, IDE_TREE_MBR, number, sizeof( number ) - 1 );
	ns_settings_set_neurite_attachment_ratio( &____settings_current, ( nsfloat )ns_atod( number ) );

	GetDlgItemText( hDlg, IDE_TREE_MIN_LENGTH, number, sizeof( number ) - 1 );
	ns_settings_set_neurite_min_length( &____settings_current, ( nsfloat )ns_atod( number ) );

	GetDlgItemText( hDlg, IDE_DISCRETIZATION_RATIO, number, sizeof( number ) - 1 );
	ns_settings_set_neurite_vertex_radius_scalar( &____settings_current, ( nsfloat )ns_atod( number ) );

	ns_settings_set_neurite_realign_junctions(
		&____settings_current,
		( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_REALIGN_JUNCTIONS ), BM_GETCHECK, 0, 0 )
		);


	//ns_settings_set_neurite_do_3d_radii( &____settings_current,
	//	( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_USE_3D_DIAMETER_ESTIMATION ), BM_GETCHECK, 0, 0 ) );
/*
	index = SendMessage( GetDlgItem( hDlg, IDC_TREE_LABELING ), CB_GETCURSEL, 0, 0 );
   ns_assert( index < NS_MODEL_EDGE_LABELING_NUM_TYPES );

	ns_settings_set_neurite_labeling_type( &____settings_current, ( nsint )index );*/
	}


extern nsboolean _settings_confirm_refiltering( NsSettings *current, NsSettings *last_applied );
extern NsError workspace_model_filtering( nsuint handle, const NsSettings *settings );
extern nsuint *____active_workspace;
extern void ____redraw_all( void );

INT_PTR CALLBACK _settings_neurites_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			SetFocus( GetDlgItem( hDlg, IDE_TREE_MBR ) );
			_settings_neurites_show_them( hDlg );
			return FALSE;

		case WM_DESTROY:
			_settings_neurites_obtain_them( hDlg );
			return TRUE;

		case WM_COMMAND:
			if( IDB_SETTINGS_NEURITES_APPLY == LOWORD( wParam ) )
				{
				_settings_neurites_obtain_them( hDlg );

				if( _settings_confirm_refiltering( &____settings_current, ____settings_last_applied ) )
					{
					workspace_model_filtering( *____active_workspace, &____settings_current );
					*____settings_last_applied = ____settings_current;

					____redraw_all();
					}
				}
			break;
		}

	return FALSE;
	}


NS_PRIVATE void _settings_spines_show_them( HWND hDlg )
	{
   nschar number[64];
	//nsfloat32 f32;
	//nsint32   i32;
	//nsfloat64 f64;

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_spine_max_voxel_distance( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_ZONE_THICKNESS, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_spine_max_layer_spread( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_MAX_WIDTH, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_spine_elim_height( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_MIN_HEIGHT, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), NS_FMT_ULONG, ns_settings_get_spine_min_attached_voxels( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_MIN_ATTACHED_SIZE, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), NS_FMT_ULONG, ns_settings_get_spine_min_detached_voxels( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_MIN_DETACHED_SIZE, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_spine_z_spread_divisor( &____settings_current ) );
	SetDlgItemText( hDlg, IDE_SPINES_Z_SMEAR_COMPENSATION, number );

	EnableWindow(
		GetDlgItem( hDlg, IDE_SPINES_Z_SMEAR_COMPENSATION ),
		! ns_settings_get_spine_z_spread_automated( &____settings_current )
		);

	SendMessage(
		GetDlgItem( hDlg, IDC_SPINES_Z_SMEAR_COMPENSATION ),
		BM_SETCHECK,
		 ns_settings_get_spine_z_spread_automated( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED,
		0
		);

	//ns_snprint( number, "%.3f", ns_settings_get_spine_min_width( &____settings_current ) );
	//SetDlgItemText( hDlg, IDE_SPINES_MIN_WIDTH, number );


	//f32 = ns_settings_get_spine_min_volume( &____settings_current ) / ns_voxel_info_volume( &____settings_current.voxel_info );
	//NS_FLOAT32_TO_INT32_WITH_PROPER_ROUNDING( f32, i32, f64 );
	//ns_snprint( number, "%d", i32 );
	//SetDlgItemText( hDlg, IDE_SPINES_MIN_VOLUME, number );

	//f32 = ns_settings_get_spine_max_volume( &____settings_current ) / ns_voxel_info_volume( &____settings_current.voxel_info );
	//NS_FLOAT32_TO_INT32_WITH_PROPER_ROUNDING( f32, i32, f64 );
	//ns_snprint( number, "%d", i32 );
	//SetDlgItemText( hDlg, IDE_SPINES_MAX_VOLUME, number );



	//SendMessage( GetDlgItem( hDlg, IDC_SPINE_ENHANCED_DETECTION ), BM_SETCHECK,
	//				 ns_settings_get_spine_enhanced_detection( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED, 0 );
	}


NS_PRIVATE void _settings_spines_obtain_them( HWND hDlg )
	{
	nschar number[64];

	GetDlgItemText( hDlg, IDE_SPINES_ZONE_THICKNESS, number, sizeof( number ) - 1 );
	ns_settings_set_spine_max_voxel_distance( &____settings_current, ( nsfloat )ns_atod( number ) );

	GetDlgItemText( hDlg, IDE_SPINES_MAX_WIDTH, number, sizeof( number ) - 1 );
	ns_settings_set_spine_max_layer_spread( &____settings_current, ( nsfloat )ns_atod( number ) );

	GetDlgItemText( hDlg, IDE_SPINES_MIN_HEIGHT, number, sizeof( number ) - 1 );
	ns_settings_set_spine_elim_height( &____settings_current, ( nsfloat )ns_atod( number ) );

	GetDlgItemText( hDlg, IDE_SPINES_MIN_ATTACHED_SIZE, number, sizeof( number ) - 1 );
	ns_settings_set_spine_min_attached_voxels( &____settings_current, ( nssize )ns_atoi( number ) );

	GetDlgItemText( hDlg, IDE_SPINES_MIN_DETACHED_SIZE, number, sizeof( number ) - 1 );
	ns_settings_set_spine_min_detached_voxels( &____settings_current, ( nssize )ns_atoi( number ) );

	GetDlgItemText( hDlg, IDE_SPINES_Z_SMEAR_COMPENSATION, number, sizeof( number ) - 1 );
	ns_settings_set_spine_z_spread_divisor( &____settings_current, ( nsfloat )ns_atod( number ) );

	ns_settings_set_spine_z_spread_automated(
		&____settings_current,
		( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_SPINES_Z_SMEAR_COMPENSATION ), BM_GETCHECK, 0, 0 )
		);

	//GetDlgItemText( hDlg, IDE_SPINES_MIN_WIDTH, number, sizeof( number ) - 1 );
	//ns_settings_set_spine_min_width( &____settings_current, ( nsfloat )ns_atod( number ) );

	//GetDlgItemText( hDlg, IDE_SPINES_MIN_VOLUME, number, sizeof( number ) - 1 );
	//ns_settings_set_spine_min_volume( &____settings_current, ( nsfloat )ns_atod( number ) * ns_voxel_info_volume( &____settings_current.voxel_info ) );
	
	//GetDlgItemText( hDlg, IDE_SPINES_MAX_VOLUME, number, sizeof( number ) - 1 );
	//ns_settings_set_spine_max_volume( &____settings_current, ( nsfloat )ns_atod( number ) * ns_voxel_info_volume( &____settings_current.voxel_info ) );

	//ns_settings_set_spine_enhanced_detection( &____settings_current,
	//	( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_SPINE_ENHANCED_DETECTION ), BM_GETCHECK, 0, 0 ) );
	}


INT_PTR CALLBACK _settings_spines_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_INITDIALOG:
			SetFocus( GetDlgItem( hDlg, IDE_SPINES_MIN_HEIGHT ) );
			_settings_spines_show_them( hDlg );
			return FALSE;

		case WM_DESTROY:
			_settings_spines_obtain_them( hDlg );
			return TRUE;

		case WM_COMMAND:
			if( IDC_SPINES_Z_SMEAR_COMPENSATION == LOWORD( wParam ) )
				{
				EnableWindow(
					GetDlgItem( hDlg, IDE_SPINES_Z_SMEAR_COMPENSATION ),
					! SendMessage( GetDlgItem( hDlg, IDC_SPINES_Z_SMEAR_COMPENSATION ), BM_GETCHECK, 0, 0 )
					);

				return TRUE;
				}
			break;
		}

	return FALSE;
	}


NS_PRIVATE void _settings_threshold_show_them( HWND hDlg )
	{
	nschar number[64];

	ns_snprint(
		number,
		NS_ARRAY_LENGTH( number ),
		"%.3f",
		ns_voxel_intensity_convert(
			ns_settings_get_threshold_fixed_value( &____settings_current ),
			workspace_pixel_type( s_MainWindow.activeWorkspace ),
			NS_PIXEL_LUM_U8
			)
		);
	SetDlgItemText( hDlg, IDE_FIXED_THRESHOLD_VALUE, number );

	ns_snprint(
		number,
		NS_ARRAY_LENGTH( number ),
		"%.1f",
		( 1.0f - ns_settings_get_threshold_multiplier( &____settings_current ) ) * 100.0f
		);
	SetDlgItemText( hDlg, IDE_SPINE_THRESHOLD_CORRECTION, number );

	//SendMessage(
	//	GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ),
	//	BM_SETCHECK,
	//	ns_settings_get_threshold_auto_correct( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED,
	//	0
	//	);

	SendMessage(
		GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ),
		BM_SETCHECK,
		ns_settings_get_neurite_use_random_sampling( &____settings_current ) ? BST_CHECKED : BST_UNCHECKED,
		0
		);

	if( ns_settings_get_threshold_use_fixed( &____settings_current ) )
		{
		SendMessage( GetDlgItem( hDlg, IDC_USE_FIXED_THRESHOLD ), BM_SETCHECK, BST_CHECKED, 0 );
		SendMessage( GetDlgItem( hDlg, IDC_USE_DYNAMIC_THRESHOLD ), BM_SETCHECK, BST_UNCHECKED, 0 );

		EnableWindow( GetDlgItem( hDlg, IDE_FIXED_THRESHOLD_VALUE ), TRUE );
		//EnableWindow( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), FALSE );
		//EnableWindow( GetDlgItem( hDlg, IDE_SPINE_THRESHOLD_CORRECTION ), TRUE );
		EnableWindow( GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ), FALSE );
		}
	else
		{
		SendMessage( GetDlgItem( hDlg, IDC_USE_FIXED_THRESHOLD ), BM_SETCHECK, BST_UNCHECKED, 0 );
		SendMessage( GetDlgItem( hDlg, IDC_USE_DYNAMIC_THRESHOLD ), BM_SETCHECK, BST_CHECKED, 0 );

		EnableWindow( GetDlgItem( hDlg, IDE_FIXED_THRESHOLD_VALUE ), FALSE );
		//EnableWindow( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), TRUE );
		//EnableWindow(
		//	GetDlgItem( hDlg, IDE_SPINE_THRESHOLD_CORRECTION ),
			//! ns_settings_get_threshold_auto_correct( &____settings_current )
		//	);

		EnableWindow( GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ), TRUE );
		}
	}


NS_PRIVATE void _settings_threshold_obtain_them( HWND hDlg )
	{
	nsfloat  threshold;
	nschar   number[64];


	ns_settings_set_neurite_use_random_sampling(
		&____settings_current,
		( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ), BM_GETCHECK, 0, 0 )
		);

	ns_settings_set_threshold_use_fixed(
		&____settings_current,
		( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_USE_FIXED_THRESHOLD ), BM_GETCHECK, 0, 0 )
		);

	if( ns_settings_get_threshold_use_fixed( &____settings_current ) )
		{
		GetDlgItemText( hDlg, IDE_FIXED_THRESHOLD_VALUE, number, sizeof( number ) - 1 );

		threshold = ( nsfloat )ns_atod( number );

		if( threshold < 0.0f )
			threshold = 0.0f;
		else if( 255.0f < threshold )
			threshold = 255.0f;

		threshold =
			ns_voxel_intensity_convert(
				threshold,
				NS_PIXEL_LUM_U8,
				workspace_pixel_type( s_MainWindow.activeWorkspace )
				);

		ns_settings_set_threshold_fixed_value( &____settings_current, threshold );
		}

	//ns_settings_set_threshold_auto_correct(
	//	&____settings_current,
	//	( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), BM_GETCHECK, 0, 0 )
	//	);

	//if( ns_settings_get_threshold_auto_correct( &____settings_current ) )
	//	ns_settings_set_threshold_multiplier( &____settings_current, 1.0f );
	//else
	//	{
		GetDlgItemText( hDlg, IDE_SPINE_THRESHOLD_CORRECTION, number, sizeof( number ) - 1 );

		ns_settings_set_threshold_multiplier(
			&____settings_current,
			1.0f - ( ( nsfloat )ns_atod( number ) ) / 100.0f
			);
	//	}
	}


INT_PTR CALLBACK _settings_threshold_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
	nsint id;

	switch( uMsg )
		{
		case WM_INITDIALOG:
			//SetFocus( GetDlgItem( hDlg, IDE_SPINES_MIN_HEIGHT ) );
			_settings_threshold_show_them( hDlg );
			return FALSE;

		case WM_COMMAND:
			id = LOWORD( wParam );

			switch( id )
				{
				case IDC_USE_FIXED_THRESHOLD:
					if( ! SendMessage( GetDlgItem( hDlg, id ), BM_GETCHECK, 0, 0 ) )
						SendMessage( GetDlgItem( hDlg, id ), BM_SETCHECK, BST_CHECKED, 0 );
					else
						{
						SendMessage( GetDlgItem( hDlg, IDC_USE_DYNAMIC_THRESHOLD ), BM_SETCHECK, BST_UNCHECKED, 0 );

						EnableWindow( GetDlgItem( hDlg, IDE_FIXED_THRESHOLD_VALUE ), TRUE );
						//EnableWindow( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), FALSE );
						//EnableWindow( GetDlgItem( hDlg, IDE_SPINE_THRESHOLD_CORRECTION ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ), FALSE );
						}
					break;

				case IDC_USE_DYNAMIC_THRESHOLD:
					if( ! SendMessage( GetDlgItem( hDlg, id ), BM_GETCHECK, 0, 0 ) )
						SendMessage( GetDlgItem( hDlg, id ), BM_SETCHECK, BST_CHECKED, 0 );
					else
						{
						SendMessage( GetDlgItem( hDlg, IDC_USE_FIXED_THRESHOLD ), BM_SETCHECK, BST_UNCHECKED, 0 );

						EnableWindow( GetDlgItem( hDlg, IDE_FIXED_THRESHOLD_VALUE ), FALSE );
						//EnableWindow( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), TRUE );
						//EnableWindow(
						//	GetDlgItem( hDlg, IDE_SPINE_THRESHOLD_CORRECTION ),
						//	! SendMessage( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), BM_GETCHECK, 0, 0 )
						//	);
						EnableWindow( GetDlgItem( hDlg, IDC_USE_SCATTERED_SAMPLING ), TRUE );
						}
					break;

				//case IDC_AUTO_SPINE_CORRECTION:
				//	EnableWindow(
				//		GetDlgItem( hDlg, IDE_SPINE_THRESHOLD_CORRECTION ),
				//		! SendMessage( GetDlgItem( hDlg, IDC_AUTO_SPINE_CORRECTION ), BM_GETCHECK, 0, 0 )
				//		);
				//	break;
				}
			return TRUE;

		case WM_DESTROY:
			_settings_threshold_obtain_them( hDlg );
			return TRUE;
		}

	return FALSE;
	}


INT_PTR CALLBACK _settings_defaults_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
				case IDB_SETTINGS_SAVE_CURRENT:
					{
					NsSettings temp = ____settings_current;

					/* NOTE: Dont save the seed location for the default settings. */
					ns_vector3i_zero( ns_settings_neurite_seed( &temp ) );

					ns_settings_write( ____config_file, "settings", &temp );
					ns_settings_update_user_defaults( &temp );
					}
					break;

				case IDB_SETTINGS_LOAD_INITIAL:
					{
					NsVector3i seed;

					seed = *( ns_settings_neurite_seed( &____settings_current ) );

					ns_settings_init_with_program_defaults( &____settings_current );

					*( ns_settings_neurite_seed( &____settings_current ) ) = seed;
					
					ns_settings_set_threshold_fixed_value(
						&____settings_current,
						ns_voxel_intensity_convert(
							ns_settings_get_threshold_fixed_value( &____settings_current ),
							NS_PIXEL_LUM_U8,
							workspace_pixel_type( s_MainWindow.activeWorkspace )
							)
						);
					}
					break;
				}
			break;
		}

	return FALSE;
	}


NS_PRIVATE void _settings_window_create_tabs( void )
	{
	ns_tab_control_add(
		&____settings_tc,
		"Volume",
		"SETTINGS_VOLUME",
		_settings_volume_dialog_proc
		);

	ns_tab_control_add(
		&____settings_tc,
		"Neurites",
		"SETTINGS_NEURITES",
		_settings_neurites_dialog_proc
		);

	ns_tab_control_add(
		&____settings_tc,
		"Spines",
		"SETTINGS_SPINES",
		_settings_spines_dialog_proc
		);

	ns_tab_control_add(
		&____settings_tc,
		"Threshold",
		"SETTINGS_THRESHOLD",
		_settings_threshold_dialog_proc
		);
	
	ns_tab_control_add(
		&____settings_tc,
		"Defaults",
		"SETTINGS_DEFAULTS",
		_settings_defaults_dialog_proc
		);

	ns_tab_control_on_select( &____settings_tc );
	}


INT_PTR CALLBACK _SettingsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         CenterWindow( hDlg, ____settings_owner );

			ns_tab_control_init(
				&____settings_tc,
				g_Instance,
				hDlg,
				IDT_SETTINGS,
				5
				);

			_settings_window_create_tabs();

         return FALSE;

		case WM_NOTIFY:
			switch( ( ( LPNMHDR )lParam )->code )
				{
				case TCN_SELCHANGE:
					ns_tab_control_on_select( &____settings_tc );
					break;
				} 
			break; 

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
				//case IDE_TREE_MBR:
				//	if( EN_UPDATE == HIWORD( wParam ) )
				//		s_SettingsFlags |= BUILD_TREE_SETTINGS_MINIMUM_BRANCH_RATIO;
				//	break;

//            case IDC_TREE_LABELING:
  //             if( CBN_SELCHANGE == HIWORD( wParam ) )
    //              {
      //            nslong index = SendMessage( GetDlgItem( hDlg, IDC_TREE_LABELING ), CB_GETCURSEL, 0, 0 );
        //          ns_assert( index < NS_MODEL_EDGE_LABELING_NUM_TYPES );

          //        ns_settings_get_neurite_labeling_type( &____settings_current ) = ( nsint )index;
		//				s_BuildTreeSettingsFlags |= BUILD_TREE_SETTINGS_BRANCH_LABELING_TYPE;
            //      }   
              // break;


            case IDOK:
					//_settings_window_obtain_them( hDlg );
					EndDialog( hDlg, 1 );
               break;

            case IDCANCEL:
               EndDialog( hDlg, 0 );
               break;

/*
				case IDM_SETTINGS_FILE_SAVE:
					_settings_window_on_file_save( hDlg );
					break;

				case IDM_SETTINGS_FILE_IMPORT:
					_settings_window_on_file_import( hDlg );
					break;

				case IDM_SETTINGS_DEFAULTS_SET:
					if( IDYES == MessageBox( hDlg,
						 "Are you sure you want to set the current settings\n"
						 "as the default settings?",
						 "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						{
						_settings_window_obtain_them( hDlg );
						ns_settings_write( ____config_file, "settings", &____settings_current );
						}
					break;

				case IDM_SETTINGS_DEFAULTS_RESTORE:
					if( IDYES == MessageBox( hDlg,
						 "Are you sure you want to restore the current settings\n"
						 "from the default settings?",
						 "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						{
						ns_settings_defaults( &____settings_current );
						ns_settings_read( ____config_file, "settings", &____settings_current );

						_settings_window_show_them( hDlg );
						}
					break;

				case IDM_SETTINGS_EXAMPLES_1x1x1_VOXELS:
					if( IDYES == MessageBox( hDlg,
						 "Are you sure you want to overwrite the current settings\n"
						 "with example settings for a voxel size of 1.0 x 1.0 x 1.0?",
						 "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						_settings_window_on_examples( hDlg, NS_SETTINGS_DEFAULTS_1p0x1p0x1p0 );
					break;

				case IDM_SETTINGS_EXAMPLES_p2xp2xp2_VOXELS:
					if( IDYES == MessageBox( hDlg,
						 "Are you sure you want to overwrite the current settings\n"
						 "with example settings for a voxel size of 0.2 x 0.2 x 0.2?",
						 "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						_settings_window_on_examples( hDlg, NS_SETTINGS_DEFAULTS_0p2x0p2x0p2 );
					break;

				case IDM_SETTINGS_EXAMPLES_p1xp1xp1_VOXELS:
					if( IDYES == MessageBox( hDlg,
						 "Are you sure you want to overwrite the current settings\n"
						 "with example settings for a voxel size of 0.1 x 0.1 x 0.1?",
						 "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						_settings_window_on_examples( hDlg, NS_SETTINGS_DEFAULTS_0p1x0p1x0p1 );
					break;
*/
            }
         return TRUE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
      }
   
   return FALSE;
   }


nsint SettingsDialog
	(
	HWND         owner,
   NsSettings  *current_and_return,
	NsSettings  *last_applied,
	NsModel     *raw_model,
	NsModel     *filtered_model
	)
	{
   nsint         retValue;
	//NsVoxelInfo  *O, *N;
	//nschar        msg[ 256 ];


   ____settings_owner = owner;

	____settings_current       = *current_and_return;
	____settings_last_applied  = last_applied;
	*____settings_last_applied = *current_and_return;

   retValue = ( nsint )DialogBox( g_Instance, "BUILD_TREE_DIALOG", owner, _SettingsDlgProc );

   if( 1 == retValue )
		{
		/*if( ! NS_FLOAT_EQUAL(
					ns_settings_get_neurite_attachment_ratio( &____settings_current ),
				   ns_settings_get_neurite_attachment_ratio( settings ) ) )
			( *flags ) |= NS_SETTING_NEURITE_ATTACHMENT_RATIO;
		else
			( *flags ) &= ( ~NS_SETTING_NEURITE_ATTACHMENT_RATIO );

		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_neurite_min_length( &____settings_current ),
				   ns_settings_get_neurite_min_length( settings ) ) )
			( *flags ) |= NS_SETTING_NEURITE_MIN_LENGTH;
		else
			( *flags ) &= ( ~NS_SETTING_NEURITE_MIN_LENGTH );*/

		//if( ns_settings_get_neurite_labeling_type( &____settings_current ) !=
		//	 ns_settings_get_neurite_labeling_type( settings ) )
		//	( *flags ) |= NS_SETTING_NEURITE_LABELING_TYPE;

		//if( ns_settings_get_neurite_do_3d_radii( &____settings_current ) !=
		//	 ns_settings_get_neurite_do_3d_radii( settings ) )
		//	( *flags ) |= NS_SETTING_NEURITE_DO_3D_RADII;

/*
		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_max_voxel_distance( &____settings_current ),
					ns_settings_get_spine_max_voxel_distance( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_MAX_VOXEL_DISTANCE;
		else
			( *flags ) &= ( ~NS_SETTING_SPINE_MAX_VOXEL_DISTANCE );

		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_max_layer_spread( &____settings_current ),
					ns_settings_get_spine_max_layer_spread( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_MAX_LAYER_SPREAD;
		else
			( *flags ) &= ( ~NS_SETTING_SPINE_MAX_LAYER_SPREAD );

		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_elim_height( &____settings_current ),
					ns_settings_get_spine_elim_height( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_ELIM_HEIGHT;
		else
			( *flags ) &= ( ~NS_SETTING_SPINE_ELIM_HEIGHT );
		*/

		/*if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_min_width( &____settings_current ),
					ns_settings_get_spine_min_width( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_MIN_WIDTH;*/

		/*if( ns_settings_get_spine_enhanced_detection( &____settings_current ) !=
			 ns_settings_get_spine_enhanced_detection( settings ) )
			( *flags ) |= NS_SETTING_SPINE_ENHANCED_DETECTION;*/

		/*
		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_min_volume( &____settings_current ),
					ns_settings_get_spine_min_volume( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_MIN_VOLUME;
		else
			( *flags ) &= ( ~NS_SETTING_SPINE_MIN_VOLUME );
		*/

		/*if( ! NS_FLOAT_EQUAL(
					ns_settings_get_spine_max_volume( &____settings_current ),
					ns_settings_get_spine_max_volume( settings ) ) )
			( *flags ) |= NS_SETTING_SPINE_MAX_VOLUME;
		else
			( *flags ) &= ( ~NS_SETTING_SPINE_MAX_VOLUME );*/

		/*
		if( ! NS_FLOAT_EQUAL(
					ns_voxel_info_size_x( ns_settings_voxel_info( &____settings_current ) ),
					ns_voxel_info_size_x( ns_settings_voxel_info( settings ) ) ) ||
			 ! NS_FLOAT_EQUAL(
					ns_voxel_info_size_y( ns_settings_voxel_info( &____settings_current ) ),
					ns_voxel_info_size_y( ns_settings_voxel_info( settings ) ) ) ||
			 ! NS_FLOAT_EQUAL(
					ns_voxel_info_size_z( ns_settings_voxel_info( &____settings_current ) ),
					ns_voxel_info_size_z( ns_settings_voxel_info( settings ) ) )   )
			( *flags ) |= NS_SETTING_VOXEL_SIZE;
		else
			( *flags ) &= ( ~NS_SETTING_VOXEL_SIZE );
		*/

		/*
		if( ns_settings_get_threshold_use_fixed( &____settings_current ) !=
			 ns_settings_get_threshold_use_fixed( settings ) )
			( *flags ) |= NS_SETTING_THRESHOLD_USE_FIXED;

		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_threshold_fixed_value( &____settings_current ),
					ns_settings_get_threshold_fixed_value( settings ) ) )
			( *flags ) |= NS_SETTING_THRESHOLD_FIXED_VALUE;

		if( ! NS_FLOAT_EQUAL(
					ns_settings_get_threshold_multiplier( &____settings_current ),
					ns_settings_get_threshold_multiplier( settings ) ) )
			( *flags ) |= NS_SETTING_THRESHOLD_MULTIPLIER;
		*/


		/*
		if( 0 < ns_model_num_vertices( raw_model ) || 0 < ns_model_num_vertices( filtered_model ) )
			{
			N = ns_settings_voxel_info( &____settings_current );
			O = ns_settings_voxel_info( current_and_return );

			if( ! NS_FLOAT_EQUAL( ns_voxel_info_size_x( N ), ns_voxel_info_size_x( O ) ) ||
				 ! NS_FLOAT_EQUAL( ns_voxel_info_size_y( N ), ns_voxel_info_size_y( O ) ) ||
				 ! NS_FLOAT_EQUAL( ns_voxel_info_size_z( N ), ns_voxel_info_size_z( O ) )   )
				if( NS_FLOAT_EQUAL(
						ns_voxel_info_size_x( N ) / ns_voxel_info_size_x( O ),
						ns_voxel_info_size_y( N ) / ns_voxel_info_size_y( O )
						) )
					{
					ns_snprint(
						msg,
						sizeof( msg ),
						"%s%s",
						"You have changed the voxel size. Would you like to rescale the model?",
						0 < ns_model_num_spines( filtered_model ) ? "\n" : "TODO"
						);
						
					if( IDYES == MessageBox( owner, msg, "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
						{
						ns_model_resize( raw_model, ns_voxel_info_size( O ), ns_voxel_info_size( N ) );
						ns_model_resize( filtered_model, ns_voxel_info_size( O ), ns_voxel_info_size( N ) );
						}
					}

			}*/

		*current_and_return = ____settings_current;

//ns_println( "SPINE-VOLUME: %f<-->%f", ns_settings_get_spine_min_volume( settings ), ns_settings_get_spine_max_volume( settings ) );

		____volume_grid_step = -1.0f;
		}
   
   return ( 1 == retValue ) ? 1 : 0;
	}
