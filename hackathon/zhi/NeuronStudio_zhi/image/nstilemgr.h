#ifndef __NS_IMAGE_TILE_MGR_H__
#define __NS_IMAGE_TILE_MGR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslog.h>
#include <image/nsimage.h>
#include <image/nstile.h>

NS_DECLS_BEGIN

typedef struct _NsTileMgr
	{
	NsTile      *tiles;
	nssize       num_tiles_x;
	nssize       num_tiles_y;
	nssize       num_tiles_z;
	nssize       total_num_tiles;
	nssize       num_pixels_x;
	nssize       num_pixels_y;
	nssize       num_pixels_z;
	nssize       total_num_pixels;
	nssize       tile_width;
	nssize       tile_height;
	nssize       tile_length;
	nssize       last_tile_width;
	nssize       last_tile_height;
	nssize       last_tile_length;
	nsboolean    image_duplicated;
	nssize       image_row_align;
	NsPixelType  image_pixel_type;
	nssize       image_bytes_per_slice;
	nssize       image_bytes_per_row;
	}
	NsTileMgr;


NS_IMPEXP void ns_tile_mgr_construct( NsTileMgr *mgr );
NS_IMPEXP void ns_tile_mgr_destruct( NsTileMgr *mgr );

NS_IMPEXP void ns_tile_mgr_clear( NsTileMgr *mgr );


#define ns_tile_mgr_total_num_tiles( mgr )\
	( (mgr)->total_num_tiles )

#define ns_tile_mgr_total_num_pixels( mgr )\
	( (mgr)->total_num_pixels )


NS_IMPEXP NsTile* ns_tile_mgr_at( const NsTileMgr *mgr, nssize index );


#define ns_tile_mgr_num_tiles_x( mgr )\
	( (mgr)->num_tiles_x )

#define ns_tile_mgr_num_tiles_y( mgr )\
	( (mgr)->num_tiles_y )

#define ns_tile_mgr_num_tiles_z( mgr )\
	( (mgr)->num_tiles_z )

#define ns_tile_mgr_num_pixels_x( mgr )\
	( (mgr)->num_pixels_x )

#define ns_tile_mgr_num_pixels_y( mgr )\
	( (mgr)->num_pixels_y )

#define ns_tile_mgr_num_pixels_z( mgr )\
	( (mgr)->num_pixels_z )

#define ns_tile_mgr_tile_width( mgr )\
	( (mgr)->tile_width )

#define ns_tile_mgr_tile_height( mgr )\
	( (mgr)->tile_height )

#define ns_tile_mgr_tile_length( mgr )\
	( (mgr)->tile_length )

#define ns_tile_mgr_last_tile_width( mgr )\
	( (mgr)->last_tile_width )

#define ns_tile_mgr_last_tile_height( mgr )\
	( (mgr)->last_tile_height )

#define ns_tile_mgr_last_tile_length( mgr )\
	( (mgr)->last_tile_length )


typedef NsError ( *NsTileInitFunc )
	(
	const NsTileMgr  *mgr,
	NsTile           *tile,
	nspointer         user_data
	);

typedef void ( *NsTileRenderFunc )
	(
	const NsTileMgr  *mgr,
	NsTile           *tile,
	nspointer         user_data
	);


/* The width, height, and length are the preferred size in
	pixels of each tile. The last tile in each dimension may
	be smaller than this.

	If 'dup_image' is true then the passed image is duplicated
	( and tiled obviously ), otherwise the tiles will just "point"
	into the passed image.

	If 'callback' is not NULL, then it is called for each tile
	created, and passed the 'user_data'. This can be used to do
	any user-defined initialization of the tile.

	Example (in one dimension):

	- Image is 1017 pixels wide...
	- Tile width is passed as 256.
	- 1017 / 256 = 3.97 therefore we need 4 tiles.
	- Tiles = |256|256|256|249|
*/
NS_IMPEXP NsError ns_tile_mgr_create
	(
	NsTileMgr       *mgr,
	nsboolean        lockable,
	const NsImage   *image,
	nsboolean        dup_image,
	nssize           tile_width,
	nssize           tile_height,
	nssize           tile_length,
	NsTileInitFunc   init_func,
	nspointer        user_data
	);


NS_IMPEXP void ns_tile_mgr_render
	(
	const NsTileMgr   *mgr,
	NsTileRenderFunc   render_func,
	nspointer          user_data
	);




typedef void ( *NsTileRenderExFunc )
	(
	const NsImage  *image,
	const NsTile   *tile,
	nspointer       user_data
	);

NS_IMPEXP void ns_tile_mgr_render_ex
	(
	NsTile              *tile,
	const NsImage       *image,
	nsint                tile_width,
	nsint                tile_height,
	nsint                viewport_left,
	nsint                viewport_top,
	nsint                viewport_right,
	nsint                viewport_bottom,
	NsTileRenderExFunc   render_func,
	nspointer            user_data
	);

NS_DECLS_END

#endif/* __NS_IMAGE_TILE_MGR_H__ */
