#ifndef __NS_MODEL_SETTINGS_H__
#define __NS_MODEL_SETTINGS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsconfigdb.h>
#include <math/nsvector.h>
#include <image/nsvoxel-info.h>
#include <model/nsmodel-order.h>
#include <model/nssettings-decl.h>

NS_DECLS_BEGIN


typedef enum
	{
	NS_LABELING_INTRACELLULAR,
	NS_LABELING_MEMBRANE,

	_NS_LABELING_NUM_TYPES /* Internal. DO NOT USE! */
	}
	NsLabelingType;

#define NS_LABELING_NUM_TYPES  ( ( nssize )_NS_LABELING_NUM_TYPES )

NS_IMPEXP const nschar* ns_labeling_type_to_string( NsLabelingType type );


typedef enum
	{
	NS_RESIDUAL_SMEAR_NONE,
	NS_RESIDUAL_SMEAR_LOW,
	NS_RESIDUAL_SMEAR_MEDIUM,
	NS_RESIDUAL_SMEAR_HIGH,

	_NS_RESIDUAL_SMEAR_NUM_TYPES /* Internal. DO NOT USE! */
	}
	NsResidualSmearType;

#define NS_RESIDUAL_SMEAR_NUM_TYPES  ( ( nssize )_NS_RESIDUAL_SMEAR_NUM_TYPES )

NS_IMPEXP const nschar* ns_residual_smear_type_to_string( NsResidualSmearType type );

NS_IMPEXP nsfloat ns_residual_smear_type_to_value( NsResidualSmearType type );


struct _NsSettings
	{
	struct
		{
		nsfloat     attachment_ratio;
		nsfloat     min_length;
		nsint       labeling_type; /* TEMP: deprecated */
		nsboolean   do_3d_radii; /* TEMP: deprecated */
		nsfloat     origin_radius_scalar; /* TEMP: deprecated */
		nsfloat     vertex_radius_scalar;
		nssize      smooth_count; /* TEMP: deprecated */
		nsboolean   use_random_sampling;
		nsboolean   realign_junctions;
		NsVector3i  seed;
		}
		neurite;

	struct
		{
		nsfloat    max_voxel_distance;
		nsfloat    max_layer_spread;
		nsfloat    elim_height;
		nsfloat    min_width; /* TEMP: deprecated */
		nsboolean  enhanced_detection; /* TEMP: deprecated */
		nsfloat    min_volume; /* TEMP: deprecated */
		nsfloat    max_volume; /* TEMP: deprecated */
		nssize     min_attached_voxels;
		nssize     min_detached_voxels;
		nsfloat    z_spread_divisor;
		nsboolean  z_spread_automated;
		}
		spine;

	struct
		{
		nsboolean  use_fixed;
		nsfloat    fixed_value;
		nsfloat    multiplier;
		nsboolean  auto_correct; /* TEMP: deprecated */
		}
		threshold;

	struct
		{
		nsdouble  neck_ratio;
		nsdouble  thin_ratio;
		nsdouble  mushroom_size;
		/*nsdouble  invalid_ratio;*/
		}
		classify;

	struct
		{
		NsLabelingType       labeling_type;
		NsResidualSmearType  residual_smear_type;
		}
		volume;

	NsVoxelInfo  voxel_info;
	};


#define NS_NEURITE_ATTACHMENT_RATIO_DEFAULT      1.3f
#define NS_NEURITE_MIN_LENGTH_DEFAULT            3.0f/*30.0f*/
#define NS_NEURITE_VERTEX_RADIUS_SCALAR_DEFAULT  1.0f
#define NS_NEURITE_USE_RANDOM_SAMPLING_DEFAULT   NS_TRUE
#define NS_NEURITE_REALIGN_JUNCTIONS_DEFAULT     NS_TRUE
#define NS_NEURITE_SEED_X_DEFAULT                0
#define NS_NEURITE_SEED_Y_DEFAULT                0
#define NS_NEURITE_SEED_Z_DEFAULT                0

#define NS_SPINE_MAX_VOXEL_DISTANCE_DEFAULT   3.0f/*30.0f*/
#define NS_SPINE_MAX_LAYER_SPREAD_DEFAULT     3.0f/*25.0f*/
#define NS_SPINE_ELIM_HEIGHT_DEFAULT          0.2f/*5.0f*/
#define NS_SPINE_MIN_ATTACHED_VOXELS_DEFAULT  10
#define NS_SPINE_MIN_DETACHED_VOXELS_DEFAULT  5
#define NS_SPINE_Z_SPREAD_DIVISOR_DEFAULT     1.0f
#define NS_SPINE_Z_SPREAD_AUTOMATED_DEFAULT   NS_TRUE

#define NS_VOXEL_SIZE_X_DEFAULT  0.1f/*1.0f*/
#define NS_VOXEL_SIZE_Y_DEFAULT  0.1f/*1.0f*/
#define NS_VOXEL_SIZE_Z_DEFAULT  0.1f/*1.0f*/

#define NS_THRESHOLD_USE_FIXED_DEFAULT     NS_FALSE
#define NS_THRESHOLD_FIXED_VALUE_DEFAULT   128.0f
#define NS_THRESHOLD_MULTIPLIER_DEFAULT    1.0f

#define NS_CLASSIFY_NECK_RATIO_DEFAULT     1.1
#define NS_CLASSIFY_THIN_RATIO_DEFAULT     2.5
#define NS_CLASSIFY_MUSHROOM_SIZE_DEFAULT  0.35


NS_IMPEXP NsSettings* ns_settings_init_with_user_defaults( NsSettings *settings );
NS_IMPEXP NsSettings* ns_settings_init_with_program_defaults( NsSettings *settings );


#define ns_settings_get_neurite_attachment_ratio( settings )\
	( (settings)->neurite.attachment_ratio )
#define ns_settings_set_neurite_attachment_ratio( settings, v )\
	( (settings)->neurite.attachment_ratio = (v) )

#define ns_settings_get_neurite_min_length( settings )\
	( (settings)->neurite.min_length )
#define ns_settings_set_neurite_min_length( settings, v )\
	( (settings)->neurite.min_length = (v) )

#define ns_settings_get_neurite_labeling_type( settings )\
	( (settings)->neurite.labeling_type )
#define ns_settings_set_neurite_labeling_type( settings, v )\
	( (settings)->neurite.labeling_type = (v) )

#define ns_settings_get_neurite_do_3d_radii( settings )\
	( (settings)->neurite.do_3d_radii )
#define ns_settings_set_neurite_do_3d_radii( settings, v )\
	( (settings)->neurite.do_3d_radii = (v) )

#define ns_settings_get_neurite_origin_radius_scalar( settings )\
	( (settings)->neurite.origin_radius_scalar )
#define ns_settings_set_neurite_origin_radius_scalar( settings, v )\
	( (settings)->neurite.origin_radius_scalar = (v) )

#define ns_settings_get_neurite_vertex_radius_scalar( settings )\
	( (settings)->neurite.vertex_radius_scalar )
#define ns_settings_set_neurite_vertex_radius_scalar( settings, v )\
	( (settings)->neurite.vertex_radius_scalar = (v) )

#define ns_settings_get_neurite_smooth_count( settings )\
	( (settings)->neurite.smooth_count )
#define ns_settings_set_neurite_smooth_count( settings, v )\
	( (settings)->neurite.smooth_count = (v) )

#define ns_settings_get_neurite_use_random_sampling( settings )\
	( (settings)->neurite.use_random_sampling )
#define ns_settings_set_neurite_use_random_sampling( settings, v )\
	( (settings)->neurite.use_random_sampling = (v) )

#define ns_settings_get_neurite_realign_junctions( settings )\
	( (settings)->neurite.realign_junctions )
#define ns_settings_set_neurite_realign_junctions( settings, v )\
	( (settings)->neurite.realign_junctions = (v) )


#define ns_settings_neurite_seed( settings )\
	( &(settings)->neurite.seed )

#define ns_settings_neurite_seed_x( settings )\
	( (settings)->neurite.seed.x )
#define ns_settings_neurite_seed_y( settings )\
	( (settings)->neurite.seed.y )
#define ns_settings_neurite_seed_z( settings )\
	( (settings)->neurite.seed.z )


#define ns_settings_get_spine_max_voxel_distance( settings )\
	( (settings)->spine.max_voxel_distance )
#define ns_settings_set_spine_max_voxel_distance( settings, v )\
	( (settings)->spine.max_voxel_distance = (v) )

#define ns_settings_get_spine_max_layer_spread( settings )\
	( (settings)->spine.max_layer_spread )
#define ns_settings_set_spine_max_layer_spread( settings, v )\
	( (settings)->spine.max_layer_spread = (v) )

#define ns_settings_get_spine_elim_height( settings )\
	( (settings)->spine.elim_height )
#define ns_settings_set_spine_elim_height( settings, v )\
	( (settings)->spine.elim_height = (v) )

#define ns_settings_get_spine_min_width( settings )\
	( (settings)->spine.min_width )
#define ns_settings_set_spine_min_width( settings, v )\
	( (settings)->spine.min_width = (v) )

#define ns_settings_get_spine_enhanced_detection( settings )\
	( (settings)->spine.enhanced_detection )
#define ns_settings_set_spine_enhanced_detection( settings, v )\
	( (settings)->spine.enhanced_detection = (v) )

#define ns_settings_get_spine_min_volume( settings )\
	( (settings)->spine.min_volume )
#define ns_settings_set_spine_min_volume( settings, v )\
	( (settings)->spine.min_volume = (v) )

#define ns_settings_get_spine_max_volume( settings )\
	( (settings)->spine.max_volume )
#define ns_settings_set_spine_max_volume( settings, v )\
	( (settings)->spine.max_volume = (v) )

#define ns_settings_get_spine_min_attached_voxels( settings )\
	( (settings)->spine.min_attached_voxels )
#define ns_settings_set_spine_min_attached_voxels( settings, v )\
	( (settings)->spine.min_attached_voxels = (v) )

#define ns_settings_get_spine_min_detached_voxels( settings )\
	( (settings)->spine.min_detached_voxels )
#define ns_settings_set_spine_min_detached_voxels( settings, v )\
	( (settings)->spine.min_detached_voxels = (v) )

#define ns_settings_get_spine_z_spread_divisor( settings )\
	( (settings)->spine.z_spread_divisor )
#define ns_settings_set_spine_z_spread_divisor( settings, v )\
	( (settings)->spine.z_spread_divisor = (v) )

#define ns_settings_get_spine_z_spread_automated( settings )\
	( (settings)->spine.z_spread_automated )
#define ns_settings_set_spine_z_spread_automated( settings, v )\
	( (settings)->spine.z_spread_automated = (v) )


#define ns_settings_get_volume_labeling_type( settings )\
	( (settings)->volume.labeling_type )
#define ns_settings_set_volume_labeling_type( settings, v )\
	( (settings)->volume.labeling_type = (v) )

#define ns_settings_get_volume_residual_smear_type( settings )\
	( (settings)->volume.residual_smear_type )
#define ns_settings_set_volume_residual_smear_type( settings, v )\
	( (settings)->volume.residual_smear_type = (v) )


#define ns_settings_voxel_info( settings )\
	( &( (settings)->voxel_info ) )


#define ns_settings_get_threshold_use_fixed( settings )\
	( (settings)->threshold.use_fixed )
#define ns_settings_set_threshold_use_fixed( settings, v )\
	( (settings)->threshold.use_fixed = (v) )

#define ns_settings_get_threshold_fixed_value( settings )\
	( (settings)->threshold.fixed_value )
#define ns_settings_set_threshold_fixed_value( settings, v )\
	( (settings)->threshold.fixed_value = (v) )

#define ns_settings_get_threshold_multiplier( settings )\
	( (settings)->threshold.multiplier )
#define ns_settings_set_threshold_multiplier( settings, v )\
	( (settings)->threshold.multiplier = (v) )

#define ns_settings_get_threshold_auto_correct( settings )\
	( (settings)->threshold.auto_correct )
#define ns_settings_set_threshold_auto_correct( settings, v )\
	( (settings)->threshold.auto_correct = (v) )


#define ns_settings_get_classify_neck_ratio( settings )\
	( (settings)->classify.neck_ratio )
#define ns_settings_set_classify_neck_ratio( settings, v )\
	( (settings)->classify.neck_ratio = (v) )

#define ns_settings_get_classify_thin_ratio( settings )\
	( (settings)->classify.thin_ratio )
#define ns_settings_set_classify_thin_ratio( settings, v )\
	( (settings)->classify.thin_ratio = (v) )

#define ns_settings_get_classify_mushroom_size( settings )\
	( (settings)->classify.mushroom_size )
#define ns_settings_set_classify_mushroom_size( settings, v )\
	( (settings)->classify.mushroom_size = (v) )


/* 'did_read' is set to false if the settings tagged
	with the 'group_name' are not in the file. */
NS_IMPEXP NsError ns_settings_read
	(
	const nschar  *file_name,
	const nschar  *group_name,
	NsSettings    *settings,
	nsboolean     *did_read
	);

NS_IMPEXP NsError ns_settings_write
	(
	const nschar      *file_name,
	const nschar      *group_name,
	const NsSettings  *settings
	);


/* Writes only a subset of the settings, specifically neurite
	'attachment_ratio', 'min_length', and 'realign_junctions'.  */
NS_IMPEXP NsError ns_settings_write_neurite_filtering
	(
	const nschar      *file_name,
	const nschar      *group_name,
	const NsSettings  *settings
	);


/* This routine will set program defaults for any key names that
	are missing. Note that if the 'group_name' doesnt exist than
	the values are set to all the program defaults. */
NS_IMPEXP NsError ns_settings_read_user_defaults( NsConfigDb *db, const nschar *group_name );

NS_IMPEXP void ns_settings_update_user_defaults( const NsSettings *settings );

NS_DECLS_END

#endif/* __NS_MODEL_SETTINGS_H__ */
