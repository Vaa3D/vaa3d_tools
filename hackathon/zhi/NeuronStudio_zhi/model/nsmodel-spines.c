#include "nsmodel-spines.h"


const nschar* ns_spine_type_to_string( NsSpineType type )
   {
   NS_PRIVATE const nschar* _ns_spine_type_strings[ NS_SPINE_NUM_TYPES ] =
      {
      "invalid",
      "stubby",
      "thin",
      "mushroom",
      "other"
      };

   return ( ( nssize )type < NS_SPINE_NUM_TYPES ) ?
          _ns_spine_type_strings[ type ] : NULL;
   }


#include <ext/spinetype.inl>


typedef struct _NsSpineAxisPoint
	{
	NsVector3f P;
	}
	NsSpineAxisPoint;

typedef NsSpineAxisPoint NsSpinePoint;


NS_PRIVATE NsError ns_spine_axis_point_new( NsSpineAxisPoint **point, const NsVector3f *P )
	{
	ns_assert( NULL != point );
	ns_assert( NULL != P );

	if( NULL == ( *point = ns_new( NsSpineAxisPoint ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*point)->P = *P;

	return ns_no_error();
	}

#define _ns_spine_point_new ns_spine_axis_point_new


NS_PRIVATE void ns_spine_axis_point_delete( NsSpineAxisPoint *point )
	{
	ns_assert( NULL != point );
	ns_delete( point );
	}

#define _ns_spine_point_delete ns_spine_axis_point_delete


typedef struct _NsSpineAxis
	{
	NsList points;
	}
	NsSpineAxis;


NS_PRIVATE void ns_spine_axis_construct( NsSpineAxis *axis )
	{
	ns_assert( NULL != axis );
	ns_list_construct( &axis->points, ns_spine_axis_point_delete );
	}


NS_PRIVATE void ns_spine_axis_clear( NsSpineAxis *axis )
	{
	ns_assert( NULL != axis );
	ns_list_clear( &axis->points );
	}


NS_PRIVATE void ns_spine_axis_destruct( NsSpineAxis *axis )
	{
	ns_assert( NULL != axis );
	ns_list_destruct( &axis->points );
	}


NS_PRIVATE NsError ns_spine_axis_add_point( NsSpineAxis *axis, const NsVector3f *P )
	{
	NsSpineAxisPoint  *point;
	NsError            error;


	ns_assert( NULL != axis );
	ns_assert( NULL != P );

	if( NS_FAILURE( ns_spine_axis_point_new( &point, P ), error ) )
		return error;

	if( NS_FAILURE( ns_list_push_back( &axis->points, point ), error ) )
		{
		ns_spine_axis_point_delete( point );
		return error;
		}

	return ns_no_error();
	}


enum{ NS_SPINE_CLUMP_UNSEEN, NS_SPINE_CLUMP_SEEN };

typedef struct _NsSpineClump
	{
	nsint        id;
	nsfloat      min_intensity;
	nsfloat      max_intensity;
	nsdouble     min_dts;
	nsdouble     max_dts;
	//nsdouble   mean_dts;
	NsList       voxels;
	NsVoxel     *maximum;
	NsVoxel     *minimum;
	//nsboolean    touches_model;
	NsVector3d   centroid;
	nsboolean    has_centroid;
	NsList       seen;
	NsList       connections;
	nsint        color;
	nsboolean    touches_clump0;
	NsVoxel     *origin;
	}
	NsSpineClump;


typedef struct _NsSpineEvent
   {
   NsVoxel     *origin;
   NsVector3f   O;
	NsVector3f   M;
	NsVector3f   A;
   /* nsboolean    is_attached; */
   NsList       layers;
   NsList       _curr_shell;
   NsList       _next_shell;
   NsList      *curr_shell;
   NsList      *next_shell;
	SPINESTATS   stats;
	/*
   nsint        has_neck;
   nsdouble     head_diameter;
   nsdouble     neck_diameter;
	*/
	nsdouble     correction_factor;
   NsVector3f   min;
   NsVector3f   max;
	nsfloat      threshold;
	nsfloat      contrast;
	nsboolean    can_threshold;
	NsVector3f   head_center;
	nsboolean    has_head_center;
	NsVector3f   neck_center;
	nsboolean    has_neck_center;
   }
   NsSpineEvent;


/* Forward declaration. */
struct _NsSpines;
typedef struct _NsSpines NsSpines;


struct _NsSpine;
typedef struct _NsSpine NsSpine;

struct _NsSpine
   {
   nsint         id;
   nsulong       section;
   nsulong       order;
   nsfloat       section_length;
   nsulong       threshold;
   NsList        voxels;
   NsPoint3f     max;
   NsPoint3f     center;
   NsPoint3f     base;
   NsPoint3f     attach;
   NsPoint3f     min;
	NsPoint3f     rays_origin;
   nsfloat       min_distance;
   nsfloat       max_distance;
   //nsfloat     medial_axis_length;
   //nsfloat     base_distance;
   nsfloat       min_floor;
   NsPoint3f     surface;
   //NsPoint3f   first_layer_center;
   //NsPoint3f   last_layer_center;
   nsdouble      total_volume;
   nsdouble      total_sa;
   nsdouble      attach_sa;
   nsdouble      non_attach_sa;
   nsdouble      voxel_volume;
   nsdouble      layers_volume;
   nsdouble      layers_sa;
   NsVoxel      *origin;
	NsVoxel      *maxima;
	NsVoxel      *minima;
   nsfloat      *distances;
   NsVector3f   *normals;
   nsuint8      *interior;
   NsSpines     *owner;
   NsSpineType   type;
   nsboolean     auto_detected;
	nsboolean     is_analyzed;
   nslistiter    iter;
   nsboolean     is_selected;
   nsboolean     is_attached;
	nsboolean     hidden;
	SPINESTATS    stats;
   NsAABBox3d    bounding_box;
   NsVector3f    bbox_min;
   NsVector3f    bbox_max;
	NsVector3f    approx_axis;
   NsList        layers;
   //NsSpine    *merged;
	nsdouble      xyplane_angle;
	nsfloat       height;
	nsdouble      correction_factor;
	nsfloat       stretch_factor;
	nslong        attach_swc_vertex_id;
	nsfloat       attach_swc_distance;
	nsfloat       soma_distance;
	nsfloat       sholl_distance;
	NsSpineAxis   axis;
	NsVector3f    head_center;
	nsboolean     has_head_center;
	NsVector3f    neck_center;
	nsboolean     has_neck_center;

	const NsVector3f         *vectors;
	nssize                    num_vectors;
	const NsIndexTriangleus  *triangles;
	nssize                    num_triangles;

   /* Experimental */
   //MCSTRIP     **strips;
   //nssize        num_strips;
   //nsdouble      average_intensity;
   //nsulong       max_intensity;
   //NsImage       image;
   //NsImage       proj_xy;
   //NsImage       proj_zy;
   //NsImage       proj_xz;
   };


/* Power-up used in center of mass calculations. */
#define __INTENSITY_POWER  1.5f


#define __MAXIMA        0x0001
#define __MINIMA        0x0002
#define __LAYERED       0x0004
#define __SHELLED       0x0008
#define __VALID         0x0010
#define __TIP_ATTACH    0x0020
#define __OUTLIER       0x0040
#define __FINALIZED     0x0080
#define __ENQUEUED      0x0100
#define __LAST_LAYER    0x0200
#define __HIGH_ANGLE    0x0400
#define __CLUMP_SEED    0x0800
#define __TOUCH_CLUMP0  0x1000
#define __CLUMP_IFACE   0x2000
//#define __NEXT_TO_BK    0x4000

#define __NO_CLUSTER  -1

#define __GET( voxel, bit )\
   ( (voxel)->flags & (bit) )

#define __SET( voxel, bit )\
   ( (voxel)->flags |= (bit) )

#define __CLEAR( voxel, bit )\
   ( (voxel)->flags &= ~(bit) )


#define __DATA( voxel, field )\
   ( ( ( NsSpineData* )(voxel)->data )->field )


#define _NS_SPINE_XYPLANE_ANGLE_IS_UNSET( spine )\
	( (spine)->xyplane_angle < -90.0 || 90.0 < (spine)->xyplane_angle )


nsenum ns_spine_xyplane_angle_type( nsdouble angle )
	{
	nsenum type;

	angle = NS_ABS( angle );
	type  = NS_SPINE_XYPLANE_ANGLE_UNKNOWN;

	if( angle <= 45.0 )
		type = NS_SPINE_XYPLANE_ANGLE_PARALLEL;
	else if( angle <= 90.0 )
		type = NS_SPINE_XYPLANE_ANGLE_OBLIQUE;

	return type;
	}


/* Values for 'direction' field. */
enum{
   _NS_SPINES_NO_DIR,
   _NS_SPINES_DIR_IN,
   _NS_SPINES_DIR_OUT_OR_PAR,

   __NS_SPINES_NUM_DIRS
   };

typedef struct _NsSpineData
   {
   NsSpine       *spine;
   //NsVector3f   gradient;
   //nsfloat      magnitude; /* of the gradient vector */
   nsulong        intensity;
   nsfloat        distance; /* distance to surface D.T.S. */
	//nsfloat      dta; /* distance to axis D.T.A. */
	NsSpineClump  *clump;
	nsint          clump_id;
	nsint          dts_id;
	nsfloat        db;
	nsfloat        ds;
	nsfloat        a;
	NsVoxel       *next;
	NsVector3b     N;
   //nspointer    attach;
   //nsuint       rejected;
	//nslong       cluster;
   //nsushort     direction;
	//nsushort     layer_id;
	//nsushort     num_neighbors;
   //nsuchar      layer_color;
   //nsuchar      layer_index;
   //NsVector3b   N;
   //nsuint8      is_sink; /* or pad */
   }
   NsSpineData;


typedef struct _NsSpineMaxima
	{
	NsVoxel   *voxel;
	nsdouble   correction_factor;
	}
	NsSpineMaxima;


typedef struct _NsSpineLayer
   {
   nsint        index;
   nsdouble     max_spread;
   nsfloat      min_distance;
//	nsfloat      max_advance;
   nsfloat      height;
   nsfloat      cutoff_intensity;
   nsboolean    done;
   NsList       voxels;
   NsVector3f   min;
   NsVector3f   max;
   NsVector3f   center;
   nsboolean    calc_width;
   nsfloat      width;
   nssize       num_samples;
   nsfloat     *distances;
   NsVector3f   F;
   NsVector3f   R;
   NsVector3f   C;
   nsdouble     count_ratio;
   nsdouble     spread_ratio;
	nsdouble     avg_half_spread;
	nsfloat      avg_num_neighbors;
   nsboolean    definites;
   NsLine3f     L;
   NsAABBox3d   B;
   NsList       render;
	nsushort     id;
   }
   NsSpineLayer;


NS_PRIVATE NsError _ns_spine_data_new( NsSpineData **data )
   {
   if( NULL == ( *data = ns_new( NsSpineData ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   //(*data)->layer_color = ( nsuchar )-1;

   return ns_no_error();
   }


NS_PRIVATE void _ns_spine_data_delete( NsSpineData *data )
   {  ns_delete( data );  }


NS_PRIVATE void _ns_spine_layer_voxel_finalize( NsVoxel *voxel )
   {
   ns_assert( __GET( voxel, __LAYERED ) );
   __CLEAR( voxel, __LAYERED );
   }


/* A unique, incrementing I.D. for every layer. */
NS_PRIVATE nsushort ____ns_spines_layer_id;


NS_PRIVATE NsError _ns_spine_layer_new( NsSpineLayer **ret_layer, nsint index )
   {
   NsSpineLayer *layer;

   if( NULL == ( layer = ns_new0( NsSpineLayer ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   layer->index         = index;
   layer->max_spread    = 0.0;
   layer->min_distance  = 0.0f;
   layer->width         = 0.0f;
   layer->height        = 0.0f;
   layer->count_ratio   = 0.0;
   layer->spread_ratio  = 0.0;

   ns_list_construct( &layer->voxels, _ns_spine_layer_voxel_finalize );
   ns_list_construct( &layer->render, NULL );

   ns_model_init_voxel_bounds( &layer->min, &layer->max );

   layer->num_samples = 0;
   layer->distances   = NULL;

	/* Note that we're assuming it doesnt matter if the number wraps around. */
	layer->id = ____ns_spines_layer_id++;

   *ret_layer = layer;
   return ns_no_error();
   }


NS_PRIVATE void _ns_spine_layer_init_distance_to_bottom( NsSpineLayer *layer )
	{
	nslistiter   curr, end;
	NsVoxel     *voxel;


	ns_assert( NULL != layer );
	ns_assert( ! ns_list_is_empty( &layer->voxels ) );

	curr = ns_list_begin( &layer->voxels );
	end  = ns_list_end( &layer->voxels );

	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
		{
		voxel = ns_list_iter_get_object( curr );

		__DATA( voxel, db ) = 0.0f;
		__SET( voxel, __LAST_LAYER );
		}
	}


NS_PRIVATE void _ns_spine_layer_delete_samples( NsSpineLayer *layer )
   {
   ns_free( layer->distances );

   layer->num_samples = 0;
   layer->distances   = NULL;
   }


NS_PRIVATE NsError _ns_spine_layer_alloc_samples( NsSpineLayer *layer, nssize num_samples )
   {
   ns_assert( NULL == layer->distances );

   if( 0 < num_samples )
      {
      if( NULL == ( layer->distances = ns_new_array( nsfloat, num_samples ) ) )
         {
         _ns_spine_layer_delete_samples( layer );
         return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
         }

      ns_assert( 0 == layer->num_samples );
      layer->num_samples = num_samples;
      }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spine_layer_new_samples( NsSpineLayer *layer, NsRayburst *rayburst )
   {
   nssize   num_samples;
   NsError  error;


   num_samples = ns_rayburst_num_samples( rayburst );

   if( NS_FAILURE( _ns_spine_layer_alloc_samples( layer, num_samples ), error ) )
      return error;

   ns_assert( num_samples == layer->num_samples );

   if( 0 < layer->num_samples )
      ns_rayburst_sample_distances(
         rayburst,
         NS_RAYBURST_SAMPLE_DISTANCE_FORWARD,
         layer->distances
         );

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_spine_layer_clone_samples
   (
   const NsSpineLayer  *orig_layer,
   NsSpineLayer        *dup_layer
   )
   {
   NsError error;

   if( NS_FAILURE( _ns_spine_layer_alloc_samples( dup_layer, orig_layer->num_samples ), error ) )
      return error;

   ns_assert( dup_layer->num_samples == orig_layer->num_samples );

   if( 0 < orig_layer->num_samples )
      ns_memcpy(
         dup_layer->distances,
         orig_layer->distances,
         orig_layer->num_samples * sizeof( nsfloat )
         );

   return ns_no_error();
   }


NS_PRIVATE void _ns_spine_layer_delete( NsSpineLayer *layer )
   {
   if( NULL != layer )
      {
      ns_list_destruct( &layer->voxels );
      ns_list_destruct( &layer->render );

      _ns_spine_layer_delete_samples( layer );

      ns_delete( layer );
      }
   }


NS_PRIVATE NsError _ns_spine_layer_add_voxel
   (
   NsSpineLayer       *layer,
   NsVoxel            *voxel,
   const NsVoxelInfo  *voxel_info
   )
   {
   NsVector3f  V;
   NsError     error;


   ns_assert( ! __GET( voxel, __LAYERED ) );
   __SET( voxel, __LAYERED );

   if( NS_FAILURE( ns_list_push_back( &layer->voxels, voxel ), error ) )
      return error;

   //if( 1 == ns_list_size( &layer->voxels ) )layer->min = layer->max = V;

   ns_to_voxel_space( &voxel->position, &V, voxel_info );
   ns_model_update_voxel_bounds( &layer->min, &layer->max, &V, 0.0f );

	//__DATA( voxel, layer_id ) = layer->id;

   return ns_no_error();
   }


#define _NS_SPINES_NUM_LAYER_COLORS  7

#define _NS_VOXEL_FIXED_REJECT_COLOR   7
#define _NS_VOXEL_LAYER_DEVIANT_COLOR  8


NS_PRIVATE const NsColor4ub* _ns_spines_get_layer_colors( void )
   {
   NS_PRIVATE nsboolean  ____ns_spines_layer_colors_init = NS_FALSE;
   NS_PRIVATE NsColor4ub ____ns_spines_layer_colors[ _NS_SPINES_NUM_LAYER_COLORS + 2 ];

   if( ! ____ns_spines_layer_colors_init )
      {
      ____ns_spines_layer_colors[0] = NS_COLOR4UB_RED;
      ____ns_spines_layer_colors[1] = NS_COLOR4UB_ORANGE;
      ____ns_spines_layer_colors[2] = NS_COLOR4UB_YELLOW;
      ____ns_spines_layer_colors[3] = NS_COLOR4UB_GREEN;
      ____ns_spines_layer_colors[4] = NS_COLOR4UB_BLUE;
      ____ns_spines_layer_colors[5] = NS_COLOR4UB_INDIGO;
      ____ns_spines_layer_colors[6] = NS_COLOR4UB_VIOLET;

		/* Special Colors */
		____ns_spines_layer_colors[ _NS_VOXEL_FIXED_REJECT_COLOR  ] = NS_COLOR4UB_WHITE;
		____ns_spines_layer_colors[ _NS_VOXEL_LAYER_DEVIANT_COLOR ] = NS_COLOR4UB_DARK_GREY;

      ____ns_spines_layer_colors_init = NS_TRUE;
      }

   return ____ns_spines_layer_colors;
   }


NS_PRIVATE nsshort _ns_spines_layer_index_to_color_index[ _NS_SPINES_NUM_LAYER_COLORS + 2 ] =
   {
   NS_COLOR4UB_RED_INDEX,
   NS_COLOR4UB_ORANGE_INDEX,
   NS_COLOR4UB_YELLOW_INDEX,
   NS_COLOR4UB_GREEN_INDEX,
   NS_COLOR4UB_BLUE_INDEX,
   NS_COLOR4UB_INDIGO_INDEX,
   NS_COLOR4UB_VIOLET_INDEX,

	/* Special Colors */
	NS_COLOR4UB_WHITE_INDEX,
	NS_COLOR4UB_DARK_GREY_INDEX
   };


NS_PRIVATE void _ns_spine_voxel_finalize( NsVoxel *voxel )
   {
   ns_assert( NULL != __DATA( voxel, spine ) );
   __DATA( voxel, spine ) = NULL;
   }


NS_PRIVATE const NsSpineLayer* _ns_spine_layer_at( const NsSpine *spine, nssize index )
   {
   nslistiter           curr, end;
   const NsSpineLayer  *layer;


   curr = ns_list_begin( &spine->layers );
   end  = ns_list_end( &spine->layers );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      layer = ns_list_iter_get_object( curr );

      if( ( nssize )layer->index == index )
         return layer;
      }

   return NULL;
   }


NS_PRIVATE NsError _ns_spine_add_voxel
   (
   NsSpine            *spine,
   NsVoxel            *voxel,
   const NsVoxelInfo  *voxel_info
   )
   {
   NsVector3f  V;
   NsError     error;


   ns_assert( NULL == __DATA( voxel, spine ) );
   __DATA( voxel, spine ) = spine;

   if( NS_FAILURE( ns_list_push_back( &spine->voxels, voxel ), error ) )
      return error;

   ns_to_voxel_space( &voxel->position, &V, voxel_info );
   ns_model_update_voxel_bounds( &spine->bbox_min, &spine->bbox_max, &V, 0.0f );

   return ns_no_error();
   }


struct _NsSpines
   {
   NsList                 list;
   NsList                 maxima;
	NsList                 hold_maxima;
	nsint                  id;
	nsint                  hold_id;
	nsint                  hold_type;
	nsint                  file;
   NsVoxelTable           voxel_table;
   NsModel               *model;
   const NsSettings      *settings;
   const NsVoxelInfo     *voxel_info;
   const NsImage         *image;
	const NsCubei         *roi;
   NsProcDb              *pixel_proc_db;
   NsRayburstInterpType   interp_type;
   nsboolean              use_2d_bbox;
   nsdouble               average_intensity;
   nsboolean              remove_base_layers;
   nsboolean              enable_merging;
   nsboolean              save_layers;
   nsint                  render_layer;
   nsint                  cutoff_layer;
   nsfloat                clump_dts_merge;
   nsfloat                merge_value;
   nsfloat                merge_power;
	nsint                  axis_smoothing;
   NsProgress            *progress;
   NsVoxelBuffer          voxel_buffer;
   nssize                 num_maxima;
   NsOctreeStats          octree_stats;
   nssize                 max_octree_recursion;
   nsuint                 event_index;
   NsRayburst             rb_surface;
   NsRayburst             rb_layer_radius;
  // NsRayburst             rb_layer_perimeter;
   const NsVector3i      *voxel_offsets;
   const NsVector3i      *voxel_face_offsets;
   const NsVector3i      *voxel_corner_offsets;
   nsboolean              is_running;
   NsFile                 files[ NS_SPINE_NUM_TYPES ];
   nsboolean              do_file;
   nsboolean              active_render;
   void                   ( *render_func )( void );
	const nschar          *config_file;
   nsboolean              do_additional_graphics;
   nstimer                start;
   nstimer                stop;
   nsint                  width;
   nsint                  height;
   nsint                  length;
	nsboolean              use_2d_sampling;
	nsfloat                aabbox_scalar;
	nsint                  min_window;
	nsboolean              optimize_isodata;
	nsboolean              flatten_isodata;
	nsboolean              do_edge_thresholds;
	nsboolean              keep_voxels;
	nsfloat                stretch_factor;
	nsfloat                clump_intensity_merge;
	nsfloat                max_declumping_spread;
	NsList                 clumps;
	NsSpineClump           clump0;
	NsAABBox3d             roi_box;
	NsVector3i             buffer_max;
	NsByteArray            samples;
	NsResidualSmearType    residual_smear_type;
	nsfloat                residual_smear_value;
	nsboolean              clump_merge_by_intensity;
	nsboolean              clump_merge_by_dts;
	nsdouble               min_clump_connectivity;
	NsList                 centroids;
	NsList                 borderers;
	NsList                 iface_voxels;
	nsdouble               max_clump_pinch;
	nsdouble               max_clump_dts_range;
	nsdouble               max_clump_intensity_range;
   };


NS_PRIVATE NsError _ns_spine_new( NsSpine **ret_spine, NsSpines *spines )
   {
   NsSpine *spine;

   if( NULL == ( spine = ns_new0( NsSpine ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   ns_list_construct( &spine->voxels, _ns_spine_voxel_finalize );

   spine->distances = NULL;
   spine->normals   = NULL;
   spine->interior  = NULL;

	spine->stats.head_diameter = NS_SPINE_DEFAULT_HEAD_DIAMETER;
	spine->stats.neck_diameter = NS_SPINE_DEFAULT_NECK_DIAMETER;

	spine->xyplane_angle = NS_SPINE_DEFAULT_XYPLANE_ANGLE;

	spine->stretch_factor = 1.0f;

	spine->attach_swc_vertex_id = -1;
	spine->attach_swc_distance  = -1.0f;

   ns_model_init_voxel_bounds( &spine->bbox_min, &spine->bbox_max );

   ns_list_construct( &spine->layers, _ns_spine_layer_delete );

	ns_spine_axis_construct( &spine->axis );

	NS_USE_VARIABLE( spines );
   //ns_image_construct( &spine->image );
   //ns_image_construct( &spine->proj_xy );
   //ns_image_construct( &spine->proj_zy );
   //ns_image_construct( &spine->proj_xz );
   //ns_image_set_pixel_proc_db( &spine->image, spines->pixel_proc_db );
   //ns_image_set_pixel_proc_db( &spine->proj_xy, spines->pixel_proc_db );
   //ns_image_set_pixel_proc_db( &spine->proj_zy, spines->pixel_proc_db );
   //ns_image_set_pixel_proc_db( &spine->proj_xz, spines->pixel_proc_db );

   *ret_spine = spine;
   return ns_no_error();
   }


NS_PRIVATE void _ns_spine_delete( NsSpine *spine )
   {
   ns_list_destruct( &spine->voxels );

   ns_free( spine->distances );
   ns_free( spine->normals );
   ns_free( spine->interior );

   ns_list_destruct( &spine->layers );

	ns_spine_axis_destruct( &spine->axis );

   //ns_image_destruct( &spine->image );
   //ns_image_destruct( &spine->proj_xy );
   //ns_image_destruct( &spine->proj_zy );
   //ns_image_destruct( &spine->proj_xz );

   ns_delete( spine );
   }


NS_PRIVATE NsError _ns_spine_layer_clone
   (
   const NsSpineLayer   *orig_layer,
   const NsSpines       *spines,
   NsSpineLayer        **ret_layer
   )
   {
   NsSpineLayer  *dup_layer;
   NsError        error;


   if( NULL == ( dup_layer = ns_new( NsSpineLayer ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   dup_layer->num_samples = 0;
   dup_layer->distances   = NULL;

   if( NS_FAILURE( _ns_spine_layer_clone_samples( orig_layer, dup_layer ), error ) )
      {
      ns_free( dup_layer );
      return error;
      }

   /* NOTE: Just initalizing voxel list, not actually cloning it. */
   ns_list_construct( &dup_layer->voxels, _ns_spine_layer_voxel_finalize );
   ns_list_construct( &dup_layer->render, NULL );

   /* Copy the layers voxels for rendering if this is the right layer. */
   /*
   if( orig_layer->index == spines->render_layer )
      {
      nslistiter curr, end;

      curr = ns_list_begin( &orig_layer->voxels );
      end  = ns_list_end( &orig_layer->voxels );

      /* NOTE: Ignore error since this is for display only. *//*
      for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
         ns_list_push_back( &dup_layer->render, ns_list_iter_get_object( curr ) );
      }
   */

   dup_layer->index        = orig_layer->index;
   dup_layer->max_spread   = orig_layer->max_spread;
   dup_layer->min_distance = orig_layer->min_distance;
   dup_layer->height       = orig_layer->height;
   dup_layer->min          = orig_layer->min;
   dup_layer->max          = orig_layer->max;
   dup_layer->center       = orig_layer->center;
   dup_layer->calc_width   = orig_layer->calc_width;
   dup_layer->width        = orig_layer->width;
   dup_layer->C            = orig_layer->C;
   dup_layer->F            = orig_layer->F;
   dup_layer->R            = orig_layer->R;
   dup_layer->count_ratio  = orig_layer->count_ratio;
   dup_layer->spread_ratio = orig_layer->spread_ratio;
   dup_layer->definites    = orig_layer->definites;
   dup_layer->L            = orig_layer->L;
	dup_layer->id           = orig_layer->id;

	dup_layer->avg_half_spread   = orig_layer->avg_half_spread;
	dup_layer->avg_num_neighbors = orig_layer->avg_num_neighbors;

   ns_model_voxel_bounds_to_aabbox( &dup_layer->min, &dup_layer->max, &dup_layer->B );

   /* NOTE: Translate the bounding box by the voxel dimensions if drawing
      it over voxel rectangles. */
   dup_layer->B.width  += ns_voxel_info_size_x( spines->voxel_info );

   dup_layer->B.O.y    += ns_voxel_info_size_y( spines->voxel_info );
   dup_layer->B.height += ns_voxel_info_size_y( spines->voxel_info );

   dup_layer->B.O.z    += ns_voxel_info_size_z( spines->voxel_info );
   dup_layer->B.length += ns_voxel_info_size_z( spines->voxel_info );

   ns_assert( ns_list_is_empty( &dup_layer->voxels ) );

   *ret_layer = dup_layer;

   return ns_no_error();
   }


NS_PRIVATE nsboolean _ns_spine_rayburst_voxel_func
   (
   const NsSpine  *spine,
   nsint           x,
   nsint           y,
   nsint           z,
   nsulong        *value
   )
   {
   NsVoxel   V;
   NsVoxel  *voxel;


   V.position.x = x;
   V.position.y = y;
   V.position.z = z;

   if( NULL != ( voxel = ns_voxel_table_find( &(spine->owner->voxel_table), &V.position ) ) )
      if( spine == __DATA( voxel, spine ) )
         {
         *value = __DATA( voxel, intensity );
         return NS_TRUE;
         }

   return NS_FALSE;
   }


/*
NS_PRIVATE nsboolean _ns_spine_voxel_octree_func
   (
   const NsVector3f  *Vf,
   const NsAABBox3d  *box
   )
   {
   NsPoint3d Vd;

   ns_vector3f_to_3d( Vf, &Vd );
   return ns_point3d_inside_aabbox( &Vd, box );
   }
*/


/* TEMP? Could alter octree intersection function to allow a user-defined void* to
	be passed to the function in addition to the object and bounding box.
	i.e. the parameters would then become ( nspointer, const NsAABBox3d*, nspointer ). */
extern const NsVoxelInfo* _ns_spines_retrieve_voxel_info( void );

NS_PRIVATE nsboolean _ns_spine_voxel_octree_func_special
   (
   const NsVector3f  *Vf,
   const NsAABBox3d  *B
   )
   {
	const NsVoxelInfo  *voxel_info;
   NsPoint3d           Vd;
	nsdouble            dx, dy, dz;


	voxel_info = _ns_spines_retrieve_voxel_info();
   ns_vector3f_to_3d( Vf, &Vd );

	/* Add a little room to the bounding boxes edges to account for voxels that
		are not fully within the bounding box. */

	dx = ( nsdouble )ns_voxel_info_size_x( voxel_info ) * 0.5;
	dy = ( nsdouble )ns_voxel_info_size_y( voxel_info ) * 0.5;
	dz = ( nsdouble )ns_voxel_info_size_z( voxel_info ) * 0.5;

	return
		Vd.x >= ( B->O.x - dx ) && Vd.x < ( B->O.x + B->width  + dx ) &&
		Vd.y <= ( B->O.y + dy ) && Vd.y > ( B->O.y - B->height - dy ) &&
		Vd.z <= ( B->O.z + dz ) && Vd.z > ( B->O.z - B->length - dz );
   }



NS_PRIVATE void _ns_spine_reset_voxels_pointer( NsSpine *spine )
   {
   nslistiter   curr_voxel;
   nslistiter   end_voxels;   
   NsVoxel     *voxel;


   curr_voxel = ns_list_begin( &spine->voxels );
   end_voxels = ns_list_end( &spine->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );
      __DATA( voxel, spine ) = spine;
      }   
   }


/*
NS_PRIVATE void _ns_spines_splice_out( NsSpines *spines, nslistiter iter )
   {
   NsSpine *spine;

   ns_model_lock( spines->model );

   spine = ns_list_iter_get_object( iter );
   ns_assert( ns_list_iter_equal( spine->iter, iter ) );

   ns_list_splice_out( &spines->list, iter, 1 );

   ns_model_unlock( spines->model );
   }


NS_PRIVATE void _ns_spines_splice_in
   (
   NsSpines    *spines,
   nslistiter   iter,
   nsboolean    reset_pointers
   )
   {
   NsSpine *spine;

   ns_model_lock( spines->model );

   spine = ns_list_iter_get_object( iter );
   ns_assert( ns_list_iter_equal( spine->iter, iter ) );

   ns_list_splice_in_back( &spines->list, iter, 1 );

   if( reset_pointers )
      _ns_spine_reset_voxels_pointer( spine );

   ns_model_unlock( spines->model );
   }
*/


NS_PRIVATE void _ns_spines_maintain_ids( NsSpines *spines )
	{
	if( ns_list_is_empty( &spines->list ) )
		spines->id = spines->file = 0;
	}


NS_PRIVATE void _ns_spines_erase_unsync( NsSpines *spines, nslistiter spine )
	{
   ns_list_erase( &spines->list, spine );
	_ns_spines_maintain_ids( spines );
	}


NS_PRIVATE void _ns_spines_erase_sync( NsSpines *spines, nslistiter spine )
   {
   ns_model_lock( spines->model );
	_ns_spines_erase_unsync( spines, spine );
   ns_model_unlock( spines->model );
   }


NS_PRIVATE void _ns_spines_clear_list_by_roi
	(
	NsSpines           *spines,
	const NsVoxelInfo  *voxel_info,
	const NsCubei      *roi
	)
	{
	nslistiter   curr, next, end;
	NsSpine     *spine;
	NsAABBox3d   roi_box;


	ns_assert( NULL != voxel_info );
	ns_assert( NULL != roi );

	ns_model_voxel_bounds_to_aabbox_ex(
		&roi->C1,
		&roi->C2,
		voxel_info,
		&roi_box
		);

	curr = ns_list_begin( &spines->list );
	end  = ns_list_end( &spines->list );

	while( ns_list_iter_not_equal( curr, end ) )
		{
		next  = ns_list_iter_next( curr );
		spine = ns_list_iter_get_object( curr );

		if( ns_aabbox3d_intersects_aabbox( &spine->bounding_box, &roi_box ) )
			ns_list_erase( &spines->list, curr );

		curr = next;
		}
	}


NS_PRIVATE void _ns_spines_clear
	(
	NsSpines           *spines,
	const NsVoxelInfo  *voxel_info,
	const NsCubei      *roi
	)
   {
	if( NULL == roi )
		ns_list_clear( &spines->list );
	else
		_ns_spines_clear_list_by_roi( spines, voxel_info, roi );

   ns_voxel_table_clear( &spines->voxel_table );

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
	_ns_spines_maintain_ids( spines );
   }


NS_PRIVATE void _ns_spines_clear_but_not_voxels( NsSpines *spines )
   {
	ns_list_clear( &spines->list );
	_ns_spines_maintain_ids( spines );
	}


NS_PRIVATE void _ns_spine_clump_delete( NsSpineClump *clump )
	{
	ns_assert( NULL != clump );

	ns_list_destruct( &clump->voxels );
	ns_list_destruct( &clump->seen );
	ns_list_destruct( &clump->connections );

	ns_delete( clump );
	}


NS_PRIVATE NsError _ns_spines_init
   (
   NsSpines              *spines,
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
   )
   {
   NsError error;

   spines->event_index = 0;

   spines->interp_type = interp_type;
   spines->voxel_info  = ns_settings_voxel_info( settings );

   ns_println( "interp type       = %d", ( nsint )interp_type );
   ns_println( "use_2d_bbox       = %d", ( nsint )use_2d_bbox );
   ns_println( "average_intensity = %f", average_intensity );
   ns_println( "multiplier        = %f", ns_settings_get_threshold_multiplier( settings ) );

   if( NS_FAILURE( ns_rayburst_construct(
                     &spines->rb_surface,
                     kernel_type,
                     NS_RAYBURST_RADIUS_NONE,
                     interp_type,
                     image,
                     spines->voxel_info,
                     _ns_spine_rayburst_voxel_func
                     ),
                     error ) )
      return error;

   /* TEMP? Should we allow user to set kernel and/or interpolation type? */
   if( NS_FAILURE( ns_rayburst_construct(
                     &spines->rb_layer_radius,
                     NS_RAYBURST_KERNEL_2D,
                     NS_RAYBURST_RADIUS_MLBD,
                     NS_RAYBURST_INTERP_BILINEAR,
                     image,
                     spines->voxel_info,
                     NULL
                     ),
                     error ) )
      {
      ns_rayburst_destruct( &spines->rb_surface );
      return error;
      }

   /*if( NS_FAILURE( ns_rayburst_construct(
                     &spines->rb_layer_perimeter,
                     NS_RAYBURST_KERNEL_2D_VIZ,
                     NS_RAYBURST_RADIUS_NONE,
                     NS_RAYBURST_INTERP_BILINEAR,
                     image,
                     spines->voxel_info,
                     NULL
                     ),
                     error ) )
      {
      ns_rayburst_destruct( &spines->rb_surface );
      ns_rayburst_destruct( &spines->rb_layer_radius );

      return error;
      }*/

   ns_assert( model->spines == spines );
   ns_assert( spines->model == model );

   spines->settings          = settings;
   spines->image             = image;
	spines->roi               = roi;
   spines->pixel_proc_db     = pixel_proc_db;
   spines->use_2d_bbox       = use_2d_bbox;
   spines->average_intensity = average_intensity;
   spines->progress          = progress;
   spines->render_func       = render_func;
	spines->config_file       = config_file;
	spines->keep_voxels       = keep_voxels;

	spines->residual_smear_type  = residual_smear_type;
	spines->residual_smear_value = ns_residual_smear_type_to_value( residual_smear_type );

	#ifdef NS_DEBUG
	ns_println( "residual smear value = " NS_FMT_DOUBLE, spines->residual_smear_value );
	#endif

   ns_voxel_buffer_init( &spines->voxel_buffer, image );

   ns_voxel_table_set_dimensions(
      &spines->voxel_table,
      ns_voxel_buffer_width( &spines->voxel_buffer ),
      ns_voxel_buffer_height( &spines->voxel_buffer ),
      ns_voxel_buffer_length( &spines->voxel_buffer )
      );

   spines->width  = ns_voxel_buffer_width( &spines->voxel_buffer );
   spines->height = ns_voxel_buffer_height( &spines->voxel_buffer );
   spines->length = ns_voxel_buffer_length( &spines->voxel_buffer );

   ns_list_construct( &spines->maxima, ns_free );
	ns_list_construct( &spines->hold_maxima, ns_free );

	spines->clump0.id = 0;

	if( NULL != roi )
		ns_model_voxel_bounds_to_aabbox_ex(
			&roi->C1,
			&roi->C2,
			ns_settings_voxel_info( settings ),
			&spines->roi_box
			);

   ns_vector3i(
      &spines->buffer_max,
      ns_voxel_buffer_width( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_height( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_length( &spines->voxel_buffer ) - 1
      );

	ns_verify( NS_SUCCESS( ns_byte_array_construct( &spines->samples, 0 ), error ) );

	ns_list_clear( &spines->centroids );
	ns_list_clear( &spines->borderers );
	ns_list_clear( &spines->iface_voxels );

   return ns_no_error();
   }


NS_PRIVATE void _ns_spines_finalize( NsSpines *spines )
   {
   ns_list_destruct( &spines->maxima );
	ns_list_destruct( &spines->hold_maxima );

   ns_rayburst_destruct( &spines->rb_surface );

   ns_rayburst_destruct( &spines->rb_layer_radius );

   //ns_rayburst_destruct( &spines->rb_layer_perimeter );

	ns_byte_array_destruct( &spines->samples );
   }


NS_PRIVATE NsError _ns_spines_find_voxel_angle( NsSpines *spines, NsVector3f *P, nsdouble *angle );


NS_PRIVATE nsboolean _ns_spines_voxel_dts_greater_func( const NsSpineMaxima *M1, const NsSpineMaxima *M2 )
   {
	return __DATA( M1->voxel, distance ) * M1->correction_factor
				>
			 __DATA( M2->voxel, distance ) * M2->correction_factor;
	}


NS_PRIVATE NsError _ns_spines_add_maxima_to_list( NsSpines *spines, NsVoxel *voxel )
	{
	NsSpineMaxima  *M;
	NsVector3f      P;
	nsdouble        angle;
	nsdouble        sine, cosine, sf;
	NsError         error;


	if( NULL == ( M = ns_new( NsSpineMaxima ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	M->voxel = voxel;

	ns_to_voxel_space( &voxel->position, &P, spines->voxel_info );
	_ns_spines_find_voxel_angle( spines, &P, &angle );

	//M->correction_factor = spines->stretch_factor - ns_sin( angle ) * ( spines->stretch_factor - 1.0 );
	
	sine   = ns_sin( angle );
	cosine = ns_cos( angle );
	sf     = spines->stretch_factor;

	M->correction_factor = ns_sqrt( sf * sf * cosine * cosine + sine * sine );

	if( NS_FAILURE( ns_list_push_back( &spines->maxima, M ), error ) )
		{
		ns_free( M );
      return error;
		}

	__SET( voxel, __MAXIMA );

	return ns_no_error();
	}


NS_PRIVATE void _ns_spines_clear_maxima_flags( NsSpines *spines )
	{
	NsSpineMaxima  *M;
	nslistiter      iter;


	NS_LIST_FOREACH( &spines->maxima, iter )
		{
		M = ns_list_iter_get_object( iter );
		__CLEAR( M->voxel, __MAXIMA );
		}
	}


NS_PRIVATE NsError _ns_spines_build_maxima_list( NsSpines *spines )
   {
   NsVoxel  *voxel;
   NsError   error;


   ns_progress_set_title( spines->progress, "Sorting maxima..." );

   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      if( __GET( voxel, __MAXIMA ) )
			if( NS_FAILURE( _ns_spines_add_maxima_to_list( spines, voxel ), error ) )
				return error;

   ns_assert( ns_list_size( &spines->maxima ) == spines->num_maxima );
   ns_list_sort( &spines->maxima, _ns_spines_voxel_dts_greater_func );

/*TEMP*/ns_println( "# of maxima = " NS_FMT_ULONG, ns_list_size( &spines->maxima ) );

   return ns_no_error();
   }


NS_PRIVATE nsfloat _ns_spine_voxel_volume( const NsSpine *spine, const NsSpines *spines )
   {
   return ( nsfloat )ns_list_size( &spine->voxels ) *
          ns_voxel_info_volume( spines->voxel_info );
   }


NS_PRIVATE nsfloat _ns_spine_layer_min_voxel_distance( NsSpineLayer *layer )
   {
   nslistiter   curr_voxel;
   nslistiter   end_voxels;
   NsVoxel     *voxel;
   nsfloat      min_distance;


   min_distance = NS_FLOAT_MAX;

   ns_assert( ! ns_list_is_empty( &layer->voxels ) );

   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
         curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

      if( __DATA( voxel, distance ) < min_distance )
         min_distance = __DATA( voxel, distance );
      }

   return min_distance;
   }


NS_PRIVATE nsdouble _ns_spines_calc_spread
   (
   const NsSpines    *spines,
   const NsVector3f  *min,
   const NsVector3f  *max
   )
   {
   NsVector3f D;

   ns_vector3f_sub( &D, max, min ); /* D = max - min */
   ns_vector3f_cmpd_add( &D, ns_voxel_info_size( spines->voxel_info ) ); /* D += voxel_size */

	D.z /= spines->stretch_factor;

   return ns_sqrtf(
            D.x*D.x +
            D.y*D.y +
            ( spines->use_2d_bbox ? 0.0f : D.z*D.z )
            );
   }


NS_PRIVATE nsdouble _ns_spine_layer_max_spread
   (
   NsSpineLayer  *layer,
   NsSpines      *spines
   )
   {  return _ns_spines_calc_spread( spines, &layer->min, &layer->max );  }


#include "nsmodel-spines-candidates.inl"


NS_PRIVATE nsboolean _ns_spines_calc_attach_and_distance_at
   (
   NsSpines          *spines,
   const NsVector3f  *V,
   NsVector3f        *save_attach,
	nsfloat           *save_distance
	/*,
	nsboolean          ( *voxel_octree_func )( const NsVector3f*Vf, const NsAABBox3d* )*/
   )
   {
   NsVector             nodes;
   const NsOctreeNode  *node;
   nsfloat              min_distance;
   nsfloat              distance;
   nsvectoriter         curr_node;
   nsvectoriter         end_nodes;
   nsvectoriter         curr_object;
   nsvectoriter         end_objects;
   nsmodeledge          edge;
   NsVector3f           curr_attach;
   nsboolean            did_attach;
   NsError              error;


   did_attach = NS_FALSE;

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

   /* TEMP: Ignore failure? */
   if( NS_SUCCESS(
         ns_model_octree_intersections(
            spines->model,
            NS_MODEL_EDGE_OCTREE,
            ( nspointer )V,
            _ns_spine_voxel_octree_func_special,//voxel_octree_func,
            &nodes
            ),
         error ) )
      {
      min_distance = NS_FLOAT_MAX;

      curr_node = ns_vector_begin( &nodes );
      end_nodes = ns_vector_end( &nodes );

      for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
             curr_node = ns_vector_iter_next( curr_node ) )
         {
         node = ns_vector_iter_get_object( curr_node );

         curr_object = ns_octree_node_begin_objects( node );
         end_objects = ns_octree_node_end_objects( node );

         for( ; ns_vector_iter_not_equal( curr_object, end_objects );
                curr_object = ns_vector_iter_next( curr_object ) )
            {
            edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

            /* Get the distance and attachemnt to each tapered cylinder. */
            distance = _ns_spines_distance_to_conical_frustum(
									spines,
                           V,
                           edge,
                           &curr_attach,
                           NULL,
									NULL,
                           NULL,
									NULL
                           );

            if( distance < min_distance )
               {
               min_distance   = distance;
               *save_attach   = curr_attach;
					*save_distance = distance;
               did_attach     = NS_TRUE;
               }
            }
         }
      }

   ns_vector_destruct( &nodes );
   return did_attach;
   }


NS_PRIVATE nsdouble _ns_spine_voxels_max_angle
	(
	NsSpines           *spines,
	const NsVector3f   *center,
	const NsList       *list
	)
	{
	nsdouble        max_angle, angle;
	NsVector3f      A, B, V, attach;
	nslistiter      iter;
	const NsVoxel  *voxel;
	nsfloat         distance;



	max_angle = 0.0;

	if( _ns_spines_calc_attach_and_distance_at( spines, center, &attach, &distance ) )
		{
		ns_vector3f_sub( &A, center, &attach );

		NS_LIST_FOREACH( list, iter )
			{
			voxel = ns_list_iter_get_object( iter );
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

			ns_vector3f_sub( &B, &V, &attach );

			angle = ns_vector3f_angle( &A, &B );
			angle = NS_ABS( angle );
			angle = NS_RADIANS_TO_DEGREES( angle );

			if( angle > max_angle )
				max_angle = angle;
			}
		}

	return max_angle;
	}


NS_PRIVATE void _ns_spine_voxels_set_high_angled
	(
	NsSpines           *spines,
	const NsVector3f   *center,
	const NsList       *list,
	nsdouble            max_angle
	)
	{
	nsdouble     angle;
	NsVector3f   A, B, V, attach;
	nslistiter   iter;
	NsVoxel     *voxel;
	nsfloat      distance;


	if( _ns_spines_calc_attach_and_distance_at( spines, center, &attach, &distance ) )
		{
		ns_vector3f_sub( &A, center, &attach );

		NS_LIST_FOREACH( list, iter )
			{
			voxel = ns_list_iter_get_object( iter );
			ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );

			ns_vector3f_sub( &B, &V, &attach );

			angle = ns_vector3f_angle( &A, &B );
			angle = NS_ABS( angle );
			angle = NS_RADIANS_TO_DEGREES( angle );

			if( angle > max_angle )
				__SET( voxel, __HIGH_ANGLE );
			}
		}
	}


NS_PRIVATE void _ns_spine_layer_remove_high_angled( NsSpineLayer *layer )
	{
	nslistiter   curr, next, end;
	NsVoxel     *voxel;


	curr = ns_list_begin( &layer->voxels );
	end  = ns_list_end( &layer->voxels );

	while( ns_list_iter_not_equal( curr, end ) )
		{
		next  = ns_list_iter_next( curr );
		voxel = ns_list_iter_get_object( curr );

		if( __GET( voxel, __HIGH_ANGLE ) )
			ns_list_erase( &layer->voxels, curr );

		curr = next;
		}
	}


NS_PRIVATE NsError _ns_spines_find_voxel_angle( NsSpines *spines, NsVector3f *P, nsdouble *angle )
	{
	NsVector3f           A, attach;
	nsfloat              distance;
	nsfloat              min_distance;
	const NsOctreeNode  *node;
	nsmodeledge          edge;
	NsVector             nodes;
	nsvectoriter         curr_node, end_nodes;
   nsvectoriter         curr_object, end_objects;
	NsVector3f           S;
	NsVector3f           Z;
	NsError              error;


	*angle = 0.0;

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

   if( NS_FAILURE(
         ns_model_octree_intersections(
            spines->model,
            NS_MODEL_EDGE_OCTREE,
            P,
            _ns_spine_voxel_octree_func_special,
            &nodes
            ),
         error ) )
		{
		ns_vector_destruct( &nodes );
		return error;
		}

   min_distance = NS_FLOAT_MAX;

   curr_node = ns_vector_begin( &nodes );
   end_nodes = ns_vector_end( &nodes );

   for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
          curr_node = ns_vector_iter_next( curr_node ) )
      {
      node = ns_vector_iter_get_object( curr_node );
      //ns_assert( 0 < ns_octree_node_num_objects( node ) );

      curr_object = ns_octree_node_begin_objects( node );
      end_objects = ns_octree_node_end_objects( node );

      for( ; ns_vector_iter_not_equal( curr_object, end_objects );
             curr_object = ns_vector_iter_next( curr_object ) )
         {
         edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

         distance = _ns_spines_distance_to_conical_frustum(
							spines,
                     P,
                     edge,
                     &A,
                     NULL,
							NULL,
                     NULL,
							NULL
                     );

         if( distance < min_distance )
            {
            min_distance = distance;
            attach       = A;
            }
         }
      }

	/* Was 'attach' actually set? Only would happen if the model
		has no vertices or edges. */
	if( 0 < ns_vector_size( &nodes ) )
		{
		ns_vector3f_sub( &S, P, &attach ); /* S = origin - attach */
		ns_vector3f( &Z, 0.0f, 0.0f, 1.0f ); /* Z-axis vector */

		/* Get the angle in radians. */
		*angle = ( nsdouble )ns_vector3f_angle( &S, &Z );

		/* Correct for any floating point error. */
		if( *angle < 0.0 )
			*angle = 0.0;
		else if( *angle > NS_PI )
			*angle = NS_PI;

//ns_println( NS_FMT_DOUBLE " ", NS_RADIANS_TO_DEGREES( *angle ) );
		}

	ns_vector_destruct( &nodes );
	return ns_no_error();
	}


NS_PRIVATE void _ns_spines_calc_threshold_and_contrast_at
   (
   NsSpines          *spines,
   const NsVector3f  *V,
	nsfloat           *threshold,
	nsfloat           *contrast
   )
   {
   NsVector             nodes;
   const NsOctreeNode  *node;
   nsfloat              min_distance;
   nsfloat              distance;
   nsvectoriter         curr_node;
   nsvectoriter         end_nodes;
   nsvectoriter         curr_object;
   nsvectoriter         end_objects;
   nsmodeledge          edge;
   nsfloat              local_threshold;
	nsfloat              local_contrast;
   NsError              error;


   *threshold = 0.0f;
	*contrast  = 0.0f;

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

   /* TEMP: Ignore failure? */
   if( NS_SUCCESS(
         ns_model_octree_intersections(
            spines->model,
            NS_MODEL_EDGE_OCTREE,
            ( nspointer )V,
            _ns_spine_voxel_octree_func_special,
            &nodes
            ),
         error ) )
      {
      min_distance = NS_FLOAT_MAX;

      curr_node = ns_vector_begin( &nodes );
      end_nodes = ns_vector_end( &nodes );

      for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
             curr_node = ns_vector_iter_next( curr_node ) )
         {
         node = ns_vector_iter_get_object( curr_node );

         curr_object = ns_octree_node_begin_objects( node );
         end_objects = ns_octree_node_end_objects( node );

         for( ; ns_vector_iter_not_equal( curr_object, end_objects );
                curr_object = ns_vector_iter_next( curr_object ) )
            {
            edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

            /* Get the distance and attachemnt to each tapered cylinder. */
            distance = _ns_spines_distance_to_conical_frustum(
									spines,
                           V,
                           edge,
                           NULL,
                           &local_threshold,
									&local_contrast,
                           NULL,
									NULL
                           );

            if( distance < min_distance )
               {
               min_distance = distance;

               *threshold = local_threshold;
					*contrast  = local_contrast;
               }
            }
         }
      }

   ns_vector_destruct( &nodes );
   }


NS_PRIVATE void _ns_spine_layer_calc_weighted_center
   (
   NsSpineLayer       *layer,
   const NsVoxelInfo  *voxel_info,
   nsfloat             power
   )
   {
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   const NsVoxel  *voxel;
   nsfloat         curr_intensity;
   nsfloat         sum_intensity;
   NsVector3f      V;


   /* Do a WEIGHTED average on the x,y,z of the layers voxel
      to get the center point of the layer. */

   sum_intensity = 0.0f;
   ns_vector3f_zero( &layer->center );

   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );
         
      curr_intensity = ns_powf( voxel->intensity, power );

      sum_intensity += curr_intensity;

      ns_to_voxel_space( &voxel->position, &V, voxel_info );

      ns_vector3f_cmpd_scale( &V, curr_intensity );
      ns_vector3f_cmpd_add( &layer->center, &V );
      }

   ns_vector3f_cmpd_scale( &layer->center, 1.0f / sum_intensity );
   }


/*
NS_PRIVATE NsVector3f* _ns_spine_layer_calc_positional_center
   (
   const NsSpineLayer  *layer,
   const NsVoxelInfo   *voxel_info,
	NsVector3f          *center
   )
   {
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   const NsVoxel  *voxel;
   NsVector3f      V;


   ns_vector3f_zero( center );

   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );
      ns_to_voxel_space( &voxel->position, &V, voxel_info );

      ns_vector3f_cmpd_add( center, &V );
      }

	if( ! ns_list_is_empty( &layer->voxels ) )
		ns_vector3f_cmpd_scale(
			center,
			1.0f / ( ( nsfloat )ns_list_size( &layer->voxels ) )
			);

	return center;
   }
*/


/*
NS_PRIVATE void _ns_spine_layer_calc_average_spread
	(
	NsSpineLayer       *layer,
   const NsVoxelInfo  *voxel_info
	)
	{
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   const NsVoxel  *voxel;
   NsVector3f      V;
	NsVector3f      center;
	nsdouble        sum_distances;


	_ns_spine_layer_calc_positional_center( layer, voxel_info, &center );

   sum_distances = 0.0;

   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );
      ns_to_voxel_space( &voxel->position, &V, voxel_info );

		sum_distances += ( ( nsdouble )ns_vector3f_distance( &V, &center ) );
      }

	if( ! ns_list_is_empty( &layer->voxels ) )
		layer->avg_half_spread = sum_distances / ( ( nsdouble )ns_list_size( &layer->voxels ) );
	else
		layer->avg_half_spread = 0.0;
	}
*/


/*
NS_PRIVATE void _ns_spine_layer_voxels_set_neighbors_count( NsSpineLayer *layer, NsSpines *spines )
	{
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   NsVoxel        *voxel;
	NsVector3i      N;
	const NsVoxel  *neighbor;
	nssize          i;
	nsushort        layer_id;
	nsushort        num_neighbors;


   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

		layer_id      = __DATA( voxel, layer_id );
		num_neighbors = 0;

		for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
			{
			ns_vector3i_add( &N, &voxel->position, spines->voxel_offsets + i );

			if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N ) ) )
				if( __DATA( neighbor, layer_id ) == layer_id )
					++num_neighbors;
			}

		__DATA( voxel, num_neighbors ) = num_neighbors;
      }
	}
*/

/*
NS_PRIVATE void _ns_spine_layer_calc_average_neighbor_count( NsSpineLayer *layer )
	{
   nslistiter      curr_voxel;
   nslistiter      end_voxels;
   const NsVoxel  *voxel;
	nsfloat         sum_num_neigbors;


   sum_num_neigbors = 0.0f;

   curr_voxel = ns_list_begin( &layer->voxels );
   end_voxels = ns_list_end( &layer->voxels );

   for( ; ns_list_iter_not_equal( curr_voxel, end_voxels );
          curr_voxel = ns_list_iter_next( curr_voxel ) )
      {
      voxel = ns_list_iter_get_object( curr_voxel );

		sum_num_neigbors += ( ( nsfloat )__DATA( voxel, num_neighbors ) );
      }

	if( ! ns_list_is_empty( &layer->voxels ) )
		layer->avg_num_neighbors = sum_num_neigbors / ( ( nsfloat )ns_list_size( &layer->voxels ) );
	else
		layer->avg_num_neighbors = 0.0f;
	}
*/


NS_PRIVATE void _ns_spine_layer_calc_rayburst_width( NsSpineLayer *layer, NsSpines *spines, NsSpineEvent *event )
   {
   nsfloat threshold, contrast, radius, offset, multiplier;

   ns_assert( layer->calc_width );

	NS_USE_VARIABLE( event );

   multiplier = ns_settings_get_threshold_multiplier( spines->settings );

	/*if( NULL != event )
		{
		ns_assert( event->can_threshold );

		threshold = event->threshold;
		contrast  = event->contrast;
		}
	else*/
		_ns_spines_calc_threshold_and_contrast_at( spines, &layer->center, &threshold, &contrast );

	threshold = ns_model_scale_threshold( threshold, contrast, multiplier );

   radius = 0.0f;

   if( 0.0f < threshold )
      {
      ns_rayburst_set_threshold( &spines->rb_layer_radius, threshold );
      ns_rayburst( &spines->rb_layer_radius, &layer->center );

      radius = ns_rayburst_radius( &spines->rb_layer_radius );

      /* NOTE: Ignoring failure of this function. */
      if( spines->save_layers )
         {
         ns_rayburst_mlbd( &spines->rb_layer_radius, &layer->C, &layer->F, &layer->R, NULL );

         /* Offset the chosen radius vectors just by a little so they can
            be rendered properly, i.e. dont overlap vectors calculated below. */
         offset = ns_voxel_info_size_z( spines->voxel_info ) / 10.0f;

         layer->C.z -= offset;
         layer->F.z -= offset;
         layer->R.z -= offset;

        // ns_rayburst_set_threshold( &spines->rb_layer_perimeter, threshold );
         //ns_rayburst( &spines->rb_layer_perimeter, &layer->center );
         
         //_ns_spine_layer_new_samples( layer, &spines->rb_layer_perimeter );
         }
      }

   layer->width = radius * 2.0f;
   }


void ns_model_spines_hold_id_and_type( const NsModel *model, nsspine spine )
	{
	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

	model->spines->hold_id   = ( nsint )ns_spine_get_id( spine );
	model->spines->hold_type = ( nsint )ns_spine_get_type( spine );
	}


void _ns_spines_sort_node( NsSpines *spines, nsspine spine )
	{
	nsspine scan, end;

	/* IMPORTANT: All the nodes in the spine list are assumed to be sorted
		in ascending order except for the 'spine' node. Therefore we just splice
		it out of the list, and scan the list from back to front for a node with
		an id that is less than the 'spine' node. When we find it we need to
		re-insert the spine node after it. */

	ns_list_splice_out( &spines->list, spine, 1 );

	scan = ns_list_rev_begin( &spines->list );
	end  = ns_list_rev_end( &spines->list );

	for( ; ns_list_rev_iter_not_equal( scan, end ); scan = ns_list_rev_iter_next( scan ) )
		if( ns_spine_get_id( spine ) > ns_spine_get_id( scan ) )
			{
			/* This will work even if the scan node is pointing to the last node in the
				list since the list is circular (see std/nslist.c implementation) and therefore
				ns_list_iter_next( scan ) gives us the dummy header node. When 'spine' is inserted
				before the dummy header node it will actually end up as the new last node in the
				list, which is correct. Note also we could just insert a break statement in this
				loop but I think its more clear to just do it this way. */
			ns_list_splice_in( &spines->list, ns_list_iter_next( scan ), spine, 1 );
			return;
			}

	/* If the scan proceeded all the way to the dummy header node, this 'spine' is going to
		become the new first node in the list. Again because of the circular nature of the list,
		scan is pointing to the dummy header so ns_list_iter_next( scan ) gives us the first node
		in the list. Splicing in the spine in front of it makes it the new first node, which is
		correct. */
	ns_list_splice_in( &spines->list, ns_list_iter_next( scan ), spine, 1 );
	}


void _ns_spines_increment_id( NsSpines *spines, nsspine spine )
	{
	nsint    id;
	nsspine  end;


	end = ns_list_end( &spines->list );

	/* Scan through the list for an available ID. Note that the
		'id' in the loop advances with each iteration but that the
		current spine pointer doesnt have to. */

	id    = ( nsint )ns_spine_get_id( spine ) + 1;
	spine = ns_spine_next( spine );

	while( ns_spine_not_equal( spine, end ) )
		{
		if( id == ( nsint )ns_spine_get_id( spine ) )
			spine = ns_spine_next( spine );
		else
			break;

		++id;
		}

	spines->id = id;
	}


nsboolean _ns_spines_decrement_id( NsSpines *spines, nsspine spine )
	{
	nsint    id;
	nsspine  end;


	end = ns_list_rev_end( &spines->list );

	/* Scan through the list for an available ID. Note that the
		'id' in the loop advances with each iteration but that the
		current spine pointer doesnt have to. */

	id    = ( nsint )ns_spine_get_id( spine ) - 1;
	spine = ns_spine_prev( spine );

	while( ns_spine_not_equal( spine, end ) )
		{
		if( id == ( nsint )ns_spine_get_id( spine ) )
			spine = ns_spine_prev( spine );
		else
			break;

		--id;
		}

	/* If ID goes negative, then there is no available previous ID. */
	if( 0 <= id )
		spines->id = id;

	return 0 <= id;
	}


void _ns_spines_set_id( NsSpines *spines, nsspine spine )
	{
	if( 0 <= spines->hold_id )
		{
		ns_spine_set_id( spine, spines->hold_id );
		_ns_spines_sort_node( spines, spine );

		ns_spine_set_type( spine, spines->hold_type );

		spines->hold_id   = -1;
		spines->hold_type = -1;
		}
	else
		{
		ns_spine_set_id( spine, spines->id );
		_ns_spines_sort_node( spines, spine );
		_ns_spines_increment_id( spines, spine );
		}
	}


void _ns_model_spines_increment_selected_spine_id( NsModel *model, nsspine spine )
	{
	NsSpines *spines = model->spines;

	_ns_spines_increment_id( spines, spine );
	ns_spine_set_id( spine, spines->id );
	_ns_spines_sort_node( spines, spine );
	_ns_spines_increment_id( spines, spine );
	}


void _ns_model_spines_decrement_selected_spine_id( NsModel *model, nsspine spine )
	{
	NsSpines *spines = model->spines;

	if( _ns_spines_decrement_id( spines, spine ) )
		{
		ns_spine_set_id( spine, spines->id );
		_ns_spines_sort_node( spines, spine );
		_ns_spines_increment_id( spines, spine );
		}
	}


void ns_model_spines_offset_selected_spine_id( NsModel *model, nsboolean increment )
	{
	nsspine spine;

	ns_assert( NULL != model );

	if( 1 == ns_model_num_selected_spines_ex( model, &spine ) )
		{
		if( increment )
			_ns_model_spines_increment_selected_spine_id( model, spine );
		else
			_ns_model_spines_decrement_selected_spine_id( model, spine );
		}
	}


NS_PRIVATE nsboolean _ns_spine_clump_has_been_seen( const NsSpineClump *A, const NsSpineClump *B )
	{
	nslistiter iter;

	NS_LIST_FOREACH( &A->seen, iter )
		if( ( const NsSpineClump* )ns_list_iter_get_object( iter ) == B )
			return NS_TRUE;

	return NS_FALSE;
	}


NS_PRIVATE nsboolean _ns_spine_clump_has_connection( const NsSpineClump *A, const NsSpineClump *B )
	{
	nslistiter iter;

	NS_LIST_FOREACH( &A->connections, iter )
		if( ( const NsSpineClump* )ns_list_iter_get_object( iter ) == B )
			return NS_TRUE;

	return NS_FALSE;
	}


#include "nsmodel-spines-event.inl"

#include "nsmodel-spines-shape.inl"

#include "nsmodel-spines-analyze.inl"

#include "nsmodel-spines-render.inl"

#include "nsmodel-spines-io.inl"

#include "nsmodel-spines-edit.inl"

#include "nsmodel-spines-classes.inl"


NS_PRIVATE NsError _ns_spines_do_construct( NsSpines *spines )
   {
   NsError error;

   if( NS_FAILURE( ns_voxel_table_construct(
                     &spines->voxel_table,
                     _ns_spine_data_delete
                     ),
                     error ) )
      return error;

   ns_list_construct( &spines->list, _ns_spine_delete );

   spines->voxel_offsets        = ns_voxel_offsets();
   spines->voxel_face_offsets   = ns_voxel_face_offsets();
   spines->voxel_corner_offsets = ns_voxel_corner_offsets();

	ns_list_construct( &spines->clumps, _ns_spine_clump_delete );

	spines->hold_id   = -1;
	spines->hold_type = -1;

	ns_list_construct( &spines->centroids, _ns_spine_point_delete );
	ns_list_construct( &spines->borderers, _ns_spine_point_delete );

	ns_list_construct( &spines->iface_voxels, NULL );

   return ns_no_error();
   }


NS_PRIVATE void _ns_spines_do_destruct( NsSpines *spines )
   {
	ns_list_destruct( &spines->clumps );

   ns_list_destruct( &spines->list );

   ns_voxel_table_destruct( &spines->voxel_table );

	ns_list_destruct( &spines->centroids );
	ns_list_destruct( &spines->borderers );

	ns_list_destruct( &spines->iface_voxels );
   }


NsError ns_model_new_spines( NsModel *model )
   {
   NsError error;

   ns_assert( NULL != model );
   ns_assert( NULL == model->spines );

   if( NULL == ( model->spines = ns_new0( NsSpines ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   if( NS_FAILURE( _ns_spines_do_construct( model->spines ), error ) )
      {
      ns_delete( model->spines );
      model->spines = NULL;

      return error;
      }

   model->spines->model = model;

   return ns_no_error();
   }


void ns_model_delete_spines( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   _ns_spines_do_destruct( model->spines );

   ns_delete( model->spines );
   model->spines = NULL;
   }


nsboolean ns_model_spines_is_running( const NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   return model->spines->is_running;
   }


const NsVoxelTable* ns_model_spines_voxel_table( const NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   return &( model->spines->voxel_table );
   }


nsboolean ns_model_spine_voxel_is_maxima( const NsVoxel *voxel )
	{
	ns_assert( NULL != voxel );
	return __GET( voxel, __MAXIMA );
	}


nsfloat ns_model_spine_voxel_dts( const NsVoxel *voxel )
	{
	ns_assert( NULL != voxel );
	return __DATA( voxel, distance );
	}


void ns_model_clear_spines
	(
	NsModel            *model,
	const NsVoxelInfo  *voxel_info,
	const NsCubei      *roi
	)
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   ns_model_lock( model );
   _ns_spines_clear( model->spines, voxel_info, roi );
   ns_model_unlock( model );
   }


void ns_model_clear_spines_but_not_voxels( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   ns_model_lock( model );
   _ns_spines_clear_but_not_voxels( model->spines );
   ns_model_unlock( model );
   }


NS_PRIVATE void _ns_spines_calc_bounding_boxes( NsSpines *spines )
   {
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ! spine->is_analyzed )
			ns_model_voxel_bounds_to_aabbox(
				&spine->bbox_min,
				&spine->bbox_max,
				&spine->bounding_box
				);
      }
   }


#include <ext/neighbors.inl>


/*
NS_PRIVATE NsError _ns_spines_fix_rejected_voxels( NsSpines *spines )
   {
   NsVoxel    *voxel, *neighbor;
   nssize      before_count;
   nssize      after_count;
   nssize      i;
   NsVoxel     N;
   NsSpine    *spine;
   NsError     error;
   nspointer   neighbors[ NS_VOXEL_NUM_OFFSETS ];


   ns_progress_set_title( spines->progress, "Fixing rejected voxels..." );

   before_count = 0;
   after_count  = 0;

   /* Try to add the rejected voxels to a spine, that is if they arent
      already in one. *//*

   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      if( 0 < __DATA( voxel, rejected ) && NULL == __DATA( voxel, spine ) )
         {
         ns_assert( NULL == __DATA( voxel, spine ) );
         ++before_count;

         for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
            {
            neighbors[i] = NULL;

            ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );

            if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
               neighbors[i] = __DATA( neighbor, spine );
            }

         spine = select_neighbor_id( neighbors );

         /* NOTE: Dont add any that are below the minimum DTS for the entire
            spine. This prevents adding voxels along the base layers that have
            already been removed. *//*

         if( NULL != spine )
            if( spine->min_floor <= __DATA( voxel, distance ) )
               {
               if( NS_FAILURE( _ns_spine_add_voxel( spine, voxel, spines->voxel_info ), error ) )
                  return error;

               __DATA( voxel, layer_index ) = _NS_VOXEL_FIXED_REJECT_COLOR;
               __DATA( voxel, layer_color ) = _NS_VOXEL_FIXED_REJECT_COLOR;

               ++after_count;
               }
         }

   /*TEMP*//*ns_println(
            "fixed " NS_FMT_ULONG " out of " NS_FMT_ULONG " rejected voxels.",
            after_count, before_count
            );

   return ns_no_error();
   }*/


NS_PRIVATE const nschar* ____ns_spines_file_names[ NS_SPINE_NUM_TYPES ] =
   {
   "ns-profiles-invalid.txt",
   "ns-profiles-other.txt",
   "ns-profiles-thin.txt",
   "ns-profiles-mushroom.txt",
   "ns-profiles-stubby.txt"
   };


NS_PRIVATE void _ns_spines_open_files( NsSpines *spines )
   {
   nssize i;

   ns_assert( spines->do_file );

   for( i = 0; i < NS_SPINE_NUM_TYPES; ++i )
      {
      ns_file_construct( spines->files + i );
      ns_file_open( spines->files + i, ____ns_spines_file_names[i], NS_FILE_MODE_WRITE );
      }
   }


NS_PRIVATE void _ns_spines_close_files( NsSpines *spines )
   {
   nssize i;

   ns_assert( spines->do_file );

   for( i = 0; i < NS_SPINE_NUM_TYPES; ++i )
      ns_file_destruct( spines->files + i );
   }


NS_PRIVATE void _ns_spines_get_voxel_corners
   (
   NsSpines          *spines,
   const NsVector3i  *V,
   nsfloat           *corners
   )
   {
   NsVector3i  N;
   nssize      i;


   for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
      {
      ns_vector3i_add( &N, V, spines->voxel_corner_offsets + i );

      if( 0 <= N.x && N.x < spines->width  &&
          0 <= N.y && N.y < spines->height &&
          0 <= N.z && N.z < spines->length   )
         corners[i] = ( nsfloat )ns_voxel_get( &spines->voxel_buffer, N.x, N.y, N.z );
      else
         corners[i] = 0.0f;
      }
   }


NS_PRIVATE void _ns_spines_compute_normals( NsSpines *spines )
   {
/*TEMP: Avoid lengthy normal calculation process. Its only for visual display anyways. */
NS_USE_VARIABLE( spines );
/*
   NsVoxel  *voxel;
   nsfloat   corners[ NS_VOXEL_NUM_CORNER_OFFSETS ];
   nsfloat   normal[3];


   ns_progress_set_title( spines->progress, "Computing Normals..." );

   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      {
      _ns_spines_get_voxel_corners( spines, &( voxel->position ), corners );
      ComputeImageNormal( corners, normal );

      /* Reverse normal y and z since the data axis orientation requires
         a rotation to align it with OpenGL's axis orientation. *//*
      __DATA( voxel, N ).x = NS_NORMAL_FLOAT_TO_SIGNED_BYTE(  normal[0] );
      __DATA( voxel, N ).y = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[1] );
      __DATA( voxel, N ).z = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[2] );
      }*/
   }


NS_PRIVATE void _ns_spines_make_polygon_strips( NsSpines *spines )
   {
   /* TODO */
   NS_USE_VARIABLE( spines );
   }


/*
NS_PRIVATE NsError _ns_spine_write_projections( const NsSpine *spine, const nschar *dir_name )
   {
   TiffFile     tiff;
   NsIO         io;
   NsFile       file;
   NsPixelType  pixel_type;
   NsError      error;
   nschar       file_name[ NS_PATH_SIZE ];


   error = ns_no_error();

   ns_file_construct( &file );

   io.progress = NULL;
   io.file     = &file;

   tiff_file_construct( &tiff, &io );

   pixel_type = ns_image_pixel_type( &spine->proj_xy );

   ns_assert( ns_image_pixel_type( &spine->proj_zy ) == pixel_type );
   ns_assert( ns_image_pixel_type( &spine->proj_xz ) == pixel_type );

   switch( pixel_type )
      {
      case NS_PIXEL_LUM_U8 :
         tiff.pixel_type         = TIFF_PIXEL_LUM8;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM8_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM8_NUM_BITS;
         break;

      case NS_PIXEL_LUM_U12:
         tiff.pixel_type         = TIFF_PIXEL_LUM12;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM16_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM12_NUM_BITS;
         break;

      case NS_PIXEL_LUM_U16:
         tiff.pixel_type         = TIFF_PIXEL_LUM16;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM16_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM16_NUM_BITS;
         break;

      default:
         ns_assert_not_reached();
      }

   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%sspine%04d_xy.tif", dir_name, spine->id );

   tiff.width  = ns_image_width( &spine->proj_xy );
   tiff.height = ns_image_height( &spine->proj_xy );
   tiff.length = ns_image_length( &spine->proj_xy );
   tiff.pixels = ns_image_pixels( &spine->proj_xy );

   if( TIFF_NO_ERROR != tiff_file_write( &tiff, file_name, NULL, NULL, &io ) )
      {
      error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      goto _NS_SPINE_WRITE_PROJECTIONS_EXIT;
      }

   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%sspine%04d_zy.tif", dir_name, spine->id );

   tiff.width  = ns_image_width( &spine->proj_zy );
   tiff.height = ns_image_height( &spine->proj_zy );
   tiff.length = ns_image_length( &spine->proj_zy );
   tiff.pixels = ns_image_pixels( &spine->proj_zy );

   if( TIFF_NO_ERROR != tiff_file_write( &tiff, file_name, NULL, NULL, &io ) )
      {
      error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      goto _NS_SPINE_WRITE_PROJECTIONS_EXIT;
      }

   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%sspine%04d_xz.tif", dir_name, spine->id );

   tiff.width  = ns_image_width( &spine->proj_xz );
   tiff.height = ns_image_height( &spine->proj_xz );
   tiff.length = ns_image_length( &spine->proj_xz );
   tiff.pixels = ns_image_pixels( &spine->proj_xz );

   if( TIFF_NO_ERROR != tiff_file_write( &tiff, file_name, NULL, NULL, &io ) )
      {
      error = ns_error_io( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      goto _NS_SPINE_WRITE_PROJECTIONS_EXIT;
      }

   _NS_SPINE_WRITE_PROJECTIONS_EXIT:

   tiff.pixels = NULL;
   tiff_file_destruct( &tiff, &io );

   ns_file_destruct( &file );
   return error;
   }
*/


NS_PRIVATE void _ns_spines_write_montages( NsSpines *spines, const nschar *dir_name )
   {
   NS_USE_VARIABLE( spines );
   NS_USE_VARIABLE( dir_name );
   }


NS_PRIVATE void  _ns_spines_make_images( NsSpines *spines )
   {
NS_USE_VARIABLE( spines );
/*
   nslistiter      curr;
   nslistiter      end;
   NsSpine        *spine;
   NsPoint3i       bbox_min;
   NsPoint3i       bbox_max;
   NsPixelRegion   region;
   nsenum          project_type;
   NsError         error;


   ns_progress_set_title( spines->progress, "Creating spine gallery..." );
   ns_progress_num_iters( spines->progress, ns_list_size( &spines->list ) );
   ns_progress_begin( spines->progress );

   project_type = NS_PIXEL_PROC_ORTHO_PROJECT_MAX;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      if( ns_progress_cancelled( spines->progress ) )
         return;

      spine = ns_list_iter_get_object( curr );

		if( spine->is_analyzed )
			{
			ns_progress_next_iter( spines->progress );
			continue;
			}

      ns_to_image_space( &spine->bbox_min, &bbox_min, spines->voxel_info );
      ns_to_image_space( &spine->bbox_max, &bbox_max, spines->voxel_info );

      ns_assert( bbox_min.x <= bbox_max.x );
      ns_assert( bbox_min.y <= bbox_max.y );
      ns_assert( bbox_min.z <= bbox_max.z );

      region.x      = bbox_min.x;
      region.y      = bbox_min.y;
      region.z      = bbox_min.z;
      region.width  = ( nssize )( bbox_max.x - bbox_min.x ) + 1;
      region.height = ( nssize )( bbox_max.y - bbox_min.y ) + 1;
      region.length = ( nssize )( bbox_max.z - bbox_min.z ) + 1;

      ns_assert( region.x < ns_image_width( spines->image ) );
      ns_assert( region.y < ns_image_height( spines->image ) );
      ns_assert( region.z < ns_image_length( spines->image ) );
      ns_assert( region.x + region.width  <= ns_image_width( spines->image ) );
      ns_assert( region.y + region.height <= ns_image_height( spines->image ) );
      ns_assert( region.z + region.length <= ns_image_length( spines->image ) );

      if( 0 < region.width && 0 < region.height && 0 < region.length )
         if( NS_SUCCESS( ns_image_assign( &spine->image, spines->image, &region, NULL ), error ) )
            if( NS_SUCCESS(
                  ns_image_ortho_project_create(
                     &spine->image,
                     NULL,
                     1, &spine->proj_xy,
                     1, &spine->proj_zy,
                     1, &spine->proj_xz,
                     project_type
                     ),
                  error ) )
               {
               ns_image_ortho_project_init( &spine->proj_xy, &spine->proj_zy, &spine->proj_xz, project_type );

               ns_image_ortho_project(
                  &spine->image,
                  NULL,
                  NULL,
                  0,
                  ns_image_row_align( &spine->proj_xy ),
                  &spine->proj_xy,
                  ns_image_row_align( &spine->proj_zy ),
                  &spine->proj_zy,
                  ns_image_row_align( &spine->proj_xz ),
                  &spine->proj_xz,
                  project_type,
                  &spine->average_intensity,
                  &spine->max_intensity,
                  NULL
                  );

               /*TEMP*///ns_println( "G" );
               //_ns_spine_write_projections( spine, "E:\\usr\\douglas\\projects\\ns\\gallery\\" );
/*
               }

      ns_progress_next_iter( spines->progress );
      }

   //_ns_spines_write_montages( spines, "E:\\usr\\douglas\\projects\\ns\\gallery\\" );

   ns_progress_end( spines->progress );*/
   }


NS_PRIVATE void _ns_spines_set_fixed_size_aabbox( NsSpines *spines )
	{
	nslistiter   curr, end;
	NsSpine     *spine;
	NsVector3d   C;


   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( ! spine->is_analyzed )
			ns_model_position_and_radius_to_aabbox_ex(
				ns_vector3f_to_3d( &spine->center, &C ),
				NS_MODEL_SPINES_FIXED_RADIUS,
				&spine->bounding_box
				);
		}
	}


NS_PRIVATE void _ns_spines_clear_voxel_lists( NsSpines *spines )
	{
	nslistiter   curr, end;
	NsSpine     *spine;


   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		ns_list_clear( &spine->layers );
		ns_list_clear( &spine->voxels );

		spine->origin = NULL;
		}
	}






/*
NS_PRIVATE void _ns_spines_set_path_cluster( NsSet *path, nslong cluster, NsVoxel *sink )
	{
	nssetiter   curr, end;
	NsVoxel    *voxel;


	curr = ns_set_begin( path );
	end  = ns_set_end( path );

	for( ; ns_set_iter_not_equal( curr, end ); curr = ns_set_iter_next( curr ) )
		{
		voxel = ns_set_iter_get_object( curr );

		ns_assert( __NO_CLUSTER == __DATA( voxel, cluster ) );
		__DATA( voxel, cluster ) = cluster;
		}

	if( NULL != sink )
		__DATA( sink, is_sink ) = 1;
	}


NS_PRIVATE nsint _ns_voxel_compare_by_pointer( const NsVoxel *l, const NsVoxel *r )
	{
	if( l < r )
		return -1;
	else if( l > r )
		return 1;
	else
		return 0;
	}


NS_PRIVATE void _ns_spines_create_clusters_by_gradients( NsSpines *spines )
	{
	NsSet              path;
	NsVoxel           *voxel, *curr, *next;
	nslong             cluster;
	nsdouble           angle;
	NsVector3f         Cf, Nf;
	NsVector3i         Ci, Ni;
	nssize             i, num_clusters;
	nssize             connections;
	NsList            *clusters;
	nssize             src_cluster;
	nssize             dest_cluster;
	nsboolean          src_is_minima;
	nsboolean          dest_is_minima;
	nslistiter         scan;
	NsCompareBind2nd   pred;


	ns_set_construct( &path, _ns_voxel_compare_by_pointer, NULL );

	cluster = __NO_CLUSTER + 1;

   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
      if( ! __GET( voxel, __MINIMA ) && __NO_CLUSTER == __DATA( voxel, cluster ) )
			{
			curr = voxel;
			/*error*//*ns_set_insert( &path, curr, NULL );

			NS_INFINITE_LOOP
				{
				//if( NS_FLOAT_EQUAL( __DATA( curr, magnitude ), 0.0f ) )

				Ci = curr->position;
				ns_to_voxel_space( &Ci, &Cf, spines->voxel_info );

				ns_vector3f_add( &Nf, &Cf, &( __DATA( curr, gradient ) ) );
				ns_to_image_space( &Nf, &Ni, spines->voxel_info );

				next = ns_voxel_table_find( &spines->voxel_table, &Ni );

				/* Did the gradient path go out of bounds? *//*
				if( NULL == next )
					{
					/*TODO*//*ns_println( "GC:OUTOFBOUNDS" );
					break;
					}

				/* Was the gradient too "small" to advance? *//*
				if( next == curr )
					{
					/*TODO*//*ns_println( "GC:NEXT==CURR" );
					break;
					}

				//if( NS_FLOAT_EQUAL( __DATA( next, magnitude ), 0.0f ) )

				pred.second = next;
				pred.func   = _ns_voxel_compare_by_pointer;

				/* Did we go in a loop? *//*
				if( ns_set_exists( &path, &pred ) )
					{
					/*TODO*//*ns_println( "GC:LOOP" );
					break;
					}

				/* Did this path intersect an existing cluster? *//*
				if( __NO_CLUSTER != __DATA( next, cluster ) )
					{
					_ns_spines_set_path_cluster( &path, __DATA( next, cluster ), NULL );
					break;
					}

				angle = ns_vector3f_angle( &( __DATA( curr, gradient ) ), &( __DATA( next, gradient ) ) );
				angle = NS_RADIANS_TO_DEGREES( angle );

//#error todo experiment with angles grater than 90 degrees now that we can detect loops!

				/* Is the next voxel a "sink"? i.e. its gradient is pointing away or its a minima. *//*
				if( __GET( next, __MINIMA ) || angle >= 90.0 )
					{
					if( __NO_CLUSTER != __DATA( next, cluster ) )
						{
						_ns_spines_set_path_cluster( &path, __DATA( next, cluster ), next );
						break;
						}
					else
						{
						/*error*//*ns_set_insert( &path, next, NULL );
						_ns_spines_set_path_cluster( &path, cluster++, next );
						break;
						}
					}

				/*error*//*ns_set_insert( &path, next, NULL );
				curr = next;
				}

			ns_set_clear( &path );
			}

	ns_set_destruct( &path );

	num_clusters = ( nssize )cluster;
	ns_println( "# of clusters = %lu", num_clusters );

	/*error*//*clusters = ns_new_array( NsList, num_clusters );
	for( i = 0; i < num_clusters; ++i )
		ns_list_construct( clusters + i, NULL );

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		if( __NO_CLUSTER != __DATA( voxel, cluster ) )
			{
			ns_assert( ( nssize )__DATA( voxel, cluster ) < num_clusters );
			/*error*//*ns_list_push_back( clusters + ( nssize )__DATA( voxel, cluster ), voxel );
			}

	connections = 0;

   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, curr )
      if( __DATA( curr, is_sink ) )
			{
			ns_assert( __NO_CLUSTER != __DATA( curr, cluster ) );
			dest_cluster = ( nssize )__DATA( curr, cluster );
			ns_assert( dest_cluster < num_clusters );

			dest_is_minima = __GET( curr, __MINIMA );

         for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
            {
            ns_vector3i_add( &Ni, &curr->position, spines->voxel_offsets + i );

            if( NULL != ( next = ns_voxel_table_find( &spines->voxel_table, &Ni ) ) )
               if( __DATA( next, is_sink ) && ( __DATA( curr, cluster ) != __DATA( next, cluster ) ) )
						{
						ns_assert( __NO_CLUSTER != __DATA( next, cluster ) );
						src_cluster = __DATA( next, cluster );
						ns_assert( src_cluster < num_clusters );

						src_is_minima = __GET( next, __MINIMA );

						if( ( dest_is_minima && src_is_minima ) || ( ! dest_is_minima && ! src_is_minima ) )
							{
							/* Reset cluster numbers for the source cluster. *//*
							NS_LIST_FOREACH( clusters + src_cluster, scan )
								{
								voxel = ns_list_iter_get_object( scan );
								__DATA( voxel, cluster ) = ( nslong )dest_cluster;
								}

							ns_list_append( clusters + dest_cluster, clusters + src_cluster );
							++connections;
							}
						}
            }
			}

	ns_println( "# of connections = %lu", connections );

	for( i = 0; i < num_clusters; ++i )
		ns_list_destruct( clusters + i );
	ns_delete( clusters );
	}
*/





nsboolean _ns_spines_aabbox_octree_node_intersect_func
   (
   nsconstpointer     object,
   const NsAABBox3d  *box
   )
	{  return ns_aabbox3d_intersects_aabbox( object, box );  }


NS_PRIVATE NsError _ns_spines_establish_thresholds_and_contrast( NsSpines *spines )
	{
	nsint                image_width, image_height, image_length;
   NsVector3i           max;
   NsVector3f           Pf;
	NsVector3i           Pi;
   nsfloat              radius;
   nsmodelvertex        curr_vertex, end_vertices;
	nsvectoriter         curr_node, end_nodes;
	nsvectoriter         curr_object, end_objects;
   NsVector3i           C1i, C2i;
	NsVector3i           Vi;
	NsVector3f           Vf;
   nssize               sample_width, sample_height, sample_length;
   nssize               num_values;
	nssize               count;
   nsfloat             *values;
   nssize               bytes;
   ThresholdStruct      s;
   NsByteArray          buffer;
	NsVector             nodes;
	NsAABBox3d           roi_box;
	NsAABBox3d           vertex_box;
	const NsOctreeNode  *node;
	nsmodeledge          edge;
	nsfloat              distance;
	NsVoxel             *voxel;
	NsModelMask          mask;
	NsError              error;


	error = ns_no_error();

   if( NS_FAILURE( _ns_spines_build_normal_model_octree( spines ), error ) )
      return error;

   ns_progress_set_title( spines->progress, "Calculating vertex spine thresholds..." );
   ns_progress_num_iters( spines->progress, ns_model_num_vertices( spines->model ) );
   ns_progress_begin( spines->progress );

	ns_model_mask_construct( &mask );

	if( NS_FAILURE( ns_model_mask_create( &mask, spines->model, spines->image, spines->settings ), error ) )
		goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;

	ns_model_voxel_bounds_to_aabbox_ex( &spines->roi->C1, &spines->roi->C2, spines->voxel_info, &roi_box );

	ns_voxel_table_clear( &spines->voxel_table );

   ns_vector3i(
      &max,
      ns_voxel_buffer_width( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_height( &spines->voxel_buffer ) - 1,
      ns_voxel_buffer_length( &spines->voxel_buffer ) - 1
      );

	image_width  = ( nsint )ns_image_width( spines->image );
	image_height = ( nsint )ns_image_height( spines->image );
	image_length = ( nsint )ns_image_length( spines->image );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer, 0 ), error ) );
	ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

   curr_vertex  = ns_model_begin_vertices( spines->model );
   end_vertices = ns_model_end_vertices( spines->model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
		if( ns_progress_cancelled( spines->progress ) )
			goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;

      if( ns_settings_get_threshold_use_fixed( spines->settings ) )
			{
         ns_model_vertex_set_threshold(
            curr_vertex,
            NS_MODEL_VERTEX_SPINE_THRESHOLD,
            ns_settings_get_threshold_fixed_value( spines->settings )
            );

			ns_model_vertex_set_contrast(
				curr_vertex,
				NS_MODEL_VERTEX_SPINE_CONTRAST,
				ns_settings_get_threshold_fixed_value( spines->settings )
				);
			}
      else
         {
         ns_model_vertex_get_position( curr_vertex, &Pf );
			ns_to_image_space( &Pf, &Pi, spines->voxel_info );

			/* Check for out of bounds vertex. */
			if( Pi.x < 0 || Pi.x >= image_width  ||
				 Pi.y < 0 || Pi.y >= image_height ||
				 Pi.z < 0 || Pi.z >= image_length )
				{
				/* Set the threshold so "high" that no voxel intensity would ever be greater. */
				ns_model_vertex_set_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD, NS_FLOAT_MAX );

				/* Set the contrast really low? */
				ns_model_vertex_set_contrast( curr_vertex, NS_MODEL_VERTEX_SPINE_CONTRAST, 0.0f );

				ns_progress_next_iter( spines->progress );
				continue;
				}

         radius = ns_model_vertex_get_radius( curr_vertex );

			ns_grafting_calc_aabbox(
            spines->voxel_info,
            radius,
            &Pf,
            &max,
            spines->use_2d_sampling,
				spines->aabbox_scalar,
				spines->min_window,
            &C1i,
            &C2i
            );

			ns_model_voxel_bounds_to_aabbox_ex( &C1i, &C2i, spines->voxel_info, &vertex_box );

			if( ! ns_aabbox3d_intersects_aabbox( &vertex_box, &roi_box ) )
				{
				ns_progress_next_iter( spines->progress );
				continue;
				}

         sample_width  = ( nssize )( C2i.x - C1i.x ) + 1;
         sample_height = ( nssize )( C2i.y - C1i.y ) + 1;
         sample_length = ( nssize )( C2i.z - C1i.z ) + 1;

         num_values = sample_width * sample_height * sample_length;

         ns_assert( 0 < num_values );
         bytes = num_values * sizeof( nsfloat );

         if( ns_byte_array_size( &buffer ) < bytes )
            if( NS_FAILURE( ns_byte_array_resize( &buffer, bytes ), error ) )
               goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;

         values = ( nsfloat* )ns_byte_array_begin( &buffer );
			
			count = 0;

			NS_PROFILER_BEGIN( "test_voxels" );

			for( Vi.z = C1i.z; Vi.z <= C2i.z; ++Vi.z )
				for( Vi.y = C1i.y; Vi.y <= C2i.y; ++Vi.y )
					for( Vi.x = C1i.x; Vi.x <= C2i.x; ++Vi.x )
						{
						#define _NS_SPINE_VOXEL_OUTSIDE_MODEL( mask, V )\
							ns_model_mask_at( (mask), (V) )

						/* Check projection renderings (i.e. model mask) first */
						if( spines->optimize_isodata &&
							 _NS_SPINE_VOXEL_OUTSIDE_MODEL( &mask, &Vi ) )
							{
							values[ count ] = ( nsfloat )ns_voxel_get( &spines->voxel_buffer, Vi.x, Vi.y, Vi.z );
							++count;
							}
						else
							{
							if( NULL == ( voxel = ns_voxel_table_find( &spines->voxel_table, &Vi ) ) )
								{
								ns_to_voxel_space( &Vi, &Vf, spines->voxel_info );

								if( NS_FAILURE( ns_vector_resize( &nodes, 0 ), error ) )
									goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;

								/* Now put this voxel into the octree. */
								if( NS_FAILURE(
										ns_model_octree_intersections(
											spines->model,
											NS_MODEL_EDGE_OCTREE,
											&Vf,
											_ns_spine_voxel_octree_func_special,
											&nodes
											),
										error ) )
									goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;

								distance = NS_FLOAT_MAX;

								curr_node = ns_vector_begin( &nodes );
								end_nodes = ns_vector_end( &nodes );

								for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
										 curr_node = ns_vector_iter_next( curr_node ) )
									{
									node = ns_vector_iter_get_object( curr_node );

									curr_object = ns_octree_node_begin_objects( node );
									end_objects = ns_octree_node_end_objects( node );

									for( ; ns_vector_iter_not_equal( curr_object, end_objects );
											 curr_object = ns_vector_iter_next( curr_object ) )
										{
										edge     = ( nsmodeledge )ns_vector_iter_get_object( curr_object );
										distance = _ns_spines_distance_to_conical_frustum( spines, &Vf, edge, NULL, NULL, NULL, NULL, NULL );

										if( distance < 0.0f )
											goto _NS_SPINE_ESTABLISH_THRESHOLDS_AND_CONTRAST_ADD;
										}
									}

								_NS_SPINE_ESTABLISH_THRESHOLDS_AND_CONTRAST_ADD:

								/* NOTE: Using the intensity field of the voxel to hold the distance. */
								if( NS_FAILURE( ns_voxel_table_add(
														&spines->voxel_table,
														&Vi,
														distance,
														NULL,
														NULL
														),
														error ) )
									goto _NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT;
								}
							else
								distance = voxel->intensity;

							if( 0.0f <= distance )
								{
								values[ count ] = ( nsfloat )ns_voxel_get( &spines->voxel_buffer, Vi.x, Vi.y, Vi.z );
								++count;
								}
							}
						}

			ns_assert( count <= num_values );

         ComputeThreshold(
            values,
            /*num_values,*/( nsint )count,
            &s,
            ns_voxel_buffer_dynamic_range( &spines->voxel_buffer ),
				spines->flatten_isodata
            );

			ns_model_vertex_set_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD, s.threshold );
			ns_model_vertex_set_contrast( curr_vertex, NS_MODEL_VERTEX_SPINE_CONTRAST, s.contrast );
         }

		ns_progress_next_iter( spines->progress );
		}

	_NS_SPINES_ESTABLISH_THRESHOLDS_AND_CONTRAST_EXIT:

	ns_progress_end( spines->progress );

ns_println( "\n# of voxels in the table = " NS_FMT_ULONG, ns_voxel_table_size( &spines->voxel_table ) );

	ns_voxel_table_clear( &spines->voxel_table );

   ns_byte_array_destruct( &buffer );
	ns_vector_destruct( &nodes );

	ns_model_mask_destruct( &mask );

   return error;
	}


NS_PRIVATE void _ns_spines_adjust_thresholds_and_contrast( NsSpines *spines )
	{
   nsmodelvertex  curr_vertex;
	nsmodelvertex  end_vertices;
	//nsdouble       count;
	nsdouble       T1, T2;
	//nsdouble       sum;
	//nsdouble       avg;
	//nsdouble       std_dev;
	nsdouble       min_drop;


	ns_assert( 1 == ns_model_get_thresh_count( spines->model ) );

	//count = ( nsdouble )ns_model_num_vertices( spines->model );
	//ns_assert( 0.0 < count );

	/* Compute the average of the ratio of the spine to neurite threshold.
		i.e. spine-threshold / neurite-threshold. These ratios should for
		the most part be less than one since the spine-threshold comes out
		lower (darker). */
/*
	sum = 0.0;

   curr_vertex  = ns_model_begin_vertices( spines->model );
   end_vertices = ns_model_end_vertices( spines->model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		T1 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD );
		T2 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD );

		sum += ( T2 / T1 );
		}

	avg = sum / count;
/*TEMP*//*ns_println( "AVG( T2/T1 ) = " NS_FMT_DOUBLE, avg );*/

	/* Compute the standard deviation of the above ratio. */
/*
	sum = 0.0;

   curr_vertex  = ns_model_begin_vertices( spines->model );
   end_vertices = ns_model_end_vertices( spines->model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		T1 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD );
		T2 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD );

		sum += NS_POW2( ( T2 / T1 ) - avg );
		}

	std_dev = ns_sqrt( sum / count );
/*TEMP*//*ns_println( "STD-DEV( T2/T1 ) = " NS_FMT_DOUBLE, std_dev );*/

	//min_drop = avg /*- 1.0 * std_dev*/;
/*TEMP*///ns_println( "MIN-DROP = " NS_FMT_DOUBLE, min_drop );

	/* Do we need this? Make sure that any vertices spine threshold is
		always at least some minimum percentage of its neurite threshold. */
	//if( min_drop < 0.75 )
		min_drop = 0.75;

	/* Assure that 'min_drop' is always less than one (which it should be)
		but lets be paranoid. */
	//if( min_drop > 1.0 )
	//	min_drop = 1.0;

/*TEMP*/ns_println( "MIN-DROP (clamped) = " NS_FMT_DOUBLE, min_drop );

   curr_vertex  = ns_model_begin_vertices( spines->model );
   end_vertices = ns_model_end_vertices( spines->model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		T1 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD );
		T2 = ( nsdouble )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD );

		if( ( T2 / T1 ) < min_drop )
			{
			T2 = T1 * min_drop;

//ns_println( "RESET: %f to %f",
//	ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_SPINE_THRESHOLD ),
	//T2 );
			}

/*TEMP*///ns_println( "T1=" NS_FMT_DOUBLE " T2=" NS_FMT_DOUBLE, T1, T2 );

		/* The rest of the spine routines use the threshold stored at the
			index NS_MODEL_VERTEX_NEURITE_THRESHOLD, so we need to copy over
			the spine threshold. */
		ns_model_vertex_set_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD, ( nsfloat )T2 );
		}
	}


NS_PRIVATE void _ns_spines_set_analyzed_flag( NsSpines *spines )
	{
	nslistiter curr;

	NS_LIST_FOREACH( &spines->list, curr )
		( ( NsSpine* )ns_list_iter_get_object( curr ) )->is_analyzed = NS_TRUE;
	}


NS_PRIVATE NsError _ns_spine_clump_new( NsSpineClump **ret_clump, nsint id )
	{
	NsSpineClump *clump;

	ns_assert( NULL != ret_clump );

	if( NULL == ( clump = ns_new( NsSpineClump ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	clump->id             = id;
	clump->min_intensity  = NS_FLOAT_MAX;
	clump->max_intensity  = -NS_FLOAT_MAX;
	clump->min_dts        = NS_DOUBLE_MAX;
	clump->max_dts        = -NS_DOUBLE_MAX;
	//clump->mean_dts      = 0.0;
	clump->maximum        = NULL;
	clump->minimum        = NULL;
	clump->has_centroid   = NS_FALSE;
	clump->touches_clump0 = NS_FALSE;
	clump->origin         = NULL;

	ns_list_construct( &clump->voxels, NULL );
	ns_list_construct( &clump->seen, NULL );
	ns_list_construct( &clump->connections, NULL );

	clump->color = NS_SPINE_CLUMP_UNSEEN;

	*ret_clump = clump;
	return ns_no_error();
	}


#define _ns_spine_clump_num_voxels( clump )\
	ns_list_size( &(clump)->voxels )


#define _ns_spine_clump_is_empty( clump )\
	( 0 == _ns_spine_clump_num_voxels( (clump) ) )


NS_PRIVATE NsError _____ns_spine_clump_add_voxel( NsSpineClump *clump, NsVoxel *voxel, NsSpines *spines, const nschar *file, nsint line )
	{
	//nsdouble  num_voxels;
	NsError   error;


	ns_assert( NULL != clump );
	ns_assert( NULL != voxel );

	NS_USE_VARIABLE( spines );

	
//ns_assert( NULL == __DATA( voxel, clump ) );
if( NULL != __DATA( voxel, clump ) )
ns_println( "DUP VOXEL at %s::%d", file, line );


	if( NS_FAILURE( ns_list_push_back( &clump->voxels, voxel ), error ) )
		return error;

	__DATA( voxel, clump ) = clump;

	if( voxel->intensity < clump->min_intensity )
		clump->min_intensity = voxel->intensity;

	if( clump->max_intensity < voxel->intensity )
		clump->max_intensity = voxel->intensity;

	if( __DATA( voxel, distance ) < clump->min_dts )
		{
		clump->min_dts = __DATA( voxel, distance );
		clump->minimum = voxel;
		}

	if( clump->max_dts < __DATA( voxel, distance ) )
		{
		clump->max_dts = __DATA( voxel, distance );
		clump->maximum = voxel;
		}

	//num_voxels = ( nsdouble )_ns_spine_clump_num_voxels( clump );

	//clump->mean_dts = ( clump->mean_dts * num_voxels + ( nsdouble )__DATA( voxel, distance ) ) / ( num_voxels + 1.0 );

	return ns_no_error();
	}


#define _ns_spine_clump_add_voxel( clump, voxel, spines )\
	_____ns_spine_clump_add_voxel( (clump), (voxel), (spines), __FILE__, __LINE__ )


NS_PRIVATE NsError _ns_spines_merge_clumps( NsSpines *spines, NsSpineClump *dest, NsSpineClump *src )
	{
	NsVoxel     *voxel;
	nslistiter   iter;
	NsError      error;


	ns_assert( NULL != spines );
	ns_assert( NULL != dest );
	ns_assert( NULL != src );
	ns_assert( src != dest );

	NS_LIST_FOREACH( &src->voxels, iter )
		{
		voxel = ns_list_iter_get_object( iter );
		__DATA( voxel, clump ) = NULL;

		if( NS_FAILURE( _ns_spine_clump_add_voxel( dest, voxel, spines ), error ) )
			return error;
		}

	ns_list_clear( &src->voxels );
	src->maximum = NULL;

	if( src->touches_clump0 )
		dest->touches_clump0 = NS_TRUE;

	return ns_no_error();
	}


NS_PRIVATE nssize _ns_spines_count_clump_voxels( const NsSpines *spines )
	{
	const NsSpineClump  *clump;
	nslistiter           iter;
	nssize               count;


	count = 0;

	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump  = ns_list_iter_get_object( iter );
		count += _ns_spine_clump_num_voxels( clump );
		}

	return count;
	}


NS_PRIVATE nssize _ns_spines_num_unempty_clumps( const NsSpines *spines )
	{
	const NsSpineClump  *clump;
	nslistiter           iter;
	nssize               count;


	count = 0;

	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );

		if( ! _ns_spine_clump_is_empty( clump ) )
			++count;
		}

	return count;
	}


/*
NS_PRIVATE void _ns_spines_clear_clump_voxels( NsSpines *spines )
	{
	NsSpineClump  *clump;
	nslistiter     iter;


	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );
		ns_list_clear( &clump->voxels );
		}
	}
*/


NS_PRIVATE void _ns_spines_nullify_clump_voxel_pointers( NsSpines *spines )
	{
	NsSpineClump  *clump;
	NsVoxel       *voxel;
	nslistiter     c, v;


	NS_LIST_FOREACH( &spines->clumps, c )
		{
		clump = ns_list_iter_get_object( c );

		NS_LIST_FOREACH( &clump->voxels, v )
			{
			voxel = ns_list_iter_get_object( v );
			__DATA( voxel, clump ) = NULL;
			}
		}
	}


NS_PRIVATE void _ns_spines_delete_empty_clumps( NsSpines *spines )
	{
	NsSpineClump  *clump;
	nslistiter     curr, next, end;


	curr = ns_list_begin( &spines->clumps );
	end  = ns_list_end( &spines->clumps );

	while( ns_list_iter_not_equal( curr, end ) )
		{
		next  = ns_list_iter_next( curr );
		clump = ns_list_iter_get_object( curr );

		if( _ns_spine_clump_is_empty( clump ) )
			ns_list_erase( &spines->clumps, curr );

		curr = next;
		}
	}


nslistiter ns_model_begin_clumps( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ns_list_begin( &(model->spines->clumps) );
	}


nslistiter ns_model_end_clumps( const NsModel *model )
	{
	ns_assert( NULL != model );
	return ns_list_end( &(model->spines->clumps) );
	}


NS_PRIVATE NsError _ns_spines_compute_and_save_clump_centroids( NsSpines *spines )
	{
	nslistiter           c, v;
	const NsSpineClump  *clump;
	const NsVoxel       *first, *voxel;
	NsVector3f           V, S;
	nssize               n;
	NsSpinePoint        *point;
	NsError              error;


	NS_LIST_FOREACH( &spines->clumps, c )
		{
		clump = ns_list_iter_get_object( c );

		if( ! ns_list_is_empty( &clump->voxels ) )
			{
			ns_vector3f_zero( &S );
			n = 0;

			first = ns_list_iter_get_object( ns_list_begin( &clump->voxels ) );

			NS_LIST_FOREACH( &clump->voxels, v )
				{
				voxel = ns_list_iter_get_object( v );

				if( NS_FLOAT_EQUAL( voxel->intensity, first->intensity ) )
					{
					ns_to_voxel_space( &voxel->position, &V, spines->voxel_info );
					ns_vector3f_cmpd_add( &S, &V );
					++n;
					}
				}

			ns_assert( 0 < n );
			ns_vector3f_cmpd_scale( &S, 1.0f / ( nsfloat )n );

			if( NS_FAILURE( _ns_spine_point_new( &point, &S ), error ) )
				return error;

			if( NS_FAILURE( ns_list_push_back( &spines->centroids, point ), error ) )
				{
				_ns_spine_point_delete( point );
				return error;
				}

			/*TEMP*///ns_println( "%f, %f, %f", point->P.x, point->P.y, point->P.z );
			}
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_spines_compute_and_save_clump_borderers( NsSpines *spines )
	{
	NsVoxel       *voxel, *neighbor, N;
	nsint          o;
	NsVector3f     P;
	NsSpinePoint  *point;
	NsError        error;


	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		if( NULL != __DATA( voxel, clump ) && 0 != __DATA( voxel, clump )->id )
			for( o = NS_VOXEL_NUM_OFFSETS - 1; 0 <= o; --o )
				{
				ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + o );

				if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) &&
					 NULL != __DATA( neighbor, clump ) &&
					 0 != __DATA( neighbor, clump )->id &&
					 __DATA( neighbor, clump ) != __DATA( voxel, clump ) )
					{
					ns_to_voxel_space( &neighbor->position, &P, spines->voxel_info );

					if( NS_FAILURE( _ns_spine_point_new( &point, &P ), error ) )
						return error;

					if( NS_FAILURE( ns_list_push_back( &spines->borderers, point ), error ) )
						{
						_ns_spine_point_delete( point );
						return error;
						}
					}
				}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_spines_do_execute_clump_mergers( NsSpines *spines, NsSpineClump *sink, NsSpineClump *clump )
	{
	nslistiter  iter;
	NsError     error;


	if( NS_SPINE_CLUMP_UNSEEN != clump->color )
		return ns_no_error();

	clump->color = NS_SPINE_CLUMP_SEEN;

	if( clump != sink )
		if( NS_FAILURE( _ns_spines_merge_clumps( spines, sink, clump ), error ) )
			return error;

	NS_LIST_FOREACH( &clump->connections, iter )
		if( NS_FAILURE( _ns_spines_do_execute_clump_mergers( spines, sink, ns_list_iter_get_object( iter ) ), error ) )
			return error;

	return ns_no_error();
	}


#include "nsmodel-spines-merge.inl"
#include "nsmodel-spines-clumps.inl"

/*
NS_PRIVATE NsError _ns_spines_recreate_maxima_list( NsSpines *spines )
   {
	nslistiter     iter;
	NsSpineClump  *clump;
   NsError        error;


//_ns_spines_clear_maxima_flags( spines );
//ns_list_clear( &spines->maxima );

	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );

		/* Add the clump voxel with the maximum distance to surface, except those that
			would attach to an external vertex in the model. *//*
		if( NULL != clump->maximum && ( ! __GET( clump->maximum, __TIP_ATTACH ) ) )
			if( NS_FAILURE( _ns_spines_add_maxima_to_list( spines, clump->maximum ), error ) )
				return error;
		}

   spines->num_maxima = ns_list_size( &spines->maxima );

   ns_list_sort( &spines->maxima, _ns_spines_voxel_dts_greater_func );

/*TEMP*//*ns_println( "# of maxima = " NS_FMT_ULONG, ns_list_size( &spines->maxima ) );

   return ns_no_error();
   }*/


#include "nsmodel-spines-axis.inl"


/*
NS_PRIVATE void _ns_spines_init_clump_ids( NsSpines *spines, nsint clump_id )
	{
	NsVoxel *voxel;

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		__DATA( voxel, clump )    = NULL;
		__DATA( voxel, clump_id ) = clump_id;//__DATA( voxel, distance ) >= 0.0 ? clump_id : 0;
		}
	}
*/


//NS_PRIVATE void _ns_spines_reset_clump_ids_based_on_dts( NsSpines *spines )
//	{
//	NsSpineClump  *clump;
//	nslistiter     iter;
//	NsVoxel       *voxel;
//
//
//	/* Initalize all the clumps 'touches_model' to false. */
//	NS_LIST_FOREACH( &spines->clumps, iter )
//		{
//		clump = ns_list_iter_get_object( iter );
//		clump->touches_model = NS_FALSE;
//		}
//
//	/* Go through each voxel and see if it has a DTS <= 0. If so mark the clump
//		that its in (if it is in one) as touching the model. */
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		if( NULL != __DATA( voxel, clump ) )
//			if( __DATA( voxel, distance ) <= 0.0f )
//				__DATA( voxel, clump )->touches_model = NS_TRUE;
//
//	/* Go through the voxels again. If its part of a clump that touches the model
//		then reset its clump_id to 0, else set it to 2! Also clear its clump pointer
//		while we're in this step. */
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
//		if( NULL != __DATA( voxel, clump ) )
//			__DATA( voxel, clump_id ) = ( __DATA( voxel, clump )->touches_model ) ? 0 : 2;
//		else
//			__DATA( voxel, clump_id ) = 0;
//
//		__DATA( voxel, clump ) = NULL;
//		}
//
//	/* Clear all the clump structures. */
//	ns_list_clear( &spines->clumps );
//	}


//NS_PRIVATE void _ns_spines_put_maxima_on_hold( NsSpines *spines )
//	{
//	nslistiter      first, curr, end;
//	nssize          count;
//	NsSpineMaxima  *M;
//
//
//	/* Clear the maxima flag for the voxels. */
//
//	curr = ns_list_begin( &spines->maxima );
//	end  = ns_list_end( &spines->maxima );
//
//	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
//		{
//		M = ns_list_iter_get_object( curr );
//		__CLEAR( M->voxel, __MAXIMA );
//		}
//
//	first = ns_list_begin( &spines->maxima );
//	count = ns_list_size( &spines->maxima );
//
//	if( 0 < count )
//		{
//		ns_list_splice_out( &spines->maxima, first, count );
//		ns_assert( ns_list_is_empty( &spines->maxima ) );
//
//		ns_assert( ns_list_is_empty( &spines->hold_maxima ) );
//		ns_list_splice_in_back( &spines->hold_maxima, first, count );
//		}
//
///*TEMP*/ns_println( "Put " NS_FMT_ULONG " maxima on hold.", ns_list_size( &spines->hold_maxima ) );
//	}


//NS_PRIVATE void _ns_spines_restore_maxima_on_hold( NsSpines *spines )
//	{
//	nslistiter      first, curr, end;
//	nssize          count;
//	NsSpineMaxima  *M;
//
//
//	ns_list_clear( &spines->maxima );
//
//	first = ns_list_begin( &spines->hold_maxima );
//	count = ns_list_size( &spines->hold_maxima );
//
//	if( 0 < count )
//		{
//		ns_list_splice_out( &spines->hold_maxima, first, count );
//		ns_assert( ns_list_is_empty( &spines->hold_maxima ) );
//
//		ns_assert( ns_list_is_empty( &spines->maxima ) );
//		ns_list_splice_in_back( &spines->maxima, first, count );
//		}
//
//	/* Reset the maxima flag for the voxels. */
//
//	curr = ns_list_begin( &spines->maxima );
//	end  = ns_list_end( &spines->maxima );
//
//	for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
//		{
//		M = ns_list_iter_get_object( curr );
//		__SET( M->voxel, __MAXIMA );
//		}
//
///*TEMP*/ns_println( "Restored " NS_FMT_ULONG " maxima.", ns_list_size( &spines->maxima ) );
//	}


//NS_PRIVATE NsError _ns_spines_create_maxima_for_dendrite_clump( NsSpines *spines )
//	{
  // NsVoxel   N;
   //NsVoxel  *voxel;
//   NsVoxel  *neighbor;
//   nssize    i;
	//nsint     length;
	//NsError   error;
//
//
	//length = ( nsint )ns_image_length( spines->image );

//	/* NOTE: Dont allow any maxima that are on the edges (in Z).
//		Have to allow for 2D spine detection though. */
//   NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
  //    if( ( ! __GET( voxel, __MINIMA ) ) && ( 0 == __DATA( voxel, clump_id ) ) )
//			if( 1 == length || ( 0 < voxel->position.z && voxel->position.z < length - 1 ) )
//				{
//				/* Dont check any voxels that are on the outer edges of the image. */
//				__SET( voxel, __MAXIMA );
//
//				/* Find which voxels are maxima, i.e. have no neighbors closer
//					then themselves to the model. */
//				for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
//					{
//					ns_vector3i_add( &N.position, &voxel->position, spines->voxel_offsets + i );
//
//					if( NULL != ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
//						if( 0 == __DATA( neighbor, clump_id ) )
//							if( __DATA( voxel, distance ) < __DATA( neighbor, distance ) )
//								{
//								__CLEAR( voxel, __MAXIMA );
//								break;
//								}
//					}
//				}
//		}
//
//	ns_assert( ns_list_is_empty( &spines->maxima ) );
//
  // /* Clear maxima that attach to an external vertex,
    //  i.e. a tip of a branch since these have too many
      //artifcats and are probably not spines anyway. */
   //NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
     // {
      //if( __GET( voxel, __MAXIMA ) )
        // if( __GET( voxel, __TIP_ATTACH ) )
          //  __CLEAR( voxel, __MAXIMA );

//		if( __GET( voxel, __MAXIMA ) )
//			{
//			if( NS_FAILURE( _ns_spines_add_maxima_to_list( spines, voxel ), error ) )
//				return error;
//			}
  //    }
//
//	return ns_no_error();
//	}


//NS_PRIVATE void _ns_spines_reset_clump_ids_to_zero_or_one( NsSpines *spines )
//	{
//	NsVoxel *voxel;
//
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
//		if( 0 < __DATA( voxel, clump_id ) )
//			__DATA( voxel, clump_id ) = 1;
//		}
//	}


NS_PRIVATE void _ns_spines_smooth_out_vertex_and_edge_thresholds_and_contrasts( NsSpines *spines )
	{
	nsmodelvertex  curr_vertex, end_vertices;
	nsmodeledge    curr_edge, end_edges;


	curr_vertex  = ns_model_begin_vertices( spines->model );
	end_vertices = ns_model_end_vertices( spines->model );

	for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		ns_model_vertex_set_threshold(
			curr_vertex,
			NS_MODEL_VERTEX_AVERAGE_THRESHOLD,
			ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD )
			);

		ns_model_vertex_set_contrast(
			curr_vertex,
			NS_MODEL_VERTEX_AVERAGE_CONTRAST,
			ns_model_vertex_get_contrast( curr_vertex, NS_MODEL_VERTEX_NEURITE_CONTRAST )
			);

		curr_edge = ns_model_vertex_begin_edges( curr_vertex );
		end_edges = ns_model_vertex_end_edges( curr_vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			{
			ns_model_edge_set_threshold(
				curr_edge,
				NS_MODEL_EDGE_AVERAGE_THRESHOLD,
				ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD )
					+
				ns_model_vertex_get_threshold( ns_model_edge_src_vertex( curr_edge ), NS_MODEL_VERTEX_NEURITE_THRESHOLD )
					+
				ns_model_vertex_get_threshold( ns_model_edge_dest_vertex( curr_edge ), NS_MODEL_VERTEX_NEURITE_THRESHOLD ),
				NS_FALSE
				);

			ns_model_edge_set_contrast(
				curr_edge,
				NS_MODEL_EDGE_AVERAGE_CONTRAST,
				ns_model_edge_get_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST )
					+
				ns_model_vertex_get_contrast( ns_model_edge_src_vertex( curr_edge ), NS_MODEL_VERTEX_NEURITE_CONTRAST )
					+
				ns_model_vertex_get_contrast( ns_model_edge_dest_vertex( curr_edge ), NS_MODEL_VERTEX_NEURITE_CONTRAST ),
				NS_FALSE
				);

			ns_model_vertex_set_threshold(
				curr_vertex,
				NS_MODEL_VERTEX_AVERAGE_THRESHOLD,
				ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_AVERAGE_THRESHOLD )
					+
				ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD )
				);

			ns_model_vertex_set_contrast(
				curr_vertex,
				NS_MODEL_VERTEX_AVERAGE_CONTRAST,
				ns_model_vertex_get_contrast( curr_vertex, NS_MODEL_VERTEX_AVERAGE_CONTRAST )
					+
				ns_model_edge_get_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST )
				);
			}
		}

	curr_vertex  = ns_model_begin_vertices( spines->model );
	end_vertices = ns_model_end_vertices( spines->model );

	for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
		{
		/* NOTE: Each vertex threshold is summed with its N edges threshold, thats
			why we divide by "No. Of Edges" + 1. */

//ns_print( "V: %f,%f to ",
//	ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD ),
//	ns_model_vertex_get_contrast( curr_vertex, NS_MODEL_VERTEX_NEURITE_CONTRAST )
//	);

		ns_model_vertex_set_threshold(
			curr_vertex,
			NS_MODEL_VERTEX_NEURITE_THRESHOLD,
			ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_AVERAGE_THRESHOLD )
				/
			( nsfloat )( ns_model_vertex_num_edges( curr_vertex ) + 1 )
			);

		ns_model_vertex_set_contrast(
			curr_vertex,
			NS_MODEL_VERTEX_NEURITE_CONTRAST,
			ns_model_vertex_get_contrast( curr_vertex, NS_MODEL_VERTEX_AVERAGE_CONTRAST )
				/
			( nsfloat )( ns_model_vertex_num_edges( curr_vertex ) + 1 )
			);

//ns_println( "%f,%f",
//	ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD ),
//	ns_model_vertex_get_contrast( curr_vertex, NS_MODEL_VERTEX_NEURITE_CONTRAST )
//	);

		curr_edge = ns_model_vertex_begin_edges( curr_vertex );
		end_edges = ns_model_vertex_end_edges( curr_vertex );

		for( ; ns_model_edge_not_equal( curr_edge, end_edges );
				 curr_edge = ns_model_edge_next( curr_edge ) )
			{
			/* NOTE: Each edge threshold is summed with its two vertex endpoint
				thresholds, thats why we divide by 3. */

//ns_print( "E: %f,%f to ",
//	ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD ),
//	ns_model_edge_get_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST )
//	);

			ns_model_edge_set_threshold(
				curr_edge,
				NS_MODEL_EDGE_NEURITE_THRESHOLD,
				ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_AVERAGE_THRESHOLD ) / 3.0f,
				NS_FALSE
				);

			ns_model_edge_set_contrast(
				curr_edge,
				NS_MODEL_EDGE_NEURITE_CONTRAST,
				ns_model_edge_get_contrast( curr_edge, NS_MODEL_EDGE_AVERAGE_CONTRAST ) / 3.0f,
				NS_FALSE
				);

//ns_println( "%f,%f",
//	ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD ),
//	ns_model_edge_get_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST )
//	);
			}
		}
	}


NS_PRIVATE nssize _ns_spines_init_edge_thresholds_and_contrasts( NsSpines *spines )
	{
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
	nssize         num_edges;


   curr_vertex  = ns_model_begin_vertices( spines->model );
   end_vertices = ns_model_end_vertices( spines->model );
	num_edges    = 0;

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			{
			/* Set these to a value less than zero to indicate they havent
				been calculated yet. */
         ns_model_edge_set_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD, -1.0f, NS_FALSE );
			ns_model_edge_set_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST, -1.0f, NS_FALSE );

			++num_edges;
			}
      }

	/* Count and return the number of edges just to save having to do 
		it again in the later routine where the thresholds are actually
		computed. */
	return num_edges / 2;
	}


NS_PRIVATE NsError _ns_spines_calc_edge_thresholds_and_contrasts( NsSpines *spines )
	{
   const NsSettings   *settings;
   const NsImage      *image;
	const NsCubei      *roi;
   nsboolean           use_2d_sampling;
	nsfloat             aabbox_scalar;
	nsint               min_window;
   NsModel            *model;
   NsProgress         *progress;
	const NsVoxelInfo  *voxel_info;
   NsVoxelBuffer       voxel_buffer;
   NsByteArray         buffer;
   ThresholdStruct     s;
   nsmodelvertex       curr_vertex, end_vertices;
	nsmodelvertex       src_vertex, dest_vertex;
	nsmodeledge         curr_edge, end_edges;
	NsVector3f          V1, V2;
   NsVector3f          Pf;
	NsVector3i          Pi;
	nsfloat             r1, r2;
   nsfloat             radius;
	nsint               image_width, image_height, image_length;
   nssize              sample_width, sample_height, sample_length;
   nssize              num_values;
   nsfloat            *values;
   nssize              bytes;
   NsVector3i          C1, C2;
   NsVector3i          max;
	NsAABBox3d          roi_box;
	NsAABBox3d          edge_box;
	nssize              num_edges;
   NsError             error;


	settings        = spines->settings;
	image           = spines->image;
	roi             = spines->roi;
	use_2d_sampling = spines->use_2d_sampling;
	aabbox_scalar   = spines->aabbox_scalar;
	min_window      = spines->min_window;
	model           = spines->model;
	progress        = spines->progress;

	num_edges = _ns_spines_init_edge_thresholds_and_contrasts( spines );

   ns_progress_set_title( progress, "Establishing edge thresholds and contrasts..." );
   ns_progress_num_iters( progress, num_edges );

	voxel_info = ns_settings_voxel_info( settings );

	if( NULL != roi )
		ns_model_voxel_bounds_to_aabbox_ex( &roi->C1, &roi->C2, voxel_info, &roi_box );

   ns_voxel_buffer_init( &voxel_buffer, image );

   ns_vector3i(
      &max,
      ns_voxel_buffer_width( &voxel_buffer ) - 1,
      ns_voxel_buffer_height( &voxel_buffer ) - 1,
      ns_voxel_buffer_length( &voxel_buffer ) - 1
      );

	image_width  = ( nsint )ns_image_width( image );
	image_height = ( nsint )ns_image_height( image );
	image_length = ( nsint )ns_image_length( image );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer, 0 ), error ) );

   ns_progress_begin( progress );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
			 curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			{
			if( ns_model_edge_get_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD ) < 0.0f )
				{
				if( ns_settings_get_threshold_use_fixed( settings ) )
					{
					ns_model_edge_set_threshold(
						curr_edge,
						NS_MODEL_EDGE_NEURITE_THRESHOLD,
						ns_settings_get_threshold_fixed_value( settings ),
						NS_FALSE
						);

					ns_model_edge_set_contrast(
						curr_edge,
						NS_MODEL_EDGE_NEURITE_CONTRAST,
						ns_settings_get_threshold_fixed_value( settings ),
						NS_FALSE
						);
					}
				else
					{
					src_vertex  = ns_model_edge_src_vertex( curr_edge );
					dest_vertex = ns_model_edge_dest_vertex( curr_edge );

					ns_model_vertex_get_position( src_vertex, &V1 );
					ns_model_vertex_get_position( dest_vertex, &V2 );

					/* Compute the threshold at the midpoint of the edge. */
					ns_vector3f_add( &Pf, &V1, &V2 ); /* Pf = V1 + V2 */
					ns_vector3f_cmpd_scale( &Pf, 0.5f ); 

					ns_to_image_space( &Pf, &Pi, voxel_info );

					/* Check for out of bounds edge. */
					if( Pi.x < 0 || Pi.x >= image_width  ||
						 Pi.y < 0 || Pi.y >= image_height ||
						 Pi.z < 0 || Pi.z >= image_length )
						{
						/* Set the threshold so "high" that no voxel intensity would never be greater. */
						ns_model_edge_set_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD, NS_FLOAT_MAX, NS_TRUE );

						/* Set the contrast really low? */
						ns_model_edge_set_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST, 0.0f, NS_TRUE );

						ns_progress_next_iter( progress );
						continue;
						}

					/* The size of the sample around the edge is the average of the
						radius of the endpoints. */
					r1     = ns_model_vertex_get_radius( src_vertex );
					r2     = ns_model_vertex_get_radius( dest_vertex );
					radius = ( r1 + r2 ) * 0.5f;

					ns_grafting_calc_aabbox(
						voxel_info,
						radius,
						&Pf,
						&max,
						use_2d_sampling,
						aabbox_scalar,
						min_window,
						&C1,
						&C2
						);

					if( NULL != roi )
						{
						ns_model_voxel_bounds_to_aabbox_ex( &C1, &C2, voxel_info, &edge_box );

						if( ! ns_aabbox3d_intersects_aabbox( &edge_box, &roi_box ) )
							{
							ns_progress_next_iter( progress );
							continue;
							}
						}

					sample_width  = ( nssize )( C2.x - C1.x ) + 1;
					sample_height = ( nssize )( C2.y - C1.y ) + 1;
					sample_length = ( nssize )( C2.z - C1.z ) + 1;

					num_values = sample_width * sample_height * sample_length;

					ns_assert( 0 < num_values );
					bytes = num_values * sizeof( nsfloat );

					if( ns_byte_array_size( &buffer ) < bytes )
						if( NS_FAILURE( ns_byte_array_resize( &buffer, bytes ), error ) )
							{
							ns_byte_array_destruct( &buffer );
							return error;
							}

					values = ( nsfloat* )ns_byte_array_begin( &buffer );

					ns_grafting_sample_values( &voxel_buffer, values, num_values, &C1, &C2 );

					ComputeThreshold(
						values,
						( nsint )num_values,
						&s,
						ns_voxel_buffer_dynamic_range( &voxel_buffer ),
						NS_TRUE
						);

					ns_model_edge_set_threshold( curr_edge, NS_MODEL_EDGE_NEURITE_THRESHOLD, s.threshold, NS_TRUE );
					ns_model_edge_set_contrast( curr_edge, NS_MODEL_EDGE_NEURITE_CONTRAST, s.contrast, NS_TRUE );
					}

				ns_progress_next_iter( progress );
				}
			}
      }

   ns_progress_end( progress );

   ns_byte_array_destruct( &buffer );

	return ns_no_error();
	}


void _ns_spines_set_splats_colors( NsSpines *spines );


NS_PRIVATE nsboolean _ns_voxel_is_not_part_of_spine( const NsVoxel *voxel, nspointer user_data )
   {
	NS_USE_VARIABLE( user_data );
	return NULL == __DATA( voxel, spine );
	}


void _ns_spines_clear_non_spine_voxels( NsSpines *spines )
	{  ns_voxel_table_clean( &spines->voxel_table, _ns_voxel_is_not_part_of_spine, NULL );  }


void _ns_spines_set_spine_voxel_normals( NsSpines *spines )
	{
	NsVoxelTable      *voxel_table;
	NsVoxelBuffer     *voxel_buffer;
	NsVoxel           *voxel;
   nsfloat            corners[ NS_VOXEL_NUM_CORNER_OFFSETS ];
   nsfloat            normal[3];
   NsVector3i         N;
	const NsVector3i  *P;
   nssize             i;
	nsint              width, height, length;
	const NsVector3i  *voxel_corner_offsets;


	voxel_table  = &spines->voxel_table;
	voxel_buffer = &spines->voxel_buffer;
	width        = spines->width;
	height       = spines->height;
	length       = spines->length;

	voxel_corner_offsets = spines->voxel_corner_offsets;

	NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )
		{
		P = &voxel->position;

		for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
			{
			ns_vector3i_add( &N, P, voxel_corner_offsets + i );

			if( 0 <= N.x && N.x < width  &&
				 0 <= N.y && N.y < height &&
				 0 <= N.z && N.z < length   )
				corners[i] = ( nsfloat )ns_voxel_get( voxel_buffer, N.x, N.y, N.z );
			else
				corners[i] = 0.0f;
			}

		ComputeImageNormal( corners, normal );

		/* Reverse normal y and z since the data axis orientation requires
			a rotation to align it with OpenGL's axis orientation. */
		__DATA( voxel, N ).x = NS_NORMAL_FLOAT_TO_SIGNED_BYTE(  normal[0] );
		__DATA( voxel, N ).y = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[1] );
		__DATA( voxel, N ).z = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[2] );
		}
	}


//NS_PRIVATE void _ns_spines_init_voxel_clump_intensity_ids( NsSpines *spines )
//	{
//	NsVoxel *voxel;
//
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
//		__DATA( voxel, clump )  = NULL;
//		__DATA( voxel, ins_id ) = 1;
//		}
//	}


//NS_PRIVATE void _ns_spines_init_voxel_clump_dts_ids( NsSpines *spines )
//	{
//	NsVoxel *voxel;
//
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
//		__DATA( voxel, clump )  = NULL;
//		__DATA( voxel, dts_id ) = 1;
//		}
//	}


//NS_PRIVATE void _ns_spines_set_voxel_final_clump_ids( NsSpines *spines )
//	{
//	NsVoxel *voxel;
//
//	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
//		{
//		__DATA( voxel, clump )    = NULL;
//		__DATA( voxel, clump_id ) = ( __DATA( voxel, ins_id ) % 32 ) + ( ( __DATA( voxel, dts_id ) % 32 ) << 5 );
//		}
//	}



NS_PRIVATE void _ns_spines_init_voxel_clump_ids( NsSpines *spines, nsboolean init_dts_id )
	{
	NsVoxel *voxel;

	NS_VOXEL_TABLE_FOREACH( &spines->voxel_table, voxel )
		{
		__DATA( voxel, clump )    = NULL;
		__DATA( voxel, clump_id ) = 1;

		if( init_dts_id )
			__DATA( voxel, dts_id ) = 0;

		if( __DATA( voxel, distance ) <= 0.0f )
			__DATA( voxel, clump_id ) = 0;
		}
	}


NS_PRIVATE nsboolean _ns_spines_voxels_compare_desc_dts( const void *vp1, const void *vp2 )
	{
	const NsVoxel *V1, *V2;

	V1 = ( const NsVoxel* )vp1;
	V2 = ( const NsVoxel* )vp2;

	return __DATA( V1, distance ) > __DATA( V2, distance );
	}


NS_PRIVATE void _ns_spine_clumps_sort_voxel_lists_by_desc_dts( NsSpines *spines )
	{
	NsSpineClump  *clump;
	nslistiter     iter;
	//nssize         num_voxels;


/*TEMP*///ns_println( "Total # of voxels in the hash = " NS_FMT_ULONG, ns_voxel_table_size( &spines->voxel_table ) );
//num_voxels = 0;

	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		clump = ns_list_iter_get_object( iter );

		//num_voxels += ns_list_size( &clump->voxels );

		ns_list_sort( &clump->voxels, _ns_spines_voxels_compare_desc_dts );
		}

/*TEMP*///ns_println( "Total # of voxels in the clumps = " NS_FMT_ULONG, num_voxels );
	}


NS_PRIVATE nsboolean _ns_spine_clump_voxel_is_not_model_interior( NsVoxel *voxel, NsSpines *spines )
	{
	nssize   i;
	NsVoxel  N, *neighbor;


	for( i = 0; i < NS_VOXEL_NUM_FACE_OFFSETS; ++i )
		{
		ns_vector3i_add( &N.position, &voxel->position, spines->voxel_face_offsets + i );

		if( N.position.x >= 0 && N.position.x < spines->width  &&
			 N.position.y >= 0 && N.position.y < spines->height &&
			 N.position.z >= 0 && N.position.z < spines->length    )
			if( NULL == ( neighbor = ns_voxel_table_find( &spines->voxel_table, &N.position ) ) )
				return NS_TRUE;
		}

	return NS_FALSE;
	}


NS_PRIVATE void _ns_spine_clumps_remove_model_interior_voxels( NsSpines *spines )
	{
	NsSpineClump  *clump;
	nslistiter     iter;
	nslistiter     curr_voxel, end_voxels;
	NsVoxel       *voxel;
	nsint          found_any;
	nssize         clump_zero_size;


	clump_zero_size = 0;

	NS_LIST_FOREACH( &spines->clumps, iter )
		{
		found_any = NS_FALSE;
		clump     = ns_list_iter_get_object( iter );

		/* Note that the list is traversed from back to front. */
		curr_voxel = ns_list_rev_begin( &clump->voxels );
		end_voxels = ns_list_rev_end( &clump->voxels );

		for( ; ns_list_rev_iter_not_equal( curr_voxel, end_voxels );
				 curr_voxel = ns_list_rev_iter_next( curr_voxel ) )
			{
			voxel = ns_list_rev_iter_get_object( curr_voxel );

			//ns_assert( __DATA( voxel, clump ) == clump );

			/* Note that once we find a voxel that is NOT a model interior, all
				the remaining ones in the list have their clump ID's set to 1. */

			if( ! found_any )
				{
				/* Model interior voxels go into clump zero. */

				if( __DATA( voxel, distance ) > 0.0f /*_ns_spine_clump_voxel_is_not_model_interior( voxel, spines )*/ )
					{
					__DATA( voxel, dts_id )   = __DATA( voxel, clump_id );
					__DATA( voxel, clump_id ) = 1;

					found_any = NS_TRUE;
					}
				else
					{
					__DATA( voxel, dts_id )   = 0;
					__DATA( voxel, clump_id ) = 0;

					++clump_zero_size;
					}
				}
			else
				{
				__DATA( voxel, dts_id )   = __DATA( voxel, clump_id );
				__DATA( voxel, clump_id ) = 1;
				}


//ns_print( "%f:%d ", __DATA( voxel, distance ), __DATA( voxel, clump_id ) );


			__DATA( voxel, clump ) = NULL;
			}
		}

/*TEMP*/ns_println( "Clump-0 size = " NS_FMT_ULONG, clump_zero_size );
	}


NS_PRIVATE NsError _ns_do_run_spines( NsSpines *spines )
   {
   nsint    section_type;
   NsError  error;


   error        = ns_no_error();
   section_type = ns_model_get_section_type( spines->model );

	ns_model_set_thresh_count( spines->model, 0 );

	if( ns_settings_get_spine_z_spread_automated( spines->settings ) )
		{
		//nsboolean  use_2d_sampling;
		//nsfloat    aabbox_scalar;
		//nsint      min_window;

		//ns_grafting_read_params( spines->config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

		ns_model_compute_z_spread_divisor(
			spines->model,
			spines->settings,
			spines->image,
			spines->roi,
			spines->use_2d_sampling,
			spines->aabbox_scalar,
			spines->min_window,
			&spines->stretch_factor,
			spines->progress
			);

		ns_settings_set_spine_z_spread_divisor( ( NsSettings* )spines->settings, spines->stretch_factor );
		}
	else
		spines->stretch_factor = ns_settings_get_spine_z_spread_divisor( spines->settings );

ns_println( "spines->stretch_factor = %f", spines->stretch_factor );


	if( NS_FAILURE( ns_grafting_establish_thresholds_and_contrasts( 
							spines->settings,
							spines->image,
							spines->roi,
							spines->use_2d_sampling,
							spines->aabbox_scalar,
							spines->min_window,
							spines->model,
							spines->progress
							),
							error ) )
		goto _NS_DO_RUN_SPINES_EXIT;

	//if( ns_settings_get_threshold_auto_correct( spines->settings ) )
	//	{
/*TEMP*///nstimer start = ns_timer();

		//if( ! NS_FLOAT_EQUAL( 1.0f, ns_settings_get_threshold_multiplier( spines->settings ) ) )
		//	ns_settings_set_threshold_multiplier( ( NsSettings* )spines->settings, 1.0f );

		//if( NS_FAILURE( _ns_spines_establish_thresholds_and_contrast( spines ), error ) )
		//	goto _NS_DO_RUN_SPINES_EXIT;

		//_ns_spines_adjust_thresholds_and_contrast( spines );

/*TEMP*///ns_println( "Time to hollow out dendrites: %f seconds.", ns_difftimer( ns_timer(), start ) );
		//}
	/*else*/

	if( spines->do_edge_thresholds )
		{
		if( NS_FAILURE( _ns_spines_calc_edge_thresholds_and_contrasts( spines ), error ) )
			goto _NS_DO_RUN_SPINES_EXIT;

		_ns_spines_smooth_out_vertex_and_edge_thresholds_and_contrasts( spines );
		}

	//else
	//	_ns_spines_smooth_out_thresholds_and_contrasts( spines->model );


   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   if( NS_FAILURE( ns_model_section( spines->model, NS_MODEL_SPINES_SECTIONING, spines->progress ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;
   
   if( NS_FAILURE( ns_model_calc_section_lengths( spines->model ), error ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   if( NS_FAILURE( _ns_spines_build_inflated_model_octree( spines ), error ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   if( NS_FAILURE( _ns_spines_get_candidates( spines ), error ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   _ns_spines_classify_candidates( spines );

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   if( NS_FAILURE( _ns_spines_build_maxima_list( spines ), error ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;






	/*TEMP!!!!!*/ns_spines_create_random_colors( 1024 );


	//_ns_spines_init_clump_ids( spines, 1 );

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//if( NS_FAILURE( _ns_spines_create_clumps_by_intensity( spines, NS_FALSE ), error ) )
	//	goto _NS_DO_RUN_SPINES_EXIT;

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//_ns_spines_reset_clump_ids_based_on_dts( spines );

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//_ns_spines_put_maxima_on_hold( spines );

	//if( NS_FAILURE( _ns_spines_create_maxima_for_dendrite_clump( spines ), error ) )
	//	goto _NS_DO_RUN_SPINES_EXIT;

   //if( NS_FAILURE( _ns_spines_create_dendrite_clump( spines ), error ) )
	//	goto _NS_DO_RUN_SPINES_EXIT;

	//_ns_spines_restore_maxima_on_hold( spines );

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//_ns_spines_reset_clump_ids_to_zero_or_one( spines );



	_ns_spines_init_voxel_clump_ids( spines, NS_TRUE );
	if( NS_FAILURE( _ns_spines_create_clumps_by_dts( spines ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//_ns_spine_clumps_sort_voxel_lists_by_desc_dts( spines );
	//_ns_spine_clumps_remove_model_interior_voxels( spines );

   if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	_ns_spines_init_voxel_clump_ids( spines, NS_FALSE );
	if( NS_FAILURE( _ns_spines_create_clumps_by_intensity( spines ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;




	//_ns_spines_set_voxel_final_clump_ids( spines );



   if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;

	//if( NS_FAILURE( _ns_spines_recreate_maxima_list( spines ), error ) )
	//	goto _NS_DO_RUN_SPINES_EXIT;

   //if( ns_progress_cancelled( spines->progress ) )goto _NS_DO_RUN_SPINES_EXIT;



   if( spines->do_file )
      _ns_spines_open_files( spines );

   if( NS_FAILURE( _ns_spines_create_shapes( spines ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;

   if( spines->do_file )
      _ns_spines_close_files( spines );



	/* Can remove the clumps right here. Comment out to enable clicking on
		clumps to print merging information to the console. */
	_ns_spines_nullify_clump_voxel_pointers( spines );
	ns_list_clear( &spines->clumps );



   //if( NS_FAILURE( _ns_spines_fix_rejected_voxels( spines ), error ) )
     // goto _NS_DO_RUN_SPINES_EXIT;

   //if( ns_progress_cancelled( spines->progress ) )
     // goto _NS_DO_RUN_SPINES_EXIT;


	_ns_spines_analyze_shapes( spines );

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   /* TEMP */
   ns_println( "** spine analysis BEFORE rayburst's took " NS_FMT_DOUBLE " seconds.",
      ns_difftimer( ns_timer(), spines->start ) );

   if( NS_FAILURE( _ns_spines_calc_shapes( spines ), error ) )
      goto _NS_DO_RUN_SPINES_EXIT;

   /* TEMP */
   ns_println( "** spine analysis AFTER rayburst's took " NS_FMT_DOUBLE " seconds.",
      ns_difftimer( ns_timer(), spines->start ) );

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;


   /*
   if( spines->enable_merging )
      if( NS_FAILURE( _ns_spines_run_merging( spines ), error ) )
         goto _NS_DO_RUN_SPINES_EXIT;

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;
   */


   /*_ns_spines_reset_ids( spines ); */

   _ns_spines_calc_bounding_boxes( spines );

   //ns_spines_create_random_colors( 1000/*_ns_spines_size( spines )*/ );

   _ns_spines_compute_normals( spines );

	/* Comment out if enabling the display of each spine's voxels. */
	_ns_spines_set_fixed_size_aabbox( spines );



/** AXIS CALCULATION!!!!!!!! **/
	if( NS_FAILURE( _ns_spines_calc_axis( spines ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;

	if( NS_FAILURE( _ns_spines_calc_attachments( spines ), error ) )
		goto _NS_DO_RUN_SPINES_EXIT;

	ns_model_spines_calc_xyplane_angles( spines->model );

   if( ns_progress_cancelled( spines->progress ) )
      goto _NS_DO_RUN_SPINES_EXIT;



/* TEMP!!!!!!!! */
//_ns_spines_create_clusters_by_gradients( spines );


	/* If we're using a custom classifier, then we finally apply the types to the
		detected spines. For the default classifier, the type has already been set.*/
	if( NULL != ns_spines_classifier_get() )
		if( NS_FAILURE(
				ns_model_spines_retype_by_ann_classifier(
					spines->model,
					ns_spines_classifier_get(),
					spines->progress
					),
				error ) )
			goto _NS_DO_RUN_SPINES_EXIT;


   _NS_DO_RUN_SPINES_EXIT:

   if( ns_is_error( error ) )
		ns_model_clear_spines( spines->model, spines->voxel_info, spines->roi );

   /* Restore old sectionation if necessary. */
   if( section_type != NS_MODEL_SPINES_SECTIONING )
      error = ns_model_section( spines->model, section_type, spines->progress );

   /* Routines for display purposes only. Ignore error. */
   //if( spines->do_additional_graphics && ! ns_is_error( error ) )
     // {
      //_ns_spines_make_polygon_strips( spines );
      //_ns_spines_make_images( spines );
      //}

	ns_model_lock( spines->model );


	if( ! spines->keep_voxels )
		{
		/*TEMP*/ns_println( "Deleting voxels..." );

		_ns_spines_clear_voxel_lists( spines );
		ns_voxel_table_clear( &spines->voxel_table );
		ns_list_clear( &spines->clumps );
		}
	else
		{
		//_ns_spines_set_splats_colors( spines );

		//_ns_spines_clear_non_spine_voxels( spines );

		_ns_spines_set_spine_voxel_normals( spines );
		}


	//ns_model_clear_octree( spines->model, NS_MODEL_EDGE_OCTREE );

	_ns_spines_set_analyzed_flag( spines );

	ns_model_unlock( spines->model );

/*TEMP*/ns_println( "THRESH_COUNT = " NS_FMT_UINT, ns_model_get_thresh_count( spines->model ) );
	ns_model_set_thresh_count( spines->model, 0 );

	return error;
	}


/*
void _ns_model_select_spines_by_count_and_spread_ratios
   (
   NsModel       *model,
   const nschar  *config_file
   )
   {
   NsConfigDb           db;
   nsdouble             max_count_ratio;
   nsdouble             max_spread_ratio;
   const NsSpines      *spines;
   nslistiter           curr;
   nslistiter           end;
   nslistiter           iter;
   NsSpine             *spine;
   const NsSpineLayer  *layer;
   NsError              error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
   ns_assert( NULL != config_file );

   if( NS_FAILURE( ns_config_db_construct( &db );

   if( NS_FAILURE( ns_config_db_read( &db, config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return;
      }

   if( ! ns_config_db_has_group( &db, "spines" ) )
      {
      ns_config_db_destruct( &db );
      return;
      }

   if( ! ns_config_db_has_key( &db, "spines", "max_count_ratio" ) ||
       ! ns_config_db_has_key( &db, "spines", "max_spread_ratio" )  )
      {
      ns_config_db_destruct( &db );
      return;
      }

   max_count_ratio =
      ns_config_db_get_double( &db, "spines", "max_count_ratio" );

   max_spread_ratio =
      ns_config_db_get_double( &db, "spines", "max_spread_ratio" );

   ns_config_db_destruct( &db );

   ns_println( "max_count_ratio  = " NS_FMT_DOUBLE, max_count_ratio );
   ns_println( "max_spread_ratio = " NS_FMT_DOUBLE, max_spread_ratio );

   spines = model->spines;

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      NS_LIST_FOREACH( &spine->layers, iter )
         {
         layer = ns_list_iter_get_object( iter );

         if( ( max_count_ratio < layer->count_ratio || max_spread_ratio < layer->spread_ratio )
             &&
             spine->is_attached
            )
            {
            spine->is_selected = NS_TRUE;
            break;
            }
         }
      }
   }
*/


nsboolean _ns_spines_read_merge_params( NsSpines *spines, const nschar *config_file )
   {
   NsConfigDb  db;
   NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return NS_FALSE;

   if( NS_FAILURE( ns_config_db_read( &db, config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_group( &db, "spines" ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_key( &db, "spines", "merge_value" ) ||
       ! ns_config_db_has_key( &db, "spines", "merge_power" ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   spines->merge_value =
      ( nsfloat )ns_config_db_get_double( &db, "spines", "merge_value" );

   spines->merge_power =
      ( nsfloat )ns_config_db_get_double( &db, "spines", "merge_power" );

   ns_config_db_destruct( &db );

   ns_println( "merge_value = " NS_FMT_DOUBLE, spines->merge_value );
   ns_println( "merge_power = " NS_FMT_DOUBLE, spines->merge_power );

   return NS_TRUE;
   }


NS_PRIVATE nsboolean _ns_spines_read_config_file( NsSpines *spines, const nschar *config_file )
   {
   NsConfigDb  db;
   NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return NS_FALSE;

   if( NS_FAILURE( ns_config_db_read( &db, config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_group( &db, "spines" ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( /*! ns_config_db_has_key( &db, "spines", "thin_ratio" ) ||*/
       ! ns_config_db_has_key( &db, "spines", "invalid_ratio" ) ||
       /*! ns_config_db_has_key( &db, "spines", "neck_ratio" ) ||*/
       /*! ns_config_db_has_key( &db, "spines", "mushroom_size" ) ||*/
       ! ns_config_db_has_key( &db, "spines", "remove_base_layers" ) ||
       ! ns_config_db_has_key( &db, "spines", "enable_merging" ) ||
       ! ns_config_db_has_key( &db, "spines", "max_octree_recursion" ) ||
       ! ns_config_db_has_key( &db, "spines", "save_layers" ) ||
       ! ns_config_db_has_key( &db, "spines", "render_layer" ) ||
       ! ns_config_db_has_key( &db, "spines", "cutoff_layer" ) ||
       ! ns_config_db_has_key( &db, "spines", "max_count_ratio" ) ||
       ! ns_config_db_has_key( &db, "spines", "max_spread_ratio" ) ||
       ! ns_config_db_has_key( &db, "spines", "clump_dts_merge" ) ||
       ! ns_config_db_has_key( &db, "spines", "merge_value" ) ||
       ! ns_config_db_has_key( &db, "spines", "merge_power" ) ||
		 ! ns_config_db_has_key( &db, "spines", "thresh_is_2d" ) ||
		 ! ns_config_db_has_key( &db, "spines", "thresh_aabbox_scalar" ) ||
		 ! ns_config_db_has_key( &db, "spines", "thresh_min_window" ) ||
		 ! ns_config_db_has_key( &db, "spines", "clump_intensity_merge" ) ||
		 ! ns_config_db_has_key( &db, "spines", "max_declumping_spread" ) ||
		 ! ns_config_db_has_key( &db, "spines", "axis_smoothing" ) ||
		 ! ns_config_db_has_key( &db, "spines", "clump_merge_by_intensity" ) ||
		 ! ns_config_db_has_key( &db, "spines", "clump_merge_by_dts" ) ||
		 ! ns_config_db_has_key( &db, "spines", "min_clump_connectivity" ) ||
		 ! ns_config_db_has_key( &db, "spines", "max_clump_pinch" ) ||
		 ! ns_config_db_has_key( &db, "spines", "max_clump_dts_range" ) ||
		 ! ns_config_db_has_key( &db, "spines", "max_clump_intensity_range" )
		)
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

	spines->use_2d_sampling =
		ns_config_db_get_boolean( &db, "spines", "thresh_is_2d" );

	spines->aabbox_scalar =
		( nsfloat )ns_config_db_get_double( &db, "spines", "thresh_aabbox_scalar" );

	spines->min_window =
		ns_config_db_get_int( &db, "spines", "thresh_min_window" );

   /* Not a critical parameter so dont report error if not found. */
   spines->do_file =
      ns_config_db_has_key( &db, "spines", "do_file" ) ?
         ns_config_db_get_boolean( &db, "spines", "do_file" ) : NS_FALSE;
   
   ____thin_ratio =
		ns_settings_get_classify_thin_ratio( spines->settings );
      /*ns_config_db_get_double( &db, "spines", "thin_ratio" );*/

   ____invalid_ratio =
      ns_config_db_get_double( &db, "spines", "invalid_ratio" );

   ____neck_ratio =
		ns_settings_get_classify_neck_ratio( spines->settings );
      /*ns_config_db_get_double( &db, "spines", "neck_ratio" );*/

   ____mushroom_size =
		ns_settings_get_classify_mushroom_size( spines->settings );
      /*ns_config_db_get_double( &db, "spines", "mushroom_size" );*/

   spines->remove_base_layers =
      ns_config_db_get_boolean( &db, "spines", "remove_base_layers" );

   spines->enable_merging =
      ns_config_db_get_boolean( &db, "spines", "enable_merging" );

   spines->max_octree_recursion =
      ( nssize )ns_config_db_get_int( &db, "spines", "max_octree_recursion" );

   spines->save_layers =
      ns_config_db_get_boolean( &db, "spines", "save_layers" );

   spines->render_layer =
      ns_config_db_get_int( &db, "spines", "render_layer" );

   spines->cutoff_layer =
      ns_config_db_get_int( &db, "spines", "cutoff_layer" );

   ____max_count_ratio =
      ns_config_db_get_double( &db, "spines", "max_count_ratio" );

   ____max_spread_ratio =
      ns_config_db_get_double( &db, "spines", "max_spread_ratio" );

	____min_height = 
		ns_settings_get_spine_elim_height( spines->settings );

   spines->clump_dts_merge =
      ( nsfloat )ns_config_db_get_double( &db, "spines", "clump_dts_merge" );

   spines->merge_value =
      ( nsfloat )ns_config_db_get_double( &db, "spines", "merge_value" );

   spines->merge_power =
      ( nsfloat )ns_config_db_get_double( &db, "spines", "merge_power" );

	spines->clump_intensity_merge =
		( nsfloat )ns_config_db_get_double( &db, "spines", "clump_intensity_merge" );

	spines->max_declumping_spread =
		( nsfloat )ns_config_db_get_double( &db, "spines", "max_declumping_spread" );

   spines->active_render =
      ns_config_db_has_key( &db, "spines", "active_render" ) ?
         ns_config_db_get_boolean( &db, "spines", "active_render" ) : NS_TRUE;

   spines->do_additional_graphics =
      ns_config_db_has_key( &db, "spines", "do_additional_graphics" ) ?
         ns_config_db_get_boolean( &db, "spines", "do_additional_graphics" ) : NS_FALSE;

	spines->optimize_isodata =
		ns_config_db_has_key( &db, "spines", "optimize_isodata" ) ?
			ns_config_db_get_boolean( &db, "spines", "optimize_isodata" ) : NS_TRUE;

	spines->flatten_isodata =
		ns_config_db_has_key( &db, "spines", "flatten_isodata" ) ?
			ns_config_db_get_boolean( &db, "spines", "flatten_isodata" ) : NS_TRUE;

	spines->axis_smoothing = 
		ns_config_db_get_int( &db, "spines", "axis_smoothing" );

	spines->do_edge_thresholds =
		ns_config_db_has_key( &db, "spines", "do_edge_thresholds" ) ?
			ns_config_db_get_boolean( &db, "spines", "do_edge_thresholds" ) : NS_FALSE;

	spines->clump_merge_by_intensity =
		ns_config_db_has_key( &db, "spines", "clump_merge_by_intensity" ) ?
			ns_config_db_get_boolean( &db, "spines", "clump_merge_by_intensity" ) : NS_TRUE;

	spines->clump_merge_by_dts =
		ns_config_db_has_key( &db, "spines", "clump_merge_by_dts" ) ?
			ns_config_db_get_boolean( &db, "spines", "clump_merge_by_dts" ) : NS_FALSE;

	spines->min_clump_connectivity =
		ns_config_db_has_key( &db, "spines", "min_clump_connectivity" ) ?
			ns_config_db_get_double( &db, "spines", "min_clump_connectivity" ) : 1.0;

	spines->max_clump_pinch           = ns_config_db_get_double( &db, "spines", "max_clump_pinch" );
	spines->max_clump_dts_range       = ns_config_db_get_double( &db, "spines", "max_clump_dts_range" );
	spines->max_clump_intensity_range = ns_config_db_get_double( &db, "spines", "max_clump_intensity_range" );

   ns_config_db_destruct( &db );

   ns_println( "thin_ratio                = " NS_FMT_DOUBLE, ____thin_ratio );
   ns_println( "invalid_ratio             = " NS_FMT_DOUBLE, ____invalid_ratio );
   ns_println( "neck_ratio                = " NS_FMT_DOUBLE, ____neck_ratio );
   ns_println( "mushroom_size             = " NS_FMT_DOUBLE, ____mushroom_size );
   ns_println( "remove_base_layers        = " NS_FMT_INT, ( nsint )spines->remove_base_layers );
   ns_println( "enable_merging            = " NS_FMT_INT, ( nsint )spines->enable_merging );
   ns_println( "max_octree_recursion      = " NS_FMT_ULONG, spines->max_octree_recursion );
   ns_println( "save_layers               = " NS_FMT_INT, ( nsint )spines->save_layers );
   ns_println( "render_layer              = " NS_FMT_INT, spines->render_layer );
   ns_println( "cutoff_layer              = " NS_FMT_INT, spines->cutoff_layer );
   ns_println( "max_count_ratio           = " NS_FMT_DOUBLE, ____max_count_ratio );
   ns_println( "max_spread_ratio          = " NS_FMT_DOUBLE, ____max_spread_ratio );
   ns_println( "merge_value               = " NS_FMT_DOUBLE, spines->merge_value );
   ns_println( "merge_power               = " NS_FMT_DOUBLE, spines->merge_power );
   ns_println( "active_render             = " NS_FMT_INT, ( nsint )spines->active_render );
   ns_println( "do_additional_graphics    = " NS_FMT_INT, ( nsint )spines->do_additional_graphics );
	ns_println( "use_2d_sampling           = " NS_FMT_STRING, spines->use_2d_sampling ? "true" : "false" );
	ns_println( "aabbox_scalar             = " NS_FMT_DOUBLE, spines->aabbox_scalar );
	ns_println( "min_window                = " NS_FMT_INT, spines->min_window );
	ns_println( "optimize_isodata          = " NS_FMT_STRING, spines->optimize_isodata ? "true" : "false" );
	ns_println( "flatten_isodata           = " NS_FMT_STRING, spines->flatten_isodata ? "true" : "false" );
	ns_println( "clump_intensity_merge     = " NS_FMT_DOUBLE, spines->clump_intensity_merge );
   ns_println( "clump_dts_merge           = " NS_FMT_DOUBLE, spines->clump_dts_merge );
   ns_println( "max_declumping_spread     = " NS_FMT_DOUBLE, spines->max_declumping_spread );
	ns_println( "axis_smoothing            = " NS_FMT_INT, spines->axis_smoothing );
	ns_println( "do_edge_thresholds        = " NS_FMT_STRING, spines->do_edge_thresholds ? "true" : "false" );
	ns_println( "clump_merge_by_intensity  = " NS_FMT_INT, spines->clump_merge_by_intensity );
	ns_println( "clump_merge_by_dts        = " NS_FMT_INT, spines->clump_merge_by_dts );
	ns_println( "min_clump_connectivity    = " NS_FMT_DOUBLE, spines->min_clump_connectivity );
	ns_println( "max_clump_pinch           = " NS_FMT_DOUBLE, spines->max_clump_pinch );
	ns_println( "max_clump_dts_range       = " NS_FMT_DOUBLE, spines->max_clump_dts_range );
	ns_println( "max_clump_intensity_range = " NS_FMT_DOUBLE, spines->max_clump_intensity_range );

   return NS_TRUE;
   }


NS_PRIVATE NsError _ns_do_spine_analysis
   (
   NsSpines              *spines,
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
   )
   {
   NsError error = ns_no_error();

	spines->settings = settings;

   if( ! _ns_spines_read_config_file( spines, config_file ) )
      {
      /*TEMP*/ns_println( "MUST SPECIFY SPINE CONSTANTS IN CONFIGURATION FILE!!!" );
      return error;
      }

   if( NS_FAILURE(
			_ns_spines_init(
            spines,
            model,
            settings,
            image,
				roi,
            pixel_proc_db,
            interp_type,
            use_2d_bbox,
            average_intensity,
            kernel_type,
            progress,
            render_func,
				keep_voxels,
				config_file,
				residual_smear_type
            ),
			error ) )
      return error;

   spines->is_running = NS_TRUE;

   if( NULL != render_func )
      ( render_func )();

   spines->start = ns_timer();
   error = _ns_do_run_spines( spines );
   spines->stop = ns_timer();

   _ns_spines_finalize( spines );

   spines->is_running = NS_FALSE;

   /* TEMP */
   ns_println( "** spine analysis took " NS_FMT_DOUBLE " seconds.", ns_difftimer( spines->stop, spines->start ) );
   
   return error;
   }


NsError ns_model_spine_analysis
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
   )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
   ns_assert( NULL != settings );
   ns_assert( NULL != image );
   ns_assert( NULL != progress );

   ns_log_entry(
      NS_LOG_ENTRY_FUNCTION,
      NS_FUNCTION
      "( model=" NS_FMT_STRING_DOUBLE_QUOTED
      ", settings=" NS_FMT_POINTER
      ", image=" NS_FMT_POINTER
      ", roi=" NS_FMT_POINTER
      ", pixel_proc_db=" NS_FMT_POINTER
      ", interp_type=" NS_FMT_STRING_DOUBLE_QUOTED
      ", use_2d_bbox=" NS_FMT_INT
      ", average_intensity=" NS_FMT_DOUBLE
      ", kernel_type=" NS_FMT_STRING_DOUBLE_QUOTED
      ", progress=" NS_FMT_POINTER
      ", render_func=" NS_FMT_POINTER
		", keep_voxels=" NS_FMT_INT
      ", config_file=" NS_FMT_STRING_DOUBLE_QUOTED
		", residual_smear_type=" NS_FMT_STRING_DOUBLE_QUOTED
      " )",
      ns_model_get_name( model ),
      settings,
      image,
      roi,
      pixel_proc_db,
      ns_rayburst_interp_type_to_string( interp_type ),
      use_2d_bbox,
      average_intensity,
      ns_rayburst_kernel_type_to_string( kernel_type ),
      progress,
      render_func,
		keep_voxels,
      config_file,
		ns_residual_smear_type_to_string( residual_smear_type )
      );

   ns_model_clear_spines( model, ns_settings_voxel_info( settings ), roi );

   return _ns_do_spine_analysis(
            model->spines,
            model,
            settings,
            image,
				roi,
            pixel_proc_db,
            interp_type,
            use_2d_bbox,
            average_intensity,
            kernel_type,
            progress,
            render_func,
				keep_voxels,
            config_file,
				residual_smear_type
            );
   }


NS_PRIVATE void _ns_model_spine_calc_xyplane_angle( NsSpine *spine )
	{
	NsVector3f  S;
	NsVector3f  Z;
	nsdouble    angle;


	ns_vector3f_sub( &S, &spine->center, &spine->attach ); /* S = center - attach */
	ns_vector3f( &Z, 0.0f, 0.0f, 1.0f ); /* Z-axis vector */

	/* Get the angle in degrees. */
	angle = ( nsdouble )ns_vector3f_angle( &S, &Z );
	angle = NS_RADIANS_TO_DEGREES( angle );

	/* NOTE: We want the angle from the XY plane, so subtract 90 degrees. */
	spine->xyplane_angle = angle - 90.0;
	}


void ns_model_spine_calc_xyplane_angle( nsspine S )
	{  _ns_model_spine_calc_xyplane_angle( ns_list_iter_get_object( S ) );  }


void ns_model_spines_calc_xyplane_angles( NsModel *model )
	{
   nslistiter   curr, end;
	NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! spine->is_analyzed )
			_ns_model_spine_calc_xyplane_angle( spine );
		}
	}




NS_PRIVATE NsError _ns_model_spine_do_calc_xyplane_angle_if_unset
   (
   NsSpines  *spines,
   NsSpine   *spine,
   NsVector  *nodes
   )
	{
	if( _NS_SPINE_XYPLANE_ANGLE_IS_UNSET( spine ) )
		{
		nsvectoriter         curr_node;
		nsvectoriter         end_nodes;
		const NsOctreeNode  *node;
		nsvectoriter         curr_object;
		nsvectoriter         end_objects;
		nsmodeledge          edge;
		NsVector3f           attach;
		nsfloat              distance;
		nsfloat              min_distance;
		NsError              error;

		if( NS_FAILURE(
				ns_model_octree_intersections(
					spines->model,
					NS_MODEL_EDGE_OCTREE,
					&spine->center,
					_ns_spine_voxel_octree_func_special,
					nodes
					),
				error ) )
			return error;

		min_distance = NS_FLOAT_MAX;

		curr_node = ns_vector_begin( nodes );
		end_nodes = ns_vector_end( nodes );

		for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
				 curr_node = ns_vector_iter_next( curr_node ) )
			{
			node = ns_vector_iter_get_object( curr_node );
			ns_assert( 0 < ns_octree_node_num_objects( node ) );

			curr_object = ns_octree_node_begin_objects( node );
			end_objects = ns_octree_node_end_objects( node );

			for( ; ns_vector_iter_not_equal( curr_object, end_objects );
					 curr_object = ns_vector_iter_next( curr_object ) )
				{
				edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

				distance = _ns_spines_distance_to_conical_frustum(
								spines,
								&spine->center,
								edge,
								&attach,
								NULL,
								NULL,
								NULL,
								NULL
								);

				if( distance < min_distance )
					{
					min_distance  = distance;
					spine->attach = attach;
					}
				}
			}

		if( 0 < ns_vector_size( nodes ) )
			_ns_model_spine_calc_xyplane_angle( spine );

		ns_verify( NS_SUCCESS( ns_vector_resize( nodes, 0 ), error ) );
		}

	return ns_no_error();
	}


NsError ns_model_spines_calc_xyplane_angles_if_unset
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	)
	{
	NsVector    nodes;
   nslistiter  curr, end;
	NsError     error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != settings );

	error = ns_no_error();

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

	/* The build octree routine needs a few things set before calling. */
	model->spines->settings             = settings;
	model->spines->voxel_info           = ns_settings_voxel_info( settings );
	model->spines->progress             = progress;
	model->spines->max_octree_recursion = 5; /*TEMP?*/

	if( NS_FAILURE( _ns_spines_build_inflated_model_octree( model->spines ), error ) )
		goto _NS_MODEL_SPINES_CALC_UNSET_XYPLANE_ANGLES_EXIT;

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		if( NS_FAILURE(
				_ns_model_spine_do_calc_xyplane_angle_if_unset(
					model->spines,
					ns_list_iter_get_object( curr ),
					&nodes
					),
				error ) )
			goto _NS_MODEL_SPINES_CALC_UNSET_XYPLANE_ANGLES_EXIT;

	_NS_MODEL_SPINES_CALC_UNSET_XYPLANE_ANGLES_EXIT:

   ns_vector_destruct( &nodes );
	ns_model_clear_octree( model, NS_MODEL_EDGE_OCTREE );

	return error;
	}


nsboolean ns_model_spines_has_unset_xyplane_angles( const NsModel *model )
	{
   nslistiter   curr, end;
	NsSpine     *spine;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( _NS_SPINE_XYPLANE_ANGLE_IS_UNSET( spine ) )
			return NS_TRUE;
		}

	return NS_FALSE;
	}


NS_PRIVATE NsError _ns_manual_spine_calc_volume_and_surface_area
	(
	NsSpine               *spine,
	NsSpines              *spines,
	const NsSettings      *settings,
	const NsImage         *image,
	nsfloat                threshold,
   NsRayburstInterpType   interp_type,
   NsRayburstKernelType   kernel_type,
	const NsVector3f      *max_attach,
	nsboolean             *do_remove
	)
	{
	NsRayburst                rayburst;
   nssize                    num_samples;
   const NsVector3f         *vectors;
   nssize                    num_triangles;
   const NsIndexTriangleus  *triangles;
	nssize                    i;
	nsdouble                  distance;
	nsdouble                  max_ray_distance;
	NsVector3f                M;
	NsVector3f                A;
	NsVector3f                V;
	nsfloat                   d;
	nsfloat                   max_d;
   NsTriangle3f              T;
	const NsVoxelInfo        *voxel_info;
   nsdouble                  attach_sa_fold_out;
   nsdouble                  attach_sa_fold_in;
	NsRayburstSample         *samples;
	nsboolean                 has_exteriors;
	NsError                   error;


	voxel_info = ns_settings_voxel_info( settings );

   if( NS_FAILURE( ns_rayburst_construct(
                     &rayburst,
                     kernel_type,
                     NS_RAYBURST_RADIUS_NONE,
                     interp_type,
                     image,
                     voxel_info,
                     NULL
                     ),
                     error ) )
      return error;

	ns_rayburst_set_threshold( &rayburst, threshold );
	spine->threshold = ( nsulong )threshold;


NS_USE_VARIABLE( max_attach );
	/* Find the unit vector from the max to its attachment point. */
	//ns_vector3f_sub( &A, max_attach, &spine->max );
	//ns_vector3f_norm( &A );

	/* Position the rayburst origin a 1/4 of a ways along this vector.
		i.e. origin = max + (0.25 * max_distance) * A
	*/
	//ns_vector3f_add(
	//	&spine->rays_origin,
	//	&spine->max,
	//	ns_vector3f_cmpd_scale( &A, 0.25f * spine->max_distance )
	//	);

	//max_ray_distance = 0.75f * spine->max_distance;

	
	/* Find the distance between the spine's center and its maximum. */
	distance = ( nsdouble )ns_vector3f_distance( &spine->max, &spine->center );

	/* Subtract off half of this distance from the spine's max D.T.S. to
		limit how far the rays can travel. Note that the rayburst's "limit rays"
		function expects a distance squared. */
	max_ray_distance = ( nsdouble )spine->max_distance - distance / 2.0;

	ns_rayburst_limit_rays( &rayburst, NS_TRUE, NS_POW2( max_ray_distance ) );

	/* Find the halfway point between the center and the maximum. */
	ns_vector3f_add( &M, &spine->max, &spine->center );
	ns_vector3f_cmpd_scale( &M, 0.5f );

	spine->rays_origin = M;



	ns_rayburst( &rayburst, &spine->rays_origin );

	spine->total_volume = ns_rayburst_volume( &rayburst );
	spine->total_sa     = ns_rayburst_surface_area( &rayburst );
ns_println( "MANUAL: VOLUME = %f, SURFACE_AREA = %f", spine->total_volume, spine->total_sa );

	attach_sa_fold_in  = 0.0;
	attach_sa_fold_out = 0.0;
	spine->attach_sa   = 0.0;

	spine->vectors       = ns_rayburst_kernel_type_vectors( kernel_type );
	spine->num_vectors   = ns_rayburst_kernel_type_num_vectors( kernel_type );
	spine->triangles     = ns_rayburst_kernel_type_triangles( kernel_type );
	spine->num_triangles = ns_rayburst_kernel_type_num_triangles( kernel_type );

   num_samples   = ns_rayburst_num_samples( &rayburst );
   vectors       = ns_rayburst_vectors( &rayburst );
   triangles     = ns_rayburst_triangles( &rayburst );
   num_triangles = ns_rayburst_num_triangles( &rayburst );

	if( NULL == ( spine->distances = ns_new_array( nsfloat, num_samples ) ) )
		{
		ns_rayburst_destruct( &rayburst );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	ns_rayburst_sample_distances(
		&rayburst,
		NS_RAYBURST_SAMPLE_DISTANCE_FORWARD,
		spine->distances
		);

	if( NULL == ( spine->interior = ns_new_array( nsuint8, NS_BITS_TO_BYTES( num_samples ) ) ) )
		{
		ns_rayburst_destruct( &rayburst );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	samples = ( NsRayburstSample* )_ns_rayburst_samples( &rayburst );

	for( i = 0; i < num_samples; ++i )
		{
		ns_bit_buf_clear( spine->interior, i );

		if( spine->distances[i] >= max_ray_distance )
			{
			ns_bit_buf_set( spine->interior, i );

			/* This creates a spherical "bottom" on the spine. */
			spine->distances[i] = ( nsfloat )max_ray_distance;
			samples[i].distance = ( nsfloat )max_ray_distance;
			}
		}

	/* Find the normalized attachment vector. */
	ns_vector3f_sub( &A, &spine->attach, &spine->rays_origin );
	ns_vector3f_norm( &A );

	/* Find the maximum distance of the exterior rays. */

	max_d         = -NS_FLOAT_MAX;
	has_exteriors = NS_FALSE;

	for( i = 0; i < num_samples; ++i )
		if( ns_bit_buf_is_clear( spine->interior, i ) )
			{
			has_exteriors = NS_TRUE;

			ns_vector3f_scale( &V, spine->vectors + i, spine->distances[i] );

			d = ns_vector3f_dot( &V, &A );

			if( d > max_d )
				max_d = d;
			}

	/* If there are no exterior rays, then just make max_d huge so that no
		updating of the rays will be performed in the next loop. */
	if( ! has_exteriors )
		max_d = NS_FLOAT_MAX;

	if( max_d <= 0.0f )
		goto _AFTER_FLATTENING;

	for( i = 0; i < num_samples; ++i )
		if( ns_bit_buf_is_set( spine->interior, i ) )
			{
			ns_vector3f_scale( &V, spine->vectors + i, spine->distances[i] );

			d = ns_vector3f_dot( &V, &A );

			if( d > max_d )
				{
				spine->distances[i] = ( nsfloat )( max_d * max_ray_distance / d );
				samples[i].distance = spine->distances[i];
				}
			}

	/* Have to re-compute volume and surface area since we moved the rays. */
	ns_rayburst_compute_volume_and_surface_area( &rayburst, 1.0f );

	spine->total_volume = ns_rayburst_volume( &rayburst );
	spine->total_sa     = ns_rayburst_surface_area( &rayburst );
ns_println( "MANUAL: VOLUME = %f, SURFACE_AREA = %f (ADJUSTED)", spine->total_volume, spine->total_sa );

	_AFTER_FLATTENING:

	if( NS_FAILURE( _ns_spines_calc_normals( spines, spine, &rayburst ), error ) )
		{
		ns_rayburst_destruct( &rayburst );
		return error;
		}

	for( i = 0; i < num_triangles; ++i )
		if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, || ) )
			{
			_ns_spines_make_triangle( spine, vectors, triangles, spine->distances, i, &T );
			attach_sa_fold_out += ns_triangle3f_surface_area( &T );
			}

	for( i = 0; i < num_triangles; ++i )
		if( __IS_INTERIOR_TRIANGLE( spine->interior, triangles, i, && ) )
			{
			_ns_spines_make_triangle( spine, vectors, triangles, spine->distances, i, &T );
			attach_sa_fold_in += ns_triangle3f_surface_area( &T );
			}

	/* The actual neurite surface area is the average of the two. */
	spine->attach_sa = ( attach_sa_fold_out + attach_sa_fold_in ) / 2.0;

	/* TEMP: Is this a bug? Check for roundoff error. */
	if( spine->attach_sa > spine->total_sa )
		spine->attach_sa = spine->total_sa;

	spine->non_attach_sa = spine->total_sa - spine->attach_sa;

	/* Check for a spine with a tiny non-attachment area. These are not valid. */
	if( NS_ABS( spine->non_attach_sa ) < ns_voxel_info_surface_area( voxel_info ) / 100.0f )
		{
ns_println( "MANUAL: NON-ATTACH-SA is too small!!!" );
		*do_remove = NS_TRUE;
		}

	/* Dont allow spines to have a greater attachment area than non-attachment area. */
	if( spine->non_attach_sa < spine->attach_sa )
		{
ns_println( "MANUAL: ATTACH-SA > NON-ATTACH-SA" );
		*do_remove = NS_TRUE;
		}

ns_println( "MANUAL: ATTACH-SA = %f, NON-ATTACH-SA = %f, BOTH = %f",
	spine->attach_sa, spine->non_attach_sa, spine->attach_sa + spine->non_attach_sa );

	ns_rayburst_destruct( &rayburst );
	return ns_no_error();
	}


NsError ns_model_spines_on_add_manual
	(
	NsModel               *model,
	const NsSettings      *settings,
	const NsImage         *image,
	nsfloat                f_threshold,
   NsRayburstInterpType   interp_type,
   NsRayburstKernelType   kernel_type,
	nsspine                S,
	const NsVector3f      *attach_finder,
	nsboolean             *do_remove
	)
	{
	NsVoxelTable        voxel_table;
	//NsList              edges;
	const NsVoxelInfo  *voxel_info;
	const NsVector3i   *voxel_offsets;
	nsmodelvertex       curr_vertex, end_vertices;
	nsmodeledge         curr_edge, end_edges;
	nsmodeledge         actual_edge;
	nsfloat             max_voxel_distance;
	nsfloat             distance;
	nsfloat             min_distance;
	nsfloat             curr_distance;
	nsfloat             next_distance;
	nsfloat             max_distance;
	NsVector3f          position;
	NsVector3f          curr_attach;
	NsVector3f          actual_attach;
	NsVector3f          V;
	NsVector3f          A;
	NsVector3i          curr_position;
	NsVector3i          next_position;
	NsVector3i          max_position;
	NsAABBox3d          box;
	nssize              edge_count;
	//nslistiter          iter;
	nssize              i;
	nsint               width;
	nsint               height;
	nsint               length;
	NsVoxelBuffer       voxel_buffer;
	NsVoxel            *voxel;
	NsSpine            *this_spine;
	NsSpine            *other_spine;
	nsspine             curr_spine;
	nsspine             end_spines;
	nsulong             u_threshold;
	NsError             error;


/*TEMP*/nstimer start; start = ns_timer();

   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != settings );
	ns_assert( NULL != do_remove );


	error      = ns_no_error();
	*do_remove = NS_FALSE;

   if( NS_FAILURE( ns_voxel_table_construct( &voxel_table, NULL ), error ) )
      return error;

	ns_voxel_buffer_init( &voxel_buffer, image );

	ns_voxel_table_recycle( &voxel_table, NS_FALSE );

   ns_voxel_table_set_dimensions(
      &voxel_table,
      ns_voxel_buffer_width( &voxel_buffer ),
      ns_voxel_buffer_height( &voxel_buffer ),
      ns_voxel_buffer_length( &voxel_buffer )
      );

	//ns_list_construct( &edges, NULL );

	/* Have to reset the stretch factor to 1.0 (no correction) for manual spines. */
	model->spines->stretch_factor = 1.0f;

	u_threshold = ( nsulong )f_threshold;

	this_spine = ns_list_iter_get_object( S );

	this_spine->is_analyzed = NS_TRUE;

	voxel_info    = ns_settings_voxel_info( settings );
	voxel_offsets = model->spines->voxel_offsets;

	max_voxel_distance = ns_settings_get_spine_max_voxel_distance( settings );

	ns_spine_get_position( S, &position );




	/* First find which edge this spine attaches to. */

	A = position;

	if( NULL == attach_finder )
		attach_finder = &A;

	edge_count   = 0;
	min_distance = NS_FLOAT_MAX;
	actual_edge  = NS_MODEL_EDGE_NIL;

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			{
			///* "Flag" the edge as not being in the list yet. */
			//ns_model_edge_set_data( curr_edge, NS_INT_TO_POINTER( NS_FALSE ), NS_FALSE );

			++edge_count;

			distance = _ns_spines_distance_to_conical_frustum(
							model->spines,
							attach_finder,//&position,
							curr_edge,
							&curr_attach,
							NULL,
							NULL,
							NULL,
							NULL
							);

			if( distance < min_distance )
				{
				min_distance  = distance;
				actual_attach = curr_attach;
				actual_edge   = curr_edge;
				}
			}
      }

	/* This routine cant function if there arent any edges in the model. */
	if( 0 == edge_count )
		{
/*TEMP*/ns_println( "CANT FIND INFO. FOR SPINE... NO EDGES!" );
		goto _NS_MODEL_SPINES_FIND_DTS_EXIT;
		}




/*
	if( max_voxel_distance < min_distance )
		{
/*TEMP*//*ns_println( "SPINE IS TOO FAR FROM THE MODEL! IT should be removed." );
		*do_remove = NS_TRUE;

		goto _NS_MODEL_SPINES_FIND_DTS_EXIT;
		}
*/



	/* Create a bounding box around the attachment point the size of the
		maximum distance to surface setting. */
	ns_model_position_and_radius_to_aabbox(
		&actual_attach,
		max_voxel_distance,
		&box
		);

	ns_spine_set_attach_point( S, &actual_attach );
	ns_model_spine_calc_xyplane_angle( S );


	/* TEMP???????????????????????? */
	this_spine->section = ns_model_edge_get_section( actual_edge );


	if( NS_MODEL_EDGE_SECTION_NONE == this_spine->section )
		{
ns_println( "RUNNING SECTIONING for manual spine addition." );

		ns_model_section( model, NS_MODEL_SPINES_SECTIONING, NULL );
		ns_model_calc_section_lengths( model );

		this_spine->section = ns_model_edge_get_section( actual_edge );
		}

	this_spine->section_length = ns_model_section_length( model, this_spine->section );
	this_spine->order          = ns_model_edge_get_order( actual_edge );

	width  = ns_voxel_buffer_width( &voxel_buffer );
	height = ns_voxel_buffer_height( &voxel_buffer );
	length = ns_voxel_buffer_length( &voxel_buffer );



	/* Create a list of those edges that intersect the bounding box. */
/*
   ns_value_set_float( ns_model_value( model ), 0.0f );

   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge = ns_model_vertex_begin_edges( curr_vertex );
      end_edges = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge, end_edges );
             curr_edge = ns_model_edge_next( curr_edge ) )
			if( ! NS_POINTER_TO_INT( ns_model_edge_get_data( curr_edge ) ) )
				if( ns_spines_conical_frustum_octree_intersect_func( ( nspointer )curr_edge, &box ) )
					{
					if( NS_FAILURE( ns_list_push_back( &edges, ( nspointer )curr_edge ), error ) )
						goto _NS_MODEL_SPINES_FIND_DTS_EXIT;

					// "Flag" the edge as being in the list. This eliminates mirror
					//	edges from also going in the list.
					ns_model_edge_set_data( curr_edge, NS_INT_TO_POINTER( NS_TRUE ), NS_TRUE );
					}
      }
*/



	/* Traverse the list. If the minimum distance to the set of edges is greater
		than the user-defined max D.T.S., then default to an undefined spine. */
	min_distance = NS_FLOAT_MAX;

	//NS_LIST_FOREACH( &edges, iter )
		{
		distance = _ns_spines_distance_to_conical_frustum(
						model->spines,
						&position,
						actual_edge,//ns_list_iter_get_object( iter ),
						NULL,
						NULL,
						NULL,
						NULL,
						NULL
						);

		if( distance < min_distance )
			min_distance = distance;
		}

	if( max_voxel_distance < min_distance )
		{
/*TEMP*/ns_println( "The spine is far from the edge set. Not computing DTS,volume,S.A.,etc..." );
		goto _NS_MODEL_SPINES_FIND_DTS_EXIT;
		}


/*TEMP*///ns_println( "NUMBER OF EDGES IN THE LIST FOR INFO. CALC = " NS_FMT_ULONG, ns_list_size( &edges ) );


	ns_to_image_space( &position, &curr_position, voxel_info );
	curr_distance = min_distance;

	_NS_MODEL_SPINES_FIND_DTS_LOOP:

	max_distance = -NS_FLOAT_MAX;
	max_position = curr_position;

	for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
		{
		ns_vector3i_add( &next_position, &curr_position, voxel_offsets + i );

		if( next_position.x < 0 || width  <= next_position.x ||
			 next_position.y < 0 || height <= next_position.y ||
			 next_position.z < 0 || length <= next_position.z )
			continue;

		if( ns_voxel_get( &voxel_buffer, next_position.x, next_position.y, next_position.z ) < u_threshold )
			continue;

		if( NULL != ( voxel = ns_voxel_table_find( &voxel_table, &next_position ) ) )
			{
			/* NOTE: Using the intensity field of the voxel to hold the distance. */
			next_distance = voxel->intensity;
			}
		else
			{
			ns_to_voxel_space( &next_position, &position, voxel_info );
			min_distance = NS_FLOAT_MAX;

			//NS_LIST_FOREACH( &edges, iter )
				{
				distance = _ns_spines_distance_to_conical_frustum(
								model->spines,
								&position,
								actual_edge,//ns_list_iter_get_object( iter ),
								NULL,
								NULL,
								NULL,
								NULL,
								NULL
								);

				if( distance < min_distance )
					min_distance = distance;
				}

			next_distance = min_distance;

			/* NOTE: Using the intensity field of the voxel to hold the distance. */
			if( NS_FAILURE(
					ns_voxel_table_add(
						&voxel_table,
						&next_position,
						next_distance,
						NULL,
						NULL
						),
					error ) )
				goto _NS_MODEL_SPINES_FIND_DTS_EXIT;
			}

		if( next_distance < max_voxel_distance )
			if( next_distance > max_distance )
				{
				max_distance = next_distance;
				max_position = next_position;
				}
		}

	if( max_distance > curr_distance )
		{
		curr_distance = max_distance;
		curr_position = max_position;

		goto _NS_MODEL_SPINES_FIND_DTS_LOOP;
		}

	this_spine->height = this_spine->max_distance = curr_distance;

/*TEMP*/ns_println( "MANUAL SPINE MAX-DTS is %f", this_spine->max_distance );

	ns_to_voxel_space( &curr_position, &position, voxel_info );
	this_spine->max = position;



	/* Minimum DTS not being calculated, so set it below zero to indicate its invalid. */
	this_spine->min_distance = -1.0f;

	/* Go through the spines and see if the user clicked on this spine already.
		In other words check position of the maximum and see if its already in the
		spine list. */

	curr_spine = ns_model_begin_spines( model );
	end_spines = ns_model_end_spines( model );

	for( ; ns_spine_not_equal( curr_spine, end_spines ); curr_spine = ns_spine_next( curr_spine ) )
		{
		other_spine = ns_list_iter_get_object( curr_spine );

		if( other_spine != this_spine )
			if( NS_FLOAT_EQUAL( other_spine->max.x, this_spine->max.x ) &&
				 NS_FLOAT_EQUAL( other_spine->max.y, this_spine->max.y ) &&
				 NS_FLOAT_EQUAL( other_spine->max.z, this_spine->max.z ) )
				{
/*TEMP*/ns_println( "The spine is a DUPLICATE and should be removed." );
				*do_remove = NS_TRUE;

				goto _NS_MODEL_SPINES_FIND_DTS_EXIT;
				}
		}

   /* The attachment point is on the medial axis. Now find where it
      hits the surface. Use the maximum to attach vector. */
   ns_vector3f_sub( &this_spine->approx_axis, &this_spine->attach, &this_spine->max );
   ns_vector3f_norm( &this_spine->approx_axis );
   ns_vector3f_scale( &V, &this_spine->approx_axis, this_spine->max_distance );
   ns_vector3f_add( &this_spine->surface, &this_spine->max, &V );

	/* Reset axis for rendering. Use the maximum to surface vector. */
	ns_vector3f_sub( &this_spine->approx_axis, &this_spine->surface, &this_spine->max );
   ns_vector3f_norm( &this_spine->approx_axis );

	/* Make the spine base (lower 20% for automatic spines) halfway between the
		center of mass and the attachment on the medial axis. */
	ns_vector3f_add( &this_spine->base, &this_spine->center, &this_spine->attach );
	ns_vector3f_cmpd_scale( &this_spine->base, 0.5f );



	/* Find the attachment point of the spine's maximum. */
	//min_distance = NS_FLOAT_MAX;

	//NS_LIST_FOREACH( &edges, iter )
		//{
		//distance = _ns_spines_distance_to_conical_frustum(
		//				model->spines,
		//				&this_spine->max,
		//				ns_list_iter_get_object( iter ),
		//				&curr_attach,
		//				NULL,
		//				NULL,
		//				NULL,
		//				NULL
		//				);

		//if( distance < min_distance )
		//	{
		//	min_distance  = distance;
		//	actual_attach = curr_attach;
		//	}
		//}


	if( NS_FAILURE(
			_ns_manual_spine_calc_volume_and_surface_area(
				this_spine,
				model->spines,
				settings,
				image,
				f_threshold,
				interp_type,
				kernel_type,
				&actual_attach,
				do_remove
				),
			error ) )
		goto _NS_MODEL_SPINES_FIND_DTS_EXIT;


	_NS_MODEL_SPINES_FIND_DTS_EXIT:

	//ns_list_destruct( &edges );
	ns_voxel_table_destruct( &voxel_table );

/*TEMP*/ns_println( "TIME TO CALC INFO. WAS %f seconds.", ns_difftimer( ns_timer(), start ) );

	return error;
	}


nssize ns_model_num_manual_spines( const NsModel *model )
	{
   nslistiter  curr, end;
	nssize      count;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

	count = 0;
   curr  = ns_list_begin( &(model->spines->list) );
   end   = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		if( ! ns_spine_get_auto_detected( curr ) )
			++count;

	return count;
	}


nssize ns_model_num_auto_spines( const NsModel *model )
	{
   nslistiter  curr, end;
	nssize      count;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

	count = 0;
   curr  = ns_list_begin( &(model->spines->list) );
   end   = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		if( ns_spine_get_auto_detected( curr ) )
			++count;

	return count;
	}


nssize ns_model_num_selected_spines( const NsModel *model )
	{
   nslistiter  curr, end;
	nssize      count;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

	count = 0;
   curr  = ns_list_begin( &(model->spines->list) );
   end   = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		if( ns_spine_is_selected( curr ) )
			++count;

	return count;
	}


nssize ns_model_num_selected_spines_ex( const NsModel *model, nsspine *S )
	{
   nslistiter  curr, end;
	nssize      count;


	ns_assert( NULL != model );
	ns_assert( NULL != model->spines );

	count = 0;
   curr  = ns_list_begin( &(model->spines->list) );
   end   = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		if( ns_spine_is_selected( curr ) )
			{
			++count;
			*S = curr;
			}

	return count;
	}


NsError ns_model_recompute_spines_attachment
	(
	NsModel           *model,
	const NsSettings  *settings,
	NsProgress        *progress
	)
	{
	NsVector        nodes;
   nslistiter      curr, end;
	NsSpine        *spine;
	nsmodelvertex   V;
	nsmodeledge     E;
	NsError         error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
	ns_assert( NULL != settings );

	error = ns_no_error();

   ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

	/* The build octree routine needs a few things set before calling. */
	model->spines->settings             = settings;
	model->spines->voxel_info           = ns_settings_voxel_info( settings );
	model->spines->progress             = progress;
	model->spines->max_octree_recursion = 5; /*TEMP?*/

	if( NS_FAILURE( _ns_spines_build_inflated_model_octree( model->spines ), error ) )
		goto _NS_MODEL_RECOMPUTE_SPINES_ATTACHMENT;

   ns_progress_set_title( progress, "Recomputing spine attachments..." );
   ns_progress_num_iters( progress, ns_model_num_spines( model ) );
   ns_progress_begin( progress );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( NS_FAILURE(
				_ns_model_spine_do_find_attach_vertex_and_edge(
					model->spines,
					spine,
					&nodes,
					&V,
					&E,
					&spine->attach
					),
				error ) )
			goto _NS_MODEL_RECOMPUTE_SPINES_ATTACHMENT;

		ns_progress_next_iter( progress );
		}

	_NS_MODEL_RECOMPUTE_SPINES_ATTACHMENT:

	ns_progress_end( progress );

   ns_vector_destruct( &nodes );
	ns_model_clear_octree( model, NS_MODEL_EDGE_OCTREE );

	return error;
	}


NsError ns_model_recompute_manual_spines
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
	)
	{
   nslistiter          curr, end;
	NsSpine            *spine;
	const NsVoxelInfo  *voxel_info;
	NsVector3f          P1, P2;
	nsfloat             radius;
	nsfloat             length;
	nsboolean           did_create_sample;
	nspointer           sample;
	nsfloat             threshold;
	nsboolean           do_remove;
	NsError             error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );

   ns_progress_set_title( progress, "Recomputing manual spines..." );
   ns_progress_num_iters( progress, ns_model_num_manual_spines( model ) );
   ns_progress_begin( progress );

	voxel_info = ns_settings_voxel_info( settings );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( ! ns_spine_get_auto_detected( curr ) )
			{
			ns_spine_get_position( curr, &P1 );
			P2 = P1;

			if( NS_FAILURE(
					ns_sampler_run(
						sampler,
						image,
						&P1,
						&P2,
						0,
						do_3d_rayburst,
						kernel_type,
						interp_type,
						voxel_info,
						average_intensity,
						jitter,
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
				return error;

			if( did_create_sample )
				{
				ns_assert( 0 < ns_sampler_size( sampler ) );

				sample    = ns_sampler_last( sampler );
				threshold = ns_sample_get_threshold( sample );

				ns_sampler_remove( sampler, sample );

				if( NS_FAILURE(
						ns_model_spines_on_add_manual(
							model,
							settings,
							image,
							threshold,
							interp_type,
							kernel_type,
							curr,
							NULL,
							&do_remove
							),
						error ) )
					return error;
				}

			ns_progress_next_iter( progress );
			}
		}

   ns_progress_end( progress );
	return ns_no_error();
	}



/*
nsboolean ns_model_spines_all_layers_color
   (
   NsModel           *model, 
   const NsVector3i  *V,
   nsshort           *color
   )
   {
   NsVoxel         *voxel;
   const NsSpines  *spines = model->spines;


   if( NULL != ( voxel = ns_voxel_table_find( &spines->voxel_table, V ) ) )
      if( NULL != __DATA( voxel, spine ) )
         {
         *color = _ns_spines_layer_index_to_color_index[ __DATA( voxel, layer_color ) ];
         return NS_TRUE;
         }

   return NS_FALSE;
   }


nsboolean ns_model_spines_render_layer_color
   (
   NsModel           *model, 
   const NsVector3i  *V,
   nsshort           *color
   )
   {
   NsVoxel         *voxel;
   const NsSpines  *spines = model->spines;


   if( NULL != ( voxel = ns_voxel_table_find( &spines->voxel_table, V ) ) )
      if( NULL != __DATA( voxel, spine ) && spines->render_layer == __DATA( voxel, layer_color ) )
         {
         *color = _ns_spines_layer_index_to_color_index[ __DATA( voxel, layer_color ) ];
         return NS_TRUE;
         }

   return NS_FALSE;
   }*/


NS_PRIVATE void _ns_spines_do_create_random_colors( NsColor4ub *colors, nssize count )
   {
   nssize   i;
   nssize   c;
   nssize   min_channel_index;
   nsuint8  max_channel_color;
   nsuint8  color[3];


   for( i = 0; i < count; ++i )
      {
      color[0] = ( nsuint8 )( ns_global_randi() % 256 );
      color[1] = ( nsuint8 )( ns_global_randi() % 256 );
      color[2] = ( nsuint8 )( ns_global_randi() % 256 );

      min_channel_index = 0;
      max_channel_color = color[0];

      for( c = 0; c < 3; ++c )
         {
         if( color[c] < color[ min_channel_index ] )
            min_channel_index = c;

         if( color[c] > max_channel_color )
            max_channel_color = color[c];
         }

      if( max_channel_color < 128 )
         color[ min_channel_index ] = ( nsuint8 )( ns_global_randi() % ( 255 - 128 + 1 ) + 128 );

      colors[i].x = color[0];
      colors[i].y = color[1];
      colors[i].z = color[2];
      colors[i].w = 255;
      }
   }


#define _NS_SPINES_MAX_RANDOM_COLORS  1024

NsColor4ub ____ns_spines_random_colors[ _NS_SPINES_MAX_RANDOM_COLORS ];
nssize     ____ns_spines_num_random_colors = 1;
NS_PRIVATE nsboolean  ____ns_spines_random_color_creation_enabled = NS_TRUE;

void ns_spines_enable_random_color_creation( nsboolean yes_or_no )
	{  ____ns_spines_random_color_creation_enabled = yes_or_no;  }


void ns_spines_create_random_colors( nssize count )
   {
   nssize n;

	if( ! ____ns_spines_random_color_creation_enabled )
		return;

	n = NS_MIN( count, _NS_SPINES_MAX_RANDOM_COLORS );

	if( 0 == n )
		n = 1;


	n = _NS_SPINES_MAX_RANDOM_COLORS;


   //if( ____ns_spines_num_random_colors < n )
      {
		/*TEMP*/ns_println( "GENERATING SPINE RANDOM COLORS!" );

      ____ns_spines_num_random_colors = n;
      _ns_spines_do_create_random_colors( ____ns_spines_random_colors, ____ns_spines_num_random_colors );
      }

	/*TEMP*/
	//____ns_spines_random_colors[0] = NS_COLOR4UB_BLACK;
   }


/* NOTE: ____ns_spines_num_random_colors should not be 0, so we dont get a divide by zero error! */
NsColor4ub ns_spines_random_color( nssize index )
   {  return ____ns_spines_random_colors[ index % ____ns_spines_num_random_colors ];  }


const NsColor4ub* ns_spines_random_colors( void )
   {  return ____ns_spines_random_colors;  }


nssize ns_spines_num_random_colors( void )
   {  return ____ns_spines_num_random_colors;  }


/*
#include <image/nssplats.h>
extern NsSplats ____splats;

#error TODO remove non-spine voxels from hash.

void _ns_spines_do_set_splats_colors( NsSpines *spines, SPLATDEF *DC, SPLATDEF *DE )
	{
	NsVector3i   P;
	NsVoxel     *voxel;


	for( ; DC < DE; ++DC )
		{
		P.x = ( nsint )DC->V.x;
		P.y = ( nsint )DC->V.y;
		P.z = ( nsint )DC->V.z;

		if( NULL != ( voxel = ns_voxel_table_find( &spines->voxel_table, &P ) ) )
			if( NULL != __DATA( voxel, spine ) )
				DC->color = ( nsshort )( __DATA( voxel, spine )->id % ____ns_spines_num_random_colors );
		}
	}

void _ns_spines_set_splats_colors( NsSpines *spines )
	{
	if( NULL != ____splats.voxels_xy )
		_ns_spines_do_set_splats_colors( spines, ____splats.voxels_xy, ( ( SPLATDEF* )____splats.voxels_xy ) + ____splats.size );

	if( NULL != ____splats.voxels_xz )
		_ns_spines_do_set_splats_colors( spines, ____splats.voxels_xz, ( ( SPLATDEF* )____splats.voxels_xz ) + ____splats.size );

	if( NULL != ____splats.voxels_zy )
		_ns_spines_do_set_splats_colors( spines, ____splats.voxels_zy, ( ( SPLATDEF* )____splats.voxels_zy ) + ____splats.size );
	}
*/


void ns_model_spines_retype_by_default_classifier
	(
	NsModel           *model,
	const NsSettings  *settings
	)
	{
   nslistiter   curr, end;
	NsSpine     *spine;


	ns_assert( NULL != model );
	ns_assert( NULL != settings );

	____neck_ratio    = ns_settings_get_classify_neck_ratio( settings );
	____thin_ratio    = ns_settings_get_classify_thin_ratio( settings );
	____mushroom_size = ns_settings_get_classify_mushroom_size( settings );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( spine->auto_detected )
			{
			SetSpineType( &spine->stats );
			spine->type = _ns_spines_convert_type( spine->stats.spine_type );
			}
		}
	}


typedef struct _NsSpineComp
	{
	NsSpine      *spine;
	NsSpineType   orig_type;
	}
	NsSpineComp;

typedef struct _NsSpineSelect
	{
	NsSpineType  orig_type;
	NsSpineType  curr_type;
	fann_type    input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	}
	NsSpineSelect;

NsError ns_model_spines_do_retype_comparison
	(
	NsModel             *model,
	const nschar        *file_name,
	const NsSettings    *settings,
	NsSpinesClassifier  *classifier
	)
	{
	NsSpineComp    *comps;
	NsSpineSelect  *best, *worst;
	nssize          num_total, num_stubby, num_thin, num_mushroom;
	nssize          num_total_correct, num_stubby_correct, num_thin_correct, num_mushroom_correct;
	nssize          num_total_selected, num_stubby_selected, num_thin_selected, num_mushroom_selected;
	nssize          num_total_unselected, num_stubby_unselected, num_thin_unselected, num_mushroom_unselected;
	nssize          count, counter;
	nssize          i, n, r, iter;
   nslistiter      curr, end;
	nsdouble        total_accuracy, sum_total_accuracy, min_total_accuracy, max_total_accuracy, mean_total_accuracy;
	nsdouble        stubby_accuracy, sum_stubby_accuracy, min_stubby_accuracy, max_stubby_accuracy, mean_stubby_accuracy;
	nsdouble        thin_accuracy, sum_thin_accuracy, min_thin_accuracy, max_thin_accuracy, mean_thin_accuracy;
	nsdouble        mushroom_accuracy, sum_mushroom_accuracy, min_mushroom_accuracy, max_mushroom_accuracy, mean_mushroom_accuracy;
	NsFile          file;
	NsError         error;

	//nsfloat percents[] =
	//	{
	//	1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
	//	10.0f, 15.0f, 20.0f, 25.0f, 30.0f, 35.0f, 40.0f, 45.0f, 50.0f,
		//60.0f,
		//70.0f,
		//80.0f,
		//90.0f,
		//100.0f,
	//	-1.0f
	//	};

	ns_assert( NS_SPINE_NUM_TYPES - 1 == ns_spines_classifier_num_classes( classifier ) );

	error = ns_no_error();

	comps = NULL;
	best  = NULL;
	worst = NULL;
	ns_file_construct( &file );

	/* Assure default classification scheme is set. */
	ns_model_spines_retype_by_default_classifier( model, settings );

	/* Allocate and initialize array of spines. Makes finding one at
		a random index much faster than traversing the list. */

	num_total = ns_list_size( &(model->spines->list) );

	if( NULL == ( comps = ns_new_array( NsSpineComp, num_total ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _EXIT;
		}

	num_stubby = num_thin = num_mushroom = 0;

	i    = 0;
	curr = ns_list_begin( &(model->spines->list) );
	end  = ns_list_end( &(model->spines->list) );

	for( ; ns_list_iter_not_equal( curr, end );
			 curr = ns_list_iter_next( curr ) )
		{
		comps[i].spine     = ns_list_iter_get_object( curr );
		comps[i].orig_type = (comps[i].spine)->type;

		switch( comps[i].orig_type )
			{
			case NS_SPINE_STUBBY:
				++num_stubby;
				break;

			case NS_SPINE_THIN:
				++num_thin;
				break;

			case NS_SPINE_MUSHROOM:
				++num_mushroom;
				break;
			}

		++i;
		}

	ns_assert( i == num_total );

	#define _NS_MODEL_SPINES_RETYPE_COMPARISON_MAX_COUNTER  225
	#define _NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS   200

	if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_WRITE ), error ) )
		goto _EXIT;

	if( NS_FAILURE(
			ns_file_print(
				&file,
				"%u\t%u\t%u\n",
				( nsuint )num_stubby,
				( nsuint )num_thin,
				( nsuint )num_mushroom
				),
			error ) )
		return error;

	/*if( NS_FAILURE(
			ns_file_print(
				&file,
				/*"Percent\t*//*"Count\t"
				"MinTotal\tMaxTotal\tMeanTotal\t"
				"MinStubby\tMaxStubby\tMeanStubby\t"
				"MinThin\tMaxThin\tMeanThin\t"
				"MinMushroom\tMaxMushroom\tMeanMushroom\n"
				),
			error ) )
		goto _EXIT;*/

	for( counter = 1;
		counter <= _NS_MODEL_SPINES_RETYPE_COMPARISON_MAX_COUNTER;
		++counter/*p = 0; 0.0f < percents[p]; ++p*/ )
		{
		sum_total_accuracy = sum_stubby_accuracy = sum_thin_accuracy = sum_mushroom_accuracy = 0.0f;
		min_total_accuracy = min_stubby_accuracy = min_thin_accuracy = min_mushroom_accuracy = NS_DOUBLE_MAX;
		max_total_accuracy = max_stubby_accuracy = max_thin_accuracy = max_mushroom_accuracy = -NS_DOUBLE_MAX;

		count = counter;//( nssize )( ( nsfloat )num_total * ( percents[p] / 100.0f ) );

		/* Lets be paranoid and clip the 'count' value. */

		if( 0 == count )
			count = 1;

		if( num_total < count )
			count = num_total;

		/*ns_delete( best );

		if( NULL == ( best = ns_new_array( NsSpineSelect, count ) ) )
			{
			error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			goto _EXIT;
			}

		ns_delete( worst );

		if( NULL == ( worst = ns_new_array( NsSpineSelect, count ) ) )
			{
			error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			goto _EXIT;
			}*/

		for( iter = 0; iter < _NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS; ++iter )
			{
			/* Clear selected flag of all the spines and reset types back to the original ones. */
			for( i = 0; i < num_total; ++i )
				{
				(comps[i].spine)->is_selected = NS_FALSE;
				(comps[i].spine)->type        = comps[i].orig_type;
				}

			/* Randomly pick a certain percentage of the spines. */

			num_stubby_selected   = num_thin_selected   = num_mushroom_selected = 0;
			num_stubby_unselected = num_thin_unselected = num_mushroom_unselected = 0;

			n = 0;

			while( n < count )
				{
				r = ( nssize )ns_global_randi() % num_total;

				/* Dont count a previously picked spine. */
				if( ! (comps[r].spine)->is_selected )
					{
					(comps[r].spine)->is_selected = NS_TRUE;

					switch( comps[r].orig_type )
						{
						case NS_SPINE_STUBBY:
							++num_stubby_selected;
							break;

						case NS_SPINE_THIN:
							++num_thin_selected;
							break;

						case NS_SPINE_MUSHROOM:
							++num_mushroom_selected;
							break;
						}

					++n;
					}
				}

			num_stubby_unselected   = num_stubby   - num_stubby_selected;
			num_thin_unselected     = num_thin     - num_thin_selected;
			num_mushroom_unselected = num_mushroom - num_mushroom_selected;

			ns_assert( num_stubby_selected   + num_stubby_unselected   == num_stubby );
			ns_assert( num_thin_selected     + num_thin_unselected     == num_thin );
			ns_assert( num_mushroom_selected + num_mushroom_unselected == num_mushroom );

			num_total_selected   = num_stubby_selected   + num_thin_selected   + num_mushroom_selected;
			num_total_unselected = num_stubby_unselected + num_thin_unselected + num_mushroom_unselected;

			ns_assert( num_total_selected + num_total_unselected == num_total );

			/*if( NS_FAILURE(
					ns_file_print(
						&file,
						"%u\t%u\t%u\n",
						( nsuint )num_stubby_selected,
						( nsuint )num_thin_selected,
						( nsuint )num_mushroom_selected
						),
					error ) )
				return error;*/

			/* Create a training data set out of the selected spines. */
			ns_spines_classifier_clear_data_and_network( classifier );

			if( NS_FAILURE( ns_model_spines_add_selected_to_classifier( model, classifier ), error ) )
				{
				ns_delete( comps );
				return error;
				}

			/* Run the spines classifier on the spines that were not selected for training. */
			if( NS_FAILURE( ns_model_spines_retype_unselected_by_ann_classifier( model, classifier, NULL ), error ) )
				{
				ns_delete( comps );
				return error;
				}

			/* Now do the actual accuracy comparison. */

			num_total_correct = num_stubby_correct = num_thin_correct = num_mushroom_correct = 0;

			for( i = 0; i < num_total; ++i )
				if( ! (comps[i].spine)->is_selected )
					{
					if( (comps[i].spine)->type == comps[i].orig_type )
						++num_total_correct;

					switch( comps[i].orig_type )
						{
						case NS_SPINE_STUBBY:
							if( (comps[i].spine)->type == comps[i].orig_type )
								++num_stubby_correct;
							break;

						case NS_SPINE_THIN:
							if( (comps[i].spine)->type == comps[i].orig_type )
								++num_thin_correct;
							break;

						case NS_SPINE_MUSHROOM:
							if( (comps[i].spine)->type == comps[i].orig_type )
								++num_mushroom_correct;
							break;
						}
					}

			/*total_accuracy    = ( ( nsdouble )num_total_correct    / ( nsdouble )num_total )    * 100.0;
			stubby_accuracy   = ( ( nsdouble )num_stubby_correct   / ( nsdouble )num_stubby )   * 100.0;
			thin_accuracy     = ( ( nsdouble )num_thin_correct     / ( nsdouble )num_thin )     * 100.0;
			mushroom_accuracy = ( ( nsdouble )num_mushroom_correct / ( nsdouble )num_mushroom ) * 100.0;*/

			total_accuracy    = ( ( nsdouble )num_total_correct    / ( nsdouble )num_total_unselected )    * 100.0;
			stubby_accuracy   = ( ( nsdouble )num_stubby_correct   / ( nsdouble )num_stubby_unselected )   * 100.0;
			thin_accuracy     = ( ( nsdouble )num_thin_correct     / ( nsdouble )num_thin_unselected )     * 100.0;
			mushroom_accuracy = ( ( nsdouble )num_mushroom_correct / ( nsdouble )num_mushroom_unselected ) * 100.0;

			sum_total_accuracy    += total_accuracy;
			sum_stubby_accuracy   += stubby_accuracy;
			sum_thin_accuracy     += thin_accuracy;
			sum_mushroom_accuracy += mushroom_accuracy;

			if( total_accuracy < min_total_accuracy )
				{
				/*n = 0;

				for( i = 0; i < num_total; ++i )
					if( (comps[i].spine)->is_selected )
						{
						ns_assert( n < count );

						worst[n].orig_type = comps[i].orig_type;
						worst[n].curr_type = (comps[i].spine)->type;

						_ns_spine_set_classifier_input( comps[i].spine, worst[n].input );

						++n;
						}

				ns_assert( n == count );*/

				min_total_accuracy = total_accuracy;
				}

			if( stubby_accuracy < min_stubby_accuracy )
				min_stubby_accuracy = stubby_accuracy;

			if( thin_accuracy < min_thin_accuracy )
				min_thin_accuracy = thin_accuracy;

			if( mushroom_accuracy < min_mushroom_accuracy )
				min_mushroom_accuracy = mushroom_accuracy;

			if( max_total_accuracy < total_accuracy )
				{
				/*n = 0;

				for( i = 0; i < num_total; ++i )
					if( (comps[i].spine)->is_selected )
						{
						ns_assert( n < count );

						best[n].orig_type = comps[i].orig_type;
						best[n].curr_type = (comps[i].spine)->type;

						_ns_spine_set_classifier_input( comps[i].spine, best[n].input );

						++n;
						}

				ns_assert( n == count );*/

				max_total_accuracy = total_accuracy;
				}

			if( max_stubby_accuracy < stubby_accuracy )
				max_stubby_accuracy = stubby_accuracy;

			if( max_thin_accuracy < thin_accuracy )
				max_thin_accuracy = thin_accuracy;

			if( max_mushroom_accuracy < mushroom_accuracy )
				max_mushroom_accuracy = mushroom_accuracy;

			ns_print( "." );
			}

		ns_println( "%d done", ( nsint )counter/*" %.1f%% done", percents[p]*/ );

		mean_total_accuracy    = sum_total_accuracy    / ( nsdouble )_NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS;
		mean_stubby_accuracy   = sum_stubby_accuracy   / ( nsdouble )_NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS;
		mean_thin_accuracy     = sum_thin_accuracy     / ( nsdouble )_NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS;
		mean_mushroom_accuracy = sum_mushroom_accuracy / ( nsdouble )_NS_MODEL_SPINES_RETYPE_COMPARISON_ITERATIONS;


		ns_println(
			"%d %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %3f %.3f",
			( nsint )count,
			min_total_accuracy,
			max_total_accuracy,
			mean_total_accuracy,
			min_stubby_accuracy,
			max_stubby_accuracy,
			mean_stubby_accuracy,
			min_thin_accuracy,
			max_thin_accuracy,
			mean_thin_accuracy,
			min_mushroom_accuracy,
			max_mushroom_accuracy,
			mean_mushroom_accuracy
			);


		if( NS_FAILURE(
				ns_file_print(
					&file,
					/*"%.2f\t*/"%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
					//percents[p],
					( nsint )count,
					min_total_accuracy,
					max_total_accuracy,
					mean_total_accuracy,
					min_stubby_accuracy,
					max_stubby_accuracy,
					mean_stubby_accuracy,
					min_thin_accuracy,
					max_thin_accuracy,
					mean_thin_accuracy,
					min_mushroom_accuracy,
					max_mushroom_accuracy,
					mean_mushroom_accuracy
					),
				error ) )
			goto _EXIT;

		/*if( NS_FAILURE( ns_file_print( &file, "\n" ), error ) )
			goto _EXIT;

		for( n = 0; n < count; ++n )
			if( NS_FAILURE(
					ns_file_print(
						&file,
						"%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
						best[n].orig_type,
						best[n].curr_type,
						best[n].input[0],
						best[n].input[1],
						best[n].input[2],
						best[n].input[3],
						best[n].input[4],
						best[n].input[5],
						best[n].input[6]
						),
					error ) )
				goto _EXIT;

		if( NS_FAILURE( ns_file_print( &file, "\n" ), error ) )
			goto _EXIT;

		for( n = 0; n < count; ++n )
			if( NS_FAILURE(
					ns_file_print(
						&file,
						"%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
						worst[n].orig_type,
						worst[n].curr_type,
						worst[n].input[0],
						worst[n].input[1],
						worst[n].input[2],
						worst[n].input[3],
						worst[n].input[4],
						worst[n].input[5],
						worst[n].input[6]
						),
					error ) )
				goto _EXIT;*/
		}

	_EXIT:

	ns_println( "DONE!" );

	ns_delete( comps );
	ns_delete( best );
	ns_delete( worst );
	ns_file_destruct( &file );

	return error;
	}
