#include "nsoctree.h"


void ns_octree_construct( NsOctree *octree )
   {
	ns_assert( NULL != octree );

	octree->root               = NULL;
	octree->num_objects        = 0;
	octree->min_node_objects   = 0;
	octree->max_recursion      = 0;
	octree->init_node_capacity = 0;
	octree->min_aabbox_size    = 0.0;
	octree->intersect_func     = NULL;

	ns_list_construct( &octree->list, NULL );
   }


void ns_octree_destruct( NsOctree *octree )
   {
	ns_assert( NULL != octree );

	ns_octree_clear( octree );
	ns_list_destruct( &octree->list );
	}


NS_PRIVATE NsError _ns_octree_node_new( NsOctreeNode **ret_node, NsOctree *octree )
	{
	NsOctreeNode  *node;
	nssize         i;
	NsError        error;


	ns_assert( NULL != ret_node );
	ns_assert( NULL != octree );

	if( NULL == ( node = ns_new( NsOctreeNode ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_vector_construct(
							&node->objects,
							octree->init_node_capacity,
							NULL
							),
							error ) )
		{
		ns_delete( node );
		return error;
		}

	/* Assure the children pointers are initially null. */
	for( i = 0; i < 8; ++i )
		node->children[i] = NULL;

	for( i = 0; i < NS_OCTREE_NODE_NUM_VALUES; ++i )
		ns_value_construct( node->values + i );

	*ret_node = node;
	return ns_no_error();
	}


NS_PRIVATE void _ns_octree_delete_node( NsOctree *octree, NsOctreeNode *node )
	{
	nssize i;

	ns_assert( NULL != octree );
	ns_assert( NULL != node );

	NS_USE_VARIABLE( octree );

	for( i = 0; i < NS_OCTREE_NODE_NUM_VALUES; ++i )
		ns_value_destruct( node->values + i );

	ns_vector_destruct( &node->objects );

	ns_delete( node );
	}


void ns_octree_get_root_aabbox( const NsOctree *octree, NsAABBox3d *root_box )
	{
	ns_assert( NULL != octree );
	ns_assert( NULL != root_box );

	*root_box = octree->root_box;
	}


void ns_octree_set_root_aabbox( NsOctree *octree, const NsAABBox3d *root_box )
	{
	ns_assert( NULL != octree );
	ns_assert( NULL != root_box );

	octree->root_box = *root_box;
	}


nssize ns_octree_get_min_node_objects( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->min_node_objects;
	}


void ns_octree_set_min_node_objects( NsOctree *octree, nssize min_node_objects )
	{
	ns_assert( NULL != octree );
	octree->min_node_objects = min_node_objects;
	}


nssize ns_octree_get_max_recursion( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->max_recursion;
	}


void ns_octree_set_max_recursion( NsOctree *octree, nssize max_recursion )
	{
	ns_assert( NULL != octree );
	octree->max_recursion = max_recursion;
	}


nssize ns_octree_get_init_node_capacity( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->init_node_capacity;
	}


void ns_octree_set_init_node_capacity( NsOctree *octree, nssize init_node_capacity )
	{
	ns_assert( NULL != octree );
	octree->init_node_capacity = init_node_capacity;
	}


nsdouble ns_octree_get_min_aabbox_size( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->min_aabbox_size;
	}


void ns_octree_set_min_aabbox_size( NsOctree *octree, nsdouble min_aabbox_size )
	{
	ns_assert( NULL != octree );
	octree->min_aabbox_size = min_aabbox_size;
	}


NsOctreeIntersectFunc ns_octree_get_intersect_func( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->intersect_func;
	}


void ns_octree_set_intersect_func( NsOctree *octree, NsOctreeIntersectFunc intersect_func )
	{
	ns_assert( NULL != octree );
	octree->intersect_func = intersect_func;
	}


NS_PRIVATE void _ns_octree_node_set_child_boxes( const NsOctreeNode *node )
	{
	NsAABBox3d  ltn;
	nsdouble    width;
	nsdouble    height;
	nsdouble    length;


	ns_assert( NULL != node );
	ns_aabbox3d_scale( &ltn, &node->box, .5 );

	width  = ns_aabbox3d_width( &ltn );
	height = ns_aabbox3d_height( &ltn );
	length = ns_aabbox3d_length( &ltn );

	node->children[ NS_OCTANT_LEFT_TOP_NEAR ]->box = ltn;

	ns_aabbox3d_translate_x(
		&( node->children[ NS_OCTANT_RIGHT_TOP_NEAR ]->box ),
		&ltn,
		width
		);

	ns_aabbox3d_translate_y(
		&( node->children[ NS_OCTANT_LEFT_BOTTOM_NEAR ]->box ),
		&ltn,
		-height
		);

	ns_aabbox3d_translate_xy(
		&( node->children[ NS_OCTANT_RIGHT_BOTTOM_NEAR ]->box ),
		&ltn,
		width,
		-height
		);

	ns_aabbox3d_translate_z(
		&( node->children[ NS_OCTANT_LEFT_TOP_FAR ]->box ),
		&ltn,
		-length
		);

	ns_aabbox3d_translate_xz(
		&( node->children[ NS_OCTANT_RIGHT_TOP_FAR ]->box ),
		&ltn,
		width,
		-length
		);

	ns_aabbox3d_translate_yz(
		&( node->children[ NS_OCTANT_LEFT_BOTTOM_FAR ]->box ),
		&ltn,
		-height,
		-length
		);

	ns_aabbox3d_translate_xyz(
		&( node->children[ NS_OCTANT_RIGHT_BOTTOM_FAR ]->box ),
		&ltn,
		width,
		-height,
		-length
		);
	}


NS_PRIVATE NsError _ns_octree_do_add
	(
	NsOctree      *octree,
	NsOctreeNode  *node,
	nssize         recursive_depth,
	nspointer      object,
	nsboolean     *was_added
	)
	{
	nsvectoriter  curr, end;
	nssize        i;
	NsError       error;


	/* Does it even intersect this node? */
	if( ! ( octree->intersect_func )( object, &node->box ) )
		return ns_no_error();

	/* If we're at the maximum recursive depth or we've hit a leaf node? */
	if( /*octree->max_recursion == recursive_depth ||*/ ns_octree_node_is_leaf( node ) )
		{
		ns_assert( ns_octree_node_is_leaf( node ) );

		/* Add the object to this leaf node. */
		if( NS_FAILURE( ns_vector_push_back( &node->objects, object ), error ) )
			return error;

		if( NULL != was_added )
			*was_added = NS_TRUE;

		/* If theres too many objects in this leaf node then recursively
			sub-divide it. (Up to the allowed recursive depth.) Also check to
			see if the size of this node's bounding box is greater or equal than
			the minimum. Any of width, height, or length, will do since these
			are cubes! */
		if( octree->min_node_objects < ns_vector_size( &node->objects ) &&
			 recursive_depth < octree->max_recursion &&
			 octree->min_aabbox_size <= ns_aabbox3d_width( &node->box ) )
			{
			for( i = 0; i < 8; ++i )
				if( NS_FAILURE( _ns_octree_node_new( &( node->children[i] ), octree ), error ) )
					return error;

			_ns_octree_node_set_child_boxes( node );

			/* NOTE: There is the potential for all the objects to keep ending up
				in one child. If 'max_recursion' is set to infinity then we could
				keep recursing forever since the above test...
				"octree->min_node_objects < ns_vector_size( &node->objects )"
				will keep being true. Is this right? Do we need to prevent this? */

			curr = ns_vector_begin( &node->objects );
			end  = ns_vector_end( &node->objects );

			for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
				for( i = 0; i < 8; ++i )
					if( NS_FAILURE(
							_ns_octree_do_add(
								octree,
								node->children[i],
								recursive_depth + 1,
								ns_vector_iter_get_object( curr ),
								was_added
								),
							error ) )
						return error;

			/* Clear this node since it is not a leaf anymore. */
			ns_assert( ns_octree_node_is_internal( node ) );
			ns_vector_clear( &node->objects );
			}
		}
	else
		{
		ns_assert( ns_octree_node_is_internal( node ) );

		for( i = 0; i < 8; ++i )
			if( NS_FAILURE(
					_ns_octree_do_add(
						octree,
						node->children[i],
						recursive_depth + 1,
						object,
						was_added
						),
					error ) )
				return error;
		}

	return ns_no_error();
	}


NsError ns_octree_add( NsOctree *octree, nspointer object, nsboolean *ret_was_added )
	{
	nsboolean  was_added;
	NsError    error;


	ns_assert( NULL != octree );
	ns_assert( NULL != octree->intersect_func );

	if( NULL != ret_was_added )
		*ret_was_added = NS_FALSE;

	if( NULL == octree->root )
		{
		if( NS_FAILURE( _ns_octree_node_new( &( octree->root ), octree ), error ) )
			return error;

		octree->root->box = octree->root_box;
		}

	was_added = NS_FALSE;
	error     = _ns_octree_do_add( octree, octree->root, 0, object, &was_added );

	if( ! ns_is_error( error ) && was_added )
		{
		++(octree->num_objects);

		if( NULL != ret_was_added )
			*ret_was_added = NS_TRUE;
		}

	return error;
	}


/* Returns the end iterator if not found. */
NS_PRIVATE nsvectoriter _ns_octree_node_find
	(
	const NsOctreeNode  *node,
	nspointer            object,
	NsEqualFunc          equal_func
	)
	{
	nsvectoriter curr, end;

	curr = ns_vector_begin( &node->objects );
	end  = ns_vector_end( &node->objects );

	for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
		if( ( equal_func )( ns_vector_iter_get_object( curr ), object ) )
			return curr;

	return end;
	}


NS_PRIVATE NsError _ns_octree_do_remove
	(
	NsOctree      *octree,
	NsOctreeNode  *node,
	nspointer      object,
	NsEqualFunc    equal_func,
	nsboolean     *was_removed
	)
	{
	nsvectoriter  curr, end;
	nssize        i;
	nssize        num_leaves;
	nssize        num_objects;
	NsError       error;


	/* Does it even intersect this node? */
	if( ! ( octree->intersect_func )( object, &node->box ) )
		return ns_no_error();

	if( ns_octree_node_is_leaf( node ) )
		{
		/* Remove the object from this leaf node, that is if its there.
			NOTE: Assuming the objects are unique and that each leaf node can only
			store the object once. */

		curr = _ns_octree_node_find( node, object, equal_func );

		if( ns_vector_iter_not_equal( curr, ns_vector_end( &node->objects ) ) )
			{
			ns_vector_erase( &node->objects, curr );

			if( NULL != was_removed )
				*was_removed = NS_TRUE;
			}
		}
	else
		{
		ns_assert( ns_octree_node_is_internal( node ) );

		for( i = 0; i < 8; ++i )
			if( NS_FAILURE(
					_ns_octree_do_remove(
						octree,
						node->children[i],
						object,
						equal_func,
						was_removed
						),
					error ) )
				return error;

		/* There are 2 criteria for collapsing sibling leaf nodes into an internal
			node...

			1) All the children of the current internal node must be leaves. In other
				words if one or more is not a leaf, then obviously the combined number
				of objects "underneath" this internal node is greater than  the
				'min_node_objects' setting.

			2) The combined number of objects of all the leaf nodes must be less than
				or equal to the 'min_node_objects' setting.

			NOTE: We have to also check for duplicates when removing the leaf node objects
			and placing in them in the current internal node( which is becoming a leaf ) since
			more than one child node could intersect a given object. */

		num_leaves = 0;

		for( i = 0; i < 8 && ns_octree_node_is_leaf( node->children[i] ); ++i )
			++num_leaves;

		if( 8 == num_leaves )
			{
			num_objects = 0;

			for( i = 0; i < 8; ++i )
				num_objects += ns_vector_size( &(node->children[i]->objects) );

			if( num_objects <= octree->min_node_objects )
				{
				for( i = 0; i < 8; ++i )
					{
					curr = ns_vector_begin( &(node->children[i]->objects) );
					end  = ns_vector_end( &(node->children[i]->objects) );

					/* NOTE: This is a N^2 operation, but it should be fast since 'min_node_objects'
						should be a reasonably small number, e.g. around 16. */
					for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
						if( ns_vector_iter_equal(
								_ns_octree_node_find( node, ns_vector_iter_get_object( curr ), equal_func ),
								ns_vector_end( &node->objects ) ) )
							{
							if( NS_FAILURE(
									ns_vector_push_back(
										&node->objects,
										ns_vector_iter_get_object( curr )
										),
									error ) )
								return error;
							}
					}

				for( i = 0; i < 8; ++i )
					{
					_ns_octree_delete_node( octree, node->children[i] );
					node->children[i] = NULL;
					}

				ns_assert( ns_octree_node_is_leaf( node ) );
				}
			}
		}

	return ns_no_error();
	}


NsError ns_octree_remove
	(
	NsOctree     *octree,
	nspointer     object,
	NsEqualFunc   equal_func,
	nsboolean    *ret_was_removed
	)
	{
	nsboolean  was_removed;
	NsError    error;


	ns_assert( NULL != octree );
	ns_assert( NULL != octree->intersect_func );
	ns_assert( NULL != equal_func );

	if( NULL != ret_was_removed )
		*ret_was_removed = NS_FALSE;

	if( NULL == octree->root )
		return ns_no_error();

	was_removed = NS_FALSE;
	error       = _ns_octree_do_remove( octree, octree->root, object, equal_func, &was_removed );

	if( ! ns_is_error( error ) && was_removed )
		{
		--(octree->num_objects);

		if( NULL != ret_was_removed )
			*ret_was_removed = NS_TRUE;
		}

	return error;
	}


NS_PRIVATE NsError _ns_octree_do_objects( NsOctree *octree, NsOctreeNode *node )
	{
	nsvectoriter  curr;
	nssize        i;
	NsError       error;


	if( ns_octree_node_is_leaf( node ) )
		{
		NS_VECTOR_FOREACH( &node->objects, curr )
			if( NS_FAILURE( ns_list_push_back( &octree->list, ns_vector_iter_get_object( curr ) ), error ) )
				return error;
		}
	else
		{
		for( i = 0; i < 8; ++i )
			if( NS_FAILURE( _ns_octree_do_objects( octree, node->children[i] ), error ) )
				return error;
		}

	return ns_no_error();
	}


NsError ns_octree_restructure
	(
	NsOctree               *octree,
	const NsAABBox3d       *root_box,
	nssize                  min_node_objects,
	nssize                  max_recursion,
	nssize                  init_node_capacity,
	nsdouble                min_aabbox_size,
	NsOctreeIntersectFunc   intersect_func
	)
	{
	nslistiter  iter;
	NsError     error;


	ns_assert( NULL != octree );
	ns_assert( NULL != root_box );

	/* First have to store the objects that are in the octree. Then clear
		the octree, set new parameters, and re-build it. */

	error = ns_no_error();

	ns_list_clear( &octree->list );

	if( NULL != octree->root )
		error = _ns_octree_do_objects( octree, octree->root );

	if( ! ns_is_error( error ) )
		{
		ns_octree_clear( octree );

		ns_octree_set_root_aabbox( octree, root_box );
		ns_octree_set_min_node_objects( octree, min_node_objects );
		ns_octree_set_max_recursion( octree, max_recursion );
		ns_octree_set_init_node_capacity( octree, init_node_capacity );
		ns_octree_set_min_aabbox_size( octree, min_aabbox_size );
		ns_octree_set_intersect_func( octree, intersect_func );

		NS_LIST_FOREACH( &octree->list, iter )
			if( NS_FAILURE( ns_octree_add( octree, ns_list_iter_get_object( iter ), NULL ), error ) )
				break;
		}

	ns_list_clear( &octree->list );
	return error;
	}


NS_PRIVATE void _ns_octree_do_clear( NsOctree *octree, NsOctreeNode *node )
	{
	if( NULL != node )
		{
		nssize i;

		if( ns_octree_node_is_internal( node ) )
			for( i = 0; i < 8; ++i )
				_ns_octree_do_clear( octree, node->children[ i ] );

		_ns_octree_delete_node( octree, node );
		}
	}


void ns_octree_clear( NsOctree *octree )
	{
	ns_assert( NULL != octree );

	_ns_octree_do_clear( octree, octree->root );

	octree->root        = NULL;
	octree->num_objects = 0;
	}


NS_PRIVATE NsError _ns_octree_do_intersections
	(
	const NsOctree      *octree,
	const NsOctreeNode  *node,
	nspointer            object,
	NsVector            *nodes
	)
	{
	nssize   i;
	NsError  error;


	ns_assert( NULL != node );

	/* If the object doesnt intersect this bounding box then
		it definitely wont intersect any of its childrens
		bounding boxes. */
	if( ! ( octree->intersect_func )( object, &node->box ) )
		return ns_no_error();

	if( ns_octree_node_is_internal( node ) )
		{
		for( i = 0; i < 8; ++i )
			if( NS_FAILURE( _ns_octree_do_intersections(
									octree,
									node->children[i],
									object,
									nodes
									),
									error ) )
				return error;
		}
	else if( NS_FAILURE( ns_vector_push_back(
								nodes,
								( NsOctreeNode* )node
								),
								error ) )
		return error;

	return ns_no_error();
	}


NsError ns_octree_intersections
	(
	const NsOctree  *octree,
	nspointer        object,
	NsVector        *nodes
	)
	{
	ns_assert( NULL != octree );
	ns_assert( NULL != octree->intersect_func );
	ns_assert( NULL != nodes );

	return NULL != octree->root ?
				_ns_octree_do_intersections( octree, octree->root, object, nodes ) :
				ns_no_error();
	}


nssize ns_octree_num_objects( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->num_objects;
	}


const NsOctreeNode* ns_octree_root( const NsOctree *octree )
	{
	ns_assert( NULL != octree );
	return octree->root;
	}


NsValue* ns_octree_node_value( const NsOctreeNode *node, nssize which )
	{
	ns_assert( NULL != node );
	ns_assert( which < NS_OCTREE_NODE_NUM_VALUES );

	return ( NsValue* )( node->values + which );
	}


/* NOTE: Quick test whether or not a node is a leaf. i.e assuming
	if the first one is NULL, then they're all NULL. */
nsboolean ns_octree_node_is_leaf( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return NULL == node->children[ 0 ];
	}


nsboolean ns_octree_node_is_internal( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return ! ns_octree_node_is_leaf( node );
	}


const NsOctreeNode* ns_octree_node_child( const NsOctreeNode *node, NsOctantType octant )
	{
	ns_assert( NULL != node );
	ns_assert( octant >= 0 );
	ns_assert( octant < 8 );
	ns_assert( ns_octree_node_is_internal( node ) );
	ns_assert( NULL != node->children[ octant ] );

	return node->children[ octant ];
	}


const NsAABBox3d* ns_octree_node_aabbox( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return &node->box;
	}


nssize ns_octree_node_num_objects( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return ns_vector_size( &node->objects );
	}


nsvectoriter ns_octree_node_begin_objects( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return ns_vector_begin( &node->objects );
	}


nsvectoriter ns_octree_node_end_objects( const NsOctreeNode *node )
	{
	ns_assert( NULL != node );
	return ns_vector_end( &node->objects );
	}


NS_PRIVATE void _ns_octree_do_stats
	(
	const NsOctreeNode  *node,
	NsOctreeStats       *stats,
	nssize               recursive_depth
	)
	{
	nssize    num_objects;
	nssize    i;
	nsdouble  size;


	ns_assert( NULL != node );

	if( stats->max_recursion < recursive_depth )
		stats->max_recursion = recursive_depth;

	if( ns_octree_node_is_internal( node ) )
		{
		++(stats->num_internal_nodes);

		for( i = 0; i < 8; ++i )
			_ns_octree_do_stats(
				node->children[ i ],
				stats,
				recursive_depth + 1
				);
		}
	else
		{
		num_objects = ns_octree_node_num_objects( node );

		if( 0 < num_objects )
			stats->num_objects += num_objects;
		else
			++(stats->num_empty_leaf_nodes);

		++(stats->num_leaf_nodes);

		if( stats->max_node_objects < num_objects )
			stats->max_node_objects = num_objects;

		/* NOTE: Any of width, height, or length, will do since these
			are cubes! */
		size = ns_aabbox3d_width( &node->box );

		if( size < stats->min_leaf_aabbox_size )
			stats->min_leaf_aabbox_size = size;

		if( size > stats->max_leaf_aabbox_size )
			stats->max_leaf_aabbox_size = size;
		}
	}


void ns_octree_stats( const NsOctree *octree, NsOctreeStats *stats )
	{
	ns_assert( NULL != octree );
	ns_assert( NULL != stats );

	ns_memzero( stats, sizeof( NsOctreeStats ) );

	stats->min_leaf_aabbox_size = NS_DOUBLE_MAX;
	stats->max_leaf_aabbox_size = -NS_DOUBLE_MAX;

	if( NULL != octree->root )
		_ns_octree_do_stats( octree->root, stats, 0 );
	}
