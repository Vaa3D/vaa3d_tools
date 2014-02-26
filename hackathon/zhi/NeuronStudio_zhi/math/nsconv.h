#ifndef __NS_MATH_CONV_H__
#define __NS_MATH_CONV_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmemory.h>
#include <std/nsprogress.h>
#include <math/nsconvdecls.h>

NS_DECLS_BEGIN

typedef struct _NsConvRegion2
	{
	nssize  x;
	nssize  y;
	nssize  width;
	nssize  height;
	}
	NsConvRegion2;

NS_IMPEXP NsConvRegion2* ns_conv_region2
	(
	NsConvRegion2  *region,
	nssize          x,
	nssize          y,
	nssize          width,
	nssize          height
	);


typedef struct _NsConvRegion3
	{
	nssize  x;
	nssize  y;
	nssize  z;
	nssize  width;
	nssize  height;
	nssize  length;
	}
	NsConvRegion3;

NS_IMPEXP NsConvRegion3* ns_conv_region3
	(
	NsConvRegion3  *region,
	nssize          x,
	nssize          y,
	nssize          z,
	nssize          width,
	nssize          height,
	nssize          length
	);


typedef enum
	{
	/* Ignore edges. */
	NS_CONV_EDGE_OP_NONE,

	/* The edges of the destination pixels are simply
		copied from the source pixels. */
	NS_CONV_EDGE_OP_COPY_SRC,

	/* The edges of the destination pixels are simply
		copied from the destination pixels. */
	NS_CONV_EDGE_OP_COPY_DEST,
	
	/* The edges of the destination pixels are zeros. */
	NS_CONV_EDGE_OP_ZERO
	}
	NsConvEdgeOpType;


NS_IMPEXP nsdouble* ns_conv33d_array ( NsConv33d  *C );
NS_IMPEXP nsfloat*  ns_conv33f_array ( NsConv33f  *C );
NS_IMPEXP nslong*   ns_conv33l_array ( NsConv33l  *C );
NS_IMPEXP nsdouble* ns_conv55d_array ( NsConv55d  *C );
NS_IMPEXP nsfloat*  ns_conv55f_array ( NsConv55f  *C );
NS_IMPEXP nslong*   ns_conv55l_array ( NsConv55l  *C );
NS_IMPEXP nsdouble* ns_conv333d_array( NsConv333d *C );
NS_IMPEXP nsfloat*  ns_conv333f_array( NsConv333f *C );
NS_IMPEXP nslong*   ns_conv333l_array( NsConv333l *C );
NS_IMPEXP nsdouble* ns_conv555d_array( NsConv555d *C );
NS_IMPEXP nsfloat*  ns_conv555f_array( NsConv555f *C );
NS_IMPEXP nslong*   ns_conv555l_array( NsConv555l *C );

NS_IMPEXP const nsdouble* ns_conv33d_const_array ( const NsConv33d  *C );
NS_IMPEXP const nsfloat*  ns_conv33f_const_array ( const NsConv33f  *C );
NS_IMPEXP const nslong*   ns_conv33l_const_array ( const NsConv33l  *C );
NS_IMPEXP const nsdouble* ns_conv55d_const_array ( const NsConv55d  *C );
NS_IMPEXP const nsfloat*  ns_conv55f_const_array ( const NsConv55f  *C );
NS_IMPEXP const nslong*   ns_conv55l_const_array ( const NsConv55l  *C );
NS_IMPEXP const nsdouble* ns_conv333d_const_array( const NsConv333d *C );
NS_IMPEXP const nsfloat*  ns_conv333f_const_array( const NsConv333f *C );
NS_IMPEXP const nslong*   ns_conv333l_const_array( const NsConv333l *C );
NS_IMPEXP const nsdouble* ns_conv555d_const_array( const NsConv555d *C );
NS_IMPEXP const nsfloat*  ns_conv555f_const_array( const NsConv555f *C );
NS_IMPEXP const nslong*   ns_conv555l_const_array( const NsConv555l *C );

/* Kernel is all zeros. */
NS_IMPEXP NsConv33d*  ns_conv33d_zero ( NsConv33d  *C );
NS_IMPEXP NsConv33f*  ns_conv33f_zero ( NsConv33f  *C );
NS_IMPEXP NsConv33l*  ns_conv33l_zero ( NsConv33l  *C );
NS_IMPEXP NsConv55d*  ns_conv55d_zero ( NsConv55d  *C );
NS_IMPEXP NsConv55f*  ns_conv55f_zero ( NsConv55f  *C );
NS_IMPEXP NsConv55l*  ns_conv55l_zero ( NsConv55l  *C );
NS_IMPEXP NsConv333d* ns_conv333d_zero( NsConv333d *C );
NS_IMPEXP NsConv333f* ns_conv333f_zero( NsConv333f *C );
NS_IMPEXP NsConv333l* ns_conv333l_zero( NsConv333l *C );
NS_IMPEXP NsConv555d* ns_conv555d_zero( NsConv555d *C );
NS_IMPEXP NsConv555f* ns_conv555f_zero( NsConv555f *C );
NS_IMPEXP NsConv555l* ns_conv555l_zero( NsConv555l *C );

/* Kernel is all zeros except for a one in the middle. */
NS_IMPEXP NsConv33d*  ns_conv33d_ident ( NsConv33d  *C );
NS_IMPEXP NsConv33f*  ns_conv33f_ident ( NsConv33f  *C );
NS_IMPEXP NsConv33l*  ns_conv33l_ident ( NsConv33l  *C );
NS_IMPEXP NsConv55d*  ns_conv55d_ident ( NsConv55d  *C );
NS_IMPEXP NsConv55f*  ns_conv55f_ident ( NsConv55f  *C );
NS_IMPEXP NsConv55l*  ns_conv55l_ident ( NsConv55l  *C );
NS_IMPEXP NsConv333d* ns_conv333d_ident( NsConv333d *C );
NS_IMPEXP NsConv333f* ns_conv333f_ident( NsConv333f *C );
NS_IMPEXP NsConv333l* ns_conv333l_ident( NsConv333l *C );
NS_IMPEXP NsConv555d* ns_conv555d_ident( NsConv555d *C );
NS_IMPEXP NsConv555f* ns_conv555f_ident( NsConv555f *C );
NS_IMPEXP NsConv555l* ns_conv555l_ident( NsConv555l *C );

/* For 'l' type kernel is all ones, else 1/n^d, e.g. 1/3^2 for NsConv33d. */
NS_IMPEXP NsConv33d*  ns_conv33d_blur ( NsConv33d  *C );
NS_IMPEXP NsConv33f*  ns_conv33f_blur ( NsConv33f  *C );
NS_IMPEXP NsConv33l*  ns_conv33l_blur ( NsConv33l  *C );
NS_IMPEXP NsConv55d*  ns_conv55d_blur ( NsConv55d  *C );
NS_IMPEXP NsConv55f*  ns_conv55f_blur ( NsConv55f  *C );
NS_IMPEXP NsConv55l*  ns_conv55l_blur ( NsConv55l  *C );
NS_IMPEXP NsConv333d* ns_conv333d_blur( NsConv333d *C );
NS_IMPEXP NsConv333f* ns_conv333f_blur( NsConv333f *C );
NS_IMPEXP NsConv333l* ns_conv333l_blur( NsConv333l *C );
NS_IMPEXP NsConv555d* ns_conv555d_blur( NsConv555d *C );
NS_IMPEXP NsConv555f* ns_conv555f_blur( NsConv555f *C );
NS_IMPEXP NsConv555l* ns_conv555l_blur( NsConv555l *C );

NS_IMPEXP NsConv33l* ns_conv33l_emboss( NsConv33l *C );
NS_IMPEXP NsConv33l* ns_conv33l_emboss_heavy( NsConv33l *C );

NS_IMPEXP NsConv33d* ns_conv33d_low_pass_spatial1( NsConv33d *C );
NS_IMPEXP NsConv33d* ns_conv33d_low_pass_spatial2( NsConv33d *C );
NS_IMPEXP NsConv33d* ns_conv33d_low_pass_spatial3( NsConv33d *C );

NS_IMPEXP NsConv33l* ns_conv33l_high_pass_spatial( NsConv33l *C );

NS_IMPEXP NsConv33l* ns_conv33l_shift_and_diff_horiz_edges( NsConv33l *C );
NS_IMPEXP NsConv33l* ns_conv33l_shift_and_diff_vert_edges( NsConv33l *C );
NS_IMPEXP NsConv33l* ns_conv33l_shift_and_diff_horiz_and_vert_edges( NsConv33l *C );


#include "nsconv.inl"


/* Example: NS_CONV33D_DECLARE( foo )

	void foo
		(
		const NsConv33D      *C,
		NsConvEdgeOpType      edge_op,
		nsconstpointer        src,
		nssize                width,
		nssize                height,
		nssize                bytes_per_row,
		const NsConvRegion2  *region,
		nspointer             dest,
		NsProgress           *progress,
		nsfloat               percent_min,
		nsfloat               percent_max,
		nspointer             user_data
		);

	Example: NS_CONV333D_DECLARE( foo )

	void foo
		(
		const NsConv333D     *C,
		NsConvEdgeOpType      edge_op,
		nsconstpointer        src,
		nssize                width,
		nssize                height,
		nssize                length,
		nssize                bytes_per_row,
		const NsConvRegion3  *region,
		nspointer             dest,
		NsProgress           *progress,
		nsfloat               percent_min,
		nsfloat               percent_max,
		nspointer             user_data
		);
*/

#define NS_CONV33D_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv33d )

#define NS_CONV33F_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv33f )

#define NS_CONV33L_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv33l )

#define NS_CONV55D_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv55d )

#define NS_CONV55F_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv55f )

#define NS_CONV55L_DECLARE( name )\
	_NS_CONVNN_DECLARE( name, NsConv55l )

#define NS_CONV333D_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv333d )

#define NS_CONV333F_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv333f )

#define NS_CONV333L_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv333l )

#define NS_CONV555D_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv555d )

#define NS_CONV555F_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv555f )

#define NS_CONV555L_DECLARE( name )\
	_NS_CONVNNN_DECLARE( name, NsConv555l )


/* #define _long_to_double( pl, x, pd, user_data ) *(pd) = e_type_cast( nsdouble, ((const nslong*)(pl))[x] )
	#define _double_to_long( pl, x, pd, user_data ) ((nslong*)(pl))[x] = e_type_cast( nslong, *(pd) )

	The above functions( and/or macros ) convert to and from
	the kernel and data types.

	NS_CONV33D_IMPLEMENT( foo, _long_to_double, _double_to_long )
*/

#define NS_CONV33D_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		3,\
		NsConv33d,\
		nsdouble,\
		_ns_conv_mult_double,\
		_ns_conv_add_double,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV33F_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		3,\
		NsConv33f,\
		nsfloat,\
		_ns_conv_mult_float,\
		_ns_conv_add_float,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV33L_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		3,\
		NsConv33l,\
		nslong,\
		_ns_conv_mult_long,\
		_ns_conv_add_long,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV55D_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		5,\
		NsConv55d,\
		nsdouble,\
		_ns_conv_mult_double,\
		_ns_conv_add_double,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV55F_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		5,\
		NsConv55f,\
		nsfloat,\
		_ns_conv_mult_float,\
		_ns_conv_add_float,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV55L_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNN_IMPLEMENT(\
		name,\
		5,\
		NsConv55l,\
		nslong,\
		_ns_conv_mult_long,\
		_ns_conv_add_long,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV333D_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		3,\
		NsConv333d,\
		nsdouble,\
		_ns_conv_mult_double,\
		_ns_conv_add_double,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV333F_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		3,\
		NsConv333f,\
		nsfloat,\
		_ns_conv_mult_float,\
		_ns_conv_add_float,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV333L_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		3,\
		NsConv333l,\
		nslong,\
		_ns_conv_mult_long,\
		_ns_conv_add_long,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV555D_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		5,\
		NsConv555d,\
		nsdouble,\
		_ns_conv_mult_double,\
		_ns_conv_add_double,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV555F_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		5,\
		NsConv555f,\
		nsfloat,\
		_ns_conv_mult_float,\
		_ns_conv_add_float,\
		to_kernel_type,\
		to_data_type\
		)

#define NS_CONV555L_IMPLEMENT( name, to_kernel_type, to_data_type )\
	_NS_CONVNNN_IMPLEMENT(\
		name,\
		5,\
		NsConv555l,\
		nslong,\
		_ns_conv_mult_long,\
		_ns_conv_add_long,\
		to_kernel_type,\
		to_data_type\
		)

NS_DECLS_END

#endif/* __NS_MATH_CONV_H__ */
