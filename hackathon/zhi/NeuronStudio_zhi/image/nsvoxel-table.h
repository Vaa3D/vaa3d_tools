#ifndef __NS_IMAGE_VOXEL_TABLE_H__
#define __NS_IMAGE_VOXEL_TABLE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nsprimes.h>
#include <std/nsenumeration.h>
#include <std/nslog.h>
#include <math/nsvector.h>

NS_DECLS_BEGIN

#define NS_VOXEL_NUM_OFFSETS  26
NS_IMPEXP const NsVector3i* ns_voxel_offsets( void );

/* L = left, R = right, T = top, B = bottom, N = near, F = far */
typedef enum
	{
	NS_VOXEL_OFFSET_LTN,
	NS_VOXEL_OFFSET_RTN,
	NS_VOXEL_OFFSET_LBN,
	NS_VOXEL_OFFSET_RBN,
	NS_VOXEL_OFFSET_LTF,
	NS_VOXEL_OFFSET_RTF,
	NS_VOXEL_OFFSET_LBF,
	NS_VOXEL_OFFSET_RBF,

	NS_VOXEL_OFFSET_LT,
	NS_VOXEL_OFFSET_RT,
	NS_VOXEL_OFFSET_LB,
	NS_VOXEL_OFFSET_RB,
	NS_VOXEL_OFFSET_LN,
	NS_VOXEL_OFFSET_RN,
	NS_VOXEL_OFFSET_LF,
	NS_VOXEL_OFFSET_RF,
	NS_VOXEL_OFFSET_TN,
	NS_VOXEL_OFFSET_BN,
	NS_VOXEL_OFFSET_TF,
	NS_VOXEL_OFFSET_BF,

	NS_VOXEL_OFFSET_L,
	NS_VOXEL_OFFSET_R,
	NS_VOXEL_OFFSET_T,
	NS_VOXEL_OFFSET_B,
	NS_VOXEL_OFFSET_N,
	NS_VOXEL_OFFSET_F,

	_NS_VOXEL_OFFSET_NUM_TYPES
	}
	NsVoxelOffsetType;

NS_COMPILE_TIME_BOOLEAN( _NS_VOXEL_OFFSET_NUM_TYPES, ==, NS_VOXEL_NUM_OFFSETS );


#define NS_VOXEL_NUM_FACE_OFFSETS  6
NS_IMPEXP const NsVector3i* ns_voxel_face_offsets( void );


#define NS_VOXEL_NUM_CORNER_OFFSETS  8
NS_IMPEXP const NsVector3i* ns_voxel_corner_offsets( void );


typedef struct _NsVoxel
	{
	nsfloat           intensity;
	nspointer         data;
	struct _NsVoxel  *next;
	NsVector3i        position;
	nsushort          flags;
	}
	NsVoxel;


typedef struct _NsVoxelTable
	{
	NsVoxel         **buckets;
	nssize            num_buckets;
	nssize            num_voxels;
	nssize            max_voxels;
	nssize            width_times_height;
	nssize            bucket;
	NsFinalizeFunc    finalize_func;
	nsboolean         recycle;
	nsint             width;
	nsint             height;
	nsint             length;
	}
	NsVoxelTable;



/* Call this to actually free th recycled NsVoxel structures. */
NS_IMPEXP void ns_voxel_table_end_recycling( void );



/* The 'finalize_func' is applied to the NsVoxel::data member.
	Pass NULL if not needed. */
NS_IMPEXP NsError ns_voxel_table_construct
	(
	NsVoxelTable    *voxel_table,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_voxel_table_destruct( NsVoxelTable *voxel_table );

NS_IMPEXP void ns_voxel_table_recycle( NsVoxelTable *voxel_table, nsboolean recycle );

#define ns_voxel_table_size( voxel_table )\
	( (voxel_table)->num_voxels )

#define ns_voxel_table_is_empty( voxel_table )\
	( 0 == ns_voxel_table_size( (voxel_table) ) )


#define ns_voxel_table_get_width( voxel_table )\
	( (voxel_table)->width )

#define ns_voxel_table_get_height( voxel_table )\
	( (voxel_table)->height )

#define ns_voxel_table_get_length( voxel_table )\
	( (voxel_table)->length )


NS_IMPEXP void ns_voxel_table_get_dimensions
	(
	const NsVoxelTable  *voxel_table,
	nsint               *width,
	nsint               *height,
	nsint               *length
	);

NS_IMPEXP void ns_voxel_table_set_dimensions
	(
	NsVoxelTable  *voxel_table,
	nsint          width,
	nsint          height,
	nsint          length
	);

NS_IMPEXP void ns_voxel_table_clear( NsVoxelTable *voxel_table );

/* Applies the 'callback' function to the voxels. If the
	function returns TRUE then the voxel is eliminated. */
NS_IMPEXP void ns_voxel_table_clean
	(
	NsVoxelTable  *voxel_table,
	nsboolean      ( *callback )( const NsVoxel*, nspointer ),
	nspointer      user_data
	);

NS_IMPEXP NsError ns_voxel_table_add
	(
	NsVoxelTable       *voxel_table,
	const NsVector3i   *position,
	nsfloat             intensity,
	nspointer           data,
	NsVoxel           **voxel
	);

NS_IMPEXP nsboolean ns_voxel_table_remove( NsVoxelTable *voxel_table, NsVoxel *voxel );

NS_IMPEXP NsVoxel* ns_voxel_table_find( const NsVoxelTable *voxel_table, const NsVector3i *position );
NS_IMPEXP nsboolean ns_voxel_table_exists( const NsVoxelTable *voxel_table, const NsVector3i *position );


#define NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )\
	for( ( ( NsVoxelTable* )(voxel_table) )->bucket = 0;\
		  (voxel_table)->bucket < (voxel_table)->num_buckets;\
		  ++( ( NsVoxelTable* )(voxel_table) )->bucket )\
		for( (voxel) = (voxel_table)->buckets[ (voxel_table)->bucket ]; NULL != (voxel); (voxel) = (voxel)->next )

NS_DECLS_END

#endif/* __NS_IMAGE_VOXEL_TABLE_H__ */
