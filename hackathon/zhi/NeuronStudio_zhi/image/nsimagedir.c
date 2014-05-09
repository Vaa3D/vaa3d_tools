#include "nsimagedir.h"


typedef struct _NsImageDirEntry
	{
	nschar  *file;
	nssize   number;
	nssize   prefix_length;
	}
	NsImageDirEntry;


NS_PRIVATE NsError ns_image_dir_entry_new
	(
	NsImageDirEntry  **ret_entry,
	const nschar      *file,
	nssize             number,
	nssize             prefix_length
	)
	{
	NsImageDirEntry *entry;

	if( NULL == ( entry = ns_new( NsImageDirEntry ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL == ( entry->file = ns_ascii_strdup( file ) ) )
		{
		ns_delete( entry );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	entry->number        = number;
	entry->prefix_length = prefix_length;

	*ret_entry = entry;
	return ns_no_error();
	}


NS_PRIVATE void ns_image_dir_entry_delete( NsImageDirEntry *entry )
	{
	ns_free( entry->file );
	ns_delete( entry );
	}
	

void ns_image_dir_construct( NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	ns_list_construct( &dir->entries, ns_image_dir_entry_delete );
	}


void ns_image_dir_destruct( NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	ns_list_destruct( &dir->entries );
	}

/*
#ifdef NS_DEBUG
NS_PRIVATE nsboolean _ns_image_dir_no_match_return
	(
	const nschar *template_file,
	const nschar *tp,
	const nschar *curr_file,
	const nschar *cp
	)
	{
	nssize         i, td, tl, cd;
	const nschar  *ti, *ci;


	ns_println( "%s != %s", template_file, curr_file );

	if( tp >= template_file )
		{
		td = ( nssize )( tp - template_file );
		ti = "^";
		}
	else
		{
		td = 0;
		ti = "<";
		}

	tl = ns_ascii_strlen( template_file );

	for( i = 0; i < tl; ++i )
		ns_print( i == td ? ti : " " );

	ns_print( "    " );

	if( cp >= curr_file )
		{
		cd = ( nssize )( cp - curr_file );
		ci = "^";
		}
	else
		{
		cd = 0;
		ci = "<";
		}

	for( i = 0; i < cd; ++i )
		ns_print( " " );

	ns_println( ci );

	return NS_FALSE;
	}
#else*/
	#define _ns_image_dir_no_match_return( template_file, tp, curr_file, cp )\
		NS_FALSE
//#endif


enum{
	_NS_IMAGE_DIR_MATCH_EXTENSION,
	_NS_IMAGE_DIR_MATCH_SUFFIX,
	_NS_IMAGE_DIR_MATCH_NUMBER,
	_NS_IMAGE_DIR_MATCH_PREFIX
	};

NS_PRIVATE nsboolean _ns_image_dir_match_file
	(
	const nschar  *template_file,
	const nschar  *curr_file,
	nsboolean      case_sensitive,
	nssize        *number,
	nssize        *prefix_length
	)
	{
	const nschar  *tp, *cp;
	nschar         t, c;
	nsenum         state;


	*number        = 0;
	*prefix_length = 0;

	state = _NS_IMAGE_DIR_MATCH_EXTENSION;

	tp = template_file + ns_ascii_strlen( template_file ) - 1;
	cp = curr_file + ns_ascii_strlen( curr_file ) - 1;

	while( template_file <= tp && curr_file <= cp )
		{
		t = *tp;
		c = *cp;

		if( ! case_sensitive )
			{
			t = ns_ascii_tolower( t );
			c = ns_ascii_tolower( c );
			}

		switch( state )
			{
			case _NS_IMAGE_DIR_MATCH_EXTENSION:
				/* If in extension, all characters must match. If we see a '.' character
					then we move to the suffix state. */
				if( c != t )
					return _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );

				if( NS_ASCII_FULL_STOP == c )
					state = _NS_IMAGE_DIR_MATCH_SUFFIX;
				break;

			case _NS_IMAGE_DIR_MATCH_SUFFIX:
				/* Keep matching characters in the suffix until we find digits
					that dont match. That indicates that we're in the number part. */ 
				if( c != t )
					{
					if( ns_ascii_isdigit( c ) || ns_ascii_isdigit( t ) )
						state = _NS_IMAGE_DIR_MATCH_NUMBER;
					else
						return _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );
					}
				break;

			case _NS_IMAGE_DIR_MATCH_NUMBER:
				ns_assert_not_reached();
				break;

			case _NS_IMAGE_DIR_MATCH_PREFIX:
				/* All characters of the prefix must match! */
				if( c != t )
					return _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );
				break;

			default:
				ns_assert_not_reached();
			}

		if( _NS_IMAGE_DIR_MATCH_NUMBER == state )
			{
			/* Traverse to just past the beginning of the number parts. */
			while( template_file <= tp && ns_ascii_isdigit( *tp ) )--tp;
			while( curr_file <= cp && ns_ascii_isdigit( *cp ) )--cp;

			/* If the template has a number but the current doesnt, or vica-versa, then
				its not a match. */

			if( ns_ascii_isdigit( *( tp + 1 ) ) && ! ns_ascii_isdigit( *( cp + 1 ) ) )
				return _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );

			if( ! ns_ascii_isdigit( *( tp + 1 ) ) && ns_ascii_isdigit( *( cp + 1 ) ) )
				return _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );

			/* Extract the number part and prefix length for the current file name. */
			if( ns_ascii_isdigit( *( cp + 1 ) ) )
				{
				*number        = ( nssize )ns_atoi( cp + 1 );
				*prefix_length = ( nssize )( ( cp + 1 ) - curr_file );
				}

			state = _NS_IMAGE_DIR_MATCH_PREFIX;
			}
		else
			{
			--tp;
			--cp;
			}
		}

#ifdef NS_DEBUG
	/* If exhausted all characters in both file names then return true. */
	return ( tp == template_file - 1 ) && ( cp == curr_file - 1 ) ?
				NS_TRUE : _ns_image_dir_no_match_return( template_file, tp, curr_file, cp );
#else
	return ( tp == template_file - 1 ) && ( cp == curr_file - 1 );
#endif
	}


#define _NS_IMAGE_DIR_NUM_PREFIX_LENGTHS  512

NS_PRIVATE nsboolean _ns_image_dir_match_template( NsImageDir *dir, const nschar *template_file, nsboolean *any_problems )
	{
	nsimagediriter    curr, next, end;
	NsImageDirEntry  *entry;
	nssize            prefix_length;
	nssize            num_maxima;
	nssize            i;
	nsushort          counts[ _NS_IMAGE_DIR_NUM_PREFIX_LENGTHS ];


	ns_memzero( counts, sizeof( counts ) );

	/* Find the most common of the prefix lengths. */

	curr = ns_image_dir_begin( dir );
	end  = ns_image_dir_end( dir );

	for( ; ns_image_dir_iter_not_equal( curr, end ); curr = ns_image_dir_iter_next( curr ) )
		{
		entry = ns_list_iter_get_object( curr );

		/* Check for array index in bounds and dont allow overflow of the count.
			These are unlikely cases but could possibly happen! Also dont let the
			template file contribute to the counts. */
		if( ! ns_ascii_streq( entry->file, template_file ) )
			if( entry->prefix_length < _NS_IMAGE_DIR_NUM_PREFIX_LENGTHS )
				if( counts[ entry->prefix_length ] < NS_USHORT_MAX )
					++( counts[ entry->prefix_length ] );
		}

	prefix_length = 0;

	for( i = 0; i < _NS_IMAGE_DIR_NUM_PREFIX_LENGTHS; ++i )
		if( counts[ prefix_length ] < counts[i] )
			prefix_length = i;

#ifdef NS_DEBUG
	ns_println( "The most common prefix length was " NS_FMT_ULONG ".", prefix_length );
#endif

	dir->prefix_length = prefix_length;

	/* Check if more than one count at the maximum. */

	num_maxima = 0;

	if( 0 < counts[ prefix_length ] )
		for( i = 0; i < _NS_IMAGE_DIR_NUM_PREFIX_LENGTHS; ++i )
			if( counts[ prefix_length ] == counts[i] )
				++num_maxima;

	if( 1 < num_maxima )
		{
	#ifdef NS_DEBUG
		ns_println( "Found more than one prefix length with the maximum count." );
	#endif

		*any_problems = NS_TRUE;
		}

	/* Remove any entries that dont have the most common prefix length. */

	curr = ns_image_dir_begin( dir );
	end  = ns_image_dir_end( dir );

	while( ns_image_dir_iter_not_equal( curr, end ) )
		{
		next  = ns_image_dir_iter_next( curr );
		entry = ns_list_iter_get_object( curr );

		/* Never remove the template file from the list. */
		if( entry->prefix_length != prefix_length &&
			 ! ns_ascii_streq( entry->file, template_file ) )
			ns_list_erase( &dir->entries, curr );

		curr = next;
		}

	/* Now find the prefix length value for all entries except the one
		that is the template file. */

	prefix_length = 0;

	curr = ns_image_dir_begin( dir );
	end  = ns_image_dir_end( dir );

	for( ; ns_image_dir_iter_not_equal( curr, end ); curr = ns_image_dir_iter_next( curr ) )
		{
		entry = ns_list_iter_get_object( curr );

		/* The prefix length for all the entries should be the same. */
		if( 0 != entry->prefix_length )
			{
			if( 0 == prefix_length )
				prefix_length = entry->prefix_length;
			else if( prefix_length != entry->prefix_length ) /* Shouldn't happen! */
				return NS_FALSE;
			}
		}

	/* Now find the template file entry and set its number. */

	curr = ns_image_dir_begin( dir );
	end  = ns_image_dir_end( dir );

	for( ; ns_image_dir_iter_not_equal( curr, end ); curr = ns_image_dir_iter_next( curr ) )
		{
		entry = ns_list_iter_get_object( curr );

		if( ns_ascii_streq( entry->file, template_file ) )
			{
			ns_assert( 0 == entry->number );
			ns_assert( 0 == entry->prefix_length );
			ns_assert( prefix_length <= ns_ascii_strlen( entry->file ) );

			/* Just to make it correct. */
			entry->prefix_length = prefix_length;

			if( ns_ascii_isdigit( *( entry->file + prefix_length ) ) )
				entry->number = ( nssize )ns_atoi( entry->file + prefix_length );

			return NS_TRUE;
			}
		}

	/* The template file entry should be in the list somewhere! */
	return NS_FALSE;
	}


NS_PRIVATE NsError _ns_image_dir_add_entry
	(
	NsImageDir    *dir,
	const nschar  *file,
	nssize         number,
	nssize         prefix_length
	)
	{
	NsImageDirEntry  *entry;
	NsError           error;


	if( NS_FAILURE( ns_image_dir_entry_new( &entry, file, number, prefix_length ), error ) )
		return error;

	if( NS_FAILURE( ns_list_push_back( &dir->entries, entry ), error ) )
		{
		ns_image_dir_entry_delete( entry );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE nsboolean _ns_image_dir_xfix_equal
	(
	const nschar  *xfix1,
	nssize         length1,
	const nschar  *xfix2,
	nssize         length2,
	nsboolean      ( *neq_func )( const nschar*, const nschar*, nssize )
	)
	{
	if( length1 != length2 )
		return NS_FALSE;

	return ( neq_func )( xfix1, xfix2, length1 );
	}


NS_PRIVATE nsboolean _ns_image_dir_entry_less
	(
	const NsImageDirEntry  *entry1,
	const NsImageDirEntry  *entry2
	)
	{  return entry1->number < entry2->number;  }


NS_PRIVATE void _ns_image_dir_sort( NsImageDir *dir )
	{  ns_list_sort( &dir->entries, _ns_image_dir_entry_less );  }


NS_PRIVATE void _ns_image_dir_print( const NsImageDir *dir )
	{
	nsimagediriter          curr, end;
	const NsImageDirEntry  *entry;


	curr = ns_image_dir_begin( dir );
	end  = ns_image_dir_end( dir );

	for( ; ns_image_dir_iter_not_equal( curr, end ); curr = ns_image_dir_iter_next( curr ) )
		{
		entry = ns_list_iter_get_object( curr );
		ns_println( NS_FMT_STRING ":" NS_FMT_ULONG ":" NS_FMT_ULONG, entry->file, entry->number, entry->prefix_length );
		}
	}


NsError ns_image_dir_create
	(
	NsImageDir    *dir,
	const nschar  *template_file,
	nsboolean      case_sensitive,
	nsboolean      detect_sequence,
	nsboolean     *any_problems
	) 
	{
	NsDir          D;
	const nschar  *curr_file;
	nssize         prefix_length;
	nssize         number;
	NsError        error;


	ns_assert( NULL != dir );
	ns_assert( NULL != template_file );
	ns_assert( NULL != any_problems );

	error = ns_no_error();

	*any_problems = NS_FALSE;

	ns_image_dir_clear( dir );

	if( ! detect_sequence )
		return _ns_image_dir_add_entry( dir, template_file, 0, 0 );

	ns_dir_construct( &D );

	if( NS_FAILURE( ns_dir_open( &D ), error ) )
		{
		ns_dir_destruct( &D );
		return error;
		}

	while( NULL != ( curr_file = ns_dir_read( &D ) ) )
		if( _ns_image_dir_match_file( template_file, curr_file, case_sensitive, &number, &prefix_length ) )
			if( NS_FAILURE( _ns_image_dir_add_entry( dir, curr_file, number, prefix_length ), error ) )
				break;

	ns_dir_close( &D );
	ns_dir_destruct( &D );

	if( ! ns_is_error( error ) )
		{
		if( ! _ns_image_dir_match_template( dir, template_file, any_problems ) )
			{
			ns_println(
				"image/nsimagedir.c::_ns_image_dir_match_template()"
				" Failed to match template file entry. A parsing error must have occurred."
				);

			ns_log_entry(
				NS_LOG_ENTRY_ERROR,
				"image/nsimagedir.c::_ns_image_dir_match_template()"
				" Failed to match template file entry. A parsing error must have occurred."
				);
			}

		_ns_image_dir_sort( dir );

		#ifdef NS_DEBUG
		//_ns_image_dir_print( dir );
		#endif
		}

	return error;
	}


nssize ns_image_dir_size( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	return ns_list_size( &dir->entries );
	}


nssize ns_image_dir_prefix_length( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	return dir->prefix_length;
	}


void ns_image_dir_clear( NsImageDir *dir )
	{
	ns_assert( NULL != dir );

	ns_list_clear( &dir->entries );
	dir->prefix_length = 0;
	}


const nschar* ns_image_dir_first_file( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	ns_assert( ! ns_image_dir_is_empty( dir ) );

	return ns_image_dir_iter_file( ns_image_dir_begin( dir ) );
	}


const nschar* ns_image_dir_last_file( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	ns_assert( ! ns_image_dir_is_empty( dir ) );

	return ns_image_dir_iter_file( ns_list_rev_begin( &dir->entries ) );
	}


nsimagediriter ns_image_dir_begin( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	return ns_list_begin( &dir->entries );
	}


nsimagediriter ns_image_dir_end( const NsImageDir *dir )
	{
	ns_assert( NULL != dir );
	return ns_list_end( &dir->entries );
	}


const nschar* ns_image_dir_iter_file( const nsimagediriter I )
	{  return ( ( const NsImageDirEntry* )ns_list_iter_get_object( I ) )->file;  }
