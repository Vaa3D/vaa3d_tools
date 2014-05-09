#include "clrdlg.h"


#pragma pack( push, 1 )

typedef struct tagPixelRgb24
   {
   unsigned char  red;
   unsigned char  green;
   unsigned char  blue;
   }
   PixelRgb24;


typedef struct tagPixelBgr24
   {
   unsigned char  blue;
   unsigned char  green;
   unsigned char  red;
   }
   PixelBgr24;


typedef struct tagPixelHsv32
   {
   unsigned short  hue;        /* 0 - 359 */
   unsigned char   saturation;
   unsigned char   value;
   }
   PixelHsv32;

#pragma pack( pop )


extern char _compile_time_size_check[ ( 3 == sizeof( PixelRgb24 ) ) ? 1 : -1 ];
extern char _compile_time_size_check[ ( 3 == sizeof( PixelBgr24 ) ) ? 1 : -1 ];
extern char _compile_time_size_check[ ( 4 == sizeof( PixelHsv32 ) ) ? 1 : -1 ];



void PixelRgb24toHsv32( const PixelRgb24 *const src, PixelHsv32 *const dest )
   {
   double red, green, blue, h, s, v, min, max, delta;


   red   = src->red / 255.0;
   green = src->green / 255.0;
   blue  = src->blue / 255.0;
  
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
 
   dest->hue        = ( unsigned short )( h * 359.0 + .5 );
   dest->saturation = ( unsigned char )( s * 255.0 + .5 );
   dest->value      = ( unsigned char )( v * 255.0 + .5 );

   }/* PixelRgb24toHsv32() */


void PixelHsv32toRgb24( const PixelHsv32 *const src, PixelRgb24 *const dest )
   {
   double h, s, v, hue, saturation, value, f, p, q, t;


   h = src->hue / 359.0;
   s = src->saturation / 255.0;
   v = src->value / 255.0;
  
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

   dest->red   = ( unsigned char )( h * 255.0 + .5 );
   dest->green = ( unsigned char )( s * 255.0 + .5 );
   dest->blue  = ( unsigned char )( v * 255.0 + .5 );

   }/* PixelHsv32toRgb24() */


void PixelHsv32toBgr24( const PixelHsv32 *const src, PixelBgr24 *const dest )
   {
   double h, s, v, hue, saturation, value, f, p, q, t;


   h = src->hue / 359.0;
   s = src->saturation / 255.0;
   v = src->value / 255.0;
  
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

   dest->red   = ( unsigned char )( h * 255.0 + .5 );
   dest->green = ( unsigned char )( s * 255.0 + .5 );
   dest->blue  = ( unsigned char )( v * 255.0 + .5 );

   }/* PixelHsv32toBgr24() */



typedef struct _ChooseColorsStruct
   {
   HWND             elementColorDlgOwner;
   HWND             elementColorDlg;

   HDC              hueChannelMemDC;
   HBITMAP          hueChannelBitmap;
   HBITMAP          hueChannelOrigBitmap;

   PixelBgr24*      elementColorGridPixels;
   HDC              elementColorGridMemDC;
   HBITMAP          elementColorGridBitmap;
   HBITMAP          elementColorGridOrigBitmap;
   HPEN             elementColorGridPen;
   HPEN             elementColorGridOldPen;
   unsigned         elementColorGridBytesPerRow;
   unsigned         elementColorGridWidth;
   unsigned         elementColorGridHeight;
   float            elementColorGridSaturationStep;
   float            elementColorGridValueStep;

   unsigned         numElements;
   unsigned         selectedElement;

   const char     **elementNames;
   COLORREF        *elementColors;
   const COLORREF  *elementDefaultColors;

   PixelRgb24      *elementRgb;
   PixelRgb24      *elementPreviousRgb;
   PixelHsv32      *elementHsv;

   WNDPROC          channelControlOrigFunc;
   WNDPROC          gridControlOrigFunc;

   void             ( *elementColorsApplyFunc )( const unsigned, const COLORREF );

   int              elementColorDlgLockEdits;
   int              elementColorDlgActiveEdit;
   }
   ChooseColorsStruct;


#define COLOR_CHANNEL_SLIDER_OFFSET  2

#define vRECT_WIDTH( rect )   ( ( rect ).right - ( rect ).left )
#define vRECT_HEIGHT( rect )  ( ( rect ).bottom - ( rect ).top )


void _CenterDialogBox( ChooseColorsStruct *ccs, HWND hDlg, HWND hOwner )
   {
   RECT  rcParent;
   RECT  rcChild;
   int   midX;
   int   midY;


   GetWindowRect( hOwner, &rcParent );
   GetWindowRect( hDlg, &rcChild );

   midX = rcParent.left + ( rcParent.right - rcParent.left )/2 - 
                          ( rcChild.right - rcChild.left )/2;

   midY = rcParent.top  + ( rcParent.bottom - rcParent.top )/2 - 
                          ( rcChild.bottom - rcChild.top )/2;

   SetWindowPos( hDlg, NULL, midX, midY, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
   }


void _ColorHorizGradientRectangle
   ( 
   ChooseColorsStruct  *ccs,
   HDC                  hDC,
   const RECT *const    rect, 
   const COLORREF       color1,
   const COLORREF       color2
   )
   {
   TRIVERTEX      vertices[2];
   GRADIENT_RECT  gradient;
   HBRUSH         hBrush;
   RECT           rc;

      
   vertices[0].x     = rect->left + COLOR_CHANNEL_SLIDER_OFFSET;
   vertices[0].y     = rect->top;
   vertices[0].Red   = ( ( ( unsigned short )GetRValue( color1 ) ) << 8 );
   vertices[0].Green = ( ( ( unsigned short )GetGValue( color1 ) ) << 8 );
   vertices[0].Blue  = ( ( ( unsigned short )GetBValue( color1 ) ) << 8 );
   vertices[0].Alpha = 0;

   vertices[1].x     = rect->right - COLOR_CHANNEL_SLIDER_OFFSET;
   vertices[1].y     = rect->bottom;
   vertices[1].Red   = ( ( ( unsigned short )GetRValue( color2 ) ) << 8 );
   vertices[1].Green = ( ( ( unsigned short )GetGValue( color2 ) ) << 8 );
   vertices[1].Blue  = ( ( ( unsigned short )GetBValue( color2 ) ) << 8 );
   vertices[1].Alpha = 0;

   gradient.UpperLeft  = 0;
   gradient.LowerRight = 1;

   GradientFill( hDC, vertices, 2, &gradient, 1, GRADIENT_FILL_RECT_H );

   rc.top    = 0;
   rc.bottom = rect->bottom;

   hBrush = CreateSolidBrush( color1 );
   rc.left   = 0;
   rc.right  = COLOR_CHANNEL_SLIDER_OFFSET;
   FillRect( hDC, &rc, hBrush );
   DeleteObject( hBrush );

   hBrush = CreateSolidBrush( color2 );
   rc.left   = rect->right - COLOR_CHANNEL_SLIDER_OFFSET;
   rc.right  = rect->right;
   FillRect( hDC, &rc, hBrush );
   DeleteObject( hBrush );

   }/* _ColorHorizGradientRectangle() */


void _DrawChannelSliderControl
   (
   ChooseColorsStruct  *ccs,
   HDC                  hDC, 
   const RECT *const    rect,
   const unsigned       value,
   const unsigned       maxValue
   )
   {
   unsigned x = ( unsigned )
                ( ( ( float )( vRECT_WIDTH( *rect ) - COLOR_CHANNEL_SLIDER_OFFSET*2 - 1 ) / maxValue ) * value + .5 );

   Rectangle( hDC,
              x,
              0,
              x + COLOR_CHANNEL_SLIDER_OFFSET*2 + 1,
              vRECT_HEIGHT( *rect )
            );
   }


void _SetColorsChannelText( ChooseColorsStruct *ccs, const int ID, int value )
   {
   char text[ 32 ];

   sprintf( text, "%d", value );

   ccs->elementColorDlgLockEdits = 1;

   if( ID != ccs->elementColorDlgActiveEdit )
      SetDlgItemText( ccs->elementColorDlg, ID, text );

   ccs->elementColorDlgLockEdits = 0;

   }/* _SetColorsChannelText() */


void _DrawElementRedChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   _ColorHorizGradientRectangle
      (
      ccs,
      hDC,
      rect,
      RGB( 0, ccs->elementRgb[ ccs->selectedElement ].green, ccs->elementRgb[ ccs->selectedElement ].blue ),
      RGB( 255, ccs->elementRgb[ ccs->selectedElement ].green, ccs->elementRgb[ ccs->selectedElement ].blue ) 
      );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementRgb[ ccs->selectedElement ].red, 255 );
   _SetColorsChannelText( ccs, IDE_COLORS_RED_CHANNEL, ccs->elementRgb[ ccs->selectedElement ].red );
   }


void _DrawElementGreenChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   _ColorHorizGradientRectangle
      (
      ccs, 
      hDC,
      rect,
      RGB( ccs->elementRgb[ ccs->selectedElement ].red, 0, ccs->elementRgb[ ccs->selectedElement ].blue ),
      RGB( ccs->elementRgb[ ccs->selectedElement ].red, 255, ccs->elementRgb[ ccs->selectedElement ].blue )
      );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementRgb[ ccs->selectedElement ].green, 255 );
   _SetColorsChannelText( ccs, IDE_COLORS_GREEN_CHANNEL, ccs->elementRgb[ ccs->selectedElement ].green );
   }


void _DrawElementBlueChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   _ColorHorizGradientRectangle
      (
      ccs, 
      hDC,
      rect,
      RGB( ccs->elementRgb[ ccs->selectedElement ].red, ccs->elementRgb[ ccs->selectedElement ].green, 0 ),
      RGB( ccs->elementRgb[ ccs->selectedElement ].red, ccs->elementRgb[ ccs->selectedElement ].green, 255 ) 
      );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementRgb[ ccs->selectedElement ].blue, 255 );
   _SetColorsChannelText( ccs, IDE_COLORS_BLUE_CHANNEL, ccs->elementRgb[ ccs->selectedElement ].blue );
   }


void _CreateHueChannelMemDC( ChooseColorsStruct *ccs, HWND hDlg )
   {
   HWND          hWnd;
   HDC           hDC;
   RECT          rc;
   unsigned      width;
   unsigned      height;
   PixelRgb24    rgb24;
   PixelHsv32    hsv32;
   float         xf;
   float         xfstep;
   unsigned      xi;
   unsigned      yi;
   COLORREF      color;
   HBRUSH        hBrush;


   hWnd = GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL );
         
   GetClientRect( hWnd, &rc );
   width  = vRECT_WIDTH( rc );
   height = vRECT_HEIGHT( rc );
         
   hDC = GetDC( hWnd );
 
   ccs->hueChannelMemDC  = CreateCompatibleDC( hDC );
   ccs->hueChannelBitmap = CreateCompatibleBitmap( hDC, width, height );

   ReleaseDC( hWnd, hDC );

   ccs->hueChannelOrigBitmap = SelectObject( ccs->hueChannelMemDC, ccs->hueChannelBitmap );
 
   hBrush = CreateSolidBrush( 0x0000FF );
   FillRect( ccs->hueChannelMemDC, &rc, hBrush );
   DeleteObject( hBrush );

   hsv32.saturation = 255;
   hsv32.value      = 255;

   xf     = COLOR_CHANNEL_SLIDER_OFFSET;
   xfstep = ( width - COLOR_CHANNEL_SLIDER_OFFSET * 2 ) / 359.0f;

   for( hsv32.hue = 0; hsv32.hue < 360; ++hsv32.hue )
      {
      PixelHsv32toRgb24( &hsv32, &rgb24 );

      xi    = ( unsigned )xf;
      color = RGB( rgb24.red, rgb24.green, rgb24.blue );

      for( yi = 0; yi < height; ++yi )
         SetPixel( ccs->hueChannelMemDC, xi, yi, color );

      xf += xfstep;
      }

   }/* _CreateHueChannelMemDC() */


void _DeleteHueChannelMemDC( ChooseColorsStruct *ccs )
   {
   SelectObject( ccs->hueChannelMemDC, ccs->hueChannelOrigBitmap );

   DeleteObject( ccs->hueChannelBitmap );
   DeleteDC( ccs->hueChannelMemDC );

   ccs->hueChannelMemDC      = NULL;
   ccs->hueChannelBitmap     = NULL;
   ccs->hueChannelOrigBitmap = NULL;
   }


void _UpdateElementColorGrid( ChooseColorsStruct *ccs )
   {
   unsigned       x;
   unsigned       y;
   PixelBgr24    *rowStart;
   PixelBgr24    *currentPixel;
   PixelHsv32     hsv32;

    
   if( NULL != ccs->elementColorGridPixels )
      {
      hsv32.hue   = ccs->elementHsv[ ccs->selectedElement ].hue;
      hsv32.value = 255;

      rowStart = ccs->elementColorGridPixels;

      for( y = 0; y < ccs->elementColorGridHeight; ++y )
         {
         currentPixel     = rowStart;
         hsv32.saturation = 0;

         for( x = 0; x < ccs->elementColorGridWidth; ++x )
            {
            PixelHsv32toBgr24( &hsv32, currentPixel );

            hsv32.saturation = ( unsigned char )( ( float )hsv32.saturation + ccs->elementColorGridSaturationStep );

            ++currentPixel;
            }

         rowStart = ( PixelBgr24* )( ( ( unsigned char* )rowStart ) + ccs->elementColorGridBytesPerRow );

         hsv32.value = ( unsigned char )( hsv32.value - ccs->elementColorGridValueStep );
         } 
      }

   }/* _UpdateElementColorGrid() */


void _DrawElementColorGrid( ChooseColorsStruct *ccs, HWND hDlg )
   {
   HWND        hWnd;
   HDC         hDC;
   BITMAPINFO  info;
   int         x;
   int         y;
   int         oldROP;


   memset( &info, 0, sizeof( BITMAPINFO ) );

   /* NOTE: -height for top-down images. */

   info.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
   info.bmiHeader.biWidth       = ccs->elementColorGridWidth;
   info.bmiHeader.biHeight      = -( ( int )ccs->elementColorGridHeight );
   info.bmiHeader.biPlanes      = 1;
   info.bmiHeader.biBitCount    = 24;
   info.bmiHeader.biCompression = BI_RGB;

   SetDIBits( ccs->elementColorGridMemDC,
              ccs->elementColorGridBitmap,
              0,
              ccs->elementColorGridHeight,
              ccs->elementColorGridPixels,
              &info,
              DIB_RGB_COLORS
            );


   hWnd = GetDlgItem( hDlg, IDS_COLORS_GRID );

   x = ( int )( ccs->elementHsv[ ccs->selectedElement ].saturation / ccs->elementColorGridSaturationStep );
   y = 255 - ( int )( ccs->elementHsv[ ccs->selectedElement ].value / ccs->elementColorGridValueStep );

   oldROP = SetROP2( ccs->elementColorGridMemDC, R2_XORPEN );

   Arc( ccs->elementColorGridMemDC,
        x - 8,
        y - 8,
        x + 8 + 1,
        y + 8 + 1,
        x - 8,
        y,
        x - 8,
        y
      );

   SetROP2( ccs->elementColorGridMemDC, oldROP );

   hDC = GetDC( hWnd );
   BitBlt( hDC, 0, 0, ccs->elementColorGridWidth, ccs->elementColorGridHeight, ccs->elementColorGridMemDC, 0, 0, SRCCOPY );
   ReleaseDC( hWnd, hDC );

   }/* _DrawElementColorGrid() */


void _CreateElementColorGrid( ChooseColorsStruct *ccs, HWND hDlg )
   {
   HWND  hWnd;
   HDC   hDC;
   RECT  rc;


#define ALIGN_TO_LONG_BOUNDARY( bytes )\
   ( ( bytes + ( sizeof( LONG ) - 1 ) ) & ( ~( sizeof( LONG ) - 1 ) ) )


   hWnd = GetDlgItem( hDlg, IDS_COLORS_GRID );
         
   GetClientRect( hWnd, &rc );

   ccs->elementColorGridWidth  = vRECT_WIDTH( rc );
   ccs->elementColorGridHeight = vRECT_HEIGHT( rc );

   ccs->elementColorGridSaturationStep = 255.0f / ( ccs->elementColorGridWidth - 1 );
   ccs->elementColorGridValueStep      = 255.0f / ( ccs->elementColorGridHeight - 1 );

   ccs->elementColorGridBytesPerRow = ALIGN_TO_LONG_BOUNDARY( ccs->elementColorGridWidth * sizeof( PixelBgr24 ) );

   ccs->elementColorGridPixels = malloc( ccs->elementColorGridBytesPerRow * ccs->elementColorGridHeight );
         
   hDC = GetDC( hWnd );
 
   ccs->elementColorGridMemDC  = CreateCompatibleDC( hDC );
   ccs->elementColorGridBitmap = CreateCompatibleBitmap( hDC, ccs->elementColorGridWidth, ccs->elementColorGridHeight );

   ReleaseDC( hWnd, hDC );

   ccs->elementColorGridOrigBitmap = SelectObject( ccs->elementColorGridMemDC, ccs->elementColorGridBitmap );

   ccs->elementColorGridPen    = CreatePen( PS_SOLID, 1, 0xCCCCCC );
   ccs->elementColorGridOldPen = SelectObject( ccs->elementColorGridMemDC, ccs->elementColorGridPen );

   _UpdateElementColorGrid( ccs );

   }/* _CreateElementColorGrid() */


void _DeleteElementColorGrid( ChooseColorsStruct *ccs )
   {
   if( NULL != ccs->elementColorGridPixels )
      free( ccs->elementColorGridPixels );

   SelectObject( ccs->elementColorGridMemDC, ccs->elementColorGridOrigBitmap );
   DeleteObject( ccs->elementColorGridBitmap );

   SelectObject( ccs->elementColorGridMemDC, ccs->elementColorGridOldPen );
   DeleteObject( ccs->elementColorGridPen );

   DeleteDC( ccs->elementColorGridMemDC );

   ccs->elementColorGridPixels     = NULL;
   ccs->elementColorGridMemDC      = NULL;
   ccs->elementColorGridBitmap     = NULL;
   ccs->elementColorGridOrigBitmap = NULL;
   ccs->elementColorGridPen        = NULL;
   ccs->elementColorGridOldPen     = NULL;

   }/* _DeleteElementColorGrid() */


void _DrawElementHueChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   BitBlt( hDC, 0, 0, vRECT_WIDTH( *rect ), vRECT_HEIGHT( *rect ), ccs->hueChannelMemDC, 0, 0, SRCCOPY );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementHsv[ ccs->selectedElement ].hue, 359 );
   _SetColorsChannelText( ccs, IDE_COLORS_HUE_CHANNEL, ccs->elementHsv[ ccs->selectedElement ].hue );
   }


void _DrawElementSaturationChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   PixelRgb24  rgb24fullSat;
   PixelRgb24  rgb24noSat;
   PixelHsv32  hsv32fullSat;
   PixelHsv32  hsv32noSat;


   hsv32fullSat = hsv32noSat = ccs->elementHsv[ ccs->selectedElement ];

   hsv32fullSat.saturation = 255;
   hsv32noSat.saturation   = 0;

   PixelHsv32toRgb24( &hsv32fullSat, &rgb24fullSat );
   PixelHsv32toRgb24( &hsv32noSat, &rgb24noSat );

   _ColorHorizGradientRectangle( ccs,
                                 hDC,
                                 rect,
                                 RGB( rgb24noSat.red, rgb24noSat.green, rgb24noSat.blue ),
                                 RGB( rgb24fullSat.red, rgb24fullSat.green, rgb24fullSat.blue )
                               );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementHsv[ ccs->selectedElement ].saturation, 255 );
   _SetColorsChannelText( ccs, IDE_COLORS_SATURATION_CHANNEL, ccs->elementHsv[ ccs->selectedElement ].saturation );
   }


void _DrawElementValueChannel( ChooseColorsStruct *ccs, HDC hDC, const RECT *const rect )
   {
   PixelRgb24  rgb24fullValue;
   PixelRgb24  rgb24noValue;
   PixelHsv32  hsv32fullValue;
   PixelHsv32  hsv32noValue;


   hsv32fullValue = hsv32noValue = ccs->elementHsv[ ccs->selectedElement ];

   hsv32fullValue.value = 255;
   hsv32noValue.value   = 0;

   PixelHsv32toRgb24( &hsv32fullValue, &rgb24fullValue );
   PixelHsv32toRgb24( &hsv32noValue, &rgb24noValue );

   _ColorHorizGradientRectangle( ccs,
                                 hDC,
                                 rect,
                                 RGB( rgb24noValue.red, rgb24noValue.green, rgb24noValue.blue ),
                                 RGB( rgb24fullValue.red, rgb24fullValue.green, rgb24fullValue.blue )
                               );

   _DrawChannelSliderControl( ccs, hDC, rect, ccs->elementHsv[ ccs->selectedElement ].value, 255 );
   _SetColorsChannelText( ccs, IDE_COLORS_VALUE_CHANNEL, ccs->elementHsv[ ccs->selectedElement ].value );
   }


void _DrawElementColorChannels( ChooseColorsStruct *ccs, HWND hDlg )
   {
   HWND  hWnd;
   HDC   hDC;
   RECT  rc;

   hWnd = GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementRedChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );
   
   hWnd = GetDlgItem( hDlg, IDS_COLORS_GREEN_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementGreenChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );

   hWnd = GetDlgItem( hDlg, IDS_COLORS_BLUE_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementBlueChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );

   hWnd = GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementHueChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );

   hWnd = GetDlgItem( hDlg, IDS_COLORS_SATURATION_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementSaturationChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );

   hWnd = GetDlgItem( hDlg, IDS_COLORS_VALUE_CHANNEL );
   GetClientRect( hWnd, &rc );
   hDC = GetDC( hWnd );
   _DrawElementValueChannel( ccs, hDC, &rc );
   ReleaseDC( hWnd, hDC );

   }/* _DrawElementColorChannels() */


void _DrawElementColorBox( ChooseColorsStruct *ccs, HWND hDlg )
   {
   HWND    hBox;
   HDC     hDC;
   RECT    rc;
   HBRUSH  hCurrColorBrush;
   HBRUSH  hPrevColorBrush;
   HBRUSH  hDefColorBrush;
   long    bottom;
   long    height;


   hBox = GetDlgItem( hDlg, IDS_COLORS_COLORBOX );

   GetClientRect( hBox, &rc );
   bottom = rc.bottom;

   hCurrColorBrush = 
     CreateSolidBrush( RGB( ccs->elementRgb[ ccs->selectedElement ].red,
                            ccs->elementRgb[ ccs->selectedElement ].green,
                            ccs->elementRgb[ ccs->selectedElement ].blue 
                          )
                     );

   hPrevColorBrush =
     CreateSolidBrush( RGB( ccs->elementPreviousRgb[ ccs->selectedElement ].red,
                            ccs->elementPreviousRgb[ ccs->selectedElement ].green,
                            ccs->elementPreviousRgb[ ccs->selectedElement ].blue
                          )
                     );

   hDefColorBrush = CreateSolidBrush( ccs->elementDefaultColors[ ccs->selectedElement ] );

   hDC = GetDC( hBox );

   height = vRECT_HEIGHT( rc );

   rc.bottom = height / 3;
//   rc.bottom = bottom / 2;
   FillRect( hDC, &rc, hPrevColorBrush );

//   rc.top    = bottom / 2;
//   rc.bottom = bottom;
   rc.top = height / 3;
   rc.bottom = height * 2 / 3;
   FillRect( hDC, &rc, hCurrColorBrush );

   rc.top = height * 2 / 3;
   rc.bottom = height;
   FillRect( hDC, &rc, hDefColorBrush );

   ReleaseDC( hBox, hDC );

   DeleteObject( hCurrColorBrush );
   DeleteObject( hPrevColorBrush );
   DeleteObject( hDefColorBrush );

   }/* _DrawElementColorBox() */


void _UpdateElementColor( ChooseColorsStruct *ccs, int value, int ID )
   {
   unsigned oldHue = ccs->elementHsv[ ccs->selectedElement ].hue;

   switch( ID )
      {
      case IDS_COLORS_RED_CHANNEL:
      case IDSP_COLORS_RED_CHANNEL:
      case IDE_COLORS_RED_CHANNEL:
         ccs->elementRgb[ ccs->selectedElement ].red = value;
         PixelRgb24toHsv32( &ccs->elementRgb[ ccs->selectedElement ], &ccs->elementHsv[ ccs->selectedElement ] );
         break;

      case IDS_COLORS_GREEN_CHANNEL:
      case IDSP_COLORS_GREEN_CHANNEL:
      case IDE_COLORS_GREEN_CHANNEL:
         ccs->elementRgb[ ccs->selectedElement ].green = value;
         PixelRgb24toHsv32( &ccs->elementRgb[ ccs->selectedElement ], &ccs->elementHsv[ ccs->selectedElement ] );
         break;

      case IDS_COLORS_BLUE_CHANNEL:
      case IDSP_COLORS_BLUE_CHANNEL:
      case IDE_COLORS_BLUE_CHANNEL:
         ccs->elementRgb[ ccs->selectedElement ].blue = value;
         PixelRgb24toHsv32( &ccs->elementRgb[ ccs->selectedElement ], &ccs->elementHsv[ ccs->selectedElement ] );
         break;

      case IDS_COLORS_HUE_CHANNEL:
      case IDSP_COLORS_HUE_CHANNEL:
      case IDE_COLORS_HUE_CHANNEL:
         ccs->elementHsv[ ccs->selectedElement ].hue = value;
         PixelHsv32toRgb24( &ccs->elementHsv[ ccs->selectedElement ], &ccs->elementRgb[ ccs->selectedElement ] );
         break;

      case IDS_COLORS_SATURATION_CHANNEL:
      case IDSP_COLORS_SATURATION_CHANNEL:
      case IDE_COLORS_SATURATION_CHANNEL:
         ccs->elementHsv[ ccs->selectedElement ].saturation = value;
         PixelHsv32toRgb24( &ccs->elementHsv[ ccs->selectedElement ], &ccs->elementRgb[ ccs->selectedElement ] );
         break;

      case IDS_COLORS_VALUE_CHANNEL:
      case IDSP_COLORS_VALUE_CHANNEL:
      case IDE_COLORS_VALUE_CHANNEL:
         ccs->elementHsv[ ccs->selectedElement ].value = value;
         PixelHsv32toRgb24( &ccs->elementHsv[ ccs->selectedElement ], &ccs->elementRgb[ ccs->selectedElement ] );
         break;
      }

   /* Only if the hue changes does the color grid need to be udpated. */

   if( oldHue != ccs->elementHsv[ ccs->selectedElement ].hue )
      _UpdateElementColorGrid( ccs );

   }/* _UpdateElementColor() */


void _OnChannelControlMouseEvent( ChooseColorsStruct *ccs, HWND hWnd, int x )
   {
   RECT      rc;
   unsigned  width;
   unsigned  value;
   unsigned  maxValue;
   int       ID;


   GetClientRect( hWnd, &rc );

   width = vRECT_WIDTH( rc );

   if( x < 0 )
      x = 0;
   else if( x > ( int )( width - COLOR_CHANNEL_SLIDER_OFFSET*2 - 1 ) )
      x = width - COLOR_CHANNEL_SLIDER_OFFSET*2 - 1;

   ID = GetWindowLong( hWnd, GWL_ID );

   maxValue = ( IDS_COLORS_HUE_CHANNEL != ID ) ? 255 : 359;

   value = ( unsigned )
           ( ( ( double )maxValue / ( width - COLOR_CHANNEL_SLIDER_OFFSET*2 - 1 ) ) * x + .5 );

   _UpdateElementColor( ccs, value, ID );

   _DrawElementColorBox( ccs, ccs->elementColorDlg );
   _DrawElementColorChannels( ccs, ccs->elementColorDlg );
   _DrawElementColorGrid( ccs, ccs->elementColorDlg );

   }/* _OnChannelControlMouseEvent() */


LRESULT CALLBACK _ChannelControlSubclassFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   ChooseColorsStruct *ccs = ( ChooseColorsStruct* )GetWindowLongPtr( GetParent( hWnd ), GWLP_USERDATA );
 
   switch( uMsg )
      {
      case WM_MOUSEMOVE:
         if( 1 == GetWindowLongPtr( hWnd, GWLP_USERDATA ) )/* i.e. has capture. */
            _OnChannelControlMouseEvent( ccs, hWnd, ( int )( short )LOWORD( lParam ) );
         break;

      case WM_LBUTTONDOWN:
         SetCapture( hWnd );
         SetWindowLongPtr( hWnd, GWLP_USERDATA, 1 );
         _OnChannelControlMouseEvent( ccs, hWnd, ( int )( short )LOWORD( lParam ) );
         break;

      case WM_LBUTTONUP:
         ReleaseCapture();
         SetWindowLongPtr( hWnd, GWLP_USERDATA, 0 );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;

   }/* _ChannelControlSubclassFunc() */


void _OnGridControlMouseEvent( ChooseColorsStruct *ccs, HWND hWnd, int x, int y )
   {
   if( x < 0 )
      x = 0;
   else if( x > ( int )( ccs->elementColorGridWidth - 1 ) )
      x = ( int )( ccs->elementColorGridWidth - 1 );

   if( y < 0 )
      y = 0;
   else if( y > ( int )( ccs->elementColorGridHeight - 1 ) )
      y = ( int )( ccs->elementColorGridHeight - 1 );

   ccs->elementHsv[ ccs->selectedElement ].saturation = ( int )( x * ccs->elementColorGridSaturationStep );
   ccs->elementHsv[ ccs->selectedElement ].value      = 255 - ( int )( y * ccs->elementColorGridValueStep );

   PixelHsv32toRgb24( &ccs->elementHsv[ ccs->selectedElement ], &ccs->elementRgb[ ccs->selectedElement ] );

   _DrawElementColorBox( ccs, ccs->elementColorDlg );
   _DrawElementColorChannels( ccs, ccs->elementColorDlg );
   _DrawElementColorGrid( ccs, ccs->elementColorDlg );

   }/* _OnChannelControlMouseEvent() */


LRESULT CALLBACK _GridControlSubclassFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   ChooseColorsStruct *ccs = ( ChooseColorsStruct* )GetWindowLongPtr( GetParent( hWnd ), GWLP_USERDATA );

   switch( uMsg )
      {
      case WM_MOUSEMOVE:
         if( 1 == GetWindowLongPtr( hWnd, GWLP_USERDATA ) )/* i.e. has capture. */
            _OnGridControlMouseEvent( ccs,
                                      hWnd,
                                      ( int )( short )LOWORD( lParam ),
                                      ( int )( short )HIWORD( lParam )
                                    );
         break;

      case WM_LBUTTONDOWN:
         SetCapture( hWnd );
         SetWindowLongPtr( hWnd, GWLP_USERDATA, 1 );
         _OnGridControlMouseEvent( ccs,
                                   hWnd,
                                   ( int )( short )LOWORD( lParam ),
                                   ( int )( short )HIWORD( lParam )
                                 );
         break;

      case WM_LBUTTONUP:
         ReleaseCapture();
         SetWindowLongPtr( hWnd, GWLP_USERDATA, 0 );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;

   }/* _GridControlSubclassFunc() */


void _ColorsDlgProcOnResetDefaults( ChooseColorsStruct *ccs, HWND hDlg )
   {
/*
   unsigned i;

   for( i = 0; i < ccs->numElements; ++i )
      {
      ccs->elementRgb[ i ].red   = GetRValue( ccs->elementDefaultColors[ i ] );
      ccs->elementRgb[ i ].green = GetGValue( ccs->elementDefaultColors[ i ] );
      ccs->elementRgb[ i ].blue  = GetBValue( ccs->elementDefaultColors[ i ] );

      PixelRgb24toHsv32( &ccs->elementRgb[ i ], &ccs->elementHsv[ i ] );
      }
*/

   ccs->elementRgb[ ccs->selectedElement ].red   = GetRValue( ccs->elementDefaultColors[ ccs->selectedElement ] );
   ccs->elementRgb[ ccs->selectedElement ].green = GetGValue( ccs->elementDefaultColors[ ccs->selectedElement ] );
   ccs->elementRgb[ ccs->selectedElement ].blue  = GetBValue( ccs->elementDefaultColors[ ccs->selectedElement ] );

   PixelRgb24toHsv32( &ccs->elementRgb[ ccs->selectedElement ], &ccs->elementHsv[ ccs->selectedElement ] );

   _DrawElementColorBox( ccs, hDlg );
   _DrawElementColorChannels( ccs, hDlg );
   _UpdateElementColorGrid( ccs );
   _DrawElementColorGrid( ccs, hDlg );

   }/* _ColorsDlgProcOnResetDefaults() */


void _CleanupElementColorsDlgProc( ChooseColorsStruct *ccs, HWND hDlg )
   {
   _DeleteHueChannelMemDC( ccs );
   _DeleteElementColorGrid( ccs );

   /* Reset them! */

   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );
   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GREEN_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );
   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_BLUE_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );
   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );
   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_SATURATION_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );
   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_VALUE_CHANNEL ), GWLP_WNDPROC, ( LONG )ccs->channelControlOrigFunc );

   SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GRID ), GWLP_WNDPROC, ( LONG )ccs->gridControlOrigFunc );

   }/* _CleanupElementColorsDlgProc() */



RECT ____choose_element_colors_grid_rect;
int ____choose_element_colors_grid_is_shrunken;


void _toggle_choose_element_colors_grid( HWND hDlg )
	{
	if( ____choose_element_colors_grid_is_shrunken )
		{
		SetDlgItemText( hDlg, IDB_COLORS_GRID_TOGGLE, "<<< Grid" );

		SetWindowPos(
			hDlg,
			NULL,
			0,
			0,
			____choose_element_colors_grid_rect.right -
			____choose_element_colors_grid_rect.left,
			____choose_element_colors_grid_rect.bottom -
			____choose_element_colors_grid_rect.top,
			SWP_NOZORDER | SWP_NOMOVE
			);
		}
	else
		{
		SetDlgItemText( hDlg, IDB_COLORS_GRID_TOGGLE, "Grid >>>" );

		SetWindowPos(
			hDlg,
			NULL,
			0,
			0,
			____choose_element_colors_grid_rect.right -
			____choose_element_colors_grid_rect.left - 280,
			____choose_element_colors_grid_rect.bottom -
			____choose_element_colors_grid_rect.top,
			SWP_NOZORDER | SWP_NOMOVE
			);
		}

	____choose_element_colors_grid_is_shrunken = ! ____choose_element_colors_grid_is_shrunken;
	}


INT_PTR CALLBACK _ChooseElementColorsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   ChooseColorsStruct *ccs = ( ChooseColorsStruct* )GetWindowLongPtr( hDlg, GWLP_USERDATA );

   switch( uMsg )
      {
      case WM_INITDIALOG:
         {
         unsigned i;

			GetWindowRect( hDlg, &____choose_element_colors_grid_rect );

			____choose_element_colors_grid_is_shrunken = 0;
			_toggle_choose_element_colors_grid( hDlg );

         ccs = ( ChooseColorsStruct* )lParam;
         SetWindowLongPtr( hDlg, GWLP_USERDATA, ( LPARAM )ccs );

         ccs->elementColorDlg = hDlg;

         ccs->elementColorDlgLockEdits  = 0;
         ccs->elementColorDlgActiveEdit = 0;

         _CenterDialogBox( ccs, hDlg, ccs->elementColorDlgOwner );

         for( i = 0; i < ccs->numElements; ++i )
            SendMessage( GetDlgItem( hDlg, IDC_COLOR_ELEMENTS ),
                         CB_ADDSTRING, 
                         0,
                        ( LPARAM )ccs->elementNames[ i ]
                       );

         ccs->selectedElement = 0;
         SendMessage( GetDlgItem( hDlg, IDC_COLOR_ELEMENTS ), CB_SETCURSEL, ccs->selectedElement, 0 );

         _CreateHueChannelMemDC( ccs, hDlg );
         _CreateElementColorGrid( ccs, hDlg );

         /* Can use any of them to subclass the windows since theyre all of the "Static" class. */

         ccs->channelControlOrigFunc = ( WNDPROC )
                                        GetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL ), GWLP_WNDPROC );
  
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL ), GWLP_ID, IDS_COLORS_RED_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_RED_CHANNEL ), GWLP_USERDATA, 0 );

         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GREEN_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GREEN_CHANNEL ), GWLP_ID, IDS_COLORS_GREEN_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GREEN_CHANNEL ), GWLP_USERDATA, 0 );

         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_BLUE_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_BLUE_CHANNEL ), GWLP_ID, IDS_COLORS_BLUE_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_BLUE_CHANNEL ), GWLP_USERDATA, 0 );

         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL ), GWLP_ID, IDS_COLORS_HUE_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_HUE_CHANNEL ), GWLP_USERDATA, 0 );

         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_SATURATION_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_SATURATION_CHANNEL ), GWLP_ID, IDS_COLORS_SATURATION_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_SATURATION_CHANNEL ), GWLP_USERDATA, 0 );

         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_VALUE_CHANNEL ), GWLP_WNDPROC, ( LONG )_ChannelControlSubclassFunc );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_VALUE_CHANNEL ), GWLP_ID, IDS_COLORS_VALUE_CHANNEL );
         SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_VALUE_CHANNEL ), GWLP_USERDATA, 0 );

         ccs->gridControlOrigFunc = ( WNDPROC )SetWindowLongPtr( GetDlgItem( hDlg, IDS_COLORS_GRID ), 
                                                              GWLP_WNDPROC,
                                                              ( LONG )_GridControlSubclassFunc
                                                            );

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_RED_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_RED_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_RED_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 255, 0 ) );

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_GREEN_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_GREEN_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_GREEN_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 255, 0 ) ); 

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_BLUE_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_BLUE_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_BLUE_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 255, 0 ) );

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_HUE_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_HUE_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_HUE_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 359, 0 ) );

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_SATURATION_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_SATURATION_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_SATURATION_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 255, 0 ) ); 

         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_VALUE_CHANNEL ), UDM_SETBUDDY,
                      ( WPARAM )GetDlgItem( hDlg, IDE_COLORS_VALUE_CHANNEL ), 0 );
         SendMessage( GetDlgItem( hDlg, IDSP_COLORS_VALUE_CHANNEL ), UDM_SETRANGE,
                      0, ( LPARAM )MAKELONG( 255, 0 ) );

         SendMessage( GetDlgItem( hDlg, IDE_COLORS_RED_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_COLORS_GREEN_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_COLORS_BLUE_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_COLORS_HUE_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_COLORS_SATURATION_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_COLORS_VALUE_CHANNEL ), EM_LIMITTEXT, 3, 0 );
         }
         return FALSE;

      case WM_NOTIFY:
         switch( wParam )
            {
            case IDSP_COLORS_RED_CHANNEL:
            case IDSP_COLORS_GREEN_CHANNEL:
            case IDSP_COLORS_BLUE_CHANNEL:
            case IDSP_COLORS_HUE_CHANNEL:
            case IDSP_COLORS_SATURATION_CHANNEL:
            case IDSP_COLORS_VALUE_CHANNEL:
               if( UDN_DELTAPOS == ( ( LPNMHDR )lParam )->code )
                  {
                  int pos;
                  int max;

                  pos = ( ( LPNMUPDOWN )lParam )->iPos + ( ( LPNMUPDOWN )lParam )->iDelta;
                  max = ( IDSP_COLORS_HUE_CHANNEL == wParam ) ? 359 : 255;

                  if( pos < 0 )
                     pos = 0;
                  else if( max < pos )
                     pos = max;
                   
                  _UpdateElementColor( ccs, pos, ( int )wParam );
                  _DrawElementColorBox( ccs, hDlg );
                  _DrawElementColorChannels( ccs, hDlg );
                  _DrawElementColorGrid( ccs, hDlg );
                  return TRUE;
                  }
               break;
            }
         return FALSE;

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            case IDC_COLOR_ELEMENTS:
               if( CBN_SELCHANGE == HIWORD( wParam ) )
                  {
                  ccs->selectedElement = ( unsigned int )SendMessage( GetDlgItem( hDlg, IDC_COLOR_ELEMENTS ), 
                                                      CB_GETCURSEL, 0, 0 );
                  _DrawElementColorBox( ccs, hDlg );
                  _DrawElementColorChannels( ccs, hDlg );
                  _UpdateElementColorGrid( ccs );
                  _DrawElementColorGrid( ccs, hDlg );
                  }
               break;

            case IDB_APPLY_COLORS:
               if( NULL != ccs->elementColorsApplyFunc )
                  ( ccs->elementColorsApplyFunc )( ccs->selectedElement,
                                                   RGB( ccs->elementRgb[ ccs->selectedElement ].red,
                                                        ccs->elementRgb[ ccs->selectedElement ].green,
                                                        ccs->elementRgb[ ccs->selectedElement ].blue 
                                                       )
                                                 );
               break;

            case IDE_COLORS_RED_CHANNEL:
            case IDE_COLORS_GREEN_CHANNEL:
            case IDE_COLORS_BLUE_CHANNEL:
            case IDE_COLORS_HUE_CHANNEL:
            case IDE_COLORS_SATURATION_CHANNEL:
            case IDE_COLORS_VALUE_CHANNEL:
               if( EN_CHANGE == HIWORD( wParam ) && ! ccs->elementColorDlgLockEdits )
                  {
                  int   ID;
                  int   value;
                  int   max;
                  char  buffer[32];


                  ID = LOWORD( wParam );

                  ccs->elementColorDlgActiveEdit = ID;

                  GetDlgItemText( hDlg, ID, buffer, 32 );
                  value = atoi( buffer );
                  max   = ( IDE_COLORS_HUE_CHANNEL == ID ) ? 359 : 255;
       
                  if( value < 0 )
                     value = 0;
                  else if( max < value )
                     {
                     value = max;
 
                     sprintf( buffer, "%d", value );
                     
                     ccs->elementColorDlgLockEdits = 1;
                     SetDlgItemText( hDlg, ID, buffer );
                     ccs->elementColorDlgLockEdits = 0;
                     }

                  _UpdateElementColor( ccs, value, ID );
                  _DrawElementColorBox( ccs, hDlg );
                  _DrawElementColorChannels( ccs, hDlg );
                  _DrawElementColorGrid( ccs, hDlg );

                  ccs->elementColorDlgActiveEdit = 0;
                  }
               break;

            case IDB_COLORS_DEFAULTS:
               _ColorsDlgProcOnResetDefaults( ccs, hDlg );
               break;

				case IDB_COLORS_GRID_TOGGLE:
					_toggle_choose_element_colors_grid( hDlg );
					break;

            case IDOK:
               _CleanupElementColorsDlgProc( ccs, hDlg );
               EndDialog( hDlg, 1 );
               break;

            case IDCANCEL:
               _CleanupElementColorsDlgProc( ccs, hDlg );
               EndDialog( hDlg, 0 );
               break;
            }
         return TRUE;

      case WM_PAINT:
         _DrawElementColorBox( ccs, hDlg );
         _DrawElementColorChannels( ccs, hDlg );
         _DrawElementColorGrid( ccs, hDlg );
         return FALSE;

      case WM_CLOSE:
         _CleanupElementColorsDlgProc( ccs, hDlg );
         EndDialog( hDlg, 0 );
         return TRUE;
      }

   return FALSE;

   }/* _ChooseElementColorsDlgProc() */


void _InitChooseElementColors( ChooseColorsStruct *ccs )
   {
   unsigned i;

   for( i = 0; i < ccs->numElements; ++i )
      {
      ccs->elementRgb[ i ].red   = GetRValue( ccs->elementColors[ i ] );
      ccs->elementRgb[ i ].green = GetGValue( ccs->elementColors[ i ] );
      ccs->elementRgb[ i ].blue  = GetBValue( ccs->elementColors[ i ] );

      PixelRgb24toHsv32( &ccs->elementRgb[ i ], &ccs->elementHsv[ i ] );

      ccs->elementPreviousRgb[ i ] = ccs->elementRgb[ i ];
      }

   }/* _InitChooseElementColors() */


void _SetChooseElementColors( ChooseColorsStruct *ccs )
   {
   unsigned i;

   for( i = 0; i < ccs->numElements; ++i )
      ccs->elementColors[ i ] = RGB( ccs->elementRgb[ i ].red, ccs->elementRgb[ i ].green, ccs->elementRgb[ i ].blue );
   }
      

int ChooseColorsDialog
   (
   HINSTANCE       hInstance,
   HWND            hOwner,
   const unsigned  numItems,
   const char      **itemNames,
   COLORREF        *itemColors,
   const COLORREF  *defaultColors,
   const unsigned  selectedItem,
   void            ( *onApply )( const unsigned item, const COLORREF color )
   )
   {
   ChooseColorsStruct  ccs;
   int                 retValue;


   memset( &ccs, 0, sizeof( ChooseColorsStruct ) );

   assert( 0 < numItems );
   assert( NULL != itemNames );
   assert( NULL != itemColors );

   ccs.elementColorDlgOwner   = hOwner;
   ccs.numElements            = numItems;
   ccs.elementNames           = itemNames;
   ccs.elementColors          = itemColors;
   ccs.elementDefaultColors   = defaultColors;
   ccs.elementColorsApplyFunc = onApply;

   ccs.elementRgb         = malloc( numItems * sizeof( PixelRgb24 ) );
   ccs.elementPreviousRgb = malloc( numItems * sizeof( PixelRgb24 ) );
   ccs.elementHsv         = malloc( numItems * sizeof( PixelHsv32 ) );

   if( NULL == ccs.elementRgb         ||
       NULL == ccs.elementPreviousRgb ||
       NULL == ccs.elementHsv           )
      {
      free( ccs.elementRgb );
      free( ccs.elementPreviousRgb );
      free( ccs.elementHsv );
      return -1;
      }

   _InitChooseElementColors( &ccs );

   retValue = ( int )DialogBoxParam( hInstance,
                              "DLG_COLORS",
                              hOwner,
                              _ChooseElementColorsDlgProc,
                              ( LPARAM )&ccs
                            );
   
   if( 1 == retValue )
      _SetChooseElementColors( &ccs );

   free( ccs.elementRgb );
   free( ccs.elementPreviousRgb );
   free( ccs.elementHsv );

   return ( -1 != retValue ) ? retValue : 0;

   }/* ChooseColorsDialog() */
