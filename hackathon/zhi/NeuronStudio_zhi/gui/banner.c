#include <assert.h>
#include "banner.h"


typedef struct tagINTROBANNERBMP
   {
   HINSTANCE  hInstance;
   HBITMAP    hBitmap;
   int        width;
   int        height;
   int        seconds;
   }
   INTROBANNERBMP;

static INTROBANNERBMP s_IntroBannerBmp;


static HWND s_IntroBannerStatus = NULL;


#define _NUM_BS_OPERATIONS  6

const char *s_BS_Strings[ _NUM_BS_OPERATIONS ] =
   {
   "Loading statistical model...",
   "Initializing 3D engine...",
   "Generating memory layout...",
   "Establishing hardware profile...",
   "Synchronizing transitional projections...",
   "Optimizing functional time-phase..."
   };

const unsigned s_BS_Timeout_Min[ _NUM_BS_OPERATIONS ] =
   { 400, 300, 300, 500, 400, 100 };

const unsigned s_BS_Timeout_Max[ _NUM_BS_OPERATIONS ] =
   { 1000, 800, 1000, 700, 500, 700 };

int s_BS_Operation = 0;

#define _BS_TIMER_ID  102


void _SetBSTimer( HWND hWnd )
   {
   SetTimer( hWnd, 
             _BS_TIMER_ID,
             rand()%( s_BS_Timeout_Max[ s_BS_Operation ] - s_BS_Timeout_Min[ s_BS_Operation ] + 1 ) 
             + s_BS_Timeout_Min[ s_BS_Operation ],
             NULL 
           );
   }


#define _INTRO_BANNER_TIMER_ID  101


LRESULT CALLBACK _IntroBannerWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   HDC          hDC;
   HDC          hMemDC;
   PAINTSTRUCT  ps;


   switch( uMsg )
      {
      case WM_TIMER:
         switch( wParam )
            {
            //case _INTRO_BANNER_TIMER_ID:
              // KillTimer( hWnd, _INTRO_BANNER_TIMER_ID );
              // DestroyWindow( hWnd );
              // break;

            case _BS_TIMER_ID:
               SendMessage( s_IntroBannerStatus, SB_SETTEXT, 0, ( LPARAM )s_BS_Strings[ s_BS_Operation ] );
               ++s_BS_Operation;
               if( _NUM_BS_OPERATIONS == s_BS_Operation )
                  DestroyWindow( hWnd );
               else
                  _SetBSTimer( hWnd );
               break;
            }
         break;

      case WM_PAINT:
         hDC    = BeginPaint( hWnd, &ps );
         hMemDC = CreateCompatibleDC( hDC );
         SelectObject( hMemDC, s_IntroBannerBmp.hBitmap );
         BitBlt( hDC, 0, 0, 
                 s_IntroBannerBmp.width, s_IntroBannerBmp.height, 
                 hMemDC, 0, 0, 
                 SRCCOPY );
         DeleteDC( hMemDC ); 
         EndPaint( hWnd, &ps );
         break;

      case WM_LBUTTONDOWN:
        // DestroyWindow( hWnd );
         break;

      case WM_CREATE:
         //if( ! SetTimer( hWnd, _INTRO_BANNER_TIMER_ID, s_IntroBannerBmp.seconds*1000, NULL ) ) 
           // DestroyWindow( hWnd );
         s_BS_Operation = 0;
         _SetBSTimer( hWnd );
         break;

      case WM_DESTROY:
         PostQuitMessage( 0 );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }
   
   return 0;

   }//_IntroBannerWndProc()


static char *s_IntroBannerClass = "Intro_Banner_x0394jdkfj";


void IntroBanner
   ( 
   HINSTANCE    hInstance,
   char        *bmpResourceName,
   int          hasBorder,
   int          hasStatus,
   const char  *statusText,
   int          seconds
   )
   {
   HWND                  hWnd;
   WNDCLASSEX            wcx;
   DWORD                 style;
   MSG                   msg;
   BITMAP                bmp;
   INITCOMMONCONTROLSEX  iccx;
   HWND                  hStatus;
   RECT                  rc;
   unsigned              width;
   unsigned              height;


   assert( NULL != hInstance );
   assert( NULL != bmpResourceName );
   assert( 0 <= seconds );

   iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
   iccx.dwICC  = ICC_WIN95_CLASSES;
   InitCommonControlsEx( &iccx );

   s_IntroBannerBmp.hInstance = hInstance;
   s_IntroBannerBmp.seconds   = seconds;
   s_IntroBannerBmp.hBitmap   = LoadBitmap( hInstance, bmpResourceName );

   if( NULL == s_IntroBannerBmp.hBitmap )
      return;   

   GetObject( s_IntroBannerBmp.hBitmap, sizeof( BITMAP ), &bmp );

   s_IntroBannerBmp.width  = bmp.bmWidth;
   s_IntroBannerBmp.height = bmp.bmHeight;

   wcx.cbSize         = sizeof( WNDCLASSEX );
   wcx.style          = CS_HREDRAW | CS_VREDRAW;
   wcx.lpfnWndProc    = _IntroBannerWndProc;
   wcx.cbClsExtra     = 0;
   wcx.cbWndExtra     = 0;
   wcx.hInstance      = hInstance;
   wcx.hIcon          = NULL;
   wcx.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wcx.hbrBackground  = NULL;
   wcx.lpszClassName  = s_IntroBannerClass;
   wcx.lpszMenuName   = NULL;
   wcx.hIconSm        = NULL;

   if( 0 == RegisterClassEx( &wcx ) )
      {
      DeleteObject( s_IntroBannerBmp.hBitmap );
      return;
      }

   style = WS_POPUP;

   if( hasBorder )
      style |= WS_DLGFRAME;

   hWnd = CreateWindowEx( WS_EX_TOOLWINDOW,
                          s_IntroBannerClass,
                          NULL,
                          style,
                          GetSystemMetrics( SM_CXSCREEN )/2 - bmp.bmWidth/2,
                          GetSystemMetrics( SM_CYSCREEN )/2 - bmp.bmHeight/2,
                          bmp.bmWidth,
                          bmp.bmHeight,
                          NULL,
                          NULL,
                          hInstance,
                          NULL
                        );

   if( NULL == hWnd )
      {
      DeleteObject( s_IntroBannerBmp.hBitmap );
      return;
      }

   if( hasStatus )
      {
      hStatus = CreateWindowEx( 0, 
                                STATUSCLASSNAME,
                                statusText,
                                WS_CHILD | WS_VISIBLE,
                                0, 0, 0, 0, 
                                hWnd,
                                0,
                                hInstance, 
                                NULL
                              );

      GetWindowRect( hStatus, &rc );

      width  = bmp.bmWidth;
      height = bmp.bmHeight;

      if( hasBorder )
         {
         width  += GetSystemMetrics( SM_CXDLGFRAME )*2;
         height += GetSystemMetrics( SM_CYDLGFRAME )*2;
         }

      SetWindowPos( hWnd,
                    HWND_TOPMOST,
                    0, 0,
                    width,
                    height + rc.bottom - rc.top, 
                    SWP_NOMOVE | SWP_NOREDRAW
                  );

      SetWindowPos( hStatus,
                    NULL,
                    0,
                    height,
                    width,
                    rc.bottom - rc.top,
                    SWP_NOZORDER | SWP_NOREDRAW
                  );
 
      s_IntroBannerStatus = hStatus;
      }

   ShowWindow( hWnd, SW_SHOW );
   UpdateWindow( hWnd );

   while( 0 < GetMessage( &msg, NULL, 0, 0 ) )
      {
      TranslateMessage( &msg );
      DispatchMessage( &msg ); 
      }

   DeleteObject( s_IntroBannerBmp.hBitmap );
   }
