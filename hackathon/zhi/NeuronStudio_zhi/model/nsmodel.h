#ifndef __NS_MODEL_MODEL_H__
#define __NS_MODEL_MODEL_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsgraph.h>
#include <std/nstree.h>
#include <std/nsbytearray.h>
#include <std/nshashtable.h>
#include <std/nsprint.h>
#include <std/nslog.h>
#include <std/nsmutex.h>
#include <std/nsvalue.h>
#include <std/nsstring.h>
#include <std/nsprogress.h>
#include <math/nsaabbox.h>
#include <math/nsfrustum.h>
#include <math/nssphere.h>
#include <math/nsline.h>
#include <math/nsvector.h>
#include <math/nspoint.h>
#include <math/nsoctree.h>
#include <math/nsdragrect.h>
#include <math/nsray.h>
#include <math/nsprojector.h>
#include <image/nsvoxel-info.h>
#include <image/nsimage.h>
#include <image/nscolor.h>
#include <model/nsmodel-decl.h>
#include <render/nsrender.h>
#include <ext/circles.h>

NS_DECLS_BEGIN

enum
   {
   NS_MODEL_VERTEX_UNSEEN = 1,
   NS_MODEL_VERTEX_SEEN
   };

enum
   {
   NS_MODEL_EDGE_UNSEEN = 1,
   NS_MODEL_EDGE_SEEN
   };

enum
   {
   NS_MODEL_COLOR_WHITE = 1,
   NS_MODEL_COLOR_GREY,
   NS_MODEL_COLOR_BLACK
   };

enum
   {
   NS_MODEL_EDGE_PATH_EXIT = 1,
   NS_MODEL_EDGE_PATH_ENTRANCE
   };


#define NS_MODEL_EDGE_ORDER_NONE   0
#define NS_MODEL_EDGE_ORDER_FIRST  1

#define NS_MODEL_EDGE_SECTION_NONE   0
#define NS_MODEL_EDGE_SECTION_FIRST  1


typedef nsgraphvertexiter  nsmodelvertex;
typedef nsgraphedgeiter    nsmodeledge;
typedef nslistiter         nsmodelorigin;
typedef nslistiter         nsmodelseed;
typedef nslistiter         nsmodeldata;
typedef nslistiter         nsmodeltree;


#define NS_MODEL_VERTEX_NIL  NULL
#define NS_MODEL_EDGE_NIL    NULL
#define NS_MODEL_ORIGIN_NIL  NULL
#define NS_MODEL_SEED_NIL    NULL


#define NS_MODEL_VERTEX_FIXED_RADIUS  0.15f


typedef enum
   {
   NS_MODEL_VERTEX_ORIGIN,
   NS_MODEL_VERTEX_EXTERNAL,
   NS_MODEL_VERTEX_JUNCTION,
   NS_MODEL_VERTEX_LINE,

   _NS_MODEL_VERTEX_NUM_TYPES
   }
   NsModelVertexType;

#define NS_MODEL_VERTEX_NUM_TYPES  ( ( nssize )_NS_MODEL_VERTEX_NUM_TYPES )

NS_IMPEXP const nschar* ns_model_vertex_type_to_string( NsModelVertexType type );


#define NS_MODEL_EDGE_OCTREE    0
#define NS_MODEL_VERTEX_OCTREE  1

#define NS_MODEL_NUM_OCTREES  2


#define NS_MODEL_LOCK( model )\
	{ ns_model_lock( (model) ); locked = NS_TRUE; }

#define NS_MODEL_UNLOCK( model )\
	{ ns_model_unlock( (model) ); locked = NS_FALSE; }

#define NS_MODEL_TRY_UNLOCK( model )\
	if( locked )\
		{\
		ns_model_unlock( (model) );\
		locked = NS_FALSE;\
		}


typedef enum
	{
	NS_MODEL_FUNCTION_UNKNOWN,
	NS_MODEL_FUNCTION_SOMA,
	NS_MODEL_FUNCTION_BASAL_DENDRITE,
	NS_MODEL_FUNCTION_APICAL_DENDRITE,
	NS_MODEL_FUNCTION_AXON,

	NS_MODEL_FUNCTION_INVALID,

	_NS_MODEL_FUNCTION_NUM_TYPES
	}
	NsModelFunctionType;

#define NS_MODEL_FUNCTION_NUM_TYPES ( ( nssize )_NS_MODEL_FUNCTION_NUM_TYPES )

NS_IMPEXP const nschar* ns_model_function_type_to_string( NsModelFunctionType type );


/* See also nsmodel-filter.h. */
typedef struct _NsModelFilterParams
	{
	const NsImage  *volume;
	const NsImage  *proj_xy;
	const NsImage  *proj_zy;
	const NsImage  *proj_xz;
	nsdouble        volume_average_intensity;
	nsboolean       grafting_use_2d_sampling;
	nsfloat         grafting_aabbox_scalar;
	nsint           grafting_min_window;
	nssize          reposition_iterations;
	nsenum          rayburst_interp_type;
	}
	NsModelFilterParams;


/* Forward declarations. */
struct _NsSpines;
typedef struct _NsSpines NsSpines;

struct _NsGrafter;
typedef struct _NsGrafter NsGrafter;


struct _NsModel
   {
   NsString            name;
   NsList              origins;
	NsList              seeds;
   NsGraph             graph;
   NsList              data;
   NsByteArray         buffer;
   NsList              conn_comps;
	NsList              trees;
   NsByteArray         section_lengths;
   nssize              auto_iter;
   NsValue             value;
   NsSpines           *spines;
   NsGrafter          *grafter;
   const NsVoxelInfo  *voxel_info;
   nsint               order_type;
   nsint               section_type;
   NsMutex             mutex;
	nsulong             flags;
	nsboolean           ( *edge_filter_func )( nsmodeledge );
	nsboolean           ( *vertex_filter_func )( nsmodelvertex );
   NsAABBox3d          boxes[ NS_MODEL_NUM_OCTREES ];
   NsOctree            octrees[ NS_MODEL_NUM_OCTREES ];
	nsuint              thresh_count;
   nsshort             render_iter;
   };


#define NS_MODEL_VERTEX_NUM_DATA       2
#define NS_MODEL_VERTEX_DATA_RESERVED  0
#define NS_MODEL_VERTEX_DATA_SLOT0     NS_MODEL_VERTEX_DATA_RESERVED
#define NS_MODEL_VERTEX_DATA_SLOT1     1


#define NS_MODEL_VERTEX_NUM_THRESHOLDS     2
#define NS_MODEL_VERTEX_NEURITE_THRESHOLD  0
#define NS_MODEL_VERTEX_AVERAGE_THRESHOLD  1
#define NS_MODEL_VERTEX_SPINE_THRESHOLD    1

#define NS_MODEL_VERTEX_NUM_CONTRASTS     NS_MODEL_VERTEX_NUM_THRESHOLDS
#define NS_MODEL_VERTEX_NEURITE_CONTRAST  NS_MODEL_VERTEX_NEURITE_THRESHOLD
#define NS_MODEL_VERTEX_AVERAGE_CONTRAST  NS_MODEL_VERTEX_AVERAGE_THRESHOLD
#define NS_MODEL_VERTEX_SPINE_CONTRAST    NS_MODEL_VERTEX_SPINE_THRESHOLD


typedef struct _NsModelVertex
   {
   nsulong        id;
   nsuint         file_num;
   NsModel       *model;
   NsVector3f     position;
   //NsVector3f     fwd;
   //NsVector3f     rev;
   nsfloat        radius;
	nsfloat        scoop;
	nsfloat        stretch;
   nsulong        flags;
   nssize         conn_comp;
   nssize         auto_iter;
   nsmodelvertex  next;
	nsenum         func_type;
	//nsfloat        size_ratio;
	nsfloat        soma_distance;
	nsfloat        contrasts[ NS_MODEL_VERTEX_NUM_CONTRASTS ];
   nsfloat        thresholds[ NS_MODEL_VERTEX_NUM_THRESHOLDS ];
   nspointer      data[ NS_MODEL_VERTEX_NUM_DATA ];
   }
   NsModelVertex;


#define NS_MODEL_EDGE_NUM_THRESHOLDS     2
#define NS_MODEL_EDGE_NEURITE_THRESHOLD  0
#define NS_MODEL_EDGE_AVERAGE_THRESHOLD  1

#define NS_MODEL_EDGE_NUM_CONTRASTS     NS_MODEL_EDGE_NUM_THRESHOLDS
#define NS_MODEL_EDGE_NEURITE_CONTRAST  NS_MODEL_EDGE_NEURITE_THRESHOLD
#define NS_MODEL_EDGE_AVERAGE_CONTRAST  NS_MODEL_EDGE_AVERAGE_THRESHOLD

typedef struct _NsModelEdge
   {
   nsulong    section;
	nsulong    flags;
   nspointer  data;
   nsenum     func_type;
	nsfloat    contrasts[ NS_MODEL_EDGE_NUM_CONTRASTS ];
   nsfloat    thresholds[ NS_MODEL_EDGE_NUM_THRESHOLDS ];
   nsushort   order;
   nsshort    render_iter;
   }
   NsModelEdge;


/* Traverse list by using the vertex ...get_list() function. */
typedef struct _NsModelConnComp
   {
   nssize         id;
   nsmodelvertex  origin;
   nsmodelvertex  list;
   }
   NsModelConnComp;


typedef struct _NsModelTree
	{
	nssize               conn_comp;
	nssize               total_index;
	nssize               cell_index;
	nssize               type_index;
	NsModelFunctionType  func_type;
	nsmodelvertex        parent;
	nsmodelvertex        root;
	nsboolean            initialized;
	}
	NsModelTree;


#define NS_MODEL_ORIGIN_FOREACH( model, O )\
   NS_TYPE_FOREACH(\
      (model),\
      (O),\
      ns_model_begin_origins,\
      ns_model_end_origins,\
      ns_model_origin_next,\
      ns_model_origin_not_equal\
      )

#define NS_MODEL_SEED_FOREACH( model, O )\
   NS_TYPE_FOREACH(\
      (model),\
      (O),\
      ns_model_begin_seeds,\
      ns_model_end_seeds,\
      ns_model_seed_next,\
      ns_model_seed_not_equal\
      )

#define NS_MODEL_VERTEX_FOREACH( model, V )\
   NS_TYPE_FOREACH(\
      (model),\
      (V),\
      ns_model_begin_vertices,\
      ns_model_end_vertices,\
      ns_model_vertex_next,\
      ns_model_vertex_not_equal\
      )

#define NS_MODEL_EDGE_FOREACH( V, E )\
   NS_TYPE_FOREACH(\
      (V),\
      (E),\
      ns_model_vertex_begin_edges,\
      ns_model_vertex_end_edges,\
      ns_model_edge_next,\
      ns_model_edge_not_equal\
      )


NS_IMPEXP void ns_model_construct( NsModel *model );
NS_IMPEXP void ns_model_destruct( NsModel *model );

NS_IMPEXP void ns_model_lock( NsModel *model );
NS_IMPEXP nsboolean ns_model_try_lock( NsModel *model );
NS_IMPEXP void ns_model_unlock( NsModel *model );

NS_IMPEXP NsValue* ns_model_value( const NsModel *model );


NS_IMPEXP nsuint ns_model_get_thresh_count( const NsModel *model );
NS_IMPEXP void ns_model_set_thresh_count( NsModel *model, nsuint thresh_count );
NS_IMPEXP void ns_model_up_thresh_count( NsModel *model );

NS_IMPEXP const nschar* ns_model_get_name( const NsModel *model );
NS_IMPEXP NsError ns_model_set_name( NsModel *model, const nschar *name );


/* Computes a centroid... a weighted average based on radius. */
NS_IMPEXP NsVector3f* ns_model_find_center( const NsModel *model, NsVector3f *C );


NS_IMPEXP nsint ns_model_get_order_type( const NsModel *model );
NS_IMPEXP void ns_model_set_order_type( NsModel *model, nsint type );

NS_IMPEXP nsint ns_model_get_section_type( const NsModel *model );
NS_IMPEXP void ns_model_set_section_type( NsModel *model, nsint type );


/* Call to indicate that ns_model_calc_edge_order_centers() and
	ns_model_calc_edge_section_centers() needs to be run. */
NS_IMPEXP void ns_model_needs_edge_centers( NsModel *model );

/* Call to indicate that ns_model_calc_edge_order_centers()
	needs to be run. */
NS_IMPEXP void ns_model_needs_edge_order_centers( NsModel *model );

/* Call to indicate that  ns_model_calc_edge_section_centers()
	needs to be run. */
NS_IMPEXP void ns_model_needs_edge_section_centers( NsModel *model );


/* Returns whether ns_model_calc_edge_order_centers() needs to be run or not.
	If 'true' then it doesnt need to be run, else for 'false' it does. */
NS_IMPEXP nsboolean ns_model_has_edge_order_centers( const NsModel *model );

NS_IMPEXP NsError ns_model_calc_edge_order_centers( NsModel *model );


NS_IMPEXP void ns_model_set_is_filtering( NsModel *model, nsboolean yes_or_no );
NS_IMPEXP nsboolean ns_model_get_is_filtering( const NsModel *model );


/* Returns whether ns_model_calc_edge_section_centers() needs to be run or not.
	If 'true' then it doesnt need to be run, else for 'false' it does. */
NS_IMPEXP nsboolean ns_model_has_edge_section_centers( const NsModel *model );

NS_IMPEXP NsError ns_model_calc_edge_section_centers( NsModel *model );


NS_IMPEXP void ns_model_set_soma_distances( NsModel *model );


NS_IMPEXP void ns_model_resize
	(
	NsModel           *model,
	const NsVector3f  *old_voxel_size,
	const NsVector3f  *new_voxel_size
	);


/* The 'auto_iter' can be used similar to a connected component
   number, but it is only set for those vertices that were not
   manually traced. Manually traced should have an auto number
   of zero while all others a value greater or equal to 1. */
NS_IMPEXP void ns_model_auto_iter_up( NsModel *model );
/* NS_IMPEXP void ns_model_auto_iter_down( NsModel *model ); */

NS_IMPEXP nssize ns_model_auto_iter( const NsModel *model );

NS_IMPEXP nssize ns_model_vertex_get_auto_iter( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_auto_iter( nsmodelvertex V );


NS_IMPEXP NsError ns_model_auto_iter_list_add_by_vertex( NsList *list, nsmodelvertex V );
NS_IMPEXP NsError ns_model_auto_iter_list_add_by_number( NsList *list, nssize auto_iter );


/* lhs = rhs */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_assign( NsModel *lhs, const NsModel *rhs );


/* lhs += rhs */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_merge( NsModel *lhs, const NsModel *rhs );


NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear( NsModel *model );
NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear_non_manually_traced( NsModel *model );

NS_IMPEXP nssize ns_model_num_vertices( const NsModel *model );

NS_IMPEXP NsList* ns_model_vertex_list( const NsModel *model );

NS_IMPEXP const NsVoxelInfo* ns_model_get_voxel_info( const NsModel *model );
NS_IMPEXP void ns_model_set_voxel_info( NsModel *model, const NsVoxelInfo *voxel_info );

/* Returns total number of UNDIRECTED edges. */
NS_IMPEXP nssize ns_model_num_edges( const NsModel *model );

NS_IMPEXP nssize ns_model_num_loops( NsModel *model );

NS_IMPEXP void ns_model_color_vertices( NsModel *model, nslong color );
NS_IMPEXP void ns_model_color_edges( NsModel *model, nslong color );

/* NOTE: Recursive paths are terminated when a vertex having the
   specified 'color' is reached. */
NS_IMPEXP void ns_model_color_vertices_recursive( nsmodelvertex V, nslong color );


/* Keeps going to all connected vertices no matter what their current color is. */
NS_IMPEXP void ns_model_color_all_vertices_recursive( nsmodelvertex V, nslong color );


NS_IMPEXP NsError ns_model_create_conn_comps( NsModel *model );

NS_IMPEXP nssize ns_model_num_conn_comps( const NsModel *model );

NS_IMPEXP nslistiter ns_model_begin_conn_comps( const NsModel *model );
NS_IMPEXP nslistiter ns_model_end_conn_comps( const NsModel *model );


/* Returns the number of connected components. */
NS_IMPEXP nssize ns_model_set_conn_comp_numbers( NsModel *model );

/* NOTE: Marks the vertices as NS_MODEL_VERTEX_SEEN as it traverses. */
NS_IMPEXP void ns_model_vertex_set_conn_comp_number_recursive( nsmodelvertex V, nssize conn_comp );


/* Sets the order/section of all edges to the passed parameter. */
NS_IMPEXP void ns_model_order_edges( NsModel *model, nsushort order );
NS_IMPEXP void ns_model_section_edges( NsModel *model, nsulong section );


NS_IMPEXP nssize ns_model_num_sections( const NsModel *model );

NS_IMPEXP NsError ns_model_calc_section_lengths( NsModel *model );
NS_IMPEXP nsfloat ns_model_section_length( const NsModel *model, nsulong section );

NS_IMPEXP nsdouble ns_model_total_dendritic_length( const NsModel *model );


/* Returns true if any edges are set to NS_MODEL_EDGE_SECTION_NONE. */
NS_IMPEXP nsboolean ns_model_has_unset_sections( const NsModel *model );


/* Pass NULL for 'V' if not needed. */
NS_IMPEXP NsError ns_model_add_vertex( NsModel *model, nsmodelvertex *V );
NS_IMPEXP void ns_model_remove_vertex( NsModel *model, nsmodelvertex V );


/* Merges 'src' vertex and 'dest' vertex into one vertex pointed to by 'dest'.
   NOTE: Any edges of 'src' pointing to 'dest' or neighbors of 'dest' are not
   retained. */
NS_IMPEXP NsError ns_model_merge_vertices
   (
   NsModel       *model,
   nsmodelvertex  dest,
   nsmodelvertex  src
   );

#define ns_model_begin_origins( model )\
   ns_list_begin( &(model)->origins )

#define ns_model_end_origins( model )\
   ns_list_end( &(model)->origins )

/* Returns NS_MODEL_ORIGIN_NIL if not found. */
NS_IMPEXP nsmodelorigin ns_model_origin_by_conn_comp( const NsModel *model, nssize conn_comp );

NS_IMPEXP nssize ns_model_num_origins( const NsModel *model );

/* Pass NULL for 'O' if not needed. */
NS_IMPEXP NsError ns_model_add_origin( NsModel *model, nsmodelvertex V, nsmodelorigin *O );

NS_IMPEXP void ns_model_remove_origin( NsModel *model, nsmodelorigin O );
NS_IMPEXP void ns_model_remove_origin_by_conn_comp( NsModel *model, nssize conn_comp );
NS_IMPEXP void ns_model_remove_origin_by_vertex( NsModel *model, nsmodelvertex V );


/* NOTE: The 'find' vertex functions return NS_MODEL_VERTEX_NIL if not found. */
NS_IMPEXP nsmodelvertex ns_model_find_vertex_by_position( const NsModel *model, const NsVector3f *position );


/* NOTE: The 'find' origin functions return NS_MODEL_ORIGIN_NIL if not found. */
NS_IMPEXP nsmodelorigin ns_model_find_origin_by_conn_comp( const NsModel *model, nssize conn_comp );
NS_IMPEXP nsmodelorigin ns_model_find_origin_by_vertex( const NsModel *model, nsmodelvertex V );


NS_IMPEXP void ns_model_reset_origin( NsModel *model, nsmodelorigin O, nsmodelvertex V );


NS_IMPEXP nssize ns_model_num_seeds( const NsModel *model );

NS_IMPEXP NsError ns_model_add_seed( NsModel *model, nsmodelvertex V, nsmodelseed *S );
NS_IMPEXP void ns_model_remove_seed( NsModel *model, nsmodelseed S );

NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear_seeds( NsModel *model );

NS_IMPEXP void ns_model_reset_seed( NsModel *model, nsmodelseed S, nsmodelvertex V );

#define ns_model_begin_seeds( model )\
   ns_list_begin( &(model)->seeds )

#define ns_model_end_seeds( model )\
   ns_list_end( &(model)->seeds )

#define ns_model_seed_next( S )\
   ns_list_iter_next( ( S ) )

#define ns_model_seed_prev( S )\
   ns_list_iter_prev( ( S ) )

#define ns_model_seed_equal( S1, S2 )\
   ns_list_iter_equal( ( S1 ), ( S2 ) )

#define ns_model_seed_not_equal( S1, S2 )\
   ns_list_iter_not_equal( ( S1 ), ( S2 ) )

#define ns_model_seed_vertex( S )\
   ( ( nsmodelvertex )ns_list_iter_get_object( ( S ) ) )


/* Optimized version. Keeps track of which numbers have been removed already so that
   effort isnt duplicated in traversing the whole vertex list. */
NS_IMPEXP NS_SYNCHRONIZED NsError ns_model_remove_conn_comp_by_auto_iter_list( NsModel *model, const NsList *list );


NS_IMPEXP nssize ns_model_num_conn_comps( const NsModel *model );

NS_IMPEXP NS_SYNCHRONIZED void ns_model_clear_origins( NsModel *model );


NS_IMPEXP void ns_model_deselect_all_hidden_vertices( NsModel *model );


NS_IMPEXP void ns_model_select_vertices( NsModel *model, nsboolean selected );

NS_IMPEXP NsError ns_model_select_vertices_by_aabbox( NsModel *model, const NsAABBox3d *B );

NS_IMPEXP NsError ns_model_select_vertices_by_aabbox_ex
	(
	NsModel           *model,
	const NsAABBox3d  *B,
	nssize            *num_intersections
	);

NS_IMPEXP NsError ns_model_select_vertices_by_projection_onto_drag_rect
	(
	NsModel            *model,
	const NsDragRect   *drag_rect,
	const NsProjector  *proj
	);


/* All vertices of the passed 'type' are selected. All other vertices become deselected. */
NS_IMPEXP void ns_model_select_vertices_by_type( NsModel *model, NsModelVertexType type );

NS_IMPEXP void ns_model_select_vertices_by_function( NsModel *model, NsModelFunctionType type );


NS_IMPEXP nsboolean ns_model_has_any_selected_vertices( const NsModel *model );


/* This function will store up to 'count' many vertices. Returns NS_FALSE
   if the total number of selected vertices in the model is not equal to 'count'. */
NS_IMPEXP nsboolean ns_model_buffer_selected_vertices
   (
   const NsModel  *model,
   nsmodelvertex  *vertices,
   nssize          count
   );

/* Stores all seleted vertices in the list. The finalize callback on the
   list should be NULL. */
NS_IMPEXP NsError ns_model_buffer_selected_vertices_ex
   (
   const NsModel  *model,
   NsList         *list
   );


/* Returns true if any of the vertices and/or edges are marked as "unknown". */
NS_IMPEXP nsboolean ns_model_are_function_types_complete( const NsModel *model );

NS_IMPEXP void ns_model_set_function_type( NsModel *model, NsModelFunctionType func_type );


/* IMPORTANT: The function type's of the model have to be set first to
	create valid trees. */
NS_IMPEXP NsError ns_model_create_trees( NsModel *model );
NS_IMPEXP nssize ns_model_num_trees( const NsModel *model );

NS_IMPEXP nsmodeltree ns_model_begin_trees( const NsModel *model );
NS_IMPEXP nsmodeltree ns_model_end_trees( const NsModel *model );

#define ns_model_tree_equal( T1, T2 )\
	ns_list_iter_equal( (T1), (T2) )

#define ns_model_tree_not_equal( T1, T2 )\
	ns_list_iter_not_equal( (T1), (T2) )

#define ns_model_tree_next( T )\
	ns_list_iter_next( (T) )

#define ns_model_tree_prev( T )\
	ns_list_iter_prev( (T) )

NS_IMPEXP NsModelFunctionType ns_model_tree_function_type( const nsmodeltree T );
NS_IMPEXP nssize ns_model_tree_conn_comp( const nsmodeltree T );

/* The index for all trees of all components. */
NS_IMPEXP nssize ns_model_tree_total_index( const nsmodeltree T );

/* The index for the trees of 1 component. */
NS_IMPEXP nssize ns_model_tree_cell_index( const nsmodeltree T );

/* The index for the trees of its type within 1 component. */
NS_IMPEXP nssize ns_model_tree_type_index( const nsmodeltree T );

NS_IMPEXP nsmodelvertex ns_model_tree_parent( const nsmodeltree T );
NS_IMPEXP nsmodelvertex ns_model_tree_root( const nsmodeltree T );


NS_IMPEXP nssize ns_model_num_external_vertices( const NsModel *model );
NS_IMPEXP nssize ns_model_num_junction_vertices( const NsModel *model );
NS_IMPEXP nssize ns_model_num_line_vertices( const NsModel *model );

#define ns_model_begin_vertices( model )\
   ns_graph_begin_vertices( &(model)->graph )

#define ns_model_end_vertices( model )\
   ns_graph_end_vertices( &(model)->graph )

NS_IMPEXP nsmodelvertex ns_model_rev_begin_vertices( const NsModel *model );
NS_IMPEXP nsmodelvertex ns_model_rev_end_vertices( const NsModel *model );

/* NOTE: 0 based index. */
NS_IMPEXP nsmodelvertex ns_model_vertex_at( const NsModel *model, nssize index );

#define ns_model_origin_next( O )\
   ns_list_iter_next( ( O ) )

#define ns_model_origin_prev( O )\
   ns_list_iter_prev( ( O ) )

#define ns_model_origin_equal( O1, O2 )\
   ns_list_iter_equal( ( O1 ), ( O2 ) )

#define ns_model_origin_not_equal( O1, O2 )\
   ns_list_iter_not_equal( ( O1 ), ( O2 ) )

#define ns_model_origin_vertex( O )\
   ( ( nsmodelvertex )ns_list_iter_get_object( ( O ) ) )

#define ns_model_vertex_next( V )\
   ns_graph_vertex_iter_next( ( V ) )

#define ns_model_vertex_prev( V )\
   ns_graph_vertex_iter_prev( ( V ) )

#define ns_model_vertex_offset( V, n )\
   ns_graph_vertex_iter_offset( ( V ), ( n ) )

#define ns_model_vertex_equal( V1, V2 )\
   ns_graph_vertex_iter_equal( ( V1 ), ( V2 ) )

#define ns_model_vertex_not_equal( V1, V2 )\
   ns_graph_vertex_iter_not_equal( ( V1 ), ( V2 ) )

#define ns_model_vertex_num_edges( V )\
   ns_graph_vertex_num_edges( ( V ) )


NS_IMPEXP NsModelVertexType ns_model_vertex_get_type( const nsmodelvertex V );

NS_IMPEXP nsmodelvertex ns_model_vertex_get_list( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_list( nsmodelvertex V, nsmodelvertex next );

NS_IMPEXP nsulong ns_model_vertex_get_id( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_id( nsmodelvertex V, nsulong id );

NS_IMPEXP nsboolean ns_model_vertex_is_origin( const nsmodelvertex V );
NS_IMPEXP nsboolean ns_model_vertex_is_seed( const nsmodelvertex V );

NS_IMPEXP nsboolean ns_model_vertex_is_selected( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_selected( nsmodelvertex V, nsboolean b );
NS_IMPEXP void ns_model_vertex_toggle_selected( nsmodelvertex V );

NS_IMPEXP nsboolean ns_model_vertex_is_valid( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_valid( nsmodelvertex V, nsboolean b );

NS_IMPEXP nsboolean ns_model_vertex_is_unremovable( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_unremovable( nsmodelvertex V, nsboolean b );

NS_IMPEXP nsboolean ns_model_vertex_is_joinable( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_joinable( nsmodelvertex V, nsboolean b );

NS_IMPEXP void ns_model_mark_joinable( NsModel *model, nsboolean b );


NS_IMPEXP nsboolean ns_model_vertex_is_manually_traced( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_manually_traced( nsmodelvertex V, nsboolean b );

NS_IMPEXP void ns_model_vertex_mark_manually_traced_recursive( nsmodelvertex V, nsboolean b );
NS_IMPEXP void ns_model_mark_manually_traced( NsModel *model, nsboolean b );


NS_IMPEXP nsboolean ns_model_vertex_is_hidden( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_mark_hidden( nsmodelvertex V, nsboolean b );

NS_IMPEXP void ns_model_mark_hidden( NsModel *model, nsboolean b );
NS_IMPEXP void ns_model_mark_selected_vertices_hidden( NsModel *model, nsboolean b );


NS_IMPEXP nsuint ns_model_vertex_get_file_num( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_file_num( nsmodelvertex V, nsuint file_num );
NS_IMPEXP void ns_model_set_file_num( NsModel *model, nsuint file_num );


NS_IMPEXP NsModelFunctionType ns_model_vertex_get_function_type( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_function_type( nsmodelvertex V, NsModelFunctionType func_type );


//NS_IMPEXP nsfloat ns_model_vertex_get_size_ratio( const nsmodelvertex V );
//NS_IMPEXP void ns_model_vertex_set_size_ratio( nsmodelvertex V, nsfloat size_ratio );


/* Returns the distance between any 2 vertices. */
NS_IMPEXP nsfloat ns_model_vertex_distance( const nsmodelvertex V1, const nsmodelvertex V2 );

NS_IMPEXP nsfloat ns_model_vertex_get_soma_distance( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_soma_distance( nsmodelvertex V, nsfloat soma_distance );


NS_IMPEXP void ns_model_set_function_type_recursive( nsmodelvertex V, NsModelFunctionType func_type );


#define ns_model_vertex_is_tip( V )\
   ( ns_model_vertex_num_edges( ( V ) ) == 1 )

#define ns_model_vertex_is_external( V )\
   ( ns_model_vertex_num_edges( ( V ) ) <= 1 )

#define ns_model_vertex_is_junction( V )\
   ( 2 < ns_model_vertex_num_edges( ( V ) ) )

#define ns_model_vertex_is_line( V )\
   ( 2 == ns_model_vertex_num_edges( ( V ) ) )

#define ns_model_vertex_get_color( V )\
   ns_graph_vertex_get_color( ( V ) )

#define ns_model_vertex_set_color( V, color )\
   ns_graph_vertex_set_color( ( V ), ( color ) )

NS_IMPEXP NsModel* ns_model_vertex_model( const nsmodelvertex V );

NS_IMPEXP nssize ns_model_vertex_get_conn_comp( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_conn_comp( nsmodelvertex V, nssize conn_comp );

/* Returns 'P' */
NS_IMPEXP NsVector3f* ns_model_vertex_get_position( const nsmodelvertex V, NsVector3f *P );
NS_IMPEXP NsVector3d* ns_model_vertex_get_position_ex( const nsmodelvertex V, NsVector3d *P );

NS_IMPEXP void ns_model_vertex_set_position( nsmodelvertex V, const NsVector3f *P );

NS_IMPEXP NsSpheref* ns_model_vertex_to_sphere( const nsmodelvertex V, NsSpheref *sphere );
NS_IMPEXP NsSphered* ns_model_vertex_to_sphere_ex( const nsmodelvertex V, NsSphered *sphere );

NS_IMPEXP nsfloat ns_model_vertex_get_position_x( const nsmodelvertex V );
NS_IMPEXP nsfloat ns_model_vertex_get_position_y( const nsmodelvertex V );
NS_IMPEXP nsfloat ns_model_vertex_get_position_z( const nsmodelvertex V );

NS_IMPEXP void ns_model_vertex_set_position_x( nsmodelvertex V, nsfloat x );
NS_IMPEXP void ns_model_vertex_set_position_y( nsmodelvertex V, nsfloat y );
NS_IMPEXP void ns_model_vertex_set_position_z( nsmodelvertex V, nsfloat z );

NS_IMPEXP nsfloat ns_model_vertex_get_radius( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_radius( nsmodelvertex V, nsfloat radius );

NS_IMPEXP nsfloat ns_model_vertex_get_scoop( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_scoop( nsmodelvertex V, nsfloat scoop );

NS_IMPEXP nsfloat ns_model_vertex_get_stretch( const nsmodelvertex V );
NS_IMPEXP void ns_model_vertex_set_stretch( nsmodelvertex V, nsfloat stretch );

NS_IMPEXP nsfloat ns_model_vertex_get_threshold( const nsmodelvertex V, nssize which );
NS_IMPEXP void ns_model_vertex_set_threshold( nsmodelvertex V, nssize which, nsfloat threshold );

NS_IMPEXP nsfloat ns_model_vertex_get_contrast( const nsmodelvertex V, nssize which );
NS_IMPEXP void ns_model_vertex_set_contrast( nsmodelvertex V, nssize which, nsfloat contrast );

NS_IMPEXP nspointer ns_model_vertex_get_data( const nsmodelvertex V, nssize index );
NS_IMPEXP void ns_model_vertex_set_data( nsmodelvertex V, nssize index, nspointer data );

/* Pass NULL for 'FT' and/or 'TF' if not needed. */
NS_IMPEXP NsError ns_model_vertex_add_edge
   (
   nsmodelvertex   from,
   nsmodelvertex   to,
   nsmodeledge    *FT,
   nsmodeledge    *TF
   );

NS_IMPEXP void ns_model_vertex_remove_edge( nsmodelvertex V, nsmodeledge E );

/* Returns ns_model_vertex_end_edges() if not found. */
NS_IMPEXP nsmodeledge ns_model_vertex_find_edge( const nsmodelvertex from, const nsmodelvertex to );

NS_IMPEXP nsboolean ns_model_vertex_edge_exists( const nsmodelvertex from, const nsmodelvertex to );

NS_IMPEXP void ns_model_vertex_transpose_edges( nsmodelvertex V );

/* NOTE: 0 based index. */
NS_IMPEXP nsmodeledge ns_model_vertex_edge_at( const nsmodelvertex V, nssize index );

#define ns_model_vertex_begin_edges( V )\
   ns_graph_vertex_begin_edges( ( V ) )

#define ns_model_vertex_end_edges( V )\
   ns_graph_vertex_end_edges( ( V ) )

#define ns_model_edge_next( E )\
   ns_graph_edge_iter_next( ( E ) )

#define ns_model_edge_prev( E )\
   ns_graph_edge_iter_prev( ( E ) )

#define ns_model_edge_offset( E, n )\
   ns_graph_edge_iter_offset( ( E ), ( n ) )

#define ns_model_edge_equal( E1, E2 )\
   ns_graph_edge_iter_equal( ( E1 ), ( E2 ) )

#define ns_model_edge_not_equal( E1, E2 )\
   ns_graph_edge_iter_not_equal( ( E1 ), ( E2 ) )

#define ns_model_edge_src_vertex( E )\
   ns_graph_edge_src_vertex( ( E ) )

#define ns_model_edge_dest_vertex( E )\
   ns_graph_edge_dest_vertex( ( E ) )

#define ns_model_edge_mirror( E )\
   ns_graph_undir_edge_mirror( ( E ) )

NS_IMPEXP nsfloat ns_model_edge_length( const nsmodeledge E );

NS_IMPEXP nslong ns_model_edge_get_color( const nsmodeledge E );
/* NOTE: The edge's mirror color is set as well if 'mirrored' is true. */
NS_IMPEXP void ns_model_edge_set_color( nsmodeledge E, nslong color, nsboolean mirrored );

NS_IMPEXP nsushort ns_model_edge_get_order( const nsmodeledge E );
/* NOTE: The edge's mirror order is set as well if 'mirrored' is true. */
NS_IMPEXP void ns_model_edge_set_order( nsmodeledge E, nsushort order, nsboolean mirrored );


NS_IMPEXP nsboolean ns_model_edge_is_order_center( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_mark_order_center( nsmodeledge E, nsboolean b, nsboolean mirrored );

NS_IMPEXP nsboolean ns_model_edge_is_section_center( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_mark_section_center( nsmodeledge E, nsboolean b, nsboolean mirrored );

NS_IMPEXP nsboolean ns_model_edge_is_unremovable( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_mark_unremovable( nsmodeledge E, nsboolean b, nsboolean mirrored );


NS_IMPEXP nsulong ns_model_edge_get_section( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_set_section( nsmodeledge E, nsulong section, nsboolean mirrored );

NS_IMPEXP nspointer ns_model_edge_get_data( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_set_data( nsmodeledge E, nspointer data, nsboolean mirrored );

NS_IMPEXP NsConicalFrustumd* ns_model_edge_to_conical_frustum( const nsmodeledge E, NsConicalFrustumd *frustum );

/* Returns the angle in degrees. */
NS_IMPEXP nsdouble ns_model_edge_angle_between( const nsmodeledge E1, const nsmodeledge E2 );


NS_IMPEXP NsModelFunctionType ns_model_edge_get_function_type( const nsmodeledge E );
NS_IMPEXP void ns_model_edge_set_function_type( nsmodeledge E, NsModelFunctionType func_type, nsboolean mirrored );

NS_IMPEXP nsfloat ns_model_edge_get_threshold( const nsmodeledge E, nssize which );
NS_IMPEXP void ns_model_edge_set_threshold( nsmodeledge E, nssize which, nsfloat threshold, nsboolean mirrored );

NS_IMPEXP nsfloat ns_model_edge_get_contrast( const nsmodeledge E, nssize which );
NS_IMPEXP void ns_model_edge_set_contrast( nsmodeledge E, nssize which, nsfloat contrast, nsboolean mirrored );


#define ns_model_edge_get_prune_length( E )\
	ns_model_edge_get_threshold( (E), NS_MODEL_EDGE_AVERAGE_THRESHOLD )

#define ns_model_edge_set_prune_length( E, length, mirrored )\
	ns_model_edge_set_threshold( (E), NS_MODEL_EDGE_AVERAGE_THRESHOLD, (length), (mirrored) )


/* NOTE: The objects data destructor is initially NULL. */
NS_IMPEXP NsFinalizeFunc ns_model_get_data_finalize_func( const NsModel *model );
NS_IMPEXP void ns_model_set_data_finalize_func( NsModel *model, NsFinalizeFunc finalize_func );

/* Pass NULL for 'D' if not needed. */
NS_IMPEXP NsError ns_model_add_data( NsModel *model, nspointer data, nsmodeldata *D );
NS_IMPEXP void ns_model_remove_data( NsModel *model, nsmodeldata D );

NS_IMPEXP nssize ns_model_data_size( const NsModel *model );

NS_IMPEXP nsmodeldata ns_model_begin_data( NsModel *model );
NS_IMPEXP nsmodeldata ns_model_end_data( NsModel *model );

#define ns_model_data_next( D )\
   ns_list_iter_next( ( D ) )

#define ns_model_data_prev( D )\
   ns_list_iter_prev( ( D ) )

#define ns_model_data_equal( D1, D2 )\
   ns_list_iter_equal( ( D1 ), ( D2 ) )

#define ns_model_data_not_equal( D1, D2 )\
   ns_list_iter_not_equal( ( D1 ), ( D2 ) )

#define ns_model_data_get_object( D )\
   ns_list_iter_get_object( ( D ) )

#define ns_model_data_set_object( D, data )\
   ns_list_iter_set_object( ( D ), ( data ) )

NS_IMPEXP void ns_model_clear_data( NsModel *model );

NS_IMPEXP nsuint8* ns_model_begin_buffer( const NsModel *model );
NS_IMPEXP nsuint8* ns_model_end_buffer( const NsModel *model );

NS_IMPEXP nssize ns_model_buffer_size( const NsModel *model );

NS_IMPEXP NsError ns_model_resize_buffer( NsModel *model, nssize bytes );
NS_IMPEXP void ns_model_clear_buffer( NsModel *model );


NS_IMPEXP void ns_model_init_voxel_bounds
   (
   NsVector3f  *min,
   NsVector3f  *max
   );

NS_IMPEXP void ns_model_update_voxel_bounds
   (
   NsVector3f        *min,
   NsVector3f        *max,
   const NsVector3f  *V,
   nsfloat            offset
   );


/* IMPORTANT: The coordinate system for the model is rotated
   180 degrees around the X-axis! Therefore for the origin of
   the bounding box we want the minimum x coordinate, but the
   maximum y and z coordinate.

        AABBox:                              Model;

          \ +Y                                / +Z
           \                                 /
            \_____                          /____
            /    /\                        /    /\
    (1)--> /____/  \______ +X      (2)--> /____/__\______ +X 
           \    \  /                      \    \  /
            \____\/                        \____\/      
            /                               \
           /                                 \
          / +Z                                \ +Y


   (1) = the origin of the bounding box
   (2) = the origin of the model volume
*/

/* See also above rotation. */
NS_IMPEXP void ns_model_voxel_bounds_to_aabbox
   (
   const NsVector3f  *min,
   const NsVector3f  *max,
   NsAABBox3d        *box
   );


NS_IMPEXP void ns_model_voxel_bounds_to_aabbox_ex
   (
   const NsVector3i   *min,
   const NsVector3i   *max,
	const NsVoxelInfo  *voxel_info,
   NsAABBox3d         *box
   );



/* See also above rotation. */
NS_IMPEXP void ns_model_image_space_drag_rect_to_aabbox
   (
   const NsDragRect   *drag_rect,
   const NsImage      *image,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
   NsAABBox3d         *B
   );

NS_IMPEXP void ns_model_client_space_drag_rect_to_aabbox
   (
   const NsDragRect   *drag_rect,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
	const NsPoint2i    *corner,
	nsfloat             zoom,
   NsAABBox3d         *B
   );


/* See also above rotation. */
NS_IMPEXP NsAABBox3d* ns_model_position_and_radius_to_aabbox
   (
   const NsVector3f  *P,
   nsfloat            radius,
   NsAABBox3d        *bbox
   );

NS_IMPEXP NsAABBox3d* ns_model_position_and_radii_to_aabbox
   (
   const NsVector3f  *P,
   const NsVector3f  *R,
   NsAABBox3d        *bbox
   );

NS_IMPEXP NsAABBox3d* ns_model_position_and_radius_to_aabbox_ex
   (
   const NsVector3d  *P,
   nsdouble           radius,
   NsAABBox3d        *bbox
   );

/* See also above rotation. */
NS_IMPEXP NsAABBox3d* ns_model_vertex_to_aabbox_ex( const nsmodelvertex V, NsAABBox3d *bbox );

/* See also above rotation. */
NS_IMPEXP NsError ns_model_build_edge_octree_ex
   (
   NsModel                *model,
   nssize                  min_node_objects,
   nssize                  max_recursion,
   nssize                  init_node_capacity,
   NsOctreeIntersectFunc   intersect_func,
   nsdouble                aabbox_increment,
	nsdouble                min_aabbox_size,
   NsProgress             *progress
   );

/* See also above rotation. */
NS_IMPEXP NsError ns_model_build_vertex_octree_ex
   (
   NsModel                *model,
   nssize                  min_node_objects,
   nssize                  max_recursion,
   nssize                  init_node_capacity,
   NsOctreeIntersectFunc   intersect_func,
   nsdouble                aabbox_increment,
   NsProgress             *progress
   );


/* The 'filter_func' allows for finer control over which edges or
	vertices go into the octree. Can be NULL if not necessary. */

NS_IMPEXP NsError ns_model_build_edge_octree
   (
	NsModel           *model,
	const NsAABBox3d  *bbox,
	nssize             min_node_objects,
	nssize             max_recursion,
	nssize             init_node_capacity,
	nsboolean          ( *filter_func )( nsmodeledge ),
	NsProgress        *progress
   );

NS_IMPEXP NsError ns_model_build_vertex_octree
   (
	NsModel           *model,
	const NsAABBox3d  *bbox,
	nssize             min_node_objects,
	nssize             max_recursion,
	nssize             init_node_capacity,
	nsboolean          ( *filter_func )( nsmodelvertex ),
	NsProgress        *progress
   );


NS_IMPEXP void ns_model_clear_octree( NsModel *model, nsint which );
NS_IMPEXP void ns_model_clear_octrees( NsModel *model );


NS_IMPEXP nssize ns_model_octree_size( const NsModel *model, nsint which );
NS_IMPEXP nsboolean ns_model_octree_is_empty( const NsModel *model, nsint which );

NS_IMPEXP NsError ns_model_octree_add( NsModel *model, nsint which, nspointer object );


#define NS_MODEL_OCTREE_THRESHOLD  0

NS_IMPEXP void ns_model_octree_calc_thresholds_and_contrasts
	(
	NsModel           *model,
	const NsSettings  *settings,
	nsint              which,
	nsboolean          include_edges
	);


NS_IMPEXP const NsOctreeNode* ns_model_octree_root( const NsModel *model, nsint which );

NS_IMPEXP NsError ns_model_octree_intersections
   (
   const NsModel          *model,
   nsint                   which,
   nspointer               object,
   NsOctreeIntersectFunc   intersect_func,
   NsVector               *nodes /* stores const NsOctreeNode* */
   );

/* The 'user_data' is passed as the second parameter to
   the 'render_func'. */
NS_IMPEXP void ns_model_render_octree
   (
   const NsModel  *model,
   nsint           which,
   void            ( *render_func )( const NsOctreeNode*, nspointer ),
	void            ( *set_color )( nsfloat r, nsfloat g, nsfloat b, nspointer ),
   nspointer       user_data
   );

NS_IMPEXP void ns_model_octree_stats( const NsModel *model, nsint which, NsOctreeStats *stats );

NS_IMPEXP const NsAABBox3d* ns_model_octree_aabbox( const NsModel *model, nsint which );


/* This formula assumes that 'factor' is e.g. 0.90 if spine correction factor
	was set at 10%. */
#define ns_model_scale_threshold( threshold, contrast, factor )\
   ( (threshold) - ( 1.0f - (factor) ) * (contrast) )



/* Careful with these! */
NS_IMPEXP void ns_model_vertex_get( const nsmodelvertex V, NsModelVertex *vertex );
NS_IMPEXP void ns_model_vertex_set( nsmodelvertex V, const NsModelVertex *vertex );
NS_IMPEXP void ns_model_edge_get( const nsmodeledge E, NsModelEdge *edge );
NS_IMPEXP void ns_model_edge_set( nsmodeledge E, const NsModelEdge *edge );


NS_IMPEXP NS_INTERNAL NsModelVertex* ____ns_model_vertex( nsmodelvertex V, const nschar *details );
NS_IMPEXP NS_INTERNAL NsModelEdge* ____ns_model_edge( nsmodeledge E, const nschar *details );

#ifdef NS_DEBUG

#define ns_model_vertex( V )\
   ____ns_model_vertex( (V), NS_MODULE )

#define ns_model_edge( E )\
   ____ns_model_edge( (E), NS_MODULE )

#else

#define ns_model_vertex( V )\
   ____ns_model_vertex( (V), NULL )

#define ns_model_edge( E )\
   ____ns_model_edge( (E), NULL )

#endif/* NS_DEBUG */


typedef struct _NsModelInfo
   {
   nssize  num_vertices;
   nssize  num_origin_vertices;
   nssize  num_external_vertices;
   nssize  num_line_vertices;
   nssize  num_junction_vertices;
   nssize  num_edges;
	nssize  num_soma_vertices;
	nssize  num_basal_vertices;
	nssize  num_apical_vertices;
	nssize  num_axon_vertices;
	nssize  num_unknown_vertices;
	nssize  num_trees;
	nssize  num_basal_trees;
	nssize  num_apical_trees;
	nssize  num_axon_trees;
	nssize  num_unknown_trees;
	nssize  num_invalid_trees;
   }
   NsModelInfo;

#define NS_MODEL_INFO_VERTICES  0x0001
#define NS_MODEL_INFO_EDGES     0x0002
#define NS_MODEL_INFO_TREES     0x0004

NS_IMPEXP void ns_model_info
	(
	const NsModel  *model,
	NsModelInfo    *info,
	nsuint          flags
	);


NS_IMPEXP NsError ns_model_compute_z_spread_divisor
	(
	NsModel           *model,
	const NsSettings  *settings,
	const NsImage     *image,
	const NsCubei     *roi,
	nsboolean          use_2d_sampling,
	nsfloat            aabbox_scalar,
	nsint              min_window,
	nsfloat           *z_spread_divisor,
	NsProgress        *progress
	);


NS_IMPEXP nsboolean ns_model_vertex_intersects_ray
   (
   const nsmodelvertex   V,
   const NsRay3f        *R
   );

/* This picks the closest intersecting vertex, if there
   is one, to the origin of the ray. */
NS_IMPEXP nsboolean ns_model_vertices_intersect_ray
   (
   const NsModel  *model,
   const NsRay3f  *ray,
   nsmodelvertex  *which
   );


NS_IMPEXP void ns_model_render_vertices_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_vertices_fixed_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_vertex_list_spheres
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_vertices_points
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_vertex_list_points
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_vertices_selected
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_edges_lines
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_edges_frustums
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_edges_selected
   (
   const NsModel  *model,
   NsRenderState  *state
   );

/* 'msh' means maximum spine height */
NS_IMPEXP void ns_model_render_msh_zone
   (
   const NsModel  *model,
   NsRenderState  *state
   );

NS_IMPEXP void ns_model_render_edge_order_centers
   (
   const NsModel  *model,
	void            ( *callback )( const NsVector3f*, nsulong, nspointer ),
	nspointer       user_data
   );

NS_IMPEXP void ns_model_render_edge_section_centers
   (
   const NsModel  *model,
	void            ( *callback )( const NsVector3f*, nsulong, nspointer ),
	nspointer       user_data
   );


NS_IMPEXP NsError ns_model_offset_xy_random( NsModel *model, nsfloat max_xy_offset );

NS_IMPEXP nsdouble ns_model_calc_medial_axis_error_2d( NsModel *new_model, NsModel *ref_model );
NS_IMPEXP nsdouble ns_model_calc_medial_axis_error_3d( NsModel *new_model, NsModel *ref_model );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_H__ */
