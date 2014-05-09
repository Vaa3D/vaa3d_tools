#include "nsvoxel-table.h"


NS_PRIVATE nspointer ____ns_voxel_table_chunks    = NULL;
NS_PRIVATE nspointer ____ns_voxel_table_free_list = NULL;


#define _NS_VOXEL_TABLE_CHUNK_NUM_VOXELS\
	( 1024 )

#define _NS_VOXEL_TABLE_CHUNK_NUM_BYTES\
	( sizeof( NsVoxel ) * _NS_VOXEL_TABLE_CHUNK_NUM_VOXELS )


#define _NS_VOXEL_TABLE_NEXT( memblock )\
	( *( ( nspointer* )(memblock) ) )


/* Frees all the chunks. */
void ns_voxel_table_end_recycling( void )
	{
	nspointer curr, next;

#ifdef NS_DEBUG
	nssize count = 0;
#endif

	curr = ____ns_voxel_table_chunks;

	while( NULL != curr )
		{
		next = _NS_VOXEL_TABLE_NEXT( curr );
		ns_delete( curr );
		curr = next;

#ifdef NS_DEBUG
		++count;
#endif
		}

	____ns_voxel_table_chunks    = NULL;
	____ns_voxel_table_free_list = NULL;

#ifdef NS_DEBUG
	ns_log_entry(
		NS_LOG_ENTRY_NOTE,
		NS_FUNCTION ":: Free'd " NS_FMT_ULONG " voxel chunk structures (" NS_FMT_ULONG " bytes).",
		count,
		count * _NS_VOXEL_TABLE_CHUNK_NUM_BYTES
		);
#endif
	}


NS_PRIVATE nspointer _ns_voxel_table_alloc( void );


NS_PRIVATE nspointer _ns_voxel_table_pop( void )
	{
	nspointer voxel = ____ns_voxel_table_free_list;

	if( NULL == voxel )
		return _ns_voxel_table_alloc();

	____ns_voxel_table_free_list = _NS_VOXEL_TABLE_NEXT( voxel );

	return voxel;
	}


NS_PRIVATE void _ns_voxel_table_push( nspointer voxel )
	{
	_NS_VOXEL_TABLE_NEXT( voxel ) = ____ns_voxel_table_free_list;
	____ns_voxel_table_free_list = voxel;
	}


/* The simple way is just to return ns_new( NsVoxel ) */
NS_PRIVATE nspointer _ns_voxel_table_alloc( void )
	{
	nspointer  chunk;
	nspointer  voxel;
	nssize     i;


	/* Allocate a chunk of voxels. Add space for a pointer. */
	if( NULL == ( chunk = ns_malloc( _NS_VOXEL_TABLE_CHUNK_NUM_BYTES + sizeof( nspointer ) ) ) )
		return NULL;

	/* Push this new chunk to the front of the chunk list. */
	_NS_VOXEL_TABLE_NEXT( chunk ) = ____ns_voxel_table_chunks;
	____ns_voxel_table_chunks     = chunk;

	/* Add all the voxels in this chunk to the free list. */

	voxel = NS_OFFSET_POINTER( void, chunk, sizeof( nspointer ) );

	for( i = 0; i < _NS_VOXEL_TABLE_CHUNK_NUM_VOXELS; ++i )
		{
		_ns_voxel_table_push( voxel );
		voxel = NS_OFFSET_POINTER( void, voxel, sizeof( NsVoxel ) );
		}

	/* This call to pop is potentially recursive, but since the free list
		is now not empty, an infinite recursion is avoided. */
	return _ns_voxel_table_pop();
	}




const NsVector3i* ns_voxel_offsets( void )
	{
	NS_PRIVATE const NsVector3i _ns_voxel_offsets[ NS_VOXEL_NUM_OFFSETS ] =
		{
		{ -1, -1, -1 },//NS_VOXEL_OFFSET_LTN
		{  1, -1, -1 },//NS_VOXEL_OFFSET_RTN
		{ -1,  1, -1 },//NS_VOXEL_OFFSET_LBN
		{  1,  1, -1 },//NS_VOXEL_OFFSET_RBN
		{ -1, -1,  1 },//NS_VOXEL_OFFSET_LTF
		{  1, -1,  1 },//NS_VOXEL_OFFSET_RTF
		{ -1,  1,  1 },//NS_VOXEL_OFFSET_LBF
		{  1,  1,  1 },//NS_VOXEL_OFFSET_RBF

		{ -1, -1,  0 },//NS_VOXEL_OFFSET_LT
		{  1, -1,  0 },//NS_VOXEL_OFFSET_RT
		{ -1,  1,  0 },//NS_VOXEL_OFFSET_LB
		{  1,  1,  0 },//NS_VOXEL_OFFSET_RB
		{ -1,  0, -1 },//NS_VOXEL_OFFSET_LN
		{  1,  0, -1 },//NS_VOXEL_OFFSET_RN
		{ -1,  0,  1 },//NS_VOXEL_OFFSET_LF
		{  1,  0,  1 },//NS_VOXEL_OFFSET_RF
		{  0, -1, -1 },//NS_VOXEL_OFFSET_TN
		{  0,  1, -1 },//NS_VOXEL_OFFSET_BN
		{  0, -1,  1 },//NS_VOXEL_OFFSET_TF
		{  0,  1,  1 },//NS_VOXEL_OFFSET_BF

		{ -1,  0,  0 },//NS_VOXEL_OFFSET_L
		{  1,  0,  0 },//NS_VOXEL_OFFSET_R
		{  0, -1,  0 },//NS_VOXEL_OFFSET_T
		{  0,  1,  0 },//NS_VOXEL_OFFSET_B
		{  0,  0, -1 },//NS_VOXEL_OFFSET_N
		{  0,  0,  1 } //NS_VOXEL_OFFSET_F
		};

	return _ns_voxel_offsets;
	}


const NsVector3i* ns_voxel_face_offsets( void )
	{
	NS_PRIVATE const NsVector3i _ns_voxel_face_offsets[ NS_VOXEL_NUM_FACE_OFFSETS ] =
		{
		{ -1,  0,  0 },//NS_VOXEL_OFFSET_L
		{  1,  0,  0 },//NS_VOXEL_OFFSET_R
		{  0, -1,  0 },//NS_VOXEL_OFFSET_T
		{  0,  1,  0 },//NS_VOXEL_OFFSET_B
		{  0,  0, -1 },//NS_VOXEL_OFFSET_N
		{  0,  0,  1 } //NS_VOXEL_OFFSET_F
		};

	return _ns_voxel_face_offsets;
	}


const NsVector3i* ns_voxel_corner_offsets( void )
	{
	NS_PRIVATE const NsVector3i _ns_voxel_corner_offsets[ NS_VOXEL_NUM_CORNER_OFFSETS ] =
		{
		{ -1, -1, -1 },//NS_VOXEL_OFFSET_LTN
		{  1, -1, -1 },//NS_VOXEL_OFFSET_RTN
		{ -1,  1, -1 },//NS_VOXEL_OFFSET_LBN
		{  1,  1, -1 },//NS_VOXEL_OFFSET_RBN
		{ -1, -1,  1 },//NS_VOXEL_OFFSET_LTF
		{  1, -1,  1 },//NS_VOXEL_OFFSET_RTF
		{ -1,  1,  1 },//NS_VOXEL_OFFSET_LBF
		{  1,  1,  1 },//NS_VOXEL_OFFSET_RBF
		};

	return _ns_voxel_corner_offsets;
	}


#define ns_voxel_table_hash( x, y, z, voxel_table )\
	(\
	((nssize)(z)) * (voxel_table)->width_times_height +\
	((nssize)(y)) * ((nssize)(voxel_table)->width) +\
	((nssize)(x))\
	)


NS_PRIVATE nsboolean _ns_voxel_table_lookup
	(
	const NsVoxelTable   *voxel_table,
	const NsVector3i     *position,
	nssize               *ret_bucket,
	NsVoxel             **ret_voxel
	)
	{
	nssize          bucket;
	const NsVoxel  *voxel;


	bucket = ns_voxel_table_hash(
					position->x,
					position->y,
					position->z,
					voxel_table
					) % voxel_table->num_buckets;

	*ret_bucket = bucket;

	voxel = voxel_table->buckets[ bucket ];

	for( ; NULL != voxel; voxel = voxel->next )
		if( position->x == voxel->position.x &&
			 position->y == voxel->position.y &&
			 position->z == voxel->position.z    )
			{
			*ret_voxel = ( NsVoxel* )voxel;
			return NS_TRUE;
			}

	return NS_FALSE;
	}


#define _ns_voxel_table_calc_max( voxel_table )\
	( 3 * (voxel_table)->num_buckets )


NS_PRIVATE void _ns_voxel_table_do_rehash( NsVoxelTable *voxel_table )
	{
	NsVoxel  **new_buckets;
	nssize     new_num_buckets;
	nssize     new_bucket;
	NsVoxel   *voxel;
	NsVoxel   *next;
	nssize     bucket;


	new_num_buckets = ns_prime( ns_prime_nearest( ( nsuint )voxel_table->num_voxels ) );

	if( new_num_buckets == voxel_table->num_buckets )
		return;

	if( NULL == ( new_buckets = ns_new_array0( NsVoxel*, new_num_buckets ) ) )
		return;

	/* Splice out all voxels from the old buckets to the new ones. */
	for( bucket = 0; bucket < voxel_table->num_buckets; ++bucket )
		{
		voxel = voxel_table->buckets[ bucket ];

		while( NULL != voxel )
			{
			next = voxel->next;

			new_bucket = ns_voxel_table_hash(
								voxel->position.x,
								voxel->position.y,
								voxel->position.z,
								voxel_table
								) % new_num_buckets;

			voxel->next = new_buckets[ new_bucket ];
			new_buckets[ new_bucket ] = voxel;

			voxel = next;
			}
		}

	ns_delete( voxel_table->buckets );

	voxel_table->buckets     = new_buckets;
	voxel_table->num_buckets = new_num_buckets;
	voxel_table->max_voxels  = _ns_voxel_table_calc_max( voxel_table );
	}


NsError ns_voxel_table_construct
	(
	NsVoxelTable    *voxel_table,
	NsFinalizeFunc   finalize_func
	)
	{
	ns_assert( NULL != voxel_table );

	voxel_table->finalize_func = finalize_func;
	voxel_table->num_voxels    = 0;
	voxel_table->num_buckets   = ns_prime( NS_PRIME0 );
	voxel_table->max_voxels    = _ns_voxel_table_calc_max( voxel_table );
	voxel_table->recycle       = NS_FALSE;//NS_TRUE;

	voxel_table->buckets = ns_new_array0( NsVoxel*, voxel_table->num_buckets );

	return ( NULL != voxel_table->buckets ) ?
			 ns_no_error() : ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


void ns_voxel_table_destruct( NsVoxelTable *voxel_table )
	{
	ns_assert( NULL != voxel_table );

	ns_voxel_table_clear( voxel_table );
	ns_delete( voxel_table->buckets );
	}


void ns_voxel_table_get_dimensions
	(
	const NsVoxelTable  *voxel_table,
	nsint               *width,
	nsint               *height,
	nsint               *length
	)
	{
	ns_assert( NULL != voxel_table );
	ns_assert( NULL != width );
	ns_assert( NULL != height );
	ns_assert( NULL != length );

	*width  = voxel_table->width;
	*height = voxel_table->height;
	*length = voxel_table->length;
	}


void ns_voxel_table_set_dimensions
	(
	NsVoxelTable  *voxel_table,
	nsint          width,
	nsint          height,
	nsint          length
	)
	{
	ns_assert( NULL != voxel_table );

	voxel_table->width  = width;
	voxel_table->height = height;
	voxel_table->length = length;

	voxel_table->width_times_height = ( nssize )width * ( nssize )height;
	}


void ns_voxel_table_clear( NsVoxelTable *voxel_table )
	{
	nssize    bucket;
	NsVoxel  *voxel;
	NsVoxel  *next;


	ns_assert( NULL != voxel_table );

	for( bucket = 0; bucket < voxel_table->num_buckets; ++bucket )
		{
		voxel = voxel_table->buckets[ bucket ];

		while( NULL != voxel )
			{
			next = voxel->next;

			if( NULL != voxel_table->finalize_func )
				( voxel_table->finalize_func )( voxel->data );

			if( voxel_table->recycle )
				_ns_voxel_table_push( voxel );
			else
				ns_delete( voxel );

			voxel = next;
			}

		voxel_table->buckets[ bucket ] = NULL;
		}

	voxel_table->num_voxels = 0;
	}


void ns_voxel_table_recycle( NsVoxelTable *voxel_table, nsboolean recycle )
	{
	ns_assert( NULL != voxel_table );
	voxel_table->recycle = recycle;
	}


void ns_voxel_table_clean
	(
	NsVoxelTable  *voxel_table,
	nsboolean      ( *callback )( const NsVoxel*, nspointer ),
	nspointer      user_data
	)
	{
	nssize    bucket;
	NsVoxel  *voxel;
	NsVoxel  *prev;
	NsVoxel  *next;


	ns_assert( NULL != voxel_table );

	for( bucket = 0; bucket < voxel_table->num_buckets; ++bucket )
		{
		prev  = NULL;
		voxel = voxel_table->buckets[ bucket ];

		while( NULL != voxel )
			{
			next = voxel->next;

			if( ( callback )( voxel, user_data ) )
				{
				/* If first voxel in this bucket... */
				if( NULL == prev )
					voxel_table->buckets[ bucket ] = next;
				else
					prev->next = next;

				if( NULL != voxel_table->finalize_func )
					( voxel_table->finalize_func )( voxel->data );

				if( voxel_table->recycle )
					_ns_voxel_table_push( voxel );
				else
					ns_delete( voxel );

				--(voxel_table->num_voxels);
				}
			else
				prev = voxel;

			voxel = next;
			}
		}
	}


NsError ns_voxel_table_add
	(
	NsVoxelTable       *voxel_table,
	const NsVector3i   *position,
	nsfloat             intensity,
	nspointer           data,
	NsVoxel           **ret_voxel
	)
	{
	nssize    bucket;
	NsVoxel  *voxel;


	//ns_assert( NULL != voxel_table );
	//ns_assert( NULL != position );

	bucket = ns_voxel_table_hash(
					position->x,
					position->y,
					position->z,
					voxel_table
					) % voxel_table->num_buckets;

	voxel = voxel_table->recycle ?
				_ns_voxel_table_pop() : ns_new( NsVoxel );

	if( NULL == voxel )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	voxel->position  = *position;
	voxel->intensity = intensity;
	voxel->data      = data;
	voxel->flags     = 0;

	voxel->next = voxel_table->buckets[ bucket ];
	voxel_table->buckets[ bucket ] = voxel;

	++(voxel_table->num_voxels);

	if( voxel_table->max_voxels <= voxel_table->num_voxels )
		_ns_voxel_table_do_rehash( voxel_table );

	if( NULL != ret_voxel )
		*ret_voxel = voxel;

	return ns_no_error();
	}


/* NOTE: The table does NOT rehash after a remove is performed. i.e. the table
	buckets can only grow in size, not shrink. */
nsboolean ns_voxel_table_remove( NsVoxelTable *voxel_table, NsVoxel *voxel )
	{
	NsVector3i   position;
	nssize       bucket;
	NsVoxel     *prev, *next;


	//ns_assert( NULL != voxel_table );
	//ns_assert( NULL != voxel );

	position = voxel->position;

	bucket = ns_voxel_table_hash(
					position.x,
					position.y,
					position.z,
					voxel_table
					) % voxel_table->num_buckets;

	prev  = NULL;
	voxel = voxel_table->buckets[ bucket ];

	while( NULL != voxel )
		{
		next = voxel->next;

		if( voxel->position.x == position.x &&
			 voxel->position.y == position.y &&
			 voxel->position.z == position.z   )
			{
			/* If first voxel in this bucket... */
			if( NULL == prev )
				voxel_table->buckets[ bucket ] = next;
			else
				prev->next = next;

			if( NULL != voxel_table->finalize_func )
				( voxel_table->finalize_func )( voxel->data );

			if( voxel_table->recycle )
				_ns_voxel_table_push( voxel );
			else
				ns_delete( voxel );

			--(voxel_table->num_voxels);

			return NS_TRUE;
			}
		else
			prev = voxel;

		voxel = next;
		}

	return NS_FALSE;
	}


NsVoxel* ns_voxel_table_find( const NsVoxelTable *voxel_table, const NsVector3i *position )
	{
	NsVoxel  *voxel;
	nssize    bucket;


	//ns_assert( NULL != voxel_table );
	//ns_assert( NULL != position );

	voxel = NULL;
	_ns_voxel_table_lookup( voxel_table, position, &bucket, &voxel );

	return voxel;
	}


nsboolean ns_voxel_table_exists( const NsVoxelTable *voxel_table, const NsVector3i *position )
	{
	NsVoxel  *voxel;
	nssize    bucket;


	//ns_assert( NULL != voxel_table );
	//ns_assert( NULL != position );

	return _ns_voxel_table_lookup( voxel_table, position, &bucket, &voxel );
	}
