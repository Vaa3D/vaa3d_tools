#include "nstilemgr.h"


void ns_tile_mgr_construct( NsTileMgr *mgr )
	{
	ns_assert( NULL != mgr );

	ns_memzero( mgr, sizeof( NsTileMgr ) );
	mgr->tiles = NULL;
	}


void ns_tile_mgr_destruct( NsTileMgr *mgr )
	{
	ns_assert( NULL != mgr );
	ns_tile_mgr_clear( mgr );
	}


void ns_tile_mgr_clear( NsTileMgr *mgr )
	{
	nssize i;

	ns_assert( NULL != mgr );

	for( i = 0; i < mgr->total_num_tiles; ++i )
		ns_tile_destruct( mgr->tiles + i );

	ns_free( mgr->tiles );

	ns_memzero( mgr, sizeof( NsTileMgr ) );
	mgr->tiles = NULL;
	}


NsTile* ns_tile_mgr_at( const NsTileMgr *mgr, nssize index )
	{
	ns_assert( NULL != mgr );
	ns_assert( index < mgr->total_num_tiles );
	ns_assert( NULL != mgr->tiles );

	return mgr->tiles + index;
	}


NS_PRIVATE NsError _ns_tile_mgr_alloc
	(
	NsTileMgr      *mgr,
	nsboolean       lockable,
	const NsImage  *image,
	nssize          tile_width,
	nssize          tile_height,
	nssize          tile_length
	)
	{
	nssize   i;
	NsError  error;


	ns_tile_mgr_clear( mgr );

	mgr->num_pixels_x     = ns_image_width( image );
	mgr->num_pixels_y     = ns_image_height( image );
	mgr->num_pixels_z     = ns_image_length( image );
	mgr->total_num_pixels = mgr->num_pixels_x * mgr->num_pixels_y * mgr->num_pixels_z;

	mgr->tile_width  = tile_width;
	mgr->tile_height = tile_height;
	mgr->tile_length = tile_length;

	mgr->num_tiles_x = mgr->num_pixels_x / tile_width;
	mgr->num_tiles_y = mgr->num_pixels_y / tile_height;
	mgr->num_tiles_z = mgr->num_pixels_z / tile_length;

	mgr->last_tile_width  = 0;
	mgr->last_tile_height = 0;
	mgr->last_tile_length = 0;

	/* NOTE: The following will work even if e.g. tile_width > image_width. */

	if( 0 < mgr->num_pixels_x % tile_width )
		{
		mgr->last_tile_width = mgr->num_pixels_x - mgr->num_tiles_x * tile_width;
		++(mgr->num_tiles_x);
		}

	if( 0 < mgr->num_pixels_y % tile_height )
		{
		mgr->last_tile_height = mgr->num_pixels_y - mgr->num_tiles_y * tile_height;
		++(mgr->num_tiles_y);
		}

	if( 0 < mgr->num_pixels_z % tile_length )
		{
		mgr->last_tile_length = mgr->num_pixels_z - mgr->num_tiles_z * tile_length;
		++(mgr->num_tiles_z);
		}

	mgr->total_num_tiles = mgr->num_tiles_x * mgr->num_tiles_y * mgr->num_tiles_z;

	ns_assert( NULL == mgr->tiles );

	if( NULL == ( mgr->tiles = ns_new_array( NsTile, mgr->total_num_tiles ) ) )
		{
		ns_memzero( mgr, sizeof( NsTileMgr ) );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	for( i = 0; i < mgr->total_num_tiles; ++i )
		ns_tile_construct( mgr->tiles + i, lockable );

	for( i = 0; i < mgr->total_num_tiles; ++i )
		if( NS_FAILURE( ns_tile_create( mgr->tiles + i ), error ) )
			return error;

	ns_log_entry(
		NS_LOG_ENTRY_EVENT_BEGIN,
		NS_FUNCTION
		":: num_tiles={" NS_FMT_ULONG "," NS_FMT_ULONG "," NS_FMT_ULONG "}"
		" num_pixels={" NS_FMT_ULONG "," NS_FMT_ULONG "," NS_FMT_ULONG "}"
		" tile_size={" NS_FMT_ULONG "," NS_FMT_ULONG "," NS_FMT_ULONG "}"
		" last_tile_size={" NS_FMT_ULONG "," NS_FMT_ULONG "," NS_FMT_ULONG "}"
		" total_num_tiles=" NS_FMT_ULONG
		" total_num_pixels=" NS_FMT_ULONG,
		mgr->num_tiles_x, mgr->num_tiles_y, mgr->num_tiles_z,
		mgr->num_pixels_x, mgr->num_pixels_y, mgr->num_pixels_z,
		mgr->tile_width, mgr->tile_height, mgr->tile_length,
		mgr->last_tile_width, mgr->last_tile_height, mgr->last_tile_length,
		mgr->total_num_tiles,
		mgr->total_num_pixels
		);

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_tile_mgr_fill
	(
	NsTileMgr       *mgr,
	const NsImage   *image,
	nsboolean        dup_image,
	NsTileInitFunc   init_func,
	nspointer        user_data
	)
	{
	nssize         i;
	nssize         tx, ty, tz;
	nssize         px, py, pz;
	nssize         width, height, length;
	NsPixelType    pixel_type;
	nssize         row_align;
	nssize         bytes_per_row;
	nssize         bytes_per_slice;
	nssize         bits_per_pixel;
	nssize         bytes_per_pixel;
	nspointer      pixels;
	NsPixelRegion  region;
	NsError        error;
	nschar         name[ NS_MUTEX_NAME_SIZE + 16 ];


	pixel_type = ns_image_pixel_type( image );
	row_align  = ns_image_row_align( image );

	/* NOTE: This code will only work on pixels that dont cross byte boundaries! */
	bits_per_pixel = ns_pixel_bits( pixel_type );
	ns_verify( 0 == ( bits_per_pixel % 8 ) );

	bytes_per_pixel = bits_per_pixel / 8;
	bytes_per_row   = ns_pixel_buffer_row_size( pixel_type, mgr->num_pixels_x, row_align );
	bytes_per_slice = ns_pixel_buffer_slice_size( pixel_type, mgr->num_pixels_x, mgr->num_pixels_y, row_align );

	mgr->image_duplicated      = dup_image;
	mgr->image_pixel_type      = pixel_type;
	mgr->image_row_align       = row_align;
	mgr->image_bytes_per_slice = bytes_per_slice;
	mgr->image_bytes_per_row   = bytes_per_row;

	i = 0;

	for( pz = 0, tz = 0; tz < mgr->num_tiles_z; ++tz, pz += mgr->tile_length )
		for( py = 0, ty = 0; ty < mgr->num_tiles_y; ++ty, py += mgr->tile_height )
			for( px = 0, tx = 0; tx < mgr->num_tiles_x; ++tx, px += mgr->tile_width )
				{
				width  = ( tx == mgr->num_tiles_x - 1 && 0 < mgr->last_tile_width  ) ?
							mgr->last_tile_width : mgr->tile_width;

				height = ( ty == mgr->num_tiles_y - 1 && 0 < mgr->last_tile_height ) ?
							mgr->last_tile_height : mgr->tile_height;

				length = ( tz == mgr->num_tiles_z - 1 && 0 < mgr->last_tile_length ) ?
							mgr->last_tile_length : mgr->tile_length;

				if( dup_image )
					{
					/* IMPORTANT: This branch hasnt been tested! */

					region.x      = px;
					region.y      = py;
					region.z      = pz;
					region.width  = width;
					region.height = height;
					region.length = length;

					if( NS_FAILURE(
							ns_image_assign(
								ns_tile_image( mgr->tiles + i ),
								image,
								&region,
								NULL
								),
							error ) )
						return error;
					}
				else
					{
					pixels = ns_image_pixels( image );
					pixels = NS_OFFSET_POINTER( void, pixels, pz * bytes_per_slice );
					pixels = NS_OFFSET_POINTER( void, pixels, py * bytes_per_row );
					pixels = NS_OFFSET_POINTER( void, pixels, px * bytes_per_pixel );

					ns_assert( ( nssize )NS_POINTER_DISTANCE( pixels, ns_image_pixels( image ) ) < ns_image_size( image ) );

					ns_image_buffer_and_release(
						ns_tile_image( mgr->tiles + i ),
						pixel_type,
						width,
						height,
						length,
						1,
						pixels
						);
					}

				ns_tile_set_index_x( mgr->tiles + i, ( nsint )tx );
				ns_tile_set_index_y( mgr->tiles + i, ( nsint )ty );
				ns_tile_set_index_z( mgr->tiles + i, ( nsint )tz );

				ns_tile_set_pixel_x( mgr->tiles + i, px );
				ns_tile_set_pixel_y( mgr->tiles + i, py );
				ns_tile_set_pixel_z( mgr->tiles + i, pz );

				ns_assert( px < ns_image_width( image ) );
				ns_assert( py < ns_image_height( image ) );
				ns_assert( pz < ns_image_length( image ) );
				ns_assert( width <= ns_image_width( image ) );
				ns_assert( height <= ns_image_height( image ) );
				ns_assert( length <= ns_image_length( image ) );
				ns_assert( px + width <= ns_image_width( image ) );
				ns_assert( py + height <= ns_image_height( image ) );
				ns_assert( pz + length <= ns_image_length( image ) );

				if( NULL != init_func )
					if( NS_FAILURE( ( init_func )( mgr, mgr->tiles + i, user_data ), error ) )
						return error;

				ns_snprint(
					name,
					NS_MUTEX_NAME_SIZE,
					"tile" NS_FMT_ULONG "," NS_FMT_ULONG "," NS_FMT_ULONG,
					tx, ty, tz
					);

				ns_mutex_set_name( ns_tile_mutex( mgr->tiles + i ), name );

				++i;
				}

	ns_assert( i == mgr->total_num_tiles );
	return ns_no_error();
	}


NsError ns_tile_mgr_create
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
	)
	{
	NsError error;

	ns_assert( NULL != mgr );
	ns_assert( NULL != image );
	ns_assert( 0 < tile_width );
	ns_assert( 0 < tile_height );
	ns_assert( 0 < tile_length );

	/* TEMP?: An optimization... if we've already allocated tiles and
		the sizes i.e., image size, tile size, etc. are the same as a
		previous call to this create function, then we dont need to
		clear and allocate. */
	if( ns_image_width( image )  != mgr->num_pixels_x ||
		 ns_image_height( image ) != mgr->num_pixels_y ||
		 ns_image_length( image ) != mgr->num_pixels_z ||
		 tile_width  != mgr->tile_width  ||
		 tile_height != mgr->tile_height ||
		 tile_length != mgr->tile_length )
		if( NS_FAILURE(
				_ns_tile_mgr_alloc(
					mgr,
					lockable,
					image,
					tile_width,
					tile_height,
					tile_length
					),
				error ) )
			{
			ns_tile_mgr_clear( mgr );
			return error;
			}

	if( NS_FAILURE( _ns_tile_mgr_fill( mgr, image, dup_image, init_func, user_data ), error ) )
		{
		ns_tile_mgr_clear( mgr );
		return error;
		}

	return ns_no_error();
	}


void ns_tile_mgr_render
	(
	const NsTileMgr   *mgr,
	NsTileRenderFunc   render_func,
	nspointer          user_data
	)
	{
	nssize i;

	ns_assert( NULL != mgr );
	ns_assert( NULL != render_func );

	for( i = 0; i < mgr->total_num_tiles; ++i )
		( render_func )( mgr, mgr->tiles + i, user_data );
	}


void ns_tile_mgr_render_ex
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
	)
	{
	nsint  num_tiles_x, num_tiles_y;
	nsint  num_pixels_x, num_pixels_y;
	nsint  last_tile_width, last_tile_height;
	nsint  tx, ty;
	nsint  px, py;
	nsint  tx1, tx2, ty1, ty2;


	num_pixels_x     = ( nsint )ns_image_width( image );
	num_pixels_y     = ( nsint )ns_image_height( image );
	num_tiles_x      = num_pixels_x / tile_width;
	num_tiles_y      = num_pixels_y / tile_height;
	last_tile_width  = 0;
	last_tile_height = 0;

	/* Compute the number of tiles. */

	if( 0 < num_pixels_x % tile_width )
		{
		last_tile_width = num_pixels_x - num_tiles_x * tile_width;
		++(num_tiles_x);
		}

	if( 0 < num_pixels_y % tile_height )
		{
		last_tile_height = num_pixels_y - num_tiles_y * tile_height;
		++(num_tiles_y);
		}

	tx1 = viewport_left   / tile_width;
	tx2 = viewport_right  / tile_width;
	ty1 = viewport_top    / tile_height;
	ty2 = viewport_bottom / tile_height;

	/* Check for image completely out of view. */

	if( tx2 < 0 || tx1 >= num_tiles_x || ty2 < 0 || ty1 >= num_tiles_y )
		return;

	/* Clip the tiles to the viewport. */

	if( tx1 < 0 )
		tx1 = 0;

	if( tx2 >= num_tiles_x )
		tx2 = num_tiles_x - 1;

	if( ty1 < 0 )
		ty1 = 0;

	if( ty2 >= num_tiles_y )
		ty2 = num_tiles_y - 1;

	for( py = ty1 * tile_height, ty = ty1; ty <= ty2; ++ty, py += tile_height )
		for( px = tx1 * tile_width, tx = tx1; tx <= tx2; ++tx, px += tile_width )
			{
			tile->image.image_info.width =
				( nssize )( ( tx == num_tiles_x - 1 && 0 < last_tile_width  ) ? last_tile_width : tile_width );

			tile->image.image_info.height =
				( nssize )( ( ty == num_tiles_y - 1 && 0 < last_tile_height ) ? last_tile_height : tile_height );

			ns_tile_set_pixel_x( tile, ( nssize )px );
			ns_tile_set_pixel_y( tile, ( nssize )py );

			( render_func )( image, tile, user_data );
			}
	}
