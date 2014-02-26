#include "nsmodel-io-swc.h"


#define _NS_SWC_VERTEX_UNDEFINED        0
#define _NS_SWC_VERTEX_SOMA             1
#define _NS_SWC_VERTEX_AXON             2
#define _NS_SWC_VERTEX_DENDRITE         3
#define _NS_SWC_VERTEX_APICAL_DENDRITE  4
#define _NS_SWC_VERTEX_FORK_POINT       5
#define _NS_SWC_VERTEX_END_POINT        6
#define _NS_SWC_VERTEX_CUSTOM           7


NS_PRIVATE NsError _ns_swc_write_vertex
	(
	NsFile         *file,
	nsmodelvertex   vertex,
	nsmodeledge     parent_edge,
	nslong          parent_label,
	nslong          label,
	nsulong         flags
	)
	{
	NsVector3f  position;
	nslong      type;
	nsulong     section;
	nsulong     order;
	NsError     error;


	type = _NS_SWC_VERTEX_UNDEFINED;

	/* The functional type of the vertex has "higher priority" than
		the other possible labels. */
	switch( ns_model_vertex_get_function_type( vertex ) )
		{
		case NS_MODEL_FUNCTION_UNKNOWN:
			if( ns_model_vertex_is_origin( vertex ) )
				type = _NS_SWC_VERTEX_SOMA;
			else if( ns_model_vertex_is_junction( vertex ) )
				type = _NS_SWC_VERTEX_FORK_POINT;
			else if( ns_model_vertex_is_external( vertex ) )
				type = _NS_SWC_VERTEX_END_POINT;
			else
				type = _NS_SWC_VERTEX_DENDRITE;
			break;

		case NS_MODEL_FUNCTION_SOMA:
			type = _NS_SWC_VERTEX_SOMA;
			break;

		case NS_MODEL_FUNCTION_BASAL_DENDRITE:
			type = _NS_SWC_VERTEX_DENDRITE;
			break;

		case NS_MODEL_FUNCTION_APICAL_DENDRITE:
			type = _NS_SWC_VERTEX_APICAL_DENDRITE;
			break;

		case NS_MODEL_FUNCTION_AXON:
			type = _NS_SWC_VERTEX_AXON;
			break;

		default:
			ns_assert_not_reached();
		}

	ns_model_vertex_get_position( vertex, &position );

	if( NS_FAILURE(
			ns_file_print(
				file,
				NS_FMT_LONG
				" "
				NS_FMT_LONG
				" "
				NS_FMT_DOUBLE
				" "
				NS_FMT_DOUBLE
				" "
				NS_FMT_DOUBLE
				" "
				NS_FMT_DOUBLE
				" "
				NS_FMT_LONG,
				label,
				type,
				position.x,
				position.y,
				position.z,
				ns_model_vertex_get_radius( vertex ), /* .swc stores radii! */
				parent_label
				),
			error ) )
		return error;

	if( ( nsboolean )( flags & NS_SWC_OUTPUT_EDGES ) )
		{
		if( ns_model_edge_not_equal( parent_edge, NS_MODEL_EDGE_NIL ) )
			{
			section = ns_model_edge_get_section( parent_edge );
			order   = ns_model_edge_get_order( parent_edge );
			}
		else
			{
			section = 0;
			order   = 0;
			}

		if( NS_FAILURE( ns_file_print( file, " " NS_FMT_ULONG " " NS_FMT_ULONG, section, order ), error ) )
			return error;
		}

	return ns_file_print( file, NS_STRING_NEWLINE );
	}


NS_PRIVATE NsError _ns_swc_do_write_vertices
	( 
	NsFile         *file,
	nsulong         flags,
	nsmodeledge     parent_edge,
	nsmodelvertex   parent_vertex,
	nsmodelvertex   curr_vertex,
	nslong          parent_label,
	nslong         *label
	)
	{
	nsmodelvertex  dest_vertex;
	nslong         curr_label;
	NsError        error;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nslong         color;


	ns_assert( NS_MODEL_COLOR_WHITE == ns_model_vertex_get_color( curr_vertex ) );
	ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_GREY );

	curr_label = *label;

	if( NS_FAILURE( _ns_swc_write_vertex(
							file,
							curr_vertex,
							parent_edge,
							parent_label,
							curr_label,
							flags
							),
							error ) )
		return error;

	*label = *label + 1;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			color = ns_model_vertex_get_color( dest_vertex );

			if( NS_MODEL_COLOR_GREY == color )
				{
				#ifdef NS_DEBUG
				ns_warning(
					NS_WARNING_LEVEL_RECOVERABLE
					NS_MODULE
					" .SWC: loop detected"
					);
				#endif

				ns_assert_not_reached();
				}
			else if( NS_MODEL_COLOR_WHITE == color )
				{
				if( NS_FAILURE( _ns_swc_do_write_vertices(
										file,
										flags,
										curr_edge,
										curr_vertex,
										dest_vertex,
										curr_label,
										label
										),
										error ) )
					return error;
				}
			}
		}

	ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_BLACK );
	return ns_no_error();
	}


NS_PRIVATE const nschar* _ns_swc_header_fields[ NS_SWC_HEADER_NUM_FIELDS ] =
	{
	"ORIGINAL_SOURCE",
	"CREATURE",
	"REGION",
	"FIELD/LAYER",
	"TYPE",
	"CONTRIBUTOR",
	"REFERENCE",
	"RAW",
	"EXTRAS",
	"SOMA_AREA",
	"SHINKAGE_CORRECTION",
	"VERSION_NUMBER",
	"VERSION_DATE",
	"SCALE"
	};


NS_PRIVATE void _ns_swc_header_init_strings( const NsSwcHeader *swc_header, const nschar **strings )
	{
	strings[ 0] = swc_header->source;
	strings[ 1] = swc_header->creature;
	strings[ 2] = swc_header->region;
	strings[ 3] = swc_header->field;
	strings[ 4] = swc_header->type;
	strings[ 5] = swc_header->contributor;
	strings[ 6] = swc_header->reference;
	strings[ 7] = swc_header->raw;
	strings[ 8] = swc_header->extras;
	strings[ 9] = swc_header->soma_area;
	strings[10] = swc_header->shrinkage;
	strings[11] = swc_header->version_number;
	strings[12] = swc_header->version_date;
	strings[13] = swc_header->scale;
	}


NS_PRIVATE NsError _ns_swc_write_header( NsFile *file, const NsSwcHeader *swc_header )
	{
	nssize         field;
	NsError        error;
	const nschar*  strings[ NS_SWC_HEADER_NUM_FIELDS ];


	_ns_swc_header_init_strings( swc_header, strings );

	for( field = 0; field < NS_SWC_HEADER_NUM_FIELDS; ++field )
		if( NS_FAILURE( ns_file_print(
								file,
								"# "
								NS_FMT_STRING
								" "
								NS_FMT_STRING
								NS_STRING_NEWLINE,
								_ns_swc_header_fields[ field ],
								strings[ field ]
								),
								error ) )
			return error;

	return ns_file_print( file, NS_STRING_NEWLINE );
	}


NS_PRIVATE NsError _ns_swc_write_single_file
	(
	NsModel            *model,
	const NsSwcHeader  *swc_header,
	const nschar       *file_name,
	nsulong             flags
	)
	{
	NsFile         file;
	nsmodelorigin  curr_origin;
	nsmodelorigin  end_origins;
	nslong         label;
	NsError        error;


	NS_USE_VARIABLE( flags );

	ns_file_construct( &file );
	
	if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_WRITE ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	if( NS_FAILURE( _ns_swc_write_header( &file, swc_header ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	ns_model_color_vertices( model, NS_MODEL_COLOR_WHITE );

	label = 1;

	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		if( NS_FAILURE( _ns_swc_do_write_vertices(
								&file,
								flags,
								NS_MODEL_EDGE_NIL,
								NS_MODEL_VERTEX_NIL,
								ns_model_origin_vertex( curr_origin ),
								-1,
								&label
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

	ns_file_destruct( &file );
	return ns_no_error();
	}


NS_PRIVATE const nschar* _ns_swc_ext = ".swc";

NS_PRIVATE NsError _ns_swc_write_multi_file
	(
	NsModel            *model,
	const NsSwcHeader  *swc_header,
	const nschar       *file_name,
	nsulong             flags
	)
	{
	NsFile          file;
	nssize          num_origins;
	nsmodelorigin   curr_origin;
	nsmodelorigin   end_origins;
	nslong          label;
	nssize          file_name_length;
	nssize          file_num_length;
	nssize          file_ext_length;
	nssize          file_num;
	nschar         *ptr;
	NsError         error;
	nschar          buffer[ NS_PATH_SIZE ] = { NS_ASCII_NULL };


	num_origins = ns_model_num_origins( model );

	file_name_length = ns_ascii_strlen( file_name );
	file_ext_length  = ns_ascii_strlen( _ns_swc_ext );

	/* NOTE: This will be the file number with the most digits. */
	if( 1 < num_origins )
		{
		if( flags & NS_SWC_FILE_NUM_ZERO_PAD )
			ns_sprint( buffer,
						  NS_MAKE_FMT( "04", NS_FMT_TYPE_ULONG ),
						  num_origins
						);
		else
			ns_sprint( buffer, NS_FMT_ULONG, num_origins );
		}

	file_num_length = ns_ascii_strlen( buffer );

	if( NS_PATH_SIZE - 1 < file_name_length + file_num_length + file_ext_length )
		return ns_error_nametoolong( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	ns_sprint( buffer, NS_FMT_STRING, file_name );

	/* Chop out the extension from the file name if its present. */
	ptr = ns_ascii_strrchr( buffer, '.' );
	if( NULL != ptr && ns_ascii_streq( ptr, _ns_swc_ext ) )
		*ptr = NS_ASCII_NULL;

	/* Set a pointer to the end of the string so we can concatenate
		the file number and extension. */
	ptr = buffer + ns_ascii_strlen( buffer );

	ns_model_color_vertices( model, NS_MODEL_COLOR_WHITE );

	file_num    = ( flags & NS_SWC_FILE_NUM_ONE_BASED ) ? 1 : 0;
	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		if( 1 < num_origins )
			{
			if( flags & NS_SWC_FILE_NUM_ZERO_PAD )
				ns_sprint( ptr, NS_MAKE_FMT( "04", NS_FMT_TYPE_ULONG ), file_num );
			else
				ns_sprint( ptr, NS_FMT_ULONG, file_num );
			}

		ns_ascii_strcat( ptr, _ns_swc_ext );

		ns_assert( ns_ascii_strlen( buffer ) <= NS_PATH_SIZE - 1 );

		label = 1;

		ns_file_construct( &file );
	
		if( NS_FAILURE( ns_file_open( &file, buffer, NS_FILE_MODE_WRITE ), error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		if( NS_FAILURE( _ns_swc_write_header( &file, swc_header ), error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		if( NS_FAILURE( _ns_swc_do_write_vertices(
								&file,
								flags,
								NS_MODEL_EDGE_NIL,
								NS_MODEL_VERTEX_NIL,
								ns_model_origin_vertex( curr_origin ),
								-1,
								&label
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		ns_file_destruct( &file );
		++file_num;
		}

	return ns_no_error();
	}


NsError ns_model_write_swc
	(
	NsModel            *model,
	const NsSwcHeader  *swc_header,
	const nschar       *file_name,
	nsulong             flags
	)
	{
	ns_assert( NULL != model );
	ns_assert( NULL != swc_header );
	ns_assert( NULL != file_name );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( model=" NS_FMT_STRING_DOUBLE_QUOTED
		", swc_header=" NS_FMT_POINTER
		", file_name=" NS_FMT_STRING_DOUBLE_QUOTED
		", flags=" NS_FMT_ULONG
		" )",
		ns_model_get_name( model ),
		swc_header,
		file_name,
		flags
		);

	return ( flags & NS_SWC_MULTIPLE_FILES ) ?
			 _ns_swc_write_multi_file( model, swc_header, file_name, flags ) :
			 _ns_swc_write_single_file( model, swc_header, file_name, flags );
	}


NS_PRIVATE void _ns_swc_parse_header_field( NsSwcHeader *swc_header, const nschar *ptr )
	{
	nssize   length;
	nssize   field;
	nschar*  strings[ NS_SWC_HEADER_NUM_FIELDS ];


	_ns_swc_header_init_strings( swc_header, strings );

	/* Skip the '#' character. */
	++ptr;

	/* NOTE: White-space in between the '#' and the field is OK. */
	while( *ptr && ns_ascii_isspace( *ptr ) )
		++ptr;

	/* No field label... */
	if( ! *ptr )
		return;

	/* Find the field. */
	for( field = 0; field < NS_SWC_HEADER_NUM_FIELDS; ++field )
		{
		length = ns_ascii_strlen( _ns_swc_header_fields[ field ] );

		if( ns_ascii_strneq( ptr, _ns_swc_header_fields[ field ], length ) )
			{
			/* Skip field label. */
			ptr += length;

			/* Skip leading white-space of the field value. */
			while( *ptr && ns_ascii_isspace( *ptr ) )
				++ptr;

			/* Assure string termination. */
			strings[ field ][0] = NS_ASCII_NULL;

			/* Copy the field value. */
			ns_ascii_strncpy( strings[ field ], ptr, NS_SWC_HEADER_FIELD_MAX_CHARS );
			return;
			}
		}
	}


NS_PRIVATE NsError _ns_swc_parse_vertex
	(
	NsModel       *model,
	const nschar  *ptr,
	nslong        *last_index,
	NsHashTable   *vertices
	)
	{
	nsmodelvertex  curr_vertex;
	nsmodelvertex  parent_vertex;
	nsmodeledge    edge;
	nslong         curr_index;
	nslong         parent_index;
	nslong         vertex_type;
	NsVector3f     V;
	nsfloat        radius;
	nsulong        section;
	nsulong        order;
	NsError        error;


	section = 0;
	order   = 0;

	if( 7 > _ns_sscan(
					ptr,
					NS_FMT_LONG
					NS_FMT_LONG
					NS_FMT_DOUBLE
					NS_FMT_DOUBLE
					NS_FMT_DOUBLE
					NS_FMT_DOUBLE
					NS_FMT_LONG
					NS_FMT_ULONG
					NS_FMT_ULONG,
					&curr_index,
					&vertex_type,
					&V.x,
					&V.y,
					&V.z,
					&radius,
					&parent_index,
					&section,
					&order
					) )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Index number must continually increment starting at 1. */
	if( curr_index != *last_index + 1 )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Parent index of -1 indicates soma vertex, otherwise must be >= 1.
		Also parent indices must always be less than the current index. */
	if( 0 == parent_index || parent_index < -1 || curr_index <= parent_index )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NS_FAILURE( ns_model_add_vertex( model, &curr_vertex ), error ) )
		return error;

	if( NS_FAILURE( ns_hash_table_add(
							vertices,
							NS_LONG_TO_POINTER( curr_index ),
							curr_vertex
							),
							error ) )
		return error;

	ns_model_vertex_set_position( curr_vertex, &V );
	ns_model_vertex_set_radius( curr_vertex, radius );

	/* Add edge or origin, depending on the parent index. */
	if( -1 == parent_index )
		{
		if( NS_FAILURE( ns_model_add_origin( model, curr_vertex, NULL ), error ) )
			return error;
		}
	else
		{
		if( ! ns_hash_table_lookup( vertices, NS_LONG_TO_POINTER( parent_index ), &parent_vertex ) )
			return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

		if( NS_FAILURE( ns_model_vertex_add_edge( curr_vertex, parent_vertex, &edge, NULL ), error ) )
			return error;

		ns_model_edge_set_section( edge, section, NS_TRUE );
		ns_model_edge_set_order( edge, ( nsushort )order, NS_TRUE );
		}

	*last_index = curr_index;

	switch( ( nsint )vertex_type )
		{
		case _NS_SWC_VERTEX_SOMA:
			ns_model_vertex_set_function_type( curr_vertex, NS_MODEL_FUNCTION_SOMA );
			break;

		case _NS_SWC_VERTEX_AXON:
			ns_model_vertex_set_function_type( curr_vertex, NS_MODEL_FUNCTION_AXON );
			break;

		case _NS_SWC_VERTEX_DENDRITE:
			ns_model_vertex_set_function_type( curr_vertex, NS_MODEL_FUNCTION_BASAL_DENDRITE );
			break;

		case _NS_SWC_VERTEX_APICAL_DENDRITE:
			ns_model_vertex_set_function_type( curr_vertex, NS_MODEL_FUNCTION_APICAL_DENDRITE );
			break;
		}

	return ns_no_error();
	}


NS_PRIVATE nsuint _ns_swc_parse_file_key_hash_func( nsconstpointer i )
	{  return NS_POINTER_TO_UINT( i );  }

NS_PRIVATE nsboolean _ns_swc_parse_file_key_equal_func( nsconstpointer i1, nsconstpointer i2 )
	{  return i1 == i2;  }


#define _NS_SWC_MAX_LINE  1024

NS_PRIVATE NsError _ns_swc_parse_file
	(
	NsFile       *file,
	NsModel      *model,
	NsSwcHeader  *swc_header,
	nslong       *line_num
	)
	{
	nsboolean     eof;
	nschar       *ptr;
	nslong        last_index;
	NsHashTable   vertices;
	NsError       error;
	nschar        line[ _NS_SWC_MAX_LINE ];


	if( NS_FAILURE( ns_hash_table_construct(
							&vertices,
							_ns_swc_parse_file_key_hash_func,
							_ns_swc_parse_file_key_equal_func,
							NULL,
							NULL
							),
							error ) )
		return error;

	error      = ns_no_error();
	eof        = NS_FALSE;
	last_index = 0;

	if( NULL != line_num )
		*line_num = 0;

	while( ! eof )
		{
		*line = NS_ASCII_NULL;

		if( NS_FAILURE( ns_file_readline( file, line, _NS_SWC_MAX_LINE, &eof ), error ) )
			break;

		if( NULL != line_num )
			*line_num = *line_num + 1;

		ptr = line;

		/* Skip leading white-space. */
		while( *ptr && ns_ascii_isspace( *ptr ) )
			++ptr;

		/* Skip empty lines. */
		if( ! *ptr )
			continue;

		/* If '#' is first character, assume header field... */
		if( '#' == *ptr )
			_ns_swc_parse_header_field( swc_header, ptr );
		else if( NS_FAILURE( _ns_swc_parse_vertex(
										model,
										ptr,
										&last_index,
										&vertices
										),
										error ) )
			break;
		}

	ns_hash_table_destruct( &vertices );
	return error;
	}


NS_PRIVATE NsError _ns_swc_read_single_file
	(
	NsModel       *model,
	NsSwcHeader   *swc_header,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	)
	{
	NsFile   file;
	NsError  error;


	NS_USE_VARIABLE( flags );

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_READ ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	error = _ns_swc_parse_file( &file, model, swc_header, line_num );

	ns_file_destruct( &file );

	return error;
	}


/* TODO??? */
NS_PRIVATE NsError _ns_swc_read_multi_file
	(
	NsModel       *model,
	NsSwcHeader   *swc_header,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	)
	{
	NS_USE_VARIABLE( model );
	NS_USE_VARIABLE( swc_header );
	NS_USE_VARIABLE( file_name );
	NS_USE_VARIABLE( flags );
	NS_USE_VARIABLE( line_num );

	return ns_no_error();
	}


NsError ns_model_read_swc
	(
	NsModel       *model,
	NsSwcHeader   *swc_header,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	)
	{
	NsError error;

	ns_assert( NULL != model );
	ns_assert( NULL != swc_header );
	ns_assert( NULL != file_name );

	ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( model=" NS_FMT_STRING_DOUBLE_QUOTED
		", swc_header=" NS_FMT_POINTER
		", file_name=" NS_FMT_STRING_DOUBLE_QUOTED
		", flags=" NS_FMT_ULONG
		", line_num=" NS_FMT_POINTER
		" )",
		ns_model_get_name( model ),
		swc_header,
		file_name,
		flags,
		line_num
		);

	error = ( flags & NS_SWC_MULTIPLE_FILES ) ?
					_ns_swc_read_multi_file( model, swc_header, file_name, flags, line_num ) :
					_ns_swc_read_single_file( model, swc_header, file_name, flags, line_num );

	if( ! ns_is_error( error ) )
		ns_model_function_edges_from_vertices( model );

	return error;
	}




NS_PRIVATE void _ns_swc_do_label_vertices
	( 
	nsmodelvertex   parent_vertex,
	nsmodelvertex   curr_vertex,
	nslong         *label
	)
	{
	nsmodelvertex  dest_vertex;
	nsmodeledge    curr_edge;
	nsmodeledge    end_edges;
	nslong         color;


	ns_assert( NS_MODEL_COLOR_WHITE == ns_model_vertex_get_color( curr_vertex ) );
	ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_GREY );

	ns_model_vertex_set_data( curr_vertex, NS_MODEL_VERTEX_DATA_SLOT0, NS_LONG_TO_POINTER( *label ) );

	*label = *label + 1;

	curr_edge = ns_model_vertex_begin_edges( curr_vertex );
	end_edges = ns_model_vertex_end_edges( curr_vertex );

	for( ; ns_model_edge_not_equal( curr_edge, end_edges );
			 curr_edge = ns_model_edge_next( curr_edge ) )
		{
		dest_vertex = ns_model_edge_dest_vertex( curr_edge );

		if( ns_model_vertex_not_equal( dest_vertex, parent_vertex ) )
			{
			color = ns_model_vertex_get_color( dest_vertex );

			if( NS_MODEL_COLOR_GREY == color )
				{
				#ifdef NS_DEBUG
				ns_warning(
					NS_WARNING_LEVEL_RECOVERABLE
					NS_MODULE
					" .SWC: loop detected"
					);
				#endif

				ns_assert_not_reached();
				}
			else if( NS_MODEL_COLOR_WHITE == color )
				_ns_swc_do_label_vertices( curr_vertex, dest_vertex, label );
			}
		}

	ns_model_vertex_set_color( curr_vertex, NS_MODEL_COLOR_BLACK );
	}


NS_PRIVATE void _ns_swc_set_labels_single_file( NsModel *model )
	{
	nsmodelorigin  curr_origin;
	nsmodelorigin  end_origins;
	nslong         label;


	ns_model_color_vertices( model, NS_MODEL_COLOR_WHITE );

	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	/* NOTE: For single file, the label is NOT reset for each origin. */
	label = 1;

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		_ns_swc_do_label_vertices(
			NS_MODEL_VERTEX_NIL,
			ns_model_origin_vertex( curr_origin ),
			&label
			);
	}


NS_PRIVATE void _ns_swc_set_labels_multi_file( NsModel *model )
	{
	nsmodelorigin  curr_origin;
	nsmodelorigin  end_origins;
	nslong         label;


	ns_model_color_vertices( model, NS_MODEL_COLOR_WHITE );

	curr_origin = ns_model_begin_origins( model );
	end_origins = ns_model_end_origins( model );

	for( ; ns_model_origin_not_equal( curr_origin, end_origins );
			 curr_origin = ns_model_origin_next( curr_origin ) )
		{
		/* NOTE: For multiple files, the label is reset for each origin. */
		label = 1;

		_ns_swc_do_label_vertices(
			NS_MODEL_VERTEX_NIL,
			ns_model_origin_vertex( curr_origin ),
			&label
			);
		}
	}


void ns_model_set_swc_labels( NsModel *model, nsboolean multiple_files )
	{
	if( multiple_files )
		_ns_swc_set_labels_multi_file( model );
	else
		_ns_swc_set_labels_single_file( model );
	}
