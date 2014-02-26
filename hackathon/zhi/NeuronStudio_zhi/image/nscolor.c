#include "nscolor.h"


NsColor4ub* ns_color4ub( NsColor4ub *C, nsuint8 x, nsuint8 y, nsuint8 z, nsuint8 w )
	{
	ns_assert( NULL != C );

	C->x = x;
	C->y = y;
	C->z = z;
	C->w = w;

	return C;
	}


NsColor4ub* ns_color4ub_invert( const NsColor4ub *src, NsColor4ub *dest )
	{
	ns_assert( NULL != src );
	ns_assert( NULL != dest );

	dest->x = ~src->x;
	dest->y = ~src->y;
	dest->z = ~src->z;

	/* NOTE: Dont invert the alpha channel. */
	dest->w = src->w;

	return dest;
	}


NsColor4ub* ns_color4ub_red_blue_swap( const NsColor4ub *src, NsColor4ub *dest )
	{
	ns_assert( NULL != src );
	ns_assert( NULL != dest );

	dest->x = src->z;
	dest->y = src->y;
	dest->z = src->x;
	dest->w = src->w;

	return dest;
	}


NsColor4ub* ns_color4ub_xor( const NsColor4ub *A, const NsColor4ub *B, NsColor4ub *C )
	{
	ns_assert( NULL != A );
	ns_assert( NULL != B );
	ns_assert( NULL != C );

	C->x = A->x ^ B->x;
	C->y = A->y ^ B->y;
	C->z = A->z ^ B->z;

	/* NOTE: Dont XOR the alpha channel. */
	C->w = NS_MAX( A->w, B->w );

	return C;
	}


NsColor4ub* ns_color4ub_lighten_or_darken( const NsColor4ub *src, NsColor4ub *dest, nsfloat percent )
	{
	NsColor3ub     rgb;
	NsColor1us2ub  hsv;
	nsfloat        v;


	ns_assert( NULL != src );
	ns_assert( NULL != dest );

	/* NOTE: The alpha channel is not affected. */

	rgb.x = src->x;
	rgb.y = src->y;
	rgb.z = src->z;

	ns_color_rgb_to_hsv( &rgb, &hsv );

	percent /= 100.0f;

	v = ( nsfloat )hsv.v + ( nsfloat )hsv.v * percent;
	hsv.v = ( nsuchar )NS_CLAMP( v, 0.0f, 255.0f );

	ns_color_hsv_to_rgb( &hsv, &rgb );

	dest->x = rgb.x;
	dest->y = rgb.y;
	dest->z = rgb.z;
	dest->w = src->w;

	return dest;
	}


NsColor1us2ub* ns_color_rgb_to_hsv( const NsColor3ub *src, NsColor1us2ub *dest )
   {
   nsdouble red, green, blue, h, s, v, min, max, delta;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );

   red   = src->x / 255.0;
   green = src->y / 255.0;
   blue  = src->z / 255.0;
  
   h = 0.0;

   /* Find min and max values. */

   if( red > green )
      {
      if( red > blue )
	     max = red;
      else
	     max = blue;
      
      if( green < blue )
	     min = green;
      else
	     min = blue;
      }
   else
      {
      if( green > blue )
	     max = green;
      else
	     max = blue;
      
      if( red < blue )
	     min = red;
      else
	     min = blue;
      }
  
   v = max;
  
   if( max != 0.0 )
      s = ( max - min ) / max;
   else
      s = 0.0;
  
   if( s == 0.0 )
      h = 0.0;
   else
      {
      delta = max - min;
      
      if( red == max )
	     h = ( green - blue ) / delta;
      else if( green == max )
	     h = 2 + (blue - red) / delta;
      else if( blue == max )
	     h = 4 + (red - green) / delta;
      
      h /= 6.0;
      
      if( h < 0.0 )
	     h += 1.0;
      else if( h > 1.0 )
	     h -= 1.0;
      }
 
   dest->h = ( nsushort )( h * 359.0 + .5 );
   dest->s = ( nsuchar )( s * 255.0 + .5 );
   dest->v = ( nsuchar )( v * 255.0 + .5 );

	return dest;
   }


NsColor3ub* ns_color_hsv_to_rgb( const NsColor1us2ub *src, NsColor3ub *dest )
   {
   nsdouble h, s, v, hue, saturation, value, f, p, q, t;

	ns_assert( NULL != src );
	ns_assert( NULL != dest );

   h = src->h / 359.0;
   s = src->s / 255.0;
   v = src->v / 255.0;
  
   if( s == 0.0 )
      {
      h = v;
      s = v;
      v = v;
      }
   else
      {
      hue        = h * 6.0;
      saturation = s;
      value      = v;
      
      if( hue == 6.0 )
	     hue = 0.0;
      
      f = hue - ( int )hue;
      p = value * ( 1.0 - saturation );
      q = value * ( 1.0 - saturation * f );
      t = value * ( 1.0 - saturation * ( 1.0 - f ) );
      
      switch( ( int )hue )
	     {
	     case 0:
	        h = value;
	        s = t;
	        v = p;
	     break;
	  
	     case 1:
	        h = q;
	        s = value;
	        v = p;
	     break;
	  
	     case 2:
	        h = p;
	        s = value;
	        v = t;
	     break;
	  
	     case 3:
	        h = p;
	        s = q;
	        v = value;
	     break;
	  
	     case 4:
	        h = t;
	        s = p;
	        v = value;
	     break;
	  
	     case 5:
	        h = value;
	        s = p;
	        v = q;
	     break;
  	     }
      }

   dest->x = ( nsuchar )( h * 255.0 + .5 );
   dest->y = ( nsuchar )( s * 255.0 + .5 );
   dest->z = ( nsuchar )( v * 255.0 + .5 );

	return dest;
   }


NsColor4ub NS_COLOR4UB( nsuint8 x, nsuint8 y, nsuint8 z, nsuint8 w )
	{
	NsColor4ub C;

	ns_color4ub( &C, x, y, z, w );
	return C;
	}


NsColor4ub ns_color4ub_table( nssize index )
	{
	NS_PRIVATE NsColor4ub ____ns_color4ub_table[ NS_COLOR4UB_TABLE_SIZE ] =
		{
		{   0,   0,   0, 255 },//NS_COLOR4UB_BLACK
		{ 255, 255, 255, 255 },//NS_COLOR4UB_WHITE
		{ 255,   0,   0, 255 },//NS_COLOR4UB_RED
		{   0, 255,   0, 255 },//NS_COLOR4UB_GREEN
		{   0,   0, 255, 255 },//NS_COLOR4UB_BLUE
		{  32,  32,  32, 255 },//NS_COLOR4UB_DARK_GREY
		{ 255, 165,   0, 255 },//NS_COLOR4UB_ORANGE
		{ 255, 255,   0, 255 },//NS_COLOR4UB_YELLOW
		{  75,   0, 130, 255 },//NS_COLOR4UB_INDIGO
		{ 238, 130, 238, 255 },//NS_COLOR4UB_VIOLET
		{   0,   0,  32, 255 },//NS_COLOR4UB_DARK_BLUE
		{   0,  32,   0, 255 },//NS_COLOR4UB_DARK_GREEN
		{ 120, 120, 120, 255 },//NS_COLOR4UB_MID_GREY
		{   0, 255, 255, 255 },//NS_COLOR4UB_CYAN
		};

	ns_assert( index < NS_COLOR4UB_TABLE_SIZE );
	return ____ns_color4ub_table[ index ];
	}
