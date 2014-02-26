#include "nstile.h"


void ns_tile_construct( NsTile *tile, nsboolean lockable )
	{
	ns_assert( NULL != tile );

	ns_mutex_construct( &tile->mutex );
	ns_image_construct( &tile->image );

	tile->lockable      = lockable;
	tile->object        = NULL;
	tile->finalize_func = NULL;
	}


void ns_tile_destruct( NsTile *tile )
	{
	ns_assert( NULL != tile );

	ns_tile_clear( tile );

	ns_image_destruct( &tile->image );
	ns_mutex_destruct( &tile->mutex );
	}


NsError ns_tile_create( NsTile *tile )
	{
	NsError error = ns_no_error();

	ns_assert( NULL != tile );

	if( tile->lockable )
		{
		ns_assert( ! ns_mutex_is_created( &tile->mutex ) );
		error = ns_mutex_create( &tile->mutex );
		}

	return error;
	}


void ns_tile_clear( NsTile *tile )
	{
	ns_assert( NULL != tile );

	if( NULL != tile->finalize_func )
		( tile->finalize_func )( tile->object );

	tile->object        = NULL;
	tile->finalize_func = NULL;

	ns_image_clear( &tile->image );
	}


NsImage* ns_tile_image( const NsTile *tile )
	{
	ns_assert( NULL != tile );
	return ( NsImage* )( &tile->image );
	}


nssize ns_tile_width( const NsTile *tile )
	{
	ns_assert( NULL != tile );
	return ns_image_width( &tile->image );
	}


nssize ns_tile_height( const NsTile *tile )
	{
	ns_assert( NULL != tile );
	return ns_image_height( &tile->image );
	}


nssize ns_tile_length( const NsTile *tile )
	{
	ns_assert( NULL != tile );
	return ns_image_length( &tile->image );
	}


/* NOTE: Ignoring error return on mutex functions,
	just for simplicity. */


void ns_tile_lock( NsTile *tile )
	{
	ns_assert( NULL != tile );

	if( tile->lockable )
		{
		ns_assert( ns_mutex_is_created( &tile->mutex ) );
		ns_mutex_lock( &tile->mutex );
		}
	}


void ns_tile_unlock( NsTile *tile )
	{
	ns_assert( NULL != tile );

	if( tile->lockable )
		{
		ns_assert( ns_mutex_is_created( &tile->mutex ) );
		ns_mutex_unlock( &tile->mutex );
		}
	}


nsboolean ns_tile_try_lock( NsTile *tile )
	{
	nsboolean locked = NS_FALSE;

	ns_assert( NULL != tile );

	if( tile->lockable )
		{
		ns_assert( ns_mutex_is_created( &tile->mutex ) );
		ns_mutex_try_lock( &tile->mutex, &locked );
		}

	return locked;
	}
