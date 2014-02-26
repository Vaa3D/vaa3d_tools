#include "app_data.h"
#include <std/nsconfigdb.h>
#include <std/nsutil.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-table.h>
#include <image/nsvoxel-buffer.h>
#include <std/nsthread.h>
#include <image/nsio-writetiff.h>
#include <image/nsio-writeraw.h>


//nsdouble __g_min_intensity;
//nsdouble __g_max_intensity;
//nsdouble __g_max_efficiency;


NsPixelRegion* _ns_pixel_region_from_roi( NsPixelRegion *R, const NsCubei *roi )
	{
	R->x      = ( nssize )roi->C1.x;
	R->y      = ( nssize )roi->C1.y;
	R->z      = ( nssize )roi->C1.z;
	R->width  = ( nssize )ns_cubei_width( roi );
	R->height = ( nssize )ns_cubei_height( roi );
	R->length = ( nssize )ns_cubei_length( roi );

	return R;
	}


#define _IMAGE_ROW_ALIGN  sizeof( LONG )


extern void _adjust_slice_viewer_due_to_roi_update( const NsCubei *roi );

//nsboolean ____workspace_has_tiles = NS_FALSE;
//nssize ____workspace_async_proj_iterations = 0;


//#define _TILES_LOCKABLE  NS_FALSE/*NS_TRUE*/

#define _PRINT_LAST_ERROR()\
	ns_println( "GetLastError() = " NS_FMT_LONG, ( nslong )GetLastError() )

/*
nspointer ____volume_pixels = NULL;
nssize ____volume_width  = 0;
nssize ____volume_height = 0;
nssize ____volume_length = 0;
nssize ____volume_bpp    = 0;
NsImage ____image, ____proj_xy, ____proj_zy, ____proj_xz;
*/


//extern nsboolean ____ns_value_print_invoke;



//#define _MAX_NUM_TILES  4000.0
//#define _MAX_TILE_SIZE  1024
//#define _MIN_TILE_SIZE   256
//NS_PRIVATE nssize ____tile_size = _MIN_TILE_SIZE;


//nsdouble _get_current_num_tiles( nsdouble width, nsdouble height, nsdouble length )
//	{
//	nsdouble num_tiles = 0.0;

//	num_tiles += ns_ceil( width  / ____tile_size ) * ns_ceil( height / ____tile_size ); /* XY Projection */
//	num_tiles += ns_ceil( width  / ____tile_size ) * ns_ceil( height / ____tile_size ); /* XY Slice */
//	num_tiles += ns_ceil( length / ____tile_size ) * ns_ceil( height / ____tile_size ); /* ZY Projection */
//	num_tiles += ns_ceil( width  / ____tile_size ) * ns_ceil( length / ____tile_size ); /* XZ Projection */

//	return num_tiles;
//	}


void _set_optimal_tile_size( nsdouble width, nsdouble height, nsdouble length )
	{
	//nsdouble num_tiles;

	//____tile_size = _MIN_TILE_SIZE;

	//while( ____tile_size < _MAX_TILE_SIZE )
	//	{
	//	num_tiles = _get_current_num_tiles( width, height, length );
///*TEMP*/ns_println( "Total # of Tiles = " NS_FMT_DOUBLE, num_tiles );

//		if( num_tiles > _MAX_NUM_TILES )
//			____tile_size *= 2;
//		else
//			break;
//		}

//	if( _get_current_num_tiles( width, height, length ) > _MAX_NUM_TILES )
//		ns_println( "WARNING: The number of tiles is still too high!" );

///*TEMP*/ns_println( "Optimal Tile Size = " NS_FMT_ULONG, ____tile_size );
	}


extern nschar _startup_directory[];
extern nschar ____config_file[];
extern nschar ____settings_file[];

extern nschar *____volume_file_filter;


extern nsboolean ____draw_while_grafting;

//extern nsboolean ____use_2d_neurite_sampling;

extern nsboolean ____use_2d_spine_bounding_box;

extern nsboolean ____gamma_correction;

extern nsint ____neurites_interp_type;
extern nsint ____spines_interp_type;

extern nsboolean ____projected_2d_grafting;
extern nsboolean ____run_model_filtering;

extern nsboolean ____visualize_spine_voxels;

extern nsboolean ____measurement_do_3d_rayburst;


extern nsboolean ____xy_slice_enabled;
extern nssize    ____xy_slice_index;


extern nsboolean ____allow_volume_labeling;


nsboolean ____spine_analysis_running = NS_FALSE;


nsboolean ____projections_running   = NS_FALSE;
nsboolean ____projections_terminate = NS_FALSE;
NsThread  ____projections_thread;



/*
const nschar* s_DisplayLabels[] = 
   {
   "Forward",
   "Side",
   "Top",
   "Montage"
   };


const nschar* GetDisplayLabel( const nsint display )
   {
   ns_assert( display < mNUM_DISPLAYS );  
   return s_DisplayLabels[ display ];
   }
*/


/*
typedef struct _Volume
   {
   void      *pixels;
   nssize   width;
   nssize   height;
   nssize   length;
   nssize   bitsPerPixel;
   }
   Volume;


void _ConstructVolume( Volume* volume )
   {
   volume->pixels       = NULL;
   volume->width        = 0;
   volume->height       = 0;
   volume->length       = 0;
   volume->bitsPerPixel = 0;
   }


void _ClearVolume( Volume* volume )
   {
   //if( NULL != volume->pixels )
      ns_free( volume->pixels );

   volume->pixels       = NULL;
   volume->width        = 0;
   volume->height       = 0;
   volume->length       = 0;
   volume->bitsPerPixel = 0;
   }


void _DestructVolume( Volume* volume )
   {  _ClearVolume( volume );  }
*/


NsVector4ub _dc_background;
NsVector4ub _dc_line_vertex;
NsVector4ub _dc_junction_vertex;
NsVector4ub _dc_origin_vertex;
NsVector4ub _dc_external_vertex;
NsVector4ub _dc_vertex;
NsVector4ub _dc_edge;
NsVector4ub _dc_splat;
NsVector4ub _dc_spine_other;
NsVector4ub _dc_spine_thin;
NsVector4ub _dc_spine_mushroom;
NsVector4ub _dc_spine_stubby;
NsVector4ub _dc_spine;
NsVector4ub _dc_function_soma;
NsVector4ub _dc_function_basal_dendrite;
NsVector4ub _dc_function_apical_dendrite;
NsVector4ub _dc_function_axon;
NsVector4ub COLOR_DarkBlue;
NsVector4ub COLOR_Blue;
NsVector4ub COLOR_Black;
NsVector4ub COLOR_Red;
NsVector4ub COLOR_Orange;
NsVector4ub COLOR_DarkRed;
NsVector4ub COLOR_Purple;
NsVector4ub COLOR_Indigo;
NsVector4ub COLOR_White;
NsVector4ub COLOR_Yellow;
NsVector4ub COLOR_Pink;
NsVector4ub COLOR_Violet;
NsVector4ub COLOR_Green;



typedef struct _WorkspaceResource
   {
	HWND                      hWnd;
	NsImage                   volume;
	NsImage                   proj_xy;
	NsImage                   proj_zy;
	NsImage                   proj_xz;
	NsTileMgr                 tile_mgr_xy;
	NsTileMgr                 tile_mgr_zy;
	NsTileMgr                 tile_mgr_xz;
	NsTileMgr                 tile_mgr_slice;
	//NsMutex                 tile_mutex;
	NsCubei                   update_roi;
	NsCubei                   saved_roi;
	//nsboolean               has_tiles;
	//Image                   image_xy;
	//Image                   image_zy;
	//Image                   image_xz;
	//MemoryGraphics          gc_xy;
	//MemoryGraphics          gc_zy;
	//MemoryGraphics          gc_xz;
	nsuint                    displayFlags;
	Palette                   palette;
	Palette                   inverted_palette;
	//nsint                   hasTransparentPalette;
	eWORKSPACE_PALETTE_TYPE   paletteType;
	NsVector4ub               paletteColors[ mMAX_WORKSPACE_FILTER_COLORS ];
	nsuint                    paletteSteps[ mMAX_WORKSPACE_FILTER_COLORS - 1 ];
	//nsfloat                 brightness;
	//nsfloat                 contrast;
	Vector2i                  corner[ mNUM_DISPLAY_WINDOWS ][ mNUM_DISPLAYS ];
	nsfloat                   zoom[ mNUM_DISPLAY_WINDOWS ][ mNUM_DISPLAYS ];
	nschar                    path[ _MAX_PATH ];
	//nsulong                 setting_flags;
	//nsint                   doRawTree;
	//nsint                   doFilteredTree;
	//nsint                   doBranchLabeling;
	//NsModel                *temp_raw_tree;
	//NsModel                *temp_filtered_tree;
	NsModel                  *rawTree;
	NsModel                  *filteredTree;
	NsModel                   models[ 2/*4*/ ];
	//NsVoxelTable            voxel_table;
	NsSettings                settings;
	//nsuint                  volume_width;
	//nsuint                  volume_height;
	//nsuint                  volume_length;
	NsProcDb                  ppdb;
	NsProcDb                  iodb;
	nsdouble                  average_intensity;
	nsulong                   max_intensity;
	nssize                    num_seen_voxels;
	nsfloat                   grafting_first_threshold;
	nsfloat                   grafting_first_contrast;
	nsboolean                 grafting_use_2d_sampling;
	nsfloat                   grafting_aabbox_scalar;
	nsint                     grafting_min_window;
	nsdouble                  signal_to_noise_ratio;
	NsSampler                 sampler;
	NsList                    records;
	nssize                    iterations;
	nsushort                  roi_control_selections;
	//NsImage                 thumbnail_xy;
	//NsImage                 thumbnail_zy;
	//NsImage                 thumbnail_xz;
	//NsImage                 tile;
	NsImage                   slice_xy;
	//NsImage                 flipped_xz;
	//nssize                  bits_per_pixel;
	NsColorDb                 pen_db;
	NsColorDb                 brush_db;
	NsList                    image_extra_info;
	NsShollAnalysis           sholl_analysis;
	NsVector3d                subsample_scale;
	nsdouble                  brightness;
	nsdouble                  contrast;
	nsfloat                   channel_min;
	nsfloat                   channel_max;
	nsboolean                 is_lsm;
	TiffLsmInfo               lsm_info;
	NsVector4ub               colors[ _WORKSPACE_NUM_COLORS ];
   }
   WorkspaceResource;

typedef WorkspaceResource NsWorkspace;


#define _mMAX_WORKSPACES  64

static NsWorkspace*  s_Workspaces[ _mMAX_WORKSPACES ];
static nsuint            s_NumWorkspaces = 0;



void _LinearInterpolateWorkspacePalette( NsWorkspace* wr, nschar *file_name )
   {
   nsfloat redSlope, redIntercept;
   nsfloat greenSlope, greenIntercept;
   nsfloat blueSlope, blueIntercept;
   nsuint index;
   nsuint color;
   nsuint i;
   nsuchar red, green,blue;


   index = 0;

   for( color = 0; color < mMAX_WORKSPACE_FILTER_COLORS - 1; ++color )
      {
      redSlope = ( ( nsfloat )NS_COLOR_GET_RED_U8( wr->paletteColors[ color + 1 ] ) -
                   ( nsfloat )NS_COLOR_GET_RED_U8( wr->paletteColors[ color ] ) )/
                 ( ( nsfloat )( wr->paletteSteps[ color ] - 1 ) );
      redIntercept = ( nsfloat )NS_COLOR_GET_RED_U8( wr->paletteColors[ color ] );

      greenSlope = ( ( nsfloat )NS_COLOR_GET_GREEN_U8( wr->paletteColors[ color + 1 ] ) -
                   ( nsfloat )NS_COLOR_GET_GREEN_U8( wr->paletteColors[ color ] ) )/
                 ( ( nsfloat )( wr->paletteSteps[ color ] - 1 ) );
      greenIntercept = ( nsfloat )NS_COLOR_GET_GREEN_U8( wr->paletteColors[ color ] );

      blueSlope = ( ( nsfloat )NS_COLOR_GET_BLUE_U8( wr->paletteColors[ color + 1 ] ) -
                   ( nsfloat )NS_COLOR_GET_BLUE_U8( wr->paletteColors[ color ] ) )/
                 ( ( nsfloat )( wr->paletteSteps[ color ] - 1 ) );
      blueIntercept = ( nsfloat )NS_COLOR_GET_BLUE_U8( wr->paletteColors[ color ] );


      for( i = 0; i < wr->paletteSteps[ color ]; ++i )
         {
         red   = ( nsuchar )( redSlope*i + redIntercept );
         green   = ( nsuchar )( greenSlope*i + greenIntercept );
         blue   = ( nsuchar )( blueSlope*i + blueIntercept );

         NS_COLOR_SET_RED_U8( wr->palette.colors[ index + i ], blue );
         NS_COLOR_SET_GREEN_U8( wr->palette.colors[ index + i ], green );
         NS_COLOR_SET_BLUE_U8( wr->palette.colors[ index + i ], red );
         //wr->palette.colors[ index + i ] = RGB( red, green, blue );
         }

      index += wr->paletteSteps[ color ];

      }/* for( colors ) */
   }



void _CreateWorkspacePaletteSolarFlare( NsWorkspace* wr )
   {
   wr->paletteColors[ 0 ] = COLOR_Black;
   wr->paletteColors[ 1 ] = COLOR_Black;
   wr->paletteColors[ 2 ] = COLOR_Black;
   wr->paletteColors[ 3 ] = COLOR_Black;
   wr->paletteColors[ 4 ] = COLOR_Black;
   wr->paletteColors[ 5 ] = COLOR_Red;
   wr->paletteColors[ 6 ] = COLOR_Orange;
   wr->paletteColors[ 7 ] = COLOR_Yellow;
   wr->paletteColors[ 8 ] = COLOR_White;

   wr->paletteSteps[ 0 ] = 2;
   wr->paletteSteps[ 1 ] = 2;
   wr->paletteSteps[ 2 ] = 2;
   wr->paletteSteps[ 3 ] = 2;
   wr->paletteSteps[ 4 ] = 20;
   wr->paletteSteps[ 5 ] = 102;
   wr->paletteSteps[ 6 ] = 102;
   wr->paletteSteps[ 7 ] = 24;

   _LinearInterpolateWorkspacePalette( wr, /*"D://usr//douglas//projects//ns//pal_solarflare"*/0 );
   }


void _CreateWorkspacePaletteRainbow( NsWorkspace* wr )
   {
   wr->paletteColors[ 0 ] = COLOR_Black;
   wr->paletteColors[ 1 ] = COLOR_Violet;
   wr->paletteColors[ 2 ] = COLOR_Indigo;
   wr->paletteColors[ 3 ] = COLOR_Blue;
   wr->paletteColors[ 4 ] = COLOR_Green;
   wr->paletteColors[ 5 ] = COLOR_Yellow;
   wr->paletteColors[ 6 ] = COLOR_Orange;
   wr->paletteColors[ 7 ] = COLOR_Red;
   wr->paletteColors[ 8 ] = COLOR_Red;

   wr->paletteSteps[ 0 ] = 2;
   wr->paletteSteps[ 1 ] = 16;
   wr->paletteSteps[ 2 ] = 16;
   wr->paletteSteps[ 3 ] = 16;
   wr->paletteSteps[ 4 ] = 16;
   wr->paletteSteps[ 5 ] = 16;
   wr->paletteSteps[ 6 ] = 16;
   wr->paletteSteps[ 7 ] = 158;

   _LinearInterpolateWorkspacePalette( wr, 0 );
   }


void _CreateWorkspacePaletteIntensity( NsWorkspace* wr )
   {
   wr->paletteColors[ 0 ] = COLOR_Black;
   wr->paletteColors[ 1 ] = COLOR_Black;
   wr->paletteColors[ 2 ] =COLOR_DarkBlue;
   wr->paletteColors[ 3 ] = COLOR_Purple;
   wr->paletteColors[ 4 ] = COLOR_DarkRed;
   wr->paletteColors[ 5 ] = COLOR_Red;
   wr->paletteColors[ 6 ] = COLOR_Pink;
   wr->paletteColors[ 7 ] = COLOR_White;
   wr->paletteColors[ 8 ] = COLOR_White;

   wr->paletteSteps[ 0 ] = 2;
   wr->paletteSteps[ 1 ] = 16;
   wr->paletteSteps[ 2 ] = 32;
   wr->paletteSteps[ 3 ] = 32;
   wr->paletteSteps[ 4 ] = 16;
   wr->paletteSteps[ 5 ] = 32;
   wr->paletteSteps[ 6 ] = 32;
   wr->paletteSteps[ 7 ] = 94;

   _LinearInterpolateWorkspacePalette( wr, /*"D://usr//douglas//projects//ns//pal_intensity"*/0 );
   }


void _CreateWorkspacePaletteStandard( NsWorkspace* wr )
   {
   nsuint      i;
   nsuchar grays[ mPALETTE_MAX_COLORS ];


   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
      grays[i] = ( nsuchar )i;


   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
      {
      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], grays[i] );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], grays[i] );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], grays[i] );

/*
      wr->palette.colors[ i ] = ( ((nsuint)grays[i]) << 16 ) | 
                                         ( ((nsuint)grays[i]) << 8 )  | 
                                           ((nsuint)grays[i]);*/
      }
   }


/* NOTE: Windows expects BGR. */
void _ns_palette_red_create( NsWorkspace *wr )
	{
	nsuint i;

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
		{
      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], 0 );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], 0 );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], ( nsuint8 )i );
		}
	}


void _ns_palette_green_create( NsWorkspace *wr )
	{
	nsuint i;

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
		{
      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], 0 );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], ( nsuint8 )i );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], 0 );
		}
	}


void _ns_palette_blue_create( NsWorkspace *wr )
	{
	nsuint i;

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
		{
      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], ( nsuint8 )i );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], 0 );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], 0 );
		}
	}


void _ns_palette_yellow_create( NsWorkspace *wr )
	{
	nsuint i;

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
		{
      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], 0 );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], ( nsuint8 )i );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], ( nsuint8 )i );
		}
	}


void _CreateWorkspacePaletteInverse( NsWorkspace* wr )
   {
   nsuint i;
   nsuchar red,green,blue;

   _CreateWorkspacePaletteStandard( wr );

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
      {
      red = NS_COLOR_GET_RED_U8( wr->palette.colors[i]);
      green = NS_COLOR_GET_GREEN_U8( wr->palette.colors[i]);
      blue = NS_COLOR_GET_BLUE_U8( wr->palette.colors[i]);

       red = ~red;
       green = ~green;
       blue = ~blue;


      NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], red );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], green );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], blue );
//       wr->palette.colors[i]=RGB(red,green,blue );
       }
   }



void _create_workspace_palette_gamma_correction( NsWorkspace *wr, nsdouble gamma )
	{
	nsuint    i;
	nsdouble  max;
	nsdouble  value;
	nsuint8   pixel;


	max = ( nsdouble )( mPALETTE_MAX_COLORS - 1 );

	for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
		{
		value = ns_pow( ( nsdouble )i / max, 1.0 / gamma );
		value = value * max + 0.5; /* Proper rounding! */

		pixel = ( nsuint8 )value;

		NS_COLOR_SET_RED_U8( wr->palette.colors[ i ], pixel );
		NS_COLOR_SET_GREEN_U8( wr->palette.colors[ i ], pixel );
		NS_COLOR_SET_BLUE_U8( wr->palette.colors[ i ], pixel );
		}
	}


void _CreateWorkspacePaletteMonochrome( NsWorkspace* wr )
   {
    nsuint i;


   for( i = 0; i < mPALETTE_MAX_COLORS / 4; ++i )
       wr->palette.colors[i]=COLOR_Black;

   for( i = mPALETTE_MAX_COLORS / 4; i < mPALETTE_MAX_COLORS; ++i )
       wr->palette.colors[i]=COLOR_White;
   }


void _CreateWorkspacePaletteFromTable
   ( 
   NsWorkspace*    wr,
   const nsuchar*  table,
   const nsuint    size
   )
   {
   nsuint      i            = 0;
   nsuint      paletteIndex = 0;
   nsuint      channelIndex = 0;
   nsuchar channels[3];


   for( ; i < size; ++i )
      {
      if( ( 0 != i ) && ( 0 == i % 3 ) )
         {
         ns_assert( 3 == channelIndex );
         ns_assert( paletteIndex < mPALETTE_MAX_COLORS );

      NS_COLOR_SET_RED_U8( wr->palette.colors[ paletteIndex ], channels[2] );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ paletteIndex ], channels[1] );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ paletteIndex ], channels[0] );
         //wr->palette.colors[ paletteIndex ] =
           // RGB( channels[0], channels[1], channels[2] );
        
         channelIndex = 0;
         ++paletteIndex;
         }

      if( mPALETTE_MAX_COLORS <= paletteIndex )
         break;

      channels[ channelIndex ] = table[ i ];
      ++channelIndex;
      }

   if( 3 == channelIndex && paletteIndex < mPALETTE_MAX_COLORS )
      {
      NS_COLOR_SET_RED_U8( wr->palette.colors[ paletteIndex ], channels[2] );
      NS_COLOR_SET_GREEN_U8( wr->palette.colors[ paletteIndex ], channels[1] );
      NS_COLOR_SET_BLUE_U8( wr->palette.colors[ paletteIndex ], channels[0] );

      //wr->palette.colors[ paletteIndex ] =
        // RGB( channels[0], channels[1], channels[2] );
      }
   }


static nsuchar ____ns_palette_intensity_mask[] =
   {
   #include "palettes/intensitymask.pal"
   };

void _ns_palette_intensity_mask_crate( NsWorkspace* wr )
   {  _CreateWorkspacePaletteFromTable( wr, ____ns_palette_intensity_mask, sizeof( ____ns_palette_intensity_mask ) );  }


static nsuchar s_Palette_Topographic[] = 
   {
   #include "palettes/topographic.pal"
   };

void _CreateWorkspacePaletteTopographic( NsWorkspace* wr )
   {  _CreateWorkspacePaletteFromTable( wr, s_Palette_Topographic, sizeof( s_Palette_Topographic ) );  }



void _CreateOpaqueWorkspacePalette( NsWorkspace* wr )
   {
   switch( wr->paletteType )
      {
      case eWORKSPACE_PALETTE_STANDARD:
         _CreateWorkspacePaletteStandard( wr );
         break;

      //case eWORKSPACE_PALETTE_SOLAR_FLARE:
        // _CreateWorkspacePaletteSolarFlare( wr );
         //break;

      //case eWORKSPACE_PALETTE_INTENSITY:
        // _CreateWorkspacePaletteIntensity( wr );
         //break;

      case eWORKSPACE_PALETTE_INVERSE:
         _CreateWorkspacePaletteInverse( wr );
         break;

      //case eWORKSPACE_PALETTE_MONOCHROME:
        // _CreateWorkspacePaletteMonochrome( wr );
         //break;

      //case eWORKSPACE_PALETTE_COLDFIRE:
        // _CreateWorkspacePaletteColdFire( wr );
         //break;

      case eWORKSPACE_PALETTE_TOPOGRAPHIC:
         _CreateWorkspacePaletteTopographic( wr );
         break;

		case eWORKSPACE_PALETTE_GAMMA_CORRECTION_1_7:
			_create_workspace_palette_gamma_correction( wr, 1.7 );
			break;

		case eWORKSPACE_PALETTE_GAMMA_CORRECTION_2_2:
			_create_workspace_palette_gamma_correction( wr, 2.2 );
			break;

		case NS_PALETTE_INTENSITY_MASK:
			_ns_palette_intensity_mask_crate( wr );
			break;

		case NS_PALETTE_RED:
			_ns_palette_red_create( wr );
			break;

		case NS_PALETTE_GREEN:
			_ns_palette_green_create( wr );
			break;

		case NS_PALETTE_BLUE:
			_ns_palette_blue_create( wr );
			break;

		case NS_PALETTE_YELLOW:
			_ns_palette_yellow_create( wr );
			break;

      default:
         ns_assert( 0 );
      }
   }


/*
void _CreateTransparentWorkspacePalette( NsWorkspace* wr )
   {
   nsuint i;

   for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
      wr->palette.colors[ i ] = wr->colors[ WORKSPACE_COLOR_BACKGROUND ];
   }
*/


void _invert_workspace_palette( NsWorkspace* wr )
   {
   nsuint i;

	NS_PRIVATE nsboolean ____invert_workspace_palette_init = NS_FALSE;

	if( ! ____invert_workspace_palette_init )
		{
		for( i = 0; i < mPALETTE_MAX_COLORS; ++i )
			{
			//wr->inverted_palette.colors[ i ] = ~( wr->palette.colors[ i ] );

			/*TEMP*/
			NS_COLOR_SET_RED_U8( wr->inverted_palette.colors[ i ], 0 );
			NS_COLOR_SET_GREEN_U8( wr->inverted_palette.colors[ i ], i );
			NS_COLOR_SET_BLUE_U8( wr->inverted_palette.colors[ i ], 0 );

			//wr->inverted_palette.colors[ i ] = RGB( 0, i, 0 );
			}

		____invert_workspace_palette_init = NS_TRUE;
		}
   }



NS_PRIVATE void _workspace_do_set_palette( NsWorkspace* wr )
   {
   ns_memset( &wr->palette, 0, sizeof( Palette ) );

   //wr->hasTransparentPalette =
     //( wr->displayFlags & mDISPLAY_GRAPHICS_IMAGE ) ? 0 : 1;
 
   //if( wr->hasTransparentPalette )
     // _CreateTransparentWorkspacePalette( wr );
   //else
      _CreateOpaqueWorkspacePalette( wr );

   _invert_workspace_palette( wr );
   }


void _init_workspace_colors( void )
   {
   NS_PRIVATE nsboolean _did_init_workspace_colors = NS_FALSE;

   if( ! _did_init_workspace_colors )
      {
      NS_COLOR_SET_RED_U8( _dc_background, 192 );
      NS_COLOR_SET_GREEN_U8( _dc_background, 192 );
      NS_COLOR_SET_BLUE_U8( _dc_background, 192 );

      NS_COLOR_SET_RED_U8( _dc_line_vertex, 0 );
      NS_COLOR_SET_GREEN_U8( _dc_line_vertex, 0xFF );
      NS_COLOR_SET_BLUE_U8( _dc_line_vertex, 0 );

      NS_COLOR_SET_RED_U8( _dc_junction_vertex, 0xFF );
      NS_COLOR_SET_GREEN_U8( _dc_junction_vertex, 0xFF );
      NS_COLOR_SET_BLUE_U8( _dc_junction_vertex, 0 );

      NS_COLOR_SET_RED_U8( _dc_origin_vertex, 0xFF );
      NS_COLOR_SET_GREEN_U8( _dc_origin_vertex, 0 );
      NS_COLOR_SET_BLUE_U8( _dc_origin_vertex, 0 );

      NS_COLOR_SET_RED_U8( _dc_external_vertex, 0xFF );
      NS_COLOR_SET_GREEN_U8( _dc_external_vertex, 0 );
      NS_COLOR_SET_BLUE_U8( _dc_external_vertex, 0xFF );

      NS_COLOR_SET_RED_U8( _dc_vertex, 0  );
      NS_COLOR_SET_GREEN_U8( _dc_vertex, 0 );
      NS_COLOR_SET_BLUE_U8( _dc_vertex, 0xFF );

      NS_COLOR_SET_RED_U8( _dc_edge, 0  );
      NS_COLOR_SET_GREEN_U8( _dc_edge, 0xFF );
      NS_COLOR_SET_BLUE_U8( _dc_edge, 0xFF );

      NS_COLOR_SET_RED_U8( _dc_splat, 32 );
      NS_COLOR_SET_GREEN_U8( _dc_splat, 128 );
      NS_COLOR_SET_BLUE_U8( _dc_splat, 0 );


      NS_COLOR_SET_RED_U8( _dc_spine_other,   128 );
      NS_COLOR_SET_GREEN_U8( _dc_spine_other, 128 );
      NS_COLOR_SET_BLUE_U8( _dc_spine_other,  128 );

      NS_COLOR_SET_RED_U8( _dc_spine_thin,   255 );
      NS_COLOR_SET_GREEN_U8( _dc_spine_thin, 255 );
      NS_COLOR_SET_BLUE_U8( _dc_spine_thin,  128 );

      NS_COLOR_SET_RED_U8( _dc_spine_mushroom,   199 );
      NS_COLOR_SET_GREEN_U8( _dc_spine_mushroom, 133 );
      NS_COLOR_SET_BLUE_U8( _dc_spine_mushroom,  18 );

      NS_COLOR_SET_RED_U8( _dc_spine_stubby,   201 );
      NS_COLOR_SET_GREEN_U8( _dc_spine_stubby, 42 );
      NS_COLOR_SET_BLUE_U8( _dc_spine_stubby,  110 );

      NS_COLOR_SET_RED_U8( _dc_spine,   0 );
      NS_COLOR_SET_GREEN_U8( _dc_spine, 255 );
      NS_COLOR_SET_BLUE_U8( _dc_spine,  0 );

		NS_COLOR_SET_RED_U8( _dc_function_soma, 212 );
		NS_COLOR_SET_GREEN_U8( _dc_function_soma, 175 );
		NS_COLOR_SET_BLUE_U8( _dc_function_soma, 55 );

		NS_COLOR_SET_RED_U8( _dc_function_basal_dendrite, 152 );
		NS_COLOR_SET_GREEN_U8( _dc_function_basal_dendrite, 118 );
		NS_COLOR_SET_BLUE_U8( _dc_function_basal_dendrite, 84 );

		NS_COLOR_SET_RED_U8( _dc_function_apical_dendrite, 34 );
		NS_COLOR_SET_GREEN_U8( _dc_function_apical_dendrite, 139 );
		NS_COLOR_SET_BLUE_U8( _dc_function_apical_dendrite, 34 );

		NS_COLOR_SET_RED_U8( _dc_function_axon, 112 );
		NS_COLOR_SET_GREEN_U8( _dc_function_axon, 128 );
		NS_COLOR_SET_BLUE_U8( _dc_function_axon, 144 );
		
      NS_COLOR_SET_RED_U8( COLOR_DarkBlue,  0 );
      NS_COLOR_SET_GREEN_U8( COLOR_DarkBlue, 0  );
      NS_COLOR_SET_BLUE_U8( COLOR_DarkBlue, 139 );

      NS_COLOR_SET_RED_U8( COLOR_Blue, 0 );
      NS_COLOR_SET_GREEN_U8( COLOR_Blue, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_Blue, 255 );

      NS_COLOR_SET_RED_U8( COLOR_Black, 0 );
      NS_COLOR_SET_GREEN_U8( COLOR_Black, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_Black, 0 );

      NS_COLOR_SET_RED_U8( COLOR_Red, 255 );
      NS_COLOR_SET_GREEN_U8( COLOR_Red, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_Red, 0 );

      NS_COLOR_SET_RED_U8( COLOR_Orange, 255  );
      NS_COLOR_SET_GREEN_U8( COLOR_Orange, 165 );
      NS_COLOR_SET_BLUE_U8( COLOR_Orange, 0 );

      NS_COLOR_SET_RED_U8( COLOR_DarkRed, 139 );
      NS_COLOR_SET_GREEN_U8( COLOR_DarkRed, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_DarkRed, 0 );

      NS_COLOR_SET_RED_U8( COLOR_Purple, 128 );
      NS_COLOR_SET_GREEN_U8( COLOR_Purple, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_Purple, 128 );

      NS_COLOR_SET_RED_U8( COLOR_Indigo, 75 );
      NS_COLOR_SET_GREEN_U8( COLOR_Indigo, 0 );
      NS_COLOR_SET_BLUE_U8( COLOR_Indigo, 130 );

      NS_COLOR_SET_RED_U8( COLOR_White, 255 );
      NS_COLOR_SET_GREEN_U8( COLOR_White, 255 );
      NS_COLOR_SET_BLUE_U8( COLOR_White, 255 );

      NS_COLOR_SET_RED_U8( COLOR_Yellow, 255 );
      NS_COLOR_SET_GREEN_U8( COLOR_Yellow, 255 );
      NS_COLOR_SET_BLUE_U8( COLOR_Yellow, 0 );

      NS_COLOR_SET_RED_U8( COLOR_Pink, 255 );
      NS_COLOR_SET_GREEN_U8( COLOR_Pink, 200  );
      NS_COLOR_SET_BLUE_U8( COLOR_Pink, 203 );

      NS_COLOR_SET_RED_U8( COLOR_Violet, 238 );
      NS_COLOR_SET_GREEN_U8( COLOR_Violet, 130 );
      NS_COLOR_SET_BLUE_U8( COLOR_Violet, 238 );

      NS_COLOR_SET_RED_U8( COLOR_Green, 0 );
      NS_COLOR_SET_GREEN_U8( COLOR_Green, 128 );
      NS_COLOR_SET_BLUE_U8( COLOR_Green, 0 );

      _did_init_workspace_colors = NS_TRUE;
      }
   }



NsVector4ub workspace_get_spine_color_single( nsuint handle )
   {  return workspace_get_color( handle, WORKSPACE_COLOR_SPINE );  }


NsVector4ub workspace_get_spine_color_by_type( nsuint handle, NsSpineType type )
   {
   NsVector4ub C;

   switch( type )
      {
      case NS_SPINE_OTHER:
         C = workspace_get_color( handle, WORKSPACE_COLOR_SPINE_OTHER );
         break;

      case NS_SPINE_THIN:
         C = workspace_get_color( handle, WORKSPACE_COLOR_SPINE_THIN );
         break;

      case NS_SPINE_MUSHROOM:
         C = workspace_get_color( handle, WORKSPACE_COLOR_SPINE_MUSHROOM );
         break;

      case NS_SPINE_STUBBY:
         C = workspace_get_color( handle, WORKSPACE_COLOR_SPINE_STUBBY );
         break;

      default:
         C = NS_COLOR4UB_BLACK;
         break;
      }

   return C;
   }


NsVector4ub workspace_get_vertex_color_by_type( HWorkspace handle, NsModelVertexType type )
   {
   NsVector4ub C;

   switch( type )
      {
      case NS_MODEL_VERTEX_ORIGIN:
         C = workspace_get_color( handle, WORKSPACE_COLOR_ORIGIN_VERTEX );
         break;

      case NS_MODEL_VERTEX_EXTERNAL:
         C = workspace_get_color( handle, WORKSPACE_COLOR_EXTERNAL_VERTEX );
         break;

      case NS_MODEL_VERTEX_JUNCTION:
         C = workspace_get_color( handle, WORKSPACE_COLOR_JUNCTION_VERTEX );
         break;

      case NS_MODEL_VERTEX_LINE:
         C = workspace_get_color( handle, WORKSPACE_COLOR_LINE_VERTEX );
         break;

      default:
         C = NS_COLOR4UB_BLACK;
         break;
      }

   return C;
   }


NsError _ns_closure_record_new
   (
   NsClosureRecord  **record,
   const nschar      *description,
   nspointer          user_data
   )
   {
   NsError error;

   if( NULL == ( *record = ns_new( NsClosureRecord ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   if( NS_FAILURE( ns_closure_record_construct( *record, description, user_data ), error ) )
      {
      ns_delete( *record );
      return error;
      }

   return ns_no_error();
   }


void _ns_closure_record_delete( NsClosureRecord *record )
   {
   ns_closure_record_destruct( record );
   ns_delete( record );
   }


NsClosureRecord* _workspace_create_record
	(
	NsWorkspace   *wr,
	const nschar  *description,
	nspointer      user_data
	)
   {
   NsClosureRecord  *record;
   NsError           error;


   record = NULL;

   if( NS_FAILURE( _ns_closure_record_new( &record, description, user_data ), error ) )
      return NULL;

   if( NS_FAILURE( ns_list_push_back( &wr->records, record ), error ) )
      {
      _ns_closure_record_delete( record );
      return NULL;
      }

   return record;
   }


void _workspace_remove_last_record( NsWorkspace *wr, NsClosureRecord *record )
   {
   if( NULL != record && ! ns_list_is_empty( &wr->records ) )
      ns_list_pop_back( &wr->records );
   }




void _ConstructWorkspaceResource
   ( 
   NsWorkspace   *wr,
   const nschar  *path
   )
   {
   nsuint   window;
   nsuint   display;
	//NsError  error;


	//____workspace_has_tiles = NS_FALSE;
//	____workspace_async_proj_iterations = 0;

   _init_workspace_colors();
 
   ns_assert( NULL != wr );
   ns_assert( NULL != path );

   ns_memset( wr, 0, sizeof( NsWorkspace ) );

	//ns_mutex_construct( &wr->tile_mutex );

	//if( NS_FAILURE( ns_mutex_create( &wr->tile_mutex ), error ) )
	//	{
	//	g_Error = eERROR_OUT_OF_MEMORY;
	//	return;
	//	}

   ns_ascii_strncpy( wr->path, path, _MAX_PATH - 1 );

   wr->displayFlags = mDISPLAY_GRAPHICS_IMAGE        |
                               mDISPLAY_GRAPHICS_LOW_CONTOUR  |
                               mDISPLAY_GRAPHICS_HIGH_CONTOUR |
                               mDISPLAY_NEURON_TREE_VERTICES  |
                               mDISPLAY_NEURON_TREE_EDGES;

   wr->paletteType           = eWORKSPACE_PALETTE_STANDARD;
   //wr->hasTransparentPalette = 0;


   wr->colors[ WORKSPACE_COLOR_BACKGROUND ] = WORKSPACE_DEFAULT_COLOR_BACKGROUND;

	wr->colors[ WORKSPACE_COLOR_FUNCTION_SOMA ] = WORKSPACE_DEFAULT_COLOR_FUNCTION_SOMA;
	wr->colors[ WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ] = WORKSPACE_DEFAULT_COLOR_FUNCTION_BASAL_DENDRITE;
	wr->colors[ WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ] = WORKSPACE_DEFAULT_COLOR_FUNCTION_APICAL_DENDRITE;
	wr->colors[ WORKSPACE_COLOR_FUNCTION_AXON ] = WORKSPACE_DEFAULT_COLOR_FUNCTION_AXON;

   wr->colors[ WORKSPACE_COLOR_LINE_VERTEX ] = WORKSPACE_DEFAULT_COLOR_LINE_VERTEX;
   wr->colors[ WORKSPACE_COLOR_JUNCTION_VERTEX ] = WORKSPACE_DEFAULT_COLOR_JUNCTION_VERTEX;
   wr->colors[ WORKSPACE_COLOR_EXTERNAL_VERTEX ] = WORKSPACE_DEFAULT_COLOR_EXTERNAL_VERTEX;
   wr->colors[ WORKSPACE_COLOR_ORIGIN_VERTEX ] = WORKSPACE_DEFAULT_COLOR_ORIGIN_VERTEX;
   wr->colors[ WORKSPACE_COLOR_VERTEX ] = WORKSPACE_DEFAULT_COLOR_VERTEX;
   wr->colors[ WORKSPACE_COLOR_EDGE ] = WORKSPACE_DEFAULT_COLOR_EDGE;
   wr->colors[ WORKSPACE_COLOR_SPLAT ] = WORKSPACE_DEFAULT_COLOR_SPLAT;

   wr->colors[ WORKSPACE_COLOR_SPINE_OTHER ] = WORKSPACE_DEFAULT_COLOR_SPINE_OTHER;
   wr->colors[ WORKSPACE_COLOR_SPINE_THIN ] = WORKSPACE_DEFAULT_COLOR_SPINE_THIN;
   wr->colors[ WORKSPACE_COLOR_SPINE_MUSHROOM ] = WORKSPACE_DEFAULT_COLOR_SPINE_MUSHROOM;
   wr->colors[ WORKSPACE_COLOR_SPINE_STUBBY ] = WORKSPACE_DEFAULT_COLOR_SPINE_STUBBY;
   wr->colors[ WORKSPACE_COLOR_SPINE ] = WORKSPACE_DEFAULT_COLOR_SPINE;

 //  wr->brightness = 0;
 //  wr->contrast   = 0;

	wr->max_intensity = 0;

   wr->rawTree            = wr->models + 0;
   wr->filteredTree       = wr->models + 1;
   //wr->temp_raw_tree      = wr->models + 2;
   //wr->temp_filtered_tree = wr->models + 3;

   ns_model_construct( wr->rawTree );
   ns_model_construct( wr->filteredTree );
   //ns_model_construct( wr->temp_raw_tree );
   //ns_model_construct( wr->temp_filtered_tree );


	/*error?*/ns_model_set_name( wr->rawTree, "raw" );
	/*error?*///ns_model_set_name( wr->temp_raw_tree, "raw" );
	/*error?*/ns_model_set_name( wr->filteredTree, "filtered" );
	/*error?*///ns_model_set_name( wr->temp_filtered_tree, "filtered" );


/* TEMP!!!!!!! check for error!!!!!!!!!!! */
//ns_voxel_table_construct( &wr->voxel_table, NULL );


   ns_settings_init_with_user_defaults( &wr->settings );

   //wr->setting_flags = NS_SETTINGS_ALL;


	ns_image_construct( &wr->volume );
	ns_image_construct( &wr->proj_xy );
	ns_image_construct( &wr->proj_zy );
	ns_image_construct( &wr->proj_xz );

	ns_tile_mgr_construct( &wr->tile_mgr_xy );
	ns_tile_mgr_construct( &wr->tile_mgr_zy );
	ns_tile_mgr_construct( &wr->tile_mgr_xz );
	ns_tile_mgr_construct( &wr->tile_mgr_slice );

	ns_cubei_zero( &wr->update_roi );
	ns_cubei_zero( &wr->saved_roi );

	/*error*/ns_image_set_name( &wr->volume, "volume" );
	/*error*/ns_image_set_name( &wr->proj_xy, "proj_xy" );
	/*error*/ns_image_set_name( &wr->proj_zy, "proj_zy" );
	/*error*/ns_image_set_name( &wr->proj_xz, "proj_xz" );

   //ConstructImage( &wr->image_xy );
   //ConstructImage( &wr->image_zy );
   //ConstructImage( &wr->image_xz );

   //____forward = &wr->image_xy;
   //____side    = &wr->image_zy;
   //____top     = &wr->image_xz;

   //wr->gc_xy.hDC = NULL;
   //wr->gc_zy.hDC    = NULL;
   //wr->gc_xz.hDC     = NULL;

   for( window = 0; window < mNUM_DISPLAY_WINDOWS; ++window )
      for( display = 0; display < mNUM_DISPLAYS; ++display )
         wr->zoom[ window ][ display ] = 1.0f;

   ns_proc_db_construct( &wr->ppdb );
	ns_proc_db_construct( &wr->iodb );

	ns_pixel_register_value_types();

   ns_pixel_proc_db_register_std( &wr->ppdb );
	ns_image_io_proc_db_register_std( &wr->iodb );

   ns_sampler_construct( &wr->sampler );

   ns_list_construct( &wr->records, _ns_closure_record_delete );

   //ns_image_construct( &wr->thumbnail_xy );
   //ns_image_construct( &wr->thumbnail_zy );
   //ns_image_construct( &wr->thumbnail_xz );

	/*error*///ns_image_set_name( &wr->thumbnail_xy, "thumbnail_xy" );
	/*error*///ns_image_set_name( &wr->thumbnail_zy, "thumbnail_zy" );
	/*error*///ns_image_set_name( &wr->thumbnail_xz, "thumbnail_xz" );

	ns_image_construct( &wr->slice_xy );
	/*error*/ns_image_set_name( &wr->slice_xy, "slice_xy" );

	ns_image_set_pixel_proc_db( &wr->slice_xy, &wr->ppdb );
	ns_image_set_io_proc_db( &wr->slice_xy, &wr->iodb );

	//ns_image_construct( &wr->flipped_xz );
	///*error*/ns_image_set_name( &wr->flipped_xz, "flipped_xz" );
	//ns_image_set_pixel_proc_db( &wr->flipped_xz, &wr->ppdb );

	ns_image_set_pixel_proc_db( &wr->volume, &wr->ppdb );
	ns_image_set_io_proc_db( &wr->volume, &wr->iodb );

	ns_image_set_pixel_proc_db( &wr->proj_xy, &wr->ppdb );
	ns_image_set_io_proc_db( &wr->proj_xy, &wr->iodb );

	ns_image_set_pixel_proc_db( &wr->proj_zy, &wr->ppdb );
	ns_image_set_io_proc_db( &wr->proj_zy, &wr->iodb );

	ns_image_set_pixel_proc_db( &wr->proj_xz, &wr->ppdb );
	ns_image_set_io_proc_db( &wr->proj_xz, &wr->iodb );

	//ns_image_construct( &wr->tile );
	//ns_image_set_pixel_proc_db( &wr->tile, &wr->ppdb );
	//ns_image_set_buffer_type( &wr->tile, NS_IMAGE_BUFFER_GROW_ONLY );

	ns_list_construct( &wr->image_extra_info, ns_free );

   ns_color_db_construct( &wr->brush_db );
   ns_color_db_construct( &wr->pen_db );

	_workspace_do_set_palette( wr );

	ns_sholl_analysis_construct( &wr->sholl_analysis );

	wr->roi_control_selections = 0;
   }
		

void _DestructWorkspaceResource( NsWorkspace* wr )
   {
	//____workspace_has_tiles = NS_FALSE;
	//____workspace_async_proj_iterations = 0;

   ns_assert( NULL != wr );

	ns_sholl_analysis_destruct( &wr->sholl_analysis );

   ns_color_db_destruct( &wr->brush_db );
   ns_color_db_destruct( &wr->pen_db );

	ns_list_destruct( &wr->image_extra_info );

	//ns_image_destruct( &wr->tile );

	ns_image_destruct( &wr->slice_xy );
	//ns_image_destruct( &wr->flipped_xz );

   //ns_image_destruct( &wr->thumbnail_xy );
   //ns_image_destruct( &wr->thumbnail_zy );
   //ns_image_destruct( &wr->thumbnail_xz );

   ns_list_destruct( &wr->records );

   ns_sampler_destruct( &wr->sampler );
   
   //DestructImage( &wr->image_xy );
   //DestructImage( &wr->image_zy );
   //DestructImage( &wr->image_xz );

   //DestroyMemoryGraphics( &wr->gc_xy );
   //DestroyMemoryGraphics( &wr->gc_zy );
   //DestroyMemoryGraphics( &wr->gc_xz );

   ns_model_destruct( wr->rawTree );
   ns_model_destruct( wr->filteredTree );
   //ns_model_destruct( wr->temp_raw_tree );
   //ns_model_destruct( wr->temp_filtered_tree );
   
   //ns_voxel_table_destruct( &wr->voxel_table );

	ns_tile_mgr_destruct( &wr->tile_mgr_xy );
	ns_tile_mgr_destruct( &wr->tile_mgr_zy );
	ns_tile_mgr_destruct( &wr->tile_mgr_xz );
	ns_tile_mgr_destruct( &wr->tile_mgr_slice );

	ns_image_destruct( &wr->volume );
	ns_image_destruct( &wr->proj_xy );
	ns_image_destruct( &wr->proj_zy );
	ns_image_destruct( &wr->proj_xz );

	ns_proc_db_destruct( &wr->ppdb );
	ns_proc_db_destruct( &wr->iodb );

	//ns_mutex_destruct( &wr->tile_mutex );

   ns_free( wr );

	//ns_println( "free volume.pixels = %p\n", ____volume_pixels );

   //ns_free( ____volume_pixels );

  // ____volume_pixels = NULL;
  // ____volume_width = 0;
  // ____volume_height = 0;
  // ____volume_length = 0;
  // ____volume_bpp = 0;
   }


nsint WorkspaceIsValid( const HWorkspace handle )
   {  return ( 0 < handle && handle <= s_NumWorkspaces ) ? 1 : 0; }



NsWorkspace* _ns_workspace( const HWorkspace handle )
   {
   NsWorkspace *wr;

   ns_assert( WorkspaceIsValid( handle ) );
   wr = s_Workspaces[ handle - 1 ];

   ns_assert( NULL != wr );
   return wr;
   }


nsint workspace_get_palette_type( HWorkspace handle )
	{  return _ns_workspace( handle )->paletteType;  }

void workspace_set_palette_type( HWorkspace handle, nsint palette_type )
	{
	NsWorkspace *wr = _ns_workspace( handle );

	wr->paletteType = palette_type;
	_workspace_do_set_palette( wr );
	}


nsboolean workspace_is_lsm_image( const HWorkspace handle )
	{  return _ns_workspace( handle )->is_lsm;  }


nsushort workspace_get_roi_control_selections( nsuint handle )
	{  return _ns_workspace( handle )->roi_control_selections;  }


void workspace_set_roi_control_selections( nsuint handle, nsushort selections )
	{  _ns_workspace( handle )->roi_control_selections = selections; }


const TiffLsmInfo* workspace_get_lsm_info( const HWorkspace handle )
	{
	NsWorkspace *wr = _ns_workspace( handle );

	ns_assert( wr->is_lsm );
	return &(wr->lsm_info);
	}


void workspace_set_lsm_info( HWorkspace handle, const TiffLsmInfo *lsm_info )
	{
	NsWorkspace *wr = _ns_workspace( handle );

	ns_assert( NULL != lsm_info );

	wr->is_lsm   = NS_TRUE;
	wr->lsm_info = *lsm_info;
	}


NsList* workspace_image_extra_info( HWorkspace handle )
	{  return &( _ns_workspace( handle )->image_extra_info );  }



NsCubei* workspace_update_roi( HWorkspace handle )
	{  return &( _ns_workspace( handle )->update_roi );  }

NsCubei* workspace_visual_roi( HWorkspace handle )
	{  return &( _ns_workspace( handle )->saved_roi );  }

nssize workspace_visual_roi_length( HWorkspace handle )
	{  return ( nssize )ns_cubei_length( &( _ns_workspace( handle )->saved_roi ) );  }



/*
void DestroyAllWorkspaces( void )
   {
   nsuint i;

   for( i = 0; i < s_NumWorkspaces; ++i )
      _DestructWorkspaceResource( s_Workspaces[ i ] );

   s_NumWorkspaces = 0;
   }
*/


void DestructWorkspace( const HWorkspace handle )
   { 
   ns_assert( WorkspaceIsValid( handle ) );

	workspace_end_projections( handle );

   /* TEMP!!!!!!!!!!!!!!!!!!!! */
   _DestructWorkspaceResource( _ns_workspace( handle ) );
   --s_NumWorkspaces;
   }


/*
eERROR_TYPE _DoBuildWorkspaceResourceGraphics
   (
   NsWorkspace*  wr,
   MemoryGraphics*    graphics,
   NsImage*            projection,
   HWND                     hWnd
   )
   {
   eERROR_TYPE error = eNO_ERROR;

   HDC hDC;

   hDC = GetDC( hWnd );

   if( ! CreateMemoryGraphics( graphics, hDC,
			( nsuint )ns_image_width( projection ),
			( nsuint )ns_image_height( projection ) ) )
       error = g_Error;

   ReleaseDC( hWnd, hDC );

   if( eNO_ERROR != error )
      return error;

   return eNO_ERROR;
   }*/


//eERROR_TYPE _workspace_allocate_graphics( NsWorkspace *wr, HWND hWnd )
  // {
	/*
   if( eNO_ERROR != _DoBuildWorkspaceResourceGraphics(
                    wr,
                    &wr->gc_xy,
                    &wr->proj_xy,
                    hWnd
               ) ||
  
       eNO_ERROR != _DoBuildWorkspaceResourceGraphics(
                    wr,
                    &wr->gc_zy,
                    &wr->proj_zy,
                    hWnd
                ) ||
       
       eNO_ERROR != _DoBuildWorkspaceResourceGraphics(
                    wr,
                    &wr->gc_xz,
                    &wr->proj_xz,
                    hWnd
                )
      )
      return g_Error;
	*/

   //return eNO_ERROR;
   //}


/*
eERROR_TYPE _CleanWorkspaceProjections( NsWorkspace *wr, eERROR_TYPE error )
   {
   DestructImage( &wr->image_xy );
   DestructImage( &wr->image_zy );
   DestructImage( &wr->image_xz );
   return error;
   }
*/

/*
void _OutputVolumeProjectionsGrayscale8
   ( 
   const Volume*  volume,
   const nschar*    forwardFileName,
   const nschar*    sideFileName,
   const nschar*    topFileName
   );
*/


nsboolean __progress_cancelled( NsProgress *progress )
   {
   return IsProgressCancelled();
   //return progress_was_cancelled( ( Progress* )ns_progress_get_user_data( progress ) );
   }

void __progress_update( NsProgress *progress, nsfloat percent )
   {
   SetProgressPercent( percent );
   /*progress_set_percent_complete( ( Progress* )ns_progress_get_user_data( progress ), percent );*/ }

void __progress_set_title( NsProgress *progress, const nschar *title )
   {
   SetProgressTitle( title );
   /*progress_set_title( ( Progress* )ns_progress_get_user_data( progress ), title );*/
   }


//#include <image/nsimagedir.h>

NS_PRIVATE NsPixelType _bits_per_pixel_to_pixel_type( nssize bits_per_pixel )
	{
	NsPixelType pixel_type = NS_PIXEL_UNSUPPORTED;

	switch( bits_per_pixel )
		{
		case  8: pixel_type = NS_PIXEL_LUM_U8;  break;
		case 12: pixel_type = NS_PIXEL_LUM_U12; break;
		case 16: pixel_type = NS_PIXEL_LUM_U16; break;
		}

	return pixel_type;
	}


eERROR_TYPE _ReadVolume_2D_sequence( NsWorkspace *wr, NsImage *volume, const NsImageDir *dir, void *dialog )
   {
	nssize          z;
	nssize          bytes_per_slice;
	nssize          length;
	nsimagediriter  curr_file;
	nsimagediriter  end_files;
	NsProgress      nsprogress;
	Progress        progress;
	Image           image;
	NsError         error;


	ns_assert( 0 == ns_image_size( volume ) );

   length = ns_image_dir_size( dir );
   ns_assert( 0 < length );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	ns_progress_set_title( &nsprogress, "Reading files..." );
	ns_progress_num_iters( &nsprogress, length );
	ns_progress_begin( &nsprogress );

   curr_file = ns_image_dir_begin( dir );
   end_files = ns_image_dir_end( dir );

	bytes_per_slice = 0;
 
   for( z = 0; z < length; ++z )
      {
      ConstructImage( &image );

      if( ns_progress_cancelled( &nsprogress ) )
         {
			ns_assert( NULL == image.pixels );
         ns_image_clear( volume );
         return eNO_ERROR;
         }

      if( eNO_ERROR !=
				ReadImage2D(
					&image,
					ns_image_dir_iter_file( curr_file ),
					0 < z ? ns_image_pixels_slice( volume, z ) : NULL,
					bytes_per_slice,
					&wr->max_intensity
					) )
         {
			ns_assert( NULL == image.pixels );
         ns_image_clear( volume );
         return g_Error;
         }

      if( ns_progress_cancelled( &nsprogress ) )
         {
			ns_assert( NULL == image.pixels );
         ns_image_clear( volume );
         return eNO_ERROR;
         }

      if( 1 != image.length )
         {
         ns_println( "File is not 2D!" );
         DestructImage( &image );
         ns_image_clear( volume );
         return ( g_Error = eERROR_FILE_INVALID );
         }

		ns_assert( 0 < image.width );
		ns_assert( 0 < image.height );

      if( 0 == z )
         {
			ns_assert( NULL != image.pixels );

         if( NS_PIXEL_UNSUPPORTED == _bits_per_pixel_to_pixel_type( image.bitsPerPixel ) )
            {
				DestructImage( &image );
				ns_assert( 0 == ns_image_size( volume ) );
            return ( g_Error = eERROR_PIXEL_FORMAT );
            }

			ns_assert( 0 == ns_image_size( volume ) );

			if( NS_FAILURE(
					ns_image_create(
						volume,
						_bits_per_pixel_to_pixel_type( image.bitsPerPixel ),
						image.width, image.height, length, 1
						),
					error ) )
				{
				DestructImage( &image );
				return ( g_Error = eERROR_OUT_OF_MEMORY );
				}

			bytes_per_slice = ns_pixel_buffer_slice_size(
										ns_image_pixel_type( volume ),
										ns_image_width( volume ),
										ns_image_height( volume ),
										ns_image_row_align( volume )
										);

         ns_memcpy( ns_image_pixels_slice( volume, z ), image.pixels, bytes_per_slice );
         }
      else
         {
			ns_assert( NULL == image.pixels );
 
         if( _bits_per_pixel_to_pixel_type( image.bitsPerPixel ) != ns_image_pixel_type( volume ) )
            {
            ns_image_clear( volume );
            return ( g_Error = eERROR_MULTIPLE_PIXEL_FORMATS );
            }

         if( image.width != ns_image_width( volume ) || image.height != ns_image_height( volume ) )
				{
            ns_image_clear( volume );
            return ( g_Error = eERROR_MULTIPLE_WIDTH_HEIGHT );
            }
         }

      if( NS_PIXEL_UNSUPPORTED == _bits_per_pixel_to_pixel_type( image.bitsPerPixel ) )
         {
         DestructImage( &image );
         ns_image_clear( volume );
         return ( g_Error = eERROR_PIXEL_FORMAT );
         }

      DestructImage( &image );

		curr_file = ns_image_dir_iter_next( curr_file );
		ns_progress_next_iter( &nsprogress );
      }

   ns_assert( ns_image_dir_iter_equal( curr_file, end_files ) );
	ns_progress_end( &nsprogress );

ns_println( "max_intensity = " NS_FMT_ULONG, wr->max_intensity );

	if( NS_PIXEL_LUM_U16 == ns_image_pixel_type( volume ) && wr->max_intensity < 4096 )
		ns_pixel_info( &volume->pixel_info, NS_PIXEL_LUM_U12 );

   return eNO_ERROR;
   }


eERROR_TYPE _ReadVolume_3D( NsWorkspace *wr, NsImage* volume, const nschar* path, void *dialog )
   {
   //nssize    bitsPerPixel;
   //nssize    bytesPerPixel;
   Image       image;
   NsProgress  nsprogress;
   Progress    progress;


   g_Progress = dialog;

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   SetProgressTitle( "Reading file..." );

   ns_progress_begin( &nsprogress );

   image.pixels = NULL;
   if( eNO_ERROR != ReadImage3D( &image, path, &nsprogress, &wr->max_intensity ) )
      {
      ns_assert( NULL == image.pixels );
      return g_Error;
      }

   ns_progress_end( &nsprogress );

   if( IsProgressCancelled() )
		{
		ns_assert( NULL == image.pixels );
		return eNO_ERROR;
		}

	ns_assert( NULL != image.pixels );

   //if( 8 != image.bitsPerPixel && 12 != image.bitsPerPixel && 16 != image.bitsPerPixel )
	if( NS_PIXEL_UNSUPPORTED == _bits_per_pixel_to_pixel_type( image.bitsPerPixel ) )
      {
      DestructImage( &image );
      return ( g_Error = eERROR_PIXEL_FORMAT );
      }

   //bitsPerPixel = image.bitsPerPixel;

   //if( 12 == bitsPerPixel )
     // bitsPerPixel = 16;

//   ns_assert( 0 == bitsPerPixel % 8 );

   //bytesPerPixel = bitsPerPixel / 8;
   //ns_assert( 1 == bytesPerPixel || 2 == bytesPerPixel );

	ns_assert( 0 == ns_image_size( volume ) );
	ns_image_buffer_and_free(
		volume,
		_bits_per_pixel_to_pixel_type( image.bitsPerPixel ),
		image.width, image.height, image.length, 1,
		image.pixels
		);

/*   volume->bitsPerPixel = bitsPerPixel;

   volume->width  = image.width;
   volume->height = image.height;
   volume->length = image.length;
   volume->pixels = image.pixels;
*/

ns_println( "max_intensity = " NS_FMT_ULONG, wr->max_intensity );

	if( NS_PIXEL_LUM_U16 == ns_image_pixel_type( volume ) && wr->max_intensity < 4096 )
		ns_pixel_info( &volume->pixel_info, NS_PIXEL_LUM_U12 );

   return eNO_ERROR;
   }




/*
eERROR_TYPE _convert_image_to_win32_scan_aligned_image( Image *projection )
   {
   nssize    bytesPerRow;
   nssize    alignedBytesPerRow;

   nssize     i;
   void         *temp;
   nsuchar *src;
   nsuchar *dest;


   bytesPerRow = projection->width * NS_BITS_TO_BYTES( projection->bitsPerPixel );

   // For Win32, each scan line of the image must be aligned to LONG boundaries.

   if( 0 != ( bytesPerRow % sizeof( LONG ) ) )
      {
      alignedBytesPerRow = mALIGN( bytesPerRow, sizeof( LONG ) );

      if( NULL == ( temp = ns_calloc( alignedBytesPerRow * projection->height, 1 ) ) )
         return ( g_Error = eERROR_OUT_OF_MEMORY );

      src  = ( nsuchar * )projection->pixels;
      dest = ( nsuchar* )temp;

      for( i = 0; i < projection->height; ++i )
          {
          ns_memcpy( dest, src, bytesPerRow );
          src += bytesPerRow;
          dest += alignedBytesPerRow;
          }

      ns_free( projection->pixels );
      projection->pixels = temp;
      projection->saBoundary = sizeof( LONG );
      }

   return eNO_ERROR;
   }
*/

/*
void _hack_image( NsImage *image, NsProcDb *ppdb, nspointer pixels,
nssize width, nssize height, nssize length, nssize bpp )
   {
   ns_memset( image, 0, sizeof( NsImage ) );

   image->pixel_proc_db        = ppdb;
   image->image_info.width     = width;
   image->image_info.height    = height;
   image->image_info.length    = length;
   image->image_info.row_align = 1;
   image->pixels.bytes         = pixels;

   ns_pixel_info( &image->pixel_info, NS_PIXEL_LUM_U8  );

   switch( bpp )
      {
      case 12: ns_pixel_info( &image->pixel_info, NS_PIXEL_LUM_U12 ); break;
      case 16: ns_pixel_info( &image->pixel_info, NS_PIXEL_LUM_U16 ); break;
      }
   }
*/


/*
void _flip_image_vertical( nspointer pixels, nssize width, nssize height, nssize bytes_per_row )
   {
   nspointer  row1;
   nspointer  row2;
   nspointer  temp;


   if( NULL == ( temp = ns_malloc( bytes_per_row ) ) )
      return;

   row1 = pixels;
   row2 = NS_OFFSET_POINTER( void, pixels, ( height - 1 ) * bytes_per_row );

   while( row1 < row2 )
      {
      ns_memcpy( temp, row1, bytes_per_row ); /* temp = row1 *//*
      ns_memcpy( row1, row2, bytes_per_row ); /* row1 = row2 *//*
      ns_memcpy( row2, temp, bytes_per_row ); /* row2 = temp *//*

      row1 = NS_INCREMENT_POINTER( void, row1, bytes_per_row );
      row2 = NS_DECREMENT_POINTER( void, row2, bytes_per_row );
      }

   ns_free( temp );
   }*/



extern nsint ____projection_type;

NS_PRIVATE nsint ____workspace_read_project_type( void )
   {
   return ____projection_type;
/*
   nsint project_type;
   NsConfigDb db;

   project_type = NS_PIXEL_PROC_ORTHO_PROJECT_MAX;

   if( NS_FAILURE( ns_config_db_construct( &db );
   ns_config_db_read( &db, ____config_file, NULL );

   if( ns_config_db_has_key( &db, "projection", "type" ) )
      project_type = ns_config_db_get_int( &db, "projection", "type" );

   ns_config_db_destruct( &db );

   return project_type;
*/
   }



NsError _workspace_do_build_slice_view( NsWorkspace*, nssize );
void _workspace_do_set_graphics_xy( NsWorkspace* );


NS_PRIVATE void _workspace_validate_projection
	(
	const NsWorkspace  *wr,
	const NsImage      *image,
	nsint               which
	)
	{
	ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( image ) );
	ns_assert( _IMAGE_ROW_ALIGN == ns_image_row_align( image ) );
	ns_assert( 1 == ns_image_length( image ) );

	switch( which )
		{
		case NS_XY:
			ns_assert( ns_image_width( image )  == ns_image_width( &wr->volume ) );
			ns_assert( ns_image_height( image ) == ns_image_height( &wr->volume ) );
			break;

		case NS_ZY:
			ns_assert( ns_image_width( image )  == ns_image_length( &wr->volume ) );
			ns_assert( ns_image_height( image ) == ns_image_height( &wr->volume ) );
			break;

		case NS_XZ:
			ns_assert( ns_image_width( image )  == ns_image_width( &wr->volume ) );
			ns_assert( ns_image_height( image ) == ns_image_length( &wr->volume ) );
			break;

		default:
			ns_assert_not_reached();
		}
	}


void _do_redraw_workspace_display_graphics( NsWorkspace *wr );

extern void ____redraw_2d();
extern void _set_display_window_workspace_handle( void );
extern void _fit_display_window_projection_xy( void );
//extern void _redraw_display_bypass_queue( void );


NS_PRIVATE nsboolean _workspace_projections_progress_cancelled( NsProgress *progress )
	{  return ____projections_terminate;  }

NS_PRIVATE void _workspace_projections_progress_update( NsProgress *progress, nsfloat percent )
	{}

NS_PRIVATE void _workspace_projections_set_title( NsProgress *progress, const nschar *title )
	{}

NS_PRIVATE void _workspace_projections_set_description( NsProgress *progress, const nschar *description )
	{}


NS_PRIVATE NsError _workspace_do_run_projections
	(
	NsWorkspace  *wr,
	NsImage      *image_xy,
	NsImage      *image_zy,
	NsImage      *image_xz,
	NsImage      *ranged_xy,
	NsImage      *ranged_zy,
	NsImage      *ranged_xz,
	nsint         project_type,
	nssize       *z_indices,
	nssize        num_z_indices,
	nsboolean     need_convert
	)
	{
	NsProgress     progress;
	NsImage        conv_xy, conv_zy, conv_xz;
	nsfloat        channel_min, channel_max;
	NsPixelRegion  region;
	nssize         i;
	NsError        error;



	error = ns_no_error();

	ns_progress(
		&progress,
		_workspace_projections_progress_cancelled,
		_workspace_projections_progress_update,
		_workspace_projections_set_title,
		_workspace_projections_set_description,
		NULL
		);

	ns_image_construct( &conv_xy );
	ns_image_construct( &conv_zy );
	ns_image_construct( &conv_xz );

	ns_image_set_pixel_proc_db( &conv_xy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &conv_zy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &conv_xz, &wr->ppdb );

	region.x      = ( nssize )wr->saved_roi.C1.x;
	region.y      = ( nssize )wr->saved_roi.C1.y;
	region.z      = ( nssize )wr->saved_roi.C1.z;
	region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	region.length = ( nssize )ns_cubei_length( &wr->saved_roi );

	/* Adjust the indices so that they fall within the region. */
	for( i = 0; i < num_z_indices; ++i )
		{
		ns_assert( z_indices[i] < region.length );
		z_indices[i] += region.z;
		}

	ns_image_ortho_project(
		&wr->volume,
		&region,
		z_indices,
		num_z_indices,
		ns_image_row_align( image_xy ), image_xy,
		ns_image_row_align( image_zy ), image_zy,
		ns_image_row_align( image_xz ), image_xz,
		project_type,
		&wr->average_intensity,
		&wr->num_seen_voxels,
		//&wr->max_intensity,
		NS_TRUE,
		&progress
		);

/*TEMP*/
//ns_println( "Current average intensity = %f", wr->average_intensity );
//ns_println( "Current number of voxels  = " NS_FMT_ULONG, wr->num_seen_voxels );

	if( need_convert )
		{
		if( NS_PIXEL_PROC_ORTHO_PROJECT_SUM == project_type )
			{
			channel_min = 0.0f;
			channel_max = 255.0f;

			ns_assert( image_xy != ranged_xy );
			ns_image_copy( ranged_xy, image_xy, NULL, NULL );

			ns_assert( image_zy != ranged_zy );
			ns_image_copy( ranged_zy, image_zy, NULL, NULL );

			ns_assert( image_xz != ranged_xz );
			ns_image_copy( ranged_xz, image_xz, NULL, NULL );

			ns_image_dynamic_range( ranged_xy, NULL, channel_min, channel_max, NULL, NULL );
			ns_image_dynamic_range( ranged_zy, NULL, channel_min, channel_max, NULL, NULL );
			ns_image_dynamic_range( ranged_xz, NULL, channel_min, channel_max, NULL, NULL );

			image_xy = ranged_xy;
			image_zy = ranged_zy;
			image_xz = ranged_xz;
			}

		/* IMPORTANT: Remember for Windows must pad rows to the sizeof( LONG )! */
		/*error*/ns_image_convert( image_xy, NS_PIXEL_LUM_U8, _IMAGE_ROW_ALIGN, &conv_xy, NULL );
		/*error*/ns_image_convert( image_zy, NS_PIXEL_LUM_U8, _IMAGE_ROW_ALIGN, &conv_zy, NULL );
		/*error*/ns_image_convert( image_xz, NS_PIXEL_LUM_U8, _IMAGE_ROW_ALIGN, &conv_xz, NULL );

		ns_assert( image_xy != &wr->proj_xy );
 		ns_image_copy( &wr->proj_xy, &conv_xy, NULL, NULL );

		ns_assert( image_zy != &wr->proj_zy );
		ns_image_copy( &wr->proj_zy, &conv_zy, NULL, NULL );

		ns_assert( image_xz != &wr->proj_xz );
		ns_image_copy( &wr->proj_xz, &conv_xz, NULL, NULL );
      }

	_workspace_validate_projection( wr, &wr->proj_xy, NS_XY );
	_workspace_validate_projection( wr, &wr->proj_zy, NS_ZY );
	_workspace_validate_projection( wr, &wr->proj_xz, NS_XZ );

	ns_image_destruct( &conv_xy );
	ns_image_destruct( &conv_zy );
	ns_image_destruct( &conv_xz );

	//ns_image_copy( &wr->flipped_xz, &wr->proj_xz, NULL, NULL );
	//ns_image_flip_vertical( &wr->flipped_xz, NULL, NULL );

	_do_redraw_workspace_display_graphics( wr );
	____redraw_2d();

	return ns_no_error();
	}


#include <ext/progressive.inl>

NS_PRIVATE void _workspace_run_projections_thread( NsWorkspace *wr )
   {
	NsImage       proj_xy, proj_zy, proj_xz;
	NsImage       ranged_xy, ranged_zy, ranged_xz;
   NsImage      *image_xy, *image_zy, *image_xz;
   nsint         project_type;
	PROGRESSIVE   pr;
	nssize        length;
	nsint         prev_z;
	nsint         curr_z;
	nsboolean     do_log;
	nsboolean     need_convert;
	nsboolean     done;
	nssize       *z_indices;
	nssize        num_z_indices;
	NsThread      thread;
	nssize        iterations;
	NsError       error;


_PRINT_LAST_ERROR();

	ns_thread_construct( &thread );
	ns_thread_self( &thread );
	ns_thread_set_priority( &thread, NS_THREAD_PRIORITY_LOW );
	ns_thread_destruct( &thread );

	do_log = ns_log_is_enabled();
	error  = ns_no_error();

	ns_image_construct( &proj_xy );
	ns_image_construct( &proj_zy );
	ns_image_construct( &proj_xz );
	ns_image_construct( &ranged_xy );
	ns_image_construct( &ranged_zy );
	ns_image_construct( &ranged_xz );

	ns_image_set_pixel_proc_db( &proj_xy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &proj_zy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &proj_xz, &wr->ppdb );
	ns_image_set_pixel_proc_db( &ranged_xy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &ranged_zy, &wr->ppdb );
	ns_image_set_pixel_proc_db( &ranged_xz, &wr->ppdb );

	wr->average_intensity = 0.0;
	wr->num_seen_voxels   = 0;

	project_type = ____workspace_read_project_type();

	length = ( nssize )ns_cubei_length( &wr->saved_roi );/* ns_image_length( &wr->volume ); */
	ns_assert( 0 < length && length <= ns_image_length( &wr->volume ) );

	if( NULL == ( z_indices = ns_new_array( nssize, length ) ) )
		goto _WORKSPACE_RUN_PROJECTIONS_EXIT;

	if( NS_PIXEL_LUM_U8 != ns_image_pixel_type( &wr->volume ) ||
		 NS_PIXEL_PROC_ORTHO_PROJECT_SUM == project_type         )
		{
		if( NS_FAILURE(
				ns_image_ortho_project_create(
					&wr->volume,
					NULL,
					1, &proj_xy,
					1, &proj_zy,
					1, &proj_xz,
					project_type,
					NS_TRUE
					),
				error ) )
			goto _WORKSPACE_RUN_PROJECTIONS_EXIT;

		if( NS_PIXEL_PROC_ORTHO_PROJECT_SUM == project_type )
			if( NS_FAILURE(
					ns_image_ortho_project_create(
						&wr->volume,
						NULL,
						1, &ranged_xy,
						1, &ranged_zy,
						1, &ranged_xz,
						project_type,
						NS_TRUE
						),
					error ) )
				goto _WORKSPACE_RUN_PROJECTIONS_EXIT;

		image_xy = &proj_xy;
		image_zy = &proj_zy;
		image_xz = &proj_xz;

		need_convert = NS_TRUE;
		}
	else
		{
		image_xy = &wr->proj_xy;
		image_zy = &wr->proj_zy;
		image_xz = &wr->proj_xz;

		need_convert = NS_FALSE;
		}

	ns_image_ortho_project_init( image_xy, image_zy, image_xz, project_type );

	progressive_init( &pr, ( nsint )length );

	iterations    = 0;
	done          = NS_FALSE;
	prev_z        = 0;
	num_z_indices = 0;

	NS_INFINITE_LOOP
		{
		//ns_print( "running async projection iteration " NS_FMT_ULONG " out of " NS_FMT_INT "...", iterations + 1, pr.iterations );

		if( 0 < iterations )
			ns_log_enable( NS_FALSE );

		NS_INFINITE_LOOP
			{
			curr_z = progressive_get_next( &pr );

			if( 0 == curr_z )
				{
				done = NS_TRUE;
				break;
				}

			/* NOTE: We check for when we have reached the "end"
				of the image by examining the returned indices.
				This is an optimization this is not technically
				necessary but it reduces the number of projections
				and redraws. */

			if( 0 == prev_z || prev_z < curr_z )
				{
				ns_assert( num_z_indices < length );
				z_indices[ num_z_indices ] = curr_z - 1;


/*TEMP???*/
ns_assert( 0 <= z_indices[ num_z_indices ] );
ns_assert( z_indices[ num_z_indices ] < ( nsint )length );


				++num_z_indices;

				prev_z = curr_z;
				}
			else
				break;
			}

		if( ____projections_terminate )
			{
			//ns_println( NS_MODULE "projections terminate" );
			goto _WORKSPACE_RUN_PROJECTIONS_EXIT;
			}

		if( 0 < num_z_indices )
			/*error*/_workspace_do_run_projections(
				wr,
				image_xy,
				image_zy,
				image_xz,
				&ranged_xy,
				&ranged_zy,
				&ranged_xz,
				project_type,
				z_indices,
				num_z_indices,
				need_convert
				);

		++iterations;

		if( done )
			break;

		ns_assert( 0 < curr_z );

		z_indices[0] = curr_z - 1;

/*TEMP???*/
ns_assert( 0 <= z_indices[0] );
ns_assert( z_indices[0] < ( nsint )length );


		num_z_indices = 1;
		prev_z        = curr_z;

		//ns_println( " done" );
		}

	ns_assert(
		wr->num_seen_voxels ==
		( nssize )ns_cubei_width( &wr->saved_roi ) *
		( nssize )ns_cubei_height( &wr->saved_roi ) *
		( nssize )ns_cubei_length( &wr->saved_roi )
		);

	//wr->average_intensity /=
	//	( ( nsdouble )ns_cubei_width( &wr->saved_roi ) *
	//	  ( nsdouble )ns_cubei_height( &wr->saved_roi ) *
	//	  ( nsdouble )ns_cubei_length( &wr->saved_roi ) );

//   if( NS_PIXEL_LUM_U16 == ns_image_pixel_type( &wr->volume ) &&
  //     wr->max_intensity <= /*TEMP hard-coded!*/4095 )
      //{
//ns_println( "12-bit dataset detected. Converting pixel information." );

  //    ns_pixel_info( &wr->volume.pixel_info, NS_PIXEL_LUM_U12 );

    //  if( NS_PIXEL_PROC_ORTHO_PROJECT_SUM != project_type )
      //   {
        // ns_pixel_info( &image_xy->pixel_info, NS_PIXEL_LUM_U12 );
         //ns_pixel_info( &image_zy->pixel_info, NS_PIXEL_LUM_U12 );
         //ns_pixel_info( &image_xz->pixel_info, NS_PIXEL_LUM_U12 );
         //}
      //}

	/* TEMP: Hard-code gamma value??? */
   /*if( ____gamma_correction )
      {
      ns_image_gamma_correct( &wr->proj_xy, NULL, 1.7, NULL, NULL );
      ns_image_gamma_correct( &wr->proj_zy, NULL, 1.7, NULL, NULL );
      ns_image_gamma_correct( &wr->proj_xz, NULL, 1.7, NULL, NULL );
      }*/

	/* NOTE: Anytime the projections are run then the volume COULD have been changed.
		Therefore we should regenerate the current slice view if it is enabled. */
	if( ____xy_slice_enabled )
		{
		_workspace_do_build_slice_view( wr, ____xy_slice_index );
		_workspace_do_set_graphics_xy( wr );
		}

	//ns_image_copy( &wr->flipped_xz, &wr->proj_xz, NULL, NULL );
	//ns_image_flip_vertical( &wr->flipped_xz, NULL, NULL );

//ns_println( "PROJECTIONS DONE. DRAWING FOR THE LAST TIME!" );
	_do_redraw_workspace_display_graphics( wr );
	____redraw_2d();

	_WORKSPACE_RUN_PROJECTIONS_EXIT:

	ns_log_enable( do_log );

	ns_delete( z_indices );

	ns_image_destruct( &proj_xy );
	ns_image_destruct( &proj_zy );
	ns_image_destruct( &proj_xz );
	ns_image_destruct( &ranged_xy );
	ns_image_destruct( &ranged_zy );
	ns_image_destruct( &ranged_xz );

_PRINT_LAST_ERROR();

	____projections_running = NS_FALSE;
   }


NS_PRIVATE void _workspace_do_end_projections( NsWorkspace *wr )
	{
	if( ____projections_running )
		{
		//ns_println( NS_MODULE "begin projections wait" );

		ns_assert( 0 != ____projections_thread.handle );
		ns_thread_set_priority( &____projections_thread, NS_THREAD_PRIORITY_NORMAL );

		____projections_terminate = NS_TRUE;

		while( ____projections_running )
			ns_thread_yield();

		//ns_println( NS_MODULE "end projetions wait" );
		}
	}


void workspace_end_projections( HWorkspace handle )
	{  _workspace_do_end_projections( _ns_workspace( handle ) );  }


NS_PRIVATE eERROR_TYPE _workspace_run_projections( NsWorkspace *wr )
	{
	NsError error;

	_workspace_do_end_projections( wr );

	____projections_terminate = NS_FALSE;
	____projections_running   = NS_TRUE;

	ns_thread_construct( &____projections_thread );
	error = ns_thread_run( &____projections_thread, _workspace_run_projections_thread, wr );

	/* In the unlikely case that the thread didnt launch. */
	if( ns_is_error( error ) )
		____projections_running = NS_FALSE;

	return eNO_ERROR;
	}


#include "oglwin.h"
extern OpenGLWindow *____ogl;

eERROR_TYPE workspace_accept_roi( HWorkspace handle )
	{
	NsWorkspace *wr = _ns_workspace( handle );

	wr->saved_roi = wr->update_roi;
	_adjust_slice_viewer_due_to_roi_update( &wr->saved_roi );

	SendMessage( ____ogl->hWnd, WM_COMMAND, MAKEWPARAM( IDM_GL_SPLATS_REGENERATE, 0 ), 0 );

	return _workspace_run_projections( wr );
	}


const NsImage* workspace_thumbnail( HWorkspace handle, int display )
   {
   //NsWorkspace *wr;
   const NsImage *image = NULL;
/*
   wr = _ns_workspace( handle );

   switch( display )
      {
      case NS_XY:
         image = &wr->thumbnail_xy;
         break;

      case NS_ZY:
         image = &wr->thumbnail_zy;
         break;

      case NS_XZ:
         image = &wr->thumbnail_xz;
         break;

      default:
         ns_assert_not_reached();
      }
*/

   return image;
   }


const NsImage* workspace_volume( HWorkspace handle )
	{  return &( _ns_workspace( handle )->volume ); }


NsProcDb* workspace_pixel_proc_db( HWorkspace handle )
	{  return &( _ns_workspace( handle )->ppdb );  }

NsProcDb* workspace_io_proc_db( HWorkspace handle )
	{  return &( _ns_workspace( handle )->iodb );  }


NsColorDb* workspace_brush_db( HWorkspace handle )
   {  return &( _ns_workspace( handle )->brush_db );  }

NsColorDb* workspace_pen_db( HWorkspace handle )
   {  return &( _ns_workspace( handle )->pen_db );  }


extern nsboolean MakeIntegrationFileName( const nschar* prefix, const nschar* extension, nsboolean doPadNumber, 
                               nsint fileNumber, nsint maxFileNumber, nschar* fileName );






#include <std/nsio.h>

void ____ns_progress_update( nsfloat percent, NsIO *io )
   {  ns_progress_update( io->progress, percent );  }

nsint ____ns_progress_cancelled( NsIO *io )
   {  return ns_progress_cancelled( io->progress );  }






typedef struct _NsWorkspaceWriteImage
   {
	const NsWorkspace  *wr;
   const nschar       *prefix;
   const nschar       *extension;
   nsboolean           pad_number;
   nsboolean           as_series;
	nsint               bit_depth;
   }
   NsWorkspaceWriteImage;


NS_PRIVATE void _workspace_write_image_raw( void *dialog )
   {
	NsWorkspaceWriteImage  *s;
	nsint                  *not_used;
	NsProgress              nsprogress;
	Progress                progress;
	NsIO                    io;
	NsFile                  file;
	const NsWorkspace      *wr;
   const nschar           *prefix;
   const nschar           *extension;
   nsboolean               pad_number;
   nsboolean               as_series;
	nsint                   bit_depth;
	nssize                  zs, ze;
	nssize                  fs, fe;
	nssize                  width, height, length;
	//nssize                  channels_per_pixel;
	//nssize                  bits_per_pixel;
	//nssize                  luminance_bits;
	RawPixelType            pixel_type;
	NsPixelType             conv_pixel_type;
	nspointer               pixels;
	RawRegion               raw_region;
	NsPixelRegion           pixel_region;
	nschar                  file_name[ NS_PATH_SIZE ];


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &s, &not_used );
   ns_assert( NULL != s );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   ns_file_construct( &file );

   io.progress = &nsprogress;
   io.file     = &file;

	wr         = s->wr;
   prefix     = s->prefix;
   extension  = s->extension;
   pad_number = s->pad_number;
   as_series  = s->as_series;
	bit_depth  = s->bit_depth;

   width  = ns_image_width( &wr->volume );
   height = ns_image_height( &wr->volume );
   length = as_series ? 1 : ns_image_length( &wr->volume );

	pixel_type      = ns_pixel_type_to_raw_pixel_type( ns_image_pixel_type( &wr->volume ) );
	conv_pixel_type = NS_PIXEL_UNSUPPORTED;

	switch( bit_depth )
		{
		case 8:
			if( RAW_PIXEL_LUM12 == pixel_type || RAW_PIXEL_LUM16 == pixel_type )
				conv_pixel_type = NS_PIXEL_LUM_U8;
			break;

		case 12:
			if( RAW_PIXEL_LUM8 == pixel_type || RAW_PIXEL_LUM16 == pixel_type )
				conv_pixel_type = NS_PIXEL_LUM_U12;
			break;

		case 16:
			if( RAW_PIXEL_LUM8 == pixel_type || RAW_PIXEL_LUM12 == pixel_type )
				conv_pixel_type = NS_PIXEL_LUM_U16;
			break;

		default:
			ns_assert_not_reached();
		}

   //channels_per_pixel = 1;
   //bits_per_pixel     = ns_pixel_bits( ns_image_pixel_type( &wr->volume ) );
   //luminance_bits     = bits_per_pixel;

	pixel_region.x      = ( nssize )wr->saved_roi.C1.x;
	pixel_region.y      = ( nssize )wr->saved_roi.C1.y;
	pixel_region.z      = as_series ? 0 : ( nssize )wr->saved_roi.C1.z;
	pixel_region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	pixel_region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	pixel_region.length = as_series ? 1 : ( nssize )ns_cubei_length( &wr->saved_roi );

	raw_region.x      = pixel_region.x;
	raw_region.y      = pixel_region.y;
	raw_region.z      = pixel_region.z;
	raw_region.width  = pixel_region.width;
	raw_region.height = pixel_region.height;
	raw_region.length = pixel_region.length;

   ns_progress_update( &nsprogress, NS_PROGRESS_BEGIN );

	if( as_series )
		{
		ns_progress_set_title( &nsprogress, "Writing files..." );

		zs = ( nssize )wr->saved_roi.C1.z;
		ze = zs + ( nssize )ns_cubei_length( &wr->saved_roi );
		fs = 0;
		fe = ( nssize )ns_cubei_length( &wr->saved_roi );

		for( ; zs < ze; ++zs, ++fs )
			{
			if( ns_progress_cancelled( &nsprogress ) )
				goto _WORKSPACE_WRITE_IMAGE_RAW_EXIT;

			/*TEMP*/
			MakeIntegrationFileName( prefix, extension, pad_number, ( nsint )( fs + 1 ), ( nsint )fe, file_name );

			pixels = ns_image_pixels_slice( &wr->volume, zs );

			/*error*/raw_file_write_ex(
				&raw_region,
				file_name,
				NULL,
				0,
				pixel_type,
				width,
				height,
				length,
				pixels,
				ns_pixel_type_to_raw_pixel_type( conv_pixel_type ),
				NULL,
				NULL,
				&io
				);

			ns_progress_update( &nsprogress, 100.0f * ( nsfloat )fs / ( nsfloat )fe ); 
			}
      }
	else
		{
		//ns_progress_set_title( &nsprogress, "Writing file..." );
		ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s%s", prefix, extension );

		//pixels = ns_image_pixels( &wr->volume );

		/*error*//*raw_file_write(
			file_name,
			NULL,
			0,
			pixel_type,
			width,
			height,
			length,
			pixels,
			____ns_progress_cancelled,
			____ns_progress_update,
			&io
			);*/
		/*error*/ns_image_write_raw( &wr->volume, &pixel_region, file_name, conv_pixel_type, &nsprogress );
		}

	_WORKSPACE_WRITE_IMAGE_RAW_EXIT:

   ns_file_destruct( &file );

   ns_progress_update( &nsprogress, NS_PROGRESS_END );
   EndProgressDialog( dialog, eNO_ERROR );
   }


NS_PRIVATE void _workspace_write_image_tiff( void *dialog )
   {
	NsWorkspaceWriteImage  *s;
	nsint                  *not_used;
	NsProgress              nsprogress;
	Progress                progress;
	NsIO                    io;
	NsFile                  file;
	const NsWorkspace      *wr;
   const nschar           *prefix;
   const nschar           *extension;
   nsboolean               pad_number;
   nsboolean               as_series;
	nsint                   bit_depth;
	TiffFile                tiff;
	TiffRegion              tiff_region;
	NsPixelType             conv_pixel_type;
	NsPixelRegion           pixel_region;
	nssize                  zs, ze, fs, fe;
	nschar                  file_name[ NS_PATH_SIZE ];


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &s, &not_used );
   ns_assert( NULL != s );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   ns_file_construct( &file );

   io.progress = &nsprogress;
   io.file     = &file;

	wr         = s->wr;
   prefix     = s->prefix;
   extension  = s->extension;
   pad_number = s->pad_number;
   as_series  = s->as_series;
	bit_depth  = s->bit_depth;

	tiff_file_construct( &tiff, &io );

   tiff.width  = ns_image_width( &wr->volume );
   tiff.height = ns_image_height( &wr->volume );
   tiff.length = as_series ? 1 : ns_image_length( &wr->volume );

	ns_pixel_type_to_tiff_pixel_info( ns_image_pixel_type( &wr->volume ), &tiff.pixel_info );

	conv_pixel_type = NS_PIXEL_UNSUPPORTED;

	switch( bit_depth )
		{
		case 8:
			if( TIFF_PIXEL_LUM12 == tiff.pixel_info.pixel_type ||
				 TIFF_PIXEL_LUM16 == tiff.pixel_info.pixel_type   )
				conv_pixel_type = NS_PIXEL_LUM_U8;
			break;

		case 12:
			if( TIFF_PIXEL_LUM8 == tiff.pixel_info.pixel_type ||
				 TIFF_PIXEL_LUM16 == tiff.pixel_info.pixel_type   )
				conv_pixel_type = NS_PIXEL_LUM_U12;
			break;

		case 16:
			if( TIFF_PIXEL_LUM8 == tiff.pixel_info.pixel_type ||
				 TIFF_PIXEL_LUM12 == tiff.pixel_info.pixel_type   )
				conv_pixel_type = NS_PIXEL_LUM_U16;
			break;

		default:
			ns_assert_not_reached();
		}

	if( NS_PIXEL_UNSUPPORTED != conv_pixel_type )
		ns_pixel_type_to_tiff_pixel_info( conv_pixel_type, &tiff.conv_pixel_info );

	pixel_region.x      = ( nssize )wr->saved_roi.C1.x;
	pixel_region.y      = ( nssize )wr->saved_roi.C1.y;
	pixel_region.z      = as_series ? 0 : ( nssize )wr->saved_roi.C1.z;
	pixel_region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	pixel_region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	pixel_region.length = as_series ? 1 : ( nssize )ns_cubei_length( &wr->saved_roi );

	tiff_region.x      = pixel_region.x;
	tiff_region.y      = pixel_region.y;
	tiff_region.z      = pixel_region.z;
	tiff_region.width  = pixel_region.width;
	tiff_region.height = pixel_region.height;
	tiff_region.length = pixel_region.length;

   ns_progress_update( &nsprogress, NS_PROGRESS_BEGIN );

	if( as_series )
		{
		ns_progress_set_title( &nsprogress, "Writing files..." );

		zs = ( nssize )wr->saved_roi.C1.z;
		ze = zs + ( nssize )ns_cubei_length( &wr->saved_roi );
		fs = 0;
		fe = ( nssize )ns_cubei_length( &wr->saved_roi );

		for( ; zs < ze; ++zs, ++fs )
			{
			if( ns_progress_cancelled( &nsprogress ) )
				goto _WORKSPACE_WRITE_IMAGE_TIFF_EXIT;

			/*TEMP*/
			MakeIntegrationFileName( prefix, extension, pad_number, ( nsint )( fs + 1 ), ( nsint )fe, file_name );

			tiff.pixels = ns_image_pixels_slice( &wr->volume, zs );
			/*error*/tiff_file_write_ex( &tiff, &tiff_region, file_name, NULL, NULL, &io );

			ns_progress_update( &nsprogress, 100.0f * ( nsfloat )fs / ( nsfloat )fe ); 
			}
      }
	else
		{
		//ns_progress_set_title( &nsprogress, "Writing file..." );
		ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s%s", prefix, extension );

		/*error*/ns_image_write_tiff( &wr->volume, &pixel_region, file_name, conv_pixel_type, &nsprogress );

		//tiff.pixels = ns_image_pixels( &wr->volume );
		///*error*/tiff_file_write( &tiff, file_name, ____ns_progress_cancelled, ____ns_progress_update, &io );
		}

	_WORKSPACE_WRITE_IMAGE_TIFF_EXIT:

	tiff.pixels = NULL;
	tiff_file_destruct( &tiff, &io );

   ns_file_destruct( &file );

   ns_progress_update( &nsprogress, NS_PROGRESS_END );
   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE workspace_write_image
	(
	HWorkspace     handle,
	HWND           wnd,
	const nschar  *prefix,
	const nschar  *extension,
	nsboolean      pad_number,
	nsboolean      as_series,
	nsint          bit_depth
	)
	{
   NsWorkspaceWriteImage  s;
   nsint                  was_cancelled;
   eERROR_TYPE            error;


	s.wr         = _ns_workspace( handle );
   s.prefix     = prefix;
   s.extension  = extension;
   s.pad_number = pad_number;
   s.as_series  = as_series;
	s.bit_depth  = bit_depth;

   if( ns_ascii_streq( extension, ".tif" ) )
		error =
			ProgressDialog(
				wnd,
				NULL,
				_workspace_write_image_tiff,
				&s,
				NULL,
				&was_cancelled
				);
   else if( ns_ascii_streq( extension, ".raw" ) )
		error =
			ProgressDialog(
				wnd,
				NULL,
				_workspace_write_image_raw,
				&s,
				NULL,
				&was_cancelled
				);      
   else
      {  ns_assert_not_reached();  }

   if( ! was_cancelled && eNO_ERROR != error )
      return error;
 
   return eNO_ERROR;
	}






NS_PRIVATE eERROR_TYPE _workspace_write_projections_raw
   (
	const NsWorkspace *wr,
   const nschar   *prefix,
   const NsImage  *xy,
   const NsImage  *zy,
   const NsImage  *xz
   )
   {
   //RawPixelType pixel_type;
   //NsIO io;
   //NsFile file;
	NsPixelRegion region;
   nschar file_name[ NS_PATH_SIZE ];


   //ns_file_construct( &file );

   //io.progress = NULL;
   //io.file     = &file;

   //switch( ns_image_pixel_type( xy ) )
     // {
      //case NS_PIXEL_LUM_U8 : pixel_type = RAW_PIXEL_LUM8;  break;
      //case NS_PIXEL_LUM_U12: pixel_type = RAW_PIXEL_LUM12; break;
      //case NS_PIXEL_LUM_U16: pixel_type = RAW_PIXEL_LUM16; break;
      //default:
         //ns_assert_not_reached();
      //}


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_xy.raw", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.x;
	region.y      = ( nssize )wr->saved_roi.C1.y;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_raw( xy, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_zy.raw", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.z;
	region.y      = ( nssize )wr->saved_roi.C1.y;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_length( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_raw( zy, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_xz.raw", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.x;
	region.y      = ( nssize )wr->saved_roi.C1.z;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_length( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_raw( xz, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );



   //ns_file_destruct( &file );
   return eNO_ERROR;
   }


NS_PRIVATE eERROR_TYPE _workspace_write_projections_tiff
   (
	const NsWorkspace *wr,
   const nschar   *prefix,
   const NsImage  *xy,
   const NsImage  *zy,
   const NsImage  *xz
   )
   {
   //TiffFile  tiff;
   //NsIO      io;
   //NsFile    file;
	NsPixelRegion region;
   nschar    file_name[ NS_PATH_SIZE ];


   //ns_file_construct( &file );

   //io.progress = NULL;
   //io.file     = &file;

   //tiff_file_construct( &tiff, &io );
/*
   switch( ns_image_pixel_type( xy ) )
      {
      case NS_PIXEL_LUM_U8 :
         tiff.pixel_type         = TIFF_PIXEL_LUM8;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM8_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM8_NUM_BITS;
         break;

      case NS_PIXEL_LUM_U12:
         tiff.pixel_type         = TIFF_PIXEL_LUM12;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM16_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM12_NUM_BITS;
         break;

      case NS_PIXEL_LUM_U16:
         tiff.pixel_type         = TIFF_PIXEL_LUM16;
         tiff.channels_per_pixel = TIFF_PIXEL_LUM_NUM_CHANNELS;
         tiff.bits_per_pixel     = TIFF_PIXEL_LUM16_NUM_BITS;
         tiff.luminance_bits     = TIFF_PIXEL_LUM16_NUM_BITS;
         break;

      default:
         ns_assert_not_reached();
      }*/


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_xy.tif", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.x;
	region.y      = ( nssize )wr->saved_roi.C1.y;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_tiff( xy, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_zy.tif", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.z;
	region.y      = ( nssize )wr->saved_roi.C1.y;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_length( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_height( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_tiff( zy, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );


   ns_snprint( file_name, NS_ARRAY_LENGTH( file_name ), "%s_xz.tif", prefix );

	region.x      = ( nssize )wr->saved_roi.C1.x;
	region.y      = ( nssize )wr->saved_roi.C1.z;
	region.z      = 0;
	region.width  = ( nssize )ns_cubei_width( &wr->saved_roi );
	region.height = ( nssize )ns_cubei_length( &wr->saved_roi );
	region.length = 1;

	/*error*/ns_image_write_tiff( xz, &region, file_name, NS_PIXEL_UNSUPPORTED, NULL );


	//tiff.pixels = NULL;
	//tiff_file_destruct( &tiff, &io );

   //ns_file_destruct( &file );
   return eNO_ERROR;
   }


eERROR_TYPE _workspace_do_write_projections
	(
	const NsWorkspace  *wr,
	const nschar       *prefix,
	const nschar       *extension
	)
	{
   NsImage image_xy, image_zy, image_xz;
	const NsImage *proj_xy, *proj_zy, *proj_xz;
	eERROR_TYPE error;


   error = eNO_ERROR;

   ns_image_construct( &image_xy );
   ns_image_construct( &image_zy );
   ns_image_construct( &image_xz );

   /* Have to remove the scan-alignment of the projections! */

	if( 1 != ns_image_row_align( &wr->proj_xy ) )
		{
		/*error*/ns_image_convert(
			&wr->proj_xy,
			ns_image_pixel_type( &wr->proj_xy ),
			1,
			&image_xy,
			NULL
			);

		proj_xy = &image_xy;
		}
	else
		proj_xy = &wr->proj_xy;

	if( 1 != ns_image_row_align( &wr->proj_zy ) )
		{
		/*error*/ns_image_convert(
			&wr->proj_zy,
			ns_image_pixel_type( &wr->proj_zy ),
			1,
			&image_zy,
			NULL
			);

		proj_zy = &image_zy;
		}
	else
		proj_zy = &wr->proj_zy;

	if( 1 != ns_image_row_align( &wr->proj_xz ) )
		{
		/*error*/ns_image_convert(
			&wr->proj_xz,
			ns_image_pixel_type( &wr->proj_xz ),
			1,
			&image_xz,
			NULL
			);

		proj_xz = &image_xz;
		}
	else
		proj_xz = &wr->proj_xz;

	ns_assert( ns_image_pixel_type( proj_xy ) == ns_image_pixel_type( proj_zy ) );
	ns_assert( ns_image_pixel_type( proj_xy ) == ns_image_pixel_type( proj_xz ) );
	ns_assert( ns_image_pixel_type( proj_zy ) == ns_image_pixel_type( proj_xz ) );

	ns_image_set_io_proc_db( ( NsImage* )proj_xy, ( NsProcDb* )&wr->iodb );
	ns_image_set_io_proc_db( ( NsImage* )proj_zy, ( NsProcDb* )&wr->iodb );
	ns_image_set_io_proc_db( ( NsImage* )proj_xz, ( NsProcDb* )&wr->iodb );

   if( ns_ascii_streq( extension, ".tif" ) )
      error = _workspace_write_projections_tiff( wr, prefix, proj_xy, proj_zy, proj_xz );
   else if( ns_ascii_streq( extension, ".raw" ) )
      error = _workspace_write_projections_raw( wr, prefix, proj_xy, proj_zy, proj_xz );

   ns_image_destruct( &image_xy );
   ns_image_destruct( &image_zy );
   ns_image_destruct( &image_xz );

   return error;
	}


eERROR_TYPE workspace_write_projections( HWorkspace handle, const nschar *prefix, const nschar *extension )
   {  return _workspace_do_write_projections( _ns_workspace( handle ), prefix, extension );  }






nsdouble workspace_get_average_intensity( nsuint handle )
   {  return _ns_workspace( handle )->average_intensity;  }


void workspace_set_average_intensity( nsuint handle, nsdouble average_intensity )
	{  _ns_workspace( handle )->average_intensity = average_intensity;  }



nsfloat workspace_get_grafting_first_threshold( nsuint handle )
	{  return _ns_workspace( handle )->grafting_first_threshold;  }


void workspace_set_grafting_first_threshold( nsuint handle, nsfloat threshold )
	{  _ns_workspace( handle )->grafting_first_threshold = threshold;  }


nsfloat workspace_get_grafting_first_contrast( nsuint handle )
	{  return _ns_workspace( handle )->grafting_first_contrast;  }


void workspace_set_grafting_first_contrast( nsuint handle, nsfloat contrast )
	{  _ns_workspace( handle )->grafting_first_contrast = contrast;  }




nsulong workspace_get_max_intensity( nsuint handle )
	{  return _ns_workspace( handle )->max_intensity;  }


NsSampler* workspace_sampler( nsuint handle )
   { return &( _ns_workspace( handle )->sampler ); }

const NsList* workspace_records( nsuint handle )
   { return &( _ns_workspace( handle )->records ); }


void GetWorkspaceSeed( const HWorkspace handle, Vector3i *seed )
   {
   NsWorkspace *wr = _ns_workspace( handle );

   seed->x = ns_settings_neurite_seed( &wr->settings )->x;
   seed->y = ns_settings_neurite_seed( &wr->settings )->y;
   seed->z = ns_settings_neurite_seed( &wr->settings )->z;
   }

void SetWorkspaceSeed( const HWorkspace handle, const Vector3i *seed )
   {
   NsWorkspace *wr = _ns_workspace( handle );

   ns_settings_neurite_seed( &wr->settings )->x = ( nsint )seed->x;
   ns_settings_neurite_seed( &wr->settings )->y = ( nsint )seed->y;
   ns_settings_neurite_seed( &wr->settings )->z = ( nsint )seed->z;
   }



void workspace_get_image_dimensions( nsuint handle, NsVector3f *D )
   {
	NsWorkspace *wr;
   const NsVoxelInfo *voxel_info;

	voxel_info = workspace_get_voxel_info( handle );

	wr = _ns_workspace( handle );

   D->x = ns_to_voxel_space_component_ex( ( nsfloat )ns_image_width( &wr->volume ),  voxel_info, NS_COMPONENT_X );
   D->y = ns_to_voxel_space_component_ex( ( nsfloat )ns_image_height( &wr->volume ), voxel_info, NS_COMPONENT_Y );
   D->z = ns_to_voxel_space_component_ex( ( nsfloat )ns_image_length( &wr->volume ), voxel_info, NS_COMPONENT_Z );
   }



void workspace_clear_grafter_voxels( nsuint handle )
	{
   NsWorkspace *wr = _ns_workspace( handle );

	ns_model_grafter_clear_voxels( wr->filteredTree );
	ns_model_grafter_clear_voxels( wr->rawTree );
	}


void workspace_clear_model( nsuint handle )
   {
   NsWorkspace *wr = _ns_workspace( handle );

   ns_model_clear( wr->filteredTree );
   ns_model_clear( wr->rawTree );

   //ns_voxel_table_clear( &wr->voxel_table );

   //wr->setting_flags = NS_SETTINGS_ALL;
   }


void _set_workspace_handle( NsWorkspace *wr, HWorkspace *handle )
	{
   ns_assert( s_NumWorkspaces < _mMAX_WORKSPACES );
   s_Workspaces[ s_NumWorkspaces ] = wr;
   ++s_NumWorkspaces;
   *handle = s_NumWorkspaces;
	}


void _unset_workspace_handle( NsWorkspace *wr, HWorkspace *handle )
	{
	--s_NumWorkspaces;
	*handle = 0;
	}





/*
void _workspace_tile_finalize_func( nspointer object )
	{
	if( NULL != object )
		{
		DestroyMemoryGraphics( object );
		ns_free( object );
		}
	}
*/

NsError _workspace_alloc_tile_dc( NsWorkspace *wr, NsTile *tile )
	{
	/*
	if( NULL == tile->object )
		{
		if( NULL == ( tile->object = ns_calloc( 1, sizeof( MemoryGraphics ) ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

		ns_assert( 1 == ns_image_length( ns_tile_image( tile ) ) );

		if( eNO_ERROR !=
				_DoBuildWorkspaceResourceGraphics(
					wr,
					tile->object,
					ns_tile_image( tile ),
					wr->hWnd
               ) )
			{
			ns_free( tile->object );
			tile->object = NULL;

			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			}
		}

	tile->finalize_func = _workspace_tile_finalize_func;
	*/
	return ns_no_error();
	}


NsError _workspace_tile_mgr_create( const NsTileMgr *mgr, NsTile *tile, NsWorkspace *wr )
	{  return _workspace_alloc_tile_dc( wr, tile );  }


NS_PRIVATE NsError _workspace_allocate_projections( NsWorkspace *wr )
	{
	NsError error;

   /* IMPORTANT: For Win32, each scan line of the image must be aligned to LONG boundaries!!! */

	if( NS_FAILURE(
			ns_image_create(
				&wr->proj_xy,
				NS_PIXEL_LUM_U8,
				ns_image_width( &wr->volume ),
				ns_image_height( &wr->volume ),
				1,
				_IMAGE_ROW_ALIGN
				),
			error ) )
		return error;

	if( NS_FAILURE(
			ns_image_create(
				&wr->proj_zy,
				NS_PIXEL_LUM_U8,
				ns_image_length( &wr->volume ),
				ns_image_height( &wr->volume ),
				1,
				_IMAGE_ROW_ALIGN
				),
			error ) )
		return error;

	if( NS_FAILURE(
			ns_image_create(
				&wr->proj_xz,
				NS_PIXEL_LUM_U8,
				ns_image_width( &wr->volume ),
				ns_image_length( &wr->volume ),
				1,
				_IMAGE_ROW_ALIGN
				),
			error ) )
		return error;

	/*
	if( NS_FAILURE(
			ns_image_create(
				&wr->flipped_xz,
				NS_PIXEL_LUM_U8,
				ns_image_width( &wr->volume ),
				ns_image_length( &wr->volume ),
				1,
				_IMAGE_ROW_ALIGN
				),
			error ) )
		return error;
	*/

   ns_sampler_init(
      &wr->sampler,
      ns_settings_voxel_info( &wr->settings ),
      &wr->volume,
      &wr->proj_xy,
      &wr->proj_zy,
      &wr->proj_xz
      );

	/*if( NS_FAILURE(
			ns_tile_mgr_create(
				&wr->tile_mgr_xy,
				_TILES_LOCKABLE,
				&wr->proj_xy,
				NS_FALSE,
				____tile_size, ____tile_size, ____tile_size,
				_workspace_tile_mgr_create,
				wr
				),
			error ) )
		return error;*/

	/*if( NS_FAILURE(
			ns_tile_mgr_create(
				&wr->tile_mgr_zy,
				_TILES_LOCKABLE,
				&wr->proj_zy,
				NS_FALSE,
				____tile_size, ____tile_size, ____tile_size,
				_workspace_tile_mgr_create,
				wr
				),
			error ) )
		return error;*/

	/*if( NS_FAILURE(
			ns_tile_mgr_create(
				&wr->tile_mgr_xz,
				_TILES_LOCKABLE,
				&wr->proj_xz,
				NS_FALSE,
				____tile_size, ____tile_size, ____tile_size,
				_workspace_tile_mgr_create,
				wr
				),
			error ) )
		return error;*/

/*
ns_println( "# XY tiles = %lu, %lu, %lu",
	ns_tile_mgr_num_tiles_x( &wr->tile_mgr_xy ),
	ns_tile_mgr_num_tiles_y( &wr->tile_mgr_xy ),
	ns_tile_mgr_num_tiles_z( &wr->tile_mgr_xy )
	);

ns_println( "# ZY tiles = %lu, %lu, %lu",
	ns_tile_mgr_num_tiles_x( &wr->tile_mgr_zy ),
	ns_tile_mgr_num_tiles_y( &wr->tile_mgr_zy ),
	ns_tile_mgr_num_tiles_z( &wr->tile_mgr_zy )
	);

ns_println( "# XZ tiles = %lu, %lu, %lu",
	ns_tile_mgr_num_tiles_x( &wr->tile_mgr_xz ),
	ns_tile_mgr_num_tiles_y( &wr->tile_mgr_xz ),
	ns_tile_mgr_num_tiles_z( &wr->tile_mgr_xz )
	);
*/

	return ns_no_error();
	}






NS_PRIVATE NsError _workspace_do_build_slice_view( NsWorkspace *wr, nssize z )
   {
	NsImage  slice, converted, *src_image, *dest_image;
	NsError  error;


	ns_assert( z < ns_image_length( &wr->volume ) );

	error = ns_no_error();

	ns_image_construct( &slice );
	ns_image_construct( &converted );

	ns_image_set_pixel_proc_db( &slice, &wr->ppdb );
	ns_image_set_pixel_proc_db( &converted, &wr->ppdb );

	ns_image_buffer_and_free(
		&slice,
		ns_image_pixel_type( &wr->volume ),
		ns_image_width( &wr->volume ),
		ns_image_height( &wr->volume ),
		1,
		ns_image_row_align( &wr->volume ),
		ns_image_pixels_slice( &wr->volume, z )
		);

	src_image  = &slice;
	dest_image = &wr->slice_xy;

	if( NS_PIXEL_LUM_U8 != ns_image_pixel_type( src_image ) )
		{
		/*if( NS_PALETTE_INTENSITY_MASK != wr->paletteType )
			{
			if( NS_FAILURE( ns_image_convert( src_image, NS_PIXEL_LUM_U8, 1, &converted, NULL ), error ) )
				goto _BUILD_SLICE_VIEW_EXIT;
			}
		else
			{*/
			if( NS_FAILURE(
					ns_image_convert_for_intensity_mask_palette(
						src_image,
						NS_PIXEL_LUM_U8,
						1,
						&converted,
						NULL
						),
					error ) )
				goto _BUILD_SLICE_VIEW_EXIT;
		//	}

		src_image = &converted;
		}

	ns_image2d_copy_ex( dest_image, src_image );

	_BUILD_SLICE_VIEW_EXIT:

	ns_image_release( &slice );

	ns_image_destruct( &slice );
	ns_image_destruct( &converted );

	return error;
   }


NsError workspace_build_slice_view( HWorkspace handle, nssize z )
	{  return _workspace_do_build_slice_view( _ns_workspace( handle ), z );  }


NS_PRIVATE void _workspace_do_finalize_slice_view( NsWorkspace *wr )
	{
	ns_tile_mgr_clear( &wr->tile_mgr_slice );
	ns_image_clear( &wr->slice_xy );
	}


NS_PRIVATE NsError _workspace_do_init_slice_view( NsWorkspace *wr )
	{
	NsError error;

	ns_assert( ns_image_width( &wr->volume ) == ns_image_width( &wr->proj_xy ) );
	ns_assert( ns_image_height( &wr->volume ) == ns_image_height( &wr->proj_xy ) );
	ns_assert( 1 == ns_image_length( &wr->proj_xy ) );
	ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( &wr->proj_xy ) );

	//wr->bits_per_pixel = ns_pixel_bits( ns_image_pixel_type( &wr->volume ) );

	if( NS_FAILURE(
			ns_image_create(
				&wr->slice_xy,
				NS_PIXEL_LUM_U8,
				ns_image_width( &wr->proj_xy ),
				ns_image_height( &wr->proj_xy ),
				1,
				ns_image_row_align( &wr->proj_xy )
				),
			error ) )
		{
		_workspace_do_finalize_slice_view( wr );
		return error;
		}

/*
	if( NS_FAILURE(
			ns_tile_mgr_create(
				&wr->tile_mgr_slice,
				_TILES_LOCKABLE,
				&wr->slice_xy,
				NS_FALSE,
				____tile_size, ____tile_size, ____tile_size,
				_workspace_tile_mgr_create,
				wr
				),
			error ) )
		{
		_workspace_do_finalize_slice_view( wr );
		return error;
		}

ns_println( "# slice tiles = %lu, %lu, %lu",
	ns_tile_mgr_num_tiles_x( &wr->tile_mgr_slice ),
	ns_tile_mgr_num_tiles_y( &wr->tile_mgr_slice ),
	ns_tile_mgr_num_tiles_z( &wr->tile_mgr_slice )
	);
*/

	return ns_no_error();
	}


NsError workspace_init_slice_view( HWorkspace handle )
	{  return _workspace_do_init_slice_view( _ns_workspace( handle ) );  }


void workspace_finalize_slice_view( HWorkspace handle )
	{  _workspace_do_finalize_slice_view( _ns_workspace( handle ) );  }



void _workspace_reset_regions_of_interest( NsWorkspace *wr )
	{
	ns_vector3i_zero( &wr->update_roi.C1 );
	
	ns_vector3i(
		&wr->update_roi.C2,
		( ( nsint )ns_image_width( &wr->volume ) ) - 1,
		( ( nsint )ns_image_height( &wr->volume ) ) - 1,
		( ( nsint )ns_image_length( &wr->volume ) ) - 1
		);

	ns_assert( 0 <= wr->update_roi.C2.x );
	ns_assert( 0 <= wr->update_roi.C2.y );
	ns_assert( 0 <= wr->update_roi.C2.z );

	wr->saved_roi = wr->update_roi;
	}


void BuildWorkspace( void *dialog )
   {
   WorkspaceArgs      *args;
   HWorkspace         *handle;
   NsWorkspace  *wr;
	NsError error;
   //Volume              volume;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &args, &handle );

   ns_assert( NULL != args->path );
   ns_assert( NULL != handle );

   if( NULL == ( wr = ns_malloc( sizeof( NsWorkspace ) ) ) )
      {
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   _ConstructWorkspaceResource( wr, args->path );
   //_ConstructVolume( &volume );

	_set_workspace_handle( wr, handle );

   if( args->use_path )
      {
		ns_print( "Reading 3D file... " );

      if( eNO_ERROR != _ReadVolume_3D( wr, &wr->volume, wr->path, dialog ) )
         {
			ns_println( "error" );

			_unset_workspace_handle( wr, handle );
         _DestructWorkspaceResource( wr );

         EndProgressDialog( dialog, g_Error );
         return;
         }
		else
			ns_println( "OK" );
      }
   else
      {
		ns_print( "Reading 2D sequence... " );

      if( eNO_ERROR != _ReadVolume_2D_sequence( wr, &wr->volume, args->dir, dialog ) )
         {
			ns_println( "error" );

			_unset_workspace_handle( wr, handle );
         _DestructWorkspaceResource( wr );

         EndProgressDialog( dialog, g_Error );
         return;
         }
		else
			ns_println( "OK" );
      }

   if( IsProgressCancelled() )
      {
		_unset_workspace_handle( wr, handle );
      _DestructWorkspaceResource( wr );

      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

	_set_optimal_tile_size(
		( nsdouble )ns_image_width( &wr->volume ),
		( nsdouble )ns_image_height( &wr->volume ),
		( nsdouble )ns_image_length( &wr->volume )
		);

   wr->hWnd = args->window;

	if( NULL != wr->hWnd )
		{
		if( NS_FAILURE( _workspace_allocate_projections( wr ), error ) )
			{
			_unset_workspace_handle( wr, handle );
			_DestructWorkspaceResource( wr );

			EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
			return;
			}

		//if( eNO_ERROR != _workspace_allocate_graphics( wr, args->window ) )
		//	{
		//	_unset_workspace_handle( wr, handle );
		//	_DestructWorkspaceResource( wr );

		//	EndProgressDialog( dialog, g_Error );
		//	return;
		//	}
		}

	_workspace_reset_regions_of_interest( wr );

	//ns_println( "created volume.pixels = %p", volume.pixels );

   //____volume_pixels = volume.pixels;
   //____volume_width = volume.width;
   //____volume_height = volume.height;
   //____volume_length = volume.length;
   //____volume_bpp    = volume.bitsPerPixel;

	_set_display_window_workspace_handle();
	//_fit_display_window_projection_xy();

	//ns_assert( ! ____workspace_has_tiles );
	//____workspace_has_tiles = NS_TRUE;

	if( NULL != wr->hWnd )
		if( eNO_ERROR != _workspace_run_projections( wr ) )
			{
			_unset_workspace_handle( wr, handle );
			_DestructWorkspaceResource( wr );

			EndProgressDialog( dialog, g_Error );
			return;
			}

/*
   if( IsProgressCancelled() )
      {
		_unset_workspace_handle( wr, handle );
      _DestructWorkspaceResource( wr );

      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }
*/

	/*
   ns_assert( s_NumWorkspaces < _mMAX_WORKSPACES );
   s_Workspaces[ s_NumWorkspaces ] = wr;
   ++s_NumWorkspaces;
   *handle = s_NumWorkspaces;
	*/

   //RedrawWorkspaceDisplayGraphics( *handle );
   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing
   (
   HWND              hWnd,
   const HWorkspace  handle,
   void              ( *func )( void* )
   )
   {
   NsWorkspace *wr;
   eERROR_TYPE        error;
   nsint                wasCancelled;


   wr = _ns_workspace( handle );

   //LockWorkspace( handle );

   error = ProgressDialog( hWnd,
                           NULL,
                           func,
                           wr,
                           NULL,
                           &wasCancelled
                         );

   //UnlockWorkspace( handle );

   if( ! wasCancelled && eNO_ERROR != error )
      return error;
 
   //RedrawWorkspaceDisplayGraphics( handle );
   return eNO_ERROR;
   }


/*
void _write_brightness_contrast( nsdouble brightness, nsdouble contrast )
   {
   NsConfigDb db;

   if( NS_FAILURE( ns_config_db_construct( &db );
   ns_config_db_read( &db, ____config_file, NULL );

   ns_config_db_set_double( &db, "brightness_contrast", "brightness", brightness );
   ns_config_db_set_double( &db, "brightness_contrast", "contrast", contrast );

   ns_config_db_write( &db, ____config_file );
   ns_config_db_destruct( &db );
   }
*/


/*
[brightness_contrast]
brightness=0.000000
contrast=0.450000

void _read_brightness_contrast( nsdouble *brightness, nsdouble *contrast )
   {
   NsConfigDb  db;
	NsError     error;


	*brightness = 0.0;
	*contrast   = 0.0;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

	if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

	*brightness = ns_config_db_get_double( &db, "brightness_contrast", "brightness" );
	*contrast   = ns_config_db_get_double( &db, "brightness_contrast", "contrast" );

	ns_config_db_destruct( &db );
   }
*/

#include <image/nspixels-brightnesscontrast.h>
#include "nsbrightnesscontrast.inl"

void _do_image_processing_brightness_contrast( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
  // nsdouble          brightness, contrast;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	/*TEMP*/
	ns_println( "brightness = " NS_FMT_DOUBLE, wr->brightness );
	ns_println( "contrast   = " NS_FMT_DOUBLE, wr->contrast );

   //_hack_image( &image, &wr->ppdb, ____volume_pixels,
   //   ____volume_width, ____volume_height, ____volume_length,
   //   ____volume_bpp );

   //_read_brightness_contrast( &brightness, &contrast );

   record = _workspace_create_record( wr, ns_pixel_proc_brightness_contrast()->title, NULL );

   ns_image_brightness_contrast(
		&wr->volume,
		_ns_pixel_region_from_roi( &region, &wr->saved_roi ),
		wr->brightness,
		wr->contrast,
		&nsprogress,
		record
		);

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   //_write_brightness_contrast( brightness, contrast );

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_brightness_contrast( HWND hWnd, const HWorkspace handle )
   {
   NsWorkspace *wr = _ns_workspace( handle );

	wr->brightness = wr->contrast = 0.0;

	if( ns_brightness_contrast_dialog( g_Instance, hWnd, &wr->brightness, &wr->contrast ) )
		if( ! NS_DOUBLE_EQUAL( wr->brightness, 0.0 ) || ! NS_DOUBLE_EQUAL( wr->contrast, 0.0 ) )
			return image_processing( hWnd, handle, _do_image_processing_brightness_contrast );	

	return eNO_ERROR;

	//return image_processing( hWnd, handle, _do_image_processing_brightness_contrast );
	}




void _read_resize( NsWorkspace *wr, nssize *width, nssize *height, nssize *length, nsint *interp_type )
   {
   NsConfigDb  db;
	NsError     error;


	*width  = ns_image_width( &wr->volume );
	*height = ns_image_height( &wr->volume );
	*length = ns_image_length( &wr->volume );

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   *width  = ns_config_db_get_int( &db, "resize", "width" );
   *height = ns_config_db_get_int( &db, "resize", "height" );
   *length = ns_config_db_get_int( &db, "resize", "length" );

   *interp_type = ns_config_db_get_int( &db, "resize", "interp_type" );

   ns_config_db_destruct( &db );
   }


#include <image/nspixels-resize.h>

void _workspace_reset_on_volume_size_change( NsWorkspace *wr );

void _do_image_processing_resize( void *dialog )
   {
	NsProgress        nsprogress;
	Progress          progress;
	NsWorkspace      *wr;
	NsClosureRecord  *record;
	nsint            *not_used;
	//nssize            prev_length;
	//nssize            curr_length;
	nssize            width, height, length;
	nsint             interp_type;
	NsImage           resized;
	//NsLerpf           lerp;
	NsError           error;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &not_used );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   ns_image_construct( &resized );
   ns_image_set_pixel_proc_db( &resized, &wr->ppdb );

   _read_resize( wr, &width, &height, &length, &interp_type );


	/* IMPORTANT: Have to set these so _workspace_reset_on_volume_size_change()
		will work properly to set a new voxel size. */
	wr->subsample_scale.x = ( ( nsdouble )width  / ( nsdouble )ns_image_width( &wr->volume ) )  * 100.0;
	wr->subsample_scale.y = ( ( nsdouble )height / ( nsdouble )ns_image_height( &wr->volume ) ) * 100.0;
	wr->subsample_scale.z = ( ( nsdouble )length / ( nsdouble )ns_image_length( &wr->volume ) ) * 100.0;


   record = _workspace_create_record( wr, ns_pixel_proc_resize()->title, NULL );

   if( NS_FAILURE( ns_image_resize(
                     &wr->volume,
                     interp_type,
                     width,
                     height,
                     length,
                     1,
                     &resized,
                     &nsprogress,
                     record
                     ),
                     error ) )
      {
      _workspace_remove_last_record( wr, record );
		ns_image_destruct( &resized );
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      ns_image_destruct( &resized );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

	//prev_length = ns_image_length( &wr->volume );

	ns_image_clear( &wr->volume );

	ns_image_buffer_and_free(
		&wr->volume,
		ns_image_pixel_type( &resized ),
		ns_image_width( &resized ),
		ns_image_height( &resized ),
		ns_image_length( &resized ),
		ns_image_row_align( &resized ),
		ns_image_pixels( &resized )
		);

	ns_image_release( &resized );
	ns_image_destruct( &resized );

	ns_image_clear( &wr->proj_xy );
	ns_image_clear( &wr->proj_zy );
	ns_image_clear( &wr->proj_xz );
	ns_image_clear( &wr->slice_xy );

	//DestroyMemoryGraphics( &wr->gc_xy );
	//DestroyMemoryGraphics( &wr->gc_zy );
	//DestroyMemoryGraphics( &wr->gc_xz );
	//wr->gc_xy.hDC = NULL;
	//wr->gc_zy.hDC = NULL;
	//wr->gc_xz.hDC = NULL;

//	if( NS_FAILURE( _workspace_allocate_projections( wr ), error ) )
//		{
  //    EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
    //  return;
		//}

	/* Reset the seed back to zeroes since it might be invalid. */
//	ns_vector3i_zero( ns_settings_neurite_seed( &wr->settings ) );	

//	if( ____xy_slice_enabled )
//		{
		/* IMPORTANT: The index might be invalid now since we resized the volume. */

//		curr_length = ns_image_length( &wr->volume );

//		ns_lerpf_init( &lerp, 0.0f, 0.0f, ( nsfloat )( prev_length - 1 ), ( nsfloat )( curr_length - 1 ) );
//		____xy_slice_index = ( nssize )ns_lerpf( &lerp, ( nsfloat )____xy_slice_index );

//		if( curr_length <= ____xy_slice_index )
//			____xy_slice_index = curr_length - 1;

//		if( NS_FAILURE( _workspace_do_init_slice_view( wr ), error ) )
//			{
//			EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
//			return;
//			}
//		}

//   if( eNO_ERROR != _workspace_allocate_graphics( wr, wr->hWnd ) )
  //    {
    //  EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      //return;
      //}

//	_workspace_reset_regions_of_interest( wr );

  // if( eNO_ERROR != _workspace_run_projections( wr ) )
    //  {
      //EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      //return;
      //}

	if( NS_FAILURE( _workspace_allocate_projections( wr ), error ) )
		{
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
		}

	_workspace_reset_on_volume_size_change( wr );

	if( ____xy_slice_enabled )
		if( NS_FAILURE( _workspace_do_init_slice_view( wr ), error ) )
			{
			EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
			return;
			}

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   _do_redraw_workspace_display_graphics( wr );
   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_resize( HWND hWnd, const HWorkspace handle )
   {
	workspace_end_projections( handle );
	return image_processing( hWnd, handle, _do_image_processing_resize );
	}





void _read_noise( nsdouble *level, nssize *iterations )
   {
   NsConfigDb  db;
	NsError     error;


	*level      = 0.0;
	*iterations = 0;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   *level      = ns_config_db_get_double( &db, "noise", "level" );
   *iterations = ( nssize )ns_config_db_get_int( &db, "noise", "iterations" );

   ns_config_db_destruct( &db );
   }


#include <image/nspixels-noise.h>

void _do_image_processing_noise( void *dialog )
   {
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
   nsdouble          level;
   nssize            iterations;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   _read_noise( &level, &iterations );

   record = _workspace_create_record( wr, ns_pixel_proc_noise()->title, NULL );

   ns_image_noise(
      &wr->volume,
      _ns_pixel_region_from_roi( &region, &wr->saved_roi ),
      level,
      iterations,
      wr->average_intensity,
      &wr->signal_to_noise_ratio,
      &nsprogress,
      record
      );

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   /*TEMP*/
   ns_println( "signal_to_noise_ratio = " NS_FMT_DOUBLE, wr->signal_to_noise_ratio );

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_noise( HWND wnd, const HWorkspace handle )
   {  return image_processing( wnd, handle, _do_image_processing_noise );  }






#include <image/nspixels-flip.h>


void _do_image_processing_flip( void *dialog, nsenum which )
   {
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   record = _workspace_create_record( wr, ns_pixel_proc_flip()->title, NULL );

//____ns_value_print_invoke = NS_TRUE;

	switch( which )
		{
		case NS_PIXEL_PROC_FLIP_VERTICAL:
			ns_image_flip_vertical( &wr->volume, &nsprogress, record );
			break;
	
		case NS_PIXEL_PROC_FLIP_HORIZONTAL:
			ns_image_flip_horizontal( &wr->volume, &nsprogress, record );
			break;

		case NS_PIXEL_PROC_FLIP_OPTICAL_AXIS:
			ns_image_flip_optical_axis( &wr->volume, &nsprogress, record );
			break;

		default:
			ns_assert_not_reached();
		}

//____ns_value_print_invoke = NS_FALSE;

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


void _do_image_processing_flip_vertical( void *dialog )
   {  _do_image_processing_flip( dialog, NS_PIXEL_PROC_FLIP_VERTICAL );  }

void _do_image_processing_flip_horizontal( void *dialog )
   {  _do_image_processing_flip( dialog, NS_PIXEL_PROC_FLIP_HORIZONTAL );  }

void _do_image_processing_flip_optical_axis( void *dialog )
   {  _do_image_processing_flip( dialog, NS_PIXEL_PROC_FLIP_OPTICAL_AXIS );  }


eERROR_TYPE image_processing_flip_vertical( HWND wnd, const HWorkspace handle )
   {  return image_processing( wnd, handle, _do_image_processing_flip_vertical );  }

eERROR_TYPE image_processing_flip_horizontal( HWND wnd, const HWorkspace handle )
   {  return image_processing( wnd, handle, _do_image_processing_flip_horizontal );  }

eERROR_TYPE image_processing_flip_optical_axis( HWND wnd, const HWorkspace handle )
   {  return image_processing( wnd, handle, _do_image_processing_flip_optical_axis );  }


extern void display_window_center_image( void );
extern void _show_seed_on_status_bar( void );
extern void _do_show_roi_on_status_bar( nsuint );
extern nsint s_CurrentMouseMode;
extern nsuint *____active_workspace;
extern nsboolean ____alert_user_about_splats;


void _workspace_reset_on_volume_size_change( NsWorkspace *wr )
	{
	NsVoxelInfo      *voxel_info;
	nsfloat           xy_slice_index;
	NsVector3f        seed;
	NsCubef           update_roi;
	NsCubef           saved_roi;
	NsVector3i        max;


	voxel_info = ns_settings_voxel_info( &wr->settings );

	xy_slice_index = NS_TO_VOXEL_SPACE( ____xy_slice_index, ns_voxel_info_size_z( voxel_info ) );

	seed.x = NS_TO_VOXEL_SPACE( ns_settings_neurite_seed_x( &wr->settings ), ns_voxel_info_size_x( voxel_info ) );
	seed.y = NS_TO_VOXEL_SPACE( ns_settings_neurite_seed_y( &wr->settings ), ns_voxel_info_size_y( voxel_info ) );
	seed.z = NS_TO_VOXEL_SPACE( ns_settings_neurite_seed_z( &wr->settings ), ns_voxel_info_size_z( voxel_info ) );

	ns_to_voxel_space( &wr->update_roi.C1, &update_roi.C1, voxel_info );
	ns_to_voxel_space( &wr->update_roi.C2, &update_roi.C2, voxel_info );
	ns_to_voxel_space( &wr->saved_roi.C1, &saved_roi.C1, voxel_info );
	ns_to_voxel_space( &wr->saved_roi.C2, &saved_roi.C2, voxel_info );

	ns_voxel_info(
		voxel_info,
		( nsfloat )( ns_voxel_info_size_x( voxel_info ) / ( wr->subsample_scale.x / 100.0 ) ),
		( nsfloat )( ns_voxel_info_size_y( voxel_info ) / ( wr->subsample_scale.y / 100.0 ) ),
		( nsfloat )( ns_voxel_info_size_z( voxel_info ) / ( wr->subsample_scale.z / 100.0 ) )
		);

	display_window_center_image();

	max.x = ( nsint )ns_image_width( &wr->volume ) - 1;
	max.y = ( nsint )ns_image_height( &wr->volume ) - 1;
	max.z = ( nsint )ns_image_length( &wr->volume ) - 1;

	ns_to_image_space( &update_roi.C1, &wr->update_roi.C1, voxel_info );
	ns_to_image_space( &update_roi.C2, &wr->update_roi.C2, voxel_info );
	ns_to_image_space( &saved_roi.C1, &wr->saved_roi.C1, voxel_info );
	ns_to_image_space( &saved_roi.C2, &wr->saved_roi.C2, voxel_info );

	/* All these tests are technically not necessary if the math is correct, but
		lets be super paranoid and check anyway. */

	wr->update_roi.C1.x = NS_CLAMP( wr->update_roi.C1.x, 0, max.x );
	wr->update_roi.C1.y = NS_CLAMP( wr->update_roi.C1.y, 0, max.y );
	wr->update_roi.C1.z = NS_CLAMP( wr->update_roi.C1.z, 0, max.z );
	wr->update_roi.C2.x = NS_CLAMP( wr->update_roi.C2.x, 0, max.x );
	wr->update_roi.C2.y = NS_CLAMP( wr->update_roi.C2.y, 0, max.y );
	wr->update_roi.C2.z = NS_CLAMP( wr->update_roi.C2.z, 0, max.z );

	if( wr->update_roi.C1.x > wr->update_roi.C2.x )NS_SWAP( nsint, wr->update_roi.C1.x, wr->update_roi.C2.x );
	if( wr->update_roi.C1.y > wr->update_roi.C2.y )NS_SWAP( nsint, wr->update_roi.C1.y, wr->update_roi.C2.y );
	if( wr->update_roi.C1.z > wr->update_roi.C2.z )NS_SWAP( nsint, wr->update_roi.C1.z, wr->update_roi.C2.z );

	wr->saved_roi.C1.x = NS_CLAMP( wr->saved_roi.C1.x, 0, max.x );
	wr->saved_roi.C1.y = NS_CLAMP( wr->saved_roi.C1.y, 0, max.y );
	wr->saved_roi.C1.z = NS_CLAMP( wr->saved_roi.C1.z, 0, max.z );
	wr->saved_roi.C2.x = NS_CLAMP( wr->saved_roi.C2.x, 0, max.x );
	wr->saved_roi.C2.y = NS_CLAMP( wr->saved_roi.C2.y, 0, max.y );
	wr->saved_roi.C2.z = NS_CLAMP( wr->saved_roi.C2.z, 0, max.z );

	if( wr->saved_roi.C1.x > wr->saved_roi.C2.x )NS_SWAP( nsint, wr->saved_roi.C1.x, wr->saved_roi.C2.x );
	if( wr->saved_roi.C1.y > wr->saved_roi.C2.y )NS_SWAP( nsint, wr->saved_roi.C1.y, wr->saved_roi.C2.y );
	if( wr->saved_roi.C1.z > wr->saved_roi.C2.z )NS_SWAP( nsint, wr->saved_roi.C1.z, wr->saved_roi.C2.z );

	ns_to_image_space( &seed, ns_settings_neurite_seed( &wr->settings ), voxel_info );

	seed.x = NS_CLAMP( seed.x, wr->update_roi.C1.x, wr->update_roi.C2.x );
	seed.y = NS_CLAMP( seed.y, wr->update_roi.C1.y, wr->update_roi.C2.y );
	seed.z = NS_CLAMP( seed.z, wr->update_roi.C1.z, wr->update_roi.C2.z );

	____xy_slice_index = ( nssize )NS_TO_IMAGE_SPACE( xy_slice_index, ns_voxel_info_one_over_size_z( voxel_info ) );
	____xy_slice_index = ( nssize )NS_CLAMP( ( nsint )____xy_slice_index, wr->update_roi.C1.z, wr->update_roi.C2.z );

	if( eDISPLAY_WINDOW_MOUSE_MODE_SEED == s_CurrentMouseMode )
		_show_seed_on_status_bar();
	else if( eDISPLAY_WINDOW_MOUSE_MODE_ROI == s_CurrentMouseMode )
		_do_show_roi_on_status_bar( *____active_workspace );

	____alert_user_about_splats = NS_TRUE;
	}


void _do_image_processing_subsample( void *dialog )
   {
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
	NsImage           resized;
	NsError           error;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   record = _workspace_create_record( wr, ns_pixel_proc_subsample()->title, NULL );

	ns_image_construct( &resized );
	ns_image_set_pixel_proc_db( &resized, &wr->ppdb );

	if( NS_FAILURE(
			ns_image_subsample(
				&wr->volume,
				&resized,
				1,
				wr->subsample_scale.x,
				wr->subsample_scale.y,
				wr->subsample_scale.z,
				&nsprogress,
				record
			),
			error ) )
		{
      _workspace_remove_last_record( wr, record );
		ns_image_destruct( &resized );
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
		}

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
		ns_image_destruct( &resized );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

	ns_image_clear( &wr->volume );

	ns_image_buffer_and_free(
		&wr->volume,
		ns_image_pixel_type( &resized ),
		ns_image_width( &resized ),
		ns_image_height( &resized ),
		ns_image_length( &resized ),
		ns_image_row_align( &resized ),
		ns_image_pixels( &resized )
		);

	ns_image_release( &resized );
	ns_image_destruct( &resized );

	ns_image_clear( &wr->proj_xy );
	ns_image_clear( &wr->proj_zy );
	ns_image_clear( &wr->proj_xz );
	ns_image_clear( &wr->slice_xy );

	if( NS_FAILURE( _workspace_allocate_projections( wr ), error ) )
		{
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
		}

	_workspace_reset_on_volume_size_change( wr );

	if( ____xy_slice_enabled )
		if( NS_FAILURE( _workspace_do_init_slice_view( wr ), error ) )
			{
			EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
			return;
			}

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   _do_redraw_workspace_display_graphics( wr );
   EndProgressDialog( dialog, eNO_ERROR );
   }


#include "nssubsample.h"

eERROR_TYPE image_processing_subsample( HWND wnd, const HWorkspace handle )
	{
   NsWorkspace  *wr;
	nsint         old_width, old_height, old_length;
	nsint         new_width, new_height, new_length;


	wr = _ns_workspace( handle );

	if( ns_subsample_dialog( g_Instance, wnd, &wr->volume, &wr->subsample_scale ) )
		{
		/* If all scale factors are 100%, then no subsampling should occur. */
		if( wr->subsample_scale.x < 100.0 ||
			 wr->subsample_scale.y < 100.0 ||
			 wr->subsample_scale.z < 100.0   )
			{
			old_width  = ( nsint )ns_image_width( &wr->volume );
			old_height = ( nsint )ns_image_height( &wr->volume );
			old_length = ( nsint )ns_image_length( &wr->volume );

			SubsamplerSizes(
				old_width,
				old_height,
				old_length,
				wr->subsample_scale.x / 100.0,
				wr->subsample_scale.y / 100.0,
				wr->subsample_scale.z / 100.0,
				&new_width,
				&new_height,
				&new_length
				);

			/* If the amount of pixels hasnt changed, then no subsampling should occur. */
			if( new_width  != old_width  ||
				 new_height != old_height ||
				 new_length != old_length   )
				return image_processing( wnd, handle, _do_image_processing_subsample ); 
			}
		}
	
	return eNO_ERROR;
	}


void _read_gamma_correct( nsdouble *gamma )
   {
   NsConfigDb  db;
	NsError     error;


	*gamma = 1.0;

   if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return;
		}

   *gamma = ns_config_db_get_double( &db, "gamma", "value" );

   ns_config_db_destruct( &db );
   }

#include <image/nspixels-gammacorrect.h>

void _do_image_processing_gamma_correct( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
   nsdouble          gamma;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   _read_gamma_correct( &gamma );
   record = _workspace_create_record( wr, ns_pixel_proc_gamma_correct()->title, NULL );

   ns_image_gamma_correct(
		&wr->volume,
		_ns_pixel_region_from_roi( &region, &wr->saved_roi ),
		gamma,
		&nsprogress,
		record
		);

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_gamma_correct( HWND hWnd, const HWorkspace handle )
   {  return image_processing( hWnd, handle, _do_image_processing_gamma_correct );  }







/*
void _do_regenerate_projections( void *dialog )
   {
   NsProgress     nsprogress;
   Progress       progress;
   NsWorkspace *wr;
   nsint           *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }
*/


eERROR_TYPE regenerate_projections( HWND hWnd, const HWorkspace handle )
   {
	return _workspace_run_projections( _ns_workspace( handle ) );
	//return image_processing( hWnd, handle, _do_regenerate_projections );
	}



/*
void _write_dynamic_range( const nsfloat *channel_min, const nsfloat *channel_max )
   {
   NsConfigDb db;

   if( NS_FAILURE( ns_config_db_construct( &db );
   ns_config_db_read( &db, ____config_file, NULL );

   ns_config_db_set_double( &db, "dynamic_range", "min", ( nsdouble )channel_min[ NS_PIXEL_CHANNEL_LUM ] );
   ns_config_db_set_double( &db, "dynamic_range", "max", ( nsdouble )channel_max[ NS_PIXEL_CHANNEL_LUM ] );

   ns_config_db_write( &db, ____config_file );
   ns_config_db_destruct( &db );
   }
*/



//extern nsboolean ____ns_config_db_verbose;

//[dynamic_range]
//min=0.000000
//max=255.000000

//void _read_dynamic_range( NsWorkspace *wr, nsfloat *channel_min, nsfloat *channel_max )
  // {
   //NsConfigDb  db;
	//NsError     error;


	//*channel_min = 0.0f;
	//*channel_max = 0.0f;

   //if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
	//	return;

//ns_println( "config file = %s", ____config_file );
   //if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
	//	{
	//	ns_config_db_destruct( &db );
	//	return;
	//	}

   //(*channel_min)/*[ NS_PIXEL_CHANNEL_LUM ]*/ = ( nsfloat )ns_config_db_get_double( &db, "dynamic_range", "min" );
   //(*channel_max)/*[ NS_PIXEL_CHANNEL_LUM ]*/ = ( nsfloat )ns_config_db_get_double( &db, "dynamic_range", "max" );

   /* TEMP Hack! 8-bit to X-bit conversions. */
   //if( NS_PIXEL_LUM_U12 == ns_image_pixel_type( &wr->volume ) )
     // {
      ///* 8-bit to 12-bit. */
      //(*channel_min)/*[ NS_PIXEL_CHANNEL_LUM ]*/ *= 16.0f;
      //(*channel_max)/*[ NS_PIXEL_CHANNEL_LUM ]*/ *= 16.0f;
      //}
   //else if( NS_PIXEL_LUM_U16 == ns_image_pixel_type( &wr->volume ) )
     // {
      /* 8-bit to 16-bit. */
      //(*channel_min)/*[ NS_PIXEL_CHANNEL_LUM ]*/ *= 256.0f;
      //(*channel_max)/*[ NS_PIXEL_CHANNEL_LUM ]*/ *= 256.0f;
      //}

//ns_println( "about to destruct configdb..." );
//____ns_config_db_verbose = NS_TRUE;
   //ns_config_db_destruct( &db );
//____ns_config_db_verbose = NS_FALSE;
//ns_println( "done destructing configdb..." );
   //}


#include <image/nspixels-dynamicrange.h>
#include "nsdynamicrange.inl"

/* TEMP??? Must be global for record to work. */
//nsfloat ____channel_min[ NS_PIXEL_NUM_CHANNELS ];
//nsfloat ____channel_max[ NS_PIXEL_NUM_CHANNELS ];

void _do_image_processing_dynamic_range( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsClosureRecord  *record;
   NsWorkspace      *wr;
   nsint            *notUsed;
   //nsfloat           channel_min, channel_max;
	NsPixelRegion     region;
	NsPixelType       pixel_type;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   //_hack_image( &image, &wr->ppdb, ____volume_pixels,
   //   ____volume_width, ____volume_height, ____volume_length,
   //   ____volume_bpp );

//ns_println( "read dynamic range params..." );

	pixel_type = ns_image_pixel_type( &wr->volume );

	wr->channel_min = ns_voxel_intensity_convert( wr->channel_min, NS_PIXEL_LUM_U8, pixel_type );
	wr->channel_max = ns_voxel_intensity_convert( wr->channel_max, NS_PIXEL_LUM_U8, pixel_type );

	/* TEMP */
	ns_println( "channel_min = " NS_FMT_DOUBLE, wr->channel_min );
	ns_println( "channel_max = " NS_FMT_DOUBLE, wr->channel_max );

   //_read_dynamic_range( wr, /*____*/&channel_min, /*____*/&channel_max );

   record = _workspace_create_record( wr, ns_pixel_proc_dynamic_range()->title, NULL );

//ns_println( "dynamic range... " );

//____ns_value_print_invoke = NS_TRUE;

   ns_image_dynamic_range(
		&wr->volume,
		_ns_pixel_region_from_roi( &region, &wr->saved_roi ),
		/*____*/wr->channel_min,
		/*____*/wr->channel_max,
		&nsprogress,
		record
		);

//____ns_value_print_invoke = NS_FALSE;

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

//ns_println( "write dynamic range params..." );
//   _write_dynamic_range( channel_min, channel_max );

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_dynamic_range( HWND hWnd, const HWorkspace handle )
   {
   NsWorkspace *wr = _ns_workspace( handle );

	if( ns_dynamic_range_dialog( g_Instance, hWnd, &wr->channel_min, &wr->channel_max ) )
		return image_processing( hWnd, handle, _do_image_processing_dynamic_range );	

	return eNO_ERROR;

	//return image_processing( hWnd, handle, _do_image_processing_dynamic_range );
	}


nsint ____mp_filter_type;
nssize ____mp_filter_num_cpu;

void _do_image_processing_mp_filter( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   //_hack_image( &image, &wr->ppdb, ____volume_pixels,
   //   ____volume_width, ____volume_height, ____volume_length,
   //   ____volume_bpp );

   record = _workspace_create_record( wr, ns_pixel_proc_mp_filter_type_to_string( ____mp_filter_type ), NULL );

   ns_image_mp_filter(
		&wr->volume,
		_ns_pixel_region_from_roi( &region, &wr->saved_roi ),
		____mp_filter_type,
		____mp_filter_num_cpu,
		&nsprogress,
		record
		);

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }

eERROR_TYPE image_processing_mp_filter( HWND hWnd, const HWorkspace handle, nsint type, nssize num_cpu )
   {
   ____mp_filter_type = type;
   ____mp_filter_num_cpu = num_cpu;

   return image_processing( hWnd, handle, _do_image_processing_mp_filter );
   }


#include <image/nspixels-blur.h>

void _do_image_processing_blur( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   //_hack_image( &image, &wr->ppdb, ____volume_pixels,
   //   ____volume_width, ____volume_height, ____volume_length,
   //   ____volume_bpp );

   record = _workspace_create_record( wr, ns_pixel_proc_blur()->title, NULL );

   ns_image_blur(
		&wr->volume,
		_ns_pixel_region_from_roi( &region, &wr->saved_roi ),
		&wr->volume,
		&nsprogress,
		record
		);

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_blur( HWND hWnd, const HWorkspace handle )
   {  return image_processing( hWnd, handle, _do_image_processing_blur );  }


void _do_image_processing_blur_more( void *dialog )
   {
   //NsImage         image;
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   NsClosureRecord  *record;
   nsint            *notUsed;
	NsPixelRegion     region;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   //_hack_image( &image, &wr->ppdb, ____volume_pixels,
   //   ____volume_width, ____volume_height, ____volume_length,
   //   ____volume_bpp );

   record = _workspace_create_record( wr, /* TEMP??? Special case! */"Blur More", NULL );

	_ns_pixel_region_from_roi( &region, &wr->saved_roi );

   /* Do it twice. Only pass the record to the second one though. */
   ns_image_blur( &wr->volume, &region, &wr->volume, &nsprogress, NULL );
   ns_image_blur( &wr->volume, &region, &wr->volume, &nsprogress, record );

   if( ns_progress_cancelled( &nsprogress ) )
      {
      _workspace_remove_last_record( wr, record );
      EndProgressDialog( dialog, eNO_ERROR );
      return;
      }

   if( eNO_ERROR != _workspace_run_projections( wr ) )
      {
      EndProgressDialog( dialog, g_Error );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE image_processing_blur_more( HWND hWnd, const HWorkspace handle )
   {  return image_processing( hWnd, handle, _do_image_processing_blur_more );  }




//NS_PRIVATE void _workspace_lock_tiles( NsWorkspace *wr )
//	{  ns_mutex_lock( &wr->tile_mutex );  }

//void workspace_lock_tiles( HWorkspace handle )
//	{  _workspace_lock_tiles( _ns_workspace( handle ) );  }

//NS_PRIVATE void _workspace_unlock_tiles( NsWorkspace *wr )
//	{  ns_mutex_unlock( &wr->tile_mutex );  }

//void workspace_unlock_tiles( HWorkspace handle )
//	{  _workspace_unlock_tiles( _ns_workspace( handle ) );  }



/*
review below. we dont need an allocated tile each time? create onle local to this procedure and make it growable only.
that will be thread safe. retest!

create a copy of the palette in this procedure that will be thread safe. check using the wr->palette which is
kind of global? retest!
*/

/*
void _workspace_draw_tiles( NsWorkspace *wr, const NsTileMgr *mgr, const NsImage *proj )
	{
	nssize          i, n;
	NsTile         *tile;
	NsPixelRegion   region;
	NsImage         image;
	NsError         error;
	

	_workspace_lock_tiles( wr );

	n = ns_tile_mgr_total_num_tiles( mgr );

	for( i = 0; i < n; ++i )
		{
		tile = ns_tile_mgr_at( mgr, i );

		ns_image_construct( &image );
		ns_image_set_pixel_proc_db( &image, &wr->ppdb );

		region.x      = ns_tile_get_pixel_x( tile );
		region.y      = ns_tile_get_pixel_y( tile );
		region.z      = ns_tile_get_pixel_z( tile );
		region.width  = ns_tile_width( tile );
		region.height = ns_tile_height( tile );
		region.length = ns_tile_length( tile );

		ns_assert( region.x < ns_image_width( proj ) );
		ns_assert( region.y < ns_image_height( proj ) );
		ns_assert( 0 == region.z );
		ns_assert( region.width <= ns_image_width( proj ) );
		ns_assert( region.height <= ns_image_height( proj ) );
		ns_assert( 1 == region.length );
		ns_assert( region.x + region.width <= ns_image_width( proj ) );
		ns_assert( region.y + region.height <= ns_image_height( proj ) );
		ns_assert( region.z + region.length <= ns_image_length( proj ) );

		if( NS_SUCCESS( ns_image_assign( &image, proj, &region, NULL ), error ) )
			DrawImageIntoMemoryGraphics( ns_tile_get_object( tile ), &image, &wr->palette );

		ns_image_destruct( &image );
		}

	_workspace_unlock_tiles( wr );
	}*/


//void _workspace_draw_tiles( NsWorkspace *wr, const NsTileMgr *mgr, const NsImage *proj )
//	{
	//nssize          i, n;
	//NsTile         *tile;
	//NsPixelRegion   region;
	//NsImage         image;
	//NsError         error;
	

	//_workspace_lock_tiles( wr );

//NS_PRIVATE nsulong ____draw_tiles_current_thread = 0;
//if( 0 != ____draw_tiles_current_thread )
//ns_println( NS_FUNCTION " IS NOT MULTI-THREAD SAFE!!!" );
//____draw_tiles_current_thread = ns_thread_id();


//	if( /*____workspace_has_tiles &&*/ 1/*0 < ____workspace_async_proj_iterations*/ )
//		{
	//	n = ns_tile_mgr_total_num_tiles( mgr );

	//	for( i = 0; i < n; ++i )
	//		{
	//		tile = ns_tile_mgr_at( mgr, i );

		//	ns_tile_lock( tile );

	//		ns_image_construct( &image );
	//		ns_image_set_pixel_proc_db( &image, &wr->ppdb );

	//		region.x      = ns_tile_get_pixel_x( tile );
	//		region.y      = ns_tile_get_pixel_y( tile );
	//		region.z      = ns_tile_get_pixel_z( tile );
	//		region.width  = ns_tile_width( tile );
	//		region.height = ns_tile_height( tile );
	//		region.length = ns_tile_length( tile );

	//		ns_assert( region.x < ns_image_width( proj ) );
	//		ns_assert( region.y < ns_image_height( proj ) );
	//		ns_assert( 0 == region.z );
	//		ns_assert( region.width <= ns_image_width( proj ) );
	//		ns_assert( region.height <= ns_image_height( proj ) );
	//		ns_assert( 1 == region.length );
	//		ns_assert( region.x + region.width <= ns_image_width( proj ) );
	//		ns_assert( region.y + region.height <= ns_image_height( proj ) );
	//		ns_assert( region.z + region.length <= ns_image_length( proj ) );

			//ns_image_create( &image, NS_PIXEL_LUM_U8, region.width, region.height, region.length, ns_image_row_align( proj ) );
	//		if( NS_SUCCESS( ns_image_assign( &image/*&wr->tile*/, proj, &region, NULL ), error ) )
	//			{
	//			DrawImageIntoMemoryGraphics( ns_tile_get_object( tile ), &image/*&wr->tile*/, &wr->palette );
	//			}

	//		ns_image_destruct( &image );

		//	ns_tile_unlock( tile );
	//		}
//		}

	//_workspace_unlock_tiles( wr );

//____draw_tiles_current_thread = 0;
	//}



/*
void _workspace_draw_graphics
   (
   NsWorkspace     *wr,
   MemoryGraphics  *graphics,
   const NsImage   *image
   )
   {
	ns_assert( NULL != image );
	DrawImageIntoMemoryGraphics( graphics, image, &wr->palette );
	}
*/


NS_PRIVATE void _workspace_do_set_graphics_xy( NsWorkspace *wr )
	{
	//if( ____xy_slice_enabled )
	//	_workspace_draw_tiles( wr, &wr->tile_mgr_slice, &wr->slice_xy );
	//else
	//	_workspace_draw_tiles( wr, &wr->tile_mgr_xy, &wr->proj_xy );

	//_workspace_draw_graphics( wr, &wr->gc_xy, ____xy_slice_enabled ? &wr->slice_xy : &wr->proj_xy );
	}


void _do_redraw_workspace_display_graphics( NsWorkspace *wr )
   {
	//_workspace_do_set_graphics_xy( wr );

	//_workspace_draw_tiles( wr, &wr->tile_mgr_zy, &wr->proj_zy );
	//_workspace_draw_tiles( wr, &wr->tile_mgr_xz, &wr->proj_xz );

   //_workspace_draw_graphics( wr, &wr->gc_xy, ____xy_slice_enabled ? &wr->slice_xy : &wr->proj_xy );
   //_workspace_draw_graphics( wr, &wr->gc_zy, &wr->proj_zy );
   //_workspace_draw_graphics( wr, &wr->gc_xz, &wr->flipped_xz /*&wr->proj_xz*/ );
   }


void workspace_set_graphics_xy( HWorkspace handle )
	{  _workspace_do_set_graphics_xy( _ns_workspace( handle ) );  }


void RedrawWorkspaceDisplayGraphics( const HWorkspace handle )
	{  _do_redraw_workspace_display_graphics( _ns_workspace( handle ) );  }












MemoryGraphics* GetWorkspaceDisplayGraphics( const HWorkspace handle, const nsint display )
   {
	/*
   switch( display )
      {
      case NS_XY:
         return &( _ns_workspace( handle )->gc_xy );

      case NS_ZY:
         return &( _ns_workspace( handle )->gc_zy ); 

      case NS_XZ:
         return &( _ns_workspace( handle )->gc_xz );

      default:
         ns_assert( 0 );
      }
	*/

   return NULL;
   }


const NsTileMgr* workspace_tile_mgr( HWorkspace handle, nsint display )
	{
   switch( display )
      {
      case NS_XY:
         return ____xy_slice_enabled ?
						&( _ns_workspace( handle )->tile_mgr_slice )
							:
						&( _ns_workspace( handle )->tile_mgr_xy );

      case NS_ZY:
         return &( _ns_workspace( handle )->tile_mgr_zy ); 

      case NS_XZ:
         return &( _ns_workspace( handle )->tile_mgr_xz );

      default:
         ns_assert_not_reached();
      }

   return NULL;
	}


const NsShollAnalysis* workspace_sholl_analysis( HWorkspace handle )
	{  return &( _ns_workspace( handle )->sholl_analysis );  }


const NsImage* workspace_dataset( HWorkspace handle )
	{  return &( _ns_workspace( handle )->volume );  }


const NsImage* GetWorkspaceDisplayImage( const HWorkspace handle, const nsint display )
   {
   switch( display )
      {
      case NS_XY:
         return &( _ns_workspace( handle )->proj_xy );

      case NS_ZY:
         return &( _ns_workspace( handle )->proj_zy ); 

      case NS_XZ:
         return &( _ns_workspace( handle )->proj_xz );

      default:
         ns_assert( 0 );
      }

   return NULL;
   }


const NsImage* workspace_slice_image( const HWorkspace handle )
	{  return &( _ns_workspace( handle )->slice_xy );  }


/*
nsuchar workspace_get_pixel( nsuint handle, nsint display, nssize x, nssize y )
   {
   const Image *image;

   image = GetWorkspaceDisplayImage( handle, display );

   return *( ( ( nsuchar* )image->pixels ) + y * image->width + x );
   }
*/


NsPixelType workspace_pixel_type( nsuint handle )
	{  return ns_image_pixel_type( &( _ns_workspace( handle )->volume ) );  }


const Palette* GetWorkspacePalette( const HWorkspace handle )
   {  return &( _ns_workspace( handle )->palette );  }


const Palette* workspace_get_inverted_palette( HWorkspace handle )
   {  return &( _ns_workspace( handle )->inverted_palette );  }


const nschar* GetWorkspacePath( const HWorkspace handle )
   {  return _ns_workspace( handle )->path;  }


nsfloat GetWorkspaceZoom( const HWorkspace handle, const nsuint window, const nsint display )
   {
   ns_assert( window < mNUM_DISPLAY_WINDOWS );
   ns_assert( display < mNUM_DISPLAYS );
   return _ns_workspace( handle )->zoom[ window ][ display ];
   }


void SetWorkspaceZoom( const HWorkspace handle, const nsuint window, const nsint display,
                       const nsfloat zoom )
   {
   ns_assert( window < mNUM_DISPLAY_WINDOWS );
   ns_assert( display < mNUM_DISPLAYS );
   _ns_workspace( handle )->zoom[ window ][ display ] = zoom;
   }


Vector2i GetWorkspaceCorner( const HWorkspace handle, const nsuint window, const nsint display )
   {
   ns_assert( window < mNUM_DISPLAY_WINDOWS );
   ns_assert( display < mNUM_DISPLAYS );
   return _ns_workspace( handle )->corner[ window ][ display ];
   }


void SetWorkspaceCorner( const HWorkspace handle, const nsuint window, const nsint display,
                         const Vector2i corner )
   {
   ns_assert( window < mNUM_DISPLAY_WINDOWS );
   ns_assert( display < mNUM_DISPLAYS );
   _ns_workspace( handle )->corner[ window ][ display ] = corner;
   }


NsVector4ub workspace_get_color( HWorkspace handle, nsint which )
   {
	NsColor4ub C;

   ns_assert( which < _WORKSPACE_NUM_COLORS );
   C = _ns_workspace( handle )->colors[ which ];

	NS_COLOR_SET_ALPHA_U8( C, 255 );
	return C;
   }


void workspace_set_color( HWorkspace handle, nsint which, NsVector4ub color )
   {
   ns_assert( which < _WORKSPACE_NUM_COLORS );
   _ns_workspace( handle )->colors[ which ] = color;
   }



void workspace_roi_info( HWorkspace hwsp, NsRoiInfo *info )
	{
	const NsCubei  *roi;
	const NsImage  *volume;
	nssize          volume_width, volume_height, volume_length;
	nsdouble        roi_size, volume_size;


	roi    = workspace_update_roi( hwsp );
	volume = workspace_volume( hwsp );

	info->width  = ( nssize )ns_cubei_width( roi );
	info->height = ( nssize )ns_cubei_height( roi );
	info->length = ( nssize )ns_cubei_length( roi );

	info->x1 = ( nssize )roi->C1.x;
	info->y1 = ( nssize )roi->C1.y;
	info->z1 = ( nssize )roi->C1.z;
	info->x2 = ( nssize )roi->C2.x;
	info->y2 = ( nssize )roi->C2.y;
	info->z2 = ( nssize )roi->C2.z;

	volume_width  = ns_image_width( volume );
	volume_height = ns_image_height( volume );
	volume_length = ns_image_length( volume );

	info->percent_x = ( nsdouble )info->width  / ( nsdouble )volume_width * 100.0;
	info->percent_y = ( nsdouble )info->height / ( nsdouble )volume_height * 100.0;
	info->percent_z = ( nsdouble )info->length / ( nsdouble )volume_length * 100.0;

	roi_size    = ( nsdouble )info->width * ( nsdouble )info->height * ( nsdouble )info->length;
	volume_size = ( nsdouble )volume_width * ( nsdouble )volume_height * ( nsdouble )volume_length;

	info->total_percent = roi_size / volume_size * 100.0;
	}


NsVector4ub workspace_get_spine_color( HWorkspace handle )
   {  return workspace_get_color( handle, WORKSPACE_COLOR_ORIGIN_VERTEX );  }


NS_PRIVATE const NsVoxelInfo* ____workspace_get_voxel_info( NsWorkspace *wr )
   {  return ns_settings_voxel_info( &wr->settings );  }


const NsVoxelInfo* workspace_get_voxel_info( HWorkspace handle )
   {  return ____workspace_get_voxel_info( _ns_workspace( handle ) );  }


void workspace_set_voxel_info( HWorkspace handle, nsfloat x, nsfloat y, nsfloat z )
   {
   NsWorkspace *wr = _ns_workspace( handle );
   ns_voxel_info( ns_settings_voxel_info( &wr->settings ), x, y, z );
   }


NsSettings* workspace_settings( HWorkspace handle )
   {  return &_ns_workspace( handle )->settings;  }


//nsulong* workspace_setting_flags( HWorkspace handle )
  // {  return &_ns_workspace( handle )->setting_flags;  }


void SetWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag )
   {
   NsWorkspace *wr = _ns_workspace( handle );
   wr->displayFlags |= flag;
   }


void ClearWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag )
   {
   NsWorkspace *wr = _ns_workspace( handle );
   wr->displayFlags &= ~flag;
   }


void ToggleWorkspaceDisplayFlag( const HWorkspace handle, const nsuint flag )
   {
   NsWorkspace *wr = _ns_workspace( handle );

   if( ( wr->displayFlags & flag ) )
      wr->displayFlags &= ~flag;
   else
      wr->displayFlags |= flag;
   }


nsint WorkspaceDisplayFlagIsSet( const HWorkspace handle, const nsuint flag )
   {
   NsWorkspace *wr = _ns_workspace( handle );
   return ( wr->displayFlags & flag ) ? 1 : 0;
   }


nsint CallTreeStats( NsModel *neuron, const nschar* file_name )
   {
   NsModelStats  stats;

   ns_model_stats_construct( &stats, neuron, NULL );
   ns_model_stats_write( &stats, file_name );
   ns_model_stats_destruct( &stats );

   return 0;
   }


nsboolean ____return_raw_model = NS_FALSE;

NsModel* GetWorkspaceNeuronTree( const HWorkspace handle )
   {
   if( ____return_raw_model )
      return _ns_workspace( handle )->rawTree;
   else
      return _ns_workspace( handle )->filteredTree;
   }


NsModel* workspace_raw_model( HWorkspace handle )
   {  return _ns_workspace( handle )->rawTree;  }


NsModel* workspace_filtered_model( HWorkspace handle )
   {  return _ns_workspace( handle )->filteredTree;  }


void GetWorkspaceNeuronTrees( const HWorkspace handle, NsModel **raw, NsModel **filtered )
   {
   NsWorkspace *workspace = _ns_workspace( handle );

   *raw = workspace->rawTree;
   *filtered = workspace->filteredTree;
   }


/*
#define __SEEN  ( ( nsushort )0x8000 )

void _ns_voxel_table_mark_seen( NsVoxelTable *voxel_table )
   {
   NsVoxel *voxel;

   NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )
      voxel->flags |= __SEEN;
   }


NS_PRIVATE nsboolean ____ns_voxel_unseen( const NsVoxel *voxel, nspointer user_data )
   {  return ! ( voxel->flags & __SEEN );  }


void _ns_voxel_table_remove_unseen( NsVoxelTable *voxel_table )
   {
   ns_assert( NULL != voxel_table );
   ns_voxel_table_clean( voxel_table, ____ns_voxel_unseen, NULL );
   }
*/


NsError workspace_pre_build_model( NsWorkspace *wr )
   {
   //NsError error = ns_no_error();

   //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
      //if( NS_FAILURE( ns_model_assign( wr->temp_raw_tree, wr->rawTree ), error ) )
        // return error;//goto _WORKSPACE_PRE_BUILD_MODEL_ON_ERROR;

   //if( wr->setting_flags & NS_SETTINGS_FILTER_NEURITES )
      //if( NS_FAILURE( ns_model_assign( wr->temp_filtered_tree, wr->filteredTree ), error ) )
        // return error;//goto _WORKSPACE_PRE_BUILD_MODEL_ON_ERROR;

   //goto _WORKSPACE_PRE_BUILD_MODEL_NO_ERROR;

   //_WORKSPACE_PRE_BUILD_MODEL_ON_ERROR:

   //ns_model_clear( wr->temp_raw_tree );
   //ns_model_clear( wr->temp_filtered_tree );

   //_WORKSPACE_PRE_BUILD_MODEL_NO_ERROR:

   //return error;
	return ns_no_error();
   }


void workspace_post_build_model( NsWorkspace *wr, nsboolean undo )
   {
   if( undo )
      {
      //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
         {
         //ns_model_clear( wr->rawTree );

         //ns_model_lock( wr->rawTree );
			//ns_model_lock( wr->temp_raw_tree );
         //NS_SWAP( NsModel*, wr->rawTree, wr->temp_raw_tree );
         //ns_model_unlock( wr->rawTree );
			//ns_model_unlock( wr->temp_raw_tree );
         }

      //if( wr->setting_flags & NS_SETTINGS_FILTER_NEURITES )
         {
         //ns_model_clear( wr->filteredTree );

         //ns_model_lock( wr->filteredTree );
			//ns_model_lock( wr->temp_filtered_tree );
         //NS_SWAP( NsModel*, wr->filteredTree, wr->temp_filtered_tree );
         //ns_model_unlock( wr->filteredTree );
			//ns_model_unlock( wr->temp_filtered_tree );
         }

      //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
        // _ns_voxel_table_remove_unseen( &wr->voxel_table );
      }
   else
      {
      //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
         //ns_model_clear( wr->temp_raw_tree );

      //if( wr->setting_flags & NS_SETTINGS_FILTER_NEURITES )
         //ns_model_clear( wr->temp_filtered_tree );

      //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
        // _ns_voxel_table_mark_seen( &wr->voxel_table );
      }
   }


/*
extern void ____redraw_2d( void );
extern void ____redraw_3d( void );

void ____ns_model_graft_render_func( void )
   {
   ____redraw_2d();
   ____redraw_3d();
   }
*/


extern void ____redraw_all( void );



nsfloat ____min_confidence;
nsfloat ____update_rate;


/* TEMP!!!!!!!!!! **************************/

void ____load_graft_settings_hard_coded( void )
   {
   ____min_confidence = 0.2f;
   ____update_rate    = 0.5f;
   }

//void ____load_graft_settings_from_file( void )
  // {
   //FILE    *fp;
   //nschar   path[ 1024 ];

  // ____load_graft_settings_hard_coded();

   //ns_sprint( path, "%s\\graft.txt", _startup_directory );

   //if( NULL != ( fp = fopen( path, "r" ) ) )
     // {
      //fscanf( fp, "%f%f", &____min_confidence, &____update_rate );
      //fclose( fp );
      //}
   //}

void ____load_graft_settings( void )
   {
   ____load_graft_settings_hard_coded();

   ns_println( "min confidence = " NS_FMT_DOUBLE, ____min_confidence );
   ns_println( "update rate    = " NS_FMT_DOUBLE, ____update_rate );
   }



NsError _workspace_do_model_filtering
	(
	NsWorkspace       *wr,
	const NsSettings  *settings,
	NsProgress        *progress,
	nsboolean          write_settings
	)
	{
	NsModelFilterParams  params;
	NsError              error;


	ns_model_clear_origins( wr->rawTree );

	if( NS_FAILURE( ns_model_create_origins( wr->rawTree, progress ), error ) )
		return error;

	/* IMPORTANT: It is essential that any seeds that were previously passed from the
		raw model to the filtered model are cleared first. */
	ns_model_clear_seeds( wr->filteredTree );

	ns_model_clear_non_manually_traced( wr->filteredTree );

	if( NS_FAILURE( ns_model_merge( wr->filteredTree, wr->rawTree ), error ) )
		return error;

ns_println( "# of origins = " NS_FMT_ULONG, ns_model_num_origins( wr->filteredTree ) );

	//if( ____run_model_filtering )
	if( ns_grafting_do_filter() )
		{
		params.volume                    = &wr->volume;
		params.proj_xy                   = &wr->proj_xy;
		params.proj_zy                   = &wr->proj_zy;
		params.proj_xz                   = &wr->proj_xz;
		params.volume_average_intensity  = wr->average_intensity;
		params.grafting_use_2d_sampling  = wr->grafting_use_2d_sampling;
		params.grafting_aabbox_scalar    = wr->grafting_aabbox_scalar;
		params.grafting_min_window       = wr->grafting_min_window;
		params.reposition_iterations     = 50;
		params.rayburst_interp_type      = ____neurites_interp_type;

		ns_model_set_is_filtering( wr->filteredTree, NS_TRUE );
		error = ns_model_filter( wr->filteredTree, settings, &params, progress );
		ns_model_set_is_filtering( wr->filteredTree, NS_FALSE );

		if( ns_is_error( error ) )
			return error;
		}

	ns_model_set_conn_comp_numbers( wr->filteredTree );

	//ns_model_label_edges(
	//	wr->filteredTree,
	//	ns_settings_get_neurite_labeling_type( settings ),
	//	&nsprogress
	//	);

	//#ifdef NS_DEBUG
	//ns_model_verify( wr->filteredTree );
	//#endif

	if( write_settings )
		/*error*/ns_settings_write_neurite_filtering(
			____settings_file,
			____volume_file_filter,
			settings
			);

	return ns_no_error();
	}


NsError workspace_model_filtering( HWorkspace handle, const NsSettings *settings )
	{  return _workspace_do_model_filtering( _ns_workspace( handle ), settings, NULL, NS_TRUE );  }


void _DoBuildWorkspaceNeuronTree( void *dialog )
   {
   NsWorkspace  *wr;
   nsint        *notUsed;
   NsProgress    nsprogress;
   Progress      progress;
   NsError       error;
	nstimer       start, stop;
	nsboolean     did_run;


   error = ns_no_error();

   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
		&nsprogress,
		__progress_cancelled,
		__progress_update,
		__progress_set_title,
		NULL,
		&progress
      );


/*TEMP*/
/*
ns_println( "" );
ns_println( "" );
if( wr->setting_flags & NS_SETTING_NEURITE_ATTACHMENT_RATIO )
   ns_println( "NS_SETTING_NEURITE_ATTACHMENT_RATIO" );
if( wr->setting_flags & NS_SETTING_NEURITE_MIN_LENGTH )
   ns_println( "NS_SETTING_NEURITE_MIN_LENGTH" );
*/
//if( wr->setting_flags & NS_SETTING_NEURITE_LABELING_TYPE )
//  ns_println( "NS_SETTING_NEURITE_LABELING_TYPE" );
//if( wr->setting_flags & NS_SETTING_NEURITE_DO_3D_RADII )
   //ns_println( "NS_SETTING_NEURITE_DO_3D_RADII" );
//if( wr->setting_flags & NS_SETTING_NEURITE_SEED )
  // ns_println( "NS_SETTING_NEURITE_SEED" );
//if( wr->setting_flags & NS_SETTING_VOXEL_SIZE )
  // ns_println( "NS_SETTING_VOXEL_SIZE" );

ns_println( "" );

ns_println( "settings.voxel_info.size_x = " NS_FMT_DOUBLE,
   ns_voxel_info_size_x( ns_settings_voxel_info( &wr->settings ) ) );

ns_println( "settings.voxel_info.size_y = " NS_FMT_DOUBLE,
   ns_voxel_info_size_y( ns_settings_voxel_info( &wr->settings ) ) );

ns_println( "settings.voxel_info.size_z = " NS_FMT_DOUBLE,
   ns_voxel_info_size_z( ns_settings_voxel_info( &wr->settings ) ) );

ns_println( "settings.neurite.attachment_ratio = " NS_FMT_DOUBLE,
   ns_settings_get_neurite_attachment_ratio( &wr->settings ) );

ns_println( "settings.neurite.min_length = " NS_FMT_DOUBLE,
   ns_settings_get_neurite_min_length( &wr->settings ) );

ns_println( "settings.neurite.use_random_sampling = " NS_FMT_INT,
	ns_settings_get_neurite_use_random_sampling( &wr->settings ) );

ns_println( "settings.neurite.realign_junctions = " NS_FMT_INT,
	ns_settings_get_neurite_realign_junctions( &wr->settings ) );

ns_println( "settings.neurite.labeling_type = " NS_FMT_INT,
   ns_settings_get_neurite_labeling_type( &wr->settings ) );

ns_println( "settings.neurite.do_3d_radii = " NS_FMT_INT,
   ns_settings_get_neurite_do_3d_radii( &wr->settings ) );

ns_println( "settings.neurite.origin_radius_scalar = " NS_FMT_DOUBLE,
   ns_settings_get_neurite_origin_radius_scalar( &wr->settings ) );

ns_println( "settings.neurite.vertex_radius_scalar = " NS_FMT_DOUBLE,
   ns_settings_get_neurite_vertex_radius_scalar( &wr->settings ) );

ns_println( "settings.neurite.smooth_count = " NS_FMT_INT,
   ns_settings_get_neurite_smooth_count( &wr->settings ) );

ns_println( "settings.neurite.seed = " NS_FMT_INT ", " NS_FMT_INT ", " NS_FMT_INT,
   ns_settings_neurite_seed( &wr->settings )->x,
   ns_settings_neurite_seed( &wr->settings )->y,
   ns_settings_neurite_seed( &wr->settings )->z );

ns_println( "settings.threshold.use_fixed = " NS_FMT_INT,
   ns_settings_get_threshold_use_fixed( &wr->settings ) );

ns_println( "settings.threshold.fixed_value = " NS_FMT_DOUBLE,
   ns_settings_get_threshold_fixed_value( &wr->settings ) );

ns_println( "settings.volume.labeling_type = " NS_FMT_STRING_DOUBLE_QUOTED,
	ns_labeling_type_to_string( ns_settings_get_volume_labeling_type( &wr->settings ) ) );

ns_println( "settings.volume.residual_smear_type = " NS_FMT_STRING_DOUBLE_QUOTED,
	ns_residual_smear_type_to_string( ns_settings_get_volume_residual_smear_type( &wr->settings ) ) );

ns_println( "" );


	/* First see if the seed is in the region of interest. */
	if( ! ns_point3i_inside_cube( ns_settings_neurite_seed( &wr->settings ), &wr->update_roi ) )
		{
ns_println( "The SEED is outside of the R.O.I. Cant graft!" );
		EndProgressDialog( dialog, eNO_ERROR );
      return;
		}


/*error!!!*/workspace_pre_build_model( wr );

   //if( wr->setting_flags & NS_SETTINGS_REBUILD_NEURITES )
      {
   //ns_println( "NS_SETTINGS_REBUILD_NEURITES" );


start = ns_timer();

			____return_raw_model = NS_TRUE;

         ____load_graft_settings();

			/*error*/
			ns_grafting_read_params(
				____config_file,
				&wr->grafting_use_2d_sampling,
				&wr->grafting_aabbox_scalar,
				&wr->grafting_min_window
				);


			ns_println( "ABOUT TO GRAFT!!!" );

         if( NS_FAILURE(
					ns_model_graft(
                  wr->rawTree,
						wr->filteredTree,
                  &wr->settings,
                  //&wr->voxel_table,
                  &wr->volume,
						&wr->update_roi,//&wr->saved_roi,
                  ____projected_2d_grafting,
                  &wr->proj_xy,
                  &wr->proj_zy,
                  &wr->proj_xz,
						wr->grafting_first_threshold,
						wr->grafting_first_contrast,
                  ____neurites_interp_type,
                  wr->average_intensity,
                  ____min_confidence,
                  ____update_rate,
                  NULL != wr->hWnd ? ____draw_while_grafting : NS_FALSE,
                  //____use_2d_neurite_sampling,
						wr->grafting_use_2d_sampling,
						wr->grafting_aabbox_scalar,
						wr->grafting_min_window,
                  &nsprogress,
                  ____redraw_all,
						____allow_volume_labeling ?
							ns_settings_get_volume_labeling_type( &wr->settings ) : NS_LABELING_INTRACELLULAR,
						ns_settings_get_volume_residual_smear_type( &wr->settings ),
						&did_run
                  ),
                error ) )
            {
            workspace_post_build_model( wr, NS_TRUE );
            EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );

				____return_raw_model = NS_FALSE;
            return;
            }

		____return_raw_model = NS_FALSE;


stop = ns_timer();
ns_println( "time to build/radii = " NS_FMT_DOUBLE " seconds.", ns_difftimer( stop, start ) );


      //if( ____projected_2d_grafting )
        // ns_model_project_vertices( wr->rawTree, &wr->settings, &wr->volume, &nsprogress );


		if( ! did_run )
			{
			ns_println( "GRAFTING NEVER RAN!!!" );
         workspace_post_build_model( wr, NS_TRUE );
         EndProgressDialog( dialog, eNO_ERROR );
         return;
			}


      //if( IsProgressCancelled() )
        // {
        // workspace_post_build_model( wr, NS_TRUE );
        // EndProgressDialog( dialog, eNO_ERROR );
        // return;
        // }

      //ns_model_clear_origins( wr->rawTree );

      //if( NS_FAILURE( ns_model_create_origins( wr->rawTree, &nsprogress ), error ) )
        // {
        // workspace_post_build_model( wr, NS_TRUE );
        // EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
        // return;
        // }

      //if( IsProgressCancelled() )
        // {
        // workspace_post_build_model( wr, NS_TRUE );
        // EndProgressDialog( dialog, eNO_ERROR );
        // return;
        // }
      
      //#ifdef NS_DEBUG
      //ns_model_verify( wr->rawTree );
      //#endif
      }

	if( NS_FAILURE( _workspace_do_model_filtering( wr, &wr->settings, &nsprogress, NS_FALSE ), error ) )
		{
		workspace_post_build_model( wr, NS_TRUE );
		EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
		return;
		}

   workspace_post_build_model( wr, NS_FALSE );
   //wr->setting_flags = 0;/*&= ~NS_SETTINGS_NEURITES;*/
   EndProgressDialog( dialog, eNO_ERROR );
   }



extern void progress_enable_slider_mode( nsboolean );

eERROR_TYPE BuildWorkspaceNeuronTree( HWND hWnd, const HWorkspace handle )
   {
   NsWorkspace  *wr;
   eERROR_TYPE   error;
   nsint         wasCancelled;


   wr = _ns_workspace( handle );

	progress_enable_slider_mode( NS_TRUE );

   error = ProgressDialog( hWnd,
                           NULL,
                           _DoBuildWorkspaceNeuronTree,
                           wr,
                           NULL,
                           &wasCancelled
                         );

	progress_enable_slider_mode( NS_FALSE );

   if( ! wasCancelled && eNO_ERROR != error )
      return error;
 
   return eNO_ERROR;
   }


extern NsRayburstKernelType ____spines_kernel_type;




extern nsint ____jitter_x;
extern nsint ____jitter_y;
extern nsint ____jitter_z;

void _workspace_do_recompute_manual_spines( void *dialog )
	{
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   nsint            *notUsed;
	NsVector3i        jitter;
	nsboolean         use_2d_sampling;
	nsfloat           aabbox_scalar;
	nsint             min_window;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );
   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

	/*error*/
	ns_model_recompute_manual_spines(
		wr->filteredTree,
		&wr->settings,
		&wr->volume,
		wr->average_intensity,
		&wr->sampler,
		&jitter,
		____measurement_do_3d_rayburst,
		use_2d_sampling,
		aabbox_scalar,
		min_window,
		____spines_interp_type,
		____spines_kernel_type,
		&nsprogress
		);

	____redraw_all();

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_recompute_manual_spines( HWND hWnd, HWorkspace handle )
   {
   NsWorkspace  *wr;
   nsint         wasCancelled;


   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_recompute_manual_spines,
		wr,
		NULL,
		&wasCancelled
		);
   }


void _workspace_do_recompute_spines_attachment( void *dialog )
	{
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   nsint            *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	/*error*/ns_model_recompute_spines_attachment( wr->filteredTree, &wr->settings, &nsprogress );

	____redraw_all();

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_recompute_spines_attachment( HWND hWnd, HWorkspace handle )
	{
   NsWorkspace  *wr;
   nsint         wasCancelled;


   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_recompute_spines_attachment,
		wr,
		NULL,
		&wasCancelled
		);
	}


NS_PRIVATE NsError _workspace_on_run_sholl_analysis( NsWorkspace *wr, NsProgress *progress )
	{
	NsConfigDb  db;
	nsfloat     min_radius;
	nsfloat     max_radius;
	nsfloat     radius_delta;
	NsError     error;


	if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
		return error;

   if( NS_FAILURE( ns_config_db_read( &db, ____config_file, NULL ), error ) )
		{
		ns_config_db_destruct( &db );
		return error;
		}

	min_radius   = 0.0f;
	max_radius   = 500000.0f;
	radius_delta = 1.0f;

	if( ns_config_db_has_group( &db, "sholl" ) )
		if( ns_config_db_has_key( &db, "sholl", "min_radius" ) )
			min_radius = ( nsfloat )ns_config_db_get_double( &db, "sholl", "min_radius" );

	if( ns_config_db_has_group( &db, "sholl" ) )
		if( ns_config_db_has_key( &db, "sholl", "max_radius" ) )
			max_radius = ( nsfloat )ns_config_db_get_double( &db, "sholl", "max_radius" );

	if( ns_config_db_has_group( &db, "sholl" ) )
		if( ns_config_db_has_key( &db, "sholl", "radius_delta" ) )
			radius_delta = ( nsfloat )ns_config_db_get_double( &db, "sholl", "radius_delta" );

   ns_config_db_destruct( &db );

/*TEMP!!!*/
ns_println( "Sholl Analysis Min Radius   = " NS_FMT_DOUBLE, min_radius );
ns_println( "Sholl Analysis Max Radius   = " NS_FMT_DOUBLE, max_radius );
ns_println( "Sholl Analysis Radius Delta = " NS_FMT_DOUBLE, radius_delta );

	return ns_sholl_analysis_run(
				&wr->sholl_analysis,
				wr->filteredTree,
				min_radius,
				max_radius,
				radius_delta,
				progress
				);
	}


NS_PRIVATE void _workspace_do_run_sholl_analysis( void *dialog )
	{
   NsProgress    nsprogress;
   Progress      progress;
   NsWorkspace  *wr;
   nsint        *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	/*error*/_workspace_on_run_sholl_analysis( wr, &nsprogress );

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_run_sholl_analysis( HWND hWnd, HWorkspace handle )
	{
   NsWorkspace  *wr;
   nsint         wasCancelled;
   OPENFILENAME   ofn;
	nschar         path[ NS_PATH_SIZE ];


   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_run_sholl_analysis,
		wr,
		NULL,
		&wasCancelled
		);

/*TEMP*/ns_println( "Sholl Analysis Done." );

	if( ! wasCancelled )
		{
		ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );
		path[0] = NS_ASCII_NULL;

		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner   = hWnd;
		ofn.lpstrTitle  = "Save Sholl Analysis";
		ofn.lpstrFile   = path;
		ofn.nMaxFile    = NS_PATH_SIZE;
		ofn.lpstrFilter = "Text (*.txt)\0*.txt\0\0\0";
		ofn.lpstrDefExt = ".txt";
		ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if( GetSaveFileName( &ofn ) )
			{
			/*error*/ns_sholl_analysis_write( &wr->sholl_analysis, path );
			/*TEMP*/ns_println( "Saved Sholl Analysis." );
			}
		}

	//ns_sholl_analysis_clear( &wr->sholl_analysis );

	____redraw_2d();
	}


void _workspace_do_select_spines_on_selected_parts( void *dialog )
	{
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   nsint            *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	/*error*/
	ns_model_select_spines_on_selected_parts(
		wr->filteredTree,
		&wr->settings,
		&nsprogress
		);

	____redraw_all();

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_select_spines_on_selected_parts( HWND hWnd, HWorkspace handle )
   {
   NsWorkspace  *wr;
   nsint         wasCancelled;


   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_select_spines_on_selected_parts,
		wr,
		NULL,
		&wasCancelled
		);
   }


nsboolean ____set_soma_distances;

void _workspace_do_set_swc_attachments( void *dialog )
	{
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   nsint            *notUsed;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

	/*error*/
	ns_model_spines_set_swc_attachments(
		wr->filteredTree,
		&wr->settings,
		____set_soma_distances,
		&nsprogress
		);

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_set_swc_attachments( HWND hWnd, HWorkspace handle, nsboolean set_soma_distances )
   {
   NsWorkspace  *wr;
   nsint         wasCancelled;


	____set_soma_distances = set_soma_distances;

   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_set_swc_attachments,
		wr,
		NULL,
		&wasCancelled
		);
   }


void _workspace_do_compute_z_spread( void *dialog )
	{
   NsProgress        nsprogress;
   Progress          progress;
   NsWorkspace      *wr;
   nsint            *notUsed;
	nsboolean         use_2d_sampling;
	nsfloat           aabbox_scalar;
	nsint             min_window;
	nsfloat           z_spread_divisor;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
      &nsprogress,
      __progress_cancelled,
      __progress_update,
      __progress_set_title,
      NULL,
      &progress
      );

   ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

	/*error*/
	ns_model_compute_z_spread_divisor(
		wr->filteredTree,
		&wr->settings,
		&wr->volume,
		&wr->update_roi,
		use_2d_sampling,
		aabbox_scalar,
		min_window,
		&z_spread_divisor,
		&nsprogress
		);

	ns_settings_set_spine_z_spread_divisor( &wr->settings, z_spread_divisor );

   EndProgressDialog( dialog, eNO_ERROR );
	}


void workspace_compute_z_spread( HWND hWnd, HWorkspace handle )
	{
   NsWorkspace  *wr;
   nsint         wasCancelled;


   wr = _ns_workspace( handle );

   ProgressDialog(
		hWnd,
		NULL,
		_workspace_do_compute_z_spread,
		wr,
		NULL,
		&wasCancelled
		);
	}





/*
void _read_eliminate_bases( nsboolean *eliminate_bases )
   {
   NsConfigDb db;
   NsError error;

   *eliminate_bases  = NS_FALSE;

   if( NS_FAILURE( ns_config_db_construct( &db );

   if( NS_SUCCESS( ns_config_db_read( &db, ____config_file, NULL ), error ) )
      {
      if( ns_config_db_has_group( &db, "spine_bases" ) )
         if( ns_config_db_has_key( &db, "spine_bases", "eliminate" ) )
            *eliminate_bases = ns_config_db_get_boolean( &db, "spine_bases", "eliminate" );
      }

   ns_config_db_destruct( &db );
   }
*/


void _workspace_do_run_spine_analysis( void *dialog )
   {
   NsWorkspace  *wr;
   nsint                *notUsed;
   Progress            progress;
   NsProgress          nsprogress;
   NsError             error;
   //nsboolean           eliminate_bases;
   //nsfloat             threshold_factor;



   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

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

   progress_set_title( &progress, "Running spine analysis..." );


ns_println( "" );
ns_println( "settings.voxel_info.size_x = " NS_FMT_DOUBLE,
   ns_voxel_info_size_x( ns_settings_voxel_info( &wr->settings ) ) );
ns_println( "settings.voxel_info.size_y = " NS_FMT_DOUBLE,
   ns_voxel_info_size_y( ns_settings_voxel_info( &wr->settings ) ) );
ns_println( "settings.voxel_info.size_z = " NS_FMT_DOUBLE,
   ns_voxel_info_size_z( ns_settings_voxel_info( &wr->settings ) ) );
ns_println( "settings.spine.max_voxel_distance = " NS_FMT_DOUBLE,
   ns_settings_get_spine_max_voxel_distance( &wr->settings ) );
ns_println( "settings.spine.max_layer_spread   = " NS_FMT_DOUBLE,
   ns_settings_get_spine_max_layer_spread( &wr->settings ) );
ns_println( "settings.spine.elim_height        = " NS_FMT_DOUBLE,
   ns_settings_get_spine_elim_height( &wr->settings ) );

ns_println( "settings.spines.min_attached_voxels = " NS_FMT_ULONG,
	ns_settings_get_spine_min_attached_voxels( &wr->settings ) );

ns_println( "settings.spines.min_detached_voxels = " NS_FMT_ULONG,
	ns_settings_get_spine_min_detached_voxels( &wr->settings ) );

ns_println( "settings.spines.z_spread_divisor    = " NS_FMT_DOUBLE,
	ns_settings_get_spine_z_spread_divisor( &wr->settings ) );

ns_println( "settings.spines.z_spread_automated  = " NS_FMT_STRING,
	ns_settings_get_spine_z_spread_automated( &wr->settings ) ? "true" : "false" );


ns_println( "settings.threshold.multiplier   = " NS_FMT_DOUBLE,
   ns_settings_get_threshold_multiplier( &wr->settings ) );

//ns_println( "settings.threshold.auto_correct = " NS_FMT_STRING,
  // ns_settings_get_threshold_auto_correct( &wr->settings ) ? "true" : "false" );

ns_println( "" );


   //_read_eliminate_bases( &eliminate_bases );


   if( NS_FAILURE( ns_model_spine_analysis(
                     wr->filteredTree,
                     &wr->settings,
                     &wr->volume,
							&wr->update_roi,//&wr->saved_roi,
							&wr->ppdb,
                     ____spines_interp_type,
                     ____use_2d_spine_bounding_box,
                     //eliminate_bases,
                     wr->average_intensity,
                     ____spines_kernel_type,
                     &nsprogress,
                     ____redraw_all,
							____visualize_spine_voxels,
							____config_file,
							ns_settings_get_volume_residual_smear_type( &wr->settings )
                     ),
                     error ) )
      {
		MessageBox( wr->hWnd, ns_error_code_to_string( error ), "NeuronStudio", MB_OK | MB_ICONERROR );
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


eERROR_TYPE workspace_run_spine_analysis( HWND hWnd, const HWorkspace handle )
   {
   NsWorkspace *wr;
   eERROR_TYPE        error;
   nsint                wasCancelled;

	ns_assert( ! ____spine_analysis_running );
	____spine_analysis_running = NS_TRUE;

   wr = _ns_workspace( handle );

   error = ProgressDialog( hWnd,
                           NULL,
                           _workspace_do_run_spine_analysis,
                           wr,
                           NULL,
                           &wasCancelled
                         );

	____spine_analysis_running = NS_FALSE;

   if( ! wasCancelled && eNO_ERROR != error )
      return error;

   return eNO_ERROR;
   }





extern NsRayburstKernelType ____measuring_rays_kernel_type;
extern NsRayburstInterpType ____measurement_interp_type;
extern nsint ____jitter_x;
extern nsint ____jitter_y;
extern nsint ____jitter_z;
extern nsboolean ____launching_neurite_tracer;
//extern nsboolean ____use_2d_measurement_sampling;

NsVector3f ____neurite_tracer_V1;
NsVector3f ____neurite_tracer_V2;

extern void ____redraw_all( void );

void _workspace_do_run_neurite_tracer_render( nspointer user_data )
   {  ____redraw_all();  }

extern void _on_convert_measurements( void );

void _workspace_do_run_neurite_tracer( void *dialog )
   {
   NsWorkspace  *wr;
   nsint              *notUsed;
   const NsSettings   *settings;
   const NsVoxelInfo  *voxel_info;
   NsSampler          *sampler;
   Progress            progress;
   NsProgress          nsprogress;
   nssize              num_created_samples;
   NsVector3i          jitter;
	nsboolean           use_2d_sampling;
	nsfloat             aabbox_scalar;
	nsint               min_window;
   NsError             error;


   ____launching_neurite_tracer = NS_FALSE;

   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

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

   progress_set_title( &progress, "Running neurite tracer..." );

   sampler = &wr->sampler;
   settings = &wr->settings;
   voxel_info = ns_settings_voxel_info( settings );

/*TEMP*/
ns_println( "" );
ns_vector3i( &jitter, ____jitter_x, ____jitter_y, ____jitter_z );
ns_println( "jitter=%d,%d,%d", jitter.x, jitter.y, jitter.z );

	ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

   if( NS_FAILURE(
         ns_sampler_run_ex(
            sampler,
            ns_sampler_last( sampler ),
            &____neurite_tracer_V1,
            &____neurite_tracer_V2,
            wr->filteredTree,
            &wr->volume,
				&wr->update_roi,//&wr->saved_roi,
            ____measurement_do_3d_rayburst,//ns_settings_get_neurite_do_3d_radii( settings ),
            ____measuring_rays_kernel_type,
            ____measurement_interp_type,
            voxel_info,
            wr->average_intensity,
            &jitter,
            //____use_2d_measurement_sampling,
				use_2d_sampling,
				aabbox_scalar,
				min_window,
            ns_settings_get_threshold_use_fixed( settings ),
            ns_settings_get_threshold_fixed_value( settings ),
            &num_created_samples,
            _workspace_do_run_neurite_tracer_render,
            NULL,
            &nsprogress
            ),
         error ) )
      {
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }


   _on_convert_measurements();


   EndProgressDialog( dialog, eNO_ERROR );

   ____redraw_all();
   }


eERROR_TYPE workspace_run_neurite_tracer( HWND hWnd, HWorkspace handle,
   const NsVector3f *V1, const NsVector3f *V2 )
   {
   NsWorkspace *wr;
   eERROR_TYPE        error;
   nsint                wasCancelled;
   

   ____neurite_tracer_V1 = *V1;
   ____neurite_tracer_V2 = *V2;

   wr = _ns_workspace( handle );

   error = ProgressDialog( hWnd,
                           NULL,
                           _workspace_do_run_neurite_tracer,
                           wr,
                           NULL,
                           &wasCancelled
                         );

   if( ! wasCancelled && eNO_ERROR != error )
      return error;

   return eNO_ERROR;
   }





void _workspace_do_run_ann_classifier_retyping( void *dialog )
   {
   NsWorkspace  *wr;
   Progress      progress;
   NsProgress    nsprogress;
   nsint        *notUsed;
   NsError       error;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

   ns_progress(
		&nsprogress,
		 __progress_cancelled,
		 __progress_update,
		 __progress_set_title,
		 NULL,
		 &progress
		);

   progress_set_title( &progress, "Running Artifical Neural Network Classifier..." );

	if( NULL != ns_spines_classifier_get() )
		if( NS_FAILURE(
				ns_model_spines_retype_by_ann_classifier(
					wr->filteredTree,
					ns_spines_classifier_get(),
					&nsprogress
					),
				error ) )
			{
			EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
			return;
			}

   EndProgressDialog( dialog, eNO_ERROR );
   ____redraw_all();
   }


eERROR_TYPE workspace_run_ann_classifier_retyping( HWND wnd, HWorkspace handle )
	{
	NsWorkspace  *wr;
	eERROR_TYPE   error;
	nsint         wasCancelled;


	wr = _ns_workspace( handle );

	progress_enable_slider_mode( NS_TRUE );

	error =
		ProgressDialog(
			wnd,
			NULL,
			_workspace_do_run_ann_classifier_retyping,
			wr,
			NULL,
			&wasCancelled
			);

	progress_enable_slider_mode( NS_FALSE );

	if( ! wasCancelled && eNO_ERROR != error )
		return error;

	return eNO_ERROR;
	}





NsModel* ____establish_thresholds_model = NULL;

void _workspace_do_establish_thresholds_and_contrasts( void *dialog )
   {
   NsWorkspace  *wr;
	NsModel *model;
   nsint                *notUsed;
   Progress            progress;
   NsProgress          nsprogress;
   NsError             error;
	nsboolean     use_2d_sampling;
	nsfloat       aabbox_scalar;
	nsint         min_window;


   g_Progress = dialog;
   GetProgressDialogArgs( dialog, &wr, &notUsed );
   ns_assert( NULL != wr );

	model = ____establish_thresholds_model;
	____establish_thresholds_model = NULL;

   ns_progress( &nsprogress,
                __progress_cancelled,
                __progress_update,
                __progress_set_title,
                NULL,
                &progress
              );

	ns_grafting_read_params( ____config_file, &use_2d_sampling, &aabbox_scalar, &min_window );

   if( NS_FAILURE( ns_grafting_establish_thresholds_and_contrasts( 
                     &wr->settings,
                     &wr->volume,
							NULL, /* dont use the region of interest! */
                     //____use_2d_neurite_sampling,
							use_2d_sampling,
							aabbox_scalar,
							min_window,
                     model,
                     &nsprogress
                     ),
                     error ) )
      {
      EndProgressDialog( dialog, ( g_Error = eERROR_OUT_OF_MEMORY ) );
      return;
      }

   EndProgressDialog( dialog, eNO_ERROR );
   }


void workspace_establish_thresholds_and_contrasts( HWND hWnd, HWorkspace handle, NsModel *model )
   {
   NsWorkspace *wr;
   nsint                wasCancelled;


   wr = _ns_workspace( handle );

	____establish_thresholds_model = model;

   ProgressDialog( hWnd,
                           NULL,
                           _workspace_do_establish_thresholds_and_contrasts,
                           wr,
                           NULL,
                           &wasCancelled
                         );
   }



