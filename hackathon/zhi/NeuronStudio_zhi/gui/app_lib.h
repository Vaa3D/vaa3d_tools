#ifndef __BIGLIB_H__
#define __BIGLIB_H__

/*
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
*/
#define _WIN32_WINNT 0x0400
//#define _WIN32_WINDOWS 0x0500

/* Cant get WM_MOUSEWHEEL to be defined??? */
#define _DEFINE_WM_MOUSE_WHEEL 0x020A




#define eDISPLAY_WINDOW_NO_MOUSE_MODE  -1

typedef enum
   {
   eDISPLAY_WINDOW_MOUSE_MODE_ZOOM,
   eDISPLAY_WINDOW_MOUSE_MODE_DRAG,
   eDISPLAY_WINDOW_MOUSE_MODE_SEED,
	eDISPLAY_WINDOW_MOUSE_MODE_RAYS,
	eDISPLAY_WINDOW_MOUSE_MODE_SPINE,
	eDISPLAY_WINDOW_MOUSE_MODE_NEURITE,
	eDISPLAY_WINDOW_MOUSE_MODE_ROI,
	DISPLAY_WINDOW_MOUSE_MODE_MAGNET,

	mDISPLAY_WINDOW_NUM_MOUSE_MODES
   }
   eDISPLAY_WINDOW_MOUSE_MODE;


enum{
	_ROI_CURSOR_NW,
	_ROI_CURSOR_SE,
	_ROI_CURSOR_NE,
	_ROI_CURSOR_SW,
	_ROI_CURSOR_W,
	_ROI_CURSOR_E,
	_ROI_CURSOR_N,
	_ROI_CURSOR_S,
	_ROI_CURSOR_ALL,
	_ROI_CURSOR_ARROW,
	_ROI_CURSOR_CROSS,

	_ROI_NUM_CURSORS
	};



#include <windows.h>


/* Set mode message. Sent to set the mouse mode for the window.
   wParam = 0, lParam = one of the above eDISPLAY_WINDOW_MOUSE_MODE
   constants. */

#define mMSG_USER_SetMouseMode  ( WM_USER + 3 )


#include <commctrl.h>
#ifndef _MT
#define _MT
#endif
#include <process.h>
#include <io.h>
//#include <conio.h>
#include "resource.h"
#include <image/tiff.h>
#include <image/raw.h>
#include <image/jpeg.h>

#include "statusbar.h"

#include <std/nsinit.h>
#include <std/nslog.h>
#include <std/nsstring.h>
#include <std/nsthread.h>
#include <model/nsmodel.h>
#include <model/nsmodel-filter.h>
#include <model/nsmodel-order.h>
#include <model/nsmodel-section.h>
#include <model/nsmodel-stats.h>
#include <model/nsmodel-origs.h>
#include <model/nsmodel-join.h>
#include <model/nsmodel-remove.h>
#include <model/nsmodel-edge.h>
#include <model/nsmodel-vertex.h>
#include <model/nsmodel-function.h>
#include <model/nsmodel-junctions.h>
#include <model/nsmodel-io.h>
#include <model/nsmodel-graft.h>
//#include <model/nsmodel-project.h>
#include <model/nsmodel-spines.h>
#include <model/nsmodel-transform.h>
#include <model/nsspines-transform.h>
#include <model/nsspines-classifier.h>
#include <model/nsshollanalysis.h>
#include <image/nsimage.h>
#include <image/nsimagedir.h>
#include <image/nscolor.h>
#include <image/nssampler.h>
#include <image/nssplats.h>
#include <image/nstilemgr.h>
#include <math/nsfrustum.h>
#include <math/nsline.h>
#include <math/nsmatrix.h>
#include <math/nsvector.h>
#include <math/nscube.h>
#include <model/nsrayburst.h>
#include <ext/circles.h>
#include "nsseed3d.h"


#define mDEBUG


typedef enum
   {
   eNO_ERROR,
   eERROR_OUT_OF_MEMORY,
   eERROR_FILE_OPEN,
   eERROR_FILE_READ,
   eERROR_FILE_WRITE,
   eERROR_FILE_INVALID,
   eERROR_FILE_FORMAT,
   eERROR_EOF,
   eERROR_OPER_SYSTEM,
   eERROR_PIXEL_FORMAT,
   eERROR_COMPRESSION,
   eERROR_MULTIPLE_PIXEL_FORMATS,
   eERROR_MULTIPLE_WIDTH_HEIGHT,
   eERROR_MAX_SIZE,
   eERROR_DIRECTORY,
   eERROR_MULTIPLE_2D_3D_FILES
   }
   eERROR_TYPE;


#define _Rnd  rand()

extern const nschar* g_ErrorStrings[];


extern eERROR_TYPE g_Error;


extern HINSTANCE g_Instance;


#define mARRAY_LENGTH( ar )\
   ( sizeof( ( ar ) ) / sizeof( ( ar )[ 0 ] ) )


#define mMAX3( a, b, c )\
   ( ( a > b ) ? ( ( a > c ) ? a : c ) : ( ( b > c ) ? b : c ) )



#define ____TO_COLORREF( C )\
   RGB( NS_COLOR_GET_RED_U8( C ), NS_COLOR_GET_GREEN_U8( C ), NS_COLOR_GET_BLUE_U8( C ) )


#define ____FROM_COLORREF( color, C )\
   NS_COLOR_SET_RED_U8( C, GetRValue( color ) );\
   NS_COLOR_SET_GREEN_U8( C, GetGValue( color ) );\
   NS_COLOR_SET_BLUE_U8( C, GetBValue( color ) )\



/* Matrices in COLUMN MAJOR order!!! */

void IdentityMatrix44( nsfloat matrix[16] );


#ifdef mDEBUG

void _Log( const nschar *format, ... );

//#define _mLOG_FILE_PATH  "D://usr//douglas//projects//neuronstudio//nslog.txt"

#define mLOG0( format )\
   _Log( ( format ) )

#define mLOG1( format, arg1 )\
   _Log( ( format ), ( arg1 ) )

#define mLOG2( format, arg1, arg2 )\
   _Log( ( format ), ( arg1 ), ( arg2 ) )

#define mLOG3( format, arg1, arg2, arg3 )\
   _Log( ( format ), ( arg1 ), ( arg2 ), ( arg3 ) )

#define mLOG4( format, arg1, arg2, arg3, arg4 )\
   _Log( ( format ), ( arg1 ), ( arg2 ), ( arg3 ), ( arg4 ) )

#else

#define mLOG0( format )
#define mLOG1( format, arg1 )
#define mLOG2( format, arg1, arg2 )
#define mLOG3( format, arg1, arg2, arg3 )
#define mLOG4( format, arg1, arg2, arg3, arg4 )

#endif/* mDEBUG */


enum{
   __COLOR_SCHEME_BY_VERTEX_TYPE,
   __COLOR_SCHEME_BY_EDGE_ORDER,
   __COLOR_SCHEME_BY_COMPONENT
   };



#define mDISTANCE_SQUARED_2D( x1, y1, x2, y2 )\
   ( ( (x2) - (x1) )*( (x2) - (x1) ) + ( (y2) - (y1) )*( (y2) - (y1) ) )

#define mDISTANCE_2D( x1, y1, x2, y2 )\
   ( ( nsfloat )sqrt( mDISTANCE_SQUARED_2D( x1, y1, x2, y2 ) ) )



#ifdef mDEBUG

#define _mDEBUG_MSGBOX( msg ) MessageBox( NULL, ( msg ), "", MB_OK )

#define mDEBUG_MSGBOX_0( format )\
   { _mDEBUG_MSGBOX( format ); }

#define mDEBUG_MSGBOX_1( format, arg1 )\
   { nschar msg[1024]; sprintf( msg, ( format ), ( arg1 ) ); _mDEBUG_MSGBOX( msg ); }

#define mDEBUG_MSGBOX_2( format, arg1, arg2 )\
   { nschar msg[1024]; sprintf( msg, ( format ), ( arg1 ), ( arg2 ) ); _mDEBUG_MSGBOX( msg ); }

#define mDEBUG_MSGBOX_3( format, arg1, arg2, arg3 )\
   { nschar msg[1024]; sprintf( msg, ( format ), ( arg1 ), ( arg2 ), ( arg3 ) ); _mDEBUG_MSGBOX( msg ); }

#else

#define mDEBUG_MSGBOX_1( format, arg1 )
#define mDEBUG_MSGBOX_2( format, arg1, arg2 )
#define mDEBUG_MSGBOX_3( format, arg1, arg2, arg3 )

#endif/* mDEBUG */



#define mBIT_0    0x00000001
#define mBIT_1    0x00000002
#define mBIT_2    0x00000004
#define mBIT_3    0x00000008
#define mBIT_4    0x00000010


#define mPI  3.141592654f

#define m180_OVER_PI   ( 180.0f / mPI )



/*
typedef struct _BoundingCube
   {
   nsfloat  minX;
   nsfloat  minY;
   nsfloat  minZ;
   nsfloat  maxX;
   nsfloat  maxY;
   nsfloat  maxZ;
   }
   BoundingCube;
*/

//void InitBoundingCube( BoundingCube* cube );



typedef struct tagVector3i
   {
   nslong x;
   nslong y;
   nslong z;
   }
   Vector3i;

Vector3i* MakeVector3i( Vector3i* v, nslong x, nslong y, nslong z );


typedef struct tagVector2i
   {
   nslong x;
   nslong y;
   }
   Vector2i;

Vector2i* MakeVector2i( Vector2i* v, nslong x, nslong y );



/* C = A + B
   Returns C */
Vector2i* AddVector2i( const Vector2i* A, const Vector2i* B, Vector2i* C );


/* C = A - B  
   Returns C */
Vector2i* SubVector2i( const Vector2i* A, const Vector2i* B, Vector2i* C );
   


typedef struct tagVector3f
   {
   nsfloat x;
   nsfloat y;
   nsfloat z;
   }
   Vector3f;


#define MakeVector3f( V, _x, _y, _z )\
   ( V )->x = ( _x );\
   ( V )->y = ( _y );\
   ( V )->z = ( _z )


Vector3f* AddVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C );

Vector3f* SubVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C );

nsfloat MagnitudeVector3f( const Vector3f* V );

Vector3f* NormalizeVector3f( Vector3f* V );

Vector3f* CrossProductVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C );

nsfloat DotProductVector3f( const Vector3f* A, const Vector3f* B );

Vector3f* ScaleVector3f( Vector3f* V, const nsfloat scalar );



RECT* MakeRectangle( RECT *r, nslong left, nslong top, nslong right, nslong bottom );


typedef struct tagImage
   {
   nssize  width;
   nssize  height;
   nssize  length;
   void     *pixels;
   nssize bitsPerPixel;
   nssize saBoundary;
   }
   Image;


void ConstructImage( Image* image );

void DestructImage( Image* image );



#define mABS( v ) ( ( ( v ) < 0 ) ? -( v ) : ( v ) )


#define mALIGN( value, boundary )\
   ( ( ( value ) + ( boundary ) - 1 ) & ( ~( ( boundary ) - 1 ) ) )


typedef enum
   {
   eIMAGE_FILE_UNSUPPORTED,
   eIMAGE_FILE_TIFF,
   eIMAGE_FILE_RAW,
	IMAGE_FILE_LSM
   }
   eIMAGE_FILE_TYPE;


eIMAGE_FILE_TYPE GetImageFileType( const nschar* fileName );


eERROR_TYPE ReadImage3D( Image *image, const nschar *file, NsProgress *progress, nsulong *max_intensity );
eERROR_TYPE ReadImage2D( Image *image, const nschar *file, nspointer buffer, nssize bytes, nsulong *max_intensity );



#define mPALETTE_MAX_COLORS  256


typedef struct tagPalette
   {
/*
   DWORD  biSize; 
   LONG   biWidth; 
   LONG   biHeight; 
   WORD   biPlanes; 
   WORD   biBitCount;
   DWORD  biCompression;
   DWORD  biSizeImage; 
   LONG   biXPelsPerMeter; 
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed; 
   DWORD  biClrImportant;
*/
   BITMAPINFOHEADER h;
   NsVector4ub colors[ mPALETTE_MAX_COLORS ];
   }
   Palette;


typedef enum
   {
   e3D_BORDER_NONE,
   e3D_BORDER_RAISED,
   e3D_BORDER_SUNKEN
   }
   e3D_BORDER_TYPE;


void ColorThinGraphicsBorder3D( HDC graphics,
                                  const RECT *rect,
                                  nsuint hiliteColor,
                                  nsuint shadowColor,
                                  e3D_BORDER_TYPE type
                                );


void DrawGraphicsLine( HDC graphics, int x1, int y1, int x2, int y2, HPEN hPen );


void DrawGraphicsCircle( HDC graphics, int ox, int oy, int radius, HPEN hPen );

void DrawGraphicsEllipse( HDC graphics, int ox, int oy, int radiusX, int radiusY, HPEN hPen );

void FillGraphicsEllipse( HDC graphics, int ox, int oy, int radiusX, int radiusY, HBRUSH hBrush, HPEN hPen );


void DrawFrustum( HDC graphics, POINT points[4], HPEN hPen );
void FillFrustum( HDC graphics, POINT points[4], HBRUSH hBrush, HPEN hPen );

void ColorGraphicsBorder( HDC graphics, const RECT* rect, nsuint width, nsuint color );


NS_IMPEXP void ____graphics_fill_rectangle( HDC graphics, int left, int top, int right, int bottom, HBRUSH brush );


NS_IMPEXP void ____draw_text( HDC graphics, nsint x, nsint y, const nschar *text, COLORREF color );


NS_IMPEXP void graphics_draw_triangle
   (
   HDC graphics,
   int x1, int y1,
   int x2, int y2,
   int x3, int y3,
   HBRUSH brush,
   HPEN pen
   );



void DrawImage(
   HDC graphics,
   int dest_x,
   int  dest_y,
   int dest_width,
   int dest_height,
   const NsImage *image,
   Palette *palette
   );


void DrawBorder(
   HDC graphics,
   int x,
   int  y,
   int width,
   int height,
   int aabbox_selected_style
   );

SIZE* MakeSize( SIZE* size, nslong cx, nslong cy );

POINT* MakePoint( POINT* pt, nslong x, nslong y );


#define mGET_MOUSE_X( lParam )\
   ( ( int )( ( short )LOWORD( lParam ) ) )

#define mGET_MOUSE_Y( lParam )\
   ( ( int )( ( short )HIWORD( lParam ) ) )


int FileOpenNameDialog
   (
   HWND                owner, 
   const nschar*         title,
   nschar*         path,
   const nsuint      maxPath,
   const nschar*         filter,
   const nschar*   directory,
   const nschar*   extension
   );


typedef struct tagMemoryGraphics
   {
   HDC        hDC;
   HBITMAP    hBuffer;
   HBITMAP    hOriginalBuffer;
   int        originalSwapMode;
   nsuint   width;
   nsuint   height;
   }
   MemoryGraphics;


int CreateMemoryGraphics( MemoryGraphics* graphics, HDC hDC, nsuint width, nsuint height );

void DestroyMemoryGraphics( MemoryGraphics* graphics );


void DrawImageIntoMemoryGraphics( MemoryGraphics* graphics, const NsImage* image, 
                                  Palette* palette );



typedef void ( *mouse_event_handler )
   ( 
   HWND            window,
   int             x,
   int             y,
   const nsuint  flags
   );


typedef void ( *mouse_mode_handler )( HWND hWnd );


typedef struct tagMouseMode
  {
  mouse_event_handler  hook;
  mouse_event_handler  onMouseMove;
  mouse_event_handler  onLButtonDown;
  mouse_event_handler  onLButtonUp;
  mouse_event_handler  onLButtonDblClk;
  mouse_event_handler  onRButtonDown;
  mouse_event_handler  onRButtonUp;
  mouse_mode_handler  onUnset;
  mouse_mode_handler  onSet;
  }
  MouseMode;


#define mMAX_MOUSE_MODES   16

typedef struct tagMouse
  {
  int               currentID;
  const MouseMode  *currentMode;
  MouseMode         modes[mMAX_MOUSE_MODES];
  }
  Mouse;


eERROR_TYPE ProgressDialog
	(
	HWND hWnd,
	const nschar *title,
	void ( *function )( void *dialog ),
	void *inArgs,
	void *outArgs,
	int *wasCancelled
	);

/* IMPORTANT: Only the thread passed in UIDialogProgress() should call these
   functions! */

int ProgressDialogWasCancelled( void *dialog );

void EndProgressDialog( void *dialog, const eERROR_TYPE error );

void GetProgressDialogArgs( void *dialog, void** inArgs, void** outArgs );

void UpdateProgressDialog( void *dialog, const nsfloat percentDone );

void SetProgressDialogTitle( void *dialog, const nschar* title );

void SetProgressDialogMessage( void *dialog, const nschar* message );


void CenterWindow( HWND hChild, HWND hParent );


/* Returns 1 if yes, else 0. */
//int YesNoDialog( HWND hWnd, const nschar *question );


typedef struct tagFileList
   {
   nschar **names;
   nsuint length;
   }
   FileList;


eERROR_TYPE BuildFileList( FileList *list, const nschar* fullPath );

void DestroyFileList( FileList *list );


int ParseVolumeFileListName
   ( 
   const nschar*   fileName,
   nschar               *prefix, 
   const nsuint          prefixChars,
   nschar               *extension, 
   const nsuint          extensionChars
   );

const nschar* FindFileNameInPath( const nschar* path, const nschar dirMark );


void CenterRectangleHoriz( RECT* src, const RECT* dest );
   
void CenterRectangleVert( RECT* src, const RECT* dest );


nsfloat ScaleAndCenterRectangle( RECT* src, const RECT* dest );


void ScaleRectangle( RECT* rect, nsfloat scalar );


void TranslateRectangle( RECT* rect, const nslong amount, const int vertically );





NS_IMPEXP int SettingsDialog
   (
   HWND         owner,
   NsSettings  *current_and_return,
	NsSettings  *last_applied,
	NsModel     *raw_model,
	NsModel     *filtered_model
   );


void AboutNeuronStudioDialog( HWND hOwner );



typedef enum
   {
   eMENU_EVENT_FILE_OPEN,
   eMENU_EVENT_FILE_CLOSE,

   _eMENU_EVENT_SENTINEL /* Dont use! */
   }
   eMENU_EVENT;

#define mNUM_MENU_EVENTS  ( ( int )_eMENU_EVENT_SENTINEL )


void OnMenuEvent( const eMENU_EVENT event, HWND hOwner );


void EnableMenuItems( HWND hWnd, int enabled );

void SetMenuItemsChecked( HWND hWnd, int checked );


nsboolean GetMenuItemEnabled( HWND hWnd, int id );
void SetMenuItemEnabled( HWND hWnd, int id, int enabled );


void SetMenuItemChecked( HWND hWnd, int id, int checked );

int IsMenuItemChecked( HWND hWnd, int id );



typedef struct _DragRectRenderer
   {
   HWND   wnd;
   HDC    dc;
   HPEN   pen;
   HPEN   old_pen;
   nsint  old_rop;
   nsint  old_bk_mode;
   }
   DragRectRenderer;

void drag_rect_renderer_init( DragRectRenderer *state, HWND wnd );

void drag_rect_renderer_finalize( DragRectRenderer *state );

void drag_rect_renderer_line( DragRectRenderer *state, nsint x1, nsint y1, nsint x2, nsint y2 );

#endif/* __BIGLIB_H__ */

