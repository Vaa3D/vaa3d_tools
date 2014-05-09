#include "nsrecdir.h"


NS_PRIVATE NsError _ns_dir_new( NsDir **dir )
	{
	if( NULL == ( *dir = ns_malloc( sizeof( NsDir ) ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_dir_construct( *dir );
	return ns_no_error();
	}


NS_PRIVATE void _ns_dir_delete( NsDir *dir )
	{
	ns_dir_destruct( dir );
	ns_free( dir );
	}


void ns_rec_dir_construct( NsRecDir *rdir )
	{
	ns_stack_construct( &rdir->stack, _ns_dir_delete );
	rdir->recursed = NS_FALSE;
	}


void ns_rec_dir_destruct( NsRecDir *rdir )
	{
   if( ns_rec_dir_is_open( rdir ) )
      ns_rec_dir_close( rdir );

	ns_stack_destruct( &rdir->stack );
	}


nsboolean ns_rec_dir_is_open( const NsRecDir *rdir )
	{
   ns_assert( NULL != rdir );
	return 0 < ns_stack_size( &rdir->stack );
	}


nssize ns_rec_dir_depth( const NsRecDir *rdir )
	{
	nssize depth;

	ns_assert( NULL != rdir );
	depth = ns_stack_size( &rdir->stack );

	if( rdir->recursed )
		{
		ns_assert( 0 < depth );
		--depth;
		}

	return depth;
	}


NS_PRIVATE NsError _ns_rec_dir_enter( NsRecDir *rdir, const nschar *name )
	{
	NsDir    *dir;
	NsError   error;


	if( NS_FAILURE( ns_chdir( name ), error ) )
		return error;

	if( NS_FAILURE( _ns_dir_new( &dir ), error ) )
		return error;

	if( NS_FAILURE( ns_dir_open( dir ), error ) )
		{
		_ns_dir_delete( dir );
		return error;
		}

	if( NS_FAILURE( ns_stack_push( &rdir->stack, dir ), error ) )
		{
		_ns_dir_delete( dir );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_rec_dir_leave( NsRecDir *rdir )
	{
	ns_assert( ! ns_stack_is_empty( &rdir->stack ) );
	ns_stack_pop( &rdir->stack );

	return ns_chdir( ".." );
	}


NsError ns_rec_dir_open( NsRecDir *rdir )
	{
	ns_assert( NULL != rdir );
	ns_assert( ! ns_rec_dir_is_open( rdir ) );

	return _ns_rec_dir_enter( rdir, "." );
	}


void ns_rec_dir_close( NsRecDir *rdir )
	{
	ns_assert( NULL != rdir );
	ns_assert( ns_rec_dir_is_open( rdir ) );

	ns_stack_clear( &rdir->stack );
	rdir->recursed = NS_FALSE;
	}


NsError ns_rec_dir_rewind( NsRecDir *rdir )
	{
	NsError error;

	ns_assert( NULL != rdir );
	ns_assert( ns_rec_dir_is_open( rdir ) );

	/* Go back to the original directory where open() was called. */
	while( 1 < ns_stack_size( &rdir->stack ) )
		if( NS_FAILURE( _ns_rec_dir_leave( rdir ), error ) )
			return error;

	/* Simpler to just close and re-open. */
	ns_rec_dir_close( rdir );
	return ns_rec_dir_open( rdir );
	}


NS_PRIVATE NsError _ns_rec_dir_do_read( NsRecDir *rdir, const nschar **name, nsboolean *ret_is_dir )
	{
	NsDir      *dir;
	NsError     error;
	nsboolean   is_dir;


	rdir->recursed = NS_FALSE;

	/* Read a name... if null, we are at the end of a directory,
		so go back to the parent directory. Continue until we find
		a non-null name or we are at the original directory where
		open() was called. */
	dir   = ns_stack_peek( &rdir->stack );
	*name = ns_dir_read( dir );

	while( NULL == *name && 1 < ns_stack_size( &rdir->stack ) )
		{
		if( NS_FAILURE( _ns_rec_dir_leave( rdir ), error ) )
			return error;

		dir   = ns_stack_peek( &rdir->stack );
		*name = ns_dir_read( dir );
		}

	if( NULL != *name )
		{
		if( NS_FAILURE( ns_isdir( *name, &is_dir ), error ) )
			return error;

		if( NULL != ret_is_dir )
			*ret_is_dir = is_dir;

		if( is_dir &&
			 ! ns_ascii_streq( *name, "." ) &&
			 ! ns_ascii_streq( *name, ".." ) )
			{
			if( NS_FAILURE( _ns_rec_dir_enter( rdir, *name ), error ) )
				return error;

			/* Used to report an accurate depth(). */
			rdir->recursed = NS_TRUE;
			}
		}

	return ns_no_error();
	}


NsError ns_rec_dir_read( NsRecDir *rdir, const nschar **name )
	{
	ns_assert( NULL != rdir );
	ns_assert( NULL != name );
	ns_assert( ns_rec_dir_is_open( rdir ) );

	return _ns_rec_dir_do_read( rdir, name, NULL );
	}


NsError ns_rec_dir_read_ex( NsRecDir *rdir, const nschar **name, nsboolean *is_dir )
	{
	ns_assert( NULL != rdir );
	ns_assert( NULL != name );
	ns_assert( NULL != is_dir );

	return _ns_rec_dir_do_read( rdir, name, is_dir );
	}


NsError ns_rec_dir_advance( NsRecDir *rdir, const nschar **name )
	{
	nsboolean  is_dir;
	NsError    error;


	ns_assert( NULL != rdir );
	ns_assert( NULL != name );

	do
		{
		if( NS_FAILURE( ns_rec_dir_read_ex( rdir, name, &is_dir ), error ) )
			return error;
		}
	while( NULL != *name && ! is_dir );

	return ns_no_error();
	}


const NsDir* ns_rec_dir_curr( const NsRecDir *rdir )
	{
	const NsDir *dir = NULL;

	ns_assert( NULL != rdir );

	if( ns_rec_dir_is_open( rdir ) )
		dir = ns_stack_peek( &rdir->stack );

	return dir;
	}



NsError ns_rec_dir_curr_full_path( const NsRecDir *rdir, NsString *path )
	{
	ns_assert( NULL != rdir );
	ns_assert( NULL != path );

	return ns_string_set( path, ns_dir_name( ns_rec_dir_curr( rdir ) ) );
	}


NsError ns_rec_dir_curr_relative_path( const NsRecDir *rdir, NsString *path )
	{
	nslistiter     curr, end;
	const nschar  *at;
	NsError        error;


	ns_assert( NULL != rdir );
	ns_assert( NULL != path );

	ns_string_set( path, "." );

	/* IMPORTANT: This exposes the implementation of the stack, but oh well... */
	curr = ns_list_begin( &rdir->stack.list );
	end  = ns_list_end( &rdir->stack.list );

	if( ns_list_iter_not_equal( curr, end ) )
		{
		/* NOTE: Skip over the directory where the 'rdir' was opened.
			We could assert that the list has at least one node, but
			its just as easy to do the previous check. */
		curr = ns_list_iter_next( curr );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			if( NULL != ( at = ns_ascii_strrchr( ns_dir_name( ns_list_iter_get_object( curr ) ), NS_UNICHAR_PATH ) ) )
				if( NS_FAILURE( ns_string_append( path, at ), error ) )
					return error;
		}

	return ns_no_error();
	}
