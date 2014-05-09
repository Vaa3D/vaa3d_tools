#ifndef __NS_IMAGE_COLOR_H__
#define __NS_IMAGE_COLOR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <math/nsvectordecls.h>

NS_DECLS_BEGIN

typedef NsVector3f   NsColor3f;
typedef NsVector4f   NsColor4f;
typedef NsVector3ub  NsColor3ub;
typedef NsVector4ub  NsColor4ub;


NS_IMPEXP NsColor4ub* ns_color4ub( NsColor4ub *C, nsuint8 x, nsuint8 y, nsuint8 z, nsuint8 w );

NS_IMPEXP NsColor4ub* ns_color4ub_invert( const NsColor4ub *src, NsColor4ub *dest );

/* Use to convert from RGB to BGR and vica-versa. */
NS_IMPEXP NsColor4ub* ns_color4ub_red_blue_swap( const NsColor4ub *src, NsColor4ub *dest );

/* C = A mixed with B */
NS_IMPEXP NsColor4ub* ns_color4ub_xor( const NsColor4ub *A, const NsColor4ub *B, NsColor4ub *C );

/* Pass positive percents to lighten, negative to darken. */
NS_IMPEXP NsColor4ub* ns_color4ub_lighten_or_darken( const NsColor4ub *src, NsColor4ub *dest, nsfloat percent );


typedef struct _NsColor1us2ub
   {
	nsushort  h;  /* 0 - 359 */
	nsuchar   s;
	nsuchar   v;
   }
   NsColor1us2ub;


NS_IMPEXP NsColor1us2ub* ns_color_rgb_to_hsv( const NsColor3ub *src, NsColor1us2ub *dest );
NS_IMPEXP NsColor3ub* ns_color_hsv_to_rgb( const NsColor1us2ub *src, NsColor3ub *dest );


#define NS_COLOR_GET_RED_U8( color )\
	( (color).x )

#define NS_COLOR_GET_GREEN_U8( color )\
	( (color).y )

#define NS_COLOR_GET_BLUE_U8( color )\
	( (color).z )

#define NS_COLOR_GET_ALPHA_U8( color )\
	( (color).w )


#define NS_COLOR_SET_RED_U8( color, red )\
	( (color).x = (red) )

#define NS_COLOR_SET_GREEN_U8( color, green )\
	( (color).y = (green) )

#define NS_COLOR_SET_BLUE_U8( color, blue )\
	( (color).z = (blue) )

#define NS_COLOR_SET_ALPHA_U8( color, alpha )\
	( (color).w = (alpha) )


/* NOTE: The returned float values are in the range [0,1]. */

#define NS_COLOR_GET_RED_F( color )\
	( ( nsfloat )NS_COLOR_GET_RED_U8( (color) ) * ( 1.0f / 255.0f ) )

#define NS_COLOR_GET_GREEN_F( color )\
	( ( nsfloat )NS_COLOR_GET_GREEN_U8( (color) ) * ( 1.0f / 255.0f ) )

#define NS_COLOR_GET_BLUE_F( color )\
	( ( nsfloat )NS_COLOR_GET_BLUE_U8( (color) ) * ( 1.0f / 255.0f ) )


NS_IMPEXP NsColor4ub NS_COLOR4UB( nsuint8 x, nsuint8 y, nsuint8 z, nsuint8 w );


#define NS_COLOR4UB_TO_UINT32( C )\
	( ( ( nsuint32 )NS_COLOR_GET_BLUE_U8( C )  << 16 ) |\
	  ( ( nsuint32 )NS_COLOR_GET_GREEN_U8( C ) <<  8 ) |\
	  ( ( nsuint32 )NS_COLOR_GET_RED_U8( C )         )  )


NS_IMPEXP NsColor4ub ns_color4ub_table( nssize index );

#define NS_COLOR4UB_BLACK_INDEX       0
#define NS_COLOR4UB_WHITE_INDEX       1
#define NS_COLOR4UB_RED_INDEX         2
#define NS_COLOR4UB_GREEN_INDEX       3
#define NS_COLOR4UB_BLUE_INDEX        4
#define NS_COLOR4UB_DARK_GREY_INDEX   5
#define NS_COLOR4UB_ORANGE_INDEX      6
#define NS_COLOR4UB_YELLOW_INDEX      7
#define NS_COLOR4UB_INDIGO_INDEX      8
#define NS_COLOR4UB_VIOLET_INDEX      9
#define NS_COLOR4UB_DARK_BLUE_INDEX   10
#define NS_COLOR4UB_DARK_GREEN_INDEX  11
#define NS_COLOR4UB_MID_GREY_INDEX    12
#define NS_COLOR4UB_CYAN_INDEX        13

#define NS_COLOR4UB_BLACK       ns_color4ub_table( NS_COLOR4UB_BLACK_INDEX )
#define NS_COLOR4UB_WHITE       ns_color4ub_table( NS_COLOR4UB_WHITE_INDEX )
#define NS_COLOR4UB_RED         ns_color4ub_table( NS_COLOR4UB_RED_INDEX )
#define NS_COLOR4UB_GREEN       ns_color4ub_table( NS_COLOR4UB_GREEN_INDEX )
#define NS_COLOR4UB_BLUE        ns_color4ub_table( NS_COLOR4UB_BLUE_INDEX )
#define NS_COLOR4UB_DARK_GREY   ns_color4ub_table( NS_COLOR4UB_DARK_GREY_INDEX )
#define NS_COLOR4UB_ORANGE      ns_color4ub_table( NS_COLOR4UB_ORANGE_INDEX )
#define NS_COLOR4UB_YELLOW      ns_color4ub_table( NS_COLOR4UB_YELLOW_INDEX )
#define NS_COLOR4UB_INDIGO      ns_color4ub_table( NS_COLOR4UB_INDIGO_INDEX )
#define NS_COLOR4UB_VIOLET      ns_color4ub_table( NS_COLOR4UB_VIOLET_INDEX )
#define NS_COLOR4UB_DARK_BLUE   ns_color4ub_table( NS_COLOR4UB_DARK_BLUE_INDEX )
#define NS_COLOR4UB_DARK_GREEN  ns_color4ub_table( NS_COLOR4UB_DARK_GREEN_INDEX )
#define NS_COLOR4UB_MID_GREY    ns_color4ub_table( NS_COLOR4UB_MID_GREY_INDEX )
#define NS_COLOR4UB_CYAN        ns_color4ub_table( NS_COLOR4UB_CYAN_INDEX )

#define NS_COLOR4UB_TABLE_SIZE  14

NS_DECLS_END

#endif/* __NS_IMAGE_COLOR_H__ */
