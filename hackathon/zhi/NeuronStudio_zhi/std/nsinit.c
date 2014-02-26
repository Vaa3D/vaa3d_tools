#include "nsinit.h"


NS_PRIVATE void _ns_mutexes_construct( void )
	{
	/* Assure all bits zero. */
	ns_memzero( ns_value_static_mutex(), sizeof( NsMutex ) );

	ns_mutex_construct( ns_value_static_mutex() );
	ns_mutex_set_name( ( NsMutex* )ns_value_static_mutex(), "ns_value_static_mutex" );
	}


NS_PRIVATE NsError _ns_mutexes_create( void )
	{
	NsError error;

	if( NS_FAILURE( ns_mutex_create( ns_value_static_mutex() ), error ) )
		return error;

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_do_init( void )
	{
	NsError error;

	_ns_pixels_init();

	_ns_mutexes_construct();

	/* NOTE: Mutex creation should always probably
		be called first. */
	if( NS_FAILURE( _ns_mutexes_create(), error ) )
		return error;

	if( NS_FAILURE( _ns_value_db_init(), error ) )
		return error;

	ns_value_register_std();

	_ns_profiler_init();

	return ns_no_error();
	}


NS_PRIVATE void _ns_mutexes_destruct( void )
	{
	ns_mutex_destruct( ns_value_static_mutex() );
	}


NS_PRIVATE void _ns_do_finalize( void )
	{
	_ns_profiler_finalize();

	_ns_value_db_finalize();

	_ns_mutexes_destruct();
	}


NsError ns_init( nsint argc, const nschar* argv[] )
	{
	NsError error;

	NS_USE_VARIABLE( argc );
	NS_USE_VARIABLE( argv );

	if( NS_FAILURE( _ns_do_init(), error ) )
		{
		_ns_do_finalize();
		return error;
		}

	return ns_no_error();
	}


void ns_finalize( void )
	{
	_ns_do_finalize();
	}
