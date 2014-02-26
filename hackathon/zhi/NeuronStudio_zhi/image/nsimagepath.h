#ifndef __NS_IMAGE_IMAGE_PATH_H__
#define __NS_IMAGE_IMAGE_PATH_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsbinheap.h>
#include <math/nspoint.h>
#include <math/nsvector.h>
#include <image/nspixels.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nsvoxel-table.h>

NS_DECLS_BEGIN

typedef struct _NsImagePathNode
	{
	NsVector3i  position;
	}
	NsImagePathNode;

typedef struct _NsImagePath
	{
	NsList  nodes;
	}
	NsImagePath;

typedef nslistiter nsimagepathiter;


/* Function to terminate path creation. For a given x,y,z in image
	space the function should return true to continue, else false to
	terminate. */
typedef nsboolean ( *NsImagePathFunc )( const NsPoint3i *position, nspointer user_data );


NS_IMPEXP void ns_image_path_construct( NsImagePath *path );
NS_IMPEXP void ns_image_path_destruct( NsImagePath *path );

/* NOTE: 'user_data' is passed to the 'path_func'. */
NS_IMPEXP NsError ns_image_path_create
	(
	NsImagePath         *path,
	const NsImage       *image,
	const NsPoint3f     *origin,
	const NsVoxelTable  *voxel_table,
	const NsVoxelInfo   *voxel_info,
	nsfloat              max_path_length,
	NsImagePathFunc      path_func,
	nspointer            user_data
	);

NS_IMPEXP nssize ns_image_path_size( const NsImagePath *path );
NS_IMPEXP nsboolean ns_image_path_is_empty( const NsImagePath *path );

NS_IMPEXP void ns_image_path_clear( NsImagePath *path );

NS_IMPEXP nsimagepathiter ns_image_path_begin( const NsImagePath *path );
NS_IMPEXP nsimagepathiter ns_image_path_end( const NsImagePath *path );

#define ns_image_path_iter_next( I )\
	ns_list_iter_next( (I) )

#define ns_image_path_iter_prev( I )\
	ns_list_iter_prev( (I) )

#define ns_image_path_iter_equal( I1, I2 )\
	ns_list_iter_equal( (I1), (I2) )

#define ns_image_path_iter_offset( I, n )\
	ns_list_iter_offset( (I), (n) )

#define ns_image_path_iter_not_equal( I1, I2 )\
	ns_list_iter_not_equal( (I1), (I2) )

#define ns_image_path_node( I )\
	( ( const NsImagePathNode* )ns_list_iter_get_object( (I) ) )

NS_DECLS_END

#endif/* __NS_IMAGE_IMAGE_PATH_H__ */
