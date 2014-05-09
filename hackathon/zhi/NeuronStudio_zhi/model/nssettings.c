#include "nssettings.h"


const nschar* ns_labeling_type_to_string( NsLabelingType type )
	{
	NS_PRIVATE const nschar* ____ns_labeling_type_strings[ NS_LABELING_NUM_TYPES ] =
		{
		"Intracellular",
		"Membrane"
		};

	ns_assert( 0 <= type && ( nssize )type < NS_LABELING_NUM_TYPES );
	return ____ns_labeling_type_strings[ type ];
	}


const nschar* ns_residual_smear_type_to_string( NsResidualSmearType type )
	{
	NS_PRIVATE const nschar* ____ns_residual_smear_type_strings[ NS_RESIDUAL_SMEAR_NUM_TYPES ] =
		{
		"None",
		"Low",
		"Medium",
		"High",
		};

	ns_assert( 0 <= type && ( nssize )type < NS_RESIDUAL_SMEAR_NUM_TYPES );
	return ____ns_residual_smear_type_strings[ type ];
	}


nsfloat ns_residual_smear_type_to_value( NsResidualSmearType type )
	{
	NS_PRIVATE nsfloat ____ns_residual_smear_type_values[ NS_RESIDUAL_SMEAR_NUM_TYPES ] =
		{
		1.0f,
		1.2f,
		1.5f,
		1.9f
		};

	ns_assert( 0 <= type && ( nssize )type < NS_RESIDUAL_SMEAR_NUM_TYPES );
	return ____ns_residual_smear_type_values[ type ];
	}


NS_PRIVATE NsSettings ____ns_settings_user_defaults;


NsSettings* ns_settings_init_with_user_defaults( NsSettings *settings )
	{
	ns_assert( NULL != settings );

	*settings = ____ns_settings_user_defaults;
	return settings;
	}


NsSettings* ns_settings_init_with_program_defaults( NsSettings *settings )
   {
   ns_assert( NULL != settings );

	/* Deprecated */
   ns_settings_set_neurite_origin_radius_scalar( settings, 1.0f );
   ns_settings_set_neurite_labeling_type( settings, NS_MODEL_ORDER_NONE );
   ns_settings_set_neurite_smooth_count( settings, 1 );
   ns_settings_set_neurite_do_3d_radii( settings, NS_FALSE );
   ns_settings_set_spine_min_width( settings, 0.0f );
   ns_settings_set_spine_enhanced_detection( settings, NS_FALSE );
	ns_settings_set_spine_min_volume( settings, 0.0f );
	ns_settings_set_spine_max_volume( settings, 0.0f );
	ns_settings_set_threshold_auto_correct( settings, NS_FALSE );

   ns_settings_set_neurite_attachment_ratio(
      settings,
      NS_NEURITE_ATTACHMENT_RATIO_DEFAULT
      );

   ns_settings_set_neurite_min_length(
      settings,
      NS_NEURITE_MIN_LENGTH_DEFAULT
      );

	ns_settings_set_neurite_vertex_radius_scalar(
		settings,
		NS_NEURITE_VERTEX_RADIUS_SCALAR_DEFAULT
		);

	ns_settings_set_neurite_use_random_sampling(
		settings,
		NS_NEURITE_USE_RANDOM_SAMPLING_DEFAULT
		);

	ns_settings_set_neurite_realign_junctions(
		settings,
		NS_NEURITE_REALIGN_JUNCTIONS_DEFAULT
		);

   ns_vector3i(
      ns_settings_neurite_seed( settings ),
      NS_NEURITE_SEED_X_DEFAULT,
      NS_NEURITE_SEED_Y_DEFAULT,
      NS_NEURITE_SEED_Z_DEFAULT
      );

   ns_settings_set_spine_max_voxel_distance(
      settings,
      NS_SPINE_MAX_VOXEL_DISTANCE_DEFAULT
      );

   ns_settings_set_spine_max_layer_spread(
      settings,
      NS_SPINE_MAX_LAYER_SPREAD_DEFAULT
      );

   ns_settings_set_spine_elim_height(
      settings,
      NS_SPINE_ELIM_HEIGHT_DEFAULT
      );

   ns_voxel_info(
      ns_settings_voxel_info( settings ),
      NS_VOXEL_SIZE_X_DEFAULT,
      NS_VOXEL_SIZE_Y_DEFAULT,
      NS_VOXEL_SIZE_Z_DEFAULT
      );

   ns_settings_set_spine_min_attached_voxels(
      settings,
      NS_SPINE_MIN_ATTACHED_VOXELS_DEFAULT
      );

   ns_settings_set_spine_min_detached_voxels(
      settings,
      NS_SPINE_MIN_DETACHED_VOXELS_DEFAULT
      );

	ns_settings_set_spine_z_spread_divisor(
		settings,
		NS_SPINE_Z_SPREAD_DIVISOR_DEFAULT
		);

	ns_settings_set_spine_z_spread_automated(
		settings,
		NS_SPINE_Z_SPREAD_AUTOMATED_DEFAULT
		);

   ns_settings_set_threshold_use_fixed(
      settings,
      NS_THRESHOLD_USE_FIXED_DEFAULT
      );

   ns_settings_set_threshold_fixed_value(
      settings,
      NS_THRESHOLD_FIXED_VALUE_DEFAULT
      );

   ns_settings_set_threshold_multiplier(
      settings,
      NS_THRESHOLD_MULTIPLIER_DEFAULT
      );

	ns_settings_set_classify_neck_ratio(
		settings,
		NS_CLASSIFY_NECK_RATIO_DEFAULT
		);

	ns_settings_set_classify_thin_ratio(
		settings,
		NS_CLASSIFY_THIN_RATIO_DEFAULT
		);

	ns_settings_set_classify_mushroom_size(
		settings,
		NS_CLASSIFY_MUSHROOM_SIZE_DEFAULT
		);

	ns_settings_set_volume_labeling_type(
		settings,
		NS_LABELING_INTRACELLULAR
		);

	ns_settings_set_volume_residual_smear_type(
		settings,
		NS_RESIDUAL_SMEAR_NONE
		);

	return settings;
   }


enum
   {
   _NEURITE_ATTACHMENT_RATIO,
   _NEURITE_MIN_LENGTH,
	_NEURITE_VERTEX_RADIUS_SCALAR,
	_NEURITE_USE_RANDOM_SAMPLING,
	_NEURITE_REALIGN_JUNCTIONS,
   _NEURITE_SEED_X,
   _NEURITE_SEED_Y,
   _NEURITE_SEED_Z,
   _SPINE_MAX_VOXEL_DISTANCE,
   _SPINE_MAX_LAYER_SPREAD,
   _SPINE_ELIM_HEIGHT,
   _SPINE_MIN_ATTACHED_VOXELS,
   _SPINE_MIN_DETACHED_VOXELS,
	_SPINE_Z_SPREAD_DIVISOR,
	_SPINE_Z_SPREAD_AUTOMATED,
   _VOXEL_SIZE_X,
   _VOXEL_SIZE_Y,
   _VOXEL_SIZE_Z,
   _THRESHOLD_USE_FIXED,
   _THRESHOLD_FIXED_VALUE,
   _THRESHOLD_MULTIPLIER,
	_CLASSIFY_NECK_RATIO,
	_CLASSIFY_THIN_RATIO,
	_CLASSIFY_MUSHROOM_SIZE,
	_VOLUME_LABELING_TYPE,
	_VOLUME_RESIDUAL_SMEAR_TYPE,

   __NUM_KEYS
   };


NS_PRIVATE NsConfigDbKey _ns_settings_config_db_keys[] =
   {
   { /*"neurite_attachment_ratio"*/     "nar",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"neurite_min_length"*/           "nml",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"neurite_vertex_radius_scalar"*/ "nvrs", NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"neurite_use_random_sampling"*/  "nrs",  NS_CONFIG_DB_VALUE_BOOLEAN },
	{ /*"neurite_realign_junctions"*/    "nrj",  NS_CONFIG_DB_VALUE_BOOLEAN },
   { /*"neurite_seed_x"*/               "nsx",  NS_CONFIG_DB_VALUE_INT     },
   { /*"neurite_seed_y"*/               "nsy",  NS_CONFIG_DB_VALUE_INT     },
   { /*"neurite_seed_z"*/               "nsz",  NS_CONFIG_DB_VALUE_INT     },
   { /*"spine_max_voxel_distance"*/     "smvd", NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"spine_max_layer_spread"*/       "smls", NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"spine_elim_height"*/            "seh",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"spine_min_attached_voxels"*/    "smav", NS_CONFIG_DB_VALUE_INT     },
   { /*"spine_min_detached_voxels"*/    "smdv", NS_CONFIG_DB_VALUE_INT     },
	{ /*"spine_z_spread_divisor"*/       "szsd", NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"spine_z_spread_automated"*/     "szsa", NS_CONFIG_DB_VALUE_BOOLEAN },
   { /*"voxel_size_x"*/                 "vsx",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"voxel_size_y"*/                 "vsy",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"voxel_size_z"*/                 "vsz",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"threshold_use_fixed"*/          "tuf",  NS_CONFIG_DB_VALUE_BOOLEAN },
   { /*"threshold_fixed_value"*/        "tfv",  NS_CONFIG_DB_VALUE_DOUBLE  },
   { /*"threshold_multiplier"*/         "tm",   NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"classify_neck_ratio"*/          "cnr",  NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"classify_thin_ratio"*/          "ctr",  NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"classify_mushroom_size"*/       "cms",  NS_CONFIG_DB_VALUE_DOUBLE  },
	{ /*"volume_labeling_type"*/         "vlt",  NS_CONFIG_DB_VALUE_INT     },
	{ /*"volume_residual_smear_type"*/   "vrst", NS_CONFIG_DB_VALUE_INT     }
   };


NS_COMPILE_TIME_BOOLEAN( __NUM_KEYS, ==, NS_ARRAY_LENGTH( _ns_settings_config_db_keys ) );


NS_PRIVATE NsConfigDbGroup _ns_settings_config_db_group =
   {
   NULL,
   NS_ARRAY_LENGTH( _ns_settings_config_db_keys ),
   _ns_settings_config_db_keys
   };


NS_PRIVATE NsError _ns_settings_validate_keys
   (
   NsConfigDb    *db,
   const nschar  *group,
	nsboolean      user_defaults
   )
   {
	NsSettings       settings;
   nssize           i;
   nsconfigdbiter   curr_group;
   nsconfigdbiter   curr_key;
   nsconfigdbiter   end_keys;
   const nschar    *key;
   nsboolean        remove;
   NsError          error;


	if( user_defaults )
		settings = ____ns_settings_user_defaults;
	else
		ns_settings_init_with_program_defaults( &settings );

   ns_verify( ns_config_db_has_group( db, group ) );

   /* First remove any keys that are not recognized. */

   curr_group = ns_config_db_lookup_group( db, group );
   curr_key   = ns_config_db_begin_keys( curr_group );
   end_keys   = ns_config_db_end_keys( curr_group );

   for( ; ns_config_db_iter_not_equal( curr_key, end_keys );
          curr_key = ns_config_db_iter_next( curr_key ) )
      {
      key    = ns_config_db_iter_string( curr_key );
      remove = NS_TRUE;

      for( i = 0; i < NS_ARRAY_LENGTH( _ns_settings_config_db_keys ); ++i )
         if( ns_ascii_streq( key, _ns_settings_config_db_keys[i].name ) )
            {
            remove = NS_FALSE;
            break;
            }

      if( remove )
         ns_config_db_remove_key( db, group, key );
      }

   /* Now add any keys that are missing. */

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_ATTACHMENT_RATIO ].name,
                     settings.neurite.attachment_ratio
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_MIN_LENGTH ].name,
                     settings.neurite.min_length
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_VERTEX_RADIUS_SCALAR ].name,
                     settings.neurite.vertex_radius_scalar
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_boolean_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_USE_RANDOM_SAMPLING ].name,
                     settings.neurite.use_random_sampling
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_boolean_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_REALIGN_JUNCTIONS ].name,
                     settings.neurite.realign_junctions
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_SEED_X ].name,
                     settings.neurite.seed.x
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_SEED_Y ].name,
                     settings.neurite.seed.y
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _NEURITE_SEED_Z ].name,
                     settings.neurite.seed.z
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_MAX_VOXEL_DISTANCE ].name,
                     settings.spine.max_voxel_distance
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_MAX_LAYER_SPREAD ].name,
                     settings.spine.max_layer_spread
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_ELIM_HEIGHT ].name,
                     settings.spine.elim_height
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_MIN_ATTACHED_VOXELS ].name,
                     ( nsint )settings.spine.min_attached_voxels
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_MIN_DETACHED_VOXELS ].name,
                     ( nsint )settings.spine.min_detached_voxels
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_Z_SPREAD_DIVISOR ].name,
                     settings.spine.z_spread_divisor
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_boolean_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _SPINE_Z_SPREAD_AUTOMATED ].name,
                     settings.spine.z_spread_automated
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _VOXEL_SIZE_X ].name,
                     ns_voxel_info_size_x( &settings.voxel_info )
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _VOXEL_SIZE_Y ].name,
                     ns_voxel_info_size_y( &settings.voxel_info )
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _VOXEL_SIZE_Z ].name,
                     ns_voxel_info_size_z( &settings.voxel_info )
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_boolean_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _THRESHOLD_USE_FIXED ].name,
                     settings.threshold.use_fixed
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _THRESHOLD_FIXED_VALUE ].name,
                     settings.threshold.fixed_value
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _THRESHOLD_MULTIPLIER ].name,
                     settings.threshold.multiplier
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _CLASSIFY_NECK_RATIO ].name,
                     settings.classify.neck_ratio
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _CLASSIFY_THIN_RATIO ].name,
                     settings.classify.thin_ratio
                     ),
                     error ) )
      return error;


   if( NS_FAILURE( ns_config_db_add_double_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _CLASSIFY_MUSHROOM_SIZE ].name,
                     settings.classify.mushroom_size
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _VOLUME_LABELING_TYPE ].name,
                     settings.volume.labeling_type
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_config_db_add_int_if_not_exist(
                     db,
                     group,
                     _ns_settings_config_db_keys[ _VOLUME_RESIDUAL_SMEAR_TYPE ].name,
                     settings.volume.residual_smear_type
                     ),
                     error ) )
      return error;

   return ns_no_error();
   }


NS_PRIVATE void _ns_settings_extract_keys( NsSettings *settings, const NsConfigDb *db, const nschar *group_name )
	{
   NsConfigDbGroup group;

   group      = _ns_settings_config_db_group;
   group.name = group_name;

	ns_config_db_get_all( db, &group );

	ns_settings_set_neurite_attachment_ratio(
		settings,
		( nsfloat )group.keys[ _NEURITE_ATTACHMENT_RATIO ].value.as_double
		);

	ns_settings_set_neurite_min_length(
		settings,
		( nsfloat )group.keys[ _NEURITE_MIN_LENGTH ].value.as_double
		);

	ns_settings_set_neurite_vertex_radius_scalar(
		settings,
		( nsfloat )group.keys[ _NEURITE_VERTEX_RADIUS_SCALAR ].value.as_double
		);

	ns_settings_set_neurite_use_random_sampling(
		settings,
		group.keys[ _NEURITE_USE_RANDOM_SAMPLING ].value.as_boolean
		);

	ns_settings_set_neurite_realign_junctions(
		settings,
		group.keys[ _NEURITE_REALIGN_JUNCTIONS ].value.as_boolean
		);

	ns_vector3i(
		ns_settings_neurite_seed( settings ),
		group.keys[ _NEURITE_SEED_X ].value.as_int,
		group.keys[ _NEURITE_SEED_Y ].value.as_int,
		group.keys[ _NEURITE_SEED_Z ].value.as_int
		);

	ns_settings_set_spine_max_voxel_distance(
		settings,
		( nsfloat )group.keys[ _SPINE_MAX_VOXEL_DISTANCE ].value.as_double
		);

	ns_settings_set_spine_max_layer_spread(
		settings,
		( nsfloat )group.keys[ _SPINE_MAX_LAYER_SPREAD ].value.as_double
		);

	ns_settings_set_spine_elim_height(
		settings,
		( nsfloat )group.keys[ _SPINE_ELIM_HEIGHT ].value.as_double
		);

	ns_voxel_info(
		ns_settings_voxel_info( settings ),
		( nsfloat )group.keys[ _VOXEL_SIZE_X ].value.as_double,
		( nsfloat )group.keys[ _VOXEL_SIZE_Y ].value.as_double,
		( nsfloat )group.keys[ _VOXEL_SIZE_Z ].value.as_double
		);

	ns_settings_set_spine_min_attached_voxels(
		settings,
		( nssize )group.keys[ _SPINE_MIN_ATTACHED_VOXELS ].value.as_int
		);

	ns_settings_set_spine_min_detached_voxels(
		settings,
		( nssize )group.keys[ _SPINE_MIN_DETACHED_VOXELS ].value.as_int
		);

	ns_settings_set_spine_z_spread_divisor(
		settings,
		( nsfloat )group.keys[ _SPINE_Z_SPREAD_DIVISOR ].value.as_double
		);

	ns_settings_set_spine_z_spread_automated(
		settings,
		group.keys[ _SPINE_Z_SPREAD_AUTOMATED ].value.as_boolean
		);

	ns_settings_set_threshold_use_fixed(
		settings,
		group.keys[ _THRESHOLD_USE_FIXED ].value.as_boolean
		);

	ns_settings_set_threshold_fixed_value(
		settings,
		( nsfloat )group.keys[ _THRESHOLD_FIXED_VALUE ].value.as_double
		);

	ns_settings_set_threshold_multiplier(
		settings,
		( nsfloat )group.keys[ _THRESHOLD_MULTIPLIER ].value.as_double
		);

	ns_settings_set_classify_neck_ratio(
		settings,
		group.keys[ _CLASSIFY_NECK_RATIO ].value.as_double
		);

	ns_settings_set_classify_thin_ratio(
		settings,
		group.keys[ _CLASSIFY_THIN_RATIO ].value.as_double
		);

	ns_settings_set_classify_mushroom_size(
		settings,
		group.keys[ _CLASSIFY_MUSHROOM_SIZE ].value.as_double
		);

	ns_settings_set_volume_labeling_type(
		settings,
		group.keys[ _VOLUME_LABELING_TYPE ].value.as_int
		);

	ns_settings_set_volume_residual_smear_type(
		settings,
		group.keys[ _VOLUME_RESIDUAL_SMEAR_TYPE ].value.as_int
		);
	}


NsError ns_settings_read
   (
   const nschar  *file_name,
   const nschar  *group_name,
   NsSettings    *settings,
   nsboolean     *did_read
   )
   {
   NsConfigDb  db;
   nssize      num_lines;
   NsError     error;


   ns_assert( NULL != file_name );
   ns_assert( NULL != group_name );
   ns_assert( NULL != settings );
   ns_assert( NULL != did_read );

   *did_read = NS_FALSE;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

   if( NS_FAILURE( ns_config_db_read( &db, file_name, &num_lines ), error ) )
      {
      ns_config_db_destruct( &db );
      return error;
      }

   if( ns_config_db_has_group( &db, group_name ) )
      {
      if( NS_FAILURE( _ns_settings_validate_keys( &db, group_name, NS_TRUE ), error ) )
         {
         ns_config_db_destruct( &db );
         return error;
         }

		_ns_settings_extract_keys( settings, &db, group_name );

      *did_read = NS_TRUE;
      }

   ns_config_db_destruct( &db );
   return ns_no_error();
   }


NsError ns_settings_write
   (
   const nschar      *file_name,
   const nschar      *group_name,
   const NsSettings  *settings
   )
   {
   NsConfigDb       db;
   NsConfigDbGroup  group;
   nssize           num_lines;
   NsError          error;


   ns_assert( NULL != file_name );
   ns_assert( NULL != group_name );
   ns_assert( NULL != settings );

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

   if( NS_FAILURE( ns_config_db_read( &db, file_name, &num_lines ), error ) )
      {
      ns_config_db_destruct( &db );
      return error;
      }

   group      = _ns_settings_config_db_group;
   group.name = group_name;

   if( ! ns_config_db_has_group( &db, group_name ) )
      {
      if( NS_FAILURE( ns_config_db_add_all( &db, &group ), error ) )
         {
         ns_config_db_destruct( &db );
         return error;
         }
      }
   else
      {
      if( NS_FAILURE( _ns_settings_validate_keys( &db, group_name, NS_TRUE ), error ) )
         {
         ns_config_db_destruct( &db );
         return error;
         }
      }

   group.keys[ _NEURITE_ATTACHMENT_RATIO ].value.as_double =
      ( nsdouble )ns_settings_get_neurite_attachment_ratio( settings );

   group.keys[ _NEURITE_MIN_LENGTH ].value.as_double =
      ( nsdouble )ns_settings_get_neurite_min_length( settings );

   group.keys[ _NEURITE_VERTEX_RADIUS_SCALAR ].value.as_double =
      ( nsdouble )ns_settings_get_neurite_vertex_radius_scalar( settings );

   group.keys[ _NEURITE_USE_RANDOM_SAMPLING ].value.as_boolean =
      ns_settings_get_neurite_use_random_sampling( settings );

   group.keys[ _NEURITE_REALIGN_JUNCTIONS ].value.as_boolean =
      ns_settings_get_neurite_realign_junctions( settings );

   group.keys[ _NEURITE_SEED_X ].value.as_int = 
      ( nsint )ns_settings_neurite_seed( settings )->x;

   group.keys[ _NEURITE_SEED_Y ].value.as_int = 
      ( nsint )ns_settings_neurite_seed( settings )->y;

   group.keys[ _NEURITE_SEED_Z ].value.as_int = 
      ( nsint )ns_settings_neurite_seed( settings )->z;

   group.keys[ _SPINE_MAX_VOXEL_DISTANCE ].value.as_double =
      ( nsdouble )ns_settings_get_spine_max_voxel_distance( settings );

   group.keys[ _SPINE_MAX_LAYER_SPREAD ].value.as_double =
      ( nsdouble )ns_settings_get_spine_max_layer_spread( settings );

   group.keys[ _SPINE_ELIM_HEIGHT ].value.as_double =
      ( nsdouble )ns_settings_get_spine_elim_height( settings );

   group.keys[ _SPINE_MIN_ATTACHED_VOXELS ].value.as_int =
      ( nsint )ns_settings_get_spine_min_attached_voxels( settings );

   group.keys[ _SPINE_MIN_DETACHED_VOXELS ].value.as_int =
      ( nsint )ns_settings_get_spine_min_detached_voxels( settings );

   group.keys[ _SPINE_Z_SPREAD_DIVISOR ].value.as_double =
      ( nsdouble )ns_settings_get_spine_z_spread_divisor( settings );

   group.keys[ _SPINE_Z_SPREAD_AUTOMATED ].value.as_boolean =
      ns_settings_get_spine_z_spread_automated( settings );

   group.keys[ _VOXEL_SIZE_X ].value.as_double =
      ( nsdouble )ns_voxel_info_size_x( ns_settings_voxel_info( settings ) );

   group.keys[ _VOXEL_SIZE_Y ].value.as_double =
      ( nsdouble )ns_voxel_info_size_y( ns_settings_voxel_info( settings ) );

   group.keys[ _VOXEL_SIZE_Z ].value.as_double =
      ( nsdouble )ns_voxel_info_size_z( ns_settings_voxel_info( settings ) );

   group.keys[ _THRESHOLD_USE_FIXED ].value.as_boolean =
      ns_settings_get_threshold_use_fixed( settings );

   group.keys[ _THRESHOLD_FIXED_VALUE ].value.as_double =
      ns_settings_get_threshold_fixed_value( settings );

   group.keys[ _THRESHOLD_MULTIPLIER ].value.as_double =
      ns_settings_get_threshold_multiplier( settings );

   group.keys[ _CLASSIFY_NECK_RATIO ].value.as_double =
      ns_settings_get_classify_neck_ratio( settings );

   group.keys[ _CLASSIFY_THIN_RATIO ].value.as_double =
      ns_settings_get_classify_thin_ratio( settings );

   group.keys[ _CLASSIFY_MUSHROOM_SIZE ].value.as_double =
      ns_settings_get_classify_mushroom_size( settings );

   group.keys[ _VOLUME_LABELING_TYPE ].value.as_int =
      ns_settings_get_volume_labeling_type( settings );

   group.keys[ _VOLUME_RESIDUAL_SMEAR_TYPE ].value.as_int =
      ns_settings_get_volume_residual_smear_type( settings );

   ns_config_db_set_all( &db, &group );

   error = ns_config_db_write( &db, file_name );
   ns_config_db_destruct( &db );

   return error;
   }


NsError ns_settings_write_neurite_filtering
	(
	const nschar      *file_name,
	const nschar      *group_name,
	const NsSettings  *settings
	)
	{
   NsConfigDb       db;
   NsConfigDbGroup  group;
   nssize           num_lines;
   NsError          error;


   ns_assert( NULL != file_name );
   ns_assert( NULL != group_name );
   ns_assert( NULL != settings );

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

   if( NS_FAILURE( ns_config_db_read( &db, file_name, &num_lines ), error ) )
      {
      ns_config_db_destruct( &db );
      return error;
      }

   group      = _ns_settings_config_db_group;
   group.name = group_name;

   if( ! ns_config_db_has_group( &db, group_name ) )
      {
      if( NS_FAILURE( ns_config_db_add_all( &db, &group ), error ) )
         {
         ns_config_db_destruct( &db );
         return error;
         }
      }
   else
      {
      if( NS_FAILURE( _ns_settings_validate_keys( &db, group_name, NS_TRUE ), error ) )
         {
         ns_config_db_destruct( &db );
         return error;
         }
      }

   ns_config_db_set_double(
		&db,
		group_name,
		group.keys[ _NEURITE_ATTACHMENT_RATIO ].name,
      ( nsdouble )ns_settings_get_neurite_attachment_ratio( settings )
		);

   ns_config_db_set_double(
		&db,
		group_name,
		group.keys[ _NEURITE_MIN_LENGTH ].name,
      ( nsdouble )ns_settings_get_neurite_min_length( settings )
		);

   ns_config_db_set_double(
		&db,
		group_name,
		group.keys[ _NEURITE_VERTEX_RADIUS_SCALAR ].name,
      ( nsdouble )ns_settings_get_neurite_vertex_radius_scalar( settings )
		);

   ns_config_db_set_boolean(
		&db,
		group_name,
		group.keys[ _NEURITE_REALIGN_JUNCTIONS ].name,
      ns_settings_get_neurite_realign_junctions( settings )
		);

   error = ns_config_db_write( &db, file_name );
   ns_config_db_destruct( &db );

   return error;
	}


NsError ns_settings_read_user_defaults( NsConfigDb *db, const nschar *group_name )
	{
   NsError error;

   ns_assert( NULL != db );
   ns_assert( NULL != group_name );

	ns_settings_init_with_program_defaults( &____ns_settings_user_defaults );

   if( ns_config_db_has_group( db, group_name ) )
      {
      if( NS_FAILURE( _ns_settings_validate_keys( db, group_name, NS_FALSE ), error ) )
         return error;

		_ns_settings_extract_keys( &____ns_settings_user_defaults, db, group_name );
      }

	return ns_no_error();
	}


void ns_settings_update_user_defaults( const NsSettings *settings )
	{
	ns_assert( NULL != settings );
	____ns_settings_user_defaults = *settings;
	}
