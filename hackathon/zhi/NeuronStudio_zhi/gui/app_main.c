#include "oglwin.h"
#include "app_lib.h"
#include "app_display.h"
#include "app_data.h"
#include "clrdlg.h"
#include "resource.h"
#include <conio.h>
#include <commctrl.h>
#include <image/nspixels-orthoproject.h>



//extern nspointer  ____volume_pixels;
//extern nssize     ____volume_width;
//extern nssize     ____volume_height;
//extern nssize     ____volume_length;
//extern nssize     ____volume_bpp;


/*
nsint ____naas_dim1;
nsint ____naas_dim2;
nsint ____naas_dim3;
NsVector3i ____naas_center;
nsfloat ____naas_xdir;
nsfloat ____naas_ydir;
nsfloat ____naas_zdir;
NsVector3i  *____naas_points = NULL;
nssize ____naas_num_points = 0;
NsVector3i ____naas_corners[8];
*/


nsboolean ____alert_user_about_splats = NS_FALSE;


nsboolean ____allow_volume_labeling = NS_FALSE;


nsboolean ____workspace_show_tiles = NS_FALSE;
nsboolean ____visualize_spine_voxels = NS_FALSE;


nsboolean ____xy_slice_enabled = NS_FALSE;
nssize ____xy_slice_index = 0;


nsboolean ____use_splats_alpha = NS_FALSE;


nsboolean ____optimize_splats_movement = NS_TRUE;


nsboolean ____grafting_is_running = NS_FALSE;


nsboolean ____draw_while_grafting = NS_TRUE;
nsboolean ____draw_junction_vertices_only = NS_TRUE;

nsboolean ____run_model_filtering = NS_TRUE;


nsboolean ____2d_display_center_and_limit_image = NS_FALSE;


nsint ____projection_type = NS_PIXEL_PROC_ORTHO_PROJECT_MAX;


nsint ____selected_num_cpu = IDM_RUN_NUM_CPU_1;


nsboolean ____view_measurement_text = NS_FALSE;


nsboolean ____view_sholl_analysis = NS_FALSE;


//nsboolean ____use_2d_neurite_sampling = NS_FALSE;

nsboolean ____use_2d_spine_bounding_box = NS_FALSE;

//nsboolean ____use_2d_measurement_sampling = NS_FALSE;


nsboolean ____use_antialiasing = NS_FALSE;


nsuint ____import_file_number = 1;


nsboolean ____view_3d_volume_grid = NS_FALSE;


nsboolean ____splats_generate_from_spine_voxels;


nsboolean ____gamma_correction = NS_FALSE;


nsboolean ____projected_2d_grafting = NS_FALSE;


nsboolean ____flicker_free_rendering = NS_TRUE;

nsboolean ____view_maximum_spine_height = NS_FALSE;


nsboolean ____view_roi_always_on_top = NS_FALSE;
nsboolean ____view_seed_always_on_top = NS_FALSE;

nsboolean ____view_edge_order_numbers = NS_FALSE;
nsboolean ____view_edge_section_numbers = NS_FALSE;

nsboolean ____show_roi_bounding_box = NS_FALSE;


//nsboolean ____markers_active = NS_FALSE;
//nsint ____markers_type = NS_SAMPLER_MARK_STUBBY;


nsboolean ____view_spine_vectors = NS_FALSE;
nsboolean ____view_spine_attachments = NS_FALSE;

nsboolean ____spines_color_by_layer = NS_FALSE;
nsboolean ____view_spine_voxels_dts = NS_FALSE;
nsboolean ____view_non_spine_voxels = NS_FALSE;
nsboolean ____view_2d_polygon_borders = NS_TRUE;

nsboolean ____view_spine_maxima_voxels = NS_FALSE;
nsboolean ____view_spine_candidate_voxels = NS_FALSE;

nsboolean ____spines_use_round_corners = NS_FALSE;

nsboolean ____view_spine_ids = NS_FALSE;


nsboolean ____show_2d_axes = NS_TRUE;
nsboolean ____show_memory_status_widget = NS_FALSE;


NsVector3f ____model_3d_rotation_center;

nsint ____opengl_centering_mode = IDM_GL_VIEW_CENTER_ON_IMAGE;


nsint ____vertex_color_scheme = IDM_COLOR_SCHEME_VERTEX_BY_TYPE;
nsint ____edge_color_scheme = IDM_COLOR_SCHEME_EDGE_SINGLE_COLOR;


nsint ____neurites_vertex_render_mode = IDM_RENDER_MODE_VERTEX_SOLID_ELLIPSES;
nsint ____neurites_edge_render_mode = IDM_RENDER_MODE_EDGE_LINES;


nsint ____spines_render_mode = IDM_VIEW_SPINE_MODE_SOLID_ELLIPSES;
//IDM_VIEW_SPINE_MODE_SOLID_BALL_AND_STICK;
//IDM_VIEW_SPINE_MODE_POINTS;
nsint ____spines_color_scheme = IDM_RENDER_SPINE_COLOR_BY_TYPE;


NsRayburstKernelType ____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_SUPER_VIZ;
nsint                ____measuring_rays_count = IDM_MEASURING_RAYS_COUNT_HIGH;


NsRayburstKernelType ____spines_kernel_type = NS_RAYBURST_KERNEL_SUPER_VIZ;
nsint                ____spines_rays_count  = SPINES_RAYS_COUNT_HIGH;


NsRayburstInterpType ____measurement_interp_type = NS_RAYBURST_INTERP_BILINEAR;
nsint                ____measurement_interpolation = IDM_MEASURING_BILINEAR;
nsboolean            ____measurement_do_3d_rayburst = NS_FALSE;
nsboolean            ____measurement_view_thresholding = NS_FALSE;//NS_TRUE;
nsboolean            ____measurement_view_surface_mesh = NS_TRUE;


nsfloat ____msh_zone_alpha = 1.0f;


nsint ____neurites_interpolation = IDM_NEURITES_BILINEAR;
nsint ____spines_interpolation = IDM_SPINES_BILINEAR;


NsRayburstInterpType ____neurites_interp_type = NS_RAYBURST_INTERP_BILINEAR;
NsRayburstInterpType ____spines_interp_type = NS_RAYBURST_INTERP_BILINEAR;


//extern nsboolean ____projections_running;


nsboolean ____splats_need_config = NS_TRUE;
nsulong ____splats_current_threshold;
nsulong ____splats_default_threshold;
//nsint ____splats_contrast_menu_id;

nsboolean ____splats_generate;

nsboolean ____splats_view;

nsboolean ____solid_background;


nsint ____jitter_x = 0;
nsint ____jitter_y = 0;
nsint ____jitter_z = 3;


nschar _startup_directory[ NS_PATH_SIZE + 1 ];
nschar ____config_file[ NS_PATH_SIZE + 1 ];
nschar ____settings_file[ NS_PATH_SIZE + 1 ];
nschar ____log_file[ NS_PATH_SIZE + 1 ];
nschar ____executable_path[ NS_PATH_SIZE + 1 ];
nschar ____classifiers_dir[ NS_PATH_SIZE + 1 ];


nschar      ____spines_file_path[ NS_PATH_SIZE + 1 ];
nschar      ____spines_file_extension[ 32 ];
nschar      ____neurites_file_path[ NS_PATH_SIZE + 1 ];
nschar      ____neurites_file_extension[ 32 ];
nsboolean   ____neurites_file_exists;
nsint       ____neurites_file_type;
NsSwcHeader ____neurites_file_swc_header;
nsboolean   ____neurites_file_out_edge_info;


nsfloat ____volume_grid_step = -1.0f;


void _read_jitter_from_config_file( void )
   {
   NsConfigDb  db;
   NsError     error;


   if( NS_SUCCESS( ns_config_db_construct( &db ), error ) )
      {
      if( NS_SUCCESS( ns_config_db_read( &db, ____config_file, NULL ), error ) )
         if( ns_config_db_has_group( &db, "jitter" ) )
            if( ns_config_db_has_key( &db, "jitter", "x" ) &&
                ns_config_db_has_key( &db, "jitter", "y" ) &&
                ns_config_db_has_key( &db, "jitter", "z" )   )
               {
               ____jitter_x = ns_config_db_get_int( &db, "jitter", "x" );
               ____jitter_y = ns_config_db_get_int( &db, "jitter", "y" );
               ____jitter_z = ns_config_db_get_int( &db, "jitter", "z" );   
               }

      ns_config_db_destruct( &db );
      }
/*
   NsError error;
   NsConfigDbKey keys[] =
      {
      { "x", NS_CONFIG_DB_VALUE_INT },
      { "y", NS_CONFIG_DB_VALUE_INT },
      { "z", NS_CONFIG_DB_VALUE_INT }
      };


   if( NS_SUCCESS(
         ns_config_db_read_keys(
            ____config_file,
            "jitter",
            keys,
            NS_ARRAY_LENGTH( keys )
            ),
         error ) )
      {
      ____jitter_x = ( nsint )keys[0].value.as_int;
      ____jitter_y = ( nsint )keys[1].value.as_int;
      ____jitter_z = ( nsint )keys[2].value.as_int;
      }
*/

ns_println( "JITTER=%d,%d,%d", ____jitter_x, ____jitter_y, ____jitter_z );
   }


#define _mDISPLAY_WINDOW_ID_BASE             10000


#define _mDISPLAY_WINDOW_MINIMUM_ASPECT      .08f
#define _mDISPLAY_WINDOW_MAXIMUM_ASPECT      .90f

#define _mMAIN_DISPLAY_BORDER_SIZE           6


#define _mSPLITTER_SIZE                      5

#define _mBIT_VERT_SPLITTER_ACTIVE           mBIT_0
#define _mBIT_HORIZ_SPLITTER_ACTIVE          mBIT_1


#define _mMAIN_WINDOW_RIGHT_MARGIN_WIDTH     46
//#define _mMAIN_WINDOW_BOTTOM_MARGIN_HEIGHT   21/*16*/


#define _mMAIN_CTRLBOX_WIDTH                 38
#define _mMAIN_CTRLBOX_HEIGHT                282

#define _mMAIN_CTRLBOX_X( size )             ( ( ( size ) - _mMAIN_WINDOW_RIGHT_MARGIN_WIDTH ) +\
                                             ( _mMAIN_WINDOW_RIGHT_MARGIN_WIDTH - _mMAIN_CTRLBOX_WIDTH )/2 + 1 )

#define _mMAIN_CTRLBOX_Y                     0


#define _mMAIN_CTRLBOX_ZOOM_BUTTON           0//eDISPLAY_WINDOW_MOUSE_MODE_ZOOM
#define _mMAIN_CTRLBOX_DRAG_BUTTON           1//eDISPLAY_WINDOW_MOUSE_MODE_DRAG
#define _mMAIN_CTRLBOX_SEED_BUTTON           2//eDISPLAY_WINDOW_MOUSE_MODE_SEED
#define _mMAIN_CTRLBOX_RAYS_BUTTON           3//eDISPLAY_WINDOW_MOUSE_MODE_RAYS
#define _mMAIN_CTRLBOX_SPINE_BUTTON          4
#define _mMAIN_CTRLBOX_NEURITE_BUTTON        5
#define _mMAIN_CTRLBOX_ROI_BUTTON            6
#define MAIN_CTRLBOX_MAGNET_BUTTON           7
#define _mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON   8
#define _mMAIN_CTRLBOX_SIDE_VIEW_BUTTON      9
#define _mMAIN_CTRLBOX_TOP_VIEW_BUTTON       10

#define _mMAIN_CTRLBOX_NUM_BUTTONS           11

#define _mMAIN_CTRLBOX_ZOOM_BUTTON_ID        11000
#define _mMAIN_CTRLBOX_DRAG_BUTTON_ID        11001
#define _mMAIN_CTRLBOX_SEED_BUTTON_ID        11002
#define _mMAIN_CTRLBOX_RAYS_BUTTON_ID        11003
#define _mMAIN_CTRLBOX_SPINE_BUTTON_ID       11004
#define _mMAIN_CTRLBOX_NEURITE_BUTTON_ID     11005
#define _mMAIN_CTRLBOX_ROI_BUTTON_ID         11006
#define MAIN_CTRLBOX_MAGNET_BUTTON_ID        11007

//#define _mMAIN_CTRLBOX_LOW_SLIDER_ID         11003
//#define _mMAIN_CTRLBOX_HIGH_SLIDER_ID        11004
//#define _mMAIN_CTRLBOX_LOW_EDIT_ID           11005
//#define _mMAIN_CTRLBOX_HIGH_EDIT_ID          11006
//#define _mMAIN_CTRLBOX_LOW_UPDOWN_ID         11007
//#define _mMAIN_CTRLBOX_HIGH_UPDOWN_ID        11008
#define _mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON_ID  11009
#define _mMAIN_CTRLBOX_SIDE_VIEW_BUTTON_ID     11010
#define _mMAIN_CTRLBOX_TOP_VIEW_BUTTON_ID      11011

//#define _mMAIN_CTRLBOX_NUM_THRESHOLDS        1/*2*/

//#define _mMAIN_CTRLBOX_LOW_SLIDER            0
//#define _mMAIN_CTRLBOX_HIGH_SLIDER           1

//#define _mMAIN_CTRLBOX_LOW_EDIT              0
//#define _mMAIN_CTRLBOX_HIGH_EDIT             1

//#define _mMAIN_CTRLBOX_LOW_UPDOWN            0
//#define _mMAIN_CTRLBOX_HIGH_UPDOWN           1

//#define _mMAIN_CTRLBOX_EDIT_MAX_DIGITS       3

#define _mMAIN_WINDOW_MINIMUM_HEIGHT         348
#define _mMAIN_WINDOW_MINIMUM_WIDTH          432

/*
typedef enum
   {
   _eSPLITTER_NONE,
   _eSPLITTER_VERT,
   _eSPLITTER_HORIZ,
   _eSPLITTER_BOTH
   }
   _eSPLITTER_TYPE;
*/

typedef struct tagMainDisplay
   {
   //HWND       hWnd;
   //nsuint   width;
   //nsuint   height;
   //nsuint   flags;
   //nsint        vertSplitterX;
   //nsint        horizSplitterY;
   //nsfloat      vertSplitterAspect;
   //nsfloat      horizSplitterAspect;
   HWND       windows[ mNUM_DISPLAY_WINDOWS ];
   }
   MainDisplay;


typedef enum
   {
   eBUTTON_STATE_UP,
   eBUTTON_STATE_DOWN,
   eBUTTON_STATE_DISABLED
   }
   eBUTTON_STATE;


typedef struct tagSwitchButton
   {
   eBUTTON_STATE  state;
   nsuint      ID;
   const nschar    *image;
   }
   SwitchButton;


typedef struct tagMainControlbox
   {
   HWND          hWnd;
   SwitchButton  buttonObjects[ _mMAIN_CTRLBOX_NUM_BUTTONS ];
   HWND          buttonHandles[ _mMAIN_CTRLBOX_NUM_BUTTONS ];
   //HWND          sliders[ _mMAIN_CTRLBOX_NUM_THRESHOLDS ];
   //HWND          edits[ _mMAIN_CTRLBOX_NUM_THRESHOLDS ];
   //HWND          upDowns[ _mMAIN_CTRLBOX_NUM_THRESHOLDS ];
   }
   MainControlbox;


typedef struct tagMainWindow
   {
   HWND            hWnd;
   HWorkspace      activeWorkspace;
   MainDisplay     display;
   MainControlbox  ctrlbox;
   nsuint        width;
   nsuint        height;
   nsint             mouseMode;
   nsint   view;
   nschar            path[ NS_PATH_SIZE ];
   nschar            filter[ NS_PATH_SIZE ];
   OpenGLWindow    ogl;
   OpenGLLight     light0;
   nsint             oglActive;
   //nsint             model_mode;
   nsint             octree_mode;
   //nsint             spine_mode;
   nsint             lightingOn;
   //nsint             transparencyOn;
   nsint             usePalette3D;
   nsint             polygonsSolid;
   nsint             showAxis;
   nsint             showBoundingCube;
   nsint             show_octree_bounding_box;
   nsint             show_flow_sums;
   nsint             show_external_maxima;
   nsint             show_internal_maxima;
   nsint             useTexturing;
   nsint             complexity;
   nsint             complexityMenuID;
   nsint             line_width;
   //nsint             pen_thickness;
	const nschar     *ogl_text;
   }
   MainWindow;





nsint ____pen_thickness_id = IDM_VIEW_PEN_THICKNESS_1;

//nsint ____show_radii = 0;


static nsint s_DisplayTypeToButton[3] = 
   {
   _mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON,
   _mMAIN_CTRLBOX_SIDE_VIEW_BUTTON,
   _mMAIN_CTRLBOX_TOP_VIEW_BUTTON
   };


static nsint s_MouseModeToButton[8] = 
   {
   _mMAIN_CTRLBOX_ZOOM_BUTTON,
   _mMAIN_CTRLBOX_DRAG_BUTTON,
   _mMAIN_CTRLBOX_SEED_BUTTON,
   _mMAIN_CTRLBOX_RAYS_BUTTON,
   _mMAIN_CTRLBOX_SPINE_BUTTON,
   _mMAIN_CTRLBOX_NEURITE_BUTTON,
	_mMAIN_CTRLBOX_ROI_BUTTON,
	MAIN_CTRLBOX_MAGNET_BUTTON
   };


static MainWindow s_MainWindow;


void ____redraw_main_buttons( void )
   {
   nssize i;

   for( i = 0; i < _mMAIN_CTRLBOX_NUM_BUTTONS; ++i )
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[i], NULL, FALSE );
   }



//nsint *____model_mode = &s_MainWindow.model_mode;
//nsint *____spine_mode = &s_MainWindow.spine_mode;
HWND *____the_main_window = &s_MainWindow.hWnd;
nsuint *____active_workspace = &s_MainWindow.activeWorkspace;
OpenGLWindow *____ogl = &s_MainWindow.ogl;
nsint *____line_width = &s_MainWindow.line_width;
nsint *____polygon_complexity = &s_MainWindow.complexity;
nschar *____volume_file_filter = s_MainWindow.filter;


nsint _workspace_get_volume_length( void )
	{
	nsint length = 0;

	if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
		length = ( nsint )ns_image_length( workspace_dataset( s_MainWindow.activeWorkspace ) );

	return length;		
	}



void _set_status_bar_file( const nschar *path, nsboolean exists )
   {
   const nschar *ptr = ns_ascii_strrchr( path, NS_UNICHAR_PATH );

   if( NULL != ptr && exists )
		{
ns_println( "path = %s\n", path );
      SetStatusBarText( ptr + 1, STATUS_BAR_FILE );
		}
   else
      SetStatusBarText( "", STATUS_BAR_FILE );
   }



nschar ____window_title_buffer[ NS_PATH_SIZE + 128 ];

void _set_window_title( HWND wnd, const nschar *path )
   {
   ns_snprint(
      ____window_title_buffer,
      NS_ARRAY_LENGTH( ____window_title_buffer ),
      NS_FMT_STRING " - NeuronStudio",
      path
      );

   SetWindowText( wnd, ____window_title_buffer );
   }


void _do_workspace_add_image_extra_info( const nschar *title, const nschar *info )
   {
   if( NULL != info )
      {
      NsList      *extra_info;
      nslistiter   curr, end;
      nschar      *s1, *s2;
      nssize       n;
      nsboolean    found;


      extra_info = workspace_image_extra_info( s_MainWindow.activeWorkspace );

      /* TEMP on the 512 hard-coded size????? */
      s1 = ns_malloc( 512 );
      s2 = ns_malloc( 512 );

      if( NULL != s1 && NULL != s2 )
         {
         ns_snprint( s2, 512, "%s%s", title, ns_ascii_strntab( s1, info, 512, 3 ) );

         /* Check if the list has the 'title' already. */
         n     = ns_ascii_strlen( title );
         found = NS_FALSE;
         curr  = ns_image_begin_strings( extra_info );
         end   = ns_image_end_strings( extra_info );

         for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
            if( ns_ascii_strneq( ns_list_iter_get_object( curr ), title, n ) )
               {
               found = NS_TRUE;
               break;
               }
               

         if( ! found )
            ns_image_add_string( extra_info, s2 );
         }

      ns_free( s1 );
      ns_free( s2 );
      }
   }


void _do_workspace_set_lsm_info( const TiffLsmInfo *lsm_info )
	{  workspace_set_lsm_info( s_MainWindow.activeWorkspace, lsm_info );  }


nsboolean ____redraw_2d_need_render = NS_TRUE;

void ____redraw_2d( void )
   {
   nssize i;

	____redraw_2d_need_render = NS_TRUE;

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
		if( NULL != s_MainWindow.display.windows[ i ] )
        InvalidateRect( s_MainWindow.display.windows[ i ], NULL, FALSE );
   }


void ____redraw_3d( void )
   {
   if( NULL != s_MainWindow.ogl.hGLRC )
      RepaintOpenGLWindow( &s_MainWindow.ogl );
   }


void ____redraw_all( void )
   {
   ____redraw_2d();
   ____redraw_3d();
   }


void _run_neurite_tracer( const NsVector3f *V1, const NsVector3f *V2 )
   {
   workspace_run_neurite_tracer( s_MainWindow.hWnd, s_MainWindow.activeWorkspace, V1, V2 );
   }


void _EnableMainControlbox( nsint enabled )
   {
   //nsint i;

   if( ! enabled )
      {
      s_MainWindow.ctrlbox.buttonObjects[s_DisplayTypeToButton[ s_MainWindow.view ]].state = eBUTTON_STATE_UP;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_DisplayTypeToButton[ s_MainWindow.view ]], NULL, FALSE );

      if( eDISPLAY_WINDOW_NO_MOUSE_MODE != s_MainWindow.mouseMode )
         {
         s_MainWindow.ctrlbox.buttonObjects[s_MainWindow.mouseMode].state = eBUTTON_STATE_UP;
         InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_MainWindow.mouseMode], NULL, FALSE );
         }

      //SendMessage( s_MainWindow.ctrlbox.edits[ _mMAIN_CTRLBOX_LOW_EDIT ], WM_SETTEXT, 0, ( LPARAM )"" );
      //SendMessage( s_MainWindow.ctrlbox.sliders[ _mMAIN_CTRLBOX_LOW_SLIDER ], TBM_SETPOS, TRUE, 255 );
      }
   //else
     // {
      //SendMessage( s_MainWindow.ctrlbox.edits[ _mMAIN_CTRLBOX_LOW_EDIT ], WM_SETTEXT, 0, ( LPARAM )"0" );
     // }

   //for( i = 0; i < _mMAIN_CTRLBOX_NUM_THRESHOLDS; ++i )
     // {
      //EnableWindow( s_MainWindow.ctrlbox.sliders[ i ], enabled );
      //EnableWindow( s_MainWindow.ctrlbox.edits[ i ], enabled );
     // }
   }


/*
void _SaveMainDisplaySize( void )
   {
   RECT client;

   GetClientRect( s_MainWindow.display.windows[0], &client );
   s_MainWindow.display.width  = client.right - client.left;
   s_MainWindow.display.height = client.bottom - client.top;
   }
*/


void _MainWindowOnFitImages( void )
   {
   nsuint i;

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      SendMessage( s_MainWindow.display.windows[ i ], mMSG_USER_FitImage, 0, 0 );
   }


void _make_file_filter( const nschar *path, nssize prefix_length, nschar *filter )
   {
	nssize         i;
	const nschar  *src;
	nschar        *sep, *dest;
   nschar         temp;
   nschar         buffer[ _MAX_FNAME + 1 ];


   src  = FindFileNameInPath( path, '\\' );
	sep  = ( nschar* )src;
	dest = buffer;

	/* Copy all characters of the prefix. */
	for( i = 0; i < prefix_length; ++i )
		*dest++ = *src++;

	/* Set a '0' character and skip over any digits. */
	*dest++ = '0';
	while( ns_ascii_isdigit( *src ) )
		++src;

	/* Copy any remaining characters. */
	while( *src )
		*dest++ = *src++;

	*dest = '\0';

   temp = *sep;
   *sep = '\0';

	ns_sprint( filter, "%s%s", path, buffer );

   *sep = temp;

	/* Check for special characters that the module std/nsconfigdb.c cant handle. */
	for( dest = filter; '\0' != *dest; ++dest )
		if( '[' == *dest || ']' == *dest )
			*dest = '_';

	ns_println( "The SETTINGS FILE entry will be named %s", filter );
   }


void _read_settings( void );
//void _read_neurites_settings( void );
//void _read_spines_settings( void );


void _MainWindowOnSetMouseMode( nsint how, nsint id, nsint mode, nsint notifyCode );


#include <image/nsimagedir.h>


//const nschar* ____extensions[] = { ".tif", ".tiff", ".raw" };

nsint ____detect_sequence( NsImageDir *dir )
   {
   const nschar *file_name;
	nsboolean any_problems;
      nschar msg[512];


   file_name = ns_ascii_strrchr( s_MainWindow.path, '\\' );

   if( NULL == file_name )
      file_name = s_MainWindow.path;
   else
      ++file_name;

   ns_image_dir_create( dir, file_name, NS_FALSE, NS_TRUE, &any_problems/*, ____extensions, NS_ARRAY_LENGTH( ____extensions )*/ );

	if( any_problems )
		{
      ns_snprint(
         msg,
         NS_ARRAY_LENGTH( msg ),
         "Unable to determine the proper sequence of files.\n"
         "Please consider moving or renaming the files."
         );

		if( NULL != s_MainWindow.hWnd )
			MessageBox( s_MainWindow.hWnd, msg, "NeuronStudio", MB_OK | MB_ICONWARNING );
		}

   if( 1 < ns_image_dir_size( dir ) )
      {
      ns_snprint(
         msg,
         NS_ARRAY_LENGTH( msg ),
         "Series detected.\n"
         "\nWould you like to open ALL files " NS_FMT_STRING " ... " NS_FMT_STRING
         "\ninstead of just file " NS_FMT_STRING "?",
         ns_image_dir_first_file( dir ),
         ns_image_dir_last_file( dir ),
         file_name
         );

      return NULL != s_MainWindow.hWnd ?
					MessageBox( s_MainWindow.hWnd, msg, "NeuronStudio", MB_YESNOCANCEL | MB_ICONQUESTION ) : IDYES;
      }

   /*curr = ns_image_dir_begin( &dir );
   end  = ns_image_dir_end( &dir );

   for( ; ns_image_dir_iter_not_equal( curr, end ); curr = ns_image_dir_iter_next( curr ) )
      ns_println( NS_FMT_STRING, ns_image_dir_iter_file( curr ) );
*/

   return IDNO;
   }


#include "settings.inl"


#include "describeraw.inl"

NS_PRIVATE int _describe_raw_dialog( nsboolean allow_length )
   {
   ____raw_allow_length = allow_length;
   return RawOptionsDialog( s_MainWindow.hWnd, s_MainWindow.path );
   }



void ____projection_type_do_check( nsboolean do_check )
   {
   nsint ID = 0;

   switch( ____projection_type )
      {
      case NS_PIXEL_PROC_ORTHO_PROJECT_MAX:
         ID = IDM_VIEW_PROJECTION_MAX;
         break;

      case NS_PIXEL_PROC_ORTHO_PROJECT_MIN:
         ID = IDM_VIEW_PROJECTION_MIN;
         break;

      case NS_PIXEL_PROC_ORTHO_PROJECT_SUM:
         ID = IDM_VIEW_PROJECTION_SUM;
         break;

      default:
         ns_assert_not_reached();
      }

   SetMenuItemChecked( s_MainWindow.hWnd, ID, do_check );
   }


/*
void ____on_run_sampling( nsboolean do_2d )
   {
   ____use_2d_neurite_sampling = do_2d;

   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SAMPLING_2D, ____use_2d_neurite_sampling );
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SAMPLING_3D, ! ____use_2d_neurite_sampling );
   }
*/

/*
void ____on_run_measurement_sampling( nsboolean do_2d )
   {
   ____use_2d_measurement_sampling = do_2d;

   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MEAS_SAMPLING_2D, ____use_2d_measurement_sampling );
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MEAS_SAMPLING_3D, ! ____use_2d_measurement_sampling );
   }
*/


void ____on_run_spine_bounding_box( nsboolean do_2d )
   {
   ____use_2d_spine_bounding_box = do_2d;

   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SPINE_BOUNDING_BOX_2D, ____use_2d_spine_bounding_box );
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SPINE_BOUNDING_BOX_3D, ! ____use_2d_spine_bounding_box );
   }


void ____on_measurement_do_3d_rayburst( nsboolean do_3d )
   {
   ____measurement_do_3d_rayburst = do_3d;

   SetMenuItemChecked( s_MainWindow.hWnd, IDM_MEASUREMENT_RAYBURST_2D, ! ____measurement_do_3d_rayburst );
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_MEASUREMENT_RAYBURST_3D, ____measurement_do_3d_rayburst );
   }



void _set_display_window_workspace_handle( void )
   {
   nssize i;

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      SendMessage( s_MainWindow.display.windows[ i ], mMSG_USER_SetWorkspace, 0,
         s_MainWindow.activeWorkspace/* hWorkspace*/ );
   }


void _fit_display_window_projection_xy( void )
   {
   SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetDisplayType, 1, NS_XY );
   _MainWindowOnFitImages();
   }


void _image_file_remove_extension( nschar *path )
	{
	nschar *ext;

	if( NULL != ( ext = ns_ascii_strrchr( path, '.' ) ) )
		{
		if( ns_ascii_strieq( ext, ".tif" ) ||
			 ns_ascii_strieq( ext, ".tiff" ) ||
			 ns_ascii_strieq( ext, ".raw" ) )
			*ext = NS_ASCII_NULL;
		}
	}


void _neurites_file_remove_extension( void )
	{
	nschar *ext;

	if( NULL != ( ext = ns_ascii_strrchr( ____neurites_file_path, '.' ) ) )
		{
		if( ns_ascii_strieq( ext, ".swc" ) ||
			 ns_ascii_strieq( ext, ".hoc" ) )
			{
			ns_ascii_strncpy( ____neurites_file_extension, ext, sizeof( ____neurites_file_extension ) );
			*ext = NS_ASCII_NULL;
			}
		}
	}


void _neurites_file_replace_extension( void )
	{  ns_ascii_strcat( ____neurites_file_path, ____neurites_file_extension );  }


void _spines_file_remove_extension( void )
	{
	nschar *ext;

	if( NULL != ( ext = ns_ascii_strrchr( ____spines_file_path, '.' ) ) )
		{
		if( ns_ascii_strieq( ext, ".txt" ) )
			{
			ns_ascii_strncpy( ____spines_file_extension, ext, sizeof( ____spines_file_extension ) );
			*ext = NS_ASCII_NULL;
			}
		}
	}


void _spines_file_replace_extension( void )
	{  ns_ascii_strcat( ____spines_file_path, ____spines_file_extension );  }


void _neurites_file_do_init( nsboolean set_default_name )
   {
	if( set_default_name )
		{
		ns_ascii_strncpy( ____neurites_file_path, s_MainWindow.path, sizeof( ____neurites_file_path ) );
		_image_file_remove_extension( ____neurites_file_path );

		ns_ascii_strncpy( ____spines_file_path, s_MainWindow.path, sizeof( ____spines_file_path ) );
		_image_file_remove_extension( ____spines_file_path );		
		}
	else
		{
		____neurites_file_path[0] = '\0';
		____spines_file_path[0]   = '\0';
		}

   ____neurites_file_exists  = NS_FALSE;
   ____neurites_file_type    = 0;

   ns_memzero( &____neurites_file_swc_header, sizeof( NsSwcHeader ) );

   _set_status_bar_file( ____neurites_file_path, ____neurites_file_exists );

	____neurites_file_out_edge_info = NS_FALSE;
   }


nsuint get_pen_thickness( nsint id )
	{
	nsuint thickness = 1;

	switch( id )
		{
      case IDM_VIEW_PEN_THICKNESS_1: thickness = 1; break;
      case IDM_VIEW_PEN_THICKNESS_2: thickness = 2; break;
      case IDM_VIEW_PEN_THICKNESS_3: thickness = 3; break;
      case IDM_VIEW_PEN_THICKNESS_4: thickness = 4; break;
      case IDM_VIEW_PEN_THICKNESS_5: thickness = 5; break;
      case IDM_VIEW_PEN_THICKNESS_6: thickness = 6; break;
      case IDM_VIEW_PEN_THICKNESS_7: thickness = 7; break;
      case IDM_VIEW_PEN_THICKNESS_8: thickness = 8; break;
		}

	return thickness;
	}


void _set_voxel_sizes_by_lsm_info( void )
	{
	NsSettings         *settings;
	const TiffLsmInfo  *lsm_info;
	NsVoxelInfo        *voxel_info;


	settings   = workspace_settings( s_MainWindow.activeWorkspace );
	voxel_info = ns_settings_voxel_info( settings );
	lsm_info   = workspace_get_lsm_info( s_MainWindow.activeWorkspace );

	ns_voxel_info(
		voxel_info,
		( nsfloat )lsm_info->voxel_size_x,
		( nsfloat )lsm_info->voxel_size_y,
		( nsfloat )lsm_info->voxel_size_z
		);
	}


#include "dirdlg.h"
#include "saveimage.inl"

extern nsuint* ____pen_thickness;

void _on_file_open( void )
   {
   static const nschar*  filter =
		"All Supported Files (*.tif;*.tiff;*.lsm;*.raw)\0*.tif;*.tiff;*.lsm;*.raw\0"
		"TIFF (*.tif;*.tiff)\0*.tif;*.tiff\0"
		"Zeiss LSM (*.lsm)\0*.lsm\0"
		"RAW (*.raw)\0*.raw\0"
		"All Files (*.*)\0*.*\0"
		"\0\0";
   nsint          wasCancelled;
   nsint          error;
   WorkspaceArgs  inArgs;
   //nsuint       i;
   NsImageDir     dir;
   nsint          file_type;
	nssize         prefix_length;
   //HWorkspace   hWorkspace = 0;


   if( FileOpenNameDialog( s_MainWindow.hWnd, NULL, s_MainWindow.path, NS_PATH_SIZE, filter, NULL, NULL ) )
      {

      _read_jitter_from_config_file();

      
      //____neurites_vertex_render_mode = IDM_RENDER_MODE_VERTEX_SOLID_ELLIPSES;
      //____neurites_edge_render_mode = IDM_RENDER_MODE_EDGE_LINES;
      //____projection_type = NS_PIXEL_PROC_ORTHO_PROJECT_MAX;
      //____selected_num_cpu = IDM_RUN_NUM_CPU_1;
      //____view_measurement_text = NS_TRUE;
      //____use_2d_neurite_sampling = NS_FALSE;
      //____use_2d_spine_bounding_box = NS_FALSE;
      //____use_antialiasing = NS_FALSE;
      //____gamma_correction = NS_FALSE;
      //____vertex_color_scheme = IDM_COLOR_SCHEME_VERTEX_BY_TYPE;
      //____edge_color_scheme = IDM_COLOR_SCHEME_EDGE_SINGLE_COLOR;

      //____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_VIZ;
      //____measuring_rays_count = IDM_MEASURING_RAYS_COUNT_NORMAL;

      //____measurement_interp_type = NS_RAYBURST_INTERP_BILINEAR;
      //____measurement_interpolation = IDM_MEASURING_BILINEAR;
      //____use_2d_measurement_sampling = NS_FALSE;

      //____neurites_interp_type = NS_RAYBURST_INTERP_BILINEAR;
      //____neurites_interpolation = IDM_NEURITES_BILINEAR;
      //____spines_interp_type = NS_RAYBURST_INTERP_BILINEAR;
      //____spines_interpolation = IDM_SPINES_BILINEAR;
      


      ns_image_dir_construct( &dir );

      switch( ____detect_sequence( &dir ) )
         {
         case IDNO:
            inArgs.use_path = NS_TRUE;
            break;

         case IDYES:
            inArgs.use_path = NS_FALSE;
            break;

         case IDCANCEL:
            return;

         default:
            ns_assert_not_reached();
         }

      if( eIMAGE_FILE_RAW == ( file_type = GetImageFileType( s_MainWindow.path ) ) )
         {
         if( inArgs.use_path ) /* 3D? */
            {
            if( ! _describe_raw_dialog( NS_TRUE ) )
               return;
            }
         else
            {
            if( ! _describe_raw_dialog( NS_FALSE ) )
               return;
            }
         }

      inArgs.path   = s_MainWindow.path;
      inArgs.window = s_MainWindow.display.windows[ 0 ]; /* Doesnt matter which one. */
      //inArgs.do_sequence = do_sequence;
      inArgs.dir = &dir;


      ns_log_entry(
         NS_LOG_ENTRY_FUNCTION,
         NS_FUNCTION
         "( path=" NS_FMT_STRING_DOUBLE_QUOTED
         ", file_type=" NS_FMT_INT
         ", is_sequence=" NS_FMT_INT
         " )",
         s_MainWindow.path,
         file_type,
         ! inArgs.use_path
         );

      s_MainWindow.activeWorkspace = 0;

      error = ProgressDialog( s_MainWindow.hWnd,
                              NULL/*"Running Maximal Projections..."*/,
                              BuildWorkspace,
                              &inArgs,
                              &s_MainWindow.activeWorkspace,//&hWorkspace,
                              &wasCancelled
                            );

		prefix_length = ns_image_dir_prefix_length( &dir );
      ns_image_dir_destruct( &dir );
       
      if( ! wasCancelled )
         {
         if( eNO_ERROR != error )
            {
            MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ error ], "NeuronStudio", MB_OK | MB_ICONERROR );
            g_Error = eNO_ERROR;
            }
         else
            {
            nsboolean b;

            //s_MainWindow.activeWorkspace = hWorkspace;

            _set_display_window_workspace_handle();

            _neurites_file_do_init( NS_TRUE );

            _EnableMainControlbox( 1 );

            s_MainWindow.mouseMode = eDISPLAY_WINDOW_MOUSE_MODE_DRAG;//eDISPLAY_WINDOW_NO_MOUSE_MODE;

            s_MainWindow.view = NS_XY;
 
            //TEMP Fit the views for all the windows.

            b = ____2d_display_center_and_limit_image;
            ____2d_display_center_and_limit_image = NS_TRUE;

            //if( NS_XY != s_MainWindow.view )
               {
               SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetDisplayType, 1, NS_ZY );
               _MainWindowOnFitImages();
               }
            //if( NS_ZY != s_MainWindow.view )
               {
               SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetDisplayType, 1, NS_XZ );
               _MainWindowOnFitImages();
               }
            //if( NS_XZ != s_MainWindow.view )
               {
               SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetDisplayType, 1, NS_XY );
               _MainWindowOnFitImages();
               }
            //SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetDisplayType, 0, NS_XY/*s_MainWindow.view*/ );
            //_MainWindowOnFitImages();

            ____2d_display_center_and_limit_image = b;
            

            s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON].state = eBUTTON_STATE_DOWN;
            InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON], NULL, FALSE );

            EnableMenuItems( s_MainWindow.hWnd, 1 );
            SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN, 0 );
            SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN_3D, 0 );
            //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_SAVE_TREE, 0 );
            //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_RUN_UPDATE_TREE, 0 );
            //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_BRANCH_STATS, 0 );


				SetMenuItemEnabled( s_MainWindow.hWnd, IDM_VIEW_SPINE_MODE_POINTS, ____visualize_spine_voxels );


            SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_SAVE_ALL_MODEL, 0 );

            SetMenuItemEnabled(
               s_MainWindow.hWnd,
               IDM_RUN_FILTER_BRIGHTNESS_CONTRAST,
               NS_PIXEL_LUM_U8 == ns_image_pixel_type( workspace_volume( s_MainWindow.activeWorkspace ) )
               );


            ____xy_slice_enabled = NS_FALSE;
            ____xy_slice_index   = 0;
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_XY_SLICE_VIEWER, ____xy_slice_enabled );


            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_PROJECTION_MAX, 1 );
            ____projection_type_do_check( NS_TRUE );


            SetMenuItemChecked( s_MainWindow.hWnd, ____selected_num_cpu/*IDM_RUN_NUM_CPU_1*/, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_MEASUREMENT_TEXT, ____view_measurement_text/*1*/ );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_MEASUREMENT_THRESHOLD_SAMPLES, ____measurement_view_thresholding );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_PROJECTED_2D_GRAFTING, ____projected_2d_grafting );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MODEL_FILTERING, ____run_model_filtering );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_VECTORS, ____view_spine_vectors );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_ATTACHMENTS, ____view_spine_attachments );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_LAYERS, ____spines_color_by_layer );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_VOXELS_DTS, ____view_spine_voxels_dts );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_NON_SPINE_VOXELS, ____view_non_spine_voxels );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_2D_POLYGON_BORDERS, ____view_2d_polygon_borders );

				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_IDS, ____view_spine_ids );

				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_EDGE_ORDER_NUMBERS, ____view_edge_order_numbers );
				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_EDGE_SECTION_NUMBERS, ____view_edge_section_numbers );

				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SHOLL_ANALYSIS, ____view_sholl_analysis );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_SPINE_ROUNDED_CORNERS, ____spines_use_round_corners );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_MAXIMA_VOXELS, ____view_spine_maxima_voxels );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_CANDIDATE_VOXELS, ____view_spine_candidate_voxels );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_SPINE_MSH_ZONE, ____view_maximum_spine_height );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_MEASUREMENT_SURFACE_MESH, ____measurement_view_surface_mesh );


            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_CENTER_AND_LIMIT_IMAGE, ____2d_display_center_and_limit_image );   

            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_USE_SEED_AS_3D_PIVOT, ____use_seed_as_3d_pivot );

				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SEED_ALWAYS_VISIBLE, ____view_seed_always_on_top );
				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_ROI_ALWAYS_VISIBLE, ____view_roi_always_on_top );

				SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_PROJECTION_AXES, ____show_2d_axes );

//            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MARKERS, ____markers_active );


            /* TEMP!!!!!!!!!!!!!!!! */
            //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_COLOR_SCHEME_EDGE_BY_ORDER, NS_FALSE );
            //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_COLOR_SCHEME_EDGE_BY_SECTION, NS_FALSE );
   


            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SAMPLING_3D, 1 );
            //____on_run_sampling( ____use_2d_neurite_sampling );

            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_SPINE_BOUNDING_BOX_3D, 1 );
            ____on_run_spine_bounding_box( ____use_2d_spine_bounding_box );

            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MEAS_SAMPLING_3D, 1 );
            //____on_run_measurement_sampling( ____use_2d_measurement_sampling );

            ____on_measurement_do_3d_rayburst( ____measurement_do_3d_rayburst );


            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_FORWARD, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_IMAGE, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_LOW, 1 );
            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_TREE_VERTICES, 1 );
            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_TREE_EDGES, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_PALETTE_STANDARD, 1 );
            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_FLOW_SIMULATION, 1 );
            //SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINES, 1 );

            //s_MainWindow.model_mode = IDM_VIEW_MODEL_MODE_LINES_AND_SPHERES;

            //SetMenuItemChecked( s_MainWindow.hWnd, s_MainWindow.model_mode, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_vertex_render_mode, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_edge_render_mode, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____measuring_rays_count, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, ____spines_rays_count, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____measurement_interpolation, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_interpolation, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, ____spines_interpolation, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_FLICKER_FREE, ____flicker_free_rendering );

            SetMenuItemChecked( s_MainWindow.hWnd, ____spines_render_mode, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, ____spines_color_scheme, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____vertex_color_scheme, 1 );
            SetMenuItemChecked( s_MainWindow.hWnd, ____edge_color_scheme, 1 );

            SetMenuItemChecked( s_MainWindow.hWnd, ____pen_thickness_id/*IDM_VIEW_PEN_THICKNESS_1*/, 1 );
            //SendMessage( s_MainWindow.display.windows[0], mMSG_USER_SetPenThickness, 0, 1 );
				*____pen_thickness = get_pen_thickness( ____pen_thickness_id );

            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_NEURITES_BUILD, ____draw_while_grafting );
            SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_JUNCTION_VERTICES_ONLY,
               ____draw_junction_vertices_only );

            _set_window_title( s_MainWindow.hWnd, s_MainWindow.path );

            _make_file_filter( s_MainWindow.path, prefix_length, s_MainWindow.filter );

            _read_settings();
            //_read_neurites_settings();
            //_read_spines_settings();

				_MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, 0 );

				s_VI_LastDir[0]    = '\0';
				s_VI_LastPrefix[0] = '\0';

				if( workspace_is_lsm_image( s_MainWindow.activeWorkspace ) )
					_set_voxel_sizes_by_lsm_info();
      
            }/* else( no error on creation ) */
         }
      else
         {
         /* Its possible the workspace was created if the cancel button was
            pressed too late. */

         if( WorkspaceIsValid( s_MainWindow.activeWorkspace/*hWorkspace*/ ) )
            DestructWorkspace( s_MainWindow.activeWorkspace/*hWorkspace*/ );
         }  

      ____redraw_main_buttons();

      }/* if( got file path ) */
   }


//void _on_file_open_3d( void )
//   {  _on_file_open( NS_FALSE, "Open" );  }


//void _MainWindowOnFileOpen( void )
  // {  _on_file_open( NS_TRUE, "Open Series" );  }



void _status_clear_measurement( void )
   {
   SetStatusBarText( "", STATUS_BAR_LENGTH );
   SetStatusBarText( "", STATUS_BAR_WIDTH );
   SetStatusBarText( "", STATUS_BAR_VOLUME );
   SetStatusBarText( "", STATUS_BAR_SURFACE_AREA );
   }


void _status_measurement( void )
   {
   NsSampler *sampler;
   nspointer sample;
   nschar buffer[128];


   sampler = workspace_sampler( s_MainWindow.activeWorkspace );

   if( ! ns_sampler_is_empty( sampler ) )
      {
      sample = ns_sampler_last( sampler );

      /*
      ns_snprint(
         buffer,
         NS_ARRAY_LENGTH( buffer ),
         "L: %.3f   W: %.3f   V: %.3f   SA: %.3f",
         ns_sample_get_length( sample ),
         ns_sample_get_radius( sample ) * 2.0f,
         ns_sample_get_volume( sample ),
         ns_sample_get_surface_area( sample )
         );

      SetStatusBarText( buffer, STATUS_BAR_INFO );
      */

      ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "L: %.3f", ns_sample_get_length( sample ) );
      SetStatusBarText( buffer, STATUS_BAR_LENGTH );

      ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "W: %.3f", ns_sample_get_radius( sample ) * 2.0f );
      SetStatusBarText( buffer, STATUS_BAR_WIDTH );

      ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "V: %.3f", ns_sample_get_volume( sample ) );
      SetStatusBarText( buffer, STATUS_BAR_VOLUME );

      ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), "SA: %.3f", ns_sample_get_surface_area( sample ) );
      SetStatusBarText( buffer, STATUS_BAR_SURFACE_AREA );
      }
   else
      _status_clear_measurement();
   }



extern nsint ____ogl_curr_mouse_mode;
extern nsboolean ____ogl_drag_rect_active;


nsint _mouse_mode_to_ogl_menu_id( nsint mode )
	{
	nsint id = -1; /* i.e. no menu item */

	switch( mode )
		{
		//case eDISPLAY_WINDOW_MOUSE_MODE_ZOOM:    id = IDM_GL_MODE_ZOOM;      break;
		case eDISPLAY_WINDOW_MOUSE_MODE_DRAG:    id = IDM_GL_MODE_TRANSLATE; break;
		case eDISPLAY_WINDOW_MOUSE_MODE_SEED:    id = IDM_GL_MODE_SEED;      break;
		case eDISPLAY_WINDOW_MOUSE_MODE_RAYS:    id = IDM_GL_MODE_RAYS;      break;
		case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:   id = IDM_GL_MODE_SPINE;     break;
		case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE: id = IDM_GL_MODE_NEURITE;   break;
		case eDISPLAY_WINDOW_MOUSE_MODE_ROI:     id = IDM_GL_MODE_ROI;       break;
		//case DISPLAY_WINDOW_MOUSE_MODE_MAGNET:   id = IDM_GL_MODE_MAGNET;    break;
		}

	return id;
	}


void _SetDisplayWindowsMouseMode
   (
   const nsint  newMode,
   const nsint  byMenu,
   const nsint  menuID
   )
   {
   nsuint  i;
   nsint   oldMenuID;
	nsint   ogl_menu_id;


   if( eDISPLAY_WINDOW_NO_MOUSE_MODE != s_MainWindow.mouseMode )
      {
      switch( s_MainWindow.mouseMode )
         {
         case eDISPLAY_WINDOW_MOUSE_MODE_ZOOM:    oldMenuID = IDM_FILE_MODE_ZOOM;      break;
         case eDISPLAY_WINDOW_MOUSE_MODE_DRAG:    oldMenuID = IDM_FILE_MODE_TRANSLATE; break;
         case eDISPLAY_WINDOW_MOUSE_MODE_SEED:    oldMenuID = IDM_MODE_SEED;           break;
         case eDISPLAY_WINDOW_MOUSE_MODE_RAYS:    oldMenuID = IDM_MODE_RAYS;           break;
         case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:   oldMenuID = IDM_MODE_SPINE;          break;
         case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE: oldMenuID = IDM_MODE_NEURITE;        break;
         case eDISPLAY_WINDOW_MOUSE_MODE_ROI:     oldMenuID = IDM_MODE_ROI;            break;
         case DISPLAY_WINDOW_MOUSE_MODE_MAGNET:   oldMenuID = IDM_MODE_MAGNET;         break;
         }

      SetMenuItemChecked( s_MainWindow.hWnd, oldMenuID, 0 );

      s_MainWindow.ctrlbox.buttonObjects[s_MainWindow.mouseMode].state = eBUTTON_STATE_UP;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_MainWindow.mouseMode], NULL, FALSE );

		if( NULL != s_MainWindow.ogl.hGLRC )
			if( -1 != ( ogl_menu_id = _mouse_mode_to_ogl_menu_id( ( nsint )s_MainWindow.mouseMode ) ) )
				SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ogl_menu_id, 0 );
      }

   if( byMenu && eDISPLAY_WINDOW_NO_MOUSE_MODE != newMode )
      {
      s_MainWindow.ctrlbox.buttonObjects[newMode].state = eBUTTON_STATE_DOWN;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[newMode], NULL, FALSE );
      }

	SetMenuItemChecked( s_MainWindow.hWnd, menuID, eDISPLAY_WINDOW_NO_MOUSE_MODE != newMode );

	if( NULL != s_MainWindow.ogl.hGLRC )
		if( -1 != ( ogl_menu_id = _mouse_mode_to_ogl_menu_id( newMode ) ) )
			SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ogl_menu_id, eDISPLAY_WINDOW_NO_MOUSE_MODE != newMode );

   // for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
   //   InvalidateRect( s_MainWindow.display.windows[i ], NULL, FALSE );

   s_MainWindow.mouseMode = newMode;

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      SendMessage( s_MainWindow.display.windows[i], mMSG_USER_SetMouseMode, 0, newMode );

	if( NULL != s_MainWindow.ogl.hGLRC )
		SendMessage( s_MainWindow.ogl.hWnd, mMSG_USER_SetMouseMode, 0, newMode );

	____ogl_curr_mouse_mode = newMode; /* Assure that its set, since the 3D window may not be active. */

   if( IDM_MODE_SEED == oldMenuID                 ||
		 IDM_MODE_ROI  == oldMenuID                 ||
		 eDISPLAY_WINDOW_MOUSE_MODE_SEED == newMode ||
		 eDISPLAY_WINDOW_MOUSE_MODE_ROI  == newMode   )
      ____redraw_all();
   }



void _MainWindowOnFileClose( void )
   {
   nsint i;


   ns_log_entry( NS_LOG_ENTRY_FUNCTION, NS_FUNCTION "()" );

   if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
      DestructWorkspace( s_MainWindow.activeWorkspace );

   ____xy_slice_enabled = NS_FALSE;

   s_MainWindow.activeWorkspace = 0;

   _EnableMainControlbox( 0 );

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      {
     _SetDisplayWindowsMouseMode( eDISPLAY_WINDOW_NO_MOUSE_MODE, 0, 0 );
      SendMessage( s_MainWindow.display.windows[ i ], mMSG_USER_SetWorkspace, 0, 0 );
      }

   if( s_MainWindow.oglActive )
      {
      s_MainWindow.oglActive = 0;
      DestroyOpenGLWindow( &s_MainWindow.ogl );
      }

   SetMenuItemsChecked( s_MainWindow.hWnd, 0 );
   EnableMenuItems( s_MainWindow.hWnd, 0 );

   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN, 1 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN_3D, 1 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_ABOUT, 1 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_CONTENTS, 1 );
   //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_TOPICS, 1 );
   //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_ABOUT, 1 );

   SetWindowText( s_MainWindow.hWnd, "NeuronStudio" );

   _status_clear_measurement();
   _neurites_file_do_init( NS_FALSE );

   SetStatusBarText( "", STATUS_BAR_SLICE );
   SetStatusBarText( "", STATUS_BAR_MSG );

____redraw_main_buttons();
   }


void _ResizeMainDisplay( void )
   {
   RECT rc;


//   _SaveMainDisplaySize();


//MoveWindow( s_MainWindow.display.windows[ 0 ],
  //          _mMAIN_DISPLAY_BORDER_SIZE,
    //        _mMAIN_DISPLAY_BORDER_SIZE,
      //      s_MainWindow.display.width - _mMAIN_DISPLAY_BORDER_SIZE*2,
        //    s_MainWindow.display.height - _mMAIN_DISPLAY_BORDER_SIZE*2,
          //  FALSE
//          );
            

   rc.bottom = rc.top = 0;
   GetWindowRect( s_hStatusBar, &rc );

         MoveWindow( s_MainWindow.display.windows[0],
                     0,
                     0,
                     s_MainWindow.width - _mMAIN_WINDOW_RIGHT_MARGIN_WIDTH,
                     s_MainWindow.height - ( rc.bottom - rc.top ),
                     TRUE
                   );


   //____redraw_2d();

   //InvalidateRect( s_MainWindow.display.windows[0], NULL, FALSE );
   }


void _TileMainDisplay( void )
   {
//   _SaveMainDisplaySize();

   _ResizeMainDisplay();
   }



void _MainDisplayOnCommand( const nsint ID )
   {
   if( mMSG_USER_Update == ID )
      ____redraw_all();
   }


LRESULT CALLBACK _MainControlboxProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


SwitchButton* _MapSwitchButtonHandleToObject( const HWND hWnd )
   {
   nsint i;
   
   for( i = 0; i < _mMAIN_CTRLBOX_NUM_BUTTONS/*mDISPLAY_WINDOW_NUM_MOUSE_MODES*/; ++i )
      if( hWnd == s_MainWindow.ctrlbox.buttonHandles[ i ] )
         return &( s_MainWindow.ctrlbox.buttonObjects[ i ] );

   return NULL;
   }


LRESULT CALLBACK _SwitchButtonProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   SwitchButton *button = _MapSwitchButtonHandleToObject( hWnd );

   if( NULL == button )
      return DefWindowProc( hWnd, uMsg, wParam, lParam );

   switch( uMsg )
      {
      case WM_PAINT:
         {
         PAINTSTRUCT ps;
         HDC hDC;
         RECT rc;
         HCURSOR hCursor;
         nsint     color;


         hDC = BeginPaint( hWnd, &ps );

         GetClientRect( hWnd, &rc );
         
         switch( button->state )
            {
            case eBUTTON_STATE_UP       : color = COLOR_3DFACE;    break; 
            case eBUTTON_STATE_DOWN     : color = COLOR_3DHILIGHT; break;
            }

         FillRect( hDC, &rc, ( HBRUSH )( color + 1 ) );

         hCursor = LoadCursor( g_Instance, button->image );

         if( 0 != s_MainWindow.activeWorkspace )
            DrawIcon( hDC, ( eBUTTON_STATE_UP == button->state )?0:1,
                        ( eBUTTON_STATE_UP == button->state )?0:1, hCursor );
         else
          DrawState(
            hDC, NULL, NULL, ( LPARAM )hCursor, 0, 0, 0, 0, 0, DST_ICON | DSS_DISABLED );


         DrawEdge( hDC, &rc, ( eBUTTON_STATE_UP == button->state )? EDGE_RAISED : EDGE_SUNKEN,
                   BF_RECT );

         EndPaint( hWnd, &ps );
         }
         break;

      case WM_LBUTTONDOWN:
         //SetFocus( hWnd );
         if( 0 != s_MainWindow.activeWorkspace )
            {
            if( eBUTTON_STATE_UP == button->state )
               button->state = eBUTTON_STATE_DOWN;
            else
               button->state = eBUTTON_STATE_UP;
            
            InvalidateRect( hWnd, NULL, FALSE );
            SendMessage( GetParent( hWnd ), WM_COMMAND, MAKEWPARAM( button->ID, button->state ), ( LPARAM )hWnd );
            }
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;
   }


eERROR_TYPE _CreateMainControlbox( nsuint ownerWidth, nsuint ownerHeight )
   {
   WNDCLASSEX wcx;

   wcx.cbSize        = sizeof( WNDCLASSEX );
   wcx.style         = CS_HREDRAW | CS_VREDRAW;
   wcx.lpfnWndProc   = _MainControlboxProcedure;
   wcx.cbClsExtra    = 0;
   wcx.cbWndExtra    = 0;
   wcx.hIcon         = NULL;
   wcx.hCursor       = LoadCursor( NULL, IDC_ARROW );
   wcx.hInstance     = g_Instance;
   wcx.hbrBackground = ( HBRUSH )( COLOR_BTNFACE + 1 );//NULL;
   wcx.lpszClassName = "MainControlbox";
   wcx.lpszMenuName  = NULL;
   wcx.hIconSm       = NULL;

   if( 0 == RegisterClassEx( &wcx ) )
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.hWnd = CreateWindowEx
        ( 0,
          "MainControlbox",
          NULL,
          WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
          _mMAIN_CTRLBOX_X( ownerWidth ),
          _mMAIN_CTRLBOX_Y,
          _mMAIN_CTRLBOX_WIDTH,
          _mMAIN_CTRLBOX_HEIGHT,
          s_MainWindow.hWnd,
          NULL, g_Instance, NULL
         );

   if( NULL == s_MainWindow.ctrlbox.hWnd )
       return ( g_Error = eERROR_OPER_SYSTEM );

   wcx.cbSize        = sizeof( WNDCLASSEX );
   wcx.style         = CS_HREDRAW | CS_VREDRAW;
   wcx.lpfnWndProc   = _SwitchButtonProcedure;
   wcx.cbClsExtra    = 0;
   wcx.cbWndExtra    = 0;
   wcx.hIcon         = NULL;
   wcx.hCursor       = LoadCursor( NULL, IDC_ARROW );
   wcx.hInstance     = g_Instance;
   wcx.hbrBackground = NULL;
   wcx.lpszClassName = "SwitchButton";
   wcx.lpszMenuName  = NULL;
   wcx.hIconSm       = NULL;

   if( 0 == RegisterClassEx( &wcx ) )
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ZOOM_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ZOOM_BUTTON].ID = _mMAIN_CTRLBOX_ZOOM_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ZOOM_BUTTON].image = "ZOOM_IN_TOOL_CURSOR";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_DRAG_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_DRAG_BUTTON].ID = _mMAIN_CTRLBOX_DRAG_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_DRAG_BUTTON].image = "DRAG_TOOL_CURSOR";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON].ID = _mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON].image = "CUR_FORWARD";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON].ID = _mMAIN_CTRLBOX_SIDE_VIEW_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON].image = "CUR_SIDE";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_TOP_VIEW_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_TOP_VIEW_BUTTON].ID = _mMAIN_CTRLBOX_TOP_VIEW_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_TOP_VIEW_BUTTON].image = "CUR_TOP";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SEED_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SEED_BUTTON].ID = _mMAIN_CTRLBOX_SEED_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SEED_BUTTON].image = "SEED_TOOL_CURSOR";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_RAYS_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_RAYS_BUTTON].ID = _mMAIN_CTRLBOX_RAYS_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_RAYS_BUTTON].image = "RAY_TOOL_CURSOR";


   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SPINE_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SPINE_BUTTON].ID = _mMAIN_CTRLBOX_SPINE_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_SPINE_BUTTON].image = "SPINE_TOOL_CURSOR";


   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_NEURITE_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_NEURITE_BUTTON].ID = _mMAIN_CTRLBOX_NEURITE_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_NEURITE_BUTTON].image = "NEURITE_TOOL_CURSOR";

   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ROI_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ROI_BUTTON].ID = _mMAIN_CTRLBOX_ROI_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[_mMAIN_CTRLBOX_ROI_BUTTON].image = "ROI_TOOL_CURSOR";


   s_MainWindow.ctrlbox.buttonObjects[MAIN_CTRLBOX_MAGNET_BUTTON].state = eBUTTON_STATE_UP;
   s_MainWindow.ctrlbox.buttonObjects[MAIN_CTRLBOX_MAGNET_BUTTON].ID = MAIN_CTRLBOX_MAGNET_BUTTON_ID;
   s_MainWindow.ctrlbox.buttonObjects[MAIN_CTRLBOX_MAGNET_BUTTON].image = "MAGNET_TOOL_CURSOR";


   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_ZOOM_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     /*6,6,24,16*/6, 0, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_ZOOM_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_ZOOM_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );
                     
   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_DRAG_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     /*6,22,24,16*/6, 24, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_DRAG_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_DRAG_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SEED_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                   6, 48, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_SEED_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SEED_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_RAYS_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                   6, 72, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_RAYS_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_RAYS_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );


   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SPINE_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                   6, 96, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_SPINE_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SPINE_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );


   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_NEURITE_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                   6, 120, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_NEURITE_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_NEURITE_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );


   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_ROI_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     6, 144, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_ROI_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_ROI_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );


   s_MainWindow.ctrlbox.buttonHandles[MAIN_CTRLBOX_MAGNET_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     6, 168, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )MAIN_CTRLBOX_MAGNET_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[MAIN_CTRLBOX_MAGNET_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );


   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     6, 202, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     6, 226, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_SIDE_VIEW_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_TOP_VIEW_BUTTON ]=
     CreateWindowEx( 0, "SwitchButton", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                     6, 250, 24, 24, s_MainWindow.ctrlbox.hWnd,
                     ( HMENU )_mMAIN_CTRLBOX_TOP_VIEW_BUTTON_ID, g_Instance, NULL );
   if( NULL == s_MainWindow.ctrlbox.buttonHandles[_mMAIN_CTRLBOX_TOP_VIEW_BUTTON ])
      return ( g_Error = eERROR_OPER_SYSTEM );

   return eNO_ERROR;
   }


void _MainWindowOnGetMinMaxInfo
   (
   SIZE *const    minWindowSize,
   SIZE *const    maxWindowSize
   )
   {  MakeSize( minWindowSize, _mMAIN_WINDOW_MINIMUM_WIDTH, _mMAIN_WINDOW_MINIMUM_HEIGHT );  }


void _SetDisplayWindowsView( const nsint view, const nsint byMenu, const nsint menuID )
   {
   nsuint  i;
   nsint       oldMenuID;


   if( s_MainWindow.view == view )
      {
      s_MainWindow.ctrlbox.buttonObjects[s_DisplayTypeToButton[ view ]].state = eBUTTON_STATE_DOWN;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_DisplayTypeToButton[ view ]], NULL, FALSE );
      return;
      }

   s_MainWindow.ctrlbox.buttonObjects[s_DisplayTypeToButton[ s_MainWindow.view ]].state = eBUTTON_STATE_UP;
   InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_DisplayTypeToButton[ s_MainWindow.view ]], NULL, FALSE );

   if( byMenu )
      {
      s_MainWindow.ctrlbox.buttonObjects[s_DisplayTypeToButton[ view ]].state = eBUTTON_STATE_DOWN;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_DisplayTypeToButton[ view ]], NULL, FALSE );
      }

   switch( s_MainWindow.view )
      {
      case NS_XY : oldMenuID = IDM_VIEW_FORWARD; break;
      case NS_ZY    : oldMenuID = IDM_VIEW_SIDE;    break;
      case NS_XZ     : oldMenuID = IDM_VIEW_TOP;     break;
      }

   SetMenuItemChecked( s_MainWindow.hWnd, oldMenuID, 0 );

   s_MainWindow.view = view;

   SetMenuItemChecked( s_MainWindow.hWnd, menuID, 1 );
 
   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      SendMessage( s_MainWindow.display.windows[i], mMSG_USER_SetDisplayType, 0, view );
   }


void _MainWindowOnChangeView(  const nsuint displayFlag, const nsint menuID )
   {
//   nsuint i;

   ToggleWorkspaceDisplayFlag( s_MainWindow.activeWorkspace, displayFlag );

  // RedrawWorkspaceDisplayGraphics( s_MainWindow.activeWorkspace );

   ____redraw_all();
   
   SetMenuItemChecked( s_MainWindow.hWnd,
                       menuID,
                       WorkspaceDisplayFlagIsSet( s_MainWindow.activeWorkspace, displayFlag )
                     );
   }


LRESULT CALLBACK _MainControlboxProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
     {
/*
     case WM_PAINT:
        {
        PAINTSTRUCT ps;
        HDC hDC;
        RECT  rc;

  
        hDC = BeginPaint( hWnd, &ps );

        GetClientRect( hWnd, &rc );
        FillRect( hDC, &rc, ( HBRUSH )( COLOR_3DFACE + 1 ) );
    
        EndPaint( hWnd, &ps );
        }
        break;
*/


     case WM_COMMAND:
        SendMessage( GetParent( hWnd ), uMsg, wParam, lParam );
        break;


     default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
     } 

   return 0;
   }


void _MainWindowOnChangeViewPalette( const eWORKSPACE_PALETTE_TYPE paletteType, const nsint menuID )
   {
//   nsuint                 i;
   eWORKSPACE_PALETTE_TYPE  oldPalette;
   nsint                      oldMenuID;


   oldPalette = workspace_get_palette_type( s_MainWindow.activeWorkspace );

   if( oldPalette != paletteType )
      {
      switch( oldPalette )
         {
         case eWORKSPACE_PALETTE_STANDARD    : oldMenuID = IDM_VIEW_PALETTE_STANDARD;    break;
         //case eWORKSPACE_PALETTE_SOLAR_FLARE : oldMenuID = IDM_VIEW_PALETTE_SOLAR_FLARE; break;
         //case eWORKSPACE_PALETTE_INTENSITY   : oldMenuID = IDM_VIEW_PALETTE_INTENSITY;   break;
         case eWORKSPACE_PALETTE_INVERSE     : oldMenuID = IDM_VIEW_PALETTE_INVERSE;     break;
         //case eWORKSPACE_PALETTE_COLDFIRE    : oldMenuID = IDM_VIEW_PALETTE_COLDFIRE;    break;
         case eWORKSPACE_PALETTE_TOPOGRAPHIC : oldMenuID = IDM_VIEW_PALETTE_TOPOGRAPHIC; break;
         case eWORKSPACE_PALETTE_GAMMA_CORRECTION_1_7 : oldMenuID = IDM_VIEW_PALETTE_GAMMA_CORRECTION_1_7; break;
         case eWORKSPACE_PALETTE_GAMMA_CORRECTION_2_2 : oldMenuID = IDM_VIEW_PALETTE_GAMMA_CORRECTION_2_2; break;

			case NS_PALETTE_INTENSITY_MASK:
				oldMenuID = IDM_VIEW_PALETTE_INTENSITY_MASK;
				break;

         case NS_PALETTE_RED:
				oldMenuID = IDM_VIEW_PALETTE_RED;
				break;

         case NS_PALETTE_GREEN:
				oldMenuID = IDM_VIEW_PALETTE_GREEN;
				break;

         case NS_PALETTE_BLUE:
				oldMenuID = IDM_VIEW_PALETTE_BLUE;
				break;

         case NS_PALETTE_YELLOW:
				oldMenuID = IDM_VIEW_PALETTE_YELLOW;
				break;
         }

      SetMenuItemChecked( s_MainWindow.hWnd, oldMenuID, 0 );
      SetMenuItemChecked( s_MainWindow.hWnd, menuID, 1 );      

      workspace_set_palette_type( s_MainWindow.activeWorkspace, paletteType );
      RedrawWorkspaceDisplayGraphics( s_MainWindow.activeWorkspace );

      ____redraw_all();
      }
   }


void _on_run_filter( eERROR_TYPE ( *func )( HWND, const HWorkspace ) )
   {
   //nsuint i;
   eERROR_TYPE error;


	EnableWindow( s_MainWindow.ogl.hWnd, FALSE );
   error = ( func )( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
	EnableWindow( s_MainWindow.ogl.hWnd, TRUE );

   if( eNO_ERROR != error )
      {
      MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );
      g_Error = eNO_ERROR;
      }

   ____redraw_all();

   //SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_ALL );
   }


void _on_run_filter_dynamic_range( void )
   {  _on_run_filter( image_processing_dynamic_range );  }

void _on_run_filter_blur( void )
   {  _on_run_filter( image_processing_blur );  }

void _on_run_filter_blur_more( void )
   {  _on_run_filter( image_processing_blur_more );  }

void _on_run_filter_brightness_contrast( void )
   {  _on_run_filter( image_processing_brightness_contrast );  }

void _on_run_filter_gamma_correction( void )
   {  _on_run_filter( image_processing_gamma_correct );  }

void _on_run_filter_resize( void )
   {  _on_run_filter( image_processing_resize );  }

void _on_run_filter_noise( void )
   {  _on_run_filter( image_processing_noise );  }

void _on_run_filter_flip_vertical( void )
   {  _on_run_filter( image_processing_flip_vertical );  }

void _on_run_filter_flip_horizontal( void )
   {  _on_run_filter( image_processing_flip_horizontal );  }

void _on_run_filter_flip_optical_axis( void )
   {  _on_run_filter( image_processing_flip_optical_axis );  }

void _on_run_filter_subsample( void )
   {  _on_run_filter( image_processing_subsample );  }


nssize ____get_num_cpu( void )
   {
   nssize num_cpu = 1;

   switch( ____selected_num_cpu )
      {
      case IDM_RUN_NUM_CPU_1: num_cpu = 1; break;
      case IDM_RUN_NUM_CPU_2: num_cpu = 2; break;
      case IDM_RUN_NUM_CPU_4: num_cpu = 4; break;
      case IDM_RUN_NUM_CPU_8: num_cpu = 8; break;

      default:
         ns_assert_not_reached();
      }

   return num_cpu;
   }


void _on_run_filter_mp( nsint type )
   {
   //nsuint i;
   eERROR_TYPE error;

	EnableWindow( s_MainWindow.ogl.hWnd, FALSE );
   error = image_processing_mp_filter( s_MainWindow.hWnd, s_MainWindow.activeWorkspace, type, ____get_num_cpu() );
	EnableWindow( s_MainWindow.ogl.hWnd, TRUE );

   if( eNO_ERROR != error )
      {
      MessageBox( s_MainWindow.hWnd, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );
      g_Error = eNO_ERROR;
      }

   ____redraw_all();

   //SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_ALL );
   }


void _write_settings( void )
   {
   ns_settings_write(
      ____settings_file,
      s_MainWindow.filter,
      workspace_settings( s_MainWindow.activeWorkspace )
      );
   }


void _write_settings_ex( const NsSettings *settings )
   {  ns_settings_write( ____settings_file, s_MainWindow.filter, settings );  }


void _read_settings( void )
   {
   NsSettings  *settings;
   nsboolean    did_read;


   settings = workspace_settings( s_MainWindow.activeWorkspace );

   /* Try to read the settings first from the "database".
      If they're not there then apply default settings. Note
      that the defaults may not be set yet either, but thats
      OK, the settings will remain as the program defaults. */

   /*error*/ns_settings_read( ____settings_file, s_MainWindow.filter, settings, &did_read );

   if( ! did_read )
		{
		ns_settings_init_with_user_defaults( settings );
		//ns_settings_read( ____config_file, "settings", settings, &did_read );

		//if( ! did_read )
		//	{
			/* Settings werent read from either the .settings or the .ns file. */
			ns_settings_set_threshold_fixed_value(
				settings,
				ns_voxel_intensity_convert(
					ns_settings_get_threshold_fixed_value( settings ),
					NS_PIXEL_LUM_U8,
					workspace_pixel_type( s_MainWindow.activeWorkspace )
					)
				);
		 //}
		}

   //RedrawWorkspaceDisplayGraphics( s_MainWindow.activeWorkspace );
   ____redraw_all();
   }


void _reset_interactor( nsint which )
   {
   NsVector3f  D;
   nsfloat     s, c;
	RECT        rc;

	#define _TAN_RADS  0.52f

   workspace_get_image_dimensions( s_MainWindow.activeWorkspace, &D );
   InitInteractor( &s_MainWindow.ogl.interactor );

	switch( which )
		{
		case NS_XY:
			s = D.x > D.y ? D.x : D.y;
			c = s / ( nsfloat )ns_tanf( _TAN_RADS );
			break;

		case NS_ZY:
			s = D.z > D.y ? D.z : D.y;
			c = s / ( nsfloat )ns_tanf( _TAN_RADS );

			glPushMatrix();
			glLoadMatrixf( s_MainWindow.ogl.interactor.imatrix );
			glRotatef(
				-90.0f,
				s_MainWindow.ogl.interactor.imatrix[1],
				s_MainWindow.ogl.interactor.imatrix[5],
				s_MainWindow.ogl.interactor.imatrix[9]
				);
			glGetFloatv( GL_MODELVIEW_MATRIX, s_MainWindow.ogl.interactor.imatrix );
			glPopMatrix();
			break;

		case NS_XZ:
			s = D.x > D.z ? D.x : D.z;
			c = s / ( nsfloat )ns_tanf( _TAN_RADS );

			glPushMatrix();
			glLoadMatrixf( s_MainWindow.ogl.interactor.imatrix );
			glRotatef(
				-90.0f,
				s_MainWindow.ogl.interactor.imatrix[0],
				s_MainWindow.ogl.interactor.imatrix[4],
				s_MainWindow.ogl.interactor.imatrix[8]
				);
			glGetFloatv( GL_MODELVIEW_MATRIX, s_MainWindow.ogl.interactor.imatrix );
			glPopMatrix();
			break;

		default:
			ns_assert_not_reached();
		}

   s_MainWindow.ogl.interactor.izpos = c;

	GetClientRect( s_MainWindow.ogl.hWnd, &rc );

	SetInteractorWindowSize( &s_MainWindow.ogl.interactor,
		( nsint )( rc.right - rc.left ), ( nsint )( rc.bottom - rc.top ) );
   }


void _MainWindowOnBuildNeuronTree( void )
   {
   nsfloat             radius, length;
   const NsVoxelInfo  *voxel_info;
   NsSampler          *sampler;
   nspointer           curr_sample;
   const NsSettings   *settings;
   nsboolean           did_create_sample;
   nsboolean           use_2d_sampling;
   nsfloat             aabbox_scalar;
   nsint               min_window;
	nsfloat             threshold;
	nsfloat             contrast;
   NsVector3i          jitter3i;
   NsError             error;
	NsVector3f          P1f, P2f;


   if( 0 == s_MainWindow.activeWorkspace || ____grafting_is_running )
		return;

	____grafting_is_running = NS_TRUE;

	EnableWindow( s_MainWindow.ogl.hWnd, FALSE );

   sampler    = workspace_sampler( s_MainWindow.activeWorkspace );
   voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );
   settings   = workspace_settings( s_MainWindow.activeWorkspace );

   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );
   ns_vector3i( &jitter3i, ____jitter_x, ____jitter_y, ____jitter_z );

	ns_to_voxel_space( ns_settings_neurite_seed( settings ), &P1f, voxel_info );
	P2f = P1f;

//ns_println( "Current avg intensity is %f", workspace_get_average_intensity( s_MainWindow.activeWorkspace ) );

   error =
      ns_sampler_run(
         sampler,
         workspace_volume( s_MainWindow.activeWorkspace ),
         &P1f,
         &P2f,
         0,
         ____measurement_do_3d_rayburst,
         ____measuring_rays_kernel_type,
         ____measurement_interp_type,
         voxel_info,
         workspace_get_average_intensity( s_MainWindow.activeWorkspace ),
         &jitter3i,
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
         ns_assert( 0 < ns_sampler_size( sampler ) );
         curr_sample = ns_sampler_last( sampler );

			threshold = ns_sample_get_threshold( curr_sample );
			contrast  = ns_sample_get_contrast( curr_sample );

         ns_println( "GRAFTING FIRST-THRESHOLD = " NS_FMT_DOUBLE, threshold );
         ns_println( "GRAFTING FIRST-CONTRAST  = " NS_FMT_DOUBLE, contrast );

			ns_sampler_remove( sampler, curr_sample );

			workspace_set_grafting_first_threshold( s_MainWindow.activeWorkspace, threshold );
			workspace_set_grafting_first_contrast( s_MainWindow.activeWorkspace, contrast );

			if( eNO_ERROR != BuildWorkspaceNeuronTree( s_MainWindow.hWnd, s_MainWindow.activeWorkspace ) )
				{
				ns_println( NS_FMT_STRING, g_ErrorStrings[ g_Error ] ); g_Error = eNO_ERROR;
				goto _RUN_GRAFTING_EXIT;
				}
			else
				_write_settings();
         }
		else
			{
			ns_println( "not enough contrast to obtain sample to begin grafting" );
			goto _RUN_GRAFTING_EXIT;
			}
      }
	else
		{
      ns_println( "not enough memory to complete sample to begin grafting" );
		goto _RUN_GRAFTING_EXIT;
		}

	_RUN_GRAFTING_EXIT:

	EnableWindow( s_MainWindow.ogl.hWnd, TRUE );

   ____redraw_all();

	____grafting_is_running = NS_FALSE;
   }


extern nsboolean ____spine_analysis_running;


void _main_window_on_run_spine_analysis( void )
   {
//   nsuint i;

   if( 0 == s_MainWindow.activeWorkspace )
      return;

   //if( ! WorkspaceHasNeuronTree( s_MainWindow.activeWorkspace ) )
   //   return;

//if( NULL != s_MainWindow.ogl.hGLRC )
//   open_gl_begin_timer_by_message( &s_MainWindow.ogl );

   if( ! ____spine_analysis_running )
      {
		EnableWindow( s_MainWindow.ogl.hWnd, FALSE );
      workspace_run_spine_analysis( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
		EnableWindow( s_MainWindow.ogl.hWnd, TRUE );

      ____redraw_all();
      _write_settings();
      }
   }



static const nschar *s_ColorItemNames[ _WORKSPACE_NUM_COLORS ] =
   {
   "Background",
	"Soma Objects",
	"Basal Dendrites",
	"Apical Dendrites",
	"Axon Objects",
   "Line Vertices",
   "Junction Vertices",
   "External Vertices",
   "Origin Vertices",
   "Vertices",
   "Edges",
   "Splats",
   "Stubby Spines",
   "Thin Spines",
   "Mushroom Spines",
   "Other Spines",
   "Spines"
   };


void _MainWindowOnViewColorsApplyFunc( const nsuint item, const COLORREF color )
   {
   NsVector4ub C;
//   nsuint i;

   //MessageBox( NULL, s_ColorItemNames[item], "", MB_OK );

   if( item < _WORKSPACE_NUM_COLORS )
      {
      ____FROM_COLORREF( color, C );
      workspace_set_color( s_MainWindow.activeWorkspace, item, C );
      }

   ____redraw_all();
   }


struct _workspace_colors
   {
   COLORREF colors[ _WORKSPACE_NUM_COLORS ];
   };


void _MainWindowOnViewColors( void )
   {
   NsVector4ub C;

//   nsuint  i;
   struct _workspace_colors  _save;
   struct _workspace_colors  _colors;
   struct _workspace_colors  _default;

   _default.colors[ WORKSPACE_COLOR_BACKGROUND ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_BACKGROUND );
	_default.colors[ WORKSPACE_COLOR_FUNCTION_SOMA ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_FUNCTION_SOMA );
	_default.colors[ WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_FUNCTION_BASAL_DENDRITE );
	_default.colors[ WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_FUNCTION_APICAL_DENDRITE );
	_default.colors[ WORKSPACE_COLOR_FUNCTION_AXON ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_FUNCTION_AXON );
   _default.colors[ WORKSPACE_COLOR_LINE_VERTEX ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_LINE_VERTEX );
   _default.colors[ WORKSPACE_COLOR_JUNCTION_VERTEX ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_JUNCTION_VERTEX );
   _default.colors[ WORKSPACE_COLOR_EXTERNAL_VERTEX ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_EXTERNAL_VERTEX );
   _default.colors[ WORKSPACE_COLOR_ORIGIN_VERTEX ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_ORIGIN_VERTEX );
   _default.colors[ WORKSPACE_COLOR_VERTEX ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_VERTEX );
   _default.colors[ WORKSPACE_COLOR_EDGE ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_EDGE );
   _default.colors[ WORKSPACE_COLOR_SPLAT ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPLAT );
   _default.colors[ WORKSPACE_COLOR_SPINE_OTHER ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPINE_OTHER );
   _default.colors[ WORKSPACE_COLOR_SPINE_THIN ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPINE_THIN );
   _default.colors[ WORKSPACE_COLOR_SPINE_MUSHROOM ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPINE_MUSHROOM );
   _default.colors[ WORKSPACE_COLOR_SPINE_STUBBY ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPINE_STUBBY );
   _default.colors[ WORKSPACE_COLOR_SPINE ] = ____TO_COLORREF( WORKSPACE_DEFAULT_COLOR_SPINE );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_BACKGROUND );
   _colors.colors[ WORKSPACE_COLOR_BACKGROUND ] =
      ____TO_COLORREF( C );

	C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_SOMA );
	_colors.colors[ WORKSPACE_COLOR_FUNCTION_SOMA ] = ____TO_COLORREF( C );

	C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE );
	_colors.colors[ WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ] = ____TO_COLORREF( C );

	C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE );
	_colors.colors[ WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ] = ____TO_COLORREF( C );

	C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_AXON );
	_colors.colors[ WORKSPACE_COLOR_FUNCTION_AXON ] = ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_LINE_VERTEX ); 
   _colors.colors[ WORKSPACE_COLOR_LINE_VERTEX ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_JUNCTION_VERTEX );
   _colors.colors[ WORKSPACE_COLOR_JUNCTION_VERTEX ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_EXTERNAL_VERTEX );
   _colors.colors[ WORKSPACE_COLOR_EXTERNAL_VERTEX ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_ORIGIN_VERTEX );
   _colors.colors[ WORKSPACE_COLOR_ORIGIN_VERTEX ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_VERTEX );
   _colors.colors[ WORKSPACE_COLOR_VERTEX ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_EDGE );
   _colors.colors[ WORKSPACE_COLOR_EDGE ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPLAT );
   _colors.colors[ WORKSPACE_COLOR_SPLAT ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPINE_OTHER );
   _colors.colors[ WORKSPACE_COLOR_SPINE_OTHER ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPINE_THIN );
   _colors.colors[ WORKSPACE_COLOR_SPINE_THIN ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPINE_MUSHROOM );
   _colors.colors[ WORKSPACE_COLOR_SPINE_MUSHROOM ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPINE_STUBBY );
   _colors.colors[ WORKSPACE_COLOR_SPINE_STUBBY ] =
      ____TO_COLORREF( C );

   C = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPINE );
   _colors.colors[ WORKSPACE_COLOR_SPINE ] =
      ____TO_COLORREF( C );


   _save = _colors;

   if( ! ChooseColorsDialog( g_Instance,
                             s_MainWindow.hWnd,
                             _WORKSPACE_NUM_COLORS,
                             ( const nschar** )&s_ColorItemNames,
                             _colors.colors,
                             _default.colors,
                             0,
                             _MainWindowOnViewColorsApplyFunc
                           ) )
      _colors = _save;

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_BACKGROUND ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_BACKGROUND, C );

	____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_FUNCTION_SOMA ], C );
	workspace_set_color( s_MainWindow.activeWorkspace,
		WORKSPACE_COLOR_FUNCTION_SOMA, C );

	____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ], C );
	workspace_set_color( s_MainWindow.activeWorkspace,
		WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE, C );

	____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ], C );
	workspace_set_color( s_MainWindow.activeWorkspace,
		WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE, C );

	____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_FUNCTION_AXON ], C );
	workspace_set_color( s_MainWindow.activeWorkspace,
		WORKSPACE_COLOR_FUNCTION_AXON, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_LINE_VERTEX ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_LINE_VERTEX, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_JUNCTION_VERTEX ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_JUNCTION_VERTEX, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_EXTERNAL_VERTEX ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_EXTERNAL_VERTEX, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_ORIGIN_VERTEX ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_ORIGIN_VERTEX, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_VERTEX ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_VERTEX, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_EDGE ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_EDGE, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPLAT ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPLAT, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPINE_OTHER ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPINE_OTHER, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPINE_THIN ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPINE_THIN, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPINE_MUSHROOM ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPINE_MUSHROOM, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPINE_STUBBY ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPINE_STUBBY, C );

   ____FROM_COLORREF( _colors.colors[ WORKSPACE_COLOR_SPINE ], C );
   workspace_set_color( s_MainWindow.activeWorkspace,
      WORKSPACE_COLOR_SPINE, C );


  // RedrawWorkspaceDisplayGraphics( s_MainWindow.activeWorkspace );

   ____redraw_all();
   }


void _MainWindowOnView3DModel( void );



NsSwcHeader *____swc_header;
nsboolean ____swc_out_edge_info;

INT_PTR CALLBACK _SWCDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         {
         SYSTEMTIME  st;

         CenterWindow( hDlg, s_MainWindow.hWnd );

         SendMessage( GetDlgItem( hDlg, IDE_SWC_ORIGINAL_SOURCE ), EM_LIMITTEXT, sizeof( ____swc_header->source )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_CREATURE ), EM_LIMITTEXT, sizeof( ____swc_header->creature )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_REGION ), EM_LIMITTEXT, sizeof( ____swc_header->region )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_FIELDLAYER ), EM_LIMITTEXT, sizeof( ____swc_header->field )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_TYPE ), EM_LIMITTEXT, sizeof( ____swc_header->type )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_CONTRIBUTOR ), EM_LIMITTEXT, sizeof( ____swc_header->contributor )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_REFERENCE ), EM_LIMITTEXT, sizeof( ____swc_header->reference )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_RAW ), EM_LIMITTEXT, sizeof( ____swc_header->raw )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_EXTRAS ), EM_LIMITTEXT, sizeof( ____swc_header->extras )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_SOMA_AREA ), EM_LIMITTEXT, sizeof( ____swc_header->soma_area )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_SHRINKAGE_CORRECTION ), EM_LIMITTEXT, sizeof( ____swc_header->shrinkage )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_VERSION_NUMBER ), EM_LIMITTEXT, sizeof( ____swc_header->version_number )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_VERSION_DATE ), EM_LIMITTEXT, sizeof( ____swc_header->version_date )-1, 0 );
         SendMessage( GetDlgItem( hDlg, IDE_SWC_SCALE ), EM_LIMITTEXT, sizeof( ____swc_header->scale )-1, 0 );

         if( 0 == ns_ascii_strlen( ____swc_header->source ) )
            ns_sprint( ____swc_header->source, "NeuronStudio %d.%d.%d",
               NS_VERSION_MAJOR, NS_VERSION_MINOR, NS_VERSION_MICRO );

         SetDlgItemText( hDlg, IDE_SWC_ORIGINAL_SOURCE, ____swc_header->source );

         SetDlgItemText( hDlg, IDE_SWC_CREATURE, ____swc_header->creature );
         SetDlgItemText( hDlg, IDE_SWC_REGION, ____swc_header->region );
         SetDlgItemText( hDlg, IDE_SWC_FIELDLAYER, ____swc_header->field );
         SetDlgItemText( hDlg, IDE_SWC_TYPE, ____swc_header->type );
         SetDlgItemText( hDlg, IDE_SWC_CONTRIBUTOR, ____swc_header->contributor );
         SetDlgItemText( hDlg, IDE_SWC_REFERENCE, ____swc_header->reference );
         SetDlgItemText( hDlg, IDE_SWC_RAW, ____swc_header->raw );
         SetDlgItemText( hDlg, IDE_SWC_EXTRAS, ____swc_header->extras );
         SetDlgItemText( hDlg, IDE_SWC_SOMA_AREA, ____swc_header->soma_area );

         if( 0 == ns_ascii_strlen( ____swc_header->shrinkage ) )
            ns_ascii_strcpy( ____swc_header->shrinkage, "1.0 1.0 1.0" );
         SetDlgItemText( hDlg, IDE_SWC_SHRINKAGE_CORRECTION, ____swc_header->shrinkage );

         if( 0 == ns_ascii_strlen( ____swc_header->version_number ) )
            ns_ascii_strcpy( ____swc_header->version_number, "1.0" );
         SetDlgItemText( hDlg, IDE_SWC_VERSION_NUMBER, ____swc_header->version_number );

         GetSystemTime( &st );
         ns_sprint( ____swc_header->version_date, "%d-%02d-%02d", (nsint)st.wYear, (nsint)st.wMonth, (nsint)st.wDay );
         SetDlgItemText( hDlg, IDE_SWC_VERSION_DATE, ____swc_header->version_date );

         if( 0 == ns_ascii_strlen( ____swc_header->scale ) )
            ns_ascii_strcpy( ____swc_header->scale, "1.0 1.0 1.0" );
         SetDlgItemText( hDlg, IDE_SWC_SCALE, ____swc_header->scale );

         SetFocus( GetDlgItem( hDlg, IDE_SWC_CREATURE ) );
         }
         return FALSE;

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            case IDOK:
               {
               GetDlgItemText( hDlg, IDE_SWC_ORIGINAL_SOURCE, ____swc_header->source, sizeof( ____swc_header->source )-1 );
               GetDlgItemText( hDlg, IDE_SWC_CREATURE, ____swc_header->creature, sizeof( ____swc_header->creature )-1 );
               GetDlgItemText( hDlg, IDE_SWC_REGION, ____swc_header->region, sizeof( ____swc_header->region )-1 );
               GetDlgItemText( hDlg, IDE_SWC_FIELDLAYER, ____swc_header->field, sizeof( ____swc_header->field )-1 );
               GetDlgItemText( hDlg, IDE_SWC_TYPE, ____swc_header->type, sizeof( ____swc_header->type )-1 );
               GetDlgItemText( hDlg, IDE_SWC_CONTRIBUTOR, ____swc_header->contributor, sizeof( ____swc_header->contributor )-1 );
               GetDlgItemText( hDlg, IDE_SWC_REFERENCE, ____swc_header->reference, sizeof( ____swc_header->reference )-1 );
               GetDlgItemText( hDlg, IDE_SWC_RAW, ____swc_header->raw, sizeof( ____swc_header->raw )-1 );
               GetDlgItemText( hDlg, IDE_SWC_EXTRAS, ____swc_header->extras, sizeof( ____swc_header->extras )-1 );
               GetDlgItemText( hDlg, IDE_SWC_SOMA_AREA, ____swc_header->soma_area, sizeof( ____swc_header->soma_area )-1 );
               GetDlgItemText( hDlg, IDE_SWC_SHRINKAGE_CORRECTION, ____swc_header->shrinkage, sizeof( ____swc_header->shrinkage )-1 );
               GetDlgItemText( hDlg, IDE_SWC_VERSION_NUMBER, ____swc_header->version_number, sizeof( ____swc_header->version_number )-1 );
               GetDlgItemText( hDlg, IDE_SWC_VERSION_DATE, ____swc_header->version_date, sizeof( ____swc_header->version_date )-1 );
               GetDlgItemText( hDlg, IDE_SWC_SCALE, ____swc_header->scale, sizeof( ____swc_header->scale )-1 );

					____swc_out_edge_info =
						( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_SWC_OUTPUT_SECTIONS_AND_ORDERS ), BM_GETCHECK, 0, 0 );

               EndDialog( hDlg, 1 );
               }
               break;

            case IDCANCEL:
               EndDialog( hDlg, 0 );
               break;
            }
         return TRUE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
      }

   return FALSE;

   }/* _SWCDlgProc() */


nsboolean _SWCDialog( void )
   {
   nsint retValue = ( nsint )DialogBox( g_Instance, "SWC_DIALOG", s_MainWindow.hWnd, _SWCDlgProc );

   return ( 1 == retValue ) ? TRUE : FALSE;

   }/* _SWCDialog() */


nsboolean _showing_section_colors( void )
	{
	return
		( IDM_RENDER_MODE_VERTEX_OFF != ____neurites_vertex_render_mode
		  &&
		  IDM_COLOR_SCHEME_VERTEX_BY_SECTION == ____vertex_color_scheme )
		||
		( IDM_RENDER_MODE_EDGE_OFF != ____neurites_edge_render_mode
		  &&
		  IDM_COLOR_SCHEME_EDGE_BY_SECTION == ____edge_color_scheme );
	}


enum { _FILE_TYPE_UNKNOWN, _FILE_TYPE_SWC, _FILE_TYPE_HOC };

NsError _on_save_neurites_file
   (
   const nschar       *path_buffer,
   nsint               file_type,
   const NsSwcHeader  *swc_header,
	nsboolean           out_edge_info
   )
   {
	NsModel    *model;
	nsulong     flags;
	nsint       section_type;
	nsboolean   redraw;
   NsError     error;


	error  = ns_no_error();
	redraw = NS_FALSE;
	flags  = 0;
	model  = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

	section_type = ns_model_get_section_type( model );

	if( out_edge_info )
		{
		if( NS_MODEL_SECTION_NONE == section_type )
			{
			/*TEMP*/ns_println( "No sections set. Defaulting to spine sectioning type." );

			ns_model_section( model, NS_MODEL_SPINES_SECTIONING, NULL );
			redraw = NS_TRUE;
			}
		else /*if( ns_model_has_unset_sections( model ) )*/
			{
			/*TEMP*/
			ns_println(
				"Running sectioning type " NS_FMT_STRING ".",
				ns_model_section_type_to_string( section_type )
				);

			ns_model_section( model, section_type, NULL );
			redraw = NS_TRUE;
			}
		}

   switch( file_type )
      {
      case _FILE_TYPE_SWC:
			if( out_edge_info )
				flags |= NS_SWC_OUTPUT_EDGES;

			error = ns_model_write_swc( model, swc_header, path_buffer, flags );
         break;

      case _FILE_TYPE_HOC:
			error = ns_model_write_hoc(
						model,
						workspace_get_voxel_info( s_MainWindow.activeWorkspace ),
						path_buffer,
						NS_HOC_ACCESS_SOMAS /*| NS_HOC_MULTIPLE_FILES*/,
						"soma",
						"dend",
						"invalid",
						"unknown_tree",
						"soma_not_used!!!",
						"basal_tree",
						"apical_tree",
						"axon"
						);
         break;

      default:
         ns_assert_not_reached();
      }

   if( ns_is_error( error ) )
      MessageBox(
         s_MainWindow.hWnd,
         ns_error_code_to_string( error ),
         "NeuronStudio",
         MB_OK | MB_ICONERROR
         );

	if( redraw && _showing_section_colors() )
		{
		/*TEMP*/ns_println( "Sectioning was updated. Redrawing..." );
		____redraw_all();
		}

   return error;
   }


nsboolean _on_save_neurites_as
	(
	nschar       *path_buffer,
	nsint        *file_type,
	NsSwcHeader  *swc_header,
	nsboolean    *out_edge_info
	)
   {
   OPENFILENAME   ofn;
   nschar        *ptr;
   nsboolean      do_save;
	NsModel       *model;
   NsError        error;


   error   = ns_no_error();
   do_save = NS_FALSE;

   ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = s_MainWindow.hWnd;
   ofn.lpstrTitle  = "Save Neurites";
   ofn.lpstrFile   = path_buffer;
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "SWC (*.swc)\0*.swc\0HOC (*.hoc)\0*.hoc\0\0\0";
   ofn.lpstrDefExt = ".swc";
   ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( GetSaveFileName( &ofn ) )
      {
		*out_edge_info = NS_TRUE;

      ptr = ns_ascii_strrchr( ofn.lpstrFile, '.' );

      if( ns_ascii_streq( ptr, ".swc" ) || ns_ascii_streq( ptr, ".SWC" ) )
         {
         *file_type = _FILE_TYPE_SWC;

         ____swc_header        = swc_header;
			____swc_out_edge_info = NS_FALSE;

         if( _SWCDialog() )
            do_save = NS_TRUE;

			if( do_save )
				*out_edge_info = ____swc_out_edge_info;
         }
      else if( ns_ascii_streq( ptr, ".hoc" ) || ns_ascii_streq( ptr, ".HOC" ) )
         {
         *file_type = _FILE_TYPE_HOC;
         do_save    = NS_TRUE;
         }

      if( do_save )
			{
			model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

			if( _FILE_TYPE_HOC == *file_type )
				{
				/*if( ! ns_model_valid_hoc_somas( model ) )
					{
					MessageBox(
						s_MainWindow.hWnd,
						"One or more of the somas of the model is not a valid HOC section.\n"
						"The soma must be a non-branching set of 3D points.\n",
						"NeuronStudio",
						MB_OK | MB_ICONERROR
						);

					return NS_FALSE;
					}*/

				/*if( ! ns_model_valid_hoc_function_labeling( model ) )
					if( IDNO == MessageBox(
							s_MainWindow.hWnd,
							"One or more of the sub-trees of the model has invalid function labeling\n"
							"that will affect the saving of the HOC file.\n"
							"\n"
							"Are you sure you want to proceed?",
							"NeuronStudio",
							MB_YESNO | MB_ICONQUESTION ) )
						return NS_FALSE;*/
				}

         error = _on_save_neurites_file(
                  path_buffer,
                  *file_type,
                  swc_header,
						*out_edge_info
                  );
			}
      }

   return do_save && ( ! ns_is_error( error ) );
   }


void _MainWindowOnFileSaveTree( nsboolean check_for_existing_file )
   {
	NsModel      *model;
   nsboolean     do_save_as;
   NsSwcHeader   swc_header;
   nsint         file_type;
	nsboolean     out_edge_info;
   NsError       error;
   //nschar        path_buffer[ NS_PATH_SIZE ] = { '\0' };


	model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

	if( ! ns_model_are_function_types_complete( model ) )
		if( IDNO == MessageBox(
			s_MainWindow.hWnd,
			"One or more of the vertices and/or edges of the model still has a function\n"
			"type of \"unknown\". The format of the saved file will be affected.\n"
			"\n"
			"Are you sure you want to proceed?",
			"NeuronStudio",
			MB_YESNO | MB_ICONQUESTION ) )
		return;

   do_save_as = NS_TRUE;

   if( check_for_existing_file && ____neurites_file_exists )
      {
      do_save_as = NS_FALSE;

      ns_assert( _FILE_TYPE_UNKNOWN != ____neurites_file_type );

		if( _FILE_TYPE_HOC == ____neurites_file_type )
			{
			/*if( ! ns_model_valid_hoc_somas( model ) )
				{
				MessageBox(
					s_MainWindow.hWnd,
					"One or more of the somas of the model is not a valid HOC section.\n"
					"The soma must be a non-branching set of 3D points.\n",
					"NeuronStudio",
					MB_OK | MB_ICONERROR
					);

				return;
				}*/

			/*if( ! ns_model_valid_hoc_function_labeling( model ) )
				if( IDNO == MessageBox(
						s_MainWindow.hWnd,
						"One or more of the sub-trees of the model has invalid function labeling\n"
						"that will affect the saving of the HOC file.\n"
						"\n"
						"Are you sure you want to proceed?",
						"NeuronStudio",
						MB_YESNO | MB_ICONQUESTION ) )
					return;*/
			}

      if( NS_SUCCESS(
            _on_save_neurites_file(
               ____neurites_file_path,
               ____neurites_file_type,
               &____neurites_file_swc_header,
					____neurites_file_out_edge_info
               ),
            error ) )
         status_bar_print( STATUS_BAR_MSG, "Saved " NS_FMT_STRING, ____neurites_file_path );
      }
   
   if( do_save_as )
      {
      ns_memzero( &swc_header, sizeof( NsSwcHeader ) );

		_neurites_file_remove_extension();

      if( _on_save_neurites_as( ____neurites_file_path, &file_type, &swc_header, &out_edge_info ) )
         {
         //ns_ascii_strcpy( ____neurites_file_path, path_buffer );

         ____neurites_file_exists        = NS_TRUE;
         ____neurites_file_type          = file_type;
         ____neurites_file_swc_header    = swc_header;
			____neurites_file_out_edge_info = out_edge_info;
         }
		else
			_neurites_file_replace_extension();
      }

   _set_status_bar_file( ____neurites_file_path, ____neurites_file_exists );
   }





void _MainWindowOnFileBranchStats()
{
   NsModel *model;
   OPENFILENAME ofn;
   nschar path[NS_PATH_SIZE + 1] = { '\0' };
   nsint rres = 0;
   //FILE *file;

   //if(WorkspaceHasNeuronTree( s_MainWindow.activeWorkspace ) ) 
   //{
      model = GetWorkspaceNeuronTree(s_MainWindow.activeWorkspace );


        ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

      ofn.lStructSize = sizeof( OPENFILENAME );
      ofn.hwndOwner   = s_MainWindow.hWnd;
      ofn.lpstrFile   = path;
      ofn.lpstrTitle  = "Save Labeling";
      ofn.nMaxFile    = NS_PATH_SIZE;
      ofn.lpstrFilter = "Text (*.txt)\0*.txt\0\0\0";
      ofn.lpstrDefExt = ".txt";
      ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;


      if( GetSaveFileName( &ofn ))
      {
         /*file  = fopen( path, "w" );
         if (file == NULL) 
         {
            MessageBox(NULL, "Could not open file. Try again.", "OK.", MB_OK);
            return;
         }*/
         rres = CallTreeStats(model, ofn.lpstrFile);
         if (rres > 0) 
            MessageBox(NULL, "There was an error saving stats to file. Try again.", "NeuronStudio", MB_OK | MB_ICONERROR );
         
         //if( NULL != file )
         //   fclose( file );
      }
   //}
   }


void _get_model_spines_write_flags( nsulong *flags )
   {
   NsConfigDb  db;
   NsError     error;


   #define _GET_SPINES_FLAG( name, value )\
      if( ns_config_db_has_key( &db, "file-spines", name ) )\
         if( ns_config_db_get_boolean( &db, "file-spines", name ) )\
            *flags |= value

   if( NS_SUCCESS( ns_config_db_construct( &db ), error ) )
      {
      if( NS_SUCCESS( ns_config_db_read( &db, ____config_file, NULL ), error ) )
         if( ns_config_db_has_group( &db, "file-spines" ) )
            {
            _GET_SPINES_FLAG( "id", NS_MODEL_SPINES_WRITE_ID );
            _GET_SPINES_FLAG( "section_number", NS_MODEL_SPINES_WRITE_SECTION_NUMBER );
            _GET_SPINES_FLAG( "section_length", NS_MODEL_SPINES_WRITE_SECTION_LENGTH );
            _GET_SPINES_FLAG( "branch_order", NS_MODEL_SPINES_WRITE_BRANCH_ORDER );
            _GET_SPINES_FLAG( "rayburst_volume", NS_MODEL_SPINES_WRITE_RAYBURST_VOLUME );
            _GET_SPINES_FLAG( "voxel_volume", NS_MODEL_SPINES_WRITE_VOXEL_VOLUME );
            _GET_SPINES_FLAG( "layers_volume", NS_MODEL_SPINES_WRITE_LAYERS_VOLUME );
            _GET_SPINES_FLAG( "non_attach_sa", NS_MODEL_SPINES_WRITE_NON_ATTACH_SA );
            _GET_SPINES_FLAG( "attach_sa", NS_MODEL_SPINES_WRITE_ATTACH_SA );
            _GET_SPINES_FLAG( "layers_sa", NS_MODEL_SPINES_WRITE_LAYERS_SA );
            _GET_SPINES_FLAG( "x", NS_MODEL_SPINES_WRITE_X );
            _GET_SPINES_FLAG( "y", NS_MODEL_SPINES_WRITE_Y );
            _GET_SPINES_FLAG( "z", NS_MODEL_SPINES_WRITE_Z );
            _GET_SPINES_FLAG( "head_diameter", NS_MODEL_SPINES_WRITE_HEAD_DIAMETER );
            _GET_SPINES_FLAG( "neck_diameter", NS_MODEL_SPINES_WRITE_NECK_DIAMETER );
				_GET_SPINES_FLAG( "min_dts", NS_MODEL_SPINES_WRITE_MIN_DTS );
            _GET_SPINES_FLAG( "max_dts", NS_MODEL_SPINES_WRITE_MAX_DTS );
            _GET_SPINES_FLAG( "type", NS_MODEL_SPINES_WRITE_TYPE );
            _GET_SPINES_FLAG( "auto", NS_MODEL_SPINES_WRITE_AUTO );
				_GET_SPINES_FLAG( "xyplane_angle", NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE );
				_GET_SPINES_FLAG( "swc_node_id", NS_MODEL_SPINES_WRITE_SWC_NODE_ID );
				_GET_SPINES_FLAG( "swc_node_offset", NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET );
            _GET_SPINES_FLAG( "attach_x", NS_MODEL_SPINES_WRITE_ATTACH_X );
            _GET_SPINES_FLAG( "attach_y", NS_MODEL_SPINES_WRITE_ATTACH_Y );
            _GET_SPINES_FLAG( "attach_z", NS_MODEL_SPINES_WRITE_ATTACH_Z );
				_GET_SPINES_FLAG( "soma_distance", NS_MODEL_SPINES_WRITE_SOMA_DISTANCE );
            _GET_SPINES_FLAG( "head_center_x", NS_MODEL_SPINES_WRITE_HEAD_CENTER_X );
            _GET_SPINES_FLAG( "head_center_y", NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y );
            _GET_SPINES_FLAG( "head_center_z", NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z );
            _GET_SPINES_FLAG( "neck_center_x", NS_MODEL_SPINES_WRITE_NECK_CENTER_X );
            _GET_SPINES_FLAG( "neck_center_y", NS_MODEL_SPINES_WRITE_NECK_CENTER_Y );
            _GET_SPINES_FLAG( "neck_center_z", NS_MODEL_SPINES_WRITE_NECK_CENTER_Z );
            }

      ns_config_db_destruct( &db );
      }
   }


nsboolean _message_box_callback_func( const nschar *message, nspointer user_data )
	{  return IDYES == MessageBox( ( HWND )user_data, message, "NeuronStudio", MB_YESNO | MB_ICONWARNING );  }

void _on_file_spine_stats()
{
   NsModel *model;
   OPENFILENAME ofn;
   //nschar path[NS_PATH_SIZE + 1] = { '\0' };
   nsint rres = 0;
   nsulong flags;
	const nschar *ext;
   NsError error;

   //if(WorkspaceHasNeuronTree( s_MainWindow.activeWorkspace ) ) 
   //{
      model = GetWorkspaceNeuronTree(s_MainWindow.activeWorkspace );


        ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

		_spines_file_remove_extension();

      ofn.lStructSize = sizeof( OPENFILENAME );
      ofn.hwndOwner   = s_MainWindow.hWnd;
      ofn.lpstrFile   = ____spines_file_path;
      ofn.lpstrTitle  = "Save Spines";
      ofn.nMaxFile    = NS_PATH_SIZE;
      ofn.lpstrFilter = "Text (*.txt)\0*.txt\0Comma-Separated Values (*.csv)\0*.csv\0\0\0";
      ofn.lpstrDefExt = ".txt";
      ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;


      if( NS_MODEL_SPINES_SHOW_RESULTS && GetSaveFileName( &ofn ))
         {
         flags = 0;
         _get_model_spines_write_flags( &flags );

			if( ( nsboolean )( NS_MODEL_SPINES_WRITE_SWC_NODE_ID & flags )     ||
				 ( nsboolean )( NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET & flags ) ||
				 ( nsboolean )( NS_MODEL_SPINES_WRITE_SOMA_DISTANCE & flags )         )
				{
				if( ( nsboolean )( NS_MODEL_SPINES_WRITE_SOMA_DISTANCE & flags ) )
					ns_model_set_soma_distances( model );

				workspace_set_swc_attachments(
					s_MainWindow.hWnd,
					s_MainWindow.activeWorkspace,
					( nsboolean )( NS_MODEL_SPINES_WRITE_SOMA_DISTANCE & flags ) 
					);
				}

			if( NULL != ( ext = ns_ascii_strrchr( ____spines_file_path, '.' ) ) )
				{
				if( ns_ascii_strieq( ext, ".txt" ) )
					{
					if( NS_FAILURE(
							ns_model_spines_write_txt(
								model,
								____spines_file_path,//ofn.lpstrFile,
								flags,
								workspace_settings( s_MainWindow.activeWorkspace ),
								_message_box_callback_func,
								s_MainWindow.hWnd,
								NULL
								),
							error ) )
						MessageBox(
							s_MainWindow.hWnd,
							"There was an error saving the spines to file. Try again.",
							"NeuronStudio",
							MB_OK | MB_ICONERROR
							);
					}
				else if( ns_ascii_strieq( ext, ".csv" ) )
					{
					if( NS_FAILURE(
							ns_model_spines_write_csv(
								model,
								____spines_file_path,//ofn.lpstrFile,
								flags,
								workspace_settings( s_MainWindow.activeWorkspace ),
								_message_box_callback_func,
								s_MainWindow.hWnd,
								NULL
								),
							error ) )
						MessageBox(
							s_MainWindow.hWnd,
							"There was an error saving the spines to file. Try again.",
							"NeuronStudio",
							MB_OK | MB_ICONERROR
							);
					}
				}
         }
		else
			_spines_file_replace_extension();
   //}
}



void _MainWindowOnPenThickness( nsint id, nsuint thickness )
   {
   nsuint i;

   SetMenuItemChecked( s_MainWindow.hWnd, ____pen_thickness_id, 0 );

   ____pen_thickness_id = id;
   s_MainWindow.line_width = thickness;
   SetMenuItemChecked( s_MainWindow.hWnd, ____pen_thickness_id, 1 );

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      SendMessage( s_MainWindow.display.windows[ i ], mMSG_USER_SetPenThickness, 0, thickness );

   if( NULL != s_MainWindow.ogl.hGLRC )
      RepaintOpenGLWindow( &s_MainWindow.ogl );
   }


extern void GetWorkspaceNeuronTrees( const HWorkspace hwsp, NsModel **raw, NsModel **filtered );


/*
nsboolean __________cancelled( NsProgress *progress ){ return NS_FALSE; }
void __________update( NsProgress *progress, nsfloat percent ){}
void __________set_title( NsProgress *progress, const nschar *title ){}
void __________set_description( NsProgress *progress, const nschar *description ){}
*/



NS_PRIVATE nsint ____compare_vectors( const NsVector3f *V1, const NsVector3f *V2 )
   {
   if( V1->z < V2->z ) return -1;
   if( V1->z > V2->z ) return  1;
   if( V1->y < V2->y ) return -1;
   if( V1->y > V2->y ) return  1;
   if( V1->x < V2->x ) return -1;
   if( V1->x > V2->x ) return  1;

   return 0;
   }


void ns_model_clean_dup_vertices( NsModel *model )
   {
   nsmodelvertex  curr_vertex1;
   nsmodelvertex  curr_vertex2;
   nsmodelvertex  end_vertices;
   NsVector3f     V1;
   NsVector3f     V2;


   curr_vertex1 = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex1, end_vertices );
          curr_vertex1 = ns_model_vertex_next( curr_vertex1 ) )
      {
      ns_model_vertex_get_position( curr_vertex1, &V1 );

//      _NS_MODEL_CLEAN_DUP_VERTICES_RESTART:

      curr_vertex2 = ns_model_begin_vertices( model );

      for( ; ns_model_vertex_not_equal( curr_vertex2, end_vertices );
             curr_vertex2 = ns_model_vertex_next( curr_vertex2 ) )
         {
         ns_model_vertex_get_position( curr_vertex2, &V2 );

         if( ns_model_vertex_not_equal( curr_vertex1, curr_vertex2 ) &&
             0 == ____compare_vectors( &V1, &V2 ) )
            {
            ns_println( "duplicate vertex detected %f,%f,%f,%f,%u == %f,%f,%f,%f,%u",
V1.x, V1.y, V1.z, ns_model_vertex_get_radius( curr_vertex1 ), ns_model_vertex_num_edges( curr_vertex1 ),
V2.x, V2.y, V2.z, ns_model_vertex_get_radius( curr_vertex2 ), ns_model_vertex_num_edges( curr_vertex2 ) );
//ns_model_merge_vertices( model, curr_vertex1, curr_vertex2 );
            //goto _NS_MODEL_CLEAN_DUP_VERTICES_RESTART;
            }
         }
      }
   }


void ns_model_clean_self_loops( NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  dest_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;


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
         dest_vertex = ns_model_edge_dest_vertex( curr_edge );

         if( ns_model_vertex_equal( curr_vertex, dest_vertex ) )
            ns_println( "self-loop detected" );
         }
      }
   }


void ns_model_clean_dup_edges( NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  dest_vertex1;
   nsmodelvertex  dest_vertex2;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge1;
   nsmodeledge    curr_edge2;
   nsmodeledge    end_edges;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      curr_edge1 = ns_model_vertex_begin_edges( curr_vertex );
      end_edges  = ns_model_vertex_end_edges( curr_vertex );

      for( ; ns_model_edge_not_equal( curr_edge1, end_edges );
             curr_edge1 = ns_model_edge_next( curr_edge1 ) )
         {
         dest_vertex1 = ns_model_edge_dest_vertex( curr_edge1 );

         curr_edge2 = ns_model_vertex_begin_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge2, end_edges );
                curr_edge2 = ns_model_edge_next( curr_edge2 ) )
            {
            dest_vertex2 = ns_model_edge_dest_vertex( curr_edge2 );

            if( ns_model_vertex_equal( dest_vertex1, dest_vertex2 ) &&
                ! ns_model_edge_equal( curr_edge1, curr_edge2 )        )
               ns_println( "duplicate edge detected" );
            }
         }
      }
   }


/*
void _print_model_vertex_thresholds( const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;


   curr_vertex  = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      ns_print( NS_FMT_ULONG " ", ( nsulong )ns_model_vertex_get_threshold( curr_vertex, NS_MODEL_VERTEX_NEURITE_THRESHOLD ) );
   }
*/


NS_PRIVATE nsboolean _read_shrinkage_and_translation_from_config_file( NsVector3f *S, NsVector3f *T )
   {
   NsConfigDb  db;
   NsError     error;
   

   ns_vector3f_zero( S );
   ns_vector3f_zero( T );

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return NS_FALSE;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_group( &db, "shrinkage" ) ||
       ! ns_config_db_has_group( &db, "translate" ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_key( &db, "shrinkage", "x" ) ||
       ! ns_config_db_has_key( &db, "shrinkage", "y" ) ||
       ! ns_config_db_has_key( &db, "shrinkage", "z" ) ||
       ! ns_config_db_has_key( &db, "translate", "x" ) ||
       ! ns_config_db_has_key( &db, "translate", "y" ) ||
       ! ns_config_db_has_key( &db, "translate", "z" )   )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   S->x = ( nsfloat )ns_config_db_get_double( &db, "shrinkage", "x" );
   S->y = ( nsfloat )ns_config_db_get_double( &db, "shrinkage", "y" );
   S->z = ( nsfloat )ns_config_db_get_double( &db, "shrinkage", "z" );
   T->x = ( nsfloat )ns_config_db_get_double( &db, "translate", "x" );
   T->y = ( nsfloat )ns_config_db_get_double( &db, "translate", "y" );
   T->z = ( nsfloat )ns_config_db_get_double( &db, "translate", "z" );

   ns_config_db_destruct( &db );

   return NS_TRUE;
   }


void _import_swc( void )
   {
   static const nschar*  filter = "SWC (*.swc)\0*.swc\0\0\0";
   nschar          path[NS_PATH_SIZE + 1 ] = { '\0' };
   NsSwcHeader header;
   nslong line_num;
//   nssize i;
   //NsVector3f voxel_info;
   NsModel temp;
   NsVector3f percent, T;
	NsModel *model;


   if( FileOpenNameDialog( s_MainWindow.hWnd, "Import SWC", path, NS_PATH_SIZE, filter, NULL, NULL ) )
      {

//ns_progress( &p, __________cancelled, __________update, __________set_title, __________set_description, NULL );


      //ns_model_clear( raw );
      //ns_model_clear( filtered );

      ns_model_construct( &temp );

      ns_model_read_swc( &temp, &header, path, 0, &line_num );

ns_print_newline();
ns_println( "read swc OK" );

      ns_println( "marking manually traced..." );
      ns_model_mark_manually_traced( &temp, NS_TRUE );


      //workspace_set_voxel_info( s_MainWindow.activeWorkspace, 1.0f, 1.0f, 1.0f );
      //ns_vector3f( &voxel_size, 1.0f, 1.0f, 1.0f );
      //ns_model_set_voxel_size( raw, &voxel_size );
      //ns_model_set_voxel_size( filtered, &voxel_size );



ns_println( "begin ns_model_clean_dup_vertices()" );
ns_model_clean_dup_vertices( &temp );

ns_println( "begin ns_model_clean_self_loops()" );
ns_model_clean_self_loops( &temp );

ns_println( "begin ns_model_clean_dup_edges()" );
ns_model_clean_dup_edges( &temp );



      ns_println( "setting file number..." );
      ns_model_set_file_num( &temp, ____import_file_number );
      ++____import_file_number;


_read_shrinkage_and_translation_from_config_file( &percent, &T );


ns_println( "translating vertices by %f, %f, %f", T.x, T.y, T.z );
ns_model_filter_translate_vertices( &temp, &T );


		//ns_println( "establishing thresholds and contrasts..." );
		//workspace_establish_thresholds_and_contrasts( s_MainWindow.hWnd, s_MainWindow.activeWorkspace, &temp );


ns_println( "applying SHRINKAGE %f%%, %f%%, %f%%", percent.x, percent.y, percent.z );
ns_model_filter_adjust_shrinkage( &temp, &percent );


ns_println( "adding imported model to existing model..." );

		model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );



ns_print( "Calculating error..." );
ns_model_section( &temp, NS_MODEL_SPINES_SECTIONING, NULL );
ns_println(
	" 2D=" NS_FMT_DOUBLE
	" 3D=" NS_FMT_DOUBLE,
	ns_model_calc_medial_axis_error_2d( &temp, model ),
	ns_model_calc_medial_axis_error_3d( &temp, model )
	);



      ns_model_merge( model, &temp );
      ns_model_set_conn_comp_numbers( model );

      ns_model_destruct( &temp );

ns_print_newline();

      
      //SetTreeBoundingCube( s_MainWindow.activeWorkspace );

      //ns_model_mark_manually_traced( filtered, NS_TRUE );
      //ns_model_set_conn_comps( filtered );

      /*
      ns_model_label_edges(
         filtered,
         ns_settings_get_neurite_labeling_type(
            workspace_settings( s_MainWindow.activeWorkspace )
            ),
         NULL
         );
      */

      //ns_ascii_strcpy( ____neurites_file_path, path );
      //____neurites_file_exists = NS_TRUE;
      //____neurites_file_type = _FILE_TYPE_SWC;
      //____neurites_file_swc_header = header;

      //_set_status_bar_file( ____neurites_file_path );


//_print_model_vertex_thresholds( filtered );

      ____redraw_all();
      }
   }




void _on_file_import_spines( void )
   {
   static const nschar*  filter = "TXT (*.txt)\0*.txt\0\0\0";
   nschar          path[NS_PATH_SIZE + 1 ] = { '\0' };
   nslong line_num;
	NsModel *model;
   NsError error;
	const nschar *unmatched;


   if( FileOpenNameDialog( s_MainWindow.hWnd, "Import Spines", path, NS_PATH_SIZE, filter, NULL, NULL ) )
      {
		model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

      if( NS_SUCCESS( ns_model_spines_read( model, path, &line_num ), error ) )
			if( ns_model_spines_has_unset_xyplane_angles( model ) )
				error = ns_model_spines_calc_xyplane_angles_if_unset(
							model,
							workspace_settings( s_MainWindow.activeWorkspace ),
							NULL
							);

		if( ns_is_error( error ) )
			{
         ns_snprint(
            path,
            NS_ARRAY_LENGTH( path ) - 1,
            "An error was found on line " NS_FMT_LONG ".",
            line_num
            );

         MessageBox( s_MainWindow.hWnd, path, "NeuronStudio", MB_OK | MB_ICONERROR );
			}

		unmatched = ns_model_spines_read_unmatched();

		if( 0 < ns_ascii_strlen( unmatched ) )
			{
			/* Just re-use the path buffer for this message. */
			ns_snprint(
				path,
				NS_ARRAY_LENGTH( path ),
				"Unknown spine types: " NS_FMT_STRING "\n\n"
				"The above types were not defined in the current classification scheme.\n\n"
				"Spines of these types were set to invalid. You may re-classify by selecting\n"
				"the appropriate classifier from the Spine Classifier dialog box.",
				unmatched
				);

         MessageBox( s_MainWindow.hWnd, path, "NeuronStudio", MB_OK | MB_ICONWARNING );
			}

      ____redraw_all();
      }
   }




/*
void _on_import_markers( void )
   {
   static const nschar*  filter = "All Files (*.*)\0*.*\0\0\0";
   nschar path[ NS_PATH_SIZE + 1 ] = { '\0' };
   nslong line_num;
   NsError error;


   if( FileOpenNameDialog( s_MainWindow.hWnd, "Import Markers", path, NS_PATH_SIZE, filter, NULL, NULL ) )
      {
      if( NS_FAILURE( ns_sampler_input_markers( workspace_sampler( s_MainWindow.activeWorkspace ), path, &line_num ), error ) )
         {
         /*TEMP*//*ns_println( __FUNCTION__ "::error occurred on line " NS_FMT_LONG ".", line_num );

         if( NS_ERROR_INVAL == ns_error_code( error ) )
            MessageBox( s_MainWindow.hWnd, "The file is not a valid markers file.", "NeuronStudio", MB_OK | MB_ICONERROR );
         else
            MessageBox( s_MainWindow.hWnd, ns_error_code_to_string( error ), "NeuronStudio", MB_OK | MB_ICONERROR );
         }
      /*TEMP*//*else ns_println( __FUNCTION__ "::read " NS_FMT_LONG " lines.", line_num );
      

      ____redraw_all();
      }
   }*/
                                                                                                                                                                                                                                                             

//extern void ____spines_destruct( void );
//extern void ____spines_clear( void );



//extern nsfloat __model_origin_samples[];
//extern nsmodelvertex __model_origin_vertex;
//extern nsboolean ____model_origin_samples_init;

extern void _write_workspace_volume( const nschar* );


void _on_mode_change_redraw( void )
   {  ____redraw_all();  }


/*
void _on_view_model_mode( nsint menuID )
   {
   if( menuID != s_MainWindow.model_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, s_MainWindow.model_mode, 0 );
//SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.model_mode, 0 );
      s_MainWindow.model_mode = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, s_MainWindow.model_mode, 1 );
//SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.model_mode, 1 );

      _on_mode_change_redraw();
      }
   }
*/


void _on_neurites_vertex_render_mode( nsint menuID )
   {
   if( menuID != ____neurites_vertex_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_vertex_render_mode, 0 );
      ____neurites_vertex_render_mode = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_vertex_render_mode, 1 );

      ____redraw_all();
      }
   }


void _on_neurites_edge_render_mode( nsint menuID )
   {
   if( menuID != ____neurites_edge_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_edge_render_mode, 0 );
      ____neurites_edge_render_mode = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_edge_render_mode, 1 );

      ____redraw_all();
      }
   }


void _on_view_spine_mode( nsint menuID )
   {
   if( menuID != ____spines_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_render_mode, 0 );
//SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____spines_render_mode, 0 );
      ____spines_render_mode = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_render_mode, 1 );
//SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____spines_render_mode, 1 );

      _on_mode_change_redraw();
      }
   }


void _on_vertex_color_scheme( nsint menuID )
   {
   if( menuID != ____vertex_color_scheme )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____vertex_color_scheme, 0 );
      ____vertex_color_scheme = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____vertex_color_scheme, 1 );

      ____redraw_all();
      }
   }


void _on_edge_color_scheme( nsint menuID )
   {
   if( menuID != ____edge_color_scheme )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____edge_color_scheme, 0 );
      ____edge_color_scheme = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____edge_color_scheme, 1 );

      ____redraw_all();
      }
   }


void _on_spine_color_scheme( nsint menuID )
   {
   if( menuID != ____spines_color_scheme )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_color_scheme, 0 );
      ____spines_color_scheme = menuID;
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_color_scheme, 1 );

      ____redraw_all();
      }
   }


void _on_measuring_rays_count( nsint id )
   {
   if( id != ____measuring_rays_count )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____measuring_rays_count, 0 );
      ____measuring_rays_count = id;
      SetMenuItemChecked( s_MainWindow.hWnd, ____measuring_rays_count, 1 );

      switch( id )
         {
         case IDM_MEASURING_RAYS_COUNT_LOW:
            ____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_SUB_VIZ;
            break;

         case IDM_MEASURING_RAYS_COUNT_NORMAL:
            ____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_VIZ;
            break;

         case IDM_MEASURING_RAYS_COUNT_HIGH:
            ____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_SUPER_VIZ;
            break;

         case IDM_MEASURING_RAYS_COUNT_VERY_HIGH:
            ____measuring_rays_kernel_type = NS_RAYBURST_KERNEL_HYPER_VIZ;
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }



void _on_spines_rays_count( nsint id )
   {
   if( id != ____spines_rays_count )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_rays_count, 0 );
      ____spines_rays_count = id;
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_rays_count, 1 );

      switch( id )
         {
         case SPINES_RAYS_COUNT_LOW:
            ____spines_kernel_type = NS_RAYBURST_KERNEL_SUB_VIZ;
            break;

         case SPINES_RAYS_COUNT_NORMAL:
            ____spines_kernel_type = NS_RAYBURST_KERNEL_VIZ;
            break;

         case SPINES_RAYS_COUNT_HIGH:
            ____spines_kernel_type = NS_RAYBURST_KERNEL_SUPER_VIZ;
            break;

         case SPINES_RAYS_COUNT_VERY_HIGH:
            ____spines_kernel_type = NS_RAYBURST_KERNEL_HYPER_VIZ;
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }




void _on_measurement_interpolation( nsint id )
   {
   if( id != ____measurement_interpolation )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____measurement_interpolation, 0 );
      ____measurement_interpolation = id;
      SetMenuItemChecked( s_MainWindow.hWnd, ____measurement_interpolation, 1 );

      switch( id )
         {
         case IDM_MEASURING_BILINEAR:
            ____measurement_interp_type = NS_RAYBURST_INTERP_BILINEAR;
            break;

         case IDM_MEASURING_TRILINEAR:
            ____measurement_interp_type = NS_RAYBURST_INTERP_TRILINEAR;
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }

void _on_neurites_interpolation( nsint id )
   {
   if( id != ____neurites_interpolation )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_interpolation, 0 );
      ____neurites_interpolation = id;
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_interpolation, 1 );

      switch( id )
         {
         case IDM_NEURITES_BILINEAR:
            ____neurites_interp_type = NS_RAYBURST_INTERP_BILINEAR;
            break;

         case IDM_NEURITES_TRILINEAR:
            ____neurites_interp_type = NS_RAYBURST_INTERP_TRILINEAR;
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }

void _on_spines_interpolation( nsint id )
   {
   if( id != ____spines_interpolation )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_interpolation, 0 );
      ____spines_interpolation = id;
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_interpolation, 1 );

      switch( id )
         {
         case IDM_SPINES_BILINEAR:
            ____spines_interp_type = NS_RAYBURST_INTERP_BILINEAR;
            break;

         case IDM_SPINES_TRILINEAR:
            ____spines_interp_type = NS_RAYBURST_INTERP_TRILINEAR;
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }






void _do_clear_neurites( void )
   {
   workspace_clear_model( s_MainWindow.activeWorkspace );
   //SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_ALL );
   }


void _do_clear_spines( void )
   {  ns_model_clear_spines( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), NULL, NULL );  }


void _do_clear_samples( /*nsboolean all*/ )
   {
   //if( all )
      ns_sampler_clear( workspace_sampler( s_MainWindow.activeWorkspace ) );
   //else
     // ns_sampler_clear_non_markers( workspace_sampler( s_MainWindow.activeWorkspace ) );

   _status_measurement();
   }


void _on_clear_neurites( void )
   {
   if( IDYES == MessageBox(
         s_MainWindow.hWnd,
         "Are you sure you want to delete the neurites?",
         "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
      {
      _do_clear_neurites();
      ____redraw_all();
      }
   }


void _on_clear_spines( void )
   {
   if( IDYES == MessageBox(
         s_MainWindow.hWnd,
         "Are you sure you want to delete the spines?",
         "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
      {
      _do_clear_spines();
      ____redraw_all();
      }
   }


void _on_clear_model( void )
   {
   if( IDYES == MessageBox(
         s_MainWindow.hWnd,
         "Are you sure you want to delete the model?",
         "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
      {
      _do_clear_neurites();
      _do_clear_spines();
      ____redraw_all();
      }
   }


void _on_clear_samples( nsboolean all )
   {
   _do_clear_samples( all );
   ____redraw_all();
   }


//void _on_clear_markers( void )
  // {
  // ns_sampler_clear_markers( workspace_sampler( s_MainWindow.activeWorkspace ) );
  // _status_measurement();
  // ____redraw_all();
  // }


void _on_clear_all( void )
   {
   if( IDYES == MessageBox(
         s_MainWindow.hWnd,
         "Are you sure you want to delete everything?",
         "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
      {
      _do_clear_neurites();
      _do_clear_spines();
      _do_clear_samples( NS_TRUE );
      ____redraw_all();
      }
   }



/*
NsError _create_filtered_model_from_raw_model( void )
   {
   NsModel           *raw_model, *filtered_model;
   const NsSettings  *settings;
   NsError            error;


   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );
   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   settings       = workspace_settings( s_MainWindow.activeWorkspace );

   ns_model_clear_origins( raw_model );
   
   if( NS_FAILURE( ns_model_create_origins( raw_model, &ns_progress_null ), error ) )   
      return error;

   //#ifdef NS_DEBUG
   //ns_model_verify( raw_model );
   //#endif

   ns_model_clear_non_manually_traced( filtered_model );
   
   if( NS_FAILURE( ns_model_merge( filtered_model, raw_model ), error ) )
      return error;

   if( ____run_model_filtering )
      if( NS_FAILURE( ns_model_filter( filtered_model, settings, &ns_progress_null ), error ) )
         return error;

   ns_model_set_conn_comp_numbers( filtered_model );

   //ns_model_label_edges(
     // filtered_model,
     // ns_settings_get_neurite_labeling_type( settings ),
     // NULL
     // );

   //#ifdef NS_DEBUG
   //ns_model_verify( filtered_model );
   //#endif

   return ns_no_error();
   }
*/


#include <model/nsmodel-filter.h>


void _on_convert_measurements( void )
   {
   NsModel        *filtered_model;
   NsModel        *raw_model;
   NsSampler      *sampler;
   NsModel         temp_model;
	nsboolean       one_selected;
	nsmodelvertex   Vf, Vt;
	NsVector3f      P;
   NsList          join_points;
	NsError         error;


   sampler = workspace_sampler( s_MainWindow.activeWorkspace );

   if( ! ns_sampler_is_empty( sampler )/*ns_sampler_has_non_markers( sampler )*/ )
      {
      filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
      raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

      ns_list_construct( &join_points, NULL );
      ns_model_construct( &temp_model );

		//ns_println( "before sampler to model..." );
      /*error*/ns_sampler_to_model( sampler, &temp_model );
		//ns_println( "after sampler to model" );

		/* NOTE: Temporarily mark the manual as NOT manually traced so the discretization
			can occur! Kind of a hack. */
		ns_model_mark_manually_traced( &temp_model, NS_FALSE );
		/*error*/ns_model_filter_discretize( &temp_model, workspace_settings( s_MainWindow.activeWorkspace ), NULL );
		ns_model_mark_manually_traced( &temp_model, NS_TRUE );

		ns_model_select_vertices( &temp_model, NS_TRUE );

		one_selected = ns_model_buffer_selected_vertices( filtered_model, &Vf, 1 );
		ns_model_select_vertices( filtered_model, NS_FALSE );

		Vt = NS_MODEL_VERTEX_NIL;

		if( 0 < ns_model_num_vertices( &temp_model ) )
			Vt = ns_model_begin_vertices( &temp_model );

		//ns_println( "before joining temp model..." );
      /*error*/ns_model_join( filtered_model, &temp_model, &join_points );
		//ns_println( "after joining temp model..." );
      /*error*///ns_model_merge( filtered_model, &temp_model );

///*TEMP*/ns_println( "# of join points = " NS_FMT_ULONG, ns_list_size( &join_points ) );
///*TEMP*/ns_println( "Is only one selected? = " NS_FMT_INT, one_selected );

		/* If we didnt do any joining and there was only 1 selected vertex in the
			filtered model, then create an edge between the last vertex
			in the 'temp_model' and the 'filtered_model'. */
		if( one_selected &&
			 ns_list_is_empty( &join_points ) &&
			 ns_model_vertex_not_equal( Vt, NS_MODEL_VERTEX_NIL ) )
			{
			Vt = ns_model_find_vertex_by_position( filtered_model, ns_model_vertex_get_position( Vt, &P ) );

			if( ns_model_vertex_not_equal( Vt, NS_MODEL_VERTEX_NIL ) )
				{
				ns_assert( ns_model_vertex_model( Vf ) == ns_model_vertex_model( Vt ) );

				if( ( ns_model_vertex_get_conn_comp( Vf ) != ns_model_vertex_get_conn_comp( Vt ) )
						&&
					 ( ! ns_model_vertex_edge_exists( Vf, Vt ) ) )
					{
					/*TEMP Should handle error properly. */
					if( NS_SUCCESS( ns_model_vertex_add_edge( Vf, Vt, NULL, NULL ), error ) )
						ns_model_adjust_origins_on_adding_edge( filtered_model, Vf, Vt );
					}
				}
			}

      ns_model_destruct( &temp_model );

      ns_model_set_conn_comp_numbers( filtered_model );
      ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &join_points );

      ns_list_destruct( &join_points );

      /*error*///_create_filtered_model_from_raw_model();

      ns_sampler_clear( sampler );//ns_sampler_clear_non_markers( sampler );
      ____redraw_all();
      }
   }


void _do_delete_selected_vertices( void )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*/ns_model_remove_selected_vertices( filtered_model, &list );
   /*error*/ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );
   }


void _on_delete_selected_vertices( void )
   {
   /*if( IDYES == MessageBox(
         s_MainWindow.hWnd,
         "Are you sure you want to delete the selected vertices?",
         "NeuronStudio",
         MB_YESNO | MB_ICONQUESTION ) )
      {*/

      _do_delete_selected_vertices();
      ____redraw_all();
      //}
   }


extern nsint s_CurrentMouseMode;



void _on_delete_selected_spines( void )
   {
   ns_model_delete_selected_spines( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
   ____redraw_all();
   }


void _on_delete_selected_samples( void )
   {
   ns_sampler_remove_selected( workspace_sampler( s_MainWindow.activeWorkspace ) );
   ____redraw_all();

   _status_measurement();
   }


/*
void _main_window_on_delete_sample( void )
   {
   NsSampler *sampler;


   if( 0 == s_MainWindow.activeWorkspace )
      return;

   sampler = workspace_sampler( s_MainWindow.activeWorkspace );

   if( ! ns_sampler_is_empty( sampler ) )
      {
      ns_sampler_remove( sampler, ns_sampler_last( sampler ) );
      ____redraw_all();
      }

   _status_measurement();
   }
*/


void _on_delete_selected( void )
   {
   switch( s_CurrentMouseMode )
      {
      case eDISPLAY_WINDOW_MOUSE_MODE_RAYS:
         _on_delete_selected_samples();
         break;

      case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE:
         _on_delete_selected_vertices();
         break;

      case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:
         _on_delete_selected_spines();
         break;
      }
   }


void _on_select_or_deselect_all( nsboolean selected )
   {
   NsModel    *model;
   NsSampler  *sampler;
   nsboolean   redraw = NS_TRUE;


   model   = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );
   sampler = workspace_sampler( s_MainWindow.activeWorkspace );

	if( selected )
		{
		switch( s_CurrentMouseMode )
			{
			case eDISPLAY_WINDOW_MOUSE_MODE_RAYS:
				ns_sampler_select( sampler, selected );
				break;

			case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:
				ns_model_select_spines( model, selected );
				break;
      
			case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE:
				ns_model_select_vertices( model, selected );
				break;

			default:
				redraw = NS_FALSE;
			}
		}
	else
		{
		ns_sampler_select( sampler, selected );
		ns_model_select_spines( model, selected );
		ns_model_select_vertices( model, selected );
		}

   if( redraw )
      ____redraw_all();
   }


void _MainWindowOnSetMouseMode( nsint how, nsint id, nsint mode, nsint notifyCode ) 
   {
   if( 0 == s_MainWindow.activeWorkspace )
      return;

   if( s_CurrentMouseMode == mode && ( 0 <= mode && mode <= NS_ARRAY_LENGTH( s_MouseModeToButton ) ) )
      {
      s_MainWindow.ctrlbox.buttonObjects[s_MouseModeToButton[ mode ]].state = eBUTTON_STATE_DOWN;
      InvalidateRect( s_MainWindow.ctrlbox.buttonHandles[s_MouseModeToButton[ mode ]], NULL, FALSE );
      return;
      }

   switch( how )
      {
      case 0:
         _SetDisplayWindowsMouseMode( ( ! IsMenuItemChecked( s_MainWindow.hWnd, id ) ) ?
                                mode : eDISPLAY_WINDOW_NO_MOUSE_MODE, 
                                1,
                                id
                              );
         break;

      case 1:
         _SetDisplayWindowsMouseMode( ( eBUTTON_STATE_DOWN == notifyCode ) ?
                                       mode : eDISPLAY_WINDOW_NO_MOUSE_MODE,
                                      0,
                                      id
                                    );
         break;
      }
   }


//extern eERROR_TYPE __workspace_write_image_hack
 //  ( HWND hWnd, nschar* prefix, const nschar* extension, nsboolean doPadNumber, nsboolean as_series );


void _main_window_on_save_image( void )
   {
   //nschar name[ NS_PATH_SIZE + 1 ] = { '\0' };

   if( 0 == s_MainWindow.activeWorkspace )
      return;

   if( SaveImageDialog() )
      {
      ns_chdir( s_VI_CurrDir );

      workspace_write_image(
         s_MainWindow.activeWorkspace,
         s_MainWindow.hWnd,
         s_VI_CurrPrefix,
         GetFileExtension( s_VI_CurrFileType ),
         s_VI_CurrDoPadNumber,
         s_VI_CurrSaveAsSeries,
			__bit_depth_values[ ____current_bit_depth ]
         );
      }
   }


extern eERROR_TYPE workspace_write_projections( HWorkspace, const nschar*, const nschar* );

void _on_save_projections( void )
   {
   //nschar name[ NS_PATH_SIZE ] = { '\0' };

   if( 0 == s_MainWindow.activeWorkspace )
      return;

   if( SaveProjectionsDialog() )
      {
      ns_chdir( s_VI_CurrDir );

      //ns_println( NS_FMT_STRING, s_VI_CurrDir );

      workspace_write_projections(
         s_MainWindow.activeWorkspace,
         s_VI_CurrPrefix,
         GetFileExtension( s_VI_CurrFileType )
         );
      }
   }


void _main_window_on_save_samples( void )
   {
   OPENFILENAME ofn;
   nschar path[NS_PATH_SIZE + 1] = { '\0' };
   NsError error;

   ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = s_MainWindow.hWnd;
   ofn.lpstrFile   = path;
   ofn.lpstrTitle  = "Save Measurements";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "Text (*.txt)\0*.txt\0\0\0";
   ofn.lpstrDefExt = ".txt";
   ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( GetSaveFileName( &ofn ) )
      if( NS_FAILURE( ns_sampler_output( workspace_sampler( s_MainWindow.activeWorkspace ), ofn.lpstrFile ), error ) )
         MessageBox(NULL, "An error occurred while saving the measurements to file. Please try again.", "NeuronStudio", MB_OK | MB_ICONERROR );
   }


void _on_save_spine_classifier( void )
	{
	const NsSpinesClassifier *sc;
   OPENFILENAME ofn;
   nschar path[NS_PATH_SIZE + 1] = { '\0' };
   NsError error;


	if( NULL != ( sc = ns_spines_classifier_get() ) )
		{
		ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner   = s_MainWindow.hWnd;
		ofn.lpstrFile   = path;
		ofn.lpstrTitle  = "Save Spine Classifier";
		ofn.nMaxFile    = NS_PATH_SIZE;
		ofn.lpstrFilter = "NeuronStudio Spine Classifier (*.nssc)\0*.nssc\0\0\0";
		ofn.lpstrDefExt = ".nssc";
		ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if( GetSaveFileName( &ofn ) )
			if( NS_FAILURE( ns_spines_classifier_write( sc, ofn.lpstrFile ), error ) )
				MessageBox(NULL, "An error occurred while saving the spine classifier to file. Please try again.", "NeuronStudio", MB_OK | MB_ICONERROR );
		}
	}


/*
void _main_window_on_save_ann_data( void )
   {
   OPENFILENAME ofn;
   nschar training_path[NS_PATH_SIZE + 1] = { '\0' };
	nschar running_path[NS_PATH_SIZE + 1] = { '\0' };
   NsError error;


   ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = s_MainWindow.hWnd;
   ofn.lpstrFile   = training_path;
   ofn.lpstrTitle  = "Save ANN Data (training)";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "Data (*.data)\0*.data\0\0\0";
   ofn.lpstrDefExt = ".data";
   ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( GetSaveFileName( &ofn ) )
		{
		ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner   = s_MainWindow.hWnd;
		ofn.lpstrFile   = running_path;
		ofn.lpstrTitle  = "Save ANN Data (running)";
		ofn.nMaxFile    = NS_PATH_SIZE;
		ofn.lpstrFilter = "Data (*.data)\0*.data\0\0\0";
		ofn.lpstrDefExt = ".data";
		ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if( GetSaveFileName( &ofn ) )
			{
			if( NS_FAILURE(
					ns_model_spines_write_ann_data(
						GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
						training_path,
						running_path
						),
					error ) )
				MessageBox(
					NULL,
					"An error occurred while saving the ANN data to file. Please try again.",
					"NeuronStudio",
					MB_OK | MB_ICONERROR
					);
			}
		}
   }*/



/*
void _main_window_on_save_markers( void )
   {
   OPENFILENAME ofn;
   nschar path[NS_PATH_SIZE + 1] = { '\0' };
   NsError error;

   ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = s_MainWindow.hWnd;
   ofn.lpstrFile   = path;
   ofn.lpstrTitle  = "Save Markers";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "DAT (*.dat)\0*.dat\0\0\0";
   ofn.lpstrDefExt = ".dat";
   ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( GetSaveFileName( &ofn ) )
      if( NS_FAILURE( ns_sampler_output_markers( workspace_sampler( s_MainWindow.activeWorkspace ), ofn.lpstrFile ), error ) )
         MessageBox(NULL, "An error occurred while saving the markers to file. Please try again.", "NeuronStudio", MB_OK | MB_ICONERROR );
   }*/



void ____projection_type_uncheck( void )
   {  ____projection_type_do_check( NS_FALSE );  }

void ____projection_type_check( void )
   {  ____projection_type_do_check( NS_TRUE );  }


void ____on_set_projection_type( nsint projection_type )
   {
   if( /*! ____projections_running && */ projection_type != ____projection_type )
      {
      ____projection_type_uncheck();
      ____projection_type = projection_type;
      ____projection_type_check();

      regenerate_projections( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
      ____redraw_all();
      }
   }


void ____on_run_projection( void )
   {
   //if( ! ____projections_running )
      {
      regenerate_projections( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
      ____redraw_all();
      }
   }


void ____selected_num_cpu_do_check( nsboolean do_check )
   {
   SetMenuItemChecked( s_MainWindow.hWnd, ____selected_num_cpu, do_check );
   }

void ____selected_num_cpu_uncheck( void )
   {  ____selected_num_cpu_do_check( NS_FALSE );  }

void ____selected_num_cpu_check( void )
   {  ____selected_num_cpu_do_check( NS_TRUE );  }

void ____on_set_selected_num_cpu( nsint selected_num_cpu )
   {
   if( selected_num_cpu != ____selected_num_cpu )
      {
		NsConfigDb  db;
		nsint       num_cpu;
		NsError     error;


      ____selected_num_cpu_uncheck();
      ____selected_num_cpu = selected_num_cpu;
      ____selected_num_cpu_check();

	   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
			return;

		if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
			{
			ns_config_db_destruct( &db );
			return;
			}

		if( ns_config_db_has_group( &db, "cpu" ) )
			if( ns_config_db_has_key( &db, "cpu", "count" ) )
				{
				num_cpu = ( nsint )____get_num_cpu();

				ns_config_db_set_int( &db, "cpu", "count", num_cpu );
				ns_config_db_write( &db, ____config_file );

ns_println( "WROTE CPU COUNT of " NS_FMT_INT " to config file.", num_cpu );
				}

		ns_config_db_destruct( &db );
      }
   }


extern void ____set_display_window_actual_pixels( void );

void _on_actual_pixels( void )
   {  ____set_display_window_actual_pixels();  }


void _on_view_measurement_text( void )
   {
   ____view_measurement_text = ! ____view_measurement_text;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_MEASUREMENT_TEXT, ____view_measurement_text );

   ____redraw_2d();
   }


void _on_view_measurement_threshold_samples( void )
   {
   ____measurement_view_thresholding = ! ____measurement_view_thresholding;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_MEASUREMENT_THRESHOLD_SAMPLES, ____measurement_view_thresholding );

   ____redraw_2d();
   }


void _on_view_spine_vectors( void )
   {
   ____view_spine_vectors = ! ____view_spine_vectors;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_VECTORS, ____view_spine_vectors );

   ____redraw_all();
   }


void _on_view_spine_attachments( void )
   {
   ____view_spine_attachments = ! ____view_spine_attachments;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_ATTACHMENTS, ____view_spine_attachments );

   ____redraw_all();
   }


void _on_view_spine_ids( void )
	{
	____view_spine_ids = ! ____view_spine_ids;
	SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_IDS, ____view_spine_ids );

	____redraw_all();
	}


void _on_projected_2d_grafting( void )
   {
   ____projected_2d_grafting = ! ____projected_2d_grafting;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_PROJECTED_2D_GRAFTING, ____projected_2d_grafting );
   }


/*
void _on_run_model_filtering( void )
   {
   ____run_model_filtering = ! ____run_model_filtering;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MODEL_FILTERING, ____run_model_filtering );

   /*error*//*_create_filtered_model_from_raw_model();
   ____redraw_all();
   }
*/

void _on_view_spine_layers( void )
   {
   ____spines_color_by_layer = ! ____spines_color_by_layer;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_LAYERS, ____spines_color_by_layer );

   ____redraw_all();
   }


void _on_view_spine_voxels_dts( void )
   {
   ____view_spine_voxels_dts = ! ____view_spine_voxels_dts;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SPINE_VOXELS_DTS, ____view_spine_voxels_dts );

   ____redraw_all();
   }


void _on_view_non_spine_voxels( void )
   {
   ____view_non_spine_voxels = ! ____view_non_spine_voxels;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_NON_SPINE_VOXELS, ____view_non_spine_voxels );

   ____redraw_all();
   }


void _on_view_2d_polygon_borders( void )
   {
   ____view_2d_polygon_borders = ! ____view_2d_polygon_borders;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_2D_POLYGON_BORDERS, ____view_2d_polygon_borders );

   ____redraw_2d();
   }


void _on_user_toggle_boolean( nsboolean *value, nsint id, void ( *redraw_func )( void ) )
   {
   *value = ! (*value);
   SetMenuItemChecked( s_MainWindow.hWnd, id, *value );
   ( redraw_func )();
   }


void _on_gamma_correction( void )
   {
   //if( ! ____projections_running )
      {
      ____gamma_correction = ! ____gamma_correction;
      SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_PROJECTIONS_GAMMA_CORRECTION, ____gamma_correction );

      regenerate_projections( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
      ____redraw_2d();
      }
   }


#include "imageinfo.inl"
void ____on_image_info( void )
   {  _image_info_dialog();  }


#include "modelinfo.inl"
void ____on_model_info( void )
   {  _model_info_dialog();  }


//#include <ext/htmlhelp.h>

void _on_help_contents( void )
   {
   nschar path[ NS_PATH_SIZE + 1 ];

   ns_ascii_strcpy( path, _startup_directory );
   ns_ascii_strcat( path, "\\help\\index.html" );

//ns_println( "help path = " NS_FMT_STRING, path );

  // if( NULL == HtmlHelp( GetDesktopWindow(), path, HH_DISPLAY_TOPIC, ( DWORD )NULL ) )
    //  MessageBox( s_MainWindow.hWnd, "Could not launch HTML help window.", "NeuronStudio", MB_OK | MB_ICONERROR );


    /* Do the call to the shell */
    ShellExecute(GetDesktopWindow(),(LPCTSTR) "open", (LPCTSTR) path,
                    NULL,NULL,0);
   }




void _redraw_display_bypass_queue( void )
   {
	____redraw_2d_need_render = NS_TRUE;

   RedrawWindow( s_MainWindow.display.windows[0], NULL, NULL, RDW_INVALIDATE );
   UpdateWindow( s_MainWindow.display.windows[0] );
   }


void _xy_slice_viewer_draw( void )
   {
   ns_assert( ____xy_slice_enabled );

   workspace_build_slice_view( s_MainWindow.activeWorkspace, ____xy_slice_index );
   workspace_set_graphics_xy( s_MainWindow.activeWorkspace );

   _redraw_display_bypass_queue();
   ____redraw_3d();
   }


#define _slice_viewer_do_beep()\

/*   Beep( 440, 100 ) */


void _on_view_xy_slice_viewer_first( void )
   {
	const NsCubei *roi = workspace_update_roi( s_MainWindow.activeWorkspace );

   if( ( nssize )roi->C1.z < ____xy_slice_index )
      {
      ____xy_slice_index = ( nssize )roi->C1.z;
      _xy_slice_viewer_draw();
      }
   else
      _slice_viewer_do_beep();
   }


void _on_view_xy_slice_viewer_last( void )
   {
	const NsCubei *roi = workspace_update_roi( s_MainWindow.activeWorkspace );

   if( ____xy_slice_index < ( nssize )roi->C2.z )
      {
      ____xy_slice_index = ( nssize )roi->C2.z;
      _xy_slice_viewer_draw();
      }
   else
      _slice_viewer_do_beep();
   }


void _on_view_xy_slice_viewer_up( void )
   {
	const NsCubei *roi = workspace_update_roi( s_MainWindow.activeWorkspace );

   if( ____xy_slice_index < ( nssize )roi->C2.z )
      {
      ++____xy_slice_index;
      _xy_slice_viewer_draw();
      }
   else
      _slice_viewer_do_beep();
   }


void _on_view_xy_slice_viewer_down( void )
   {
	const NsCubei *roi = workspace_update_roi( s_MainWindow.activeWorkspace );

   if( ( nssize )roi->C1.z < ____xy_slice_index )
      {
      --____xy_slice_index;
      _xy_slice_viewer_draw();
      }
   else
      _slice_viewer_do_beep();
   }


void _on_view_xy_slice_viewer( void )
   {
   NsError error;

   if( ____xy_slice_enabled )
      workspace_finalize_slice_view( s_MainWindow.activeWorkspace );
   else if( NS_FAILURE( workspace_init_slice_view( s_MainWindow.activeWorkspace ), error ) )
      {
      MessageBox( s_MainWindow.hWnd, "Not enough memory to create slice viewer.", "NeuronStudio", MB_OK );
      return;
      }

   ____xy_slice_enabled = ! ____xy_slice_enabled;

   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_XY_SLICE_VIEWER, ____xy_slice_enabled );

   if( ____xy_slice_enabled )
      _xy_slice_viewer_draw();
   else
      {
      workspace_set_graphics_xy( s_MainWindow.activeWorkspace );

      _redraw_display_bypass_queue();
      ____redraw_3d();

      SetStatusBarText( "", STATUS_BAR_SLICE );
      }
   }



void _on_create_or_delete_edge_between_two_vertices( void );
//void _on_create_edge_between_two_vertices( void );
//void _on_delete_edge_between_two_vertices( void );

void _on_join_or_unjoin_selected( void )
   {
   if( eDISPLAY_WINDOW_MOUSE_MODE_NEURITE == s_CurrentMouseMode ||
		 DISPLAY_WINDOW_MOUSE_MODE_MAGNET == s_CurrentMouseMode )
      {
      //_on_create_edge_between_two_vertices();
      _on_create_or_delete_edge_between_two_vertices();
      }
   }
/*
void _on_cut_selected( void )
   {
   if( eDISPLAY_WINDOW_MOUSE_MODE_NEURITE == s_CurrentMouseMode )
      _on_delete_edge_between_two_vertices();
   }
*/




/*
void _on_run_markers( void )
   {
   ____markers_active = ! ____markers_active;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RUN_MARKERS, ____markers_active );
   }
*/






void _on_neurite_change_radii( nsboolean up )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*/ns_model_change_selected_vertices_radii(
      filtered_model,
      up ? 105.0f : 95.0f,
      /*ns_voxel_info_tolerance*/ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) ),
      &list
      );

   /*error*/ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );

   ____redraw_all();
   }


void _on_spines_change_radii( nsboolean up )
   {
   NsModel *model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

   ns_model_change_selected_spines_radii(
      model,
      up ? 105.0f : 95.0f,
      ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) )
      );

   ____redraw_all();
   }


void _on_set_selected_as_origin( void )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*/ns_model_selected_vertex_to_origin( filtered_model, &list );
   /*error*/ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );

   ____redraw_all();
   }


void _on_render_flicker_free( void )
   {
   ____flicker_free_rendering = ! ____flicker_free_rendering;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_FLICKER_FREE, ____flicker_free_rendering );
   }


void _on_render_spine_msh_zone( void )
   {
   ____view_maximum_spine_height = ! ____view_maximum_spine_height;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_SPINE_MSH_ZONE, ____view_maximum_spine_height );

   ____redraw_all();
   }


void _on_render_measurement_surface_mesh( void )
   {
   ____measurement_view_surface_mesh = ! ____measurement_view_surface_mesh;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_MEASUREMENT_SURFACE_MESH, ____measurement_view_surface_mesh );

   ____redraw_all();
   }


void _on_render_spine_rounded_corners( void )
   {
   ____spines_use_round_corners = ! ____spines_use_round_corners;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_RENDER_SPINE_ROUNDED_CORNERS, ____spines_use_round_corners );

   ____redraw_3d();
   }


void _on_run_select_spine_bases( void )
   {
   ns_model_select_spine_bases( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), ____config_file );
   ____redraw_all();
   }


void _on_edit_select_component( void )
   {
   /*error*/ns_model_select_conn_comps_of_selected_vertices( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
   ____redraw_all();
   }


void _on_edit_select_sub_tree( void )
   {
   /*error*/ns_model_select_sub_trees_of_selected_vertices( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
   ____redraw_all();
   }


void _on_view_center_and_limit_image( void )
   {
   ____2d_display_center_and_limit_image = ! ____2d_display_center_and_limit_image;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_CENTER_AND_LIMIT_IMAGE, ____2d_display_center_and_limit_image );
   ____redraw_2d();
   }


/*
void _on_use_seed_as_3d_pivot( void )
   {
   ____use_seed_as_3d_pivot = ! ____use_seed_as_3d_pivot;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_USE_SEED_AS_3D_PIVOT, ____use_seed_as_3d_pivot );
   ____redraw_3d();
   }
*/



void _on_seed_always_visible( void )
   {
   ____view_seed_always_on_top = ! ____view_seed_always_on_top;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SEED_ALWAYS_VISIBLE, ____view_seed_always_on_top );
   ____redraw_all();
   }


void _on_roi_always_visible( void )
	{
   ____view_roi_always_on_top = ! ____view_roi_always_on_top;
   SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_ROI_ALWAYS_VISIBLE, ____view_roi_always_on_top );
   ____redraw_all();
	}


void _on_view_projection_axes( void )
	{
	____show_2d_axes = ! ____show_2d_axes;
	SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_PROJECTION_AXES, ____show_2d_axes );
	____redraw_2d();
	}	


void _on_view_edge_order_numbers( void )
	{
	____view_edge_order_numbers = ! ____view_edge_order_numbers;
	SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_EDGE_ORDER_NUMBERS, ____view_edge_order_numbers );
	____redraw_2d();
	}


void _on_view_edge_section_numbers( void )
	{
	____view_edge_section_numbers = ! ____view_edge_section_numbers;
	SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_EDGE_SECTION_NUMBERS, ____view_edge_section_numbers );
	____redraw_2d();
	}


void _on_edit_hide_selected( void )
   {
   switch( s_MainWindow.mouseMode )
      {
      case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:
			ns_model_mark_selected_spines_hidden(
				GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
				NS_TRUE
				);
			break;

      case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE:
			ns_model_mark_selected_vertices_hidden(
				GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
				NS_TRUE
				);
			break;
      }

   ____redraw_all();
   }


void _on_edit_create_selection_paths( void )
	{
	/*error*/ns_model_select_paths_of_selected_vertices( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
   ____redraw_all();
	}


void _on_edit_unhide_all( void )
   {
   switch( s_MainWindow.mouseMode )
      {
      case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:
			ns_model_mark_spines_hidden( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), NS_FALSE );
			break;

      case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE:
			ns_model_mark_hidden( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), NS_FALSE );
			break;
      }

   ____redraw_all();
   }


void _on_convert_spines( void )
   {
   //ns_model_spines_convert_to_sampler_markers(
   //   GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
   //   workspace_sampler( s_MainWindow.activeWorkspace )
   //   );

   //____redraw_all();
   }


void _on_edge_order_select( NsModelOrderType type )
   {
   /*error*/ns_model_order(
      GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
      type,
      NULL
      );

   ____redraw_all();
   }


void _on_edge_sections_select( NsModelSectionType type )
   {
   /*error*/ns_model_section(
      GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
      type,
      NULL
      );

   ____redraw_all();
   }


void _on_select_spines_by_type( NsSpineType type )
	{
	ns_model_select_spines_by_type( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), type );
	____redraw_all();
	}



void _on_select_vertices_by_type( NsModelVertexType type )
	{
	ns_model_select_vertices_by_type( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), type );
	____redraw_all();
	}


void _on_select_vertices_by_function( NsModelFunctionType type )
	{
	ns_model_select_vertices_by_function( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), type );
	____redraw_all();
	}


void _do_flash_frames( HWND );
void _do_flash_hidden( HWND );


void _on_run_recompute_manual_spines( void )
	{  workspace_recompute_manual_spines( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );  }


void _on_run_recompute_spine_attachments( void )
	{  workspace_recompute_spines_attachment( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );  }


void _on_run_select_spines_on_selected_neurites( void )
	{  workspace_select_spines_on_selected_parts( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );  }


void _on_run_compute_z_spread( void )
	{  workspace_compute_z_spread( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );  }


nsboolean _settings_confirm_refiltering( NsSettings *current, NsSettings *last_applied )
	{
	nsboolean filter = NS_FALSE;

	if( filter ||
			( ! NS_FLOAT_EQUAL(
				ns_settings_get_neurite_attachment_ratio( current ),
			   ns_settings_get_neurite_attachment_ratio( last_applied ) ) )
		)
		filter = NS_TRUE;

	if( filter ||
			( ! NS_FLOAT_EQUAL(
				ns_settings_get_neurite_min_length( current ),
				ns_settings_get_neurite_min_length( last_applied ) ) )
		)
		filter = NS_TRUE;

	if( filter ||
			( ! NS_FLOAT_EQUAL(
				ns_settings_get_neurite_vertex_radius_scalar( current ),
				ns_settings_get_neurite_vertex_radius_scalar( last_applied ) ) )
		)
		filter = NS_TRUE;

	if( filter ||
			( ns_settings_get_neurite_realign_junctions( current )
				!=
			  ns_settings_get_neurite_realign_junctions( last_applied ) )
		)
		filter = NS_TRUE;

	return filter;
	}


void _on_run_settings( void )
	{
	NsSettings  *current_and_return;
	NsSettings   last_applied;


	current_and_return = workspace_settings( s_MainWindow.activeWorkspace );
	last_applied       = *current_and_return;

	SettingsDialog(
		s_MainWindow.hWnd,
		current_and_return,
		&last_applied,
		workspace_raw_model( s_MainWindow.activeWorkspace ),
		workspace_filtered_model( s_MainWindow.activeWorkspace )
		);

	/* There are 2 cases where we may need to update the workspace. Both cases can
		be handled by comparing the 'current_and_return' settings with the 'last_applied'
		settings.

		1) The user applied some settings and then cancelled the dialog. In this
			case we need to revert back the changes.

		2) The user updated some settings but did not apply those changes. In this
			case we need to apply those changes. */

	if( _settings_confirm_refiltering( current_and_return, &last_applied ) )
		workspace_model_filtering( s_MainWindow.activeWorkspace, current_and_return );

	____redraw_all();


	//nsulong *flags = workspace_setting_flags( s_MainWindow.activeWorkspace );

//	if( SettingsDialog(
//		s_MainWindow.hWnd,
//		,
		/*,flags*/
//		) )
//	{
//	____redraw_all();

	//if( ( nsboolean )( (*flags) & NS_SETTING_VOXEL_SIZE ) )
	//	{
		/*ns_println( "establishing thresholds and contrasts..." );
		workspace_establish_thresholds_and_contrasts(
			s_MainWindow.hWnd,
			s_MainWindow.activeWorkspace,
			GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
			);*/
	//	}

	//if( ( flags & BUILD_TREE_SETTINGS_VOLUME_SMOOTHING_PASSES ) ||
	//    ( flags & BUILD_TREE_SETTINGS_SPACING_X               ) ||
	//    ( flags & BUILD_TREE_SETTINGS_SPACING_Y               ) ||
	//    ( flags & BUILD_TREE_SETTINGS_SPACING_Z               )   )
	//   {
		//NsModel *model;
		//NsVector3f voxel_size;
	//               nssize i;


		//model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

		//ns_vector3f( &voxel_size, settings.spacingX, settings.spacingY, settings.spacingZ );
		//ns_model_set_voxel_size( model, &voxel_size );

	//   SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_ALL );

		//_reset_interactor();

		//____redraw_all();
	//   }
	//else if( ( flags & BUILD_TREE_SETTINGS_MINIMUM_BRANCH_RATIO  ) ||
	//         ( flags & BUILD_TREE_SETTINGS_DISCRETIZATION        ) ||
	//         ( flags & BUILD_TREE_SETTINGS_TREE_SMOOTHING_PASSES ) ||
	//         ( flags & BUILD_TREE_SETTINGS_SOMA_RADIUS_SCALAR    ) ||
	//         ( flags & BUILD_TREE_SETTINGS_BRANCH_RADIUS_SCALAR  ) ||
				//( flags & BUILD_TREE_SETTINGS_BREAK_LOOPS           ) 
	//            ( flags & BUILD_TREE_SETTINGS_SMALL_LOOP_RATIO      ) ||
	//         ( flags & BUILD_TREE_SETTINGS_MIN_NUM_VERTICES      )   
	//            )
	//      SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_FILTER );
	//   else if( ( flags & BUILD_TREE_SETTINGS_BRANCH_LABELING_TYPE ) )
	//      SetWorkspaceNeuronTreeBuildFlags( s_MainWindow.activeWorkspace, NEURON_TREE_BUILD_LABEL );

	//if( WorkspaceHasNeuronTree( s_MainWindow.activeWorkspace ) )
	//   SetMenuItemEnabled( s_MainWindow.hWnd,
	//                       IDM_FILE_BRANCH_STATS,
	//                       NS_MODEL_EDGE_LABELING_NONE != settings.branch_labeling_type
	//                     );

	//   SetWorkspaceBuildTreeSettings( s_MainWindow.activeWorkspace, &settings );
//	}
//	}
	}


nsulong ____set_order_value;
nsboolean ____set_order_propagate;

void _set_order_set_values( HWND hDlg )
	{
	nschar number[64];

	SendMessage( GetDlgItem( hDlg, IDC_SET_ORDER_PROPAGATE ), BM_SETCHECK, ____set_order_propagate ? BST_CHECKED : BST_UNCHECKED, 0 );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), NS_FMT_ULONG, ____set_order_value );
	SetDlgItemText( hDlg, IDE_SET_ORDER_VALUE, number );
	}

void _set_order_get_values( HWND hDlg )
	{
	nschar number[64];

	____set_order_propagate = ( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_SET_ORDER_PROPAGATE ), BM_GETCHECK, 0, 0 );

	GetDlgItemText( hDlg, IDE_SET_ORDER_VALUE, number, sizeof( number ) - 1 );
	____set_order_value = ( nsulong )ns_atoi( number );
	}

INT_PTR CALLBACK _set_order_dlg_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
				{
				case IDOK:
					_set_order_get_values( hDlg );
					EndDialog( hDlg, 1 );
					return TRUE;

				case IDCANCEL:
					EndDialog( hDlg, 0 );
					return TRUE;
				}
			return FALSE;

		case WM_INITDIALOG:
			CenterWindow( hDlg, s_MainWindow.hWnd );
			SendMessage( GetDlgItem( hDlg, ID_SET_ORDER_UD ), UDM_SETRANGE, 0, MAKELONG( ( short )1000, ( short )0 ) );
			_set_order_set_values( hDlg );
			SetFocus( GetDlgItem( hDlg, IDE_SET_ORDER_VALUE ) );
			return FALSE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
		}

	return FALSE;
	}


void _on_edit_set_order_of_selected_neurites( void )
	{
	NsModel *model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

	____set_order_value = 0;
	/*error*/ns_model_get_order_of_selected_vertices( model, &____set_order_value );

	____set_order_propagate = NS_TRUE;

	if( ( nsint )DialogBox( g_Instance, "SET_ORDER", s_MainWindow.hWnd, _set_order_dlg_proc ) )
		{
		if( ____set_order_value > ( nsulong )NS_USHORT_MAX )
			____set_order_value = ( nsulong )NS_USHORT_MAX;

		/*error*/ns_model_set_order_of_selected_vertices( model, ____set_order_value );
		____redraw_all();
		}
	}


void _on_edit_select_spines_with_invalid_sa( void )
	{
	ns_model_select_spines_with_invalid_surface_area( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
	____redraw_all();
	}


#include "spineclassifier.inl"

void _on_run_spine_classifier( void )
	{
	/*error*/_spine_classifier_dialog( s_MainWindow.hWnd );
	____redraw_all();
	}


void _on_add_selected_spines_to_classifier( void )
	{
	if( NULL != ns_spines_classifier_get() )
		{
		const NsModel *model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

		if( ns_model_spines_any_selected_are_invalid( model ) )
			MessageBox(
				s_MainWindow.hWnd,
				"You must set the class for all selected spines before\n"
				"adding them as examples in the current classifier.",
				"NeuronStudio",
				MB_OK | MB_ICONWARNING
				);
		else
			ns_model_spines_add_selected_to_classifier( model, ns_spines_classifier_get() );
		}
	}


void _on_run_sholl_analysis( void )
	{
	NsModel *model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

	switch( ns_model_num_origins( model ) )
		{
		case 0:
			MessageBox(
				s_MainWindow.hWnd,
				"No model origin detected. You must create a\nmodel before running Sholl Analysis.",
				"NeuronStudio",
				MB_OK | MB_ICONERROR
				);
			break;

		case 1:
			workspace_run_sholl_analysis( s_MainWindow.hWnd, s_MainWindow.activeWorkspace );
			break;

		default:
			MessageBox(
				s_MainWindow.hWnd,
				"Multiple origins detected. There must be only\none model origin to run a Sholl Analysis.",
				"NeuronStudio",
				MB_OK | MB_ICONERROR
				);
		}
	}


void _on_view_sholl_analysis( void )
	{
	____view_sholl_analysis = ! ____view_sholl_analysis;
	SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_SHOLL_ANALYSIS, ____view_sholl_analysis );
	____redraw_2d();
	}


void _on_render_model( nsint vertex_mode, nsint edge_mode, nsint spine_mode )
	{
   if( vertex_mode != ____neurites_vertex_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_vertex_render_mode, 0 );
      ____neurites_vertex_render_mode = vertex_mode;
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_vertex_render_mode, 1 );
      }

   if( edge_mode != ____neurites_edge_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_edge_render_mode, 0 );
      ____neurites_edge_render_mode = edge_mode;
      SetMenuItemChecked( s_MainWindow.hWnd, ____neurites_edge_render_mode, 1 );
      }

   if( spine_mode != ____spines_render_mode )
      {
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_render_mode, 0 );
      ____spines_render_mode = spine_mode;
      SetMenuItemChecked( s_MainWindow.hWnd, ____spines_render_mode, 1 );
      }

	____redraw_all();
	}


void _on_transpose_child_edges_of_selected_vertices( void )
	{
	ns_model_transpose_child_edges_of_selected_vertices(
		GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
		);

	____redraw_all();
	}


void _MainWindowOnCommand( const nsint ID, const nsint notifyCode )
   {
   //nssize i;

   switch( ID )
      {
/*
      case IDM_VIEW_MODEL_MODE_OFF:
      case IDM_VIEW_MODEL_MODE_POINTS:
      case IDM_VIEW_MODEL_MODE_LINES:
      case IDM_VIEW_MODEL_MODE_LINES_AND_SPHERES:
      //case IDM_GL_VIEW_PIPES:
      //case IDM_GL_VIEW_PIPES_AND_SPHERES:
      case IDM_VIEW_MODEL_MODE_SPHERES:
      case IDM_VIEW_MODEL_MODE_FRUSTUMS:
      case IDM_VIEW_MODEL_MODE_LINES_AND_SOLID_FRUSTUMS:
      case IDM_VIEW_MODEL_MODE_HOLLOW_FRUSTUMS:
      case IDM_VIEW_MODEL_MODE_LINES_AND_HOLLOW_FRUSTUMS:
      case IDM_VIEW_MODEL_MODE_CIRCLES:
      case IDM_VIEW_MODEL_MODE_LINES_AND_CIRCLES:
      //case IDM_GL_VIEW_FLOW_SIMULATION:
         _on_view_model_mode( ID );
         break;
*/

      case IDM_RENDER_MODE_VERTEX_SOLID_ELLIPSES:
      case IDM_RENDER_MODE_VERTEX_HOLLOW_ELLIPSES:
      case IDM_RENDER_MODE_VERTEX_POINTS:
      case IDM_RENDER_MODE_VERTEX_OFF:
         _on_neurites_vertex_render_mode( ID );
         break;

      case IDM_RENDER_MODE_EDGE_LINES:
      case IDM_RENDER_MODE_EDGE_SOLID_FRUSTUMS:
      case IDM_RENDER_MODE_EDGE_HOLLOW_FRUSTUMS:
      case IDM_RENDER_MODE_EDGE_OFF:
         _on_neurites_edge_render_mode( ID );
         break;


      case IDM_VIEW_SPINE_MODE_OFF:
      case IDM_VIEW_SPINE_MODE_POINTS:
      case IDM_VIEW_SPINE_MODE_SOLID_RECTANGLES:
      case IDM_VIEW_SPINE_MODE_HOLLOW_RECTANGLES:
      //case IDM_VIEW_SPINE_MODE_CUBES:
      case IDM_VIEW_SPINE_MODE_SOLID_SURFACE:
      case IDM_VIEW_SPINE_MODE_HOLLOW_SURFACE:
      case IDM_VIEW_SPINE_MODE_SOLID_VOLUMETRIC:
		case IDM_VIEW_SPINE_MODE_HOLLOW_VOLUMETRIC:
		case IDM_VIEW_SPINE_MODE_SOLID_ELLIPSES:
		case IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSES:
		case IDM_VIEW_SPINE_MODE_SOLID_BALL_AND_STICK:
		case IDM_VIEW_SPINE_MODE_HOLLOW_BALL_AND_STICK:
		case IDM_VIEW_SPINE_MODE_SOLID_ELLIPSE_HEAD_DIAM:
		case IDM_VIEW_SPINE_MODE_HOLLOW_ELLIPSE_HEAD_DIAM:
		case IDM_VIEW_SPINE_MODE_CLUMPS:
         _on_view_spine_mode( ID );
         break;

	
		case IDM_RENDER_MODEL_OFF:
			_on_render_model(
				IDM_RENDER_MODE_VERTEX_OFF,
				IDM_RENDER_MODE_EDGE_OFF,
				IDM_VIEW_SPINE_MODE_OFF
				);
			break;

		case IDM_RENDER_MODEL_STANDARD:
			_on_render_model(
				IDM_RENDER_MODE_VERTEX_SOLID_ELLIPSES,
				IDM_RENDER_MODE_EDGE_LINES,
				IDM_VIEW_SPINE_MODE_SOLID_ELLIPSES
				);
			break;


      case IDM_VIEW_CENTER_AND_LIMIT_IMAGE:
         _on_view_center_and_limit_image();
         break;


      //case IDM_VIEW_USE_SEED_AS_3D_PIVOT:
        // _on_use_seed_as_3d_pivot();
        // break;


      case IDM_VIEW_SEED_ALWAYS_VISIBLE:
			_on_seed_always_visible();
         break;

      case IDM_VIEW_ROI_ALWAYS_VISIBLE:
			_on_roi_always_visible();
         break;


		case IDM_VIEW_PROJECTION_AXES:
			_on_view_projection_axes();
			break;


		case IDM_VIEW_EDGE_ORDER_NUMBERS:
			_on_view_edge_order_numbers();
			break;

		case IDM_VIEW_EDGE_SECTION_NUMBERS:
			_on_view_edge_section_numbers();
			break;


		case IDM_VIEW_SHOLL_ANALYSIS:
			_on_view_sholl_analysis();
			break;


      case IDM_RENDER_FLICKER_FREE:
         _on_render_flicker_free();
         break;


      case IDM_COLOR_SCHEME_VERTEX_SINGLE_COLOR:
		case IDM_COLOR_SCHEME_VERTEX_BY_TYPE:
		case IDM_COLOR_SCHEME_VERTEX_BY_FUNCTION:
      case IDM_COLOR_SCHEME_VERTEX_BY_COMPONENT:
		case IDM_COLOR_SCHEME_VERTEX_BY_ORDER:
		case IDM_COLOR_SCHEME_VERTEX_BY_SECTION:
      case IDM_COLOR_SCHEME_VERTEX_BY_FILE_NUMBER:
      case IDM_COLOR_SCHEME_VERTEX_IS_MANUALLY_TRACED:
         _on_vertex_color_scheme( ID );
         break;

      case IDM_COLOR_SCHEME_EDGE_SINGLE_COLOR:
		case IDM_COLOR_SCHEME_EDGE_BY_FUNCTION:
      case IDM_COLOR_SCHEME_EDGE_BY_CONN_COMP:
      case IDM_COLOR_SCHEME_EDGE_BY_ORDER:
      case IDM_COLOR_SCHEME_EDGE_BY_SECTION:
         _on_edge_color_scheme( ID );
         break;

      case IDM_RENDER_SPINE_COLOR_SINGLE:
      case IDM_RENDER_SPINE_COLOR_BY_TYPE:
      case IDM_RENDER_SPINE_COLOR_RANDOM:
		case IDM_RENDER_SPINE_COLOR_IS_MANUAL:
		case IDM_RENDER_SPINE_COLOR_BY_ANGLE:
         _on_spine_color_scheme( ID );
         break;

      case IDM_VIEW_ACTUAL_PIXELS:
         _on_actual_pixels();
         break;

      case IDM_VIEW_NEURITES_BUILD:
         ____draw_while_grafting = ! ____draw_while_grafting;
         SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_NEURITES_BUILD, ____draw_while_grafting );
         break;

      case IDM_VIEW_JUNCTION_VERTICES_ONLY:
         ____draw_junction_vertices_only = ! ____draw_junction_vertices_only;
         SetMenuItemChecked( s_MainWindow.hWnd, IDM_VIEW_JUNCTION_VERTICES_ONLY,
            ____draw_junction_vertices_only );
         break;


      case IDM_MEASUREMENT_RAYBURST_2D:
         ____on_measurement_do_3d_rayburst( NS_FALSE );
         break;

      case IDM_MEASUREMENT_RAYBURST_3D:
         ____on_measurement_do_3d_rayburst( NS_TRUE );
         break;



      case IDM_EDIT_HIDE_SELECTED:
         _on_edit_hide_selected();
         break;

      case IDM_EDIT_UNHIDE_ALL:
         _on_edit_unhide_all();
         break;


		case IDM_EDIT_SET_ORDER_OF_SELECTED_NEURITES:
			_on_edit_set_order_of_selected_neurites();
			break;

		case IDM_EDIT_ADD_SELECTED_SPINES_TO_CLASSIFIER:
			_on_add_selected_spines_to_classifier();
			break;


		case IDM_EDIT_TRANSPOSE_EDGES:
			_on_transpose_child_edges_of_selected_vertices();
			break;


		case IDM_EDIT_SELECT_SPINES_WITH_INVALID_SA:
			_on_edit_select_spines_with_invalid_sa();
			break;


		case IDM_EDIT_CREATE_SELECTION_PATHS:
			_on_edit_create_selection_paths();
         break;


      case IDM_MEASURING_RAYS_COUNT_LOW:
      case IDM_MEASURING_RAYS_COUNT_NORMAL:
      case IDM_MEASURING_RAYS_COUNT_HIGH:
      case IDM_MEASURING_RAYS_COUNT_VERY_HIGH:
         _on_measuring_rays_count( ID );
         break;

      case SPINES_RAYS_COUNT_LOW:
      case SPINES_RAYS_COUNT_NORMAL:
      case SPINES_RAYS_COUNT_HIGH:
      case SPINES_RAYS_COUNT_VERY_HIGH:
         _on_spines_rays_count( ID );
         break;

      case IDM_MEASURING_BILINEAR:
      case IDM_MEASURING_TRILINEAR:
         _on_measurement_interpolation( ID );
         break;

      case IDM_NEURITES_BILINEAR:
      case IDM_NEURITES_TRILINEAR:
         _on_neurites_interpolation( ID );
         break;

      case IDM_SPINES_BILINEAR:
      case IDM_SPINES_TRILINEAR:
         _on_spines_interpolation( ID );
         break;

      case IDM_VIEW_PROJECTION_MAX:
         ____on_set_projection_type( NS_PIXEL_PROC_ORTHO_PROJECT_MAX );
         break;

      case IDM_VIEW_PROJECTION_MIN:
         ____on_set_projection_type( NS_PIXEL_PROC_ORTHO_PROJECT_MIN );
         break;

      case IDM_VIEW_PROJECTION_SUM:
         ____on_set_projection_type( NS_PIXEL_PROC_ORTHO_PROJECT_SUM );
         break;

      case IDM_RUN_NUM_CPU_1:
      case IDM_RUN_NUM_CPU_2:
      case IDM_RUN_NUM_CPU_4:
      case IDM_RUN_NUM_CPU_8:
         ____on_set_selected_num_cpu( ID );
         break;

      case IDM_RUN_PROJECTION:
         ____on_run_projection();
         break;


		case IDM_RUN_ACCEPT_ROI:
			workspace_accept_roi( s_MainWindow.activeWorkspace );
			break;


      case IDM_RUN_SELECT_SPINE_BASES:
         _on_run_select_spine_bases();
         break;


		case IDM_RUN_RECOMPUTE_SPINE_ATTACHMENTS:
			_on_run_recompute_spine_attachments();
			break;


		case IDM_RUN_RECOMPUTE_MANUAL_SPINES:
			_on_run_recompute_manual_spines();
			break;


		case IDM_RUN_SELECT_SPINES_ON_SELECTED_NEURITES:
			_on_run_select_spines_on_selected_neurites();
			break;


		case IDM_RUN_COMPUTE_Z_SPREAD:
			_on_run_compute_z_spread();
			break;


      //case IDM_RUN_SAMPLING_2D:
       //  ____on_run_sampling( NS_TRUE );
         //break;

      //case IDM_RUN_SAMPLING_3D:
        // ____on_run_sampling( NS_FALSE );
        // break;


      //case IDM_RUN_MEAS_SAMPLING_2D:
        // ____on_run_measurement_sampling( NS_TRUE );
        // break;

      //case IDM_RUN_MEAS_SAMPLING_3D:
        // ____on_run_measurement_sampling( NS_FALSE );
        // break;


      case IDM_RUN_SPINE_BOUNDING_BOX_2D:
         ____on_run_spine_bounding_box( NS_TRUE );
         break;

      case IDM_RUN_SPINE_BOUNDING_BOX_3D:
         ____on_run_spine_bounding_box( NS_FALSE );
         break;


      case IDM_RUN_PROJECTED_2D_GRAFTING:
         _on_projected_2d_grafting();
         break;


		case IDM_RUN_SHOLL_ANALYSIS:
			_on_run_sholl_analysis();
         break;


      //case IDM_RUN_MODEL_FILTERING:
        // _on_run_model_filtering();
        // break;


      //case IDM_RUN_MARKERS:
        // _on_run_markers();
        // break;


      case IDM_VIEW_MEASUREMENT_TEXT:
         _on_view_measurement_text();
         break;

      case IDM_VIEW_MEASUREMENT_THRESHOLD_SAMPLES:
         _on_view_measurement_threshold_samples();
         break;

      case IDM_RENDER_MEASUREMENT_SURFACE_MESH:
         _on_render_measurement_surface_mesh();
         break;


      case IDM_VIEW_SPINE_VECTORS:
         _on_view_spine_vectors();
         break;

      case IDM_VIEW_SPINE_ATTACHMENTS:
         _on_view_spine_attachments();
         break;


      case IDM_VIEW_SPINE_LAYERS:
         _on_view_spine_layers();
         break;

      case IDM_VIEW_SPINE_VOXELS_DTS:
         _on_view_spine_voxels_dts();
         break;


		case IDM_VIEW_SPINE_IDS:
			_on_view_spine_ids();
			break;


      case IDM_RENDER_SPINE_ROUNDED_CORNERS:
         _on_render_spine_rounded_corners();
         break;

      case IDM_VIEW_NON_SPINE_VOXELS:
         _on_view_non_spine_voxels();
         break;

      case IDM_VIEW_2D_POLYGON_BORDERS:
         _on_view_2d_polygon_borders();
         break;


      case IDM_RENDER_SPINE_MSH_ZONE:
         _on_render_spine_msh_zone();
         break;


      case IDM_VIEW_SPINE_MAXIMA_VOXELS:
         _on_user_toggle_boolean( &____view_spine_maxima_voxels, ID, ____redraw_all );
         break;

      case IDM_VIEW_SPINE_CANDIDATE_VOXELS:
         _on_user_toggle_boolean( &____view_spine_candidate_voxels, ID, ____redraw_all );
         break;



      case IDM_RUN_PROJECTIONS_GAMMA_CORRECTION:
         _on_gamma_correction();
         break;

      case IDM_FILE_OPEN:
         _on_file_open();
         //_MainWindowOnFileOpen();
         break;

      //case IDM_FILE_OPEN_3D:
      //   _on_file_open_3d();
      //   break;

      case IDM_FILE_CLOSE:
         _MainWindowOnFileClose();
         //____spines_destruct();
         break;


      case IDM_FILE_SAVE_TREE:
         _MainWindowOnFileSaveTree( NS_TRUE );
         break;

      case IDM_FILE_SAVE_NEURITES_AS:
         _MainWindowOnFileSaveTree( NS_FALSE );
         break;


      case IDM_FILE_SAVE_IMAGE:
         _main_window_on_save_image();
         break;

      case IDM_FILE_SAVE_PROJECTIONS:
         _on_save_projections();
         break;

      case IDM_FILE_SAVE_SAMPLES:
         _main_window_on_save_samples();
         break;

		case IDM_FILE_SAVE_SPINE_CLASSIFIER:
			_on_save_spine_classifier();
			break;


		//case IDM_FILE_SAVE_ANN_DATA:
		//	_main_window_on_save_ann_data();
		//	break;


//      case IDM_FILE_SAVE_MARKERS:
  //       _main_window_on_save_markers();         
    //     break;


      case IDM_FILE_IMPORT_SWC:
         _import_swc();
         break;

      case IDM_FILE_IMPORT_SPINES:
         _on_file_import_spines();
         break;

//      case IDM_FILE_IMPORT_MARKERS:
  //       _on_import_markers();
    //     break;

      case IDM_FILE_IMAGE_INFO:
         ____on_image_info();
         break;

      case IDM_FILE_MODEL_INFO:
         ____on_model_info();
         break;

      case IDM_FILE_MODE_ZOOM:
         _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_ZOOM, eDISPLAY_WINDOW_MOUSE_MODE_ZOOM, 0 ); 
         break;

      case _mMAIN_CTRLBOX_ZOOM_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_FILE_MODE_ZOOM, eDISPLAY_WINDOW_MOUSE_MODE_ZOOM, notifyCode ); 
         break;

      case IDM_FILE_MODE_TRANSLATE:
         _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, 0 );
         break;

      case _mMAIN_CTRLBOX_DRAG_BUTTON_ID:
         _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, notifyCode );
         break;

      case IDM_MODE_SEED:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_SEED, eDISPLAY_WINDOW_MOUSE_MODE_SEED, 0 );
         break;

      case _mMAIN_CTRLBOX_SEED_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_SEED, eDISPLAY_WINDOW_MOUSE_MODE_SEED, notifyCode );
         break;


      case IDM_MODE_RAYS:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_RAYS, eDISPLAY_WINDOW_MOUSE_MODE_RAYS, 0 );
         break;

      case _mMAIN_CTRLBOX_RAYS_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_RAYS, eDISPLAY_WINDOW_MOUSE_MODE_RAYS, notifyCode );
         break;



      case IDM_MODE_SPINE:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_SPINE, eDISPLAY_WINDOW_MOUSE_MODE_SPINE, 0 );
         break;

      case _mMAIN_CTRLBOX_SPINE_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_SPINE, eDISPLAY_WINDOW_MOUSE_MODE_SPINE, notifyCode );
         break;


      case IDM_MODE_NEURITE:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_NEURITE, eDISPLAY_WINDOW_MOUSE_MODE_NEURITE, 0 );
         break;

      case _mMAIN_CTRLBOX_NEURITE_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_NEURITE, eDISPLAY_WINDOW_MOUSE_MODE_NEURITE, notifyCode );
         break;



      case IDM_MODE_ROI:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_ROI, eDISPLAY_WINDOW_MOUSE_MODE_ROI, 0 ); 
         break;

      case _mMAIN_CTRLBOX_ROI_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_ROI, eDISPLAY_WINDOW_MOUSE_MODE_ROI, notifyCode ); 
         break;


      case IDM_MODE_MAGNET:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_MAGNET, DISPLAY_WINDOW_MOUSE_MODE_MAGNET, 0 ); 
         break;

      case MAIN_CTRLBOX_MAGNET_BUTTON_ID:
         _MainWindowOnSetMouseMode( 1, IDM_MODE_MAGNET, DISPLAY_WINDOW_MOUSE_MODE_MAGNET, notifyCode ); 
         break;



      case IDM_VIEW_FORWARD:
         _SetDisplayWindowsView( NS_XY, 1, IDM_VIEW_FORWARD );
         break;

      case _mMAIN_CTRLBOX_FORWARD_VIEW_BUTTON_ID:
         _SetDisplayWindowsView( NS_XY, 0, IDM_VIEW_FORWARD );
         break;

      case IDM_VIEW_SIDE:
         _SetDisplayWindowsView( NS_ZY, 1, IDM_VIEW_SIDE );
         break;

      case _mMAIN_CTRLBOX_SIDE_VIEW_BUTTON_ID:
         _SetDisplayWindowsView( NS_ZY, 0, IDM_VIEW_SIDE );
         break;

      case IDM_VIEW_TOP:
         _SetDisplayWindowsView( NS_XZ, 1, IDM_VIEW_TOP );
         break;

      case _mMAIN_CTRLBOX_TOP_VIEW_BUTTON_ID:
         _SetDisplayWindowsView( NS_XZ, 0, IDM_VIEW_TOP );
         break;

      case IDM_VIEW_IMAGE:
         _MainWindowOnChangeView( mDISPLAY_GRAPHICS_IMAGE, IDM_VIEW_IMAGE );
         break;

      case IDM_VIEW_LOW:
         _MainWindowOnChangeView( mDISPLAY_GRAPHICS_LOW_CONTOUR, IDM_VIEW_LOW );
         break;


      case IDM_VIEW_3D_MODEL:
         _MainWindowOnView3DModel();
         break;

      case IDM_VIEW_XY_SLICE_VIEWER:
         _on_view_xy_slice_viewer();
         break;


      case IDM_VIEW_FIT_IMAGES:
         _MainWindowOnFitImages();
         break;

      case IDM_VIEW_COLORS:
         _MainWindowOnViewColors();
         break;

      case IDM_VIEW_PALETTE_STANDARD:
         _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_STANDARD, IDM_VIEW_PALETTE_STANDARD );
         break;

      case IDM_VIEW_PALETTE_INVERSE:
         _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_INVERSE, IDM_VIEW_PALETTE_INVERSE );
         break;

      //case IDM_VIEW_PALETTE_SOLAR_FLARE:
        // _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_SOLAR_FLARE, IDM_VIEW_PALETTE_SOLAR_FLARE );
         //break;

      //case IDM_VIEW_PALETTE_INTENSITY:
        // _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_INTENSITY, IDM_VIEW_PALETTE_INTENSITY );
         //break;

      //case IDM_VIEW_PALETTE_COLDFIRE:
        // _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_COLDFIRE, IDM_VIEW_PALETTE_COLDFIRE );
         //break;

      case IDM_VIEW_PALETTE_TOPOGRAPHIC:
         _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_TOPOGRAPHIC, IDM_VIEW_PALETTE_TOPOGRAPHIC );
         break;

      case IDM_VIEW_PALETTE_GAMMA_CORRECTION_1_7:
         _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_GAMMA_CORRECTION_1_7, IDM_VIEW_PALETTE_GAMMA_CORRECTION_1_7 );
         break;

      case IDM_VIEW_PALETTE_GAMMA_CORRECTION_2_2:
         _MainWindowOnChangeViewPalette( eWORKSPACE_PALETTE_GAMMA_CORRECTION_2_2, IDM_VIEW_PALETTE_GAMMA_CORRECTION_2_2 );
         break;

		case IDM_VIEW_PALETTE_INTENSITY_MASK:
         _MainWindowOnChangeViewPalette( NS_PALETTE_INTENSITY_MASK, IDM_VIEW_PALETTE_INTENSITY_MASK );
			break;

		case IDM_VIEW_PALETTE_RED:
         _MainWindowOnChangeViewPalette( NS_PALETTE_RED, IDM_VIEW_PALETTE_RED );
			break;

		case IDM_VIEW_PALETTE_GREEN:
         _MainWindowOnChangeViewPalette( NS_PALETTE_GREEN, IDM_VIEW_PALETTE_GREEN );
			break;

		case IDM_VIEW_PALETTE_BLUE:
         _MainWindowOnChangeViewPalette( NS_PALETTE_BLUE, IDM_VIEW_PALETTE_BLUE );
			break;

		case IDM_VIEW_PALETTE_YELLOW:
         _MainWindowOnChangeViewPalette( NS_PALETTE_YELLOW, IDM_VIEW_PALETTE_YELLOW );
			break;

      case IDM_VIEW_PEN_THICKNESS_1:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_1, 1 );
         break;
      case IDM_VIEW_PEN_THICKNESS_2:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_2, 2 );
         break;
      case IDM_VIEW_PEN_THICKNESS_3:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_3, 3 );
         break;
      case IDM_VIEW_PEN_THICKNESS_4:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_4, 4 );
         break;
      case IDM_VIEW_PEN_THICKNESS_5:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_5, 5 );
         break;
      case IDM_VIEW_PEN_THICKNESS_6:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_6, 6 );
         break;
      case IDM_VIEW_PEN_THICKNESS_7:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_7, 7 );
         break;
      case IDM_VIEW_PEN_THICKNESS_8:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_8, 8 );
         break;
      case IDM_VIEW_PEN_THICKNESS_9:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_9, 9 );
         break;
      case IDM_VIEW_PEN_THICKNESS_10:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_10, 10 );
         break;
      case IDM_VIEW_PEN_THICKNESS_11:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_11, 11 );
         break;
      case IDM_VIEW_PEN_THICKNESS_12:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_12, 12 );
         break;
      case IDM_VIEW_PEN_THICKNESS_13:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_13, 13 );
         break;
      case IDM_VIEW_PEN_THICKNESS_14:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_14, 14 );
         break;
      case IDM_VIEW_PEN_THICKNESS_15:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_15, 15 );
         break;
      case IDM_VIEW_PEN_THICKNESS_16:
         _MainWindowOnPenThickness( IDM_VIEW_PEN_THICKNESS_16, 16 );
         break;


		case IDM_VIEW_FLASH_IMAGE:
			_do_flash_frames( s_MainWindow.hWnd );
			break;

		case IDM_VIEW_FLASH_HIDDEN:
			_do_flash_hidden( s_MainWindow.hWnd );
			break;


      case IDM_RUN_BUILD_TREE:
         _MainWindowOnBuildNeuronTree();
         break;

		//case IDM_RUN_BUILD_NEURITES_DISCRETE_SAMPLING:
        // _MainWindowOnBuildNeuronTree( NS_FALSE );
			//break;



      case IDM_RUN_SPINE_ANALYSIS:
         _main_window_on_run_spine_analysis();
         break;


      case IDM_RUN_CLEAR_NEURITES:
         _on_clear_neurites();
         break;

      case IDM_CLEAR_MODEL:
         _on_clear_model();
         break;

      case IDM_RUN_CLEAR_SPINES:
         _on_clear_spines();
         break;

      case IDM_RUN_CLEAR_SAMPLES:
         _on_clear_samples( NS_FALSE );
         break;

//      case IDM_RUN_CLEAR_MARKERS:
  //       _on_clear_markers();
    //     break;

      case IDM_CLEAR_ALL:
         _on_clear_all();
         break;


		case IDM_SELECT_SPINES_STUBBY:
			_on_select_spines_by_type( NS_SPINE_STUBBY );
			break;

		case IDM_SELECT_SPINES_THIN:
			_on_select_spines_by_type( NS_SPINE_THIN );
			break;

		case IDM_SELECT_SPINES_MUSHROOM:
			_on_select_spines_by_type( NS_SPINE_MUSHROOM );
			break;

		case IDM_SELECT_SPINES_OTHER:
			_on_select_spines_by_type( NS_SPINE_OTHER );
			break;


		case IDM_SELECT_ORIGIN_VERTICES:
			_on_select_vertices_by_type( NS_MODEL_VERTEX_ORIGIN );
			break;

		case IDM_SELECT_EXTERNAL_VERTICES:
			_on_select_vertices_by_type( NS_MODEL_VERTEX_EXTERNAL );
			break;

		case IDM_SELECT_LINE_VERTICES:
			_on_select_vertices_by_type( NS_MODEL_VERTEX_LINE );
			break;

		case IDM_SELECT_JUNCTION_VERTICES:
			_on_select_vertices_by_type( NS_MODEL_VERTEX_JUNCTION );
			break;


		case IDM_SELECT_SOMA_VERTICES:
			_on_select_vertices_by_function( NS_MODEL_FUNCTION_SOMA );
			break;

		case IDM_SELECT_BASAL_DENDRITE_VERTICES:
			_on_select_vertices_by_function( NS_MODEL_FUNCTION_BASAL_DENDRITE );
			break;

		case IDM_SELECT_APICAL_DENDRITE_VERTICES:
			_on_select_vertices_by_function( NS_MODEL_FUNCTION_APICAL_DENDRITE );
			break;

		case IDM_SELECT_AXON_VERTICES:
			_on_select_vertices_by_function( NS_MODEL_FUNCTION_AXON );
			break;

		case IDM_SELECT_UNKNOWN_VERTICES:
			_on_select_vertices_by_function( NS_MODEL_FUNCTION_UNKNOWN );
			break;



      //case IDM_EDIT_DELETE_SAMPLE:
        // _main_window_on_delete_sample();
         //break;

      case IDM_EDIT_CONVERT_MEASUREMENTS:
         _on_convert_measurements();
         break;

      case IDM_EDIT_CONVERT_SPINES:
         _on_convert_spines();
         break;

      case IDM_EDIT_DELETE_SELECTED:
         _on_delete_selected();
         break;


      case IDM_EDIT_JOIN_SELECTED:
         _on_join_or_unjoin_selected();
         break;

      /*
      case IDM_EDIT_CUT_SELECTED:
         _on_cut_selected();
         break;
      */

      case IDM_EDIT_SET_SELECTED_AS_ORIGIN:
         _on_set_selected_as_origin();
         break;


      case IDM_EDIT_SELECT_ALL:
         _on_select_or_deselect_all( NS_TRUE );
         break;

      case IDM_EDIT_DESELECT_ALL:
         _on_select_or_deselect_all( NS_FALSE );
         break;


      case IDM_EDIT_SELECT_COMPONENT:
         _on_edit_select_component();
         break;

		case IDM_EDIT_SELECT_SUB_TREE:
			_on_edit_select_sub_tree();
         break;

		case IDM_RUN_FILTER_SUBSAMPLE:
			_on_run_filter_subsample();
			break;

      case IDM_RUN_FILTER_DYNAMIC_RANGE:
         _on_run_filter_dynamic_range();
         break;

      case IDM_RUN_FILTER_BLUR:
         _on_run_filter_blur();
         break;

      case IDM_RUN_FILTER_BLUR_MORE:
         _on_run_filter_blur_more();
         break;

      case IDM_RUN_FILTER_BLUR_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_BLUR );
         break;

      case IDM_RUN_FILTER_BLUR_MORE_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_BLUR_MORE );
         break;

      case IDM_RUN_FILTER_MEDIAN_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_MEDIAN );
         break;

      case IDM_RUN_FILTER_INVERT_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_INVERT );
         break;

      case IDM_RUN_FILTER_Z_BLUR_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_Z_BLUR );
         break;

      case IDM_RUN_FILTER_LUM_U12_TO_LUM_U16_MP:
         _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_LUM_U12_TO_LUM_U16 );
         break;

      case IDM_RUN_FILTER_BRIGHTNESS_CONTRAST:
         _on_run_filter_brightness_contrast();
         break;

      case IDM_RUN_FILTER_GAMMA_CORRECTION:
         _on_run_filter_gamma_correction();
         break;

      case IDM_RUN_FILTER_RESIZE:
         _on_run_filter_resize();
         break;

      case IDM_RUN_FILTER_FLIP_VERTICAL:
         _on_run_filter_flip_vertical();
         break;

      case IDM_RUN_FILTER_FLIP_HORIZONTAL:
         _on_run_filter_flip_horizontal();
         break;

      case IDM_RUN_FILTER_FLIP_OPTICAL_AXIS:
         _on_run_filter_flip_optical_axis();
         break;

      case IDM_RUN_FILTER_NOISE:
         _on_run_filter_noise();
         break;
      

     case IDM_FILE_BRANCH_STATS:
        _MainWindowOnFileBranchStats();
       break;

      case IDM_FILE_SPINE_STATS:
         _on_file_spine_stats();
         break;


      case IDM_RUN_SETTINGS:
			_on_run_settings();
         break;

		case IDM_RUN_ADVANCED_SETTINGS:
			//_on_run_advanced_settings();
			_on_run_spine_classifier();
			break;


      case IDM_EDGE_NO_ORDER:
         _on_edge_order_select( NS_MODEL_ORDER_NONE );
         break;

      case IDM_EDGE_CENTRIFUGAL_ORDER:
         _on_edge_order_select( NS_MODEL_ORDER_CENTRIFUGAL );
         break;

      case IDM_EDGE_CENTRIPETAL_ORDER:
         _on_edge_order_select( NS_MODEL_ORDER_CENTRIPETAL );
         break;

      case IDM_EDGE_NO_SECTIONS:
         _on_edge_sections_select( NS_MODEL_SECTION_NONE );
         break;

      case IDM_EDGE_DEPTH_FIRST_SECTIONS:
         _on_edge_sections_select( NS_MODEL_SECTION_DEPTH_FIRST );
         break;

      case IDM_EDGE_BREADTH_FIRST_SECTIONS:
         _on_edge_sections_select( NS_MODEL_SECTION_BREADTH_FIRST );
         break;


      case IDM_HELP_ABOUT:
         AboutNeuronStudioDialog( s_MainWindow.hWnd );
         break;

      case IDM_HELP_CONTENTS:
         _on_help_contents();
         break;
      }

   }/* _MainWindowOnCommand() */



/*
eERROR_TYPE _MainDisplayOnCreate( void )
   {
   nsint i;

   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      {
      s_MainWindow.display.windows[ i ] = 
         CreateDisplayWindow( s_MainWindow.display.hWnd,
                              _mDISPLAY_WINDOW_ID_BASE + i,
                              i,
                              0, 0, 0, 0,
                              ( nsint )i
                             );
       
      if( NULL == s_MainWindow.display.windows[ i ] )
         return g_Error;
      }

   _TileMainDisplay();

   return eNO_ERROR;
   }
*/


/*
void _MainDisplayOnPaint( HWND hWnd, HDC hDC )
   {
   RECT      client;
  // HICON     image;
   //nsuint  width;
   //nsuint  height;
 
  
   GetClientRect( hWnd, &client );
   FillRect( hDC, &client, ( HBRUSH )( COLOR_3DSHADOW + 1 ) );

/*
   client.left += _mMAIN_DISPLAY_BORDER_SIZE - 1;
   client.top  += _mMAIN_DISPLAY_BORDER_SIZE - 1;
   client.right -= _mMAIN_DISPLAY_BORDER_SIZE - 1;
   client.bottom -= _mMAIN_DISPLAY_BORDER_SIZE - 1;

   ColorThinGraphicsBorder3D( hDC,
                             &client,
                              GetSysColor( COLOR_3DHILIGHT ),
                              GetSysColor( COLOR_3DDKSHADOW ),
                              e3D_BORDER_SUNKEN
                           );*/
   //}


//LRESULT CALLBACK _MainDisplayProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
  // {
   //switch( uMsg )
     // {
      /*
      case WM_PAINT:
         {
         PAINTSTRUCT ps;
         HDC         hDC;

         hDC = BeginPaint( hWnd, &ps );
         _MainDisplayOnPaint( hWnd, hDC );
         EndPaint( hWnd, &ps );
         }
         break;
      */

      //case mMSG_USER_Update:
        // ____redraw_2d();
         //break;

      //case WM_COMMAND:
        // _MainDisplayOnCommand( LOWORD( wParam ) );
         //break;

      //case WM_CREATE:
        // s_MainWindow.display.hWnd = hWnd;
         //if( eNO_ERROR != _MainDisplayOnCreate() )
           // return -1;
         //break;

      //default:
        // return DefWindowProc( hWnd, uMsg, wParam, lParam );
      //}

   //return 0;
   //}


void _MainWindowOnSize( WPARAM type, LPARAM size )
   {
   s_MainWindow.width  = LOWORD( size );
   s_MainWindow.height = HIWORD( size ); 

   switch( type )
      {
      case SIZE_MAXIMIZED:
      case SIZE_RESTORED:
         {
         /*MoveWindow( s_MainWindow.display.windows[0],
                     0,
                     0,
                     s_MainWindow.width - _mMAIN_WINDOW_RIGHT_MARGIN_WIDTH,
                     s_MainWindow.height - _mMAIN_WINDOW_BOTTOM_MARGIN_HEIGHT,
                     TRUE
                   );*/

//         _SaveMainDisplaySize();

         _ResizeMainDisplay();

         SetWindowPos( s_MainWindow.ctrlbox.hWnd,
                       NULL,
                     _mMAIN_CTRLBOX_X( s_MainWindow.width ),
                     _mMAIN_CTRLBOX_Y,
                      0, 0,
                     SWP_NOZORDER | SWP_NOSIZE
                     );
          }
         break;

      }/* switch( size type ) */
   }


/*
void _MainWindowOnPaint( HWND hWnd, HDC hDC )
   {
   RECT client;
   //HICON hIcon;


   GetClientRect( s_MainWindow.hWnd, &client );
   FillRect( hDC, &client, ( HBRUSH )( COLOR_3DFACE + 1 ) );
   }
*/

nssize ____initial_num_cpu;

eERROR_TYPE _MainWindowOnCreate( void )
   {
   //WNDCLASSEX wcx;
   RECT rc;
   nsint i;


   //s_MainWindow.display.flags = 0;

//   wcx.cbSize        = sizeof( WNDCLASSEX );
  // wcx.style         = CS_HREDRAW | CS_VREDRAW;
   //wcx.lpfnWndProc   = _MainDisplayProcedure;
   //wcx.cbClsExtra    = 0;
   //wcx.cbWndExtra    = 0;
   //wcx.hIcon         = NULL;
   //wcx.hCursor       = LoadCursor( NULL, IDC_ARROW );
   //wcx.hInstance     = g_Instance;
   //wcx.hbrBackground = NULL;
   //wcx.lpszClassName = "MainDisplay";
   //wcx.lpszMenuName  = NULL;
   //wcx.hIconSm       = NULL;

   //if( 0 == RegisterClassEx( &wcx ) )
     // return ( g_Error = eERROR_OPER_SYSTEM );

   GetClientRect( s_MainWindow.hWnd, &rc );   

   s_MainWindow.width = rc.right - rc.left;
   s_MainWindow.height = rc.bottom - rc.top;
  
   //s_MainWindow.display.hWnd = CreateWindowEx
     //   ( 0,
       //   "MainDisplay",
         // NULL,
          //WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
          //0, 0, s_MainWindow.width, s_MainWindow.height,
          //s_MainWindow.hWnd,
          //NULL, g_Instance, NULL
         //);

   //if( NULL == s_MainWindow.display.hWnd )
     //  return ( g_Error = eERROR_OPER_SYSTEM );


   for( i = 0; i < mNUM_DISPLAY_WINDOWS; ++i )
      {
      s_MainWindow.display.windows[ i ] = 
         CreateDisplayWindow( s_MainWindow.hWnd/*s_MainWindow.display.hWnd*/,
                              _mDISPLAY_WINDOW_ID_BASE + i,
                              i,
                              0, 0, 0, 0,
                              ( nsint )i
                             );
       
      if( NULL == s_MainWindow.display.windows[ i ] )
         return g_Error;
      }

   _TileMainDisplay();


   if( eNO_ERROR != _CreateMainControlbox( s_MainWindow.width, s_MainWindow.height ) )
      return g_Error;

   SetWindowText( s_MainWindow.hWnd, "NeuronStudio" );

   EnableMenuItems( s_MainWindow.hWnd, 0 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN, 1 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_FILE_OPEN_3D, 1 );   
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_ABOUT, 1 );
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_CONTENTS, 1 );
   //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_TOPICS, 1 );
   //SetMenuItemEnabled( s_MainWindow.hWnd, IDM_HELP_ABOUT, 1 );


	switch( ____initial_num_cpu )
		{
		case 1:  ____selected_num_cpu = IDM_RUN_NUM_CPU_1; break;
		case 2:  ____selected_num_cpu = IDM_RUN_NUM_CPU_2; break;
		case 4:  ____selected_num_cpu = IDM_RUN_NUM_CPU_4; break;
		case 8:  ____selected_num_cpu = IDM_RUN_NUM_CPU_8; break;
		default: ____selected_num_cpu = IDM_RUN_NUM_CPU_1; break;
		}

	____on_set_selected_num_cpu( ____selected_num_cpu );

   return eNO_ERROR;
   }


extern void _display_window_set_zoom_out( void );
extern void _display_window_set_zoom_in( void );




void _on_rays_handle_user_edit( nsint key )
   {
   NsSampler         *sampler;
   nspointer          sample;
   nsfloat            radius;
   NsVector3i         jitter;
   const NsSettings  *settings;
   const NsImage     *volume;
	nsboolean          rerun;
	NsError            error;


   sampler  = workspace_sampler( s_MainWindow.activeWorkspace );
   settings = workspace_settings( s_MainWindow.activeWorkspace );

   if( ! ns_sampler_is_empty( sampler ) )
      {
		rerun  = NS_TRUE;
      sample = ns_sampler_last( sampler );

      if( 0 < ns_sample_get_num_steps( sample ) )
         {
         switch( key )
            {
            case VK_LEFT:
               //ns_print( "L" );
               ns_sample_rotate_left( sample );
               break;

            case VK_RIGHT:
               //ns_print( "R" );
               ns_sample_rotate_right( sample );
               break;

            case VK_UP:
               //ns_print( "U" );
               ns_sample_translate_up( sample );
               break;

            case VK_DOWN:
               //ns_print( "D" );
               ns_sample_translate_down( sample );
               break;

				case VK_OEM_PLUS:
				case VK_ADD:
					rerun = NS_FALSE;

					ns_sample_change_radius(
						sample,
						105.0f,
						ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) )
						);
					break;

				case VK_OEM_MINUS:
				case VK_SUBTRACT:
					rerun = NS_FALSE;

					ns_sample_change_radius(
						sample,
						95.0f,
						ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) )
						);
					break;

            default:
               ns_assert_not_reached();
            }

         ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );
         //ns_println( "jitter=%d,%d,%d", jitter.x, jitter.y, jitter.z );

         volume = workspace_volume( s_MainWindow.activeWorkspace );

			if( rerun )
				{
				if( NS_SUCCESS(
						ns_sample_run(
							sample,
							volume,
							workspace_get_voxel_info( s_MainWindow.activeWorkspace ),
							workspace_get_average_intensity( s_MainWindow.activeWorkspace ),
							&jitter,
							//____use_2d_measurement_sampling,
							ns_settings_get_threshold_use_fixed( settings ),
							ns_settings_get_threshold_fixed_value( settings ),
							&radius
							),
						error ) )
					{  /*ns_println( "radius = " NS_FMT_DOUBLE, radius );*/  }
				else
					ns_println( "not enough memory to complete sample" );
				}
         }
		else
			{
			switch( key )
				{
				case VK_OEM_PLUS:
				case VK_ADD:
					rerun = NS_FALSE;

					ns_sample_change_radius(
						sample,
						105.0f,
						ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) )
						);
					break;

				case VK_OEM_MINUS:
				case VK_SUBTRACT:
					rerun = NS_FALSE;

					ns_sample_change_radius(
						sample,
						95.0f,
						ns_voxel_info_min_radius( workspace_get_voxel_info( s_MainWindow.activeWorkspace ) )
						);
					break;
				}
			}

		____redraw_all();
      }

	_status_measurement();
   }



//nsboolean ____control_key_is_down = NS_FALSE;



nsboolean _key_is_down( nsint key )
   {  return ( nsboolean )( GetAsyncKeyState( key ) & 0x8000 );  }



nsboolean ____alt_key_is_down = NS_FALSE;


extern nsboolean ____drag_rect_active;






void _on_create_or_delete_edge_between_two_vertices( void )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*/ns_model_selected_vertices_add_or_remove_edge( filtered_model, &list );
   /*error*/ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );

   ____redraw_all();
   }


/*
void _on_delete_edge_between_two_vertices( void )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*//*ns_model_selected_vertices_remove_edge( filtered_model, &list );
   /*error*//*ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );

   ____redraw_all();
   }
*/

/*
void _on_create_edge_between_two_vertices( void )
   {
   NsModel  *filtered_model, *raw_model;
   NsList    list;


   filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
   raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

   ns_list_construct( &list, NULL );

   /*error*//*ns_model_selected_vertices_add_edge( filtered_model, &list );
   /*error*//*ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

   ns_list_destruct( &list );

   ____redraw_all();
   }
*/


/*
void _on_set_marker_type_callback( NsSampler *sampler, nspointer sample, nsboolean *did_increment )
   {
   if( ns_sample_get_is_marker( sample ) && ns_sample_get_is_selected( sample ) )
      {
      if( ! (*did_increment) )
         {
         ____markers_type = ( ____markers_type + 1 ) % NS_SAMPLER_NUM_MARKS;
         *did_increment = NS_TRUE;
         }

      ns_sample_set_user_type( sample, ____markers_type );
      }
   }*/

/*
void _on_set_marker_type( void )
   {
   if( eDISPLAY_WINDOW_MOUSE_MODE_RAYS == s_CurrentMouseMode )
      {
      NsSampler  *sampler;
      nsboolean   did_increment;


      sampler       = workspace_sampler( s_MainWindow.activeWorkspace );
      did_increment = NS_FALSE;

      ns_sampler_foreach(
         sampler,
         _on_set_marker_type_callback,
         &did_increment
         );

      ____redraw_all();
      }
   }*/


extern void _ns_model_print_all_spines_spr_vs_exp_ratio( NsModel *model );

void _do_flash_frames( HWND );
void _do_flash_hidden( HWND );


void _on_test_create_trees( void )
	{
	NsModel *model;
	nsmodeltree curr, end;

	model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );
	ns_model_create_trees( model );

	curr = ns_model_begin_trees( model );
	end  = ns_model_end_trees( model );

	for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
		{
		ns_println( "TREE:" );
		ns_println( "\tconn_comp   = " NS_FMT_ULONG, ns_model_tree_conn_comp( curr ) );
		ns_println( "\ttotal_index = " NS_FMT_ULONG, ns_model_tree_total_index( curr ) );
		ns_println( "\tcell_index  = " NS_FMT_ULONG, ns_model_tree_cell_index( curr ) );
		ns_println( "\ttype_index  = " NS_FMT_ULONG, ns_model_tree_type_index( curr ) );
		ns_println( "\tfunc_type   = " NS_FMT_STRING, ns_model_function_type_to_string( ns_model_tree_function_type( curr ) ) );
		ns_println( "\tparent      = " NS_FMT_POINTER, ns_model_tree_parent( curr ) );
		ns_println( "\troot        = " NS_FMT_POINTER, ns_model_tree_root( curr ) );
		}
	}


extern nsboolean ____display_window_do_screen_capture;
extern nsboolean ____opengl_window_do_screen_capture;


void _opengl_begin_rotation( OpenGLWindow *ogl, void *args );
void _opengl_end_rotation( OpenGLWindow *ogl, void *args );


void _on_model_offset_xy_random( void )
	{
	NsConfigDb  db;
	nsfloat     max_xy_offset;
	NsError     error;


   if( NS_SUCCESS( ns_config_db_construct( &db ), error ) )
      {
      if( NS_SUCCESS( ns_config_db_read( &db, ____config_file, NULL ), error ) )
         if( ns_config_db_has_group( &db, "max-xy-offset" ) )
            if( ns_config_db_has_key( &db, "max-xy-offset", "value" ) )
               {
					max_xy_offset = ( nsfloat )ns_config_db_get_double( &db, "max-xy-offset", "value" );

					/*error*/
					ns_model_offset_xy_random(
						GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
						max_xy_offset
						);

					____redraw_all();
               }

      ns_config_db_destruct( &db );
      }
	}


void _on_spines_increment_or_decrement_id( nsint key )
	{
	nsboolean increment = NS_TRUE;

	switch( key )
		{
		case VK_OEM_PLUS:
		case VK_ADD:
			increment = NS_TRUE;
			break;

		case VK_OEM_MINUS:
		case VK_SUBTRACT:
			increment = NS_FALSE;
			break;

      default:
         ns_assert_not_reached();
		}

	ns_model_spines_offset_selected_spine_id(
		GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
		increment
		);

	____redraw_2d();
	}


nsint _main_window_on_keydown( nsint key )
   {
   if( ____drag_rect_active )
      return 0;

   switch( key )
      {
      case VK_NEXT:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) && ____xy_slice_enabled )
            _on_view_xy_slice_viewer_up();
         break;

      case VK_PRIOR:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) && ____xy_slice_enabled )
            _on_view_xy_slice_viewer_down();
         break;

      case VK_HOME:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) && ____xy_slice_enabled )
            _on_view_xy_slice_viewer_first();
         break;

      case VK_END:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) && ____xy_slice_enabled )
            _on_view_xy_slice_viewer_last();
         break;


      case VK_ESCAPE:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            _on_select_or_deselect_all( NS_FALSE );
         break;


      case 'O':
         if( ! WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( VK_CONTROL ) )
               _on_file_open();
            }
         break;

      case VK_LEFT:
      case VK_RIGHT:
      case VK_UP:
      case VK_DOWN:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( eDISPLAY_WINDOW_MOUSE_MODE_RAYS == s_CurrentMouseMode )
               _on_rays_handle_user_edit( key );
            else if( eDISPLAY_WINDOW_MOUSE_MODE_NEURITE == s_CurrentMouseMode ||
							DISPLAY_WINDOW_MOUSE_MODE_MAGNET == s_CurrentMouseMode )
               {
               if( VK_UP == key || VK_DOWN == key )
                  _on_neurite_change_radii( ( nsboolean )( VK_UP == key ) );
               }
				else if( eDISPLAY_WINDOW_MOUSE_MODE_SPINE == s_CurrentMouseMode )
					{
               if( VK_UP == key || VK_DOWN == key )
                  _on_spines_change_radii( ( nsboolean )( VK_UP == key ) );
					}
            }
         break;

		case VK_OEM_PLUS:
		case VK_OEM_MINUS:
		case VK_ADD:
		case VK_SUBTRACT:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				{
            if( eDISPLAY_WINDOW_MOUSE_MODE_RAYS == s_CurrentMouseMode )
               _on_rays_handle_user_edit( key );
				else if( eDISPLAY_WINDOW_MOUSE_MODE_SPINE == s_CurrentMouseMode )
					_on_spines_increment_or_decrement_id( key );
				}
			break;

      //case VK_CONTROL:
        // if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
          //  {
            //if( eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode )
            //   _display_window_set_zoom_to_translate();
            //}
         //break;

      /* NOTE: Return 1 so that the default Alt key action occurs. This
         usually will activate the main menu. */
      case VK_MENU:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            ____alt_key_is_down = NS_TRUE;

            if( eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode )
               _display_window_set_zoom_out();
            else
               return 1;
            }
         else
            return 1;
         break;

      case 'N':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( 'X' ) )
               _on_clear_neurites();
            else
               _MainWindowOnBuildNeuronTree();
            }
         break;


		case 'L':
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				{
				workspace_clear_grafter_voxels( s_MainWindow.activeWorkspace );
				____redraw_all();
				}
			break;


      case 'S':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( 'X' ) )
               _on_clear_spines();
            else if( _key_is_down( VK_CONTROL ) )
               _MainWindowOnFileSaveTree( NS_TRUE );
            else
               _main_window_on_run_spine_analysis();
            }
         break;

      case VK_DELETE:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _on_delete_selected();
            }
         break;

      case 'Z':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_ZOOM, eDISPLAY_WINDOW_MOUSE_MODE_ZOOM, 0 ); 
            }
         break;


      case 'I':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_MODE_SPINE, eDISPLAY_WINDOW_MOUSE_MODE_SPINE, 0 ); 
            }
         break;


      case 'U':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_MODE_NEURITE, eDISPLAY_WINDOW_MOUSE_MODE_NEURITE, 0 ); 
            }
         break;


      case 'R':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_MODE_ROI, eDISPLAY_WINDOW_MOUSE_MODE_ROI, 0 ); 
            }
         break;


      case 'G':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_MODE_MAGNET, DISPLAY_WINDOW_MOUSE_MODE_MAGNET, 0 ); 
            }
         break;


      case 'T':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, 0 );
            }
         break;

      case 'E':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            /*if( _key_is_down( 'X' ) )
               _on_clear_samples();
            else*/
               _MainWindowOnSetMouseMode( 0, IDM_MODE_SEED, eDISPLAY_WINDOW_MOUSE_MODE_SEED, 0 );
            }
         break;

      case 'M':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( 'X' ) )
               _on_clear_samples( NS_FALSE );/*_on_clear_model();*/
            else
               _MainWindowOnSetMouseMode( 0, IDM_MODE_RAYS, eDISPLAY_WINDOW_MOUSE_MODE_RAYS, 0 );
            }
         break;

      case 'F':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _MainWindowOnFitImages();
            }
         break;

      case 'A':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( 'X' ) )
               _on_clear_all();
            else if( _key_is_down( VK_CONTROL ) )
               _on_select_or_deselect_all( NS_TRUE );
            else
               _on_actual_pixels();
            }
         break;

      case 'V':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            if( _key_is_down( 'X' ) )
               _on_delete_selected_vertices();
            }
         break;

      case 'C':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _on_convert_measurements();
            }
         break;

      case 'J':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            _on_join_or_unjoin_selected();
         break;

      /*
      case 'K':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            _on_cut_selected();
         break;
      */

     // case VK_SPACE:
       //  if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
         //   _on_set_marker_type();
         //break;

      case VK_F1:
         _on_help_contents();
         break;

		case VK_F4:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				_do_flash_frames( s_MainWindow.hWnd );
			break;

		case VK_F3:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				_do_flash_hidden( s_MainWindow.hWnd );
			break;


/*
		case VK_F6:
			{
			NsSpinesPlugins scp;
			nschar dir[ NS_PATH_SIZE ];

			ns_spines_plugins_construct( &scp );
			ns_ascii_strcpy( dir, _startup_directory );
			ns_ascii_strcat( dir, "\\classifiers" );
			ns_spines_plugins_read( &scp, dir );
			ns_spines_plugins_print( &scp );
			ns_spines_plugins_destruct( &scp );
			}
			break;
*/

/*
		case VK_F6:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				{
				nschar path[ NS_PATH_SIZE ];

				ns_ascii_strcpy( path, _startup_directory );
				ns_ascii_strcat( path, "\\poster-comparison.txt" );

				ns_model_spines_do_retype_comparison(
					GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
					path,
					workspace_settings( s_MainWindow.activeWorkspace ),
					ns_spines_classifier_get()
					);
				}
			break;
*/
		/* TEMP!!!!!!!!!!!! */
		//case VK_F6:
		//	if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
		//		_on_model_offset_xy_random();
		//	break;


      case VK_F5:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            ____on_run_projection();
            }
         break;

		//case VK_F7:
			//____display_window_do_screen_capture = NS_TRUE;
			//____redraw_2d();
		//	____opengl_window_do_screen_capture = NS_TRUE;
		//	____redraw_3d();
		//	break;


		
      //   case VK_F7:
		//		if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
		//			 {
		//			 NsModel  *filtered_model, *raw_model;
		//			 NsList    list;


		//			 filtered_model = workspace_filtered_model( s_MainWindow.activeWorkspace );
		//			 raw_model      = workspace_raw_model( s_MainWindow.activeWorkspace );

		//			 ns_list_construct( &list, NULL );

		//			 /*error*/ns_model_reposition_selected_junction_vertices( filtered_model, &list );
		//			 /*error*/ns_model_remove_conn_comp_by_auto_iter_list( raw_model, &list );

		//			 ns_list_destruct( &list );

		//			 ____redraw_all();
		//			 }
		//		break;


      case VK_F8:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _on_run_filter_mp( NS_PIXEL_PROC_MP_FILTER_BLUR_MORE );
            }
         break;

		case VK_F9:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				workspace_accept_roi( s_MainWindow.activeWorkspace );
			break;

		case VK_F11:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				_opengl_begin_rotation( &s_MainWindow.ogl, NULL );
			break;

		case VK_F12:
			if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
				_opengl_end_rotation( &s_MainWindow.ogl, NULL );
			break;

		/*
		case VK_F7:
			{
			const NsImage *src;
			NsImage dest;
			NsError error;

			src = GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_XY );

			ns_image_construct( &dest );
			ns_image_convert( src, NS_PIXEL_RGB_U8_U8_U8, 1, &dest, NULL );
			ns_image_set_io_proc_db( &dest, workspace_io_proc_db( s_MainWindow.activeWorkspace ) );
			error = ns_image_write_jpeg( &dest, "C:\\usr\\doug\\projects\\ns\\test.jpg", 50, NULL );
			ns_println( NS_FMT_STRING, ns_error_code_to_string( error ) );
			ns_image_destruct( &dest );
			}
			break;
		*/


		//case VK_F7:
		//	if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
		//		_on_test_create_trees();
		//	break;
      
      //case VK_F7:
      //   if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
      //      _ns_model_print_all_spines_spr_vs_exp_ratio(
      //         GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
      //         );
      //   break;

/*TEMP!!!!!!!!!!!!!!!!!!*/
      /*
      case 'R':
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _ns_model_select_spines_by_count_and_spread_ratios(
               GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
               ____config_file );
            ____redraw_all();
            }
         break;
      */
      }

   return 0;
   }


extern void _display_windows_cancel_alt_drag( void );


nsint _main_window_on_keyup( nsint key )
   {
   switch( key )
      {
      case VK_MENU:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            {
            _display_windows_cancel_alt_drag();
				_ogl_window_cancel_alt_drag();           

            ____alt_key_is_down = NS_FALSE;

            if( eDISPLAY_WINDOW_MOUSE_MODE_ZOOM == s_CurrentMouseMode )
               _display_window_set_zoom_in();
            else
               return 1;
            }
         else
            return 1;
         break;
      }

   return 0;
   }


void _write_main_window_dimensions( HWND hWnd )
   {
   NsConfigDb  db;
   RECT        rc;
   nsboolean   is_iconic;
   nsboolean   is_zoomed;
   NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_SUCCESS( ns_config_db_read( &db, ____config_file, NULL ), error ) )
      if( ns_config_db_has_group( &db, "window" ) )
         {
         is_iconic = ( nsboolean )IsIconic( hWnd );
         is_zoomed = ( nsboolean )IsZoomed( hWnd );

         if( ! ( is_iconic || is_zoomed ) )
            {
            GetWindowRect( hWnd, &rc );

            if( ns_config_db_has_key( &db, "window", "left" ) )
               ns_config_db_set_int( &db, "window", "left", ( nsint )rc.left );

            if( ns_config_db_has_key( &db, "window", "right" ) )
               ns_config_db_set_int( &db, "window", "right", ( nsint )rc.right );

            if( ns_config_db_has_key( &db, "window", "top" ) )
               ns_config_db_set_int( &db, "window", "top", ( nsint )rc.top );

            if( ns_config_db_has_key( &db, "window", "bottom" ) )
               ns_config_db_set_int( &db, "window", "bottom", ( nsint )rc.bottom );
            }

         if( ns_config_db_has_key( &db, "window", "maximized" ) )
            ns_config_db_set_boolean( &db, "window", "maximized", is_zoomed );

         ns_config_db_write( &db, ____config_file );
         }

   ns_config_db_destruct( &db );
   }


extern void _display_window_on_mouse_wheel( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );




nsboolean ____flash_frames_active = NS_FALSE;
nsboolean ____flash_hidden_active = NS_FALSE;

#define   _FLASH_FRAMES_TIMER_ID   79
#define   _FLASH_HIDDEN_TIMER_ID   89

#define   _FLASH_FRAMES_TIME_OUT  500
#define   _FLASH_HIDDEN_TIME_OUT  500


void _main_window_on_timer( HWND wnd, nsint id )
	{
	if( wnd == s_MainWindow.hWnd )
		switch( id )
			{
			case _FLASH_FRAMES_TIMER_ID:
				KillTimer( wnd, id );

				____flash_frames_active = NS_FALSE;
				____redraw_all();
				break;

			case _FLASH_HIDDEN_TIMER_ID:
				KillTimer( wnd, id );

				____flash_hidden_active = NS_FALSE;
				____redraw_all();
				break;
			}
	}


void _do_flash_frames( HWND wnd )
	{
	if( wnd == s_MainWindow.hWnd && ! ____flash_frames_active )
		{
		/* Set the flag and redraw. The flag being active means to inhibit drawing
			of anything but the data. */
		____flash_frames_active = NS_TRUE;
		____redraw_all();

		/* In the unlikely case that creating a timer fails, then we need to
			clear the flag and redraw. */
		if( ! ( ( nsboolean )SetTimer( wnd, _FLASH_FRAMES_TIMER_ID, _FLASH_FRAMES_TIME_OUT, NULL ) ) )
			{
			____flash_frames_active = NS_FALSE;
			____redraw_all();
			}
		}
	}


void _do_flash_hidden( HWND wnd )
	{
	if( wnd == s_MainWindow.hWnd && ! ____flash_hidden_active )
		{
		____flash_hidden_active = NS_TRUE;
		____redraw_all();

		/* In the unlikely case that creating a timer fails, then we need to
			clear the flag and redraw. */
		if( ! ( ( nsboolean )SetTimer( wnd, _FLASH_HIDDEN_TIMER_ID, _FLASH_HIDDEN_TIME_OUT, NULL ) ) )
			{
			____flash_hidden_active = NS_FALSE;
			____redraw_all();
			}
		}
	}


LRESULT CALLBACK _MainWindowProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {

      case mMSG_USER_Update:
         ____redraw_2d();
         break;

      case _DEFINE_WM_MOUSE_WHEEL:
         _display_window_on_mouse_wheel( hWnd, uMsg, wParam, lParam );
         break;


      /*
      case WM_PAINT:
         {
         PAINTSTRUCT ps;
         HDC         hDC;

         hDC = BeginPaint( hWnd, &ps );
         _MainWindowOnPaint( hWnd, hDC );
         EndPaint( hWnd, &ps );
         }
         break;
      */

      case WM_COMMAND:
         _MainWindowOnCommand( LOWORD( wParam ), HIWORD( wParam ) );
         break;


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



      case WM_GETMINMAXINFO:
         _MainWindowOnGetMinMaxInfo( ( SIZE *const )
                                         ( &( ( ( LPMINMAXINFO )lParam )->ptMinTrackSize ) ),
                                         ( SIZE *const )
                                         ( &( ( ( LPMINMAXINFO )lParam )->ptMaxTrackSize ) )
                                    );
         break;

      case WM_SIZE:
         _MainWindowOnSize( wParam, lParam );
         ResizeStatusBar( wParam, lParam );
         break;


		case WM_TIMER:
			_main_window_on_timer( hWnd, ( nsint )wParam );
			break;


      case WM_CREATE:
         s_MainWindow.hWnd = hWnd;
         if( eNO_ERROR != _MainWindowOnCreate() )
            return -1;
         break;

      case WM_DESTROY:
         _write_main_window_dimensions( hWnd );
         break;

      case WM_NCDESTROY:
         if( WorkspaceIsValid( s_MainWindow.activeWorkspace ) )
            DestructWorkspace( s_MainWindow.activeWorkspace );
         //____spines_destruct();
         PostQuitMessage( 0 );
         break;

      default:
         return DefWindowProc( hWnd, uMsg, wParam, lParam );
      }

   return 0;
   }





#define _OPENGL_LOWEST_COMPLEXITY     2
#define _OPENGL_LOW_COMPLEXITY        4
#define _OPENGL_MEDIUM_COMPLEXITY     8
#define _OPENGL_HIGH_COMPLEXITY      12
#define _OPENGL_HIGHEST_COMPLEXITY   16


#define _OPENGL_CYLINDER_COMPLEXITY  ( s_MainWindow.complexity )
#define _OPENGL_SPHERE_COMPLEXITY    ( s_MainWindow.complexity )


void _OpenGLViewTransform( OpenGLWindow *const ogl, void *userData )
   {
   //gluLookAt( ogl->xEye, ogl->yEye, ogl->zEye, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );

   /* NOTE: OpenGL lights are transformed by the modelview matrix ( not the projection matrix )
      but we want the light to remain fixed in its position in world coordinates so therefore we
      specify its position before any modeling transformations are made. */

   ApplyOpenGLLight( &s_MainWindow.light0 );
   }


void _OpenGLRenderAxis( OpenGLWindow *const ogl, nsfloat dx, nsfloat dy, nsfloat dz )
   {
   glBegin( GL_LINES );

   glColor3f( 1.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( dx, 0.0f, 0.0f );

   glColor3f( 0.0f, 1.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, dy, 0.0f );

   glColor3f( 0.0f, 0.0f, 1.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, dz );

   glEnd();



/*
   glBegin( GL_LINES );

   glColor3f( 1.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( dx, 0.0f, 0.0f );

   glColor3f( 0.0f, 1.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, dy, 0.0f );

   glColor3f( 0.0f, 0.0f, 1.0f );
   glVertex3f( 0.0f, 0.0f, 0.0f );
   glVertex3f( 0.0f, 0.0f, dz );

   glEnd();

   glDisable( GL_CULL_FACE );

   glBegin( GL_TRIANGLES );

   glColor3f( 1.0f, 0.0f, 0.0f );
   glVertex3f( dx + 4.0f, 0.0f, 0.0f );
   glVertex3f( dx - 1.0f, 2.0f, 0.0f );
   glVertex3f( dx - 1.0f, -2.0f, 0.0f );

   glColor3f( 0.0f, 1.0f, 0.0f );
   glVertex3f( 0.0f, dy + 4.0f, 0.0f ); 
   glVertex3f( 2.0f, dy - 1.0f, 0.0f );
   glVertex3f( -2.0f, dy - 1.0f, 0.0f );

   glColor3f( 0.0f, 0.0f, 1.0f );
   glVertex3f( 0.0f, 0.0f, dz + 4.0f );
   glVertex3f( 0.0f, 2.0f, dz - 1.0f );
   glVertex3f( 0.0f, -2.0f, dz - 1.0f );

   glEnd();

   glEnable( GL_CULL_FACE );
*/
   }


static GLuint s_OpenGLRenderBoundingCubeEdges[32] =
   {
   0, 1, 1, 3, 3, 2, 2, 0,
   1, 3, 3, 7, 7, 5, 5, 1,
   5, 4, 4, 6, 6, 7, 7, 5,
   4, 0, 0, 2, 2, 6, 6, 4
   };

static GLuint s_OpenGLRenderBoundingCubeFaces[32] =
   {
   0, 2, 3, 1,
   1, 3, 7, 5,
   5, 7, 6, 4,
   4, 6, 2, 0,
   4, 0, 1, 5,
   2, 6, 7, 3
   };

static GLfloat s_OpenGLRenderBoundingCubeVertices[8][3];

#define _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( i, x, y, z )\
   s_OpenGLRenderBoundingCubeVertices[ ( i ) ][ 0 ] = ( x );\
   s_OpenGLRenderBoundingCubeVertices[ ( i ) ][ 1 ] = ( y );\
   s_OpenGLRenderBoundingCubeVertices[ ( i ) ][ 2 ] = ( z )


void _opengl_line_func( nspointer user_data, const NsVector3d *V1, const NsVector3d *V2 )
   {
   glVertex3dv( ns_vector3d_const_array( V1 ) );
   glVertex3dv( ns_vector3d_const_array( V2 ) );
   }


void _render_roi_bounding_box( OpenGLWindow *const ogl, nsint transparent, const NsColor4f *color )
	{
   const NsVoxelInfo *voxel_info;
	const NsCubei *roi;
	NsVector3f C1, C2;


	roi        = workspace_visual_roi( s_MainWindow.activeWorkspace );
   voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

	/* NOTE: +1 on C2 since we want to include the volume of the voxels on those edges. */

	C1.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.x, voxel_info, NS_COMPONENT_X );
	C1.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.y, voxel_info, NS_COMPONENT_Y );
	C1.z = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.z, voxel_info, NS_COMPONENT_Z );
	C2.x = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.x + 1 ), voxel_info, NS_COMPONENT_X );
	C2.y = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.y + 1 ), voxel_info, NS_COMPONENT_Y );
	C2.z = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.z + 1 ), voxel_info, NS_COMPONENT_Z );

   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, C1.x,  C1.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1, C2.x,  C1.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, C1.x,  C2.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3, C2.x,  C2.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, C1.x,  C1.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5, C2.x,  C1.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, C1.x,  C2.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7, C2.x,  C2.y,  C2.z );

	glDepthMask( GL_FALSE );

   glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, ( const GLvoid* )s_OpenGLRenderBoundingCubeVertices );

   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glColor4f( color->x, color->y, color->z, color->w );   

   glDrawElements( GL_QUADS,
                   mARRAY_LENGTH( s_OpenGLRenderBoundingCubeFaces ),
                   GL_UNSIGNED_INT,
                   ( const GLvoid* )s_OpenGLRenderBoundingCubeFaces
                 );

   glDisable( GL_BLEND );

   glDisableClientState( GL_VERTEX_ARRAY );

	glDepthMask( GL_TRUE );
	}


void _render_roi_bounding_box_edges( OpenGLWindow *const ogl, const NsColor4f *color )
	{
   const NsVoxelInfo *voxel_info;
	const NsCubei *roi;
	NsVector3f C1, C2;


	roi        = workspace_visual_roi( s_MainWindow.activeWorkspace );
   voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

	/* NOTE: +1 on C2 since we want to include the volume of the voxels on those edges. */

	C1.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.x, voxel_info, NS_COMPONENT_X );
	C1.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.y, voxel_info, NS_COMPONENT_Y );
	C1.z = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.z, voxel_info, NS_COMPONENT_Z );
	C2.x = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.x + 1 ), voxel_info, NS_COMPONENT_X );
	C2.y = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.y + 1 ), voxel_info, NS_COMPONENT_Y );
	C2.z = ns_to_voxel_space_component_ex( ( nsfloat )( roi->C2.z + 1 ), voxel_info, NS_COMPONENT_Z );

   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, C1.x,  C1.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1, C2.x,  C1.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, C1.x,  C2.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3, C2.x,  C2.y,  C1.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, C1.x,  C1.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5, C2.x,  C1.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, C1.x,  C2.y,  C2.z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7, C2.x,  C2.y,  C2.z );

   glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, ( const GLvoid* )s_OpenGLRenderBoundingCubeVertices );

   glColor4f( color->x, color->y, color->z, color->w );   

   glDrawElements( GL_LINES,
                   mARRAY_LENGTH( s_OpenGLRenderBoundingCubeEdges ),
                   GL_UNSIGNED_INT,
                   ( const GLvoid* )s_OpenGLRenderBoundingCubeEdges
                 );

   glDisableClientState( GL_VERTEX_ARRAY );
	}


void _open_gl_render_bounding_cube_edges
   (
   OpenGLWindow *const ogl, nsfloat dx, nsfloat dy, nsfloat dz,
   const NsColor4f *color )
	{
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, 0.0f,  0.0f,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1,   dx,  0.0f,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, 0.0f,    dy,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3,   dx,    dy,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, 0.0f,  0.0f,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5,   dx,  0.0f,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, 0.0f,    dy,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7,   dx,    dy,    dz );

   glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, ( const GLvoid* )s_OpenGLRenderBoundingCubeVertices );

   glColor4f( color->x, color->y, color->z, color->w );   

   glDrawElements( GL_LINES,
                   mARRAY_LENGTH( s_OpenGLRenderBoundingCubeEdges ),
                   GL_UNSIGNED_INT,
                   ( const GLvoid* )s_OpenGLRenderBoundingCubeEdges
                 );

   glDisableClientState( GL_VERTEX_ARRAY );
	}


void _OpenGLRenderBoundingCube
   (
   OpenGLWindow *const ogl, nsfloat dx, nsfloat dy, nsfloat dz, nsint transparent,
   const NsColor4f *color )
   {
   //NsAABBox3d B;
   //NsVector3d L;


   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, 0.0f,  0.0f,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1,   dx,  0.0f,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, 0.0f,    dy,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3,   dx,    dy,  0.0f );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, 0.0f,  0.0f,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5,   dx,  0.0f,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, 0.0f,    dy,    dz );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7,   dx,    dy,    dz );

	glDepthMask( GL_FALSE );

   glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, ( const GLvoid* )s_OpenGLRenderBoundingCubeVertices );

   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glColor4f( color->x, color->y, color->z, color->w );   

   glDrawElements( GL_QUADS,
                   mARRAY_LENGTH( s_OpenGLRenderBoundingCubeFaces ),
                   GL_UNSIGNED_INT,
                   ( const GLvoid* )s_OpenGLRenderBoundingCubeFaces
                 );

   glDisable( GL_BLEND );



   glDisableClientState( GL_VERTEX_ARRAY );

	glDepthMask( GL_TRUE );

/*
   ns_aabbox3d( &B, 0.0, 0.0, 0.0, dx, -dy, -dz );
   ns_vector3d( &L, 10.0, -10.0, -10.0 );

   glBegin( GL_LINES );
   glColor3f( 1.0f, 1.0f, 1.0f );
   ns_aabbox3d_render_ex( &B, _opengl_line_func, &L, NULL );
   glEnd();*/
   }


void _opengl_render_xy_slice( OpenGLWindow *ogl, /*nsfloat dx, nsfloat dy,*/ const NsColor4f *color )
   {
   GLfloat z;
   const NsVoxelInfo *voxel_info;
	const NsCubei *roi;
	NsVector2f C1, C2;


	roi        = workspace_visual_roi( s_MainWindow.activeWorkspace );
   voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

	C1.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.x, voxel_info, NS_COMPONENT_X );
	C1.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.y, voxel_info, NS_COMPONENT_Y );
	C2.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C2.x, voxel_info, NS_COMPONENT_X );
	C2.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C2.y, voxel_info, NS_COMPONENT_Y );

   z = ns_to_voxel_space_component_ex(
         ( nsfloat )____xy_slice_index,
         voxel_info,
         NS_COMPONENT_Z
         );

   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, C1.x,  C1.y,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1, C2.x,  C1.y,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, C1.x,  C2.y,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3, C2.x,  C2.y,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, C1.x,  C1.y,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5, C2.x,  C1.y,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, C1.x,  C2.y,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7, C2.x,  C2.y,  z + ns_voxel_info_size_z( voxel_info ) );


	/*
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 0, 0.0f,  0.0f,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 1,   dx,  0.0f,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 2, 0.0f,    dy,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 3,   dx,    dy,  z );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 4, 0.0f,  0.0f,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 5,   dx,  0.0f,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 6, 0.0f,    dy,  z + ns_voxel_info_size_z( voxel_info ) );
   _mOPENGL_RENDER_BOUNDING_CUBE_VERTEX( 7,   dx,    dy,  z + ns_voxel_info_size_z( voxel_info ) );
	*/

	glDepthMask( GL_FALSE );

   glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, ( const GLvoid* )s_OpenGLRenderBoundingCubeVertices );

   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glColor4f( color->x, color->y, color->z, color->w );   

   glDrawElements( GL_QUADS,
                   mARRAY_LENGTH( s_OpenGLRenderBoundingCubeFaces ),
                   GL_UNSIGNED_INT,
                   ( const GLvoid* )s_OpenGLRenderBoundingCubeFaces
                 );

   glDisable( GL_BLEND );

   glDisableClientState( GL_VERTEX_ARRAY );

	glDepthMask( GL_TRUE );
   }


extern nsboolean ____grafting;

extern NsColor4ub *____order_colors;
extern NsColor4ub *____section_colors;


void _aabbox3d_render( const NsOctreeNode *node, nspointer user_data )
   {
   nsboolean do_draw = NS_TRUE;

   ns_assert( ns_octree_node_is_leaf( node ) );

   if( 0 < ns_octree_node_num_objects( node ) )
      {
      if( IDM_GL_VIEW_OCTREE_EMPTY_LEAFS == s_MainWindow.octree_mode )
         do_draw = NS_FALSE;
      }
   else
      {
      if( IDM_GL_VIEW_OCTREE_NON_EMPTY_LEAFS == s_MainWindow.octree_mode )
         do_draw = NS_FALSE;
      }

   if( do_draw )
      {
      NsVector3d corners[8];
      //nsvectoriter curr, end;
      //nsmodeledge edge;
      //NsVector3f V1, V2, P1, P2;
      const NsAABBox3d *box;
      //NsVector3d P;


      box = ns_octree_node_aabbox( node );

      //V1.x = ( nsfloat )( ns_aabbox3d_left( box ) + ns_aabbox3d_width( box )  / 2.0 );
      //V1.y = ( nsfloat )( ns_aabbox3d_top( box )  - ns_aabbox3d_height( box ) / 2.0 );
      //V1.z = ( nsfloat )( ns_aabbox3d_near( box ) - ns_aabbox3d_length( box ) / 2.0 );

      //ns_vector3d( &P, 873.0, 328.0, 0.0 );

      //if( ns_point3d_inside_aabbox( &P, box ) )
      //   {
         //ns_println( "%p", node );

      ns_aabbox3d_corners( box, corners );

      //glColor3f( 0.0f, 0.0f, 0.0f );

      glVertexPointer( 3, GL_DOUBLE, 0, corners );

      glDrawElements( GL_LINES,
                      mARRAY_LENGTH( s_OpenGLRenderBoundingCubeEdges ),
                      GL_UNSIGNED_INT,
                      ( const GLvoid* )s_OpenGLRenderBoundingCubeEdges
                    );

      //glDrawElements( GL_QUADS,
      //                mARRAY_LENGTH( s_OpenGLRenderBoundingCubeFaces ),
      //                GL_UNSIGNED_INT,
      //                ( const GLvoid* )s_OpenGLRenderBoundingCubeFaces
      //              );

/*
         glBegin( GL_LINES );
         glColor3f( 1.0f, 0.0f, 0.0f );

         curr = ns_octree_node_begin_objects( node );
         end  = ns_octree_node_end_objects( node );

         for( ; ns_vector_iter_not_equal( curr, end ); curr = ns_vector_iter_next( curr ) )
            {
            edge = ( ( const NsOctreeObject* )ns_vector_iter_get_object( curr ) )->data;

            ns_model_vertex_get_position( ns_model_edge_src_vertex( edge ), &P1 );
            ns_model_vertex_get_position( ns_model_edge_dest_vertex( edge ), &P2 );

            ns_vector3f_add( &V2, &P1, &P2 );
            ns_vector3f_cmpd_scale( &V2, 0.5f );

            glVertex3fv( ns_vector3f_const_array( &V1 ) );
            glVertex3fv( ns_vector3f_const_array( &V2 ) );
            }

         glEnd();*/
         //}
      }
   }






#include <std/nsfps.h>

NsFramesPerSecond ____fps;
nschar ____fps_string[64];


NsSplats ____splats;
nsboolean ____splats_init = NS_FALSE;

nsfloat ____splats_alpha = 1.0f;

nsint ____splats_render_type = NS_SPLATS_RENDER_TEXTURES;

nsboolean ____splats_clip = NS_FALSE;

nsboolean ____splats_is_volume = NS_FALSE;

nsboolean ____splats_all_layers = NS_FALSE;

nsint ____rotation_speed = 1;


extern nsboolean __progress_cancelled( NsProgress* );
extern void __progress_update( NsProgress*, nsfloat );
extern void __progress_set_title( NsProgress*, const nschar* );



NS_PRIVATE NsVector3f ____opengl_roi_center;


#include <ext/naas.h>

void _do_create_splats( void *dialog )
   {
   NsProgress nsprogress;
   Progress progress;
   NsError error;
   NsConfigDb db;
   nsint splats_create;
	const NsCubei *roi;
	const NsVoxelInfo *voxel_info;
   nstimer start, stop;
	NsVector3f C1, C2;


	g_Progress = dialog;
   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );


	if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   /*
   if( ns_config_db_has_group( &db, "rotation" ) )
      if( ns_config_db_has_key( &db, "rotation", "speed" ) )
            ____rotation_speed = ns_config_db_get_int( &db, "rotation", "speed" );
   */

/*TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*
   if( ns_config_db_has_group( &db, "naas" ) )
      {
      ____naas_dim1 =
         ns_config_db_has_key( &db, "naas", "dim1" ) ?
            ns_config_db_get_int( &db, "naas", "dim1" ) : 0;

      ____naas_dim2 =
         ns_config_db_has_key( &db, "naas", "dim2" ) ?
            ns_config_db_get_int( &db, "naas", "dim2" ) : 0;

      ____naas_dim3 =
         ns_config_db_has_key( &db, "naas", "dim3" ) ?
            ns_config_db_get_int( &db, "naas", "dim3" ) : 0;

      ____naas_center.x =
         ns_config_db_has_key( &db, "naas", "center_x" ) ?
            ns_config_db_get_int( &db, "naas", "center_x" ) : 0;

      ____naas_center.y =
         ns_config_db_has_key( &db, "naas", "center_y" ) ?
            ns_config_db_get_int( &db, "naas", "center_y" ) : 0;

      ____naas_center.z =
         ns_config_db_has_key( &db, "naas", "center_z" ) ?
            ns_config_db_get_int( &db, "naas", "center_z" ) : 0;

      ____naas_xdir =
         ns_config_db_has_key( &db, "naas", "xdir" ) ?
            ( nsfloat )ns_config_db_get_double( &db, "naas", "xdir" ) : 0.0f;

      ____naas_ydir =
         ns_config_db_has_key( &db, "naas", "ydir" ) ?
            ( nsfloat )ns_config_db_get_double( &db, "naas", "ydir" ) : 0.0f;

      ____naas_zdir =
         ns_config_db_has_key( &db, "naas", "zdir" ) ?
            ( nsfloat )ns_config_db_get_double( &db, "naas", "zdir" ) : 0.0f;
      }

   ns_println( "dim1     = " NS_FMT_INT, ____naas_dim1 );
   ns_println( "dim2     = " NS_FMT_INT, ____naas_dim2 );
   ns_println( "dim3     = " NS_FMT_INT, ____naas_dim3 );
   ns_println( "center.x = " NS_FMT_INT, ____naas_center.x );
   ns_println( "center.y = " NS_FMT_INT, ____naas_center.y );
   ns_println( "center.z = " NS_FMT_INT, ____naas_center.z );
   ns_println( "xdir     = " NS_FMT_DOUBLE, ____naas_xdir );
   ns_println( "ydir     = " NS_FMT_DOUBLE, ____naas_ydir );
   ns_println( "zdir     = " NS_FMT_DOUBLE, ____naas_zdir );

   ns_free( ____naas_points );

   ____naas_points =
      get_non_axis_aligned_set(
         ____naas_dim1,
         ____naas_dim2,
         ____naas_dim3,
         ____naas_xdir,
         ____naas_ydir,
         ____naas_zdir,
         &____naas_num_points,
         ____naas_corners
         );*/

   if( ns_config_db_has_group( &db, "splats" ) )
      {
      if( ns_config_db_has_key( &db, "splats", "create" ) )
         splats_create = ns_config_db_get_int( &db, "splats", "create" );
      else
         splats_create = 0;

      //if( ns_config_db_has_key( &db, "splats", "alpha" ) )
        // ____splats_alpha = ( nsfloat )ns_config_db_get_double( &db, "splats", "alpha" );
      //else
        // ____splats_alpha = 1.0f;

      if( ns_config_db_has_key( &db, "splats", "type" ) )
         ____splats_render_type = ns_config_db_get_int( &db, "splats", "type" );
      else
         ____splats_render_type = NS_SPLATS_RENDER_TEXTURES;

      if( ns_config_db_has_key( &db, "splats", "clip" ) )
         ____splats_clip = ns_config_db_get_boolean( &db, "splats", "clip" );
      else
         ____splats_clip = NS_FALSE;      

      if( ns_config_db_has_key( &db, "splats", "all_layers" ) )
         ____splats_all_layers = ns_config_db_get_boolean( &db, "splats", "all_layers" );
      else
         ____splats_all_layers = NS_FALSE;

		if( ____splats_need_config )
			{
			#define _SPLATS_DEFAULT_THRESHOLD  20

			if( ns_config_db_has_key( &db, "splats", "default_threshold" ) )
				____splats_default_threshold = ns_config_db_get_int( &db, "splats", "default_threshold" );
			else
				____splats_default_threshold = _SPLATS_DEFAULT_THRESHOLD;

			if( ns_config_db_has_key( &db, "splats", "current_threshold" ) )
				____splats_current_threshold = ns_config_db_get_int( &db, "splats", "current_threshold" );
			else
				____splats_current_threshold = _SPLATS_DEFAULT_THRESHOLD;
			}
      }

   ns_config_db_destruct( &db );

	____splats_need_config = NS_FALSE;

	roi        = workspace_visual_roi( s_MainWindow.activeWorkspace );
	voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

	C1.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.x, voxel_info, NS_COMPONENT_X );
	C1.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.y, voxel_info, NS_COMPONENT_Y );
	C1.z = ns_to_voxel_space_component_ex( ( nsfloat )roi->C1.z, voxel_info, NS_COMPONENT_Z );
	C2.x = ns_to_voxel_space_component_ex( ( nsfloat )roi->C2.x, voxel_info, NS_COMPONENT_X );
	C2.y = ns_to_voxel_space_component_ex( ( nsfloat )roi->C2.y, voxel_info, NS_COMPONENT_Y );
	C2.z = ns_to_voxel_space_component_ex( ( nsfloat )roi->C2.z, voxel_info, NS_COMPONENT_Z );

	____opengl_roi_center.x = C1.x + ( C2.x - C1.x ) / 2.0f;
	____opengl_roi_center.y = C1.y + ( C2.y - C1.y ) / 2.0f;
	____opengl_roi_center.z = C1.z + ( C2.z - C1.z ) / 2.0f;


ns_println( "splats_create = " NS_FMT_INT, splats_create );

start = ns_timer();

   if( ____splats_generate_from_spine_voxels )
      {
      if( NS_SUCCESS( ns_splats_create_by_table(
                        &____splats,
                        workspace_volume( s_MainWindow.activeWorkspace ),
								roi,
                        ns_model_spines_voxel_table( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) ),
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_XY ),
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_ZY ),
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_XZ ),
                        splats_create,
                        ____splats_is_volume,
                        &nsprogress,
								NULL
                        ),
                        error ) )
         ____splats_init = NS_TRUE;
      }
   else
      {
      if( NS_SUCCESS( ns_splats_create/*_ex*/(
                        &____splats,
                        workspace_volume( s_MainWindow.activeWorkspace ),
								roi,
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_XY ),
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_ZY ),
                        GetWorkspaceDisplayImage( s_MainWindow.activeWorkspace, NS_XZ ),
                        ns_ceil( workspace_get_average_intensity( s_MainWindow.activeWorkspace ) ),
                        splats_create,
                        ____splats_is_volume,
                        ____splats_current_threshold,
                        &nsprogress,
								NULL
								/*,
                        ____splats_all_layers ?
                           ns_model_spines_all_layers_color :
                           ns_model_spines_render_layer_color,
                        GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )*/
                        ),
                        error ) )
         ____splats_init = NS_TRUE;
      }
 
   if( ns_is_error( error ) )
      ns_println( "couldnt allocate splats!" );

stop = ns_timer();

ns_println( "time to create splats = " NS_FMT_DOUBLE " seconds.", ns_difftimer( stop, start ) );

   EndProgressDialog( dialog, 0 );
   }


void _create_splats( OpenGLWindow *ogl, nspointer user_data )
   {
	if( ____splats_generate )
		{
		nsint wasCancelled;

		ns_assert( ! ____splats_init );

		EnableWindow( ogl->hOwner, FALSE );
		ProgressDialog( /*s_MainWindow.*/ogl->hWnd, NULL, _do_create_splats, NULL, NULL, &wasCancelled );
		EnableWindow( ogl->hOwner, TRUE );

		____redraw_3d();
		}
   }


void _destroy_splats( void )
   {
   ____splats_init = NS_FALSE;
   ns_splats_clear( &____splats );
   }


/*
void _write_splats_alpha( void )
   {
   NsConfigDb db;

   if( NS_FAILURE( ns_config_db_construct( &db );
   ns_config_db_read( &db, ____config_file, NULL );

   ns_config_db_set_double( &db, "splats", "alpha", ____splats_alpha );

   ns_config_db_write( &db, ____config_file );
   ns_config_db_destruct( &db );
   }

void _read_splats_alpha( void )
   {

   NsConfigDb db;

   if( NS_FAILURE( ns_config_db_construct( &db );
   ns_config_db_read( &db, ____config_file, NULL );

   ____splats_alpha = ( nsfloat )ns_config_db_get_double( &db, "splats", "alpha" );

   ns_config_db_destruct( &db );
   }
*/


#include <ext/background.inl>

void _clear_background( void )
   {
   NsVector4ub color;
   nsfloat bk_color[3];


   color = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_BACKGROUND );

   bk_color[0] = NS_COLOR_GET_RED_F( color );
   bk_color[1] = NS_COLOR_GET_GREEN_F( color );
   bk_color[2] = NS_COLOR_GET_BLUE_F( color );

   if( ____solid_background )
      {
      glClearColor( bk_color[0], bk_color[1], bk_color[2], 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      }
   else
      {
      if( s_MainWindow.lightingOn )
         {
         GLfloat specular[4]={0.0f,0.0f,0.0f,1.0f};
         GLfloat shininess[1]={0.0f};

         glEnable( GL_NORMALIZE );
         glEnable( GL_LIGHTING );
         glEnable( GL_LIGHT0 );

         glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specular );
         glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, shininess );
         }

      ClearWithGradient( bk_color );

      if( s_MainWindow.lightingOn )
         {
         glDisable( GL_NORMALIZE );
         glDisable( GL_LIGHT0 );
         glDisable( GL_LIGHTING );
         }
      }
   }


void _opengl_enable_line_anti_aliasing( void )
   {
   glEnable( GL_LINE_SMOOTH );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glHint( GL_LINE_SMOOTH, GL_DONT_CARE );
   }


void _opengl_disable_line_anti_aliasing( void )
   {
   glDisable( GL_BLEND );
   glDisable( GL_LINE_SMOOTH );
   }


void _opengl_enable_point_anti_aliasing( void )
   {
   glEnable( GL_POINT_SMOOTH );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glHint( GL_POINT_SMOOTH, GL_DONT_CARE );
   }


void _opengl_disable_point_anti_aliasing( void )
   {
   glDisable( GL_BLEND );
   glDisable( GL_POINT_SMOOTH );
   }





extern void OpenGLWindowDrawAxis( OpenGLWindow *const ogl, int n );
extern void OpenGLWindowDrawRevolution( OpenGLWindow *const ogl, float *radii, int n, int complexity );

nsfloat ____radii[64];
nssize ____num_radii = 0;


void ____read_axis_of_revolution( void )
   {
   FILE    *fp;
   nschar   line[ 512 ];
   nschar   path[ NS_PATH_SIZE ];


   ns_sprint( path, "%s\\%s", _startup_directory, "radii.txt" );

   if( NULL != ( fp = fopen( path, "r" ) ) )
      {
      while( NULL != fgets( line, NS_ARRAY_LENGTH( line ), fp ) )
         {
         sscanf( line, "%f", ____radii + ____num_radii );
         ____radii[ ____num_radii ] /= 2.0f;
         ++____num_radii;
         }

      fclose( fp );
      }
   }


/*
void ____axis_of_revolution( OpenGLWindow *ogl )
   {
   if( 0 == ____num_radii )
      ____read_axis_of_revolution();

   if( 0 < ____num_radii )
      {
      glPushMatrix();
      glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );

      glColor3ub( 255, 0, 0 );
      glDisable( GL_LIGHTING );
      OpenGLWindowDrawAxis( ogl, ( int )____num_radii );
      glEnable( GL_LIGHTING );

      glDisable( GL_CULL_FACE );
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

      glColor4ub( 0, 255, 0, 128 );
      OpenGLWindowDrawRevolution(
         ogl,
         ____radii,
         ( int )____num_radii,
         32
         );

      glDisable( GL_BLEND );
      glEnable( GL_CULL_FACE );

      glPopMatrix();
      }
   }
*/


/*
#include <std/nsthread.h>
#include <std/nsmutex.h>

NsMutex ____rayburst_mutex;
NsRayburst *____rayburst_struct;
nsboolean ____rayburst_running = NS_FALSE;


void _opengl_rayburst_redraw( const NsRayburst *not_used1, nspointer not_used2 )
   {
   RepaintOpenGLWindow( &s_MainWindow.ogl );
   ns_thread_sleep( 10 );
   }


void _opengl_rayburst_thread( void *args )
   {
   NsVector3f P;

   ns_mutex_lock( &____rayburst_mutex );

   if( NULL != ____rayburst_struct )
      {
      ns_rayburst_destruct( ____rayburst_struct );
      ns_free( ____rayburst_struct );
      }

   ____rayburst_struct = ns_malloc( sizeof( NsRayburst ) );

   ns_rayburst_construct(
      ____rayburst_struct,
      ____measuring_rays_kernel_type,
      NS_RAYBURST_RADIUS_NONE,
      NS_RAYBURST_INTERP_BILINEAR,
      &____image,
      workspace_get_voxel_info( s_MainWindow.activeWorkspace ),
      NULL
      );

   ns_rayburst_set_threshold( ____rayburst_struct, 57.0f );
   ____rayburst_struct->render_func = _opengl_rayburst_redraw;

   ns_mutex_unlock( &____rayburst_mutex );

   ____rayburst_running = NS_TRUE;
   ns_rayburst( ____rayburst_struct, ns_vector3f( &P, 51.0f, 74.0f, 86.0f ) );
   ____rayburst_running = NS_FALSE;
   }


void _opengl_rayburst_animation( OpenGLWindow *ogl, void *user_data )
   {
   NsThread t;

   ns_thread_construct( &t );
   ns_thread_run( &t, _opengl_rayburst_thread, NULL );
   ns_thread_destruct( &t );
   }


void _opengl_do_render_rayburst( const NsRayburst *rayburst, nsboolean running )
   {
   const NsVector3f         *vectors;
   const NsIndexTriangleus  *triangles;
   nssize                    num_triangles;
   const NsRayburstSample   *samples;
   nssize                    num_samples;
   NsTriangle3f              T;
   nssize                    i;


   if( NULL == ( samples = _ns_rayburst_samples( rayburst ) ) )
      return;

   glColor3ub( 255, 0, 0 );

   num_samples = ns_rayburst_num_samples( rayburst );

   if( running )
      {
      glBegin( GL_LINES );

      for( i = 0; i < num_samples; ++i )
         {
         glVertex3fv( ns_vector3f_const_array( &(rayburst->origin) ) );
         glVertex3fv( ns_vector3f_const_array( &(samples[i].fwd.curr) ) );
         }

      glEnd();
      }
   else
      {
      vectors       = ns_rayburst_vectors( rayburst );
      triangles     = ns_rayburst_triangles( rayburst );
      num_triangles = ns_rayburst_num_triangles( rayburst );

      glBegin( GL_LINES );

      for( i = 0; i < num_triangles; ++i )
         {
         T.A = vectors[ triangles[i].a ];
         T.B = vectors[ triangles[i].b ];
         T.C = vectors[ triangles[i].c ];

         ns_vector3f_cmpd_scale( &T.A, samples[ triangles[i].a ].distance );
         ns_vector3f_cmpd_scale( &T.B, samples[ triangles[i].b ].distance );
         ns_vector3f_cmpd_scale( &T.C, samples[ triangles[i].c ].distance );

         ns_vector3f_cmpd_add( &T.A, &(rayburst->origin) );
         ns_vector3f_cmpd_add( &T.B, &(rayburst->origin) );
         ns_vector3f_cmpd_add( &T.C, &(rayburst->origin) );

         glVertex3fv( ns_vector3f_const_array( &T.A ) );
         glVertex3fv( ns_vector3f_const_array( &T.B ) );

         glVertex3fv( ns_vector3f_const_array( &T.B ) );
         glVertex3fv( ns_vector3f_const_array( &T.C ) );

         glVertex3fv( ns_vector3f_const_array( &T.C ) );
         glVertex3fv( ns_vector3f_const_array( &T.A ) );
         }

      glEnd();
      }
   }


void _opengl_render_rayburst( void )
   {
   ns_mutex_lock( &____rayburst_mutex );

   if( NULL != ____rayburst_struct )
      _opengl_do_render_rayburst( ____rayburst_struct, ____rayburst_running );

   ns_mutex_unlock( &____rayburst_mutex );
   }
*/



/*
void _ogl_window_sampler_render
   (
   const NsRenderInterface   *renderer,
   nsconstpointer      sample,
   const NsVoxelInfo  *voxel_info,
   OpenGLWindow       *ogl
   )
   {
   if( ns_sample_get_is_marker( sample ) )
      {
      NsVector3f  V;
      nsfloat     radius;
      NsColor4ub  C;


      ns_sample_get_center( sample, &V );
      radius = ns_voxel_info_max_size( voxel_info ) * 2.0f;

      switch( ns_sample_get_user_type( sample ) )
         {
         case 1:
            C = workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_STUBBY );
            break;

         case 2:
            C = workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_THIN );
            break;

         case 3:
            C = workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_MUSHROOM );
            break;

         case 4:
            C = workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_OTHER );
            break;

         default:
            C = workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_INVALID );
         }

      glPushMatrix();

      glTranslatef( V.x, V.y, V.z );
      glScalef( radius, radius, radius );

      glColor3ub(
         NS_COLOR_GET_RED_U8( C ),
         NS_COLOR_GET_GREEN_U8( C ),
         NS_COLOR_GET_BLUE_U8( C )
         );

      OpenGLWindowSphere( ogl, _OPENGL_SPHERE_COMPLEXITY );

      glPopMatrix();
      }
   }
*/



#include <render/nsrender3d-opengl.h>
extern NsColor4ub *____order_colors;
extern NsColor4ub *____conn_comp_colors;
extern NsColor4ub *____section_colors;
extern nssize ____num_colors;


GLdouble ____modelview_matrix[16];
GLdouble ____projection_matrix[16];


void _draw_naas_points( void )
   {
/*
   NsVector3f          Vf;
   NsVector3i          Vi;
   nssize              i;
   const NsVoxelInfo  *voxel_info;
   NsVector3f          corners[8];


   glBegin( GL_POINTS );
   //glColor3f( 1.0f, 1.0f, 1.0f );

   voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

   for( i = 0; i < ____naas_num_points; ++i )
      {
      if( ____naas_points[i].x < 0 )
         glColor3f( 1.0f, 0.0f, 0.0f );
      else
         glColor3f( 0.0f, 0.0f, 1.0f );

      ns_vector3i_add( &Vi, ____naas_points + i, &____naas_center );

      if( Vi.x >= 0 && Vi.y >= 0 && Vi.z >= 0 )
         {
         ns_to_voxel_space( &Vi, &Vf, voxel_info );
         glVertex3fv( ns_vector3f_const_array( &Vf ) );
         }
      }

   glEnd();

   for( i = 0; i < 8; ++i )
      {
      ns_vector3i_add( &Vi, ____naas_corners + i, &____naas_center );

      if( Vi.x >= 0 && Vi.y >= 0 && Vi.z >= 0 )
         ns_to_voxel_space( &Vi, corners + i, voxel_info );
      else
         ns_vector3f_zero( corners + i );
      }

   glBegin( GL_LINES );
   glColor3f( 1.0f, 1.0f, 1.0f );
      glVertex3fv( ns_vector3f_const_array( corners + 0 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 1 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 2 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 3 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 0 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 2 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 1 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 3 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 4 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 5 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 6 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 7 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 4 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 6 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 5 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 7 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 0 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 4 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 1 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 5 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 2 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 6 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 3 ) );
      glVertex3fv( ns_vector3f_const_array( corners + 7 ) );
   glEnd();
*/
   }


NS_PRIVATE NsRenderState  ____render_state_3d;
NS_PRIVATE nsboolean      ____render_state_3d_init = NS_FALSE;
NS_PRIVATE NsColor4ub     ____vertex_type_colors_3d[ NS_MODEL_VERTEX_NUM_TYPES ];
NS_PRIVATE NsColor4ub     ____spine_type_colors_3d[ NS_SPINE_NUM_TYPES ];
NS_PRIVATE NsColor4ub     ____function_type_colors_3d[ NS_MODEL_FUNCTION_NUM_TYPES ];


void _set_octree_node_color( nsfloat r, nsfloat g, nsfloat b, nspointer user_data )
	{
	user_data;
	glColor3f( r, g, b );
	}


#define GRID_IDEAL_NUM_STEPS  10.0f

NS_PRIVATE nsfloat ____volume_grid_steps[] =
	{
	   0.01f,  0.025f, 0.05f,
	   0.1f,   0.25f,  0.5f,
	   1.0f,   2.5f,   5.0f,
	  10.0f,  25.0f,  50.0f,
	 100.0f, 250.0f, 500.0f,
	1000.0f
	};


NS_PRIVATE nschar  ____volume_grid_text[32];

void opengl_render_volume_grid( nsfloat xmax, nsfloat ymax, nsfloat zmax )
	{
	nsfloat  dim;
	nsfloat  xmin, ymin, zmin;
	nsfloat  x1, y1, z1, x2, y2, z2;
	nssize   i;
	nsfloat  error;
	nsfloat  min_error;
	nsint    ideal;


	if( ____volume_grid_step <= 0.0f )
		{
		dim = NS_MIN3( xmax, ymax, zmax );

		/*TEMP*/ns_println( "dim = " NS_FMT_DOUBLE, dim );

		/* This if statement will probably not be true very often (if ever),
			but lets protect against this scenario anyway. */
		if( dim < ____volume_grid_steps[0] )
			____volume_grid_step = dim;
		else
			{
			ideal     = -1;
			min_error = NS_FLOAT_MAX;

			for( i = 0; i < NS_ARRAY_LENGTH( ____volume_grid_steps ); ++i )
				{
				if( ____volume_grid_steps[i] <= dim )
					{
					error = GRID_IDEAL_NUM_STEPS - dim / ____volume_grid_steps[i];
					error = NS_ABS( error );
					}
				else
					error = NS_FLOAT_MAX;

			/*TEMP*///ns_println( "At step " NS_FMT_DOUBLE " the error is " NS_FMT_DOUBLE, ____volume_grid_steps[i], error );

				if( error < min_error )
					{
					min_error = error;
					ideal     = ( nsint )i;
					}
				}

			____volume_grid_step =
						( 0 <= ideal ) ?
						____volume_grid_steps[ ideal ]
						:
						____volume_grid_steps[ NS_ARRAY_LENGTH( ____volume_grid_steps ) - 1 ];
			}

		/*TEMP*/ns_println( "step size was chosen to be " NS_FMT_DOUBLE, ____volume_grid_step );
		}

	glColor3f( 1.0f, 1.0f, 1.0f );

	xmin = ymin = zmin = 0.0f;

	/* draw back panel */
	x1 = x2 = xmin;
	y1 = ymax;
	y2 = ymin;
	z1 = z2 = zmax;

	glBegin( GL_LINES );

	while( x1 <= xmax )
		{
		glVertex3f( x1, y1, z1 );
		glVertex3f( x2, y2, z2 );
		x1 += ____volume_grid_step;
		x2 = x1;
		}

	glVertex3f( xmax, y1, z1 );
	glVertex3f( xmax, y2, z2 );

	glEnd();

	x1 = xmin;
	x2 = xmax;
	y1 = y2 = ymin;

	glBegin( GL_LINES );

	while( y1 <= ymax )
		{
		glVertex3f( x1, y1, z1 );
		glVertex3f( x2, y2, z2 );
		y1 += ____volume_grid_step;
		y2 = y1;
		}

	glVertex3f( x1, ymax, z1 );
	glVertex3f( x2, ymax, z2 );
	glEnd();

	/* draw base panel */
	x1 = x2 = xmin;
	y1 = y2 = ymax;
	z1 = zmin;
	z2 = zmax;

	glBegin( GL_LINES );

	while( x1 <= xmax )
		{
		glVertex3f( x1, y1, z1 );
		glVertex3f( x2, y2, z2 );
		x1 += ____volume_grid_step;
		x2 = x1;
		}

	glVertex3f( xmax, y1, z1 );
	glVertex3f( xmax, y2, z2 );

	glEnd();

	x1 = xmin;
	x2 = xmax;
	z1 = z2 = zmin;

	glBegin( GL_LINES );

	while( z1 <= zmax )
		{
		glVertex3f( x1, y1, z1 );
		glVertex3f( x2, y2, z2 );
		z1 += ____volume_grid_step;
		z2 = z1;
		}

	glVertex3f( x1, y1, zmax );
	glVertex3f( x2, y2, zmax );

	glEnd();
	}


void _ns_gui_3d_draw_roi_border( const NsVector3f *C1, const NsVector3f *C2 )
	{
	GLfloat C1x, C1y, C1z, C2x, C2y, C2z;

	C1x = C1->x; C1y = C1->y; C1z = C1->z;
	C2x = C2->x; C2y = C2->y; C2z = C2->z;

	glDisable( GL_LIGHTING );
	glEnable( GL_LINE_STIPPLE );
	glLineStipple( 3, 0x5555 );
	glColor3f( 1.0f, 1.0f, 1.0f );
	glBegin( GL_LINES );

	glVertex3f( C1x, C1y, C1z ); glVertex3f( C2x, C1y, C1z );
	glVertex3f( C2x, C1y, C1z ); glVertex3f( C2x, C2y, C1z );
	glVertex3f( C2x, C2y, C1z ); glVertex3f( C1x, C2y, C1z );
	glVertex3f( C1x, C2y, C1z ); glVertex3f( C1x, C1y, C1z );

	glVertex3f( C1x, C1y, C2z ); glVertex3f( C2x, C1y, C2z );
	glVertex3f( C2x, C1y, C2z ); glVertex3f( C2x, C2y, C2z );
	glVertex3f( C2x, C2y, C2z ); glVertex3f( C1x, C2y, C2z );
	glVertex3f( C1x, C2y, C2z ); glVertex3f( C1x, C1y, C2z );

	glVertex3f( C1x, C1y, C1z ); glVertex3f( C1x, C1y, C2z );
	glVertex3f( C2x, C1y, C1z ); glVertex3f( C2x, C1y, C2z );
	glVertex3f( C2x, C2y, C1z ); glVertex3f( C2x, C2y, C2z );
	glVertex3f( C1x, C2y, C1z ); glVertex3f( C1x, C2y, C2z );

	glEnd();
	glDisable( GL_LINE_STIPPLE );
	}


#define NS_ROI_CONTROL_DRAW_BASE( vx, vy, vz, ox, oy, oz, rd, rx, ry, rz, sz )\
	glPushMatrix();\
	glTranslatef( (vx) + (ox), (vy) + (oy), (vz) + (oz) );\
	glRotatef( (rd), (rx), (ry), (rz) );\
	OpenGLWindowDrawCylinderWithCaps(\
		&s_MainWindow.ogl,\
		(sz) * NS_ROI_CONTROL_BASE_RADIUS_SCALAR,\
		(sz) * NS_ROI_CONTROL_BASE_RADIUS_SCALAR,\
		(sz) * NS_ROI_CONTROL_BASE_HEIGHT_SCALAR,\
		NS_ROI_CONTROL_COMPLEXITY,\
		NS_FALSE, NS_TRUE );\
	glPopMatrix()

#define NS_ROI_CONTROL_DRAW_TIP( vx, vy, vz, ox, oy, oz, rd, rx, ry, rz, sz )\
	glPushMatrix();\
	glTranslatef(\
		(vx) + (ox) * NS_ROI_CONTROL_BASE_HEIGHT_SCALAR,\
		(vy) + (oy) * NS_ROI_CONTROL_BASE_HEIGHT_SCALAR,\
		(vz) + (oz) * NS_ROI_CONTROL_BASE_HEIGHT_SCALAR );\
	glRotatef( (rd), (rx), (ry), (rz) );\
	OpenGLWindowDrawConeWithBase(\
		&s_MainWindow.ogl,\
		(sz) * NS_ROI_CONTROL_TIP_RADIUS_SCALAR,\
		(sz) * NS_ROI_CONTROL_TIP_HEIGHT_SCALAR,\
		NS_ROI_CONTROL_COMPLEXITY );\
	glPopMatrix()


void _ns_gui_3d_draw_roi_controls( const NsRoiControls *rc, NsRenderState *state )
	{
	glEnable( GL_LIGHTING );

	if( 1 < rc->dim[2] )
		{
		NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_NEAR_BIT );
		NS_ROI_CONTROL_DRAW_BASE( rc->d2a, rc->d2b, rc->C1.z, 0.0f, 0.0f, 0.0f, -90.0f, 1.0f, 0.0f, 0.0f, rc->sz ); // Near
		NS_ROI_CONTROL_DRAW_TIP( rc->d2a, rc->d2b, rc->C1.z, 0.0f, 0.0f, -rc->sz, -90.0f, 1.0f, 0.0f, 0.0f, rc->sz );
		NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_FAR_BIT );
		NS_ROI_CONTROL_DRAW_BASE( rc->d2a, rc->d2b, rc->C2.z, 0.0f, 0.0f, 0.0f, 90.0f, 1.0f, 0.0f, 0.0f, rc->sz ); // Far
		NS_ROI_CONTROL_DRAW_TIP( rc->d2a, rc->d2b, rc->C2.z, 0.0f, 0.0f, rc->sz, 90.0f, 1.0f, 0.0f, 0.0f, rc->sz );
		}

	NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_TOP_BIT );
	NS_ROI_CONTROL_DRAW_BASE( rc->d1a, rc->C1.y, rc->d1b, 0.0f, 0.0f, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f, rc->sz ); // Top
	NS_ROI_CONTROL_DRAW_TIP( rc->d1a, rc->C1.y, rc->d1b, 0.0f, -rc->sz, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f, rc->sz );
	NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_BOTTOM_BIT );
	NS_ROI_CONTROL_DRAW_BASE( rc->d1a, rc->C2.y, rc->d1b, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, rc->sz ); // Bottom
	NS_ROI_CONTROL_DRAW_TIP( rc->d1a, rc->C2.y, rc->d1b, 0.0f, rc->sz, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, rc->sz );

	NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_LEFT_BIT );
	NS_ROI_CONTROL_DRAW_BASE( rc->C1.x, rc->d0a, rc->d0b, 0.0f, 0.0f, 0.0f, 90.0f, 0.0f, 0.0f, 1.0f, rc->sz ); // Left
	NS_ROI_CONTROL_DRAW_TIP( rc->C1.x, rc->d0a, rc->d0b, -rc->sz, 0.0f, 0.0f, 90.0f, 0.0f, 0.0f, 1.0f, rc->sz );
	NS_ROI_CONTROL_SET_COLOR( rc->selected, NS_ROI_CONTROL_RIGHT_BIT );
	NS_ROI_CONTROL_DRAW_BASE( rc->C2.x, rc->d0a, rc->d0b, 0.0f, 0.0f, 0.0f, -90.0f, 0.0f, 0.0f, 1.0f, rc->sz ); // Right
	NS_ROI_CONTROL_DRAW_TIP( rc->C2.x, rc->d0a, rc->d0b, rc->sz, 0.0f, 0.0f, -90.0f, 0.0f, 0.0f, 1.0f, rc->sz );

	glDisable( GL_LIGHTING );

	if( rc->selected )
		{
		ns_assert( NULL != state );
		ns_assert( NULL != NS_RENDER( state, aabbox_func ) );

		state->variables.color = NS_COLOR4UB_WHITE;
		state->variables.shape = NS_RENDER_SHAPE_AABBOX;

		NS_RENDER_PRE_OR_POST( state, pre_aabboxes_func );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_NEAR_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_NEAR_IDX, state );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_FAR_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_FAR_IDX, state );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_TOP_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_TOP_IDX, state );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_BOTTOM_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_BOTTOM_IDX, state );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_LEFT_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_LEFT_IDX, state );

		if( ( nsboolean )( rc->selected & NS_ROI_CONTROL_RIGHT_BIT ) )
			NS_RENDER( state, aabbox_func )( rc->B + NS_ROI_CONTROL_RIGHT_IDX, state );

		NS_RENDER_PRE_OR_POST( state, post_aabboxes_func );
		}

	/*
	#define _NS_ROI_CONTROL_DRAW_AXIS( which )\
		glColor3f( 0.0f, 1.0f, 0.0f );\
		glVertex3d( rc->L[ which ].P1.x, rc->L[ which ].P1.y, rc->L[ which ].P1.z );\
		glColor3f( 1.0f, 0.0f, 0.0f );\
		glVertex3d( rc->L[ which ].P2.x, rc->L[ which ].P2.y, rc->L[ which ].P2.z )

	glBegin( GL_LINES );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_NEAR_IDX );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_FAR_IDX );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_TOP_IDX );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_BOTTOM_IDX );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_LEFT_IDX );
	_NS_ROI_CONTROL_DRAW_AXIS( NS_ROI_CONTROL_RIGHT_IDX );
	glEnd();
	*/
	/*
	if( s_MainWindow.ogl.isDragging )
		{
		NsVector3f P;

		state->variables.color = NS_COLOR4UB_RED;
		state->variables.shape = NS_RENDER_SHAPE_SPHERE;

		NS_RENDER_PRE_OR_POST( state, pre_spheres_func );
		NS_RENDER( state, sphere_func )( ns_vector3d_to_3f( &s_MainWindow.ogl.old_voxel, &P ), 0.5f, state );
		NS_RENDER_PRE_OR_POST( state, post_spheres_func );
		}
	*/
	}


void _ns_gui_3d_draw_roi( NsRenderState *state, nsuint workspace, const NsVoxelInfo *voxel_info )
	{
	NsRoiControls rc;

	ns_roi_controls_init(
		&rc,
		NS_ROI_CONTROL_ALL_BITS,
		workspace_update_roi( s_MainWindow.activeWorkspace ),
		workspace_volume( s_MainWindow.activeWorkspace ),
		voxel_info,
		workspace_get_roi_control_selections( s_MainWindow.activeWorkspace )
		);

	_ns_gui_3d_draw_roi_border( &rc.C1, &rc.C2 );
	_ns_gui_3d_draw_roi_controls( &rc, state );
	}


GLfloat ____specular[4]={1.0f,1.0f,1.0f,1.0f};
GLfloat ____shininess[1]={50.0f};


void _OpenGLRenderFrame( OpenGLWindow *ogl, void *userData )
   {
   NsModel            *model;
   const NsVoxelInfo  *voxel_info;
   NsVector3f          dimensions;
   HWorkspace          hWorkspace;
   //BoundingCube      *cube;
//const Image           *image;
   nsfloat             halfX;
   nsfloat             halfY;
   nsfloat             halfZ;
   //nsfloat              centerX;
   //nsfloat              centerY;
   //nsfloat              centerZ;
   nsfloat             max;
   //nsdouble           fps;
//   NsVector4ub           color;
   NsVector4ub         splats_color;
  // GLfloat            original;
   NsColor4f           color;
   //const NsSampler *sampler;
   //NsColor4ub C1/*, C2*/;
     //GLfloat old_line_width;
	NsRenderState       *state;

//   nschar             buffer[32];



   hWorkspace = s_MainWindow.activeWorkspace;

   /* 7/01/03 Added synchronization to shared resource. */

   if( 0 == hWorkspace /*|| ! TryLockWorkspace( hWorkspace )*/ )
     return;


   _clear_background();


   //if( ! WorkspaceHasNeuronTree( hWorkspace ) )
     // {
      //UnlockWorkspace( hWorkspace );
      //return;
      //}


   //fps = ns_frames_per_second( &____fps );
   //ns_sprint( ____fps_string, "%f", fps );



   model      = GetWorkspaceNeuronTree( hWorkspace );
   voxel_info = workspace_get_voxel_info( hWorkspace );
   
   workspace_get_image_dimensions( hWorkspace, &dimensions );
      halfX = dimensions.x / 2.0f;
      halfY = dimensions.y / 2.0f;
      halfZ = dimensions.z / 2.0f;

   max = mMAX3( halfX, halfY, halfZ );

   OpenGLWindowPerspectiveProjection(
		ogl,
		ogl->fieldOfView,
		( nsfloat )ogl->width / ( nsfloat )ogl->height,
		ns_voxel_info_size_z( voxel_info ),//ogl->zNear,
		//4096.0f * /* + ogl->interactor.izpos */
		32000.0f * ns_voxel_info_size_z( voxel_info )
		);

   //if( ____use_antialiasing )
     // _opengl_enable_line_anti_aliasing();

   //glGetFloatv( GL_LINE_WIDTH, &original );
   //glLineWidth( ( GLfloat )s_MainWindow.line_width );

//   if( s_MainWindow.showAxis )
  //    _OpenGLRenderAxis( ogl, dimensions.x, dimensions.y, dimensions.z/*halfX / 2.0f, halfY / 2.0f, halfZ / 2.0f*/ );

   /* Rotate coordinate system 180 degrees around X-axis since its
      coordinates were set in a rotated world coordinate system and
      center the model. */

   glPushMatrix();

//   if( ! s_MainWindow.polygonsSolid )
  //    {
    //  glDisable( GL_CULL_FACE );
     // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
     // }

   //____axis_of_revolution( ogl );

   glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );

	switch( ____opengl_centering_mode )
		{
		case IDM_GL_VIEW_CENTER_ON_IMAGE:
			glTranslatef( -halfX, -halfY, -halfZ );
			break;

		case IDM_GL_VIEW_CENTER_ON_ROI:
			glTranslatef(
				-____opengl_roi_center.x,
				-____opengl_roi_center.y,
				-____opengl_roi_center.z
				);
			break;

		case IDM_GL_VIEW_CENTER_ON_MODEL:
			glTranslatef(
				-____model_3d_rotation_center.x,
				-____model_3d_rotation_center.y,
				-____model_3d_rotation_center.z
				);
			break;

		case IDM_GL_VIEW_CENTER_ON_SEED:
			{
			NsVector3f Sf;
			NsVector3i Si;

			Si = *( ns_settings_neurite_seed( workspace_settings( hWorkspace ) ) );
			ns_to_voxel_space( &Si, &Sf, workspace_get_voxel_info( hWorkspace ) );

			glTranslatef( -Sf.x, -Sf.y, -Sf.z );
			}
			break;
		}

   if( s_MainWindow.showAxis )
      _OpenGLRenderAxis( ogl, dimensions.x, dimensions.y, dimensions.z/*halfX / 2.0f, halfY / 2.0f, halfZ / 2.0f*/ );

   if( s_MainWindow.lightingOn )
      {
      glEnable( GL_NORMALIZE );
      glEnable( GL_LIGHTING );
      glEnable( GL_LIGHT0 );
      glEnable( GL_COLOR_MATERIAL );

      glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, ____specular );
      glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, ____shininess );
      }

	state = &____render_state_3d;

	if( ! ____render_state_3d_init )
		{
		ns_render_state_init( state );

		state->constants.iface.pre_images_func     = ns_render3d_pre_images;
		state->constants.iface.pre_borders_func    = ns_render3d_pre_borders;
		state->constants.iface.pre_lines_func      = ns_render3d_pre_lines;
		state->constants.iface.pre_points_func     = ns_render3d_pre_points;
		state->constants.iface.pre_texts_func      = ns_render3d_pre_texts;
		state->constants.iface.pre_spheres_func    = ns_render3d_pre_spheres;
		state->constants.iface.pre_aabboxes_func   = ns_render3d_pre_aabboxes;
		state->constants.iface.pre_frustums_func   = ns_render3d_pre_frustums;
		state->constants.iface.pre_triangles_func  = ns_render3d_pre_triangles;
		state->constants.iface.pre_ellipses_func   = ns_render3d_pre_ellipses;
		state->constants.iface.pre_rectangles_func = ns_render3d_pre_rectangles;

		state->constants.iface.post_images_func     = ns_render3d_post_images;
		state->constants.iface.post_borders_func    = ns_render3d_post_borders;
		state->constants.iface.post_lines_func      = ns_render3d_post_lines;
		state->constants.iface.post_points_func     = ns_render3d_post_points;
		state->constants.iface.post_texts_func      = ns_render3d_post_texts;
		state->constants.iface.post_spheres_func    = ns_render3d_post_spheres;
		state->constants.iface.post_aabboxes_func   = ns_render3d_post_aabboxes;
		state->constants.iface.post_frustums_func   = ns_render3d_post_frustums;
		state->constants.iface.post_triangles_func  = ns_render3d_post_triangles;
		state->constants.iface.post_ellipses_func   = ns_render3d_post_ellipses;
		state->constants.iface.post_rectangles_func = ns_render3d_post_rectangles;

		state->constants.iface.image_func     = ns_render3d_image;
		state->constants.iface.border_func    = ns_render3d_border;
		state->constants.iface.line_func      = ns_render3d_line;
		state->constants.iface.point_func     = ns_render3d_point;
		state->constants.iface.text_func      = ns_render3d_text;
		state->constants.iface.sphere_func    = ns_render3d_sphere;
		state->constants.iface.aabbox_func    = ns_render3d_aabbox;
		state->constants.iface.frustum_func   = ns_render3d_frustum;
		state->constants.iface.triangle_func  = ns_render3d_triangle;
		state->constants.iface.ellipse_func   = ns_render3d_ellipse;
		state->constants.iface.rectangle_func = ns_render3d_rectangle;

		ns_render_state_set_default_steps_order( state );

		state->constants.dimensions = 3;

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

		state->constants.palette_colors = NULL;
		state->constants.palette_size   = 0;

		state->constants.vertex_type_colors     = ____vertex_type_colors_3d;
		state->constants.manually_traced_color  = NS_COLOR4UB_RED;
		state->constants.num_vertex_type_colors = NS_MODEL_VERTEX_NUM_TYPES;

		state->constants.spine_type_colors     = ____spine_type_colors_3d;
		state->constants.num_spine_type_colors = NS_SPINE_NUM_TYPES;

		state->constants.function_type_colors     = ____function_type_colors_3d;
		state->constants.num_function_type_colors = NS_MODEL_FUNCTION_NUM_TYPES;

		____render_state_3d_init = NS_TRUE;
		}

	state->constants.dataset    = workspace_dataset( hWorkspace );
	state->constants.proj_xy    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_XY );
	state->constants.proj_zy    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_ZY );
	state->constants.proj_xz    = NULL;//GetWorkspaceDisplayImage( hWorkspace, NS_XZ );
	state->constants.model      = model;
	state->constants.sampler    = workspace_sampler( hWorkspace );
	state->constants.voxel_info = workspace_get_voxel_info( hWorkspace );
	state->constants.settings   = workspace_settings( hWorkspace );
	state->constants.classifier = ns_spines_classifier_get();

   state->constants.flicker_free_rendering = ____flicker_free_rendering;
   state->constants.lighting               = s_MainWindow.lightingOn;
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

   ____vertex_type_colors_3d[ NS_MODEL_VERTEX_ORIGIN   ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_ORIGIN_VERTEX );
   ____vertex_type_colors_3d[ NS_MODEL_VERTEX_EXTERNAL ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_EXTERNAL_VERTEX );
   ____vertex_type_colors_3d[ NS_MODEL_VERTEX_JUNCTION ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_JUNCTION_VERTEX );
   ____vertex_type_colors_3d[ NS_MODEL_VERTEX_LINE     ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_LINE_VERTEX );

   state->constants.vertex_color_scheme = ____vertex_color_scheme;
   state->constants.vertex_single_color = workspace_get_color( hWorkspace, WORKSPACE_COLOR_VERTEX );

   state->constants.edge_color_scheme = ____edge_color_scheme;
   state->constants.edge_single_color = workspace_get_color( hWorkspace, WORKSPACE_COLOR_EDGE );

   ____spine_type_colors_3d[ NS_SPINE_INVALID  ] = workspace_get_spine_color_by_type( hWorkspace, NS_SPINE_INVALID );
   ____spine_type_colors_3d[ NS_SPINE_OTHER    ] = workspace_get_spine_color_by_type( hWorkspace, NS_SPINE_OTHER );
   ____spine_type_colors_3d[ NS_SPINE_THIN     ] = workspace_get_spine_color_by_type( hWorkspace, NS_SPINE_THIN );
   ____spine_type_colors_3d[ NS_SPINE_MUSHROOM ] = workspace_get_spine_color_by_type( hWorkspace, NS_SPINE_MUSHROOM );
   ____spine_type_colors_3d[ NS_SPINE_STUBBY   ] = workspace_get_spine_color_by_type( hWorkspace, NS_SPINE_STUBBY );

	____function_type_colors_3d[ NS_MODEL_FUNCTION_UNKNOWN         ] = NS_COLOR4UB_BLACK;
	____function_type_colors_3d[ NS_MODEL_FUNCTION_SOMA            ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_FUNCTION_SOMA );
	____function_type_colors_3d[ NS_MODEL_FUNCTION_BASAL_DENDRITE  ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE );
	____function_type_colors_3d[ NS_MODEL_FUNCTION_APICAL_DENDRITE ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE );
	____function_type_colors_3d[ NS_MODEL_FUNCTION_AXON            ] = workspace_get_color( hWorkspace, WORKSPACE_COLOR_FUNCTION_AXON );

   state->constants.spine_color_scheme      = ____spines_color_scheme;
   state->constants.spine_single_color      = workspace_get_color( hWorkspace, WORKSPACE_COLOR_SPINE );
	state->constants.spine_random_colors     = ns_spines_random_colors();
	state->constants.num_spine_random_colors = ns_spines_num_random_colors();

   state->constants.line_size          = s_MainWindow.line_width;
   state->constants.point_size         = s_MainWindow.line_width * 2;
   state->constants.polygon_complexity = s_MainWindow.complexity;

	state->constants.anti_aliasing = ____use_antialiasing;

   //state->constants.msh_zone_alpha = ____msh_zone_alpha;

   glGetDoublev( GL_PROJECTION_MATRIX, ____projection_matrix );
   glGetDoublev( GL_MODELVIEW_MATRIX, ____modelview_matrix );

   ns_render( state );


   if( ( eDISPLAY_WINDOW_MOUSE_MODE_SEED == s_CurrentMouseMode || ____view_seed_always_on_top )
			&& ! ns_model_grafter_is_running( model ) && ! ns_model_spines_is_running( model ) )
		{
		Vector3i seed;
		NsVector3f S;

		GetWorkspaceSeed( s_MainWindow.activeWorkspace, &seed );
		S.x = ns_to_voxel_space_component_ex( ( nsfloat )seed.x, voxel_info, NS_COMPONENT_X );
		S.y = ns_to_voxel_space_component_ex( ( nsfloat )seed.y, voxel_info, NS_COMPONENT_Y );
		S.z = ns_to_voxel_space_component_ex( ( nsfloat )seed.z, voxel_info, NS_COMPONENT_Z );

		glDisable( GL_LIGHTING );
		glEnable( GL_LINE_STIPPLE );
		glLineStipple( 3, 0x5555 );
		glColor3f( 1.0f, 1.0f, 1.0f );
		glBegin( GL_LINES );
		glVertex3f( 0.0f, S.y, S.z ); glVertex3f( dimensions.x, S.y, S.z );
		glVertex3f( S.x, 0.0f, S.z ); glVertex3f( S.x, dimensions.y, S.z );
		glVertex3f( S.x, S.y, 0.0f ); glVertex3f( S.x, S.y, dimensions.z );
		glEnd();
		glDisable( GL_LINE_STIPPLE );
		}

	if( ( eDISPLAY_WINDOW_MOUSE_MODE_ROI == s_CurrentMouseMode  || ____view_roi_always_on_top )
			&& ! ns_model_grafter_is_running( model ) && ! ns_model_spines_is_running( model ) )
		_ns_gui_3d_draw_roi( state, s_MainWindow.activeWorkspace, voxel_info );

   //if( s_MainWindow.lightingOn )
     // {
     // glDisable( GL_NORMALIZE );
     // glDisable( GL_LIGHT0 );
     // glDisable( GL_LIGHTING );
     // glDisable( GL_COLOR_MATERIAL );
     // }

   if( IDM_GL_VIEW_OCTREE_OFF != s_MainWindow.octree_mode )
      {
		if( s_MainWindow.lightingOn )glDisable( GL_LIGHTING );

		glEnableClientState( GL_VERTEX_ARRAY );
		ns_model_render_octree( model, NS_MODEL_EDGE_OCTREE, _aabbox3d_render, _set_octree_node_color, ogl );
		glDisableClientState( GL_VERTEX_ARRAY );

		if( s_MainWindow.lightingOn )glEnable( GL_LIGHTING );
      }


	if( ____view_3d_volume_grid )
		{
		if( s_MainWindow.lightingOn )glDisable( GL_LIGHTING );
		opengl_render_volume_grid( dimensions.x, dimensions.y, dimensions.z );
		if( s_MainWindow.lightingOn )glEnable( GL_LIGHTING );
		}


   //if( ____use_antialiasing )
     // _opengl_enable_line_anti_aliasing();

      //glEnable( GL_NORMALIZE );
      //glEnable( GL_LIGHTING );
      //glEnable( GL_LIGHT0 );
      //glEnable( GL_COLOR_MATERIAL );
      //C1.x = 255, C1.y = 174, C1.z = 0; C1.w = 255;

   //if( ! s_MainWindow.polygonsSolid )
     // {
     // glEnable( GL_CULL_FACE );
     // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
     // }

      //glDisable( GL_NORMALIZE );
      //glDisable( GL_LIGHT0 );
      //glDisable( GL_LIGHTING );
      //glDisable( GL_COLOR_MATERIAL );

/*TEMP!!!!!!!!!!!!!!!!!!!!!!!! */
   //_draw_naas_points();

//   _opengl_render_rayburst();

	if( ____show_roi_bounding_box )
		{
		if( s_MainWindow.lightingOn )glDisable( GL_LIGHTING );

		ns_vector4f( &color, 0.2f, 0.2f, 0.2f, 1.0f );
      _render_roi_bounding_box_edges( ogl, &color );

		if( s_MainWindow.lightingOn )glEnable( GL_LIGHTING );
		}

   if( s_MainWindow.showBoundingCube )
      {
		if( s_MainWindow.lightingOn )glDisable( GL_LIGHTING );

		ns_vector4f( &color, 0.2f, 0.2f, 0.2f, 1.0f );
		_open_gl_render_bounding_cube_edges( ogl, dimensions.x, dimensions.y, dimensions.z, &color );

		if( s_MainWindow.lightingOn )glEnable( GL_LIGHTING );
      }



   if( ( eDISPLAY_WINDOW_MOUSE_MODE_RAYS == ____ogl_curr_mouse_mode ||
			eDISPLAY_WINDOW_MOUSE_MODE_NEURITE == ____ogl_curr_mouse_mode ||
			eDISPLAY_WINDOW_MOUSE_MODE_SPINE == ____ogl_curr_mouse_mode )
       && s_MainWindow.ogl.isDragging
		 && ( ! ____ogl_drag_rect_active )
		 /*&& ( _DRAG_MODE_NORMAL == s_MainWindow.ogl.drag_mode )*/ )
		{
		/*ns_println( "%f,%f,%f to %f,%f,%f",
			 s_MainWindow.ogl.old_voxel.x, s_MainWindow.ogl.old_voxel.y, s_MainWindow.ogl.old_voxel.z,
			s_MainWindow.ogl.new_voxel.x, s_MainWindow.ogl.new_voxel.y, s_MainWindow.ogl.new_voxel.z );*/
		if( s_MainWindow.lightingOn )glDisable( GL_LIGHTING );
		glBegin( GL_LINES );
			glColor3f( 1.0f, 0.0f, 0.0f );
			glVertex3d( s_MainWindow.ogl.old_voxel.x, s_MainWindow.ogl.old_voxel.y, s_MainWindow.ogl.old_voxel.z );
			glVertex3d( s_MainWindow.ogl.new_voxel.x, s_MainWindow.ogl.new_voxel.y, s_MainWindow.ogl.new_voxel.z );
		glEnd();
		if( s_MainWindow.lightingOn )glEnable( GL_LIGHTING );
		}



   if( ____splats_init &&
       ____splats_view /*&&
       WorkspaceDisplayFlagIsSet( hWorkspace, mDISPLAY_GRAPHICS_IMAGE )*/ )
      {
      nsulong flags;
      NsVector3f C;


      flags = 0;
      if( s_MainWindow.lightingOn )flags |= NS_SPLATS_RENDER_LIGHTING;
      if( ____use_splats_alpha )flags |= NS_SPLATS_RENDER_ALPHA;
      if( s_MainWindow.usePalette3D )flags |= NS_SPLATS_RENDER_PALETTE;


		if( ____optimize_splats_movement &&
			( ogl->is_mouse_dragging ||
			  ogl->is_sizing_or_moving ||
			  ogl->is_wheeling ||
			  ns_model_grafter_is_running( model ) ||
			  ns_model_spines_is_running( model ) ) )
			flags |= NS_SPLATS_RENDER_OPTIMIZE;


      splats_color = workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_SPLAT );

      C.x = NS_COLOR_GET_RED_F( splats_color );
      C.y = NS_COLOR_GET_GREEN_F( splats_color );
      C.z = NS_COLOR_GET_BLUE_F( splats_color );

      if( ____xy_slice_enabled )
         ns_splats_set_at_slice( &____splats, ( nsint )____xy_slice_index );
      else
         ns_splats_unset_at_slice( &____splats );

       ns_splats_render(
         &____splats,
         voxel_info,
         flags,
         GetWorkspacePalette( hWorkspace )->colors,
         &C,
         ____splats_alpha,
         ____splats_render_type,
         ____splats_clip
         );
      }

   if( ____xy_slice_enabled )
      {
      ns_vector4f( &color, 1.0f, 1.0f, 1.0f, .2f );
      //_opengl_render_xy_slice( ogl, /*dimensions.x, dimensions.y,*/ &color );
      }

	if( ____show_roi_bounding_box )
		{
      ns_vector4f( &color, 0.0f, 0.0f, .1f, .2f );
      _render_roi_bounding_box( ogl, 1, &color );
		}

   if( s_MainWindow.showBoundingCube )
      {
      ns_vector4f( &color, 0.0f, 0.0f, .1f, .2f );
      _OpenGLRenderBoundingCube( ogl, dimensions.x, dimensions.y, dimensions.z, 1, &color );
      }


/*
   if( s_MainWindow.show_octree_bounding_box )
      {
      const NsAABBox3d *B;
      NsVector3d corners[8];
      B = ns_model_octree_aabbox( model, NS_MODEL_EDGE_OCTREE );
      ns_aabbox3d_corners( B, corners );

      ns_vector4f( &color, 0.1f, 0.0f, 0.0f, .2f );

      glEnableClientState( GL_VERTEX_ARRAY );
      glVertexPointer( 3, GL_DOUBLE, 0, corners );

      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

      glColor4f( color.x, color.y, color.z, color.w );   

      glDrawElements( GL_QUADS,
                      mARRAY_LENGTH( s_OpenGLRenderBoundingCubeFaces ),
                      GL_UNSIGNED_INT,
                      ( const GLvoid* )s_OpenGLRenderBoundingCubeFaces
                    );

      glDisable( GL_BLEND );

      glDisableClientState( GL_VERTEX_ARRAY );
      }
*/


/* TEMP!!!!!!!!!!!!! */
/*
   if( ns_model_try_lock( model ) )
      {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      glDisable( GL_CULL_FACE );

      ns_vector4f( &color, 0.1f, 0.0f, 0.0f, .05f );
      glColor4f( color.x, color.y, color.z, color.w );   

      _OpenGLRenderSpineMaxHeightCylinders( ogl, model );

      glEnable( GL_CULL_FACE );
      glDisable( GL_BLEND );

      ns_model_unlock( model );
      }
*/

   //glLineWidth( original );

   //if( ____use_antialiasing )
     // _opengl_disable_line_anti_aliasing();

   glPopMatrix();

	if( ____view_3d_volume_grid )
		ns_snprint( ____volume_grid_text, sizeof( ____volume_grid_text ), "Grip Step: %.1f µm", ____volume_grid_step );

	ogl->text = s_MainWindow.ogl_text;

	//ogl->model = model;
   }


void OpenGLRender( void )
   {  RepaintOpenGLWindow( &s_MainWindow.ogl );  }



//void _OpenGLOnLButtonUp( OpenGLWindow *const ogl, void *userData )
  // {
	//NsVector3i  seed;
	//nsboolean   found;


	//RepaintOpenGLWindow( ogl );

/*
	ns_seed_find_3d(
		workspace_get_voxel_info( s_MainWindow.activeWorkspace ),
		workspace_volume( s_MainWindow.activeWorkspace ),
		workspace_visual_roi( s_MainWindow.activeWorkspace ),
		&____splats,
		____modelview_matrix,
		____projection_matrix,
		ogl->mouseX,
		ogl->mouseY,
		&seed,
		&found,
		NULL
		);

	if( found )
		ns_println( "SEED = %d, %d, %d", seed.x, seed.y, seed.z );
*/
	//}


void _OpenGLOnViewPolygonsComplexity( nsint complexity, nsint menuID )
   {
   if( complexity != s_MainWindow.complexity )
      {
      DestroyOpenGLWindowCylinder( &s_MainWindow.ogl );
      DestroyOpenGLWindowSphere( &s_MainWindow.ogl );
      DestroyOpenGLWindowHalfSphere( &s_MainWindow.ogl );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.complexityMenuID, 0 );

      s_MainWindow.complexity       = complexity;
      s_MainWindow.complexityMenuID = menuID;

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.complexityMenuID, 1 );
      }
   }


/*
void _open_gl_on_splats_contrast( nsint id, nsulong contrast )
   {
   if( id != ____splats_contrast_menu_id )
      {
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____splats_contrast_menu_id, 0 );

      ____splats_contrast_menu_id = id;
      ____splats_current_threshold         = contrast;

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____splats_contrast_menu_id, 1 );
      }
   }
*/


/*
void _OpenGLOnViewPolygonMode( nsint solid )
   {
   if( solid != s_MainWindow.polygonsSolid )
      {
      s_MainWindow.polygonsSolid = solid;

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_POLYGON_WIREFRAME, ! solid );
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_POLYGON_SOLID, solid );
      }
   }
*/


void _open_gl_on_view_octree_mode( nsint menu_id )
   {
   if( menu_id != s_MainWindow.octree_mode )
      {
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.octree_mode, 0 );
      s_MainWindow.octree_mode = menu_id;
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.octree_mode, 1 );
      }
   }


void _open_gl_on_center_on( OpenGLWindow *ogl, nsint menu_id )
	{
	if( menu_id == ____opengl_centering_mode )
		return;

	SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____opengl_centering_mode, NS_FALSE );

	switch( menu_id )
		{
		case IDM_GL_VIEW_CENTER_ON_IMAGE:
			break;

		case IDM_GL_VIEW_CENTER_ON_ROI:
			break;

		case IDM_GL_VIEW_CENTER_ON_MODEL:
			ns_print( "finding model center..." );
			ns_model_find_center( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), &____model_3d_rotation_center );
			ns_print( " DONE" );
			break;

		case IDM_GL_VIEW_CENTER_ON_SEED:
			break;

		default:
			ns_assert_not_reached();
		}

	____opengl_centering_mode = menu_id;
	SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____opengl_centering_mode, NS_TRUE );

	RepaintOpenGLWindow( ogl );
	}


void _OpenGLRotateLeft( OpenGLWindow *const ogl, void *userData )
   {
   nsfloat angle = ogl->interactor.iorientation + 1.0f;

   if( 360.0f <= angle )
      angle -= 360.0f;
   else if( angle < 0.0f )
      angle += 360.0f;

   ogl->interactor.iorientation = angle;

   RepaintOpenGLWindow( ogl );
   }


void _OpenGLRotateRight( OpenGLWindow *const ogl, void *userData )
   {
   nsfloat angle = ogl->interactor.iorientation - 1.0f;

   if( 360.0f <= angle )
      angle -= 360.0f;
   else if( angle < 0.0f )
      angle += 360.0f;

   ogl->interactor.iorientation = angle;

   RepaintOpenGLWindow( ogl );
   }


void _open_gl_on_view_reset( OpenGLWindow *ogl, nsint which )
	{
	const NsVoxelInfo *voxel_info;

	_reset_interactor( which );

	voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

	SetInteractorRotationalTouch( &s_MainWindow.ogl.interactor, .5f );
	SetInteractorTranslationalTouch( &s_MainWindow.ogl.interactor, .5f * ns_voxel_info_size_x( voxel_info ) );

	RepaintOpenGLWindow( ogl );
	}


void _open_gl_on_regenerate_splats( nsboolean from_spine_voxels )
	{
   ____splats_generate_from_spine_voxels = from_spine_voxels;

   _destroy_splats();
   _create_splats( &s_MainWindow.ogl, NULL );
	}


nsulong ____splats_threshold_old_value;
nsulong ____splats_threshold_current_value;
nsboolean ____splats_threshold_regenerate = NS_TRUE;

void _splats_threshold_set_values( HWND hDlg )
	{
	nschar number[64];

	SendMessage( GetDlgItem( hDlg, IDC_SPLATS_THRESHOLD_REGENERATE ), BM_SETCHECK,
		____splats_threshold_regenerate ? BST_CHECKED : BST_UNCHECKED, 0 );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), NS_FMT_ULONG, ____splats_threshold_current_value );
	SetDlgItemText( hDlg, IDE_SPLATS_THRESHOLD_VALUE, number );
	}

void _splats_threshold_get_values( HWND hDlg )
	{
	nschar number[64];

	____splats_threshold_regenerate =
		( nsboolean )SendMessage( GetDlgItem( hDlg, IDC_SPLATS_THRESHOLD_REGENERATE ), BM_GETCHECK, 0, 0 );

	GetDlgItemText( hDlg, IDE_SPLATS_THRESHOLD_VALUE, number, sizeof( number ) - 1 );
	____splats_threshold_current_value = ( nsulong )ns_atoi( number );
	}


#define _SPLATS_THRESHOLD_MIN  0
#define _SPLATS_THRESHOLD_MAX  255


INT_PTR CALLBACK _splats_threshold_dlg_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
				{
				case IDOK:
					_splats_threshold_get_values( hDlg );
					EndDialog( hDlg, 1 );
					return TRUE;

				case IDC_SPLATS_THRESHOLD_REGENERATE:
					____splats_threshold_regenerate = ! ____splats_threshold_regenerate;
					return TRUE;

				case IDB_SPLATS_THRESHOLD_DEFAULT:
					____splats_threshold_current_value = ____splats_default_threshold;
					_splats_threshold_set_values( hDlg );
					return TRUE;

				case IDCANCEL:
					EndDialog( hDlg, 0 );
					return TRUE;
				}
			return FALSE;

		case WM_INITDIALOG:
			CenterWindow( hDlg, s_MainWindow.ogl.hWnd );

			SendMessage(
				GetDlgItem( hDlg, ID_SPLATS_THRESHOLD_UD ),
				UDM_SETRANGE,
				0,
				MAKELONG( ( short )_SPLATS_THRESHOLD_MAX, ( short )_SPLATS_THRESHOLD_MIN )
				);

			_splats_threshold_set_values( hDlg );

			SetFocus( GetDlgItem( hDlg, IDE_SPLATS_THRESHOLD_VALUE ) );
			return FALSE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
		}

	return FALSE;
	}


void _write_splats_current_threshold( void )
   {
   NsConfigDb  db;
	NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

	if( ns_config_db_has_group( &db, "splats" ) )
		if( ns_config_db_has_key( &db, "splats", "current_threshold" ) )
			{
			/*TEMP*/
			ns_println(
				"Writing splats current threshold of " NS_FMT_ULONG " to configuration file...",
				____splats_current_threshold
				);

			ns_config_db_set_int( &db, "splats", "current_threshold", ( nsint )____splats_current_threshold );
			ns_config_db_write( &db, ____config_file );
			}

   ns_config_db_destruct( &db );
   }


void _open_gl_on_splats_threshold( void )
	{
	nsboolean  old_do_regenerate;
	nsint      ret_value;


	old_do_regenerate                  = ____splats_threshold_regenerate;
	____splats_threshold_old_value     = ____splats_current_threshold;
	____splats_threshold_current_value = ____splats_current_threshold;

	EnableWindow( s_MainWindow.hWnd, FALSE );
	ret_value = ( nsint )DialogBox( g_Instance, "SPLATS_THRESHOLD", s_MainWindow.ogl.hWnd, _splats_threshold_dlg_proc );
	EnableWindow( s_MainWindow.hWnd, TRUE );

	if( ret_value )
		{
		if( ____splats_threshold_current_value < _SPLATS_THRESHOLD_MIN )
			____splats_threshold_current_value = _SPLATS_THRESHOLD_MIN;
		else if( ____splats_threshold_current_value > _SPLATS_THRESHOLD_MAX )
			____splats_threshold_current_value = _SPLATS_THRESHOLD_MAX;

		ns_println( "regenerate: %s", ____splats_threshold_regenerate ? "yes" : "no" );
		ns_println( "current T : " NS_FMT_ULONG, ____splats_threshold_current_value );
		ns_println( "old     T : " NS_FMT_ULONG, ____splats_threshold_old_value );

		if( ____splats_threshold_current_value != ____splats_threshold_old_value )
			{
			____splats_current_threshold = ____splats_threshold_current_value;
			ns_println( "saved T: " NS_FMT_ULONG, ____splats_current_threshold );

			_write_splats_current_threshold();

			if( ____splats_threshold_regenerate )
				_open_gl_on_regenerate_splats( NS_FALSE );
			}
		}
	else
		____splats_threshold_regenerate = old_do_regenerate;
	}


void _enable_or_disable_splats_opacity( HWND dlg )
	{
	EnableWindow( GetDlgItem( dlg, IDE_SPLATS_OPACITY_VALUE ), ____use_splats_alpha );
	EnableWindow( GetDlgItem( dlg, IDS_SPLATS_OPACITY_VALUE ), ____use_splats_alpha );
	}


#define _SPLATS_ALPHA_SCALAR  100.0f


nsuint _splats_alpha_to_uint( void )
	{
	nsuint alpha = ( nsuint )( ____splats_alpha * _SPLATS_ALPHA_SCALAR );

	if( 100 < alpha )
		alpha = 100;

	return alpha;
	}


nsfloat _splats_alpha_to_float( void )
	{
	nsfloat alpha = ____splats_alpha * _SPLATS_ALPHA_SCALAR;

	if( 100.0f < alpha )
		alpha = 100.0f;

	return alpha;
	}


void _splats_opacity_show_value( HWND hDlg )
	{
	nschar number[64];

	//ns_println( "SPLATS ALPHA = " NS_FMT_DOUBLE, ____splats_alpha );

	ns_snprint(
		number,
		NS_ARRAY_LENGTH( number ),
		"%.0f%%",
		_splats_alpha_to_float()
		);

	SetDlgItemText( hDlg, IDE_SPLATS_OPACITY_VALUE, number );
	}


INT_PTR CALLBACK _splats_opacity_dlg_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
	switch( uMsg )
		{
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
				{
				case IDC_SC_ANN_ENABLE:
					____use_splats_alpha = ! ____use_splats_alpha;
					_enable_or_disable_splats_opacity( hDlg );
					____redraw_3d();
					break;

				case IDOK:
					EndDialog( hDlg, 1 );
					break;

				case IDCANCEL:
					EndDialog( hDlg, 0 );
					break;
				}
			return TRUE;


     case WM_HSCROLL:
        {
        int code     = ( int )LOWORD( wParam );
        int position = -1;

        if( TB_THUMBTRACK == code )
           position = ( short )HIWORD( wParam );
        else if( TB_LINEUP == code || TB_LINEDOWN == code || TB_PAGEUP == code || TB_PAGEDOWN == code )
           position = ( int )( SendMessage( ( HWND )lParam, TBM_GETPOS, 0, 0 ) );
      
			if( -1 != position )
				{
				____splats_alpha = position / _SPLATS_ALPHA_SCALAR;

				if( ____splats_alpha < 0.0f )
					____splats_alpha = 0.0f;
				else if( 1.0f < ____splats_alpha )
					____splats_alpha = 1.0f;

				_splats_opacity_show_value( hDlg );
				____redraw_3d();
				}
        }
        break;


		case WM_INITDIALOG:
			CenterWindow( hDlg, s_MainWindow.ogl.hWnd );

			SendMessage(
				GetDlgItem( hDlg, IDC_SPLATS_OPACITY_AUTOMATIC ),
				BM_SETCHECK,
				! ____use_splats_alpha ? BST_CHECKED : BST_UNCHECKED,
				0
				);

         SendMessage(
				GetDlgItem( hDlg, IDS_SPLATS_OPACITY_VALUE ),
            TBM_SETPOS,
            TRUE,
            _splats_alpha_to_uint()
            );

			_splats_opacity_show_value( hDlg );
			_enable_or_disable_splats_opacity( hDlg );

			if( ____use_splats_alpha )
				SetFocus( GetDlgItem( hDlg, IDE_SPLATS_OPACITY_VALUE ) );

			return FALSE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
		}

	return FALSE;
	}


void _open_gl_on_splats_opacity( void )
	{
	nsboolean  use_alpha;
	nsfloat    alpha;
	nsint      ret;


	use_alpha = ____use_splats_alpha;
	alpha     = ____splats_alpha;

	EnableWindow( s_MainWindow.hWnd, FALSE );
	ret = ( nsint )DialogBox( g_Instance, "SPLATS_OPACITY", s_MainWindow.ogl.hWnd, _splats_opacity_dlg_proc );
	EnableWindow( s_MainWindow.hWnd, TRUE );

	if( ! ret )
		{
		____use_splats_alpha = use_alpha;
		____splats_alpha     = alpha;
		}

	____redraw_3d();
	}


void _open_gl_on_optimize_splats_for_movement( void )
	{
	____optimize_splats_movement = ! ____optimize_splats_movement;
	SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_SPLATS_OPTIMIZE_FOR_MOVEMENT, ____optimize_splats_movement );
	}


void _OpenGLMenuSelection( OpenGLWindow *const ogl, void *userData )
   {
   switch( ogl->menuID )
      {
      //case IDM_GL_MODE_ZOOM:
        // _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_ZOOM, eDISPLAY_WINDOW_MOUSE_MODE_ZOOM, 0 ); 
         //break;

      case IDM_GL_MODE_TRANSLATE:
         _MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, 0 );
         break;

      case IDM_GL_MODE_SEED:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_SEED, eDISPLAY_WINDOW_MOUSE_MODE_SEED, 0 );
         break;

      case IDM_GL_MODE_RAYS:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_RAYS, eDISPLAY_WINDOW_MOUSE_MODE_RAYS, 0 );
         break;

      case IDM_GL_MODE_SPINE:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_SPINE, eDISPLAY_WINDOW_MOUSE_MODE_SPINE, 0 );
         break;

      case IDM_GL_MODE_NEURITE:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_NEURITE, eDISPLAY_WINDOW_MOUSE_MODE_NEURITE, 0 );
         break;

      case IDM_GL_MODE_ROI:
         _MainWindowOnSetMouseMode( 0, IDM_MODE_ROI, eDISPLAY_WINDOW_MOUSE_MODE_ROI, 0 );
			break;

      //case IDM_GL_MODE_MAGNET:
        // _MainWindowOnSetMouseMode( 0, IDM_MODE_MAGNET, DISPLAY_WINDOW_MOUSE_MODE_MAGNET, 0 ); 
         //break;


      case IDM_GL_VIEW_LIGHTING:
         s_MainWindow.lightingOn = ! s_MainWindow.lightingOn;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.lightingOn );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.lightingOn ) ?
               //                   "Lighting Off" : "Lighting On"
                 //               );
         break;

      //case IDM_GL_VIEW_TRANSPARENCY:
         //s_MainWindow.transparencyOn = ! s_MainWindow.transparencyOn;
         //SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.transparencyOn );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.transparencyOn ) ?
               //                   "Transparency Off" : "Transparency On"
                 //               );
         //break;

      case IDM_GL_VIEW_PALETTE:
         s_MainWindow.usePalette3D = ! s_MainWindow.usePalette3D;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.usePalette3D );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.usePalette3D ) ?
               //                   "Palette Off" : "Palette On"
                 //               );
         break;

      case IDM_GL_VIEW_AXIS:
         s_MainWindow.showAxis = ! s_MainWindow.showAxis;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.showAxis );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.showAxis ) ?
               //                   "Hide Axis'" : "Show Axis'"
                 //               );
         break;

      case IDM_GL_VIEW_ANTIALIASING:
         ____use_antialiasing = ! ____use_antialiasing;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, ____use_antialiasing );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( ____use_antialiasing ) ?
               //                   "Anti-Aliasing Off" : "Anti-Aliasing On"
                 //               );
         break;



      case IDM_GL_VIEW_GRID:
         ____view_3d_volume_grid = ! ____view_3d_volume_grid;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, ____view_3d_volume_grid );

			s_MainWindow.ogl_text = ____view_3d_volume_grid ? ____volume_grid_text : NULL;
         break;


      case IDM_GL_VIEW_BOUNDING_CUBE:
         s_MainWindow.showBoundingCube = ! s_MainWindow.showBoundingCube;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.showBoundingCube );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.showBoundingCube ) ?
               //                   "Hide Bounding Box" : "Show Bounding Box"
                 //               );
         break;


		case IDM_GL_ROI_BOUNDING_BOX:
			____show_roi_bounding_box = ! ____show_roi_bounding_box;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID,____show_roi_bounding_box );
			break;


      case IDM_GL_VIEW_OCTREE_SHOW_BOUNDING_BOX:
         s_MainWindow.show_octree_bounding_box = ! s_MainWindow.show_octree_bounding_box;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, s_MainWindow.show_octree_bounding_box );
         //SetOpenGLWindowMenuText( ogl,
           //                       ogl->menuID,
             //                     ( s_MainWindow.show_octree_bounding_box ) ?
               //                   "Hide Bounding Box" : "Show Bounding Box"
                 //               );
         break;


      case IDM_GL_VIEW_SPLATS:
         ____splats_view = ! ____splats_view;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, ____splats_view );
         //SetOpenGLWindowMenuText(
           // ogl,
            //ogl->menuID,
            //____splats_view ?
            //"Splats Off" : "Splats On"
            //);
         break;

      case IDM_GL_VIEW_SOLID_BACKGROUND:
         ____solid_background = ! ____solid_background;
         SetOpenGLWindowMenuChecked( ogl, ogl->menuID, ____solid_background );
         //SetOpenGLWindowMenuText(
         //   ogl,
         //   ogl->menuID,
         //   ____solid_background ?
         //   "Solid Background Off" : "Solid Background On"
         //   );
         break;


		case IDM_GL_VIEW_CENTER_ON_IMAGE:
		case IDM_GL_VIEW_CENTER_ON_ROI:
		case IDM_GL_VIEW_CENTER_ON_MODEL:
		case IDM_GL_VIEW_CENTER_ON_SEED:
			_open_gl_on_center_on( ogl, ogl->menuID );
			break;


      case IDM_GL_VIEW_OCTREE_OFF:
      case IDM_GL_VIEW_OCTREE_EMPTY_LEAFS:
      case IDM_GL_VIEW_OCTREE_NON_EMPTY_LEAFS:
      case IDM_GL_VIEW_OCTREE_ALL_LEAFS:
         _open_gl_on_view_octree_mode( ogl->menuID );
         break;


		case IDM_GL_SPLATS_THRESHOLD:
			_open_gl_on_splats_threshold();
			break;

		case IDM_GL_SPLATS_OPACITY:
			_open_gl_on_splats_opacity();
			break;

/*
      case IDM_SPLATS_CONTRAST_16:
         _open_gl_on_splats_contrast( ogl->menuID, 16 );
         break;

      case IDM_SPLATS_CONTRAST_32:
         _open_gl_on_splats_contrast( ogl->menuID, 32 );
         break;

      case IDM_SPLATS_CONTRAST_48:
         _open_gl_on_splats_contrast( ogl->menuID, 48 );
         break;

      case IDM_SPLATS_CONTRAST_64:
         _open_gl_on_splats_contrast( ogl->menuID, 64 );
         break;
*/

      //case IDM_GL_VIEW_POLYGON_SOLID:
        // _OpenGLOnViewPolygonMode( 1 );
         //break;

      //case IDM_GL_VIEW_POLYGON_WIREFRAME:
        // _OpenGLOnViewPolygonMode( 0 );
         //break;

      case IDM_GL_VIEW_POLYGONS_COMPLEXITY_LOWEST:
         _OpenGLOnViewPolygonsComplexity( _OPENGL_LOWEST_COMPLEXITY, ogl->menuID );
         break;

      case IDM_GL_VIEW_POLYGONS_COMPLEXITY_LOW:
         _OpenGLOnViewPolygonsComplexity( _OPENGL_LOW_COMPLEXITY, ogl->menuID );
         break;

      case IDM_GL_VIEW_POLYGONS_COMPLEXITY_MEDIUM:
         _OpenGLOnViewPolygonsComplexity( _OPENGL_MEDIUM_COMPLEXITY, ogl->menuID );
         break;

      case IDM_GL_VIEW_POLYGONS_COMPLEXITY_HIGH:
         _OpenGLOnViewPolygonsComplexity( _OPENGL_HIGH_COMPLEXITY, ogl->menuID );
         break;

      case IDM_GL_VIEW_POLYGONS_COMPLEXITY_HIGHEST:
         _OpenGLOnViewPolygonsComplexity( _OPENGL_HIGHEST_COMPLEXITY, ogl->menuID );
         break;

      case IDM_GL_SPLATS_REGENERATE:
			_open_gl_on_regenerate_splats( NS_FALSE );
         break;

      case IDM_GL_SPLATS_REGENERATE_FROM_SPINE_VOXELS:
         _open_gl_on_regenerate_splats( NS_TRUE );
         break;

      case IDM_GL_SPLATS_SURFACE_ONLY:
         ____splats_is_volume = ! ____splats_is_volume;
         SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ogl->menuID, ! ____splats_is_volume );
         break;

		case IDM_GL_SPLATS_OPTIMIZE_FOR_MOVEMENT:
			_open_gl_on_optimize_splats_for_movement();
			break;

		case IDM_GL_VIEW_RESET:
			_open_gl_on_view_reset( &s_MainWindow.ogl, NS_XY );
			break;

		case IDM_GL_VIEW_RESET_SIDE:
			_open_gl_on_view_reset( &s_MainWindow.ogl, NS_ZY );
			break;

		case IDM_GL_VIEW_RESET_BOTTOM:
			_open_gl_on_view_reset( &s_MainWindow.ogl, NS_XZ );
			break;
      }

   RepaintOpenGLWindow( ogl );
   }


void _open_gl_pre_nc_destroy( OpenGLWindow *ogl, nspointer user_data )
   {
   //clear_with_gradient_finalize();
   }


void _OpenGLPostNcDestroy( OpenGLWindow *const ogl, void *userData )
   {
   s_MainWindow.oglActive = 0;
   SetMenuItemEnabled( s_MainWindow.hWnd, IDM_VIEW_3D_MODEL, 1 );

   ns_frames_per_second_destruct( &____fps );

   _destroy_splats();
   ns_splats_destruct( &____splats );

   ____num_radii = 0;
   }


extern void _OpenGLWindowOnPaint( OpenGLWindow *ogl );


nsuint ____frame_count;


void _run_frame_rate_loop( OpenGLWindow *ogl, void *args )
   {
   ____frame_count = 0;

   NS_INFINITE_LOOP
      {
      if( GetAsyncKeyState( 'G' ) & 0x8000 )
         break;

      _OpenGLWindowOnPaint( ogl );
      ++____frame_count;
      }
   }



void _write_camera_position( OpenGLWindow *ogl, void *args )
   {
   NsConfigDb  db;
	NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   ns_config_db_set_int( &db, "interactor", "imode", ogl->interactor.imode );
   ns_config_db_set_double( &db, "interactor", "idtouch", ogl->interactor.idtouch );
   ns_config_db_set_double( &db, "interactor", "iatouch", ogl->interactor.iatouch );
   ns_config_db_set_double( &db, "interactor", "ixpos", ogl->interactor.ixpos );
   ns_config_db_set_double( &db, "interactor", "iypos", ogl->interactor.iypos );
   ns_config_db_set_double( &db, "interactor", "izpos", ogl->interactor.izpos );
   ns_config_db_set_double( &db, "interactor", "iorientation", ogl->interactor.iorientation );
   ns_config_db_set_double( &db, "interactor", "itilt", ogl->interactor.itilt );
   ns_config_db_set_int( &db, "interactor", "iprevx", ogl->interactor.iprevx );
   ns_config_db_set_int( &db, "interactor", "iprevy", ogl->interactor.iprevy );
   ns_config_db_set_int( &db, "interactor", "interacting", ogl->interactor.interacting );

   ns_config_db_set_double( &db, "interactor", "imatrix0", ogl->interactor.imatrix[0] );
   ns_config_db_set_double( &db, "interactor", "imatrix1", ogl->interactor.imatrix[1] );
   ns_config_db_set_double( &db, "interactor", "imatrix2", ogl->interactor.imatrix[2] );
   ns_config_db_set_double( &db, "interactor", "imatrix3", ogl->interactor.imatrix[3] );
   ns_config_db_set_double( &db, "interactor", "imatrix4", ogl->interactor.imatrix[4] );
   ns_config_db_set_double( &db, "interactor", "imatrix5", ogl->interactor.imatrix[5] );
   ns_config_db_set_double( &db, "interactor", "imatrix6", ogl->interactor.imatrix[6] );
   ns_config_db_set_double( &db, "interactor", "imatrix7", ogl->interactor.imatrix[7] );
   ns_config_db_set_double( &db, "interactor", "imatrix8", ogl->interactor.imatrix[8] );
   ns_config_db_set_double( &db, "interactor", "imatrix9", ogl->interactor.imatrix[9] );
   ns_config_db_set_double( &db, "interactor", "imatrix10", ogl->interactor.imatrix[10] );
   ns_config_db_set_double( &db, "interactor", "imatrix11", ogl->interactor.imatrix[11] );
   ns_config_db_set_double( &db, "interactor", "imatrix12", ogl->interactor.imatrix[12] );
   ns_config_db_set_double( &db, "interactor", "imatrix13", ogl->interactor.imatrix[13] );
   ns_config_db_set_double( &db, "interactor", "imatrix14", ogl->interactor.imatrix[14] );
   ns_config_db_set_double( &db, "interactor", "imatrix15", ogl->interactor.imatrix[15] );

   ns_config_db_write( &db, ____config_file );
   ns_config_db_destruct( &db );
   }


nsint ____x = 0;

void _opengl_rotate_left( OpenGLWindow *ogl, void *args )
   {
   StartInteractor( &ogl->interactor, ogl->hWnd, ____x, ogl->interactor.iprevy );
   ____x += ____rotation_speed;
   UpdateInteractor( &ogl->interactor, ____x, ogl->interactor.iprevy );
   EndInteractor( &ogl->interactor );
   RepaintOpenGLWindow( ogl );
   }

void _opengl_rotate_right( OpenGLWindow *ogl, void *args )
   {
   StartInteractor( &ogl->interactor, ogl->hWnd, ____x, ogl->interactor.iprevy );
   ____x -= ____rotation_speed;
   UpdateInteractor( &ogl->interactor, ____x, ogl->interactor.iprevy );
   EndInteractor( &ogl->interactor );
   RepaintOpenGLWindow( ogl );
   }


void _opengl_begin_rotation( OpenGLWindow *ogl, void *args )
   {
   open_gl_begin_timer( ogl, 20/*2000*/ );
   }


void _opengl_end_rotation( OpenGLWindow *ogl, void *args )
   {
   open_gl_end_timer( ogl );
   }


void _read_camera_position( OpenGLWindow *ogl, void *args )
   {
   NsConfigDb  db;
	NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   ogl->interactor.imode        = ns_config_db_get_int( &db, "interactor", "imode" );
   ogl->interactor.idtouch      = ( nsfloat )ns_config_db_get_double( &db, "interactor", "idtouch" );
   ogl->interactor.iatouch      = ( nsfloat )ns_config_db_get_double( &db, "interactor", "iatouch" );
   ogl->interactor.ixpos        = ( nsfloat )ns_config_db_get_double( &db, "interactor", "ixpos" );
   ogl->interactor.iypos        = ( nsfloat )ns_config_db_get_double( &db, "interactor", "iypos" );
   ogl->interactor.izpos        = ( nsfloat )ns_config_db_get_double( &db, "interactor", "izpos" );
   ogl->interactor.iorientation = ( nsfloat )ns_config_db_get_double( &db, "interactor", "iorientation" );
   ogl->interactor.itilt        = ( nsfloat )ns_config_db_get_double( &db, "interactor", "itilt" );
   ogl->interactor.iprevx       = ns_config_db_get_int( &db, "interactor", "iprevx" );
   ogl->interactor.iprevy       = ns_config_db_get_int( &db, "interactor", "iprevy" );
   ogl->interactor.interacting  = ns_config_db_get_int( &db, "interactor", "interacting" );

   ogl->interactor.imatrix[0] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix0" );
   ogl->interactor.imatrix[1] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix1" );
   ogl->interactor.imatrix[2] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix2" );
   ogl->interactor.imatrix[3] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix3" );
   ogl->interactor.imatrix[4] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix4" );
   ogl->interactor.imatrix[5] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix5" );
   ogl->interactor.imatrix[6] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix6" );
   ogl->interactor.imatrix[7] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix7" );
   ogl->interactor.imatrix[8] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix8" );
   ogl->interactor.imatrix[9] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix9" );
   ogl->interactor.imatrix[10] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix10" );
   ogl->interactor.imatrix[11] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix11" );
   ogl->interactor.imatrix[12] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix12" );
   ogl->interactor.imatrix[13] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix13" );
   ogl->interactor.imatrix[14] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix14" );
   ogl->interactor.imatrix[15] = ( nsfloat )ns_config_db_get_double( &db, "interactor", "imatrix15" );

   ns_config_db_destruct( &db );

   RepaintOpenGLWindow( ogl );
   }


//nssize ____num_selected;





/*
void _do_select_model_vertex_3d( NsModel *model, const NsRay3d *R )
   {
   nsmodelvertex  curr, end, closest;
   NsSphered      S;
   NsPoint3d      I;
   nsdouble       distance, min_distance;


   closest      = NS_MODEL_VERTEX_NIL;
   min_distance = NS_DOUBLE_MAX;

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      {
      ns_model_vertex_to_sphere_ex( curr, &S );

      if( ns_ray3d_intersects_sphere( R, &S, &I ) )
         {
         distance = ns_vector3d_distance( &R->O, &I );

         if( distance < min_distance )
            {
            min_distance = distance;
            closest      = curr;
            }
         }
      }

   if( ns_model_vertex_not_equal( closest, NS_MODEL_VERTEX_NIL ) )
      ns_model_vertex_mark_selected(
         closest,
         ! ns_model_vertex_is_selected( closest )
         );
   }
*/

/*
void _do_select_model_spine_3d( NsModel *model, NsRay3d *R )
   {
   nsspine     curr, end, closest;
	NsSphered   S;
   NsPoint3d   I;
   nsdouble    distance, min_distance;


   closest      = NS_SPINE_NIL;
   min_distance = NS_DOUBLE_MAX;

   curr = ns_model_begin_spines( model );
   end  = ns_model_end_spines( model );

   for( ; ns_spine_not_equal( curr, end ); curr = ns_spine_next( curr ) )
      {
      ns_spine_to_sphere_ex( curr, &S );

      if( ns_ray3d_intersects_sphere( R, &S, &I ) )
         {
         distance = ns_vector3d_distance( &R->O, &I );

         if( distance < min_distance )
            {
            min_distance = distance;
            closest      = curr;
            }
         }
      }

   if( ns_spine_not_equal( closest, NS_SPINE_NIL ) )
      ns_spine_mark_selected(
         closest,
         ! ns_spine_is_selected( closest )
         );
   }
*/


/*
void _do_select_sample_3d( NsSampler *sampler, NsRay3d *R )
   {
   nspointer  curr, end, closest;
   NsSphered  S;
   NsPoint3d  I;
   nsdouble   distance, min_distance;


   closest      = NS_SAMPLE_NIL;
   min_distance = NS_DOUBLE_MAX;

   curr = ns_sampler_begin( sampler );
   end  = ns_sampler_end( sampler );

   for( ; ns_sample_not_equal( curr, end ); curr = ns_sample_next( curr ) )
      {
      ns_sample_to_sphere_ex( sampler, curr, &S );

      if( ns_ray3d_intersects_sphere( R, &S, &I ) )
         {
         distance = ns_vector3d_distance( &R->O, &I );

         if( distance < min_distance )
            {
            min_distance = distance;
            closest      = curr;
            }
         }
      }

   if( ns_sample_not_equal( closest, NS_SAMPLE_NIL ) )
      {
/* TEMP!!!!!!!!!
NsVector3f C;
NsVector3i S1;
Vector3i S2;
ns_sample_get_center( closest, &C );
____use_seed_as_3d_pivot = NS_TRUE;
ns_to_image_space( &C, &S1, workspace_get_voxel_info( s_MainWindow.activeWorkspace ) );
S2.x = S1.x; S2.y = S1.y; S2.z = S1.z;
SetWorkspaceSeed( s_MainWindow.activeWorkspace, &S2 );*//*

      ns_sample_set_is_selected(
         closest,
         ! ns_sample_get_is_selected( closest )
         );
      }
   }
*/

//NsRay3d ____splat_selecting_ray;

extern void _show_seed_on_status_bar( void );
/*
void _do_select_seed_3d_func( void *dialog )
   {
   NsProgress          nsprogress;
   Progress            progress;
	const NsImage      *volume;
	const NsVoxelInfo  *voxel_info;
	NsVector3d          S;
	NsVoxelBuffer       voxels;
	nsulong             intensity;
	NsError             error;


	g_Progress = dialog;
   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );
	volume     = workspace_volume( s_MainWindow.activeWorkspace );

	if( ns_splats_find_closest_by_ray(
			&____splats,
			voxel_info,
			&____splat_selecting_ray,
			&S,
			&nsprogress
			) )
		{
		NsRayburst rayburst;

		if( NS_SUCCESS(
				ns_rayburst_construct(
					&rayburst,
					NS_RAYBURST_KERNEL_SINGLE_3D,
					NS_RAYBURST_RADIUS_MLBD,
					NS_RAYBURST_INTERP_BILINEAR,
					volume,
					voxel_info,
					NULL
					),
				error ) )
			{
			NsVector3f  P, N, E, M;
			NsPoint3i   I;
			Vector3i    seed;


			ns_vector3d_to_3f( &____splat_selecting_ray.D, &N );
			ns_vector3f_norm( &N );
			ns_rayburst_init_single_ex( &rayburst, N.x, N.y, N.z );

			ns_vector3d_to_3f( &S, &P );
			ns_to_image_space( &P, &I, voxel_info );

			ns_voxel_buffer_init( &voxels, volume );
			intensity = ns_voxel_get( &voxels, I.x, I.y, I.z );

/*TEMP*///ns_println( "Setting rayburst intensity at " NS_FMT_DOUBLE, ( nsfloat )intensity * 0.999f );

			/* Only pass 99.9% of the splats threshold value to avoid the rayburst from
				terminating at the origin due to roundoff error in the interpolation of
				the threshold at the origin. See also nsrayburst.c. *//*
			ns_rayburst_set_threshold( &rayburst, ( nsfloat )intensity * 0.999f );
			ns_rayburst( &rayburst, &P );

			if( 0.0f < ns_rayburst_radius( &rayburst ) )
				{
				ns_assert( 1 == ns_rayburst_num_samples( &rayburst ) );
				ns_rayburst_sample_points( &rayburst, NS_TRUE, &E );

				ns_vector3f_add( &M, &P, &E );
				ns_vector3f_cmpd_scale( &M, 0.5f );
				
				ns_to_image_space( &M, &I, voxel_info );

				seed.x = I.x;
				seed.y = I.y;
				seed.z = I.z;

				SetWorkspaceSeed( s_MainWindow.activeWorkspace, &seed );
				_show_seed_on_status_bar();
				}
			/*TEMP*//*else
				ns_println( "3D::Rayburst radius was zero." );

			ns_rayburst_destruct( &rayburst );
			}
		}
	/*TEMP*//*else
		ns_println( "3D::No splat intersection found." );

   EndProgressDialog( dialog, 0 );
   }*/

/*
void _do_select_seed_3d( OpenGLWindow *ogl, NsRay3d *R )
   {
   nsint wasCancelled;

	if( ____splats_init )
		{
		EnableWindow( ogl->hOwner, FALSE );

		____splat_selecting_ray = *R;
		ProgressDialog( ogl->hWnd, NULL, _do_select_seed_3d_func, NULL, NULL, &wasCancelled );

		EnableWindow( ogl->hOwner, TRUE );
		}
   }
*/

/*
void _select_3d( OpenGLWindow *ogl, void *user_data )
   {
   GLdouble  xi, yi, xn, yn, zn, xf, yf, zf;
   NsRay3d   R;
   GLint     viewport[4];


   glGetIntegerv( GL_VIEWPORT, viewport );

   xi = ogl->mouseX;
   yi = viewport[3] - ogl->mouseY;

   gluUnProject( xi, yi, 0.0, ____modelview_matrix, ____projection_matrix, viewport, &xn, &yn, &zn );
   gluUnProject( xi, yi, 1.0, ____modelview_matrix, ____projection_matrix, viewport, &xf, &yf, &zf );

   R.O.x = xn;
   R.O.y = yn;
   R.O.z = zn;

   R.D.x = xf - xn;
   R.D.y = yf - yn;
   R.D.z = zf - zn;

   ns_vector3d_norm( &R.D );

   switch( s_MainWindow.mouseMode )
      {
      case eDISPLAY_WINDOW_MOUSE_MODE_RAYS:
         _do_select_sample_3d( workspace_sampler( s_MainWindow.activeWorkspace ), &R );
         break;

      case eDISPLAY_WINDOW_MOUSE_MODE_SPINE:
         _do_select_model_spine_3d( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), &R ); 
         break;

      case eDISPLAY_WINDOW_MOUSE_MODE_NEURITE:
         _do_select_model_vertex_3d( GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ), &R );
         break;

		case eDISPLAY_WINDOW_MOUSE_MODE_SEED:
			_do_select_seed_3d( ogl, &R );
			break;
      }

   ____redraw_all();
	}
*/

nsfloat _find_model_largest_z( void )
   {
   NsModel *model;
   nsmodelvertex curr_vertex;
   nsmodelvertex end_vertices;
   nsfloat max;
   nsfloat z;

   max = 0.0f;

   model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

   curr_vertex = ns_model_begin_vertices( model );
   end_vertices = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
          curr_vertex = ns_model_vertex_next( curr_vertex ) )
      {
      z = ns_model_vertex_get_position_z( curr_vertex );

      if( z > max )
         max = z;
      }

   return max;
   }


void just_call_for_opengl_repaint( OpenGLWindow *ogl, nspointer user )
   {  RepaintOpenGLWindow( ogl );  }


nsboolean ____3d_view_is_init = NS_FALSE;

void _init_3d_view( void )
	{
	if( ____3d_view_is_init )
		return;

	____3d_view_is_init = NS_TRUE;

   s_MainWindow.octree_mode      = IDM_GL_VIEW_OCTREE_OFF;
   s_MainWindow.lightingOn       = 1;
   //s_MainWindow.transparencyOn   = 1;
   s_MainWindow.usePalette3D     = 0;
   s_MainWindow.polygonsSolid    = 1;
   s_MainWindow.showAxis         = 0;
   s_MainWindow.showBoundingCube = 0;
	____show_roi_bounding_box = NS_FALSE;
   s_MainWindow.show_octree_bounding_box = 0;
   s_MainWindow.complexity       = _OPENGL_LOW_COMPLEXITY;
   s_MainWindow.complexityMenuID = IDM_GL_VIEW_POLYGONS_COMPLEXITY_LOW;

   //____splats_contrast_menu_id = IDM_SPLATS_CONTRAST_16;
   //____splats_current_threshold         = 16;
   ____splats_is_volume        = NS_FALSE;
   ____splats_view             = NS_TRUE;

	____view_3d_volume_grid = NS_FALSE;
	____volume_grid_step    = -1.0f;
   s_MainWindow.ogl_text   = NULL;

   ____solid_background = NS_FALSE;

	____opengl_centering_mode = IDM_GL_VIEW_CENTER_ON_IMAGE;
	//____use_model_3d_rotation_center = NS_FALSE;

	____use_antialiasing = NS_FALSE;

	____splats_alpha = 1.0f;
	____use_splats_alpha = NS_FALSE;

	____optimize_splats_movement = NS_TRUE;
	}


void _MainWindowOnView3DModel( void )
   {
//   nsuint i;
   //NsModel *model;
   //NsOctreeStats stats;
//   NsVector3f voxel_size;
   //nsfloat c;
   RECT rc;


   if( s_MainWindow.oglActive )
      return;

	____splats_generate = NS_TRUE;

	if( ! WorkspaceDisplayFlagIsSet( s_MainWindow.activeWorkspace, mDISPLAY_GRAPHICS_IMAGE ) )
		____splats_generate =
			( IDYES ==
				MessageBox(
				s_MainWindow.hWnd,
				"The menu item \"View >> Image\" is not checked.\nDo you want to generate the volume rendering?",
				"NeuronStudio",
				MB_YESNO | MB_ICONQUESTION ) );

   InitOpenGLWindow( &s_MainWindow.ogl, g_Instance, s_MainWindow.hWnd );

   GetWindowRect( s_MainWindow.hWnd, &rc );

/* TEMP????? */
   s_MainWindow.ogl.x = rc.left + 64;
   s_MainWindow.ogl.y = rc.top + 128;
   s_MainWindow.ogl.width = ( rc.right - rc.left ) / 2;
   s_MainWindow.ogl.height = ( rc.bottom - rc.top ) / 2;

   ns_frames_per_second_construct( &____fps, NS_FRAMES_PER_SECOND_DEFAULT_INTERVAL );

   s_MainWindow.ogl.title = s_MainWindow.path;

   _reset_interactor( NS_XY );

   //s_MainWindow.ogl.interactor.izpos = 300.0f;
   s_MainWindow.ogl.menuName         = "OPENGL_WINDOW_MENU";
   s_MainWindow.ogl.iconName         = "APP_ICON";
   s_MainWindow.ogl.viewTransform    = _OpenGLViewTransform;
   s_MainWindow.ogl.renderFrame      = _OpenGLRenderFrame;
   //s_MainWindow.ogl.mouseMode        = OGLWIN_MOUSE_MODE_INTERACTOR;
   //s_MainWindow.ogl.mouseLButtonUp   = _OpenGLOnLButtonUp;
   s_MainWindow.ogl.menuSelection    = _OpenGLMenuSelection;
   s_MainWindow.ogl.preNcDestroy     = _open_gl_pre_nc_destroy;
   s_MainWindow.ogl.postNcDestroy    = _OpenGLPostNcDestroy;
	//s_MainWindow.ogl.mouseLButtonDblClk = _select_3d;

//s_MainWindow.ogl.keyMap[ OGLWIN_KEY_V ] = _write_camera_position;//_decrease_x;
//s_MainWindow.ogl.keyMap[ OGLWIN_KEY_B ] = _read_camera_position;//_increase_x;

   s_MainWindow.ogl.on_timer = /*just_call_for_opengl_repaint;*/_opengl_rotate_right;
   //s_MainWindow.ogl.keyMap[ OGLWIN_KEY_T ] = _opengl_begin_rotation;
   //s_MainWindow.ogl.keyMap[ OGLWIN_KEY_K ] = _opengl_end_rotation;
   //s_MainWindow.ogl.keyMap[ OGLWIN_KEY_LEFT ] = _opengl_rotate_left;
   //s_MainWindow.ogl.keyMap[ OGLWIN_KEY_RIGHT ] = _opengl_rotate_right;

   //s_MainWindow.ogl.keyMap[ OGLWIN_KEY_R ] = _opengl_rayburst_animation;

	_init_3d_view();

	/* This one needs to be calculated every time. */
	____volume_grid_step = -1.0f;

   InitOpenGLLight( &s_MainWindow.light0,
                    0,
                    0.0f, 1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 1.0f,
                    0.1f, 0.1f, 0.1f, 1.0f,
                    //0.5f, 0.5f, 0.5f, 1.0f
                    1.0f, 1.0f, 1.0f, 1.0f
                   );

	____alert_user_about_splats = NS_FALSE;

   if( CreateOpenGLWindow( &s_MainWindow.ogl ) )
      {
      const NsVoxelInfo *voxel_info;

		_MainWindowOnSetMouseMode( 0, IDM_FILE_MODE_TRANSLATE, eDISPLAY_WINDOW_MOUSE_MODE_DRAG, 0 );
		SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_MODE_TRANSLATE, 1 );

		voxel_info = workspace_get_voxel_info( s_MainWindow.activeWorkspace );

      SetInteractorRotationalTouch( &s_MainWindow.ogl.interactor, .5f );
      SetInteractorTranslationalTouch( &s_MainWindow.ogl.interactor, .5f * ns_voxel_info_size_x( voxel_info ) );

      s_MainWindow.oglActive = 1;
      SetMenuItemEnabled( s_MainWindow.hWnd, IDM_VIEW_3D_MODEL, 0 );

		SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, s_MainWindow.octree_mode, 1 );

		SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____opengl_centering_mode, NS_TRUE );

        SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_AXIS, s_MainWindow.showAxis );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
                         //IDM_GL_VIEW_AXIS,
                         //( s_MainWindow.showAxis ) ?
                         //"Hide Axis'" : "Show Axis'"
                       //);


      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_BOUNDING_CUBE, s_MainWindow.showBoundingCube );
		SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_ROI_BOUNDING_BOX, ____show_roi_bounding_box );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_BOUNDING_CUBE,
    //                     ( s_MainWindow.showBoundingCube ) ?
      //                   "Hide Bounding Box" : "Show Bounding Box"
        //               );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_OCTREE_SHOW_BOUNDING_BOX, s_MainWindow.show_octree_bounding_box );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_OCTREE_SHOW_BOUNDING_BOX,
    //                     ( s_MainWindow.show_octree_bounding_box ) ?
      //                   "Hide Bounding Box" : "Show Bounding Box"
        //               );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_LIGHTING, s_MainWindow.lightingOn );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_LIGHTING,
    //                     ( s_MainWindow.lightingOn ) ?
      //                   "Lighting Off" : "Lighting On"
      //                 );

      //SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_TRANSPARENCY, s_MainWindow.transparencyOn );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_TRANSPARENCY,
    //                     ( s_MainWindow.transparencyOn ) ?
      //                   "Transparency Off" : "Transparency On"
        //               );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_PALETTE, s_MainWindow.usePalette3D );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_PALETTE,
    //                     ( s_MainWindow.usePalette3D ) ?
      //                   "Palette Off" : "Palette On"
        //               );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_ANTIALIASING, ____use_antialiasing );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_ANTIALIASING,
    //                     ( ____use_antialiasing ) ?
      //                   "Anti-Aliasing Off" : "Anti-Aliasing On"
        //               );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_SPLATS, ____splats_view );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_SPLATS,
    //                     ( ____splats_view ) ?
      //                   "Splats Off" : "Splats On"
        //               );


      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_GRID, ____view_3d_volume_grid );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_SOLID_BACKGROUND, ____solid_background );
//SetOpenGLWindowMenuText( &s_MainWindow.ogl,
  //                       IDM_GL_VIEW_SOLID_BACKGROUND,
    //                     ( ____solid_background ) ?
      //                   "Solid Background Off" : "Solid Background On"
        //               );

      //SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_CYLINDERS_AND_SPHERES, 1 );



/* TEMP!!!! Always keep these set to these values upon opening the 3D window. */
s_MainWindow.complexity       = _OPENGL_LOW_COMPLEXITY;
s_MainWindow.complexityMenuID = IDM_GL_VIEW_POLYGONS_COMPLEXITY_LOW;
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_POLYGON_SOLID, 1 );
      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_VIEW_POLYGONS_COMPLEXITY_LOW, 1 );

		SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_SPLATS_OPTIMIZE_FOR_MOVEMENT, ____optimize_splats_movement );

      //SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, ____splats_contrast_menu_id, 1 );

      SetOpenGLWindowMenuChecked( &s_MainWindow.ogl, IDM_GL_SPLATS_SURFACE_ONLY, ! ____splats_is_volume );

      ns_assert( ! ____splats_init );
      ns_splats_construct( &____splats );

      ____splats_generate_from_spine_voxels = NS_FALSE;

      //if( WorkspaceDisplayFlagIsSet( s_MainWindow.activeWorkspace, mDISPLAY_GRAPHICS_IMAGE ) )
      _create_splats( &s_MainWindow.ogl, NULL );
      }
   else
      MessageBox( s_MainWindow.hWnd, "Couldnt launch 3D viewer", "NeuronStudio", MB_OK | MB_ICONERROR );
   }



nsint ____main_window_left;
nsint ____main_window_right;
nsint ____main_window_top;
nsint ____main_window_bottom;
nsulong ____main_window_flags;
#define __MAIN_WINDOW_LEFT 0x01
#define __MAIN_WINDOW_RIGHT 0x02
#define __MAIN_WINDOW_TOP 0x04
#define __MAIN_WINDOW_BOTTOM 0x08
#define __MAIN_WINDOW_ALL 0xf
nsboolean ____main_window_maximized;



eERROR_TYPE _CreateMainWindow
   (
   const nsint   x,
   const nsint   y,
   const nsuint  width,
   const nsuint  height
   )
   {
   WNDCLASSEX wcx;

   wcx.cbSize        = sizeof( WNDCLASSEX );
   wcx.style         = CS_HREDRAW | CS_VREDRAW;
   wcx.lpfnWndProc   = _MainWindowProcedure;
   wcx.cbClsExtra    = 0;
   wcx.cbWndExtra    = 0;
   wcx.hIcon         = LoadIcon( g_Instance, "APP_ICON" );
   wcx.hCursor       = LoadCursor( NULL, IDC_ARROW );
   wcx.hInstance     = g_Instance;
   wcx.hbrBackground = ( HBRUSH )( COLOR_BTNFACE + 1 );//NULL;
   wcx.lpszClassName = "MainWindow";
   wcx.lpszMenuName  = "MAIN_WINDOW_MENU";
   wcx.hIconSm       = NULL;

   if( 0 == RegisterClassEx( &wcx ) )
      return ( g_Error = eERROR_OPER_SYSTEM );

   s_MainWindow.hWnd = CreateWindowEx( 0,
                                       "MainWindow",
                                       "NeuronStudio",
                                       WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                       x,y,width,height,
                                       NULL, NULL, g_Instance, NULL
                                     );

   if( NULL == s_MainWindow.hWnd )
      return ( g_Error = eERROR_OPER_SYSTEM );

   CreateStatusBar( s_MainWindow.hWnd, g_Instance );
   SetStatusBarParts();

   if( __MAIN_WINDOW_ALL == ____main_window_flags )
      MoveWindow(
         s_MainWindow.hWnd,
         ____main_window_left,
         ____main_window_top,
         ____main_window_right - ____main_window_left,
         ____main_window_bottom - ____main_window_top,
         FALSE
         );

   s_MainWindow.activeWorkspace = 0;
   s_MainWindow.path[ 0 ]       = '\0';
   s_MainWindow.mouseMode       = eDISPLAY_WINDOW_NO_MOUSE_MODE;
   s_MainWindow.oglActive       = 0;

   ShowWindow( s_MainWindow.hWnd, ____main_window_maximized ? SW_MAXIMIZE : SW_SHOW );
   UpdateWindow( s_MainWindow.hWnd );

   return eNO_ERROR;
   }


eERROR_TYPE _CreateApplication
   (
   const nsint   x,
   const nsint   y,
   const nsuint  width,
   const nsuint  height
   )
   {
   MSG msg;

   if( eNO_ERROR != _CreateMainWindow( x, y, width, height ) )
      return g_Error;

   while( 0 < GetMessage( &msg, NULL, 0, 0 ) )
      {
      TranslateMessage( &msg );
      DispatchMessage( &msg ); 
      }

   return eNO_ERROR;

   }/* _CreateApplication() */



nsboolean _gui_assert( const nschar *expression, const nschar *module, const nschar *details_or_file, nsint line )
   {
  // nschar buffer[1024];
   //FILE *fp;
   //time_t t;


   /*ns_snprint( buffer, sizeof( buffer ) - 1,
              "ASSERTION: %s\n"
              "MODULE: %s\n"
              "DETAILS/FILE: %s\n"
              "LINE: %d\n\n",
               expression, module, details_or_file, line );

ns_println( NS_FMT_STRING, buffer );
*/
	ns_print(
		"ASSERTION: %s\n"
		"MODULE: %s\n"
		"DETAILS/FILE: %s\n"
		"LINE: %d\n\n",
		expression,
		module,
		details_or_file,
		line
		);


   //if( NULL != ( fp = fopen( "C:\\_neuron_studio_assert_failure.log", "a" ) ) )
   //   {
      //t = time( NULL );
      //fprintf( fp, ctime( &t ) );      
      //fprintf( fp, buffer );
      //fclose( fp );
   //   }

   //MessageBox( s_MainWindow.hWnd, buffer, "NeuronStudio", MB_OK | MB_ICONERROR );


   ns_log_entry(
		NS_LOG_ENTRY_ERROR,
		"( assertion=\"%s\""
		", module=%s"
		", details/file=\"%s\""
		", line=%d )",
		expression,
		module,
		details_or_file,
		line
		);

   return NS_TRUE;
   }


//extern void ____spines_destruct( void );
//extern void ____ns_model_graft_destruct( void );
#include <direct.h>


#include <ext/lock.h>


nsboolean ____enable_log;
nsboolean ____flush_log_after_entry;

nsboolean _read_config( /*nschar *license*/void )
   {
   NsConfigDb  db;
	RECT        screen;
	nsint       delta;
	nsint       window_width, window_height;
	nsint       screen_width, screen_height;
   NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return NS_FALSE;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   //if( ! ns_config_db_has_group( &db, "license" ) )
     // {
      //ns_config_db_destruct( &db );
      //return NS_FALSE;
      //}
   
   //if( ! ns_config_db_has_key( &db, "license", "value" ) )
     // {
      //ns_config_db_destruct( &db );
      //return NS_FALSE;
      //}

   ____main_window_flags     = 0;
   ____main_window_maximized = NS_FALSE;

   if( ns_config_db_has_group( &db, "window" ) )
      {
      if( ns_config_db_has_key( &db, "window", "left" ) )
         {
         ____main_window_flags |= __MAIN_WINDOW_LEFT;
         ____main_window_left = ns_config_db_get_int( &db, "window", "left" );
         }

      if( ns_config_db_has_key( &db, "window", "right" ) )
         {
         ____main_window_flags |= __MAIN_WINDOW_RIGHT;
         ____main_window_right = ns_config_db_get_int( &db, "window", "right" );
         }

      if( ns_config_db_has_key( &db, "window", "top" ) )
         {
         ____main_window_flags |= __MAIN_WINDOW_TOP;
         ____main_window_top = ns_config_db_get_int( &db, "window", "top" );
         }

      if( ns_config_db_has_key( &db, "window", "bottom" ) )
         {
         ____main_window_flags |= __MAIN_WINDOW_BOTTOM;
         ____main_window_bottom = ns_config_db_get_int( &db, "window", "bottom" );
         }

      if( ns_config_db_has_key( &db, "window", "maximized" ) )
         ____main_window_maximized = ns_config_db_get_boolean( &db, "window", "maximized" );

		/* Check for invalid values. */

		if( ____main_window_left > ____main_window_right )
			NS_SWAP( nsint, ____main_window_left, ____main_window_right );

		if( ____main_window_top > ____main_window_bottom )
			NS_SWAP( nsint, ____main_window_top, ____main_window_bottom );

		GetWindowRect( GetDesktopWindow(), &screen );

		/* Make sure we dont go below the minimum or maximum dimensions for the main window. */

		window_width  = ____main_window_right - ____main_window_left;
		screen_width  = screen.right - screen.left;
		window_height = ____main_window_bottom - ____main_window_top;
		screen_height = screen.bottom - screen.top;

		if( window_width < _mMAIN_WINDOW_MINIMUM_WIDTH )
			____main_window_right = ____main_window_left + _mMAIN_WINDOW_MINIMUM_WIDTH;
		else if( window_width > screen_width )
			____main_window_right = ____main_window_left + screen_width;

		if( window_height < _mMAIN_WINDOW_MINIMUM_HEIGHT )
			____main_window_bottom = ____main_window_top + _mMAIN_WINDOW_MINIMUM_HEIGHT;
		else if( window_height > screen_height )
			____main_window_bottom = ____main_window_top + screen_height;

		/* Shift the main window to the boundaries of the screen. */

		if( ____main_window_left < screen.left )
			{
			delta                  = screen.left - ____main_window_left;
			____main_window_left  += delta;
			____main_window_right += delta;
			}

		if( ____main_window_right > screen.right )
			{
			delta                  = ____main_window_right - screen.right;
			____main_window_left  -= delta;
			____main_window_right -= delta;
			}

		if( ____main_window_top < screen.top )
			{
			delta                   = screen.top - ____main_window_top;
			____main_window_top    += delta;
			____main_window_bottom += delta;
			}

		if( ____main_window_bottom > screen.bottom )
			{
			delta                   = ____main_window_bottom - screen.bottom;
			____main_window_top    -= delta;
			____main_window_bottom -= delta;
			}
      }

   if( ns_config_db_has_group( &db, "render" ) )
		{
      if( ns_config_db_has_key( &db, "render", "tiles" ) )
         ____workspace_show_tiles = ns_config_db_get_boolean( &db, "render", "tiles" );

      if( ns_config_db_has_key( &db, "render", "spine_voxels" ) )
         ____visualize_spine_voxels = ns_config_db_get_boolean( &db, "render", "spine_voxels" );
		}


   //ns_ascii_strncpy(
     // license,
      //ns_config_db_get_string( &db, "license", "value" ),
      //NS_LICENSE_NUM_DIGITS + 1
      //);


   if( ns_config_db_has_group( &db, "console" ) )
      if( ns_config_db_has_key( &db, "console", "show" ) )
         ns_print_set_enabled( ns_config_db_get_boolean( &db, "console", "show" ) );

   ____enable_log            = NS_TRUE;
   ____flush_log_after_entry = NS_TRUE;

   if( ns_config_db_has_group( &db, "log" ) )
      {
      if( ns_config_db_has_key( &db, "log", "enable" ) )
         ____enable_log = ns_config_db_get_boolean( &db, "log", "enable" );

      if( ns_config_db_has_key( &db, "log", "flush" ) )
         ____flush_log_after_entry = ns_config_db_get_boolean( &db, "log", "flush" );
      }

	____initial_num_cpu = 1;

	if( ns_config_db_has_group( &db, "cpu" ) )
		if( ns_config_db_has_key( &db, "cpu", "count" ) )
			____initial_num_cpu = ( nssize )ns_config_db_get_int( &db, "cpu", "count" );


	if( ns_config_db_has_group( &db, "widget" ) )
		if( ns_config_db_has_key( &db, "widget", "memory_status" ) )
			____show_memory_status_widget = ns_config_db_get_boolean( &db, "widget", "memory_status" );


	if( ns_config_db_has_group( &db, "volume-labeling" ) )
		if( ns_config_db_has_key( &db, "volume-labeling", "allow" ) )
			____allow_volume_labeling = ns_config_db_get_boolean( &db, "volume-labeling", "allow" );


	/*error*/ns_settings_read_user_defaults( &db, "settings" );


   ns_config_db_destruct( &db );

   return NS_TRUE;
   }


/*
nsboolean _write_license( const nschar *license )
   {
   NsConfigDb  db;
   NsError     error;


   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return NS_FALSE;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   if( ! ns_config_db_has_key( &db, "license", "value" ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   ns_config_db_set_string( &db, "license", "value", license );


   if( NS_FAILURE( ns_config_db_write( &db, ____config_file ), error ) )
      {
      ns_config_db_destruct( &db );
      return NS_FALSE;
      }

   ns_config_db_destruct( &db );

   return NS_TRUE;
   }
*/




#include <std/nsio.h>

#include <image/tiff.h>
#include <image/raw.h>
#include <image/jpeg.h>

NS_COMPILE_TIME_BOOLEAN( sizeof( FILE* ), ==, sizeof( nspointer ) );
NS_COMPILE_TIME_BOOLEAN( sizeof( size_t ), ==, sizeof( nssize ) );

/* HACKS???
   4133: 'FILE*' return value is not equivalent to 'nspointer'???
   4028: 'size_t' parameter is not equivalent to 'nssize'??? */
#pragma warning( disable : 4133 )
#pragma warning( disable : 4028 )


/* TEMP */
void* ( *jpeg_fopen )( const char*, const char*, void* ) = NULL;
int ( *jpeg_fclose )( void*, void* ) = NULL;
int ( *jpeg_remove )( const char*, void* ) = NULL;
int ( *jpeg_setjmp )( void* ) = NULL;
void ( *jpeg_longjmp )( int, void* ) = NULL;

NS_PRIVATE void ____set_io_functions( void )
   {
   tiff_fopen  = ns_io_fopen;
   tiff_fclose = ns_io_fclose;
   tiff_fread  = ns_io_fread;
   tiff_fwrite = ns_io_fwrite;
   tiff_remove = ns_io_remove;
   tiff_fseek  = ns_io_fseek;
   tiff_ftell  = ns_io_ftell;
   tiff_malloc = ns_io_malloc;
   tiff_calloc = ns_io_calloc;
   tiff_free   = ns_io_free;
   tiff_print  = ns_io_print;

   TIFF_SEEK_SET = NS_FILE_SEEK_BEGINNING;
   TIFF_SEEK_CUR = NS_FILE_SEEK_CURRENT;
   TIFF_SEEK_END = NS_FILE_SEEK_END;

   raw_fopen  = ns_io_fopen;
   raw_fclose = ns_io_fclose;
   raw_fread  = ns_io_fread;
   raw_fwrite = ns_io_fwrite;
   raw_remove = ns_io_remove;
   raw_malloc = ns_io_malloc;
   raw_free   = ns_io_free;

	jpeg_fopen   = ns_io_fopen;
	jpeg_fclose  = ns_io_fclose;
	jpeg_remove  = ns_io_remove;
	jpeg_setjmp  = ns_io_setjmp;
	jpeg_longjmp = ns_io_longjmp;
   }

#pragma warning( default : 4028 )
#pragma warning( default : 4133 )



#include "banner.h"


//nschar ____license[ NS_LICENSE_NUM_DIGITS + 32 ]; /* Plenty of room! */



//NsVector3f ____vectors[64];
//NsRayburstPlane ____planes[64];
//extern void ns_rayburst_generate_2d( NsVector3f*, NsRayburstPlane*, nssize, nsboolean );
/*
extern void _ns_rayburst_do_generate_3d
   (
   NsVector3f         *vectors,
   NsRayburstPlane    *planes,
   NsIndexTriangleus  *triangles,
   nssize              num_samples,
   nssize              num_triangles,
   nssize              recursive_depth,
   nsint               init_type
   );
extern void _ns_rayburst_do_generate_3d_print
   (
   NsVector3f         *vectors,
   NsRayburstPlane    *planes,
   NsIndexTriangleus  *triangles,
   nssize              num_samples,
   nssize              num_triangles
   );
NsVector3f ____vectors[42];
NsRayburstPlane ____planes[42];
NsIndexTriangleus ____triangles[80];
*/


void _create_settings_file( void )
   {
   NsFile   file;
   NsError  error;


   ns_file_construct( &file );

   if( NS_FAILURE( ns_file_open( &file, ____settings_file, NS_FILE_MODE_READ ), error ) )
      {
      if( NS_ERROR_NOENT == ns_error_code( error ) )
         {
         ns_println( "Creating .settings file..." );

         if( NS_FAILURE( ns_file_open( &file, ____settings_file, NS_FILE_MODE_WRITE ), error ) )
            ns_println( "An error occurred while creating .settings file!" );
         else
            ns_println( "Created .settings file." );
         }
      else
         ns_println( "An error occurred while reading .settings file!" );
      }
   else
      ns_println( "The .settings file already exists." );

   ns_file_destruct( &file );
   }




#define _NS_MAX_ARGV   64
nschar ____argv_buffer[ 1024 ];


/* IMPORTANT: Dont use the strtok() function for spaces since a path name
	might have spaces in it. Need to parse out arguments that are in quotations
	such as a path name for this function to be correct. */
nsboolean _parse_command_line( const nschar *src, nsint *argc, nschar *argv[] )
	{
	nschar  *token;
	nsint    count;


	ns_ascii_strncpy( ____argv_buffer, src, NS_ARRAY_LENGTH( ____argv_buffer ) );

	count = 0;
	token = ns_ascii_strtok( ____argv_buffer, " \t\n" );

	while( NULL != token )
		{
		if( count < _NS_MAX_ARGV )
			{
			argv[ count ] = token;
			++count;
			}
		else
			break;

		token = ns_ascii_strtok( NULL, " \t\n" );
		}

	*argc = count;

	return 1 < count;
	}



#include "nserrorreport.inl"

LONG WINAPI _unhandled_exception_filter( struct _EXCEPTION_POINTERS *ep )
   {
   ns_error_report(
      ( nsint )ep->ExceptionRecord->ExceptionCode,
      ( nsboolean )( ! ep->ExceptionRecord->ExceptionFlags ),
      ( nspointer )ep->ExceptionRecord->ExceptionAddress
      );

   return EXCEPTION_EXECUTE_HANDLER;
   }


//#include "cmdline.inl"


const NsVoxelInfo* _ns_spines_retrieve_voxel_info( void )
	{  return workspace_get_voxel_info( s_MainWindow.activeWorkspace );  }


/*
void _do_3d_rayburst_generate_testing( void )
	{
	NsVector3f         *vectors;
	NsRayburstPlane    *planes;
	NsIndexTriangleus  *triangles;
	nssize              num_samples, num_triangles;
	nssize              recursive_depth;
	nsint               init_type;
	nstimer             start, stop;


	nsboolean rotate[ NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH + 1 ] =
		{ NS_FALSE, NS_FALSE, NS_FALSE, NS_FALSE, NS_TRUE, NS_TRUE };

	init_type = NS_RAYBURST_GENERATE_3D_ICOSAHEDRON;

	for( recursive_depth = 0; recursive_depth <= NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH; ++recursive_depth )
		{
		num_samples   = ns_rayburst_generate_3d_num_samples( recursive_depth, init_type );
		num_triangles = ns_rayburst_generate_3d_num_triangles( recursive_depth, init_type );

		vectors   = ns_new_array( NsVector3f, num_samples );
		planes    = ns_new_array( NsRayburstPlane, num_samples );
		triangles = ns_new_array( NsIndexTriangleus, num_triangles );

		start = ns_timer();

		ns_rayburst_generate_3d(
			vectors, planes, triangles,
			num_samples, num_triangles,
			recursive_depth, init_type,
			rotate[ recursive_depth ]
			);

		stop = ns_timer();

		ns_println(
			"nsrayburst-gen-3d-" NS_FMT_ULONG "-" NS_FMT_ULONG ".txt: " NS_FMT_DOUBLE "s",
			num_samples,
			num_triangles,
			ns_difftimer( stop, start )
			);

		ns_rayburst_generate_3d_output(
			vectors, planes, triangles,
			num_samples, num_triangles
			);

		ns_free( vectors );
		ns_free( planes );
		ns_free( triangles );
		}
	}
*/


/*
void _do_2d_rayburst_generate_testing( void )
	{
	NsVector3f         *vectors;
	NsRayburstPlane    *planes;
	nssize              num_samples;


	num_samples = 64;

	vectors = ns_new_array( NsVector3f, num_samples );
	planes  = ns_new_array( NsRayburstPlane, num_samples );

	ns_rayburst_generate_2d( vectors, planes, num_samples, NS_FALSE );

	ns_rayburst_generate_2d_output( vectors, planes, num_samples );

	ns_free( vectors );
	ns_free( planes );
	}
*/


void ns_run( HINSTANCE hInstance, const nschar *cmd_line )
   {
   INITCOMMONCONTROLSEX iccx;
   NsError error;
   //nsuint original_floating_point_word;
	nschar *ptr;
	nsboolean is_gui;
	nsint argc;
	nschar *argv[ _NS_MAX_ARGV ];


   SetUnhandledExceptionFilter( _unhandled_exception_filter );

   //__try{


   ____set_io_functions();

   /*error*/ns_init( 0, NULL );

   ns_print_set_is_gui( NS_TRUE );


	ns_print_record_init();


	ns_global_randi_init( ( nsuint32 )_ns_time() );
	ns_global_randf_init( ( nssize )( 4 + _ns_time() % 64 ) );


//MessageBox( NULL, GetCommandLine(), NULL, MB_OK );


	is_gui = ! _parse_command_line( cmd_line, &argc, argv );


   iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
   iccx.dwICC  = ICC_WIN95_CLASSES | ICC_UPDOWN_CLASS |
         ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES | ICC_TAB_CLASSES;
   InitCommonControlsEx( &iccx );

   g_Instance = hInstance;


	//ns_getcwd( _startup_directory, NS_PATH_SIZE );
	GetModuleFileName( NULL, _startup_directory, sizeof( _startup_directory ) - 1 );

	ns_ascii_strcpy( ____executable_path, _startup_directory );

	if( NULL != ( ptr = ns_ascii_strrchr( _startup_directory, '\\' ) ) )
		*ptr = NS_ASCII_NULL;



//MessageBox( NULL, _startup_directory, NULL, MB_OK );


   ns_ascii_strcpy( ____config_file, _startup_directory );
   ns_ascii_strcat( ____config_file, "\\config" );

   ns_ascii_strcpy( ____settings_file, _startup_directory );
   ns_ascii_strcat( ____settings_file, "\\settings" );

   ns_ascii_strcpy( ____log_file, _startup_directory );
   ns_ascii_strcat( ____log_file, "\\log" );

   ns_ascii_strcpy( ____classifiers_dir, _startup_directory );
   ns_ascii_strcat( ____classifiers_dir, "\\classifiers" );



//log = fopen( ____log_file, "w" );
//ns_print_set_io( log );


   if( ! _read_config( /*____license*/ ) )
      {
      MessageBox(
			NULL,
			"Unable to find program initialization data. Please make sure that you\n"
			"are not running the executable from within the downloaded ZIP file, and\n"
			"that all files were successfully extracted from the original ZIP file.",
			"NeuronStudio",
			MB_OK | MB_ICONERROR
			);

      goto ___MAIN_EXIT;
      }


//ns_println( "old license = " NS_FMT_STRING, license );

   /*if( ! UpdateLock( ____license ) )
      {
      MessageBox( NULL,
         "The license on this copy of NeuronStudio has expired or is invalid.\n"
         "Please visit http://www.mssm.edu/cnic for more information.",
         "NeuronStudio",
         MB_OK | MB_ICONERROR
         );

      goto ___MAIN_EXIT;
      }*/

//ns_println( "new license = " NS_FMT_STRING, license );

   //if( ! _write_license( ____license ) )
     // {
      //MessageBox( NULL, "I/O error on write license.", "NeuronStudio", MB_OK | MB_ICONERROR );
      //goto ___MAIN_EXIT;
      //}



   //IntroBanner( hInstance, "NSLOGO_BIG", 1, 1, NULL, 10 );


   if( ns_print_get_enabled() )
      {
      HANDLE  console;
      COORD   dimension;

      AllocConsole();
      SetConsoleTitle( "NeuronStudio" );

      console = GetStdHandle( STD_OUTPUT_HANDLE );

      dimension.X = 80;
      dimension.Y = 2048;

      SetConsoleScreenBufferSize( console, dimension );
      }
	//else
	//	ns_print_record_enable( NS_TRUE );
 
   
   if( NS_FAILURE( ns_rayburst_init( _startup_directory ), error ) )
      {
      MessageBox( NULL, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );
      goto ___MAIN_EXIT;
      }

   //IntroBanner( g_Instance, "NSLOGO_BIG", 1, 1, "Initializing...", 3 );


   //ns_file_construct( &file );
   //ns_file_open( &file, "D:\\usr\\douglas\\projects\\ns\\ns.log", NS_FILE_MODE_WRITE );

   //ns_log_set_file( &file );

   s_MainWindow.hWnd = NULL;

	if( is_gui )
		ns_assert_set( _gui_assert );


   ns_log_init( ____enable_log, ____flush_log_after_entry, 204800 ); /* i.e. 200 KB */
   ns_log_open( ____log_file );


   _create_settings_file();


	CirclesInit();


	//ns_spines_create_random_colors( 1024 );


   /*TEMP*/
   //ns_mutex_construct( &____rayburst_mutex );
   //ns_mutex_set_name( &____rayburst_mutex, "____rayburst_mutex" );
   //ns_mutex_create( &____rayburst_mutex );



//ns_rayburst_generate_2d( ____vectors, ____planes, 64, NS_TRUE );
/*
_ns_rayburst_do_generate_3d( ____vectors, ____planes, ____triangles,
   NS_ARRAY_LENGTH( ____vectors ), NS_ARRAY_LENGTH( ____triangles ),
   1, 0 );
_ns_rayburst_do_generate_3d_print( ____vectors, ____planes, ____triangles,
   NS_ARRAY_LENGTH( ____vectors ), NS_ARRAY_LENGTH( ____triangles ) );
*/

	//_do_3d_rayburst_generate_testing();
	//_do_2d_rayburst_generate_testing();


   //ns_log( NS_LOG_LEVEL_DEBUG, "test entry 1" );
   //ns_log( NS_LOG_LEVEL_DEBUG, "test entry 2" );


//ns_println( "____config_file = %s", ____config_file );   

//   original_floating_point_word = _control87( _PC_24, MCW_PC );


	if( is_gui )
		{
		if( eNO_ERROR != _CreateApplication( 100, 100, 600, 600 ) )
			MessageBox( NULL, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );

		if( eNO_ERROR != g_Error )
			MessageBox( NULL, g_ErrorStrings[ g_Error ], "NeuronStudio", MB_OK | MB_ICONERROR );
		}
	//else
	//	_command_line_application( argc, argv );


   //_control87( _CW_DEFAULT, 0xfffff );


   //ns_file_destruct( &file );

   //____spines_destruct();
   //free( ____spines_colors );

   //____ns_model_graft_destruct();



   /*TEMP*/
   //ns_mutex_destruct( &____rayburst_mutex );


	CirclesFinalize();


   ns_rayburst_finalize();


   ns_voxel_table_end_recycling();


	ns_spines_classifier_delete();


   ns_log_close();


   if( ns_print_get_enabled() )
      FreeConsole();


   ___MAIN_EXIT:

   //ns_free( ____naas_points );

//if( NULL != log )
//   fclose( log );


	ns_print_record_enable( NS_FALSE );
	//ns_print_record_dump( _startup_directory );
	ns_print_record_finalize();


   ns_finalize();

   //}
   //__except( ns_error_report( GetExceptionCode() ) )
   //{
   //exit(0);
   //}
   }
