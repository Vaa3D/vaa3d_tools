#ifndef __APP_DATA_H__
#define __APP_DATA_H__
/*============================================================================
   Module  : app_data.h
   Author  : Doug Ehlenberger © 2002
   Purpose : Define the main data structure for holding information in this
             application.
   Date    : Last modified on December 19, 2002.
=============================================================================*/
#include "app_lib.h"
#include "progress.h"
#include <image/nscolordb.h>

#define mNUM_DISPLAY_WINDOWS   1 /* 4 */
#define mNUM_DISPLAYS          4


typedef nsuint HWorkspace;


void DestroyAllWorkspaces( void );


nsint WorkspaceIsValid( const HWorkspace handle );

void DestructWorkspace( const HWorkspace handle );


typedef struct tagWorkspaceArgs
   {
   nschar*  path;
   HWND   window;
	NsImageDir *dir;
	nsboolean use_path;
   }
   WorkspaceArgs;


/* Main function to create the workspace. This function runs as a
   separate thread and updates a progress bar. */

void BuildWorkspace( void *vpDialog );


void RedrawWorkspaceDisplayGraphics( const HWorkspace handle );
void workspace_set_graphics_xy( HWorkspace handle );


void GetWorkspaceSeed( const HWorkspace handle, Vector3i *seed );

void SetWorkspaceSeed( const HWorkspace handle, const Vector3i *seed );


MemoryGraphics* GetWorkspaceDisplayGraphics( const HWorkspace handle, const nsint display );

const NsImage* workspace_dataset( HWorkspace handle );
const NsImage* GetWorkspaceDisplayImage( const HWorkspace handle, const nsint display );
const NsImage* workspace_slice_image( const HWorkspace handle );


void workspace_select_spines_on_selected_parts( HWND hWnd, HWorkspace handle );
void workspace_set_swc_attachments( HWND hWnd, HWorkspace handle, nsboolean set_soma_distances );


void workspace_clear_grafter_voxels( HWorkspace handle );


const NsTileMgr* workspace_tile_mgr( HWorkspace handle, nsint display );

const NsShollAnalysis* workspace_sholl_analysis( HWorkspace handle );


const Palette* GetWorkspacePalette( const HWorkspace handle );
const Palette* workspace_get_inverted_palette( HWorkspace handle );

nsint workspace_get_palette_type( HWorkspace handle );
void workspace_set_palette_type( HWorkspace handle, nsint palette_type );


nsushort workspace_get_roi_control_selections( nsuint handle );
void workspace_set_roi_control_selections( nsuint handle, nsushort selections );


nsboolean workspace_is_lsm_image( const HWorkspace handle );
const TiffLsmInfo* workspace_get_lsm_info( const HWorkspace handle );
void workspace_set_lsm_info( HWorkspace handle, const TiffLsmInfo *lsm_info );

                                                            
const nschar* GetWorkspacePath( const HWorkspace handle );


float GetWorkspaceZoom( const HWorkspace handle, const nsuint window, const nsint display );

void SetWorkspaceZoom( const HWorkspace handle, const nsuint window, const nsint display,
                       const float zoom );


Vector2i GetWorkspaceCorner( const HWorkspace handle, const nsuint window, const nsint display );
                                
void SetWorkspaceCorner( const HWorkspace handle, const nsuint window, const nsint display,
                         const Vector2i corner );


enum
	{
	WORKSPACE_COLOR_BACKGROUND,
	WORKSPACE_COLOR_FUNCTION_SOMA,
	WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE,
	WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE,
	WORKSPACE_COLOR_FUNCTION_AXON,
	WORKSPACE_COLOR_LINE_VERTEX,
	WORKSPACE_COLOR_JUNCTION_VERTEX,
	WORKSPACE_COLOR_EXTERNAL_VERTEX,
	WORKSPACE_COLOR_ORIGIN_VERTEX,
	WORKSPACE_COLOR_VERTEX,
	WORKSPACE_COLOR_EDGE,
	WORKSPACE_COLOR_SPLAT,
	WORKSPACE_COLOR_SPINE_STUBBY,
	WORKSPACE_COLOR_SPINE_THIN,
	WORKSPACE_COLOR_SPINE_MUSHROOM,
	WORKSPACE_COLOR_SPINE_OTHER,
	WORKSPACE_COLOR_SPINE,

	_WORKSPACE_NUM_COLORS
	};


extern NsVector4ub _dc_background;
extern NsVector4ub _dc_line_vertex;
extern NsVector4ub _dc_junction_vertex;
extern NsVector4ub _dc_origin_vertex;
extern NsVector4ub _dc_external_vertex;
extern NsVector4ub _dc_vertex;
extern NsVector4ub _dc_edge;
extern NsVector4ub _dc_splat;
extern NsVector4ub _dc_spine_other;
extern NsVector4ub _dc_spine_thin;
extern NsVector4ub _dc_spine_mushroom;
extern NsVector4ub _dc_spine_stubby;
extern NsVector4ub _dc_spine;
extern NsVector4ub _dc_function_soma;
extern NsVector4ub _dc_function_basal_dendrite;
extern NsVector4ub _dc_function_apical_dendrite;
extern NsVector4ub _dc_function_axon;


#define WORKSPACE_DEFAULT_COLOR_BACKGROUND  _dc_background
#define WORKSPACE_DEFAULT_COLOR_FUNCTION_SOMA  _dc_function_soma
#define WORKSPACE_DEFAULT_COLOR_FUNCTION_BASAL_DENDRITE  _dc_function_basal_dendrite
#define WORKSPACE_DEFAULT_COLOR_FUNCTION_APICAL_DENDRITE  _dc_function_apical_dendrite
#define WORKSPACE_DEFAULT_COLOR_FUNCTION_AXON _dc_function_axon
#define WORKSPACE_DEFAULT_COLOR_LINE_VERTEX  _dc_line_vertex
#define WORKSPACE_DEFAULT_COLOR_JUNCTION_VERTEX  _dc_junction_vertex
#define WORKSPACE_DEFAULT_COLOR_ORIGIN_VERTEX  _dc_origin_vertex
#define WORKSPACE_DEFAULT_COLOR_EXTERNAL_VERTEX  _dc_external_vertex
#define WORKSPACE_DEFAULT_COLOR_VERTEX _dc_vertex
#define WORKSPACE_DEFAULT_COLOR_EDGE  _dc_edge
#define WORKSPACE_DEFAULT_COLOR_SPLAT  _dc_splat
#define WORKSPACE_DEFAULT_COLOR_SPINE_OTHER     _dc_spine_other
#define WORKSPACE_DEFAULT_COLOR_SPINE_THIN      _dc_spine_thin
#define WORKSPACE_DEFAULT_COLOR_SPINE_MUSHROOM  _dc_spine_mushroom
#define WORKSPACE_DEFAULT_COLOR_SPINE_STUBBY    _dc_spine_stubby
#define WORKSPACE_DEFAULT_COLOR_SPINE           _dc_spine


NsVector4ub workspace_get_color( HWorkspace handle, nsint which );
void workspace_set_color( HWorkspace handle, nsint which, NsVector4ub color );

NsVector4ub workspace_get_vertex_color_by_type( HWorkspace handle, NsModelVertexType type );


NsError workspace_model_filtering( HWorkspace handle, const NsSettings *settings );


const NsImage* workspace_thumbnail( HWorkspace handle, int display );

NsList* workspace_image_extra_info( HWorkspace handle );


NsCubei* workspace_update_roi( HWorkspace hwsp );

/* Returns the R.O.I. used the last time the projections were run. */
NsCubei* workspace_visual_roi( HWorkspace hwsp );
nssize workspace_visual_roi_length( HWorkspace hwsp );


void workspace_recompute_manual_spines( HWND hWnd, HWorkspace hwsp );
void workspace_compute_z_spread( HWND hWnd, HWorkspace hwsp );


void workspace_recompute_spines_attachment( HWND hWnd, HWorkspace hwsp );


void workspace_run_sholl_analysis( HWND hWnd, HWorkspace hwsp );


eERROR_TYPE workspace_accept_roi( HWorkspace hwsp );


const NsImage* workspace_volume( HWorkspace hwsp );


typedef struct _NsRoiInfo
	{
	nssize    x1, y1, z1;
	nssize    x2, y2, z2;
	nssize    width, height, length;
	nsdouble  percent_x;
	nsdouble  percent_y;
	nsdouble  percent_z;
	nsdouble  total_percent;
	}
	NsRoiInfo;

void workspace_roi_info( HWorkspace hwsp, NsRoiInfo *info );


NsProcDb* workspace_pixel_proc_db( HWorkspace hwsp );
NsProcDb* workspace_io_proc_db( HWorkspace hwsp );


eERROR_TYPE workspace_write_image
	(
	HWorkspace     handle,
	HWND           wnd,
	const nschar  *prefix,
	const nschar  *extension,
	nsboolean      pad_number,
	nsboolean      as_series,
	nsint          bit_depth
	);

NsColorDb* workspace_brush_db( HWorkspace handle );
NsColorDb* workspace_pen_db( HWorkspace handle );


NsError workspace_build_slice_view( HWorkspace handle, nssize z );
NsError workspace_init_slice_view( HWorkspace handle );
void workspace_finalize_slice_view( HWorkspace handle );


NsVector4ub workspace_get_spine_color_single( nsuint handle );
NsVector4ub workspace_get_spine_color_by_type( nsuint handle, NsSpineType type );


const NsVoxelInfo* workspace_get_voxel_info( HWorkspace handle );
void workspace_set_voxel_info( HWorkspace handle, float x, float y, float z );


void workspace_establish_thresholds_and_contrasts( HWND wnd, HWorkspace handle, NsModel *model );


NS_IMPEXP NsSettings* workspace_settings( HWorkspace handle );
//NS_IMPEXP nsulong* workspace_setting_flags( HWorkspace handle );

NS_IMPEXP nsdouble workspace_get_average_intensity( nsuint handle );
NS_IMPEXP void workspace_set_average_intensity( nsuint handle, nsdouble average_intensity );


NS_IMPEXP nsfloat workspace_get_grafting_first_threshold( nsuint handle );
NS_IMPEXP void workspace_set_grafting_first_threshold( nsuint handle, nsfloat threshold );

NS_IMPEXP nsfloat workspace_get_grafting_first_contrast( nsuint handle );
NS_IMPEXP void workspace_set_grafting_first_contrast( nsuint handle, nsfloat contrast );


//NS_IMPEXP nsulong workspace_get_max_intensity( nsuint handle );


void workspace_get_image_dimensions( nsuint handle, NsVector3f *V );


NsSampler* workspace_sampler( nsuint handle );

const NsList* workspace_records( nsuint handle );


nsuchar workspace_get_pixel( nsuint handle, nsint display, nssize x, nssize y );

NsPixelType workspace_pixel_type( nsuint handle );

void workspace_clear_model( nsuint handle );


//extern void* g_HackWorkspaceVolume;
//extern nssize g_hack_workspace_volume_width;
//extern nssize g_hack_workspace_volume_height;
//extern nssize g_hack_workspace_volume_length;

//extern NsImage ____image;
//extern NsImage ____proj_xy;
//extern NsImage ____proj_zy;
//extern NsImage ____proj_xz;



#define mDISPLAY_GRAPHICS_IMAGE           mBIT_0
#define mDISPLAY_GRAPHICS_LOW_CONTOUR     mBIT_1
#define mDISPLAY_GRAPHICS_HIGH_CONTOUR    mBIT_2
#define mDISPLAY_NEURON_TREE_VERTICES     mBIT_3
#define mDISPLAY_NEURON_TREE_EDGES        mBIT_4


/* Pass one of the above constants for the 'flag' parameter. */

void SetWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag );

void ClearWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag );

void ToggleWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag );

nsint WorkspaceDisplayFlagIsSet( const HWorkspace handle, const nsuint flag );



#define mMAX_WORKSPACE_FILTER_COLORS   9


typedef enum
   {
   eWORKSPACE_PALETTE_STANDARD,
   //eWORKSPACE_PALETTE_SOLAR_FLARE,
   //eWORKSPACE_PALETTE_RAINBOW,
   //eWORKSPACE_PALETTE_INTENSITY,
   eWORKSPACE_PALETTE_INVERSE,
   //eWORKSPACE_PALETTE_MONOCHROME,
	//eWORKSPACE_PALETTE_CODLFIRE,
   //eWORKSPACE_PALETTE_CHINA,
   //eWORKSPACE_PALETTE_WEB,
   eWORKSPACE_PALETTE_TOPOGRAPHIC,
   //eWORKSPACE_PALETTE_CASCADE,
   //eWORKSPACE_PALETTE_PLASMA,
   //eWORKSPACE_PALETTE_DARK_PASTELS,
   //eWORKSPACE_PALETTE_HILITE,
   //eWORKSPACE_PALETTE_BEARS,
	eWORKSPACE_PALETTE_GAMMA_CORRECTION_1_7,
	eWORKSPACE_PALETTE_GAMMA_CORRECTION_2_2,

	NS_PALETTE_INTENSITY_MASK,
	NS_PALETTE_RED,
	NS_PALETTE_GREEN,
	NS_PALETTE_BLUE,
	NS_PALETTE_YELLOW
   }
   eWORKSPACE_PALETTE_TYPE;


eERROR_TYPE BuildWorkspaceNeuronTree( HWND hOwner, const HWorkspace handle );

eERROR_TYPE image_processing_dynamic_range( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_blur( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_blur_more( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_segment( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_brightness_contrast( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_gamma_correct( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_mp_filter( HWND wnd, const HWorkspace handle, nsint type, nssize num_cpu );
eERROR_TYPE image_processing_resize( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_noise( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_flip_vertical( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_flip_horizontal( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_flip_optical_axis( HWND wnd, const HWorkspace handle );
eERROR_TYPE image_processing_subsample( HWND wnd, const HWorkspace handle );


void workspace_end_projections( HWorkspace handle );
eERROR_TYPE regenerate_projections( HWND wnd, const HWorkspace handle );


NsModel* GetWorkspaceNeuronTree( const HWorkspace handle );

NS_IMPEXP NsModel* workspace_raw_model( HWorkspace handle );
NS_IMPEXP NsModel* workspace_filtered_model( HWorkspace handle );


nsint CallTreeStats( NsModel *th, const nschar *file_name);


eERROR_TYPE workspace_run_spine_analysis( HWND wnd, const HWorkspace handle );


eERROR_TYPE workspace_run_neurite_tracer( HWND wnd, HWorkspace handle, const NsVector3f *V1, const NsVector3f *V2 );


eERROR_TYPE workspace_run_ann_classifier_retyping( HWND wnd, HWorkspace handle );

//extern nsint s_can_draw_workspace;


//NS_IMPEXP void workspace_lock_tiles( HWorkspace handle );
//NS_IMPEXP void workspace_unlock_tiles( HWorkspace handle );



#endif/* __APP_DATA_H__ */
