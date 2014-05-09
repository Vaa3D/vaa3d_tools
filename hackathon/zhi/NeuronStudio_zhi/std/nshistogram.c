#include "nshistogram.h"


NS_PRIVATE void _ns_histogram_reset( NsHistogram *histogram )
	{
	ns_assert( NULL != histogram );

	histogram->bands      = NULL;
	histogram->band_sizes = NULL;
	histogram->num_bands  = 0;
	}


NsError ns_histogram_construct
	(
	NsHistogram   *histogram,
	nssize         num_bands,
	const nssize  *band_sizes
	)
	{
	ns_assert( NULL != histogram );

	_ns_histogram_reset( histogram );
	return ns_histogram_resize( histogram, num_bands, band_sizes );
	}


void ns_histogram_destruct( NsHistogram *histogram )
	{
	ns_assert( NULL != histogram );
	ns_histogram_clear( histogram );
	}


void ns_histogram_clear( NsHistogram *histogram )
	{
	nssize band;

	ns_assert( NULL != histogram );

	for( band = 0; band < histogram->num_bands; ++band )
		ns_delete( histogram->bands[ band ] );

	ns_delete( histogram->bands );
	ns_delete( histogram->band_sizes );

	_ns_histogram_reset( histogram );
	}


NsError ns_histogram_resize
	(
	NsHistogram   *histogram,
	nssize         num_bands,
	const nssize  *band_sizes
	)
	{
	nssize band;

	ns_assert( NULL != histogram );
	ns_histogram_clear( histogram );

	if( 0 < num_bands )
		{
		ns_assert( NULL != band_sizes );

		if( NULL == ( histogram->bands = ns_new_array( nslong*, num_bands ) ) ||
			 NULL == ( histogram->band_sizes = ns_new_array0( nssize, num_bands ) ) )
			{
			/* NOTE: Ok to call clear() here since histogram->num_bands is still zero. */
			ns_histogram_clear( histogram );
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}

		histogram->num_bands = num_bands;

		/* Just make sure they're all initialized to NULL. */
		for( band = 0; band < num_bands; ++band )
			histogram->bands[ band ] = NULL;

		/* Now actually allocate the bands. */
		for( band = 0; band < num_bands; ++band )
			{
			if( 0 < band_sizes[ band ] )
				if( NULL == ( histogram->bands[ band ] =
					 ns_new_array0( nslong, band_sizes[ band ] ) ) )
					{
					ns_histogram_clear( histogram );
					return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
					}

			histogram->band_sizes[ band ] = band_sizes[ band ];
			}
		}

	return ns_no_error();
	}


nssize ns_histogram_size( const NsHistogram *histogram )
	{
	nssize num_entries, num_bands, band;

	ns_assert( NULL != histogram );

	num_entries = 0;
	num_bands   = ns_histogram_num_bands( histogram );

	for( band = 0; band < num_bands; ++band )
		num_entries += ns_histogram_band_size( histogram, band );

	return num_entries;
	}


nssize ns_histogram_band_size( const NsHistogram *histogram, nssize band )
	{
	ns_assert( NULL != histogram );
	ns_assert( band < ns_histogram_num_bands( histogram ) );

	return histogram->band_sizes[ band ];
	}


nssize ns_histogram_num_bands( const NsHistogram *histogram )
	{
	ns_assert( NULL != histogram );
	return histogram->num_bands;
	}


void ns_histogram_up( NsHistogram *histogram, nssize band, nssize at )
	{
	ns_assert( NULL != histogram );
	ns_assert( band < ns_histogram_num_bands( histogram ) );
	ns_assert( at < ns_histogram_band_size( histogram, band ) );

	++(histogram->bands[ band ][ at ]);
	}


void ns_histogram_down( NsHistogram *histogram, nssize band, nssize at )
	{
	ns_assert( NULL != histogram );
	ns_assert( band < ns_histogram_num_bands( histogram ) );
	ns_assert( at < ns_histogram_band_size( histogram, band ) );

	--(histogram->bands[ band ][ at ]);
	}


nslong ns_histogram_at( const NsHistogram *histogram, nssize band, nssize at )
	{
	ns_assert( NULL != histogram );
	ns_assert( band < ns_histogram_num_bands( histogram ) );
	ns_assert( at < ns_histogram_band_size( histogram, band ) );

	return histogram->bands[ band ][ at ];
	}


NsError ns_histogram_write( const NsHistogram *histogram, const nschar *file_name )
	{
	NsFile   file;
	nssize   band;
	nssize   num_bands;
	nssize   i;
	nssize   band_size;
	NsError  error;


	ns_assert( NULL != histogram );
	ns_assert( NULL != file_name );

	error = ns_no_error();

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_WRITE ), error ) )
		return error;

	num_bands = ns_histogram_num_bands( histogram );

	for( band = 0; band < num_bands; ++band )
		{
		band_size = ns_histogram_band_size( histogram, band );

		for( i = 0; i < band_size; ++i )
			if( NS_FAILURE( ns_file_print(
									&file,
									NS_MAKE_FMT( "6", NS_FMT_TYPE_ULONG )
									NS_STRING_TAB
									NS_MAKE_FMT( "6", NS_FMT_TYPE_LONG )
									NS_STRING_NEWLINE,
									i,
									ns_histogram_at( histogram, band, i )
									),
									error ) )
				break;
		}

	ns_file_destruct( &file );
	return error;
	}
