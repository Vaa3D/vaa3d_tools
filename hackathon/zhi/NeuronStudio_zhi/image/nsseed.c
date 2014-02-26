#include "nsseed.h"


typedef void ( *_NsSeedFindTraverseFunc )
	(
	const NsVoxelBuffer*,
	const NsCubei*,
	nsint,
	nsint,
	nsfloat*,
	NsProgress*
	);


/* Example:

	If C1 is 2, then there are 2 leading indices that should be zero'd.
	If C2 is 5 and the dimension is 9, there are 9 - 6 = 3 trailing
	indices to be zero'd.
*/
NS_PRIVATE void _ns_seed_find_2d_traverse_x
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	nsint                 z,
	nsint                 y,
	nsfloat              *values,
	NsProgress           *progress
	)
	{
	nsint x, d, leading, trailing;

	d        = ns_voxel_buffer_width( voxel_buffer );
	leading  = roi->C1.x;
	trailing = d - ( roi->C2.x + 1 );

	ns_assert( leading + ns_cubei_width( roi ) + trailing == d );

	ns_memzero( values, leading * sizeof( nsfloat ) );
	ns_memzero( values + ( roi->C2.x + 1 ), trailing * sizeof( nsfloat ) );

	ns_progress_num_iters( progress, ns_cubei_width( roi ) );
	ns_progress_begin( progress );

	for( x = roi->C1.x; x <= roi->C2.x; ++x )
		{
		values[x] = ( nsfloat )ns_voxel_get( voxel_buffer, x, y, z );
		ns_progress_next_iter( progress );
		}

	ns_progress_end( progress );
	}


NS_PRIVATE void _ns_seed_find_2d_traverse_y
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	nsint                 x,
	nsint                 z,
	nsfloat              *values,
	NsProgress           *progress
	)
	{
	nsint y, d, leading, trailing;

	d        = ns_voxel_buffer_height( voxel_buffer );
	leading  = roi->C1.y;
	trailing = d - ( roi->C2.y + 1 );

	ns_assert( leading + ns_cubei_height( roi ) + trailing == d );

	ns_memzero( values, leading * sizeof( nsfloat ) );
	ns_memzero( values + ( roi->C2.y + 1 ), trailing * sizeof( nsfloat ) );

	ns_progress_num_iters( progress, ns_cubei_height( roi ) );
	ns_progress_begin( progress );

	for( y = roi->C1.y; y <= roi->C2.y; ++y )
		{
		values[y] = ( nsfloat )ns_voxel_get( voxel_buffer, x, y, z );
		ns_progress_next_iter( progress );
		}

	ns_progress_end( progress );
	}


NS_PRIVATE void _ns_seed_find_2d_traverse_z
	(
	const NsVoxelBuffer  *voxel_buffer,
	const NsCubei        *roi,
	nsint                 x,
	nsint                 y,
	nsfloat              *values,
	NsProgress           *progress
	)
	{
	nsint z, d, leading, trailing;

	d        = ns_voxel_buffer_length( voxel_buffer );
	leading  = roi->C1.z;
	trailing = d - ( roi->C2.z + 1 );

	ns_assert( leading + ns_cubei_length( roi ) + trailing == d );

	ns_memzero( values, leading * sizeof( nsfloat ) );
	ns_memzero( values + ( roi->C2.z + 1 ), trailing * sizeof( nsfloat ) );

	ns_progress_num_iters( progress, ns_cubei_length( roi ) );
	ns_progress_begin( progress );

	for( z = roi->C1.z; z <= roi->C2.z; ++z )
		{
		values[z] = ( nsfloat )ns_voxel_get( voxel_buffer, x, y, z );
		ns_progress_next_iter( progress );
		}

	ns_progress_end( progress );
	}


#define _NS_SEED_FIND_NUM_PASSES  5

#include <ext/target.inl>


NsError ns_seed_find_2d
	(
	const NsImage  *image,
	const NsCubei  *roi,
	nsint           a,
	nsint           b,
	nsenum          which,
	nsint          *ret_index,
	NsProgress     *progress
	)
	{
	NsVoxelBuffer             voxel_buffer;
	NsByteArray               buffer;
	nsint                     index;
	nsfloat                  *values;
	nsint                     num_values;
	_NsSeedFindTraverseFunc   traverse_func;
	NsError                   error;


	ns_assert( NULL != image );
	ns_assert( NULL != ret_index );

	ns_voxel_buffer_init( &voxel_buffer, image );

	num_values    = 0;
	traverse_func = NULL;

	switch( which )
		{
		case NS_SEED_ZY_FIND_X:
			num_values    = ( nsint )ns_voxel_buffer_width( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_x;
			break;

		case NS_SEED_XZ_FIND_Y:
			num_values    = ( nsint )ns_voxel_buffer_height( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_y;
			break;

		case NS_SEED_XY_FIND_Z:
			num_values    = ( nsint )ns_voxel_buffer_length( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_z;
			break;

		default:
			ns_assert_not_reached();
		}

	if( NS_FAILURE( ns_byte_array_construct( &buffer, ( nssize )num_values * sizeof( nsfloat ) ), error ) )
		return error;

	values = ( nsfloat* )ns_byte_array_begin( &buffer );

	ns_assert( NULL != traverse_func );
	( traverse_func )( &voxel_buffer, roi, a, b, values, progress );

	index = GetTargetIndex( values, num_values, _NS_SEED_FIND_NUM_PASSES );

	ns_byte_array_destruct( &buffer );

	if( -1 == index )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	else
		{
		ns_assert( index < num_values );
		*ret_index = ( nsint )index;
		}

	return ns_no_error();
	}


NsError ns_seed_find_2d_ex
	(
	const NsImage   *image,
	const NsCubei   *roi,
	nsint            a,
	nsint            b,
	nsenum           which,
	nsint          **indices,
	nsint           *num_indices,
	NsProgress      *progress
	)
	{
	NsVoxelBuffer             voxel_buffer;
	NsByteArray               buffer;
	nsfloat                  *values;
	nsint                     num_values;
	_NsSeedFindTraverseFunc   traverse_func;
	NsError                   error;


	ns_assert( NULL != image );
	ns_assert( NULL != indices );
	ns_assert( NULL != num_indices );

	ns_voxel_buffer_init( &voxel_buffer, image );

	num_values    = 0;
	traverse_func = NULL;

	switch( which )
		{
		case NS_SEED_ZY_FIND_X:
			num_values    = ( nsint )ns_voxel_buffer_width( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_x;
			break;

		case NS_SEED_XZ_FIND_Y:
			num_values    = ( nsint )ns_voxel_buffer_height( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_y;
			break;

		case NS_SEED_XY_FIND_Z:
			num_values    = ( nsint )ns_voxel_buffer_length( &voxel_buffer );
			traverse_func = _ns_seed_find_2d_traverse_z;
			break;

		default:
			ns_assert_not_reached();
		}

	if( NS_FAILURE( ns_byte_array_construct( &buffer, ( nssize )num_values * sizeof( nsfloat ) ), error ) )
		return error;

	values = ( nsfloat* )ns_byte_array_begin( &buffer );

	ns_assert( NULL != traverse_func );
	( traverse_func )( &voxel_buffer, roi, a, b, values, progress );

	*indices = GetTargetIndices( values, num_values, _NS_SEED_FIND_NUM_PASSES, .6f, num_indices );

	ns_byte_array_destruct( &buffer );

	return NULL != *indices ?
				ns_no_error(): ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}
