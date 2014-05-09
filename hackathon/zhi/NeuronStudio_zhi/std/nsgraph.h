#ifndef __NS_STD_GRAPH_H__
#define __NS_STD_GRAPH_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nshashtable.h>

NS_DECLS_BEGIN

typedef nslistiter  nsgraphvertexiter;
typedef nslistiter  nsgraphedgeiter;

typedef struct _NsGraph
	{
	NsList          vertices;
	nsboolean       is_directed;
	NsFinalizeFunc  vertex_finalize_func;
	NsFinalizeFunc  edge_finalize_func;
	}
	NsGraph;

typedef struct _NsGraphVertex
	{
	NsList      edges;
	NsGraph    *graph;
	nspointer   object;
	nslong      color;
	}
	NsGraphVertex;

typedef struct _NsGraphEdge
	{
	nsgraphvertexiter  src_vertex;
	nsgraphvertexiter  dest_vertex;
	nsgraphedgeiter    mirror_edge;  /* Is NULL if directed graph. */
	nspointer          object;
	nslong             color;
	}
	NsGraphEdge;


#define NS_GRAPH_VERTEX_FOREACH( graph, V )\
	NS_TYPE_FOREACH(\
		(graph),\
		(V),\
		ns_graph_begin_vertices,\
		ns_graph_end_vertices,\
		ns_graph_vertex_iter_next,\
		ns_graph_vertex_iter_not_equal\
		)

#define NS_GRAPH_EDGE_FOREACH( V, E )\
	NS_TYPE_FOREACH(\
		(V),\
		(E),\
		ns_graph_vertex_begin_edges,\
		ns_graph_vertex_end_edges,\
		ns_graph_edge_iter_next,\
		ns_graph_edge_iter_not_equal\
		)


/* Just for readability in the constructor. */
#define NS_GRAPH_DIRECTED    NS_TRUE
#define NS_GRAPH_UNDIRECTED  NS_FALSE


/* Pass NULL for 'vertex_finalize_func' and/or 'edge_finalize_func'
	if not needed. */
NS_IMPEXP void ns_graph_construct
	( 
	NsGraph         *graph,
	nsboolean        is_directed,
	NsFinalizeFunc   vertex_finalize_func,
	NsFinalizeFunc   edge_finalize_func
   );

NS_IMPEXP void ns_graph_destruct( NsGraph *graph );

NS_IMPEXP nsboolean ns_graph_is_directed( const NsGraph *graph );

NS_IMPEXP nssize ns_graph_num_vertices( const NsGraph *graph );

NS_IMPEXP NsList* ns_graph_vertex_list( const NsGraph *graph );

/* NOTE: For undirected graphs, a to-->from edge and a from-->to edge
	are considered one. */
NS_IMPEXP nssize ns_graph_num_edges( const NsGraph *graph );

NS_IMPEXP void ns_graph_clear( NsGraph *graph );

/* Pass NULL for the assign functions if not needed. If an assign
	function is NULL then shallow copies of vertex->object and/or
	edge->object are made instead. 
*/
NS_IMPEXP NsError ns_graph_assign
	(
	NsGraph	      *lhs,
	const NsGraph  *rhs,
	NsAssignFunc 	 vertex_assign_func,
	NsAssignFunc    edge_assign_func
	);

/* The 'lhs' graph adds the 'rhs' graph to itself, BUT the 'rhs'
	graph remains unchaged. IMPORTANT: The source and destination
	graphs must both either be directed or undirected. Also if the
	finalizers for the vertices and edges are not the same a warning
	will be issued. */
NS_IMPEXP NsError ns_graph_add
	(
	NsGraph	      *lhs,
	const NsGraph  *rhs,
	NsAssignFunc 	 vertex_assign_func,
	NsAssignFunc    edge_assign_func
	);

NS_IMPEXP NsFinalizeFunc ns_graph_get_vertex_finalize_func( const NsGraph *graph );
NS_IMPEXP void ns_graph_set_vertex_finalize_func( NsGraph *graph, NsFinalizeFunc vertex_finalize_func );

NS_IMPEXP NsFinalizeFunc ns_graph_get_edge_finalize_func( const NsGraph *graph );
NS_IMPEXP void ns_graph_set_edge_finalize_func( NsGraph *graph, NsFinalizeFunc edge_finalize_func );

/* V can be NULL if you dont need an iterator. */
NS_IMPEXP NsError ns_graph_add_vertex( NsGraph *graph, nspointer object, nsgraphvertexiter *V );

/* If 'clean' is true, all edges are removed that point to V.
	NOTE: 'clean' is ignored if undirected graph. */
NS_IMPEXP void ns_graph_remove_vertex( NsGraph *graph, nsgraphvertexiter V, nsboolean clean );

#define ns_graph_begin_vertices( graph )\
	ns_list_begin( &(graph)->vertices )

#define ns_graph_end_vertices( graph )\
	ns_list_end( &(graph)->vertices )

NS_IMPEXP nsgraphvertexiter ns_graph_rev_begin_vertices( const NsGraph *graph );
NS_IMPEXP nsgraphvertexiter ns_graph_rev_end_vertices( const NsGraph *graph );

NS_IMPEXP nssize ns_graph_vertex_num_edges( const nsgraphvertexiter V );

NS_IMPEXP nslong ns_graph_vertex_get_color( const nsgraphvertexiter V );
NS_IMPEXP void ns_graph_vertex_set_color( nsgraphvertexiter V, nslong color );

NS_IMPEXP void ns_graph_color_vertices( NsGraph *graph, nslong color );
NS_IMPEXP void ns_graph_color_vertices_recursive( nsgraphvertexiter origin, nslong color );

NS_IMPEXP void ns_graph_color_edges( NsGraph *graph, nslong color );

/* Pass NULL for 'E' if iterator not needed. */
NS_IMPEXP NsError ns_graph_vertex_add_dir_edge
	(
	nsgraphvertexiter  from,
	nsgraphvertexiter  to,
	nspointer          object,
	nsgraphedgeiter   *E
	);

/* NOTE: a to-->from edge is also added. Pass NULL for iterators
	if not needed. */
NS_IMPEXP NsError ns_graph_vertex_add_undir_edge
	(
	nsgraphvertexiter   from,
	nsgraphvertexiter   to,
	nspointer           from_to_object,
	nspointer           to_from_object,
	nsgraphedgeiter    *FT,
	nsgraphedgeiter    *TF
	);

/* NOTE: If the graph is undirected them the to-->from edge is also removed. */
NS_IMPEXP void ns_graph_vertex_remove_edge( nsgraphvertexiter V, nsgraphedgeiter E );

NS_IMPEXP nspointer ns_graph_vertex_iter_get_object( const nsgraphvertexiter V );
NS_IMPEXP void ns_graph_vertex_iter_set_object( nsgraphvertexiter V, nspointer object );

#define ns_graph_vertex_begin_edges( V )\
	ns_list_begin( &( ( ( NsGraphVertex* )ns_list_iter_get_object( (V) ) )->edges ) )

#define ns_graph_vertex_end_edges( V )\
	ns_list_end( &( ( ( NsGraphVertex* )ns_list_iter_get_object( (V) ) )->edges ) )

NS_IMPEXP nsgraphvertexiter ns_graph_edge_src_vertex( const nsgraphedgeiter E );
NS_IMPEXP nsgraphvertexiter ns_graph_edge_dest_vertex( const nsgraphedgeiter E );

NS_IMPEXP nslong ns_graph_edge_get_color( const nsgraphedgeiter E );
NS_IMPEXP void ns_graph_edge_set_color( nsgraphedgeiter E, nslong color );

NS_IMPEXP nsgraphedgeiter ns_graph_undir_edge_mirror( const nsgraphedgeiter E );

NS_IMPEXP nspointer ns_graph_edge_iter_get_object( const nsgraphedgeiter E );
NS_IMPEXP void ns_graph_edge_iter_set_object( nsgraphedgeiter E, nspointer object );

NS_IMPEXP NsGraph* ns_graph_vertex_graph( const nsgraphvertexiter V );
NS_IMPEXP NsGraph* ns_graph_edge_graph( const nsgraphedgeiter E );

#define ns_graph_vertex_iter_next( V )\
	ns_list_iter_next( ( V ) )

#define ns_graph_vertex_iter_prev( V )\
	ns_list_iter_prev( ( V ) )

#define ns_graph_vertex_iter_equal( V1, V2 )\
	ns_list_iter_equal( ( V1 ), ( V2 ) )

#define ns_graph_edge_iter_next( E )\
	ns_list_iter_next( ( E ) )

#define ns_graph_edge_iter_prev( E )\
	ns_list_iter_prev( ( E ) )

#define ns_graph_edge_iter_equal( E1, E2 )\
	ns_list_iter_equal( ( E1 ), ( E2 ) )

#define ns_graph_vertex_iter_offset( V, n )\
	ns_list_iter_offset( ( V ), ( n ) )

#define ns_graph_edge_iter_offset( E, n )\
	ns_list_iter_offset( ( E ), ( n ) )

#define ns_graph_vertex_iter_not_equal( V1, V2 )\
	( ! ns_graph_vertex_iter_equal( ( V1 ), ( V2 ) ) )

#define ns_graph_edge_iter_not_equal( E1, E2 )\
	( ! ns_graph_edge_iter_equal( ( E1 ), ( E2 ) ) )


/* Be careful with these as they provide direct access to the
	vertices and edges of the graph data structure. */

#define ns_graph_vertex( V )\
	_ns_graph_vertex( (V) )

#define ns_graph_edge( E )\
	_ns_graph_edge( (E) )


/* Internal. DO NOT USE! */
NS_IMPEXP NsGraphVertex* _ns_graph_vertex( const nsgraphvertexiter V );
NS_IMPEXP NsGraphEdge* _ns_graph_edge( const nsgraphedgeiter E );

NS_DECLS_END

#endif/* __NS_STD_GRAPH_H__ */
