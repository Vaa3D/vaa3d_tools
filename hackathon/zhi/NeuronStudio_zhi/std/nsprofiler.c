#include "nsprofiler.h"


typedef struct _NsProfile
	{
	const nschar  *name;
   nssize         num_calls;
   nssize         recursive_count;
	nsdouble       elapsed_seconds;
	nstimer        timer;
	}
	NsProfile;


NS_PRIVATE NsError ns_profile_new( NsProfile **profile, const nschar *name )
	{
	if( NULL == ( *profile = ns_new( NsProfile ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL == ( (*profile)->name = ns_ascii_strdup( name ) ) )
		{
		ns_delete( *profile );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	(*profile)->num_calls       = 0;
	(*profile)->recursive_count = 0;
	(*profile)->elapsed_seconds = 0.0;

	return ns_no_error();
	}


NS_PRIVATE void ns_profile_delete( NsProfile *profile )
	{
	ns_delete( ( nspointer )profile->name );
	ns_delete( profile );
	}


NS_PRIVATE const nschar* ns_profile_name( const NsProfile *profile )
	{  return profile->name;  }


NS_PRIVATE nssize ns_profile_name_length( const NsProfile *profile )
	{  return ns_ascii_strlen( profile->name );  }




NS_PRIVATE void _ns_profiler_reset( NsProfiler *profiler )
	{
	profiler->elapsed_seconds = 0.0;
	profiler->curr_profile    = ns_hier_root( &profiler->profiles );
	profiler->begin_count     = 0;
	}


void ns_profiler_construct( NsProfiler *profiler )
	{
	ns_assert( NULL != profiler );

	ns_hier_construct( &profiler->profiles, ns_profile_delete );

	ns_mutex_construct( &profiler->mutex );
	ns_mutex_set_name( &profiler->mutex, "profiler->mutex" );
	ns_mutex_create( &profiler->mutex ); /* NOTE: Ignore failure. */

	_ns_profiler_reset( profiler );
	}


void ns_profiler_destruct( NsProfiler *profiler )
	{
	ns_assert( NULL != profiler );

	ns_hier_destruct( &profiler->profiles );
	ns_mutex_destruct( &profiler->mutex );
	}


void ns_profiler_clear( NsProfiler *profiler )
	{
	ns_assert( NULL != profiler );

	ns_hier_clear( &profiler->profiles );
	_ns_profiler_reset( profiler );
	}


NS_PRIVATE NsProfile* _ns_profile( const nshieriter I )
	{
	NsProfile *profile = ns_hier_iter_get_object( I );

	ns_assert( NULL != profile );
	return profile;
	}


NS_PRIVATE nsboolean _ns_profiler_not_root( const NsProfiler *profiler, nshieriter I )
	{
	ns_assert( NULL != profiler );
	return ns_hier_iter_not_equal( I, ns_hier_root( &profiler->profiles ) );
	}


NS_PRIVATE NsProfile* _ns_profiler_find( NsProfiler *profiler, const nschar *name )
   {
	NsProfile   *profile;
	nshieriter   child;
	NsError      error;


	if( _ns_profiler_not_root( profiler, profiler->curr_profile ) )
		{
		/* First check if the 'name' is the current profile. This can happen
			during recursive function calls. */
		profile = _ns_profile( profiler->curr_profile );

		if( ns_ascii_streq( ns_profile_name( profile ), name ) )
			return profile;
		}

	/* Search all children of the current profile and set the current
		profile to the child if found. */
   if( ns_hier_iter_has_child( profiler->curr_profile ) )
		{
		child = ns_hier_iter_child( profiler->curr_profile );

		NS_INFINITE_LOOP
			{
			profile = _ns_profile( child );

			if( ns_ascii_streq( ns_profile_name( profile ), name ) )
				{
				profiler->curr_profile = child;
				return profile;
				}

			if( ! ns_hier_iter_has_sibling( child ) )
				break;

			child = ns_hier_iter_sibling( child );
			}
		}

	/* Didnt find it so allocate a new profile. NOTE: Ignoring memory
		allocation failures.  */
	if( NS_FAILURE( ns_profile_new( &profile, name ), error ) )
		return NULL;

	if( NS_FAILURE( ns_hier_insert_child(
							&profiler->profiles,
							profiler->curr_profile,
							profile
							),
							error ) )
		{
		ns_profile_delete( profile );
		return NULL;
		}

	profiler->curr_profile = ns_hier_iter_child( profiler->curr_profile );
	return profile;
   }


NS_PRIVATE void _ns_profiler_begin( NsProfiler *profiler, NsProfile *profile )
	{
	if( 0 == profiler->begin_count )
		profiler->timer = ns_timer();

	++(profiler->begin_count );

	profile->timer = ns_timer();
	}


NS_PRIVATE void _ns_profiler_end( NsProfiler *profiler, NsProfile *profile )
	{
	ns_assert( 0 < profiler->begin_count );
	--(profiler->begin_count );

	if( 0 == profiler->begin_count )
		profiler->elapsed_seconds = ns_difftimer( ns_timer(), profiler->timer );

	profile->elapsed_seconds += ns_difftimer( ns_timer(), profile->timer );
	}


void ns_profiler_begin( NsProfiler *profiler, const nschar* name )
	{
#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )

   NsProfile *profile;

   ns_assert( NULL != profiler );
   ns_assert( NULL != name );

	ns_mutex_lock( &profiler->mutex );

	/* Find or allocate a profile node. */
	if( NULL != ( profile = _ns_profiler_find( profiler, name ) ) )
		{
		++(profile->num_calls);
		++(profile->recursive_count);

		/* Get the start time only if non-recursive call. */
		if( 1 == profile->recursive_count )
			_ns_profiler_begin( profiler, profile );
		}

	ns_mutex_unlock( &profiler->mutex );

#else
	NS_USE_VARIABLE( profiler );
	NS_USE_VARIABLE( name );
#endif
	}


void ns_profiler_end( NsProfiler *profiler )
	{
#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )

	NsProfile *profile;

	ns_assert( NULL != profiler );

	ns_mutex_lock( &profiler->mutex );

	/* NOTE: The current profile shouldnt be the root but it
		could happen if there was a memory allocation failure
		in the call to ns_profiler_begin(). */
   if( _ns_profiler_not_root( profiler, profiler->curr_profile ) )
      {
		profile = _ns_profile( profiler->curr_profile );

      ns_assert( 0 < profile->recursive_count );
      --(profile->recursive_count);

      /* Proceed up the "call stack" if end of recursion. */
      if( 0 == profile->recursive_count )
         {
         _ns_profiler_end( profiler, profile );
			profiler->curr_profile = ns_hier_iter_parent( profiler->curr_profile );
         }
      }

	ns_mutex_unlock( &profiler->mutex );

#else
	NS_USE_VARIABLE( profiler );
#endif
	}


#define _NS_PROFILER_END( profiler, value )\
	ns_assert( NULL != (profiler) );\
	ns_profiler_end( (profiler) );\
	return (value)


NsError ns_profiler_end_error( NsProfiler *profiler, NsError value )
	{  _NS_PROFILER_END( profiler, value );  }

nspointer ns_profiler_end_pointer( NsProfiler *profiler, nspointer value )
	{  _NS_PROFILER_END( profiler, value );  }

nslong ns_profiler_end_long( NsProfiler *profiler, nslong value )
	{  _NS_PROFILER_END( profiler, value );  }

nsulong ns_profiler_end_ulong( NsProfiler *profiler, nsulong value )
	{  _NS_PROFILER_END( profiler, value );  }

#ifdef E_HAVE_LONGLONG

nslonglong ns_profiler_end_longlong( NsProfiler *profiler, nslonglong value )
	{  _NS_PROFILER_END( profiler, value );  }

nsulonglong ns_profiler_end_ulonglong( NsProfiler *profiler, vulonglong value )
	{  _NS_PROFILER_END( profiler, value );  }

#endif/* E_HAVE_LONGLONG */

nsfloat ns_profiler_end_float( NsProfiler *profiler, nsfloat value )
	{  _NS_PROFILER_END( profiler, value );  }

nsdouble ns_profiler_end_double( NsProfiler *profiler, nsdouble value )
	{  _NS_PROFILER_END( profiler, value );  }




NS_PRIVATE void _ns_profiler_shell_cmd_cd( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_exit( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_help( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_hier( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_ls( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_pwd( NsProfiler*, nsint, nschar** );
NS_PRIVATE void _ns_profiler_shell_cmd_man( NsProfiler*, nsint, nschar** );

#define _NS_PROFILER_NUM_SHELL_CMDS  7

typedef struct _NsProfilerShellCmd
	{
	const nschar  *name;
	const nschar  *alt_name;
	const nschar  *help;
	const nschar  *usage;
	const nschar  *manual;
	void ( *func )( NsProfiler*, nsint, nschar** );
	}
	NsProfilerShellCmd;


/* Could use a hash-table, but this is simpler. */
NS_PRIVATE NsProfilerShellCmd _ns_profiler_shell_cmds[ _NS_PROFILER_NUM_SHELL_CMDS ] =
	{
	{
	"cd",
	"",
	"Changes the parent profile.",
	"[name]",
	"<none>",
	_ns_profiler_shell_cmd_cd
	},
	{
	"exit",
	"",
	"Exits the profiler shell.",
	"",
	"<none>",
	_ns_profiler_shell_cmd_exit
	},
	{
	"help",
	"",
	"Displays help information for a command.",
	"[command]",
	"<none>",
	_ns_profiler_shell_cmd_help
	},
	{
	"hier",
	"",
	"Displays the profile hierarchy.",
	"",
	"<none>",
	_ns_profiler_shell_cmd_hier
	},
	{
	"ls",
	"dir",
	"Lists the child profiles.",
	"",
	"<none>",
	_ns_profiler_shell_cmd_ls
	},
	{
	"pwd",
	"",
	"Displays the parent profile path.",
	"",
	"<none>",
	_ns_profiler_shell_cmd_pwd
	},
	{
	"man",
	"",
	"Displays user manual for a command.",
	"[command]",
	"<none>",
	_ns_profiler_shell_cmd_man
	}
	};


NS_PRIVATE nssize _ns_profiler_lookup_shell_cmd( const nschar *name )
	{
	nssize index;

	for( index = 0; index < _NS_PROFILER_NUM_SHELL_CMDS; ++index )
		if( ns_ascii_streq( _ns_profiler_shell_cmds[ index ].name, name ) ||
			 ns_ascii_streq( _ns_profiler_shell_cmds[ index ].alt_name, name ) )
			return index;

	return _NS_PROFILER_NUM_SHELL_CMDS;
	}


NS_PRIVATE void _ns_profiler_output_shell_cmd_usage( const nschar *name )
	{
	nssize index = _ns_profiler_lookup_shell_cmd( name );

	if( index < _NS_PROFILER_NUM_SHELL_CMDS )
		ns_println(
			"usage: "
			NS_FMT_STRING
			" "
			NS_FMT_STRING,
			_ns_profiler_shell_cmds[ index ].name,
			_ns_profiler_shell_cmds[ index ].usage
			);
	}


NS_PRIVATE void _ns_profiler_shell_cmd_do_hier( NsProfiler *profiler, nshieriter I, nssize level )
   {
	NsProfile *profile = _ns_profile( I );

	/* NOTE: The '*3' is in effect the tab-size. I just like 3 spaces. */
	ns_println(
		NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
		( nsint )( ns_profile_name_length( profile ) + level * 3 ),
		ns_profile_name( profile )
		);

	if( ns_hier_iter_has_child( I ) )
		_ns_profiler_shell_cmd_do_hier( profiler, ns_hier_iter_child( I ), level + 1 );

	if( ns_hier_iter_has_sibling( I ) )
		_ns_profiler_shell_cmd_do_hier( profiler, ns_hier_iter_sibling( I ), level );
   }


NS_PRIVATE void _ns_profiler_shell_cmd_hier( NsProfiler *profiler, nsint argc, nschar *argv[] )
   {
	nshieriter root;

	if( 1 != argc )
		_ns_profiler_output_shell_cmd_usage( argv[0] );
	else
		{
		root = ns_hier_root( &profiler->profiles );

		if( ns_hier_iter_has_child( root ) )
			_ns_profiler_shell_cmd_do_hier( profiler, ns_hier_iter_child( root ), 0 );
		}
   }


NS_PRIVATE void _ns_profiler_shell_cmd_exit( NsProfiler *profiler, nsint argc, nschar *argv[] )
	{
	if( 1 != argc )
		_ns_profiler_output_shell_cmd_usage( argv[0] );
	else
		profiler->exit_shell = NS_TRUE;
	}


#define _NS_PROFILER_PATH_SEPARATOR  NS_ASCII_SOLIDUS


NS_PRIVATE void _ns_profiler_shell_cmd_cd( NsProfiler *profiler, nsint argc, nschar *argv[] )
	{
	const NsProfile  *profile;
	nschar           *name;
	nschar           *term;
	nshieriter        curr;
	nshieriter        child;
	nschar            temp;
	nssize            length;


	if( 2 != argc )
		{
		_ns_profiler_output_shell_cmd_usage( argv[0] );
		return;
		}

	name = argv[1];
	curr = profiler->shell_profile;

	/* First check for jumping to the root. */
	if( _NS_PROFILER_PATH_SEPARATOR == *name )
		{
		curr = ns_hier_root( &profiler->profiles );
		++name;
		}

	NS_INFINITE_LOOP
		{
		term = ns_ascii_strchr( name, _NS_PROFILER_PATH_SEPARATOR );

		if( NULL == term )
			term = name + ns_ascii_strlen( name );

		ns_assert( name <= term );

		temp  = *term;
		*term = NS_ASCII_NULL;

		length = ns_ascii_strlen( name );

		if( 0 == length )
			{}
		else if( 1 == length && ns_ascii_streq( name, "." ) )
			{}
		else if( 2 == length && ns_ascii_streq( name, ".." ) )
			{
			/* Goto the parent of the current. */
			if( ! ns_hier_iter_has_parent( curr ) )
				{
				ns_println( "No such profile." );
				return;
				}

			curr = ns_hier_iter_parent( curr );
			}
		else
			{
			/* Search the children of the current. */
			if( ! ns_hier_iter_has_child( curr ) )
				{
				ns_println( "No such profile." );
				return;
				}

			child = ns_hier_iter_child( curr );

			NS_INFINITE_LOOP
				{
				profile = _ns_profile( child );

				/* If found the child, then goto it. */
				if( ns_profile_name_length( profile ) == length &&
					 ns_ascii_streq( name, ns_profile_name( profile ) ) )
					{
					curr = child;
					break;
					}

				if( ! ns_hier_iter_has_sibling( child ) )
					{
					ns_println( "No such profile." );
					return;
					}

				child = ns_hier_iter_sibling( child );
				}
			}

		*term = temp;

		if( NS_ASCII_NULL == *term )
			break;

		name = term + 1;
		}

	profiler->shell_profile = curr;
	}


#define _NS_PROFILE_COL_WIDTH     ( ( nsint )8 )
#define _NS_PROFILE_COL_SPACE     "  "
#define _NS_PROFILE_COL_LINE      "--------"
#define _NS_PROFILE_ROOT_NAME     "<ROOT>"

#define _NS_PROFILE_PRECISION     ( ( nsint )1 )


NS_PRIVATE void _ns_profiler_output_ls_parent( NsProfiler *profiler, NsProfile *parent )
	{
	nschar* argv[] = { "pwd" };

	ns_print_newline();

	ns_print( " Directory of " );
	_ns_profiler_shell_cmd_pwd( profiler, 1, argv ); 

	ns_print( " #seconds = " );

	if( NULL != parent )
		ns_println(
			NS_MAKE_FMT( NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE ),
			_NS_PROFILE_PRECISION,
			parent->elapsed_seconds
			);
	else
		ns_println( "N/A" );

	ns_print_newline();
	}


NS_PRIVATE void _ns_profiler_output_ls_profile
	(
	NsProfiler       *profiler,
	const NsProfile  *profile,
	nsint             width,
	const NsProfile  *parent
	)
	{
	ns_print(
		NS_MAKE_FMT( NS_FMT_FLAG_LEFT_JUSTIFY NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
		width,
		ns_profile_name( profile )
		);

	/* Percent of parent profile time. */
	if( NULL != parent )
		ns_print(
			_NS_PROFILE_COL_SPACE
			NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE ),
			_NS_PROFILE_COL_WIDTH,
			_NS_PROFILE_PRECISION,
			profile->elapsed_seconds / parent->elapsed_seconds * 100.0
			);
	else
		ns_print(
			_NS_PROFILE_COL_SPACE
			NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
			_NS_PROFILE_COL_WIDTH,
			"N/A"
			);

	/* Percent of total time. */
	ns_print(
		_NS_PROFILE_COL_SPACE
		NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE ),
		_NS_PROFILE_COL_WIDTH,
		_NS_PROFILE_PRECISION,
		profile->elapsed_seconds / profiler->elapsed_seconds * 100.0
		);

	/* Average milliseconds per call. */
	ns_print(
		_NS_PROFILE_COL_SPACE
		NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE ),
		_NS_PROFILE_COL_WIDTH,
		_NS_PROFILE_PRECISION,
		profile->elapsed_seconds / profile->num_calls * 1000.0
		);

	/* Total number of calls. */
	ns_print(
		_NS_PROFILE_COL_SPACE
		NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_ULONG ),
		_NS_PROFILE_COL_WIDTH,
		profile->num_calls
		);

	/* Total elapsed seconds. */
	ns_println(
		_NS_PROFILE_COL_SPACE
		NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE ),
		_NS_PROFILE_COL_WIDTH,
		_NS_PROFILE_PRECISION,
		profile->elapsed_seconds
		);
	}


NS_PRIVATE void _ns_profiler_output_ls_header( NsProfiler *profiler, nsint width )
	{
	NS_USE_VARIABLE( profiler );

	ns_print(
		NS_MAKE_FMT( NS_FMT_FLAG_LEFT_JUSTIFY NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
		width,
		" "
		);

	ns_println(
		_NS_PROFILE_COL_SPACE " %%parent"
		_NS_PROFILE_COL_SPACE "  %%total"
		_NS_PROFILE_COL_SPACE " ms/call"
		_NS_PROFILE_COL_SPACE "  #calls"
		_NS_PROFILE_COL_SPACE "#seconds"
		);

	ns_print(
		NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
		width,
		" "
		);

	ns_println(
		_NS_PROFILE_COL_SPACE _NS_PROFILE_COL_LINE
		_NS_PROFILE_COL_SPACE _NS_PROFILE_COL_LINE
		_NS_PROFILE_COL_SPACE _NS_PROFILE_COL_LINE
		_NS_PROFILE_COL_SPACE _NS_PROFILE_COL_LINE
		_NS_PROFILE_COL_SPACE _NS_PROFILE_COL_LINE
		);
	}


NS_PRIVATE void _ns_profiler_shell_cmd_ls( NsProfiler *profiler, nsint argc, nschar *argv[] )
   {
	NsProfile     *parent;
	const nschar  *name;
	nshieriter     child;
	nssize         max_length;
	nssize         length;
	nsint          width;


	if( 1 != argc )
		{
		_ns_profiler_output_shell_cmd_usage( argv[0] );
		return;
		}

	if( _ns_profiler_not_root( profiler, profiler->shell_profile ) )
		{
		name   = ns_profile_name( _ns_profile( profiler->shell_profile ) ); 
		parent = _ns_profile( profiler->shell_profile );
		}
	else
		{
		name   = _NS_PROFILE_ROOT_NAME;
		parent = NULL;
		}

   /* First get the maximum string length, just for formatting. */
   max_length = ns_ascii_strlen( name ); 

	if( ns_hier_iter_has_child( profiler->shell_profile ) )
		{
		child  = ns_hier_iter_child( profiler->shell_profile );
		length = ns_profile_name_length( _ns_profile( child ) );

		if( max_length < length )
			max_length = length;

		while( ns_hier_iter_has_sibling( child ) )
			{
			child  = ns_hier_iter_sibling( child );
			length = ns_profile_name_length( _ns_profile( child ) );

			if( max_length < length )
				max_length = length;
			}
		}

	width = ( nsint )max_length;

	_ns_profiler_output_ls_parent( profiler, parent );

	/* Output the child profiles. */
	if( ns_hier_iter_has_child( profiler->shell_profile ) )
		{
		_ns_profiler_output_ls_header( profiler, width );

		child = ns_hier_iter_child( profiler->shell_profile );
		_ns_profiler_output_ls_profile( profiler, _ns_profile( child ), width, parent );

		while( ns_hier_iter_has_sibling( child ) )
			{
			child = ns_hier_iter_sibling( child );
			_ns_profiler_output_ls_profile( profiler, _ns_profile( child ), width, parent );
			}
		}
	}


NS_PRIVATE void _ns_profiler_shell_cmd_do_pwd( NsProfiler *profiler, nshieriter I )
	{
	if( _ns_profiler_not_root( profiler, I ) )
		_ns_profiler_shell_cmd_do_pwd( profiler, ns_hier_iter_parent( I ) );

	if( _ns_profiler_not_root( profiler, I ) )
		ns_print( NS_FMT_STRING, ns_profile_name( _ns_profile( I ) ) );

	ns_print( NS_FMT_UNICHAR, _NS_PROFILER_PATH_SEPARATOR );
	}


NS_PRIVATE void _ns_profiler_shell_cmd_pwd( NsProfiler *profiler, nsint argc, nschar *argv[] )
	{
	if( 1 != argc )
		_ns_profiler_output_shell_cmd_usage( argv[0] );
	else
		{
		_ns_profiler_shell_cmd_do_pwd( profiler, profiler->shell_profile );
		ns_print_newline();
		}
	}


NS_PRIVATE void _ns_profiler_shell_cmd_help( NsProfiler *profiler, nsint argc, nschar *argv[] )
	{
	nssize  index;
	nssize  length;
	nssize  max_length;
	nsint   width;


	NS_USE_VARIABLE( profiler );

	if( 1 == argc )
		{
		/* First get the maximum name length, just for formatting. */
		max_length = 0; 

		for( index = 0; index < _NS_PROFILER_NUM_SHELL_CMDS; ++index )
			{
			length = ns_ascii_strlen( _ns_profiler_shell_cmds[ index ].name );

			if( max_length < length )
				max_length = length;
			}

		width = ( nsint )max_length;

		for( index = 0; index < _NS_PROFILER_NUM_SHELL_CMDS; ++index )
			{
			ns_print(
				NS_MAKE_FMT( NS_FMT_FLAG_LEFT_JUSTIFY NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING ),
				width,
				_ns_profiler_shell_cmds[ index ].name
				);

			ns_println(
				" : " NS_FMT_STRING,
				_ns_profiler_shell_cmds[ index ].help
				);
			}
		}
	else if( 2 == argc )
		{
		index = _ns_profiler_lookup_shell_cmd( argv[1] );

		if( index < _NS_PROFILER_NUM_SHELL_CMDS )
			{
			ns_println(
				NS_FMT_STRING " : " NS_FMT_STRING,
				_ns_profiler_shell_cmds[ index ].name,
				_ns_profiler_shell_cmds[ index ].help
				);

			_ns_profiler_output_shell_cmd_usage( argv[1] );
			}
		else
			ns_println(
				NS_FMT_STRING_QUOTED " is not supported by the help utility.",
				argv[ 1 ]
				);
		}
	else
		_ns_profiler_output_shell_cmd_usage( argv[0] );
	}


NS_PRIVATE void _ns_profiler_shell_cmd_man( NsProfiler *profiler, nsint argc, nschar *argv[] )
	{
	nssize index;

	NS_USE_VARIABLE( profiler );

	if( 2 != argc )
		_ns_profiler_output_shell_cmd_usage( argv[0] );
	else
		{
		index = _ns_profiler_lookup_shell_cmd( argv[1] );

		if( index < _NS_PROFILER_NUM_SHELL_CMDS )
			{
			ns_println( NS_FMT_STRING " : ", _ns_profiler_shell_cmds[ index ].name );
			ns_println( NS_FMT_STRING, _ns_profiler_shell_cmds[ index ].manual );
			}
		else
			ns_println(
				NS_FMT_STRING_QUOTED " is not supported by the manual utility.",
				argv[ 1 ]
				);
		}
	}


NS_PRIVATE nschar* _ns_profiler_parse_arg( nschar *string, nschar **curr )
	{
	nschar *start, *end;

	if( NULL != string )
		*curr = string;

	/* Eliminate leading whitespace */
	*curr = ns_ascii_chomp( *curr );

	/* Nothing left so end tokenization. */
	if( NULL == *curr )
		return NULL;

	/* Mark start and find end of token. */
	start = *curr;
	end   = ns_ascii_strchr( start, NS_ASCII_SPACE );

	/* Advance the current pointer for the next iteration. */
	if( NULL != end )
		{
		*end  = NS_ASCII_NULL;
		*curr = end + 1;
		}
	else
		*curr = start + ns_ascii_strlen( start );

	return start;
	}


#define _NS_PROFILER_MAX_CMD_LENGTH  256
#define _NS_PROFILER_MAX_ARGC         64


NS_PRIVATE nssize _ns_profiler_parse_shell_cmd( nschar *cmd, nsint *argc, nschar *argv[] )
	{
	nschar *arg, *curr;

	*argc = 0;
	arg   = _ns_profiler_parse_arg( cmd, &curr );

	while( *argc < _NS_PROFILER_MAX_ARGC && NULL != arg )
		{
		argv[ *argc ] = arg;
		++(*argc);

		arg = _ns_profiler_parse_arg( NULL, &curr );
		}

	return ( 0 < *argc ) ?
			 _ns_profiler_lookup_shell_cmd( argv[0] ) :
			 _NS_PROFILER_NUM_SHELL_CMDS;
	}


void ns_profiler_shell( NsProfiler *profiler )
	{
#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )

	nsint      argc;
	nssize     index;
	nsboolean  eof;
	NsError    error;
	nschar     cmd[ _NS_PROFILER_MAX_CMD_LENGTH ];
	nschar*    argv[ _NS_PROFILER_MAX_ARGC ];


	ns_assert( NULL != profiler );

	ns_mutex_lock( &profiler->mutex );

	profiler->shell_profile = profiler->curr_profile;
	profiler->exit_shell    = NS_FALSE;

	while( ! profiler->exit_shell )
		{
		ns_print( NS_FMT_STRING, "%profiler>" );

		if( NS_FAILURE( _ns_gets(
								NS_INT_TO_POINTER( _NS_STDIN ),
								cmd,
								_NS_PROFILER_MAX_CMD_LENGTH,
								&eof
								),
								error ) )
			{
			if( ! eof )
				ns_println( NS_FMT_STRING, " I/O error. Terminating shell..." );

			argv[0] = "exit";
			_ns_profiler_shell_cmd_exit( profiler, 1, argv );
			}
		else
			{
			index = _ns_profiler_parse_shell_cmd( cmd, &argc, argv );

			if( index < _NS_PROFILER_NUM_SHELL_CMDS )
				( _ns_profiler_shell_cmds[ index ].func )( profiler, argc, argv );
			else if( 0 < argc )
				ns_println(
					NS_FMT_STRING_QUOTED " is not a valid command.",
					argv[0]
					);

			ns_print_newline();
			}
		}

	ns_mutex_unlock( &profiler->mutex );

#else
	NS_USE_VARIABLE( profiler );
#endif
	}


NsProfiler* ns_profiler( void )
	{
	NS_PRIVATE NsProfiler _ns_profiler;
	return &_ns_profiler;
	}


NS_PRIVATE nsboolean _ns_profiler_did_init = NS_FALSE;


void _ns_profiler_init( void )
	{
#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )
	ns_verify( ! _ns_profiler_did_init );

	ns_profiler_construct( ns_profiler() );

	_ns_profiler_did_init = NS_TRUE;
#endif
	}


void _ns_profiler_finalize( void )
	{
#if defined( NS_DEBUG ) && defined( NS_ENABLE_PROFILING )
	if( _ns_profiler_did_init )
		ns_profiler_destruct( ns_profiler() );

	_ns_profiler_did_init = NS_FALSE;
#endif
	}
