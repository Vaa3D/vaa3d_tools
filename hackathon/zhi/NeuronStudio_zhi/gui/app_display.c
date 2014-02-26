#include "app_display.h"
#include <math/nsdragrect.h>


nsint s_CurrentMouseMode = -1;

extern nsboolean ____redraw_2d_need_render;
void DrawDisplayWindow( HWND wnd );

//extern nsint *____model_mode;

extern nsint ____neurites_vertex_render_mode;
extern nsint ____neurites_edge_render_mode;

extern nsint ____spines_render_mode;
extern nsint ____spines_color_scheme;

extern nsboolean ____return_raw_model;

//extern nsboolean ____use_2d_measurement_sampling;

extern nsboolean ____draw_junction_vertices_only;

extern nsint ____vertex_color_scheme;
extern nsint ____edge_color_scheme;


extern nsboolean ____view_sholl_analysis;

nsboolean ____lbutton_double_clicked = NS_FALSE;


extern nsint ____jitter_x;
extern nsint ____jitter_y;
extern nsint ____jitter_z;


extern void ____redraw_3d( void );


extern nschar _startup_directory[];
extern nschar ____config_file[];


//extern nssize     ____volume_width;
//extern nssize     ____volume_height;
//extern nssize     ____volume_length;


extern nsboolean ____xy_slice_enabled;
extern nssize    ____xy_slice_index;

nssize ____svw_bar_left = 32;
nssize ____svw_bar_height = 100;

#define _SVW_BAR_LEFT    ____svw_bar_left
#define _SVW_BAR_TOP     16
#define _SVW_BAR_WIDTH   3
#define _SVW_BAR_HEIGHT  ____svw_bar_height
#define _SVW_ARROW_TIP_X  22
#define _SVW_ARROW_TIP_Y  6
#define _SVW_ARROW_WIDTH 23
#define _SVW_ARROW_HEIGHT 13
#define _SVW_ARROW_OFFSET 6
#define _SVW_ARROW_ISECT_SPACE_X  2
#define _SVW_ARROW_ISECT_SPACE_Y  3

#define _SVW_ARROW_LEFT\
	( _SVW_BAR_LEFT - ( _SVW_ARROW_TIP_X + _SVW_ARROW_OFFSET ) )

#define _SVW_ARROW_TOP( y )\
	( ( _SVW_BAR_TOP + (y) ) - _SVW_ARROW_TIP_Y )


nssize ____svw_arrow_y;


extern nsboolean ____alt_key_is_down;


extern nsboolean ____2d_display_center_and_limit_image;


extern nschar ____config_file[];

nsboolean ____drag_rect_active = NS_FALSE;

extern nsboolean ____flash_frames_active;


NsColor4ub ____colors[] =
	{
	{ 255,   0,   0, 255 },
	{ 255, 165,   0, 255 },
	{ 255, 255,   0, 255 },
	{   0, 255,   0, 255 },
	{   0,   0, 255, 255 },
	{  75,   0, 130, 255 },
	{ 238, 130, 238, 255 }
	};

nssize ____num_colors = NS_ARRAY_LENGTH( ____colors );

NsColor4ub *____order_colors     = ____colors;
NsColor4ub *____section_colors   = ____colors;
NsColor4ub *____conn_comp_colors = ____colors;


//nsuchar *____colors = ( nsuchar* )( ____ns_colors_4ub + 1 );


/* Orders start
nsuchar *____order_colors = ____colors + 2;



/*
nsuchar __dir_vector_colors[ 256 * 3 ] =
   {
   #include "palettes/thermal.pal"
   };
*/




extern nsboolean __progress_cancelled( NsProgress *progress );
extern void __progress_update( NsProgress *progress, nsfloat percent );
extern void __progress_set_title( NsProgress *progress, const nschar *title );




extern void ____redraw_all();



#define _MAX_DISPLAY_WINDOWS  4

enum{ _DRAG_MODE_NORMAL, _DRAG_MODE_TRANSLATE, _DRAG_MODE_ROTATE,
		_DRAG_MODE_SLICE_BAR, _DRAG_MODE_SLICE_ARROW };


const nschar* _roi_cursor_to_string( nsint cursor )
	{
	NS_PRIVATE const nschar* ____roi_cursor_strings[ _ROI_NUM_CURSORS ] =
		{
		"NW",
		"SE",
		"NE",
		"SW",
		"W",
		"E",
		"N",
		"S",
		"ALL",
		"ARROW"
		};

	ns_assert( 0 <= cursor && cursor < _ROI_NUM_CURSORS );
	return ____roi_cursor_strings[ cursor ];
	}


typedef struct tagDisplayWindow
   {
   HWND               hWnd;
   //HWND             old_focus_hwnd;
   NsDragRect         drag_rect;
	NsDragRect         zoom_rect;
   nsuint             ID;
   nsuint             index;
   nsint              display;
   HWorkspace         workspace;
   nsint              haveBackBuffer;
   //nsint            isActive;
   nsint              isDragging;
   nsboolean          is_rotating;
   nsboolean          is_translating;
	nsboolean          is_slicing;
   nsint              hasCapture;
   Vector2i           clientCoord;
   MemoryGraphics     backBuffer;
   nsfloat            zoom;
   Vector2i           corner;
   RECT               globalViewport;
   //HBRUSH           vertex_brushes[ NS_MODEL_VERTEX_NUM_TYPES ];
   //HPEN             vertex_pens[ NS_MODEL_VERTEX_NUM_TYPES ];
   //HBRUSH           vertex_brush;
   //HPEN             vertex_pen;
   //HBRUSH           edge_brush;
   //HPEN             edge_pen;
   //HPEN             sample_radius_pen;
   //HPEN             sample_length_pen;
   //nsuint           counter;
   //nsmodelvertex    selectedVertex;
   //HFONT            hFont;
   nsuint             pen_thickness;
   //nsint            useSolidVertices;
   Vector3i           old_seed;
   //nsboolean        filled;
   //NsColor4ub       curr_color;
   //NsColor4ub      *conn_comp_colors;
   //NsColor4ub      *order_colors;
   //NsColor4ub      *section_colors;
   nsboolean          drag_mode;
   NsModelTransform   model_xfrm;
	NsSpinesTransform  spines_xfrm;
   nsint              xfrm_center_x;
   nsint              xfrm_center_y;
   nsint              xfrm_radius;
	nsint              mouse_x;
	nsint              mouse_y;
   NsVector3f         P1f;
   NsVector3f         P2f;
   NsVector3i         P1i;
   NsVector3i         P2i;
	Vector2i           old_client;
	Vector2i           old_image;
	Vector2i           new_client;
	Vector2i           new_image;
	nsfloat            hw_aspect_ratio;
	HCURSOR            roi_cursors[ _ROI_NUM_CURSORS ];
	nsint              roi_curr_cursor;
   nsint              curr_left, curr_top, lock_left, lock_top;
	nsint              seed_a, seed_b, *seed_c;
	const NsImage     *seed_image;
	const NsCubei     *seed_roi;
	nsenum             seed_which;
	nsint            **seed_indices;
	nsint             *seed_num_indices;
	nsboolean          ogl_is_dragging;
	nsboolean          roi_did_change;
   }
   DisplayWindow;


extern HWND *____the_main_window;

NS_PRIVATE nsboolean ____seed_finding_running;


void _display_window_do_find_seed( void *dialog )
   {
	DisplayWindow *dw;
   nsint                *notUsed;
   Progress            progress;
   NsProgress          nsprogress;
   NsError             error;



   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &dw, &notUsed );
   ns_assert( NULL != dw );

   ns_progress( &nsprogress,
                __progress_cancelled,
                __progress_update,
                __progress_set_title,
                NULL,
                &progress
              );

   if( IsProgressCancelled() )
      {
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   progress_set_title( &progress, "Finding seed..." );

	if( NS_FAILURE(
			ns_seed_find_2d(
				dw->seed_image,
				dw->seed_roi,
				dw->seed_a,
				dw->seed_b,
				dw->seed_which,
				dw->seed_c,
				&nsprogress
				),
			error ) )
		{
		/* TEMP: Ignore error? */
		ns_println( "Not enough memory to find seed!" );
		}

   EndProgressDialog( dialog, eNO_ERROR );
   }


NsError _display_window_find_seed
	(
	DisplayWindow   *dw,
	const NsImage   *image,
	const NsCubei   *roi,
	nsint            a,
	nsint            b,
	nsenum           which,
	nsint           *c
	)
   {
   //eERROR_TYPE  error;
   nsint        wasCancelled;


	ns_assert( ! ____seed_finding_running );
	____seed_finding_running = NS_TRUE;

	dw->seed_image = image;
	dw->seed_roi   = roi;
	dw->seed_a     = a;
	dw->seed_b     = b;
	dw->seed_which = which;
	dw->seed_c     = c;

   /*error = */ProgressDialog(
				dw->hWnd,
				NULL,
				_display_window_do_find_seed,
				dw,
				NULL,
				&wasCancelled
				);

	____seed_finding_running = NS_FALSE;

   //if( ! wasCancelled && eNO_ERROR != error )
     // return error;

   return ns_no_error();
   }


void _display_window_do_find_seed_ex( void *dialog )
   {
	DisplayWindow *dw;
   nsint                *notUsed;
   Progress            progress;
   NsProgress          nsprogress;
   NsError             error;



   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &dw, &notUsed );
   ns_assert( NULL != dw );

   ns_progress( &nsprogress,
                __progress_cancelled,
                __progress_update,
                __progress_set_title,
                NULL,
                &progress
              );

   if( IsProgressCancelled() )
      {
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   progress_set_title( &progress, "Finding seed..." );

	if( NS_FAILURE(
			ns_seed_find_2d_ex(
				dw->seed_image,
				dw->seed_roi,
				dw->seed_a,
				dw->seed_b,
				dw->seed_which,
				dw->seed_indices,
				dw->seed_num_indices,
				&nsprogress
				),
			error ) )
		{
		/* TEMP: Ignore error? */
		ns_println( "Not enough memory to find seed!" );
		}

   EndProgressDialog( dialog, eNO_ERROR );
   }


NsError _display_window_find_seed_ex
	(
	DisplayWindow   *dw,
	const NsImage   *image,
	const NsCubei   *roi,
	nsint            a,
	nsint            b,
	nsenum           which,
	nsint          **indices,
	nsint           *num_indices
	)
   {
   //eERROR_TYPE  error;
   nsint        wasCancelled;


	ns_assert( ! ____seed_finding_running );
	____seed_finding_running = NS_TRUE;

	dw->seed_image       = image;
	dw->seed_roi         = roi;
	dw->seed_a           = a;
	dw->seed_b           = b;
	dw->seed_which       = which;
	dw->seed_indices     = indices;
	dw->seed_num_indices = num_indices;

   /*error = */ProgressDialog(
				dw->hWnd,
				NULL,
				_display_window_do_find_seed_ex,
				dw,
				NULL,
				&wasCancelled
				);

	____seed_finding_running = NS_FALSE;

   //if( ! wasCancelled && eNO_ERROR != error )
     // return error;

   return ns_no_error();
   }


static HWND           s_DisplayWindowHandles[ _MAX_DISPLAY_WINDOWS ];
static DisplayWindow  s_DisplayWindows[ _MAX_DISPLAY_WINDOWS ];


nsint *____2d_display_type = &(s_DisplayWindows[0].display);

nsuint* ____pen_thickness = &(s_DisplayWindows[0].pen_thickness);

DisplayWindow* _MapHandleToDisplayWindow( HWND hWnd )
   {
   nsuint i;

   for( i = 0; i < _MAX_DISPLAY_WINDOWS; ++i )
      if( ( nsuint )hWnd == ( nsuint )s_DisplayWindowHandles[ i ] )
         return &s_DisplayWindows[ i ];

   return NULL;
   }


static nsuint  s_NumDisplayWindows = 0;
static nsint       s_DisplayWindowsAreRegistered = 0;
static nsint       s_DisplayWindowMouseModesAreSet = 0;


static Mouse s_Mouse;


typedef enum
   {
   _eZOOM_0p078,  /* 0.078% */
   _eZOOM_0p1,     
   _eZOOM_0p2,     
   _eZOOM_0p3,     
   _eZOOM_0p4,     
   _eZOOM_0p5,
   _eZOOM_0p7,
   _eZOOM_1p0,
   _eZOOM_1p5,
   _eZOOM_2p0,
   _eZOOM_3p0,
   _eZOOM_4p0,
   _eZOOM_5p0,
   _eZOOM_6p25,
   _eZOOM_8p33,
   _eZOOM_12p5,
   _eZOOM_16p7,
   _eZOOM_25p0,
   _eZOOM_33p3,
   _eZOOM_50p0,
   _eZOOM_66p7,
   _eZOOM_100p0,
   _eZOOM_200p0,
   _eZOOM_300p0,
   _eZOOM_400p0,
   _eZOOM_500p0,
   _eZOOM_600p0,
   _eZOOM_700p0,
   _eZOOM_800p0,
   _eZOOM_1200p0,
   _eZOOM_1600p0,  /* 1600% */
   _eZOOM_3200p0,
   _eZOOM_6400p0,
   _eZOOM_12800p0,
   _eZOOM_25600p0
   }
   _eZOOM_FACTOR;

#define _mZOOM_MIN   _eZOOM_0p078
#define _mZOOM_MAX   /*_eZOOM_1600p0*/ /*_eZOOM_6400p0*/ /*_eZOOM_12800p0*/ _eZOOM_25600p0

#define _mNUM_ZOOM_FACTORS  ( ( nsint )_mZOOM_MAX + 1 )


static nsfloat s_ZoomFactors[ _mNUM_ZOOM_FACTORS ] =
   {
   0.00078f,
   0.001f,
   0.002f,
   0.003f,
   0.004f,
   0.005f,
   0.007f,
   0.01f,
   0.015f,
   0.02f,
   0.03f,
   0.04f,
   0.05f,
   0.0625f,
   0.08333333f,
   0.125f,
   0.16666667f,
   0.25f,
   0.33333333f,
   0.5f,
   0.66666667f,
   1.0f,
   2.0f,
   3.0f,
   4.0f,
   5.0f,
   6.0f,
   7.0f,
   8.0f,
   12.0f,
   16.0f,
   32.0f,
   64.0f,
   128.0f,
   256.0f
   };

#define _mMIN_ZOOM_FACTOR  ( s_ZoomFactors[ _mZOOM_MIN ] )
#define _mMAX_ZOOM_FACTOR  ( s_ZoomFactors[ _mZOOM_MAX ] )


nsint _GetClosestLargerZoomFactor( const nsfloat zoom, nsfloat* closest )
   {
   _eZOOM_FACTOR factor;

   for( factor = _mZOOM_MIN; factor <= _mZOOM_MAX; ++factor )
      if( zoom < s_ZoomFactors[ factor ] )
         {
         *closest = s_ZoomFactors[ factor ];
         return 1;
         }
      
   return 0;

   }/* _GetClosestLargerZoomFactor() */


nsint _GetClosestSmallerZoomFactor( const nsfloat zoom, nsfloat* closest )
   {
   _eZOOM_FACTOR factor;

   for( factor = _mZOOM_MAX; _mZOOM_MIN <= factor; --factor )
      if( s_ZoomFactors[ factor ] < zoom )
         {
         *closest = s_ZoomFactors[ factor ];
         return 1;
         }
      
   return 0;

   }/* _GetClosestSmallerZoomFactor() */
   

/* NOTE: V means a vector and Vcorner is in global coordinates. */

/*  Vglobal = Vcorner + Vclient / zoom  */

static nsfloat s_DisplayVoxelX;
static nsfloat s_DisplayVoxelY;
static nsfloat s_DisplayVoxelZ;






void _DisplayWindowClientToGlobalVector
   ( 
   Vector2i*        global,
   const Vector2i*  client,
   const Vector2i*  corner,
   const nsfloat            zoom,
   const nsint    display
   )
   {
   global->x = //( NS_ZY == display ) ?
               corner->x + ( nslong )( client->x / zoom /* * s_DisplayVoxelZ */ )// :
               //corner->x + ( nslong )( client->x / zoom /* * s_DisplayVoxelX */ );
					;

   global->y = //( NS_XZ == display ) ?
               //corner->y - ( nslong )( client->y / zoom /* * s_DisplayVoxelZ */ ) :
               corner->y + ( nslong )( client->y / zoom /* * s_DisplayVoxelY */ );
   }



/* Convert from image space to client space. */
#define ____TO_CLIENT_SPACE( dw, V, C )\
   (V).x = (V).x - (C).x;\
   (V).y = /*( ( NS_XZ != (dw)->display ) ? */(V).y - (C).y /*: (C).y - (V).y )*/;\
   ns_vector2f_cmpd_scale( &(V), (dw)->zoom )


/*  Vclient = ( Vglobal - Vcorner ) * zoom  */

void _DisplayWindowGlobalToClientVector
   ( 
   Vector2i*        client, 
   const Vector2i*  global,
   const Vector2i*  corner,
   const nsfloat    zoom,
   const nsint      display
   )
   {
   client->x = ( nslong )( //( NS_ZY == display ) ?
                         ( global->x - corner->x ) * zoom /* / s_DisplayVoxelZ */// :
                        // ( global->x - corner->x ) * zoom /* / s_DisplayVoxelX */
                       );

   client->y = ( nslong )( //( NS_XZ == display ) ?
                         //( corner->y - global->y ) * zoom /* / s_DisplayVoxelZ */ :
                         ( global->y - corner->y ) * zoom/* / s_DisplayVoxelY */
                       );
   }


void _DisplayWindowClientToGlobalRectangle
   ( 
   RECT*            global, 
   const RECT*      client,
   const Vector2i*  corner,
   const nsfloat            zoom,
   const nsint    display
   )
   {
   Vector2i  clientTopLeft;
   Vector2i  clientBottomRight;
   Vector2i  globalTopLeft;
   Vector2i  globalBottomRight;

   MakeVector2i( &clientTopLeft, client->left, client->top );
   MakeVector2i( &clientBottomRight, client->right, client->bottom );

   _DisplayWindowClientToGlobalVector( &globalTopLeft, &clientTopLeft, corner, zoom, display );
   _DisplayWindowClientToGlobalVector( &globalBottomRight, &clientBottomRight, corner, zoom, display );

   MakeRectangle( global, globalTopLeft.x, globalTopLeft.y, globalBottomRight.x, globalBottomRight.y );
   }


void _DisplayWindowGlobalToClientRectangle
   ( 
   RECT*            client, 
   const RECT*      global,
   const Vector2i*  corner,
   const nsfloat            zoom,
   const nsint    display
   )
   {
   Vector2i  globalTopLeft;
   Vector2i  globalBottomRight;
   Vector2i  clientTopLeft;
   Vector2i  clientBottomRight;

   MakeVector2i( &globalTopLeft, global->left, global->top );
   MakeVector2i( &globalBottomRight, global->right, global->bottom );

   _DisplayWindowGlobalToClientVector( &clientTopLeft, &globalTopLeft, corner, zoom, display );
   _DisplayWindowGlobalToClientVector( &clientBottomRight, &globalBottomRight, corner, zoom, display );

   MakeRectangle( client, clientTopLeft.x, clientTopLeft.y, clientBottomRight.x, clientBottomRight.y );
   }












void _display_window_make_image_rectangle
   (
   HWorkspace      workspace,
   nsint           display,
   const NsImage  *image,
   RECT           *rect
   )
   {
/*
   MakeRectangle(
      rect,
      0,
      ( NS_XZ != display ) ? 0 : ( nsuint )ns_image_height( image ) - 1,
      ( nsuint )ns_image_width( image ),
      ( NS_XZ != display ) ? ( nsuint )ns_image_height( image ) : -1
      );
*/
	//if( NS_XZ != display )
		MakeRectangle( rect, 0, 0, ( nsuint )ns_image_width( image ), ( nsuint )ns_image_height( image ) );
	//else
	//	MakeRectangle( rect, 0, ( nsuint )ns_image_height( image ), ( nsuint )ns_image_width( image ), 0 );
   }


void _MakeDisplayWindowGlobalImageRectangle
   (
   HWorkspace   workspace,
   nsint        display,
   RECT        *rect
   )
   {
   _display_window_make_image_rectangle(
      workspace,
      display,
      GetWorkspaceDisplayImage( workspace, display ),
      rect
      );
   }


/*
void _display_window_render_tile_func_1
   (
   const NsTileMgr  *mgr,
   NsTile           *tile,
   DisplayWindow    *dw
   )
   {
   const NsImage  *image;
   RECT            src, dest;
   const MemoryGraphics *mg;
   //SIZE sz;


//   ns_tile_lock( tile );

   image = ns_tile_image( tile );
   ns_assert( 1 == ns_image_length( image ) );

   src.left   = ns_tile_get_pixel_x( tile );
   src.top    = ns_tile_get_pixel_y( tile );
   src.right  = src.left + ns_tile_width( tile );
   src.bottom = src.top + ns_tile_height( tile );

	// HACK!!!!!!!!!!!!! Have no frickin' idea why we need this?
	//if( NS_XZ == dw->display )
	//	{
	//	--src.top;
	//	--src.bottom;
	//	}
	
   _DisplayWindowGlobalToClientRectangle( &dest, &src, &dw->corner, dw->zoom, dw->display );

   mg = ( const MemoryGraphics* )ns_tile_get_object( tile );

   ns_assert( mg->width == ( nsuint )ns_tile_width( tile ) );
   ns_assert( mg->height == ( nsuint )ns_tile_height( tile ) );

   //#ifdef NS_DEBUG
   //GetBitmapDimensionEx( mg->hBuffer, &sz );
   //ns_assert( ( nssize )sz.cx == ( nssize )mg->width );
   //ns_assert( ( nssize )sz.cy == ( nssize )mg->height );
   //#endif

   StretchBlt(
      dw->backBuffer.hDC,
      dest.left,
      dest.top,
      dest.right  - dest.left,
      dest.bottom - dest.top,
      mg->hDC,
      0,
      0,
      mg->width,
      mg->height,
      SRCCOPY
      );

//   ns_tile_unlock( tile );
   }
*/


NS_PRIVATE nssize _get_optimal_tile_size( nssize image_width, nssize image_height, nsdouble zoom )
	{
	nssize    max_dimension;
	nssize    tile_size;
	nsdouble  bytes;


	max_dimension = NS_MAX( image_width, image_height );

	/* Find the largest a tile can be. */

	tile_size = 1;
	while( tile_size < max_dimension )
		tile_size *= 2;

	/* This seems to be the upper bound for Windows. */
	if( tile_size > 16384 )
		tile_size = 16384;

	/* Based on the current zoom, find the optimal tile size. */

	bytes = NS_POW2( ( nsdouble )tile_size * zoom );

	while( bytes > 4194304.0 ) /* 4 megabytes */
		{
		tile_size /= 2;
		bytes = NS_POW2( ( nsdouble )tile_size * zoom );
		}

	return tile_size;
	}


extern nsboolean ____workspace_show_tiles;

void _display_window_render_tile_func
   (
   const NsImage  *image,
   const NsTile   *tile,
	DisplayWindow  *dw
   )
	{
	Palette  *palette;
   RECT      src, dest;


   src.left   = ( LONG )ns_tile_get_pixel_x( tile );
   src.top    = ( LONG )ns_tile_get_pixel_y( tile );
   src.right  = ( LONG )( src.left + ns_tile_width( tile ) );
   src.bottom = ( LONG )( src.top + ns_tile_height( tile ) );

   _DisplayWindowGlobalToClientRectangle( &dest, &src, &dw->corner, dw->zoom, dw->display );

	palette = ( Palette* )GetWorkspacePalette( dw->workspace );

   palette->h.biSize          = sizeof( BITMAPINFOHEADER );
   palette->h.biWidth         = ( nsint )ns_image_width( image );
   palette->h.biHeight        = ( nsint )ns_image_height( image );
   palette->h.biPlanes        = 1;
   palette->h.biBitCount      = 8;
   palette->h.biCompression   = BI_RGB;
   palette->h.biSizeImage     = 0;
   palette->h.biXPelsPerMeter = 0;
   palette->h.biYPelsPerMeter = 0;
   palette->h.biClrUsed       = 0;
   palette->h.biClrImportant  = 0;

   StretchDIBits(
      dw->backBuffer.hDC,
      dest.left,
      dest.top + ( dest.bottom - dest.top ) - 1,
      ( dest.right  - dest.left ),
      -( dest.bottom - dest.top ),
      ( int )ns_tile_get_pixel_x( tile ),
      ( int )ns_tile_get_pixel_y( tile ),
      ( int )ns_tile_width( tile ),
      ( int )ns_tile_height( tile ),
      ns_image_pixels( image ),
	( BITMAPINFO* )palette,
	DIB_RGB_COLORS,
	SRCCOPY
	);

	if( ____workspace_show_tiles )
		FrameRect( dw->backBuffer.hDC, &dest, GetStockObject( WHITE_BRUSH ) );
	}



//extern nsboolean ____workspace_has_tiles;
//extern nssize ____workspace_async_proj_iterations;


void _DrawDisplayWindowImage
   (
   DisplayWindow  *dw,
   const RECT     *clientViewRect
   )
   {
	RECT            viewport;
	NsTile          tile;
	const NsImage  *image;
	nssize          tile_size;


   //MemoryGraphics   *image;
   //RECT              srcGlobalRect;
   //RECT              srcClientRect;
	//nstimer  start;


   //ns_assert( NULL != dw );
   //ns_assert( NULL != clientViewRect );

   //image    = GetWorkspaceDisplayGraphics( dw->workspace, dw->display );
   //dw->zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   //dw->corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );


	_DisplayWindowClientToGlobalRectangle( &viewport, clientViewRect, &dw->corner, dw->zoom, dw->display );
//ns_println( "Client image rect = %d,%d to %d,%d",
//	( nsint )viewport.left, ( nsint )viewport.top, ( nsint )viewport.right, ( nsint )viewport.bottom );


   // Get the image rectangle in global coordinates.
   //_MakeDisplayWindowGlobalImageRectangle( dw->workspace, dw->display, &srcGlobalRect );

   // Transform this global rectangle to a client rectangle.
   //_DisplayWindowGlobalToClientRectangle( &srcClientRect, &srcGlobalRect, &dw->corner, dw->zoom, dw->display );

   //dw->lock_left = dw->curr_left = srcClientRect.left;
   //dw->lock_top  = dw->curr_top  = srcClientRect.top;


   //if( /*____workspace_has_tiles &&*/ 0 < ____workspace_async_proj_iterations )
   //workspace_lock_tiles( dw->workspace );

	//start = ns_timer();
   /*ns_tile_mgr_render(
      workspace_tile_mgr( dw->workspace, dw->display ),
      _display_window_render_tile_func_1,
      dw
      );*/
	//ns_println( "Render func 1 timer = %f seconds.", ns_difftimer( ns_timer(), start ) );



	//start = ns_timer();

   
	//ns_tile_mgr_render(
     // workspace_tile_mgr( dw->workspace, dw->display ),
      //_display_window_render_tile_func,
      //dw
      //);


	if( NS_XY == dw->display && ____xy_slice_enabled )
		image = workspace_slice_image( dw->workspace );
	else
		image = GetWorkspaceDisplayImage( dw->workspace, dw->display );

	tile_size = _get_optimal_tile_size( ns_image_width( image ), ns_image_height( image ), ( nsdouble )dw->zoom );

	ns_tile_mgr_render_ex(
		&tile,
		image,
		( nsint )tile_size,
		( nsint )tile_size,
		( nsint )viewport.left,
		( nsint )viewport.top,
		( nsint )viewport.right,
		( nsint )viewport.bottom,
      _display_window_render_tile_func,
		dw
      );
//{
//nschar tsb[64];
//ns_snprint( tsb, sizeof( tsb ), "ts=" NS_FMT_ULONG, tile_size );
//TextOut( dw->backBuffer.hDC, 100, 8, tsb, ns_ascii_strlen( tsb ) );
///*TEMP*/ns_println( "tile_size = " NS_FMT_ULONG, tile_size );
//}

	//ns_println( "Render func 2 timer = %f seconds.", ns_difftimer( ns_timer(), start ) );*/

   //workspace_unlock_tiles( dw->workspace );

   /*
   StretchBlt(
      dw->backBuffer.hDC,
      srcClientRect.left,
      srcClientRect.top,
      srcClientRect.right - srcClientRect.left,
      srcClientRect.bottom - srcClientRect.top,
      image->hDC,
      srcGlobalRect.left,
      srcGlobalRect.top,
      srcGlobalRect.right - srcGlobalRect.left,
      srcGlobalRect.bottom - srcGlobalRect.top,
      SRCCOPY
      );*/
   }




/*
void _DrawDisplayWindowImage
   (
   DisplayWindow*  dw,
   const RECT*    clientViewRect
   )
   {
   MemoryGraphics  *image;
   RECT              srcFullGraphicsRect;
   RECT              srcGraphicsRect;
   RECT              srcGlobalRect;
   RECT              srcClientRect;
   RECT              clientInterRect;
   RECT              globalInterRect;
   HWorkspace               workspace;
   nsuint               index;
   nsint            display;
   nsfloat               zoom;
   Vector2i                 corner;
   nslong height;


   ns_assert( NULL != dw );
   ns_assert( NULL != clientViewRect );

   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   image            = GetWorkspaceDisplayGraphics( workspace, display );
   zoom             = GetWorkspaceZoom( workspace, index, display );
   corner = GetWorkspaceCorner( workspace, index, display );

   // Get the image rectangle in global coordinates.
   _MakeDisplayWindowGlobalImageRectangle( workspace, display, &srcGlobalRect );

   // Transform this global rectangle to a client rectangle.
   _DisplayWindowGlobalToClientRectangle( &srcClientRect, &srcGlobalRect, &corner, zoom, display );

   // Get the intersection in the client coordinate system.

   clientInterRect.left   = ( srcClientRect.left   > clientViewRect->left   ) ? 
                           srcClientRect.left   : clientViewRect->left;

   clientInterRect.right  = ( srcClientRect.right  < clientViewRect->right  ) ? 
                            srcClientRect.right  : clientViewRect->right;

   clientInterRect.top    = ( srcClientRect.top    > clientViewRect->top    ) ? 
                            srcClientRect.top    : clientViewRect->top;

   clientInterRect.bottom = ( srcClientRect.bottom < clientViewRect->bottom ) ? 
                             srcClientRect.bottom : clientViewRect->bottom;

   if( clientInterRect.left >= clientInterRect.right ||
       clientInterRect.top >= clientInterRect.bottom )
       return;

   // BUG FIX: 2/25/2003 Due to round off error in converting from global to
   //   client coordinates, just add a "zoom" amount of device pixels to the
   //   intersection. This will ensure that pixels on the right or bottom which
   //   are only partially in view will be drawn.
   clientInterRect.right  += ( nsint )zoom;
   clientInterRect.bottom += ( nsint )zoom;
   
   // Transform the intersection back to the global coordinate system.
   _DisplayWindowClientToGlobalRectangle( &globalInterRect, &clientInterRect, &corner, zoom, display );
   
   // Transform the global intersection to graphics coordinates.
  
   srcGraphicsRect.left = globalInterRect.left - srcGlobalRect.left;

   srcGraphicsRect.right = srcGraphicsRect.left +( globalInterRect.right - globalInterRect.left );

   srcGraphicsRect.top = ( NS_XZ != display ) ?
                         globalInterRect.top - srcGlobalRect.top :
                         srcGlobalRect.top - globalInterRect.top;

   height = globalInterRect.bottom - globalInterRect.top;

   if( height < 0 )
    height = -height; 

   srcGraphicsRect.bottom = ( LONG )( ( nslong )srcGraphicsRect.top + height );

   srcFullGraphicsRect.left = 0;
   srcFullGraphicsRect.top = 0;
   srcFullGraphicsRect.right = ( LONG )image->width;
   srcFullGraphicsRect.bottom = ( LONG )image->height;

   // BUG FIX: 12/31/2002. Due to round off error, make sure the 
   //   source rectangle's left, top are >= 0 and right, bottom
   //   are <= width, height of the image, AND even after clipping
   //   the zoom could be so small that the rectangles width becomes
   //   less than or equal to 0.

   if( srcGraphicsRect.left < srcFullGraphicsRect.left )
      srcGraphicsRect.left = srcFullGraphicsRect.left;

   if( srcGraphicsRect.top < srcFullGraphicsRect.top )
      srcGraphicsRect.top = srcFullGraphicsRect.top;

   if( srcFullGraphicsRect.right < srcGraphicsRect.right )
      srcGraphicsRect.right = srcFullGraphicsRect.right;

   if( srcFullGraphicsRect.bottom < srcGraphicsRect.bottom )
      srcGraphicsRect.bottom = srcFullGraphicsRect.bottom;

   if( ( srcGraphicsRect.right - srcGraphicsRect.left ) <= 0 ||
       ( srcGraphicsRect.bottom - srcGraphicsRect.top ) <= 0   )
      return;

   // BUG FIX: 2/25/2003 Have to keep the pixels in the image square in
   //   terms of the number of device pixels they will eventually occupy
   //   on the screen.
   clientInterRect.right  = clientInterRect.left + 
                            ( nsuint )( ( srcGraphicsRect.right - srcGraphicsRect.left ) * zoom );
   clientInterRect.bottom = clientInterRect.top +
                            ( nsuint )( ( srcGraphicsRect.bottom - srcGraphicsRect.top ) * zoom );
  
   StretchBlt( dw->backBuffer.hDC,
               clientInterRect.left,
               clientInterRect.top,
               clientInterRect.right - clientInterRect.left,
               clientInterRect.bottom - clientInterRect.top,
               image->hDC,
               srcGraphicsRect.left,
               srcGraphicsRect.top,
               srcGraphicsRect.right - srcGraphicsRect.left,
               srcGraphicsRect.bottom - srcGraphicsRect.top,
               SRCCOPY
              );
   }
*/




void _DrawDisplayWindow( DisplayWindow* dw, const RECT* client )
   {/*
   ns_assert( NULL != dw );
   ns_assert( NULL != client );

   switch( dw->display ) 
      {
      case NS_XY:
      case NS_ZY:
      case NS_XZ:*/
    if( WorkspaceDisplayFlagIsSet( dw->workspace, 
                                                 mDISPLAY_GRAPHICS_IMAGE
                                               ) )
         _DrawDisplayWindowImage( dw, client );
  /*       break;

      case eDISPLAY_MONTAGE:
         _DrawDisplayWindowMontage( dw, client );
         break;
      }
*/

   }/* _DrawDisplayWindow() */


void _DestroyDisplayWindowBackBuffer( DisplayWindow* dw )
   {
   ns_assert( NULL != dw );

   if( dw->haveBackBuffer )
      DestroyMemoryGraphics( &dw->backBuffer );

   dw->haveBackBuffer = 0;
   }


void _SetDisplayWindowCursor( HWND hWnd, const nschar* name, const nsint doSet )
   {
   HCURSOR hCursor;

   hCursor = LoadCursor( g_Instance, name );

   SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )hCursor );

   if( doSet )
      SetCursor( hCursor );
   }







void _DisplayWindowMouseModeOnUnset( HWND hWnd )
   {  SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )LoadCursor( NULL, IDC_ARROW ) );  }


void _DisplayWindowMouseModeZoomOnSet( HWND hWnd )
   {
/*   SetUIMouseModeArguments( mVERIFY_DYNAMIC_CAST( window, DisplayWindow )->mouse,
                            eDISPLAY_WINDOW_MOUSE_MODE_ZOOM,
                            ( void* )0
                          ); */
   
   _SetDisplayWindowCursor( hWnd, "ZOOM_IN_TOOL_CURSOR", 0 );
   }


void _DisplayWindowMouseModeDragOnSet( HWND hWnd )
   {   _SetDisplayWindowCursor( hWnd, "DRAG_TOOL_CURSOR", 0 );  }



nsboolean ____dw_show_seed_first;

void _display_window_show_seed_on_status_bar( DisplayWindow *dw )
	{
	Vector3i S;
	NsVector3f V;
	const NsVoxelInfo *voxel_info;


	GetWorkspaceSeed( dw->workspace, &S );
	voxel_info = workspace_get_voxel_info( dw->workspace );
   
	V.x = NS_TO_VOXEL_SPACE( S.x, ns_voxel_info_size_x( voxel_info ) );
	V.y = NS_TO_VOXEL_SPACE( S.y, ns_voxel_info_size_y( voxel_info ) );
	V.z = NS_TO_VOXEL_SPACE( S.z, ns_voxel_info_size_z( voxel_info ) );

	if( ____dw_show_seed_first )
		{
		status_bar_print(
			STATUS_BAR_SEED,
			"seed{(x,y,z) = %d,%d,%d : %.4f,%.4f,%.4f}",
			S.x, S.y, S.z, V.x, V.y, V.z
			);

		____dw_show_seed_first = NS_FALSE;
		}
	else
		status_bar_print_fixed(
			STATUS_BAR_SEED,
			"seed{(x,y,z) = %d,%d,%d : %.4f,%.4f,%.4f}",
			S.x, S.y, S.z, V.x, V.y, V.z
			);
	}

void _show_seed_on_status_bar( void )
	{  _display_window_show_seed_on_status_bar( s_DisplayWindows );  }


void _DisplayWindowMouseModeSeedOnSet( HWND hWnd )
   {
	_SetDisplayWindowCursor( hWnd, "SEED_TOOL_CURSOR", 0 );

	____dw_show_seed_first = NS_TRUE;
	_display_window_show_seed_on_status_bar( _MapHandleToDisplayWindow( hWnd ) );
	}

void _DisplayWindowMouseModeSeedOnUnset( HWND hWnd )
	{
	_SetDisplayWindowCursor( hWnd, IDC_ARROW, 0 );
	SetStatusBarText( "", STATUS_BAR_SEED );

	____dw_show_seed_first = NS_TRUE;
	}





extern void _status_clear_measurement( void );
extern void _status_measurement( void );

void _DisplayWindowMouseModeRaysOnUnset( HWND hWnd )
	{
	_SetDisplayWindowCursor( hWnd, IDC_ARROW, 0 );
	_status_clear_measurement();
	}

void _DisplayWindowMouseModeRaysOnSet( HWND hWnd )
   {
	_SetDisplayWindowCursor( hWnd, "RAY_TOOL_CURSOR", 0 );
	_status_measurement();
	}




void _DisplayWindowMouseModeSpineOnSet( HWND hWnd )
   {  _SetDisplayWindowCursor( hWnd, "SPINE_TOOL_CURSOR", 0 );  }


void _DisplayWindowMouseModeNeuriteOnSet( HWND hWnd )
   {  _SetDisplayWindowCursor( hWnd, "NEURITE_TOOL_CURSOR", 0 );  }


void _DisplayWindowMouseModeMagnetOnSet( HWND hWnd )
   {  _SetDisplayWindowCursor( hWnd, "MAGNET_TOOL_CURSOR", 0 );  }





nsboolean ____dw_show_roi_first;

void _do_show_roi_on_status_bar( nsuint workspace )
	{
	const NsCubei *roi;

	roi = workspace_update_roi( workspace );

	if( ____dw_show_roi_first )
		{
		status_bar_print(
			STATUS_BAR_ROI,
			"roi{(x,y,z) = %d,%d,%d (w,h,l) = %d,%d,%d}",
			roi->C1.x, roi->C1.y, roi->C1.z,
			ns_cubei_width( roi ), ns_cubei_height( roi ), ns_cubei_length( roi )
			);

		____dw_show_roi_first = NS_FALSE;
		}
	else
		status_bar_print_fixed(
			STATUS_BAR_ROI,
			"roi{(x,y,z) = %d,%d,%d (w,h,l) = %d,%d,%d}",
			roi->C1.x, roi->C1.y, roi->C1.z,
			ns_cubei_width( roi ), ns_cubei_height( roi ), ns_cubei_length( roi )
			);
	}

void _DisplayWindowMouseModeROIOnSet( HWND hWnd )
   {
	DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

	dw->roi_cursors[ _ROI_CURSOR_NW    ] = LoadCursor( NULL, IDC_SIZENWSE );
	dw->roi_cursors[ _ROI_CURSOR_SE    ] = LoadCursor( NULL, IDC_SIZENWSE );
	dw->roi_cursors[ _ROI_CURSOR_NE    ] = LoadCursor( NULL, IDC_SIZENESW );
	dw->roi_cursors[ _ROI_CURSOR_SW    ] = LoadCursor( NULL, IDC_SIZENESW );
	dw->roi_cursors[ _ROI_CURSOR_W     ] = LoadCursor( NULL, IDC_SIZEWE );
	dw->roi_cursors[ _ROI_CURSOR_E     ] = LoadCursor( NULL, IDC_SIZEWE );
	dw->roi_cursors[ _ROI_CURSOR_N     ] = LoadCursor( NULL, IDC_SIZENS );
	dw->roi_cursors[ _ROI_CURSOR_S     ] = LoadCursor( NULL, IDC_SIZENS );
	dw->roi_cursors[ _ROI_CURSOR_ALL   ] = LoadCursor( NULL, IDC_SIZEALL );
	dw->roi_cursors[ _ROI_CURSOR_ARROW ] = LoadCursor( NULL, IDC_ARROW );

   SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )dw->roi_cursors[ _ROI_CURSOR_ARROW ] );
	//_SetDisplayWindowCursor( hWnd, IDC_ARROW, 0 );

	dw->roi_curr_cursor = _ROI_CURSOR_ARROW;

	____dw_show_roi_first = NS_TRUE;
	_do_show_roi_on_status_bar( dw->workspace );
	}

void _DisplayWindowMouseModeROIOnUnset( HWND hWnd )
	{
	_SetDisplayWindowCursor( hWnd, IDC_ARROW, 0 );
	SetStatusBarText( "", STATUS_BAR_ROI );

	____dw_show_roi_first = NS_TRUE;
	}




void _GetDisplayWindowRectangles
   ( 
   DisplayWindow*  dw,
   RECT*           imageClientRect,
   RECT*           imageGlobalRect,
   RECT*           viewportClientRect,
   RECT*           viewportGlobalRect
   )
   {
   HWorkspace      workspace;
   nsuint        index;
   nsint   display;
   Vector2i        corner;
   nsfloat           zoom;


   workspace = dw->workspace;
   index     = dw->index;
   display   = dw->display;
   corner    = GetWorkspaceCorner( workspace, index, display );
   zoom      = GetWorkspaceZoom( workspace, index, display );


   GetClientRect( dw->hWnd, viewportClientRect );

   _DisplayWindowClientToGlobalRectangle( viewportGlobalRect,
                                          viewportClientRect,
                                          &corner,
                                          zoom,
                                          display
                                        );

   _MakeDisplayWindowGlobalImageRectangle( workspace, display, imageGlobalRect );

   _DisplayWindowGlobalToClientRectangle( imageClientRect,
                                          imageGlobalRect,
                                          &corner,
                                          zoom,
                                          display
                                       );
   }


void _CenterDisplayWindowImage
   (
   DisplayWindow  *dw,
   RECT           *imageClientRect,
   RECT           *imageGlobalRect,
   RECT           *viewportClientRect,
   const nsint     horizontally
   )
   {
   HWorkspace  workspace;
   nsuint      index;
   nsint       display;
   Vector2i    corner;
   nsfloat     zoom;
   Vector2i    imageOldGlobalCorner;
   Vector2i    imageNewClientCorner;
   Vector2i    imageNewGlobalCorner;
   Vector2i    difference;


   workspace = dw->workspace;
   index     = dw->index;
   display   = dw->display;
   zoom      = GetWorkspaceZoom( workspace, index, display );

   MakeVector2i( &imageOldGlobalCorner,
                 imageGlobalRect->left,
                imageGlobalRect->top
               );

   if( horizontally )
      CenterRectangleHoriz( imageClientRect, viewportClientRect );
   else
      CenterRectangleVert( imageClientRect, viewportClientRect );

   MakeVector2i( &imageNewClientCorner,
                 imageClientRect->left,
                 imageClientRect->top
               );

   corner = GetWorkspaceCorner( workspace, index, display );

   _DisplayWindowClientToGlobalVector( &imageNewGlobalCorner,
                                       &imageNewClientCorner,
                                       &corner,
                                       zoom,
                                       display
                                     );

   SubVector2i( &imageOldGlobalCorner, &imageNewGlobalCorner, &difference );
   AddVector2i( &corner, &difference, &corner );
   SetWorkspaceCorner( workspace, index, display, corner );
   }


void display_window_center_image( void )
	{
	DisplayWindow  *dw;
   RECT            imageClientRect;
   RECT            imageGlobalRect;
   RECT            viewportClientRect;
   RECT            viewportGlobalRect;


	dw = &(s_DisplayWindows[0]);

	_GetDisplayWindowRectangles(
		dw,
		&imageClientRect,
		&imageGlobalRect,
		&viewportClientRect,
		&viewportGlobalRect
		);

   _CenterDisplayWindowImage(
		dw,
		&imageClientRect,
		&imageGlobalRect,
		&viewportClientRect,
		1
		);

   _CenterDisplayWindowImage(
		dw, 
		&imageClientRect,
		&imageGlobalRect,
		&viewportClientRect,
		0
		);
	}


void _LimitDisplayWindowImageHoriz
   (
   DisplayWindow*  dw,
   RECT*    imageGlobalRect,
   RECT*    viewportGlobalRect
   )
   {
   HWorkspace     workspace;
   nsuint     index;
   nsint  display;
   Vector2i       corner;
   Vector2i       difference;


   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   if( viewportGlobalRect->left < imageGlobalRect->left )
      {
      MakeVector2i( &difference,
                    imageGlobalRect->left - viewportGlobalRect->left,
                    0
                  );

      corner = GetWorkspaceCorner( workspace, index, display );
      AddVector2i( &corner, &difference, &corner );
      SetWorkspaceCorner( workspace, index, display, corner );
      }
   else if( imageGlobalRect->right < viewportGlobalRect->right )
      {
      MakeVector2i( &difference,
                    imageGlobalRect->right - viewportGlobalRect->right,
                    0
                  );

      corner = GetWorkspaceCorner( workspace, index, display );
      AddVector2i( &corner, &difference, &corner );
      SetWorkspaceCorner( workspace, index, display, corner );
      }
   }


void _LimitDisplayWindowImageVert
   (
   DisplayWindow*  dw,
   RECT*    imageGlobalRect,
   RECT*    viewportGlobalRect
   )
   {
   HWorkspace     workspace;
   nsuint     index;
   nsint  display;
   Vector2i       corner;
   Vector2i       difference;
   nsint      outOfBounds;


   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   //if( NS_XZ != display )
      outOfBounds = ( viewportGlobalRect->top < imageGlobalRect->top )
                    ? 1 : 0;
//   else
  //    outOfBounds = ( imageGlobalRect->top < viewportGlobalRect->top )
    //                ? 1 : 0;  

   if( outOfBounds )
      {
      MakeVector2i( &difference,
                    0,
                    imageGlobalRect->top - viewportGlobalRect->top
                  );

      corner = GetWorkspaceCorner( workspace, index, display );
      AddVector2i( &corner, &difference, &corner );
      SetWorkspaceCorner( workspace, index, display, corner );
      }
   else
      {
      //if( NS_XZ != display )
         outOfBounds = ( imageGlobalRect->bottom < viewportGlobalRect->bottom )
                       ? 1 : 0;
      //else
        // outOfBounds = ( viewportGlobalRect->bottom < imageGlobalRect->bottom )
          //             ? 1 : 0;  

      if( outOfBounds )
         {
         MakeVector2i( &difference,
                       0,
                       imageGlobalRect->bottom - viewportGlobalRect->bottom
                     );

         corner = GetWorkspaceCorner( workspace, index, display );
         AddVector2i( &corner, &difference, &corner );
         SetWorkspaceCorner( workspace, index, display, corner );
         }
      }
   }


void _CenterOrLimitDisplayWindowImage( DisplayWindow* dw )
   {
   if( ____2d_display_center_and_limit_image )
      {
      RECT  imageClientRect;
      RECT  imageGlobalRect;
      RECT  viewportClientRect;
      RECT  viewportGlobalRect;


      _GetDisplayWindowRectangles( dw,
                                   &imageClientRect,
                                   &imageGlobalRect,
                                   &viewportClientRect, 
                                   &viewportGlobalRect
                                 );

      if( ( imageClientRect.right - imageClientRect.left ) <= 
          ( viewportClientRect.right - viewportClientRect.left ) )
         _CenterDisplayWindowImage( dw, 
                                    &imageClientRect,
                                    &imageGlobalRect,
                                    &viewportClientRect,
                                    1
                                  );
      else  
         _LimitDisplayWindowImageHoriz( dw,
                                        &imageGlobalRect,
                                        &viewportGlobalRect
                                      );

      _GetDisplayWindowRectangles( dw,
                                   &imageClientRect,
                                   &imageGlobalRect,
                                   &viewportClientRect, 
                                   &viewportGlobalRect
                                 );

      if( ( imageClientRect.bottom - imageClientRect.top ) <= 
          ( viewportClientRect.bottom - viewportClientRect.top ) )
         _CenterDisplayWindowImage( dw, 
                                    &imageClientRect,
                                    &imageGlobalRect,
                                    &viewportClientRect,
                                    0
                                  );
      else  
         _LimitDisplayWindowImageVert( dw,
                                       &imageGlobalRect,
                                       &viewportGlobalRect
                                     );
      }
   }


void FitDisplayWindowImage( DisplayWindow* dw, nsint dont_repaint )
   {

   HWorkspace      workspace;
   nssize      index;
   nsint   display;
   const NsImage    *image;
   RECT     clientRect;
   nssize       imageWidth;
   nssize       imageHeight;
   nssize       clientWidth;
   nssize       clientHeight;
   nsfloat      widthZoom;
   nsfloat      heightZoom;
   nsfloat      zoom;


   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   //if( eDISPLAY_MONTAGE == display )
     // return;

   GetClientRect( dw->hWnd, &clientRect );

   clientWidth  = clientRect.right - clientRect.left;
   clientHeight = clientRect.bottom - clientRect.top;
   
   image       = GetWorkspaceDisplayImage( workspace, display );
   imageWidth  = ns_image_width( image );
   imageHeight = ns_image_height( image );

   widthZoom  = ( nsfloat )clientWidth  / ( nsfloat )imageWidth;
   heightZoom = ( nsfloat )clientHeight / ( nsfloat )imageHeight;

   zoom = ( widthZoom < heightZoom ) ? widthZoom : heightZoom;

   if( zoom < _mMIN_ZOOM_FACTOR )
      zoom = _mMIN_ZOOM_FACTOR;
   else if( _mMAX_ZOOM_FACTOR < zoom )
      zoom = _mMAX_ZOOM_FACTOR;

   SetWorkspaceZoom( workspace,
                     ( nsuint )index,
                     display,
                     zoom
                   );

   _CenterOrLimitDisplayWindowImage( dw );

   if( ! dont_repaint )
      DrawDisplayWindow( dw->hWnd );
   }





static nsint ____zooming_in = 1;
#define ____allow_drag_rect ____zooming_in


void _zoom_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   DisplayWindow  *dw;
   //Vector2i        corner;
   //Vector2i        min;
   //Vector2i        max;
   //Vector2i        global;
   //nsfloat         zoom;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );
/*
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );

   global.x = 0;
   global.y = 0;

   _DisplayWindowGlobalToClientVector(
      &min,
      &global,
      &corner,
      zoom,
      dw->display
      );

   global.x = ( nsint )ns_image_width( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );
   global.y = ( nsint )ns_image_height( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );

   _DisplayWindowGlobalToClientVector(
      &max,
      &global,
      &corner,
      zoom,
      dw->display
      );
*/

   /* 
   ns_drag_rect_bounds(
      &dw->drag_rect,
      ( nsint )min.x,
      ( nsint )min.y,
      ( nsint )max.x,
      ( nsint )max.y
      );
   */
   ns_drag_rect_bounds(
      &dw->drag_rect,
      NS_INT_MIN,
      NS_INT_MIN,
      NS_INT_MAX,
      NS_INT_MAX
      );

   SetCapture( hWnd );
   dw->hasCapture = 1;

   if( ____allow_drag_rect )
      {
      ns_drag_rect_start( &dw->drag_rect, x, y );

      ns_drag_rect_set_tolerance(
         &dw->drag_rect,
         eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode ? 6 : 0
         );

      ____drag_rect_active = NS_TRUE;
      }

   dw->isDragging = 1;
   }


void _zoom_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   DisplayWindow *dw;
   dw = _MapHandleToDisplayWindow( hWnd );

   if( ____allow_drag_rect && dw->isDragging )
      {
      DragRectRenderer state;
   
      drag_rect_renderer_init( &state, hWnd );

      ns_drag_rect_erase( &dw->drag_rect, drag_rect_renderer_line, &state );
      ns_drag_rect_end( &dw->drag_rect, x, y );
      ns_drag_rect_draw( &dw->drag_rect, drag_rect_renderer_line, &state );

      drag_rect_renderer_finalize( &state );
      }
   }


void _display_window_do_zoom_rect( DisplayWindow *dw )
   {
   nsfloat   widthZoom, heightZoom;
   nsfloat   zoom, less_than_one_ratio;
   nslong    offset;
   RECT      clientRect;
   Vector2i  corner, global;
   nsint     zoom_global_width, zoom_global_height;
	nssize    clientWidth, clientHeight;


   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

   ns_drag_rect_dimensions( &dw->zoom_rect, &zoom_global_width, &zoom_global_height );


//ns_println( "zoom_rect = %d,%d, %d,%d",
//   dw->zoom_rect.start_x, dw->zoom_rect.start_y, dw->zoom_rect.end_x, dw->zoom_rect.end_y );


   GetClientRect( dw->hWnd, &clientRect );

   clientWidth  = clientRect.right - clientRect.left;
   clientHeight = clientRect.bottom - clientRect.top;

   widthZoom  = ( nsfloat )clientWidth  / ( nsfloat )zoom_global_width;
   heightZoom = ( nsfloat )clientHeight / ( nsfloat )zoom_global_height;

   if( widthZoom < heightZoom )
      {
      //ns_println( "width zoom less" );

      less_than_one_ratio = 1.0f - widthZoom / heightZoom;
      offset = ( nslong )( ( nsfloat )zoom_global_width * less_than_one_ratio );

      global.x = ns_drag_rect_start_x( &dw->zoom_rect );

      //if( NS_XZ != dw->display )
         global.y = ns_drag_rect_start_y( &dw->zoom_rect ) - offset;
      //else
        // global.y = ns_drag_rect_end_y( &dw->zoom_rect ) + offset;
         
      zoom = widthZoom;
      }
   else
      {
      //ns_println( "height zoom less" );

      less_than_one_ratio = 1.0f - heightZoom / widthZoom;
      offset = ( nslong )( ( nsfloat )zoom_global_height * less_than_one_ratio );

      global.x = ns_drag_rect_start_x( &dw->zoom_rect ) - offset;

//      if( NS_XZ != dw->display )
         global.y = ns_drag_rect_start_y( &dw->zoom_rect );
  //    else
    //     global.y = ns_drag_rect_end_y( &dw->zoom_rect );

      zoom = heightZoom;
      }

   if( zoom < _mMIN_ZOOM_FACTOR )
      zoom = _mMIN_ZOOM_FACTOR;
   else if( _mMAX_ZOOM_FACTOR < zoom )
      zoom = _mMAX_ZOOM_FACTOR;

   SetWorkspaceZoom( dw->workspace,
                     ( nsuint )dw->index,
                     dw->display,
                     zoom
                   );

   SetWorkspaceCorner( dw->workspace, dw->index, dw->display, global );

   _CenterOrLimitDisplayWindowImage( dw );

   DrawDisplayWindow( dw->hWnd );
   }



#define ____TO_IMAGE_SPACE( dw, V, C )\
   (V).x = (C).x + ( nslong )( (V).x / (dw)->zoom );\
   (V).y = /*NS_XZ == (dw)->display ?*/\
            /*(C).y - ( nslong )( (V).y / dw->zoom ) :*/\
            (C).y + ( nslong )( (V).y / dw->zoom )



nsboolean _on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, NsAABBox3d *B )
   {
   DisplayWindow     *dw;
   DragRectRenderer   state;
	NsVector2i         corner;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   if( ____allow_drag_rect )
      {
      ____drag_rect_active = NS_FALSE;

      drag_rect_renderer_init( &state, hWnd );

      ns_drag_rect_erase( &dw->drag_rect, drag_rect_renderer_line, &state );
      ns_drag_rect_end( &dw->drag_rect, x, y );

      drag_rect_renderer_finalize( &state );
      }

   if( dw->hasCapture )
      {
      dw->hasCapture = 0;
      ReleaseCapture();
      }

   dw->isDragging = 0;

   if( ____allow_drag_rect && ns_drag_rect_valid( &dw->drag_rect ) )
      {
      ns_drag_rect_clip( &dw->drag_rect );
      ns_drag_rect_normalize( &dw->drag_rect );

      if( ns_drag_rect_valid( &dw->drag_rect ) )
         {
			if( eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode )
				{
				Vector2i V, C;

				dw->zoom_rect = dw->drag_rect;
   
				C = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

				/* Convert the zoom rectangle from client space to image space. */
				V.x = ns_drag_rect_start_x( &dw->zoom_rect );
				V.y = ns_drag_rect_start_y( &dw->zoom_rect );
				____TO_IMAGE_SPACE( dw, V, C );
				ns_drag_rect_start( &dw->zoom_rect, ( nsint )V.x, ( nsint )V.y );

				V.x = ns_drag_rect_end_x( &dw->zoom_rect );
				V.y = ns_drag_rect_end_y( &dw->zoom_rect );
				____TO_IMAGE_SPACE( dw, V, C );
				ns_drag_rect_end( &dw->zoom_rect, ( nsint )V.x, ( nsint )V.y );


	//ns_println( "zoom rect in image space = %d,%d to %d,%d",
	//	dw->zoom_rect.start_x, dw->zoom_rect.start_y, dw->zoom_rect.end_x, dw->zoom_rect.end_y );

				/* Clip and normalize the rectangle again,
					but in image space this time! */

				/*ns_drag_rect_bounds(
					&dw->zoom_rect,
					0,
					0,
					( nsint )ns_image_width( GetWorkspaceDisplayImage( dw->workspace, dw->display ) ),
					( nsint )ns_image_height( GetWorkspaceDisplayImage( dw->workspace, dw->display ) )
					);*/
				
				ns_drag_rect_bounds(
				   &dw->zoom_rect,
				   NS_INT_MIN,
				   NS_INT_MIN,
				   NS_INT_MAX,
				   NS_INT_MAX
				   );

				ns_drag_rect_clip( &dw->zoom_rect );
				ns_drag_rect_normalize( &dw->zoom_rect );
				}

			corner.x = dw->corner.x;
			corner.y = dw->corner.y;

         if( NULL != B )
            ns_model_client_space_drag_rect_to_aabbox(
					&dw->drag_rect,
					workspace_get_voxel_info( dw->workspace ),
					dw->display,
					&corner,
					dw->zoom,
					B
					);

         return NS_TRUE;
         }
      }

   return NS_FALSE;
   }



void _zoom_on_lbutton_up
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   Vector2i        clientCoord;
   Vector2i        oldGlobalCoord;
   Vector2i        newGlobalCoord;
   Vector2i        difference;
   Vector2i        corner;
   HWorkspace      workspace;
   nsuint      index;
   nsint   display;
   nsfloat      zoom;
   DisplayWindow  *dw;
   nsint       zoomOut;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );


   if( _on_lbutton_up_do_drag_rect( hWnd, x, y, NULL ) )
      {
      _display_window_do_zoom_rect( dw );
      return;
      }

   workspace = dw->workspace;
   index     = dw->index;
   display   = dw->display;

   //( eDISPLAY_MONTAGE == display )
     //eturn;

   zoom    = GetWorkspaceZoom( workspace, index, display );
   zoomOut = ( nsint )!____zooming_in;

   /* If at maximum or minimum zoom then just return. */

   if( _mMAX_ZOOM_FACTOR == zoom && ! zoomOut )
      return;

   if( _mMIN_ZOOM_FACTOR == zoom && zoomOut )
      return;

   corner = GetWorkspaceCorner( workspace, index, display );

   MakeVector2i( &clientCoord, x, y );

   /* Change zoom factor and then see calculate how much
      the view needs to be translated. */

   _DisplayWindowClientToGlobalVector( &oldGlobalCoord, 
                                       &clientCoord,
                                       &corner,
                                       zoom,
                                       display
                                     );

   if( zoomOut )
      _GetClosestSmallerZoomFactor( zoom, &zoom );
   else
      _GetClosestLargerZoomFactor( zoom, &zoom );

   _DisplayWindowClientToGlobalVector( &newGlobalCoord,
                                       &clientCoord,
                                       &corner,
                                       zoom,
                                       display
                                     );

   SubVector2i( &oldGlobalCoord, &newGlobalCoord, &difference );
      
   /* Translate the upper left corner so the spot in the workspace where
      the user clicked is still at that spot after zooming. */

   AddVector2i( &corner, &difference, &corner );

   SetWorkspaceZoom( workspace, index, display, zoom );
   SetWorkspaceCorner( workspace, index, display, corner );

   _CenterOrLimitDisplayWindowImage( dw );
   DrawDisplayWindow( hWnd );
   }





#define ____FIT_IMAGE_ID      76990
#define ____ACTUAL_PIXELS_ID  76991
#define ____ZOOM_IN_ID        76992
#define ____ZOOM_OUT_ID       76993

extern void _MainWindowOnFitImages( void );
extern void _on_actual_pixels( void );

void _zoom_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   MENUITEMINFO    miim;
   HMENU           hMenu;
   POINT           pt;
   nsint           id;
   nsfloat         zoom;
   DisplayWindow  *dw;
   nsint           save_zooming_in;


   if( ! ____zooming_in )
      return;

   dw = _MapHandleToDisplayWindow( hWnd );

   hMenu = CreatePopupMenu();

   miim.cbSize = sizeof( MENUITEMINFO );
   miim.fMask  = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   miim.fType  = MFT_STRING;
   miim.fState = MFS_ENABLED;

   miim.wID        = ____FIT_IMAGE_ID;
   miim.dwTypeData = "Fit Image";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Fit Image" );
   InsertMenuItem( hMenu, 0, TRUE, &miim );

   miim.wID        = ____ACTUAL_PIXELS_ID;
   miim.dwTypeData = "Actual Pixels";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Actual Pixels" );
   InsertMenuItem( hMenu, 1, TRUE, &miim );

   miim.fType = MFT_SEPARATOR;
   InsertMenuItem( hMenu, 2, TRUE, &miim );

   miim.fType = MFT_STRING;

   zoom = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );

   miim.fState     = _mMAX_ZOOM_FACTOR == zoom ? MFS_DISABLED | MFS_GRAYED : MFS_ENABLED;
   miim.wID        = ____ZOOM_IN_ID;
   miim.dwTypeData = "Zoom In";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Zoom In" );
   InsertMenuItem( hMenu, 3, TRUE, &miim );

   miim.fState     = _mMIN_ZOOM_FACTOR == zoom ? MFS_DISABLED | MFS_GRAYED : MFS_ENABLED;
   miim.wID        = ____ZOOM_OUT_ID;
   miim.dwTypeData = "Zoom Out";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Zoom Out" );
   InsertMenuItem( hMenu, 4, TRUE, &miim );

   pt.x = x;
   pt.y = y;

   ClientToScreen( hWnd, &pt );

   id = TrackPopupMenu( hMenu,                  
                        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                        pt.x,
                        pt.y,
                        0,
                        hWnd,
                        NULL
                      );

   DestroyMenu( hMenu );

   switch( id )
      {
      case ____FIT_IMAGE_ID:
         _MainWindowOnFitImages();
         break;

      case ____ACTUAL_PIXELS_ID:
         _on_actual_pixels();
         break;

      case ____ZOOM_IN_ID:
         save_zooming_in = ____zooming_in;
         ____zooming_in     = 1;

         _zoom_on_lbutton_down( hWnd, x, y, 0 );
         _zoom_on_lbutton_up( hWnd, x, y, 0 );

         ____zooming_in = save_zooming_in;
         break;

      case ____ZOOM_OUT_ID:
         save_zooming_in = ____zooming_in;
         ____zooming_in     = 0;

         _zoom_on_lbutton_down( hWnd, x, y, 0 );
         _zoom_on_lbutton_up( hWnd, x, y, 0 );

         ____zooming_in = save_zooming_in;
         break;
      }
   }



void ____set_display_window_actual_pixels( void )
   {
   Vector2i        clientCoord;
   Vector2i        oldGlobalCoord;
   Vector2i        newGlobalCoord;
   Vector2i        difference;
   Vector2i        corner;
   HWorkspace      workspace;
   nsuint      index;
   nsint   display;
   nsint   x, y;
   nsfloat      zoom;
   DisplayWindow  *dw;
   RECT rc;
	nsboolean old_value;


	/* HACK There seems to be some sort of bug if "Center & Limit Image" is not set. */
	old_value = ____2d_display_center_and_limit_image;
	____2d_display_center_and_limit_image = NS_TRUE;


dw = s_DisplayWindows + 0;

   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   zoom = GetWorkspaceZoom( workspace, index, display );
   //zoomOut = ( nsint )!____zooming_in;

   /* If at maximum or minimum zoom then just return. */

   //if( _mMAX_ZOOM_FACTOR == zoom && ! zoomOut )
     // return;

   //if( _mMIN_ZOOM_FACTOR == zoom && zoomOut )
     // return;

   corner = GetWorkspaceCorner( workspace, index, display );

   GetClientRect( dw->hWnd, &rc );
   x = rc.right / 2;
   y = rc.bottom / 2;
   MakeVector2i( &clientCoord, x, y );

   /* Change zoom factor and then see calculate how much
      the view needs to be translated. */

   _DisplayWindowClientToGlobalVector( &oldGlobalCoord, 
                                       &clientCoord,
                                       &corner,
                                       zoom,
                                       display
                                     );

   zoom = s_ZoomFactors[ _eZOOM_100p0 ];
   //if( zoomOut )
      //_GetClosestSmallerZoomFactor( zoom, &zoom );
   //else
     // _GetClosestLargerZoomFactor( zoom, &zoom );

   _DisplayWindowClientToGlobalVector( &newGlobalCoord,
                                       &clientCoord, 
                                       &corner,
                                       zoom,
                                       display
                                     );

   SubVector2i( &oldGlobalCoord, &newGlobalCoord, &difference );
      
   /* Translate the upper left corner so the spot in the workspace where
      the user clicked is still at that spot after zooming. */

   AddVector2i( &corner, &difference, &corner );

   SetWorkspaceZoom( workspace, index, display, zoom );
   SetWorkspaceCorner( workspace, index, display, corner );

   _CenterOrLimitDisplayWindowImage( dw );

	____2d_display_center_and_limit_image = old_value;

   DrawDisplayWindow( dw->hWnd );
   }



void ____mouse_mode_rays_get_point(
   DisplayWindow *dw, nsint x, nsint y, NsVector3i *Ps, NsVector3f *Pf, nsboolean clip );


void _display_window_update_transform_center( DisplayWindow *dw )
   {
   NsVector3f  I;
	NsVector2f corner, O;


	if( 0 < ns_model_transform_size( &dw->model_xfrm ) )
		ns_model_transform_center( &dw->model_xfrm, &I );
	else
		ns_spines_transform_center( &dw->spines_xfrm, &I );

   ns_to_image_space_ex( &I, &I, workspace_get_voxel_info( dw->workspace ) );

	I.x += .5f; I.y += .5f; I.z += .5f;

	switch( dw->display ){
		case NS_XY: O.x = I.x; O.y = I.y; break;
		case NS_ZY: O.x = I.z; O.y = I.y; break;
		case NS_XZ: O.x = I.x; O.y = I.z; break;
		default: ns_assert_not_reached(); }

	corner.x = ( nsfloat )dw->corner.x;
	corner.y = ( nsfloat )dw->corner.y;

	____TO_CLIENT_SPACE( dw, O, corner );

	dw->xfrm_center_x = ( nsint )O.x;
   dw->xfrm_center_y = ( nsint )O.y;
   }


void _display_window_update_transform_radius( DisplayWindow *dw )
   {
   NsVector3f  R/*, C*/;
   nsfloat     zoom;


   zoom  = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );

	if( 0 < ns_model_transform_size( &dw->model_xfrm ) )
		{
		ns_model_transform_radius( &dw->model_xfrm, &R );

		/* If both vertices and spines are selected... then adjust radius. */
	//	if( 0 < ns_spines_transform_size( &dw->spines_xfrm ) )
	//		{
			/*TEMP*///ns_println( "VERTICES AND SPINES ARE SELECTED. ADJUST RADIUS!" );

	//		ns_model_transform_center( &dw->model_xfrm, &C );

			/* Find the furthest spine from the transform center of the vertices. */
	//		ns_spines_transform_furthest( &dw->spines_xfrm, &C, &R );

			/* Make that position into a vector (radius). */
	//		ns_vector3f_cmpd_sub( &R, &C );
	//		}
		}
	else
		ns_spines_transform_radius( &dw->spines_xfrm, &R );

   ns_to_image_space_ex( &R, &R, workspace_get_voxel_info( dw->workspace ) );

   //R.x -= .5f;
   //R.y -= .5f;
   //R.z -= .5f;

   ns_vector3f_cmpd_scale( &R, zoom );

   dw->xfrm_radius = ( nsint )NS_MAX3( R.x, R.y, R.z );
   }


nsint _dw_get_slice_index_in_client_space_zy( DisplayWindow *dw )
	{
	NsVector2f  A, B, C;
	Vector2i    corner;


	A.x = ( nsfloat )____xy_slice_index;
	A.y = 0.0f;

	B.x = ( nsfloat )( ____xy_slice_index + 1 );
	B.y = 0.0f;

	corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
	C.x    = ( nsfloat )corner.x;
	C.y    = ( nsfloat )corner.y;

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );

	return ( nsint )( ( A.x + B.x ) / 2.0f );
	}


nsint _dw_get_slice_index_in_client_space_xz( DisplayWindow *dw )
	{
	NsVector2f  A, B, C;
	Vector2i    corner;


	A.x = 0.0f;
	A.y = ( nsfloat )____xy_slice_index;

	B.x = 0.0f;
	B.y = ( nsfloat )( ____xy_slice_index + 1 );

	corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
	C.x    = ( nsfloat )corner.x;
	C.y    = ( nsfloat )corner.y;

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );

	return ( nsint )( ( A.y + B.y ) / 2.0f );
	}


extern void _xy_slice_viewer_draw( void );





void _display_window_client_to_voxel_space( DisplayWindow *dw, nsint x, nsint y, NsVector3f *P )
	{
	NsPoint2i in, corner;

	in.x = x;
	in.y = y;

	corner.x = dw->corner.x;
	corner.y = dw->corner.y;

	ns_client_to_voxel_space(
		&in,
		P,
		workspace_get_voxel_info( dw->workspace ),
		dw->display,
		&corner,
		dw->zoom
		);
	}


void _display_window_on_translation( DisplayWindow *dw, nsint x, nsint y )
	{
   NsVector3f T;

	_display_window_client_to_voxel_space( dw, x, y, &dw->P2f ); 
   //____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_FALSE );

   ns_vector3f_sub( &T, &dw->P2f, &dw->P1f );

   ns_model_translate_selected_vertices_apply( &dw->model_xfrm, &T );
	ns_spines_translate_selected_apply( &dw->spines_xfrm, &T );

   _display_window_update_transform_center( dw );

   ____redraw_all();
	}


void _display_window_on_rotation( DisplayWindow *dw, nsint x, nsint y )
	{
   _display_window_client_to_voxel_space( dw, x, y, &dw->P2f );
   //____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_FALSE );

   ns_model_rotate_selected_vertices_apply( &dw->model_xfrm, &dw->P2f );
	ns_spines_rotate_selected_apply( &dw->spines_xfrm, &dw->P2f );

   ____redraw_all();
	}


void _DisplayWindowMouseModeDragOnMouseMove
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

   if( dw->isDragging )
      {
      switch( dw->drag_mode )
         {
         case _DRAG_MODE_NORMAL:
            {
            HWorkspace     workspace;
            nsuint     index;
            nsint  display;
            Vector2i       oldClientCoord;
            Vector2i       newClientCoord;
            Vector2i       oldGlobalCoord;
            Vector2i       newGlobalCoord;
            Vector2i       difference;
            Vector2i       corner;
            nsfloat     zoom;


            workspace = dw->workspace;
            index      = dw->index;
            display    = dw->display;

            //if( eDISPLAY_MONTAGE == display )
              // return;

            corner = GetWorkspaceCorner( workspace, index, display );
            zoom             = GetWorkspaceZoom( workspace, index, display );

            oldClientCoord = dw->clientCoord;
            MakeVector2i( &newClientCoord, x, y );

            _DisplayWindowClientToGlobalVector( &oldGlobalCoord,
                                                &oldClientCoord,
                                                &corner,
                                                zoom,
                                                display
                                              );

            _DisplayWindowClientToGlobalVector( &newGlobalCoord,
                                                &newClientCoord, 
                                                &corner,
                                                zoom,
                                                display
                                              );

            /* Translate the view corner. */

            SubVector2i( &oldGlobalCoord, &newGlobalCoord, &difference );
            AddVector2i( &corner, &difference, &corner );

            SetWorkspaceCorner( workspace, index, display, corner );

            dw->clientCoord = newClientCoord;

            _CenterOrLimitDisplayWindowImage( dw );
            DrawDisplayWindow( hWnd );
            }
         break;

         case _DRAG_MODE_TRANSLATE:
            _display_window_on_translation( dw, x, y );
            break;

         case _DRAG_MODE_ROTATE:
				_display_window_on_rotation( dw, x, y );
            break;

			case _DRAG_MODE_SLICE_BAR:
				{
				const NsCubei *roi;

            ____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_FALSE );
				roi = workspace_update_roi( dw->workspace );

				if( dw->P2i.z < roi->C1.z )
					dw->P2i.z = roi->C1.z;
				else if( dw->P2i.z > roi->C2.z )
					dw->P2i.z = roi->C2.z;

				____xy_slice_index = ( nssize )dw->P2i.z;

				_xy_slice_viewer_draw();
				}
				break;

			case _DRAG_MODE_SLICE_ARROW:
				{
				NsLerpf lerp;
				const NsCubei *roi;
				nsint slice;

				roi = workspace_update_roi( dw->workspace );

				if( y < ( nsint )_SVW_BAR_TOP )
					y = ( nsint )_SVW_BAR_TOP;
				else if( y > ( nsint )( _SVW_BAR_TOP + _SVW_BAR_HEIGHT ) )
					y = ( nsint )( _SVW_BAR_TOP + _SVW_BAR_HEIGHT );

				ns_lerpf_init(
					&lerp,
					( nsfloat )_SVW_BAR_TOP,
					( nsfloat )roi->C1.z,
					( nsfloat )( _SVW_BAR_TOP + _SVW_BAR_HEIGHT ),
					( nsfloat )roi->C2.z
					);

				slice = ( nsint )ns_lerpf( &lerp, ( nsfloat )y + 0.5f );

				if( slice < roi->C1.z )
					slice = roi->C1.z;
				else if( slice > roi->C2.z )
					slice = roi->C2.z;

				____xy_slice_index = ( nssize )slice;

				_xy_slice_viewer_draw();
				}
				break;
         }
      }
   }


//void _get_display_window_saved_roi( const DisplayWindow* dw, NsDragRect *R );
void _GetDisplayWindowROI( const DisplayWindow* dw, NsDragRect *R );


void _display_window_begin_translation( DisplayWindow *dw, nsint x, nsint y )
	{
   dw->drag_mode = _DRAG_MODE_TRANSLATE;
   dw->is_translating = NS_TRUE;

	//____mouse_mode_rays_get_point( dw, x, y, &dw->P1i, &dw->P1f, NS_FALSE );
	_display_window_client_to_voxel_space( dw, x, y, &dw->P1f );

   ns_model_translate_selected_vertices_begin( &dw->model_xfrm, GetWorkspaceNeuronTree( dw->workspace ) );
	ns_spines_translate_selected_begin( &dw->spines_xfrm, GetWorkspaceNeuronTree( dw->workspace ) );

	/* For now, the spines translate around the same point as the vertices. */
	if( 0 < ns_model_transform_size( &dw->model_xfrm ) )
		{
		dw->spines_xfrm.C = dw->model_xfrm.C;
		dw->spines_xfrm.R = dw->model_xfrm.R;
		}

   _display_window_update_transform_center( dw );
   _display_window_update_transform_radius( dw );
	}


void _display_window_begin_rotation( DisplayWindow *dw, nsint x, nsint y )
	{
   dw->drag_mode = _DRAG_MODE_ROTATE;
   dw->is_rotating = NS_TRUE;

   //____mouse_mode_rays_get_point( dw, x, y, &dw->P1i, &dw->P1f, NS_FALSE );
   _display_window_client_to_voxel_space( dw, x, y, &dw->P1f );

   ns_model_rotate_selected_vertices_begin(
      &dw->model_xfrm, GetWorkspaceNeuronTree( dw->workspace ), dw->display );
	ns_spines_rotate_selected_begin( &dw->spines_xfrm, GetWorkspaceNeuronTree( dw->workspace ), dw->display );

	/* For now, the spines rotate around the same point as the vertices. */
	if( 0 < ns_model_transform_size( &dw->model_xfrm ) )
		{
		dw->spines_xfrm.C = dw->model_xfrm.C;
		dw->spines_xfrm.R = dw->model_xfrm.R;
		}

   _display_window_update_transform_center( dw );
   _display_window_update_transform_radius( dw );
	}




void _DisplayWindowMouseModeDragOnLButtonDown
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
	nsint slice;
	nsint  delta;
	nsfloat  zoom;
	NsDragRect R;
	NsPoint2i P;


	dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   SetCapture( hWnd );
   dw->hasCapture = 1;

   MakeVector2i( &dw->clientCoord, x, y );

	zoom = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );

	dw->drag_mode = _DRAG_MODE_NORMAL;

   if( flags & MK_CONTROL )
		{
		/*TEMP!!!!!!!!!!!*/
		//NsAABBox3d B;
		//_temp_on_lbutton_down_do_drag_rect( hWnd, x, y, &B );
		//ns_model_select_vertices( workspace_filtered_model( dw->workspace ), NS_FALSE );
		//ns_model_select_vertices_by_aabbox( workspace_filtered_model( dw->workspace ), &B );

      _display_window_begin_translation( dw, x, y );
		}
   else if( flags & MK_SHIFT )
		_display_window_begin_rotation( dw, x, y );
   else
		{
		if( NS_XY != dw->display && ____xy_slice_enabled )
			{
			/*_get_display_window_saved_roi*/_GetDisplayWindowROI( dw, &R );

			/* Add a voxel (in client space) to the end of the R.O.I. so the user can
				click on the last Z of the slice viewer. */
			switch( dw->display )
				{
				case NS_ZY: R.end_x += ( nsint )zoom; break;
				case NS_XZ: R.end_y += ( nsint )zoom; break;

				default:
					ns_assert_not_reached();
				}

			P.x = x;
			P.y = y;

			delta = 100000;

			if( ns_point2i_inside_drag_rect( &P, &R ) )
				switch( dw->display )
					{
					case NS_ZY:
						slice = _dw_get_slice_index_in_client_space_zy( dw );
						delta = NS_ABS( slice - x );
						break;

					case NS_XZ:
						slice = _dw_get_slice_index_in_client_space_xz( dw );
						delta = NS_ABS( slice - y );
						break;

					default:
						ns_assert_not_reached();
					}

			if( delta <= ( nsint )zoom + 8 )
				dw->drag_mode = _DRAG_MODE_SLICE_BAR;
			}

		if( ____xy_slice_enabled && _DRAG_MODE_NORMAL == dw->drag_mode )
			if( x >= 0 && y >= 0 )
				{
				nssize ux, uy;

				ux = ( nssize )x;
				uy = ( nssize )y;

				if( ( ux >= ( _SVW_ARROW_LEFT - _SVW_ARROW_ISECT_SPACE_X ) ) &&
					 ( ux <  ( _SVW_ARROW_LEFT + _SVW_ARROW_WIDTH + _SVW_ARROW_ISECT_SPACE_X ) ) &&
					 ( uy >= ( _SVW_ARROW_TOP( ____svw_arrow_y ) - _SVW_ARROW_ISECT_SPACE_Y ) ) &&
					 ( uy <  ( _SVW_ARROW_TOP( ____svw_arrow_y ) + _SVW_ARROW_HEIGHT + _SVW_ARROW_ISECT_SPACE_Y ) ) 
					)
					dw->drag_mode = _DRAG_MODE_SLICE_ARROW;
				}
		}

   dw->isDragging = 1;
   }


void _display_window_mouse_mode_drag_center_view
	(
	DisplayWindow     *dw,
	nsint              display,
	const NsVector3i  *I,
	const Vector2i    *R
	)
	{
	nsfloat   zoom;
	Vector2i  corner;
	Vector2i  G;


	zoom   = GetWorkspaceZoom( dw->workspace, dw->index, display );
	corner = GetWorkspaceCorner( dw->workspace, dw->index, display );

	_DisplayWindowClientToGlobalVector( &G, R, &corner, zoom, display );

	switch( display )
		{
		case NS_XY:
			corner.x += ( I->x - G.x );
			corner.y += ( I->y - G.y );
			break;

		case NS_ZY:
			corner.x += ( I->z - G.x );
			corner.y += ( I->y - G.y );
			break;

		case NS_XZ:
			corner.x += ( I->x - G.x );
			corner.y += ( I->z - G.y );
			break;

		default:
			ns_assert_not_reached();
		}

	SetWorkspaceCorner( dw->workspace, dw->index, display, corner );
	}


void _display_window_mouse_mode_drag_on_lbutton_dbl_clk
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
	{
   DisplayWindow  *dw;
	RECT            rc;
   const NsImage  *image;
	const NsCubei  *roi;                                                                                                                                                                                   
	nsfloat         zoom;
	Vector2i        C, R, G, corner;
	NsVector3i      I;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

	/* Find the center of the window in client coordinates. */
	GetClientRect( hWnd, &rc );
	MakeVector2i( &R, ( rc.right - rc.left ) / 2, ( rc.bottom - rc.top ) / 2 );

	/* Set all the zooms to whatever the zoom is for the current view. */
	zoom = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
	SetWorkspaceZoom( dw->workspace, dw->index, NS_XY, zoom );
	SetWorkspaceZoom( dw->workspace, dw->index, NS_ZY, zoom );
	SetWorkspaceZoom( dw->workspace, dw->index, NS_XZ, zoom );

	/* Convert the mouse's client coordinates to image space. */
	corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
	MakeVector2i( &C, x, y );
	_DisplayWindowClientToGlobalVector( &G, &C, &corner, zoom, dw->display );

	image = workspace_volume( dw->workspace );
	roi   = workspace_visual_roi( dw->workspace );

   switch( dw->display )
      {
      case NS_XY:
			I.x = ( nsint )G.x;
			I.y = ( nsint )G.y;
	
			if( 0 <= I.x && I.x < ( nsint )ns_image_width( image ) &&
				 0 <= I.y && I.y < ( nsint )ns_image_height( image ) )
				_display_window_find_seed( dw, image, roi, I.x, I.y, NS_SEED_XY_FIND_Z, &I.z );
			else
				I.z = ( nsint )( ns_image_length( image ) / 2 );
         break;

      case NS_ZY:
			I.z = ( nsint )G.x;
			I.y = ( nsint )G.y;
	
			if( 0 <= I.z && I.z < ( nsint )ns_image_length( image ) &&
				 0 <= I.y && I.y < ( nsint )ns_image_height( image ) )
				_display_window_find_seed( dw, image, roi, I.z, I.y, NS_SEED_ZY_FIND_X, &I.x );
			else
				I.x = ( nsint )( ns_image_width( image ) / 2 );
         break;

      case NS_XZ:
			I.x = ( nsint )G.x;
			I.z = ( nsint )G.y;
	
			if( 0 <= I.x && I.x < ( nsint )ns_image_width( image ) &&
				 0 <= I.z && I.z < ( nsint )ns_image_length( image ) )
				_display_window_find_seed( dw, image, roi, I.x, I.z, NS_SEED_XZ_FIND_Y, &I.y );
			else
				I.y = ( nsint )( ns_image_height( image ) / 2 );
         break;

      default:
         ns_assert_not_reached();
      }

	_display_window_mouse_mode_drag_center_view( dw, NS_XY, &I, &R );
	_display_window_mouse_mode_drag_center_view( dw, NS_ZY, &I, &R );
	_display_window_mouse_mode_drag_center_view( dw, NS_XZ, &I, &R );

	DrawDisplayWindow( hWnd );
	}


void _display_window_end_translation( DisplayWindow *dw, nsint x, nsint y )
	{
	NsList      list;
	NsVector3f  T;


	//____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_FALSE );
	_display_window_client_to_voxel_space( dw, x, y, &dw->P2f );

   ns_vector3f_sub( &T, &dw->P2f, &dw->P1f );

   ns_model_translate_selected_vertices_apply( &dw->model_xfrm, &T );
	ns_spines_translate_selected_apply( &dw->spines_xfrm, &T );

   ns_list_construct( &list, NULL );

   ns_model_translate_selected_vertices_end( &dw->model_xfrm, NS_TRUE, &list );
	ns_spines_translate_selected_end( &dw->spines_xfrm, NS_TRUE );

   ns_model_remove_conn_comp_by_auto_iter_list(
      workspace_raw_model( dw->workspace ), &list );
   ns_list_destruct( &list );

   dw->is_translating = NS_FALSE;
   ____redraw_all();
	}


void _display_window_end_rotation( DisplayWindow *dw, nsint x, nsint y )
	{
	NsList list;

   //____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_FALSE );
   _display_window_client_to_voxel_space( dw, x, y, &dw->P2f );

   ns_model_rotate_selected_vertices_apply( &dw->model_xfrm, &dw->P2f );
	ns_spines_rotate_selected_apply( &dw->spines_xfrm, &dw->P2f );

   ns_list_construct( &list, NULL );

   ns_model_rotate_selected_vertices_end( &dw->model_xfrm, NS_TRUE, &list );
	ns_spines_rotate_selected_end( &dw->spines_xfrm, NS_TRUE );

   ns_model_remove_conn_comp_by_auto_iter_list(
      workspace_raw_model( dw->workspace ), &list );
   ns_list_destruct( &list );

   dw->is_rotating = NS_FALSE;
   ____redraw_all();
	}


void _DisplayWindowMouseModeDragOnLButtonUp
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
   RECT rc;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   if( dw->hasCapture )
      {
      dw->hasCapture = 0;
      ReleaseCapture();

      GetClientRect( hWnd, &rc );

      /*
      if( x < 0 || x >= rc.right || y < 0 || y >= rc.bottom )
         ns_println( "CANCEL TRANSLATION/ROTATION!" );
      else
         ns_println( "CONFIRM TRANSLATION/ROTATION" );
      */

      switch( dw->drag_mode )
         {
         case _DRAG_MODE_TRANSLATE:
				_display_window_end_translation( dw, x, y );
            break;

         case _DRAG_MODE_ROTATE:
				_display_window_end_rotation( dw, x, y );
            break;

			//case _DRAG_MODE_SLICE_ARROW:
			//	break;
         }
      }

   dw->isDragging = 0;
   }


void 	_adjust_slice_viewer_due_to_roi_update( const NsCubei *roi )
	{
	if( ____xy_slice_index < ( nssize )roi->C1.z )
		____xy_slice_index = ( nssize )roi->C1.z;

	if( ____xy_slice_index > ( nssize )roi->C2.z )
		____xy_slice_index = ( nssize )roi->C2.z;
	}


void _do_adjust_seed_due_to_roi_update( nsuint workspace, const NsCubei *roi )
	{
	Vector3i seed;

	GetWorkspaceSeed( workspace, &seed );

	if( seed.x < roi->C1.x )
		seed.x = roi->C1.x;
	else if( seed.x > roi->C2.x )
		seed.x = roi->C2.x;

	if( seed.y < roi->C1.y )
		seed.y = roi->C1.y;
	else if( seed.y > roi->C2.y )
		seed.y = roi->C2.y;

	if( seed.z < roi->C1.z )
		seed.z = roi->C1.z;
	else if( seed.z > roi->C2.z )
		seed.z = roi->C2.z;

	SetWorkspaceSeed( workspace, &seed );
	}


void _adjust_seed_due_to_roi_update( DisplayWindow *dw, const NsCubei *roi )
	{  _do_adjust_seed_due_to_roi_update( dw->workspace, roi );  }


#define _ROI_FLAG_START_X            0x001
#define _ROI_FLAG_START_Y            0x002
#define _ROI_FLAG_END_X              0x004
#define _ROI_FLAG_END_Y              0x008
#define _ROI_FLAG_KEEP_ASPECT_RATIO  0x010

void _ResizeDisplayWindowROI( DisplayWindow* dw, NsDragRect *R, nsuint flags )
	{
	NsCubei   *roi;
   Vector3i   C1, C2;
   Vector2i   corner;
   Vector2i   client_start, client_end;
   Vector2i   global_start, global_end;
   nsfloat    zoom;
   nsint      width;
   nsint      height;
 

   roi    = workspace_update_roi( dw->workspace );
   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
   width  = ( nsint )ns_image_width( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );
   height = ( nsint )ns_image_height( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );

	if( ( nsboolean )( flags & _ROI_FLAG_KEEP_ASPECT_RATIO ) )
		{
		}

	MakeVector2i( &client_start, R->start_x, R->start_y );
	MakeVector2i( &client_end, R->end_x, R->end_y );

	_DisplayWindowClientToGlobalVector( &global_start, &client_start, &corner, zoom, dw->display );
	_DisplayWindowClientToGlobalVector( &global_end, &client_end, &corner, zoom, dw->display );

	if( ( nsboolean )( flags & _ROI_FLAG_START_X ) )
		{
		if( global_start.x < 0 )global_start.x = 0;
		if( width <= global_start.x )global_start.x = width - 1;
		}

	if( ( nsboolean )( flags & _ROI_FLAG_END_X ) )
		{
		if( global_end.x < 0 )global_end.x = 0;
		if( width <= global_end.x )global_end.x = width - 1;
		}

	if( ( nsboolean )( flags & _ROI_FLAG_START_Y ) )
		{
		if( global_start.y < 0 )global_start.y = 0;
		if( height <= global_start.y )global_start.y = height - 1;
		}

	if( ( nsboolean )( flags & _ROI_FLAG_END_Y ) )
		{
		if( global_end.y < 0 )global_end.y = 0;
		if( height <= global_end.y )global_end.y = height - 1;
		}

   switch( dw->display )
      {
      case NS_XY:
			if( ( nsboolean )( flags & _ROI_FLAG_START_X ) )
				if( global_start.x > roi->C2.x )
					global_start.x = roi->C2.x;

			if( ( nsboolean )( flags & _ROI_FLAG_END_X ) )
				if( global_end.x < roi->C1.x )
					global_end.x = roi->C1.x;

			if( ( nsboolean )( flags & _ROI_FLAG_START_Y ) )
				if( global_start.y > roi->C2.y )
					global_start.y = roi->C2.y;

			if( ( nsboolean )( flags & _ROI_FLAG_END_Y ) )
				if( global_end.y < roi->C1.y )
					global_end.y = roi->C1.y;

			MakeVector3i(
				&C1,
				( nsboolean )( flags & _ROI_FLAG_START_X ) ? global_start.x : roi->C1.x,
				( nsboolean )( flags & _ROI_FLAG_START_Y ) ? global_start.y : roi->C1.y,
				roi->C1.z
				);

			MakeVector3i(
				&C2,
				( nsboolean )( flags & _ROI_FLAG_END_X ) ? global_end.x : roi->C2.x,
				( nsboolean )( flags & _ROI_FLAG_END_Y ) ? global_end.y : roi->C2.y,
				roi->C2.z
				);
         break;

      case NS_ZY:
			if( ( nsboolean )( flags & _ROI_FLAG_START_X ) )
				if( global_start.x > roi->C2.z )
					global_start.x = roi->C2.z;

			if( ( nsboolean )( flags & _ROI_FLAG_END_X ) )
				if( global_end.x < roi->C1.z )
					global_end.x = roi->C1.z;

			if( ( nsboolean )( flags & _ROI_FLAG_START_Y ) )
				if( global_start.y > roi->C2.y )
					global_start.y = roi->C2.y;

			if( ( nsboolean )( flags & _ROI_FLAG_END_Y ) )
				if( global_end.y < roi->C1.y )
					global_end.y = roi->C1.y;

			MakeVector3i(
				&C1,
				roi->C1.x,
				( nsboolean )( flags & _ROI_FLAG_START_Y ) ? global_start.y : roi->C1.y,
				( nsboolean )( flags & _ROI_FLAG_START_X ) ? global_start.x : roi->C1.z
				);

			MakeVector3i(
				&C2,
				roi->C2.x,
				( nsboolean )( flags & _ROI_FLAG_END_Y ) ? global_end.y : roi->C2.y,
				( nsboolean )( flags & _ROI_FLAG_END_X ) ? global_end.x : roi->C2.z
				);
         break;
		
      case NS_XZ:
			if( ( nsboolean )( flags & _ROI_FLAG_START_X ) )
				if( global_start.x > roi->C2.x )
					global_start.x = roi->C2.x;

			if( ( nsboolean )( flags & _ROI_FLAG_END_X ) )
				if( global_end.x < roi->C1.x )
					global_end.x = roi->C1.x;

			if( ( nsboolean )( flags & _ROI_FLAG_START_Y ) )
				if( global_start.y > roi->C2.z )
					global_start.y = roi->C2.z;

			if( ( nsboolean )( flags & _ROI_FLAG_END_Y ) )
				if( global_end.y < roi->C1.z )
					global_end.y = roi->C1.z;


			MakeVector3i(
				&C1,
				( nsboolean )( flags & _ROI_FLAG_START_X ) ? global_start.x : roi->C1.x,
				roi->C1.y,
				( nsboolean )( flags & _ROI_FLAG_START_Y ) ? global_start.y : roi->C1.z
				);

			MakeVector3i(
				&C2,
				( nsboolean )( flags & _ROI_FLAG_END_X ) ? global_end.x : roi->C2.x,
				roi->C2.y,
				( nsboolean )( flags & _ROI_FLAG_END_Y ) ? global_end.y : roi->C2.z
				);
         break;

		default:
			ns_assert_not_reached();
      }

	roi->C1.x = ( nsint )C1.x;
	roi->C1.y = ( nsint )C1.y;
	roi->C1.z = ( nsint )C1.z;
	roi->C2.x = ( nsint )C2.x;
	roi->C2.y = ( nsint )C2.y;
	roi->C2.z = ( nsint )C2.z;

	_adjust_slice_viewer_due_to_roi_update( roi );
	_adjust_seed_due_to_roi_update( dw, roi );

	_do_show_roi_on_status_bar( dw->workspace );
	}


void _TranslateDisplayWindowROI( DisplayWindow *dw, nsint delta_x, nsint delta_y )
	{
	NsCubei  *roi;
	nsint     cube_width;
	nsint     cube_height;
	nsint     cube_length;
   nsint     image_width;
   nsint     image_height;


   roi          = workspace_update_roi( dw->workspace );
	cube_width   = ns_cubei_width( roi );
	cube_height  = ns_cubei_height( roi );
	cube_length  = ns_cubei_length( roi );
   image_width  = ( nsint )ns_image_width( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );
   image_height = ( nsint )ns_image_height( GetWorkspaceDisplayImage( dw->workspace, dw->display ) );

	ns_assert( 0 < cube_width );
	ns_assert( 0 < cube_height );
	ns_assert( 0 < cube_length );

   switch( dw->display )
      {
      case NS_XY:
			ns_assert( cube_width <= image_width );
			ns_assert( cube_height <= image_height );

			roi->C1.x += delta_x;
			roi->C2.x += delta_x;
			roi->C1.y += delta_y;
			roi->C2.y += delta_y;

			if( roi->C1.x < 0 )
				{
				roi->C1.x = 0;
				roi->C2.x = cube_width - 1;
				}
			else if( roi->C2.x >= image_width )
				{
				roi->C1.x = image_width - cube_width;
				roi->C2.x = image_width - 1;
				}

			if( roi->C1.y < 0 )
				{
				roi->C1.y = 0;
				roi->C2.y = cube_height - 1;
				}
			else if( roi->C2.y >= image_height )
				{
				roi->C1.y = image_height - cube_height;
				roi->C2.y = image_height - 1;
				}
			break;

      case NS_ZY:
			ns_assert( cube_length <= image_width );
			ns_assert( cube_height <= image_height );

			roi->C1.z += delta_x;
			roi->C2.z += delta_x;
			roi->C1.y += delta_y;
			roi->C2.y += delta_y;

			if( roi->C1.z < 0 )
				{
				roi->C1.z = 0;
				roi->C2.z = cube_length - 1;
				}
			else if( roi->C2.z >= image_width )
				{
				roi->C1.z = image_width - cube_length;
				roi->C2.z = image_width - 1;
				}

			if( roi->C1.y < 0 )
				{
				roi->C1.y = 0;
				roi->C2.y = cube_height - 1;
				}
			else if( roi->C2.y >= image_height )
				{
				roi->C1.y = image_height - cube_height;
				roi->C2.y = image_height - 1;
				}
			break;

		case NS_XZ:
			ns_assert( cube_width <= image_width );
			ns_assert( cube_length <= image_height );

			roi->C1.x += delta_x;
			roi->C2.x += delta_x;
			roi->C1.z += delta_y;
			roi->C2.z += delta_y;

			if( roi->C1.x < 0 )
				{
				roi->C1.x = 0;
				roi->C2.x = cube_width - 1;
				}
			else if( roi->C2.x >= image_width )
				{
				roi->C1.x = image_width - cube_width;
				roi->C2.x = image_width - 1;
				}

			if( roi->C1.z < 0 )
				{
				roi->C1.z = 0;
				roi->C2.z = cube_length - 1;
				}
			else if( roi->C2.z >= image_height )
				{
				roi->C1.z = image_height - cube_length;
				roi->C2.z = image_height - 1;
				}
			break;

		default:
			ns_assert_not_reached();
		}

	_adjust_slice_viewer_due_to_roi_update( roi );
	_adjust_seed_due_to_roi_update( dw, roi );

	_do_show_roi_on_status_bar( dw->workspace );
	}


void _do_get_display_window_roi( const DisplayWindow* dw, const NsCubei *roi, NsDragRect *R, nsboolean for_display )
	{
   Vector2i  C1g, C2g, C1c, C2c;
   nsfloat  zoom;
	Vector2i corner;


   ns_assert( NULL != dw );

   switch( dw->display )
      {
      case NS_XY:
			MakeVector2i( &C1g, roi->C1.x, roi->C1.y );
			MakeVector2i( &C2g, roi->C2.x, roi->C2.y );
			break;

      case NS_ZY:
			MakeVector2i( &C1g, roi->C1.z, roi->C1.y );
			MakeVector2i( &C2g, roi->C2.z, roi->C2.y );
			break;

		case NS_XZ:
			/* Y axis' reversed in top view. Note the C2 and C1 switch. */
			//MakeVector2i( &C1g, roi->C1.x, roi->C2.z );
			//MakeVector2i( &C2g, roi->C2.x, roi->C1.z );
			MakeVector2i( &C1g, roi->C1.x, roi->C1.z );
			MakeVector2i( &C2g, roi->C2.x, roi->C2.z );
			break;
      }

	if( for_display )
		{
		++C2g.x;
		++C2g.y;
		}

   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

   _DisplayWindowGlobalToClientVector( &C1c, &C1g, &corner, zoom, dw->display );
   _DisplayWindowGlobalToClientVector( &C2c, &C2g, &corner, zoom, dw->display );

	ns_drag_rect_start( R, ( nsint )C1c.x, ( nsint )C1c.y );
	ns_drag_rect_end( R, ( nsint )C2c.x, ( nsint )C2c.y );

	ns_assert( ns_drag_rect_is_normalized( R ) );
	//ns_drag_rect_normalize( R );

	//ns_println( "ROI = %d,%d to %d,%d", R->start_x, R->start_y, R->end_x, R->end_y );
	}


//void _get_display_window_visual_roi( const DisplayWindow* dw, NsDragRect *R )
  // {  _do_get_display_window_roi( dw, workspace_visual_roi( dw->workspace ), R );  }


void _GetDisplayWindowROI( const DisplayWindow* dw, NsDragRect *R )
   {  _do_get_display_window_roi( dw, workspace_update_roi( dw->workspace ), R, NS_FALSE );  }

void _display_window_get_client_roi_ex( const DisplayWindow* dw, NsDragRect *R )
   {  _do_get_display_window_roi( dw, workspace_update_roi( dw->workspace ), R, NS_TRUE );  }


void _adjust_roi_for_visual_display( DisplayWindow *dw, NsDragRect *R )
	{
	nsfloat zoom;
	//nsint offset;
	Vector2i corner, G1, G2, C1, C2;


	/* NOTE: For drawing need to add +1 or -1 voxel to an edge.
		Get the width and height of a voxel in client space. */
   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );
	MakeVector2i( &G1, 0, 0 );
	MakeVector2i( &G2, 1, 1 );
   _DisplayWindowGlobalToClientVector( &C1, &G1, &corner, zoom, dw->display );
   _DisplayWindowGlobalToClientVector( &C2, &G2, &corner, zoom, dw->display );

	//offset = ( ( nsint )zoom ) / 2;

	//R->end_x += offset;
	//R->end_y += offset;
	
	R->end_x += ( nsint )( C2.x - C1.x );

	//if( NS_XZ != dw->display )
		R->end_y += ( nsint )( C2.y - C1.y );
	//else
	//	R->start_y -= ( C1.y - C2.y );

	/* Move over by 1 pixel in client space? */

	if( R->end_x > R->start_x )
		--R->end_x;

   //if( NS_XZ != dw->display )
	//	{
		if( R->end_y > R->start_y )
			--R->end_y;
	//	}
	//else
	//	{
	//	if( R->start_y < R->end_y )
	//		++(R->start_y);
	//	}
	}


void _display_window_draw_roi( DisplayWindow *dw )
	{
	NsDragRect R;
   nsint old_rop;
   HPEN hOldPen, hNewPen;
   nsint old_bk_mode;
	HICON ci;


	_GetDisplayWindowROI( dw, &R );
	_adjust_roi_for_visual_display( dw, &R );

	hNewPen = CreatePen( PS_DOT, 1, 0x00FFFFFF );
	hOldPen = SelectObject( dw->backBuffer.hDC, hNewPen );

	old_rop     = SetROP2( dw->backBuffer.hDC, R2_XORPEN );
	old_bk_mode = SetBkMode( dw->backBuffer.hDC, TRANSPARENT );

	MoveToEx( dw->backBuffer.hDC, R.start_x, R.start_y, NULL );
	LineTo( dw->backBuffer.hDC, R.end_x, R.start_y );
	LineTo( dw->backBuffer.hDC, R.end_x, R.end_y );
	LineTo( dw->backBuffer.hDC, R.start_x, R.end_y );
	LineTo( dw->backBuffer.hDC, R.start_x, R.start_y );

	SelectObject( dw->backBuffer.hDC, hOldPen );
	DeleteObject( hNewPen );

	SetBkMode( dw->backBuffer.hDC, old_bk_mode );
	SetROP2( dw->backBuffer.hDC, old_rop );

	ci = LoadIcon( g_Instance, "ICON_CORNER" );

	/* NOTE: Assuming the corner icon is 7 pixels in width and height. i.e. -3 */
	DrawIcon( dw->backBuffer.hDC, R.start_x - 3, R.start_y - 3, ci );
	DrawIcon( dw->backBuffer.hDC, R.end_x - 3, R.start_y - 3, ci );
	DrawIcon( dw->backBuffer.hDC, R.end_x - 3, R.end_y - 3, ci );
	DrawIcon( dw->backBuffer.hDC, R.start_x - 3, R.end_y - 3, ci );

	/* NOTE: Assuming the corner icon is 15 pixels in width and height. i.e. -7 */
	DrawIcon(
		dw->backBuffer.hDC,
		R.start_x + ns_drag_rect_width( &R ) / 2 - 7,
		R.start_y + ns_drag_rect_height( &R ) / 2 - 7,
		LoadIcon( g_Instance, "ICON_TARGET" )
		);
	}


void _mouse_mode_roi_set_cursor( HWND hWnd, DisplayWindow *dw, nsint cursor )
	{
	if( cursor != dw->roi_curr_cursor )
		{
		dw->roi_curr_cursor = cursor;

		SetClassLongPtr( hWnd, GCLP_HCURSOR, ( LONG )dw->roi_cursors[ dw->roi_curr_cursor ] );
		SetCursor( dw->roi_cursors[ dw->roi_curr_cursor ] );
		}
	}


nsint _mouse_mode_roi_get_cursor( const DisplayWindow *dw, const NsDragRect *R, nsint x, nsint y )
	{
	nsint  dsx, dsy, dex, dey;
	nsint  cursor;


	cursor = -1;

	dsx = NS_ABS( x - R->start_x );
	dsy = NS_ABS( y - R->start_y );
	dex = NS_ABS( x - R->end_x );
	dey = NS_ABS( y - R->end_y );

	#define _ROI_TOLERANCE  6

	if( dsx < _ROI_TOLERANCE )
		{
		if( dsy < _ROI_TOLERANCE )
			cursor = _ROI_CURSOR_NW;
		else if( dey < _ROI_TOLERANCE )
			cursor = _ROI_CURSOR_SW;
		else if( R->start_y <= y && y <= R->end_y )
			cursor = _ROI_CURSOR_W;
		}
	else if( dex < _ROI_TOLERANCE )
		{
		if( dsy < _ROI_TOLERANCE )
			cursor = _ROI_CURSOR_NE;
		else if( dey < _ROI_TOLERANCE )
			cursor = _ROI_CURSOR_SE;
		else if( R->start_y <= y && y <= R->end_y )
			cursor = _ROI_CURSOR_E;
		}
	else if( dsy < _ROI_TOLERANCE )
		{
		if( R->start_x <= x && x <= R->end_x )
			cursor = _ROI_CURSOR_N;
		}
	else if( dey < _ROI_TOLERANCE )
		{
		if( R->start_x <= x && x <= R->end_x )
			cursor = _ROI_CURSOR_S;
		}

	if( -1 == cursor )
		{
		if( R->start_x <= x && x <= R->end_x && R->start_y <= y && y <= R->end_y )
			cursor = _ROI_CURSOR_ALL;
		else
			cursor = _ROI_CURSOR_ARROW;
		}

	ns_assert( 0 <= cursor );
	return cursor;
	}


void _mouse_mode_roi_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint mouse_flags )
	{
	DisplayWindow  *dw;
	NsDragRect      R;
	nsfloat         zoom;
	Vector2i        corner;
	Vector2i        delta;
	nsboolean       redraw;
	nsuint          roi_flags;


   dw = _MapHandleToDisplayWindow( hWnd );

	_GetDisplayWindowROI( dw, &R );

   if( ! dw->isDragging )
		{
		_adjust_roi_for_visual_display( dw, &R );
		_mouse_mode_roi_set_cursor( hWnd, dw, _mouse_mode_roi_get_cursor( dw, &R, x, y ) );
		}
	else if( _ROI_CURSOR_ARROW != dw->roi_curr_cursor )
		{
		roi_flags = 0;

		if( ( nsboolean )( mouse_flags & MK_SHIFT ) )
			roi_flags |= _ROI_FLAG_KEEP_ASPECT_RATIO;

		dw->new_client.x = x;
		dw->new_client.y = y;

		zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
		corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

		_DisplayWindowClientToGlobalVector( &dw->new_image, &dw->new_client, &corner, zoom, dw->display );

		delta.x = dw->new_image.x - dw->old_image.x;
		delta.y = dw->new_image.y - dw->old_image.y;

		redraw = NS_TRUE;

		//ns_print( NS_FMT_STRING " ", _roi_cursor_to_string( dw->roi_curr_cursor ) );

/* IMPORTANT: Becuase of roundoff error of converting from global(image) space to client
	space and vica-versa, only allow altering the appropriate direction by setting a flags
	variable. */

		switch( dw->roi_curr_cursor )
			{
			case _ROI_CURSOR_NW:
				R.start_x = x;
				R.start_y = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_START_X | _ROI_FLAG_START_Y );
				break;

			case _ROI_CURSOR_SE:
				R.end_x = x;
				R.end_y = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_END_X | _ROI_FLAG_END_Y );
				break;

			case _ROI_CURSOR_NE:
				R.end_x   = x;
				R.start_y = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_END_X | _ROI_FLAG_START_Y );
				break;

			case _ROI_CURSOR_SW:
				R.start_x = x;
				R.end_y   = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_START_X | _ROI_FLAG_END_Y );
				break;

			case _ROI_CURSOR_W:
				R.start_x = x;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_START_X );
				break;

			case _ROI_CURSOR_E:
				R.end_x = x;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_END_X );
				break;

			case _ROI_CURSOR_N:
				R.start_y = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_START_Y );
				break;

			case _ROI_CURSOR_S:
				R.end_y = y;
				_ResizeDisplayWindowROI( dw, &R, roi_flags | _ROI_FLAG_END_Y );
				break;

			case _ROI_CURSOR_ALL:
				if( ! ( 0 == delta.x && 0 == delta.y ) )
					_TranslateDisplayWindowROI( dw, ( nsint )delta.x, ( nsint )delta.y );
				else
					redraw = NS_FALSE;
				break;

			default:
				ns_assert_not_reached();
			}

		/* HACK??? Could draw faster just using XOR to erase and then draw. */
		if( redraw )
			{
			dw->roi_did_change = NS_TRUE;

			SendMessage( GetParent( hWnd ), mMSG_USER_Update, 0, dw->display );
			____redraw_3d();
			}

		dw->old_client = dw->new_client;
		dw->old_image  = dw->new_image;
		}
	}


extern void _force_opengl_is_dragging( nsboolean *old_value );
extern void _reset_opengl_is_dragging( nsboolean old_value );


void _mouse_mode_roi_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	DisplayWindow   *dw;
	nsfloat         zoom;
	Vector2i        corner;
	const NsCubei  *roi;


	SetCapture( hWnd );

   dw = _MapHandleToDisplayWindow( hWnd );

	dw->roi_did_change = NS_FALSE;

	if( _ROI_CURSOR_ARROW != dw->roi_curr_cursor )
		{
		roi = workspace_update_roi( dw->workspace );

		dw->hw_aspect_ratio = ( ( nsfloat )ns_cubei_height( roi ) ) / ( ( nsfloat )ns_cubei_width( roi ) );

		dw->old_client.x = x;
		dw->old_client.y = y;

		zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
		corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

		_DisplayWindowClientToGlobalVector( &dw->old_image, &dw->old_client, &corner, zoom, dw->display );

		dw->isDragging = 1;
		_force_opengl_is_dragging( &dw->ogl_is_dragging );
		}
	}


void _mouse_mode_roi_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
	DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

	ReleaseCapture();

   dw->isDragging = 0;
	_reset_opengl_is_dragging( dw->ogl_is_dragging );

	if( _ROI_CURSOR_ARROW != dw->roi_curr_cursor && dw->roi_did_change )
		____redraw_3d();

	dw->roi_did_change = NS_FALSE;
	}


//void _display_window_set_seed_by_roi( DisplayWindow *dw );


void _mouse_mode_roi_do_popup( HWND hWnd, nsuint workspace, nsint x, nsint y )
   {
   MENUITEMINFO    miim;
   HMENU           hMenu;
   POINT           pt;
   nsint           id;
	nsboolean       redraw;
	NsCubei        *roi;
	const NsImage  *volume;
	nsulong         flags;


	redraw = NS_FALSE;

   hMenu = CreatePopupMenu();

   miim.cbSize = sizeof( MENUITEMINFO );
   miim.fMask  = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   miim.fType  = MFT_STRING;
   miim.fState = MFS_ENABLED;

#define ____ROI_FULL_WIDTH_ID   72845
#define ____ROI_FULL_HEIGHT_ID  72846
#define ____ROI_FULL_LENGTH_ID  72847
#define ____ROI_FULL_ALL_ID     72848
#define ____ROI_ACCEPT_ID       72849
#define ____ROI_BUILD_NEURITES_ID  72850
#define ____ROI_BUILD_SPINES_ID    72851

   miim.wID        = ____ROI_FULL_WIDTH_ID;
   miim.dwTypeData = "100% Width (X)";
   miim.cch        = ( nsuint )ns_ascii_strlen( "100% Width (X)" );
   //if( NS_SPINE_STUBBY == old_spine_type )miim.fState |= MFS_CHECKED;
   InsertMenuItem( hMenu, 1, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_FULL_HEIGHT_ID;
   miim.dwTypeData = "100% Height (Y)";
   miim.cch        = ( nsuint )ns_ascii_strlen( "100% Height (Y)" );
   //if( NS_SPINE_THIN == old_spine_type )miim.fState |= MFS_CHECKED;
   InsertMenuItem( hMenu, 2, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_FULL_LENGTH_ID;
   miim.dwTypeData = "100% Length (Z)";
   miim.cch        = ( nsuint )ns_ascii_strlen( "100% Length (Z)" );
   //if( NS_SPINE_MUSHROOM == old_spine_type )miim.fState |= MFS_CHECKED;
   InsertMenuItem( hMenu, 3, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_FULL_ALL_ID;
   miim.dwTypeData = "100% All Dimensions";
   miim.cch        = ( nsuint )ns_ascii_strlen( "100% All Dimensions" );
   //if( NS_SPINE_OTHER == old_spine_type )miim.fState |= MFS_CHECKED;
   InsertMenuItem( hMenu, 4, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_ACCEPT_ID;
   miim.dwTypeData = "Visualize";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Visualize" );
   InsertMenuItem( hMenu, 5, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_BUILD_NEURITES_ID;
   miim.dwTypeData = "Build Neurites";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Build Neurites" );
   InsertMenuItem( hMenu, 6, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____ROI_BUILD_SPINES_ID;
   miim.dwTypeData = "Build Spines";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Build Spines" );
   InsertMenuItem( hMenu, 7, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.fMask  = 0;
   miim.fType  = MFT_SEPARATOR;
	InsertMenuItem( hMenu, 4, TRUE, &miim );

   miim.fMask  = 0;
   miim.fType  = MFT_SEPARATOR;
	InsertMenuItem( hMenu, 6, TRUE, &miim );

   pt.x = x;
   pt.y = y;

   ClientToScreen( hWnd, &pt );

   id = TrackPopupMenu( hMenu,                  
                        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                        pt.x,
                        pt.y,
                        0,
                        hWnd,
                        NULL
                      );

   DestroyMenu( hMenu );

	flags = 0;

	#define _ROI_FLAG_WIDTH   0x001
	#define _ROI_FLAG_HEIGHT  0x002
	#define _ROI_FLAG_LENGTH  0x004

   switch( id )
      {
      case ____ROI_FULL_WIDTH_ID:
			flags  = _ROI_FLAG_WIDTH;
			redraw = NS_TRUE;
         break;

      case ____ROI_FULL_HEIGHT_ID:
			flags  = _ROI_FLAG_HEIGHT;
			redraw = NS_TRUE;
         break;

      case ____ROI_FULL_LENGTH_ID:
			flags  = _ROI_FLAG_LENGTH;
			redraw = NS_TRUE;
         break;

      case ____ROI_FULL_ALL_ID:
			flags  = _ROI_FLAG_WIDTH | _ROI_FLAG_HEIGHT | _ROI_FLAG_LENGTH;
			redraw = NS_TRUE;
         break;

		case ____ROI_ACCEPT_ID:
			redraw = NS_TRUE;
			workspace_accept_roi( workspace );
			break;

		case ____ROI_BUILD_NEURITES_ID:
			redraw = NS_TRUE;
			//_display_window_set_seed_by_roi( dw );
			SendMessage( hWnd, WM_KEYDOWN, ( WPARAM )'N', 0 );
			break;

		case ____ROI_BUILD_SPINES_ID:
			SendMessage( hWnd, WM_KEYDOWN, ( WPARAM )'S', 0 );
			break;
      }

	if( flags )
		{
		roi    = workspace_update_roi( workspace );
		volume = workspace_volume( workspace );

		if( ( nsboolean )( flags & _ROI_FLAG_WIDTH ) )
			{
			roi->C1.x = 0;
			roi->C2.x = ( nsint )( ns_image_width( volume ) - 1 );
			}

		if( ( nsboolean )( flags & _ROI_FLAG_HEIGHT ) )
			{
			roi->C1.y = 0;
			roi->C2.y = ( nsint )( ns_image_height( volume ) - 1 );
			}

		if( ( nsboolean )( flags & _ROI_FLAG_LENGTH ) )
			{
			roi->C1.z = 0;
			roi->C2.z = ( nsint )( ns_image_length( volume ) - 1 );
			}

		_do_show_roi_on_status_bar( workspace );
		}

	if( redraw )
		____redraw_all();
   }


void _mouse_mode_roi_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
	{
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );
	_mouse_mode_roi_do_popup( dw->hWnd, dw->workspace, x, y );
	}


void _GetDisplayWindowClientSeed( const DisplayWindow* dw, Vector2i* clientSeed )
   {
   Vector3i       globalSeed3D;
   Vector2i       globalSeed2D;
   Vector2i       corner;
   HWorkspace     workspace;
   nsuint     index;
   nsint  display;
   nsfloat     zoom;


   ns_assert( NULL != dw );
   ns_assert( NULL != clientSeed);

   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   GetWorkspaceSeed( workspace, &globalSeed3D );

   switch( display )
      {
      case NS_XY : MakeVector2i( &globalSeed2D, globalSeed3D.x, globalSeed3D.y ); break;
      case NS_ZY    : MakeVector2i( &globalSeed2D, globalSeed3D.z, globalSeed3D.y ); break;
      case NS_XZ     : MakeVector2i( &globalSeed2D, globalSeed3D.x, globalSeed3D.z ); break;
      }

   zoom             = GetWorkspaceZoom( workspace, index, display );
   corner = GetWorkspaceCorner( workspace, index, display );

   _DisplayWindowGlobalToClientVector( clientSeed,
                                       &globalSeed2D,
                                       &corner,
                                       zoom,
                                       display
                                     );

   }/* _GetDisplayWindowClientSeed() */


void _DisplayWindowMouseModeSeedOnMouseMove
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
	const NsCubei *roi;


   dw = _MapHandleToDisplayWindow( hWnd );

   if( /*eDISPLAY_MONTAGE != dw->display &&*/ dw->isDragging )
      {
      Vector3i        globalSeed;
      Vector2i        corner;
      Vector2i        clientCoord;
      Vector2i        globalCoord;
      HWorkspace      workspace;
      nsuint      index;
      nsint   display;
      nsfloat      zoom;
      nsint       width;
      nsint       height;
      

      workspace = dw->workspace;
      index      = dw->index;
      display    = dw->display;

      GetWorkspaceSeed( workspace, &globalSeed );

      zoom             = GetWorkspaceZoom( workspace, index, display );
      corner = GetWorkspaceCorner( workspace, index, display );

      MakeVector2i( &clientCoord, x, y );

      _DisplayWindowClientToGlobalVector( &globalCoord,
                                          &clientCoord, 
                                          &corner,
                                          zoom,
                                          display
                                        );

      width  = ( nsint )ns_image_width( GetWorkspaceDisplayImage( workspace, display ) );
      height = ( nsint )ns_image_height( GetWorkspaceDisplayImage( workspace, display ) );

      if( globalCoord.x < 0 )
         globalCoord.x = 0;
      else if( width <= globalCoord.x )
         globalCoord.x = width - 1;

      if( globalCoord.y < 0 )
         globalCoord.y = 0;
      else if( height <= globalCoord.y )
         globalCoord.y = height - 1;

      switch( display )
         {
         case NS_XY:
            MakeVector3i( &globalSeed, globalCoord.x, globalCoord.y, globalSeed.z );
            break;

         case NS_ZY:
            MakeVector3i( &globalSeed, globalSeed.x,  globalCoord.y, globalCoord.x ); 
            break;

         case NS_XZ:
            MakeVector3i( &globalSeed, globalCoord.x, globalSeed.y,  globalCoord.y );
            break;
         }

		roi = workspace_update_roi( dw->workspace );

		if( globalSeed.x < roi->C1.x )
			globalSeed.x = roi->C1.x;
		else if( globalSeed.x > roi->C2.x )
			globalSeed.x = roi->C2.x;

		if( globalSeed.y < roi->C1.y )
			globalSeed.y = roi->C1.y;
		else if( globalSeed.y > roi->C2.y )
			globalSeed.y = roi->C2.y;

		if( globalSeed.z < roi->C1.z )
			globalSeed.z = roi->C1.z;
		else if( globalSeed.z > roi->C2.z )
			globalSeed.z = roi->C2.z;

		SetWorkspaceSeed( workspace, &globalSeed );
		_display_window_show_seed_on_status_bar( dw );

		/* HACK(s) below! */
		SendMessage( GetParent( hWnd ), mMSG_USER_Update, 0, display );
		____redraw_3d();
      }
   }


void _DisplayWindowMouseModeSeedOnLButtonDown
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   //if( eDISPLAY_MONTAGE != dw->display )
     // {
      SetCapture( hWnd );

      dw->hasCapture = 1;
      dw->isDragging = 1;

      GetWorkspaceSeed( dw->workspace, &dw->old_seed );
//      }

   }/* _DisplayWindowMouseModeSeedOnLButtonDown() */


#include <image/nsseed.h>

/*
void _display_window_set_seed_by_roi( DisplayWindow *dw )
	{
   Vector3i seed;
   NsError error;
   const NsImage *image;
	const NsCubei *roi;
	nsint _x, _y, _z;


	//*( workspace_setting_flags( dw->workspace ) ) |= NS_SETTING_NEURITE_SEED;

   image = workspace_volume( dw->workspace );
	roi   = workspace_update_roi( dw->workspace );

   _x = roi->C1.x + ns_cubei_width( roi )  / 2;
   _y = roi->C1.y + ns_cubei_height( roi ) / 2;
   _z = roi->C1.z + ns_cubei_length( roi ) / 2;

	seed.x = _x;
	seed.y = _y;
	seed.z = _z;

   error = ns_no_error();

   switch( dw->display )
      {
      case NS_XY:
         if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _x, _y, NS_SEED_XY_FIND_Z, &_z ), error ) )
            seed.z = ( nsint )_z;
         break;

      case NS_ZY:
         if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _z, _y, NS_SEED_ZY_FIND_X, &_x ), error ) )
            seed.x = ( nsint )_x;
         break;

      case NS_XZ:
         if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _x, _z, NS_SEED_XZ_FIND_Y, &_y ), error ) )
            seed.y = ( nsint )_y;
         break;

      default:
         ns_assert_not_reached();
      }

   if( ! ns_is_error( error ) )
		{
      SetWorkspaceSeed( dw->workspace, &seed );
		_display_window_show_seed_on_status_bar( dw );
		}
   else
      ns_println( "not enough memory to complete seed find" );
	}
*/


void _DisplayWindowMouseModeSeedOnLButtonUp
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
   Vector3i seed;
   //nssize z;
   NsError error;
   const NsImage *image;
	const NsCubei *roi;
nsint _x, _y, _z;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   //if( eDISPLAY_MONTAGE != dw->display )
      {
      if( dw->hasCapture )
         {
         dw->hasCapture = 0;
         ReleaseCapture();
         }

      /* Just pretend another mouse move message came. */

      _DisplayWindowMouseModeSeedOnMouseMove( hWnd, x, y, flags );
      dw->isDragging = 0;

      GetWorkspaceSeed( dw->workspace, &seed );

      if( seed.x != dw->old_seed.x ||
          seed.y != dw->old_seed.y ||
          seed.z != dw->old_seed.z   )
         {
         //*( workspace_setting_flags( dw->workspace ) ) |= NS_SETTING_NEURITE_SEED;

         image = workspace_volume( dw->workspace );
			roi   = workspace_update_roi( dw->workspace );

         _x = ( nsint )seed.x;
         _y = ( nsint )seed.y;
         _z = ( nsint )seed.z;

         error = ns_no_error();

         switch( dw->display )
            {
            case NS_XY:
					if( ____xy_slice_enabled )
						{
						seed.z = ( nsint )____xy_slice_index;
						}
					else
						{
						if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _x, _y, NS_SEED_XY_FIND_Z, &_z ), error ) )
							seed.z = ( nsint )_z;
						}
               break;

            case NS_ZY:
               if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _z, _y, NS_SEED_ZY_FIND_X, &_x ), error ) )
                  seed.x = ( nsint )_x;
               break;

            case NS_XZ:
               if( NS_SUCCESS( _display_window_find_seed( dw, image, roi, _x, _z, NS_SEED_XZ_FIND_Y, &_y ), error ) )
                  seed.y = ( nsint )_y;
               break;

            default:
               ns_assert_not_reached();
            }

         if( ! ns_is_error( error ) )
				{
            SetWorkspaceSeed( dw->workspace, &seed );
				_display_window_show_seed_on_status_bar( dw );

				ns_model_spines_print_clump_mergers( GetWorkspaceNeuronTree( dw->workspace ), seed.x, seed.y, seed.z );
				}
         else
            /*TEMP*/ns_println( "not enough memory to complete seed find" );
         }
      }
   }


void _DisplayWindowMouseModeSeedOnLButtonDblClk
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   );




extern void ____redraw_2d( void );




nsboolean ____alt_key_released = NS_FALSE;

void _display_window_cancel_alt_drag( HWND hWnd )
   {
   if( ____drag_rect_active )
      {
      DisplayWindow *dw;

      dw = _MapHandleToDisplayWindow( hWnd );
      ns_assert( NULL != dw );

      ____drag_rect_active = NS_FALSE;

      if( dw->hasCapture )
         {
         dw->hasCapture = 0;
         ReleaseCapture();
         }

      dw->isDragging = 0;

      ____alt_key_released = NS_TRUE;

      ____redraw_2d();
      }
   }

void _display_windows_cancel_alt_drag( void )
   {  _display_window_cancel_alt_drag( s_DisplayWindowHandles[0] );  }


void ____mouse_mode_rays_get_point(
      DisplayWindow *dw, nsint x, nsint y, NsVector3i *Ps, NsVector3f *Pf,
      nsboolean clip
   )
   {
   Vector2i  corner;
   Vector2i  client;
   Vector2i  global;
   nsfloat   zoom;
   const NsImage *img;
   nsint width, height;
  

   //ns_assert( NS_XY == dw->display );

   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

   MakeVector2i( &client, x, y );

   _DisplayWindowClientToGlobalVector( &global, &client, &corner, zoom, dw->display );

   if( clip )
      {
      if( global.x < 0 )
         global.x = 0;

      if( global.y < 0 )
         global.y = 0;
      }

   if( clip )
      {
      img = GetWorkspaceDisplayImage( dw->workspace, dw->display );

      width  = ( nsint )ns_image_width( img );
      height = ( nsint )ns_image_height( img );

      if( width <= global.x )
         global.x = width - 1;

      if( height <= global.y )
         global.y = height - 1;
      }

   switch( dw->display )
      {
      case NS_XY:
         Ps->x = ( nsint )global.x;
         Ps->y = ( nsint )global.y;
         Ps->z = 0;
         break;

      case NS_ZY:
         Ps->z = ( nsint )global.x;
         Ps->y = ( nsint )global.y;
         Ps->x = 0;
         break;

      case NS_XZ:
         Ps->x = ( nsint )global.x;
         Ps->z = ( nsint )global.y;
         Ps->y = 0;
         break;

      default:
         ns_assert_not_reached();
      }

   ns_to_voxel_space( Ps, Pf, workspace_get_voxel_info( dw->workspace ) );
   }


nsboolean _display_window_is_point_inside_roi( DisplayWindow *dw, const NsVector3i *Ps )
	{
	const NsCubei *roi;
	nsint a, b, a1, a2, b1, b2;
  

	roi = workspace_update_roi( dw->workspace );

   switch( dw->display )
      {
      case NS_XY:
			a     = ( nsint )Ps->x;
			b     = ( nsint )Ps->y;
			a1    = roi->C1.x;
			a2    = roi->C2.x;
			b1    = roi->C1.y;
			b2    = roi->C2.y;
         break;

      case NS_ZY:
			a     = ( nsint )Ps->z;
			b     = ( nsint )Ps->y;
			a1    = roi->C1.z;
			a2    = roi->C2.z;
			b1    = roi->C1.y;
			b2    = roi->C2.y;
         break;

      case NS_XZ:
			a     = ( nsint )Ps->x;
			b     = ( nsint )Ps->z;
			a1    = roi->C1.x;
			a2    = roi->C2.x;
			b1    = roi->C1.z;
			b2    = roi->C2.z;
         break;

      default:
         ns_assert_not_reached();
      }

	return a1 <= a && a <= a2 && b1 <= b && b <= b2;
	}


void ____mouse_mode_rays_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( ____drag_rect_active )
      _zoom_on_mouse_move( hWnd, x, y, flags );
   else
      {
      DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

      //if( NS_XY != dw->display )
      //   return;

      if( dw->isDragging )
         {
         ____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_TRUE );
         SendMessage( GetParent( hWnd ), mMSG_USER_Update, 0, dw->display );
         }
      }
   }


extern void _status_measurement( void );
extern NsRayburstKernelType ____measuring_rays_kernel_type;
extern NsRayburstInterpType ____measurement_interp_type;
extern nsboolean ____measurement_do_3d_rayburst;



#define _MOUSE_MODE_RAYS_ON_SEED( c1, c2, c3, type )\
   const NsImage  *volume;\
	const NsCubei  *roi;\
   nsint          *indices1, *indices2;\
   nsint           num_indices1, num_indices2;\
   NsVector3f      Vf;\
   NsError         error;\
   NsVector3i      Vi;\
   \
   \
   error = ns_no_error();\
   \
   volume = workspace_volume( dw->workspace );\
	roi    = workspace_update_roi( dw->workspace );\
   \
   indices1     = NULL;\
   num_indices1 = 0;\
   indices2     = NULL;\
   num_indices2 = 0;\
   \
   if( NULL != steps_lr )*steps_lr = ( nssize )NS_ABS( dw->P1i.c1 - dw->P2i.c1 );\
   if( NULL != steps_tb )*steps_tb = ( nssize )NS_ABS( dw->P1i.c2 - dw->P2i.c2 );\
   \
   if( NS_FAILURE(\
         _display_window_find_seed_ex(\
				dw,\
            volume,\
				roi,\
            dw->P1i.c1,\
            dw->P1i.c2,\
            type,\
            &indices1,\
            &num_indices1\
            ),\
         error ) )\
      goto _SAMPLE_EXIT;\
   \
   if( do_P2 )\
      if( NS_FAILURE(\
            _display_window_find_seed_ex(\
					dw,\
               volume,\
					roi,\
               dw->P2i.c1,\
               dw->P2i.c2,\
               type,\
               &indices2,\
               &num_indices2\
               ),\
            error ) )\
         goto _SAMPLE_EXIT;\
   \
   if( 1 == num_indices1 )\
      dw->P1i.c3 = ( nsint )indices1[0];\
   else\
      {\
      if( NULL != prev_sample )\
         {\
         ns_sample_get_origin( prev_sample, &Vf );\
         ns_to_image_space( &Vf, &Vi, voxel_info );\
         dw->P1i.c3 = Vi.c3;\
         }\
      else\
         {\
         /*TEMP*/ns_println( "The seed location may not be accurate!" );\
         _display_window_find_seed( dw, volume, roi, dw->P1i.c1, dw->P1i.c2, type, &dw->P1i.c3 );\
         }\
      }\
   \
   if( do_P2 )\
      {\
      if( 1 == num_indices2 )\
         dw->P2i.c3 = ( nsint )indices2[0];\
      else\
         {\
         if( NULL != prev_sample )\
            {\
            ns_sample_get_origin( prev_sample, &Vf );\
            ns_to_image_space( &Vf, &Vi, voxel_info );\
            dw->P2i.c3 = Vi.c3;\
            }\
         else\
            {\
            /*TEMP*/ns_println( "The seed location may not be accurate!" );\
            _display_window_find_seed( dw, volume, roi, dw->P2i.c1, dw->P2i.c2, type, &dw->P2i.c3 );\
            }\
         }\
      }\
   \
   _SAMPLE_EXIT:\
   \
   ns_free( indices1 );\
   ns_free( indices2 );\
   \
   return error


NS_PRIVATE NsError _mouse_mode_rays_on_xy
   (
   DisplayWindow      *dw,
   const NsVoxelInfo  *voxel_info,
   nspointer           prev_sample,
   nssize             *steps_lr,
   nssize             *steps_tb,
   nsboolean           do_P2
   )
   {  _MOUSE_MODE_RAYS_ON_SEED( x, y, z, NS_SEED_XY_FIND_Z );  }

NS_PRIVATE NsError _mouse_mode_rays_on_zy
   (
   DisplayWindow      *dw,
   const NsVoxelInfo  *voxel_info,
   nspointer           prev_sample,
   nssize             *steps_lr,
   nssize             *steps_tb,
   nsboolean           do_P2
   )
   {  _MOUSE_MODE_RAYS_ON_SEED( z, y, x, NS_SEED_ZY_FIND_X );  }

NS_PRIVATE NsError _mouse_mode_rays_on_xz
   (
   DisplayWindow      *dw,
   const NsVoxelInfo  *voxel_info,
   nspointer           prev_sample,
   nssize             *steps_lr,
   nssize             *steps_tb,
   nsboolean           do_P2
   )
   {  _MOUSE_MODE_RAYS_ON_SEED( x, z, y, NS_SEED_XZ_FIND_Y );  }


extern void ____redraw_3d( void );

//extern nsboolean ____markers_active;
//extern nsint ____markers_type;



void _display_window_do_line_of_sight_hack( DisplayWindow *dw )
	{
	NsSampler   *sampler;
	nspointer    sample1, sample2;
	NsVector3f   C1, C2;
	nsfloat      t1, t2;


	sampler = workspace_sampler( dw->workspace );

	if( 2 <= ns_sampler_size( sampler ) )
		{
		sample1 = ns_sampler_last( sampler );
		sample2 = ns_sample_prev( sample1 );

		ns_sample_get_center( sample1, &C1 );
		ns_sample_get_center( sample2, &C2 );

		t1 = ns_sample_get_threshold( sample1 );
		t2 = ns_sample_get_threshold( sample2 );

		ns_println(
			ns_rayburst_targeter(
				&C1,
				&C2,
				NS_MAX( t1, t2 ),
				workspace_volume( dw->workspace ),
				workspace_get_voxel_info( dw->workspace )
				)
			? "HIT" : "MISS"
			);
		}
	}


/*
void _display_window_build_shell_hack( DisplayWindow *dw )
	{
	NsSampler     *sampler;
	nspointer      sample;
	NsVector3f     center;
	nsfloat        radius;
	NsVoxelBuffer  voxel_buffer;
	NsVoxelTable   shell;


	sampler = workspace_sampler( dw->workspace );

	if( 1 <= ns_sampler_size( sampler ) )
		{
		sample = ns_sampler_last( sampler );

		ns_sample_get_center( sample, &center );
		radius = ns_sample_get_radius( sample ) * 1.5f;

		ns_voxel_buffer_init( &voxel_buffer, workspace_volume( dw->workspace ) );

		/*error*//*ns_voxel_table_construct( &shell, NULL );
		/*error*//*ns_grafting_build_shell( &center, radius, &voxel_buffer, workspace_get_voxel_info( dw->workspace ), &shell );

		ns_voxel_table_destruct( &shell );
		}
	}*/


void _mouse_mode_rays_on_lbutton_up_do_run_samples( HWND hWnd, nsint mx, nsint my, const nsuint flags )
   {
   DisplayWindow       *dw;
   nsfloat             radius, length;
   const NsVoxelInfo  *voxel_info;
   nssize              steps_lr, steps_tb, steps;
   NsSampler          *sampler;
   nspointer           prev_sample, curr_sample;
   const NsSettings   *settings;
   nsboolean           did_create_sample;
   NsVector2f          P1f, P2f;
   nsboolean           use_2d_sampling;
   nsfloat             aabbox_scalar;
   nsint               min_window;
   NsVector3i          jitter3i;
   NsVector2i          jitter2i;
   NsError             error;


   error = ns_no_error();

   dw = _MapHandleToDisplayWindow( hWnd );

   //if( NS_XY != dw->display )
   //   return;

   dw->isDragging = 0;

   ____mouse_mode_rays_get_point( dw, mx, my, &dw->P2i, &dw->P2f, NS_TRUE );

   ReleaseCapture();
   ____redraw_2d();

	if( ! _display_window_is_point_inside_roi( dw, &dw->P1i ) ||
		 ! _display_window_is_point_inside_roi( dw, &dw->P2i )   )
		return;

   sampler     = workspace_sampler( dw->workspace );
   prev_sample = ns_sampler_is_empty( sampler ) ? NULL : ns_sampler_last( sampler );
   voxel_info  = workspace_get_voxel_info( dw->workspace );
   settings    = workspace_settings( dw->workspace );

   switch( dw->display )
      {
      case NS_XY:
         if( ! ____xy_slice_enabled )
            error = _mouse_mode_rays_on_xy( dw, voxel_info, prev_sample, &steps_lr, &steps_tb, NS_TRUE );
         else
            {
            steps_lr = ( nssize )NS_ABS( dw->P1i.x - dw->P2i.x );
            steps_tb = ( nssize )NS_ABS( dw->P1i.y - dw->P2i.y );
            }
         break;

      case NS_ZY:
         error = _mouse_mode_rays_on_zy( dw, voxel_info, prev_sample, &steps_lr, &steps_tb, NS_TRUE );
         break;

      case NS_XZ:
         error = _mouse_mode_rays_on_xz( dw, voxel_info, prev_sample, &steps_lr, &steps_tb, NS_TRUE );
         break;

      default:
         ns_assert_not_reached();
      }

   if( ns_is_error( error ) )
      {
      ns_println( "not enough memory to complete sample" );
      return;
      }

   ns_to_voxel_space( &dw->P1i, &dw->P1f, voxel_info );
   ns_to_voxel_space( &dw->P2i, &dw->P2f, voxel_info );

   steps = steps_lr > steps_tb ? steps_lr : steps_tb;

   /*TEMP!!!!!!!!! */
   ns_print_newline();
   ns_println(
      "%.2f,%.2f,%.2f to %.2f,%.2f,%.2f w/ steps " NS_FMT_ULONG,
      dw->P1f.x,dw->P1f.y,dw->P1f.z,dw->P2f.x,dw->P2f.y,dw->P2f.z, steps
      );

   ns_vector3i( &jitter3i, ____jitter_x, ____jitter_y, ____jitter_z );
   /*TEMP*/ns_println( "jitter=%d,%d,%d", jitter3i.x, jitter3i.y, jitter3i.z );

   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

   if( NS_XY == dw->display && ____xy_slice_enabled )
      error =
         ns_sampler_run_2d(
            sampler,
            workspace_volume( dw->workspace ),
            ns_vector3f_to_2f( &dw->P1f, &P1f ),
            ns_vector3f_to_2f( &dw->P2f, &P2f ),
            ns_to_voxel_space_component( ( nsint )____xy_slice_index, voxel_info, NS_COMPONENT_Z ),
            steps,
            ____measurement_interp_type,
            voxel_info,
            workspace_get_average_intensity( dw->workspace ),
            ns_vector3i_to_2i( &jitter3i, &jitter2i ),
            aabbox_scalar,
            min_window,
            ns_settings_get_threshold_use_fixed( settings ),
            ns_settings_get_threshold_fixed_value( settings ),
            &radius,
            &length,
            &did_create_sample
            );
   else
      error =
         ns_sampler_run(
            sampler,
            workspace_volume( dw->workspace ),
            &dw->P1f,
            &dw->P2f,
            steps,
            ____measurement_do_3d_rayburst,//ns_settings_get_neurite_do_3d_radii( settings ),
            ____measuring_rays_kernel_type,
            ____measurement_interp_type,
            voxel_info,
            workspace_get_average_intensity( dw->workspace ),
            &jitter3i,
            //____use_2d_measurement_sampling,
            use_2d_sampling,
            aabbox_scalar,
            min_window,
            ns_settings_get_threshold_use_fixed( settings ),
            ns_settings_get_threshold_fixed_value( settings ),
            &radius,
            &length,
            &did_create_sample
            );

   if( ! ns_is_error( error ) )
      {
      if( did_create_sample )
         {
         if( ( flags & MK_CONTROL ) && NULL != prev_sample )
            ns_sampler_remove( sampler, prev_sample );

         ns_assert( 0 < ns_sampler_size( sampler ) );
         curr_sample = ns_sampler_last( sampler );

         /* TEMP */
         ns_println(
            "volume       = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "surface_area = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "threshold    = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "radius       = " NS_FMT_DOUBLE NS_STRING_NEWLINE
            "length       = " NS_FMT_DOUBLE NS_STRING_NEWLINE,
            ns_sample_get_volume( curr_sample ),
            ns_sample_get_surface_area( curr_sample ),
            ns_sample_get_threshold( curr_sample ),
            radius,
            length
            );

         //if( ____markers_active )
           // ns_sampler_mark( sampler, curr_sample, ____markers_type );

         ____redraw_all();

			//_display_window_do_line_of_sight_hack( dw );
			//_display_window_build_shell_hack( dw );
         }
      }
   /*TEMP*/else
      ns_println( "not enough memory to complete sample" );

   _status_measurement();
   }



void _mouse_mode_rays_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   NsAABBox3d B;

   if( _on_lbutton_up_do_drag_rect( hWnd, x, y, &B ) )
      {
      DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );
      ns_sampler_select_by_aabbox( workspace_sampler( dw->workspace ), &B );

		____redraw_all();
      }
   }



void ____mouse_mode_rays_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
	if( ! ____lbutton_double_clicked )
		{
		if( ____alt_key_is_down )
			_mouse_mode_rays_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			_mouse_mode_rays_on_lbutton_up_do_run_samples( hWnd, x, y, flags );

		____alt_key_released = NS_FALSE;
		}
   }


void ____mouse_mode_rays_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( ____alt_key_is_down )
      {
      ____drag_rect_active = NS_TRUE;
      _zoom_on_lbutton_down( hWnd, x, y, flags );
      }
   else
      {
      DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

      //if( NS_XY != dw->display )
      //   return;

      dw->isDragging = 1;
		dw->drag_mode  = _DRAG_MODE_NORMAL;

      SetCapture( hWnd );

      ____mouse_mode_rays_get_point( dw, x, y, &dw->P1i, &dw->P1f, NS_TRUE );
      }
   }







extern nsboolean ____use_2d_spine_bounding_box;
extern nsint ____spines_interp_type;
extern NsRayburstKernelType ____spines_kernel_type;
extern void _read_eliminate_bases( nsboolean *eliminate_bases );


void ____mouse_mode_spine_get_point( HWND hWnd, nsint x, nsint y, NsVector3i *V )
   {
   DisplayWindow *dw;
   Vector3i        spine_coord;
   Vector2i        corner;
   Vector2i        clientCoord;
   Vector2i        globalCoord;
   HWorkspace      workspace;
   nsuint      index;
   nsint   display;
   nsfloat      zoom;
   nsint       width;
   nsint       height;
      

   dw = _MapHandleToDisplayWindow( hWnd );

   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   spine_coord.x = spine_coord.y = spine_coord.z = 0;

   zoom             = GetWorkspaceZoom( workspace, index, display );
   corner = GetWorkspaceCorner( workspace, index, display );

   MakeVector2i( &clientCoord, x, y );

   _DisplayWindowClientToGlobalVector( &globalCoord,
                                       &clientCoord, 
                                       &corner,
                                       zoom,
                                       display
                                     );

   width  = ( nsint )ns_image_width( GetWorkspaceDisplayImage( workspace, display ) );
   height = ( nsint )ns_image_height( GetWorkspaceDisplayImage( workspace, display ) );

   if( globalCoord.x < 0 )
      globalCoord.x = 0;
   else if( width <= globalCoord.x )
      globalCoord.x = width - 1;

   if( globalCoord.y < 0 )
      globalCoord.y = 0;
   else if( height <= globalCoord.y )
      globalCoord.y = height - 1;

   switch( display )
      {
      case NS_XY:
         MakeVector3i( &spine_coord, globalCoord.x, globalCoord.y, spine_coord.z );
         break;

      case NS_ZY:
         MakeVector3i( &spine_coord, spine_coord.x,  globalCoord.y, globalCoord.x ); 
         break;

      case NS_XZ:
         MakeVector3i( &spine_coord, globalCoord.x, spine_coord.y,  globalCoord.y );
         break;
      }
   
   V->x = ( nsint )spine_coord.x;
   V->y = ( nsint )spine_coord.y;
   V->z = ( nsint )spine_coord.z;
   }


void _mouse_mode_spine_do_add_or_delete( nsuint workspace, const NsVector3f *C, const NsVector3f *A, const nsuint flags )
	{
   nsfloat             radius, length;
   const NsVoxelInfo  *voxel_info;
   NsSampler          *sampler;
   const NsSettings   *settings;
   nsboolean           did_create_sample;
   //NsVector2f        P1f, P2f;
   nsboolean           use_2d_sampling;
   nsfloat             aabbox_scalar;
   nsint               min_window;
	NsVector3i          jitter3i;
	//NsVector2i        jitter2i;
	nsspine             spine, del;
	NsVector3f          Cf;
	NsVector3d          Cd;
	nspointer           sample;
	NsAABBox3d          bbox;
	NsModel            *model;
	nsfloat             threshold;
   NsError             error;
	nsboolean           do_remove;
   //nsboolean         eliminate_bases;
   //nsboolean         did_find;
   //NsVector3i        V;


   sampler     = workspace_sampler( workspace );
   voxel_info  = workspace_get_voxel_info( workspace );
   settings    = workspace_settings( workspace );

   /*TEMP!!!!!!!!! */
   ns_println( "NEW SPINE WILL BE CENTERED AT %.2f,%.2f,%.2f", C->x, C->y, C->z );

   ns_vector3i( &jitter3i, ____jitter_x, ____jitter_y, ____jitter_z );
   /*TEMP*/ns_println( "jitter=%d,%d,%d", jitter3i.x, jitter3i.y, jitter3i.z );

   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

	error =
      ns_sampler_run(
         sampler,
         workspace_volume( workspace ),
         C,
         C,
         0,
         ____measurement_do_3d_rayburst,//ns_settings_get_neurite_do_3d_radii( settings ),
         ____measuring_rays_kernel_type,
         ____measurement_interp_type,
         voxel_info,
         workspace_get_average_intensity( workspace ),
         &jitter3i,
         //____use_2d_measurement_sampling,
         use_2d_sampling,
         aabbox_scalar,
         min_window,
         ns_settings_get_threshold_use_fixed( settings ),
         ns_settings_get_threshold_fixed_value( settings ),
         &radius,
         &length,
         &did_create_sample
         );

   if( ! ns_is_error( error ) )
      {
		if( did_create_sample )
         {
			ns_println( "ok to create a spine since a sample was found." );

         ns_assert( 0 < ns_sampler_size( sampler ) );
			sample = ns_sampler_last( sampler );

			ns_sample_get_center( sample, &Cf );
			ns_println( "NEW SPINE WILL BE CENTERED AT %.2f,%.2f,%.2f", Cf.x, Cf.y, Cf.z );

			threshold = ns_sample_get_threshold( sample );
         ns_sampler_remove( sampler, sample );

			/* Do another sample. This is so if recompute manual spines is chosen, all the results
				should be identical to clicked manual spines. */
			error =
				ns_sampler_run(
					sampler,
					workspace_volume( workspace ),
					&Cf,
					&Cf,
					0,
					____measurement_do_3d_rayburst,//ns_settings_get_neurite_do_3d_radii( settings ),
					____measuring_rays_kernel_type,
					____measurement_interp_type,
					voxel_info,
					workspace_get_average_intensity( workspace ),
					&jitter3i,
					//____use_2d_measurement_sampling,
					use_2d_sampling,
					aabbox_scalar,
					min_window,
					ns_settings_get_threshold_use_fixed( settings ),
					ns_settings_get_threshold_fixed_value( settings ),
					&radius,
					&length,
					&did_create_sample
					);

			if( did_create_sample )
				{
				ns_assert( 0 < ns_sampler_size( sampler ) );
				sample = ns_sampler_last( sampler );

				threshold = ns_sample_get_threshold( sample );
				ns_sampler_remove( sampler, sample );
				}

			model = GetWorkspaceNeuronTree( workspace );

			del = NS_SPINE_NIL;

         if( ( nsboolean )( flags & MK_CONTROL ) )
				{
				if( 1 == ns_model_num_selected_spines_ex( model, &spine ) )
					del = spine;
				else
					del = ns_model_find_last_manual_spine( model );

				if( NS_SPINE_NIL != del )
					ns_model_spines_hold_id_and_type( model, del );
				}

			if( NS_SUCCESS( ns_model_add_spine( model, &spine ), error ) )
				{
				if( NS_SPINE_NIL != del /*&& NS_SPINE_OTHER == ns_spine_get_type( del )*/ )
					ns_model_remove_spine( model, del );

				ns_spine_set_x( spine, Cf.x );
				ns_spine_set_y( spine, Cf.y );
				ns_spine_set_z( spine, Cf.z );
				ns_spine_set_auto_detected( spine, NS_FALSE );

				ns_model_position_and_radius_to_aabbox_ex(
					ns_vector3f_to_3d( &Cf, &Cd ),
					NS_MODEL_SPINES_FIXED_RADIUS,
					&bbox
					);

				/* Set the bounding box so this spine can be selected properly. */
				ns_spine_set_bounding_box( spine, &bbox );

				/* Assume the user clicked on the "head" of the spine, so record the diameter. */
				ns_spine_set_head_diameter( spine, radius * 2.0f );

				/* Deselect all other spines and then select this new one so that the user
					can more easily set its type (by right-clicking). */
				ns_model_select_spines( model, NS_FALSE );
				ns_spine_mark_selected( spine, NS_TRUE );

				if( NS_SUCCESS(
						ns_model_spines_on_add_manual(
							model,
							settings,
							workspace_volume( workspace ),
							threshold,
							____spines_interp_type,
							____spines_kernel_type,
							spine,
							A,
							&do_remove
						),
						error ) )
					if( do_remove )
						{
						/*TEMP*/ns_println( "Removing spine..." );
						ns_model_remove_spine( model, spine );
						}

				____redraw_all();

				/* Simulate a right-click to automatically bring up the pop-up menu. */
				//SendMessage( hWnd, WM_RBUTTONDOWN, 0, MAKELPARAM( mx, my ) );
				}
			else
				ns_println( "not enough memory to create spine" );
			}
      }
	else
      ns_println( "not enough memory to create spine" );


   //ns_println( "spine add begin at %d,%d,%d", V.x, V.y, V.z );

   //_read_eliminate_bases( &eliminate_bases );

   /* TEMP??? Show error if one occurs? */
   /*ns_model_add_or_delete_spine(
      GetWorkspaceNeuronTree( workspace ),
      &V,
      dw->display,
      workspace_settings( workspace ),
      workspace_volume( workspace ),
      ____spines_interp_type,
      ____use_2d_spine_bounding_box,
      workspace_get_average_intensity( workspace ),
      ____spines_kernel_type,
      &did_find
      );*/

   //ns_println( "" );

   //if( did_find )
     // ____redraw_all();
	}


void _mouse_mode_spine_on_lbutton_up_get_attachment( HWND hWnd, nsint mx, nsint my, const nsuint flags )
   {
   DisplayWindow      *dw;
   const NsVoxelInfo  *voxel_info;
   NsError             error;


   error = ns_no_error();
   dw    = _MapHandleToDisplayWindow( hWnd );

   dw->isDragging = 0;

   ____mouse_mode_rays_get_point( dw, mx, my, &dw->P2i, &dw->P2f, NS_TRUE );

	dw->hasCapture = 0;
   ReleaseCapture();

	if( ! _display_window_is_point_inside_roi( dw, &dw->P1i ) ||
		 ! _display_window_is_point_inside_roi( dw, &dw->P2i )   )
		return;

	ns_print_newline();

   voxel_info = workspace_get_voxel_info( dw->workspace );

   switch( dw->display )
      {
      case NS_XY:
         if( ! ____xy_slice_enabled )
            error = _mouse_mode_rays_on_xy( dw, voxel_info, NULL, NULL, NULL, NS_TRUE );
         else
				{
            dw->P1i.z = ( nsint )____xy_slice_index;
            dw->P2i.z = ( nsint )____xy_slice_index;
				}
         break;

      case NS_ZY:
         error = _mouse_mode_rays_on_zy( dw, voxel_info, NULL, NULL, NULL, NS_TRUE );
         break;

      case NS_XZ:
         error = _mouse_mode_rays_on_xz( dw, voxel_info, NULL, NULL, NULL, NS_TRUE );
         break;

      default:
         ns_assert_not_reached();
      }

   if( ns_is_error( error ) )
      {
      ns_println( "not enough memory to create spine" );
      return;
      }

	ns_to_voxel_space( &dw->P1i, &dw->P1f, voxel_info );
	ns_to_voxel_space( &dw->P2i, &dw->P2f, voxel_info );

	_mouse_mode_spine_do_add_or_delete( dw->workspace, &dw->P1f, &dw->P2f, flags );
   }


void _mouse_mode_spine_on_lbutton_down_get_center( HWND hWnd, nsint mx, nsint my, const nsuint flags )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

   ____mouse_mode_rays_get_point( dw, mx, my, &dw->P1i, &dw->P1f, NS_TRUE );

   SetCapture( hWnd );
   dw->hasCapture = 1;

   dw->isDragging = 1;
   }


void _mouse_mode_spine_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   NsAABBox3d B;

   if( _on_lbutton_up_do_drag_rect( hWnd, x, y, &B ) )
      {
      /*error*/ns_model_select_spines_by_aabbox(
         GetWorkspaceNeuronTree( _MapHandleToDisplayWindow( hWnd )->workspace ),
         &B
         );

		____redraw_all();
      }
   }


void _mouse_mode_spine_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
	if( ! ____lbutton_double_clicked )
		{
		if( ____alt_key_is_down )
			_mouse_mode_spine_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			_mouse_mode_spine_on_lbutton_up_get_attachment( hWnd, x, y, flags );

		____alt_key_released = NS_FALSE;
		}
   }


void _mouse_mode_spine_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
	if( ! ____lbutton_double_clicked )
		{
		if( ____alt_key_is_down )
			{
			____drag_rect_active = NS_TRUE;
			_zoom_on_lbutton_down( hWnd, x, y, flags );
			}
		else if( ! ____alt_key_released )
			_mouse_mode_spine_on_lbutton_down_get_center( hWnd, x, y, flags );
		}
   }


void _mouse_mode_spine_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( ____drag_rect_active )
      _zoom_on_mouse_move( hWnd, x, y, flags );
	else
		____mouse_mode_rays_on_mouse_move( hWnd, x, y, flags );
   }


extern void _on_add_selected_spines_to_classifier( void );


void _mouse_mode_spine_do_type_popup( HWND hWnd, nsuint workspace, nsint x, nsint y )
	{
   MENUITEMINFO         miim;
   HMENU                hMenu;
   POINT                pt;
   nsint                id;
	nsboolean            redraw;
   NsSpineType          type; //old_spine_type, new_spine_type;
	NsSpinesClassifier  *sc;
	nsspineclass         curr_class, end_classes;
	nsint                index;
   //NsVector3i           V;


	redraw = NS_FALSE;

   //____mouse_mode_spine_get_point( hWnd, x, y, &V );

   //if( ! ns_model_get_spine_type(
     //       GetWorkspaceNeuronTree( workspace ),
       //     &V,
            //dw->display,
            //workspace_settings( workspace ),
            //workspace_volume( workspace ),
            //&old_spine_type ) )
      //return;

   hMenu = CreatePopupMenu();

   miim.cbSize = sizeof( MENUITEMINFO );
   miim.fMask  = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   miim.fType  = MFT_STRING;
   miim.fState = MFS_ENABLED;

	pt.x = x;
	pt.y = y;

	ClientToScreen( hWnd, &pt );

	#define ____SPINE_BASE_ID               72840
	#define ____SPINE_DELETE_ID             ( ____SPINE_BASE_ID - 1 )
	#define ____SPINE_ADD_TO_CLASSIFIER_ID  ( ____SPINE_BASE_ID - 2 )
	#define ____SPINE_STUBBY_ID             ( ____SPINE_BASE_ID + 0 )
	#define ____SPINE_THIN_ID               ( ____SPINE_BASE_ID + 1 )
	#define ____SPINE_MUSHROOM_ID           ( ____SPINE_BASE_ID + 2 )
	#define ____SPINE_OTHER_ID              ( ____SPINE_BASE_ID + 3 )

	if( NULL == ( sc = ns_spines_classifier_get() ) )
		{
		miim.wID        = ____SPINE_STUBBY_ID;
		miim.dwTypeData = "Stubby";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Stubby" );
		//if( NS_SPINE_STUBBY == old_spine_type )miim.fState |= MFS_CHECKED;
		InsertMenuItem( hMenu, 1, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.wID        = ____SPINE_THIN_ID;
		miim.dwTypeData = "Thin";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Thin" );
		InsertMenuItem( hMenu, 2, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.wID        = ____SPINE_MUSHROOM_ID;
		miim.dwTypeData = "Mushroom";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Mushroom" );
		InsertMenuItem( hMenu, 3, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.wID        = ____SPINE_OTHER_ID;
		miim.dwTypeData = "Other";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Other" );
		InsertMenuItem( hMenu, 4, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.wID        = ____SPINE_DELETE_ID;
		miim.dwTypeData = "Delete";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Delete" );
		InsertMenuItem( hMenu, 5, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.fMask  = 0;
		miim.fType  = MFT_SEPARATOR;
		InsertMenuItem( hMenu, 4, TRUE, &miim );
		}
	else
		{
		index       = 1;
		curr_class  = ns_spines_classifier_begin( sc );
		end_classes = ns_spines_classifier_end( sc );

		for( ; ns_spine_class_not_equal( curr_class, end_classes );
				 curr_class = ns_spine_class_next( curr_class ) )
			{
			ns_assert( NS_SPINE_INVALID != ns_spine_class_value( curr_class ) );

			miim.wID        = ____SPINE_BASE_ID + ns_spine_class_value( curr_class );
			miim.dwTypeData = ( nschar* )ns_spine_class_name( curr_class );
			miim.cch        = ( nsuint )ns_ascii_strlen( ns_spine_class_name( curr_class ) );
			InsertMenuItem( hMenu, index, TRUE, &miim );
			miim.fState &= ~MFS_CHECKED;

			++index;
			}

		miim.wID        = ____SPINE_ADD_TO_CLASSIFIER_ID;
		miim.dwTypeData = "Add to Classifier";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Add to Classifier" );
		InsertMenuItem( hMenu, index, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.wID        = ____SPINE_DELETE_ID;
		miim.dwTypeData = "Delete";
		miim.cch        = ( nsuint )ns_ascii_strlen( "Delete" );
		InsertMenuItem( hMenu, index + 1, TRUE, &miim );
		miim.fState &= ~MFS_CHECKED;

		miim.fMask  = 0;
		miim.fType  = MFT_SEPARATOR;
		InsertMenuItem( hMenu, index - 1, TRUE, &miim );

		miim.fMask  = 0;
		miim.fType  = MFT_SEPARATOR;
		InsertMenuItem( hMenu, index + 1, TRUE, &miim );
		}

	id = TrackPopupMenu( 
			hMenu,                  
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
			pt.x,
			pt.y,
			0,
			hWnd,
			NULL
			);

	DestroyMenu( hMenu );

	if( NULL == ns_spines_classifier_get() )
		{
		type = NS_SPINE_INVALID;

		switch( id )
			{
			case ____SPINE_STUBBY_ID:
				type   = NS_SPINE_STUBBY;
				redraw = NS_TRUE;
				break;

			case ____SPINE_THIN_ID:
				type   = NS_SPINE_THIN;
				redraw = NS_TRUE;
				break;

			case ____SPINE_MUSHROOM_ID:
				type   = NS_SPINE_MUSHROOM;
				redraw = NS_TRUE;
				break;

			case ____SPINE_OTHER_ID:
				type   = NS_SPINE_OTHER;
				redraw = NS_TRUE;
				break;

			case ____SPINE_DELETE_ID:
				ns_model_delete_selected_spines( GetWorkspaceNeuronTree( workspace ) );
				redraw = NS_TRUE;
				break;
			}

		if( NS_SPINE_INVALID != type )
			ns_model_type_selected_spines( GetWorkspaceNeuronTree( workspace ), type );
		}
	else
		{
		curr_class = ns_spines_classifier_find_by_value( sc, id - ____SPINE_BASE_ID );

		if( ns_spine_class_not_equal( curr_class, ns_spines_classifier_end( sc ) ) )
			{
			ns_model_type_selected_spines( GetWorkspaceNeuronTree( workspace ), ( NsSpineType )ns_spine_class_value( curr_class ) );
			redraw = NS_TRUE;
			}
		else if( ____SPINE_ADD_TO_CLASSIFIER_ID == id )
			{
			ns_assert( NULL != ns_spines_classifier_get() );
			_on_add_selected_spines_to_classifier();
			}
		else if( ____SPINE_DELETE_ID == id )
			{
			ns_model_delete_selected_spines( GetWorkspaceNeuronTree( workspace ) );
			redraw = NS_TRUE;
			}
		}

	if( redraw )
		____redraw_all();
	}


void _mouse_mode_spine_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );
	_mouse_mode_spine_do_type_popup( dw->hWnd, dw->workspace, x, y );
   }


void _mouse_mode_neurite_on_mouse_move( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( ____drag_rect_active )
      _zoom_on_mouse_move( hWnd, x, y, flags );
   else
		{
		DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

		if( _DRAG_MODE_TRANSLATE == dw->drag_mode )
			_display_window_on_translation( dw, x, y );
		else if( _DRAG_MODE_ROTATE == dw->drag_mode )
			_display_window_on_rotation( dw, x, y );
		else
			____mouse_mode_rays_on_mouse_move( hWnd, x, y, flags );
		}
   }


void _mouse_mode_neurite_on_lbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   if( ____alt_key_is_down )
      {
      ____drag_rect_active = NS_TRUE;
      _zoom_on_lbutton_down( hWnd, x, y, flags );
      }
   else
		{
		DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

		dw->drag_mode  = _DRAG_MODE_NORMAL;
		dw->isDragging = 1;

		if( flags & MK_CONTROL )
			_display_window_begin_translation( dw, x, y );
		else if( flags & MK_SHIFT )
			_display_window_begin_rotation( dw, x, y );
		else
			____mouse_mode_rays_on_lbutton_down( hWnd, x, y, flags );
		}
   }


//extern void _on_delete_selected_vertices( void );


void _mouse_mode_neurite_on_lbutton_up_do_drag_rect( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   NsAABBox3d B;

   if( _on_lbutton_up_do_drag_rect( hWnd, x, y, &B ) )
      {
      DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

      ///*error*/ns_model_select_vertices_by_aabbox(
        // GetWorkspaceNeuronTree( _MapHandleToDisplayWindow( hWnd )->workspace ),
         //&B
         //);

      //ns_model_select_vertices_by_aabbox( workspace_raw_model( dw->workspace ), &B );
      ns_model_select_vertices_by_aabbox( workspace_filtered_model( dw->workspace ), &B );

      //_on_delete_selected_vertices();

		____redraw_all();
      }
   }


extern void _run_neurite_tracer( const NsVector3f*, const NsVector3f* );
extern void _on_delete_selected_vertices( void );

nsboolean ____launching_neurite_tracer;
#include <std/nsthread.h>

#define _NEURITE_TRACER_SENSITIVITY  4.0f

extern void _on_convert_measurements( void );

void _mouse_mode_neurite_on_lbutton_up_do_neurite_tracer( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   DisplayWindow      *dw;
   nsfloat             radius, length;
   const NsModel      *model;
   const NsVoxelInfo  *voxel_info;
   NsSampler          *sampler;
   const NsSettings   *settings;
   nsboolean           did_create_sample;
   //NsRay3f             R;
   NsVector3i          jitter;
   nsboolean           use_2d_sampling;
   nsfloat             aabbox_scalar;
	nsfloat             distance;
   nsint               min_window;
   NsError             error;


   did_create_sample = NS_FALSE;

   dw = _MapHandleToDisplayWindow( hWnd );

   //if( NS_XY != dw->display )
   //   return;

   dw->isDragging = 0;

   ____mouse_mode_rays_get_point( dw, x, y, &dw->P2i, &dw->P2f, NS_TRUE );

   ReleaseCapture();
   ____redraw_2d();

	if( ! _display_window_is_point_inside_roi( dw, &dw->P1i ) ||
		 ! _display_window_is_point_inside_roi( dw, &dw->P2i )   )
		return;

	if( dw->P1i.x == dw->P2i.x && dw->P1i.y == dw->P2i.y && dw->P1i.z == dw->P2i.z )
		distance = 0.0f;
	else if( ( distance = ns_vector3i_distance( &dw->P1i, &dw->P2i ) ) < _NEURITE_TRACER_SENSITIVITY )
		{
		ns_println( "have to drag that neurite tracer a little farther." );
		return;
		}

   model      = GetWorkspaceNeuronTree( dw->workspace );
   sampler    = workspace_sampler( dw->workspace );
   voxel_info = workspace_get_voxel_info( dw->workspace );
   settings   = workspace_settings( dw->workspace );
	error      = ns_no_error();

   switch( dw->display )
      {
      /* IMPORTANT: The neurite tracer directions operate in XY only,
         therefore we dont need to seed find P2 if in XY mode... BUT
         we do need to seed find if in the either viewing directions! */
      case NS_XY:
			if( ! ____xy_slice_enabled )
				error = _mouse_mode_rays_on_xy( dw, voxel_info, NULL, NULL, NULL, NS_FALSE );
			else
				{
				dw->P1i.z = ( nsint )____xy_slice_index;
				dw->P2i.z = ( nsint )____xy_slice_index;
				}
         break;

      case NS_ZY:
         error = _mouse_mode_rays_on_zy( dw, voxel_info, NULL, NULL, NULL, NS_TRUE );
         break;

      case NS_XZ:
         error = _mouse_mode_rays_on_xz( dw, voxel_info, NULL, NULL, NULL, NS_TRUE );
         break;

      default:
         ns_assert_not_reached();
      }

   if( ns_is_error( error ) )
      {
      /*TEMP*/ns_println( "not enough memory to complete sample %d", __LINE__ );
      return;
      }

/*TEMP*/ns_println( "P1i = %d,%d,%d  P2i = %d,%d,%d",
   dw->P1i.x, dw->P1i.y, dw->P1i.z, dw->P2i.x, dw->P2i.y, dw->P2i.z );

   ns_to_voxel_space( &dw->P1i, &dw->P1f, voxel_info );
   ns_to_voxel_space( &dw->P2i, &dw->P2f, voxel_info );

   /* Check if the user clicked on a vertex before running the neurite tracer. */
/*
   switch( dw->display )
      {
      case NS_XY:
         R.O.x = dw->P1f.x;
         R.O.y = dw->P1f.y;
         R.O.z = 0.0f;
         R.D.x = 0.0f;
         R.D.y = 0.0f;
         R.D.z = 1.0f;
         break;

      case NS_ZY:
         R.O.x = 0.0f;
         R.O.y = dw->P1f.y;
         R.O.z = dw->P1f.z;
         R.D.x = -1.0f;
         R.D.y = 0.0f;
         R.D.z = 0.0f;
         break;

      case NS_XZ:
         R.O.x = dw->P1f.x;
         R.O.y = 0.0f;
         R.O.z = dw->P1f.z;
         R.D.x = 0.0f;
         R.D.y = 1.0f;
         R.D.z = 0.0f;
         break;

      default:
         ns_assert_not_reached();
      }
*/

   /*
   ns_println( "running vertex intersection test..." ); //TEMP
   if( ns_model_vertices_intersect_ray( model, &R, &V ) )
      {
      ns_println( "removing intersected vertex..." );//TEMP

      ns_model_vertex_mark_selected( V, NS_TRUE );
      ____redraw_all();
      //ns_model_save_selected_vertices( model );

      //ns_model_vertex_mark_selected( V );
      _on_delete_selected_vertices();

      //ns_model_restore_selected_vertices( model );
      return;
      }
   */

   /*TEMP*/
   ns_println( "" );
   ns_println( "%.2f,%.2f,%.2f", dw->P1f.x,dw->P1f.y,dw->P1f.z );
   ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );
   ns_println( "jitter=%d,%d,%d", jitter.x, jitter.y, jitter.z );

   if( 0 < ns_sampler_size( sampler ) )
      if( IDNO == MessageBox( dw->hWnd,
         "Existing measurements may not be connected properly. Proceed?", "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
         return;

   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );


   if( NS_SUCCESS( ns_sampler_run(
                     sampler,
                     workspace_volume( dw->workspace ),
                     &dw->P1f,
                     &dw->P1f,
                     0,
                     ____measurement_do_3d_rayburst,//ns_settings_get_neurite_do_3d_radii( settings ),
                     ____measuring_rays_kernel_type,
                     ____measurement_interp_type,
                     voxel_info,
                     workspace_get_average_intensity( dw->workspace ),
                     &jitter,
                     //____use_2d_measurement_sampling,
                     use_2d_sampling,
                     aabbox_scalar,
                     min_window,
                     ns_settings_get_threshold_use_fixed( settings ),
                     ns_settings_get_threshold_fixed_value( settings ),
                     &radius,
                     &length,
                     &did_create_sample
                     ),
                     error ) )
      {
      //if( did_create_sample && 0 < ns_sampler_size( sampler ) )
      //   {
      //   sample = ns_sampler_last( sampler );
      //   ns_assert( NULL != sample );
      //   ns_println( "volume = " NS_FMT_DOUBLE ", surface_area = " NS_FMT_DOUBLE ", threshold = " NS_FMT_DOUBLE,
      //   ns_sample_get_volume( sample ), ns_sample_get_surface_area( sample ), ns_sample_get_threshold( sample ) );
      //   }

      //ns_println( "radius = " NS_FMT_DOUBLE ", length = " NS_FMT_DOUBLE, radius, length );
      }
   else
      ns_println( "not enough memory to complete sample %d", __LINE__ );

	/* NOTE: Dont need to launch neurite tracer for single click, i.e.
		where the distance between the button down and button up location
		is 0. */
	if( ! NS_FLOAT_EQUAL( distance, 0.0f ) )
		{
		____launching_neurite_tracer = NS_FALSE;

		if( did_create_sample )
			{
			____launching_neurite_tracer = NS_TRUE;
			_run_neurite_tracer( &dw->P1f, &dw->P2f );
			}

		/* NOTE: Wait until the thread that does the tracing is activated. User could
			potentially delete the last sample before the thread gets going and it needs
			this last sample. */
		while( ____launching_neurite_tracer )
			{
			ns_println( "waiting until neurite tracer is activated..." );
			ns_thread_yield();
			}
		}
	else
		{
		_on_convert_measurements();
		____redraw_all();
		}
   }


void _mouse_mode_neurite_on_lbutton_up( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
	if( ! ____lbutton_double_clicked )
		{
		DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

		if( ____alt_key_is_down )
			_mouse_mode_neurite_on_lbutton_up_do_drag_rect( hWnd, x, y, flags );
		else if( ! ____alt_key_released )
			{
			if( _DRAG_MODE_TRANSLATE == dw->drag_mode )
				_display_window_end_translation( dw, x, y );
			else if( _DRAG_MODE_ROTATE == dw->drag_mode )
				_display_window_end_rotation( dw, x, y );
			else
				_mouse_mode_neurite_on_lbutton_up_do_neurite_tracer( hWnd, x, y, flags );
			}

		____alt_key_released = NS_FALSE;

		dw->isDragging = 0;
		dw->drag_mode  = _DRAG_MODE_NORMAL;
		}
   }


extern void _on_edit_set_order_of_selected_neurites( void );



void _mouse_mode_neurite_do_labeling_popup( HWND hWnd, nsuint workspace, nsint x, nsint y )
	{
   MENUITEMINFO          miim;
   HMENU                 hMenu;
   POINT                 pt;
   nsint                 id;
	NsModelFunctionType   type;
	nsint                 result;



   hMenu = CreatePopupMenu();

   miim.cbSize = sizeof( MENUITEMINFO );
   miim.fMask  = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   miim.fType  = MFT_STRING;
   miim.fState = MFS_ENABLED;

#define ____FUNCTION_UNKNOWN_ID           72845
#define ____FUNCTION_SOMA_ID              72846
#define ____FUNCTION_BASAL_DENDRITE_ID    72847
#define ____FUNCTION_APICAL_DENDRITE_ID   72848
#define ____FUNCTION_AXON_ID              72849
#define ____FUNCTION_SELECT_SUB_TREES_ID  72850
#define ____FUNCTION_SELECT_SET_ORDER_OF_SELECTED_NEURITES  72851

   miim.wID        = ____FUNCTION_SOMA_ID;
   miim.dwTypeData = "Soma";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Soma" );
   InsertMenuItem( hMenu, 1, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_BASAL_DENDRITE_ID;
   miim.dwTypeData = "Basal Dendrite";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Basal Dendrite" );
   InsertMenuItem( hMenu, 2, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_APICAL_DENDRITE_ID;
   miim.dwTypeData = "Apical Dendrite";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Apical Dendrite" );
   InsertMenuItem( hMenu, 3, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_AXON_ID;
   miim.dwTypeData = "Axon";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Axon" );
   InsertMenuItem( hMenu, 4, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_UNKNOWN_ID;
   miim.dwTypeData = "Unknown";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Unknown" );
   InsertMenuItem( hMenu, 5, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_SELECT_SUB_TREES_ID;
   miim.dwTypeData = "Select Sub-Trees";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Select Sub-Trees" );
   InsertMenuItem( hMenu, 6, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;

   miim.wID        = ____FUNCTION_SELECT_SET_ORDER_OF_SELECTED_NEURITES;
   miim.dwTypeData = "Set Order...";
   miim.cch        = ( nsuint )ns_ascii_strlen( "Set Order" );
   InsertMenuItem( hMenu, 7, TRUE, &miim );
   miim.fState &= ~MFS_CHECKED;


   miim.fMask  = 0;
   miim.fType  = MFT_SEPARATOR;
	InsertMenuItem( hMenu, 5, TRUE, &miim );


   pt.x = x;
   pt.y = y;

   ClientToScreen( hWnd, &pt );

   id = TrackPopupMenu( hMenu,                  
                        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                        pt.x,
                        pt.y,
                        0,
                        hWnd,
                        NULL
                      );

   DestroyMenu( hMenu );

   type = -1;

   switch( id )
      {
      case ____FUNCTION_UNKNOWN_ID:
         type = NS_MODEL_FUNCTION_UNKNOWN;
         break;

      case ____FUNCTION_SOMA_ID:
         type = NS_MODEL_FUNCTION_SOMA;
         break;

      case ____FUNCTION_BASAL_DENDRITE_ID:
         type = NS_MODEL_FUNCTION_BASAL_DENDRITE;
         break;

      case ____FUNCTION_APICAL_DENDRITE_ID:
         type = NS_MODEL_FUNCTION_APICAL_DENDRITE;
         break;

      case ____FUNCTION_AXON_ID:
         type = NS_MODEL_FUNCTION_AXON;
         break;

		case ____FUNCTION_SELECT_SUB_TREES_ID:
			/*error*/ns_model_select_sub_trees_of_selected_vertices( GetWorkspaceNeuronTree( workspace ) );
			____redraw_all();
         break;

		case ____FUNCTION_SELECT_SET_ORDER_OF_SELECTED_NEURITES:
			_on_edit_set_order_of_selected_neurites();
			break;
      }

	if( 0 <= type )
		{
		ns_model_function_selected_objects( GetWorkspaceNeuronTree( workspace ), type, &result );
		____redraw_all();

		switch( result )
			{
			case NS_MODEL_FUNCTION_SELECTED_OK:
				break;

			case NS_MODEL_FUNCTION_SELECTED_SOMA_NOT_CONNECTED:
				MessageBox( hWnd, "Cannot create multiple somas within a single component of the model.", "NeuronStudio", MB_OK | MB_ICONINFORMATION );
				break;
			
			case NS_MODEL_FUNCTION_SELECTED_SOMA_DISCONNECTED:
				MessageBox( hWnd, "Dividing somas is not allowed.", "NeuronStudio", MB_OK | MB_ICONINFORMATION );
				break;

			default:
				ns_assert_not_reached();
			}
		}
	}


void _mouse_mode_neurite_on_rbutton_down( HWND hWnd, nsint x, nsint y, const nsuint flags )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );
	_mouse_mode_neurite_do_labeling_popup( dw->hWnd, dw->workspace, x, y );
   }





/*BEGIN******************************************************************/
void _mouse_mode_magnet_on_mouse_move
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

   if( dw->isDragging )
      {
      switch( dw->drag_mode )
         {
         case _DRAG_MODE_NORMAL:
            {
            HWorkspace     workspace;
            nsuint     index;
            nsint  display;
            Vector2i       oldClientCoord;
            Vector2i       newClientCoord;
            Vector2i       oldGlobalCoord;
            Vector2i       newGlobalCoord;
            Vector2i       difference;
            Vector2i       corner;
            nsfloat     zoom;


            workspace = dw->workspace;
            index      = dw->index;
            display    = dw->display;

            //if( eDISPLAY_MONTAGE == display )
              // return;

            corner = GetWorkspaceCorner( workspace, index, display );
            zoom             = GetWorkspaceZoom( workspace, index, display );

            oldClientCoord = dw->clientCoord;
            MakeVector2i( &newClientCoord, x, y );

            _DisplayWindowClientToGlobalVector( &oldGlobalCoord,
                                                &oldClientCoord,
                                                &corner,
                                                zoom,
                                                display
                                              );

            _DisplayWindowClientToGlobalVector( &newGlobalCoord,
                                                &newClientCoord, 
                                                &corner,
                                                zoom,
                                                display
                                              );

            /* Translate the view corner. */

            SubVector2i( &oldGlobalCoord, &newGlobalCoord, &difference );
            AddVector2i( &corner, &difference, &corner );

            SetWorkspaceCorner( workspace, index, display, corner );

            dw->clientCoord = newClientCoord;

            _CenterOrLimitDisplayWindowImage( dw );
            DrawDisplayWindow( hWnd );
            }
         break;

         case _DRAG_MODE_TRANSLATE:
            _display_window_on_translation( dw, x, y );
            break;
         }
      }
   }


void _temp_on_lbutton_down_do_drag_rect( HWND hWnd, nsint x, nsint y, NsAABBox3d *B )
   {
   DisplayWindow  *dw;
	NsDragRect      drag_rect;
	NsVector2i      corner;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

	ns_drag_rect_start( &drag_rect, x, y );
	ns_drag_rect_end( &drag_rect, x + 1, y + 1 );

	corner.x = dw->corner.x;
	corner.y = dw->corner.y;

   ns_model_client_space_drag_rect_to_aabbox(
      &drag_rect,
      workspace_get_voxel_info( dw->workspace ),
      dw->display,
		&corner,
		dw->zoom,
      B
      );
   }


void _mouse_mode_magnet_on_lbutton_down
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
	NsAABBox3d B;
	nssize num_intersections;


	dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   SetCapture( hWnd );
   dw->hasCapture = 1;

   MakeVector2i( &dw->clientCoord, x, y );

	dw->drag_mode = _DRAG_MODE_NORMAL;

	_temp_on_lbutton_down_do_drag_rect( hWnd, x, y, &B );
	ns_model_select_vertices( workspace_filtered_model( dw->workspace ), NS_FALSE );

	num_intersections = 0;
	ns_model_select_vertices_by_aabbox_ex( workspace_filtered_model( dw->workspace ), &B, &num_intersections );

	if( 0 < num_intersections )
		_display_window_begin_translation( dw, x, y );

   dw->isDragging = 1;
   }


void _mouse_mode_magnet_on_lbutton_up
   ( 
   HWND hWnd,
   nsint x,
   nsint y,
   const nsuint flags
   )
   {
   DisplayWindow *dw;
   RECT rc;


   dw = _MapHandleToDisplayWindow( hWnd );
   ns_assert( NULL != dw );

   if( dw->hasCapture )
      {
      dw->hasCapture = 0;
      ReleaseCapture();

      GetClientRect( hWnd, &rc );

      switch( dw->drag_mode )
         {
         case _DRAG_MODE_TRANSLATE:
				_display_window_end_translation( dw, x, y );
            break;
         }
      }

   dw->isDragging = 0;
   }


static MouseMode s_MouseModes[mDISPLAY_WINDOW_NUM_MOUSE_MODES] =
   {
	{
   NULL,
   _zoom_on_mouse_move,
   _zoom_on_lbutton_down,
   _zoom_on_lbutton_up,
   NULL,
   _zoom_on_rbutton_down,
   NULL,
   _DisplayWindowMouseModeOnUnset,
   _DisplayWindowMouseModeZoomOnSet
   },

  {
   NULL,
   _DisplayWindowMouseModeDragOnMouseMove,
   _DisplayWindowMouseModeDragOnLButtonDown,
   _DisplayWindowMouseModeDragOnLButtonUp,
   _display_window_mouse_mode_drag_on_lbutton_dbl_clk,
   NULL,
   NULL,
   _DisplayWindowMouseModeOnUnset,
   _DisplayWindowMouseModeDragOnSet
   },

   {
   NULL,
   _DisplayWindowMouseModeSeedOnMouseMove,
   _DisplayWindowMouseModeSeedOnLButtonDown,
   _DisplayWindowMouseModeSeedOnLButtonUp,
   NULL,//_DisplayWindowMouseModeSeedOnLButtonDblClk,
   NULL,
   NULL,
   _DisplayWindowMouseModeSeedOnUnset,
   _DisplayWindowMouseModeSeedOnSet
   },

   {
   NULL,
   ____mouse_mode_rays_on_mouse_move,
   ____mouse_mode_rays_on_lbutton_down,
   ____mouse_mode_rays_on_lbutton_up,
   NULL,
   NULL,//____mouse_mode_rays_on_rbutton_down,
   NULL,
   _DisplayWindowMouseModeRaysOnUnset,
   _DisplayWindowMouseModeRaysOnSet
   },

   {
   NULL,
   _mouse_mode_spine_on_mouse_move,
   _mouse_mode_spine_on_lbutton_down,
   _mouse_mode_spine_on_lbutton_up,
   NULL,
   _mouse_mode_spine_on_rbutton_down,
   NULL,
   _DisplayWindowMouseModeOnUnset,
   _DisplayWindowMouseModeSpineOnSet
   },

   {
   NULL,
   _mouse_mode_neurite_on_mouse_move,
   _mouse_mode_neurite_on_lbutton_down,
   _mouse_mode_neurite_on_lbutton_up,
   NULL,
   _mouse_mode_neurite_on_rbutton_down,
   NULL,//_mouse_mode_neurite_on_rbutton_up,
   _DisplayWindowMouseModeOnUnset,
   _DisplayWindowMouseModeNeuriteOnSet
   },

	{
   NULL,
   _mouse_mode_roi_on_mouse_move,
   _mouse_mode_roi_on_lbutton_down,
   _mouse_mode_roi_on_lbutton_up,
   NULL,
   _mouse_mode_roi_on_rbutton_down,
   NULL,
   _DisplayWindowMouseModeROIOnUnset,
   _DisplayWindowMouseModeROIOnSet
   },

	{
   NULL,
   _mouse_mode_magnet_on_mouse_move,
   _mouse_mode_magnet_on_lbutton_down,
   _mouse_mode_magnet_on_lbutton_up,
   NULL,
   NULL,
   NULL,
   _DisplayWindowMouseModeOnUnset,
   _DisplayWindowMouseModeMagnetOnSet
   }
   };


void _DrawDisplayWindowNeuronTree( DisplayWindow* dw );

nschar ____paint_text[256];


#include "invert-line.inl"


nsboolean ____display_window_do_screen_capture = NS_FALSE;

void _display_window_screen_capture( DisplayWindow *dw, nsint width, nsint height )
	{
	NsImage     scan_aligned, no_alignment;
	BITMAPINFO  info;
	nschar      file_name[ NS_PATH_SIZE ];
	NsError     error;


	if( 0 == dw->workspace )
		return;

	ns_println( "Doing 2D screen capture..." );

	ns_image_construct( &scan_aligned );
	ns_image_construct( &no_alignment );

	if( NS_FAILURE( ns_image_create( &scan_aligned, NS_PIXEL_RGB_U8_U8_U8, ( nssize )width, ( nssize )height, 1, sizeof( DWORD ) ), error ) )
		goto _SCREEN_CAPTURE_EXIT;

	info.bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
	info.bmiHeader.biWidth         = width;
	info.bmiHeader.biHeight        = -height;
	info.bmiHeader.biPlanes        = 1;
	info.bmiHeader.biBitCount      = 24;
	info.bmiHeader.biCompression   = BI_RGB;
	info.bmiHeader.biSizeImage     = 0;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biClrUsed       = 0;
	info.bmiHeader.biClrImportant  = 0;

	GetDIBits(
		dw->backBuffer.hDC,
		dw->backBuffer.hBuffer,
		0,
		( UINT )height,
		ns_image_pixels( &scan_aligned ),
		&info,
		DIB_RGB_COLORS
		);

	ns_image_set_pixel_proc_db( &scan_aligned, workspace_pixel_proc_db( dw->workspace ) );

	if( NS_FAILURE( ns_image_convert( &scan_aligned, NS_PIXEL_BGR_U8_U8_U8, 1, &no_alignment, NULL ), error ) )
		goto _SCREEN_CAPTURE_EXIT;

	ns_sprint( file_name, "%s" NS_STRING_PATH "test.jpg", _startup_directory );

	ns_image_set_io_proc_db( &no_alignment, workspace_io_proc_db( dw->workspace ) );
	ns_image_write_jpeg( &no_alignment, file_name, 85, NULL );

	_SCREEN_CAPTURE_EXIT:

	ns_image_destruct( &scan_aligned );
	ns_image_destruct( &no_alignment );
	}


extern nsboolean ____view_roi_always_on_top;
extern nsboolean ____view_seed_always_on_top;



void _dispay_window_do_draw_roi_borders
	(
	DisplayWindow *dw,
	nsint  Ax,  nsint  Ay,
	nsint  Bx,  nsint  By,
	nsint  C1x, nsint  C1y,
	nsint  C2x, nsint  C2y
	)
	{
	NsColor4ub  src, dest;
	RECT        r;
	HBRUSH      brush;


	src = workspace_get_color( dw->workspace, WORKSPACE_COLOR_BACKGROUND );
   ns_color4ub_invert( &src, &dest );
	brush = CreateSolidBrush(____TO_COLORREF( dest ) );

	/* Left rectangle. */
	r.left   = ( nsint )Ax;
	r.top    = ( nsint )Ay;
	r.right  = ( nsint )C1x;
	r.bottom = ( nsint )By;
	FillRect( dw->backBuffer.hDC, &r, brush );

	/* Right rectangle. */
	r.left   = ( nsint )C2x;
	r.top    = ( nsint )Ay;
	r.right  = ( nsint )Bx;
	r.bottom = ( nsint )By;
	FillRect( dw->backBuffer.hDC, &r, brush );

	/* Top rectangle. */
	r.left   = ( nsint )C1x;
	r.top    = ( nsint )Ay;
	r.right  = ( nsint )C2x;
	r.bottom = ( nsint )C1y;
	FillRect( dw->backBuffer.hDC, &r, brush );

	/* Bottom rectangle. */
	r.left   = ( nsint )C1x;
	r.top    = ( nsint )C2y;
	r.right  = ( nsint )C2x;
	r.bottom = ( nsint )By;
	FillRect( dw->backBuffer.hDC, &r, brush );

	DeleteObject( brush );
	}


/* These routines use the saved ROI. i.e. the one used the last time
	the projections were run. */
void _display_window_draw_roi_borders_xy( DisplayWindow *dw )
	{
	NsVector2f      A, B, C, C1, C2;
	const NsImage  *volume;
	const NsCubei  *roi;


	volume = workspace_volume( dw->workspace );
	roi    = workspace_visual_roi( dw->workspace );

	A.x  = 0.0f;
	A.y  = 0.0f;
	B.x  = ( nsfloat )ns_image_width( volume );
	B.y  = ( nsfloat )ns_image_height( volume );
	C.x  = ( nsfloat )dw->corner.x;
	C.y  = ( nsfloat )dw->corner.y;
	C1.x = ( nsfloat )roi->C1.x;
	C1.y = ( nsfloat )roi->C1.y;
	C2.x = ( nsfloat )( roi->C2.x + 1 );
	C2.y = ( nsfloat )( roi->C2.y + 1 );

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );
	____TO_CLIENT_SPACE( dw, C1, C );
	____TO_CLIENT_SPACE( dw, C2, C );

	_dispay_window_do_draw_roi_borders(
		dw,
		( nsint )A.x,  ( nsint )A.y,
		( nsint )B.x,  ( nsint )B.y,
		( nsint )C1.x, ( nsint )C1.y,
		( nsint )C2.x, ( nsint )C2.y
		);
	}


void _display_window_draw_roi_borders_zy( DisplayWindow *dw )
	{
	NsVector2f      A, B, C, C1, C2;
	const NsImage  *volume;
	const NsCubei  *roi;


	volume = workspace_volume( dw->workspace );
	roi    = workspace_visual_roi( dw->workspace );

	A.x  = 0.0f;
	A.y  = 0.0f;
	B.x  = ( nsfloat )ns_image_length( volume );
	B.y  = ( nsfloat )ns_image_height( volume );
	C.x  = ( nsfloat )dw->corner.x;
	C.y  = ( nsfloat )dw->corner.y;
	C1.x = ( nsfloat )roi->C1.z;
	C1.y = ( nsfloat )roi->C1.y;
	C2.x = ( nsfloat )( roi->C2.z + 1 );
	C2.y = ( nsfloat )( roi->C2.y + 1 );

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );
	____TO_CLIENT_SPACE( dw, C1, C );
	____TO_CLIENT_SPACE( dw, C2, C );

	_dispay_window_do_draw_roi_borders(
		dw,
		( nsint )A.x,  ( nsint )A.y,
		( nsint )B.x,  ( nsint )B.y,
		( nsint )C1.x, ( nsint )C1.y,
		( nsint )C2.x, ( nsint )C2.y
		);
	}


void _display_window_draw_roi_borders_xz( DisplayWindow *dw )
	{
	NsVector2f      A, B, C, C1, C2;
	const NsImage  *volume;
	const NsCubei  *roi;


	volume = workspace_volume( dw->workspace );
	roi    = workspace_visual_roi( dw->workspace );

	A.x  = 0.0f;
	A.y  = 0.0f;
	B.x  = ( nsfloat )ns_image_width( volume );
	B.y  = ( nsfloat )ns_image_length( volume );
	C.x  = ( nsfloat )dw->corner.x;
	C.y  = ( nsfloat )dw->corner.y;
	C1.x = ( nsfloat )roi->C1.x;
	C1.y = ( nsfloat )roi->C1.z;
	C2.x = ( nsfloat )( roi->C2.x + 1 );
	C2.y = ( nsfloat )( roi->C2.z + 1 );

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );
	____TO_CLIENT_SPACE( dw, C1, C );
	____TO_CLIENT_SPACE( dw, C2, C );

	_dispay_window_do_draw_roi_borders(
		dw,
		( nsint )A.x,  ( nsint )A.y,
		( nsint )B.x,  ( nsint )B.y,
		( nsint )C1.x, ( nsint )C1.y,
		( nsint )C2.x, ( nsint )C2.y
		);
	}


void _display_window_draw_roi_borders_xy_slice_view( DisplayWindow *dw )
	{
	NsVector2f      A, B, C, C1, C2;
	const NsImage  *volume;
	const NsCubei  *roi;


	volume = workspace_volume( dw->workspace );
	roi    = workspace_visual_roi( dw->workspace );

	A.x  = 0.0f;
	A.y  = 0.0f;
	B.x  = ( nsfloat )ns_image_width( volume );
	B.y  = ( nsfloat )ns_image_height( volume );
	C.x  = ( nsfloat )dw->corner.x;
	C.y  = ( nsfloat )dw->corner.y;
	C1.x = ( nsfloat )roi->C1.x;
	C1.y = ( nsfloat )roi->C1.y;
	C2.x = ( nsfloat )( roi->C2.x + 1 );
	C2.y = ( nsfloat )( roi->C2.y + 1 );

	____TO_CLIENT_SPACE( dw, A, C );
	____TO_CLIENT_SPACE( dw, B, C );
	____TO_CLIENT_SPACE( dw, C1, C );
	____TO_CLIENT_SPACE( dw, C2, C );

	_dispay_window_do_draw_roi_borders(
		dw,
		( nsint )A.x,  ( nsint )A.y,
		( nsint )B.x,  ( nsint )B.y,
		( nsint )C1.x, ( nsint )C1.y,
		( nsint )C2.x, ( nsint )C2.y
		);
	}






typedef struct _InvertedTextRenderer
	{
	MemoryGraphics  mg;
	HDC             dc;
	RECT            rc;
	TEXTMETRIC      tm;
	nsint           x;
	nsint           y;
	}
	InvertedTextRenderer;

#define _ITR_TEXT_DIM  16

nsboolean itr_init( InvertedTextRenderer *itr, HDC dc )
	{
	if( ! CreateMemoryGraphics( &itr->mg, dc, _ITR_TEXT_DIM, _ITR_TEXT_DIM ) )
		return NS_FALSE;

	SetBkMode( itr->mg.hDC, TRANSPARENT );
	SetTextColor( itr->mg.hDC, RGB( 255, 255, 255 ) );

	itr->dc = dc;

	itr->rc.left   = 0;
	itr->rc.top    = 0;
	itr->rc.right  = _ITR_TEXT_DIM;
	itr->rc.bottom = _ITR_TEXT_DIM;

	return NS_TRUE;
	}

void itr_finalize( InvertedTextRenderer *itr )
	{
	DestroyMemoryGraphics( &itr->mg );
	}

void itr_font( InvertedTextRenderer *itr, nsint which )
	{
	SelectObject( itr->mg.hDC, GetStockObject( which ) );
	GetTextMetrics( itr->mg.hDC, &itr->tm );
	}


void itr_char( InvertedTextRenderer *itr, const nschar *c, nsint x, nsint y )
	{
	FillRect( itr->mg.hDC, &itr->rc, GetStockObject( BLACK_BRUSH ) );
	TextOut( itr->mg.hDC, 0, 0, c, 1 );

	BitBlt( itr->dc, x, y, _ITR_TEXT_DIM, _ITR_TEXT_DIM, itr->mg.hDC, 0, 0, SRCINVERT );
	}


void itr_string( InvertedTextRenderer *itr, const nschar *s, nsint x, nsint y )
	{
	nschar  c[8];
	SIZE    size;

	for( ; '\0' != *s; ++s )
		{
		itr_char( itr, s, x, y );

		c[0] = *s;
		c[1] = '\0';

		GetTextExtentPoint32( itr->mg.hDC, c, 1, &size );
		x += size.cx;
		}
	}


void itr_char_ex( InvertedTextRenderer *itr, const nschar *c, nsint x, nsint y, COLORREF color )
	{
	nsint     old_bk_mode;
	COLORREF  old_text_color;


	FillRect( itr->mg.hDC, &itr->rc, GetStockObject( BLACK_BRUSH ) );

	old_bk_mode    = SetBkMode( itr->mg.hDC, TRANSPARENT );
	old_text_color = SetTextColor( itr->mg.hDC, color );

	TextOut( itr->mg.hDC, 0, 0, c, 1 );

	SetBkMode( itr->mg.hDC, old_bk_mode );
	SetTextColor( itr->mg.hDC, old_text_color );

	BitBlt( itr->dc, x, y, _ITR_TEXT_DIM, _ITR_TEXT_DIM, itr->mg.hDC, 0, 0, SRCINVERT );
	}


void itr_string_ex( InvertedTextRenderer *itr, const nschar *s, nsint x, nsint y, COLORREF color )
	{
	nschar  c[8];
	SIZE    size;

	for( ; '\0' != *s; ++s )
		{
		itr_char_ex( itr, s, x, y, color );

		c[0] = *s;
		c[1] = '\0';

		GetTextExtentPoint32( itr->mg.hDC, c, 1, &size );
		x += size.cx;
		}
	}


void itr_start( InvertedTextRenderer *itr, nsint x, nsint y )
	{
	itr->x = x;
	itr->y = y;
	}


void itr_line( InvertedTextRenderer *itr, const nschar *s )
	{
	itr_string( itr, s, itr->x, itr->y );
	itr->y += itr->tm.tmHeight;
	}




void _display_window_draw_2d_axes( DisplayWindow *dw, nssize width, nssize height )
	{
	RECT rc;
	InvertedTextRenderer itr;
	const nschar *lr, *tb;
static HICON ____right_arrow_icon = NULL;
static HICON ____down_arrow_icon = NULL;
	//HBRUSH brush;
   //BLENDFUNCTION   blend;
	
#define _2DA_LEFT    16
#define _2DA_TOP     16
#define _2DA_WIDTH   3
#define _2DA_EXTENT  32

	rc.left   = _2DA_LEFT + _2DA_WIDTH;
	rc.top    = _2DA_TOP;
	rc.right  = rc.left + _2DA_EXTENT;
	rc.bottom = rc.top + _2DA_WIDTH;
   InvertRect( dw->backBuffer.hDC, &rc );

	rc.left   = _2DA_LEFT;
	rc.top    = _2DA_TOP + _2DA_WIDTH;
	rc.right  = rc.left + _2DA_WIDTH;
	rc.bottom = rc.top + _2DA_EXTENT;
   InvertRect( dw->backBuffer.hDC, &rc );

	rc.left   = _2DA_LEFT;
	rc.top    = _2DA_TOP;
	rc.right  = rc.left + _2DA_WIDTH;
	rc.bottom = rc.top + _2DA_WIDTH;
   InvertRect( dw->backBuffer.hDC, &rc );

	if( NULL == ____right_arrow_icon )
		____right_arrow_icon = LoadIcon( g_Instance, "ICON_RIGHT_ARROW" );

	DrawIcon( dw->backBuffer.hDC, _2DA_LEFT + _2DA_WIDTH + _2DA_EXTENT, _2DA_TOP - 2, ____right_arrow_icon );

	if( NULL == ____down_arrow_icon )
		____down_arrow_icon = LoadIcon( g_Instance, "ICON_DOWN_ARROW" );

	DrawIcon( dw->backBuffer.hDC, _2DA_LEFT - 2, _2DA_TOP + _2DA_WIDTH + _2DA_EXTENT, ____down_arrow_icon );


	/* NOTE: Assuming drawing buffer is initially all black! */

	lr = tb = NULL;

	switch( dw->display )
		{
		case NS_XY: lr = "x"; tb = "y"; break;
		case NS_ZY: lr = "z"; tb = "y"; break;
		case NS_XZ: lr = "x"; tb = "z"; break;

		default:
			ns_assert_not_reached();
		}

	if( itr_init( &itr, dw->backBuffer.hDC ) )
		{
		itr_font( &itr, SYSTEM_FONT );
		itr_char( &itr, lr, _2DA_LEFT + _2DA_WIDTH + _2DA_EXTENT, _2DA_TOP + 2 );
		itr_char( &itr, tb, _2DA_LEFT + 8, _2DA_TOP + _2DA_WIDTH + _2DA_EXTENT - 6 );

		itr_finalize( &itr );
		}

/*
	itr_font( &itr, DEFAULT_GUI_FONT );
	itr_start( &itr, 16, 64 );
	itr_line( &itr, "volume = 2.034 um\xB3" );
	itr_line( &itr, "s.a.= 4.124 um\xB2" );
	itr_line( &itr, "d.t.s.= 8.124 um" );
	itr_line( &itr, "# voxels = 47" );

{
	brush = CreateSolidBrush( RGB( 255, 255, 225 ) );
	FillRect( itr.mg.hDC, &itr.rc, brush );
	DeleteObject( brush );

   blend.BlendOp             = AC_SRC_OVER;
   blend.BlendFlags          = 0;
   blend.SourceConstantAlpha = ( BYTE )128;
   blend.AlphaFormat         = 0;

   AlphaBlend( dw->backBuffer.hDC, 
               16, 
               64, 
               100, 
               60,
               itr.mg.hDC, 
               0, 0, 
               _ITR_TEXT_DIM, 
               _ITR_TEXT_DIM, 
               blend
             );
}*/

	{
	/*
	HPEN hOldPen, hNewPen;
	int old_rop;
	hNewPen     = CreatePen( PS_SOLID, dw->pen_thickness, 0x000000FF );
	hOldPen     = SelectObject( dw->backBuffer.hDC, hNewPen );
	old_rop     = SetROP2( dw->backBuffer.hDC, R2_XORPEN );

	SelectObject( dw->backBuffer.hDC, hOldPen );
	DeleteObject( hNewPen );
	SetROP2( dw->backBuffer.hDC, old_rop );
	*/
	 /*
	rc.left = 16; rc.top = 300;
	rc.right = 16 + 45; rc.bottom = 316;

   AlphaBlend( dw->backBuffer.hDC, 
               16, 
               300,
               16 + 45, 
               16,
               itr.mg.hDC, 
               0, 0,
               _ITR_TEXT_DIM, 
               _ITR_TEXT_DIM, 
               blend
             );

	rc.right = 16 + 100;
	FrameRect( dw->backBuffer.hDC, &rc, GetStockObject( WHITE_BRUSH ) );
	itr_start( &itr, 16, 320 );
	itr_line( &itr, "memory usage = 45%" );*/
	}
	}




void _display_window_draw_slice_viewer_widget( DisplayWindow *dw, RECT *client )
	{
	static HICON ____large_right_arrow_icon = NULL;
	const NsCubei *roi;
	RECT rc;
	//InvertedTextRenderer itr;
	nssize y;
	NsLerpf lerp;
	//nschar text[32];


	_SVW_BAR_LEFT = client->right - 19;
	_SVW_BAR_HEIGHT = ( client->bottom - client->top ) - 32;

	rc.left   = ( LONG )( _SVW_BAR_LEFT );
	rc.top    = ( LONG )( _SVW_BAR_TOP + 1 );
	rc.right  = ( LONG )( rc.left + _SVW_BAR_WIDTH );
	rc.bottom = ( LONG )( rc.top + _SVW_BAR_HEIGHT - 1 );
   InvertRect( dw->backBuffer.hDC, &rc );

	rc.left   = ( LONG )( _SVW_BAR_LEFT - 5 );
	rc.right  = ( LONG )( rc.left + _SVW_BAR_WIDTH + 10 );
	rc.top    = ( LONG )( _SVW_BAR_TOP );
	rc.bottom = ( LONG )( rc.top + 1 );
   InvertRect( dw->backBuffer.hDC, &rc );

	rc.top    = ( LONG )( rc.top + _SVW_BAR_HEIGHT );
	rc.bottom = ( LONG )( rc.top + 1 );
   InvertRect( dw->backBuffer.hDC, &rc );

	roi = workspace_update_roi( dw->workspace );

	ns_lerpf_init(
		&lerp,
		( nsfloat )roi->C1.z,
		0.0f,
		( nsfloat )roi->C2.z,
		( nsfloat )_SVW_BAR_HEIGHT
		);

	____svw_arrow_y = y = ( nssize )( ns_lerpf( &lerp, ( nsfloat )____xy_slice_index ) + 0.5f );

	if( NULL == ____large_right_arrow_icon )
		____large_right_arrow_icon = LoadIcon( g_Instance, "ICON_LARGE_RIGHT_ARROW" );

	DrawIcon(
		dw->backBuffer.hDC,
		( int )( _SVW_ARROW_LEFT ),
		( int )( _SVW_ARROW_TOP( y ) ),
		____large_right_arrow_icon
		);

/*
	if( itr_init( &itr, dw->backBuffer.hDC ) )
		{
		itr_font( &itr, SYSTEM_FONT );

		ns_snprint( text, sizeof( text ), NS_FMT_ULONG, ____xy_slice_index + 1 );
		itr_string( &itr, text, _SVW_BAR_LEFT + _SVW_BAR_WIDTH + 3, ( _SVW_BAR_TOP + y ) - 8 );

		#define _SVW_LIMITS_COLOR  RGB( 115, 140, 115 )

		y = ( nssize )ns_lerpf( &lerp, ( nsfloat )roi->C1.z ); 
		ns_snprint( text, sizeof( text ), NS_FMT_ULONG, roi->C1.z + 1 );
		itr_string_ex( &itr, text, _SVW_BAR_LEFT + _SVW_BAR_WIDTH + 3, ( _SVW_BAR_TOP + y ) - 8, _SVW_LIMITS_COLOR );

		y = ( nssize )ns_lerpf( &lerp, ( nsfloat )roi->C2.z );
		ns_snprint( text, sizeof( text ), NS_FMT_ULONG, roi->C2.z + 1 );
		itr_string_ex( &itr, text, _SVW_BAR_LEFT + _SVW_BAR_WIDTH + 3, ( _SVW_BAR_TOP + y ) - 8, _SVW_LIMITS_COLOR );

		itr_finalize( &itr );
		}
*/
	}





#include <std/nsmisc.h>
#include <psapi.h>
nssize ____total_available_memory = 0;
nschar ____total_mem_string[128] = "Total: ";
void _display_window_draw_memory_status_widget( DisplayWindow *dw )
	{
	RECT rc;
	InvertedTextRenderer itr;
	PROCESS_MEMORY_COUNTERS pmc;
	nssize currently_used_memory;
	nsdouble percent;
	nsint width;
	nschar perc_string[32];
	nschar curr_mem_string[128] = "Current: ";


#define _MEMSTAT_LEFT     16
#define _MEMSTAT_TOP      80
#define _MEMSTAT_WIDTH    100
#define _MEMSTAT_HEIGHT   16

	/* Top line */
	rc.left   = _MEMSTAT_LEFT;
	rc.top    = _MEMSTAT_TOP;
	rc.right  = rc.left + _MEMSTAT_WIDTH + 2;
	rc.bottom = rc.top + 1;
   InvertRect( dw->backBuffer.hDC, &rc );

	/* Bottom line */
	rc.left   = _MEMSTAT_LEFT;
	rc.top    = _MEMSTAT_TOP + _MEMSTAT_HEIGHT;
	rc.right  = rc.left + _MEMSTAT_WIDTH + 2;
	rc.bottom = rc.top + 1;
   InvertRect( dw->backBuffer.hDC, &rc );

	/* Left line */
	rc.left   = _MEMSTAT_LEFT;
	rc.top    = _MEMSTAT_TOP + 1;
	rc.right  = rc.left + 1;
	rc.bottom = rc.top + _MEMSTAT_HEIGHT - 1;
   InvertRect( dw->backBuffer.hDC, &rc );

	/* Right line */

	rc.left   = _MEMSTAT_LEFT + _MEMSTAT_WIDTH + 1;
	rc.top    = _MEMSTAT_TOP + 1;
	rc.right  = rc.left + 1;
	rc.bottom = rc.top + _MEMSTAT_HEIGHT - 1;
   InvertRect( dw->backBuffer.hDC, &rc );

	pmc.cb = sizeof( PROCESS_MEMORY_COUNTERS );
	if( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( PROCESS_MEMORY_COUNTERS ) ) )
		{
		if( 0 == ____total_available_memory )
			{
			MEMORYSTATUSEX ms;

			ms.ullTotalPhys = 0;
			ms.dwLength = sizeof( MEMORYSTATUSEX );
			GlobalMemoryStatusEx( &ms );

			____total_available_memory = ( nssize )ms.ullTotalPhys;

			ns_println( "\nTotal available memory = " NS_FMT_ULONG, ____total_available_memory );

			ns_num_bytes_to_string_ex(
				____total_available_memory,
				____total_mem_string + ns_ascii_strlen( "Total: " ),
				NS_ARRAY_LENGTH( ____total_mem_string ) - ns_ascii_strlen( "Total: " ),
				0
				);
			}

		currently_used_memory = pmc.WorkingSetSize
			/*pmc.QuotaPagedPoolUsage + pmc.QuotaNonPagedPoolUsage*/;

		percent = ( ( nsdouble )currently_used_memory / ( nsdouble )____total_available_memory ) * 100.0;
		//ns_println( "\nCurrently used memory = " NS_FMT_DOUBLE, percent );

		if( 100.0 < percent )
			percent = 100.0;

		ns_snprint( perc_string, sizeof( perc_string ), "%.1f %%", percent );

		width = ( nsint )percent;

		rc.left   = _MEMSTAT_LEFT + 1;
		rc.top    = _MEMSTAT_TOP + 2;
		rc.right  = rc.left + width;
		rc.bottom = rc.top + _MEMSTAT_HEIGHT - 3;
		InvertRect( dw->backBuffer.hDC, &rc );

		ns_num_bytes_to_string_ex(
			currently_used_memory,
			curr_mem_string + ns_ascii_strlen( "Current: " ),
			NS_ARRAY_LENGTH( curr_mem_string ) - ns_ascii_strlen( "Current: " ),
			0
			);

		if( itr_init( &itr, dw->backBuffer.hDC ) )
			{
			itr_font( &itr, SYSTEM_FONT );

			itr_string( &itr, perc_string, _MEMSTAT_LEFT + _MEMSTAT_WIDTH + 8, _MEMSTAT_TOP + 1 ); 

			itr_start( &itr, _MEMSTAT_LEFT, _MEMSTAT_TOP + _MEMSTAT_HEIGHT + 8 );
			itr_line( &itr, curr_mem_string );
			itr_line( &itr, ____total_mem_string );

			itr_finalize( &itr );
			}
		}
	}


extern nsboolean ____show_2d_axes;
extern nsboolean ____show_memory_status_widget;
extern nsboolean ____view_spine_ids;
extern nsboolean ____view_edge_order_numbers;
extern nsboolean ____view_edge_section_numbers;


void _display_window_render_edge_centers( const NsVector3f *V, nsulong value, DisplayWindow *dw )
	{
	NsVector3f I;
	NsVector2f A;
	NsVector2f corner;


	ns_snprint( ____paint_text, sizeof( ____paint_text ), NS_FMT_ULONG, value );

	ns_to_image_space_ex( V, &I, workspace_get_voxel_info( dw->workspace ) );

	switch( dw->display )
		{
		case NS_XY: A.x = I.x; A.y = I.y; break;
		case NS_ZY: A.x = I.z; A.y = I.y; break;
		case NS_XZ: A.x = I.x; A.y = I.z; break;

		default:
			ns_assert_not_reached();
		}

   corner.x = ( nsfloat )dw->corner.x;
   corner.y = ( nsfloat )dw->corner.y;

	____TO_CLIENT_SPACE( dw, A, corner );

	TextOut( dw->backBuffer.hDC, ( nsint )A.x, ( nsint )A.y, ____paint_text, ( int )ns_ascii_strlen( ____paint_text ) );
	}


void _DisplayWindowOnPaint( DisplayWindow* dw, HDC hDC ) 
   {
	RECT            client;
	nsuint          bkgndColor;
	HBRUSH          bk_brush;
	NsVector4ub     C;
	nsint           old_rop;
	HPEN            hOldPen, hNewPen;
	nsint           old_bk_mode;
	nsfloat         zoom;
	const NsImage  *volume;
	const NsModel  *model;
	const NsCubei  *roi;
	RECT            rc;
	NsDragRect      B;

	NS_PRIVATE HICON ____big_right_arrow_icon = NULL;
	NS_PRIVATE HICON ____big_down_arrow_icon  = NULL;



   GetClientRect( dw->hWnd, &client );

   if( ! dw->haveBackBuffer )
      {
      if( ! CreateMemoryGraphics( &dw->backBuffer, 
                                   hDC,
                                    client.right - client.left,
                                    client.bottom - client.top
                                   ) )
         return;

      dw->haveBackBuffer = 1;
      }


	if( ! ____redraw_2d_need_render )
		{
		//ns_println( "dont need re-render" );
		goto _PAINT_DO_BLITTING;
		}


   if( 0 != dw->workspace )
      {
      C = workspace_get_color( dw->workspace, WORKSPACE_COLOR_BACKGROUND );
      bkgndColor = ____TO_COLORREF( C );
      }
   else
      bkgndColor = GetSysColor( COLOR_3DSHADOW );

   bk_brush = CreateSolidBrush( bkgndColor );
   FillRect( dw->backBuffer.hDC, &client, bk_brush );

  if( 0 != dw->workspace )
      {
		dw->zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
		dw->corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

      zoom = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
      ns_snprint( ____paint_text, NS_ARRAY_LENGTH( ____paint_text ), "%.2f%%", zoom * 100.0f );
		SetStatusBarText( ____paint_text, STATUS_BAR_ZOOM );

      _DrawDisplayWindow( dw, &client );

      //hBrush = CreateSolidBrush( 0x00FFFF00 );
      //FillRect( dw->backBuffer.hDC, &rc, hBrush );
      //InvertRect( dw->backBuffer.hDC, &rc );
      //DeleteObject( hBrush );

		switch( dw->display )
			{
			case NS_XY:
				if( ____xy_slice_enabled )
					_display_window_draw_roi_borders_xy_slice_view( dw );
				else
					_display_window_draw_roi_borders_xy( dw );
				break;

			case NS_ZY:
				_display_window_draw_roi_borders_zy( dw );
				break;

			case NS_XZ:
				_display_window_draw_roi_borders_xz( dw );
				break;
			}

      _DrawDisplayWindowNeuronTree( dw );

		if( ____view_sholl_analysis )
			{
			const NsShollAnalysis *sa;
			nsshollannulus sac, sae;
			nssize num_annuli;
			NsVector3f T;
			NsVector3f I;
			NsVector2f R, O;
			NsVector2f corner;
			const NsVoxelInfo *voxel_info;
			nsint ox,oy,rx,ry;
			HPEN old_pen;

			sa         = workspace_sholl_analysis( dw->workspace );
			num_annuli = ns_sholl_analysis_size( sa );
			voxel_info = workspace_get_voxel_info( dw->workspace );

			ns_to_image_space_ex( ns_sholl_analysis_origin( sa ), &I, voxel_info );

			I.x += .5f; I.y += .5f; I.z += .5f;

			switch( dw->display ){
				case NS_XY: O.x = I.x; O.y = I.y; break;
				case NS_ZY: O.x = I.z; O.y = I.y; break;
				case NS_XZ: O.x = I.x; O.y = I.z; break;
				default: ns_assert_not_reached(); }

			corner.x = ( nsfloat )dw->corner.x;
			corner.y = ( nsfloat )dw->corner.y;

			____TO_CLIENT_SPACE( dw, O, corner );

			old_pen = SelectObject( dw->backBuffer.hDC, GetStockObject( WHITE_PEN ) );

			sac = ns_sholl_analysis_begin( sa );
			sae = ns_sholl_analysis_end( sa );

			for( ; ns_sholl_annulus_not_equal( sac, sae ); sac = ns_sholl_annulus_next( sac ) )
				{
				T.x = T.y = T.z = ns_sholl_annulus_start_radius( sac );
				ns_to_image_space_ex( &T, &I, voxel_info );

				switch( dw->display ){
					case NS_XY: R.x = I.x; R.y = I.y; break;
					case NS_ZY: R.x = I.z; R.y = I.y; break;
					case NS_XZ: R.x = I.x; R.y = I.z; break;
					default: ns_assert_not_reached(); }

				ns_vector2f_cmpd_scale( &R, zoom );

				ox = ( nsint )O.x; oy = ( nsint )O.y; rx = ( nsint )R.x; ry = ( nsint )R.y;
				Arc( dw->backBuffer.hDC, ox - rx, oy - ry, ox + rx + 1, oy + ry + 1, ox - rx, oy, ox - rx, oy );

				if( ns_sholl_annulus_index( sac ) == num_annuli - 1 )
					{
					T.x = T.y = T.z = ns_sholl_annulus_end_radius( sac );
					ns_to_image_space_ex( &T, &I, voxel_info );

					switch( dw->display ){
						case NS_XY: R.x = I.x; R.y = I.y; break;
						case NS_ZY: R.x = I.z; R.y = I.y; break;
						case NS_XZ: R.x = I.x; R.y = I.z; break;
						default: ns_assert_not_reached(); }

					ns_vector2f_cmpd_scale( &R, zoom );

					ox = ( nsint )O.x; oy = ( nsint )O.y; rx = ( nsint )R.x; ry = ( nsint )R.y;
					Arc( dw->backBuffer.hDC, ox - rx, oy - ry, ox + rx + 1, oy + ry + 1, ox - rx, oy, ox - rx, oy );
					}
				}

			SelectObject( dw->backBuffer.hDC, old_pen );
			}

      if( ____xy_slice_enabled )
         {
         NsVector2f A, B, C;
         RECT r;
         //nsint height;

         volume = workspace_volume( dw->workspace );
			roi    = workspace_update_roi( dw->workspace );

         switch( dw->display )
            {
            case NS_ZY:
               A.x = ( nsfloat )____xy_slice_index;
               A.y = ( nsfloat )roi->C1.y;
               B.x = ( nsfloat )( ____xy_slice_index + 1 );
               B.y = ( nsfloat )( roi->C2.y + 1 );
               C.x = ( nsfloat )dw->corner.x;
               C.y = ( nsfloat )dw->corner.y;
               ____TO_CLIENT_SPACE( dw, A, C );
               ____TO_CLIENT_SPACE( dw, B, C );
               r.left   = ( nsint )A.x;
               r.right  = ( nsint )B.x;
               r.top    = ( nsint )A.y;
               r.bottom = ( nsint )B.y;
               if( 0 == r.right - r.left )
                  r.right = r.left + 1;
               InvertRect( dw->backBuffer.hDC, &r );
               break;

            case NS_XZ:
               A.x = ( nsfloat )roi->C1.x;
               A.y = ( nsfloat )____xy_slice_index;
               B.x = ( nsfloat )( roi->C2.x + 1 );
               B.y = ( nsfloat )( ____xy_slice_index + 1 );
               C.x = ( nsfloat )dw->corner.x;
               C.y = ( nsfloat )dw->corner.y;
               ____TO_CLIENT_SPACE( dw, A, C );
               ____TO_CLIENT_SPACE( dw, B, C );
               r.left   = ( nsint )A.x;
               r.right  = ( nsint )B.x;
               r.top    = ( nsint )B.y /*+ 1*/; /* TEMP Why +1? */
               r.bottom = ( nsint )A.y /*+ 1*/; /* TEMP Why +1? */
               if( 0 == r.bottom - r.top )
                  r.bottom = r.top + 1;
               InvertRect( dw->backBuffer.hDC, &r );
               break;
            }

         ns_snprint( ____paint_text, NS_ARRAY_LENGTH( ____paint_text ), "XY Slice = " NS_FMT_ULONG, ____xy_slice_index + 1 );
         SetStatusBarText( ____paint_text, STATUS_BAR_SLICE );
         /*
         r.left = r.top = 0;
         height = DrawText( dw->backBuffer.hDC, ____paint_text, ns_ascii_strlen( ____paint_text ), &r, DT_CALCRECT );
         r.top = client.bottom - height;
         r.bottom = r.top + height;
         r.right += 6;
         DrawText( dw->backBuffer.hDC, ____paint_text, ns_ascii_strlen( ____paint_text ), &r, DT_CENTER );
         FrameRect( dw->backBuffer.hDC, &r, GetStockObject( BLACK_BRUSH ) );
         */
        }

		model = GetWorkspaceNeuronTree( dw->workspace );

      if( ( eDISPLAY_WINDOW_MOUSE_MODE_SEED == s_CurrentMouseMode || ____view_seed_always_on_top )
				&& ! ns_model_grafter_is_running( model )
				&& ! ns_model_spines_is_running( model )
				&& ! ____flash_frames_active )
         {
			Vector2i seed;
			nsint offset;


			_GetDisplayWindowClientSeed( dw, &seed );
			_display_window_get_client_roi_ex( dw, &B );

			offset = (( nsint )zoom) / 2;
			seed.x += offset;
			seed.y += offset;

			if( zoom < 8.0f )
				zoom = 8.0f;

			rc.left   = ( LONG )seed.x;
			rc.top    = ( LONG )seed.y;
			rc.right  = ( LONG )( seed.x + ( nslong )( nsint )zoom );
			rc.bottom = ( LONG )( seed.y + ( nslong )( nsint )zoom );

			if( NULL == ____big_right_arrow_icon )
				____big_right_arrow_icon = LoadIcon( g_Instance, "ICON_BIG_RIGHT_ARROW" );

			DrawIcon( dw->backBuffer.hDC, ( int )0, ( int )( seed.y - 7 ), ____big_right_arrow_icon );

			if( NULL == ____big_down_arrow_icon )
				____big_down_arrow_icon = LoadIcon( g_Instance, "ICON_BIG_DOWN_ARROW" );

			DrawIcon( dw->backBuffer.hDC, ( int )( seed.x - 7 ), ( int )0,____big_down_arrow_icon );

         //r.left = 0; r.right = 5; r.top = rc.top - 2; r.bottom = r.top + 5;
         //InvertRect( dw->backBuffer.hDC, &r );

         //r.left = rc.left - 2; r.right = r.left + 5; r.top = 0; r.bottom = 5;
         //InvertRect( dw->backBuffer.hDC, &r );

         hNewPen = CreatePen( PS_DOT, 1, 0x00FFFFFF );
         hOldPen = SelectObject( dw->backBuffer.hDC, hNewPen );

         old_rop     = SetROP2( dw->backBuffer.hDC, R2_XORPEN );
         old_bk_mode = SetBkMode( dw->backBuffer.hDC, TRANSPARENT );

         MoveToEx( dw->backBuffer.hDC, B.start_x/*0*/, rc.top, NULL );
         LineTo( dw->backBuffer.hDC, B.end_x/*client.right - 1*/, rc.top );

         MoveToEx( dw->backBuffer.hDC, rc.left, B.start_y/*0*/, NULL );
         LineTo( dw->backBuffer.hDC, rc.left, B.end_y/*client.bottom - 1*/ );

         //MoveToEx( dw->backBuffer.hDC, 0, 0, NULL );
         //LineTo( dw->backBuffer.hDC, rc.left, rc.top );

         //MoveToEx( dw->backBuffer.hDC, client.right - 1, 0, NULL );
         //LineTo( dw->backBuffer.hDC, rc.left, rc.top /*rc.right - 1, rc.top*/ );

         //MoveToEx( dw->backBuffer.hDC, 0, client.bottom - 1, NULL );
         //LineTo( dw->backBuffer.hDC, rc.left, rc.top /*rc.left, rc.bottom - 1*/ );

         //MoveToEx( dw->backBuffer.hDC, client.right - 1, client.bottom - 1, NULL );
         //LineTo( dw->backBuffer.hDC, rc.left, rc.top /*rc.right - 1, rc.bottom - 1*/ );

         SelectObject( dw->backBuffer.hDC, hOldPen );
         DeleteObject( hNewPen );

         SetBkMode( dw->backBuffer.hDC, old_bk_mode );
         SetROP2( dw->backBuffer.hDC, old_rop );
         }

      if( ( eDISPLAY_WINDOW_MOUSE_MODE_ROI == s_CurrentMouseMode || ____view_roi_always_on_top )
				&& ! ns_model_grafter_is_running( model )
				&& ! ns_model_spines_is_running( model )
				&& ! ____flash_frames_active )
         _display_window_draw_roi( dw );

      //C.x = 0; C.y = 0; C.z = 255;

      if( ( eDISPLAY_WINDOW_MOUSE_MODE_RAYS == s_CurrentMouseMode ||
				eDISPLAY_WINDOW_MOUSE_MODE_NEURITE == s_CurrentMouseMode ||
			   eDISPLAY_WINDOW_MOUSE_MODE_SPINE == s_CurrentMouseMode )
          && dw->isDragging
			 && ( ! ____drag_rect_active )
			 && ( _DRAG_MODE_NORMAL == dw->drag_mode ) )
			{
         _invert_line(
            dw,
            workspace_get_voxel_info( dw->workspace ),
            &dw->P1f,
            &dw->P2f
            );
			}

      if( dw->is_rotating )
         {
         HPEN old_pen, new_pen;
         nsint cross_hair_radius;

         new_pen = CreatePen( PS_SOLID, 1, RGB( 255, 255, 0 ) );
         old_pen = SelectObject( dw->backBuffer.hDC, new_pen );

         Arc(
            dw->backBuffer.hDC,
            dw->xfrm_center_x - dw->xfrm_radius,
            dw->xfrm_center_y - dw->xfrm_radius,
            dw->xfrm_center_x + dw->xfrm_radius + 1,
            dw->xfrm_center_y + dw->xfrm_radius + 1,
            dw->xfrm_center_x - dw->xfrm_radius,
            dw->xfrm_center_y,
            dw->xfrm_center_x - dw->xfrm_radius,
            dw->xfrm_center_y
            );

         cross_hair_radius = dw->xfrm_radius / 8;

         MoveToEx( dw->backBuffer.hDC, dw->xfrm_center_x - cross_hair_radius, dw->xfrm_center_y, NULL );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x + cross_hair_radius, dw->xfrm_center_y );

         MoveToEx( dw->backBuffer.hDC, dw->xfrm_center_x, dw->xfrm_center_y - cross_hair_radius, NULL );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x, dw->xfrm_center_y + cross_hair_radius );

         SelectObject( dw->backBuffer.hDC, old_pen );
         DeleteObject( new_pen );
         }
      else if( dw->is_translating )
         {
         HPEN old_pen, new_pen;
         nsint cross_hair_radius;

         new_pen = CreatePen( PS_SOLID, 1, RGB( 255, 255, 0 ) );
         old_pen = SelectObject( dw->backBuffer.hDC, new_pen );

         MoveToEx( dw->backBuffer.hDC, dw->xfrm_center_x - dw->xfrm_radius, dw->xfrm_center_y - dw->xfrm_radius, NULL );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x + dw->xfrm_radius, dw->xfrm_center_y - dw->xfrm_radius );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x + dw->xfrm_radius, dw->xfrm_center_y + dw->xfrm_radius );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x - dw->xfrm_radius, dw->xfrm_center_y + dw->xfrm_radius );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x - dw->xfrm_radius, dw->xfrm_center_y - dw->xfrm_radius );

         cross_hair_radius = dw->xfrm_radius / 8;

         MoveToEx( dw->backBuffer.hDC, dw->xfrm_center_x - cross_hair_radius, dw->xfrm_center_y, NULL );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x + cross_hair_radius, dw->xfrm_center_y );

         MoveToEx( dw->backBuffer.hDC, dw->xfrm_center_x, dw->xfrm_center_y - cross_hair_radius, NULL );
         LineTo( dw->backBuffer.hDC, dw->xfrm_center_x, dw->xfrm_center_y + cross_hair_radius );

         SelectObject( dw->backBuffer.hDC, old_pen );
         DeleteObject( new_pen );
         }

		if( ____flash_frames_active )
			FrameRect( dw->backBuffer.hDC, &client, GetStockObject( WHITE_BRUSH ) );
      }
   else
      SetStatusBarText( "", STATUS_BAR_ZOOM );

	if( 0 != dw->workspace && ____view_spine_ids && ! ____flash_frames_active )
		{
		nsspine curr_spine, end_spines;
		NsVector3f SV;
		NsVector3f SI;
		NsVector2f A;
		NsVector2f corner;
		NsVector2i SP;
		SIZE       sz;
		int        length;


		model = GetWorkspaceNeuronTree( dw->workspace );

		curr_spine = ns_model_begin_spines( model );
		end_spines = ns_model_end_spines( model );

		for( ; ns_spine_not_equal( curr_spine, end_spines ); curr_spine = ns_spine_next( curr_spine ) )
			{
			ns_snprint( ____paint_text, sizeof( ____paint_text ), NS_FMT_ULONG, ns_spine_get_id( curr_spine ) );

			ns_spine_get_position( curr_spine, &SV );
			ns_to_image_space_ex( &SV, &SI, workspace_get_voxel_info( dw->workspace ) );

			SI.x += .5f; SI.y += .5f; SI.z += .5f;

			switch( dw->display )
				{
				case NS_XY: A.x = SI.x; A.y = SI.y; break;
				case NS_ZY: A.x = SI.z; A.y = SI.y; break;
				case NS_XZ: A.x = SI.x; A.y = SI.z; break;

				default:
					ns_assert_not_reached();
				}

         corner.x = ( nsfloat )dw->corner.x;
         corner.y = ( nsfloat )dw->corner.y;

			____TO_CLIENT_SPACE( dw, A, corner );

			SP.x = ( nsint )A.x;
			SP.y = ( nsint )A.y;

			/* SP is the center of the spine but is the upper left corner of the
				string "box". Now center the string around the center of the spine. */

			length = ( int )ns_ascii_strlen( ____paint_text );

			GetTextExtentPoint32( dw->backBuffer.hDC, ____paint_text, length, &sz );

			SP.x -= ( sz.cx / 2 );
			SP.y -= ( sz.cy / 2 );

			TextOut( dw->backBuffer.hDC, SP.x, SP.y, ____paint_text, length );
			}
		}



/*
if( 0 != dw->workspace )
{
NsVector3f V1, V2, V3;
NsVector3f SI;
NsVector2f A;
NsVector2f corner;
NsVector2i SP;
		SIZE       sz;
		int        length;
nsmodelvertex curr_vertex, end_vertices;
nsmodeledge curr_edge, end_edges;
model = GetWorkspaceNeuronTree( dw->workspace );
curr_vertex  = ns_model_begin_vertices( model );
end_vertices = ns_model_end_vertices( model );

for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
       curr_vertex = ns_model_vertex_next( curr_vertex ) )
   {
   curr_edge = ns_model_vertex_begin_edges( curr_vertex );
   end_edges = ns_model_vertex_end_edges( curr_vertex );

   for( ; ns_model_edge_not_equal( curr_edge, end_edges );
          curr_edge = ns_model_edge_next( curr_edge ) )
		{
		ns_model_vertex_get_position( ns_model_edge_src_vertex( curr_edge ), &V1 );
		ns_model_vertex_get_position( ns_model_edge_dest_vertex( curr_edge ), &V2 );
		ns_vector3f_add( &V3, &V1, &V2 );
		ns_vector3f_cmpd_scale( &V3, 0.5f );

		ns_snprint( ____paint_text, sizeof( ____paint_text ), NS_FMT_DOUBLE, ns_model_edge_get_prune_length( curr_edge ) );
		ns_to_image_space_ex( &V3, &SI, workspace_get_voxel_info( dw->workspace ) );

		SI.x += .5f; SI.y += .5f; SI.z += .5f;

		switch( dw->display )
			{
			case NS_XY: A.x = SI.x; A.y = SI.y; break;
			case NS_ZY: A.x = SI.z; A.y = SI.y; break;
			case NS_XZ: A.x = SI.x; A.y = SI.z; break;

			default:
				ns_assert_not_reached();
			}

      corner.x = ( nsfloat )dw->corner.x;
      corner.y = ( nsfloat )dw->corner.y;

		____TO_CLIENT_SPACE( dw, A, corner );

		SP.x = ( nsint )A.x;
		SP.y = ( nsint )A.y;

		length = ( int )ns_ascii_strlen( ____paint_text );

		GetTextExtentPoint32( dw->backBuffer.hDC, ____paint_text, length, &sz );

		SP.x -= ( sz.cx / 2 );
		SP.y -= ( sz.cy / 2 );

		TextOut( dw->backBuffer.hDC, SP.x, SP.y, ____paint_text, length );
		}
   }
}*/



	/*if( 0 != dw->workspace &&
		 ! ns_model_grafter_is_running( model ) &&
		 ! ns_model_spines_is_running( model ) &&
		 ! ____flash_frames_active )
		{
		nslistiter  curr_clump, end_clumps;
		NsVector3f  C;
		NsVector2f  A, corner;


		model = GetWorkspaceNeuronTree( dw->workspace );

		curr_clump = ns_model_begin_clumps( model );
		end_clumps = ns_model_end_clumps( model );

		for( ; ns_list_iter_not_equal( curr_clump, end_clumps ); curr_clump = ns_list_iter_next( curr_clump ) )
			if( ns_spine_clump_has_origin( curr_clump ) )
				{
				ns_snprint( ____paint_text, sizeof( ____paint_text ), NS_FMT_ULONG, ns_spine_clump_id( curr_clump ) );
				ns_vector3i_to_3f( &(ns_spine_clump_origin( curr_clump )->position), &C );

				switch( dw->display )
					{
					case NS_XY: A.x = C.x; A.y = C.y; break;
					case NS_ZY: A.x = C.z; A.y = C.y; break;
					case NS_XZ: A.x = C.x; A.y = C.z; break;

					default:
						ns_assert_not_reached();
					}

				corner.x = ( nsfloat )dw->corner.x;
				corner.y = ( nsfloat )dw->corner.y;

				____TO_CLIENT_SPACE( dw, A, corner );

				TextOut( dw->backBuffer.hDC, ( nsint )A.x, ( nsint )A.y, ____paint_text, ns_ascii_strlen( ____paint_text ) );
				}
		}*/

/*
	if( 0 != dw->workspace &&
		 ! ns_model_grafter_is_running( model ) &&
		 ! ns_model_spines_is_running( model ) &&
		 ! ____flash_frames_active )
		{
		NsVoxel    *voxel;
		const NsVoxelTable *voxel_table;
		NsVector3f  C;
		NsVector2f  A, corner;


		voxel_table = ns_model_spines_voxel_table( GetWorkspaceNeuronTree( dw->workspace ) );

		NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )
			if( ns_model_spine_voxel_is_maxima( voxel ) )
				{
				ns_snprint( ____paint_text, sizeof( ____paint_text ), NS_FMT_DOUBLE, ns_model_spine_voxel_dts( voxel ) );
				ns_vector3i_to_3f( &voxel->position, &C );

				switch( dw->display )
					{
					case NS_XY: A.x = C.x; A.y = C.y; break;
					case NS_ZY: A.x = C.z; A.y = C.y; break;
					case NS_XZ: A.x = C.x; A.y = C.z; break;
					default:
						ns_assert_not_reached();
					}

				corner.x = ( nsfloat )dw->corner.x;
				corner.y = ( nsfloat )dw->corner.y;

				____TO_CLIENT_SPACE( dw, A, corner );
				TextOut( dw->backBuffer.hDC, ( nsint )A.x, ( nsint )A.y, ____paint_text, ns_ascii_strlen( ____paint_text ) );
				}
		}
*/




	if( 0 != dw->workspace && ____view_edge_order_numbers && ! ____return_raw_model && ! ____flash_frames_active )
		{
		model = GetWorkspaceNeuronTree( dw->workspace );
	
		if( ! ns_model_has_edge_order_centers( model ) )
			/*error*/ns_model_calc_edge_order_centers( ( NsModel* )model ); /* Ignoring error. */

		ns_model_render_edge_order_centers( model, _display_window_render_edge_centers, dw );
		}

	if( 0 != dw->workspace && ____view_edge_section_numbers && ! ____return_raw_model && ! ____flash_frames_active )
		{
		model = GetWorkspaceNeuronTree( dw->workspace );
	
		if( ! ns_model_has_edge_section_centers( model ) )
			/*error*/ns_model_calc_edge_section_centers( ( NsModel* )model ); /* Ignoring error. */

		ns_model_render_edge_section_centers( model, _display_window_render_edge_centers, dw );
		}

	if( ____display_window_do_screen_capture )
		{
		_display_window_screen_capture( dw, client.right - client.left, client.bottom - client.top );
		____display_window_do_screen_capture = NS_FALSE;
		}

	if(  0 != dw->workspace && ! ____flash_frames_active )
		{
		if( ____show_2d_axes )
			_display_window_draw_2d_axes( dw, client.right - client.left, client.bottom - client.top );

		if( ____show_memory_status_widget )
			_display_window_draw_memory_status_widget( dw );

		if( ____xy_slice_enabled )
			_display_window_draw_slice_viewer_widget( dw, &client );
		}

	_PAINT_DO_BLITTING:

	BitBlt(
		hDC,
		0, 0, 
		client.right - client.left,
		client.bottom - client.top,
		dw->backBuffer.hDC,
		0, 0,
		SRCCOPY 
		);

   DeleteObject( bk_brush );

	____redraw_2d_need_render = NS_FALSE;
   }



/*
void _MaintainDisplayWindowImageAspect( DisplayWindow* dw )
   {
   HWorkspace     workspace;
   nsuint     index;
   nsint  display;

   workspace = dw->workspace;
   index      = dw->index;
   display    = dw->display;

   //if( eDISPLAY_MONTAGE == display )
     // return;
   }
*/



void _DisplayWindowOnSize( DisplayWindow* dw, WPARAM type )
   {
	if( SIZE_MAXIMIZED == type || SIZE_RESTORED == type )
		____redraw_2d_need_render = NS_TRUE;

   _DestroyDisplayWindowBackBuffer( dw );

   if( 0 != dw->workspace )
      _CenterOrLimitDisplayWindowImage( dw );
   }


void _DisplayWindowOnLButtonDown( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
   if( 0 != dw->workspace )
      {
 //     if( ! dw->isActive )
   //      {
     //    dw->isActive = 1;
       //  DrawDisplayWindow( dw->hWnd );
//         dw->old_focus_hwnd = SetFocus( dw->hWnd );
  //       }
      //else
      //   {
         if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onLButtonDown )
            ( s_MouseModes[s_CurrentMouseMode ].onLButtonDown )( dw->hWnd, x, y, flags );
      //   }
      }
   }


void _display_window_on_rbutton_down( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
   if( 0 != dw->workspace )
      if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onRButtonDown )
         ( s_MouseModes[s_CurrentMouseMode ].onRButtonDown )( dw->hWnd, x, y, flags );
   }


void _display_window_on_rbutton_up( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
   if( 0 != dw->workspace )
      if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onRButtonUp )
         ( s_MouseModes[s_CurrentMouseMode ].onRButtonUp )( dw->hWnd, x, y, flags );
   }


void _DisplayWindowOnLButtonDblClk( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
	____lbutton_double_clicked = NS_TRUE;

   if( 0 != dw->workspace )
      if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onLButtonDblClk )
         ( s_MouseModes[s_CurrentMouseMode ].onLButtonDblClk )( dw->hWnd, x, y, flags );
   }


void _DisplayWindowOnLButtonUp( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
   if( 0 != dw->workspace )
     if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onLButtonUp )
         ( s_MouseModes[s_CurrentMouseMode ].onLButtonUp )( dw->hWnd, x, y, flags );

	____lbutton_double_clicked = NS_FALSE;
   }


void _display_window_mouse_move_hook( DisplayWindow *dw, nsint x, nsint y )
   {
   Vector2i  corner;
   Vector2i  client;
   Vector2i  global;
   nsfloat   zoom;
   nsfloat       xf, yf;
   const nschar *prefix;
   const NsVoxelInfo *voxel_info;
   NS_PRIVATE nschar _mouse_move_hook_msg[128];
 

	dw->mouse_x = x;
	dw->mouse_y = y;

   zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

   MakeVector2i( &client, x, y );

   _DisplayWindowClientToGlobalVector( &global,
                                       &client, 
                                       &corner,
                                       zoom,
                                       dw->display
                                     );

   voxel_info = workspace_get_voxel_info( dw->workspace );

   switch( dw->display )
      {
      case NS_XY:
         prefix = "(x,y)";
         xf = NS_TO_VOXEL_SPACE( global.x, ns_voxel_info_size_x( voxel_info ) );
         yf = NS_TO_VOXEL_SPACE( global.y, ns_voxel_info_size_y( voxel_info ) );
         break;

      case NS_ZY:
			prefix = "(z,y)";
         xf = NS_TO_VOXEL_SPACE( global.x, ns_voxel_info_size_z( voxel_info ) );
         yf = NS_TO_VOXEL_SPACE( global.y, ns_voxel_info_size_y( voxel_info ) );
         break;

      case NS_XZ:
			prefix = "(x,z)";
         xf = NS_TO_VOXEL_SPACE( global.x, ns_voxel_info_size_x( voxel_info ) );
         yf = NS_TO_VOXEL_SPACE( global.y, ns_voxel_info_size_z( voxel_info ) );
         break;
      }

   ns_snprint( _mouse_move_hook_msg, NS_ARRAY_LENGTH( _mouse_move_hook_msg ),
      "mouse{%s = %d,%d : %.4f,%.4f}", prefix, global.x, global.y, xf, yf );

   SetStatusBarText( _mouse_move_hook_msg, STATUS_BAR_MSG );
   }


void _DisplayWindowOnMouseMove( DisplayWindow* dw, nsint x, nsint y, nsuint flags )
   {
   if( 0 != dw->workspace )
      {
/*TEMP???*/_display_window_mouse_move_hook( dw, x, y );

     if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[s_CurrentMouseMode ].onMouseMove )
       ( s_MouseModes[s_CurrentMouseMode ].onMouseMove )( dw->hWnd, x, y, flags );
      }
   }


void _display_window_on_mouse_wheel( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   if( 1/*eDISPLAY_WINDOW_MOUSE_MODE_DRAG == s_CurrentMouseMode ||
       eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode */  )
      {
      Vector2i        clientCoord;
      Vector2i        oldGlobalCoord;
      Vector2i        newGlobalCoord;
      Vector2i        difference;
      Vector2i        corner;
      HWorkspace      workspace;
      nsuint      index;
      nsint   display;
      nsfloat      zoom;
      DisplayWindow  *dw;
      nsint       zoomOut;
      POINTS ps;
      POINT p;
      nsint x, y;


      ps = MAKEPOINTS( lParam );

      p.x = ps.x;
      p.y = ps.y;

      hWnd = s_DisplayWindowHandles[0];
      ScreenToClient( hWnd, &p );

      x = p.x;
      y = p.y;

      dw = s_DisplayWindows;
      ns_assert( NULL != dw );

      workspace = dw->workspace;
      index     = dw->index;
      display   = dw->display;

      zoom    = GetWorkspaceZoom( workspace, index, display );

      zoomOut = GET_WHEEL_DELTA_WPARAM(wParam) < 0; //( nsint )!____zooming_in;

      /* If at maximum or minimum zoom then just return. */

      if( _mMAX_ZOOM_FACTOR == zoom && ! zoomOut )
         return;

      if( _mMIN_ZOOM_FACTOR == zoom && zoomOut )
         return;

      corner = GetWorkspaceCorner( workspace, index, display );

      MakeVector2i( &clientCoord, x, y );

      /* Change zoom factor and then see calculate how much
         the view needs to be translated. */

      _DisplayWindowClientToGlobalVector( &oldGlobalCoord, 
                                          &clientCoord,
                                          &corner,
                                          zoom,
                                          display
                                        );

      if( zoomOut )
         _GetClosestSmallerZoomFactor( zoom, &zoom );
      else
         _GetClosestLargerZoomFactor( zoom, &zoom );


      _DisplayWindowClientToGlobalVector( &newGlobalCoord,
                                          &clientCoord,
                                          &corner,
                                          zoom,
                                          display
                                        );

      SubVector2i( &oldGlobalCoord, &newGlobalCoord, &difference );
      
      /* Translate the upper left corner so the spot in the workspace where
         the user clicked is still at that spot after zooming. */
      AddVector2i( &corner, &difference, &corner );

      SetWorkspaceZoom( workspace, index, display, zoom );
      SetWorkspaceCorner( workspace, index, display, corner );

      _CenterOrLimitDisplayWindowImage( dw );
      DrawDisplayWindow( hWnd );
      }
   }


/*
void _DisplayWindowOnKillFocus( DisplayWindow* dw )
   {
   dw->isActive = 0;
   DrawDisplayWindow( dw->hWnd );
   }
*/


extern nsint _main_window_on_keydown( nsint );
extern nsint _main_window_on_keyup( nsint );


void _display_window_set_zoom_out( void )
   {
   DisplayWindow *dw = s_DisplayWindows + 0;

   if( ! ____zooming_in )
      return;

   ____zooming_in = 0;
   _SetDisplayWindowCursor( dw->hWnd, "ZOOM_OUT_TOOL_CURSOR", 1 );
   }


void _display_window_set_zoom_in( void )
   {
   DisplayWindow *dw = s_DisplayWindows + 0;

   if( ____zooming_in )
      return;

   ____zooming_in = 1;
   _SetDisplayWindowCursor( dw->hWnd, "ZOOM_IN_TOOL_CURSOR", 1 );
   }


//LRESULT _DisplayWindowOnKeyDown( DisplayWindow* dw, nsint key )
  // {  return ( LRESULT )_main_window_on_keydown( key );  }


//LRESULT _DisplayWindowOnKeyUp( DisplayWindow* dw, nsint key )
  // {  return ( LRESULT )_main_window_on_keyup( key );  }


LRESULT CALLBACK _DisplayWindowProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   DisplayWindow *dw = _MapHandleToDisplayWindow( hWnd );

   if( NULL == dw )
      return DefWindowProc( hWnd, uMsg, wParam, lParam );

   switch( uMsg )
      {
      case WM_MOUSEMOVE:
         _DisplayWindowOnMouseMove( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

     case WM_LBUTTONDOWN:
         _DisplayWindowOnLButtonDown( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

      case WM_LBUTTONUP:
         _DisplayWindowOnLButtonUp( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

      case WM_RBUTTONDOWN:
         _display_window_on_rbutton_down( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

      case WM_RBUTTONUP:
         _display_window_on_rbutton_up( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

      case WM_LBUTTONDBLCLK:
         _DisplayWindowOnLButtonDblClk( dw, mGET_MOUSE_X( lParam ), mGET_MOUSE_Y( lParam ), ( nsuint )wParam );
         break;

 
      case WM_PAINT:
         {
         PAINTSTRUCT ps;
         HDC         hDC;

         hDC = BeginPaint( hWnd, &ps );
         _DisplayWindowOnPaint( dw, hDC );
         EndPaint( hWnd, &ps );
         }
         break;

      case WM_SIZE:
         _DisplayWindowOnSize( dw, wParam );
         break;

//      case WM_KILLFOCUS:
  //       _DisplayWindowOnKillFocus( dw );
    //     break;



      case WM_SYSKEYDOWN:
         if( _main_window_on_keydown( ( nsint )wParam ) )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );

      case WM_KEYDOWN:
         return ( LRESULT )_main_window_on_keydown( ( nsint )wParam );

      case WM_SYSKEYUP:
         if( _main_window_on_keyup( ( nsint )wParam ) )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );

      case WM_KEYUP:
         return ( LRESULT )_main_window_on_keyup( ( nsint )wParam );



   /*   case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
         return _DisplayWindowOnKeyDown( dw, ( nsint)wParam );

      case WM_SYSKEYUP:
      case WM_KEYUP:
         return _DisplayWindowOnKeyUp( dw, (nsint)wParam );*/


      case WM_NCDESTROY:
         _DestroyDisplayWindowBackBuffer( dw );
         //DeleteObject( dw->hFont );
         break;

      case mMSG_USER_Update:
         DrawDisplayWindow( hWnd );
         break;

      case mMSG_USER_SetMouseMode:
         {
         nsint newMode = ( nsint )lParam;

         ns_assert( -1 <= newMode && newMode < mDISPLAY_WINDOW_NUM_MOUSE_MODES );

         if( newMode != s_CurrentMouseMode )
            {
            if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[ s_CurrentMouseMode ].onUnset )
               ( s_MouseModes[ s_CurrentMouseMode ].onUnset )( hWnd );

            s_CurrentMouseMode = newMode;

            if( -1 != s_CurrentMouseMode && NULL != s_MouseModes[ s_CurrentMouseMode ].onSet )
               ( s_MouseModes[ s_CurrentMouseMode ].onSet )( hWnd );
            }
         }
         break;

      case mMSG_USER_SetDisplayType:
         dw->display = ( nsint )lParam;

         if( 0 != dw->workspace )
            _CenterOrLimitDisplayWindowImage( dw );

         DrawDisplayWindow( hWnd );
         break;

		/* HACK There seems to be some sort of bug if "Center & Limit Image is not selected"
			so we'll just set it temporarily. */
      case mMSG_USER_FitImage:
			{
			nsboolean old_value = ____2d_display_center_and_limit_image;
			____2d_display_center_and_limit_image = NS_TRUE;

         FitDisplayWindowImage( dw, ( nsint )wParam );

			____2d_display_center_and_limit_image = old_value;
			}
         break;
         
      case mMSG_USER_SetWorkspace:
         dw->workspace = ( nsuint )lParam;
         DrawDisplayWindow( hWnd );
         break;

      case mMSG_USER_SetPenThickness:
         dw->pen_thickness = ( nsuint )lParam;
         DrawDisplayWindow( hWnd );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;
   }





/*
LRESULT CALLBACK _ns_widgets_wnd_proc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
   {
	NS_PRIVATE nsboolean ____ns_widgets_support_transparency = NS_FALSE;

   switch( msg )
      {
		case WM_ENTERSIZEMOVE:
			//if( ____ns_widgets_support_transparency )
			//	{
			//	SetWindowLong( wnd, GWL_EXSTYLE, GetWindowLong( wnd, GWL_EXSTYLE ) | WS_EX_LAYERED );
			//	SetLayeredWindowAttributes( wnd, 0, 128, LWA_ALPHA );
			//	}
			break;

		case WM_EXITSIZEMOVE:
			//if( ____ns_widgets_support_transparency )
			//	SetWindowLong( wnd, GWL_EXSTYLE, GetWindowLong( wnd, GWL_EXSTYLE ) & ~WS_EX_LAYERED );
			break;
 
      case WM_CREATE:
			{
			DWORD version = GetVersion();

			// i.e. check if we're running an OS newer or same as Windows 2000.
			if( 4 < ( DWORD )( LOBYTE( LOWORD( version ) ) ) )
				____ns_widgets_support_transparency = NS_TRUE;
			}
         break;

      default:
         return DefWindowProc( wnd, msg, wparam, lparam );
      }

   return 0;
   }


void _ns_widget_create( HWND parent )
	{
   WNDCLASSEX  wcx;
   MSG         msg;
	HWND        wnd;
 

   wcx.cbSize         = sizeof( WNDCLASSEX );
   wcx.style          = CS_HREDRAW | CS_VREDRAW;
   wcx.lpfnWndProc    = _ns_widgets_wnd_proc;
   wcx.cbClsExtra     = 0;
   wcx.cbWndExtra     = 0;
   wcx.hInstance      = g_Instance;
   wcx.hIcon          = NULL;
   wcx.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wcx.hbrBackground  = NULL;
   wcx.lpszClassName  = "____ns_widget";
   wcx.lpszMenuName   = NULL;
   wcx.hIconSm        = NULL;

   if( ! RegisterClassEx( &wcx ) )
      return;

   wnd = CreateWindowEx(
				0,
				"____ns_widget",
				NULL,
				WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
				100, 
				100,
				200,
				200,
				parent,
				NULL,
				g_Instance,
				NULL
				);

   if( NULL == wnd )
      return;

   UpdateWindow( wnd );
	ShowWindow( wnd, SW_SHOW );
	}

*/







HWND CreateDisplayWindow
   ( 
   HWND  hParent, 
   nsint   ID, 
   nsint   index,
   nsint   x, 
   nsint   y, 
   nsint   width, 
   nsint   height,
   nsint   display
   )
   {
   WNDCLASSEX wcx;
   HWND hReturn;


   if( ! s_DisplayWindowsAreRegistered )
      {
      wcx.cbSize        = sizeof( WNDCLASSEX );
      wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      wcx.lpfnWndProc   = _DisplayWindowProcedure;
      wcx.cbClsExtra    = 0;
      wcx.cbWndExtra    = 0;
      wcx.hIcon         = NULL;
      wcx.hCursor       = LoadCursor( NULL, IDC_ARROW );
      wcx.hInstance     = g_Instance;
      wcx.hbrBackground = NULL;
      wcx.lpszClassName = "DisplayWindow";
      wcx.lpszMenuName  = NULL;
      wcx.hIconSm       = NULL;

      if( 0 == RegisterClassEx( &wcx ) )
         {
         g_Error = eERROR_OPER_SYSTEM;
         return NULL;
         }

      s_DisplayWindowsAreRegistered = 1;
      }
   
   ns_assert( s_NumDisplayWindows < _MAX_DISPLAY_WINDOWS );

   s_DisplayWindows[ s_NumDisplayWindows ].hWnd = CreateWindowEx
       (
       0,
       "DisplayWindow",
       NULL,
       WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
       x, y, width, height,
       hParent,
       ( HMENU )ID, g_Instance, NULL
       );

   if( NULL == s_DisplayWindows[ s_NumDisplayWindows ].hWnd )
      {
      g_Error = eERROR_OPER_SYSTEM;
      return NULL;
      }

	//_ns_widget_create( s_DisplayWindows[ s_NumDisplayWindows ].hWnd );

   if( ! s_DisplayWindowMouseModesAreSet )
      {
      s_DisplayWindowMouseModesAreSet = 1;
      }

   s_DisplayWindows[ s_NumDisplayWindows ].ID             = ID;
   s_DisplayWindows[ s_NumDisplayWindows ].index          = index;
   s_DisplayWindows[ s_NumDisplayWindows ].display        = display;
   s_DisplayWindows[ s_NumDisplayWindows ].workspace     = 0;
   s_DisplayWindows[ s_NumDisplayWindows ].haveBackBuffer = 0;
//   s_DisplayWindows[ s_NumDisplayWindows ].isActive       = 0;
   s_DisplayWindows[ s_NumDisplayWindows ].isDragging     = 0;
   s_DisplayWindows[ s_NumDisplayWindows ].hasCapture     = 0;
//   s_DisplayWindows[ s_NumDisplayWindows ].selectedVertex = NULL;
   s_DisplayWindows[ s_NumDisplayWindows ].pen_thickness  = 1;
//   s_DisplayWindows[ s_NumDisplayWindows ].useSolidVertices = 1;

   //s_DisplayWindows[ s_NumDisplayWindows ].conn_comp_colors = ____conn_comp_colors;
   //s_DisplayWindows[ s_NumDisplayWindows ].order_colors     = ____order_colors;
   //s_DisplayWindows[ s_NumDisplayWindows ].section_colors   = ____section_colors;

   //s_DisplayWindows[ s_NumDisplayWindows ].hFont =
     // CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
       //           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
         //         DEFAULT_PITCH, "Arial" );

   hReturn = s_DisplayWindows[ s_NumDisplayWindows ].hWnd;

   s_DisplayWindowHandles[ s_NumDisplayWindows ] = hReturn;
   ++s_NumDisplayWindows;

   return hReturn;

   }/* CreateDisplayWindow() */


nsint ____do_draw_model_sections = 0;


nschar _section_number[64];




nssize _display_window_get_line_and_point_size( DisplayWindow *dw )
   {  return ( nssize )dw->pen_thickness;  }



//void _display_window_set_current_color( NsColor4ub C, DisplayWindow *dw )
//   {  dw->curr_color = C;  }


//void _display_window_set_filled( nsboolean b, DisplayWindow *dw )
//   {  dw->filled = b;  }


void DrawDisplayWindow( HWND wnd )
   {
	____redraw_2d_need_render = NS_TRUE;
   InvalidateRect( wnd, NULL, FALSE );
   }



extern void ____ns_model_graft_render
   (
   void ( render )( const NsVector3i*, nsulong, nspointer ),
   nspointer user_data
   );


nsint ____do_draw_spine_voxels = 1;
nsint ____do_draw_spine_ids = 0;



extern nsboolean ____flicker_free_rendering;
extern nsboolean ____spines_color_by_layer;
extern nsboolean ____view_measurement_text;
extern nsboolean ____draw_while_grafting;
extern nsboolean ____view_spine_vectors;
extern nsboolean ____view_spine_attachments;
extern nsboolean ____view_non_spine_voxels;
extern nsboolean ____view_2d_polygon_borders;
extern nsboolean ____measurement_view_thresholding;
extern nsboolean ____measurement_view_surface_mesh;
extern nsboolean ____view_spine_candidate_voxels;
extern nsboolean ____view_spine_maxima_voxels;
extern nsboolean ____view_maximum_spine_height;
extern nsboolean ____spines_use_round_corners;

extern nsfloat ____msh_zone_alpha;
extern nsboolean ____use_antialiasing;;

extern nsboolean ____flash_frames_active;
extern nsboolean ____flash_hidden_active;

#include <render/nsrender2d-os-windows.h>


NS_PRIVATE NsRenderState  ____render_state_2d;
NS_PRIVATE nsboolean      ____render_state_2d_init = NS_FALSE;
NS_PRIVATE NsColor4ub     ____vertex_type_colors_2d[ NS_MODEL_VERTEX_NUM_TYPES ];
NS_PRIVATE NsColor4ub     ____spine_type_colors_2d[ NS_SPINE_NUM_TYPES ];
NS_PRIVATE NsColor4ub     ____function_type_colors_2d[ NS_MODEL_FUNCTION_NUM_TYPES ];

void _DrawDisplayWindowNeuronTree( DisplayWindow* dw )
   {
   NsRenderState  *state;
   RECT            clientViewport;


   //dw->zoom   = GetWorkspaceZoom( dw->workspace, dw->index, dw->display );
   //dw->corner = GetWorkspaceCorner( dw->workspace, dw->index, dw->display );

   GetClientRect( dw->hWnd, &clientViewport );

   _DisplayWindowClientToGlobalRectangle(
      &dw->globalViewport,
      &clientViewport,
      &dw->corner,
      dw->zoom,
      dw->display
      );


	state = &____render_state_2d;

	if( ! ____render_state_2d_init )
		{
		ns_render_state_init( state );

		state->constants.iface.pre_images_func     = ns_render2d_pre_images;
		state->constants.iface.pre_borders_func    = ns_render2d_pre_borders;
		state->constants.iface.pre_lines_func      = ns_render2d_pre_lines;
		state->constants.iface.pre_points_func     = ns_render2d_pre_points;
		state->constants.iface.pre_texts_func      = ns_render2d_pre_texts;
		state->constants.iface.pre_spheres_func    = ns_render2d_pre_spheres;
		state->constants.iface.pre_aabboxes_func   = ns_render2d_pre_aabboxes;
		state->constants.iface.pre_frustums_func   = ns_render2d_pre_frustums;
		state->constants.iface.pre_triangles_func  = ns_render2d_pre_triangles;
		state->constants.iface.pre_ellipses_func   = ns_render2d_pre_ellipses;
		state->constants.iface.pre_rectangles_func = ns_render2d_pre_rectangles;

		state->constants.iface.post_images_func     = ns_render2d_post_images;
		state->constants.iface.post_borders_func    = ns_render2d_post_borders;
		state->constants.iface.post_lines_func      = ns_render2d_post_lines;
		state->constants.iface.post_points_func     = ns_render2d_post_points;
		state->constants.iface.post_texts_func      = ns_render2d_post_texts;
		state->constants.iface.post_spheres_func    = ns_render2d_post_spheres;
		state->constants.iface.post_aabboxes_func   = ns_render2d_post_aabboxes;
		state->constants.iface.post_frustums_func   = ns_render2d_post_frustums;
		state->constants.iface.post_triangles_func  = ns_render2d_post_triangles;
		state->constants.iface.post_ellipses_func   = ns_render2d_post_ellipses;
		state->constants.iface.post_rectangles_func = ns_render2d_post_rectangles;

		//ns_render_state_set_default_steps_order( state );
		ns_render_state_set_step( state, 0, NS_RENDER_STEP_SAMPLES );
		ns_render_state_set_step( state, 1, NS_RENDER_STEP_GRAFTING );
		ns_render_state_set_step( state, 2, NS_RENDER_STEP_VERTICES );
		ns_render_state_set_step( state, 3, NS_RENDER_STEP_EDGES );
		ns_render_state_set_step( state, 4, NS_RENDER_STEP_SPINES );

		state->constants.dimensions = 2;

		state->constants.VERTEX_NO_SHAPE              = IDM_RENDER_MODE_VERTEX_OFF;
		state->constants.VERTEX_SHAPE_HOLLOW_ELLIPSES = IDM_RENDER_MODE_VERTEX_HOLLOW_ELLIPSES;
		state->constants.VERTEX_SHAPE_SOLID_ELLIPSES  = IDM_RENDER_MODE_VERTEX_SOLID_ELLIPSES;
		state->constants.VERTEX_SHAPE_POINTS          = IDM_RENDER_MODE_VERTEX_POINTS;

		state->constants.EDGE_NO_SHAPE              = IDM_RENDER_MODE_EDGE_OFF;
		state->constants.EDGE_SHAPE_HOLLOW_FRUSTUMS = IDM_RENDER_MODE_EDGE_HOLLOW_FRUSTUMS;
		state->constants.EDGE_SHAPE_SOLID_FRUSTUMS  = IDM_RENDER_MODE_EDGE_SOLID_FRUSTUMS;
		state->constants.EDGE_SHAPE_LINES           = IDM_RENDER_MODE_EDGE_LINES;

		state->constants.SPINE_NO_SHAPE                = IDM_VIEW_SPINE_MODE_OFF;
		state->constants.SPINE_SHAPE_POINTS            = IDM_VIEW_SPINE_MODE_POINTS;
		state->constants.SPINE_SHAPE_SOLID_RECTANGLES  = IDM_VIEW_SPINE_MODE_SOLID_RECTANGLES;
		state->constants.SPINE_SHAPE_HOLLOW_RECTANGLES = IDM_VIEW_SPINE_MODE_HOLLOW_RECTANGLES;
		state->constants.SPINE_SHAPE_SOLID_VOLUMETRIC  = IDM_VIEW_SPINE_MODE_SOLID_VOLUMETRIC;
		state->constants.SPINE_SHAPE_HOLLOW_VOLUMETRIC = IDM_VIEW_SPINE_MODE_HOLLOW_VOLUMETRIC;
		state->constants.SPINE_SHAPE_SOLID_SURFACE     = IDM_VIEW_SPINE_MODE_SOLID_SURFACE;
		state->constants.SPINE_SHAPE_HOLLOW_SURFACE    = IDM_VIEW_SPINE_MODE_HOLLOW_SURFACE;
		state->constants.SPINE_SHAPE_SOLID_ELLIPSES    = IDM_VIEW_SPINE_MODE_SOLID_ELLIPSES;
		state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES   = IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSES;
		state->constants.SPINE_SHAPE_SOLID_BALL_AND_STICK = IDM_VIEW_SPINE_MODE_SOLID_BALL_AND_STICK;
		state->constants.SPINE_SHAPE_HOLLOW_BALL_AND_STICK = IDM_VIEW_SPINE_MODE_HOLLOW_BALL_AND_STICK;
		state->constants.SPINE_SHAPE_SOLID_ELLIPSES_HD = IDM_VIEW_SPINE_MODE_SOLID_ELLIPSE_HEAD_DIAM;
		state->constants.SPINE_SHAPE_HOLLOW_ELLIPSES_HD = IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSE_HEAD_DIAM;
		state->constants.SPINE_SHAPE_CLUMPS             = IDM_VIEW_SPINE_MODE_CLUMPS;

		state->constants.VERTEX_SINGLE_COLOR             = IDM_COLOR_SCHEME_VERTEX_SINGLE_COLOR;
		state->constants.VERTEX_COLOR_BY_TYPE            = IDM_COLOR_SCHEME_VERTEX_BY_TYPE;
		state->constants.VERTEX_COLOR_BY_FUNCTION        = IDM_COLOR_SCHEME_VERTEX_BY_FUNCTION;
		state->constants.VERTEX_COLOR_BY_COMPONENT       = IDM_COLOR_SCHEME_VERTEX_BY_COMPONENT;
		state->constants.VERTEX_COLOR_BY_ORDER           = IDM_COLOR_SCHEME_VERTEX_BY_ORDER;
		state->constants.VERTEX_COLOR_BY_SECTION         = IDM_COLOR_SCHEME_VERTEX_BY_SECTION;
		state->constants.VERTEX_COLOR_BY_FILE_NUMBER     = IDM_COLOR_SCHEME_VERTEX_BY_FILE_NUMBER;
		state->constants.VERTEX_COLOR_IS_MANUALLY_TRACED = IDM_COLOR_SCHEME_VERTEX_IS_MANUALLY_TRACED;

		state->constants.manually_traced_color  = NS_COLOR4UB_RED;
		state->constants.vertex_type_colors     = ____vertex_type_colors_2d;
		state->constants.num_vertex_type_colors = NS_MODEL_VERTEX_NUM_TYPES;

		state->constants.component_colors     = ____conn_comp_colors;
		state->constants.num_component_colors = ____num_colors;

		state->constants.EDGE_SINGLE_COLOR       = IDM_COLOR_SCHEME_EDGE_SINGLE_COLOR;
		state->constants.EDGE_COLOR_BY_FUNCTION  = IDM_COLOR_SCHEME_EDGE_BY_FUNCTION;
		state->constants.EDGE_COLOR_BY_COMPONENT = IDM_COLOR_SCHEME_EDGE_BY_CONN_COMP;
		state->constants.EDGE_COLOR_BY_ORDER     = IDM_COLOR_SCHEME_EDGE_BY_ORDER;
		state->constants.EDGE_COLOR_BY_SECTION   = IDM_COLOR_SCHEME_EDGE_BY_SECTION;

		state->constants.edge_order_colors       = ____order_colors;
		state->constants.num_edge_order_colors   = ____num_colors;

		state->constants.edge_section_colors     = ____section_colors;
		state->constants.num_edge_section_colors = ____num_colors;

		state->constants.SPINE_SINGLE_COLOR    = IDM_RENDER_SPINE_COLOR_SINGLE;
		state->constants.SPINE_COLOR_BY_TYPE   = IDM_RENDER_SPINE_COLOR_BY_TYPE;
		state->constants.SPINE_COLOR_RANDOM    = IDM_RENDER_SPINE_COLOR_RANDOM;
		state->constants.SPINE_COLOR_IS_MANUAL = IDM_RENDER_SPINE_COLOR_IS_MANUAL;
		state->constants.SPINE_COLOR_BY_ANGLE  = IDM_RENDER_SPINE_COLOR_BY_ANGLE;

		state->constants.spine_type_colors     = ____spine_type_colors_2d;
		state->constants.num_spine_type_colors = NS_SPINE_NUM_TYPES;

		state->constants.function_type_colors     = ____function_type_colors_2d;
		state->constants.num_function_type_colors = NS_MODEL_FUNCTION_NUM_TYPES;

		state->constants.get_brush = ns_color_db_get;
		state->constants.set_brush = ns_color_db_set;
		state->constants.get_pen   = ns_color_db_get;
		state->constants.set_pen   = ns_color_db_set;

		state->constants.palette_colors = NULL;
		state->constants.palette_size   = 0;

		____render_state_2d_init = NS_TRUE;
		}
		
   switch( dw->display )
      {
      case NS_XY:
         state->constants.iface.image_func     = ns_render2d_image_xy;
         state->constants.iface.border_func    = ns_render2d_border_xy;
         state->constants.iface.line_func      = ns_render2d_line_xy;
         state->constants.iface.point_func     = ns_render2d_point_xy;
         state->constants.iface.text_func      = ns_render2d_text_xy;
         state->constants.iface.sphere_func    = ns_render2d_sphere_xy;
         state->constants.iface.aabbox_func    = ns_render2d_aabbox_xy;
         state->constants.iface.frustum_func   = ns_render2d_frustum_xy;
         state->constants.iface.triangle_func  = ns_render2d_triangle_xy;
         state->constants.iface.ellipse_func   = ns_render2d_ellipse_xy;
         state->constants.iface.rectangle_func = ns_render2d_rectangle_xy;
         break;

      case NS_ZY:
         state->constants.iface.image_func     = ns_render2d_image_zy;
         state->constants.iface.border_func    = ns_render2d_border_zy;
         state->constants.iface.line_func      = ns_render2d_line_zy;
         state->constants.iface.point_func     = ns_render2d_point_zy;
         state->constants.iface.text_func      = ns_render2d_text_zy;
         state->constants.iface.sphere_func    = ns_render2d_sphere_zy;
         state->constants.iface.aabbox_func    = ns_render2d_aabbox_zy;
         state->constants.iface.frustum_func   = ns_render2d_frustum_zy;
         state->constants.iface.triangle_func  = ns_render2d_triangle_zy;
         state->constants.iface.ellipse_func   = ns_render2d_ellipse_zy;
         state->constants.iface.rectangle_func = ns_render2d_rectangle_zy;
         break;

      case NS_XZ:
         state->constants.iface.image_func     = ns_render2d_image_xz;
         state->constants.iface.border_func    = ns_render2d_border_xz;
         state->constants.iface.line_func      = ns_render2d_line_xz;
         state->constants.iface.point_func     = ns_render2d_point_xz;
         state->constants.iface.text_func      = ns_render2d_text_xz;
         state->constants.iface.sphere_func    = ns_render2d_sphere_xz;
         state->constants.iface.aabbox_func    = ns_render2d_aabbox_xz;
         state->constants.iface.frustum_func   = ns_render2d_frustum_xz;
         state->constants.iface.triangle_func  = ns_render2d_triangle_xz;
         state->constants.iface.ellipse_func   = ns_render2d_ellipse_xz;
         state->constants.iface.rectangle_func = ns_render2d_rectangle_xz;
         break;
      }

	state->constants.dataset    = workspace_dataset( dw->workspace );
	state->constants.proj_xy    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_XY );
	state->constants.proj_zy    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_ZY );
	state->constants.proj_xz    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_XZ );
   state->constants.model      = GetWorkspaceNeuronTree( dw->workspace );
   state->constants.sampler    = workspace_sampler( dw->workspace );
   state->constants.voxel_info = workspace_get_voxel_info( dw->workspace );
   state->constants.settings   = workspace_settings( dw->workspace );
	state->constants.classifier = ns_spines_classifier_get();

   state->constants.flicker_free_rendering = ____flicker_free_rendering;
	state->constants.draw_dataset_only      = ____flash_frames_active;
	state->constants.draw_hidden_objects    = ____flash_hidden_active;

   state->constants.view_sampler_text         = ____view_measurement_text;
   state->constants.view_graft_thresholding   = ____draw_while_grafting;
   state->constants.view_graft_all_vertices   = ! ____draw_junction_vertices_only;
   state->constants.view_spine_candidate_voxels = ____view_spine_candidate_voxels;
   state->constants.view_spine_maxima_voxels    = ____view_spine_maxima_voxels;
   state->constants.view_spine_vectors        = ____view_spine_vectors;
   state->constants.view_spine_attachments    = ____view_spine_attachments;
   //state->constants.view_non_spine_voxels     = ____view_non_spine_voxels;
   state->constants.spines_color_by_layer     = ____spines_color_by_layer;
   state->constants.view_sampler_thresholding = ____measurement_view_thresholding;
   state->constants.view_sampler_surface_mesh = ____measurement_view_surface_mesh;
   state->constants.view_maximum_spine_height = ____view_maximum_spine_height;
   state->constants.spines_soft_corners       = ____spines_use_round_corners;

   state->constants.vertex_shape_mode = ____neurites_vertex_render_mode;
   state->constants.edge_shape_mode   = ____neurites_edge_render_mode;
   state->constants.spine_shape_mode  = ____spines_render_mode;

   ____vertex_type_colors_2d[ NS_MODEL_VERTEX_ORIGIN   ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_ORIGIN_VERTEX );
   ____vertex_type_colors_2d[ NS_MODEL_VERTEX_EXTERNAL ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_EXTERNAL_VERTEX );
   ____vertex_type_colors_2d[ NS_MODEL_VERTEX_JUNCTION ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_JUNCTION_VERTEX );
   ____vertex_type_colors_2d[ NS_MODEL_VERTEX_LINE     ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_LINE_VERTEX );

   state->constants.vertex_color_scheme = ____vertex_color_scheme;
   state->constants.vertex_single_color = workspace_get_color( dw->workspace, WORKSPACE_COLOR_VERTEX );

   state->constants.edge_color_scheme   = ____edge_color_scheme;
   state->constants.edge_single_color   = workspace_get_color( dw->workspace, WORKSPACE_COLOR_EDGE );

	____spine_type_colors_2d[ NS_SPINE_INVALID  ] = workspace_get_spine_color_by_type( dw->workspace, NS_SPINE_INVALID );
   ____spine_type_colors_2d[ NS_SPINE_OTHER    ] = workspace_get_spine_color_by_type( dw->workspace, NS_SPINE_OTHER );
   ____spine_type_colors_2d[ NS_SPINE_THIN     ] = workspace_get_spine_color_by_type( dw->workspace, NS_SPINE_THIN );
   ____spine_type_colors_2d[ NS_SPINE_MUSHROOM ] = workspace_get_spine_color_by_type( dw->workspace, NS_SPINE_MUSHROOM );
   ____spine_type_colors_2d[ NS_SPINE_STUBBY   ] = workspace_get_spine_color_by_type( dw->workspace, NS_SPINE_STUBBY );

	____function_type_colors_2d[ NS_MODEL_FUNCTION_UNKNOWN         ] = NS_COLOR4UB_BLACK;
	____function_type_colors_2d[ NS_MODEL_FUNCTION_SOMA            ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_FUNCTION_SOMA );
	____function_type_colors_2d[ NS_MODEL_FUNCTION_BASAL_DENDRITE  ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE );
	____function_type_colors_2d[ NS_MODEL_FUNCTION_APICAL_DENDRITE ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE );
	____function_type_colors_2d[ NS_MODEL_FUNCTION_AXON            ] = workspace_get_color( dw->workspace, WORKSPACE_COLOR_FUNCTION_AXON );

   state->constants.spine_color_scheme      = ____spines_color_scheme;
   state->constants.spine_single_color      = workspace_get_color( dw->workspace, WORKSPACE_COLOR_SPINE );
	state->constants.spine_random_colors     = ns_spines_random_colors();
	state->constants.num_spine_random_colors = ns_spines_num_random_colors();

   state->constants.brush_db = workspace_brush_db( dw->workspace );
   state->constants.pen_db   = workspace_pen_db( dw->workspace );

   state->constants.line_size       = ( nsint )dw->pen_thickness;
   state->constants.point_size      = ( nsint )dw->pen_thickness * 2;
   state->constants.corner_x        = ( nsint )dw->corner.x;
   state->constants.corner_y        = ( nsint )dw->corner.y;
   state->constants.display         = dw->display;
   state->constants.zoom            = dw->zoom;
   state->constants.graphics        = dw->backBuffer.hDC;
   state->constants.polygon_borders = ____view_2d_polygon_borders;
   //state->constants.msh_zone_alpha  = ____msh_zone_alpha;

	state->constants.anti_aliasing = ____use_antialiasing;

   ns_render( state );
   }





