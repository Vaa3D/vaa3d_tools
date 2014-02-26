

void _command_line_sum_intensities( const NsVector3i *V, nsulong intensity, nsulonglong *intensity_sum )
	{  *intensity_sum += ( nsulonglong )intensity;  }


void _command_line_application( nsint argc, nschar *argv[] )
   {
   nsint           wasCancelled;
   nsint           error;
   WorkspaceArgs   inArgs;
   NsImageDir      dir;
	nssize          prefix_length;
   nsint           file_type;
	NsSwcHeader     swc_header;
	NsVoxelBuffer   voxel_buffer;
	nsulonglong     intensity_sum;
	nsdouble        average_intensity;
	const NsImage  *volume;
	NsSettings     *settings;
	SYSTEMTIME      st;
	nschar         *file_name;
	nschar         *extension;
	NsVector3i      seed;
	nschar          output_dir[ NS_PATH_SIZE ];
	nschar          output_file[ NS_PATH_SIZE ];




ns_print_newline();

	if( 15 != argc )
		{
		ns_println(
			"usage: ns <file> <voxel-size-x> <voxel-size-y> <voxel-size-z>\n"
			"       <neurites-attach-ratio> <neurites-min-length> <spines-min-height>\n"
			"       <spines-max-height> <spines-max-width> <spines-min-voxels>\n"
			"       <spines-correction-percent> <seed-x|-1> <seed-y|-1> <seed-z|-1>\n"
			"\n"
			"example:\n"
			"   C:\\usr\\douglas\\ns_tests\\crop_a_small_blurred\\a0001.tif .1 .1 .1\n"
			"   1.5 3.0 .2 3.0 3.0 2 10.0 62 68 96\n"
			);
		_getche();
		return;
		}

ns_print_newline();



	s_MainWindow.hWnd = NULL;
	ns_ascii_strcpy( s_MainWindow.path, argv[1] );
	_read_jitter_from_config_file();

	ns_ascii_strcpy( output_dir, s_MainWindow.path );

	if( NULL == ( file_name = ns_ascii_strrchr( output_dir, NS_UNICHAR_PATH ) ) )
		file_name = output_dir;
	else
		*file_name++ = NS_ASCII_NULL;

	if( NULL == ( extension = ns_ascii_strrchr( file_name, '.' ) ) )
		extension = "";
	else
		*extension++ = NS_ASCII_NULL;

	ns_println( "output_dir = %s", output_dir );
	ns_println( "file_name  = %s", file_name );
	ns_println( "extension  = %s", extension );

	if( NS_FAILURE( ns_chdir( output_dir ), ( NsError )error ) )
		{
		MessageBox( s_MainWindow.hWnd, "The output directory is not valid.", "NeuronStudio", MB_OK | MB_ICONERROR );
		return;
		}



	ns_image_dir_construct( &dir );

	switch( ____detect_sequence( &dir ) )
		{
		case IDNO:
			inArgs.use_path = NS_TRUE;
			break;

		case IDYES:
			inArgs.use_path = NS_FALSE;
			break;

		case IDCANCEL:
			return;

		default:
			ns_assert_not_reached();
		}

	if( eIMAGE_FILE_RAW == ( file_type = GetImageFileType( s_MainWindow.path ) ) )
		{
	//	if( inArgs.use_path ) /* 3D? */
	//		{
	//		if( ! _describe_raw_dialog( NS_TRUE ) )
	//			return;
	//		}
	//	else
	//		{
	//		if( ! _describe_raw_dialog( NS_FALSE ) )
	//			return;
	//		}
		}

   inArgs.path   = s_MainWindow.path;
   inArgs.window = NULL;
   //inArgs.do_sequence = do_sequence;
   inArgs.dir = &dir;

   ns_log_entry(
      NS_LOG_ENTRY_FUNCTION,
      NS_FUNCTION
      "( path=" NS_FMT_STRING_DOUBLE_QUOTED
      ", file_type=" NS_FMT_INT
      ", is_sequence=" NS_FMT_INT
      " )",
      s_MainWindow.path,
      file_type,
      ! inArgs.use_path
      );

   s_MainWindow.activeWorkspace = 0;

   error = ProgressDialog( NULL,
                           NULL/*"Running Maximal Projections..."*/,
                           BuildWorkspace,
                           &inArgs,
                           &s_MainWindow.activeWorkspace,//&hWorkspace,
                           &wasCancelled
                         );

	prefix_length = ns_image_dir_prefix_length( &dir );
    ns_image_dir_destruct( &dir );

/*TEMP*/ns_chdir( _startup_directory );
       
   if( ! wasCancelled )
      {
      if( eNO_ERROR != error )
         {
         MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ error ], "NeuronStudio", MB_OK | MB_ICONERROR );
         g_Error = eNO_ERROR;
			return;
         }
      else
         {
         _make_file_filter( s_MainWindow.path, prefix_length, s_MainWindow.filter );
         _read_settings();
         }
      }
   else
      {
      if( WorkspaceIsValid( s_MainWindow.activeWorkspace/*hWorkspace*/ ) )
         DestructWorkspace( s_MainWindow.activeWorkspace/*hWorkspace*/ );

		return;
      }



	workspace_set_voxel_info(
		s_MainWindow.activeWorkspace,
		( nsfloat )ns_atod( argv[2] ),
		( nsfloat )ns_atod( argv[3] ),
		( nsfloat )ns_atod( argv[4] )
		);

	settings = workspace_settings( s_MainWindow.activeWorkspace );

	ns_settings_set_neurite_attachment_ratio( settings, ( nsfloat )ns_atod( argv[5] ) );
	ns_settings_set_neurite_min_length( settings, ( nsfloat )ns_atod( argv[6] ) );
	ns_settings_set_spine_elim_height( settings, ( nsfloat )ns_atod( argv[7] ) );
	ns_settings_set_spine_max_voxel_distance( settings, ( nsfloat )ns_atod( argv[8] ) );
	ns_settings_set_spine_max_layer_spread( settings, ( nsfloat )ns_atod( argv[9] ) );

	ns_settings_set_spine_min_volume(
		settings,
		( nsfloat )ns_atod( argv[10] ) * ns_voxel_info_volume( ns_settings_voxel_info( settings ) )
		);

	ns_settings_set_threshold_multiplier(
		settings,
		1.0f - ( ( nsfloat )ns_atod( argv[11] ) ) / 100.0f
		);

	seed.x = ns_atoi( argv[12] );	
	seed.y = ns_atoi( argv[13] );
	seed.z = ns_atoi( argv[14] );

	volume = workspace_volume( s_MainWindow.activeWorkspace );

	if( seed.x < 0 || ( nsint )ns_image_width( volume )  <= seed.x ||
		 seed.y < 0 || ( nsint )ns_image_height( volume ) <= seed.y ||
		 seed.z < 0 || ( nsint )ns_image_length( volume ) <= seed.z   )
		{
		MessageBox( s_MainWindow.hWnd, "The seed is not valid.", "NeuronStudio", MB_OK | MB_ICONERROR );
		DestructWorkspace( s_MainWindow.activeWorkspace );
		return;
		}


	ns_print( "Computing average intensity... " );

	intensity_sum = 0;

	ns_voxel_buffer_init( &voxel_buffer, workspace_volume( s_MainWindow.activeWorkspace ) );
	ns_voxel_buffer_forall( &voxel_buffer, NS_VOXEL_BUFFER_FORALL_XY, _command_line_sum_intensities, &intensity_sum, NULL );

   average_intensity  = ( nsdouble )intensity_sum;
   average_intensity /= ( nsdouble )ns_voxel_buffer_width( &voxel_buffer );
   average_intensity /= ( nsdouble )ns_voxel_buffer_height( &voxel_buffer );
   average_intensity /= ( nsdouble )ns_voxel_buffer_length( &voxel_buffer );

	workspace_set_average_intensity( s_MainWindow.activeWorkspace, average_intensity );

	ns_println( "done. Average intensity is %f", average_intensity );




	____grafting_is_running = NS_TRUE;

   error = BuildWorkspaceNeuronTree( NULL, s_MainWindow.activeWorkspace, NS_TRUE );

   if( eNO_ERROR != error )
      {
      MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );
      g_Error = eNO_ERROR;
		DestructWorkspace( s_MainWindow.activeWorkspace );
		return;
      }

   _write_settings();



   ns_memzero( &swc_header, sizeof( NsSwcHeader ) );

	ns_sprint( swc_header.source, "NeuronStudio %d.%d.%d",NS_VERSION_MAJOR, NS_VERSION_MINOR, NS_VERSION_MICRO );

	ns_ascii_strcpy( swc_header.shrinkage, "1.0 1.0 1.0" );

	ns_ascii_strcpy( swc_header.version_number, "1.0" );

	GetSystemTime( &st );
	ns_sprint( swc_header.version_date, "%d-%02d-%02d", (nsint)st.wYear, (nsint)st.wMonth, (nsint)st.wDay );

	ns_ascii_strcpy( swc_header.scale, "1.0 1.0 1.0" );

	ns_sprint( output_file, "%s" NS_STRING_PATH "%s.swc", output_dir, file_name );
	ns_println( "MODEL FILE = %s", output_file );

	error = ns_model_write_swc(
						GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
						&swc_header,
						output_file,
						0
						);

   if( ns_is_error( error ) )
		{
      MessageBox(
         s_MainWindow.hWnd,
         ns_error_code_to_string( error ),
         "NeuronStudio",
         MB_OK | MB_ICONERROR
         );
		DestructWorkspace( s_MainWindow.activeWorkspace );
		return;
		}




   error = workspace_run_spine_analysis( NULL, s_MainWindow.activeWorkspace );

   if( eNO_ERROR != error )
      {
      MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );
      g_Error = eNO_ERROR;
		DestructWorkspace( s_MainWindow.activeWorkspace );
		return;
      }

   _write_settings();





   if( NS_MODEL_SPINES_SHOW_RESULTS )
      {
      nsulong flags = 0;
      _get_model_spines_write_flags( &flags );

		ns_sprint( output_file, "%s" NS_STRING_PATH "%s.txt", output_dir, file_name );
		ns_println( "SPINES FILE = %s", output_file );

      if( NS_FAILURE(
            ns_model_spines_write(
               GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
               output_file,
               flags,
					workspace_settings( s_MainWindow.activeWorkspace ),
					NULL,
					NULL,
					NULL
               ),
            error ) )
			{
         MessageBox(NULL, "There was an error saving stats to file. Try again.", "NeuronStudio", MB_OK | MB_ICONERROR );
			DestructWorkspace( s_MainWindow.activeWorkspace );
			return;
			}
      }




	DestructWorkspace( s_MainWindow.activeWorkspace );
	_getche();
   }
