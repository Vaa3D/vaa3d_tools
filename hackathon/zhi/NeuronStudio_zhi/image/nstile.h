#ifndef __NS_IMAGE_TILE_H__
#define __NS_IMAGE_TILE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmutex.h>
#include <image/nsimage.h>

NS_DECLS_BEGIN

/* NOTE: The 'finalize_func' is applied to the 'object'
	when the tile is destructed, that is if not NULL.
	The pixel coordinate is the left,top,near pixel. */
typedef struct _NsTile
	{
	NsImage         image;
	nsint           index_x;
	nsint           index_y;
	nsint           index_z;
	nssize          pixel_x;
	nssize          pixel_y;
	nssize          pixel_z;
	nspointer       object;
	NsFinalizeFunc  finalize_func;
	nsboolean       lockable;
	NsMutex         mutex;
	}
	NsTile;


NS_IMPEXP void ns_tile_construct( NsTile *tile, nsboolean lockable );
NS_IMPEXP void ns_tile_destruct( NsTile *tile );

/* This function call is really only necessary if the tile
	is lockable, but safest just to call anyway. */
NS_IMPEXP NsError ns_tile_create( NsTile *tile );

NS_IMPEXP void ns_tile_clear( NsTile *tile );

NS_IMPEXP NsImage* ns_tile_image( const NsTile *tile );

NS_IMPEXP nssize ns_tile_width( const NsTile *tile );
NS_IMPEXP nssize ns_tile_height( const NsTile *tile );
NS_IMPEXP nssize ns_tile_length( const NsTile *tile );

NS_IMPEXP void ns_tile_lock( NsTile *tile );
NS_IMPEXP void ns_tile_unlock( NsTile *tile );
NS_IMPEXP nsboolean ns_tile_try_lock( NsTile *tile );

#define ns_tile_mutex( tile )\
	( &( (tile)->mutex ) )

#define ns_tile_get_index_x( tile )\
	( (tile)->index_x )

#define ns_tile_get_index_y( tile )\
	( (tile)->index_y )

#define ns_tile_get_index_z( tile )\
	( (tile)->index_z )

#define ns_tile_set_index_x( tile, x )\
	( (tile)->index_x = (x) )

#define ns_tile_set_index_y( tile, y )\
	( (tile)->index_y = (y) )

#define ns_tile_set_index_z( tile, z )\
	( (tile)->index_z = (z) )

#define ns_tile_get_pixel_x( tile )\
	( (tile)->pixel_x )

#define ns_tile_get_pixel_y( tile )\
	( (tile)->pixel_y )

#define ns_tile_get_pixel_z( tile )\
	( (tile)->pixel_z )

#define ns_tile_set_pixel_x( tile, x )\
	( (tile)->pixel_x = (x) )

#define ns_tile_set_pixel_y( tile, y )\
	( (tile)->pixel_y = (y) )

#define ns_tile_set_pixel_z( tile, z )\
	( (tile)->pixel_z = (z) )

#define ns_tile_get_object( tile )\
	( (tile)->object )

#define ns_tile_set_object( tile, _object )\
	( (tile)->object = (_object) )

#define ns_tile_get_finalize_func( tile )\
	( (tile)->finalize_func )

#define ns_tile_set_finalize_func( tile, _finalize_func )\
	( (tile)->finalize_func = (_finalize_func) )

NS_DECLS_END

#endif/* __NS_IMAGE_TILE_H__ */
