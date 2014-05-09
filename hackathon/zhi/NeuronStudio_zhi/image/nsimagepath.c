#include "nsimagepath.h"
#include <std/nsprofiler.h>


#define __INFINITE  -1.0

#define __IS_INFINITE( cost )\
	( (cost) < 0.0 )


#define __UNSEEN     0
#define __CANDIDATE  1
#define __FINALIZED  2

#define __COLOR( voxel )\
	( (voxel)->flags )


#define __DATA( voxel, field )\
	( ( ( NsImagePathData* )(voxel)->data )->field )


#define __OFFSETS()\
	ns_voxel_offsets()

#define __NUM_OFFSETS\
	NS_VOXEL_NUM_OFFSETS


typedef struct _NsImagePathData
	{
	nsdouble    cost;
	NsVoxel    *parent;
	nspointer   handle;
	}
	NsImagePathData;


NS_PRIVATE NsError ns_image_path_data_new( NsImagePathData **data )
	{
	if( NULL == ( *data = ns_new( NsImagePathData ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*data)->cost   = __INFINITE;
	(*data)->handle = NULL;
	(*data)->parent = NULL;

	return ns_no_error();
	}


NS_PRIVATE void ns_image_path_data_delete( NsImagePathData *data )
	{  ns_delete( data );  }


NS_PRIVATE nsboolean ns_image_path_cost_less( const NsVoxel *voxel1, const NsVoxel *voxel2 )
	{  return __DATA( voxel1, cost ) < __DATA( voxel2, cost );  }


NS_PRIVATE NsError ns_image_path_node_new( NsImagePathNode **node )
	{
	if( NULL == ( *node = ns_new( NsImagePathNode ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	return ns_no_error();
	}


NS_PRIVATE void ns_image_path_node_delete( NsImagePathNode *node )
	{  ns_delete( node );  }


void ns_image_path_construct( NsImagePath *path )
	{
	ns_assert( NULL != path );
	ns_list_construct( &path->nodes, ns_image_path_node_delete );
	}


void ns_image_path_destruct( NsImagePath *path )
	{
	ns_assert( NULL != path );
	ns_list_destruct( &path->nodes );
	}


NS_PRIVATE void _ns_image_path_set_distances
	(
	const NsVoxelInfo  *voxel_info,
	nsdouble           *distances
	)
	{
	nssize             i;
	const NsVector3i  *offsets;
	NsVector3f         O;
	NsVector3f         V;


	offsets = __OFFSETS();

	ns_vector3f_zero( &O );

	for( i = 1; i < __NUM_OFFSETS; ++i )
		{
		ns_to_voxel_space( offsets + i, &V, voxel_info );
		distances[i] = ( nsdouble )ns_vector3f_distance( &V, &O );
		}
	}


NS_PRIVATE NsError _ns_image_path_trace( NsImagePath *path, NsVoxel *curr )
	{
	NsImagePathNode  *node;
	NsError           error;

	while( NULL != curr )
		{
		if( NS_FAILURE( ns_image_path_node_new( &node ), error ) )
			return error;

		node->position = curr->position;

		/* NOTE: Have to push the node to the front of the list since we
			are traversing from the end of the path to the beginning. */
		if( NS_FAILURE( ns_list_push_front( &path->nodes, node ), error ) )
			{
			ns_image_path_node_delete( node );
			return error;
			}

		curr = __DATA( curr, parent );
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_image_path_new_voxel
	(
	NsVoxelBuffer      *voxel_buffer,
	NsVoxelTable       *seen_voxels,
	const NsVector3i   *position,
	NsVoxel           **voxel
	)
	{
	nsfloat           intensity;
	NsImagePathData  *data;
	NsError           error;


	if( NS_FAILURE( ns_image_path_data_new( &data ), error ) )
		return error;

	intensity = ( nsfloat )
						ns_voxel_get(
							voxel_buffer,
							position->x,
							position->y,
							position->z
							);

	if( NS_FAILURE( ns_voxel_table_add(
							seen_voxels,
							position,
							intensity,
							data,
							voxel
							),
							error ) )
		{
		ns_image_path_data_delete( data );
		return error;
		}

	return ns_no_error();
	}


/* Based on Dijkstra's shortest path algorithm. */
NsError ns_image_path_create
	(
	NsImagePath         *path,
	const NsImage       *image,
	const NsPoint3f     *origin,
	const NsVoxelTable  *voxel_table,
	const NsVoxelInfo   *voxel_info,
	nsfloat              max_path_length,
	NsImagePathFunc      path_func,
	nspointer            user_data
	)
	{
	NsVoxelTable       seen_voxels;
	NsBinHeap          candidate_voxels;
	NsVoxelBuffer      voxel_buffer;
	nsboolean          found_dest;
	NsVoxel            N, *curr, *dest, *voxel;
	nspointer          curr_owner, dest_owner;
	const NsVector3i  *offsets;
	nssize             i;
	nsdouble           intensity;
	nsdouble           old_cost, new_cost; 
	nsdouble           distances[ __NUM_OFFSETS ];
	NsError            error;
	NsPoint3i          O;
	nsint              width, height, length;


	ns_assert( NULL != path );
	ns_assert( NULL != image );
	ns_assert( NULL != origin );
	ns_assert( NULL != voxel_table );
	ns_assert( NULL != voxel_info );
	ns_assert( NULL != path_func );

	NS_USE_VARIABLE( max_path_length );

	if( NS_FAILURE( ns_voxel_table_construct(
							&seen_voxels,
							ns_image_path_data_delete
							),
							error ) )
		return error;

	if( NS_FAILURE( ns_bin_heap_construct(
							&candidate_voxels,
							0,
							ns_image_path_cost_less,
							NULL
							),
							error ) )
		{
		ns_voxel_table_destruct( &seen_voxels );
		return error;
		}

	ns_image_path_clear( path );

	_ns_image_path_set_distances( voxel_info, distances );

	ns_voxel_buffer_init( &voxel_buffer, image );

	error      = ns_no_error();
	found_dest = NS_TRUE;
	offsets    = __OFFSETS();

	ns_to_image_space( origin, &O, voxel_info );

	width  = ns_voxel_buffer_width( &voxel_buffer );
	height = ns_voxel_buffer_height( &voxel_buffer );
	length = ns_voxel_buffer_length( &voxel_buffer );

	ns_assert( 0 <= O.x && O.x < width );
	ns_assert( 0 <= O.y && O.y < height );
	ns_assert( 0 <= O.z && O.z < length );

	if( NS_FAILURE( _ns_image_path_new_voxel(
							&voxel_buffer,
							&seen_voxels,
							&O,
							&curr
							),
							error ) )
		goto _NS_IMAGE_PATH_CREATE_EXIT;

	__DATA( curr, cost ) = 0.0;
	__COLOR( curr )      = __CANDIDATE;

	if( NS_FAILURE( ns_bin_heap_push(
							&candidate_voxels,
							curr,
							&__DATA( curr, handle )
							),
							error ) )
		goto _NS_IMAGE_PATH_CREATE_EXIT;

	curr = ns_bin_heap_peek( &candidate_voxels );

	while( ( path_func )( &curr->position, user_data ) )
		{
		ns_bin_heap_pop( &candidate_voxels );

		/* The 'curr' is now finalized. */
		ns_assert( __CANDIDATE == __COLOR( curr ) );
		__COLOR( curr ) = __FINALIZED;

		curr_owner = NULL;
		if( NULL != ( voxel = ns_voxel_table_find( voxel_table, &curr->position ) ) )
			{}

		/* Update neighbors of the 'current'. Add unvisited neighbors to the
			'candidates' set... */
		for( i = 0; i < __NUM_OFFSETS; ++i )
			{
			ns_vector3i_add( &N.position, &curr->position, offsets + i );

			/* Make sure the position falls within the image. */
			if( 0 <= N.position.x && N.position.x < width  &&
				 0 <= N.position.y && N.position.y < height &&
				 0 <= N.position.z && N.position.z < length   )
				{
				/* Create a new voxel if we havent seen this one yet. */
				if( NULL == ( dest = ns_voxel_table_find( &seen_voxels, &N.position ) ) )
					if( NS_FAILURE( _ns_image_path_new_voxel(
											&voxel_buffer,
											&seen_voxels,
											&N.position,
											&dest
											),
											error ) )
						goto _NS_IMAGE_PATH_CREATE_EXIT;

				/* Dont update any in the finalized set! */
				if( __FINALIZED != __COLOR( dest ) )
					{
					old_cost = __DATA( dest, cost );

//#error todo: if both voxels inside clump replace 1 with e.g. 50 i.e. reducing cost!!!!!!!
					
					dest_owner = NULL;
					if( NULL != ( voxel = ns_voxel_table_find( voxel_table, &dest->position ) ) )
						{}

					/* TEMP: Do we want to power this up? e.g. ^1.5 */
					/* NOTE: Add 1 to assure the divisor is never zero. */
					intensity = ( nsdouble )( curr->intensity + dest->intensity ) + 1.0;
					new_cost  = __DATA( curr, cost ) + distances[i] / intensity;

					/* Only update if the new cost is less... */
					if( __IS_INFINITE( old_cost ) || new_cost < old_cost )
						{
						__DATA( dest, cost )   = new_cost;
						__DATA( dest, parent ) = curr;

						if( __UNSEEN == __COLOR( dest ) )
							{
							__COLOR( dest ) = __CANDIDATE;

							if( NS_FAILURE( ns_bin_heap_push(
													&candidate_voxels,
													dest,
													&__DATA( dest, handle )
													),
													error ) )
								goto _NS_IMAGE_PATH_CREATE_EXIT;
							}
						else
							{
							ns_assert( __CANDIDATE == __COLOR( dest ) );
							ns_bin_heap_repush( &candidate_voxels, dest, &__DATA( dest, handle ) );
							}
						}
					}
				}
			}

		if( ns_bin_heap_is_empty( &candidate_voxels ) )
			{
			found_dest = NS_FALSE;
			break;
			}

		curr = ns_bin_heap_peek( &candidate_voxels );
		}

	if( ! found_dest )
		{
		ns_warning( NS_WARNING_LEVEL_CAUTION NS_MODULE " Destination not found." );
		goto _NS_IMAGE_PATH_CREATE_EXIT;
		}

	/* Add the final destination to the path... */
	error = _ns_image_path_trace( path, curr );

	_NS_IMAGE_PATH_CREATE_EXIT:

	ns_bin_heap_destruct( &candidate_voxels );
	ns_voxel_table_destruct( &seen_voxels );

	return error;
	}


nssize ns_image_path_size( const NsImagePath *path )
	{
	ns_assert( NULL != path );
	return ns_list_size( &path->nodes );
	}


nsboolean ns_image_path_is_empty( const NsImagePath *path )
	{
	ns_assert( NULL != path );
	return ns_list_is_empty( &path->nodes );
	}


void ns_image_path_clear( NsImagePath *path )
	{
	ns_assert( NULL != path );
	ns_list_clear( &path->nodes );
	}


nsimagepathiter ns_image_path_begin( const NsImagePath *path )
	{
	ns_assert( NULL != path );
	return ns_list_begin( &path->nodes );
	}


nsimagepathiter ns_image_path_end( const NsImagePath *path )
	{
	ns_assert( NULL != path );
	return ns_list_end( &path->nodes );
	}
