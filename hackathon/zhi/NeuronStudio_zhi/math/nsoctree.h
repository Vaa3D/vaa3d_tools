#ifndef __NS_MATH_OCTREE_H__
#define __NS_MATH_OCTREE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsvector.h>
#include <std/nsvalue.h>
#include <math/nsaabbox.h>

NS_DECLS_BEGIN

/*  Octants:
	                    ________________________
                      /           /           /\
                     /           /           /  \
                    /     5     /     6     /    \
                   /           /           /      \
                  /___________/___________/    6   \ 
                 /           /           /\        /\              
                /           /           /  \      /  \            \ +Y
               /     1     /     2     /    \    /    \            \
              /           /           /      \  /      \            \
             @___________/___________/    2   \/    8   \            \__
             \           \           \        /\        /            /\_\__________ +X
              \           \           \      /  \      /             \/_/
               \     1     \     2     \    /    \    /              /
                \           \           \  /      \  /              /
                 \___________\___________\/        \/              /
                  \           \           \    4   /              / +Z
                   \           \           \      /  
                    \     3     \     4     \    /
                     \           \           \  /
                      \___________\___________\/


	@ = origin(minimum) of the bounding box (right-handed coordinate system)

	1 = left top near
	2 = right top near
	3 = left bottom near
	4 = right bottom near
	5 = left top far
	6 = right top far
	7 = left bottom far (not shown)
	8 = right bottom far
*/

typedef enum
	{
	NS_OCTANT_LEFT_TOP_NEAR,
	NS_OCTANT_RIGHT_TOP_NEAR,
	NS_OCTANT_LEFT_BOTTOM_NEAR,
	NS_OCTANT_RIGHT_BOTTOM_NEAR,
	NS_OCTANT_LEFT_TOP_FAR,
	NS_OCTANT_RIGHT_TOP_FAR,
	NS_OCTANT_LEFT_BOTTOM_FAR,
	NS_OCTANT_RIGHT_BOTTOM_FAR
	}
	NsOctantType;


struct _NsOctreeNode;
typedef struct _NsOctreeNode NsOctreeNode;

#define NS_OCTREE_NODE_NUM_VALUES  2

struct _NsOctreeNode
	{
	NsVector       objects;
	NsAABBox3d     box;
	NsValue        values[ NS_OCTREE_NODE_NUM_VALUES ];
	NsOctreeNode*  children[ 8 ];
	};

#define NS_OCTREE_NODE_SIZE  sizeof( NsOctreeNode )


typedef nsboolean ( *NsOctreeIntersectFunc )( nsconstpointer, const NsAABBox3d* );


typedef struct _NsOctree
	{
   NsOctreeNode           *root;
	NsAABBox3d              root_box;
	nssize                  num_objects;
	nssize                  min_node_objects;
	nssize                  max_recursion;
	nssize                  init_node_capacity;
	nsdouble                min_aabbox_size;
	NsOctreeIntersectFunc   intersect_func;
	NsList                  list;
	}
	NsOctree;


NS_IMPEXP void ns_octree_construct( NsOctree *octree );
NS_IMPEXP void ns_octree_destruct( NsOctree *octree );

/* IMPORTANT: Variables that control the construction of the octree.

	root_box: The size of the octree's root node bounding box.

	min_node_objects:    The minimum number of objects that intersect
							   an octree node where recursive division of
							   that node can proceed.

	max_recursion      : The maximum recursive division of the octree.
							   i.e. the smallest size any octree node can
							   reach.

	init_node_capacity : The initial capacity for a nodes object vector.

	min_aabbox_size : Another paramter to control sub-division. This one
                     sets the minimum size that a node can have. Recursive
                     subdivision terminates on any node that falls below
                     this value, no matter what.

	intersect_func: Intersect function for objects and octree nodes. This
						 function can vector based on object type or call some
						 virtual method.

	NOTE: Recursive division will obviously NEVER proceed into
			a bounding box where 0 objects intersect it.

	NOTE: if max_recursion == 0 then the octree is just one big
	      bounding box.
*/

NS_IMPEXP void ns_octree_get_root_aabbox( const NsOctree *octree, NsAABBox3d *root_box );
NS_IMPEXP void ns_octree_set_root_aabbox( NsOctree *octree, const NsAABBox3d *root_box );

NS_IMPEXP nssize ns_octree_get_min_node_objects( const NsOctree *octree );
NS_IMPEXP void ns_octree_set_min_node_objects( NsOctree *octree, nssize min_node_objects );


#define NS_OCTREE_INFINITE_RECURSION  NS_SIZE_MAX

NS_IMPEXP nssize ns_octree_get_max_recursion( const NsOctree *octree );
NS_IMPEXP void ns_octree_set_max_recursion( NsOctree *octree, nssize max_recursion );


NS_IMPEXP nssize ns_octree_get_init_node_capacity( const NsOctree *octree );
NS_IMPEXP void ns_octree_set_init_node_capacity( NsOctree *octree, nssize init_node_capacity );

NS_IMPEXP nsdouble ns_octree_get_min_aabbox_size( const NsOctree *octree );
NS_IMPEXP void ns_octree_set_min_aabbox_size( NsOctree *octree, nsdouble min_aabbox_size );

NS_IMPEXP NsOctreeIntersectFunc ns_octree_get_intersect_func( const NsOctree *octree );
NS_IMPEXP void ns_octree_set_intersect_func( NsOctree *octree, NsOctreeIntersectFunc intersect_func );


/* IMPORTANT: Control parameters must be set before calling!
	Adds an object to the octree. The octree is dynamically adjusted
	according to the control parameters. */
NS_IMPEXP NsError ns_octree_add( NsOctree *octree, nspointer object, nsboolean *was_added );

/* Traverses the octree to the set of intersecting leaf nodes using the intersection function.
	The 'equal_func' is applied to each object at the leaf node. If found, the 'object' is removed
	and the octree is dynamically adjusted. Specifically if the combined number of objects of all the
	siblings of the leaf node (i.e. all 8 children of the parent of the node) is less than or equal to
	the 'min_node_objects' setting, the 8 children nodes are removed and the parent becomes a leaf node
	holding the objects. 'was_removed' can be NULL if not necessary.

	IMPORTANT: Like the 'intersect_func' the 'equal_func' must handle multiple object types.
	i.e. it can switch based on type or call some virtual method.
*/ 
NS_IMPEXP NsError ns_octree_remove
	(
	NsOctree     *octree,
	nspointer     object,
	NsEqualFunc   equal_func,
	nsboolean    *was_removed
	);


/* Replaces the current octree with a new one but applying the
	passed control parameters. The set of objects in the new tree
	is a subset of the old octree (could be equal). */
NS_IMPEXP NsError ns_octree_restructure
	(
	NsOctree               *octree,
	const NsAABBox3d       *root_box,
	nssize                  min_node_objects,
	nssize                  max_recursion,
	nssize                  init_node_capacity,
	nsdouble                min_aabbox_size,
	NsOctreeIntersectFunc   intersect_func
	);


NS_IMPEXP void ns_octree_clear( NsOctree *octree );

/* Stores const NsOctreeNode* in the vector. NOTE: Only leaf
	nodes are returned. */
NS_IMPEXP NsError ns_octree_intersections
	(
	const NsOctree  *octree,
	nspointer        object,
	NsVector        *nodes
	);

NS_IMPEXP nssize ns_octree_num_objects( const NsOctree *octree );

NS_IMPEXP const NsOctreeNode* ns_octree_root( const NsOctree *octree );

NS_IMPEXP NsValue* ns_octree_node_value( const NsOctreeNode *node, nssize which );

NS_IMPEXP nsboolean ns_octree_node_is_leaf( const NsOctreeNode *node );
NS_IMPEXP nsboolean ns_octree_node_is_internal( const NsOctreeNode *node );

NS_IMPEXP const NsOctreeNode* ns_octree_node_child( const NsOctreeNode *node, NsOctantType octant );

NS_IMPEXP const NsAABBox3d* ns_octree_node_aabbox( const NsOctreeNode *node );

NS_IMPEXP nssize ns_octree_node_num_objects( const NsOctreeNode *node );

/* Cast return type of e_vector_iter_get_object() to an NsOctreeObject*. */
NS_IMPEXP nsvectoriter ns_octree_node_begin_objects( const NsOctreeNode *node );
NS_IMPEXP nsvectoriter ns_octree_node_end_objects( const NsOctreeNode *node );


typedef struct _NsOctreeStats
	{
	/* Total # of object references stored */
	nssize    num_objects;

	/* Total # of internal nodes */
	nssize    num_internal_nodes;

	/* Total # of leaf nodes */
	nssize    num_leaf_nodes;

	/* The actual maximum recursive depth, not
		necessarily equal to ns_octree_max_recursion(). */
	nssize    max_recursion;

	/* The number of object references of the leaf with
		the greateast amount. */
	nssize    max_node_objects;

	/* The number of leaf nodes with zero object references. */
	nssize    num_empty_leaf_nodes;

	/* The minimum and maximum dimensions of any of the leaf nodes. */
	nsdouble  min_leaf_aabbox_size;
	nsdouble	 max_leaf_aabbox_size;
	}
	NsOctreeStats;

NS_IMPEXP void ns_octree_stats( const NsOctree *octree, NsOctreeStats *stats );

NS_DECLS_END

#endif/* __NS_MATH_OCTREE_H__ */
