
#define _NS_VOXEL_BUFFER_FORALL_XY_TEMPLATE( type, func )\
	NsVector3i   V, C1, C2;\
	nsint        Dz;\
	const type  *slice;\
	const type  *row;\
	const type  *pixel;\
	\
	if( NULL != roi )\
		{\
		C1.x = roi->C1.x;\
		C1.y = roi->C1.y;\
		C1.z = roi->C1.z;\
		C2.x = roi->C2.x + 1;\
		C2.y = roi->C2.y + 1;\
		C2.z = roi->C2.z + 1;\
		Dz   = ns_cubei_length( roi );\
		}\
	else\
		{\
		C1.x = 0;\
		C1.y = 0;\
		C1.z = 0;\
		C2.x = voxel_buffer->width;\
		C2.y = voxel_buffer->height;\
		C2.z = voxel_buffer->length;\
		Dz   = voxel_buffer->length;\
		}\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_BEGIN );\
	\
	slice = voxel_buffer->mem_block;\
	slice = NS_OFFSET_POINTER( const type, slice, C1.z * voxel_buffer->bytes_per_slice );\
	slice = NS_OFFSET_POINTER( const type, slice, C1.y * voxel_buffer->bytes_per_row );\
	slice = NS_OFFSET_POINTER( const type, slice, C1.x * sizeof( type ) );\
	\
	for( V.z = C1.z; V.z < C2.z; ++V.z )\
		{\
		if( NULL != progress )\
			{\
			if( ns_progress_cancelled( progress ) )\
				return;\
			\
			ns_progress_update(\
				progress,\
				100.0f * ( ( nsfloat )V.z / ( nsfloat )Dz )\
				);\
			}\
		\
		row = slice;\
		\
		for( V.y = C1.y; V.y < C2.y; ++V.y )\
			{\
			pixel = row;\
			\
			for( V.x = C1.x; V.x < C2.x; ++V.x )\
				{\
				func;\
				++pixel;\
				}\
			\
			row = NS_OFFSET_POINTER( const type, row, voxel_buffer->bytes_per_row );\
			}\
		\
		slice = NS_OFFSET_POINTER( const type, slice, voxel_buffer->bytes_per_slice );\
		}\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_END )




#define _NS_VOXEL_BUFFER_FORALL_ZY_TEMPLATE( type, func )\

/*
	NsVector3i   V;\
	NsVector3i   D;\
	const type  *column;\
	const type  *row;\
	const type  *pixel;\
	\
	D.x = voxel_buffer->width;\
	D.y = voxel_buffer->height;\
	D.z = voxel_buffer->length;\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_BEGIN );\
	\
	column = voxel_buffer->mem_block;\
	\
	for( V.x = 0; V.x < D.x; ++V.x )\
		{\
		if( NULL != progress )\
			{\
			if( ns_progress_cancelled( progress ) )\
				return;\
			\
			ns_progress_update(\
				progress,\
				100.0f * ( ( nsfloat )V.x / ( nsfloat )D.x )\
				);\
			}\
		\
		row = column;\
		\
		for( V.y = 0; V.y < D.y; ++V.y )\
			{\
			pixel = row;\
			\
			for( V.z = 0; V.z < D.z; ++V.z )\
				{\
				func;\
				pixel = NS_OFFSET_POINTER( const type, pixel, voxel_buffer->bytes_per_slice );\
				}\
			\
			row = NS_OFFSET_POINTER( const type, row, voxel_buffer->bytes_per_row );\
			}\
		\
		++column;\
		}\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_END )
*/



#define _NS_VOXEL_BUFFER_FORALL_XZ_TEMPLATE( type, func )\

/*
	NsVector3i   V;\
	NsVector3i   D;\
	const type  *slice;\
	const type  *row;\
	const type  *pixel;\
	\
	D.x = voxel_buffer->width;\
	D.y = voxel_buffer->height;\
	D.z = voxel_buffer->length;\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_BEGIN );\
	\
	row = voxel_buffer->mem_block;\
	\
	for( V.y = 0; V.y < D.y; ++V.y )\
		{\
		if( NULL != progress )\
			{\
			if( ns_progress_cancelled( progress ) )\
				return;\
			\
			ns_progress_update(\
				progress,\
				100.0f * ( ( nsfloat )V.y / ( nsfloat )D.y )\
				);\
			}\
		\
		slice = row;\
		\
		for( V.z = 0; V.z < D.z; ++V.z )\
			{\
			pixel = slice;\
			\
			for( V.x = 0; V.x < D.x; ++V.x )\
				{\
				func;\
				++pixel;\
				}\
			\
			slice = NS_OFFSET_POINTER( const type, slice, voxel_buffer->bytes_per_slice );\
			}\
		\
		row = NS_OFFSET_POINTER( const type, row, voxel_buffer->bytes_per_row );\
		}\
	\
	if( NULL != progress )\
		ns_progress_update( progress, NS_PROGRESS_END )
*/
