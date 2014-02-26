#include "nsspines-classifier.h"


NS_PRIVATE NsError ns_spine_class_new( NsSpineClass **klass, const nschar *name, nsint value )
	{
	nschar *c;

	if( NULL == ( *klass = ns_new( NsSpineClass ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NULL == ( (*klass)->name = ns_ascii_strdup( name ) ) )
		{
		ns_delete( *klass );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	for( c = (*klass)->name; NS_ASCII_NULL != *c; ++c )
		if( NS_ASCII_SPACE == *c )
			*c = NS_ASCII_HYPHEN_MINUS;

	(*klass)->value = value;

	return ns_no_error();
	}


NS_PRIVATE void ns_spine_class_delete( NsSpineClass *klass )
	{
	ns_free( klass->name );
	ns_delete( klass );
	}


typedef struct _NsSpinesClassifierDatum
	{
	fann_type  *input;
	nssize      num_input;
	fann_type  *output;
	nssize      num_output;
	}
	NsSpinesClassifierDatum;


NS_PRIVATE void ns_spines_classifier_datum_delete( NsSpinesClassifierDatum *datum )
	{
	ns_free( datum->input );
	ns_free( datum->output );

	ns_delete( datum );
	}


NS_PRIVATE NsError ns_spines_classifier_datum_new
	(
	NsSpinesClassifierDatum  **datum,
	nssize                     num_input,
	nssize                     num_output
	)
	{
	if( NULL == ( *datum = ns_new( NsSpinesClassifierDatum ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	(*datum)->input  = ns_calloc( num_input, sizeof( fann_type ) );
	(*datum)->output = ns_calloc( num_output, sizeof( fann_type ) );

	if( NULL == (*datum)->input || NULL == (*datum)->output )
		{
		ns_spines_classifier_datum_delete( *datum );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	(*datum)->num_input  = num_input;
	(*datum)->num_output = num_output;

	return ns_no_error();
	}


enum{
	_NS_SPINES_CLASSIFIER_TRAIN_CASCADE,
	_NS_SPINES_CLASSIFIER_TRAIN_BULK
	};


void ns_spines_classifier_construct( NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );

	ns_list_construct( &sc->classes, ns_spine_class_delete );
	ns_list_construct( &sc->data_cache, ns_spines_classifier_datum_delete );

	sc->data = NULL;
	sc->ann  = NULL;
	sc->name = NULL;
	sc->file = NULL;

	sc->training_method = _NS_SPINES_CLASSIFIER_TRAIN_CASCADE;

	sc->lookup = NULL;
	}


void ns_spines_classifier_destruct( NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );

	ns_spines_classifier_clear( sc );

	ns_list_destruct( &sc->classes );
	ns_list_destruct( &sc->data_cache );
	}


void ns_spines_classifier_clear( NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );

	ns_list_clear( &sc->classes );
	ns_list_clear( &sc->data_cache );

	if( NULL != sc->data )
		fann_destroy_train( sc->data );

	if( NULL != sc->ann )
		fann_destroy( sc->ann );

	sc->data = NULL;
	sc->ann  = NULL;

	ns_free( sc->name );
	sc->name = NULL;

	ns_free( sc->file );
	sc->file = NULL;

	sc->training_method = _NS_SPINES_CLASSIFIER_TRAIN_CASCADE;

	ns_spines_classifier_finalize_lookup( sc );
	}


void ns_spines_classifier_clear_data_and_network( NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );

	ns_list_clear( &sc->data_cache );

	if( NULL != sc->data )
		fann_destroy_train( sc->data );

	if( NULL != sc->ann )
		fann_destroy( sc->ann );

	sc->data = NULL;
	sc->ann  = NULL;
	}


const nschar* ns_spines_classifier_get_name( const NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );
	return sc->name;
	}


NS_PRIVATE NsError _ns_spines_classifier_random_name( NsSpinesClassifier *sc )
	{
	nschar name[64];

	ns_snprint( name, sizeof( name ), "Spine Classifier " NS_FMT_UINT, ( nsuint )ns_global_randi() );

	if( NULL == ( sc->name = ns_ascii_strdup( name ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	return ns_no_error();
	}


NsError ns_spines_classifier_set_name( NsSpinesClassifier *sc, const nschar *name )
	{
	ns_assert( NULL != sc );

	ns_free( sc->name );

	/* Make a random name for this classifier if one isnt provided. */
	if( NULL == name )
		return _ns_spines_classifier_random_name( sc );

	/* Remove leading whitespace from the name. */
	while( ns_ascii_isspace( *name ) )
		++name;

	/* If the name is nothing but whitespace, get a random name. */
	if( 0 == ns_ascii_strlen( name ) )
		return _ns_spines_classifier_random_name( sc );

	if( NULL == ( sc->name = ns_ascii_strdup( name ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	return ns_no_error();
	}


const nschar* ns_spines_classifier_get_file( const NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );
	return sc->file;
	}


NsError ns_spines_classifier_set_file( NsSpinesClassifier *sc, const nschar *file )
	{
	ns_assert( NULL != sc );
	ns_assert( NULL != file );

	ns_free( sc->file );

	if( NULL == ( sc->file = ns_ascii_strdup( file ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	return ns_no_error();
	}


nsboolean ns_spines_classifier_has_class_name( const NsSpinesClassifier *sc, const nschar *name )
	{
	nsspineclass klass;

	ns_assert( NULL != sc );
	ns_assert( NULL != name );
	
	NS_LIST_FOREACH( &sc->classes, klass )
		if( ns_ascii_streq( ns_spine_class_name( klass ), name ) )
			return NS_TRUE;

	return NS_FALSE;
	}


nsboolean ns_spines_classifier_has_class_value( const NsSpinesClassifier *sc, nsint value )
	{
	nsspineclass klass;

	ns_assert( NULL != sc );

	NS_LIST_FOREACH( &sc->classes, klass )
		if( ns_spine_class_value( klass ) == value )
			return NS_TRUE;

	return NS_FALSE;
	}


nsboolean ns_spines_classifier_has_class_name_or_value( const NsSpinesClassifier *sc, const nschar *name, nsint value )
	{
	nsspineclass klass;

	ns_assert( NULL != sc );
	ns_assert( NULL != name );
	
	NS_LIST_FOREACH( &sc->classes, klass )
		if( ns_ascii_streq( ns_spine_class_name( klass ), name ) || ns_spine_class_value( klass ) == value )
			return NS_TRUE;

	return NS_FALSE;
	}


NsError ns_spines_classifier_add_class( NsSpinesClassifier *sc, const nschar *name, nsint value )
	{
	NsSpineClass  *klass;
	NsError        error;


	ns_assert( NULL != sc );
	ns_assert( NULL != name );
	ns_assert( ! ns_spines_classifier_has_class_name_or_value( sc, name, value ) );

	if( NS_FAILURE( ns_spine_class_new( &klass, name, value ), error ) )
		return error;

	if( NS_FAILURE( ns_list_push_back( &sc->classes, klass ), error ) )
		{
		ns_spine_class_delete( klass );
		return error;
		}

	return ns_no_error();
	}


nssize ns_spines_classifier_max_class_name_length( const NsSpinesClassifier *sc )
	{
	nsspineclass  curr, end;
	nssize        max_name_length, name_length;


	ns_assert( NULL != sc );

	max_name_length = 0;

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		{
		name_length = ns_ascii_strlen( ns_spine_class_name( curr ) );

		if( max_name_length < name_length )
			max_name_length = name_length;
		}

	return max_name_length;
	}


nsenum ns_spines_classifier_check_parse_classes( const nschar *string )
	{
	NsSpinesClassifier  sc;
	nsenum              ret;
	nsspineclass        klass1, klass2;


	ns_assert( NULL != string );

	ret = NS_SPINES_CLASSIFIER_CHECK_PARSE_OK;

	ns_spines_classifier_construct( &sc );
	/*error*/ns_spines_classifier_parse_classes( &sc, string );

	if( 0 == ns_spines_classifier_num_classes( &sc ) )
		ret = NS_SPINES_CLASSIFIER_CHECK_PARSE_EMPTY;

	NS_LIST_FOREACH( &sc.classes, klass1 )
		NS_LIST_FOREACH( &sc.classes, klass2 )
			if( ns_spine_class_not_equal( klass1, klass2 ) &&
				 ns_ascii_streq( ns_spine_class_name( klass1 ), ns_spine_class_name( klass2 ) ) )
				ret = NS_SPINES_CLASSIFIER_CHECK_PARSE_DUPLICATE;

	ns_spines_classifier_destruct( &sc );
	return ret;
	}


#ifdef NS_DEBUG

void ns_spines_classifier_print( const NsSpinesClassifier *sc )
	{
	nsspineclass                    curr, end;
	nsuint                          num_data, num_input, num_output;
	nsuint                          idx_data, idx_input, idx_output;
	fann_type                      *input, *output;
	nslistiter                      iter;
	const NsSpinesClassifierDatum  *datum;


	ns_assert( NULL != sc );

	ns_print_newline();
	ns_println( "+=======================================+" );
	ns_println( "| " NS_FMT_STRING " (Classes)", sc->name );
	ns_println( "+---------------------------------------+" );

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		ns_println(
			"| " NS_FMT_INT " : " NS_FMT_STRING_DOUBLE_QUOTED,
			ns_spine_class_value( curr ),
			ns_spine_class_name( curr )
			);

	if( NULL != sc->ann )
		{
		/* TODO */
		}

	ns_println( "| " );
	ns_println( "| " NS_FMT_STRING " (Current Data)", sc->name );
	ns_println( "+---------------------------------------+" );

	if( NULL != sc->data )
		{
		num_data   = fann_length_train_data( sc->data );
		num_input  = fann_num_input_train_data( sc->data );
		num_output = fann_num_output_train_data( sc->data );

		input  = NULL;
		output = NULL;

		if( NULL == ( input  = ns_malloc( num_input  * sizeof( fann_type ) ) ) ||
			 NULL == ( output = ns_malloc( num_output * sizeof( fann_type ) ) )   )
			{
			ns_free( input );
			ns_free( output );

			return;
			}

		for( idx_data = 0; idx_data < num_data; ++idx_data )
			{
			ns_verify( 0 == fann_get_train_data( sc->data, idx_data, input, output ) );

			ns_print( "| " );

			for( idx_input = 0; idx_input < num_input; ++idx_input )
				ns_print( NS_MAKE_FMT( ".3", NS_FMT_TYPE_DOUBLE ) " ", input[ idx_input ] );

			ns_print( "-> " );

			for( idx_output = 0; idx_output < num_output; ++idx_output )
				ns_print( NS_MAKE_FMT( ".3", NS_FMT_TYPE_DOUBLE ) " ", output[ idx_output ] );

			ns_print_newline();
			}

		ns_free( input );
		ns_free( output );
		}

	ns_println( "| " );
	ns_println( "| " NS_FMT_STRING " (Cached Data)", sc->name );
	ns_println( "+---------------------------------------+" );

	if( ! ns_list_is_empty( &sc->data_cache ) )
		{
		datum      = ns_list_iter_get_object( ns_list_begin( &sc->data_cache ) );
		num_input  = ( nsuint )datum->num_input;
		num_output = ( nsuint )datum->num_output;

		NS_LIST_FOREACH( &sc->data_cache, iter )
			{
			datum = ns_list_iter_get_object( iter );

			ns_assert( ( nsuint )datum->num_input == num_input );
			ns_assert( ( nsuint )datum->num_output == num_output );

			ns_print( "| " );

			for( idx_input = 0; idx_input < num_input; ++idx_input )
				ns_print( NS_MAKE_FMT( ".3", NS_FMT_TYPE_DOUBLE ) " ", datum->input[ idx_input ] );

			ns_print( "-> " );

			for( idx_output = 0; idx_output < num_output; ++idx_output )
				ns_print( NS_MAKE_FMT( ".3", NS_FMT_TYPE_DOUBLE ) " ", datum->output[ idx_output ] );

			ns_print_newline();
			}
		}

	ns_println( "+=======================================+" );
	}


void ns_spines_classifier_print_parse_classes( const nschar *string )
	{
	NsSpinesClassifier  sc;
	nsspineclass        curr, end;


	ns_assert( NULL != string );

	ns_spines_classifier_construct( &sc );
	/*error*/ns_spines_classifier_parse_classes( &sc, string );

	curr = ns_spines_classifier_begin( &sc );
	end  = ns_spines_classifier_end( &sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		ns_println(
			NS_FMT_INT " : " NS_FMT_STRING_DOUBLE_QUOTED,
			ns_spine_class_value( curr ),
			ns_spine_class_name( curr )
			);

	ns_spines_classifier_destruct( &sc );
	}

#endif/* NS_DEBUG */


NsError ns_spines_classifier_parse_classes( NsSpinesClassifier *sc, const nschar *string )
	{
	nsint          value;
	const nschar  *start;
	const nschar  *end;
	nssize         at;
	nsint          length;
	nsint          last;
	NsSpineClass  *klass;
	NsError        error;
	nschar         temp[ 128 ];


	ns_assert( NULL != sc );
	ns_assert( NULL != string );

	ns_spines_classifier_clear( sc );

	value = 1;
	start = end = string;

	NS_INFINITE_LOOP
		{
		/* Advance until a comma or end of string is found. */
		if( ( NS_ASCII_NULL == *end || NS_ASCII_COMMA == *end ) && start < end )
			{
			/* Skip leading white-space. */
			while( ns_ascii_isspace( *start ) )
				++start;

			/* Copy string into a temporary buffer. */
			for( at = 0; start < end && at < sizeof( temp ) - 1; ++at, ++start )
				temp[ at ] = *start;

			/* Always terminate the string. */
			temp[ at ] = NS_ASCII_NULL;

			/* Remove trailing white space from the temporary string. */

			length = ( nsint )ns_ascii_strlen( temp );

			for( last = length - 1; 0 <= last; --last )
				if( ! ns_ascii_isspace( temp[ last ] ) )
					break;

			ns_assert( 0 <= last + 1 );
			temp[ last + 1 ] = NS_ASCII_NULL;

			if( 0 < ns_ascii_strlen( temp ) && ! ns_ascii_streq( ",", temp ) )
				{
				if( NS_FAILURE( ns_spine_class_new( &klass, temp, value ), error ) )
					return error;

				if( NS_FAILURE( ns_list_push_back( &sc->classes, klass ), error ) )
					{
					ns_spine_class_delete( klass );
					return error;
					}
				}

			/* Advance 'start' to that location that 'end' will increment to, with the
				'++end' statement below, by adding 1 to its current location. */
			start = end + 1;
			++value;
			}

		if( NS_ASCII_NULL == *end )
			break;
		else
			++end;
		}

	return ns_no_error();
	}


nssize ns_spines_classifier_num_classes( const NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );
	return ns_list_size( &sc->classes );
	}


NsError ns_spines_classifier_classes_string( const NsSpinesClassifier *sc, NsString *string )
	{
	nsspineclass  curr, end;
	NsError       error;


	ns_assert( NULL != sc );
	ns_assert( NULL != string );

	if( NS_FAILURE( ns_string_set( string, "" ), error ) )
		return error;

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		{
		if( NS_FAILURE( ns_string_append( string, ns_spine_class_name( curr ) ), error ) )
			return error;

		if( ns_spine_class_not_equal( ns_spine_class_next( curr ), end ) )
			if( NS_FAILURE( ns_string_append( string, ", " ), error ) )
				return error;
		}

	return ns_no_error();
	}


nssize ns_spines_classifier_num_datums( const NsSpinesClassifier *sc )
	{
	nssize num_datums = 0;

	ns_assert( NULL != sc );

	if( NULL != sc->data )
		num_datums += fann_length_train_data( sc->data );

	num_datums += ns_list_size( &sc->data_cache );

	return num_datums;
	}


NS_PRIVATE nsboolean _ns_spines_classifier_less_than_func( const NsSpineClass *l, const NsSpineClass *r )
	{  return l->value < r->value;  }


/* Transfer any data from the 'data_cache' list to the 'data' training set. FANN uses
	an array of training data so it must be reallocated when new data are added. */
NS_PRIVATE NsError ns_spines_classifier_uncache_data( NsSpinesClassifier *sc )
	{
	struct fann_train_data   *temp_data;
	NsSpinesClassifierDatum  *temp_datum;
	NsSpinesClassifierDatum  *datum;
	nsuint                    old_num_data;
	nsuint                    new_num_data;
	nsuint                    num_input;
	nsuint                    num_output;
	nsuint                    index;
	nslistiter                iter;
	NsError                   error;


	ns_assert( NULL != sc );

	if( ns_list_is_empty( &sc->data_cache ) )
		return ns_no_error();

	error      = ns_no_error();
	temp_datum = NULL;
	temp_data  = NULL;

	old_num_data = NULL != sc->data ? fann_length_train_data( sc->data ) : 0;
	new_num_data = old_num_data + ( nsuint )ns_list_size( &sc->data_cache );

	datum      = ns_list_iter_get_object( ns_list_begin( &sc->data_cache ) );
	num_input  = ( nsuint )datum->num_input;
	num_output = ( nsuint )datum->num_output;

	/* Need a temporary datum for getting and setting training data. */
	if( NS_FAILURE( ns_spines_classifier_datum_new( &temp_datum, num_input, num_output ), error ) )
		goto _NS_SPINES_CLASSIFIER_UNBUFFER_DATA_EXIT;

	if( NULL == ( temp_data = fann_create_train( new_num_data, num_input, num_output ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_UNBUFFER_DATA_EXIT;
		}

	index = 0;

	/* Add the training data currently in sc->data first. */
	if( NULL != sc->data )
		{
		ns_assert( fann_num_input_train_data( temp_data ) == ( nsuint )temp_datum->num_input );
		ns_assert( fann_num_output_train_data( temp_data ) == ( nsuint )temp_datum->num_output );
		ns_assert( fann_num_input_train_data( temp_data ) == fann_num_input_train_data( sc->data ) );
		ns_assert( fann_num_output_train_data( temp_data ) == fann_num_output_train_data( sc->data ) );
	
		for( ; index < old_num_data; ++index )
			{
			ns_verify( 0 == fann_get_train_data( sc->data, index, temp_datum->input, temp_datum->output ) );
			ns_verify( 0 == fann_set_train_data( temp_data, index, temp_datum->input, temp_datum->output ) );
			}
		}

	/* Now add the training data in the list. */
	NS_LIST_FOREACH( &sc->data_cache, iter )
		{
		datum = ns_list_iter_get_object( iter );

		ns_assert( fann_num_input_train_data( temp_data ) == ( nsuint )datum->num_input );
		ns_assert( fann_num_output_train_data( temp_data ) == ( nsuint )datum->num_output );

		ns_verify( 0 == fann_set_train_data( temp_data, index, datum->input, datum->output ) );

		++index;
		}

	ns_assert( index == new_num_data );

	/* Clear the list and destroy the old training data. */

	ns_list_clear( &sc->data_cache );

	if( NULL != sc->data )
		fann_destroy_train( sc->data );

	sc->data  = temp_data;
	temp_data = NULL;

	_NS_SPINES_CLASSIFIER_UNBUFFER_DATA_EXIT:

	if( NULL != temp_datum )
		ns_spines_classifier_datum_delete( temp_datum );

	/* NOTE: If no error, then 'temp_data' should have been NULL'ed out
		as to prevent it from being destroyed here. */
	if( NULL != temp_data )
		fann_destroy_train( temp_data );

	return error;
	}


NsError ns_spines_classifier_add_datum
	(
	NsSpinesClassifier  *sc,
	const fann_type     *input,
	nssize               num_input,
	const fann_type     *output,
	nssize               num_output
	)
	{
	NsSpinesClassifierDatum  *datum;
	NsError                   error;


	ns_assert( NULL != sc );
	ns_assert( NULL != input );
	ns_assert( 0 < num_input );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );

	if( NS_FAILURE( ns_spines_classifier_datum_new( &datum, num_input, num_output ), error ) )
		return error;

	if( NS_FAILURE( ns_list_push_back( &sc->data_cache, datum ), error ) )
		{
		ns_spines_classifier_datum_delete( datum );
		return error;
		}

	ns_memcpy( datum->input, input, num_input * sizeof( fann_type ) );
	ns_memcpy( datum->output, output, num_output * sizeof( fann_type ) );

	/* IMPORTANT: Destroy the current neural network, if there is one, since
		we've now added new training data to it. */
	if( NULL != sc->ann )
		{
		fann_destroy( sc->ann );
		sc->ann = NULL;
		}

	return ns_no_error();
	}


NsError ns_spines_classifier_get_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	fann_type           *input,
	nssize               num_input,
	fann_type           *output,
	nssize               num_output
	)
	{
	NsError error;

	ns_assert( NULL != sc );
	ns_assert( at < ns_spines_classifier_num_datums( sc ) );
	ns_assert( NULL != input );
	ns_assert( 0 < num_input );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );

	if( NS_FAILURE( ns_spines_classifier_uncache_data( ( NsSpinesClassifier* )sc ), error ) )
		return error;

	ns_assert( NULL != sc->data );
	ns_assert( at < ( nssize )fann_length_train_data( sc->data ) );
	ns_assert( num_input == ( nssize )fann_num_input_train_data( sc->data ) );
	ns_assert( num_output == ( nssize )fann_num_output_train_data( sc->data ) );

	ns_verify( 0 == fann_get_train_data( sc->data, ( nsuint )at, input, output ) );

	return ns_no_error();
	}


NsError ns_spines_classifier_set_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	const fann_type     *input,
	nssize               num_input,
	const fann_type     *output,
	nssize               num_output
	)
	{
	NsError error;

	ns_assert( NULL != sc );
	ns_assert( at < ns_spines_classifier_num_datums( sc ) );
	ns_assert( NULL != input );
	ns_assert( 0 < num_input );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );

	if( NS_FAILURE( ns_spines_classifier_uncache_data( ( NsSpinesClassifier* )sc ), error ) )
		return error;

	ns_assert( NULL != sc->data );
	ns_assert( at < ( nssize )fann_length_train_data( sc->data ) );
	ns_assert( num_input == ( nssize )fann_num_input_train_data( sc->data ) );
	ns_assert( num_output == ( nssize )fann_num_output_train_data( sc->data ) );

	ns_verify( 0 == fann_set_train_data( sc->data, ( nsuint )at, ( fann_type* )input, ( fann_type* )output ) );

	return ns_no_error();
	}


NsError ns_spines_classifier_type_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	nsspineclass         klass,
	fann_type           *output,
	nssize               num_output
	)
	{
	NsError  error;
	nsint    type;


	ns_assert( NULL != sc );
	ns_assert( at < ns_spines_classifier_num_datums( sc ) );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );
	ns_assert( NULL != klass );

	type = ns_spine_class_value( klass );

	ns_assert( 0 < type );
	ns_assert( type <= ( nsint )num_output );

	if( NS_FAILURE( ns_spines_classifier_uncache_data( ( NsSpinesClassifier* )sc ), error ) )
		return error;

	ns_assert( NULL != sc->data );
	ns_assert( at < ( nssize )fann_length_train_data( sc->data ) );
	ns_assert( num_output == ( nssize )fann_num_output_train_data( sc->data ) );

	ns_memzero( output, num_output * sizeof( fann_type ) );
	output[ type - 1 ] = ( fann_type )1;

	return ns_no_error();
	}


NsError ns_spines_classifier_remove_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at
	)
	{
	struct fann_train_data   *temp_data;
	NsSpinesClassifierDatum  *temp_datum;
	nsuint                    num_data;
	nsuint                    num_input;
	nsuint                    num_output;
	nsuint                    src;
	nsuint                    dest;
	NsError                   error;


	ns_assert( NULL != sc );
	ns_assert( at < ns_spines_classifier_num_datums( sc ) );

	error = ns_no_error();

	if( NS_FAILURE( ns_spines_classifier_uncache_data( ( NsSpinesClassifier* )sc ), error ) )
		return error;

	temp_datum = NULL;
	temp_data  = NULL;

	ns_assert( NULL != sc->data );
	num_data = fann_length_train_data( sc->data );
	ns_assert( 0 < num_data );

	if( 1 == num_data )
		{
		ns_assert( 0 == at );
		goto _NS_SPINES_CLASSIFIER_REMOVE_DATUM_CLEAR;
		}

	num_input  = fann_num_input_train_data( sc->data );
	num_output = fann_num_output_train_data( sc->data );

	/* Need a temporary datum for getting and setting training data. */
	if( NS_FAILURE( ns_spines_classifier_datum_new( &temp_datum, num_input, num_output ), error ) )
		goto _NS_SPINES_CLASSIFIER_REMOVE_DATUM_EXIT;

	/* Note the -1 since we're removing one datum. */
	if( NULL == ( temp_data = fann_create_train( num_data - 1, num_input, num_output ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_REMOVE_DATUM_EXIT;
		}

	src = dest = 0;
	
	for( ; src < num_data; ++src )
		if( src != at )
			{
			ns_verify( 0 == fann_get_train_data( sc->data, src, temp_datum->input, temp_datum->output ) );
			ns_verify( 0 == fann_set_train_data( temp_data, dest, temp_datum->input, temp_datum->output ) );

			++dest;
			}

	ns_assert( src == num_data );
	ns_assert( dest == num_data - 1 );

	_NS_SPINES_CLASSIFIER_REMOVE_DATUM_CLEAR:

	fann_destroy_train( sc->data );

	sc->data  = temp_data;
	temp_data = NULL;

	_NS_SPINES_CLASSIFIER_REMOVE_DATUM_EXIT:

	if( NULL != temp_datum )
		ns_spines_classifier_datum_delete( temp_datum );

	/* NOTE: If no error, then 'temp_data' should have been NULL'ed out
		as to prevent it from being destroyed here. */
	if( NULL != temp_data )
		fann_destroy_train( temp_data );

	/* IMPORTANT: Destroy the current neural network, if there is one, since
		we've deleted training data from it. */
	if( NULL != sc->ann )
		{
		fann_destroy( sc->ann );
		sc->ann = NULL;
		}

	return error;
	}


nsint ns_spines_classifier_type
	(
	const NsSpinesClassifier  *sc,
	const fann_type           *output,
	nssize                     num_output
	)
	{
	nsuint  i, m;
	nsint   type;
	nssize  num_classes;


	ns_assert( NULL != sc );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );

	num_classes = ns_spines_classifier_num_classes( sc );
	ns_assert( num_output == num_classes );

	/* Take the maximum output value as the class. */
	m = 0;
	for( i = 1; i < num_output; ++i )
		if( output[i] > output[m] )
			m = i;

	/* Is the output class "strong" enough? */
	if( output[m] < ( fann_type )0.8 )
		{
		/* TODO? */
		}

	/* NOTE: 0 is not a valid spine type! */
	type = ( nsint )( m + 1 );

	ns_assert( type <= ( nsint )num_classes );

	return type;
	}


int FANN_API ns_spines_classifier_train_callback
	(
	struct fann             *ann,
	struct fann_train_data  *train,
	unsigned int             max_epochs,
	unsigned int             epochs_between_reports,
	float                    desired_error,
	unsigned int             epochs
	)
	{
	NS_USE_VARIABLE( train );
	NS_USE_VARIABLE( max_epochs );
	NS_USE_VARIABLE( epochs_between_reports );
	NS_USE_VARIABLE( desired_error );
	NS_USE_VARIABLE( epochs );

#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_println( "MSE: %f", ( nsfloat )fann_get_MSE( ann ) );
#endif

	//ns_progress_update( ann->user_data, ( 1.0f - MSE ) * 100.0f );

	return ns_progress_cancelled( ann->user_data ) ? -1 : 0;
	}


/* This was copied from fann_print_parameters(). Just changed printf() to ns_print() so that
	the output is visible in a windows GUI application. */
void _ns_fann_print_parameters( struct fann *ann )
	{
#ifdef NS_SPINES_CLASSIFIER_VERBOSE

	struct fann_layer *layer_it;
#ifndef FIXEDFANN
	unsigned int i;
#endif

	ns_print("Input layer                          :%4d neurons, 1 bias\n", ann->num_input);
	for(layer_it = ann->first_layer + 1; layer_it != ann->last_layer - 1; layer_it++)
	{
		if(ann->shortcut_connections)
		{
			ns_print("  Hidden layer                       :%4d neurons, 0 bias\n",
				   layer_it->last_neuron - layer_it->first_neuron);
		}
		else
		{
			ns_print("  Hidden layer                       :%4d neurons, 1 bias\n",
				   layer_it->last_neuron - layer_it->first_neuron - 1);
		}
	}
	ns_print("Output layer                         :%4d neurons\n", ann->num_output);
	ns_print("Total neurons and biases             :%4d\n", fann_get_total_neurons(ann));
	ns_print("Total connections                    :%4d\n", ann->total_connections);
	ns_print("Connection rate                      :%8.3f\n", ann->connection_rate);
	ns_print("Shortcut connections                 :%4d\n", ann->shortcut_connections);
#ifdef FIXEDFANN
	ns_print("Decimal point                        :%4d\n", ann->decimal_point);
	ns_print("Multiplier                           :%4d\n", ann->multiplier);
#else
	ns_print("Training algorithm                   :   %s\n", FANN_TRAIN_NAMES[ann->training_algorithm]);
	ns_print("Training error function              :   %s\n", FANN_ERRORFUNC_NAMES[ann->train_error_function]);
	ns_print("Training stop function               :   %s\n", FANN_STOPFUNC_NAMES[ann->train_stop_function]);
#endif
#ifdef FIXEDFANN
	ns_print("Bit fail limit                       :%4d\n", ann->bit_fail_limit);
#else
	ns_print("Learning rate                        :%8.3f\n", ann->learning_rate);
	ns_print("Learning momentum                    :%8.3f\n", ann->learning_momentum);
	ns_print("Quickprop decay                      :%11.6f\n", ann->quickprop_decay);
	ns_print("Quickprop mu                         :%8.3f\n", ann->quickprop_mu);
	ns_print("RPROP increase factor                :%8.3f\n", ann->rprop_increase_factor);
	ns_print("RPROP decrease factor                :%8.3f\n", ann->rprop_decrease_factor);
	ns_print("RPROP delta min                      :%8.3f\n", ann->rprop_delta_min);
	ns_print("RPROP delta max                      :%8.3f\n", ann->rprop_delta_max);
	ns_print("Cascade output change fraction       :%11.6f\n", ann->cascade_output_change_fraction);
	ns_print("Cascade candidate change fraction    :%11.6f\n", ann->cascade_candidate_change_fraction);
	ns_print("Cascade output stagnation epochs     :%4d\n", ann->cascade_output_stagnation_epochs);
	ns_print("Cascade candidate stagnation epochs  :%4d\n", ann->cascade_candidate_stagnation_epochs);
	ns_print("Cascade max output epochs            :%4d\n", ann->cascade_max_out_epochs);
	ns_print("Cascade max candidate epochs         :%4d\n", ann->cascade_max_cand_epochs);
	ns_print("Cascade weight multiplier            :%8.3f\n", ann->cascade_weight_multiplier);
	ns_print("Cascade candidate limit              :%8.3f\n", ann->cascade_candidate_limit);
	for(i = 0; i < ann->cascade_activation_functions_count; i++)
		ns_print("Cascade activation functions[%d]      :   %s\n", i,
			FANN_ACTIVATIONFUNC_NAMES[ann->cascade_activation_functions[i]]);
	for(i = 0; i < ann->cascade_activation_steepnesses_count; i++)
		ns_print("Cascade activation steepnesses[%d]    :%8.3f\n", i,
			ann->cascade_activation_steepnesses[i]);
		
	ns_print("Cascade candidate groups             :%4d\n", ann->cascade_num_candidate_groups);
	ns_print("Cascade no. of candidates            :%4d\n", fann_get_cascade_num_candidates(ann));
#endif

#else
	NS_USE_VARIABLE( ann );
#endif/* NS_SPINES_CLASSIFIER_VERBOSE */
	}


NS_PRIVATE NsError ns_spines_classifier_train_bulk( NsSpinesClassifier *sc, NsProgress *progress )
	{
	/* If there's already a network, then just return. */
	if( NULL != sc->ann )
		return ns_no_error();

	ns_assert( NULL != sc->data );

	#define _TRAIN_BULK_NUM_LAYERS              3
	#define _TRAIN_BULK_NUM_HIDDEN_NEURONS      14
	#define _TRAIN_BULK_MAX_EPOCHS              200
	#define _TRAIN_BULK_EPOCHS_BETWEEN_REPORTS  10
	#define _TRAIN_BULK_DESIRED_ERROR           0.0001f

	sc->ann =
		fann_create_standard(
			_TRAIN_BULK_NUM_LAYERS,
			fann_num_input_train_data( sc->data ),
			_TRAIN_BULK_NUM_HIDDEN_NEURONS,
			fann_num_output_train_data( sc->data )
			);

	if( NULL == sc->ann )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_println(
		"Bulk Training. Max epochs %8u. Desired error: %.10f.",
		_TRAIN_BULK_MAX_EPOCHS,
		_TRAIN_BULK_DESIRED_ERROR
		);
#endif

	fann_set_callback( sc->ann, ns_spines_classifier_train_callback );

#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_progress_set_title( progress, "Training classifier..." );
#endif

	ns_progress_begin( progress );

	sc->ann->user_data = progress;

	fann_train_on_data(
		sc->ann,
		sc->data,
		_TRAIN_BULK_MAX_EPOCHS,
		_TRAIN_BULK_EPOCHS_BETWEEN_REPORTS,
		_TRAIN_BULK_DESIRED_ERROR
		);

	_ns_fann_print_parameters( sc->ann );

	ns_progress_end( progress );

	return ns_no_error();
	}


NsError ns_spines_classifier_train_cascade( NsSpinesClassifier *sc, NsProgress *progress )
	{
	/* If there's already a network, then just return. */
	if( NULL != sc->ann )
		return ns_no_error();

	ns_assert( NULL != sc->data );

	#define _TRAIN_CASCADE_MAX_NEURONS              200
	#define _TRAIN_CASCADE_NEURONS_BETWEEN_REPORTS  5
	#define _TRAIN_CASCADE_DESIRED_ERROR            0.0001f

	sc->ann =
		fann_create_shortcut(
			2,
			fann_num_input_train_data( sc->data ),
			fann_num_output_train_data( sc->data )
			);

	if( NULL == sc->ann )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_println(
		"Cascade Training. Max neurons %8u. Desired error: %.10f.",
		_TRAIN_CASCADE_MAX_NEURONS,
		_TRAIN_CASCADE_DESIRED_ERROR
		);
#endif

	fann_set_callback( sc->ann, ns_spines_classifier_train_callback );

#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_progress_set_title( progress, "Training classifier..." );
#endif

	ns_progress_begin( progress );

	sc->ann->user_data = progress;

	fann_cascadetrain_on_data(
		sc->ann,
		sc->data,
		_TRAIN_CASCADE_MAX_NEURONS,
		_TRAIN_CASCADE_NEURONS_BETWEEN_REPORTS,
		_TRAIN_CASCADE_DESIRED_ERROR
		);

	_ns_fann_print_parameters( sc->ann );

	ns_progress_end( progress );

	return ns_no_error();
	}


NsError ns_spines_classifier_run
	(
	NsSpinesClassifier  *sc,
	const fann_type     *input,
	nssize               num_input,
	fann_type           *output,
	nssize               num_output,
	nsboolean           *valid,
	NsProgress          *progress
	)
	{
	const fann_type  *result;
	NsError           error;


	ns_assert( NULL != sc );
	ns_assert( NULL != input );
	ns_assert( 0 < num_input );
	ns_assert( NULL != output );
	ns_assert( 0 < num_output );
	ns_assert( NULL != valid );

	*valid = NS_TRUE;

	if( NS_FAILURE( ns_spines_classifier_uncache_data( sc ), error ) )
		return error;

	/* If no training data, then set all zeroes in the 'output'. */
	if( NULL == sc->data )
		{
		*valid = NS_FALSE;
		return ns_no_error();
		}

	ns_assert( NULL != sc->data );
	ns_assert( 0 < fann_length_train_data( sc->data ) );
	ns_assert( num_input == ( nssize )fann_num_input_train_data( sc->data ) );
	ns_assert( num_output == ( nssize )fann_num_output_train_data( sc->data ) );

	switch( sc->training_method )
		{
		case _NS_SPINES_CLASSIFIER_TRAIN_CASCADE:
			if( NS_FAILURE( ns_spines_classifier_train_cascade( sc, progress ), error ) )
				return error;
			break;

		case _NS_SPINES_CLASSIFIER_TRAIN_BULK:
			if( NS_FAILURE( ns_spines_classifier_train_bulk( sc, progress ), error ) )
				return error;
			break;

		default:
			ns_assert_not_reached();
		}

	ns_assert( NULL != sc->ann );

	result = fann_run( sc->ann, ( fann_type* )input );
	ns_memcpy( output, result, num_output * sizeof( fann_type ) );

	return ns_no_error();
	}


NsError ns_spines_classifier_read( NsSpinesClassifier *sc, const nschar *file_name )
	{
	NsConfigDb       db;
	nssize           num_lines;
	nsuint           num_data;
	nsuint           num_input, num_output;
	nsuint           curr_num_input, curr_num_output;
	fann_type       *input, *output;
	nsconfigdbiter   group, curr_group, end_groups, curr_key, end_keys;
	const nschar    *class_name;
	const nschar    *group_name;
	const nschar    *key_name;
	nsint            class_value;
	nsuint           index;
	nsuint8         *data_flags;
	nsuint8         *input_flags;
	nsuint8         *output_flags;
	NsError          error;


	ns_assert( NULL != sc );
	ns_assert( NULL != file_name );

	error = ns_no_error();

	input        = NULL;
	output       = NULL;
	data_flags   = NULL;
	input_flags  = NULL;
	output_flags = NULL;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

   if( NS_FAILURE( ns_config_db_read( &db, file_name, &num_lines ), error ) )
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;

	ns_spines_classifier_clear( sc );

	if( ! ns_config_db_has_group( &db, "name" ) ||
		 ! ns_config_db_has_key( &db, "name", "value" ) )
		{
		error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	if( NS_FAILURE( ns_spines_classifier_set_name( sc, ns_config_db_get_string( &db, "name", "value" ) ), error ) )
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;

	if( ! ns_config_db_has_group( &db, "classes" ) )
		{
		error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	group    = ns_config_db_lookup_group( &db, "classes" );
	curr_key = ns_config_db_begin_keys( group );
	end_keys = ns_config_db_end_keys( group );

	for( ; ns_config_db_iter_not_equal( curr_key, end_keys );
			 curr_key = ns_config_db_iter_next( curr_key ) )
		{
		class_name = ns_config_db_iter_string( curr_key );

		ns_assert( ns_config_db_has_key( &db, "classes", class_name ) );
		class_value = ns_config_db_get_int( &db, "classes", class_name );

		if( ns_spines_classifier_has_class_name_or_value( sc, class_name, class_value ) )
			{
			error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			goto _NS_SPINES_CLASSIFIER_READ_EXIT;
			}

		if( NS_FAILURE( ns_spines_classifier_add_class( sc, class_name, class_value ), error ) )
			goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	ns_list_sort( &sc->classes, _ns_spines_classifier_less_than_func );

	if( ! ns_config_db_has_group( &db, "training" ) ||
		 ! ns_config_db_has_key( &db, "training", "method" ) )
		{
		error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	sc->training_method = ns_config_db_get_int( &db, "training", "method" );

	switch( sc->training_method )
		{
		case _NS_SPINES_CLASSIFIER_TRAIN_CASCADE:
		case _NS_SPINES_CLASSIFIER_TRAIN_BULK:
			break;

		default:
			error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	/* Find out how many datums there are. */

	num_data = 0;

	curr_group = ns_config_db_begin_groups( &db );
	end_groups = ns_config_db_end_groups( &db );

	for( ; ns_config_db_iter_not_equal( curr_group, end_groups );
			 curr_group = ns_config_db_iter_next( curr_group ) )
		{
		group_name = ns_config_db_iter_string( curr_group );

		if( ns_ascii_strstr( group_name, "data" ) == group_name )
			++num_data;
		}

	/* If no training data, then we're done. */
	if( 0 == num_data )
		{
		error = ns_no_error();
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	/* Now check that there arent any missing. */

	if( NULL == ( data_flags = ns_malloc( NS_BITS_TO_BYTES( num_data ) ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	ns_bit_buf_clear_all( data_flags, num_data );

	curr_group = ns_config_db_begin_groups( &db );
	end_groups = ns_config_db_end_groups( &db );

	for( ; ns_config_db_iter_not_equal( curr_group, end_groups );
			 curr_group = ns_config_db_iter_next( curr_group ) )
		{
		group_name = ns_config_db_iter_string( curr_group );

		if( ns_ascii_strstr( group_name, "data" ) == group_name )
			{
			index = ( nsuint )ns_atoi( group_name + ns_ascii_strlen( "data" ) );

			/* Check for out of bounds. */
			if( num_data <= index )
				{
				error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
				goto _NS_SPINES_CLASSIFIER_READ_EXIT;
				}

			ns_bit_buf_set( data_flags, index );
			}	
		}

	/* Check all the bits are set. */
	for( index = 0; index < num_data; ++index )
		if( ns_bit_buf_is_clear( data_flags, index ) )
			{
			error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			goto _NS_SPINES_CLASSIFIER_READ_EXIT;
			}

	ns_free( data_flags );
	data_flags = NULL;

	num_input  = 0;
	num_output = 0;

	curr_group = ns_config_db_begin_groups( &db );
	end_groups = ns_config_db_end_groups( &db );

	for( ; ns_config_db_iter_not_equal( curr_group, end_groups );
			 curr_group = ns_config_db_iter_next( curr_group ) )
		{
		group_name = ns_config_db_iter_string( curr_group );

		if( ns_ascii_strstr( group_name, "data" ) == group_name )
			{
			curr_num_input  = 0;
			curr_num_output = 0;

			curr_key = ns_config_db_begin_keys( curr_group );
			end_keys = ns_config_db_end_keys( curr_group );

			for( ; ns_config_db_iter_not_equal( curr_key, end_keys );
					 curr_key = ns_config_db_iter_next( curr_key ) )
				{
				key_name = ns_config_db_iter_string( curr_key );

				if( ns_ascii_strstr( key_name, "i" ) == key_name )
					++curr_num_input;
				else if( ns_ascii_strstr( key_name, "o" ) == key_name )
					++curr_num_output;
				}

			/* Must have at least 1 input and 1 output. */
			if( 0 == curr_num_input || 0 == curr_num_output )
				{
				error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
				goto _NS_SPINES_CLASSIFIER_READ_EXIT;
				}

			/* If first datum, establish the number of inputs and outputs,
				else check that the numbers are correct. */
			if( 0 == num_input || 0 == num_output )
				{
				num_input  = curr_num_input;
				num_output = curr_num_output;
				}
			else if( curr_num_input != num_input || curr_num_output != num_output )
				{
				error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
				goto _NS_SPINES_CLASSIFIER_READ_EXIT;
				}
			}
		}

	/* If no input or no output, its an error. */
	if( 0 == num_input || 0 == num_output )
		{
		error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	/* Allocate the FANN training data structure. */

	ns_assert( NULL == sc->data );
	ns_assert( 0 < num_input );
	ns_assert( 0 < num_output );

	if( NULL == ( sc->data = fann_create_train( num_data, num_input, num_output ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	/* Allocate input and output buffer, and flags for checking that the number of
		inputs and outputs in the file are correct. */

	if( NULL == ( input        = ns_malloc( num_input  * sizeof( fann_type ) ) ) ||
		 NULL == ( output       = ns_malloc( num_output * sizeof( fann_type ) ) ) ||
		 NULL == ( input_flags  = ns_malloc( NS_BITS_TO_BYTES( num_input ) ) )    ||
		 NULL == ( output_flags = ns_malloc( NS_BITS_TO_BYTES( num_output ) ) )     )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_CLASSIFIER_READ_EXIT;
		}

	curr_group = ns_config_db_begin_groups( &db );
	end_groups = ns_config_db_end_groups( &db );

	for( ; ns_config_db_iter_not_equal( curr_group, end_groups );
			 curr_group = ns_config_db_iter_next( curr_group ) )
		{
		group_name = ns_config_db_iter_string( curr_group );

		if( ns_ascii_strstr( group_name, "data" ) == group_name )
			{
			ns_bit_buf_clear_all( input_flags, num_input );
			ns_bit_buf_clear_all( output_flags, num_output );

			curr_key = ns_config_db_begin_keys( curr_group );
			end_keys = ns_config_db_end_keys( curr_group );

			for( ; ns_config_db_iter_not_equal( curr_key, end_keys );
					 curr_key = ns_config_db_iter_next( curr_key ) )
				{
				key_name = ns_config_db_iter_string( curr_key );

				if( ns_ascii_strstr( key_name, "i" ) == key_name )
					{
					index = ( nsuint )ns_atoi( key_name + ns_ascii_strlen( "i" ) );

					/* Check for out of bounds. */
					if( num_input <= index )
						{
						error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
						goto _NS_SPINES_CLASSIFIER_READ_EXIT;
						}

					ns_bit_buf_set( input_flags, index );
					input[ index ] = ( fann_type )ns_config_db_get_double( &db, group_name, key_name );
					}
				else if( ns_ascii_strstr( key_name, "o" ) == key_name )
					{
					index = ( nsuint )ns_atoi( key_name + ns_ascii_strlen( "o" ) );

					/* Check for out of bounds. */
					if( num_output <= index )
						{
						error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
						goto _NS_SPINES_CLASSIFIER_READ_EXIT;
						}

					ns_bit_buf_set( output_flags, index );
					output[ index ] = ( fann_type )ns_config_db_get_double( &db, group_name, key_name );
					}
				}

			/* Check all the bits are set. */

			for( index = 0; index < num_input; ++index )
				if( ns_bit_buf_is_clear( input_flags, index ) )
					{
					error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
					goto _NS_SPINES_CLASSIFIER_READ_EXIT;
					}

			for( index = 0; index < num_output; ++index )
				if( ns_bit_buf_is_clear( output_flags, index ) )
					{
					error = ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
					goto _NS_SPINES_CLASSIFIER_READ_EXIT;
					}

			index = ( nsuint )ns_atoi( group_name + ns_ascii_strlen( "data" ) );
			ns_assert( index < num_data );

			/* Finally set the training data! */
			ns_verify( 0 == fann_set_train_data( sc->data, index, input, output ) );
			}
		}

	_NS_SPINES_CLASSIFIER_READ_EXIT:

	ns_free( input );
	ns_free( output );
	ns_free( data_flags );
	ns_free( input_flags );
	ns_free( output_flags );

	ns_config_db_destruct( &db );

	return error;
	}


NsError ns_spines_classifier_write( const NsSpinesClassifier *sc, const nschar *file_name )
	{
	NsConfigDb    db;
	nsspineclass  curr, end;
	nsuint        num_data, num_input, num_output;
	nsuint        idx_data, idx_input, idx_output;
	nschar        data_name[64], io_name[64];
	fann_type    *input, *output;
	NsError       error;


	ns_assert( NULL != sc );
	ns_assert( NULL != file_name );

	error = ns_no_error();

	if( NS_FAILURE( ns_spines_classifier_uncache_data( ( NsSpinesClassifier* )sc ), error ) )
		return error;

	input  = NULL;
	output = NULL;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

	if( NS_FAILURE( ns_config_db_add_group( &db, "name" ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	if( NS_FAILURE( ns_config_db_add_string( &db, "name", "value", sc->name ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	if( NS_FAILURE( ns_config_db_add_group( &db, "classes" ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		if( NS_FAILURE(
				ns_config_db_add_int(
					&db,
					"classes",
					ns_spine_class_name( curr ),
					ns_spine_class_value( curr )
					),
				error ) )
			goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	if( NS_FAILURE( ns_config_db_add_group( &db, "training" ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	if( NS_FAILURE( ns_config_db_add_int( &db, "training", "method", sc->training_method ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	if( NULL != sc->data )
		{
		num_data   = fann_length_train_data( sc->data );
		num_input  = fann_num_input_train_data( sc->data );
		num_output = fann_num_output_train_data( sc->data );

		if( NULL == ( input  = ns_malloc( num_input  * sizeof( fann_type ) ) ) ||
			 NULL == ( output = ns_malloc( num_output * sizeof( fann_type ) ) )   )
			{
			error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;
			}

		for( idx_data = 0; idx_data < num_data; ++idx_data )
			{
			ns_verify( 0 == fann_get_train_data( sc->data, idx_data, input, output ) );

			ns_snprint( data_name, sizeof( data_name ), "data" NS_FMT_UINT, idx_data );

			if( NS_FAILURE( ns_config_db_add_group( &db, data_name ), error ) )
				goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

			for( idx_input = 0; idx_input < num_input; ++idx_input )
				{
				ns_snprint( io_name, sizeof( io_name ), "i" NS_FMT_UINT, idx_input );

				if( NS_FAILURE( ns_config_db_add_double( &db, data_name, io_name, input[ idx_input ] ), error ) )
					goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;
				}

			for( idx_output = 0; idx_output < num_output; ++idx_output )
				{
				ns_snprint( io_name, sizeof( io_name ), "o" NS_FMT_UINT, idx_output );

				if( NS_FAILURE( ns_config_db_add_double( &db, data_name, io_name, output[ idx_output ] ), error ) )
					goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;
				}
			}
		}

	if( NS_FAILURE( ns_config_db_write( &db, file_name ), error ) )
		goto _NS_SPINES_CLASSIFIER_WRITE_EXIT;

	_NS_SPINES_CLASSIFIER_WRITE_EXIT:

	ns_free( input );
	ns_free( output );
	ns_config_db_destruct( &db );

	return error;
	}


nsspineclass ns_spines_classifier_begin( const NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );
	return ns_list_begin( &sc->classes );
	}


nsspineclass ns_spines_classifier_end( const NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );
	return ns_list_end( &sc->classes );
	}


NsSpineClass* _ns_spine_class( const nsspineclass klass )
	{  return ns_list_iter_get_object( klass );  }


const nschar* ns_spine_class_name( const nsspineclass klass )
	{  return _ns_spine_class( klass )->name;  }


nsint ns_spine_class_value( const nsspineclass klass )
	{  return _ns_spine_class( klass )->value;  }


nsspineclass ns_spines_classifier_find_by_name( const NsSpinesClassifier *sc, const nschar *name )
	{
	nsspineclass curr, end;

	ns_assert( NULL != sc );
	ns_assert( NULL != name );

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		if( ns_ascii_streq( ns_spine_class_name( curr ), name ) )
			return curr;

	return ns_spines_classifier_end( sc );
	}


nsspineclass ns_spines_classifier_find_by_value( const NsSpinesClassifier *sc, nsint value )
	{
	nsspineclass curr, end;

	ns_assert( NULL != sc );

	curr = ns_spines_classifier_begin( sc );
	end  = ns_spines_classifier_end( sc );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		if( ns_spine_class_value( curr ) == value )
			return curr;

	return ns_spines_classifier_end( sc );
	}


NsError ns_spines_classifier_init_lookup( NsSpinesClassifier *sc )
	{
	nsint          num_classes;
	nslistiter     iter;
	NsSpineClass  *klass;


	ns_assert( NULL != sc );

	ns_spines_classifier_finalize_lookup( sc );

	num_classes = ( nsint )ns_spines_classifier_num_classes( sc );

	if( 0 < num_classes )
		if( NULL == ( sc->lookup = ns_new_array0( NsSpineClass*, num_classes ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	NS_LIST_FOREACH( &sc->classes, iter )
		{
		klass = ns_list_iter_get_object( iter );

		ns_assert( 0 < klass->value );
		ns_assert( klass->value <= num_classes );
		ns_assert( NULL == sc->lookup[ klass->value - 1 ] );

		sc->lookup[ klass->value - 1 ] = klass;
		}

	return ns_no_error();
	}


void ns_spines_classifier_finalize_lookup( NsSpinesClassifier *sc )
	{
	ns_assert( NULL != sc );

	ns_free( sc->lookup );
	sc->lookup = NULL;
	}


const nschar* ns_spines_classifier_lookup( const NsSpinesClassifier *sc, nsint value )
	{
	ns_assert( NULL != sc );
	ns_assert( NULL != sc->lookup );

	if( value <= 0 || ( nsint )ns_spines_classifier_num_classes( sc ) < value )
		return NS_SPINES_CLASSIFIER_NA;
	else if( NULL == sc->lookup[ value - 1 ] )
		return NS_SPINES_CLASSIFIER_NA;
	else
		return sc->lookup[ value - 1 ]->name;
	}


NsError ns_spines_classifier_new_ex( NsSpinesClassifier **sc, const nschar *name )
	{
	NsError error;

	ns_assert( NULL != sc );

	if( NULL == ( *sc = ns_new( NsSpinesClassifier ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	ns_spines_classifier_construct( *sc );

	if( NS_FAILURE( ns_spines_classifier_set_name( *sc, name ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

	return ns_no_error();
	}


NsError ns_spines_classifier_new_by_classes_ex( NsSpinesClassifier **sc, const nschar *name, const nschar *classes )
	{
	NsError error;

	ns_assert( NULL != sc );

	if( NS_FAILURE( ns_spines_classifier_new_ex( sc, "" ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

	if( NS_FAILURE( ns_spines_classifier_parse_classes( *sc, classes ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

	if( NS_FAILURE( ns_spines_classifier_set_name( *sc, name ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

#if defined( NS_DEBUG ) && defined( NS_SPINES_CLASSIFIER_VERBOSE )
	ns_println(
		"new classifier " NS_FMT_STRING_DOUBLE_QUOTED " " NS_FMT_STRING_DOUBLE_QUOTED,
		ns_spines_classifier_get_name( *sc ),
		ns_spines_classifier_get_file( *sc )
		);
#endif

	return ns_no_error();
	}


NsError ns_spines_classifier_new_by_file_ex( NsSpinesClassifier **sc, const nschar *file_name )
	{
	NsError error;

	ns_assert( NULL != sc );

	if( NS_FAILURE( ns_spines_classifier_new_ex( sc, "" ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

	if( NS_FAILURE( ns_spines_classifier_read( *sc, file_name ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

	if( NS_FAILURE( ns_spines_classifier_set_file( *sc, file_name ), error ) )
		{
		ns_spines_classifier_delete_ex( sc );
		return error;
		}

#if defined( NS_DEBUG ) && defined( NS_SPINES_CLASSIFIER_VERBOSE )
	ns_println(
		"new classifier " NS_FMT_STRING_DOUBLE_QUOTED " " NS_FMT_STRING_DOUBLE_QUOTED,
		ns_spines_classifier_get_name( *sc ),
		ns_spines_classifier_get_file( *sc )
		);
#endif

	return ns_no_error();
	}


void ns_spines_classifier_delete_ex( NsSpinesClassifier **sc )
	{
	ns_assert( NULL != sc );

#if defined( NS_DEBUG ) && defined( NS_SPINES_CLASSIFIER_VERBOSE )
	if( NULL != *sc )
		ns_println(
			"delete classifier " NS_FMT_STRING_DOUBLE_QUOTED " " NS_FMT_STRING_DOUBLE_QUOTED,
			ns_spines_classifier_get_name( *sc ),
			ns_spines_classifier_get_file( *sc )
			);
#endif

	if( NULL != *sc )
		{
		if( NULL != ns_spines_classifier_get_file( *sc ) )
			/*error*/ns_spines_classifier_write( *sc, ns_spines_classifier_get_file( *sc ) );

		ns_spines_classifier_destruct( *sc );
		}

	ns_delete( *sc );
	*sc = NULL;
	}




NsSpinesClassifier *____ns_spines_classifier = NULL;


NS_PRIVATE NsError _ns_spines_classifier_new( const nschar *name )
	{  return ns_spines_classifier_new_ex( &____ns_spines_classifier, name );  }


NsError ns_spines_classifier_new_by_classes( const nschar *name, const nschar *classes )
	{  return ns_spines_classifier_new_by_classes_ex( &____ns_spines_classifier, name, classes );  }


NsError ns_spines_classifier_new_by_file( const nschar *file_name )
	{  return ns_spines_classifier_new_by_file_ex( &____ns_spines_classifier, file_name );  }


void ns_spines_classifier_delete( void )
	{  ns_spines_classifier_delete_ex( &____ns_spines_classifier );  }


NsSpinesClassifier* ns_spines_classifier_get( void )
	{  return ____ns_spines_classifier;  }


void ns_spines_classifier_set( NsSpinesClassifier *sc )
	{  ____ns_spines_classifier = sc;  }




typedef struct _NsSpinesPlugin
	{
	nschar     *name;
	nschar     *file;
	nsboolean   is_temp;
	}
	NsSpinesPlugin;


void _ns_spines_plugin_delete( NsSpinesPlugin *sp )
	{
	ns_free( sp->name );
	ns_free( sp->file );
	ns_free( sp );
	}


NsError _ns_spines_plugin_new( NsSpinesPlugin **ret_sp, const nschar *name, const nschar *file )
	{
	NsSpinesPlugin *sp;

	ns_assert( NULL != ret_sp );
	ns_assert( NULL != name );
	ns_assert( NULL != file );

	if( NULL == ( sp = ns_new( NsSpinesPlugin ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	sp->name    = NULL;
	sp->file    = NULL;
	sp->is_temp = NS_FALSE;

	if( NULL == ( sp->name = ns_ascii_strdup( name ) ) )
		{
		_ns_spines_plugin_delete( sp );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	if( NULL == ( sp->file = ns_ascii_strdup( file ) ) )
		{
		_ns_spines_plugin_delete( sp );
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		}

	*ret_sp = sp;
	return ns_no_error();
	}


void ns_spines_plugins_construct( NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );

	ns_list_construct( &scp->entries, _ns_spines_plugin_delete );
	scp->dir = NULL;
	}


void ns_spines_plugins_destruct( NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );

	ns_free( scp->dir );
	ns_list_destruct( &scp->entries );
	}


void ns_spines_plugins_clear( NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );

	ns_free( scp->dir );
	scp->dir = NULL;

	ns_list_clear( &scp->entries );
	}


void ns_spines_plugins_remove_and_delete_temps( NsSpinesPlugins *scp )
	{
	NsSpinesPlugin  *sp;
	nsspinesplugin   curr, next, end;


	curr = ns_spines_plugins_begin( scp );
	end  = ns_spines_plugins_end( scp );

	while( ns_spines_plugin_not_equal( curr, end ) )
		{
		next = ns_spines_plugin_next( curr );
		sp   = ns_list_iter_get_object( curr );

		if( sp->is_temp )
			{
			#ifdef NS_SPINES_CLASSIFIER_VERBOSE
			ns_println( "Removing classifier file " NS_FMT_STRING_DOUBLE_QUOTED, sp->file );
			#endif

			ns_spines_plugins_make_path( scp, curr );
			ns_remove( scp->path ); /* NOTE: Ignoring error! */

			ns_list_erase( &scp->entries, curr );
			}

		curr = next;
		}
	}


nssize ns_spines_plugins_size( const NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );
	return ns_list_size( &scp->entries );
	}


NS_PRIVATE void _ns_spines_plugins_make_name( NsSpinesPlugins *scp, const nschar *name )
	{
	const nschar  *src;
	nschar        *dest;
	nsint          c;
	nssize         length;


	ns_snprint( scp->path, NS_PATH_SIZE, NS_FMT_STRING NS_STRING_PATH, scp->dir );

	src    = name;
	length = ns_ascii_strlen( scp->path );
	dest   = scp->path + length;

	while( *src && length < NS_PATH_SIZE - 1 )
		{
		c = *src++;

		*dest++ =
			( nschar )(
				( ns_ascii_isdigit( c ) || ns_ascii_islower( c ) || ns_ascii_isupper( c ) )
				? c : NS_ASCII_LOW_LINE
				);

		++length;
		}

	src = NS_SPINES_PLUGIN_FILE_EXTENSION;

	while( *src && length < NS_PATH_SIZE - 1 )
		{
		*dest++ = *src++;
		++length;
		}

	*dest = NS_ASCII_NULL;
	}


NsError ns_spines_plugins_check_name( NsSpinesPlugins *scp, const nschar *name, nsboolean *ok )
	{
	NsFile   file;
	NsError  error;


	ns_assert( NULL != scp );
	ns_assert( NULL != name );
	ns_assert( NULL != ok );

	*ok = NS_FALSE;

	_ns_spines_plugins_make_name( scp, name );

	/* Check if the file exists already. */

	ns_file_construct( &file );

	error = ns_file_open( &file, scp->path, NS_FILE_MODE_READ );

	if( NS_ERROR_NOENT == ns_error_code( error ) )
		{
		*ok   = NS_TRUE;
		error = ns_no_error();
		}

	ns_file_destruct( &file );

	return error;
	}


NsError ns_spines_plugins_add
	(
	NsSpinesPlugins     *scp,
	NsSpinesClassifier  *sc,
	nsboolean            is_temp,
	nsspinesplugin      *ret_sp
	)
	{
	NsSpinesPlugin  *sp;
	const nschar    *file;
	NsError          error;


	ns_assert( NULL != scp );
	ns_assert( NULL != sc );

	_ns_spines_plugins_make_name( scp, ns_spines_classifier_get_name( sc ) );

	/* Shouldnt return NULL, but lets be paranoid. */
	if( NULL != ( file = ns_ascii_strrchr( scp->path, NS_UNICHAR_PATH ) ) )
		++file;
	else
		file = scp->path;

	if( NS_FAILURE( _ns_spines_plugin_new( &sp, ns_spines_classifier_get_name( sc ), file ), error ) )
		return error;

	if( NS_FAILURE( ns_list_push_back( &scp->entries, sp ), error ) )
		{
		_ns_spines_plugin_delete( sp );
		return error;
		}

	sp->is_temp = is_temp;

	if( NULL != ret_sp )
		*ret_sp = ns_list_rev_begin( &scp->entries );

	return ns_spines_classifier_write( sc, scp->path );
	}


void ns_spines_plugins_remove_and_delete( NsSpinesPlugins *scp, nsspinesplugin sp )
	{
	ns_assert( NULL != scp );

	#ifdef NS_SPINES_CLASSIFIER_VERBOSE
	ns_println(
		"Removing classifier file " NS_FMT_STRING_DOUBLE_QUOTED,
		( ( NsSpinesPlugin* )ns_list_iter_get_object( sp ) )->file
		);
	#endif

	ns_spines_plugins_make_path( scp, sp );
	ns_remove( scp->path ); /* NOTE: Ignoring error! */

	ns_list_erase( &scp->entries, sp );
	}


NsError ns_spines_plugins_read( NsSpinesPlugins *scp, const nschar *dir )
	{
	NsDir            D;
	NsConfigDb       db;
	NsError          error;
	const nschar    *ext;
	const nschar    *file;
	const nschar    *name;
	NsSpinesPlugin  *sp;
	nschar           cwd[ NS_PATH_SIZE ] = { NS_ASCII_NULL };


	ns_assert( NULL != scp );
	ns_assert( NULL != dir );

	error = ns_no_error();

	if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

	ns_dir_construct( &D );
	ns_spines_plugins_clear( scp );

	if( NULL == ( scp->dir = ns_ascii_strdup( dir ) ) )
		{
		error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
		goto _NS_SPINES_PLUGINS_READ_EXIT;
		}

	if( NS_FAILURE( ns_getcwd( cwd, sizeof( cwd ) ), error ) )
		goto _NS_SPINES_PLUGINS_READ_EXIT;

	if( NS_FAILURE( ns_chdir( dir ), error ) )
		goto _NS_SPINES_PLUGINS_READ_EXIT;

	if( NS_FAILURE( ns_dir_open( &D ), error ) )
		goto _NS_SPINES_PLUGINS_READ_EXIT;

	while( NULL != ( file = ns_dir_read( &D ) ) )
		if( NULL != ( ext = ns_ascii_strrchr( file, NS_ASCII_FULL_STOP ) ) )
			if( ns_ascii_streq( ext, ".nssc" ) )
				{
				ns_config_db_clear( &db );

				if( NS_FAILURE( ns_config_db_read_group( &db, file, "name", NULL, NULL ), error ) )
					goto _NS_SPINES_PLUGINS_READ_EXIT;

				if( ns_config_db_has_group( &db, "name" ) )
					if( ns_config_db_has_key( &db, "name", "value" ) )
						{
						name = ns_config_db_get_string( &db, "name", "value" );

						if( NS_FAILURE( _ns_spines_plugin_new( &sp, name, file ), error ) )
							goto _NS_SPINES_PLUGINS_READ_EXIT;

						if( NS_FAILURE( ns_list_push_back( &scp->entries, sp ), error ) )
							{
							_ns_spines_plugin_delete( sp );
							goto _NS_SPINES_PLUGINS_READ_EXIT;
							}
						}
				}

	_NS_SPINES_PLUGINS_READ_EXIT:

	ns_config_db_destruct( &db );

	ns_dir_destruct( &D );

	if( ns_is_error( error ) )
		ns_spines_plugins_clear( scp );

	ns_chdir( cwd );/* NOTE: Ignore error. */

	return error;
	}


nsboolean ns_spines_plugins_exists( const NsSpinesPlugins *scp, const nschar *name )
	{
	const NsSpinesPlugin  *sp;
	nslistiter             iter;


	ns_assert( NULL != scp );
	ns_assert( NULL != name );

	NS_LIST_FOREACH( &scp->entries, iter )
		{
		sp = ns_list_iter_get_object( iter );

		if( ns_ascii_streq( sp->name, name ) )
			return NS_TRUE;
		}

	return NS_FALSE;
	}


nsspinesplugin ns_spines_plugins_find( const NsSpinesPlugins *scp, const nschar *name )
	{
	const NsSpinesPlugin  *sp;
	nslistiter             iter;


	ns_assert( NULL != scp );
	ns_assert( NULL != name );

	NS_LIST_FOREACH( &scp->entries, iter )
		{
		sp = ns_list_iter_get_object( iter );

		if( ns_ascii_streq( sp->name, name ) )
			return iter;
		}

	return ns_spines_plugins_end( scp );
	}


const nschar* ns_spines_plugins_file( const NsSpinesPlugins *scp, const nschar *name )
	{
	const NsSpinesPlugin  *sp;
	nslistiter             iter;


	ns_assert( NULL != scp );
	ns_assert( NULL != name );

	NS_LIST_FOREACH( &scp->entries, iter )
		{
		sp = ns_list_iter_get_object( iter );

		if( ns_ascii_streq( sp->name, name ) )
			return sp->file;
		}

	return NULL;
	}


const nschar* ns_spines_plugins_make_path( NsSpinesPlugins *scp, const nsspinesplugin sp )
	{
	ns_assert( NULL != scp );

	ns_snprint( scp->path, NS_PATH_SIZE, NS_FMT_STRING NS_STRING_PATH NS_FMT_STRING, scp->dir, ns_spines_plugin_file( sp ) );
	return scp->path;
	}


nsspinesplugin ns_spines_plugins_at( const NsSpinesPlugins *scp, nssize at )
	{
	nsspinesplugin sp;

	ns_assert( NULL != scp );

	if( ns_spines_plugins_size( scp ) <= at )
		return ns_spines_plugins_end( scp );

	sp = ns_spines_plugins_begin( scp );

	for( ; 0 < at; --at )
		sp = ns_spines_plugin_next( sp );

	return sp;
	}


nsspinesplugin ns_spines_plugins_begin( const NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );
	return ns_list_begin( &scp->entries );
	}


nsspinesplugin ns_spines_plugins_end( const NsSpinesPlugins *scp )
	{
	ns_assert( NULL != scp );
	return ns_list_end( &scp->entries );
	}


NS_PRIVATE const NsSpinesPlugin* _ns_spines_plugin( const nsspinesplugin sp )
	{  return ns_list_iter_get_object( sp );  }


const nschar* ns_spines_plugin_name( const nsspinesplugin sp )
	{  return _ns_spines_plugin( sp )->name;  }


const nschar* ns_spines_plugin_file( const nsspinesplugin sp )
	{  return _ns_spines_plugin( sp )->file;  }


#ifdef NS_DEBUG

NS_IMPEXP void ns_spines_plugins_print( const NsSpinesPlugins *scp )
	{
	const NsSpinesPlugin  *sp;
	nslistiter             iter;


	ns_assert( NULL != scp );

	NS_LIST_FOREACH( &scp->entries, iter )
		{
		sp = ns_list_iter_get_object( iter );
		ns_println( NS_FMT_STRING ":" NS_FMT_STRING, sp->name, sp->file );
		}
	}

#endif/* NS_DEBUG */
