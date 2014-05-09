#ifndef __NS_MODEL_MODEL_SPINES_H__
#define __NS_MODEL_MODEL_SPINES_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nslist.h>
#include <std/nsset.h>
#include <std/nsmath.h>
#include <std/nsvalue.h>
#include <std/nsbytearray.h>
#include <std/nsprogress.h>
#include <std/nsfile.h>
#include <std/nsconfigdb.h>
#include <std/nsio.h>
#include <std/nsascii.h>
#include <std/nsprofiler.h>
#include <std/nsqueue.h>
#include <math/nsvector.h>
#include <math/nspoint.h>
#include <math/nsline.h>
#include <math/nssphere.h>
#include <math/nsfrustum.h>
#include <math/nsplane.h>
#include <math/nsrandom.h>
#include <math/nsaabbox.h>
#include <math/nsprojector.h>
#include <image/nspixels.h>
#include <image/nsimage.h>
#include <image/nssampler.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nsvoxel-table.h>
#include <image/nsimagepath.h>
#include <image/nscolor.h>
#include <image/tiff.h>
#include <model/nssettings.h>
#include <model/nsmodel.h>
#include <model/nsrayburst.h>
#include <model/nsmodel-section.h>
#include <model/nsmodel-graft.h>
#include <model/nsmodel-mask.h>
#include <model/nsspines-classifier.h>
#include <render/nsrender.h>
#include <ext/normal.h>
#include <ext/mc.h>

NS_DECLS_BEGIN


typedef nslistiter nsspine;

#define NS_SPINE_NIL  NULL


/* If set to true then the user is allowed to see the various
   spine statistics, such as count, classification, etc...
   Otherwise only visual inspection is allowed of the results. */
#define NS_MODEL_SPINES_SHOW_RESULTS\
   NS_TRUE


/* The model is sectioned by this setting as the spines are built.
   This should also be used when saving a model file (if the file
   supports it e.g. HOC) so that the section (i.e. location) of the
   spines can be determined. */
#define NS_MODEL_SPINES_SECTIONING\
   NS_MODEL_SECTION_DEPTH_FIRST


typedef enum{
	NS_SPINE_RENDER_MODE_OFF,
	NS_SPINE_RENDER_MODE_SOLID_FIXED_ELLIPSE,
	NS_SPINE_RENDER_MODE_HOLLOW_FIXED_ELLIPSE,
	NS_SPINE_RENDER_MODE_SOLID_HEAD_DIAM_ELLIPSE,
	NS_SPINE_RENDER_MODE_HOLLOW_HEAD_DIAM_ELLIPSE,
	NS_SPINE_RENDER_MODE_SOLID_SURFACE,
	NS_SPINE_RENDER_MODE_HOLLOW_SURFACE
	}
	NsSpineRenderMode;


typedef enum
   {
   NS_SPINE_INVALID,
   NS_SPINE_STUBBY,
   NS_SPINE_THIN,
   NS_SPINE_MUSHROOM,
   NS_SPINE_OTHER,

   _NS_SPINE_NUM_TYPES
   }
   NsSpineType;

#define NS_SPINE_NUM_TYPES  ( ( nssize ) _NS_SPINE_NUM_TYPES )


/* Input Parameters:

	- Head (rayburst) diameter
	- Neck (rayburst) diameter (if no neck output ?)
	- Has neck
	- Height
	- Base depth
	- Base (rayburst) diameter
	- Angle with respect to plane

	* Possibly volume and surface area later
*/
enum{
	NS_SPINES_CLASSIFIER_HEAD_DIAMETER,
	NS_SPINES_CLASSIFIER_NECK_DIAMETER,
	NS_SPINES_CLASSIFIER_HAS_NECK,
	NS_SPINES_CLASSIFIER_HEIGHT,
	NS_SPINES_CLASSIFIER_BASE_DEPTH,
	NS_SPINES_CLASSIFIER_BASE_WIDTH,
	NS_SPINES_CLASSIFIER_XYPLANE_ANGLE,

	_NS_SPINES_CLASSIFIER_NUM_INPUT
	};

#define NS_SPINES_CLASSIFIER_NUM_INPUT  ( ( nsuint )_NS_SPINES_CLASSIFIER_NUM_INPUT )


/* IMPORTANT: NS_SPINE_INVALID must have a value of 0 for NsSpinesClassifier to
	work seamlessly with the default classifier! */
NS_COMPILE_TIME_BOOLEAN( NS_SPINE_INVALID, ==, 0 );


NS_IMPEXP const nschar* ns_spine_type_to_string( NsSpineType type );


NS_IMPEXP nsboolean ns_model_spines_any_selected_are_invalid( const NsModel *model );

NS_IMPEXP NsError ns_model_spines_add_selected_to_classifier( const NsModel *model, NsSpinesClassifier *sc );


NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_spine_analysis
   (
   NsModel               *model,
   const NsSettings      *settings,
   const NsImage         *image,
	const NsCubei         *roi,
   NsProcDb              *pixel_proc_db,
   NsRayburstInterpType   interp_type,
   nsboolean              use_2d_bbox,
   nsdouble               average_intensity,
   NsRayburstKernelType   kernel_type,
   NsProgress            *progress,
   void                   ( *render_func )( void ),
	nsboolean              keep_voxels,
   const nschar          *config_file,
	NsResidualSmearType    residual_smear_type
   );


enum{
	NS_SPINE_XYPLANE_ANGLE_UNKNOWN,
	NS_SPINE_XYPLANE_ANGLE_PARALLEL,
	NS_SPINE_XYPLANE_ANGLE_OBLIQUE
	};

NS_IMPEXP nsenum ns_spine_xyplane_angle_type( nsdouble angle );


NS_IMPEXP void ns_model_spine_calc_xyplane_angle( nsspine S );
NS_IMPEXP void ns_model_spines_calc_xyplane_angles( NsModel *model );

NS_IMPEXP nsboolean ns_model_spines_has_unset_xyplane_angles( const NsModel *model );

NS_IMPEXP NsError ns_model_spines_calc_xyplane_angles_if_unset
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	);


NS_IMPEXP NsError ns_model_spines_on_add_manual
	(
	NsModel               *model,
	const NsSettings      *settings,
	const NsImage         *image,
	nsfloat                threshold,
   NsRayburstInterpType   interp_type,
   NsRayburstKernelType   kernel_type,
	nsspine                S,
	const NsVector3f      *attach_finder,
	nsboolean             *do_remove
	);


NS_IMPEXP void ns_model_change_selected_spines_radii
	(
	NsModel  *model,
	nsfloat   percent,
	nsfloat   min_radius
	);


NS_IMPEXP nssize ns_model_num_manual_spines( const NsModel *model );
NS_IMPEXP nssize ns_model_num_auto_spines( const NsModel *model );

NS_IMPEXP nssize ns_model_num_selected_spines( const NsModel *model );

/* Same as above except that it stores the last selected one in 'S'. Useful for example
	if you want to find out if only one spine is selected, and retrieve that spine at
	the same time, so that the list is only traversed once. */
NS_IMPEXP nssize ns_model_num_selected_spines_ex( const NsModel *model, nsspine *S );


NS_IMPEXP NsError ns_model_spines_set_swc_attachments
	(
	NsModel           *model,
	const NsSettings  *settings,
	nsboolean          set_soma_distances,
	NsProgress        *progress
	);


NS_IMPEXP NsError ns_model_recompute_spines_attachment
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	);


NS_IMPEXP NsError ns_model_recompute_manual_spines
	(
	NsModel               *model,
	const NsSettings      *settings,
	const NsImage         *image,
	nsdouble               average_intensity,
	NsSampler             *sampler,
	const NsVector3i      *jitter,
	nsboolean              do_3d_rayburst,
	nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   NsRayburstInterpType   interp_type,
   NsRayburstKernelType   kernel_type,
	NsProgress            *progress
	);


NS_IMPEXP NsError ns_model_new_spines( NsModel *model );
NS_IMPEXP void ns_model_delete_spines( NsModel *model );

NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear_spines
	(
	NsModel            *model,
	const NsVoxelInfo  *voxel_info,
	const NsCubei      *roi
	);

NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear_spines_but_not_voxels( NsModel *model );


NS_IMPEXP nsboolean ns_model_spines_is_running( const NsModel *model );


NS_IMPEXP const NsVoxelTable* ns_model_spines_voxel_table( const NsModel *model );


NS_IMPEXP nsboolean ns_model_spine_voxel_is_maxima( const NsVoxel *voxel );
NS_IMPEXP nsfloat ns_model_spine_voxel_dts( const NsVoxel *voxel );


NS_IMPEXP nsboolean ns_model_spines_all_layers_color
   (
   NsModel           *model, 
   const NsVector3i  *V,
   nsshort           *color
   );

NS_IMPEXP nsboolean ns_model_spines_render_layer_color
   (
   NsModel           *model, 
   const NsVector3i  *V,
   nsshort           *color
   );


NS_IMPEXP void ns_model_select_spines( NsModel *model, nsboolean selected );

/* All spines of the passed 'type' are selected. All other spines
	become deselected. */
NS_IMPEXP void ns_model_select_spines_by_type( NsModel *model, NsSpineType type );

NS_IMPEXP NsError ns_model_select_spine_bases( NsModel *model, const nschar *config_file );

/* Spines whose bounding box intertesct this bounding box, selection state is toggled. */
NS_IMPEXP NsError ns_model_select_spines_by_aabbox( NsModel *model, const NsAABBox3d *B );

NS_IMPEXP NsError ns_model_select_spines_by_projection_onto_drag_rect
	(
	NsModel            *model,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj,
	NsSpineRenderMode   mode
	);


NS_IMPEXP void ns_model_select_spines_with_invalid_surface_area( NsModel *model );


NS_IMPEXP void ns_model_mark_selected_spines_hidden( NsModel *model, nsboolean b );
NS_IMPEXP void ns_model_mark_spines_hidden( NsModel *model, nsboolean b );


NS_IMPEXP NsError ns_model_select_spines_on_selected_parts
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	);


NS_IMPEXP NS_SYNCHRONIZED void ns_model_delete_selected_spines( NsModel *model );

/* Returns the last manually added spine, else NS_SPINE_NIL. */
NS_IMPEXP nsspine ns_model_find_last_manual_spine( const NsModel *model );

NS_IMPEXP void ns_model_type_selected_spines( NsModel *model, NsSpineType type );


/* Stores all seleted spines in the list. The finalize callback on the
   list should be NULL. */
NS_IMPEXP NsError ns_model_buffer_selected_spines_ex
   (
   const NsModel  *model,
   NsList         *list
   );


/* Values for ns_model_spines_write() 'flags' parameter. */
#define NS_MODEL_SPINES_WRITE_ID               0x00000001
#define NS_MODEL_SPINES_WRITE_SECTION_NUMBER   0x00000002
#define NS_MODEL_SPINES_WRITE_SECTION_LENGTH   0x00000004
#define NS_MODEL_SPINES_WRITE_BRANCH_ORDER     0x00000008
#define NS_MODEL_SPINES_WRITE_RAYBURST_VOLUME  0x00000010
#define NS_MODEL_SPINES_WRITE_VOXEL_VOLUME     0x00000020
#define NS_MODEL_SPINES_WRITE_LAYERS_VOLUME    0x00000040
#define NS_MODEL_SPINES_WRITE_NON_ATTACH_SA    0x00000080
#define NS_MODEL_SPINES_WRITE_ATTACH_SA        0x00000100
#define NS_MODEL_SPINES_WRITE_LAYERS_SA        0x00000200
#define NS_MODEL_SPINES_WRITE_X                0x00000400
#define NS_MODEL_SPINES_WRITE_Y                0x00000800
#define NS_MODEL_SPINES_WRITE_Z                0x00001000
#define NS_MODEL_SPINES_WRITE_MIN_DTS          0x00002000
#define NS_MODEL_SPINES_WRITE_MAX_DTS          0x00004000
#define NS_MODEL_SPINES_WRITE_TYPE             0x00008000
#define NS_MODEL_SPINES_WRITE_AUTO             0x00010000
#define NS_MODEL_SPINES_WRITE_HEAD_DIAMETER    0x00020000
#define NS_MODEL_SPINES_WRITE_NECK_DIAMETER    0x00040000
#define NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE    0x00080000
#define NS_MODEL_SPINES_WRITE_SWC_NODE_ID      0x00100000
#define NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET  0x00200000
#define NS_MODEL_SPINES_WRITE_ATTACH_X         0x00400000
#define NS_MODEL_SPINES_WRITE_ATTACH_Y         0x00800000
#define NS_MODEL_SPINES_WRITE_ATTACH_Z         0x01000000
#define NS_MODEL_SPINES_WRITE_SOMA_DISTANCE    0x02000000
#define NS_MODEL_SPINES_WRITE_HEAD_CENTER_X    0x04000000
#define NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y    0x08000000
#define NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z    0x10000000
#define NS_MODEL_SPINES_WRITE_NECK_CENTER_X    0x20000000
#define NS_MODEL_SPINES_WRITE_NECK_CENTER_Y    0x40000000
#define NS_MODEL_SPINES_WRITE_NECK_CENTER_Z    0x80000000

#define NS_MODEL_SPINES_WRITE_ALL              0xffffffff

#define NS_MODEL_SPINES_FILE_MAX_COLUMNS  32


/* 'line_num' can be NULL. If not, it is passed the line
   number where an error occurs, or 0 if no error. */
NS_IMPEXP NsError ns_model_spines_read
   (
   NsModel       *model,
   const nschar  *name,
   nslong        *line_num
   );

/* Holds spine type names that were read from the file but not matched to
	the current classifier, either the default or an ANN classifier. */
NS_IMPEXP const nschar* ns_model_spines_read_unmatched( void );


NS_IMPEXP NsError ns_model_spines_write_txt
   (
   const NsModel     *model,
   const nschar      *name,
   nsulong            flags,
	const NsSettings  *settings,
	nsboolean          ( *warning )( const nschar *message, nspointer user_data ),
	nspointer          user_data,
	NsProgress        *progress
   );


NS_IMPEXP NsError ns_model_spines_write_csv
   (
   const NsModel     *model,
   const nschar      *name,
   nsulong            flags,
	const NsSettings  *settings,
	nsboolean          ( *warning )( const nschar *message, nspointer user_data ),
	nspointer          user_data,
	NsProgress        *progress
   );


typedef struct _NsSpinesInfo
   {
   nssize  num_spines;
   nssize  num_mushroom_spines;
   nssize  num_thin_spines;
   nssize  num_stubby_spines;
   nssize  num_other_spines;
   }
   NsSpinesInfo;

NS_IMPEXP void ns_spines_info_init( NsSpinesInfo *info );
NS_IMPEXP void ns_model_spines_info( const NsModel *model, NsSpinesInfo *info );


typedef struct _NsSpinesClassifierInfo
	{
	nssize   num_spines;
	nssize  *counts;
	nssize   num_counts;
	}
	NsSpinesClassifierInfo;

NS_IMPEXP void ns_spines_classifier_info_init( NsSpinesClassifierInfo *info );
NS_IMPEXP NsError ns_model_spines_classifier_info( const NsModel *model, const NsSpinesClassifier *sc, NsSpinesClassifierInfo *info );
NS_IMPEXP void ns_spines_classifier_info_finalize( NsSpinesClassifierInfo *info );


NS_IMPEXP void ns_model_spines_retype_by_default_classifier
	(
	NsModel           *model,
	const NsSettings  *settings
	);

NS_IMPEXP NsError ns_model_spines_retype_by_ann_classifier
	(
	NsModel             *model,
	NsSpinesClassifier  *classifier,
	NsProgress          *progress
	);

NS_IMPEXP NsError ns_model_spines_retype_unselected_by_ann_classifier
	(
	NsModel             *model,
	NsSpinesClassifier  *classifier,
	NsProgress          *progress
	);


NS_IMPEXP NsError ns_model_spines_do_retype_comparison
	(
	NsModel             *model,
	const nschar        *file_name,
	const NsSettings    *settings,
	NsSpinesClassifier  *classifier
	);


NS_IMPEXP nsspine ns_model_begin_spines( const NsModel *model );
NS_IMPEXP nsspine ns_model_end_spines( const NsModel *model );

NS_IMPEXP nssize ns_model_num_spines( const NsModel *model );


NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_add_spine( NsModel *model, nsspine *S );
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_add_spine_no_id( NsModel *model, nsspine *S );

NS_IMPEXP NS_SYNCHRONIZED void ns_model_remove_spine( NsModel *model, nsspine S );


#define ns_spine_not_equal( spine1, spine2 )\
   ns_list_iter_not_equal( (spine1), (spine2) )

#define ns_spine_next( spine )\
   ns_list_iter_next( ( nslistiter )(spine) )

#define ns_spine_prev( spine )\
   ns_list_iter_prev( ( nslistiter )(spine) )


#define NS_SPINE_DEFAULT_HEAD_DIAMETER  -1.0
#define NS_SPINE_DEFAULT_NECK_DIAMETER  -1.0


/* Anything outside the [-90, 90] range will do. That
	indicates that the angle is unknown or N/A. */
#define NS_SPINE_DEFAULT_XYPLANE_ANGLE  1000.0


NS_IMPEXP nssize ns_spine_get_id( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_x( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_y( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_z( const nsspine S );
NS_IMPEXP NsVector3f* ns_spine_get_position( const nsspine S, NsVector3f *P );
NS_IMPEXP nsdouble ns_spine_get_voxel_volume( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_attach_surface_area( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_non_attach_surface_area( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_min_dts( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_max_dts( const nsspine S );
NS_IMPEXP NsSpineType ns_spine_get_type( const nsspine S );
NS_IMPEXP nsboolean ns_spine_get_auto_detected( const nsspine S );
NS_IMPEXP nsulong ns_spine_get_section_number( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_section_length( const nsspine S );
NS_IMPEXP nsulong ns_spine_get_branch_order( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_rayburst_volume( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_layers_volume( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_layers_sa( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_head_diameter( const nsspine S );
NS_IMPEXP nsdouble ns_spine_get_neck_diameter( const nsspine S );
NS_IMPEXP void ns_spine_get_bounding_box( const nsspine S, NsAABBox3d *B );
NS_IMPEXP nsdouble ns_spine_get_xyplane_angle( const nsspine S );
NS_IMPEXP void ns_spine_get_attach_point( const nsspine S, NsVector3f *P );
NS_IMPEXP nslong ns_spine_get_swc_node_id( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_swc_node_offset( const nsspine S );
NS_IMPEXP nsfloat ns_spine_get_soma_distance( const nsspine S );
NS_IMPEXP void ns_spine_get_head_center( const nsspine S, NsVector3f *C );
NS_IMPEXP void ns_spine_get_neck_center( const nsspine S, NsVector3f *C );


NS_IMPEXP nsboolean ns_spine_has_head_center( const nsspine S );
NS_IMPEXP nsboolean ns_spine_has_neck_center( const nsspine S );


NS_IMPEXP void ns_spine_set_id( nsspine S, nssize id );
NS_IMPEXP void ns_spine_set_x( nsspine S, nsfloat x );
NS_IMPEXP void ns_spine_set_y( nsspine S, nsfloat y );
NS_IMPEXP void ns_spine_set_z( nsspine S, nsfloat z );
NS_IMPEXP void ns_spine_set_position( nsspine S, const NsVector3f *P );
NS_IMPEXP void ns_spine_set_voxel_volume( nsspine S, nsdouble voxel_volume );
NS_IMPEXP void ns_spine_set_attach_surface_area( nsspine S, nsdouble attach_sa );
NS_IMPEXP void ns_spine_set_non_attach_surface_area( nsspine S, nsdouble non_attach_sa );
NS_IMPEXP void ns_spine_set_min_dts( nsspine S, nsfloat min_dts );
NS_IMPEXP void ns_spine_set_max_dts( nsspine S, nsfloat max_dts );
NS_IMPEXP void ns_spine_set_type( nsspine S, NsSpineType type );
NS_IMPEXP void ns_spine_set_auto_detected( nsspine S, nsboolean auto_detected );
NS_IMPEXP void ns_spine_set_section_number( nsspine S, nsulong section_number );
NS_IMPEXP void ns_spine_set_section_length( nsspine S, nsfloat section_length );
NS_IMPEXP void ns_spine_set_branch_order( nsspine S, nsulong branch_order );
NS_IMPEXP void ns_spine_set_rayburst_volume( nsspine S, nsdouble rayburst_volume );
NS_IMPEXP void ns_spine_set_layers_volume( nsspine S, nsdouble layers_volume );
NS_IMPEXP void ns_spine_set_layers_sa( nsspine S, nsdouble layers_sa );
NS_IMPEXP void ns_spine_set_head_diameter( nsspine S, nsdouble head_diameter );
NS_IMPEXP void ns_spine_set_neck_diameter( nsspine S, nsdouble neck_diameter );
NS_IMPEXP void ns_spine_set_bounding_box( nsspine S, const NsAABBox3d *B );
NS_IMPEXP void ns_spine_set_xyplane_angle( nsspine S, nsdouble xyplane_angle );
NS_IMPEXP void ns_spine_set_attach_point( nsspine S, const NsVector3f *P );
NS_IMPEXP void ns_spine_set_swc_node_id( nsspine S, nslong swc_node_id );
NS_IMPEXP void ns_spine_set_swc_node_offset( nsspine S, nsfloat swc_node_offset );
NS_IMPEXP void ns_spine_set_soma_distance( nsspine S, nsfloat soma_distance );
NS_IMPEXP void ns_spine_set_head_center( nsspine S, const NsVector3f *C );
NS_IMPEXP void ns_spine_set_neck_center( nsspine S, const NsVector3f *C );


NS_IMPEXP nsfloat ns_spine_get_sholl_distance( const nsspine S );
NS_IMPEXP void ns_spine_set_sholl_distance( nsspine S, nsfloat sholl_distance );


NS_IMPEXP void ns_spine_mark_selected( nsspine S, nsboolean selected );
NS_IMPEXP nsboolean ns_spine_is_selected( const nsspine S );


NS_IMPEXP NsAABBox3d* ns_spine_to_aabbox_ex( const nsspine S, NsAABBox3d *B );

NS_IMPEXP NsSphered* ns_spine_to_sphere_ex( const nsspine S, NsSphered *E );
NS_IMPEXP NsSphered* ns_spine_to_sphere_by_render_mode_ex( const nsspine S, NsSpineRenderMode mode, NsSphered *E );


NS_IMPEXP NsError ns_model_spines_write_ann_data
	(
	const NsModel  *model,
	const nschar   *training_file_name,
	const nschar   *running_file_name
	);


NS_IMPEXP NsError ns_model_spines_print_clump_mergers( NsModel *model, nsint x, nsint y, nsint z );


NS_IMPEXP void ns_model_render_spines_surface
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_volumetric
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_origins
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_voxels
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_rectangles
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_ball_and_stick
	(
   const NsModel  *model,
   NsRenderState  *state
	);

NS_IMPEXP void ns_model_render_spines_layers_as_points
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_layers_as_rectangles
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_candidates_as_points
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_candidates_as_rectangles
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_dts
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_vectors
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_selected
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_samples
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_merging
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_spines_candidates_gradients
	(
	const NsModel  *model,
	NsRenderState  *state
	);


#define NS_MODEL_SPINES_FIXED_RADIUS  0.15f

NS_IMPEXP nsfloat ns_model_spine_render_radius( nsspine S, NsSpineRenderMode mode );
NS_IMPEXP nsdouble ns_model_spine_render_radius_ex( nsspine S, NsSpineRenderMode mode );


NS_IMPEXP void ns_model_render_spines_fixed_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   );


/* TEMP */
NS_IMPEXP void ns_model_render_spine_head_and_neck_centers( const NsModel *model, NsRenderState *state );


NS_IMPEXP void ns_model_render_spines_fixed_selected
   (
   const NsModel  *model,
   NsRenderState  *state
   );


NS_IMPEXP void ns_model_render_spines_ellipses_hd
   (
   const NsModel  *model,
   NsRenderState  *state
   );


NS_IMPEXP void ns_model_render_spines_axis
   (
   const NsModel  *model,
   NsRenderState  *state
   );


NS_IMPEXP void ns_model_render_spines_clumps_voxels
   (
   const NsModel  *model,
   NsRenderState  *state
   );


NS_IMPEXP void ns_model_render_spines_interface_voxels
   (
   const NsModel  *model,
   NsRenderState  *state
   );


/* TEMP!!!!!!! */
NS_IMPEXP void ns_model_render_spines_centroids( const NsModel *model, NsRenderState *state );
NS_IMPEXP void ns_model_render_spines_borderers( const NsModel *model, NsRenderState *state );


/* If this global flag is false, then ns_spines_create_random_colors() will
	do nothing. */
NS_IMPEXP void ns_spines_enable_random_color_creation( nsboolean yes_or_no );

NS_IMPEXP void ns_spines_create_random_colors( nssize count );

NS_IMPEXP NsColor4ub ns_spines_random_color( nssize index );

NS_IMPEXP const NsColor4ub* ns_spines_random_colors( void );

NS_IMPEXP nssize ns_spines_num_random_colors( void );


#define NS_SPINES_FILE_OFFSET  1000000


/* Puts this ID (and type) on hold. On other words the next created spine will get
	this ID and type. */
NS_IMPEXP void ns_model_spines_hold_id_and_type( const NsModel *model, nsspine spine );

/* Increments (if 'increment is true) or decrements a single selected spines ID to
	the next or previous available ID. Note that this function will just return if
	no spines are selected or if more than one is selected. */
NS_IMPEXP void ns_model_spines_offset_selected_spine_id( NsModel *model, nsboolean increment );


/*
Maintaining Spine ID's

spines->file is a global integer that acts as an offset.
0         = means all automatically and manually added spines
1,000,000 = all spines read from file number 1
2,000,000 = all spines read from file number 2
…

spines->id is an always increasing global integer.

1) Auto-detect spines is run.
	- Delete all existing spines in the R.O.I. (DONE)
	- If the number of spines after deletion is 0, reset spines->id and spines->file to 0 (DONE)
	- Set each detected spine's id to spines->id++ (DONE)

2) Add a spine manually
	- See _ns_spines_set_id(). (DONE)

3) Delete 1 or more spines.
	- If the number of spines after deletion is 0, reset spines->id and spines->file to 0. (DONE)

4) Delete all spines.
	- Reset spines->id and spines->file to 0. (DONE)

5) Import a spine file.
	- If the current number of spines is greater than 0, spines->file += 1,000,000 (DONE)
	- Set each read spine's id to the id in the file + spines->file. (DONE)
	- If the initial number of spines was 0, spines->id = the max. of the id's in the file + 1. (DONE)
*/

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_SPINES_H__ */


