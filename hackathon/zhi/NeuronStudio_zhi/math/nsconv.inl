

/*
if( NS_CONV_EDGE_OP_NONE != edge_op )
	switch( edge_op )\
		{\
		case NS_CONV_EDGE_OP_COPY_SRC:\
			for( x = 0; x < rank / 2; ++x )\
				{\
				*( ( dest_curr - 1 ) - x ) = *src_row;\
				*( dest_curr + x_end + x ) = *( src_row + width - 1 );\
				}\
			break;\
		\
		case NS_CONV_EDGE_OP_COPY_DEST:\
			for( x = 0; x < rank / 2; ++x )\
				{\
				*( ( dest_curr - 1 ) - x ) = *dest_row;\
				*( dest_curr + x_end + x ) = *( dest_row + x_end - 1 );\
				}\
			break;\
		\
		case NS_CONV_EDGE_OP_ZERO:\
			for( x = 0; x < rank / 2; ++x )\
				{\
				*( ( dest_curr - 1 ) - x ) = ( data_type )0;\
				*( dest_curr + x_end + x ) = ( data_type )0;\
				}\
			break;\
		}\
*/


#define _ns_conv_mult_double( v1, v2 ) ( (v1) * (v2) )
#define _ns_conv_mult_float( v1, v2 )  ( (v1) * (v2) )
#define _ns_conv_mult_long( v1, v2 )   ( (v1) * (v2) )
#define _ns_conv_add_double( v1, v2 )  ( (v1) + (v2) )
#define _ns_conv_add_float( v1, v2 )   ( (v1) + (v2) )
#define _ns_conv_add_long( v1, v2 )    ( (v1) + (v2) )


#define _NS_CONVNN_DECLARE( name, conv_type )\
void name\
	(\
	const conv_type*,\
	NsConvEdgeOpType,\
	nsconstpointer,\
	nssize,\
	nssize,\
	nssize,\
	const NsConvRegion2*,\
	nspointer,\
	NsProgress*,\
	nsfloat,\
	nsfloat,\
	nspointer\
	)


#define _NS_CONVNN_IMPLEMENT(\
	name,\
	rank,\
	conv_type,\
	kernel_type,\
	mult_kernel_type,\
	add_kernel_type,\
	to_kernel_type,\
	to_data_type\
	)\
void name\
	(\
	const conv_type      *C,\
	NsConvEdgeOpType      edge_op,\
	nsconstpointer        src,\
	nssize                width,\
	nssize                height,\
	nssize                bytes_per_row,\
	const NsConvRegion2  *sub_region,\
	nspointer             dest,\
	NsProgress           *progress,\
	nsfloat               percent_min,\
	nsfloat               percent_max,\
	nspointer             user_data\
	)\
	{\
	const NsConvRegion2  *region;\
	NsConvRegion2         full_region;\
	nsconstpointer        src_row;\
	nsconstpointer        src_kernel_row;\
	nspointer             dest_row;\
	nssize                x;\
	nssize                y;\
	nssize                x_end;\
	nssize                y_end;\
	kernel_type           value;\
	kernel_type           sum;\
	const kernel_type    *kernel;\
	nssize                kx;\
	nssize                ky;\
	\
	/* TODO: */NS_USE_VARIABLE( edge_op );\
	NS_USE_VARIABLE( user_data );\
	\
	if( NULL != sub_region )\
		region = sub_region;\
	else\
		{\
		full_region.x      = 0;\
		full_region.y      = 0;\
		full_region.width  = width;\
		full_region.height = height;\
		\
		region = &full_region;\
		}\
	\
	ns_assert( NULL != region );\
	ns_assert( region->x < width );\
	ns_assert( region->y < height );\
	ns_assert( region->x + region->width  <= width );\
	ns_assert( region->y + region->height <= height );\
	\
	if( region->width < rank || region->height < rank )\
		return;\
	\
	src_row = src;\
	src_row = NS_OFFSET_POINTER( const void, src_row, region->y * bytes_per_row );\
	\
	dest_row = dest;\
	dest_row = NS_OFFSET_POINTER( void, dest_row, region->y * bytes_per_row );\
	dest_row = NS_OFFSET_POINTER( void, dest_row, bytes_per_row * ( rank / 2 ) );\
	\
	x_end = region->width  - ( rank / 2 ) * 2;\
	y_end = region->height - ( rank / 2 ) * 2;\
	\
	for( y = 0; y < y_end; ++y )\
		{\
		if( NULL != progress )\
			{\
			if( ns_progress_cancelled( progress ) )\
				return;\
			\
			ns_progress_update(\
				progress,\
				percent_min +\
				( percent_max - percent_min ) *\
				( ( nsfloat )y / ( nsfloat )y_end )\
				);\
			}\
		\
		for( x = 0; x < x_end; ++x )\
			{\
			src_kernel_row = src_row;\
			kernel         = ( const kernel_type* )C;\
			sum            = ( kernel_type )0;\
			\
			for( ky = 0; ky < rank; ++ky )\
				{\
				for( kx = 0; kx < rank; ++kx )\
					{\
					to_kernel_type( src_kernel_row, region->x + x + kx, &value, user_data );\
					sum = add_kernel_type( sum, mult_kernel_type( *kernel, value ) );\
					\
					++kernel;\
					}\
				\
				src_kernel_row = NS_OFFSET_POINTER( const void, src_kernel_row, bytes_per_row );\
				}\
			\
			to_data_type( dest_row, region->x + x + rank / 2, &sum, user_data );\
			}\
		\
		src_row  = NS_OFFSET_POINTER( const void, src_row, bytes_per_row );\
		dest_row = NS_OFFSET_POINTER( void, dest_row, bytes_per_row );\
		}\
	}


#define _NS_CONVNNN_DECLARE( name, conv_type )\
void name\
	(\
	const conv_type*,\
	NsConvEdgeOpType,\
	nsconstpointer,\
	nssize,\
	nssize,\
	nssize,\
	nssize,\
	const NsConvRegion3*,\
	nspointer,\
	NsProgress*,\
	nsfloat,\
	nsfloat,\
	nspointer\
	)


#define _NS_CONVNNN_IMPLEMENT(\
	name,\
	rank,\
	conv_type,\
	kernel_type,\
	mult_kernel_type,\
	add_kernel_type,\
	to_kernel_type,\
	to_data_type\
	)\
void name\
	(\
	const conv_type     *C,\
	NsConvEdgeOpType      edge_op,\
	nsconstpointer        src,\
	nssize                width,\
	nssize                height,\
	nssize                length,\
	nssize                bytes_per_row,\
	const NsConvRegion3  *sub_region,\
	nspointer             dest,\
	NsProgress           *progress,\
	nsfloat               percent_min,\
	nsfloat               percent_max,\
	nspointer             user_data\
	)\
	{\
	const NsConvRegion3  *region;\
	NsConvRegion3         full_region;\
	nsconstpointer        src_slice;\
	nsconstpointer        src_row;\
	nsconstpointer        src_kernel_slice;\
	nsconstpointer        src_kernel_row;\
	nspointer             dest_slice;\
	nspointer             dest_row;\
	nssize                bytes_per_slice;\
	nssize                x;\
	nssize                y;\
	nssize                z;\
	nssize                x_end;\
	nssize                y_end;\
	nssize                z_end;\
	kernel_type          value;\
	kernel_type          sum;\
	const kernel_type   *kernel;\
	nssize                kx;\
	nssize                ky;\
	nssize                kz;\
	\
	/* TODO: */NS_USE_VARIABLE( edge_op );\
	NS_USE_VARIABLE( user_data );\
	\
	if( NULL != sub_region )\
		region = sub_region;\
	else\
		{\
		full_region.x      = 0;\
		full_region.y      = 0;\
		full_region.z      = 0;\
		full_region.width  = width;\
		full_region.height = height;\
		full_region.length = length;\
		\
		region = &full_region;\
		}\
	\
	ns_assert( NULL != region );\
	ns_assert( region->x < width );\
	ns_assert( region->y < height );\
	ns_assert( region->z < length );\
	ns_assert( region->x + region->width  <= width );\
	ns_assert( region->y + region->height <= height );\
	ns_assert( region->z + region->length <= length );\
	\
	if( region->width < rank || region->height < rank || region->length < rank )\
		return;\
	\
	bytes_per_slice = bytes_per_row * height;\
	\
	src_slice = src;\
	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->z * bytes_per_slice );\
	src_slice = NS_OFFSET_POINTER( const void, src_slice, region->y * bytes_per_row );\
	\
	dest_slice = dest;\
	dest_slice = NS_OFFSET_POINTER( void, dest_slice, region->z * bytes_per_slice );\
	dest_slice = NS_OFFSET_POINTER( void, dest_slice, region->y * bytes_per_row );\
	dest_slice = NS_OFFSET_POINTER( void, dest_slice, bytes_per_slice * ( rank / 2 ) );\
	\
	x_end = region->width  - ( rank / 2 ) * 2;\
	y_end = region->height - ( rank / 2 ) * 2;\
	z_end = region->length - ( rank / 2 ) * 2;\
	\
	for( z = 0; z < z_end; ++z )\
		{\
		if( NULL != progress )\
			{\
			if( ns_progress_cancelled( progress ) )\
				return;\
			\
			ns_progress_update(\
				progress,\
				percent_min +\
				( percent_max - percent_min ) *\
				( ( nsfloat )z / ( nsfloat )z_end )\
				);\
			}\
		\
		src_row  = src_slice;\
		dest_row = NS_OFFSET_POINTER( void, dest_slice, bytes_per_row * ( rank / 2 ) );\
		\
		for( y = 0; y < y_end; ++y )\
			{\
			for( x = 0; x < x_end; ++x )\
				{\
				src_kernel_slice = src_row;\
				kernel           = ( const kernel_type* )C;\
				sum              = ( kernel_type )0;\
				\
				for( kz = 0; kz < rank; ++kz )\
					{\
					src_kernel_row = src_kernel_slice;\
					\
					for( ky = 0; ky < rank; ++ky )\
						{\
						for( kx = 0; kx < rank; ++kx )\
							{\
							to_kernel_type( src_kernel_row, region->x + x + kx, &value, user_data );\
							sum = add_kernel_type( sum, mult_kernel_type( *kernel, value ) );\
							\
							++kernel;\
							}\
						\
						src_kernel_row = NS_OFFSET_POINTER( const void, src_kernel_row, bytes_per_row );\
						}\
					\
					src_kernel_slice = NS_OFFSET_POINTER( const void, src_kernel_slice, bytes_per_slice );\
					}\
				\
				to_data_type( dest_row, region->x + x + rank / 2, &sum, user_data );\
				}\
			\
			src_row  = NS_OFFSET_POINTER( const void, src_row, bytes_per_row );\
			dest_row = NS_OFFSET_POINTER( void, dest_row, bytes_per_row );\
			}\
		\
		src_slice  = NS_OFFSET_POINTER( const void, src_slice, bytes_per_slice );\
		dest_slice = NS_OFFSET_POINTER( void, dest_slice, bytes_per_slice );\
		}\
	}
