#include "nsconv.h"


NsConvRegion2* ns_conv_region2
	(
	NsConvRegion2  *region,
	nssize          x,
	nssize          y,
	nssize          width,
	nssize          height
	)
	{
	ns_assert( NULL != region );

	region->x      = x;
	region->y      = y;
	region->width  = width;
	region->height = height;

	return region;
	}


NsConvRegion3* ns_conv_region3
	(
	NsConvRegion3  *region,
	nssize          x,
	nssize          y,
	nssize          z,
	nssize          width,
	nssize          height,
	nssize          length
	)
	{
	ns_assert( NULL != region );

	region->x      = x;
	region->y      = y;
	region->z      = z;
	region->width  = width;
	region->height = height;
	region->length = length;

	return region;
	}


#define _NS_CONV_ARRAY( C, type )\
	ns_assert( NULL != (C) );\
	return ( type* )(C)

nsdouble* ns_conv33d_array( NsConv33d *C )
	{  _NS_CONV_ARRAY( C, nsdouble );  }

nsfloat* ns_conv33f_array( NsConv33f *C )
	{  _NS_CONV_ARRAY( C, nsfloat );  }

nslong* ns_conv33l_array( NsConv33l *C )
	{  _NS_CONV_ARRAY( C, nslong );  }

nsdouble* ns_conv55d_array( NsConv55d *C )
	{  _NS_CONV_ARRAY( C, nsdouble );  }

nsfloat* ns_conv55f_array( NsConv55f *C )
	{  _NS_CONV_ARRAY( C, nsfloat );  }

nslong* ns_conv55l_array( NsConv55l *C )
	{  _NS_CONV_ARRAY( C, nslong );  }

nsdouble* ns_conv333d_array( NsConv333d *C )
	{  _NS_CONV_ARRAY( C, nsdouble );  }

nsfloat* ns_conv333f_array( NsConv333f *C )
	{  _NS_CONV_ARRAY( C, nsfloat );  }

nslong* ns_conv333l_array( NsConv333l *C )
	{  _NS_CONV_ARRAY( C, nslong );  }

nsdouble* ns_conv555d_array( NsConv555d *C )
	{  _NS_CONV_ARRAY( C, nsdouble );  }

nsfloat* ns_conv555f_array( NsConv555f *C )
	{  _NS_CONV_ARRAY( C, nsfloat );  }

nslong* ns_conv555l_array( NsConv555l *C )
	{  _NS_CONV_ARRAY( C, nslong );  }

const nsdouble* ns_conv33d_const_array( const NsConv33d *C )
	{  _NS_CONV_ARRAY( C, const nsdouble );  }

const nsfloat* ns_conv33f_const_array( const NsConv33f *C )
	{  _NS_CONV_ARRAY( C, const nsfloat );  }

const nslong* ns_conv33l_const_array( const NsConv33l *C )
	{  _NS_CONV_ARRAY( C, const nslong );  }

const nsdouble* ns_conv55d_const_array( const NsConv55d *C )
	{  _NS_CONV_ARRAY( C, const nsdouble );  }

const nsfloat* ns_conv55f_const_array( const NsConv55f *C )
	{  _NS_CONV_ARRAY( C, const nsfloat );  }

const nslong* ns_conv55l_const_array( const NsConv55l *C )
	{  _NS_CONV_ARRAY( C, const nslong );  }

const nsdouble* ns_conv333d_const_array( const NsConv333d *C )
	{  _NS_CONV_ARRAY( C, const nsdouble );  }

const nsfloat* ns_conv333f_const_array( const NsConv333f *C )
	{  _NS_CONV_ARRAY( C, const nsfloat );  }

const nslong* ns_conv333l_const_array( const NsConv333l *C )
	{  _NS_CONV_ARRAY( C, const nslong );  }

const nsdouble* ns_conv555d_const_array( const NsConv555d *C )
	{  _NS_CONV_ARRAY( C, const nsdouble );  }

const nsfloat* ns_conv555f_const_array( const NsConv555f *C )
	{  _NS_CONV_ARRAY( C, const nsfloat );  }

const nslong* ns_conv555l_const_array( const NsConv555l *C )
	{  _NS_CONV_ARRAY( C, const nslong );  }


#define _NS_CONV_ZERO( type, C )\
	ns_assert( NULL != (C) );\
	ns_memzero( (C), sizeof( type ) );\
	return (C)

NsConv33d* ns_conv33d_zero( NsConv33d *C )
	{  _NS_CONV_ZERO( NsConv33d, C );  }

NsConv33f* ns_conv33f_zero( NsConv33f *C )
	{  _NS_CONV_ZERO( NsConv33f, C );  }

NsConv33l* ns_conv33l_zero( NsConv33l *C )
	{  _NS_CONV_ZERO( NsConv33l, C );  }

NsConv55d* ns_conv55d_zero( NsConv55d *C )
	{  _NS_CONV_ZERO( NsConv55d, C );  }

NsConv55f* ns_conv55f_zero( NsConv55f *C )
	{  _NS_CONV_ZERO( NsConv55f, C );  }

NsConv55l* ns_conv55l_zero( NsConv55l *C )
	{  _NS_CONV_ZERO( NsConv55l, C );  }

NsConv333d* ns_conv333d_zero( NsConv333d *C )
	{  _NS_CONV_ZERO( NsConv333d, C );  }

NsConv333f* ns_conv333f_zero( NsConv333f *C )
	{  _NS_CONV_ZERO( NsConv333f, C );  }

NsConv333l* ns_conv333l_zero( NsConv333l *C )
	{  _NS_CONV_ZERO( NsConv333l, C );  }

NsConv555d* ns_conv555d_zero( NsConv555d *C )
	{  _NS_CONV_ZERO( NsConv555d, C );  }

NsConv555f* ns_conv555f_zero( NsConv555f *C )
	{  _NS_CONV_ZERO( NsConv555f, C );  }

NsConv555l* ns_conv555l_zero( NsConv555l *C )
	{  _NS_CONV_ZERO( NsConv555l, C );  }


#define _NS_CONV_IDENT( conv_type, kernel_type, C, center, one )\
	ns_assert( NULL != (C) );\
	ns_memzero( (C), sizeof( conv_type ) );\
	( ( kernel_type* )(C) )[ (center) ] = (one);\
	return (C)

NsConv33d* ns_conv33d_ident( NsConv33d *C )
	{  _NS_CONV_IDENT( NsConv33d, nsdouble, C, 4, 1.0 );  }

NsConv33f* ns_conv33f_ident( NsConv33f *C )
	{  _NS_CONV_IDENT( NsConv33f, nsfloat, C, 4, 1.0f );  }

NsConv33l* ns_conv33l_ident( NsConv33l *C )
	{  _NS_CONV_IDENT( NsConv33l, nslong, C, 4, 1 );  }

NsConv55d* ns_conv55d_ident( NsConv55d *C )
	{  _NS_CONV_IDENT( NsConv55d, nsdouble, C, 12, 1.0 );  }

NsConv55f* ns_conv55f_ident( NsConv55f *C )
	{  _NS_CONV_IDENT( NsConv55f, nsfloat, C, 12, 1.0f );  }

NsConv55l* ns_conv55l_ident( NsConv55l *C )
	{  _NS_CONV_IDENT( NsConv55l, nslong, C, 12, 1 );  }

NsConv333d* ns_conv333d_ident( NsConv333d *C )
	{  _NS_CONV_IDENT( NsConv333d, nsdouble, C, 13, 1.0 );  }

NsConv333f* ns_conv333f_ident( NsConv333f *C )
	{  _NS_CONV_IDENT( NsConv333f, nsfloat, C, 13, 1.0f );  }

NsConv333l* ns_conv333l_ident( NsConv333l *C )
	{  _NS_CONV_IDENT( NsConv333l, nslong, C, 13, 1 );  }

NsConv555d* ns_conv555d_ident( NsConv555d *C )
	{  _NS_CONV_IDENT( NsConv555d, nsdouble, C, 62, 1.0 );  }

NsConv555f* ns_conv555f_ident( NsConv555f *C )
	{  _NS_CONV_IDENT( NsConv555f, nsfloat, C, 62, 1.0f );  }

NsConv555l* ns_conv555l_ident( NsConv555l *C )
	{  _NS_CONV_IDENT( NsConv555l, nslong, C, 62, 1 );  }


#define _NS_CONV_BLUR( type, kernel_type, C, size, value )\
	kernel_type  *kernel;\
	nssize         i;\
	ns_assert( NULL != (C) );\
	kernel = ns_conv##type##_array( (C) );\
	for( i = ( 0 ); i < size; ++i )\
		kernel[i] = (value);\
	return (C)

NsConv33d* ns_conv33d_blur( NsConv33d *C )
	{  _NS_CONV_BLUR( 33d, nsdouble, C, 9, 1.0 / 9.0 );  }

NsConv33f* ns_conv33f_blur( NsConv33f *C )
	{  _NS_CONV_BLUR( 33f, nsfloat, C, 9, 1.0f / 9.0f );  }

NsConv33l* ns_conv33l_blur( NsConv33l *C )
	{  _NS_CONV_BLUR( 33l, nslong, C, 9, 1 );  }

NsConv55d* ns_conv55d_blur( NsConv55d *C )
	{  _NS_CONV_BLUR( 55d, nsdouble, C, 25, 1.0 / 25.0 );  }

NsConv55f* ns_conv55f_blur( NsConv55f *C )
	{  _NS_CONV_BLUR( 55f, nsfloat, C, 25, 1.0f / 25.0f );  }

NsConv55l* ns_conv55l_blur( NsConv55l *C )
	{  _NS_CONV_BLUR( 55l, nslong, C, 25, 1 );  }

NsConv333d* ns_conv333d_blur( NsConv333d *C )
	{  _NS_CONV_BLUR( 333d, nsdouble, C, 27, 1.0 / 27.0 );  }

NsConv333f* ns_conv333f_blur( NsConv333f *C )
	{  _NS_CONV_BLUR( 333f, nsfloat, C, 27, 1.0f / 27.0f );  }

NsConv333l* ns_conv333l_blur( NsConv333l *C )
	{  _NS_CONV_BLUR( 333l, nslong, C, 27, 1 );  }

NsConv555d* ns_conv555d_blur( NsConv555d *C )
	{  _NS_CONV_BLUR( 555d, nsdouble, C, 125, 1.0 / 125.0 );  }

NsConv555f* ns_conv555f_blur( NsConv555f *C )
	{  _NS_CONV_BLUR( 555f, nsfloat, C, 125, 1.0f / 125.0f );  }

NsConv555l* ns_conv555l_blur( NsConv555l *C )
	{  _NS_CONV_BLUR( 555l, nslong, C, 125, 1 );  }


NsConv33l* ns_conv33l_emboss( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_emboss =
		{
		-1, 0, 0,
		 0, 0, 0,
		 0, 0, 1
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_emboss;

	return C;
	}


NsConv33l* ns_conv33l_emboss_heavy( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_emboss_heavy =
		{
		-1, -1, 0,
		-1,  0, 1,
		 0,  1, 1
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_emboss_heavy;

	return C;
	}


NsConv33d* ns_conv33d_low_pass_spatial1( NsConv33d *C )
	{
	NS_PRIVATE NsConv33d _ns_conv33d_low_pass_spatial1 =
		{
		1.0/9.0, 1.0/9.0, 1.0/9.0,
		1.0/9.0, 1.0/9.0, 1.0/9.0,
		1.0/9.0, 1.0/9.0, 1.0/9.0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33d_low_pass_spatial1;

	return C;
	}


NsConv33d* ns_conv33d_low_pass_spatial2( NsConv33d *C )
	{
	NS_PRIVATE NsConv33d _ns_conv33d_low_pass_spatial2 =
		{
		1.0/10.0, 1.0/10.0, 1.0/10.0,
		1.0/10.0, 1.0/ 5.0, 1.0/10.0,
		1.0/10.0, 1.0/10.0, 1.0/10.0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33d_low_pass_spatial2;

	return C;
	}


NsConv33d* ns_conv33d_low_pass_spatial3( NsConv33d *C )
	{
	NS_PRIVATE NsConv33d _ns_conv33d_low_pass_spatial3 =
		{
		1.0/16.0, 1.0/ 8.0, 1.0/16.0,
		1.0/ 8.0, 1.0/ 4.0, 1.0/ 8.0,
		1.0/16.0, 1.0/ 8.0, 1.0/16.0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33d_low_pass_spatial3;

	return C;
	}


NsConv33l* ns_conv33l_high_pass_spatial( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_high_pass_spatial =
		{
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_high_pass_spatial;

	return C;
	}


NsConv33l* ns_conv33l_shift_and_diff_horiz_edges( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_shift_and_diff_horiz_edges =
		{
		0, -1, 0,
		0,  1, 0,
		0,  0, 0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_shift_and_diff_horiz_edges;

	return C;
	}


NsConv33l* ns_conv33l_shift_and_diff_vert_edges( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_shift_and_diff_vert_edges =
		{
		 0, 0, 0,
		-1, 1, 0,
		 0, 0, 0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_shift_and_diff_vert_edges;

	return C;
	}


NsConv33l* ns_conv33l_shift_and_diff_horiz_and_vert_edges( NsConv33l *C )
	{
	NS_PRIVATE NsConv33l _ns_conv33l_shift_and_diff_horiz_and_vert_edges =
		{
		-1, 0, 0,
		 0, 1, 0,
		 0, 0, 0
		};

	ns_assert( NULL != C );
	*C = _ns_conv33l_shift_and_diff_horiz_and_vert_edges;

	return C;
	}
