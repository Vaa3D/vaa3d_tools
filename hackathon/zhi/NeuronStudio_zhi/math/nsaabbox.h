#ifndef __NS_MATH_AABBOX_H__
#define __NS_MATH_AABBOX_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <math/nsaabboxdecls.h>
#include <math/nsvectordecls.h>
#include <math/nspointdecls.h>

NS_DECLS_BEGIN

/*	3-dimensional Axis Aligned Bounding Box:

      \+Y    
       \      _______
        \    /      /\
         \  /      /  \
          \/______/____\_+X
          /\      \    /
         /  \      \  /
        /    \______\/
       /      
		/+Z   

	NOTE: Right-handed coordinate system.
*/

typedef enum
	{
	NS_AABBOX3_CORNER_LEFT_TOP_NEAR,
	NS_AABBOX3_CORNER_RIGHT_TOP_NEAR,
	NS_AABBOX3_CORNER_LEFT_BOTTOM_NEAR,
	NS_AABBOX3_CORNER_RIGHT_BOTTOM_NEAR,
	NS_AABBOX3_CORNER_LEFT_TOP_FAR,
	NS_AABBOX3_CORNER_RIGHT_TOP_FAR,
	NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR,
	NS_AABBOX3_CORNER_RIGHT_BOTTOM_FAR
	}
	NsAABBox3CornerType;


/* Returns B */
NS_IMPEXP NsAABBox2d* ns_aabbox2d
	(
	NsAABBox2d  *B,
	nsdouble     x_left,
	nsdouble     y_top,
	nsdouble     width,
	nsdouble     height
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d
	(
	NsAABBox3d  *B,
	nsdouble     x_left,
	nsdouble     y_top,
	nsdouble     z_near,
	nsdouble     width,
	nsdouble     height,
	nsdouble     length
	);


NS_IMPEXP void ns_aabbox3d_render
	(
	const NsAABBox3d  *B,
	void              ( *line_func )( nspointer, const NsVector3d*, const NsVector3d* ),
	nspointer         user_data
	);

NS_IMPEXP void ns_aabbox3d_render_ex
	(
	const NsAABBox3d   *B,
	void               ( *line_func )( nspointer, const NsVector3d*, const NsVector3d* ),
	const NsVector3d  *line_length,
	nspointer          user_data
	);


/* Internal. DO NOT USE! */
#define _ns_aabbox_left( B )    ( (B)->O.x       )
#define _ns_aabbox_top( B )     ( (B)->O.y       )
#define _ns_aabbox_near( B )    ( (B)->O.z       )

#define _ns_aabbox_right( B )   ( (B)->O.x + (B)->width  )
#define _ns_aabbox_bottom( B )  ( (B)->O.y - (B)->height )
#define _ns_aabbox_far( B )     ( (B)->O.z - (B)->length )

#define _ns_aabbox_width( B )   ( (B)->width     )
#define _ns_aabbox_height( B )  ( (B)->height    )
#define _ns_aabbox_length( B )  ( (B)->length    )


#define ns_aabbox2d_left( B )    _ns_aabbox_left( (B) )
#define ns_aabbox2d_top( B )     _ns_aabbox_top( (B) )
#define ns_aabbox2d_width( B )   _ns_aabbox_width( (B) )
#define ns_aabbox2d_height( B )  _ns_aabbox_height( (B) )

#define ns_aabbox3d_left( B )    _ns_aabbox_left( (B) )
#define ns_aabbox3d_top( B )     _ns_aabbox_top( (B) )
#define ns_aabbox3d_near( B )    _ns_aabbox_near( (B) )
#define ns_aabbox3d_width( B )   _ns_aabbox_width( (B) )
#define ns_aabbox3d_height( B )  _ns_aabbox_height( (B) )
#define ns_aabbox3d_length( B )  _ns_aabbox_length( (B) )


/* Returns 'O' */
NS_IMPEXP NsPoint2d* ns_aabbox2d_origin( const NsAABBox2d *box, NsPoint2d *O );

NS_IMPEXP NsPoint3d* ns_aabbox3d_origin( const NsAABBox3d *box, NsPoint3d *O );


/* Returns 'corner' */
NS_IMPEXP NsPoint3d* ns_aabbox3d_corner
	(
	const NsAABBox3d     *box,
	NsAABBox3CornerType   which,
	NsPoint3d            *corner
	);


NS_IMPEXP void ns_aabbox3d_corners
	(
	const NsAABBox3d  *box,
	NsPoint3d          corners[ 8 ]
	);


/* The "min" corner of the box, i.e. NS_AABBOX3_CORNER_LEFT_BOTTOM_FAR */
NS_IMPEXP NsPoint3d* ns_aabbox3d_min( const NsAABBox3d *box, NsPoint3d *P );

/* The "max" corner of the box, i.e. NS_AABBOX3_CORNER_RIGHT_TOP_NEAR */
NS_IMPEXP NsPoint3d* ns_aabbox3d_max( const NsAABBox3d *box, NsPoint3d *P );


NS_IMPEXP NsAABBox2d* ns_aabbox2d_scale
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           s
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_scale
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           s
	);


NS_IMPEXP NsAABBox2d* ns_aabbox2d_non_uni_scale
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	const NsVector2d  *S
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_non_uni_scale
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	const NsVector3d  *S
	);


NS_IMPEXP NsAABBox2d* ns_aabbox2d_scale_xy
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           sx,
	nsdouble           sy
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_scale_xyz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           sx,
	nsdouble           sy,
	nsdouble           sz
	);


NS_IMPEXP NsAABBox2d* ns_aabbox2d_translate
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	const NsVector2d  *V
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	const NsVector3d  *V
	);


NS_IMPEXP NsAABBox2d* ns_aabbox2d_translate_xy
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           x,
	nsdouble           y
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_xyz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           y,
	nsdouble           z
	);


NS_IMPEXP NsAABBox2d* ns_aabbox2d_translate_x
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           x
	);

NS_IMPEXP NsAABBox2d* ns_aabbox2d_translate_y
	(
	NsAABBox2d        *C,
	const NsAABBox2d  *A,
	nsdouble           y
	);


NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_x
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_y
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           y
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_z
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           z
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_xy
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           y
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_xz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           x,
	nsdouble           z
	);

NS_IMPEXP NsAABBox3d* ns_aabbox3d_translate_yz
	(
	NsAABBox3d        *C,
	const NsAABBox3d  *A,
	nsdouble           y,
	nsdouble           z
	);


/* NOTE: For inflation/deflation functions, BOTH the
	origin and the dimensions can be affected. Pass
	positive values for inflation, else negative for
	deflation. */
NS_IMPEXP NsAABBox2d* ns_aabbox2d_xflate( NsAABBox2d *C, const NsAABBox2d *A, nsdouble v );
NS_IMPEXP NsAABBox3d* ns_aabbox3d_xflate( NsAABBox3d *C, const NsAABBox3d *A, nsdouble v );

NS_IMPEXP NsAABBox2d* ns_aabbox2d_cmpd_xflate( NsAABBox2d *B, nsdouble v );
NS_IMPEXP NsAABBox3d* ns_aabbox3d_cmpd_xflate( NsAABBox3d *B, nsdouble v );


/*  2D version(4 bits):

         |      |
    1001 | 1000 | 1010
   ______|______|______
         |      |
    0001 | 0000 | 0010
   ______|______|______
         |      |
    0101 | 0100 | 0110
         |      |

	NOTE: 0000 is the interior of the bounding box.
*/
#define NS_COHEN_SUTHERLAND_LEFT    ( ( nsuint8 )0x0001 )
#define NS_COHEN_SUTHERLAND_RIGHT   ( ( nsuint8 )0x0002 )
#define NS_COHEN_SUTHERLAND_BOTTOM  ( ( nsuint8 )0x0004 )
#define NS_COHEN_SUTHERLAND_TOP     ( ( nsuint8 )0x0008 )
#define NS_COHEN_SUTHERLAND_NEAR    ( ( nsuint8 )0x0010 )
#define NS_COHEN_SUTHERLAND_FAR     ( ( nsuint8 )0x0020 )

NS_IMPEXP nsuint8 ns_aabbox3d_cohen_sutherland_code
	(
	const NsAABBox3d  *B,
	const NsPoint3d   *P
	);


NS_IMPEXP nsboolean ns_aabbox2d_intersects_aabbox
	(
	const NsAABBox2d  *A,
	const NsAABBox2d  *B
	);

NS_IMPEXP nsboolean ns_aabbox3d_intersects_aabbox
	(
	const NsAABBox3d  *A,
	const NsAABBox3d  *B
	);

NS_DECLS_END

#endif/* __NS_MATH_AABBOX_H__ */
